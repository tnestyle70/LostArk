#include "Effect_Tool_Internal.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_DirectAuthoredSourceIndex.h"
#include "EffectResourceCatalog.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "Logic_LanceMaster.h"
#include "MapEffectPresentationRuntime.h"
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
#include "Model.h"
#include "Transform.h"
#include "CharacterPreviewPanel.h"
#include "EffectAuthoringSequencer.h"
#include "EffectAuthoringResourceTree.h"

size_t Client::CEffect_Tool::Count_ProductCueMappings(
	const std::string& strEffectAssetId) const
{
	if (strEffectAssetId.empty())
		return 0u;

	size_t iMappingCount = 0u;
	for (const EFFECT_SKILL_TREE_ENTRY& Entry : m_AllEffects)
	{
		if (Entry.Skill.eCharacterClass ==
			LostArk::Shared::CHARACTER_CLASS_ID::END)
		{
			continue;
		}
		iMappingCount += static_cast<size_t>(std::count_if(
			Entry.ProductCues.begin(), Entry.ProductCues.end(),
			[&strEffectAssetId](
				const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue)
			{
				return ProductCue.Cue.strEffectAssetId == strEffectAssetId;
			}));
	}
	const auto BossMappings =
		m_BossProductCueMappingCounts.find(strEffectAssetId);
	if (BossMappings != m_BossProductCueMappingCounts.end())
		iMappingCount += BossMappings->second;
	if (m_ValtanAreaMapEffectDocument.Is_Ready())
	{
		iMappingCount += static_cast<size_t>(std::count_if(
			m_ValtanAreaMapEffectDocument.Get_WorldEffects().begin(),
			m_ValtanAreaMapEffectDocument.Get_WorldEffects().end(),
			[&strEffectAssetId](
				const MAP_EFFECT_WORLD_PRESENTATION& Presentation)
			{
				return Presentation.effectAssetId == strEffectAssetId;
			}));
	}
	return iMappingCount;
}

bool_t Client::CEffect_Tool::Try_ApplyDraftAndSave()
{
	const bool_t bJoinedValtanPatternDocument =
		!m_strActiveValtanPatternDraftId.empty() ||
		m_ValtanProductPreview.has_value() ||
		m_ValtanCombatObjectIndependentPreview.has_value() ||
		0u != m_iValtanWorldOwnerStageDurationMs;
	if (bJoinedValtanPatternDocument &&
		!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		m_strDocumentStatus =
			"Apply + Save requires a freshly ADMITTED Valtan Pattern view; the open draft was preserved without writing.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "There is no active Document to save.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource &&
		EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Promote or Save As this view before applying an Authored save.";
		return false;
	}

	const bool_t bParticleDraft = m_bParticleSystemDraftDirty;
	const bool_t bDetailDraft = m_bDetailDraftDirty;
	const bool_t bModelCueDraft = m_bModelCueDraftDirty;
	if (bParticleDraft || bDetailDraft || bModelCueDraft)
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if ((bParticleDraft && !Apply_ParticleSystemDraft(Staged)) ||
			(bDetailDraft && !Apply_DetailDraft(Staged)) ||
			(bModelCueDraft && !Apply_ModelCueDraft(Staged)) ||
			!Try_CommitDocument(std::move(Staged)))
		{
			m_strDocumentStatus =
				"Apply + Save rejected; active file and preview were preserved.";
			return false;
		}
		if (bParticleDraft)
		{
			m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
			m_bParticleSystemDraftDirty = false;
		}
		if (bDetailDraft)
		{
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
			{
				m_DetailDraft = *pCommitted;
				Refresh_DetailDraftAdmission(*pCommitted);
			}
			m_bDetailDraftDirty = false;
			m_bDetailDraftPreviewPending = false;
			m_bDetailDraftPreviewRestartRequested = false;
		}
		if (bModelCueDraft)
		{
			if (const EFFECT_MODEL_CUE_DESC* pCommitted = Find_SelectedModelCue())
				m_ModelCueDraft = *pCommitted;
			m_bModelCueDraftDirty = false;
		}
	}
	if (!m_bDocumentDirty)
	{
		m_strDocumentStatus =
			"Already saved. No new Effect source changes are pending.";
		return true;
	}
	const bool_t bSaved = Try_SaveDocument(
		(bParticleDraft || bDetailDraft || bModelCueDraft) &&
		m_bActiveDocumentDrawable);
	if (bSaved)
		m_strDetailStatus = m_strDocumentStatus;
	return bSaved;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
	return Try_SaveDocument(false);
}

