#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "ActionPresentationTimeline.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "BalanceTool.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "EffectResourceCatalog.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_Playback.h"
#include "Effect_ThumbnailCache.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include "Model.h"
#include "Profiler.h"
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
#include "EffectAuthoringResourceTree.h"
#include "EffectAuthoringSequencer.h"

Client::CEffect_Tool::CEffect_Tool(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext,
    shared_ptr<CCharacterPreviewPanel> pCharacterPreviewPanel,
	CBalanceTool* const pBalanceTool)
    : m_pDevice(std::move(pDevice)),
      m_pContext(std::move(pContext)),
      m_pThumbnailCache(std::make_unique<CEffectThumbnailCache>(
          m_pDevice, m_pContext)),
      m_pCharacterPreviewPanel(std::move(pCharacterPreviewPanel)),
	  m_pBalanceTool(pBalanceTool),
      m_PreviewWorldRoot(Identity_Matrix())
{
    Copy_Buffer(m_PreviewAnchorBuffer.data(),
        m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
    Reset_MeshAuthoringDraft();
}

Client::CEffect_Tool::~CEffect_Tool()
{
    Deactivate_AuthoringWorkspace();
    m_pAuthoringSequencer.reset();
	if (m_ValtanPatternProductUnlinkOperation.has_value() &&
		nullptr != m_ValtanPatternProductUnlinkOperation->hProcess)
	{
		/* The child owns a mutating source/Product transaction. Closing our
		   observation handle must never terminate it; PowerShell completes or
		   rolls back independently even when the Client closes. */
		CloseHandle(m_ValtanPatternProductUnlinkOperation->hProcess);
		m_ValtanPatternProductUnlinkOperation->hProcess = nullptr;
	}
    m_pCharacterPreviewPanel->Set_SessionLock(
        CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL, false, {});
    Release_WorldPreview(true);
}

bool_t Client::CEffect_Tool::Open_ValtanAllEffectsWorkspace()
{
	m_bAllEffectsValtanBossSelected = true;
	m_bAllEffectsKoukuBossSelected = false;
	m_bAllEffectsWorldSelected = false;
	const bool_t bHadPendingExactRefresh =
		m_bValtanGraphRefreshRequested;
	const std::string PendingExactRevision =
		m_strPendingValtanGraphRefreshRevision;
	/* Valtan is an explicit workspace selection, so its saved authored rows
	   must also become the active Data Files category. Leaving the constructor
	   default (DimensionMaster) here made the exact Valtan source index ready
	   while the visible reusable Element tree appeared empty. This metadata
	   selection does not scan Resources/Effect or decode any Effect document. */
	Select_AuthoringDomain("Valtan");
	/* Opening the workspace must stay metadata-only.  The catalog already owns
	   each stable Effect ID and authored path; decoding every authored document
	   here made the first visible frame proportional to the complete Effect
	   corpus.  Exact document decoding remains owned by Open/Play. */
	Initialize_CatalogMetadataView();
	if (bHadPendingExactRefresh)
		Process_PendingValtanGraphRefresh();
	const bool_t bResourceCatalogReady = bHadPendingExactRefresh ?
		Can_DisplayValtanView(m_eValtanEffectResourceAdmission) :
		Refresh_ValtanEffectResourceSnapshot();
	const bool_t bExactSourcesReady =
		!m_ValtanExactAuthoredSources.empty();
	const bool_t bCanonicalGraphReady = bHadPendingExactRefresh ?
		(m_strCommittedValtanGraphRevision == PendingExactRevision &&
		 Can_DisplayValtanView(m_eValtanPatternTreeAdmission)) :
		Refresh_ValtanPatternTree();
	(void)Refresh_ValtanAreaStaticEffects();

	/* The exact authored source index is the usable fallback inventory.  A
	   canonical graph failure must gate Product/Server play, but it must not
	   hide effect.valtan.* documents that can still be opened and edited. */
	if ((bResourceCatalogReady || bExactSourcesReady) &&
		!m_ValtanExactAuthoredSources.empty())
	{
		m_strElementStatus = bCanonicalGraphReady ?
			"Opened Valtan All Effects from the canonical graph and exact authored source index." :
			"Opened Valtan exact authored Effects. Canonical Product play remains unavailable: " +
				m_strValtanPatternTreeStatus;
		return true;
	}

	m_strElementStatus =
		"Valtan All Effects could not build its exact authored source index.";
	if (!m_strUnifiedCandidateStatus.empty())
		m_strElementStatus += " " + m_strUnifiedCandidateStatus;
	return false;
}

bool_t Client::CEffect_Tool::Consume_TypedEffectResourceOpenRequest(
	EFFECT_RESOURCE_KEY& OutKey)
{
	if (!m_PendingTypedEffectResourceOpen.has_value())
		return false;
	OutKey = std::move(*m_PendingTypedEffectResourceOpen);
	m_PendingTypedEffectResourceOpen.reset();
	return true;
}

bool_t Client::CEffect_Tool::Open_ValtanProductEffect(
	const EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& Request)
{
	const auto Reject = [this](std::string Reason)
	{
		m_strValtanPatternEffectStatus =
			"Valtan Boss Tool Product Effect open rejected: " + std::move(Reason);
		m_strPreviewStatus = m_strValtanPatternEffectStatus;
		return false;
	};

	if (m_ValtanPatternProductUnlinkOperation.has_value())
	{
		return Reject(
			"wait for the current Product Effect unlink transaction to finish before opening another occurrence.");
	}

	m_bAllEffectsValtanBossSelected = true;
	m_bAllEffectsKoukuBossSelected = false;
	m_bAllEffectsWorldSelected = false;
	if (Request.strPatternId.empty() || Request.strStageId.empty() ||
		Request.strCueOccurrenceId.empty() ||
		Request.strEffectAssetId.empty())
	{
		return Reject("the stable Product identity is incomplete.");
	}

	/* Valtan Boss Tool and Effect Tool deliberately own separate staged views.  Stage
	   only the catalog-owned stable ID/path metadata here; the exact selected
	   document is decoded below by Try_OpenValtan* after the Product tuple has
	   been resolved again. */
	Initialize_CatalogMetadataView();
	if (m_DirectAuthoredEditableEntries.empty())
		return Reject("the direct-authored Effect metadata index is unavailable.");
	if (!Refresh_ValtanPatternTree())
		return Reject(m_strValtanPatternTreeStatus.empty() ?
			"the joined Valtan tree could not be refreshed." :
			m_strValtanPatternTreeStatus);

	const VALTAN_PATTERN_VIEW* pPattern =
		Find_ValtanPattern(Request.strPatternId);
	if (nullptr == pPattern || !Is_ValtanAllEffectsPattern(*pPattern))
	{
		return Reject("pattern '" + Request.strPatternId +
			"' is not in the exact Boss/All Effects inventory.");
	}

	const VALTAN_STAGE_VIEW* pStage = nullptr;
	size_t iStageMatchCount = 0u;
	for (const VALTAN_STAGE_VIEW& Candidate : pPattern->Stages)
	{
		if (Candidate.strStageId != Request.strStageId)
			continue;
		pStage = &Candidate;
		++iStageMatchCount;
	}
	if (1u != iStageMatchCount || nullptr == pStage)
	{
		return Reject("stage '" + Request.strStageId +
			"' did not resolve exactly once.");
	}

	const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCue = nullptr;
	size_t iCueMatchCount = 0u;
	for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate :
		pStage->ProductCues)
	{
		if (Candidate.strOccurrenceId != Request.strCueOccurrenceId)
			continue;
		pCue = &Candidate;
		++iCueMatchCount;
	}
	if (1u != iCueMatchCount || nullptr == pCue)
	{
		return Reject("cue occurrence '" + Request.strCueOccurrenceId +
			"' did not resolve exactly once in the selected stage.");
	}
	if (pCue->strPatternId != pPattern->strPatternId ||
		pCue->strStageId != pStage->strStageId ||
		pCue->strActionId != pStage->strActionId ||
		pCue->strEffectAssetId != Request.strEffectAssetId)
	{
		return Reject(
			"the pattern/stage/action/cue/Effect tuple changed after selection.");
	}

	std::string PathStatus;
	const std::filesystem::path* pPath =
		Resolve_DirectAuthoredEditablePath(
			Request.strEffectAssetId, PathStatus);
	if (nullptr == pPath)
		return Reject(std::move(PathStatus));

	m_strSelectedValtanPatternId = pPattern->strPatternId;
	Select_SharedCompletePlayPattern(m_strSelectedValtanPatternId);
	m_SelectedValtanPatternEffect.reset();
	Copy_Buffer(m_AllEffectsSearch.data(), m_AllEffectsSearch.size(),
		std::string{});

	bool_t bOpened = false;
	if (pCue->bUsesStageClock)
	{
		/* A STAGE_CLOCK cue intentionally has no body occurrence. Keep that
		   authoring contract explicit instead of inventing a Model View clip. */
		bOpened = Try_OpenValtanStandaloneEffect(
			*pPath, Request.strEffectAssetId);
	}
	else
	{
		const VALTAN_CLIP_OCCURRENCE_VIEW* pClip = nullptr;
		size_t iClipMatchCount = 0u;
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate :
			pStage->ClipOccurrences)
		{
			if (Candidate.strClipOccurrenceId !=
				pCue->strClipOccurrenceId)
			{
				continue;
			}
			pClip = &Candidate;
			++iClipMatchCount;
		}
		if (1u != iClipMatchCount || nullptr == pClip)
		{
			return Reject("clip occurrence '" + pCue->strClipOccurrenceId +
				"' did not resolve exactly once in its semantic stage.");
		}

		VALTAN_PRODUCT_EFFECT_CUE_VIEW PlaybackCue = *pCue;
		PlaybackCue.strEffectAssetId = Request.strEffectAssetId;
		VALTAN_PRODUCT_PREVIEW Preview;
		std::string PreviewError;
		bool_t bPreviewReady = false;
		if (pPattern->bAuthoringMasterManaged)
		{
			constexpr std::array<VALTAN_PATTERN_PREVIEW_PATH, 4u>
				PreviewPaths = {
					VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
					VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
					VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
					VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK };
			for (const VALTAN_PATTERN_PREVIEW_PATH ePath : PreviewPaths)
			{
				if (Build_ValtanProductPreview(
						*pPattern, ePath, *pClip, PlaybackCue,
						Preview, PreviewError))
				{
					bPreviewReady = true;
					break;
				}
			}
		}
		else
		{
			/* Legacy/manual Product rows retain their exact semantic stage-local
			   occurrence, matching the existing All Effects fallback. */
			Preview.Clip = *pClip;
			Preview.Cue = std::move(PlaybackCue);
			VALTAN_CLIP_OCCURRENCE_VIEW TimelineClip = *pClip;
			TimelineClip.iAuthoringWallMs = pStage->iDurationMs;
			Preview.TimelineClips = { std::move(TimelineClip) };
			Preview.iTimelineDurationMs = pStage->iDurationMs;
			bPreviewReady = 0u != pStage->iDurationMs;
			if (!bPreviewReady)
				PreviewError = "the Product stage has no authoring wall.";
		}
		if (!bPreviewReady)
			return Reject(std::move(PreviewError));
		bOpened = Try_OpenValtanAuthoredEffect(
			*pPath, Request.strEffectAssetId, Preview, false);
	}

	if (bOpened)
	{
		m_strValtanPatternEffectStatus =
			"Opened exact Boss Product occurrence in Effect Tool: " +
			Request.strPatternId + " / " + Request.strStageId + " / " +
			Request.strCueOccurrenceId;
	}
	else if (m_PendingDocumentLoad.has_value() &&
		m_PendingDocumentLoad->Path == *pPath)
	{
		m_strValtanPatternEffectStatus =
			"Exact Boss Product occurrence is waiting for the current Effect Save/Discard decision.";
	}
	else
	{
		m_strValtanPatternEffectStatus =
			"Exact Product Effect could not open: " + m_strPreviewStatus;
	}
	return bOpened;
}

