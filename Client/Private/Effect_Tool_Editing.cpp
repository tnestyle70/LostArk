#include "Effect_Tool_Internal.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_VisualProgramCorpus.h"
#include "Logic_DimensionMaster.h"
#include "Logic_LanceMaster.h"
#include "RuntimeAssetRoot.h"
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
#include "Effect_ThumbnailCache.h"
#include "EffectAuthoringSequencer.h"

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
    if (Has_UnsavedWork())
    {
        m_strDocumentStatus =
            "Save or explicitly discard the active Effect changes first.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Document;
    Document.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
    Document.strEffectAssetId = m_NewAssetId.data();
    Document.strDisplayName = m_NewDisplayName.data();
	if (Document.strDisplayName.empty())
		Document.strDisplayName = Document.strEffectAssetId;
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Document, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    std::string DrawableError;
    const bool_t bDrawable =
        CEffectDocumentCodec::Validate_Drawable(Document, DrawableError);
    const std::filesystem::path ExistingPath = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(Document.strEffectAssetId).wstring() +
            L".effect.json"));
	if (ExistingPath.empty() || std::filesystem::is_regular_file(ExistingPath))
	{
        m_strDocumentStatus = ExistingPath.empty() ?
            "New Effect path escaped Data/Effects/Authored." :
            "New refuses an existing Effect ID; load that file or choose another ID.";
		return false;
	}
    if (m_pAuthoringSequencer && m_pAuthoringSequencer->Is_ElementPreview())
        m_pAuthoringSequencer->Stop();
    m_MarkedElementIds.clear();
	Release_WorldPreview(true);
    Clear_ProductCuePreview();
    m_ActiveDocument = std::move(Document);
    m_bMarkedElementIdsNeedPrune = true;
	m_ActiveRegistryBoundAuditionProvenance.reset();
    Set_ActiveDocumentDrawableStatus(bDrawable, std::move(DrawableError));
    m_ActiveDocumentPath.clear();
	m_strActiveDocumentBaselineCanonical.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT;
	m_strActiveValtanPatternDraftId.clear();
	m_eActiveValtanPatternDraftPreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	Reset_ModelCueDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedModelCueId.clear();
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_bDocumentDirty = true;
    m_bActiveDocumentMatchesRuntime = false;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration();
    m_strDocumentStatus =
		"Created a new Current Effect in memory. Create an Element, bind WModel/DDS slots, tune Details, then use Save Changes.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CreateMeshEffect(
	const bool_t bAddToCurrentEffect)
{
    if ((!m_bResourceCatalogRefreshAttempted &&
         !Refresh_ResourceCatalog()) ||
        (m_bResourceCatalogRefreshAttempted &&
         m_ResourceCatalogByDomain.end() ==
            m_ResourceCatalogByDomain.find(
                m_strSelectedAuthoringDomainId) &&
         !Refresh_ResourceCatalogDomain(
            m_strSelectedAuthoringDomainId, false)))
        return false;

	if (bAddToCurrentEffect &&
		(!m_ActiveDocument.has_value() ||
		 (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		  EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource)))
	{
		m_strElementStatus =
			"Add to Current Effect requires one open authored Effect.";
		return false;
	}
	const std::string strTargetEffectId = bAddToCurrentEffect ?
		m_ActiveDocument->strEffectAssetId : std::string(m_NewAssetId.data());
    if (strTargetEffectId.empty())
    {
        m_strElementStatus =
			"Enter an Effect Name before creating the authored Effect.";
        return false;
    }
	const EFFECT_AUTHORING_FAMILY eAuthoringFamily =
		m_eSelectedAuthoringFamily;
	const EFFECT_ELEMENT_KIND eElementKind =
		AuthoringFamily_Kind(eAuthoringFamily);
	if (!AuthoringFamily_CanCreate(eAuthoringFamily) ||
		EFFECT_ELEMENT_KIND::END == eElementKind)
	{
		m_strElementStatus =
			"Select one drawable authoring family. Presentation Light and Screen Post creation requires source-backed materialization.";
		return false;
	}

	const bool_t bUsesActiveDocument = bAddToCurrentEffect;
    if (!bUsesActiveDocument && Has_UnsavedWork())
    {
        m_strElementStatus =
            "Save or discard the different active Effect before creating this named Data File.";
        return false;
    }
    if (m_ActiveDocument.has_value() &&
        m_ActiveDocument->strEffectAssetId == strTargetEffectId &&
        !bUsesActiveDocument)
    {
        m_strElementStatus =
            "Imported/runtime Effects are read-only; enter a unique Effect Name for the authored Data File.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged;
    if (bUsesActiveDocument)
    {
        Staged = *m_ActiveDocument;
        if (m_bParticleSystemDraftDirty)
            Apply_ParticleSystemDraft(Staged);
        if (m_bDetailDraftDirty && !Apply_DetailDraft(Staged))
        {
            m_strElementStatus =
				"The open Detail draft no longer matches its Element; the authoring transaction preserved all data.";
            return false;
        }
    }
    else
    {
        Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
        Staged.strEffectAssetId = strTargetEffectId;
    }
    const std::string strRequestedDisplayName = m_NewDisplayName.data();
	if (!bUsesActiveDocument && !strRequestedDisplayName.empty())
        Staged.strDisplayName = strRequestedDisplayName;
    else if (Staged.strDisplayName.empty())
        Staged.strDisplayName = strTargetEffectId;

    EFFECT_ELEMENT_DESC Element = m_MeshAuthoringDraft;
	Element.eKind = eElementKind;
	Element.Renderer = {};
	if (Element.Material.strTemplateId.empty())
	{
		Element.Material.strTemplateId =
			std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
	}
	Element.strGroupId = "manual.hit1";
	Element.strSourceNode.clear();
    Element.SourceRecipe = {};
    Element.SourcePresentation = {};
    Element.ActionCueAttachment = {};
	Element.TransformInheritance = {};
	Element.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
    Element.strElementId = m_NewElementId.data();
    if (Element.strElementId.empty())
    {
		const std::string Prefix = std::string(
			AuthoringFamily_ElementPrefix(eAuthoringFamily)) + "_";
        size_t iCandidate = Staged.Elements.size() + 1u;
        do
        {
            Element.strElementId = Prefix + std::to_string(iCandidate++);
        }
        while (std::any_of(Staged.Elements.begin(),
            Staged.Elements.end(),
            [&Element](const EFFECT_ELEMENT_DESC& Existing)
            {
                return Existing.strElementId == Element.strElementId;
            }));
    }
    Element.strDisplayName = Element.strElementId;

    if (std::any_of(Staged.Elements.begin(),
        Staged.Elements.end(),
        [&Element](const EFFECT_ELEMENT_DESC& Existing)
        {
            return Existing.strElementId == Element.strElementId;
        }))
    {
        m_strElementStatus = "Create Effect rejected a duplicate Element ID.";
        return false;
    }

    const EFFECT_RESOURCE_BINDING_DESC* pMesh = Find_Binding(
        Element, EFFECT_MESH_SHAPE_SLOT_ID);
    const EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
        Element, EFFECT_STANDARD_MATERIAL_INPUTS[0u].strSlotId);
    const bool_t bUnsafeBase = nullptr != pBase &&
        Is_UnsafeEffectBaseTextureAssetId(pBase->strAssetId);
	const bool_t bRequiresMesh = AuthoringFamily_RequiresMesh(
		eAuthoringFamily);
	if (nullptr == pBase || bUnsafeBase || (bRequiresMesh && nullptr == pMesh))
    {
        m_strElementStatus =
			bRequiresMesh ?
			"Mesh and Mesh Particle require one WModel Mesh and one safe 2D Base texture." :
			"Sprite, Sprite Particle, Local Decal, and Trail / Ribbon require one safe 2D Base texture.";
        return false;
    }

    std::set<std::string> Slots;
    for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
        Element.ResourceBindings)
    {
		if (!Slots.insert(Binding.strSlotId).second ||
			!Slot_Allowed(Element, Binding.strSlotId) ||
			!AuthoringFamily_AllowsSlot(eAuthoringFamily, Binding.strSlotId))
        {
            m_strElementStatus =
                "Create Effect rejected a duplicate or unsupported resource slot.";
            return false;
        }
        const EFFECT_RESOURCE_FILE_KIND eExpected =
            Slot_FileKind(Element, Binding.strSlotId);
        const auto CatalogEntry = std::find_if(
            m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
            [this, &Binding, eExpected](
                const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
            {
                return Entry.strAssetId == Binding.strAssetId &&
                    Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                    Entry.eFileKind == eExpected;
            });
        if (CatalogEntry == m_ResourceCatalog.end())
        {
            m_strElementStatus =
                "Create Effect preserved the draft: a resource left the active domain or file kind.";
            return false;
        }
    }

    Staged.Elements.push_back(Element);

    std::string Error;
    if (!CEffectDocumentCodec::Validate(Staged, Error))
    {
        m_strElementStatus = Error;
        return false;
    }
    std::string DrawableError;
    if (!CEffectDocumentCodec::Validate_Drawable(Staged, DrawableError))
    {
        m_strElementStatus =
			"The authoring transaction rejected a non-drawable Element: " +
            DrawableError;
        return false;
    }

    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strTargetEffectId).wstring() +
            L".effect.json"));
    if (Path.empty())
    {
        m_strElementStatus =
            "Effect Name escaped Data/Effects/Authored.";
        return false;
    }
    if (!bUsesActiveDocument && std::filesystem::is_regular_file(Path))
    {
        m_strElementStatus =
            "That Effect Name already exists in Data Files; load it before adding another layer.";
        return false;
    }
    if (bUsesActiveDocument &&
        EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource &&
        std::filesystem::is_regular_file(Path))
    {
        m_strElementStatus =
            "The new Effect Name appeared on disk; Create Effect preserved it and refused overwrite.";
        return false;
    }

	const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
		m_ProductPreview;
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
		m_ValtanProductPreview;
	const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
		m_SourcePreviewDocument;
	const bool_t bPreviousArtistAdapterPreviewActive =
		m_bReconstructedSourceRuntimeActive;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
    const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds = m_fPreviewDurationSeconds;
	const uint32_t iPreviousValtanWorldOwnerStageDurationMs =
		m_iValtanWorldOwnerStageDurationMs;
	const uint32_t iPreviousValtanReferenceEffectStartMs =
		m_iValtanReferenceEffectStartMs;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	const f32_t fPreviousProductCueActionFacingYawDegrees =
		m_fProductCueActionFacingYawDegrees;
	const bool_t bPreviousProductCueActionFacingCaptured =
		m_bProductCueActionFacingCaptured;
	std::string ValtanRestoreError;
	const auto RestorePreviousSourcePreviewState = [this,
		&PreviousProductPreview, &PreviousValtanProductPreview,
		&PreviousSourcePreviewDocument,
		&strPreviousIsolationElement, &strPreviousIsolationGroup,
		bPreviousArtistAdapterPreviewActive,
		ePreviousPreviewFilter, fPreviousPreviewTimeSeconds,
		fPreviousPreviewDurationSeconds,
		iPreviousValtanWorldOwnerStageDurationMs, bPreviousPreviewPlaying,
		iPreviousValtanReferenceEffectStartMs,
		bPreviousPreviewVisibleRequested, &PreviousProductCueSnapshotRoot,
		bPreviousProductCueSnapshotCaptured,
		fPreviousProductCueActionFacingYawDegrees,
		bPreviousProductCueActionFacingCaptured, &ValtanRestoreError]()
	{
		m_ProductPreview = PreviousProductPreview;
		m_ValtanProductPreview = PreviousValtanProductPreview;
		m_SourcePreviewDocument = PreviousSourcePreviewDocument;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
		m_ePreviewFilter = ePreviousPreviewFilter;
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
		m_iValtanWorldOwnerStageDurationMs =
			iPreviousValtanWorldOwnerStageDurationMs;
		m_iValtanReferenceEffectStartMs =
			iPreviousValtanReferenceEffectStartMs;
		Reset_ProductCueSnapshot();
		if (!bPreviousArtistAdapterPreviewActive)
		{
			if (m_ValtanProductPreview.has_value())
				return Restore_ValtanProductPreviewPlayback(
					m_ValtanProductPreview,
					fPreviousPreviewTimeSeconds,
					fPreviousPreviewDurationSeconds,
					bPreviousPreviewPlaying,
					bPreviousPreviewVisibleRequested,
					PreviousProductCueSnapshotRoot,
					bPreviousProductCueSnapshotCaptured,
					ValtanRestoreError);
			else
			{
				m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
				m_bProductCueSnapshotCaptured =
					bPreviousProductCueSnapshotCaptured;
				m_fProductCueActionFacingYawDegrees =
					fPreviousProductCueActionFacingYawDegrees;
				m_bProductCueActionFacingCaptured =
					bPreviousProductCueActionFacingCaptured;
				Synchronize_LoadedSkillPreview();
			}
		}
		return true;
	};
	const auto RestorePreviousSourceIsolation = [this,
		&strPreviousIsolationElement, &strPreviousIsolationGroup]()
	{
		if (!strPreviousIsolationElement.empty())
		{
			Try_SetVisualPreviewOccurrenceIsolation(
				strPreviousIsolationElement);
			return;
		}
		if (strPreviousIsolationGroup.empty())
			return;
		const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection = nullptr == pObject ? nullptr :
				pObject->Get_SourceVisualProgramProjection();
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
			nullptr == pProjection ? nullptr :
				CEffectCatalog::Find_VisualProgram(
					pProjection->Get_EffectAssetId());
		if (nullptr == Program)
			return;
		for (int32_t iFamily = 0;
			iFamily < static_cast<int32_t>(EFFECT_VISUAL_PROGRAM_FAMILY::END);
			++iFamily)
		{
			const EFFECT_VISUAL_PROGRAM_FAMILY eFamily =
				static_cast<EFFECT_VISUAL_PROGRAM_FAMILY>(iFamily);
			if (strPreviousIsolationGroup ==
				VisualProgramFamilyLabel(eFamily))
			{
				Try_SetVisualPreviewFamilyIsolation(*Program, eFamily);
				return;
			}
		}
	};
	const auto RestorePreviousPlaybackState = [this,
		bPreviousArtistAdapterPreviewActive, fPreviousPreviewTimeSeconds,
		bPreviousPreviewPlaying, bPreviousPreviewVisibleRequested]()
	{
		const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
		if (nullptr == pObject)
			return;
		if (bPreviousArtistAdapterPreviewActive)
		{
			Seek_ReconstructedSourceRuntimeTimeline(
				fPreviousPreviewTimeSeconds);
		}
		else
		{
			m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
			pObject->Set_SampleTime(
				Resolve_EffectSampleTime(fPreviousPreviewTimeSeconds));
		}
		m_bPreviewPlaying = bPreviousPreviewPlaying;
		m_bPreviewVisibleRequested = bPreviousPreviewVisibleRequested;
		pObject->Set_Playing(bPreviousPreviewPlaying);
		pObject->Set_Visible(bPreviousPreviewVisibleRequested &&
			Is_ProductCueVisible(fPreviousPreviewTimeSeconds));
		Set_SynchronizedAnimationPaused(!bPreviousPreviewPlaying);
	};
	Clear_ProductCuePreview();
	m_iValtanWorldOwnerStageDurationMs =
		iPreviousValtanWorldOwnerStageDurationMs;
	m_iValtanReferenceEffectStartMs =
		iPreviousValtanReferenceEffectStartMs;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    if (!Stage_WorldPreview(Staged))
    {
		const std::string StageError = m_strPreviewStatus;
		const bool_t bRestored = RestorePreviousSourcePreviewState();
        m_strElementStatus =
			"Element preview failed; the active Document, Data File, and builder were preserved: " +
			StageError + (bRestored ? std::string{} :
				" Previous exact Valtan preview rollback failed: " +
				ValtanRestoreError);
        return false;
    }
    const std::string_view strExpectedCanonical =
        bUsesActiveDocument &&
        EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ?
            std::string_view(m_strActiveDocumentBaselineCanonical) :
            std::string_view{};
    if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
        Path, Staged, strExpectedCanonical, Error))
    {
		const bool_t bRestored = RestorePreviousSourcePreviewState();
		if (bRestored && PreviousValtanProductPreview.has_value())
		{
			/* The exact helper already re-staged the Valtan document, sample,
			   root, visibility, and held animation pose. */
		}
		else if (bRestored && bPreviousArtistAdapterPreviewActive)
		{
			Try_StartArtist31470FullPreview();
			RestorePreviousSourceIsolation();
			RestorePreviousPlaybackState();
		}
		else if (bRestored && m_ProductPreview.has_value() &&
			m_SourcePreviewDocument.has_value())
		{
			Stage_WorldPreview(*m_SourcePreviewDocument, true);
			RestorePreviousSourceIsolation();
			RestorePreviousPlaybackState();
		}
		else if (bRestored && m_ActiveDocument.has_value())
        {
            if (m_bActiveDocumentDrawable)
			{
				Stage_WorldPreview(*m_ActiveDocument);
				RestorePreviousPlaybackState();
			}
            else
                Release_WorldPreview(true);
        }
		else if (bRestored)
            Release_WorldPreview(true);
        m_strElementStatus =
			bRestored ?
				"Element save failed; previous Document and preview were restored: " +
					Error :
				"Element save failed and the previous exact Valtan preview could not be restored: " +
					Error + " Rollback failed: " + ValtanRestoreError;
        return false;
    }

    m_ActiveDocument = std::move(Staged);
    m_bMarkedElementIdsNeedPrune = true;
    Set_ActiveDocumentDrawableStatus(true, {});
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	if (!bUsesActiveDocument)
	{
		m_eActiveDocumentPreviewIntent =
			EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT;
	}
    m_strActiveDocumentBaselineCanonical =
        CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
    m_bActiveDocumentMatchesRuntime = false;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	Reset_ModelCueDraft();
    const std::string strCreatedElementId = Element.strElementId;
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = strCreatedElementId;
    m_strSelectedElementGroupId = Element.strGroupId;
    m_strSelectedComponentId.clear();
    m_strSelectedEmitterId.clear();
    m_strSelectedSourceModuleId.clear();
	m_eSelectedEffectType = Element.eKind;
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    m_strSelectedResourceAssetId.clear();
    m_strSelectedDataFileAssetId = strTargetEffectId;
	m_strSelectedDataFileElementId.clear();
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_NewElementId[0u] = '\0';
    Recalculate_PreviewDuration();
	Synchronize_LoadedSkillPreview();
    Start_WorldPreviewFromBeginning();
    Refresh_RuntimeEquivalence();
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strElementStatus =
		std::string(bUsesActiveDocument ? "Added one " : "Created one ") +
		AuthoringFamily_Label(eAuthoringFamily) +
		" Element and saved the Effect atomically; builder selections were preserved.";
    m_strDocumentStatus = "Saved Authored atomically: " + Path.string() +
        " Live preview is active; add an exact EffectCatalog direct row and gameplay cue before Product use.";
    return true;
}