bool_t Client::CEffect_Tool::Try_SaveDocument(
	const bool_t bPreviewCommittedByApply)
{
    const bool bSequencerActive = m_pAuthoringSequencer && m_pAuthoringSequencer->Is_Active();
    if (bSequencerActive) m_pAuthoringSequencer->Preserve_ClockDuringAuthoring();
	const bool_t bJoinedValtanPatternDocument =
		!m_strActiveValtanPatternDraftId.empty() ||
		m_ValtanProductPreview.has_value() ||
		m_ValtanCombatObjectIndependentPreview.has_value() ||
		0u != m_iValtanWorldOwnerStageDurationMs;
	if (bJoinedValtanPatternDocument &&
		!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		m_strDocumentStatus =
			"Save requires a freshly ADMITTED Valtan Pattern view; the open draft was preserved without writing.";
		return false;
	}
	if (EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
		return Try_SaveRuntimeOccurrenceTuning();
    if (!m_ActiveDocument.has_value())
    {
        m_strDocumentStatus = "There is no active Document to save.";
        return false;
    }
    if (Has_UnappliedDetailDraft())
    {
        m_strDocumentStatus =
            "Apply or Revert the open Detail draft before saving the Document.";
        return false;
    }
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(
            m_ActiveDocument->strEffectAssetId).wstring() +
            L".effect.json"));
    if (EFFECT_DOCUMENT_SOURCE::IMPORTED == m_eActiveDocumentSource)
    {
        m_strDocumentStatus =
            "Imported Effect must use Save As to create a unique Authored ID.";
        return false;
    }
	if (EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE ==
		m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Legacy/Rollback migration references are immutable; use Save As for a new Authored Effect.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Runtime Assembly/WFX/Visual Program views are immutable copies; use Save As for a new Authored Effect.";
		return false;
	}
    if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource &&
        std::filesystem::is_regular_file(Path))
    {
        m_strDocumentStatus =
            "Save refuses to replace an existing Authored file from New; use Save As.";
        return false;
    }
	std::string FreshnessStatus;
	if (!Validate_ActiveRegistryBoundAuditionFreshness(FreshnessStatus))
	{
		m_strDocumentStatus =
			"Save rejected: registry-bound audition source freshness failed. " +
			FreshnessStatus;
		return false;
	}
    const bool_t bRegisteredDirectProduct =
        CEffectCatalog::Is_DirectAuthoredDocument(
            m_ActiveDocument->strEffectAssetId);
    if (bRegisteredDirectProduct && !m_bActiveDocumentDrawable)
    {
        m_strDocumentStatus =
            "Save rejected: this Effect is bound to Product gameplay, so a "
            "non-drawable partial draft cannot replace its canonical source. "
            "Use Save As for an unbound draft. " +
            m_strActiveDocumentDrawableError;
        return false;
    }

    const bool_t bWasAuthored =
        EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource;
    const std::string PreviousBaselineCanonical =
        m_strActiveDocumentBaselineCanonical;
    if (bRegisteredDirectProduct &&
        (!bWasAuthored || PreviousBaselineCanonical.empty()))
    {
        m_strDocumentStatus =
            "Save rejected: a bound Product Effect has no restorable authored "
            "baseline.";
        return false;
    }
	std::optional<EFFECT_DOCUMENT_DESC> PreviousCommittedDocument;
	if (bRegisteredDirectProduct)
	{
		EFFECT_DOCUMENT_DESC Previous;
		std::string PreviousLoadError;
		if (!CEffectDocumentCodec::Load(Path, Previous, PreviousLoadError) ||
			Previous.strEffectAssetId != m_ActiveDocument->strEffectAssetId ||
			CEffectDocumentCodec::Serialize(Previous) !=
				PreviousBaselineCanonical)
		{
			m_strDocumentStatus =
				"Save rejected: the direct-authored Product Effect no longer matches its restorable disk baseline.";
			if (!PreviousLoadError.empty())
				m_strDocumentStatus += " " + PreviousLoadError;
			return false;
		}
		PreviousCommittedDocument = std::move(Previous);
	}

    std::string SavedCanonical;
    std::string Error;
    if (Path.empty() || !CEffectDocumentCodec::Save_AtomicIfUnchanged(
        Path, *m_ActiveDocument,
        bWasAuthored ? std::string_view(PreviousBaselineCanonical) :
            std::string_view{},
        Error, &SavedCanonical))
    {
        m_strDocumentStatus = Path.empty() ?
            "Effect authoring path escaped Data/Effects/Authored." : Error;
        return false;
    }

	std::string ProductReloadStatus;
	if (bRegisteredDirectProduct &&
		!CEffectPresentationService::Reload_SelectedProductEffect(
			m_pDevice, m_pContext, m_ActiveDocument->strEffectAssetId,
			Path, ProductReloadStatus))
	{
		std::string RollbackStatus;
		const bool_t bDiskRestored = PreviousCommittedDocument.has_value() &&
			CEffectDocumentCodec::Save_AtomicIfUnchanged(
				Path, *PreviousCommittedDocument, SavedCanonical,
				RollbackStatus);
		m_bDocumentDirty = true;
		m_bActiveDocumentMatchesRuntime = false;
		if (bDiskRestored)
		{
			m_strActiveDocumentBaselineCanonical = PreviousBaselineCanonical;
			m_strDocumentStatus =
				"Product Effect activation failed; the previous disk source was restored and the edited draft remains open. Product runtime rollback status: " +
				ProductReloadStatus;
		}
		else
		{
			EFFECT_DOCUMENT_DESC CurrentDisk;
			std::string CurrentDiskError;
			if (CEffectDocumentCodec::Load(
					Path, CurrentDisk, CurrentDiskError) &&
				CEffectDocumentCodec::Serialize(CurrentDisk) == SavedCanonical)
			{
				m_strActiveDocumentBaselineCanonical = SavedCanonical;
			}
			m_strDocumentStatus =
				"CRITICAL: Product Effect activation failed and exact disk rollback did not complete. The previous prepared runtime target remains active: " +
				ProductReloadStatus + " | disk rollback: " + RollbackStatus;
		}
		return false;
	}

    m_bDocumentDirty = false;
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
    m_strActiveDocumentBaselineCanonical = SavedCanonical;
    m_bActiveDocumentMatchesRuntime = false;
	/* Reload_SelectedProductEffect commits a new immutable catalog document for
	   a bound Product source.  Refresh the cached pointer/canonical comparison
	   only after both disk and Product activation have committed, so editor
	   actions that require runtime equivalence become available immediately. */
	Refresh_RuntimeEquivalence();

    /* Keep the Effect Tool's browser cache on the committed source. The
	   Product prepared target above is independently immutable per occurrence;
	   this browser refresh only updates the editor's observed saved row. */
    const auto RefreshObservedValtanProductCache = [&]()
    {
        const auto ValtanCache = m_ValtanUnifiedEffectCaches.find(
            m_ActiveDocument->strEffectAssetId);
        if (ValtanCache == m_ValtanUnifiedEffectCaches.end())
            return;
        ValtanCache->second = {};
        (void)Refresh_UnifiedEffectCache(ValtanCache->second, Path,
            m_ActiveDocument->strEffectAssetId);
    };
    RefreshObservedValtanProductCache();

    if (m_ProductPreview.has_value() &&
        m_ProductPreview->ProductCue.Cue.strEffectAssetId ==
            m_ActiveDocument->strEffectAssetId)
    {
        m_SourcePreviewDocument = *m_ActiveDocument;
        if (!bSequencerActive) Synchronize_LoadedSkillPreview();
    }

    bool_t bLocalPreviewUpdated = false;
    const EFFECT_RESOURCE_KEY SavedKey{ EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT,
        m_ActiveDocument->strEffectAssetId };
    const bool bSequencerUsesSaved = bSequencerActive && m_pAuthoringSequencer->Uses_Resource(SavedKey);
	// Apply has already prepared this exact ordinary authored document, through
	// either Stage_WorldPreview or its active Sequencer Refresh_Effects path.
	// Product/registry/Valtan sessions retain their separate reload gates.
	const bool_t bReuseAppliedPreview = bPreviewCommittedByApply &&
		!bRegisteredDirectProduct && !bJoinedValtanPatternDocument &&
		!m_ProductPreview.has_value() &&
		!m_ActiveRegistryBoundAuditionProvenance.has_value();
    if (m_bActiveDocumentDrawable)
    {
        if (bSequencerUsesSaved)
        {
            bLocalPreviewUpdated = bReuseAppliedPreview ||
				m_pAuthoringSequencer->Refresh_Effects(&SavedKey);
            m_strPreviewStatus = m_pAuthoringSequencer->Status();
        }
        else if (!bSequencerActive)
        {
            Recalculate_PreviewDuration(*m_ActiveDocument);
            bLocalPreviewUpdated = (bReuseAppliedPreview &&
				m_WorldPreviewDocument.has_value() &&
				!m_pWorldPreviewObject.expired()) ||
				Stage_WorldPreview(*m_ActiveDocument);
            if (bLocalPreviewUpdated)
                Start_WorldPreviewFromBeginning();
        }
    }

    m_strSelectedDataFileAssetId =
        m_ActiveDocument->strEffectAssetId;
    m_strSelectedDataFileElementId.clear();
	// Invalidate only the saved document's lazy Element tree. Other expanded
	// Data Files keep their parsed snapshots, including a failed source row.
	for (EFFECT_DATA_FILE_ENTRY& Entry : m_DataFiles)
	{
		if (Entry.eSource != EFFECT_DOCUMENT_SOURCE::AUTHORED ||
			Entry.strAssetId != m_ActiveDocument->strEffectAssetId)
			continue;
		Entry.pParsedDocument.reset();
		Entry.bDocumentParseAttempted = false;
		Entry.strDocumentParseStatus.clear();
	}
    if (bLocalPreviewUpdated)
    {
		m_strDocumentStatus = bRegisteredDirectProduct ?
			("Saved & hot reloaded for subsequent Product spawns. Active occurrences retain their previous immutable resources. " +
				ProductReloadStatus) :
			"Saved & applied to the local Effect preview.";
    }
    else if (bSequencerActive && !bSequencerUsesSaved)
    {
        m_strDocumentStatus = "Saved the Authored Effect. The Sequencer is previewing another resource.";
        if (bRegisteredDirectProduct) m_strDocumentStatus += " " + ProductReloadStatus;
    }
    else if (!m_bActiveDocumentDrawable)
    {
        m_strDocumentStatus =
            "Saved the Authored Effect. Local preview remains hidden because "
            "the draft is not drawable: " +
            m_strActiveDocumentDrawableError;
    }
    else
    {
        m_strDocumentStatus =
            "Saved the Authored Effect, but the local preview could not be "
            "updated: " + m_strPreviewStatus;
		if (bRegisteredDirectProduct)
			m_strDocumentStatus +=
				" Subsequent Product spawns still use the hot-reloaded source. " +
				ProductReloadStatus;
    }

    Attach_AuthoringSaved();

    return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocumentAs(
    const std::string& strAssetId, const std::string& strDisplayName,
    const std::string& strParentId)
{
    if (!m_ActiveDocument.has_value())
    {
        m_strDocumentStatus = "There is no active Document to save.";
        return false;
    }
    if (Has_UnappliedDetailDraft())
    {
        m_strDocumentStatus =
            "Apply or Revert the open Detail draft before Save As.";
        return false;
    }
	std::string FreshnessStatus;
	if (!Validate_ActiveRegistryBoundAuditionFreshness(FreshnessStatus))
	{
		m_strDocumentStatus =
			"Save As rejected: registry-bound audition source freshness failed. " +
			FreshnessStatus;
		return false;
	}
	if (m_ActiveRegistryBoundAuditionProvenance.has_value() &&
		m_ActiveDocument->strEffectAssetId ==
			m_ActiveRegistryBoundAuditionProvenance->strEffectAssetId)
	{
		m_strDocumentStatus =
			"Registry-bound audition candidates cannot be saved under another Effect ID. Use Save Changes so the exact catalog row, document, and source pin remain one contract.";
		return false;
	}
	const bool_t bWasVisualProgramCopy =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource;
	const bool_t bAdapterPacketVisualCopy = bWasVisualProgramCopy &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketVisualCopy)
	{
		m_strDocumentStatus =
			"Adapter-packet Visual Programs cannot be saved through the ordinary Authored Effect codec because that would discard the exact projector/VF/resource packet. Use stable occurrence Transform Save/Reload, or create a separately validated generic authored starting copy.";
		return false;
	}
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.strEffectAssetId = strAssetId;
    if (!strDisplayName.empty()) Staged.strDisplayName = strDisplayName;
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Staged, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strAssetId).wstring() + L".effect.json"));
    if (Path.empty())
    {
        m_strDocumentStatus =
            "Effect Save As path escaped Data/Effects/Authored.";
        return false;
    }
    if (std::filesystem::is_regular_file(Path))
    {
        m_strDocumentStatus =
            "Save As refuses to overwrite an existing Effect ID.";
        return false;
    }
    if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, Staged, std::string_view{}, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    // Commit the new editor identity only after its required parent is saved.
    // A failed tree CAS removes only our unchanged new document; the original
    // file, active draft and preview remain intact.
    if (m_pAuthoringResources && !m_pAuthoringResources->Attach_Saved(
        EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, Staged.strEffectAssetId,
        Staged.strDisplayName, strParentId, Error))
    {
        std::string Rollback;
        const bool removed = Remove_EffectDocumentIfCanonical(
            Path, CEffectDocumentCodec::Serialize(Staged), Rollback);
        m_strDocumentStatus = "Recovery copy parent save failed; the original Effect is preserved. " + Error;
        m_strDocumentStatus += removed ? " The new copy was rolled back." :
            " The new file could not be rolled back and was preserved: " + Path.string() + " | " + Rollback;
        return false;
    }
    const bool_t bWasDrawable = m_bActiveDocumentDrawable;
    std::string PreviousDrawableError = m_strActiveDocumentDrawableError;
    Clear_ProductCuePreview();
    m_ActiveDocument = std::move(Staged);
    m_bMarkedElementIdsNeedPrune = true;
    Set_ActiveDocumentDrawableStatus(
        bWasDrawable, std::move(PreviousDrawableError));
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	if (bWasVisualProgramCopy)
		m_pSelectedVisualSourceProjection.reset();
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
    Refresh_RuntimeEquivalence();
    m_strSelectedDataFileAssetId = strAssetId;
	m_strSelectedDataFileElementId.clear();
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), strAssetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(), m_ActiveDocument->strDisplayName);
    if (m_pAuthoringResources)
    {
        m_strAuthoringParentId = m_pAuthoringResources->Selected_ParentId();
        const EFFECT_RESOURCE_KEY key{ EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, strAssetId };
        m_AuthoringParents[std::to_string(static_cast<int>(key.eOwnerKind)) + ":" + key.strStableId] = m_strAuthoringParentId;
    }
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strDocumentStatus = "Saved new Authored Effect atomically: " +
        Path.string() +
		" This is an unbound source copy; add its exact ID to "
		"Data/Effects/EffectCatalog.json and a typed gameplay cue before Product "
		"playback can select it.";
	if (bWasVisualProgramCopy)
	{
		m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		if (m_bActiveDocumentDrawable && Stage_WorldPreview(*m_ActiveDocument))
		{
			m_strDocumentStatus +=
				" The preview now uses the generic Authored renderer path.";
		}
		else if (m_bActiveDocumentDrawable)
		{
			m_strDocumentStatus +=
				" The file was saved, but generic Authored preview staging failed; the previous preview was preserved.";
		}
	}
    return true;
}

bool_t Client::CEffect_Tool::
	Try_SaveSelectedAdapterElementAsGenericAuthoredCopy(
		const std::string& strAssetId)
{
	if (!m_ActiveDocument.has_value() ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM !=
			m_eActiveDocumentSource ||
		nullptr == m_pSelectedVisualSourceProjection ||
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		m_strSelectedElementId.empty())
	{
		m_strDocumentStatus =
			"Select one admitted adapter Decal or Trail Element before creating a generic Authored starting copy.";
		return false;
	}
	if (Has_UnappliedDetailDraft())
	{
		m_strDocumentStatus =
			"Close or revert the open inspection draft before creating a generic Authored starting copy.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged;
	std::string Error;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
			m_pSelectedVisualSourceProjection->Get_Document(),
			m_strSelectedElementId, strAssetId, Staged, Error))
	{
		m_strDocumentStatus =
			"Generic Authored starting copy rejected: " + Error;
		return false;
	}
	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored" /
		(std::filesystem::path(strAssetId).wstring() + L".effect.json"));
	if (Path.empty())
	{
		m_strDocumentStatus =
			"Generic Authored starting copy path escaped Data/Effects/Authored.";
		return false;
	}
	if (std::filesystem::is_regular_file(Path))
	{
		m_strDocumentStatus =
			"Generic Authored starting copy refuses to overwrite an existing Effect ID.";
		return false;
	}
	CEffectDocumentRenderer StagingRenderer(m_pDevice, m_pContext);
	if (FAILED(StagingRenderer.Initialize()) ||
		!StagingRenderer.Stage_Document(Staged, Error))
	{
		m_strDocumentStatus =
			"Generic Authored starting copy preview preflight failed; the active Effect and Data File were preserved: " +
			Error;
		return false;
	}
	if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, Staged, std::string_view{}, Error))
	{
		m_strDocumentStatus = Error;
		return false;
	}

	Clear_ProductCuePreview();
	m_ActiveDocument = std::move(Staged);
	m_bMarkedElementIdsNeedPrune = true;
	Set_ActiveDocumentDrawableStatus(true, {});
	m_ActiveDocumentPath = Path;
	m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT;
	m_pSelectedVisualSourceProjection.reset();
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
	m_bDocumentDirty = false;
	m_bActiveDocumentMatchesRuntime = false;
	m_strSelectedDataFileAssetId = strAssetId;
	m_strSelectedDataFileElementId.clear();
	Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), strAssetId);
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	Recalculate_PreviewDuration();
	Refresh_RuntimeEquivalence();
	Refresh_DataFiles();
	Refresh_AllEffects();
	m_strDocumentStatus =
		"Saved one selected Decal/Trail as a generic Authored starting copy: " +
		Path.string() +
		". Detail, Material, and resource bindings were preserved through the ordinary codec; the exact adapter projector/VF/resource packet was intentionally not copied. The preview now uses the generic Authored renderer and can be edited/saved/reloaded normally.";
	return true;
}