void Client::CEffect_Tool::Update(const f32_t fTimeDelta)
{
    ++m_iFrameNumber;
	const bool_t bSkipPreviewAdvance = m_bSkipNextWorldPreviewDelta;
	m_bSkipNextWorldPreviewDelta = false;
	Process_PendingValtanGraphRefresh();
	Update_ValtanServerPatternAudition();
	Update_ValtanPatternProductEffectUnlink();
    m_pThumbnailCache->Begin_Frame(m_iFrameNumber);
	m_pCharacterPreviewPanel->Refresh_Level();
    if (m_pAuthoringSequencer)
    {
        m_pCharacterPreviewPanel->Set_SessionLock(CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL, false, {});
        if (m_pAuthoringSequencer->Is_Active())
        {
            m_bPreviewPlaying = false;
            if (auto preview = m_pWorldPreviewObject.lock()) preview->Set_Visible(false);
            return;
        }
    }
	const bool_t bStandaloneValtanEffectActive =
		m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
			m_eActiveDocumentPreviewIntent;
	const bool_t bValtanPatternDraftActive =
		m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
			m_eActiveDocumentPreviewIntent;
	const bool_t bStaticAreaEffectActive =
		m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
			m_eActiveDocumentPreviewIntent;
	const bool_t bCombatObjectIndependentPreviewActive =
		m_ValtanCombatObjectIndependentPreview.has_value();
	/* A clean saved Effect is not an authoring transaction and must not block
	   another Tool from staging the shared Valtan Arena Clone.  Only unsaved
	   non-static work or an active multi-root combat-object playback owns a
	   target lock; the exact idle-boss lifecycle must not be overwritten by a
	   second authoring Tool mid-sample. */
	const bool_t bEffectDraftNeedsPreviewLock =
		(!m_pAuthoringSequencer && !bStaticAreaEffectActive && Has_UnsavedWork()) ||
		bCombatObjectIndependentPreviewActive;
    m_pCharacterPreviewPanel->Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL,
		bEffectDraftNeedsPreviewLock,
		bCombatObjectIndependentPreviewActive ?
			"Stop the independent combat-object preview before another Tool changes the locked Valtan target." :
		(bStandaloneValtanEffectActive ?
			"Apply or discard Standalone Valtan Effect changes before another Tool changes the preview target." :
			(bValtanPatternDraftActive ?
				"Apply or discard Valtan Pattern Draft changes before another Tool changes the preview target." :
				"Apply or discard Effect changes before changing target.")));
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    if (m_iWorldPreviewLevel != UINT32_MAX &&
        m_iWorldPreviewLevel != iCurrentLevel)
    {
        Release_WorldPreview(false);
    }
	if (!bCombatObjectIndependentPreviewActive &&
		bEffectDraftNeedsPreviewLock && bStandaloneValtanEffectActive &&
		(nullptr == CAnimationTargetService::Resolve_Boss() ||
		 nullptr == CAnimationTargetService::Resolve_Model() ||
		 CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME) &&
		!Prepare_ValtanStandaloneEffectTarget())
	{
		/* A level transition or another non-UI target publisher can invalidate the
		   dedicated boss after the selector was locked. Fail closed before the
		   ordinary root resolver can attach this Effect to a scene character. */
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = m_strPreviewAnimationStatus;
		return;
	}
	bool_t bValtanPatternDraftTimelineRebound = false;
	const bool_t bValtanPatternDraftTargetInvalid =
		bEffectDraftNeedsPreviewLock && bValtanPatternDraftActive &&
		(nullptr == CAnimationTargetService::Resolve_Boss() ||
		 nullptr == CAnimationTargetService::Resolve_Model() ||
		 CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME ||
		 m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration());
	if (bValtanPatternDraftTargetInvalid &&
		!Prepare_ActiveValtanPatternDraftTimeline(!m_bPreviewPlaying))
	{
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = m_strPreviewAnimationStatus;
		return;
	}
	if (bValtanPatternDraftTargetInvalid)
		bValtanPatternDraftTimelineRebound = true;
	if (bStaticAreaEffectActive && !Update_StaticAreaPreviewRoot())
	{
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus =
			"Static Area placement preview lost its typed transform; no actor fallback was used.";
		return;
	}
	if (m_bReconstructedDiagnosticActive)
	{
		Update_ReconstructedDiagnosticRoot();
		return;
	}
	if (m_bReconstructedSourceRuntimeActive)
	{
		Update_SynchronizedAnimationSequence();
		Update_ReconstructedSourceRuntimeTimeline(fTimeDelta);
		return;
	}
    Update_SynchronizedAnimationSequence();
    if (!m_ActiveDocument.has_value() &&
        !(m_ProductPreview.has_value() &&
          m_SourcePreviewDocument.has_value()) &&
		!std::any_of(m_SynchronizedAnimationClips.begin(),
			m_SynchronizedAnimationClips.end(),
			[](const SYNCHRONIZED_ANIMATION_CLIP& Clip)
			{
				return 0u != Clip.iAuthoringWallMs;
			}))
        return;
    f32_t fSequentialAdvance = 0.f;
    bool_t bSeekAfterLoop = bValtanPatternDraftTimelineRebound || bSkipPreviewAdvance;
    if (m_bPreviewPlaying && !bSkipPreviewAdvance)
    {
        const f32_t fPreviousTime = m_fPreviewTimeSeconds;
        const f32_t fPreviousEffectTime =
            Resolve_EffectSampleTime(fPreviousTime);
        f32_t fSynchronizedAnimationTime = 0.f;
        const bool_t bAnimationOwnsTime =
            Try_ResolveSynchronizedAnimationTime(fSynchronizedAnimationTime);
        if (bAnimationOwnsTime)
        {
            m_fPreviewTimeSeconds =
                (std::max)(0.f, fSynchronizedAnimationTime);
            bSeekAfterLoop = bSeekAfterLoop ||
				m_fPreviewTimeSeconds + 0.0001f < fPreviousTime;
        }
        else
        {
            m_fPreviewTimeSeconds += (std::max)(0.f, fTimeDelta);
			if (std::any_of(m_SynchronizedAnimationClips.begin(),
					m_SynchronizedAnimationClips.end(),
					[](const SYNCHRONIZED_ANIMATION_CLIP& Clip)
					{
						return 0u != Clip.iAuthoringWallMs;
					}))
			{
				/* A short source window can intentionally hold its final pose for
				   the rest of a semantic stage. During that hold the model clock no
				   longer advances, so drive the next occurrence from the master wall
				   clock and keep scrubbing deterministic. */
				Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
			}
        }
        if (m_fPreviewTimeSeconds > m_fPreviewDurationSeconds)
        {
            if (m_bPreviewLoop)
            {
                m_fPreviewTimeSeconds = std::fmod(
                    m_fPreviewTimeSeconds, m_fPreviewDurationSeconds);
                bSeekAfterLoop = true;
            }
            else
            {
                m_fPreviewTimeSeconds = m_fPreviewDurationSeconds;
                m_bPreviewPlaying = false;
                bSeekAfterLoop = true;
            }
        }
		if (!bSeekAfterLoop)
			fSequentialAdvance = (std::max)(
                0.f, Resolve_EffectSampleTime(m_fPreviewTimeSeconds) -
                    fPreviousEffectTime);
	}
	if (m_ValtanCombatObjectIndependentPreview.has_value())
	{
		if (!Sync_ValtanCombatObjectIndependentPreview(bSeekAfterLoop))
		{
			const std::string Failure = m_strPreviewStatus;
			Release_WorldPreview(true);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus = Failure;
		}
		return;
	}
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject)
        return;
    if (pObject->Is_RenderFailureIsolated())
    {
        pObject->Set_Playing(false);
        pObject->Set_Visible(false);
        m_bPreviewPlaying = false;
        m_bPreviewVisibleRequested = false;
        Set_SynchronizedAnimationPaused(true);
        m_strPreviewStatus = pObject->Get_Status();
        return;
    }
	if (m_bValtanBossPatternTransformHistoryRequired)
	{
		if (!m_bValtanBossPatternTransformHistoryActive ||
			!m_ActiveDocument.has_value() ||
			m_ActiveDocument->strEffectAssetId !=
				m_strValtanBossPatternPreviewEffectAssetId)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			Set_SynchronizedAnimationPaused(true);
			m_strPreviewStatus =
				"World preview hidden: Valtan 420633 exact b_effectroot history is unavailable.";
			return;
		}
		if (bSeekAfterLoop)
		{
			Reset_ProductCueSnapshot();
			Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
			[this](const f32_t fSampleTimeSeconds,
				EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
				std::string& strOutError)
			{
				return Build_ValtanBossPatternTransformSample(
					fSampleTimeSeconds, OutSample, strOutError);
			};
		const f32_t fEffectSampleTime =
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
		std::string TransformError;
		bool_t bHistoryAdvanced = true;
		if (bSeekAfterLoop ||
			std::abs(pObject->Get_PreviewFixedStepClockSeconds() -
				static_cast<f64_t>(fEffectSampleTime)) > 1.0e-5 &&
			fSequentialAdvance <= 0.f)
		{
			bHistoryAdvanced = Seek_ValtanBossPatternTransformHistory(
				pObject, fEffectSampleTime, TransformError);
		}
		else if (fSequentialAdvance > 0.f)
		{
			bHistoryAdvanced = pObject->Advance_PreviewWithTransformHistory(
				fSequentialAdvance, TransformProvider, TransformError);
		}
		if (!bHistoryAdvanced)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			Set_SynchronizedAnimationPaused(true);
			m_bValtanBossPatternTransformHistoryActive = false;
			m_strPreviewStatus =
				"World preview hidden: Valtan 420633 anchor history failed: " +
				TransformError;
			return;
		}
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		if (m_bPreviewVisibleRequested)
		{
			m_strPreviewStatus =
				"Valtan 420633 preview follows exact B_EffectRoot / b_effectroot history.";
		}
		return;
	}
    if (bSeekAfterLoop)
    {
        Reset_ProductCueSnapshot();
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
    }
	/* Resolve follow anchors after any loop seek so the world submitted for this
	   playback tick reflects the newly selected animation pose, not the final
	   pose from the previous loop. */
	const EFFECT_DOCUMENT_DESC& SourceAnchorDocument =
		m_WorldPreviewDocument.has_value() ? *m_WorldPreviewDocument :
		(m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value() ?
			*m_SourcePreviewDocument : *m_ActiveDocument);
	std::unordered_map<std::string, float4x4_t> SourceAnchorWorlds;
	std::string SourceAnchorError;
	const bool_t bSourceAnchorsResolved = Resolve_ToolSourceAnchorWorlds(
		SourceAnchorDocument,
		m_ValtanProductPreview.has_value() ?
			&m_ValtanProductPreview->Cue : nullptr,
		SourceAnchorWorlds, SourceAnchorError);
	if (!bSourceAnchorsResolved && Has_RequiredSourceFollowAttachments(SourceAnchorDocument))
	{
		m_bPreviewPlaying = false;
		pObject->Set_Playing(false);
		Set_SynchronizedAnimationPaused(true);
		m_strPreviewStatus = "Source anchor unavailable; previous preview preserved: " + SourceAnchorError;
		return;
	}
	pObject->Set_SourceAnchorWorlds(std::move(SourceAnchorWorlds));
	if (!bSourceAnchorsResolved)
	{
		m_strPreviewStatus =
			"World preview source anchor unavailable: " + SourceAnchorError;
	}
	else if (0u == m_strPreviewStatus.find(
		"World preview source anchor unavailable:"))
	{
		m_strPreviewStatus = "World preview source anchors resolved.";
	}
    float4x4_t Root{};
    const bool_t bRootResolved = Resolve_PreviewRoot(Root);
    const bool_t bCueVisible =
        Is_ProductCueVisible(m_fPreviewTimeSeconds);
    pObject->Set_Visible(
        m_bPreviewVisibleRequested && bRootResolved && bCueVisible);
    if (m_bPreviewVisibleRequested && bRootResolved && bCueVisible)
    {
        if (0u == m_strPreviewStatus.find("World preview hidden:"))
            m_strPreviewStatus = "World preview anchor resolved.";
    }
    else if (m_bPreviewVisibleRequested && bRootResolved &&
		!bCueVisible && Has_ProductCuePreview())
    {
        m_strPreviewStatus = "Product cue is outside its admitted start/end window.";
    }
    else if (m_bPreviewVisibleRequested)
    {
        if (Has_ProductCuePreview())
        {
			const std::string& strCueAnchor = m_ProductPreview.has_value() ?
				m_ProductPreview->ProductCue.Cue.strAnchorSlotId :
				m_ValtanProductPreview->Cue.strAnchorSlotId;
            m_strPreviewStatus = "World preview hidden: Product cue cannot resolve " +
                ("root" == strCueAnchor ? std::string("its root anchor.") :
                    std::string("anchor '") + strCueAnchor + "'.");
        }
        else
        {
            m_strPreviewStatus = "World preview hidden: current target cannot resolve " +
                (EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                    std::string("its root pivot.") :
                    std::string("anchor '") + m_strPreviewAnchorSlotId + "'.");
        }
    }
    if (!m_bPreviewVisibleRequested)
		return;
    if (bSeekAfterLoop)
    {
		const f32_t fEffectSampleSeconds =
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
		std::string HistoryError;
		if (!Seek_WorldPreviewWithSourceAnchorHistory(
				pObject, SourceAnchorDocument,
				fEffectSampleSeconds, HistoryError))
		{
			if (Has_RequiredSourceFollowAttachments(SourceAnchorDocument) &&
				!m_SynchronizedAnimationClips.empty())
			{
				m_bPreviewPlaying = false;
				pObject->Set_Playing(false);
				Set_SynchronizedAnimationPaused(true);
				m_strPreviewStatus = "Hand history seek failed; previous preview preserved: " + HistoryError;
				return;
			}
			if (bRootResolved)
				pObject->Set_RootWorld(Root);
			pObject->Set_SampleTime(fEffectSampleSeconds);
			m_strPreviewStatus =
				"Loop seek used current-pose fallback: " + HistoryError;
		}
    }

	else if (bRootResolved && fSequentialAdvance > 0.f)
		pObject->Advance_Preview(fSequentialAdvance, Root);
	else if (bRootResolved)
		pObject->Set_RootWorld(Root);
}

