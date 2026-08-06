#pragma once

#include "CharacterPreviewPanel.h"
#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "EffectAuthoringTransfer.h"
#include "Engine_Defines.h"

#include <array>
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

class CEffect_Tool final
{
private:
    struct EFFECT_RESOURCE_CATALOG_ENTRY final
    {
        std::string strAssetId;
        EFFECT_RESOURCE_FILE_KIND eFileKind =
            EFFECT_RESOURCE_FILE_KIND::END;
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
    bool_t Try_LoadDocument(const std::string& strAssetId);
    bool_t Refresh_ResourceCatalog();
    bool_t Try_BindResource(const std::string& strAssetId);
    bool_t Try_ClearSelectedSlot();
    bool_t Try_CommitDocument(EFFECT_DOCUMENT_DESC&& Staged);
    bool_t Ensure_WorldPreviewObject();
    bool_t Stage_WorldPreview();
    bool_t Stage_WorldPreview(const EFFECT_DOCUMENT_DESC& Document);
    bool_t Resolve_PreviewRoot(float4x4_t& OutRoot);
    void Release_WorldPreview(bool_t bRemoveFromLayer);
    void Update_Picking();
    void Discard_ActiveDocument();
    void Reset_DetailDraft();
    void Recalculate_PreviewDuration();
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
    EFFECT_ELEMENT_KIND m_eSelectedEffectType = EFFECT_ELEMENT_KIND::MESH;
    EFFECT_RESOURCE_SLOT m_eSelectedResourceSlot = EFFECT_RESOURCE_SLOT::MESH_MODEL;
    EFFECT_PREVIEW_PIVOT_KIND m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
    string m_strSelectedElementId;
    string m_strSelectedResourceAssetId;
    string m_strSelectedDataFileAssetId;
    string m_strPreviewAnchorSlotId = "root";
    string m_strDetailDraftElementId;

    array<char_t, 129> m_NewAssetId{};
    array<char_t, 65> m_NewDisplayName{};
    array<char_t, 129> m_NewElementId{};
    array<char_t, 129> m_ResourceFilter{};
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
    bool_t m_bPendingWorldPivotPick = false;
    bool_t m_bDetailDraftDirty = false;
    bool_t m_bDiscardConfirmationRequested = false;
    uint64_t m_iFrameNumber = 0u;
    uint32_t m_iCueTransferDurationMs = 250u;

    string m_strDocumentStatus;
    string m_strElementStatus;
    string m_strResourceStatus;
    string m_strPreviewStatus;
};

NS_END
