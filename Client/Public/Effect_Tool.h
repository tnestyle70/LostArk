#pragma once

#include "CharacterPreviewPanel.h"
#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "EffectAuthoringTransfer.h"
#include "Engine_Defines.h"
#include "PlayerSkillCatalog.h"

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
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
    SOLO_SELECTED,
    MUTE_SELECTED,
    END
};

enum class EFFECT_DOCUMENT_SOURCE : uint8_t
{
    NEW_DOCUMENT,
    AUTHORED,
    IMPORTED,
    IMPORTED_REFERENCE,
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
        PLAYER_SKILL_DEFINITION Skill;
        EFFECT_DOCUMENT_DESC Document;
    };

    struct EFFECT_DATA_FILE_ENTRY final
    {
        std::string strAssetId;
        std::string strDomainId;
        std::filesystem::path Path;
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
    void Render_AllEffectsWindow();
    void Render_DataFilesWindow();
    void Render_EffectTypeSelector();
    void Render_ResourceSlots();
    void Render_ResourceGrid();
    void Rebuild_ResourceBrowserView(
        EFFECT_RESOURCE_FILE_KIND eFileKind,
        const std::string& strFilter,
        const std::string& strDomainId,
        const std::string& strCategory);
    void Render_Detail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_TransformDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_ColorDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_UVDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_UVKeyframes(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_TimingDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_KindDetail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_LerpDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_AnimationControls(
        const std::shared_ptr<Engine::CModel>& pModel);

    bool_t Try_CreateDocument();
    bool_t Try_AddElement();
    bool_t Try_DeleteSelectedElement();
    bool_t Try_ClearElements();
    bool_t Try_SaveDocument();
    bool_t Try_SaveDocumentAs(const std::string& strAssetId);
    bool_t Try_ReloadActiveDocument();
    bool_t Try_LoadDocument(const std::string& strAssetId);
    bool_t Try_LoadDocumentPath(
        const std::filesystem::path& Path,
        EFFECT_DOCUMENT_SOURCE eSource,
        const std::string& strSelectionId);
    bool_t Refresh_AllEffects();
    bool_t Refresh_DataFiles();
    bool_t Refresh_ResourceCatalog();
    void Select_AuthoringDomain(const std::string& strDomainId);
    bool_t Select_AuthoringDomainForClass(
        LostArk::Shared::CHARACTER_CLASS_ID eClass);
    bool_t Try_BindResource(const std::string& strAssetId);
    bool_t Try_ClearSelectedSlot();
    bool_t Try_CommitDocument(EFFECT_DOCUMENT_DESC&& Staged);
    bool_t Ensure_WorldPreviewObject();
    bool_t Stage_WorldPreview();
    bool_t Stage_WorldPreview(const EFFECT_DOCUMENT_DESC& Document);
    EFFECT_DOCUMENT_DESC Build_PreviewDocument(
        const EFFECT_DOCUMENT_DESC& Document) const;
    bool_t Try_SelectSkill(const std::string& strEffectAssetId);
    bool_t Try_SelectElement(
        const std::string& strEffectAssetId,
        const std::string& strElementId);
    bool_t Stage_DetailDraftPreview();
    bool_t Resolve_PreviewRoot(float4x4_t& OutRoot);
    void Start_WorldPreviewFromBeginning();
    void Synchronize_LoadedSkillPreview();
    void Release_WorldPreview(bool_t bRemoveFromLayer);
    void Update_Picking();
    void Discard_ActiveDocument();
    void Reset_DetailDraft();
    void Recalculate_PreviewDuration();
    void Recalculate_PreviewDuration(const EFFECT_DOCUMENT_DESC& Document);
    bool_t Has_UnsavedWork() const;
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
    optional<EFFECT_ELEMENT_DESC> m_DetailDraft;
    vector<EFFECT_RESOURCE_CATALOG_ENTRY> m_ResourceCatalog;
    vector<EFFECT_RESOURCE_DOMAIN_CATALOG> m_ResourceDomains;
    vector<size_t> m_VisibleResourceIndices;
    vector<EFFECT_SKILL_TREE_ENTRY> m_AllEffects;
    vector<EFFECT_DATA_FILE_ENTRY> m_DataFiles;
    vector<string> m_DataFileDomains;
    EFFECT_ELEMENT_KIND m_eSelectedEffectType = EFFECT_ELEMENT_KIND::MESH;
    LostArk::Shared::CHARACTER_CLASS_ID m_eAllEffectsClass =
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER;
    EFFECT_PREVIEW_FILTER m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    EFFECT_DOCUMENT_SOURCE m_eActiveDocumentSource =
        EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
    EFFECT_PREVIEW_PIVOT_KIND m_ePreviewPivotKind =
        EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
    std::filesystem::path m_ActiveDocumentPath;
    string m_strSelectedResourceSlotId = "meshModel";
    string m_strSelectedElementId;
    string m_strSelectedResourceAssetId;
    string m_strSelectedDataFileAssetId;
    string m_strSelectedAuthoringDomainId = "DimensionMaster";
    string m_strPreviewAnchorSlotId = "root";
    string m_strDetailDraftElementId;
    string m_strResourceViewFilter;
    string m_strResourceViewDomainId;
    string m_strResourceViewCategory;

    array<char_t, 129> m_NewAssetId{};
    array<char_t, 65> m_NewDisplayName{};
    array<char_t, 129> m_NewElementId{};
    array<char_t, 129> m_ResourceFilter{};
    array<char_t, 129> m_ResourceCategory{};
    array<char_t, 129> m_AllEffectsSearch{};
    array<char_t, 129> m_PreviewAnchorBuffer{};

    float4x4_t m_PreviewWorldRoot{};
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
    bool_t m_bDocumentDirty = false;
    bool_t m_bResourceCatalogRefreshAttempted = false;
    bool_t m_bAllEffectsRefreshAttempted = false;
    bool_t m_bDataFilesRefreshAttempted = false;
    bool_t m_bPendingWorldPivotPick = false;
    bool_t m_bDetailDraftDirty = false;
    bool_t m_bDiscardConfirmationRequested = false;
    uint64_t m_iFrameNumber = 0u;
    uint64_t m_iResourceCatalogRevision = 0u;
    uint64_t m_iResourceViewRevision = UINT64_MAX;
    EFFECT_RESOURCE_FILE_KIND m_eResourceViewFileKind =
        EFFECT_RESOURCE_FILE_KIND::END;
    uint32_t m_iCueTransferDurationMs = 250u;

    string m_strDocumentStatus;
    string m_strElementStatus;
    string m_strResourceStatus;
    string m_strPreviewStatus;
    string m_strPreviewAnimationStatus;
};

NS_END