void Client::CEffect_Tool::Render()
{
    Engine::CProfilerScope Profile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.Render");
    {
        Engine::CProfilerScope InitialIndexProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.InitialIndexStep");
		Initialize_CatalogMetadataView();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.AuthoringWindow");
        Render_EffectToolWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.ModelViewWindow");
        Render_ModelViewWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DetailWindow");
        Render_EffectDetailWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.AllEffectsWindow");
        Render_AllEffectsWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DataFilesWindow");
        Render_DataFilesWindow();
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Render_Sequencer("Effect Action Benchmark###EffectAuthoring", true);
    }
    {
        Engine::CProfilerScope TrimProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.ThumbnailTrim");
        m_pThumbnailCache->Trim();
    }
}

void Client::CEffect_Tool::Render_EffectToolWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620.f, 760.f), ImGuiCond_FirstUseEver);
    const bool_t bWindowVisible = ImGui::Begin("Effect Tool V1###EffectToolV1");
    Render_PendingDocumentLoadModal();
    if (!bWindowVisible)
    {
        ImGui::End();
        return;
    }
    ImGui::TextUnformatted(
        "Build individual Elements, combine them into one Effect, then tune each Element in Effect Detail.");
    const ImGuiIO& IO = ImGui::GetIO();
    ImGui::TextDisabled("FPS %.1f | Frame %.2f ms",
        IO.Framerate,
        IO.DeltaTime > 0.f ? IO.DeltaTime * 1000.f : 0.f);
	Render_ActiveAuthoredEffectTree();
    Render_MeshAuthoringWorkbench();
	if (ImGui::CollapsingHeader("Selected Element Resources",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		const bool_t bAdapterPacketInspection =
			EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
				m_eActiveDocumentSource &&
			nullptr != m_pSelectedVisualSourceProjection &&
			m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
		ImGui::TextDisabled(
			bAdapterPacketInspection ?
			"Exact adapter resource bindings are read-only here; ordinary authored Save As cannot preserve their projector/VF packet." :
			"Select one Element under Current Effect, then bind or replace its WModel/DDS slots here.");
		ImGui::BeginDisabled(bAdapterPacketInspection);
        Render_ResourceSlots(false);
        Render_ResourceGrid(false);
		ImGui::EndDisabled();
    }
    if (!m_strResourceStatus.empty())
        ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
    ImGui::End();
}