bool_t Client::CEffect_Tool::Try_UseSelectedElementAsAuthoringPreset()
{
	const EFFECT_ELEMENT_DESC* pSelected = Find_SelectedElement();
	if (nullptr == pSelected || !m_ActiveDocument.has_value())
	{
		m_strElementStatus = "Select one drawable Element before using a preset.";
		return false;
	}
	SOURCE_ELEMENT_PRESET_SELECTION Selection;
	Selection.strSourceEffectAssetId = m_ActiveDocument->strEffectAssetId;
	Selection.strTargetElementId = pSelected->strElementId;
	Selection.strSourceRecordId = pSelected->strSourceNode.empty() ?
		pSelected->strElementId : pSelected->strSourceNode;
	Selection.strSourceFamily =
		AuthoringFamily_Label(Resolve_AuthoringFamily(*pSelected));
	return Try_StageElementAsAuthoringPreset(
		*m_ActiveDocument, pSelected->strElementId, std::move(Selection));
}

bool_t Client::CEffect_Tool::Try_StageElementAsAuthoringPreset(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::string& strElementId,
	SOURCE_ELEMENT_PRESET_SELECTION Selection)
{
	EFFECT_DOCUMENT_DESC GenericCopy;
	std::string Error;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
			SourceDocument, strElementId, "effect.authoring.preset",
			GenericCopy, Error) ||
		GenericCopy.Elements.size() != 1u)
	{
		m_strElementStatus = Error.empty() ?
			"The selected Element could not be loaded into the builder." : Error;
		return false;
	}
	EFFECT_ELEMENT_DESC Preset = GenericCopy.Elements.front();
	const EFFECT_AUTHORING_FAMILY eFamily = Resolve_AuthoringFamily(Preset);
	if (!AuthoringFamily_CanCreate(eFamily))
	{
		m_strElementStatus =
			"Presentation Light and Screen Post are edited or deleted in the active Effect; creating them from a drawable Element preset is not admitted.";
		return false;
	}
	Preset.strGroupId = "manual.hit1";
	Preset.strElementId.clear();
	Preset.strDisplayName = AuthoringFamily_Label(eFamily);
	Selection.GenericElement = Preset;
	if (Selection.strSourceFamily.empty())
		Selection.strSourceFamily = AuthoringFamily_Label(eFamily);

	/* Commit only after the complete immutable source selection and generic
	   builder copy have validated.  The current Effect document, Detail draft,
	   save baseline, and preview are intentionally not part of this transaction. */
	m_MeshAuthoringDraft = std::move(Preset);
	m_SourceElementPresetSelection = std::move(Selection);
	m_eSelectedAuthoringFamily = eFamily;
	m_eSelectedEffectType = AuthoringFamily_Kind(eFamily);
	m_bMeshAuthoringDraftInitialized = true;
	const bool_t bRequiresMesh = AuthoringFamily_RequiresMesh(eFamily);
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	m_strSelectedResourceAssetId.clear();
	m_iResourceViewRevision = UINT64_MAX;
	m_strElementStatus = std::string("Loaded one ") +
		AuthoringFamily_Label(eFamily) +
		" Source Element seed into Element Authoring. Current Effect was not changed; use Create Element, then Save Changes.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindMeshAuthoringResource(
    const std::string& strAssetId)
{
    if (!m_bMeshAuthoringDraftInitialized)
        Reset_MeshAuthoringDraft();
    if (!Slot_Allowed(m_MeshAuthoringDraft,
		m_strSelectedResourceSlotId) ||
		!AuthoringFamily_AllowsSlot(
			m_eSelectedAuthoringFamily, m_strSelectedResourceSlotId))
    {
        m_strResourceStatus =
            "Select Mesh, Base, Noise, Mask, Emissive, or Dissolve first.";
        return false;
    }
    const EFFECT_RESOURCE_FILE_KIND eExpectedKind = Slot_FileKind(
        m_MeshAuthoringDraft, m_strSelectedResourceSlotId);
    const auto CatalogEntry = std::find_if(
        m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
        [this, &strAssetId, eExpectedKind](
            const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
        {
            return Entry.strAssetId == strAssetId &&
                Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                Entry.eFileKind == eExpectedKind;
        });
    if (CatalogEntry == m_ResourceCatalog.end() && !Is_AuthoringWorldResource(strAssetId, eExpectedKind))
    {
        m_strResourceStatus =
            "The selected resource is outside the active domain or file kind.";
        return false;
    }
    const bool_t bBaseSlot = m_strSelectedResourceSlotId ==
        EFFECT_STANDARD_MATERIAL_INPUTS[0u].strSlotId;
    const bool_t bNoiseSlot = m_strSelectedResourceSlotId ==
        EFFECT_STANDARD_MATERIAL_INPUTS[1u].strSlotId;
    if ((bBaseSlot || bNoiseSlot) &&
        Is_UnsafeEffectBaseTextureAssetId(strAssetId))
    {
        m_strResourceStatus = bBaseSlot ?
            "Base rejects blank/normal/bump textures." :
            "Noise rejects normal data without source Material distortion evidence.";
        return false;
    }

    auto Binding = std::find_if(
        m_MeshAuthoringDraft.ResourceBindings.begin(),
        m_MeshAuthoringDraft.ResourceBindings.end(),
        [this](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
        {
            return Candidate.strSlotId == m_strSelectedResourceSlotId;
        });
    if (Binding == m_MeshAuthoringDraft.ResourceBindings.end())
    {
        m_MeshAuthoringDraft.ResourceBindings.push_back(
            { m_strSelectedResourceSlotId, strAssetId });
    }
    else
    {
        Binding->strAssetId = strAssetId;
    }
    m_strResourceStatus = "Selected " + strAssetId + " for " +
        Slot_Label(m_MeshAuthoringDraft,
            m_strSelectedResourceSlotId) + ".";
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearMeshAuthoringSlot()
{
    if (!m_bMeshAuthoringDraftInitialized ||
        !Slot_Allowed(m_MeshAuthoringDraft,
			m_strSelectedResourceSlotId) ||
		!AuthoringFamily_AllowsSlot(
			m_eSelectedAuthoringFamily, m_strSelectedResourceSlotId))
    {
        return false;
    }
    std::erase_if(m_MeshAuthoringDraft.ResourceBindings,
        [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == m_strSelectedResourceSlotId;
        });
    m_strSelectedResourceAssetId.clear();
	m_strResourceStatus = "Cleared the selected Element Authoring slot.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CreateElementDraft()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before creating another Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
         EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource))
    {
        m_strElementStatus =
            "Create or open one editable Current Effect before creating an Element.";
        return false;
    }
    if (m_bOccurrenceTuningDirty)
    {
        m_strElementStatus =
            "Save the occurrence tuning artifact before editing Current Effect.";
        return false;
    }

    const EFFECT_AUTHORING_FAMILY eFamily = m_eSelectedAuthoringFamily;
	const bool_t bImportedSeed = m_SourceElementPresetSelection.has_value();
	if (bImportedSeed &&
		m_SourceElementPresetSelection->strSourceEffectAssetId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		m_ActiveDocument->strEffectAssetId !=
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID)
	{
		m_strElementStatus =
			"Artist F Track A Seed belongs to the Artist F Editable Skill Effect. Open that parent before Create Element.";
		return false;
	}
    const EFFECT_ELEMENT_KIND eKind = AuthoringFamily_Kind(eFamily);
    if (!AuthoringFamily_CanCreate(eFamily) ||
        EFFECT_ELEMENT_KIND::END == eKind)
    {
		m_strElementStatus =
			"Select one drawable Element Type. Presentation Light and Screen Post creation requires source-backed materialization.";
        return false;
    }
    if (EFFECT_AUTHORING_FAMILY::MESH_PARTICLE == eFamily &&
        nullptr == Find_Binding(
            m_MeshAuthoringDraft, EFFECT_MESH_SHAPE_SLOT_ID))
    {
        m_strElementStatus =
            "Choose a WModel seed before creating Mesh Particle so its Family remains stable.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    EFFECT_ELEMENT_DESC Element = m_MeshAuthoringDraft;
    Element.eKind = eKind;
    Element.Renderer = {};
    Element.strGroupId = "manual.hit1";
    Element.strSourceNode.clear();
    Element.SourceRecipe = {};
    Element.SourcePresentation = {};
    if (!bImportedSeed)
    {
        Element.ActionCueAttachment = {};
        Element.TransformInheritance = {};
        Element.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
        Element.Material.strTemplateId =
            std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
		Element.Material.eRenderProfile =
			EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
        Element.Material.strSourceMaterialPath.clear();
        Element.Material.SourceMaterial = {};
		Element.Material.Execution = {};
        Element.Detail.Mesh.bUseModelMaterial = false;
    }

	if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
		!bImportedSeed &&
		Element.Detail.Particle.fSpawnRatePerSecond <= 0.f &&
		0u == Element.Detail.Particle.iBurstCount &&
		Element.Detail.Particle.fFixedCenterSpacingWorldUnits <= 0.f)
    {
        Element.Detail.Particle.iMaxParticles = 1u;
        Element.Detail.Particle.iBurstCount = 1u;
        Element.Detail.Particle.vLifeTimeSeconds = { 1.f, 1.f };
        if (Element.Detail.Particle.vStartSize.x <= 0.f ||
            Element.Detail.Particle.vStartSize.y <= 0.f)
        {
            Element.Detail.Particle.vStartSize = { 1.f, 1.f };
        }
    }
	float4x4_t TrailAnchorWorld{};
	const bool_t bCreateTrailFollow =
		EFFECT_ELEMENT_KIND::TRAIL == Element.eKind && !bImportedSeed &&
		!m_strPreviewAnchorSlotId.empty() &&
		(EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET == m_ePreviewPivotKind ||
		 EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE == m_ePreviewPivotKind) &&
		CAnimationTargetService::Resolve_AnchorTransform(
			m_strPreviewAnchorSlotId.c_str(), &TrailAnchorWorld);
	if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind && !bImportedSeed &&
		!bCreateTrailFollow &&
		std::abs(Element.Detail.Transform.vVelocityPerSecond.x) <= 1e-6f &&
		std::abs(Element.Detail.Transform.vVelocityPerSecond.y) <= 1e-6f &&
		std::abs(Element.Detail.Transform.vVelocityPerSecond.z) <= 1e-6f)
	{
		Element.Detail.Transform.vVelocityPerSecond = { 1.f, 0.f, 0.f };
	}

    Element.strElementId = m_NewElementId.data();
    if (Element.strElementId.empty())
    {
        const std::string Prefix = std::string(
            AuthoringFamily_ElementPrefix(eFamily)) + "_";
        size_t iCandidate = Staged.Elements.size() + 1u;
        do
        {
            Element.strElementId = Prefix + std::to_string(iCandidate++);
        }
        while (std::any_of(Staged.Elements.begin(), Staged.Elements.end(),
            [&Element](const EFFECT_ELEMENT_DESC& Existing)
            {
                return Existing.strElementId == Element.strElementId;
            }));
    }
    Element.strDisplayName = Element.strElementId;
	if (bCreateTrailFollow)
	{
		Element.ActionCueAttachment.bEnabled = true;
		Element.ActionCueAttachment.bFollow = true;
		Element.ActionCueAttachment.strSourceAnchorSlotId =
			m_strPreviewAnchorSlotId;
		Element.ActionCueAttachment.strRuntimeAnchorSlotId =
			Element.strElementId;
		Element.ActionCueAttachment.strRuntimeBoneName =
			m_strPreviewAnchorSlotId;
	}
    if (std::any_of(Staged.Elements.begin(), Staged.Elements.end(),
        [&Element](const EFFECT_ELEMENT_DESC& Existing)
        {
            return Existing.strElementId == Element.strElementId;
        }))
    {
        m_strElementStatus =
            "Create Element rejected a duplicate Element ID.";
        return false;
    }

    std::set<std::string> Slots;
    for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
        Element.ResourceBindings)
    {
        if (!Slots.insert(Binding.strSlotId).second ||
            !Slot_Allowed(Element, Binding.strSlotId) ||
            !AuthoringFamily_AllowsSlot(eFamily, Binding.strSlotId))
        {
            m_strElementStatus =
                "Create Element rejected a duplicate or unsupported resource slot.";
            return false;
        }
    }

    Staged.Elements.push_back(Element);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;

    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    Reset_MeshAuthoringDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = Element.strElementId;
    m_strSelectedElementGroupId = Element.strGroupId;
    m_strSelectedComponentId.clear();
    m_strSelectedEmitterId.clear();
    m_strSelectedSourceModuleId.clear();
    const bool_t bHasMesh = nullptr != Find_Binding(
        Element, EFFECT_MESH_SHAPE_SLOT_ID);
    m_strSelectedResourceSlotId =
        AuthoringFamily_RequiresMesh(eFamily) && !bHasMesh ?
            std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
            std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    m_eResourceLibraryFileKind = Slot_FileKind(
        Element, m_strSelectedResourceSlotId);
    m_strSelectedResourceAssetId.clear();
    m_NewElementId[0u] = '\0';
    if (m_bActiveDocumentDrawable)
        Start_WorldPreviewFromBeginning();
    m_strElementStatus = "Created one unsaved " +
        std::string(AuthoringFamily_Label(eFamily)) +
        " Element in Current Effect. Bind WModel/DDS slots and tune Details, then use Save Changes.";
	if (bImportedSeed && !m_bActiveDocumentDrawable)
	{
		m_strElementStatus +=
			" The Track A data was normalized to the standard renderer and needs a generic slot binding before preview: " +
			m_strActiveDocumentDrawableError;
	}
    m_strDocumentStatus =
        "Current Effect has unsaved Element changes; no Data File was written.";
    return true;
}