bool_t Client::CEffect_Tool::Try_PromoteImportedDocument()
{
    if (!m_ActiveDocument.has_value() ||
        EFFECT_DOCUMENT_SOURCE::IMPORTED != m_eActiveDocumentSource)
    {
        m_strDocumentStatus =
            "Load an executable Imported Effect before promotion.";
        return false;
    }
    if (Has_UnappliedDetailDraft())
    {
        m_strDocumentStatus =
            "Apply or Revert the open Detail draft before promotion.";
        return false;
    }
    constexpr std::string_view Suffix = ".imported";
    const std::string& ImportedId = m_ActiveDocument->strEffectAssetId;
    if (ImportedId.size() <= Suffix.size() ||
        0 != ImportedId.compare(
            ImportedId.size() - Suffix.size(), Suffix.size(), Suffix))
    {
        m_strDocumentStatus =
            "Imported Effect ID does not have the canonical .imported suffix.";
        return false;
    }
    const std::string TargetId = ImportedId.substr(
        0u, ImportedId.size() - Suffix.size());
    std::string CatalogStatus;
    if (!Ensure_PlayerSkillCatalog(CatalogStatus))
    {
        m_strDocumentStatus = "PlayerSkills load failed: " + CatalogStatus;
        return false;
    }
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    const auto Skill = std::find_if(
        Skills.begin(), Skills.end(),
        [&TargetId](const PLAYER_SKILL_DEFINITION& Candidate)
        {
            return Candidate.strEffectId == TargetId;
        });
    if (Skill == Skills.end())
    {
        m_strDocumentStatus =
            "Promotion target is not owned by PlayerSkills: " + TargetId;
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.strEffectAssetId = TargetId;
    Staged.strDisplayName = Skill->strDisplayName;
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(TargetId).wstring() + L".effect.json"));
    std::string Error;
    std::string ExpectedCanonicalDocument;
	if (!Path.empty() && std::filesystem::is_regular_file(Path))
    {
        EFFECT_DOCUMENT_DESC Existing;
        if (!CEffectDocumentCodec::Load(Path, Existing, Error) ||
            Existing.strEffectAssetId != TargetId)
        {
            m_strDocumentStatus =
                "Existing Authored Effect could not preserve its Model Cues: " +
                Error;
            return false;
        }
		ExpectedCanonicalDocument = CEffectDocumentCodec::Serialize(Existing);
		if (Staged.ModelCues.empty())
			Staged.ModelCues = std::move(Existing.ModelCues);
    }
    if (!CEffectDocumentCodec::Validate_Drawable(Staged, Error))
    {
        m_strDocumentStatus = "Promotion validation failed: " + Error;
        return false;
    }
    if (!Stage_WorldPreview(Staged))
    {
        m_strDocumentStatus =
            "Promotion preview stage failed; existing Authored file preserved: " +
            m_strPreviewStatus;
        return false;
    }
    if (Path.empty() || !CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, Staged, ExpectedCanonicalDocument, Error))
    {
        Stage_WorldPreview(*m_ActiveDocument);
        m_strDocumentStatus = Path.empty() ?
            "Promotion path escaped Data/Effects/Authored." :
            "Promotion failed; existing Authored file restored: " + Error;
        return false;
    }
    Clear_ProductCuePreview();
    m_ActiveDocument = std::move(Staged);
    m_bMarkedElementIdsNeedPrune = true;
    Set_ActiveDocumentDrawableStatus(true, {});
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT;
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
    Refresh_RuntimeEquivalence();
    m_strSelectedDataFileAssetId = TargetId;
	m_strSelectedDataFileElementId.clear();
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), TargetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    Refresh_DataFiles();
    Refresh_AllEffects();
    Start_WorldPreviewFromBeginning();
    m_strDocumentStatus =
        "Promoted Imported Effect to Authored skill atomically: " +
        Path.string();
    return true;
}

bool_t Client::CEffect_Tool::Try_ReloadActiveDocument(
	const bool_t bDiscardActiveDocumentDraft)
{
	const bool_t bRuntimeView =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource;
	if (!m_ActiveDocument.has_value() ||
		(m_ActiveDocumentPath.empty() && !bRuntimeView))
    {
        m_strDocumentStatus = "The active Effect has no saved source to reload.";
        return false;
    }
    if (Has_UnsavedWork() && !bDiscardActiveDocumentDraft)
    {
        m_strDocumentStatus =
            "Save or Discard changes before Reload.";
        return false;
    }
	if (bDiscardActiveDocumentDraft &&
		(m_bOccurrenceTuningDirty || m_bOccurrenceTransformDraftDirty ||
		 m_bValtanAreaMapEffectDirty ||
		 m_UnpublishedStaticAreaWorldDraft.has_value()))
	{
		m_strDocumentStatus =
			"Discard Current Effect refused to discard occurrence or Area work; use that typed owner first.";
		return false;
	}
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanPreview =
		m_ValtanProductPreview;
	const bool_t bPreviousValtanPatternDraft =
		EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
			m_eActiveDocumentPreviewIntent;
	const std::string strPreviousValtanPatternDraftId =
		m_strActiveValtanPatternDraftId;
	const VALTAN_PATTERN_PREVIEW_PATH ePreviousValtanPatternDraftPath =
		m_eActiveValtanPatternDraftPreviewPath;
	const bool_t bPreviousStaticAreaPreview =
		EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
			m_eActiveDocumentPreviewIntent;
	const optional<MAP_EFFECT_WORLD_PRESENTATION>
		PreviousStaticAreaPresentation = m_StaticAreaPreviewPresentation;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds =
		m_fPreviewDurationSeconds;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	std::string ValtanRestoreError;
	/* Parse/identity/drawability checks in Try_LoadDocumentPath all precede its
	   Release_WorldPreview commit boundary. A failed load therefore leaves the
	   current exact Valtan object and pose untouched; only a successful document
	   commit needs the full restore transaction below. */
	if (!Try_LoadDocumentPathStaged(
		m_ActiveDocumentPath, m_eActiveDocumentSource,
			bRuntimeView ? m_strSelectedDataFileAssetId :
				m_ActiveDocument->strEffectAssetId,
		bDiscardActiveDocumentDraft,
		m_eActiveDocumentPreviewIntent))
	{
		return false;
	}
	if (bDiscardActiveDocumentDraft)
	{
		m_strDocumentStatus +=
			" Discarded only the previous in-memory Current Effect/Detail draft; saved bytes were reloaded.";
	}
	if (bPreviousValtanPatternDraft)
	{
		m_strActiveValtanPatternDraftId =
			strPreviousValtanPatternDraftId;
		m_eActiveValtanPatternDraftPreviewPath =
			ePreviousValtanPatternDraftPath;
		if (!Prepare_ActiveValtanPatternDraftTimeline(
				!bPreviousPreviewPlaying))
		{
			m_strDocumentStatus +=
				" The Pattern Draft document reloaded, but its selected Pattern timeline could not be restored: " +
				m_strPreviewAnimationStatus;
			return true;
		}
		if (bPreviousPreviewPlaying && !Try_PlayActiveUnifiedEffect())
		{
			Set_SynchronizedAnimationPaused(true);
			m_strDocumentStatus +=
				" The Pattern Draft timeline was restored at t=0, but its Effect preview could not restart: " +
				m_strPreviewStatus;
		}
		return true;
	}
	if (bPreviousStaticAreaPreview)
	{
		m_StaticAreaPreviewPresentation = PreviousStaticAreaPresentation;
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_bPreviewVisibleRequested = bPreviousPreviewVisibleRequested;
		m_bPreviewPlaying = false;
		if (!Update_StaticAreaPreviewRoot())
		{
			m_strDocumentStatus +=
				" The static placement transform could not be restored; no actor fallback was used.";
			return true;
		}
		if (!bPreviousPreviewVisibleRequested)
			return true;
		if (!Stage_WorldPreview())
		{
			m_strDocumentStatus +=
				" The static placement preview could not be restaged: " +
				m_strPreviewStatus;
			return true;
		}
		const shared_ptr<CEffectObject> pPreview =
			m_pWorldPreviewObject.lock();
		if (nullptr == pPreview)
		{
			m_strDocumentStatus +=
				" The static placement preview object was not committed.";
			return true;
		}
		pPreview->Set_RootWorld(m_PreviewWorldRoot);
		if (bPreviousPreviewPlaying)
		{
			Start_WorldPreviewFromBeginning();
		}
		else
		{
			pPreview->Set_SampleTime(Resolve_EffectSampleTime(
				fPreviousPreviewTimeSeconds));
			pPreview->Set_Playing(false);
			pPreview->Set_Visible(true);
			m_bPreviewPlaying = false;
		}
		return true;
	}
	if (!PreviousValtanPreview.has_value())
		return true;

	/* The saved document commit succeeded.  Rebuild its exact Valtan cue
	   duration, then restore the previous wall clock and held pose. */
	m_ValtanProductPreview = PreviousValtanPreview;
	Recalculate_PreviewDuration();
	const f32_t fReloadedPreviewDurationSeconds =
		m_fPreviewDurationSeconds;
	if (Restore_ValtanProductPreviewPlayback(
			PreviousValtanPreview,
			fPreviousPreviewTimeSeconds,
			fReloadedPreviewDurationSeconds,
			bPreviousPreviewPlaying,
			bPreviousPreviewVisibleRequested,
			PreviousProductCueSnapshotRoot,
			bPreviousProductCueSnapshotCaptured,
			ValtanRestoreError))
	{
		return true;
	}

	m_ValtanProductPreview.reset();
	m_strDocumentStatus +=
		" The document reload committed, but its Valtan cue preview could "
		"not be replayed; the saved document remains loaded. Preview restore "
		"failed: " + ValtanRestoreError;
	return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument(
    const std::string& strAssetId)
{
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strAssetId).wstring() + L".effect.json"));
    return Try_LoadDocumentPath(
        Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strAssetId);
}

