#pragma once

#include "CharacterPreviewPanel.h"
#include "AnimationSkillBindingDocument.h"
#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_ComponentDocument.h"
#include "EffectAuthoringTransfer.h"
#include "Engine_Defines.h"
#include "PlayerSkillCatalog.h"

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CEffectObject;
class CEffectThumbnailCache;

enum class EFFECT_PREVIEW_PIVOT_KIND : uint8_t
{
    WORLD,
    PLAYER_ROOT,
    WEAPON_SOCKET,
    MODEL_BONE,
    END
};

enum class EFFECT_PREVIEW_FILTER : uint8_t
{
    COMPLETE,
    SOLO_PARTICLE_SYSTEM,
	SOLO_STANDALONE_MESHES,
	SOLO_MESH_EMITTERS,
	SOLO_STANDALONE_SPRITES,
	SOLO_SPRITE_EMITTERS,
    SOLO_SELECTED,
    MUTE_SELECTED,
    SOLO_SELECTED_GROUP,
    MUTE_SELECTED_GROUP,
    END
};

enum class EFFECT_DETAIL_SELECTION : uint8_t
{
    NONE,
	SKILL,
    PARTICLE_SYSTEM,
    ELEMENT,
	COMPONENT,
	EMITTER,
	SOURCE_MODULE,
    END
};

enum class EFFECT_DOCUMENT_SOURCE : uint8_t
{
    NEW_DOCUMENT,
    AUTHORED,
    IMPORTED,
    IMPORTED_REFERENCE,
	RUNTIME_ASSEMBLY,
	RUNTIME_COMPONENT,
    END
};

class CEffect_Tool final
{
private:
    struct EFFECT_RESOURCE_CATALOG_ENTRY final
    {
        std::string strAssetId;
        std::string strDomainId;
        std::string strCategory;
        EFFECT_RESOURCE_FILE_KIND eFileKind =
            EFFECT_RESOURCE_FILE_KIND::END;
    };

    struct EFFECT_RESOURCE_DOMAIN_CATALOG final
    {
        std::string strDomainId;
        std::array<std::vector<std::string>,
            static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)> Categories;
        std::array<size_t,
            static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)> ResourceCounts{};
    };

    struct EFFECT_SKILL_TREE_ENTRY final
    {
        struct PRODUCT_CUE final
        {
            ANIMATION_EFFECT_CUE Cue;
            size_t iBoundClipOrdinal = 0u;
        };

        PLAYER_SKILL_DEFINITION Skill;
        std::vector<PRODUCT_CUE> ProductCues;
        size_t iSourceReferenceCount = 0u;
        size_t iImportedReferenceCount = 0u;
        size_t iEmptySourceReferenceCount = 0u;
    };

    struct EFFECT_PRODUCT_PREVIEW final
    {
        LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
            LostArk::Shared::CHARACTER_CLASS_ID::END;
        LostArk::Shared::SKILL_ID iSkillId =
            LostArk::Shared::INVALID_SKILL_ID;
        EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE ProductCue;
    };

    struct EFFECT_DATA_FILE_ENTRY final
    {
        std::string strAssetId;
        std::string strDomainId;
        std::filesystem::path Path;
        EFFECT_DOCUMENT_SOURCE eSource = EFFECT_DOCUMENT_SOURCE::END;
    };

    struct PENDING_DOCUMENT_LOAD final
    {
        std::filesystem::path Path;
        std::string strSelectionId;
        EFFECT_DOCUMENT_SOURCE eSource = EFFECT_DOCUMENT_SOURCE::END;
    };

public:
    CEffect_Tool(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        shared_ptr<CCharacterPreviewPanel> pCharacterPreviewPanel);
    ~CEffect_Tool();

    void Update(f32_t fTimeDelta);
    void Render();