bool_t Client::CEffect_Tool::Try_DuplicateSelectedElement()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strElementStatus =
			"Apply or Revert the open Detail draft before duplicating Elements.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource) ||
		(m_MarkedElementIds.empty() && m_strSelectedElementId.empty()))
	{
		m_strElementStatus =
			"Open an authored Effect and select or mark Elements to duplicate.";
		return false;
	}

	const std::vector<std::string> Targets = m_MarkedElementIds.empty() ?
		std::vector<std::string>{ m_strSelectedElementId } :
		std::vector<std::string>(m_MarkedElementIds.begin(), m_MarkedElementIds.end());
	EFFECT_DOCUMENT_DESC Staged;
	std::unordered_map<std::string, std::string> DuplicateIds;
	std::string Error;
	if (!CEffectDocumentCodec::Build_DuplicatedAuthoredElements(
			*m_ActiveDocument, Targets, Staged, DuplicateIds, Error))
	{
		m_strElementStatus = Error;
		return false;
	}

	const auto OpenCopy = DuplicateIds.find(m_strSelectedElementId);
	const std::string DuplicateId = OpenCopy == DuplicateIds.end() ?
		DuplicateIds.at(Targets.front()) : OpenCopy->second;
	const auto SelectedDuplicate = std::find_if(
		Staged.Elements.begin(), Staged.Elements.end(),
		[&DuplicateId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == DuplicateId;
		});
	const std::string SelectedGroupId = SelectedDuplicate->strGroupId;
	const EFFECT_ELEMENT_KIND eSelectedKind = SelectedDuplicate->eKind;
	const std::string SelectedSlotId = Default_SlotId(eSelectedKind);
	const EFFECT_RESOURCE_FILE_KIND eSelectedFileKind =
		Slot_FileKind(*SelectedDuplicate, SelectedSlotId);
	std::set<std::string, std::less<>> CopiedMarks;
	for (const auto& [SourceId, CopyId] : DuplicateIds)
		CopiedMarks.insert(CopyId);

	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter)
	{
		m_strPreviewIsolationElementId = DuplicateId;
	}
	if (!Try_CommitDocument(std::move(Staged)))
	{
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		return false;
	}

	Reset_DetailDraft();
	m_MarkedElementIds = std::move(CopiedMarks);
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
	m_strSelectedElementId = DuplicateId;
	m_strSelectedElementGroupId = SelectedGroupId;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_eSelectedEffectType = eSelectedKind;
	m_strSelectedResourceSlotId = SelectedSlotId;
	m_eResourceLibraryFileKind = eSelectedFileKind;
	m_strSelectedResourceAssetId.clear();
	if (m_bActiveDocumentDrawable && m_bPreviewVisibleRequested)
		Start_WorldPreviewFromBeginning();
	m_strElementStatus = "Duplicated " + std::to_string(DuplicateIds.size()) +
		" Element(s); timing, material, resources, attachment, and source recipe were preserved. Copies remain marked.";
	m_strDocumentStatus = "Current Effect has " +
		std::to_string(DuplicateIds.size()) +
		" unsaved duplicated Element(s); no Data File was written.";
	return true;
}