bool_t Client::CEffect_Tool::Try_LoadDocumentPath(
    const std::filesystem::path& Path,
    const EFFECT_DOCUMENT_SOURCE eSource,
    const std::string& strSelectionId,
	const EFFECT_DOCUMENT_PREVIEW_INTENT ePreviewIntent)
{
    return Try_LoadDocumentPathStaged(
        Path, eSource, strSelectionId, false, ePreviewIntent);
}

bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged(
    const std::filesystem::path& Path,
    const EFFECT_DOCUMENT_SOURCE eSource,
    const std::string& strSelectionId,
    const bool_t bBypassUnsavedGuard,
	EFFECT_DOCUMENT_PREVIEW_INTENT ePreviewIntent)
{
    // Scene-anchored documents use the existing sequencer only on Play/Solo.
    // STANDALONE_EFFECT is the legacy static Valtan target contract.
    if (Is_SceneAnchoredEffectAssetId(strSelectionId) &&
        ePreviewIntent == EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT)
        ePreviewIntent = EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT;
    Engine::CProfilerScope LoadProfile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.DocumentLoad");
    if (EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE == eSource)
    {
        m_strDocumentStatus =
            "Extraction drafts are reference-only and cannot be played until "
            "they are converted to a validated Effect Document.";
        return false;
    }
    if (!bBypassUnsavedGuard && Has_UnsavedWork())
    {
        m_PendingDocumentLoad = PENDING_DOCUMENT_LOAD{
            Path, strSelectionId, eSource, ePreviewIntent };
        m_bPendingDocumentLoadModalRequested = true;
        m_strDocumentStatus =
            "Pending Effect load requires Save, Discard, or Cancel.";
        return false;
    }
	EFFECT_DOCUMENT_DESC Staged;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pStagedVisualProjection;
	std::string Error;
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == eSource)
	{
		constexpr std::string_view Suffix = "::assembly";
		if (!strSelectionId.ends_with(Suffix))
		{
			m_strDocumentStatus = "Runtime Assembly selection ID is invalid.";
			return false;
		}
		const std::string EffectId = strSelectionId.substr(
			0u, strSelectionId.size() - Suffix.size());
		const std::shared_ptr<const EFFECT_DOCUMENT_DESC> Runtime =
			CEffectCatalog::Find(EffectId);
		if (nullptr == Runtime)
		{
			m_strDocumentStatus =
				"Runtime Assembly is no longer admitted by EffectCatalog: " + EffectId;
			return false;
		}
		Staged = *Runtime;
	}
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource)
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(strSelectionId);
		if (nullptr == Component)
		{
			m_strDocumentStatus =
				"WFX Component is no longer admitted by EffectCatalog: " +
				strSelectionId;
			return false;
		}
		Staged = Component->Document;
	}
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM == eSource)
	{
		pStagedVisualProjection =
			CEffectCatalog::Find_VisualProjection(strSelectionId);
		if (nullptr == pStagedVisualProjection)
		{
			m_strDocumentStatus =
				"Visual Program is no longer admitted by EffectCatalog: " +
				strSelectionId;
			return false;
		}
		Staged = pStagedVisualProjection->Get_Document();
	}
    else
    {
        if (Path.empty())
        {
            m_strDocumentStatus =
                "Effect load path escaped Data/Effects/Authored.";
            return false;
        }
        Engine::CProfilerScope ParseProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.Parse");
        if (!CEffectDocumentCodec::Load(Path, Staged, Error))
        {
            m_strDocumentStatus = Error;
            return false;
        }
		if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource &&
			Staged.strEffectAssetId != strSelectionId)
		{
			m_strDocumentStatus =
				"Authored Effect identity mismatch; selected '" + strSelectionId +
				"', file contains '" + Staged.strEffectAssetId +
				"'. The previous Current Effect was preserved.";
			return false;
		}
    }
    std::string PreviewStatus;
    bool_t bDrawable = false;
    {
        Engine::CProfilerScope ValidationProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.ValidateDrawable");
        bDrawable =
            CEffectDocumentCodec::Validate_Drawable(Staged, PreviewStatus);
    }
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT == ePreviewIntent &&
		!Prepare_ValtanStandaloneEffectTarget())
	{
		m_strDocumentStatus = m_strPreviewAnimationStatus;
		return false;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
			ePreviewIntent &&
		(!m_StaticAreaPreviewPresentation.has_value() ||
		 m_StaticAreaPreviewPresentation->effectAssetId !=
			Staged.strEffectAssetId))
	{
		m_strDocumentStatus =
			"Static Area Effect load requires one exact typed placement transform.";
		return false;
	}
	optional<REGISTRY_BOUND_AUDITION_PROVENANCE> StagedAuditionProvenance;
	if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
	{
		const auto Current = m_DirectAuthoredEditableEntries.find(
			Staged.strEffectAssetId);
		const bool_t bReloadingKnownAudition =
			m_ActiveRegistryBoundAuditionProvenance.has_value() &&
			m_ActiveRegistryBoundAuditionProvenance->strEffectAssetId ==
				Staged.strEffectAssetId;
		if (Current != m_DirectAuthoredEditableEntries.end() &&
			Current->second.bRegistryBoundAuditionOnly)
		{
			const DIRECT_AUTHORED_EDITABLE_ENTRY& Entry = Current->second;
			std::string FreshnessStatus;
			EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY Expected;
			Expected.strEffectAssetId = Staged.strEffectAssetId;
			Expected.Path = Entry.Path;
			Expected.bRegistryBoundAuditionOnly = true;
			Expected.strSourceEffectAssetId = Entry.strSourceEffectAssetId;
			Expected.SourceDocumentPath = Entry.SourceDocumentPath;
			Expected.strSourceDocumentRawSha256 =
				Entry.strSourceDocumentRawSha256;
			if (!CEffectDirectAuthoredSourceIndex::
					Validate_RegistryBoundAuditionCatalogProvenanceFresh(
						CProjectDataRoot::Resolve(
							std::filesystem::path(L"Effects") /
							L"EffectCatalog.json"),
						CProjectDataRoot::Resolve(
							std::filesystem::path(L"Effects") /
							L"EffectAuditionCatalog.json"),
						CProjectDataRoot::Resolve(
							std::filesystem::path(L"Effects") / L"Authored"),
						Expected, FreshnessStatus) ||
				!Entry.bAuditionSourceFreshnessValid ||
				!Validate_RegistryBoundAuditionSourceFreshness(
					Entry.SourceDocumentPath,
					Entry.strSourceDocumentRawSha256,
					FreshnessStatus))
			{
				m_strDocumentStatus =
					"Open rejected: registry-bound audition source freshness failed. " +
					(!FreshnessStatus.empty() ? FreshnessStatus :
					 (Entry.bAuditionSourceFreshnessValid ?
						"Live audition provenance validation failed." :
						"Correct the source/hash pair and Refresh Index first."));
				return false;
			}
			StagedAuditionProvenance = REGISTRY_BOUND_AUDITION_PROVENANCE{
				Staged.strEffectAssetId,
				Entry.Path,
				Entry.strSourceEffectAssetId,
				Entry.SourceDocumentPath,
				Entry.strSourceDocumentRawSha256 };
		}
		else if (bReloadingKnownAudition)
		{
			m_strDocumentStatus =
				"Open rejected: the active registry-bound audition row disappeared or was reclassified. Refresh Index and reopen only after restoring its audition metadata.";
			return false;
		}
	}
    if (m_pAuthoringSequencer && m_pAuthoringSequencer->Is_ElementPreview())
        m_pAuthoringSequencer->Stop();
    if (!m_ActiveDocument || m_ActiveDocument->strEffectAssetId != Staged.strEffectAssetId)
        m_MarkedElementIds.clear();
	Reset_RuntimeOccurrenceTuningSession();
	m_pSelectedVisualSourceProjection = std::move(pStagedVisualProjection);
    Release_WorldPreview(true);
    std::string CanonicalBaseline;
	if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
    {
        Engine::CProfilerScope CanonicalProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.CanonicalBaseline");
        CanonicalBaseline = CEffectDocumentCodec::Serialize(Staged);
    }
	optional<EFFECT_PRODUCT_PREVIEW> RetainedProductPreview;
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT ==
			ePreviewIntent && m_ProductPreview.has_value() &&
		m_ProductPreview->ProductCue.Cue.strEffectAssetId ==
			Staged.strEffectAssetId)
	{
		RetainedProductPreview = m_ProductPreview;
	}
	Clear_ProductCuePreview();
	if (RetainedProductPreview.has_value())
		m_ProductPreview = std::move(RetainedProductPreview);
	m_ActiveDocument = std::move(Staged);
	m_bMarkedElementIdsNeedPrune = true;
	m_ActiveRegistryBoundAuditionProvenance =
		std::move(StagedAuditionProvenance);
    Set_ActiveDocumentDrawableStatus(bDrawable, PreviewStatus);
    m_ActiveDocumentPath = Path;
	m_strActiveDocumentBaselineCanonical = CanonicalBaseline;
	m_eActiveDocumentSource = eSource;
	m_eActiveDocumentPreviewIntent = ePreviewIntent;
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT !=
		ePreviewIntent)
	{
		m_strActiveValtanPatternDraftId.clear();
		m_eActiveValtanPatternDraftPreviewPath =
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT !=
		ePreviewIntent)
	{
		m_StaticAreaPreviewPresentation.reset();
		m_UnpublishedStaticAreaWorldDraft.reset();
	}
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	Reset_ModelCueDraft();
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == eSource)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::SKILL;
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::COMPONENT;
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM == eSource)
		m_eDetailSelection = m_ActiveDocument->Elements.empty() ?
			EFFECT_DETAIL_SELECTION::NONE : EFFECT_DETAIL_SELECTION::ELEMENT;
	else
	{
		m_eDetailSelection = !m_ActiveDocument->Elements.empty() ?
			EFFECT_DETAIL_SELECTION::ELEMENT :
			(!m_ActiveDocument->ModelCues.empty() ?
				EFFECT_DETAIL_SELECTION::MODEL_CUE :
				EFFECT_DETAIL_SELECTION::NONE);
	}
    m_strSelectedElementId =
        EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
            m_ActiveDocument->Elements.front().strElementId : std::string{};
	m_strSelectedElementGroupId =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
			m_ActiveDocument->Elements.front().strGroupId : std::string{};
	m_strSelectedModelCueId =
		EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection ?
			m_ActiveDocument->ModelCues.front().strCueId : std::string{};
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	m_strSelectedComponentId =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource ?
			strSelectionId : std::string{};
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    if (const EFFECT_ELEMENT_DESC* pSelected = Find_SelectedElement())
    {
        m_eSelectedEffectType = pSelected->eKind;
        m_strSelectedResourceSlotId = Default_SlotId(pSelected->eKind);
        m_eResourceLibraryFileKind = Slot_FileKind(
            *pSelected, m_strSelectedResourceSlotId);
    }
    m_strSelectedDataFileAssetId = strSelectionId;
	m_strSelectedDataFileElementId.clear();
	std::string SuggestedAssetId = m_ActiveDocument->strEffectAssetId;
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM == eSource &&
		SuggestedAssetId.size() + std::string_view(".authored-copy").size() <
			m_NewAssetId.size())
	{
		SuggestedAssetId += ".authored-copy";
	}
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), SuggestedAssetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    m_bDocumentDirty = false;
    Refresh_RuntimeEquivalence();
    m_PendingDocumentLoad.reset();
    m_fPreviewTimeSeconds = 0.f;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT == ePreviewIntent)
	{
		/* The caller owns the exact Pattern identity. It is committed only after
		   the unsaved-load guard succeeds, then stages the ordered timeline. */
		Reset_SynchronizedAnimationSequence();
		Reset_ValtanBossPatternTransformHistory();
	}
	else if (EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
		ePreviewIntent)
	{
		/* Prepare_ValtanStandaloneEffectTarget already staged and paused the
		   dedicated boss target before this document commit. */
	}
	else if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
		ePreviewIntent)
	{
		if (!Update_StaticAreaPreviewRoot())
		{
			m_strPreviewStatus =
				"Static Area placement transform was invalid after document commit.";
		}
	}
	else if (!m_pAuthoringSequencer)
	{
		Synchronize_LoadedSkillPreview();
	}
    m_bPreviewVisibleRequested = false;
    m_bPreviewPlaying = false;
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT !=
			ePreviewIntent &&
		(bDrawable || EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
			ePreviewIntent ||
		 EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
			ePreviewIntent))
        Set_SynchronizedAnimationPaused(true);

    m_strPreviewStatus = bDrawable ?
        "Document loaded; GPU resources are deferred until an explicit preview scope is played." :
        "Preview is unavailable until required resources bind: " + PreviewStatus;
    if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
    {
        m_strDocumentStatus = "Loaded saved Effect '" +
            m_ActiveDocument->strEffectAssetId +
            "' with its existing name; " +
			(EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
				std::string("selected its first Element for Effect Details.") :
				(EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection ?
					std::string("selected its first Model / Summon cue for Effect Details.") :
					std::string("it has no Element or Model Cue to select.")));
    }
    else
    {
        m_strDocumentStatus = bDrawable ?
            "Loaded advanced document for inspection without GPU staging; choose Complete, Group, or Solo Play: " +
				(Path.empty() ? strSelectionId : Path.string()) :
            "Loaded advanced draft; preview is hidden until required resources bind: " +
                PreviewStatus;
    }
    return true;
}

bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(
    const bool_t bSaveFirst)
{
    if (!m_PendingDocumentLoad.has_value())
    {
        m_strDocumentStatus = "No pending Effect document load exists.";
        return false;
    }
    if (bSaveFirst)
    {
		if (m_UnpublishedStaticAreaWorldDraft.has_value())
		{
			m_strDocumentStatus =
				"Register the unsaved static world Effect in Area or discard it before loading another document.";
			return false;
		}
        if (Has_UnappliedDetailDraft())
        {
            m_strDocumentStatus =
                "Apply or Revert the open Detail draft before Save & Load.";
            return false;
        }
		if ((m_bDocumentDirty || m_bOccurrenceTuningDirty) &&
			!Try_SaveDocument())
		{
			return false;
		}
		if (m_bValtanAreaMapEffectDirty &&
			!Try_SavePublishValtanAreaStaticEffects())
		{
			m_strDocumentStatus = m_strValtanAreaMapEffectStatus;
			return false;
		}
    }

	const PENDING_DOCUMENT_LOAD Pending = *m_PendingDocumentLoad;
	optional<CMapEffectDocument> DiscardedAreaDraft;
	if (!bSaveFirst && m_bValtanAreaMapEffectDirty)
	{
		DiscardedAreaDraft = m_ValtanAreaMapEffectDocument;
		if (!Discard_ValtanAreaStaticEffectDraft())
		{
			m_strDocumentStatus = m_strValtanAreaMapEffectStatus;
			return false;
		}
	}
	if (!Try_LoadDocumentPathStaged(
		Pending.Path,
		Pending.eSource,
		Pending.strSelectionId,
		true,
		Pending.ePreviewIntent))
	{
		if (EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
				Pending.ePreviewIntent && !Pending.strValtanPatternId.empty())
		{
			m_strValtanPatternEffectStatus =
				"Open Existing Effect failed; the current Effect is unchanged: " +
				m_strDocumentStatus;
		}
		if (DiscardedAreaDraft.has_value())
		{
			m_ValtanAreaMapEffectDocument =
				std::move(*DiscardedAreaDraft);
			m_bValtanAreaMapEffectDirty = true;
			m_strValtanAreaMapEffectStatus =
				"Pending load failed; the discarded Area draft was restored in memory.";
		}
		return false;
	}
	const auto CompleteValtanPreviewPartial =
		[this, &Pending](std::string Reason)
		{
			/* The document load already committed and cleared the pending target.
			   Close the modal instead of leaving a stale Save/Discard operation. */
			m_PendingDocumentLoad.reset();
			m_bPendingDocumentLoadModalRequested = false;
			m_strDocumentStatus =
				"Loaded saved Effect '" + Pending.strSelectionId +
				"', but its Valtan animation/effect preview could not be staged. "
				"The document remains loaded: " + std::move(Reason);
			return true;
		};
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
		Pending.ePreviewIntent)
	{
		if (Pending.strValtanPatternId.empty())
		{
			return CompleteValtanPreviewPartial(
				"pending Pattern Draft lost its exact Pattern identity");
		}
		m_strActiveValtanPatternDraftId = Pending.strValtanPatternId;
		m_eActiveValtanPatternDraftPreviewPath =
			Pending.eValtanPatternPreviewPath;
		m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
		m_strPreviewAnchorSlotId.clear();
		Copy_Buffer(m_PreviewAnchorBuffer.data(),
			m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
		if (!Prepare_ActiveValtanPatternDraftTimeline(true))
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewAnimationStatus.empty() ?
					std::string("Pattern Draft timeline unavailable") :
					m_strPreviewAnimationStatus);
		}
		if (Pending.bPlayCompleteAfterLoad &&
			!Try_PlayActiveUnifiedEffect())
		{
			Set_SynchronizedAnimationPaused(true);
			return CompleteValtanPreviewPartial(
				m_strPreviewStatus.empty() ?
					std::string("Pattern Draft Effect preview unavailable") :
					m_strPreviewStatus);
		}
		return true;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
		Pending.ePreviewIntent)
	{
		if (!Pending.strValtanIndependentEffectId.empty())
		{
			const auto Independent = std::find_if(
				m_ValtanPatternTree.IndependentEffects.begin(),
				m_ValtanPatternTree.IndependentEffects.end(),
				[&Pending](const VALTAN_INDEPENDENT_EFFECT_VIEW& Candidate)
				{
					return Candidate.strIndependentEffectId ==
						Pending.strValtanIndependentEffectId;
				});
			const VALTAN_PATTERN_VIEW* pOwnerPattern =
				Find_ValtanPattern(Pending.strValtanPatternId);
			if (Independent == m_ValtanPatternTree.IndependentEffects.end() ||
				nullptr == pOwnerPattern ||
				!Pending.bPlayCompleteAfterLoad ||
				!Try_PlayValtanCombatObjectIndependentEffect(
					Pending.Path, *Independent, *pOwnerPattern))
			{
				return CompleteValtanPreviewPartial(
					m_strPreviewStatus.empty() ?
						std::string("independent combat-object lifecycle unavailable") :
						m_strPreviewStatus);
			}
			return true;
		}
		if (!Pending.strValtanPatternId.empty())
		{
			m_strSelectedValtanPatternId = Pending.strValtanPatternId;
			Select_SharedCompletePlayPattern(
				m_strSelectedValtanPatternId);
			m_SelectedValtanPatternEffect.reset();
			m_strValtanPatternEffectStatus =
				"Opened the existing aggregate Effect for authoring only: " +
				Pending.strSelectionId + ". Product connections were not changed.";
		}
		if (Pending.bPlayCompleteAfterLoad &&
			!Try_PlayActiveUnifiedEffect())
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewStatus.empty() ?
					std::string("standalone Effect preview unavailable") :
					m_strPreviewStatus);
		}
		return true;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
		Pending.ePreviewIntent)
	{
		if (!Update_StaticAreaPreviewRoot())
		{
			return CompleteValtanPreviewPartial(
				"static Area placement transform is unavailable");
		}
		if (!Pending.bPreviewAtPlacementAfterLoad)
			return true;
		if (!Stage_WorldPreview())
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewStatus.empty() ?
					std::string("static Area Effect preview unavailable") :
					m_strPreviewStatus);
		}
		const shared_ptr<CEffectObject> pPreview =
			m_pWorldPreviewObject.lock();
		if (nullptr == pPreview)
		{
			return CompleteValtanPreviewPartial(
				"static Area Effect preview object was not committed");
		}
		pPreview->Set_RootWorld(m_PreviewWorldRoot);
		pPreview->Set_SampleTime(0.f);
		pPreview->Set_Playing(false);
		pPreview->Set_Visible(true);
		m_bPreviewVisibleRequested = true;
		m_bPreviewPlaying = false;
		if (Pending.bPlayCompleteAfterLoad)
			Start_WorldPreviewFromBeginning();
		return true;
	}
	if (Pending.ValtanProductPreview.has_value() ||
		(Pending.ValtanClip.has_value() && Pending.ValtanCue.has_value()))
	{
		const bool_t bPreviewReady = Pending.ValtanProductPreview.has_value() ?
			Play_ValtanProductCue(*Pending.ValtanProductPreview) :
			Play_ValtanProductCue(*Pending.ValtanClip, *Pending.ValtanCue);
		if (!bPreviewReady)
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewAnimationStatus.empty() ?
					std::string("animation target unavailable") :
					m_strPreviewAnimationStatus);
		}
		if (Pending.bPlayCompleteAfterLoad)
		{
			if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
			{
				return CompleteValtanPreviewPartial(
					m_strPreviewStatus.empty() ?
						std::string("Effect world preview unavailable") :
						m_strPreviewStatus);
			}
			Start_WorldPreviewFromBeginning();
		}
		else
		{
			Set_SynchronizedAnimationPaused(true);
		}
		return true;
	}
	if (Pending.ValtanReferenceClips.has_value())
	{
		m_iValtanWorldOwnerStageDurationMs =
			Pending.iValtanWorldOwnerStageDurationMs;
		m_iValtanReferenceEffectStartMs =
			Pending.iValtanReferenceEffectStartMs;
		if (0u != Pending.iValtanWorldOwnerStageDurationMs)
		{
			m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
			m_strPreviewAnchorSlotId.clear();
			Copy_Buffer(m_PreviewAnchorBuffer.data(),
				m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
		}
		Recalculate_PreviewDuration();
		const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips =
			*Pending.ValtanReferenceClips;
		const bool_t bTargetReady = Clips.empty() ?
			(nullptr != m_pCharacterPreviewPanel &&
			 (CAnimationTargetService::Resolve_AssetName() ==
				VALTAN_ANIMATION_ASSET_NAME ||
			  m_pCharacterPreviewPanel->Select_TargetAsset(
				VALTAN_ANIMATION_ASSET_NAME))) :
			Play_ValtanStageSequence(Clips);
		if (!bTargetReady)
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewAnimationStatus.empty() ?
					std::string("Valtan model or ordered animation unavailable") :
					m_strPreviewAnimationStatus);
		}
		if (0u != Pending.iValtanWorldOwnerStageDurationMs &&
			!Try_SnapshotValtanWorldPreviewRoot())
		{
			if (Pending.bPlayCompleteAfterLoad)
			{
				return CompleteValtanPreviewPartial(
					m_strPreviewStatus.empty() ?
						std::string("Valtan world preview root unavailable") :
						m_strPreviewStatus);
			}
		}
		if (Pending.bPlayCompleteAfterLoad)
		{
			if (!Try_PlayActiveUnifiedEffect())
			{
				return CompleteValtanPreviewPartial(
					m_strPreviewStatus.empty() ?
						std::string("Effect world preview unavailable") :
						m_strPreviewStatus);
			}
		}
		else
		{
			Set_SynchronizedAnimationPaused(true);
		}
		return true;
	}
	if (Pending.bPlayCompleteAfterLoad)
	{
		if (Is_SceneAnchoredEffectAssetId(Pending.strSelectionId))
		{
			if (!Try_PlayActiveUnifiedEffect())
			{
				const std::string Reason = m_strPreviewStatus.empty() ?
					"Scene-anchored Effect preview is unavailable." : m_strPreviewStatus;
				m_strDocumentStatus = "Loaded saved Effect '" + Pending.strSelectionId +
					"', but its scene-anchored preview could not be started. The document remains loaded: " + Reason;
				m_strElementStatus = m_strDocumentStatus;
			}
			return true;
		}
		if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
			return false;
		Start_WorldPreviewFromBeginning();
		return true;
	}
	if (!Pending.strElementSelectionId.empty())
	{
		return Try_SelectElement(Pending.strSelectionId,
			Pending.strElementSelectionId);
	}
	if (!Pending.strModelCueSelectionId.empty())
	{
		return Try_SelectModelCue(Pending.strSelectionId,
			Pending.strModelCueSelectionId);
	}
	return true;
}