void Client::CEffect_Tool::Render_MeshAuthoringWorkbench()
{
    if (!m_bMeshAuthoringDraftInitialized)
        Reset_MeshAuthoringDraft();
    ImGui::SeparatorText("Element Authoring");
	static constexpr std::array<EFFECT_AUTHORING_FAMILY, 6u> FAMILIES{
		EFFECT_AUTHORING_FAMILY::MESH,
		EFFECT_AUTHORING_FAMILY::SPRITE,
		EFFECT_AUTHORING_FAMILY::MESH_PARTICLE,
		EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE,
		EFFECT_AUTHORING_FAMILY::LOCAL_DECAL,
		EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON };
	ImGui::TextUnformatted("Element Type");
	for (size_t iFamily = 0u; iFamily < FAMILIES.size(); ++iFamily)
	{
		if (0u != iFamily % 3u)
			ImGui::SameLine();
		const EFFECT_AUTHORING_FAMILY eFamily = FAMILIES[iFamily];
		if (ImGui::RadioButton(AuthoringFamily_Label(eFamily),
			m_eSelectedAuthoringFamily == eFamily))
		{
			const std::string strLayerId = m_NewElementId.data();
			m_eSelectedAuthoringFamily = eFamily;
			m_eSelectedEffectType = AuthoringFamily_Kind(eFamily);
			Reset_MeshAuthoringDraft();
			Copy_Buffer(m_NewElementId.data(), m_NewElementId.size(), strLayerId);
		}
	}
	if (m_SourceElementPresetSelection.has_value())
	{
		const SOURCE_ELEMENT_PRESET_SELECTION& Loaded =
			*m_SourceElementPresetSelection;
		ImGui::SeparatorText("Imported Element Draft");
		ImGui::TextDisabled(
			"Editable seed copy. Current Effect changes only after Create Element; Save Changes is the only Data File write.");
		ImGui::Text("Editable Type: %s | Source Family: %s",
			AuthoringFamily_Label(m_eSelectedAuthoringFamily),
			Loaded.strSourceFamily.c_str());
		static constexpr std::array<std::string_view, 9u> SUMMARY_SLOTS{
			EFFECT_MESH_SHAPE_SLOT_ID, "base", "noise", "mask",
			"emissive", "dissolve", "base2", "mask2", "noise2" };
		static constexpr std::array<const char*, 9u> SUMMARY_LABELS{
			"WModel", "Base", "Noise", "Mask", "Emissive", "Dissolve",
			"Base 2", "Mask 2", "Noise 2" };
		for (size_t iSlot = 0u; iSlot < SUMMARY_SLOTS.size(); ++iSlot)
		{
			const EFFECT_RESOURCE_BINDING_DESC* pBinding = Find_Binding(
				m_MeshAuthoringDraft, SUMMARY_SLOTS[iSlot]);
			ImGui::TextWrapped("%s: %s", SUMMARY_LABELS[iSlot],
				nullptr == pBinding ? "(not bound)" :
					pBinding->strAssetId.c_str());
		}
		if (ImGui::TreeNodeEx("Source Identity",
			ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::TextWrapped("%s", Loaded.strSourceRecordId.c_str());
			ImGui::TreePop();
		}
	}
	ImGui::SeparatorText("New Effect");
    ImGui::InputText("Effect Name", m_NewAssetId.data(),
        m_NewAssetId.size());
    ImGui::InputText("Display Name (optional)", m_NewDisplayName.data(),
        m_NewDisplayName.size());
	const bool_t bHasEffectName = '\0' != m_NewAssetId[0u];
	ImGui::BeginDisabled(!bHasEffectName || Has_UnsavedWork());
	if (ImGui::Button("New Effect"))
		Try_CreateDocument();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Creates an unsaved Current Effect. No file is written until Save Changes.");

	ImGui::SeparatorText("Create Element Draft");
    ImGui::InputText("Layer Name (optional)", m_NewElementId.data(),
        m_NewElementId.size());
    ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
        m_ResourceFilter.size());

	const EFFECT_RESOURCE_BINDING_DESC* pMesh = Find_Binding(
		m_MeshAuthoringDraft, EFFECT_MESH_SHAPE_SLOT_ID);
	const bool_t bArtistFSeedTargetsDifferentParent =
		m_SourceElementPresetSelection.has_value() &&
		m_SourceElementPresetSelection->strSourceEffectAssetId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		(!m_ActiveDocument.has_value() ||
		 m_ActiveDocument->strEffectAssetId !=
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID);
	const bool_t bMeshParticleNeedsCarrier =
		EFFECT_AUTHORING_FAMILY::MESH_PARTICLE ==
			m_eSelectedAuthoringFamily && nullptr == pMesh;
	const bool_t bCanCreateElement =
		m_ActiveDocument.has_value() &&
		!Has_UnappliedDetailDraft() && !m_bOccurrenceTuningDirty &&
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
		!bMeshParticleNeedsCarrier && !bArtistFSeedTargetsDifferentParent;

	if (ImGui::Button("Reset Element Draft"))
		Reset_MeshAuthoringDraft();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanCreateElement);
	if (ImGui::Button("Create Element"))
		Try_CreateElementDraft();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Refresh Resources"))
	{
		Refresh_ResourceCatalog();
	}

	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource))
	{
		ImGui::TextDisabled(
			"Create or open one editable Current Effect before adding an Element.");
	}
	else if (bMeshParticleNeedsCarrier)
	{
		ImGui::TextDisabled(
			"Choose a WModel seed for Mesh Particle so its Family remains stable. After creation, bind DDS in Selected Element Resources above.");
	}
	else if (bArtistFSeedTargetsDifferentParent)
	{
		ImGui::TextDisabled(
			"Open Artist F > Editable Skill Effect before creating this Track A Seed.");
	}
	else
		ImGui::TextDisabled(
			"Create adds an unsaved Element to Current Effect. Select it, bind WModel/DDS slots, tune Details/Visible, then use Save Changes once.");

	if (m_SourceElementPresetSelection.has_value() ||
		EFFECT_AUTHORING_FAMILY::MESH_PARTICLE == m_eSelectedAuthoringFamily)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("Optional Element Seed Resources"))
		{
			Render_ResourceSlots(true);
			Render_ResourceGrid(true);
		}
	}
}