bool_t Client::CEffect_Tool::Try_DeleteSelectedElement()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before deleting an Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        (m_MarkedElementIds.empty() && m_strSelectedElementId.empty()))
    {
        m_strElementStatus = "Select one Element to delete.";
        return false;
    }
    /* Marked rows are the delete set when there are any; otherwise the single
       open Element stays the target so every existing caller behaves the same. */
    const std::set<std::string, std::less<>> Targets =
        m_MarkedElementIds.empty() ?
            std::set<std::string, std::less<>>{ m_strSelectedElementId } :
            m_MarkedElementIds;
    if (Targets.size() >= m_ActiveDocument->Elements.size())
    {
        m_strElementStatus =
            "Deleting every Element would leave no drawable Effect; keep at least one.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    const auto NewEnd = std::remove_if(
        Staged.Elements.begin(), Staged.Elements.end(),
        [&Targets](const EFFECT_ELEMENT_DESC& Element)
        {
            return Targets.contains(Element.strElementId);
        });
    if (NewEnd == Staged.Elements.end())
        return false;
    const size_t iRemovedCount = static_cast<size_t>(
        std::distance(NewEnd, Staged.Elements.end()));
    Staged.Elements.erase(NewEnd, Staged.Elements.end());
    if (Targets.contains(m_strSelectedElementId))
        m_strSelectedElementId.clear();
    m_MarkedElementIds.clear();
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	if (!m_strPreviewIsolationElementId.empty() &&
		std::none_of(Staged.Elements.begin(), Staged.Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId ==
					m_strPreviewIsolationElementId;
			}))
	{
		m_strPreviewIsolationElementId.clear();
		if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
			EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter)
		{
			m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		}
	}
	if (!m_strPreviewIsolationGroupId.empty() &&
		std::none_of(Staged.Elements.begin(), Staged.Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strGroupId ==
					m_strPreviewIsolationGroupId;
			}))
	{
		m_strPreviewIsolationGroupId.clear();
		if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
			EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter)
		{
			m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		}
	}
    if (!Try_CommitDocument(std::move(Staged)))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
        return false;
	}
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strElementStatus = 1u == iRemovedCount ?
        "Deleted the selected Element." :
        "Deleted " + std::to_string(iRemovedCount) + " marked Elements.";
    return true;
}

bool_t Client::CEffect_Tool::Try_MoveSelectedElement(
	const int32_t iDirection)
{
	if (-1 != iDirection && 1 != iDirection)
	{
		m_strElementStatus =
			"Element order move requires exactly Up or Down.";
		return false;
	}
	if (Has_UnappliedDetailDraft())
	{
		m_strElementStatus =
			"Apply or Revert the open Detail draft before moving an Element.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource) ||
		EFFECT_DETAIL_SELECTION::ELEMENT != m_eDetailSelection ||
		m_strSelectedElementId.empty() || !m_MarkedElementIds.empty())
	{
		m_strElementStatus =
			"Open an authored Effect and select exactly one Element to move.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	const auto Selected = std::find_if(
		Staged.Elements.begin(), Staged.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	if (Selected == Staged.Elements.end())
	{
		m_strElementStatus =
			"The selected stable Element no longer exists in Current Effect.";
		return false;
	}
	const EFFECT_AUTHORING_FAMILY eSelectedFamily =
		Resolve_AuthoringFamily(*Selected);
	auto SwapWith = Staged.Elements.end();
	if (iDirection < 0)
	{
		for (auto Candidate = Selected; Candidate != Staged.Elements.begin();)
		{
			--Candidate;
			if (Resolve_AuthoringFamily(*Candidate) == eSelectedFamily)
			{
				SwapWith = Candidate;
				break;
			}
		}
	}
	else
	{
		for (auto Candidate = std::next(Selected);
			Candidate != Staged.Elements.end(); ++Candidate)
		{
			if (Resolve_AuthoringFamily(*Candidate) == eSelectedFamily)
			{
				SwapWith = Candidate;
				break;
			}
		}
	}
	if (SwapWith == Staged.Elements.end())
	{
		m_strElementStatus = iDirection < 0 ?
			"The selected Element is already first in its Family order." :
			"The selected Element is already last in its Family order.";
		return false;
	}

	std::iter_swap(Selected, SwapWith);
	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (m_bActiveDocumentDrawable && m_bPreviewVisibleRequested)
		Start_WorldPreviewFromBeginning();
	m_strElementStatus = "Moved '" + m_strSelectedElementId +
		(iDirection < 0 ? "' Up" : "' Down") +
		" within its Family order. Stable IDs and Element payloads were preserved.";
	m_strDocumentStatus =
		"Current Effect has an unsaved Element order change; no Data File was written.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearElements()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before clearing Elements.";
        return false;
    }
    if (!m_ActiveDocument.has_value())
        return false;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.Elements.clear();
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter)
	{
		m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	}
    if (!Try_CommitDocument(std::move(Staged)))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
        return false;
	}
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strElementStatus = "Cleared all visual layers.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectProductCue(
    const EFFECT_SKILL_TREE_ENTRY& Entry,
    const size_t iCueIndex)
{
    if (iCueIndex >= Entry.ProductCues.size())
    {
        m_strElementStatus = "The selected BA stage has no playable Effect.";
        return false;
    }
    const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
        Entry.ProductCues[iCueIndex];
    if (ProductCue.Cue.strEffectAssetId.empty() ||
        !CEffectCatalog::Contains(ProductCue.Cue.strEffectAssetId))
    {
        m_strElementStatus =
            "Play Full Effect rejected an Effect that is no longer available.";
        return false;
    }
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProductVisualProjection = CEffectCatalog::Find_VisualProjection(
			ProductCue.Cue.strEffectAssetId);
	const shared_ptr<const EFFECT_DOCUMENT_DESC> pRuntimeDocument =
		nullptr == pProductVisualProjection ?
			CEffectCatalog::Find(ProductCue.Cue.strEffectAssetId) : nullptr;
	const EFFECT_DOCUMENT_DESC* pSourceDocument =
		nullptr != pProductVisualProjection ?
			&pProductVisualProjection->Get_Document() : pRuntimeDocument.get();
	if (nullptr == pSourceDocument)
	{
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
			CEffectCatalog::Find_VisualProgram(
				ProductCue.Cue.strEffectAssetId);
		if (ProductCue.Cue.strEffectAssetId ==
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
			nullptr != Program && Program->eProjectionKind ==
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
				m_ProductPreview;
			const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
				m_ValtanProductPreview;
			const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
				m_SourcePreviewDocument;
			const std::string strPreviousIsolationElement =
				m_strPreviewIsolationElementId;
			const std::string strPreviousIsolationGroup =
				m_strPreviewIsolationGroupId;
			const f32_t fPreviousPreviewTimeSeconds =
				m_fPreviewTimeSeconds;
			const f32_t fPreviousPreviewDurationSeconds =
				m_fPreviewDurationSeconds;
			const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
			const bool_t bPreviousPreviewVisibleRequested =
				m_bPreviewVisibleRequested;
			const float4x4_t PreviousProductCueSnapshotRoot =
				m_ProductCueSnapshotRoot;
			const bool_t bPreviousProductCueSnapshotCaptured =
				m_bProductCueSnapshotCaptured;
			const f32_t fPreviousProductCueActionFacingYawDegrees =
				m_fProductCueActionFacingYawDegrees;
			const bool_t bPreviousProductCueActionFacingCaptured =
				m_bProductCueActionFacingCaptured;
			const bool_t bPreviousBufferedComboAuditionActive =
				m_bBufferedComboAuditionActive;
			const LostArk::Shared::CHARACTER_CLASS_ID
				ePreviousBufferedComboAuditionClass =
					m_eBufferedComboAuditionClass;
			const LostArk::Shared::SKILL_ID
				iPreviousBufferedComboAuditionSkillId =
					m_iBufferedComboAuditionSkillId;
			const f32_t fPreviousBufferedComboAuditionDurationSeconds =
				m_fBufferedComboAuditionDurationSeconds;
			Clear_ProductCuePreview();
			if (!Try_StartArtist31470FullPreview())
			{
				const std::string ArtistStartError = m_strPreviewStatus;
				m_ProductPreview = PreviousProductPreview;
				m_ValtanProductPreview = PreviousValtanProductPreview;
				m_SourcePreviewDocument = PreviousSourcePreviewDocument;
				m_strPreviewIsolationElementId =
					strPreviousIsolationElement;
				m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
				Reset_ProductCueSnapshot();
				std::string ValtanRestoreError;
				bool_t bRestored = true;
				if (m_ValtanProductPreview.has_value())
					bRestored = Restore_ValtanProductPreviewPlayback(
						m_ValtanProductPreview,
						fPreviousPreviewTimeSeconds,
						fPreviousPreviewDurationSeconds,
						bPreviousPreviewPlaying,
						bPreviousPreviewVisibleRequested,
						PreviousProductCueSnapshotRoot,
						bPreviousProductCueSnapshotCaptured,
						ValtanRestoreError);
				else
				{
					m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
					m_bProductCueSnapshotCaptured =
						bPreviousProductCueSnapshotCaptured;
					m_fProductCueActionFacingYawDegrees =
						fPreviousProductCueActionFacingYawDegrees;
					m_bProductCueActionFacingCaptured =
						bPreviousProductCueActionFacingCaptured;
					m_bBufferedComboAuditionActive =
						bPreviousBufferedComboAuditionActive;
					m_eBufferedComboAuditionClass =
						ePreviousBufferedComboAuditionClass;
					m_iBufferedComboAuditionSkillId =
						iPreviousBufferedComboAuditionSkillId;
					m_fBufferedComboAuditionDurationSeconds =
						fPreviousBufferedComboAuditionDurationSeconds;
					Synchronize_LoadedSkillPreview();
				}
				m_strElementStatus = bRestored ?
					"Artist F full preview failed; the previous preview was restored: " +
						ArtistStartError :
					"Artist F full preview failed and the previous exact Valtan preview rollback failed: " +
						ArtistStartError + " Rollback failed: " +
						ValtanRestoreError;
				return false;
			}
			m_eAllEffectsClass = Entry.Skill.eCharacterClass;
			m_strElementStatus = "Playing full Effect | " +
				Entry.Skill.strDisplayName + " | " +
				ProductCue.Cue.strClipName;
			return true;
		}
		m_strElementStatus =
			"This Effect needs its Track A adapter prepared before playback.";
		return false;
	}
	const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
		m_ProductPreview;
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
		m_ValtanProductPreview;
	const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
		m_SourcePreviewDocument;
	const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds = m_fPreviewDurationSeconds;
	const bool_t bPreviousScreenPostEnabled = m_bPreviewScreenPostEnabled;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	const f32_t fPreviousProductCueActionFacingYawDegrees =
		m_fProductCueActionFacingYawDegrees;
	const bool_t bPreviousProductCueActionFacingCaptured =
		m_bProductCueActionFacingCaptured;
	const bool_t bPreviousBufferedComboAuditionActive =
		m_bBufferedComboAuditionActive;
	const LostArk::Shared::CHARACTER_CLASS_ID
		ePreviousBufferedComboAuditionClass =
			m_eBufferedComboAuditionClass;
	const LostArk::Shared::SKILL_ID iPreviousBufferedComboAuditionSkillId =
		m_iBufferedComboAuditionSkillId;
	const f32_t fPreviousBufferedComboAuditionDurationSeconds =
		m_fBufferedComboAuditionDurationSeconds;
	Reset_BufferedComboAudition();
    EFFECT_PRODUCT_PREVIEW Preview;
    Preview.eCharacterClass = Entry.Skill.eCharacterClass;
    Preview.iSkillId = Entry.Skill.iSkillId;
    Preview.ProductCue = ProductCue;
	m_ValtanProductPreview.reset();
    m_ProductPreview = std::move(Preview);
	m_SourcePreviewDocument = *pSourceDocument;
    Reset_ProductCueSnapshot();
    m_eAllEffectsClass = Entry.Skill.eCharacterClass;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_bPreviewScreenPostEnabled = true;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
    m_fPreviewTimeSeconds = 0.f;
	Recalculate_PreviewDuration(*m_SourcePreviewDocument);
    Synchronize_LoadedSkillPreview();
	if (!Stage_WorldPreview(*m_SourcePreviewDocument, true))
	{
		const std::string StageError = m_strPreviewStatus;
		m_ProductPreview = PreviousProductPreview;
		m_ValtanProductPreview = PreviousValtanProductPreview;
		m_SourcePreviewDocument = PreviousSourcePreviewDocument;
		m_ePreviewFilter = ePreviousPreviewFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
		m_bPreviewScreenPostEnabled = bPreviousScreenPostEnabled;
		m_bBufferedComboAuditionActive =
			bPreviousBufferedComboAuditionActive;
		m_eBufferedComboAuditionClass =
			ePreviousBufferedComboAuditionClass;
		m_iBufferedComboAuditionSkillId =
			iPreviousBufferedComboAuditionSkillId;
		m_fBufferedComboAuditionDurationSeconds =
			fPreviousBufferedComboAuditionDurationSeconds;
		Reset_ProductCueSnapshot();
		std::string ValtanRestoreError;
		bool_t bRestored = true;
		if (m_ValtanProductPreview.has_value())
			bRestored = Restore_ValtanProductPreviewPlayback(
				m_ValtanProductPreview,
				fPreviousPreviewTimeSeconds,
				fPreviousPreviewDurationSeconds,
				bPreviousPreviewPlaying,
				bPreviousPreviewVisibleRequested,
				PreviousProductCueSnapshotRoot,
				bPreviousProductCueSnapshotCaptured,
				ValtanRestoreError);
		else
		{
			m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
			m_bProductCueSnapshotCaptured =
				bPreviousProductCueSnapshotCaptured;
			m_fProductCueActionFacingYawDegrees =
				fPreviousProductCueActionFacingYawDegrees;
			m_bProductCueActionFacingCaptured =
				bPreviousProductCueActionFacingCaptured;
			Synchronize_LoadedSkillPreview();
		}
        m_strElementStatus =
			"Play Full Effect could not stage this source preview: " +
			StageError + (bRestored ? std::string{} :
				" Previous exact Valtan preview rollback failed: " +
				ValtanRestoreError);
        return false;
    }
    Start_WorldPreviewFromBeginning();
	m_strElementStatus = "Playing full Effect | " +
		Entry.Skill.strDisplayName + " | " + ProductCue.Cue.strClipName;
	if (nullptr == pProductVisualProjection)
	{
		m_strElementStatus += Describe_ProductPlaybackAuthoredDivergence(
			ProductCue.Cue.strEffectAssetId);
	}
    return true;
}