bool_t Client::CEffect_Tool::Refresh_AllEffects(
    const bool_t bReloadSkillCatalog)
{
    m_bAllEffectsRefreshAttempted = true;
	if (bReloadSkillCatalog)
		Reset_ArtistFPreparationFailureLatch();
    std::string CatalogStatus;
    if ((bReloadSkillCatalog || CPlayerSkillCatalog::Get_Skills().empty()) &&
        !CPlayerSkillCatalog::Load(CatalogStatus))
    {
        m_strElementStatus =
            "All Effects refresh preserved the previous tree: " +
            CatalogStatus;
        return false;
    }

    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    vector<EFFECT_SKILL_TREE_ENTRY> Staged;
    Staged.reserve(Skills.size());
    std::map<LostArk::Shared::SKILL_ID, size_t> EntryIndices;
    for (const PLAYER_SKILL_DEFINITION& Skill : Skills)
    {
        EntryIndices.emplace(Skill.iSkillId, Staged.size());
        EFFECT_SKILL_TREE_ENTRY Entry;
        Entry.Skill = Skill;
        Staged.push_back(std::move(Entry));
    }
	const bool_t bHadPreviousAllEffectsTree = !m_AllEffects.empty();
	if (!bHadPreviousAllEffectsTree)
	{
		/* PlayerSkills owns the navigation tree. Product presentation is optional
		   child data and must never hide Q/W/E/R/T/A/S/D/F on a cold entry. */
		m_AllEffects = Staged;
	}
    const auto FailRefresh = [this, bHadPreviousAllEffectsTree](
		const std::string& strReason)
    {
        m_strElementStatus =
			(bHadPreviousAllEffectsTree ?
				"All Effects Product enrichment preserved the previous tree: " :
				"All Effects Product enrichment is unavailable; base PlayerSkills rows remain visible: ") +
			strReason;
        return false;
    };
    constexpr LostArk::Shared::CHARACTER_CLASS_ID Classes[] = {
        LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
        LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
        LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::WARLORD };
    size_t iProductCueCount = 0u;
    size_t iProductSkillCount = 0u;
    size_t iSourceReferenceCount = 0u;
    std::set<std::string> MissingAuthoredTargets;
    for (const LostArk::Shared::CHARACTER_CLASS_ID eClass : Classes)
    {
        const char* pAnimationAsset = Animation_AssetName(eClass);
        if (nullptr == pAnimationAsset)
            return FailRefresh("a playable class has no animation asset ID.");

        std::string BindingText;
        std::string PresentationStatus;
        const std::filesystem::path BindingPath =
            CAnimationSkillBindingDocument::Resolve_Path(pAnimationAsset);
        if (!Read_TextFile(BindingPath, BindingText, PresentationStatus))
            return FailRefresh(PresentationStatus);

        ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
        if (!CAnimationSkillBindingDocument::Parse_Text(
            BindingText, Bindings, PresentationStatus))
        {
            return FailRefresh(PresentationStatus);
        }
        vector<string> BoundClipNames;
		struct BOUND_CLIP_OWNER final
		{
			LostArk::Shared::SKILL_ID iSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			ANIMATION_SKILL_CLIP Clip;
			size_t iBoundClipOrdinal = 0u;
			size_t iStageIndex = 0u;
			size_t iStageClipIndex = 0u;
		};
        std::map<string, std::vector<BOUND_CLIP_OWNER>> ClipOwners;
        for (const ANIMATION_SKILL_BINDING& Binding : Bindings.Bindings)
        {
            const auto EntryIndex = EntryIndices.find(Binding.iSkillId);
            if (EntryIndex == EntryIndices.end() ||
                Staged[EntryIndex->second].Skill.eCharacterClass != eClass)
            {
                return FailRefresh(
                    "a skill binding is not owned by its PlayerSkills class.");
            }
            const vector<ANIMATION_SKILL_CLIP> BindingClips =
                Flatten_BindingClips(Binding);
            if (BindingClips.empty())
                return FailRefresh("a skill binding has no animation clips.");
			size_t iBoundClipOrdinal = 0u;
			for (size_t iStage = 0u; iStage < Binding.Stages.size(); ++iStage)
            {
				const ANIMATION_SKILL_STAGE& Stage = Binding.Stages[iStage];
				for (size_t iStageClip = 0u;
					iStageClip < Stage.Clips.size();
					++iStageClip, ++iBoundClipOrdinal)
				{
					const string& strClipName =
						Stage.Clips[iStageClip].strClipName;
					BoundClipNames.push_back(strClipName);
					auto& Owners = ClipOwners[strClipName];
					if (std::any_of(Owners.begin(), Owners.end(),
						[&Binding](const BOUND_CLIP_OWNER& Owner)
						{ return Owner.iSkillId != Binding.iSkillId; }))
					{
						return FailRefresh(
							"one animation clip is claimed by multiple skills: " +
							strClipName);
					}
					Owners.push_back(BOUND_CLIP_OWNER{
						Binding.iSkillId, Stage.Clips[iStageClip], iBoundClipOrdinal,
						iStage, iStageClip });
				}
            }
        }
        std::sort(BoundClipNames.begin(), BoundClipNames.end());
        BoundClipNames.erase(std::unique(
            BoundClipNames.begin(), BoundClipNames.end()),
            BoundClipNames.end());
        if (!CAnimationSkillBindingDocument::Validate(
            Bindings, pAnimationAsset, eClass, Skills,
            BoundClipNames, PresentationStatus))
        {
            return FailRefresh(PresentationStatus);
        }

        const std::filesystem::path EventPath = CProjectDataRoot::Resolve(
            std::filesystem::path(L"Animation") / L"Authored" /
            std::filesystem::path(pAnimationAsset) /
            (std::filesystem::path(pAnimationAsset).wstring() +
                L".animevents"));
        std::string EventText;
        if (!Read_TextFile(EventPath, EventText, PresentationStatus))
            return FailRefresh(PresentationStatus);

        ANIMATION_EFFECT_CUE_DOCUMENT CueDocument;
        if (!CAnimationEffectCueDocument::Load_FromText(
            pAnimationAsset, EventText, BoundClipNames,
			CueDocument, PresentationStatus, true))
        {
            return FailRefresh(PresentationStatus);
        }
        for (const ANIMATION_EFFECT_CUE& Cue : CueDocument.Cues)
        {
            const auto Owner = ClipOwners.find(Cue.strClipName);
            if (Owner == ClipOwners.end())
				continue;
			for (const BOUND_CLIP_OWNER& ClipOwner : Owner->second)
			{
				if (!CAnimationEffectCueDocument::Is_CueStartInClipWindow(
						ClipOwner.Clip, Cue.iStartMs))
				{
					continue;
				}
				const auto EntryIndex = EntryIndices.find(ClipOwner.iSkillId);
				if (EntryIndex == EntryIndices.end())
					return FailRefresh(
						"an admitted Effect cue has no skill row.");
				EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE ProductCue;
				ProductCue.Cue = Cue;
				ProductCue.Clip = ClipOwner.Clip;
				ProductCue.iBoundClipOrdinal =
					ClipOwner.iBoundClipOrdinal;
				ProductCue.iStageIndex = ClipOwner.iStageIndex;
				ProductCue.iStageClipIndex = ClipOwner.iStageClipIndex;
				Staged[EntryIndex->second].ProductCues.push_back(
					std::move(ProductCue));
			}
        }

        for (const PLAYER_SKILL_DEFINITION& Skill : Skills)
        {
            if (Skill.eCharacterClass != eClass)
                continue;
            const auto EntryIndex = EntryIndices.find(Skill.iSkillId);
            if (EntryIndex == EntryIndices.end())
            {
                return FailRefresh(
                    "a PlayerSkills row has no complete animation binding.");
            }
            EFFECT_SKILL_TREE_ENTRY& Entry = Staged[EntryIndex->second];
            std::sort(Entry.ProductCues.begin(), Entry.ProductCues.end(),
                [](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Left,
                    const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Right)
                {
                    return std::tie(
                        Left.iBoundClipOrdinal,
                        Left.Cue.iStartMs,
                        Left.Cue.strEffectAssetId,
                        Left.Cue.strAnchorSlotId) <
                        std::tie(
                            Right.iBoundClipOrdinal,
                            Right.Cue.iStartMs,
                            Right.Cue.strEffectAssetId,
                            Right.Cue.strAnchorSlotId);
                });
            if (!Entry.ProductCues.empty())
                ++iProductSkillCount;
            iProductCueCount += Entry.ProductCues.size();
            for (const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue :
                Entry.ProductCues)
            {
                const std::filesystem::path AuthoredPath =
                    CProjectDataRoot::Resolve(
                        std::filesystem::path(L"Effects") / L"Authored" /
                        (std::filesystem::path(
                            ProductCue.Cue.strEffectAssetId).wstring() +
                            L".effect.json"));
                if (AuthoredPath.empty() ||
                    !std::filesystem::is_regular_file(AuthoredPath))
                {
                    MissingAuthoredTargets.insert(
                        ProductCue.Cue.strEffectAssetId);
                }
            }
        }

        std::istringstream EventRows(EventText);
        std::string EventLine;
        std::getline(EventRows, EventLine);
        while (std::getline(EventRows, EventLine))
        {
            std::string strClipName;
            bool_t bImported = false;
            bool_t bEmptyPayload = false;
            if (!Try_ParseEffectDiagnosticRow(
                EventLine, strClipName, bImported, bEmptyPayload))
            {
                continue;
            }
            const auto Owner = ClipOwners.find(strClipName);
            if (Owner == ClipOwners.end())
                continue;
            const auto EntryIndex = EntryIndices.find(
				Owner->second.front().iSkillId);
            if (EntryIndex == EntryIndices.end())
                continue;
            EFFECT_SKILL_TREE_ENTRY& Entry = Staged[EntryIndex->second];
            ++Entry.iSourceReferenceCount;
            ++iSourceReferenceCount;
            if (bImported)
                ++Entry.iImportedReferenceCount;
            if (bEmptyPayload)
                ++Entry.iEmptySourceReferenceCount;
        }
    }
    std::sort(Staged.begin(), Staged.end(),
        [](const EFFECT_SKILL_TREE_ENTRY& Left,
            const EFFECT_SKILL_TREE_ENTRY& Right)
        {
            if (Left.Skill.eCharacterClass != Right.Skill.eCharacterClass)
                return Left.Skill.eCharacterClass < Right.Skill.eCharacterClass;
            if (Left.Skill.strInputSlot != Right.Skill.strInputSlot)
                return Left.Skill.strInputSlot < Right.Skill.strInputSlot;
            return Left.Skill.iSkillId < Right.Skill.iSkillId;
        });
    m_AllEffects = std::move(Staged);
    m_strElementStatus =
        "All Effects indexed Product presentation from PlayerSkills + "
        "skillbindings + animevents: " +
        std::to_string(m_AllEffects.size()) + " skills, " +
        std::to_string(iProductSkillCount) + " Product skills, " +
        std::to_string(iProductCueCount) + " executable cues, " +
        std::to_string(iSourceReferenceCount) +
		" Source/Imported read-only rows";
    if (!MissingAuthoredTargets.empty())
        m_strElementStatus += ", " +
            std::to_string(MissingAuthoredTargets.size()) +
            " Product targets have no Authored document";
    m_strElementStatus += ".";
    return true;
}