void Client::CEffect_Tool::Render_PendingDocumentLoadModal()
{
    if (m_bPendingDocumentLoadModalRequested &&
        m_PendingDocumentLoad.has_value())
    {
        ImGui::OpenPopup("Unsaved Effect Changes");
        m_bPendingDocumentLoadModalRequested = false;
    }
    if (!ImGui::BeginPopupModal(
        "Unsaved Effect Changes", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    const char* pCurrentAsset = m_ActiveDocument.has_value() ?
        m_ActiveDocument->strEffectAssetId.c_str() : "(none)";
    const char* pTargetAsset = m_PendingDocumentLoad.has_value() ?
        m_PendingDocumentLoad->strSelectionId.c_str() : "(none)";
    ImGui::Text("Current: %s", pCurrentAsset);
    ImGui::Text("Load: %s", pTargetAsset);
    ImGui::Separator();
	ImGui::TextWrapped("The following in-memory drafts block this load:");
	if (m_bDocumentDirty || m_bOccurrenceTuningDirty ||
		Has_UnappliedDetailDraft())
	{
		ImGui::BulletText("Active Effect / occurrence tuning");
	}
	if (m_bValtanAreaMapEffectDirty)
		ImGui::BulletText("Valtan Area Map Effect placement/surface draft");
	if (m_UnpublishedStaticAreaWorldDraft.has_value())
	{
		ImGui::BulletText(
			"Unsaved static world Effect registration draft");
		ImGui::TextDisabled(
			"Register in Area or use Discard & Load; an unregistered Area draft must be registered before it can be loaded by the world.");
	}
    if (Has_UnappliedDetailDraft())
    {
        ImGui::TextDisabled(
            "Save & Load requires Apply or Revert for the open Detail draft first.");
    }

    ImGui::BeginDisabled(Has_UnappliedDetailDraft() ||
		m_UnpublishedStaticAreaWorldDraft.has_value());
    if (ImGui::Button("Save All & Load"))
    {
        if (Execute_PendingDocumentLoad(true))
            ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Discard Listed Drafts & Load"))
    {
        if (Execute_PendingDocumentLoad(false))
            ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->ePreviewIntent ==
				EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT &&
			!m_PendingDocumentLoad->strValtanPatternId.empty())
		{
			m_strValtanPatternEffectStatus =
				"Cancelled Open Existing Effect; the current Effect and Product connections are unchanged.";
		}
        m_PendingDocumentLoad.reset();
        m_strDocumentStatus =
            "Cancelled the pending Effect document load.";
        ImGui::CloseCurrentPopup();
    }
    if (!m_strDocumentStatus.empty())
        ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
    ImGui::EndPopup();
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
    ImGui::TextUnformatted("Effect Type");
    for (int32_t iKind = 0;
        iKind < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iKind)
    {
        if (0 != iKind)
            ImGui::SameLine();
        const EFFECT_ELEMENT_KIND eKind =
            static_cast<EFFECT_ELEMENT_KIND>(iKind);
        if (ImGui::RadioButton(Kind_Label(eKind),
            m_eSelectedEffectType == eKind))
        {
            m_eSelectedEffectType = eKind;
            m_strSelectedResourceSlotId = Default_SlotId(eKind);
            m_eResourceLibraryFileKind = EFFECT_ELEMENT_KIND::MESH == eKind ?
                EFFECT_RESOURCE_FILE_KIND::MODEL :
                EFFECT_RESOURCE_FILE_KIND::TEXTURE;
        }
    }
}

void Client::CEffect_Tool::Hide_WorldPreview()
{
    m_bPreviewPlaying = false;
	m_bPreviewVisibleRequested = false;
    Release_WorldPreview(true);
    m_strPreviewStatus =
        "World preview hidden; the loaded Document and Effect Detail values were preserved.";
}

void Client::CEffect_Tool::Release_WorldPreview(
    const bool_t bRemoveFromLayer)
{
	Clear_ValtanCombatObjectIndependentPreview();
	const bool_t bWasReconstructedSourceRuntimeActive =
		m_bReconstructedSourceRuntimeActive;
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (bRemoveFromLayer && nullptr != pObject &&
        m_iWorldPreviewLevel == CGameInstance::Get().Get_CurrentLevelID())
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            m_iWorldPreviewLevel, PREVIEW_LAYER, pObject);
    }
    m_pWorldPreviewObject.reset();
	m_WorldPreviewDocument.reset();
	m_bSkipNextWorldPreviewDelta = false;
	m_pVisualPreviewProjection.reset();
	m_iWorldPreviewLevel = UINT32_MAX;
	m_bReconstructedDiagnosticActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	Reset_ReconstructedSourceRuntimeTimeline();
	Reset_ValtanBossPatternTransformHistory();
	if (bWasReconstructedSourceRuntimeActive)
	{
		Set_SynchronizedAnimationPaused(true);
		Reset_SynchronizedAnimationSequence();
	}
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
    if (m_pAuthoringSequencer && m_pAuthoringSequencer->Is_ElementPreview())
        m_pAuthoringSequencer->Stop();
    m_MarkedElementIds.clear();
	Release_WorldPreview(true);
    Clear_ProductCuePreview();
	Reset_RuntimeOccurrenceTuningSession();
    m_ActiveDocument.reset();
	m_ActiveRegistryBoundAuditionProvenance.reset();
    Clear_ActiveDocumentDrawableStatus();
    m_ActiveDocumentPath.clear();
	m_strActiveDocumentBaselineCanonical.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT;
	m_strActiveValtanPatternDraftId.clear();
	m_eActiveValtanPatternDraftPreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	m_StaticAreaPreviewPresentation.reset();
	m_UnpublishedStaticAreaWorldDraft.reset();
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedModelCueId.clear();
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strSelectedResourceAssetId.clear();
    m_bDocumentDirty = false;
    m_bActiveDocumentMatchesRuntime = false;
    m_bPreviewPlaying = false;
    m_fPreviewTimeSeconds = 0.f;
    Reset_SynchronizedAnimationSequence();
    Release_WorldPreview(true);
    m_strDocumentStatus =
        "Unloaded the in-memory Effect Document and hid its preview; the saved Data File was preserved.";
}