bool_t Client::CEffect_Tool::Try_PlayVisualProgramFamily(
	const EFFECT_SKILL_TREE_ENTRY& Entry,
	const size_t iCueIndex,
	const EFFECT_VISUAL_PROGRAM_FAMILY eFamily)
{
	if (iCueIndex >= Entry.ProductCues.size())
		return false;
	const std::string& strEffectAssetId =
		Entry.ProductCues[iCueIndex].Cue.strEffectAssetId;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == Program)
	{
		m_strPreviewStatus = "This Effect has no Track A Family program.";
		return false;
	}
	if (!Try_SelectProductCue(Entry, iCueIndex))
	{
		return false;
	}
	return Try_SetVisualPreviewFamilyIsolation(*Program, eFamily);
}

bool_t Client::CEffect_Tool::Try_PlayVisualProgramElement(
	const EFFECT_SKILL_TREE_ENTRY& Entry,
	const size_t iCueIndex,
	const std::string& strTargetElementId)
{
	if (iCueIndex >= Entry.ProductCues.size())
		return false;
	const std::string& strEffectAssetId =
		Entry.ProductCues[iCueIndex].Cue.strEffectAssetId;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == Program)
		return false;
	if (!Try_SelectProductCue(Entry, iCueIndex))
	{
		return false;
	}
	return Try_SetVisualPreviewOccurrenceIsolation(strTargetElementId);
}