bool_t Client::CEffect_Tool::Refresh_DataFiles()
{
    m_bDataFilesRefreshAttempted = true;
    vector<EFFECT_DATA_FILE_ENTRY> Staged;
    std::set<std::string> AssetIds;
    std::set<std::string> StagedDomainIds;
    size_t iRejectedDocumentCount = 0u;
    std::string strFirstRejectedDocument;
    const auto RecordRejectedDocument =
        [&iRejectedDocumentCount, &strFirstRejectedDocument](
            const std::string& strReason)
    {
        ++iRejectedDocumentCount;
        if (strFirstRejectedDocument.empty())
            strFirstRejectedDocument = strReason;
    };
    for (const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain : m_ResourceDomains)
        StagedDomainIds.insert(Domain.strDomainId);

    std::map<std::string, std::string> SkillDomains;
    std::string SkillCatalogStatus;
    if (Ensure_PlayerSkillCatalog(SkillCatalogStatus))
    {
        for (const PLAYER_SKILL_DEFINITION& Skill :
            CPlayerSkillCatalog::Get_Skills())
        {
            const char* pDomainId = Resource_DomainId(Skill.eCharacterClass);
            if (!Skill.strEffectId.empty() && nullptr != pDomainId)
                SkillDomains.emplace(Skill.strEffectId, pDomainId);
        }
    }
    const auto ResolveDocumentDomain = [&SkillDomains](
        const std::string& strAssetId,
        const std::filesystem::path& RelativePath,
        const EFFECT_DOCUMENT_SOURCE eSource)
    {
        if (EFFECT_DOCUMENT_SOURCE::IMPORTED == eSource)
        {
            const std::string PathDomain = First_PathComponent(RelativePath);
            if (!PathDomain.empty())
                return PathDomain;
        }
        const auto SkillDomain = SkillDomains.find(strAssetId);
        if (SkillDomain != SkillDomains.end())
            return SkillDomain->second;
        return EffectAsset_DomainId(strAssetId);
    };
    const auto Scan = [this, &Staged, &AssetIds, &StagedDomainIds,
        &ResolveDocumentDomain, &RecordRejectedDocument](
        const std::filesystem::path& RelativeRoot,
        const EFFECT_DOCUMENT_SOURCE eSource) -> bool_t
    {
        const std::filesystem::path Root =
            CProjectDataRoot::Resolve(RelativeRoot);
        std::error_code Error;
        if (Root.empty() || !std::filesystem::exists(Root, Error))
        {
            RecordRejectedDocument(
                "missing Data Files root: " + Root.string());
            return true;
        }
        if (Error || !std::filesystem::is_directory(Root, Error))
        {
            m_strDocumentStatus = "Data Files root is invalid: " +
                Root.string();
            return false;
        }
        for (std::filesystem::recursive_directory_iterator Iterator(
            Root, std::filesystem::directory_options::skip_permission_denied,
            Error), End; Iterator != End; Iterator.increment(Error))
        {
            if (Error)
            {
                RecordRejectedDocument(
                    "Data Files directory iteration failed: " +
                    Root.string() + ": " + Error.message());
                Error.clear();
                break;
            }
            if (!Iterator->is_regular_file())
                continue;
            const std::string Name = Iterator->path().filename().string();
            if (!Name.ends_with(".effect.json"))
                continue;
            std::string EffectAssetId;
            if (!Try_DeriveEffectAssetIdFromFilename(
                Iterator->path(), eSource, EffectAssetId))
            {
                RecordRejectedDocument(
                    "noncanonical Effect filename: " +
                    Iterator->path().string());
                continue;
            }
            if (!AssetIds.insert(EffectAssetId).second)
            {
                RecordRejectedDocument(
                    "duplicate Effect ID: " + EffectAssetId);
                continue;
            }
            const std::filesystem::path RelativePath =
                Iterator->path().lexically_relative(Root);
            const std::string DomainId = ResolveDocumentDomain(
                EffectAssetId, RelativePath, eSource);
            StagedDomainIds.insert(DomainId);
			EFFECT_DATA_FILE_ENTRY Entry{
				EffectAssetId, DomainId, Iterator->path(), eSource };
			if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
			{
				Entry.strDocumentParseStatus =
					"Metadata indexed; Open or Play decodes this exact document on demand.";
			}
			Staged.push_back(std::move(Entry));
        }
        return true;
    };
    if (!Scan(L"Effects/Authored", EFFECT_DOCUMENT_SOURCE::AUTHORED) ||
        !Scan(L"Effects/Imported", EFFECT_DOCUMENT_SOURCE::IMPORTED))
        return false;

	size_t iRuntimeAssemblyCount = 0u;
	size_t iRuntimeComponentCount = 0u;
	for (const std::string& EffectId : CEffectCatalog::Get_EffectAssetIds())
	{
		if (nullptr == CEffectCatalog::Find_Assembly(EffectId))
			continue;
		const std::string SelectionId = EffectId + "::assembly";
		if (!AssetIds.insert(SelectionId).second)
		{
			m_strDocumentStatus =
				"Data Files refresh rejected duplicate Assembly ID: " + EffectId;
			return false;
		}
		const std::string DomainId = EffectAsset_DomainId(EffectId);
		StagedDomainIds.insert(DomainId);
		Staged.push_back({ SelectionId, DomainId, {},
			EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY });
		++iRuntimeAssemblyCount;
	}
	for (const std::string& ComponentId :
		CEffectCatalog::Get_ComponentAssetIds())
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(ComponentId);
		if (nullptr == Component)
			continue;
		if (!AssetIds.insert(ComponentId).second)
		{
			m_strDocumentStatus =
				"Data Files refresh rejected duplicate WFX Component ID: " +
				ComponentId;
			return false;
		}
		const std::string DomainId = EffectAsset_DomainId(
			Component->strSourceEffectAssetId);
		StagedDomainIds.insert(DomainId);
		Staged.push_back({ ComponentId, DomainId, {},
			EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT });
		++iRuntimeComponentCount;
	}

    size_t iImportedReferenceCount = 0u;
    const std::filesystem::path ImportedRoot =
        CProjectDataRoot::Resolve(L"Effects/Imported");
    std::error_code ReferenceError;
    if (!ImportedRoot.empty() &&
        std::filesystem::is_directory(ImportedRoot, ReferenceError))
    {
        for (std::filesystem::recursive_directory_iterator Iterator(
            ImportedRoot,
            std::filesystem::directory_options::skip_permission_denied,
            ReferenceError), End;
            Iterator != End; Iterator.increment(ReferenceError))
        {
            if (ReferenceError)
            {
                m_strDocumentStatus =
                    "Imported draft scan failed; previous list preserved.";
                return false;
            }
            if (!Iterator->is_regular_file())
                continue;
            const std::string Name = Iterator->path().filename().string();
            if (!Name.ends_with(".imported-effect-draft.json") &&
                !Name.ends_with(".unbound-effect-draft-index.json"))
                continue;
            const std::filesystem::path Relative =
                Iterator->path().lexically_relative(ImportedRoot);
            if (Relative.empty())
                continue;
            std::string DomainId = First_PathComponent(Relative);
            if (DomainId.empty())
                DomainId = "Uncategorized";
            StagedDomainIds.insert(DomainId);
            Staged.push_back({
                Relative.generic_string(), DomainId, Iterator->path(),
                EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE });
            ++iImportedReferenceCount;
        }
    }
    std::sort(Staged.begin(), Staged.end(),
        [](const EFFECT_DATA_FILE_ENTRY& Left,
            const EFFECT_DATA_FILE_ENTRY& Right)
        {
            if (Left.strDomainId != Right.strDomainId)
                return Left.strDomainId < Right.strDomainId;
            if (Left.eSource != Right.eSource)
                return Left.eSource < Right.eSource;
            return Left.strAssetId < Right.strAssetId;
        });
	const bool_t bDirectAuthoredEditableIndexReady =
		Refresh_DirectAuthoredEditableIndex(Staged);
    m_DataFiles = std::move(Staged);
	const auto SelectedDataFile = std::find_if(
		m_DataFiles.begin(), m_DataFiles.end(),
		[this](const EFFECT_DATA_FILE_ENTRY& Entry)
		{
			return Entry.strAssetId == m_strSelectedDataFileAssetId;
		});
	if (SelectedDataFile == m_DataFiles.end())
	{
		m_strSelectedDataFileAssetId.clear();
		m_strSelectedDataFileElementId.clear();
	}
	else if (!m_strSelectedDataFileElementId.empty() &&
		(nullptr == SelectedDataFile->pParsedDocument ||
		 1u != static_cast<size_t>(std::count_if(
			 SelectedDataFile->pParsedDocument->Elements.begin(),
			 SelectedDataFile->pParsedDocument->Elements.end(),
			 [this](const EFFECT_ELEMENT_DESC& Element)
			 {
				 return Element.strElementId ==
					 m_strSelectedDataFileElementId;
			 }))))
	{
		m_strSelectedDataFileElementId.clear();
	}
    m_DataFileDomains.assign(
        StagedDomainIds.begin(), StagedDomainIds.end());
    if (m_DataFileDomains.end() == std::find(
        m_DataFileDomains.begin(), m_DataFileDomains.end(),
        m_strSelectedAuthoringDomainId) && !m_DataFileDomains.empty())
    {
        Select_AuthoringDomain(m_DataFileDomains.front());
    }
    m_strDocumentStatus = "Data Files refreshed: " +
		std::to_string(m_DataFiles.size() - iImportedReferenceCount -
			iRuntimeAssemblyCount - iRuntimeComponentCount) +
		" authored/imported documents, " +
		std::to_string(iRuntimeAssemblyCount) + " runtime Assemblies, " +
		std::to_string(iRuntimeComponentCount) + " WFX Components, " +
        std::to_string(iImportedReferenceCount) +
        " reference-only extraction drafts.";
    if (0u != iRejectedDocumentCount)
    {
        m_strDocumentStatus += " Isolated " +
            std::to_string(iRejectedDocumentCount) +
            " invalid/duplicate entries; first: " +
            strFirstRejectedDocument;
    }
	m_strDocumentStatus += " " + m_strDirectAuthoredEditableStatus;
	if (!bDirectAuthoredEditableIndexReady)
	{
		m_strDocumentStatus +=
			" The previous direct-authored and saved unified index was preserved; Open remains unavailable only when no valid index was admitted yet.";
	}
	m_strDocumentStatus += " " + m_strUnifiedCandidateStatus;
    return true;
}