void Client::CEffect_Tool::Reset_MeshAuthoringDraft()
{
	m_SourceElementPresetSelection.reset();
    m_MeshAuthoringDraft = {};
	m_MeshAuthoringDraft.eKind =
		AuthoringFamily_Kind(m_eSelectedAuthoringFamily);
	m_MeshAuthoringDraft.Renderer = {};
	m_MeshAuthoringDraft.strGroupId = "manual.hit1";
    m_MeshAuthoringDraft.Material.strTemplateId =
        std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
    m_MeshAuthoringDraft.Material.eRenderProfile =
        EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
	m_MeshAuthoringDraft.Detail.Mesh.bUseModelMaterial = false;
	m_MeshAuthoringDraft.Detail.Timing.fLifeTimeSeconds = 5.f;
	if (AuthoringFamily_RequiresMesh(m_eSelectedAuthoringFamily))
	{
		/* WModel source-unit normalization belongs to the import transform.
		   Keep the Element transform at identity so Scaling remains the artist's
		   direct, live size control instead of applying a second 0.01 factor. */
		m_MeshAuthoringDraft.Detail.Mesh.fModelPreScale =
			EFFECT_MANUAL_MESH_DEFAULT_SCALE;
	}
	if (EFFECT_AUTHORING_FAMILY::MESH_PARTICLE ==
			m_eSelectedAuthoringFamily ||
		EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE ==
			m_eSelectedAuthoringFamily)
	{
		EFFECT_PARTICLE_DESC& Particle =
			m_MeshAuthoringDraft.Detail.Particle;
		Particle.iMaxParticles = 1u;
		Particle.fSpawnRatePerSecond = 0.f;
		Particle.iBurstCount = 1u;
		Particle.iRandomSeed = 1u;
		Particle.vLifeTimeSeconds = { 2.f, 2.f };
		Particle.vInitialPositionMin = {};
		Particle.vInitialPositionMax = {};
		Particle.vInitialVelocityMin = {};
		Particle.vInitialVelocityMax = {};
		Particle.vAcceleration = {};
		const f32_t fSize =
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE ==
				m_eSelectedAuthoringFamily ? 1.f : 0.75f;
		Particle.vStartSize = { fSize, fSize };
		Particle.vEndSize = { fSize, fSize };
		Particle.bLocalSpace = true;
		Particle.bBillboard =
			EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE ==
				m_eSelectedAuthoringFamily;
	}
	if (EFFECT_AUTHORING_FAMILY::LOCAL_DECAL ==
		m_eSelectedAuthoringFamily)
	{
		m_MeshAuthoringDraft.Detail.Decal.vSize = { 3.5f, 3.5f };
		m_MeshAuthoringDraft.Detail.Decal.fDepth = 1.f;
	}
	if (EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON ==
		m_eSelectedAuthoringFamily)
	{
		m_MeshAuthoringDraft.Detail.Transform.vVelocityPerSecond =
			{ 1.f, 0.f, 0.f };
		m_MeshAuthoringDraft.Detail.Trail.fPointLifeTimeSeconds = 0.75f;
		m_MeshAuthoringDraft.Detail.Trail.fStartWidth = 0.25f;
		m_MeshAuthoringDraft.Detail.Trail.fEndWidth = 0.1f;
	}
	m_MeshAuthoringDraft.SourceRecipe = {};
	m_MeshAuthoringDraft.SourcePresentation = {};
	m_MeshAuthoringDraft.ActionCueAttachment = {};
	m_MeshAuthoringDraft.TransformInheritance = {};
    m_bMeshAuthoringDraftInitialized = true;
    m_NewElementId[0u] = '\0';
	const bool_t bRequiresMesh =
		AuthoringFamily_RequiresMesh(m_eSelectedAuthoringFamily);
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    m_strSelectedResourceAssetId.clear();
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    m_iResourceViewRevision = UINT64_MAX;
}

void Client::CEffect_Tool::Reset_ParticleSystemDraft()
{
    m_ParticleSystemDraft.reset();
    m_bParticleSystemDraftDirty = false;
    m_strDetailStatus.clear();
}

void Client::CEffect_Tool::Refresh_DetailDraftAdmission(
	const EFFECT_ELEMENT_DESC& Element)
{
	m_bDetailDraftPortableRecipeReadOnly = false;
	m_bDetailDraftCapabilityDeferred = false;
	m_strDetailDraftCapabilityReason.clear();
	if (!m_ActiveDocument.has_value() ||
		!Is_CompilerOwnedPortableRecipe(*m_ActiveDocument, Element))
	{
		return;
	}
	m_bDetailDraftPortableRecipeReadOnly = true;
	if (EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind)
		return;

	EFFECT_ELEMENT_DESC PortableProbe = Element;
	std::string Error;
	if (CEffectDocumentCodec::Apply_PortableAuthoredParticleRuntimeCarrier(
			Element, PortableProbe, Error))
	{
		return;
	}
	m_bDetailDraftCapabilityDeferred = true;
	m_strDetailDraftCapabilityReason = Error.empty() ?
		"The current SourceRecipe is outside the ordinary portable Particle capability." :
		std::move(Error);
}