private:
    void Render_EffectToolWindow();
    void Render_ModelViewWindow();
    void Render_EffectDetailWindow();
    void Render_AuthoringSessionBar();
    void Render_AllEffectsWindow();
    void Render_LoadedEffectContents();
    bool_t Render_ManualElementGroups(
        const EFFECT_DOCUMENT_DESC& Document,
        const std::string& strEffectAssetId,
        bool_t bDefaultOpen);
    void Render_DataFilesWindow();
    void Render_PendingDocumentLoadModal();
    void Render_EffectTypeSelector();
    void Render_MeshAuthoringWorkbench();
    void Render_ParticleSystemDetail();
    void Render_ResourceSlots(bool_t bMeshAuthoringDraft);
    void Render_ResourceGrid(bool_t bMeshAuthoringDraft);
    void Rebuild_ResourceBrowserView(
        EFFECT_RESOURCE_FILE_KIND eFileKind,
        const std::string& strFilter,
        const std::string& strDomainId,
        const std::string& strCategory,
        const std::string& strShapeCategory);
    void Render_Detail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_TransformDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_ColorDetail(
        EFFECT_DETAIL_DESC& Detail,
        bool_t& bChanged,
        bool_t bHasEmissiveTexture);
    void Render_UVDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_UVKeyframes(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_TimingDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_KindDetail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_SourceRecipeDetail(
        EFFECT_CASCADE_RECIPE_DESC& Recipe,
        bool_t& bChanged);
    void Render_SourceModuleDetail(
        EFFECT_SOURCE_MODULE_DESC& Module,
		bool_t& bChanged,
		bool_t bDefaultOpen = false);
    void Render_SourceDistributionDetail(
        EFFECT_DISTRIBUTION_DESC& Distribution,
		const std::string_view strModuleClassName,
        bool_t& bChanged);
	void Render_SelectionPath() const;
	void Render_SkillSelectionDetail();
	void Render_AssemblyHierarchy(const std::string& strEffectAssetId);
	void Render_ComponentSelectionDetail();
	void Render_EmitterSelectionDetail();
	void Render_SourceModuleSelectionDetail();
    void Render_LerpDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_AnimationControls(
        const std::shared_ptr<Engine::CModel>& pModel);
    void Refresh_AnimationClipLabels(
        const std::shared_ptr<Engine::CModel>& pModel,
        bool_t bForce);

    bool_t Try_CreateDocument();
    bool_t Try_AddElement();
    bool_t Try_CreateMeshEffect();
    bool_t Try_BindMeshAuthoringResource(const std::string& strAssetId);
    bool_t Try_ClearMeshAuthoringSlot();
    bool_t Try_DeleteSelectedElement();
    bool_t Try_ClearElements();
    bool_t Try_ApplyDraftAndSave();
    bool_t Try_SaveDocument();
    bool_t Try_PublishActiveProductAndReloadRuntime();
    bool_t Try_SaveDocumentAs(const std::string& strAssetId);
    bool_t Try_PromoteImportedDocument();
    bool_t Try_ReloadActiveDocument();
    bool_t Try_LoadDocument(const std::string& strAssetId);
    bool_t Try_LoadDocumentPath(
        const std::filesystem::path& Path,
        EFFECT_DOCUMENT_SOURCE eSource,
        const std::string& strSelectionId);
    bool_t Try_LoadDocumentPathStaged(
        const std::filesystem::path& Path,
        EFFECT_DOCUMENT_SOURCE eSource,
        const std::string& strSelectionId,
        bool_t bBypassUnsavedGuard);
    bool_t Execute_PendingDocumentLoad(bool_t bSaveFirst);
    bool_t Refresh_AllEffects(bool_t bReloadSkillCatalog = false);
    bool_t Refresh_DataFiles();
    bool_t Refresh_ResourceCatalog();
    void Select_AuthoringDomain(const std::string& strDomainId);
    bool_t Select_AuthoringDomainForClass(
        LostArk::Shared::CHARACTER_CLASS_ID eClass);
    bool_t Try_BindResource(const std::string& strAssetId);
    bool_t Try_ClearSelectedSlot();
    bool_t Try_CommitDocument(EFFECT_DOCUMENT_DESC&& Staged);
    bool_t Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER eFilter);
    bool_t Ensure_WorldPreviewObject();
    bool_t Stage_WorldPreview();
    bool_t Stage_WorldPreview(const EFFECT_DOCUMENT_DESC& Document);
    EFFECT_DOCUMENT_DESC Build_PreviewDocument(
        const EFFECT_DOCUMENT_DESC& Document) const;
    bool_t Try_SelectProductCue(
        const EFFECT_SKILL_TREE_ENTRY& Entry,
        size_t iCueIndex);
    bool_t Try_SelectParticleSystem(const std::string& strEffectAssetId);
    bool_t Try_SelectElement(
        const std::string& strEffectAssetId,
        const std::string& strElementId);
	bool_t Try_SelectComponent(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId);
	bool_t Try_SelectEmitter(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId,
		const std::string& strEmitterId);
	bool_t Try_SelectSourceModule(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId,
		const std::string& strEmitterId,
		const std::string& strModuleStableId);
	bool_t Try_SelectFirstEmitter(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId);
    bool_t Try_AuditionParticleSystem();
    bool_t Try_AuditionSelectedElement();
    bool_t Stage_ParticleSystemDraftPreview();
    bool_t Stage_DetailDraftPreview();
    bool_t Apply_ParticleSystemDraft(EFFECT_DOCUMENT_DESC& Document) const;
    bool_t Apply_DetailDraft(EFFECT_DOCUMENT_DESC& Document) const;
    bool_t Resolve_PreviewRoot(float4x4_t& OutRoot);
    f32_t Resolve_EffectSampleTime(f32_t fTimelineSeconds) const;
    bool_t Is_ProductCueVisible(f32_t fTimelineSeconds) const;
    void Clear_ProductCuePreview();
    void Reset_ProductCueSnapshot();
    void Start_WorldPreviewFromBeginning();
    void Synchronize_LoadedSkillPreview();
    void Restart_SynchronizedAnimationSequence();
    void Seek_SynchronizedAnimationSequence(f32_t fTimeSeconds);
    void Set_SynchronizedAnimationPaused(bool_t bPaused);
    bool_t Try_ResolveSynchronizedAnimationTime(f32_t& fOutTimeSeconds) const;
    void Update_SynchronizedAnimationSequence();
    void Reset_SynchronizedAnimationSequence();
    void Hide_WorldPreview();
    void Release_WorldPreview(bool_t bRemoveFromLayer);
    void Update_Picking();
    void Discard_ActiveDocument();
    void Reset_MeshAuthoringDraft();
    void Reset_ParticleSystemDraft();
    void Reset_DetailDraft();
    void Recalculate_PreviewDuration();
    void Recalculate_PreviewDuration(const EFFECT_DOCUMENT_DESC& Document);
    bool_t Has_UnsavedWork() const;
    bool_t Has_UnappliedDetailDraft() const;
    void Set_ActiveDocumentDrawableStatus(
        bool_t bDrawable,
        std::string strError);
    void Clear_ActiveDocumentDrawableStatus();
    void Refresh_RuntimeEquivalence();
    EFFECT_ELEMENT_DESC* Find_SelectedElement();
    const EFFECT_ELEMENT_DESC* Find_SelectedElement() const;