bool_t Client::CEffect_Tool::Ensure_DataFileDocumentParsed(
	EFFECT_DATA_FILE_ENTRY& Entry)
{
	if (nullptr != Entry.pParsedDocument)
		return true;
	if (Entry.bDocumentParseAttempted)
		return false;
	Entry.bDocumentParseAttempted = true;

	if (EFFECT_DOCUMENT_SOURCE::AUTHORED != Entry.eSource ||
		Entry.Path.empty())
	{
		Entry.strDocumentParseStatus =
			"Only a physical Authored Effect Data File exposes reusable Element rows.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged;
	std::string Error;
	if (!CEffectDocumentCodec::Load(Entry.Path, Staged, Error))
	{
		Entry.strDocumentParseStatus =
			"Data File parse failed; the cached row was preserved: " + Error;
		return false;
	}
	if (Staged.strEffectAssetId != Entry.strAssetId)
	{
		Entry.strDocumentParseStatus =
			"Data File identity mismatch; selected '" + Entry.strAssetId +
			"', file contains '" + Staged.strEffectAssetId + "'.";
		return false;
	}

	const size_t iElementCount = Staged.Elements.size();
	Entry.pParsedDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(Staged));
	Entry.strDocumentParseStatus =
		"Physical Data File loaded: " + std::to_string(iElementCount) +
		" reusable Element" + (1u == iElementCount ? "." : "s.");
	return true;
}

bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(
	const std::filesystem::path& Path,
	const std::string& strExpectedEffectAssetId,
	const std::string& strElementId)
{
	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource))
	{
		m_strDocumentStatus =
			"Create or open a New/Authored Current Effect before loading one saved Element.";
		return false;
	}
	if (Has_UnappliedDetailDraft())
	{
		m_strDocumentStatus =
			"Apply or Revert the open Detail draft before loading one saved Element.";
		return false;
	}
	if (Path.empty() || strExpectedEffectAssetId.empty() ||
		strElementId.empty())
	{
		m_strDocumentStatus =
			"Select one saved Effect Element before loading it for editing.";
		return false;
	}

	EFFECT_DOCUMENT_DESC SourceDocument;
	std::string Error;
	if (!CEffectDocumentCodec::Load(Path, SourceDocument, Error))
	{
		m_strDocumentStatus =
			"Saved Element source changed or became invalid: " + Error;
		return false;
	}
	if (SourceDocument.strEffectAssetId != strExpectedEffectAssetId)
	{
		m_strDocumentStatus =
			"Saved Element source identity changed; refresh Data Files before retrying.";
		return false;
	}
	const size_t iExactSourceCount = static_cast<size_t>(std::count_if(
		SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
		[&strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		}));
	if (1u != iExactSourceCount)
	{
		m_strDocumentStatus =
			"Saved Element source no longer contains exactly one selected stable Element ID.";
		return false;
	}
	const EFFECT_ELEMENT_DESC& SourceElement = *std::find_if(
		SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
		[&strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});

	const EFFECT_PARTICLE_SYSTEM_DESC& SourceSystem =
		SourceDocument.ParticleSystem;
	const EFFECT_PARTICLE_SYSTEM_DESC& TargetSystem =
		m_ActiveDocument->ParticleSystem;
	const bool_t bUsesParticleSystem =
		SourceElement.eKind == EFFECT_ELEMENT_KIND::PARTICLE ||
		(SourceElement.eKind == EFFECT_ELEMENT_KIND::DECAL &&
		 SourceElement.SourceRecipe.bEnabled &&
		 SourceElement.SourceRecipe.strRendererShape == "decal");
	const bool_t bTargetUsesParticleSystem = std::any_of(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE ||
				(Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
				 Element.SourceRecipe.bEnabled &&
				 Element.SourceRecipe.strRendererShape == "decal");
		});
	if (bUsesParticleSystem && bTargetUsesParticleSystem &&
		(SourceSystem.fUniformScaleMultiplier !=
			TargetSystem.fUniformScaleMultiplier ||
		SourceSystem.fYawOffsetDegrees != TargetSystem.fYawOffsetDegrees ||
		SourceSystem.fDirectionYawDegrees !=
			TargetSystem.fDirectionYawDegrees ||
		SourceSystem.fInitialSpeedMultiplier !=
			TargetSystem.fInitialSpeedMultiplier))
	{
		m_strDocumentStatus =
			"Saved Element copy rejected different Effect-level Particle System controls; align the source and Current Effect system values first.";
		return false;
	}

	EFFECT_DOCUMENT_DESC PortableCopy;
	if (!CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(
			SourceDocument, strElementId,
			m_ActiveDocument->strEffectAssetId, PortableCopy, Error) ||
		1u != PortableCopy.Elements.size())
	{
		m_strDocumentStatus = Error.empty() ?
			"The selected saved Element cannot become a portable authored copy." :
			"Saved Element copy rejected: " + Error;
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	if (bUsesParticleSystem && !bTargetUsesParticleSystem)
	{
		/* Particle System controls are Effect-level rather than Element-level.
		   The first simulation Element therefore establishes that shared state;
		   later Saved Element loads must match it and are rejected above instead
		   of silently changing already-authored occurrences. */
		Staged.ParticleSystem = SourceSystem;
	}
	const std::string Prefix = "authored.copy.";
	std::string CopyElementId;
	for (size_t iCopy = 1u; iCopy <= Staged.Elements.size() + 1u; ++iCopy)
	{
		const std::string Suffix = "." + std::to_string(iCopy);
		const size_t iMaximumSourceLength =
			128u - Prefix.size() - Suffix.size();
		CopyElementId = Prefix +
			strElementId.substr(0u, iMaximumSourceLength) + Suffix;
		if (std::none_of(Staged.Elements.begin(), Staged.Elements.end(),
			[&CopyElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == CopyElementId;
			}))
		{
			break;
		}
		CopyElementId.clear();
	}
	if (CopyElementId.empty())
	{
		m_strDocumentStatus =
			"Saved Element copy could not allocate a unique authored Element ID.";
		return false;
	}

	EFFECT_ELEMENT_DESC AppendedElement =
		std::move(PortableCopy.Elements.front());
	AppendedElement.strElementId = CopyElementId;
	Staged.Elements.push_back(std::move(AppendedElement));
	const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	const std::string strPreviousIsolationModelCue =
		m_strPreviewIsolationModelCueId;
	const EFFECT_AUTHORING_FAMILY ePreviousIsolationAuthoringFamily =
		m_ePreviewIsolationAuthoringFamily;
	/* A saved Element may belong to a different family/group than the current
	   Solo scope.  Stage the append transaction as the Complete Effect so the
	   newly selected generic copy cannot be pruned before it is committed. */
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	if (!Try_CommitDocument(std::move(Staged)))
	{
		m_ePreviewFilter = ePreviousPreviewFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
		m_strPreviewIsolationModelCueId = strPreviousIsolationModelCue;
		m_ePreviewIsolationAuthoringFamily =
			ePreviousIsolationAuthoringFamily;
		return false;
	}

	const std::string& strActiveEffectAssetId =
		m_ActiveDocument->strEffectAssetId;
	const bool_t bForeignPlayerProduct = m_ProductPreview.has_value() &&
		m_ProductPreview->ProductCue.Cue.strEffectAssetId !=
			strActiveEffectAssetId;
	const bool_t bForeignValtanProduct = m_ValtanProductPreview.has_value() &&
		m_ValtanProductPreview->Cue.strEffectAssetId !=
			strActiveEffectAssetId;
	if (bForeignPlayerProduct || bForeignValtanProduct)
		Clear_ProductCuePreview();
	else
		m_SourcePreviewDocument.reset();

	const EFFECT_ELEMENT_DESC& Committed = m_ActiveDocument->Elements.back();
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	Reset_ModelCueDraft();
	m_MarkedElementIds.clear();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
	m_strSelectedElementId = CopyElementId;
	m_strSelectedElementGroupId = Committed.strGroupId;
	m_strSelectedModelCueId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_eSelectedAuthoringFamily = Resolve_AuthoringFamily(Committed);
	m_eSelectedEffectType = Committed.eKind;
	m_strSelectedResourceSlotId = Default_SlotId(Committed.eKind);
	m_eResourceLibraryFileKind = Slot_FileKind(
		Committed, m_strSelectedResourceSlotId);
	m_strSelectedResourceAssetId.clear();
	Recalculate_PreviewDuration(*m_ActiveDocument);
	Synchronize_LoadedSkillPreview();
	if (m_bActiveDocumentDrawable)
		Start_WorldPreviewFromBeginning();
	m_strElementStatus = "Loaded saved Element '" + strElementId +
		"' as unsaved '" + CopyElementId + "' in Current Effect.";
	m_strDocumentStatus =
		"Current Effect gained one portable authored Element copy with its executable Detail/material carrier; the source Effect was not opened or changed and no file was saved.";
	if (SourceElement.ActionCueAttachment.bFollow &&
		!Committed.ActionCueAttachment.bEnabled)
	{
		m_strDocumentStatus +=
			" Bone attachment was reset; select the target bone and use Attach Selected Element.";
	}
	return true;
}