void Client::CEffect_Tool::Reset_DetailDraft()
{
    m_DetailDraft.reset();
    m_strDetailDraftElementId.clear();
	m_strDetailDraftCapabilityReason.clear();
    m_bDetailDraftDirty = false;
	m_bDetailDraftPortableRecipeReadOnly = false;
	m_bDetailDraftCapabilityDeferred = false;
	m_bDetailDraftPreviewPending = false;
	m_bDetailDraftPreviewRestartRequested = false;
	m_fDetailDraftPreviewDueSeconds = 0.0;
    m_strDetailStatus.clear();
}

void Client::CEffect_Tool::Recalculate_PreviewDuration()
{
	if (m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value())
	{
		Recalculate_PreviewDuration(*m_SourcePreviewDocument);
		return;
	}
    if (!m_ActiveDocument.has_value())
    {
		m_fPreviewDurationSeconds = 0u ==
			m_iValtanWorldOwnerStageDurationMs ? 1.f :
			static_cast<f32_t>(m_iValtanWorldOwnerStageDurationMs) * 0.001f;
        m_fPreviewTimeSeconds = std::clamp(
            m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
        return;
    }
    Recalculate_PreviewDuration(*m_ActiveDocument);
}

void Client::CEffect_Tool::Recalculate_PreviewDuration(
    const EFFECT_DOCUMENT_DESC& Document)
{
	if (m_ValtanCombatObjectIndependentPreview.has_value())
	{
		m_fPreviewDurationSeconds = static_cast<f32_t>(
			m_ValtanCombatObjectIndependentPreview->iTimelineDurationMs) *
			0.001f;
		m_fPreviewTimeSeconds = std::clamp(
			m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
		return;
	}
    f32_t fEffectDurationSeconds = 1.f;
    for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (!Element.bVisible)
            continue;
        fEffectDurationSeconds = (std::max)(fEffectDurationSeconds,
			Element_PreviewEndSeconds(Element));
    }
    for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
    {
        if (Cue.bVisible)
        {
            fEffectDurationSeconds = (std::max)(
                fEffectDurationSeconds,
                Cue.fStartDelaySeconds + Cue.fDurationSeconds);
        }
    }
    m_fPreviewDurationSeconds = fEffectDurationSeconds;
    if (m_ProductPreview.has_value())
    {
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		const bool_t bBufferedOccurrence =
			m_bBufferedComboAuditionActive &&
			m_eBufferedComboAuditionClass ==
				m_ProductPreview->eCharacterClass &&
			m_iBufferedComboAuditionSkillId == m_ProductPreview->iSkillId;
		const f32_t fOccurrenceOffsetSeconds = bBufferedOccurrence ?
			m_fBufferedComboAuditionOccurrenceOffsetSeconds : 0.f;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		if (CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.f, Sample))
		{
			m_fPreviewDurationSeconds = fOccurrenceOffsetSeconds +
				Sample.fCueWallStartSeconds +
				fEffectDurationSeconds / Timing.fPlayRate;
			if (Timing.bHasCueSourceEnd)
			{
				m_fPreviewDurationSeconds = (std::max)(
					m_fPreviewDurationSeconds,
					fOccurrenceOffsetSeconds + Sample.fCueWallEndSeconds);
			}
		}
		else if (bBufferedOccurrence)
		{
			m_fPreviewDurationSeconds = (std::max)(
				m_fPreviewDurationSeconds,
				fOccurrenceOffsetSeconds + fEffectDurationSeconds);
		}
		f32_t fClipWallDurationSeconds = 0.f;
		if (Try_ResolvePlayerProductClipWallDuration(
				fClipWallDurationSeconds))
		{
			m_fPreviewDurationSeconds = (std::max)(
				m_fPreviewDurationSeconds, fClipWallDurationSeconds);
		}
		if (bBufferedOccurrence)
		{
			m_fPreviewDurationSeconds = (std::max)(
				m_fPreviewDurationSeconds,
				m_fBufferedComboAuditionDurationSeconds);
		}
    }
	else if (m_ValtanProductPreview.has_value())
	{
		/* Valtan Product Play now owns the complete chosen pattern branch.
		   CUE_END visibility is bounded by its source window while NATURAL
		   Effects retain their document lifetime across later stages, matching
		   the Server-authoritative boss presentation path. */
		const uint32_t iProductTimelineDurationMs =
			0u != m_ValtanProductPreview->iTimelineDurationMs ?
				m_ValtanProductPreview->iTimelineDurationMs :
				m_ValtanProductPreview->Cue.iStageDurationMs;
		m_fPreviewDurationSeconds = static_cast<f32_t>(
			iProductTimelineDurationMs) * 0.001f;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
		Timing.bHasCueSourceEnd =
			m_ValtanProductPreview->Cue.bHasSourceEnd;
		f32_t fCuePreviewDuration = 0.f;
		if (CActionPresentationTimeline::Resolve_CuePreviewDuration(
				Timing, static_cast<f32_t>(
					m_ValtanProductPreview->iOwningClipTimelineOffsetMs) * 0.001f,
				m_fPreviewDurationSeconds, fEffectDurationSeconds,
				fCuePreviewDuration))
		{
			m_fPreviewDurationSeconds = fCuePreviewDuration;
		}
		else
		{
			m_strPreviewStatus =
				"Valtan Effect preview duration rejected invalid cue timing; the pattern duration was preserved.";
		}
	}
	else if (0u != m_iValtanWorldOwnerStageDurationMs)
	{
		/* A reference visual may be staged against one Server stage or a full
		   master pattern. Keep its trigger offset and owner window authorable
		   without changing the Effect document's local timing. */
		m_fPreviewDurationSeconds = (std::max)(
			static_cast<f32_t>(m_iValtanReferenceEffectStartMs) * 0.001f +
				m_fPreviewDurationSeconds,
			static_cast<f32_t>(m_iValtanWorldOwnerStageDurationMs) * 0.001f);
	}
    m_fPreviewTimeSeconds = std::clamp(
        m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
}

bool_t Client::CEffect_Tool::Try_ResolvePlayerProductClipWallDuration(
	f32_t& fOutWallDurationSeconds) const
{
	fOutWallDurationSeconds = 0.f;
	if (!m_ProductPreview.has_value() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration())
	{
		return false;
	}

	const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
		m_ProductPreview->ProductCue;
	const ANIMATION_SKILL_CLIP& Clip = ProductCue.Clip;
	const SYNCHRONIZED_ANIMATION_CLIP& Synchronized =
		m_SynchronizedAnimationClips.front();
	if (Synchronized.strClipName != Clip.strClipName ||
		Synchronized.iSourceStartMs != Clip.iSourceStartMs ||
		Synchronized.iPlayMs != Clip.iPlayMs ||
		Synchronized.fPlayRate != Clip.fPlayRate)
	{
		return false;
	}

	const char_t* pExpectedAsset = Animation_AssetName(
		m_ProductPreview->eCharacterClass);
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pExpectedAsset || nullptr == pModel ||
		CAnimationTargetService::Resolve_AssetName() != pExpectedAsset)
	{
		return false;
	}

	uint32_t iAnimation = UINT32_MAX;
	for (uint32_t iCandidate = 0u;
		iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
	{
		const char_t* pName = pModel->Get_AnimationName(iCandidate);
		if (nullptr == pName || Clip.strClipName != pName)
			continue;
		/* Duplicate model clip names make the duration source ambiguous. */
		if (UINT32_MAX != iAnimation)
			return false;
		iAnimation = iCandidate;
	}
	if (UINT32_MAX == iAnimation)
		return false;

	f32_t fPositionTicks = 0.f;
	f32_t fDurationTicks = 0.f;
	const f32_t fTicksPerSecond =
		pModel->Get_AnimationTickPerSecond(iAnimation);
	if (!pModel->Get_AnimationProgress(
			iAnimation, fPositionTicks, fDurationTicks) ||
		!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f ||
		!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
	{
		return false;
	}

	ACTION_PRESENTATION_CLIP_TIMING Timing;
	Timing.fModelSourceDurationSeconds =
		fDurationTicks / fTicksPerSecond;
	Timing.iPlayMs = Clip.iPlayMs;
	Timing.fPlayRate = Clip.fPlayRate;
	Timing.fSourceStartSeconds =
		static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
	f32_t fSourceDurationSeconds = 0.f;
	return CActionPresentationTimeline::Resolve_ClipDuration(
		Timing, fSourceDurationSeconds, fOutWallDurationSeconds);
}

bool_t Client::CEffect_Tool::Has_UnsavedWork() const
{
    return m_bDocumentDirty || m_bOccurrenceTuningDirty ||
		m_bValtanAreaMapEffectDirty || Has_UnappliedDetailDraft();
}

void Client::CEffect_Tool::Reset_ModelCueDraft()
{
	m_ModelCueDraft.reset();
	m_bModelCueDraftDirty = false;
	m_ModelCueAssetIdDraft[0u] = '\0';
	m_ModelCueClipNameDraft[0u] = '\0';
	m_strDetailStatus.clear();
}

bool_t Client::CEffect_Tool::Has_UnappliedDetailDraft() const
{
    return m_bParticleSystemDraftDirty || m_bDetailDraftDirty ||
		m_bModelCueDraftDirty || m_bOccurrenceTransformDraftDirty;
}

void Client::CEffect_Tool::Set_ActiveDocumentDrawableStatus(
    const bool_t bDrawable,
    std::string strError)
{
    m_bActiveDocumentDrawable = bDrawable;
    m_strActiveDocumentDrawableError = bDrawable ?
        std::string{} : std::move(strError);
}

void Client::CEffect_Tool::Clear_ActiveDocumentDrawableStatus()
{
    m_bActiveDocumentDrawable = false;
    m_strActiveDocumentDrawableError.clear();
}

void Client::CEffect_Tool::Refresh_RuntimeEquivalence()
{
    m_bActiveDocumentMatchesRuntime = false;
    if (!m_ActiveDocument.has_value() || Has_UnsavedWork())
        return;

    const shared_ptr<const EFFECT_DOCUMENT_DESC> pRuntimeDocument =
        CEffectCatalog::Find(m_ActiveDocument->strEffectAssetId);
    if (nullptr == pRuntimeDocument)
        return;
    if (m_pRuntimeEquivalenceDocument != pRuntimeDocument)
    {
        m_pRuntimeEquivalenceDocument = pRuntimeDocument;
        m_strRuntimeEquivalenceCanonical =
            CEffectDocumentCodec::Serialize(*pRuntimeDocument);
    }

    std::string ActiveCanonicalStorage;
    std::string_view ActiveCanonical = m_strActiveDocumentBaselineCanonical;
    if (ActiveCanonical.empty())
    {
        ActiveCanonicalStorage =
            CEffectDocumentCodec::Serialize(*m_ActiveDocument);
        ActiveCanonical = ActiveCanonicalStorage;
    }
    m_bActiveDocumentMatchesRuntime =
        m_strRuntimeEquivalenceCanonical == ActiveCanonical;
}

// Bound direct-authored Product saves replace only the selected prepared
// target. Active occurrences retain their immutable resources while later
// spawns consume the replacement. A mismatch here therefore means the source
// was observed outside that save transaction or activation failed closed.
std::string Client::CEffect_Tool::Describe_ProductPlaybackAuthoredDivergence(
    const std::string& strProductEffectAssetId)
{
    if (!m_ActiveDocument.has_value() ||
        EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource ||
        m_ActiveDocument->strEffectAssetId != strProductEffectAssetId)
    {
        return {};
    }
    if (Has_UnsavedWork())
    {
        return " | UNSAVED SOURCE: Product keeps the last committed Authored "
            "file while this document has unsaved edits.";
    }
    Refresh_RuntimeEquivalence();
    if (m_bActiveDocumentMatchesRuntime)
        return {};
    return " | SAVED SOURCE NOT ACTIVE: the prepared Product target still "
        "uses an earlier document. Save this Effect to retry atomic activation "
        "for subsequent spawns.";
}

Client::EFFECT_ELEMENT_DESC* Client::CEffect_Tool::Find_SelectedElement()
{
    if (!m_ActiveDocument.has_value())
        return nullptr;
    const auto Iterator = std::find_if(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    return Iterator == m_ActiveDocument->Elements.end() ?
        nullptr : &*Iterator;
}

Client::EFFECT_MODEL_CUE_DESC* Client::CEffect_Tool::Find_SelectedModelCue()
{
	if (!m_ActiveDocument.has_value())
		return nullptr;
	const auto Iterator = std::find_if(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(), [this](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == m_strSelectedModelCueId; });
	return Iterator == m_ActiveDocument->ModelCues.end() ? nullptr : &*Iterator;
}

const Client::EFFECT_ELEMENT_DESC*
Client::CEffect_Tool::Find_SelectedElement() const
{
    if (!m_ActiveDocument.has_value())
        return nullptr;
    const auto Iterator = std::find_if(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    return Iterator == m_ActiveDocument->Elements.end() ?
        nullptr : &*Iterator;
}

const Client::EFFECT_MODEL_CUE_DESC*
Client::CEffect_Tool::Find_SelectedModelCue() const
{
	if (!m_ActiveDocument.has_value())
		return nullptr;
	const auto Iterator = std::find_if(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(), [this](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == m_strSelectedModelCueId; });
	return Iterator == m_ActiveDocument->ModelCues.end() ? nullptr : &*Iterator;
}


bool Client::CEffect_Tool::Resolve_AuthoringSourceAnchors(
    const std::shared_ptr<CEffectObject>& object, const float4x4_t& root, const bool useKouku, const float seconds,
    std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    anchors.clear();
    const auto found = m_AuthoringOccurrenceDocuments.find(object.get());
    if (found == m_AuthoringOccurrenceDocuments.end())
    { error = "Effect occurrence has no immutable source document."; return false; }
    const auto requests = Collect_ToolSourceAnchorRequests(*found->second);
    if (requests.empty()) { error.clear(); return true; }
    const bool needsBones = std::any_of(requests.begin(), requests.end(), [](const auto& request)
        { return request.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
    if (useKouku)
    {
        if (!m_pAuthoringSequencer)
        { error = "The Kouku model-reference owner is unavailable."; return false; }
        return m_pAuthoringSequencer->Resolve_KoukuSourceAnchors(*found->second, root, seconds, anchors, error);
    }
    if (!Resolve_ToolSourceAnchorWorlds(*found->second, nullptr, anchors, error)) return false;
    if (!needsBones) return true;
    float4x4_t modelRoot;
    if (!CAnimationTargetService::Resolve_RootTransform(&modelRoot))
    { error = "The selected model root is unavailable for source attachments."; return false; }
    vector_t determinant;
    const matrix_t inverse = XMMatrixInverse(&determinant, XMLoadFloat4x4(&modelRoot));
    if (!std::isfinite(XMVectorGetX(determinant)) || std::fabs(XMVectorGetX(determinant)) < 1e-12f)
    { error = "The selected model root is singular."; return false; }
    const matrix_t delta = inverse * XMLoadFloat4x4(&root);
    for (auto& [slot, world] : anchors)
    {
        const auto request = std::find_if(requests.begin(), requests.end(),
            [&slot](const auto& value) { return value.strRuntimeAnchorSlotId == slot; });
        if (request != requests.end() && request->eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
            continue;
        XMStoreFloat4x4(&world, XMLoadFloat4x4(&world) * delta);
    }
    return true;
}