private:
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    unique_ptr<CEffectThumbnailCache> m_pThumbnailCache;
    shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel;
    weak_ptr<CEffectObject> m_pWorldPreviewObject;
    uint32_t m_iWorldPreviewLevel = UINT32_MAX;

    optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
    optional<EFFECT_PRODUCT_PREVIEW> m_ProductPreview;
    EFFECT_ELEMENT_DESC m_MeshAuthoringDraft;
    optional<EFFECT_PARTICLE_SYSTEM_DESC> m_ParticleSystemDraft;
    optional<EFFECT_ELEMENT_DESC> m_DetailDraft;
    optional<PENDING_DOCUMENT_LOAD> m_PendingDocumentLoad;
    vector<EFFECT_RESOURCE_CATALOG_ENTRY> m_ResourceCatalog;
    vector<EFFECT_RESOURCE_DOMAIN_CATALOG> m_ResourceDomains;
    vector<size_t> m_VisibleResourceIndices;
    vector<EFFECT_SKILL_TREE_ENTRY> m_AllEffects;
    vector<EFFECT_DATA_FILE_ENTRY> m_DataFiles;
    vector<string> m_DataFileDomains;
    vector<ANIMATION_SKILL_CLIP> m_SynchronizedAnimationClips;
    vector<string> m_AnimationClipDisplayLabels;
    EFFECT_ELEMENT_KIND m_eSelectedEffectType = EFFECT_ELEMENT_KIND::MESH;
    EFFECT_DETAIL_SELECTION m_eDetailSelection =
        EFFECT_DETAIL_SELECTION::NONE;
    LostArk::Shared::CHARACTER_CLASS_ID m_eAllEffectsClass =
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER;
    EFFECT_PREVIEW_FILTER m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    EFFECT_DOCUMENT_SOURCE m_eActiveDocumentSource =
        EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
    EFFECT_PREVIEW_PIVOT_KIND m_ePreviewPivotKind =
        EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
    std::filesystem::path m_ActiveDocumentPath;
	string m_strActiveDocumentBaselineCanonical;
    string m_strSelectedResourceSlotId = "meshModel";
    string m_strSelectedElementId;
    string m_strSelectedElementGroupId;
	string m_strSelectedComponentId;
	string m_strSelectedEmitterId;
	string m_strSelectedSourceModuleId;
    string m_strSelectedResourceAssetId;
    string m_strSelectedDataFileAssetId;
    string m_strSelectedAuthoringDomainId = "DimensionMaster";
    string m_strPreviewAnchorSlotId = "root";
    string m_strDetailDraftElementId;
    string m_strResourceViewFilter;
    string m_strResourceViewDomainId;
    string m_strResourceViewCategory;
    string m_strResourceViewShapeCategory;
    string m_strMeshShapeCategory = "All";

    array<char_t, 129> m_NewAssetId{};
    array<char_t, 65> m_NewDisplayName{};
    array<char_t, 129> m_NewElementId{};
    array<char_t, 129> m_ResourceFilter{};
    array<char_t, 129> m_ResourceCategory{};
    array<char_t, 129> m_AllEffectsSearch{};
    array<char_t, 129> m_DataFilesSearch{};
    array<char_t, 129> m_PreviewAnchorBuffer{};

    float4x4_t m_PreviewWorldRoot{};
    float4x4_t m_ProductCueSnapshotRoot{};
    EFFECT_TRANSFORM_DESC m_CueTransferLocalTransform{};
    EFFECT_FOLLOW_POLICY m_eCueTransferFollowPolicy =
        EFFECT_FOLLOW_POLICY::FOLLOW;
    EFFECT_STOP_POLICY m_eCueTransferStopPolicy =
        EFFECT_STOP_POLICY::NATURAL;
    float2_t m_vMouseViewportPosition{};
    float3_t m_vPickedWorldPosition{};
    f32_t m_fPreviewTimeSeconds = 0.f;
    f32_t m_fPreviewDurationSeconds = 1.f;
    bool_t m_bPreviewPlaying = false;
    bool_t m_bPreviewLoop = true;
	bool_t m_bPreviewVisibleRequested = false;
	bool_t m_bPreviewScreenPostEnabled = true;
    bool_t m_bDocumentDirty = false;
    bool_t m_bActiveDocumentDrawable = false;
    bool_t m_bActiveDocumentMatchesRuntime = false;
    bool_t m_bResourceCatalogRefreshAttempted = false;
    bool_t m_bAllEffectsRefreshAttempted = false;
    bool_t m_bDataFilesRefreshAttempted = false;
    bool_t m_bPendingWorldPivotPick = false;
    bool_t m_bParticleSystemDraftDirty = false;
    bool_t m_bMeshAuthoringDraftInitialized = false;
    bool_t m_bDetailDraftDirty = false;
    bool_t m_bDiscardConfirmationRequested = false;
    bool_t m_bPromoteConfirmationRequested = false;
    bool_t m_bPendingDocumentLoadModalRequested = false;
    bool_t m_bProductCueSnapshotCaptured = false;
    uint64_t m_iFrameNumber = 0u;
    uint64_t m_iSynchronizedAnimationTargetGeneration = 0u;
    uint64_t m_iAnimationClipLabelTargetGeneration = 0u;
    uint64_t m_iResourceCatalogRevision = 0u;
    uint64_t m_iResourceViewRevision = UINT64_MAX;
    EFFECT_RESOURCE_FILE_KIND m_eResourceViewFileKind =
        EFFECT_RESOURCE_FILE_KIND::END;
    EFFECT_RESOURCE_FILE_KIND m_eResourceLibraryFileKind =
        EFFECT_RESOURCE_FILE_KIND::MODEL;
    uint32_t m_iCueTransferDurationMs = 250u;
    size_t m_iSynchronizedAnimationClipIndex = 0u;

    string m_strDocumentStatus;
    string m_strActiveDocumentDrawableError;
    shared_ptr<const EFFECT_DOCUMENT_DESC> m_pRuntimeEquivalenceDocument;
    string m_strRuntimeEquivalenceCanonical;
    string m_strElementStatus;
    string m_strDetailStatus;
    string m_strResourceStatus;
    string m_strPreviewStatus;
    string m_strPreviewAnimationStatus;
    string m_strAnimationClipLabelStatus;
};

NS_END