bool_t Client::CEffect_Tool::Try_SelectParticleSystem(
    const std::string& strEffectAssetId)
{
    const bool_t bChangesSelection =
        !m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
        EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM != m_eDetailSelection;
    if (bChangesSelection &&
		(Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty))
    {
        m_strElementStatus =
            "Save, Apply, or Revert the open Detail/tuning work before selecting the Particle System.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId)
    {
        if (!Try_LoadDocument(strEffectAssetId))
            return false;
    }
    const bool_t bHasParticles = std::any_of(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [](const EFFECT_ELEMENT_DESC& Element)
        {
            return EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
        });
    if (!bHasParticles)
    {
        m_strElementStatus =
            "The selected Effect has no Particle layers to group.";
        return false;
    }

    Reset_DetailDraft();
    Reset_ParticleSystemDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM;
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strSelectedResourceAssetId.clear();
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
    if (!Stage_WorldPreview())
        return false;
    m_strElementStatus =
        "Selected the complete Particle System; child layers remain available below.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectElement(
    const std::string& strEffectAssetId,
    const std::string& strElementId)
{
    const bool_t bChangesSelection =
        !m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
        EFFECT_DETAIL_SELECTION::ELEMENT != m_eDetailSelection ||
        (!strElementId.empty() &&
            m_strSelectedElementId != strElementId);
    if (bChangesSelection &&
		(Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty))
    {
        m_strElementStatus =
            "Save, Apply, or Revert the open Detail/tuning work before selecting another Element.";
        return false;
    }
	if (!bChangesSelection)
		return true;
    if (!m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId)
    {
        if (!Try_LoadDocument(strEffectAssetId))
            return false;
    }
    if (strElementId.empty())
        return true;
    const auto Iterator = std::find_if(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [&strElementId](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == strElementId;
        });
    if (Iterator == m_ActiveDocument->Elements.end())
    {
        m_strElementStatus = "Selected Element ID is no longer present.";
        return false;
    }
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = strElementId;
    m_strSelectedElementGroupId = Iterator->strGroupId;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_eSelectedEffectType = Iterator->eKind;
    m_strSelectedResourceSlotId = Default_SlotId(Iterator->eKind);
    m_eResourceLibraryFileKind = Slot_FileKind(
        *Iterator, m_strSelectedResourceSlotId);
    m_strSelectedResourceAssetId.clear();
    return true;
}

bool_t Client::CEffect_Tool::Try_SoloElement(
	const std::string& strEffectAssetId,
	const std::string& strElementId)
{
	if (strEffectAssetId.empty() || strElementId.empty())
		return false;
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const auto Element = std::find_if(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[&strElementId](const EFFECT_ELEMENT_DESC& Element)
		{ return Element.strElementId == strElementId; });
	if (Element == m_ActiveDocument->Elements.end())
	{
		m_strPreviewStatus =
			"Element Solo rejected a missing stable Element ID.";
		return false;
	}
	if (!Is_ElementPreviewAdmitted(*Element))
	{
		m_strPreviewStatus =
			!Is_EffectElementAuthoringExecutionTarget(*Element) ?
			"Element Solo is locked by material/runtime fail-closed admission; editing and Save remain available." :
			"Element Solo is unavailable while the authored Element is hidden.";
		return false;
	}
	if (m_pAuthoringSequencer && !m_ProductPreview &&
		(m_ActiveDocument->strEffectAssetId.ends_with(".restore") ||
		 Is_SceneAnchoredEffectAssetId(m_ActiveDocument->strEffectAssetId)))
		return Try_PreviewElementTimeline(strElementId);
	const std::string strPreviousElement = m_strPreviewIsolationElementId;
	const std::string strPreviousGroup = m_strPreviewIsolationGroupId;
	m_strPreviewIsolationElementId = strElementId;
	m_strPreviewIsolationGroupId.clear();
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_SELECTED))
	{
		m_strPreviewIsolationElementId = strPreviousElement;
		m_strPreviewIsolationGroupId = strPreviousGroup;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectModelCue(
	const std::string& strEffectAssetId,
	const std::string& strCueId)
{
	const bool_t bChangesSelection = !m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
		EFFECT_DETAIL_SELECTION::MODEL_CUE != m_eDetailSelection ||
		m_strSelectedModelCueId != strCueId;
	if (!bChangesSelection)
		return true;
	if (Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save, Apply, or Revert the open Detail before selecting another Model Cue.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const auto Iterator = std::find_if(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(),
		[&strCueId](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == strCueId; });
	if (Iterator == m_ActiveDocument->ModelCues.end())
	{
		m_strElementStatus = "Selected Model Cue ID is no longer present.";
		return false;
	}
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	Reset_ModelCueDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::MODEL_CUE;
	m_strSelectedModelCueId = strCueId;
	m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_strSelectedResourceAssetId.clear();
	return true;
}

bool_t Client::CEffect_Tool::Try_SoloModelCue(
	const std::string& strEffectAssetId,
	const std::string& strCueId)
{
	if (strEffectAssetId.empty() || strCueId.empty())
		return false;
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	if (std::none_of(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(),
		[&strCueId](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == strCueId; }))
	{
		m_strPreviewStatus =
			"Model Cue Solo rejected a missing stable Cue ID.";
		return false;
	}
	const std::string strPreviousCue = m_strPreviewIsolationModelCueId;
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	m_strPreviewIsolationModelCueId = strCueId;
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE))
	{
		m_strPreviewIsolationModelCueId = strPreviousCue;
		m_ePreviewFilter = ePreviousFilter;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_SoloElementGroup(
	const std::string& strEffectAssetId,
	const std::string& strGroupId)
{
	if (strEffectAssetId.empty() || strGroupId.empty())
		return false;
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const bool_t bGroupExists = std::any_of(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[&strGroupId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strGroupId == strGroupId &&
				Is_ElementPreviewAdmitted(Element);
		});
	if (!bGroupExists)
	{
		m_strPreviewStatus =
			"Group Solo has no visible authoring-admitted Element to play; hard-locked Elements remain editable and APPROXIMATE Elements remain authoring-preview targets.";
		return false;
	}
	const std::string strPreviousElement = m_strPreviewIsolationElementId;
	const std::string strPreviousGroup = m_strPreviewIsolationGroupId;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId = strGroupId;
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP))
	{
		m_strPreviewIsolationElementId = strPreviousElement;
		m_strPreviewIsolationGroupId = strPreviousGroup;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectComponent(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId)
{
	if (Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save, Apply, or Revert the open Detail/tuning work before selecting a Component.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(strComponentAssetId);
	if (nullptr == Component ||
		(Component->strSourceEffectAssetId != strEffectAssetId &&
			Component->strComponentAssetId != strEffectAssetId))
	{
		m_strElementStatus = "Selected Component is not admitted by this Assembly.";
		return false;
	}
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::COMPONENT;
	m_strSelectedComponentId = strComponentAssetId;
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus = "Selected stable Effect Component.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectEmitter(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId,
	const std::string& strEmitterId)
{
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(strComponentAssetId);
	if (nullptr == Component ||
		(Component->strSourceEffectAssetId != strEffectAssetId &&
			Component->strComponentAssetId != strEffectAssetId))
	{
		m_strElementStatus = "Selected Emitter has no admitted Component.";
		return false;
	}
	const auto Emitter = std::find_if(Component->Emitters.begin(),
		Component->Emitters.end(),
		[&strEmitterId](const EFFECT_COMPONENT_EMITTER_DESC& Value)
		{
			return Value.strEmitterId == strEmitterId;
		});
	const std::string strActiveDocumentId = m_ActiveDocument.has_value() ?
		m_ActiveDocument->strEffectAssetId : strEffectAssetId;
	if (Component->Emitters.end() == Emitter ||
		!Try_SelectElement(strActiveDocumentId, Emitter->strElementId))
	{
		m_strElementStatus = "Selected Emitter identity is no longer present.";
		return false;
	}
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::EMITTER;
	m_strSelectedComponentId = strComponentAssetId;
	m_strSelectedEmitterId = strEmitterId;
	m_strSelectedSourceModuleId.clear();
	if (const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
		nullptr != pElement && !pElement->ResourceBindings.empty())
	{
		m_strSelectedResourceSlotId =
			pElement->ResourceBindings.front().strSlotId;
		m_eResourceLibraryFileKind =
			Resource_FileKind(pElement->ResourceBindings.front());
	}
	m_strElementStatus = "Selected source Emitter; Renderer, Resources, and Module Stack are active.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectSourceModule(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId,
	const std::string& strEmitterId,
	const std::string& strModuleStableId)
{
	if (!Try_SelectEmitter(strEffectAssetId, strComponentAssetId, strEmitterId))
		return false;
	const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
	if (nullptr == pElement || std::none_of(
		pElement->SourceRecipe.Modules.begin(),
		pElement->SourceRecipe.Modules.end(),
		[&strModuleStableId](const EFFECT_SOURCE_MODULE_DESC& Module)
		{
			return Module.strStableId == strModuleStableId;
		}))
	{
		m_strElementStatus = "Selected source Module identity is no longer present.";
		return false;
	}
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::SOURCE_MODULE;
	m_strSelectedSourceModuleId = strModuleStableId;
	m_strElementStatus = "Selected source Module; Effect Detail follows its source class.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectFirstEmitter(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId)
{
	if (!strComponentAssetId.empty())
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(strComponentAssetId);
		if (nullptr != Component && !Component->Emitters.empty())
		{
			return Try_SelectEmitter(strEffectAssetId,
				Component->strComponentAssetId,
				Component->Emitters.front().strEmitterId);
		}
		m_strElementStatus =
			"The selected Component has no admitted Emitter.";
		return false;
	}

	const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
		CEffectCatalog::Find_Assembly(strEffectAssetId);
	if (nullptr != Assembly)
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
		{
			const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
				CEffectCatalog::Find_Component(Cue.strComponentAssetId);
			if (nullptr == Component || Component->Emitters.empty())
				continue;
			return Try_SelectEmitter(strEffectAssetId,
				Component->strComponentAssetId,
				Component->Emitters.front().strEmitterId);
		}
	}
	m_strElementStatus = "The selected Skill has no admitted Emitter.";
	return false;
}

bool_t Client::CEffect_Tool::Try_AuditionParticleSystem()
{
    if (!m_ActiveDocument.has_value())
    {
        m_strPreviewStatus =
            "Load an Effect before starting a Particle System audition.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (m_bParticleSystemDraftDirty &&
        !Apply_ParticleSystemDraft(Staged))
    {
        m_strPreviewStatus =
            "Particle System audition rejected: the draft is missing.";
        return false;
    }
    const bool_t bHasParticles = std::any_of(
        Staged.Elements.begin(), Staged.Elements.end(),
        [](const EFFECT_ELEMENT_DESC& Element)
        {
            return EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
        });
    if (!bHasParticles)
    {
        m_strPreviewStatus =
            "Particle System audition rejected: no Particle layers exist.";
        return false;
    }

    const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const bool_t bPreviousPlaying = m_bPreviewPlaying;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration(Build_PreviewDocument(Staged));
    if (!Stage_WorldPreview(Staged))
    {
        m_ePreviewFilter = ePreviousFilter;
        m_fPreviewTimeSeconds = fPreviousTime;
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_bPreviewPlaying = bPreviousPlaying;
        return false;
    }

    if (const shared_ptr<CEffectObject> pObject =
        m_pWorldPreviewObject.lock())
    {
        Reset_ProductCueSnapshot();
		Restart_SynchronizedAnimationSequence();
        pObject->Reset();
		const f32_t fEffectSampleSeconds =
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
		std::string TransformError;
		bool_t bTransformApplied = true;
		if (m_bValtanBossPatternTransformHistoryRequired)
		{
			bTransformApplied = Seek_ValtanBossPatternTransformHistory(
				pObject, fEffectSampleSeconds, TransformError);
		}
		else
		{
			pObject->Set_SampleTime(fEffectSampleSeconds);
		}
		if (!bTransformApplied)
		{
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			m_bPreviewPlaying = false;
			m_strPreviewStatus =
				"Particle audition refused missing Valtan exact history: " +
				TransformError;
			return false;
		}
		m_bPreviewVisibleRequested = true;
        m_bPreviewPlaying = true;
    }
    else
        m_bPreviewPlaying = false;
    m_strPreviewStatus =
        "Auditioning all Particle layers without Model Cues, Decals, or other kinds.";
    m_strDetailStatus = m_bParticleSystemDraftDirty ?
        "Audition uses the live Particle System draft; Apply then Save to persist." :
        "Auditioning the committed Particle System.";
    return true;
}

bool_t Client::CEffect_Tool::Try_AuditionSelectedElement()
{
    if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty())
    {
        m_strPreviewStatus =
            "Select an Element before starting an audition preview.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (m_bDetailDraftDirty && !Apply_DetailDraft(Staged))
    {
        m_strPreviewStatus =
            "Element audition rejected: the Detail draft target is missing.";
        return false;
    }
    const auto Selected = std::find_if(
        Staged.Elements.begin(), Staged.Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    if (Selected == Staged.Elements.end())
    {
        m_strPreviewStatus =
            "Element audition rejected: the selected Element is missing.";
        return false;
    }
	if (!Is_ElementPreviewAdmitted(*Selected))
	{
		m_strPreviewStatus =
			!Is_EffectElementAuthoringExecutionTarget(*Selected) ?
			"Element audition is locked by material/runtime fail-closed admission; editing and Save remain available." :
			"Element audition is unavailable while the authored Element is hidden.";
		return false;
	}

    const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const bool_t bPreviousPlaying = m_bPreviewPlaying;
	EFFECT_DOCUMENT_DESC AuditionDocument = Staged;
	std::erase_if(AuditionDocument.Elements,
		[this](const EFFECT_ELEMENT_DESC& Element)
		{ return Element.strElementId != m_strSelectedElementId; });
	AuditionDocument.ModelCues.clear();
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = Resolve_EffectTimelineTime(
		Selected->Detail.Timing.fStartDelaySeconds);
	Recalculate_PreviewDuration(AuditionDocument);
	if (!Stage_WorldPreview(AuditionDocument))
    {
        m_ePreviewFilter = ePreviousFilter;
        m_fPreviewTimeSeconds = fPreviousTime;
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_bPreviewPlaying = bPreviousPlaying;
        return false;
    }
	m_ePreviewFilter = ePreviousFilter;
	m_bDetailDraftPreviewRestartRequested = false;

    if (const shared_ptr<CEffectObject> pObject =
        m_pWorldPreviewObject.lock())
    {
        Reset_ProductCueSnapshot();
		/* Seek uses the requested play state when deciding whether to hold the
		   model. Commit it only after the audition document staged successfully. */
		m_bPreviewPlaying = true;
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
        float4x4_t Root{};
        const bool_t bRootResolved = Resolve_PreviewRoot(Root);
        if (bRootResolved)
            pObject->Set_RootWorld(Root);
        pObject->Reset();
		const f32_t fEffectSampleSeconds =
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
		std::string TransformError;
		bool_t bTransformApplied = true;
		if (m_bValtanBossPatternTransformHistoryRequired)
		{
			bTransformApplied = Seek_ValtanBossPatternTransformHistory(
				pObject, fEffectSampleSeconds, TransformError);
		}
		else
		{
			pObject->Set_SampleTime(fEffectSampleSeconds);
		}
		if (!bTransformApplied)
		{
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			m_bPreviewPlaying = false;
			m_strPreviewStatus =
				"Element audition refused missing Valtan exact history: " +
				TransformError;
			return false;
		}
		pObject->Set_Visible(
			bRootResolved &&
			Is_ProductCueVisible(m_fPreviewTimeSeconds));
		m_bPreviewVisibleRequested = true;
    }
    else
    {
        m_bPreviewPlaying = false;
    }
    m_strPreviewStatus = "Auditioning selected Element from " +
        std::to_string(m_fPreviewTimeSeconds) + " s.";
    m_strDetailStatus = m_bDetailDraftDirty ?
        "Audition uses the live Detail draft; Apply Detail then Save to persist." :
        "Auditioning the committed Detail; Save is only required after Apply.";
    return true;
}

bool_t Client::CEffect_Tool::Refresh_ResourceCatalog()
{
    m_bResourceCatalogRefreshAttempted = true;
    const std::filesystem::path Root = CRuntimeAssetRoot::Get();
    const std::filesystem::path EffectRoot = Root / L"Effect";
    std::error_code Error;
    if (!std::filesystem::is_directory(EffectRoot, Error) || Error)
    {
        m_strResourceStatus = "Resources/Effect is unavailable at '" +
            EffectRoot.generic_string() + "'.";
        return false;
    }

    vector<EFFECT_RESOURCE_DOMAIN_CATALOG> StagedDomains;
    for (std::filesystem::directory_iterator Iterator(
        EffectRoot,
        std::filesystem::directory_options::skip_permission_denied,
        Error), End; Iterator != End; Iterator.increment(Error))
    {
        if (Error)
        {
            m_strResourceStatus =
                "Unable to enumerate Resources/Effect categories: " +
                Error.message();
            return false;
        }
        std::error_code EntryError;
        if (!Iterator->is_directory(EntryError) || EntryError)
            continue;
        const std::string DomainId = Iterator->path().filename().string();
        if (DomainId.empty())
            continue;
        const auto Cached = m_ResourceDomainCatalogById.find(DomainId);
        if (Cached != m_ResourceDomainCatalogById.end())
            StagedDomains.push_back(Cached->second);
        else
        {
            EFFECT_RESOURCE_DOMAIN_CATALOG Domain;
            Domain.strDomainId = DomainId;
            StagedDomains.push_back(std::move(Domain));
        }
    }
    if (Error)
    {
        m_strResourceStatus =
            "Unable to enumerate Resources/Effect categories: " +
            Error.message();
        return false;
    }
    std::sort(StagedDomains.begin(), StagedDomains.end(),
        [](const EFFECT_RESOURCE_DOMAIN_CATALOG& Left,
            const EFFECT_RESOURCE_DOMAIN_CATALOG& Right)
        {
            return Left.strDomainId < Right.strDomainId;
        });
    StagedDomains.erase(std::unique(
        StagedDomains.begin(), StagedDomains.end(),
        [](const EFFECT_RESOURCE_DOMAIN_CATALOG& Left,
            const EFFECT_RESOURCE_DOMAIN_CATALOG& Right)
        {
            return Left.strDomainId == Right.strDomainId;
        }), StagedDomains.end());
    if (StagedDomains.empty())
    {
        m_strResourceStatus =
            "Resources/Effect contains no authoring category folders.";
        return false;
    }

    m_ResourceDomains = std::move(StagedDomains);
    const auto SelectedDomain = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [this](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
        {
            return Domain.strDomainId == m_strSelectedAuthoringDomainId;
        });
    if (SelectedDomain == m_ResourceDomains.end())
    {
        const char* pPreferredDomain = Resource_DomainId(m_eAllEffectsClass);
        const auto Preferred = nullptr == pPreferredDomain ?
            m_ResourceDomains.end() : std::find_if(
                m_ResourceDomains.begin(), m_ResourceDomains.end(),
                [pPreferredDomain](
                    const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
                {
                    return Domain.strDomainId == pPreferredDomain;
                });
        m_strSelectedAuthoringDomainId =
            (Preferred == m_ResourceDomains.end() ?
                m_ResourceDomains.front() : *Preferred).strDomainId;
        Copy_Buffer(m_ResourceCategory.data(),
            m_ResourceCategory.size(), "All");
        m_strSelectedResourceAssetId.clear();
    }

    return Refresh_ResourceCatalogDomain(
        m_strSelectedAuthoringDomainId, true);
}

bool_t Client::CEffect_Tool::Refresh_ResourceCatalogDomain(
    const std::string& strDomainId,
    const bool_t bForceRefresh)
{
    if (strDomainId.empty())
    {
        m_strResourceStatus =
            "Select one Resources/Effect category before loading files.";
        return false;
    }
    const auto DomainIterator = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [&strDomainId](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
        {
            return Domain.strDomainId == strDomainId;
        });
    if (DomainIterator == m_ResourceDomains.end())
    {
        m_strResourceStatus = "Resources/Effect/" + strDomainId +
            " is not an available authoring category.";
        return false;
    }
    if (!bForceRefresh &&
        m_ResourceCatalogByDomain.end() !=
            m_ResourceCatalogByDomain.find(strDomainId))
    {
        return Activate_ResourceCatalogDomain(strDomainId);
    }

    const std::filesystem::path Root = CRuntimeAssetRoot::Get();
    const std::filesystem::path DomainRoot =
        Root / L"Effect" / std::filesystem::path(strDomainId);
    std::error_code Error;
    if (!std::filesystem::is_directory(DomainRoot, Error) || Error)
    {
        m_strResourceStatus = "Resources/Effect/" + strDomainId +
            " is unavailable at '" + DomainRoot.generic_string() + "'.";
        return false;
    }

    vector<EFFECT_RESOURCE_CATALOG_ENTRY> Staged;
    for (std::filesystem::recursive_directory_iterator Iterator(
        DomainRoot,
        std::filesystem::directory_options::skip_permission_denied,
        Error), End; Iterator != End; Iterator.increment(Error))
    {
        if (Error)
        {
            m_strResourceStatus = "Unable to scan Resources/Effect/" +
                strDomainId + ": " + Error.message();
            return false;
        }
        std::error_code EntryError;
        if (!Iterator->is_regular_file(EntryError) || EntryError)
            continue;
        std::string Extension = Iterator->path().extension().string();
        std::transform(Extension.begin(), Extension.end(), Extension.begin(),
            [](const char Character)
            {
                return static_cast<char>(std::tolower(
                    static_cast<unsigned char>(Character)));
            });
        EFFECT_RESOURCE_FILE_KIND eKind = EFFECT_RESOURCE_FILE_KIND::END;
        if (".dds" == Extension)
            eKind = EFFECT_RESOURCE_FILE_KIND::TEXTURE;
        else if (".wmodel" == Extension)
            eKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
        else
            continue;
        const std::filesystem::path Relative =
            Iterator->path().lexically_relative(Root);
        const std::filesystem::path DomainRelative =
            Iterator->path().lexically_relative(DomainRoot);
        if (Relative.empty() || DomainRelative.empty())
            continue;
        const std::filesystem::path CategoryPath =
            DomainRelative.parent_path();
        const string Category = CategoryPath.empty() ?
            "Root" : CategoryPath.generic_string();
        Staged.push_back({ Relative.generic_string(), strDomainId,
            Category, eKind });
    }
    if (Error)
    {
        m_strResourceStatus = "Unable to scan Resources/Effect/" +
            strDomainId + ": " + Error.message();
        return false;
    }
    std::sort(Staged.begin(), Staged.end(),
        [](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
            const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
        {
            return Left.strAssetId < Right.strAssetId;
        });
    Staged.erase(std::unique(Staged.begin(), Staged.end(),
        [](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
            const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
        {
            return Left.strAssetId == Right.strAssetId;
        }), Staged.end());

    array<std::set<string>,
        static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)>
        StagedCategorySets;
    for (std::set<string>& Categories : StagedCategorySets)
        Categories.insert("All");
    EFFECT_RESOURCE_DOMAIN_CATALOG StagedDomain;
    StagedDomain.strDomainId = strDomainId;
    for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry : Staged)
    {
        const size_t iKind = static_cast<size_t>(Entry.eFileKind);
        StagedCategorySets[iKind].insert(Entry.strCategory);
        ++StagedDomain.ResourceCounts[iKind];
    }
    for (size_t iKind = 0u; iKind < StagedCategorySets.size(); ++iKind)
    {
        StagedDomain.Categories[iKind].assign(
            StagedCategorySets[iKind].begin(),
            StagedCategorySets[iKind].end());
    }

    m_ResourceCatalogByDomain[strDomainId] = std::move(Staged);
    m_ResourceDomainCatalogById[strDomainId] = StagedDomain;
    *DomainIterator = std::move(StagedDomain);
    if (!Activate_ResourceCatalogDomain(strDomainId))
        return false;
    m_strResourceStatus = "Loaded Resources/Effect/" + strDomainId +
        ": " + std::to_string(m_ResourceCatalog.size()) +
        " supported DDS/WModel files. Other categories remain unloaded.";
    return true;
}

bool_t Client::CEffect_Tool::Activate_ResourceCatalogDomain(
    const std::string& strDomainId)
{
    const auto Cached = m_ResourceCatalogByDomain.find(strDomainId);
    if (Cached == m_ResourceCatalogByDomain.end())
    {
        m_strResourceStatus = "Resources/Effect/" + strDomainId +
            " has not been loaded.";
        return false;
    }
    m_ResourceCatalog = Cached->second;
    ++m_iResourceCatalogRevision;
    if (0u == m_iResourceCatalogRevision)
        m_iResourceCatalogRevision = 1u;
    m_iResourceViewRevision = UINT64_MAX;
    m_VisibleResourceIndices.clear();
    m_pThumbnailCache->Invalidate(m_iResourceCatalogRevision);
    return true;
}

void Client::CEffect_Tool::Select_AuthoringDomain(
    const std::string& strDomainId)
{
    if (strDomainId.empty())
        return;
    const bool_t bSelectionChanged =
        m_strSelectedAuthoringDomainId != strDomainId;
    if (bSelectionChanged)
    {
        m_strSelectedAuthoringDomainId = strDomainId;
        Copy_Buffer(m_ResourceCategory.data(),
            m_ResourceCategory.size(), "All");
        m_strSelectedResourceAssetId.clear();
        m_iResourceViewRevision = UINT64_MAX;
    }
    if (!m_bResourceCatalogRefreshAttempted)
    {
        m_strResourceStatus = "Resource browser category selected: " +
            strDomainId + ". Files load when Resource Library opens.";
        return;
    }
    if (!bSelectionChanged &&
        m_ResourceCatalogByDomain.end() !=
            m_ResourceCatalogByDomain.find(strDomainId))
    {
        return;
    }
    if (!Refresh_ResourceCatalogDomain(strDomainId, false))
        return;
    m_strResourceStatus +=
        " The Element draft and its bound slots were preserved.";
}

bool_t Client::CEffect_Tool::Select_AuthoringDomainForClass(
    const LostArk::Shared::CHARACTER_CLASS_ID eClass)
{
    const char* pDomainId = Resource_DomainId(eClass);
    if (nullptr == pDomainId)
        return false;
    if (!m_bResourceCatalogRefreshAttempted)
    {
        Select_AuthoringDomain(pDomainId);
        return true;
    }
    const auto Domain = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [pDomainId](const EFFECT_RESOURCE_DOMAIN_CATALOG& Candidate)
        {
            return Candidate.strDomainId == pDomainId;
        });
    if (Domain == m_ResourceDomains.end())
    {
        m_strResourceStatus = std::string("Resources/Effect/") + pDomainId +
            " is not available; the previous authoring category was preserved.";
        return false;
    }
    Select_AuthoringDomain(Domain->strDomainId);
    return true;
}

bool_t Client::CEffect_Tool::Try_ResetAuthoringResourceOverride(
	const std::string& strSlotId)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before resetting resources.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
		return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pElement = nullptr;
	for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
	{
		if (Element.strElementId == m_strSelectedElementId)
		{
			pElement = &Element;
			break;
		}
	}
	if (nullptr == pElement)
		return false;
	std::string strError;
	if (!CEffectDocumentCodec::Reset_AuthoringResourceOverride(
			*pElement, strSlotId, strError))
	{
		m_strResourceStatus = "Reset to Source rejected: " + strError;
		return false;
	}
	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	m_strResourceStatus = "Reset " + strSlotId + " to the source value.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearAuthoringOverrides()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before resetting overrides.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
		return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pElement = nullptr;
	for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
	{
		if (Element.strElementId == m_strSelectedElementId)
		{
			pElement = &Element;
			break;
		}
	}
	if (nullptr == pElement || pElement->AuthoringOverrides.Is_Empty())
		return false;
	std::string strError;
	if (!Reset_AllAuthoringOverrides(*pElement, strError))
	{
		m_strResourceStatus = "Reset all to Source rejected: " + strError;
		return false;
	}
	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	m_strResourceStatus = "All authoring overrides reset to source values.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindResource(
    const std::string& strAssetId)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before changing resources.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		m_strResourceStatus =
			"The exact adapter packet is inspection-only. Save the selected Decal/Trail as a generic Authored starting copy before binding resources.";
		return false;
	}
    const EFFECT_ELEMENT_DESC* pSelectedElement = Find_SelectedElement();
    if (!m_ActiveDocument.has_value() || nullptr == pSelectedElement)
    {
        m_strResourceStatus = "Select an Element before choosing a resource.";
        return false;
    }
	if (pSelectedElement->eKind == EFFECT_ELEMENT_KIND::LIGHT ||
		pSelectedElement->eKind == EFFECT_ELEMENT_KIND::SCREEN_POST)
	{
		m_strResourceStatus =
			"Presentation Light and Screen Post have no material resource lanes; edit their typed fields or delete the occurrence.";
		return false;
	}
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    EFFECT_ELEMENT_DESC* pElement = nullptr;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
        if (Element.strElementId == m_strSelectedElementId)
        {
            pElement = &Element;
            break;
        }
    }
	EFFECT_MATERIAL_TEXTURE_LANE_DESC* pMaterialLane =
		nullptr == pElement ? nullptr : Find_MaterialExecutionLane(
			*pElement, m_strSelectedResourceSlotId);
	EFFECT_NAMED_TEXTURE_DESC* pSourceTexture =
		nullptr == pElement ? nullptr : Find_SourceMaterialTexture(
			*pElement, m_strSelectedResourceSlotId);
	EFFECT_RESOURCE_BINDING_DESC* pBinding = nullptr;
	if (nullptr != pElement)
	{
		const auto Binding = std::find_if(
			pElement->ResourceBindings.begin(), pElement->ResourceBindings.end(),
			[this](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
			{ return Candidate.strSlotId == m_strSelectedResourceSlotId; });
		if (Binding != pElement->ResourceBindings.end())
			pBinding = &*Binding;
	}
    if (nullptr == pElement ||
		(nullptr == pMaterialLane && nullptr == pSourceTexture &&
		 !Slot_Allowed(*pElement, m_strSelectedResourceSlotId)))
    {
        m_strResourceStatus = "That resource slot is not allowed for this Element.";
        return false;
    }
	const EFFECT_RESOURCE_FILE_KIND eExpectedKind =
		(nullptr != pMaterialLane || nullptr != pSourceTexture) ?
		EFFECT_RESOURCE_FILE_KIND::TEXTURE :
		Slot_FileKind(*pElement, m_strSelectedResourceSlotId);
    const auto CatalogEntry = std::find_if(
        m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
        [this, &strAssetId, eExpectedKind](
            const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
        {
            return Entry.strAssetId == strAssetId &&
                Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                Entry.eFileKind == eExpectedKind;
        });
    if (CatalogEntry == m_ResourceCatalog.end() && !Is_AuthoringWorldResource(strAssetId, eExpectedKind))
    {
        m_strResourceStatus =
            "Selected resource is outside the active authoring category or file kind.";
        return false;
    }
	std::string strSlotLabel;
	bool_t bUnlockedMissingBaseSourceDecal = false;
	const bool_t bUnlockMissingBaseSourceDecal =
		Is_BaseTextureSlot(m_strSelectedResourceSlotId) &&
		Is_MissingBaseSourceDecal(*pElement);
	/* A hand-authored Element has no compiler lane set to override. Its
	   material template is the declaration, so the first bind of a template
	   slot creates the binding instead of being rejected as undeclared.
	   Imported and runtime documents keep the compiler lane set authoritative. */
	const bool_t bAuthoredTemplateSlot =
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
		Is_DirectHandAuthoredElement(*pElement) &&
		nullptr == pMaterialLane && nullptr == pSourceTexture &&
		nullptr == pBinding &&
		(m_strSelectedResourceSlotId == EFFECT_MESH_SHAPE_SLOT_ID ?
			(EFFECT_ELEMENT_KIND::MESH == pElement->eKind ||
			 EFFECT_ELEMENT_KIND::PARTICLE == pElement->eKind) :
			nullptr != Find_EffectMaterialInput(
				pElement->Material.strTemplateId,
				m_strSelectedResourceSlotId));
	if (bUnlockMissingBaseSourceDecal && nullptr == pBinding)
	{
		// Decal Base is the sole contract that may create a previously absent
		// resource target and change fail-closed preview admission.
		pElement->ResourceBindings.push_back(
			{ m_strSelectedResourceSlotId, strAssetId });
		pElement->Material.Execution.bFailClosed = false;
		pElement->bVisible = true;
		bUnlockedMissingBaseSourceDecal = true;
		strSlotLabel = Slot_Label(*pElement, m_strSelectedResourceSlotId);
	}
	else if (bAuthoredTemplateSlot)
	{
		pElement->ResourceBindings.push_back(
			{ m_strSelectedResourceSlotId, strAssetId });
		strSlotLabel = Slot_Label(*pElement, m_strSelectedResourceSlotId);
	}
	else
	{
		if (nullptr == pMaterialLane && nullptr == pSourceTexture &&
			nullptr == pBinding)
		{
			m_strResourceStatus =
				"Bind rejected: the compiler did not declare that resource lane.";
			return false;
		}
		std::string strError;
		if (!CEffectDocumentCodec::Set_AuthoringResourceOverride(
				*pElement, m_strSelectedResourceSlotId, strAssetId, strError))
		{
			m_strResourceStatus = "Resource override rejected: " + strError;
			return false;
		}
		if (nullptr != pMaterialLane)
		{
			strSlotLabel = pMaterialLane->strRole.empty() ?
				pMaterialLane->strLaneId : pMaterialLane->strRole;
		}
		else if (nullptr != pSourceTexture)
			strSlotLabel = pSourceTexture->strName;
		else
			strSlotLabel = Slot_Label(*pElement, m_strSelectedResourceSlotId);
	}
	const bool_t bWasDrawable = m_bActiveDocumentDrawable;
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
    m_strResourceStatus = "Bound " + strAssetId + " to " +
        strSlotLabel + ".";
	if (bUnlockedMissingBaseSourceDecal)
	{
		m_strResourceStatus +=
			" The imported Decal is now visible and admitted for preview; Save persists this authored Base binding.";
	}
	if (!bWasDrawable && m_bActiveDocumentDrawable)
	{
		m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		m_strPreviewIsolationElementId.clear();
		m_strPreviewIsolationGroupId.clear();
		m_strPreviewIsolationModelCueId.clear();
		m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
		Start_WorldPreviewFromBeginning();
		m_strResourceStatus +=
			" The Effect became drawable and its Complete preview started.";
	}
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedSlot()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before clearing resources.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		m_strResourceStatus =
			"The exact adapter packet is inspection-only. Save the selected Decal/Trail as a generic Authored starting copy before clearing resources.";
		return false;
	}
    if (!m_ActiveDocument.has_value() || nullptr == Find_SelectedElement())
        return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	bool_t bReset = false;
	bool_t bDeletedOptionalAuthoredBinding = false;
	bool_t bRelockedMissingBaseSourceDecal = false;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
		if (Element.strElementId != m_strSelectedElementId)
			continue;
		const auto Binding = std::find_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[this](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
			{
				return Candidate.strSlotId == m_strSelectedResourceSlotId;
			});
		const bool_t bHasAuthoringOverride =
			Element.AuthoringOverrides.ResourceBindings.end() != std::find_if(
				Element.AuthoringOverrides.ResourceBindings.begin(),
				Element.AuthoringOverrides.ResourceBindings.end(),
				[this](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{
					return Override.strSlotId ==
						m_strSelectedResourceSlotId;
				});
		const bool_t bDeleteOptionalAuthoredBinding =
			(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
			 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
			Binding != Element.ResourceBindings.end() &&
			!bHasAuthoringOverride &&
			nullptr == Find_MaterialExecutionLane(
				Element, m_strSelectedResourceSlotId) &&
			nullptr == Find_SourceMaterialTexture(
				Element, m_strSelectedResourceSlotId) &&
			Is_OptionalHandAuthoredResourceSlot(
				Element, m_strSelectedResourceSlotId);
		if (bDeleteOptionalAuthoredBinding)
		{
			Element.ResourceBindings.erase(Binding);
			bReset = true;
			bDeletedOptionalAuthoredBinding = true;
		}
		else if (Is_BaseTextureSlot(m_strSelectedResourceSlotId) &&
			Is_SourceDecalBaseAdmissionCarrier(Element))
		{
			const size_t iPreviousCount = Element.ResourceBindings.size();
			std::erase_if(Element.ResourceBindings,
				[this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{ return Binding.strSlotId == m_strSelectedResourceSlotId; });
			std::erase_if(Element.AuthoringOverrides.ResourceBindings,
				[this](
					const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{ return Override.strSlotId == m_strSelectedResourceSlotId; });
			bReset = Element.ResourceBindings.size() != iPreviousCount;
			if (bReset)
			{
				Element.Material.Execution.bFailClosed = true;
				Element.bVisible = false;
				bRelockedMissingBaseSourceDecal = true;
			}
		}
		else
		{
			std::string strError;
			bReset = CEffectDocumentCodec::Reset_AuthoringResourceOverride(
				Element, m_strSelectedResourceSlotId, strError);
			if (!bReset)
			{
				m_strResourceStatus =
					"Compiler-owned lanes cannot be deleted. " + strError;
				return false;
			}
		}
        break;
    }
	if (!bReset)
	{
		m_strResourceStatus =
			"The selected resource lane no longer exists on this Element.";
		return false;
	}
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
	if (bDeletedOptionalAuthoredBinding)
		m_strSelectedResourceAssetId.clear();
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	m_strResourceStatus = bRelockedMissingBaseSourceDecal ?
		"Cleared the Decal Base exception." :
		bDeletedOptionalAuthoredBinding ?
			"Deleted the selected optional authored resource slot. Save Changes to persist it." :
			"Reset the selected resource lane to Source.";
	if (bRelockedMissingBaseSourceDecal)
	{
		m_strResourceStatus +=
			" The imported Decal was hidden and returned to fail-closed until a Base DDS is bound.";
	}
    return true;
}

bool_t Client::CEffect_Tool::Try_SetSelectedElementFollowAnchor(
	const std::string& strBoneName)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strPreviewStatus =
			"Apply or Revert the open Detail draft before changing Element follow.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
	{
		m_strPreviewStatus =
			"Load an authored Effect and select a Trail or native Sprite Particle before setting follow.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource)
	{
		m_strPreviewStatus =
			"Element follow is editable only on a New or Authored Effect document.";
		return false;
	}
	if (strBoneName.empty())
	{
		m_strPreviewStatus =
			"Enter a Socket / Bone before setting Element follow.";
		return false;
	}
	float4x4_t AnchorWorld{};
	if (!CAnimationTargetService::Resolve_AnchorTransform(
			strBoneName.c_str(), &AnchorWorld))
	{
		m_strPreviewStatus = "Element follow rejected: model bone '" +
			strBoneName + "' does not exist on the current target.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	auto Selected = std::find_if(Staged.Elements.begin(), Staged.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	if (Selected == Staged.Elements.end() ||
		!Can_EditElementFollowAttachment(*Selected))
	{
		m_strPreviewStatus =
			"Select a Trail or native Sprite Particle Element before setting follow.";
		return false;
	}

	EFFECT_ACTION_CUE_ATTACHMENT_DESC Attachment =
		Selected->ActionCueAttachment;
	Attachment.bEnabled = true;
	Attachment.bFollow = true;
	Attachment.strSourceAnchorSlotId = strBoneName;
	/* Element IDs are stable and unique inside a Document, which makes them a
	   safe per-playback anchor-map key without inventing a vector-index ID. */
	Attachment.strRuntimeAnchorSlotId = Selected->strElementId;
	Attachment.strRuntimeBoneName = strBoneName;
	Attachment.fSnapshotRootSourceBasisYawDegrees = 0.f;
	Selected->ActionCueAttachment = std::move(Attachment);

	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	Start_WorldPreviewFromBeginning();
	m_strPreviewStatus = "Selected Element now follows model bone '" +
		strBoneName + "'. Save Changes to persist the attachment.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedElementFollowAnchor()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strPreviewStatus =
			"Apply or Revert the open Detail draft before clearing Element follow.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
		return false;
	if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource)
	{
		m_strPreviewStatus =
			"Element follow is editable only on a New or Authored Effect document.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	auto Selected = std::find_if(Staged.Elements.begin(), Staged.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	if (Selected == Staged.Elements.end() ||
		!Can_EditElementFollowAttachment(*Selected) ||
		!Selected->ActionCueAttachment.bEnabled)
	{
		m_strPreviewStatus =
			"The selected Element has no element-local follow to clear.";
		return false;
	}
	Selected->ActionCueAttachment = {};

	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	Start_WorldPreviewFromBeginning();
	m_strPreviewStatus =
		"Cleared the selected Element follow. Save Changes to persist the attachment change.";
	return true;
}

bool_t Client::CEffect_Tool::Try_CommitDocument(
    EFFECT_DOCUMENT_DESC&& Staged)
{
	if (m_bOccurrenceTuningDirty ||
		EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
	{
		m_strElementStatus =
			"Save the occurrence tuning artifact, then select an authored Effect before editing its Document.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		m_strElementStatus =
			"The exact adapter packet is read-only in full Details. Persist position/rotation/scale through Stable occurrence tuning; material/resource Save As requires a paired adapter authoring contract.";
		return false;
	}
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Staged, Error))
    {
        m_strElementStatus = Error;
        return false;
    }
    std::string DrawableError;
    if (!CEffectDocumentCodec::Validate_Drawable(Staged, DrawableError))
    {
        m_ActiveDocument = std::move(Staged);
        m_bMarkedElementIdsNeedPrune = true;
        Set_ActiveDocumentDrawableStatus(false, DrawableError);
        m_bDocumentDirty = true;
        m_bActiveDocumentMatchesRuntime = false;
        Recalculate_PreviewDuration();
        Release_WorldPreview(true);
        m_strPreviewStatus =
            "Document draft committed; preview hidden until required resources bind: " +
            DrawableError;
        return true;
    }
    if (!Stage_WorldPreview(Staged))
    {
        m_strElementStatus =
            "Change rejected; active Document and preview were preserved: " +
            m_strPreviewStatus;
        return false;
    }
    m_ActiveDocument = std::move(Staged);
    m_bMarkedElementIdsNeedPrune = true;
    Set_ActiveDocumentDrawableStatus(true, {});
    m_bDocumentDirty = true;
    m_bActiveDocumentMatchesRuntime = false;
    Recalculate_PreviewDuration();
    return true;
}

bool_t Client::CEffect_Tool::Try_SetPreviewFilter(
    const EFFECT_PREVIEW_FILTER eFilter)
{
    if (m_pAuthoringSequencer && m_pAuthoringSequencer->Is_Active()) m_pAuthoringSequencer->Stop();
    if (EFFECT_PREVIEW_FILTER::END == eFilter)
        return false;
    if (!m_ActiveDocument.has_value())
    {
        m_strPreviewStatus =
            "Load one Data File before choosing a preview scope.";
        return false;
    }
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED == eFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED == eFilter) &&
		m_strPreviewIsolationElementId.empty())
    {
        m_strPreviewStatus =
			"Use an Element Solo button before choosing Element Solo/Mute.";
        return false;
    }
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == eFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == eFilter) &&
		m_strPreviewIsolationGroupId.empty())
    {
        m_strPreviewStatus =
			"Use a Play Group button before choosing Group Solo/Mute.";
        return false;
    }
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE == eFilter &&
		m_strPreviewIsolationModelCueId.empty())
	{
		m_strPreviewStatus =
			"Use a Model / Summon Solo button before choosing Model Cue Solo.";
		return false;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES == eFilter &&
		m_ActiveDocument->ModelCues.empty())
	{
		m_strPreviewStatus = "The active Effect has no Model / Summon cue.";
		return false;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY == eFilter &&
		EFFECT_AUTHORING_FAMILY::END == m_ePreviewIsolationAuthoringFamily)
	{
		m_strPreviewStatus =
			"Use a Play Family button before choosing Family preview.";
		return false;
	}

	// Loading an authored document is CPU-only. Solo must select its own
	// character before resolving player-root or bone attachments.
	if (m_pAuthoringSequencer && !m_ProductPreview &&
		m_ActiveDocument->strEffectAssetId.ends_with(".restore") &&
		!Prepare_RecoveryPreviewTarget())
		return false;

    const EFFECT_PREVIEW_FILTER ePrevious = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    m_ePreviewFilter = eFilter;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (m_bParticleSystemDraftDirty)
        Apply_ParticleSystemDraft(Staged);
    if (m_bDetailDraftDirty)
        Apply_DetailDraft(Staged);
	if (m_bModelCueDraftDirty)
		Apply_ModelCueDraft(Staged);
    Recalculate_PreviewDuration(Build_PreviewDocument(Staged));
    if (!Stage_WorldPreview(Staged))
    {
        m_ePreviewFilter = ePrevious;
        m_fPreviewTimeSeconds = fPreviousTime;
        m_fPreviewDurationSeconds = fPreviousDuration;
        return false;
    }
    return true;
}
