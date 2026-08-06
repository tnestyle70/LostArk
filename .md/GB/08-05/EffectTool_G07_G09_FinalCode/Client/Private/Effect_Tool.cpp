#include "imgui.h"

#include "Effect_Tool.h"

#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "EffectAuthoringTransfer.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_Object.h"
#include "Effect_ThumbnailCache.h"
#include "GameInstance.h"
#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string_view>
#include <system_error>
#include <utility>

namespace
{
    constexpr const wchar_t* PREVIEW_LAYER = L"Layer_EffectPreview";

    const char* Kind_Label(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        switch (eKind)
        {
        case Client::EFFECT_ELEMENT_KIND::MESH: return "Mesh";
        case Client::EFFECT_ELEMENT_KIND::SPRITE: return "Texture";
        case Client::EFFECT_ELEMENT_KIND::PARTICLE: return "Particle";
        case Client::EFFECT_ELEMENT_KIND::DECAL: return "Decal";
        case Client::EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
        case Client::EFFECT_ELEMENT_KIND::END:
        default: return "Invalid";
        }
    }

    const char* Slot_Label(const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        switch (eSlot)
        {
        case Client::EFFECT_RESOURCE_SLOT::MESH_MODEL: return "Mesh";
        case Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "Base";
        case Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "Noise";
        case Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "Mask";
        case Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "Emissive";
        case Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "Dissolve";
        case Client::EFFECT_RESOURCE_SLOT::END:
        default: return "Invalid";
        }
    }

    const char* Profile_Label(const Client::EFFECT_RENDER_PROFILE eProfile)
    {
        switch (eProfile)
        {
        case Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
            return "Opaque / Back / Depth Write";
        case Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
            return "Alpha / Two Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
            return "Additive / Two Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::END:
        default: return "Invalid";
        }
    }

    bool Contains_NoCase(
        const std::string& Value,
        const std::string_view Filter)
    {
        if (Filter.empty())
            return true;
        return Value.end() != std::search(
            Value.begin(), Value.end(), Filter.begin(), Filter.end(),
            [](const char Left, const char Right)
            {
                return std::tolower(static_cast<unsigned char>(Left)) ==
                    std::tolower(static_cast<unsigned char>(Right));
            });
    }

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_KIND eKind,
        const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        if (Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
            return Client::EFFECT_ELEMENT_KIND::MESH == eKind;
        return eKind < Client::EFFECT_ELEMENT_KIND::END &&
            eSlot >= Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
            eSlot <= Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE;
    }

    Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        return Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot ?
            Client::EFFECT_RESOURCE_FILE_KIND::MODEL :
            Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    }

    bool InputFloat2(const char* Label, float2_t& Value)
    {
        return ImGui::InputFloat2(Label, &Value.x, "%.3f");
    }

    bool InputFloat3(const char* Label, float3_t& Value)
    {
        return ImGui::InputFloat3(Label, &Value.x, "%.3f");
    }

    bool InputFloat4(const char* Label, float4_t& Value)
    {
        return ImGui::InputFloat4(Label, &Value.x, "%.3f");
    }

    void Copy_Buffer(char* pDestination, const size_t iCapacity,
        const std::string& Source)
    {
        if (nullptr == pDestination || 0u == iCapacity)
            return;
        const size_t Count = (std::min)(iCapacity - 1u, Source.size());
        std::memcpy(pDestination, Source.data(), Count);
        pDestination[Count] = '\0';
    }

    float4x4_t Identity_Matrix()
    {
        float4x4_t Result{};
        XMStoreFloat4x4(&Result, XMMatrixIdentity());
        return Result;
    }

    const Client::CHARACTER_SPEC* Resolve_CurrentTargetSpec()
    {
        const std::string assetName =
            Client::CAnimationTargetService::Resolve_AssetName();
        const Client::CHARACTER_SPEC* specs[] =
        {
            &Client::Spec_LanceMaster,
            &Client::Spec_GunSlinger,
            &Client::Spec_Slayer,
            &Client::Spec_Artist,
            &Client::Spec_DimensionMaster
        };
        for (const Client::CHARACTER_SPEC* pSpec : specs)
        {
            if (nullptr != pSpec && nullptr != pSpec->pAssetName &&
                assetName == pSpec->pAssetName)
                return pSpec;
        }
        return nullptr;
    }
}

Client::CEffect_Tool::CEffect_Tool(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext,
    shared_ptr<CCharacterPreviewPanel> pCharacterPreviewPanel)
    : m_pDevice(std::move(pDevice)),
      m_pContext(std::move(pContext)),
      m_pThumbnailCache(std::make_unique<CEffectThumbnailCache>(
          m_pDevice, m_pContext)),
      m_pCharacterPreviewPanel(std::move(pCharacterPreviewPanel)),
      m_PreviewWorldRoot(Identity_Matrix())
{
    Copy_Buffer(m_PreviewAnchorBuffer.data(),
        m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
}

Client::CEffect_Tool::~CEffect_Tool()
{
    m_pCharacterPreviewPanel->Set_SessionLock(
        CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL, false, {});
    Release_WorldPreview(true);
}

void Client::CEffect_Tool::Update(const f32_t fTimeDelta)
{
    ++m_iFrameNumber;
    m_pThumbnailCache->Begin_Frame(m_iFrameNumber);
    m_pCharacterPreviewPanel->Set_SessionLock(
        CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL,
        Has_UnsavedWork(),
        "Apply or discard Effect changes before changing target.");
    m_pCharacterPreviewPanel->Refresh_Level();
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    if (m_iWorldPreviewLevel != UINT32_MAX &&
        m_iWorldPreviewLevel != iCurrentLevel)
    {
        Release_WorldPreview(false);
    }
    if (!m_ActiveDocument.has_value())
        return;
    f32_t fSequentialAdvance = 0.f;
    bool_t bSeekAfterLoop = false;
    if (m_bPreviewPlaying)
    {
        const f32_t fPreviousTime = m_fPreviewTimeSeconds;
        m_fPreviewTimeSeconds += (std::max)(0.f, fTimeDelta);
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
                0.f, m_fPreviewTimeSeconds - fPreviousTime);
    }
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject)
        return;
    float4x4_t Root{};
    const bool_t bRootResolved = Resolve_PreviewRoot(Root);
    pObject->Set_Visible(bRootResolved);
    if (bRootResolved)
    {
        pObject->Set_RootWorld(Root);
        if (0u == m_strPreviewStatus.find("World preview hidden:"))
            m_strPreviewStatus = "World preview anchor resolved.";
    }
    else
    {
        m_strPreviewStatus = "World preview hidden: current target cannot resolve " +
            (EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                std::string("its root pivot.") :
                std::string("anchor '") + m_strPreviewAnchorSlotId + "'.");
    }
    if (bSeekAfterLoop)
        pObject->Set_SampleTime(m_fPreviewTimeSeconds);
    else if (fSequentialAdvance > 0.f)
        pObject->Advance_Preview(fSequentialAdvance);
}

void Client::CEffect_Tool::Render()
{
    Render_EffectToolWindow();
    Render_ModelViewWindow();
    Render_EffectDetailWindow();
    Render_AllEffectsWindow();
    Render_DataFilesWindow();
    m_pThumbnailCache->Trim();
}

void Client::CEffect_Tool::Render_EffectToolWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 660.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Effect Tool"))
    {
        ImGui::End();
        return;
    }
    ImGui::TextUnformatted("Build one Effect from typed visual Elements.");
    Render_EffectTypeSelector();
    if (ImGui::Button("Reset"))
    {
        m_fPreviewTimeSeconds = 0.f;
        if (const shared_ptr<CEffectObject> pObject =
            m_pWorldPreviewObject.lock())
            pObject->Reset();
    }
    ImGui::SameLine();
    if (ImGui::Button("CreateEffect"))
        Try_AddElement();
    ImGui::SameLine();
    if (ImGui::Button("Update Textures"))
        Refresh_ResourceCatalog();
    ImGui::SameLine();
    if (ImGui::Button("Update Meshes"))
        Refresh_ResourceCatalog();
    ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
        m_ResourceFilter.size());
    Render_ResourceSlots();
    Render_ResourceGrid();
    if (!m_strResourceStatus.empty())
        ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
    ImGui::End();
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
            m_eSelectedResourceSlot = EFFECT_ELEMENT_KIND::MESH == eKind ?
                EFFECT_RESOURCE_SLOT::MESH_MODEL :
                EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
        }
    }
}

void Client::CEffect_Tool::Render_ResourceSlots()
{
    ImGui::SeparatorText("Resource Categories / Bound Slots");
    const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
    const EFFECT_ELEMENT_KIND eKind = nullptr != pElement ?
        pElement->eKind : m_eSelectedEffectType;
    bool_t bFirstVisibleSlot = true;
    for (int32_t iSlot = 0;
        iSlot < static_cast<int32_t>(EFFECT_RESOURCE_SLOT::END); ++iSlot)
    {
        const EFFECT_RESOURCE_SLOT eSlot =
            static_cast<EFFECT_RESOURCE_SLOT>(iSlot);
        if (!Slot_Allowed(eKind, eSlot))
            continue;
        if (!bFirstVisibleSlot)
            ImGui::SameLine();
        ImGui::PushID(iSlot);
        ImGui::BeginGroup();
        const std::string* pBoundAsset = nullptr;
        if (nullptr != pElement)
        {
            const auto Binding = std::find_if(
                pElement->ResourceBindings.begin(),
                pElement->ResourceBindings.end(),
                [eSlot](const EFFECT_RESOURCE_BINDING_DESC& Resource)
                {
                    return Resource.eSlot == eSlot;
                });
            if (Binding != pElement->ResourceBindings.end())
                pBoundAsset = &Binding->strAssetId;
        }
        bool_t bSlotClicked = false;
        if (nullptr != pBoundAsset &&
            EFFECT_RESOURCE_FILE_KIND::TEXTURE == Slot_FileKind(eSlot))
        {
            const CEffectThumbnailCache::RESULT Thumbnail =
                m_pThumbnailCache->Request(*pBoundAsset);
            if (nullptr != Thumbnail.pTextureView)
            {
                ImGui::Image(Thumbnail.pTextureView, ImVec2(58.f, 52.f));
                bSlotClicked = ImGui::IsItemClicked();
            }
            else
                bSlotClicked = ImGui::Button("DDS", ImVec2(58.f, 52.f));
        }
        else
        {
            const char* pCardLabel = nullptr == pBoundAsset ?
                "Empty" : "WMODEL";
            bSlotClicked = ImGui::Button(pCardLabel, ImVec2(58.f, 52.f));
        }
        if (m_eSelectedResourceSlot == eSlot)
        {
            ImGui::GetWindowDrawList()->AddRect(
                ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 2.f);
        }
        if (bSlotClicked)
            m_eSelectedResourceSlot = eSlot;
        ImGui::TextUnformatted(Slot_Label(eSlot));
        if (nullptr != pBoundAsset)
        {
            std::string Name = std::filesystem::path(
                *pBoundAsset).filename().string();
            if (Name.size() > 9u)
                Name = Name.substr(0u, 7u) + "..";
            ImGui::TextDisabled("%s", Name.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", pBoundAsset->c_str());
        }
        ImGui::EndGroup();
        ImGui::PopID();
        bFirstVisibleSlot = false;
    }
    if (ImGui::Button("Clear Selected Slot"))
        Try_ClearSelectedSlot();
}

void Client::CEffect_Tool::Render_ResourceGrid()
{
    if (!m_bResourceCatalogRefreshAttempted)
        Refresh_ResourceCatalog();
    const EFFECT_RESOURCE_FILE_KIND eWanted =
        Slot_FileKind(m_eSelectedResourceSlot);
    const std::string Filter = m_ResourceFilter.data();
    std::string BoundAssetId;
    if (const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement())
    {
        const auto Binding = std::find_if(
            pElement->ResourceBindings.begin(),
            pElement->ResourceBindings.end(),
            [this](const EFFECT_RESOURCE_BINDING_DESC& Resource)
            {
                return Resource.eSlot == m_eSelectedResourceSlot;
            });
        if (Binding != pElement->ResourceBindings.end())
            BoundAssetId = Binding->strAssetId;
    }
    const float CardWidth = 92.f;
    const int32_t Columns = (std::max)(1,
        static_cast<int32_t>(ImGui::GetContentRegionAvail().x / CardWidth));
    vector<const EFFECT_RESOURCE_CATALOG_ENTRY*> VisibleEntries;
    VisibleEntries.reserve(m_ResourceCatalog.size());
    for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry : m_ResourceCatalog)
    {
        if (Entry.eFileKind != eWanted ||
            !Contains_NoCase(Entry.strAssetId, Filter))
            continue;
        VisibleEntries.push_back(&Entry);
    }
    const int32_t Rows = static_cast<int32_t>(
        (VisibleEntries.size() + Columns - 1u) / Columns);
    ImGuiListClipper Clipper;
    Clipper.Begin(Rows, 112.f);
    while (Clipper.Step())
    {
        for (int32_t iRow = Clipper.DisplayStart;
            iRow < Clipper.DisplayEnd; ++iRow)
        {
            for (int32_t iColumn = 0; iColumn < Columns; ++iColumn)
            {
                const size_t iEntry = static_cast<size_t>(
                    iRow * Columns + iColumn);
                if (iEntry >= VisibleEntries.size())
                    break;
                const EFFECT_RESOURCE_CATALOG_ENTRY& Entry =
                    *VisibleEntries[iEntry];
                ImGui::PushID(Entry.strAssetId.c_str());
                if (0 != iColumn)
                    ImGui::SameLine();
                ImGui::BeginGroup();
                bool_t bClicked = false;
                if (EFFECT_RESOURCE_FILE_KIND::TEXTURE == Entry.eFileKind)
                {
                    const CEffectThumbnailCache::RESULT Thumbnail =
                        m_pThumbnailCache->Request(Entry.strAssetId);
                    if (nullptr != Thumbnail.pTextureView)
                    {
                        ImGui::Image(Thumbnail.pTextureView, ImVec2(80.f, 80.f));
                        bClicked = ImGui::IsItemClicked();
                    }
                    else
                    {
                        bClicked = ImGui::Button("DDS", ImVec2(80.f, 80.f));
                    }
                }
                else
                {
                    bClicked = ImGui::Button("WMODEL", ImVec2(80.f, 80.f));
                }
                if (Entry.strAssetId == BoundAssetId)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                        ImGui::GetColorU32(ImGuiCol_HeaderActive),
                        2.f, 0, 3.f);
                }
                std::string Name = std::filesystem::path(
                    Entry.strAssetId).filename().string();
                if (Name.size() > 13u)
                    Name = Name.substr(0u, 10u) + "...";
                ImGui::TextUnformatted(Name.c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Entry.strAssetId.c_str());
                ImGui::EndGroup();
                if (bClicked)
                {
                    m_strSelectedResourceAssetId = Entry.strAssetId;
                    Try_BindResource(Entry.strAssetId);
                }
                ImGui::PopID();
            }
        }
    }
}

void Client::CEffect_Tool::Render_ModelViewWindow()
{
    ImGui::SetNextWindowPos(ImVec2(450.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(650.f, 660.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.f);
    if (!ImGui::Begin("Model View"))
    {
        ImGui::End();
        return;
    }
    m_pCharacterPreviewPanel->Render_Selector(false, {});
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    Render_AnimationControls(pModel);

    ImGui::SeparatorText("Effect Pivot");
    if (const CHARACTER_SPEC* pSpec = Resolve_CurrentTargetSpec())
    {
        if (nullptr != pSpec && nullptr != pSpec->pWeapons)
        {
            for (uint32_t iWeapon = 0u;
                iWeapon < pSpec->iNumWeapons; ++iWeapon)
            {
                const char* pSocket = pSpec->pWeapons[iWeapon].pSocketBone;
                if (nullptr == pSocket)
                    continue;
                if (ImGui::Selectable(pSocket,
                    m_strPreviewAnchorSlotId == pSocket))
                {
                    m_strPreviewAnchorSlotId = pSocket;
                    Copy_Buffer(m_PreviewAnchorBuffer.data(),
                        m_PreviewAnchorBuffer.size(),
                        m_strPreviewAnchorSlotId);
                }
            }
        }
    }
    if (ImGui::InputText("Socket / Bone", m_PreviewAnchorBuffer.data(),
        m_PreviewAnchorBuffer.size()))
    {
        m_strPreviewAnchorSlotId = m_PreviewAnchorBuffer.data();
    }
    if (ImGui::Button("Set Effect Pivot Player"))
    {
        m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
        m_strPreviewStatus = "Effect follows the selected Character root.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Effect Pivot Weapon"))
    {
        float4x4_t Test{};
        if (CAnimationTargetService::Resolve_AnchorTransform(
            m_strPreviewAnchorSlotId.c_str(), &Test))
        {
            m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET;
            m_strPreviewStatus = "Effect follows weapon socket: " +
                m_strPreviewAnchorSlotId;
        }
        else
            m_strPreviewStatus = "Selected weapon socket does not exist.";
    }
    if (ImGui::Button("Set Effect Pivot Bone"))
    {
        float4x4_t Test{};
        if (CAnimationTargetService::Resolve_AnchorTransform(
            m_strPreviewAnchorSlotId.c_str(), &Test))
        {
            m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE;
            m_strPreviewStatus = "Effect follows model bone: " +
                m_strPreviewAnchorSlotId;
        }
        else
            m_strPreviewStatus = "Selected model bone does not exist.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Effect Pivot"))
    {
        m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
        m_strPreviewStatus = "Effect uses the fixed world pivot.";
    }
    if (ImGui::Button("Pick World Pivot"))
    {
        m_bPendingWorldPivotPick = true;
        m_strPreviewStatus =
            "Click empty space in Model View to pick the world surface.";
    }

    ImGui::SeparatorText("Animation Cue Transfer");
    InputFloat3("Cue Local Position", m_CueTransferLocalTransform.vPosition);
    InputFloat3("Cue Local Rotation", m_CueTransferLocalTransform.vRotationDegrees);
    InputFloat3("Cue Local Scale", m_CueTransferLocalTransform.vScale);
    if (ImGui::RadioButton("Cue Follow",
        EFFECT_FOLLOW_POLICY::FOLLOW == m_eCueTransferFollowPolicy))
        m_eCueTransferFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
    ImGui::SameLine();
    if (ImGui::RadioButton("Cue Snapshot",
        EFFECT_FOLLOW_POLICY::SNAPSHOT == m_eCueTransferFollowPolicy))
        m_eCueTransferFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
    if (ImGui::RadioButton("Natural Stop",
        EFFECT_STOP_POLICY::NATURAL == m_eCueTransferStopPolicy))
        m_eCueTransferStopPolicy = EFFECT_STOP_POLICY::NATURAL;
    ImGui::SameLine();
    if (ImGui::RadioButton("Cue End Stop",
        EFFECT_STOP_POLICY::CUE_END == m_eCueTransferStopPolicy))
        m_eCueTransferStopPolicy = EFFECT_STOP_POLICY::CUE_END;
    if (EFFECT_STOP_POLICY::CUE_END == m_eCueTransferStopPolicy)
        ImGui::InputScalar("Cue Duration (ms)", ImGuiDataType_U32,
            &m_iCueTransferDurationMs);

    const bool_t bCueScaleValid =
        m_CueTransferLocalTransform.vScale.x > 0.f &&
        m_CueTransferLocalTransform.vScale.y > 0.f &&
        m_CueTransferLocalTransform.vScale.z > 0.f;
    const shared_ptr<const EFFECT_DOCUMENT_DESC> pPublishedDocument =
        m_ActiveDocument.has_value() ?
        CEffectCatalog::Find(m_ActiveDocument->strEffectAssetId) : nullptr;
    const bool_t bPublishedMatchesActive =
        nullptr != pPublishedDocument && m_ActiveDocument.has_value() &&
        CEffectDocumentCodec::Serialize(*pPublishedDocument) ==
            CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    const bool_t bCanTransfer = m_ActiveDocument.has_value() &&
        !Has_UnsavedWork() &&
        bPublishedMatchesActive &&
        EFFECT_PREVIEW_PIVOT_KIND::WORLD != m_ePreviewPivotKind &&
        nullptr != pModel && bCueScaleValid &&
        (EFFECT_STOP_POLICY::NATURAL == m_eCueTransferStopPolicy ||
            m_iCueTransferDurationMs > 0u);
    ImGui::BeginDisabled(!bCanTransfer);
    if (ImGui::Button("Use Selected Effect in Animation Tool"))
    {
        const uint32_t iAnimation = pModel->Get_CurrentAnimIndex();
        const char_t* pClipName = pModel->Get_AnimationName(iAnimation);
        f32_t fPosition = 0.f;
        f32_t fDuration = 0.f;
        const f32_t fTicksPerSecond =
            pModel->Get_AnimationTickPerSecond(iAnimation);
        if (nullptr == pClipName ||
            !pModel->Get_AnimationProgress(iAnimation, fPosition, fDuration) ||
            !std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
        {
            m_strPreviewStatus = "Current animation time cannot be transferred.";
        }
        else
        {
            EFFECT_AUTHORING_CUE_TRANSFER Transfer;
            Transfer.iTargetGeneration =
                CAnimationTargetService::Resolve_TargetGeneration();
            Transfer.strAnimationAssetId =
                CAnimationTargetService::Resolve_AssetName();
            Transfer.strClipName = pClipName;
            Transfer.iTimeMs = static_cast<uint32_t>((std::max)(
                0.f, fPosition / fTicksPerSecond * 1000.f));
            Transfer.iDurationMs =
                EFFECT_STOP_POLICY::CUE_END == m_eCueTransferStopPolicy ?
                m_iCueTransferDurationMs : 0u;
            Transfer.strEffectAssetId =
                m_ActiveDocument->strEffectAssetId;
            Transfer.strAnchorSlotId =
                EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                "root" : m_strPreviewAnchorSlotId;
            Transfer.ePivotKind =
                EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                EFFECT_CUE_PIVOT_KIND::PLAYER_ROOT :
                (EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET == m_ePreviewPivotKind ?
                    EFFECT_CUE_PIVOT_KIND::WEAPON_SOCKET :
                    EFFECT_CUE_PIVOT_KIND::MODEL_BONE);
            Transfer.LocalTransform = m_CueTransferLocalTransform;
            Transfer.eFollowPolicy = m_eCueTransferFollowPolicy;
            Transfer.eStopPolicy = m_eCueTransferStopPolicy;
            CEffectAuthoringTransfer::Publish(std::move(Transfer));
            m_strPreviewStatus =
                "Queued admitted Effect cue for Animation Tool: " +
                m_ActiveDocument->strEffectAssetId;
        }
    }
    ImGui::EndDisabled();
    if (!bCanTransfer)
    {
        ImGui::TextDisabled(
            "Save, publish/reload the exact Effect, then choose Player/Weapon/Bone pivot.");
    }

    ImGui::SeparatorText("Timeline");
    if (ImGui::Button(m_bPreviewPlaying ? "Pause" : "Play"))
        m_bPreviewPlaying = !m_bPreviewPlaying;
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &m_bPreviewLoop);
    ImGui::SameLine();
    if (ImGui::Button("Time Reset All"))
    {
        m_fPreviewTimeSeconds = 0.f;
        if (const shared_ptr<CEffectObject> pObject =
            m_pWorldPreviewObject.lock())
            pObject->Reset();
    }
    if (ImGui::SliderFloat("Sample Time", &m_fPreviewTimeSeconds,
        0.f, m_fPreviewDurationSeconds, "%.3f s"))
    {
        m_bPreviewPlaying = false;
        if (const shared_ptr<CEffectObject> pObject =
            m_pWorldPreviewObject.lock())
            pObject->Set_SampleTime(m_fPreviewTimeSeconds);
    }

    const ImVec2 Mouse = ImGui::GetMousePos();
    const ImVec2 WindowPosition = ImGui::GetWindowPos();
    m_vMouseViewportPosition = {
        Mouse.x - WindowPosition.x,
        Mouse.y - WindowPosition.y };
    ImGui::Text("Mouse Viewport Position: %.0f, %.0f",
        m_vMouseViewportPosition.x, m_vMouseViewportPosition.y);
    ImGui::Text("Picked World Position: %.3f, %.3f, %.3f",
        m_vPickedWorldPosition.x,
        m_vPickedWorldPosition.y,
        m_vPickedWorldPosition.z);
    if (!m_strPreviewStatus.empty())
        ImGui::TextWrapped("%s", m_strPreviewStatus.c_str());
    if (!m_pCharacterPreviewPanel->Get_Status().empty())
        ImGui::TextWrapped("%s",
            m_pCharacterPreviewPanel->Get_Status().c_str());
    Update_Picking();
    ImGui::End();
}

void Client::CEffect_Tool::Render_AnimationControls(
    const shared_ptr<Engine::CModel>& pModel)
{
    ImGui::SeparatorText("Animation");
    if (nullptr == pModel || 0u == pModel->Get_NumAnimations())
    {
        ImGui::TextDisabled("Select a Character model with animations.");
        return;
    }
    uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
    const char* pCurrentName = pModel->Get_AnimationName(iCurrent);
    if (ImGui::BeginCombo("Animation Clip",
        nullptr != pCurrentName ? pCurrentName : "Invalid"))
    {
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            if (nullptr != pName && ImGui::Selectable(
                pName, iAnimation == iCurrent))
            {
                pModel->Start_Animation(iAnimation, true);
                pModel->Set_AnimPaused(false);
                iCurrent = iAnimation;
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button(pModel->Is_AnimPaused() ?
        "Play Animation" : "Pause Animation"))
    {
        pModel->Set_AnimPaused(!pModel->Is_AnimPaused());
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart Animation"))
        pModel->Set_AnimTrackPosition(iCurrent, 0.f);
    f32_t fPosition = 0.f;
    f32_t fDuration = 0.f;
    if (pModel->Get_AnimationProgress(iCurrent, fPosition, fDuration) &&
        fDuration > 0.f && ImGui::SliderFloat(
            "Animation Frame", &fPosition, 0.f, fDuration, "%.2f"))
    {
        pModel->Set_AnimPaused(true);
        pModel->Set_AnimTrackPosition(iCurrent, fPosition);
    }
}

void Client::CEffect_Tool::Update_Picking()
{
    if (!m_bPendingWorldPivotPick ||
        !ImGui::IsWindowHovered() ||
        ImGui::IsAnyItemHovered() ||
        !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;
    float4_t Picked{};
    if (!CGameInstance::Get().Picking(Picked))
    {
        m_strPreviewStatus = "No world surface was hit.";
        return;
    }
    m_vPickedWorldPosition = { Picked.x, Picked.y, Picked.z };
    XMStoreFloat4x4(&m_PreviewWorldRoot, XMMatrixTranslation(
        Picked.x, Picked.y, Picked.z));
    m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
    m_bPendingWorldPivotPick = false;
    m_strPreviewStatus = "World pivot committed from CGameInstance::Picking.";
}

void Client::CEffect_Tool::Render_EffectDetailWindow()
{
    ImGui::SetNextWindowPos(ImVec2(1110.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 660.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Effect Detail"))
    {
        ImGui::End();
        return;
    }
    const EFFECT_ELEMENT_DESC* pCurrent = Find_SelectedElement();
    if (nullptr == pCurrent || !m_ActiveDocument.has_value())
    {
        Reset_DetailDraft();
        ImGui::TextDisabled("Select one Element in All Effects.");
        ImGui::End();
        return;
    }
    if (!m_DetailDraft.has_value() ||
        m_strDetailDraftElementId != pCurrent->strElementId)
    {
        m_DetailDraft = *pCurrent;
        m_strDetailDraftElementId = pCurrent->strElementId;
        m_bDetailDraftDirty = false;
    }
    bool_t bChanged = false;
    Render_Detail(*m_DetailDraft, bChanged);
    if (bChanged)
        m_bDetailDraftDirty = true;
    ImGui::Separator();
    ImGui::BeginDisabled(!m_bDetailDraftDirty);
    if (ImGui::Button("Apply Detail"))
    {
        EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
        for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
        {
            if (Element.strElementId != m_strDetailDraftElementId)
                continue;
            Element.Detail = m_DetailDraft->Detail;
            Element.Material = m_DetailDraft->Material;
            break;
        }
        if (Try_CommitDocument(std::move(Staged)))
        {
            m_bDetailDraftDirty = false;
            if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
                m_DetailDraft = *pCommitted;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert Detail"))
    {
        m_DetailDraft = *pCurrent;
        m_bDetailDraftDirty = false;
    }
    ImGui::EndDisabled();
    if (m_bDetailDraftDirty)
        ImGui::TextDisabled("Detail draft is local until Apply Detail.");
    ImGui::End();
}

void Client::CEffect_Tool::Render_Detail(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    ImGui::Text("Element: %s (%s)",
        Element.strElementId.c_str(), Kind_Label(Element.eKind));
    Render_TransformDetail(Element.Detail, bChanged);
    Render_ColorDetail(Element.Detail, bChanged);
    Render_UVDetail(Element.Detail, bChanged);
    Render_UVKeyframes(Element, bChanged);
    Render_TimingDetail(Element.Detail, bChanged);
    Render_KindDetail(Element, bChanged);
    Render_LerpDetail(Element.Detail, bChanged);

    ImGui::SeparatorText("Pass Name");
    if (ImGui::BeginCombo("Render Profile",
        Profile_Label(Element.Material.eRenderProfile)))
    {
        for (int32_t iProfile = 0;
            iProfile < static_cast<int32_t>(EFFECT_RENDER_PROFILE::END);
            ++iProfile)
        {
            const EFFECT_RENDER_PROFILE eProfile =
                static_cast<EFFECT_RENDER_PROFILE>(iProfile);
            if (ImGui::Selectable(Profile_Label(eProfile),
                eProfile == Element.Material.eRenderProfile))
            {
                Element.Material.eRenderProfile = eProfile;
                bChanged = true;
            }
        }
        ImGui::EndCombo();
    }
}

void Client::CEffect_Tool::Render_TransformDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= InputFloat3("Position", Detail.Transform.vPosition);
    bChanged |= InputFloat3("Rotation (Degrees)",
        Detail.Transform.vRotationDegrees);
    bChanged |= InputFloat3("Revolution (Degrees/Second)",
        Detail.Transform.vRevolutionDegreesPerSecond);
    bChanged |= InputFloat3("Scaling", Detail.Transform.vScale);
    bChanged |= InputFloat3("Velocity", Detail.Transform.vVelocityPerSecond);
}

void Client::CEffect_Tool::Render_ColorDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= InputFloat4("Color Offset", Detail.Color.vColorOffset);
    bChanged |= InputFloat4("Color Multiply", Detail.Color.vColorMultiply);
    bChanged |= ImGui::SliderFloat("Color Clip",
        &Detail.Color.fColorClip, 0.f, 1.f);
    bChanged |= ImGui::InputFloat("Bloom Intensity",
        &Detail.Color.fEmissiveIntensity, 0.05f, 0.5f, "%.3f");
    bChanged |= ImGui::InputFloat("Distortion Intensity",
        &Detail.Color.fDistortionIntensity, 0.01f, 0.1f, "%.3f");
    bChanged |= ImGui::Checkbox("Distortion On Base Material",
        &Detail.Color.bDistortionOnBaseMaterial);
    bChanged |= ImGui::InputFloat("Radial Time",
        &Detail.Color.fRadialTime, 0.01f, 0.1f, "%.3f");
    bChanged |= ImGui::InputFloat("Radial Intensity",
        &Detail.Color.fRadialIntensity, 0.01f, 0.1f, "%.3f");
}

void Client::CEffect_Tool::Render_UVDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("UV", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= InputFloat2("UV Start", Detail.UV.vStart);
    bChanged |= InputFloat2("UV Speed", Detail.UV.vSpeed);
    bChanged |= ImGui::Checkbox("UV Wave", &Detail.UV.bWave);
    bChanged |= InputFloat2("Wave Amplitude", Detail.UV.vWaveAmplitude);
    bChanged |= ImGui::InputFloat("Wave Frequency",
        &Detail.UV.fWaveFrequency, 0.05f, 0.5f, "%.3f");
    bChanged |= ImGui::Checkbox("UV Sequence", &Detail.UV.bSequence);
    bChanged |= ImGui::Checkbox("UV Loop", &Detail.UV.bLoop);
    bChanged |= ImGui::InputFloat("Sequence Term",
        &Detail.UV.fSequenceTerm, 0.01f, 0.1f, "%.3f");
    bChanged |= ImGui::InputInt("UV Tile Columns", &Detail.UV.iTileColumns);
    bChanged |= ImGui::InputInt("UV Tile Rows", &Detail.UV.iTileRows);
    bChanged |= ImGui::InputInt("UV Tile Index", &Detail.UV.iTileIndex);
}

void Client::CEffect_Tool::Render_UVKeyframes(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    EFFECT_UV_DESC& UV = Element.Detail.UV;
    if (!UV.bSequence || UV.iTileColumns <= 0 || UV.iTileRows <= 0)
        return;
    const auto Base = std::find_if(
        Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
        [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.eSlot == EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
        });
    if (Base == Element.ResourceBindings.end())
    {
        ImGui::TextDisabled("Keyframes require a Base texture.");
        return;
    }
    const CEffectThumbnailCache::RESULT Thumbnail =
        m_pThumbnailCache->Request(Base->strAssetId);
    if (nullptr == Thumbnail.pTextureView)
    {
        ImGui::TextDisabled("Keyframe thumbnail is loading.");
        return;
    }
    ImGui::SeparatorText("Keyframes");
    const int64_t iTotal = static_cast<int64_t>(UV.iTileColumns) * UV.iTileRows;
    const int32_t iVisible = static_cast<int32_t>((std::min<int64_t>)(iTotal, 64));
    for (int32_t iTile = 0; iTile < iVisible; ++iTile)
    {
        if (0 != iTile % 8)
            ImGui::SameLine();
        ImGui::PushID(iTile);
        const int32_t iColumn = iTile % UV.iTileColumns;
        const int32_t iRow = iTile / UV.iTileColumns;
        const ImVec2 UV0(
            static_cast<float>(iColumn) / UV.iTileColumns,
            static_cast<float>(iRow) / UV.iTileRows);
        const ImVec2 UV1(
            static_cast<float>(iColumn + 1) / UV.iTileColumns,
            static_cast<float>(iRow + 1) / UV.iTileRows);
        const ImVec4 Border = iTile == UV.iTileIndex ?
            ImVec4(0.2f, 0.75f, 1.f, 1.f) : ImVec4(0.f, 0.f, 0.f, 0.f);
        ImGui::Image(Thumbnail.pTextureView, ImVec2(42.f, 42.f),
            UV0, UV1, ImVec4(1.f, 1.f, 1.f, 1.f), Border);
        if (ImGui::IsItemClicked())
        {
            UV.iTileIndex = iTile;
            bool_t bCommitted = false;
            if (m_ActiveDocument.has_value())
            {
                EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
                for (EFFECT_ELEMENT_DESC& Active : Staged.Elements)
                {
                    if (Active.strElementId != Element.strElementId)
                        continue;
                    Active.Detail.UV.iTileIndex = iTile;
                    bCommitted = Try_CommitDocument(std::move(Staged));
                    break;
                }
            }
            if (bCommitted)
            {
                m_fPreviewTimeSeconds = 0.f;
                if (const shared_ptr<CEffectObject> pObject =
                    m_pWorldPreviewObject.lock())
                    pObject->Set_SampleTime(0.f);
            }
            else
                bChanged = true;
        }
        ImGui::PopID();
    }
    if (iTotal > iVisible)
        ImGui::TextDisabled("Showing the first 64 sequence tiles.");
}

void Client::CEffect_Tool::Render_TimingDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= ImGui::InputFloat("Life Time",
        &Detail.Timing.fLifeTimeSeconds, 0.05f, 0.5f, "%.3f");
    bChanged |= ImGui::InputFloat("Start Delay Timer",
        &Detail.Timing.fStartDelaySeconds, 0.05f, 0.5f, "%.3f");
    bChanged |= ImGui::InputFloat("After Image Timer",
        &Detail.Timing.fAfterImageSeconds, 0.05f, 0.5f, "%.3f");
    bChanged |= ImGui::SliderFloat("Dissolve Start",
        &Detail.Timing.fDissolveStartNormalized, 0.f, 1.f);
}

void Client::CEffect_Tool::Render_KindDetail(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Type Detail",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    EFFECT_DETAIL_DESC& Detail = Element.Detail;
    switch (Element.eKind)
    {
    case EFFECT_ELEMENT_KIND::MESH:
        bChanged |= ImGui::Checkbox("Use Model Material",
            &Detail.Mesh.bUseModelMaterial);
        break;
    case EFFECT_ELEMENT_KIND::SPRITE:
        bChanged |= ImGui::Checkbox("Billboard",
            &Detail.Sprite.bBillboard);
        break;
    case EFFECT_ELEMENT_KIND::DECAL:
        bChanged |= InputFloat2("Decal Size", Detail.Decal.vSize);
        bChanged |= ImGui::InputFloat("Decal Projection Depth",
            &Detail.Decal.fDepth, 0.05f, 0.5f, "%.3f");
        break;
    case EFFECT_ELEMENT_KIND::PARTICLE:
        bChanged |= ImGui::InputScalar("Max Particles",
            ImGuiDataType_U32, &Detail.Particle.iMaxParticles);
        bChanged |= ImGui::InputFloat("Spawn Rate / Second",
            &Detail.Particle.fSpawnRatePerSecond, 1.f, 10.f, "%.3f");
        bChanged |= ImGui::InputScalar("Burst Count",
            ImGuiDataType_U32, &Detail.Particle.iBurstCount);
        bChanged |= ImGui::InputScalar("Random Seed",
            ImGuiDataType_U32, &Detail.Particle.iRandomSeed);
        bChanged |= InputFloat2("Particle Life Min/Max",
            Detail.Particle.vLifeTimeSeconds);
        bChanged |= InputFloat3("Initial Velocity Min",
            Detail.Particle.vInitialVelocityMin);
        bChanged |= InputFloat3("Initial Velocity Max",
            Detail.Particle.vInitialVelocityMax);
        bChanged |= InputFloat3("Acceleration",
            Detail.Particle.vAcceleration);
        bChanged |= InputFloat2("Start Size", Detail.Particle.vStartSize);
        bChanged |= InputFloat2("End Size", Detail.Particle.vEndSize);
        bChanged |= ImGui::Checkbox("Particle Local Space",
            &Detail.Particle.bLocalSpace);
        bChanged |= ImGui::Checkbox("Particle Billboard",
            &Detail.Particle.bBillboard);
        break;
    case EFFECT_ELEMENT_KIND::TRAIL:
        bChanged |= ImGui::InputScalar("Trail Max Points",
            ImGuiDataType_U32, &Detail.Trail.iMaxPoints);
        bChanged |= ImGui::InputFloat("Trail Point Life",
            &Detail.Trail.fPointLifeTimeSeconds, 0.01f, 0.1f, "%.3f");
        bChanged |= ImGui::InputFloat("Trail Sample Interval",
            &Detail.Trail.fSampleIntervalSeconds, 0.001f, 0.01f, "%.4f");
        bChanged |= ImGui::InputFloat("Trail Minimum Distance",
            &Detail.Trail.fMinimumDistance, 0.001f, 0.01f, "%.4f");
        bChanged |= ImGui::InputFloat("Trail Start Width",
            &Detail.Trail.fStartWidth, 0.01f, 0.1f, "%.3f");
        bChanged |= ImGui::InputFloat("Trail End Width",
            &Detail.Trail.fEndWidth, 0.01f, 0.1f, "%.3f");
        bChanged |= ImGui::Checkbox("Trail Faces Camera",
            &Detail.Trail.bFaceCamera);
        break;
    case EFFECT_ELEMENT_KIND::END:
    default:
        break;
    }
    if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
        Element.eKind == EFFECT_ELEMENT_KIND::SPRITE)
    {
        ImGui::SeparatorText("After Image");
        bChanged |= ImGui::InputFloat("AfterImage Sample Interval",
            &Detail.AfterImage.fSampleIntervalSeconds,
            0.01f, 0.1f, "%.3f");
        bChanged |= ImGui::InputScalar("AfterImage Max Copies",
            ImGuiDataType_U32, &Detail.AfterImage.iMaxCopies);
        bChanged |= ImGui::InputFloat("AfterImage Alpha Exponent",
            &Detail.AfterImage.fAlphaExponent, 0.05f, 0.5f, "%.3f");
    }
}

void Client::CEffect_Tool::Render_LerpDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Linear Lerp",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    EFFECT_LINEAR_LERP_DESC& Lerp = Detail.LinearLerp;
    bChanged |= ImGui::Checkbox("Lerp Position", &Lerp.bPosition);
    if (Lerp.bPosition)
        bChanged |= InputFloat3("Position End", Lerp.vEndPosition);
    bChanged |= ImGui::Checkbox("Lerp Rotation", &Lerp.bRotation);
    if (Lerp.bRotation)
        bChanged |= InputFloat3("Rotation End", Lerp.vEndRotationDegrees);
    bChanged |= ImGui::Checkbox("Lerp Revolution", &Lerp.bRevolution);
    if (Lerp.bRevolution)
        bChanged |= InputFloat3("Revolution End",
            Lerp.vEndRevolutionDegreesPerSecond);
    bChanged |= ImGui::Checkbox("Lerp Scaling", &Lerp.bScale);
    if (Lerp.bScale)
        bChanged |= InputFloat3("Scaling End", Lerp.vEndScale);
    bChanged |= ImGui::Checkbox("Lerp Velocity", &Lerp.bVelocity);
    if (Lerp.bVelocity)
        bChanged |= InputFloat3("Velocity End", Lerp.vEndVelocityPerSecond);
    bChanged |= ImGui::Checkbox("Lerp ColorOffset", &Lerp.bColorOffset);
    if (Lerp.bColorOffset)
        bChanged |= InputFloat4("ColorOffset End", Lerp.vEndColorOffset);
    bChanged |= ImGui::Checkbox("Lerp Color Multiply", &Lerp.bColorMultiply);
    if (Lerp.bColorMultiply)
        bChanged |= InputFloat4("Color Multiply End",
            Lerp.vEndColorMultiply);
    bChanged |= ImGui::Checkbox("Lerp Bloom Intensity",
        &Lerp.bEmissiveIntensity);
    if (Lerp.bEmissiveIntensity)
        bChanged |= ImGui::InputFloat("Bloom Intensity End",
            &Lerp.fEndEmissiveIntensity, 0.05f, 0.5f, "%.3f");
}

void Client::CEffect_Tool::Render_AllEffectsWindow()
{
    ImGui::SetNextWindowPos(ImVec2(1110.f, 705.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 260.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("All Effects"))
    {
        ImGui::End();
        return;
    }
    ImGui::SeparatorText("Published Effects");
    if (ImGui::Button("Reload Published Effects"))
    {
        std::string Status;
        if (!CEffectCatalog::Load(Status))
            m_strElementStatus = Status;
        else
            m_strElementStatus = "Reloaded the runtime Effect catalog.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Selected Effect") &&
        !m_strSelectedDataFileAssetId.empty())
        Try_LoadDocument(m_strSelectedDataFileAssetId);
    ImGui::BeginChild("PublishedEffectList", ImVec2(0.f, 58.f), true);
    for (const std::string& EffectAssetId :
        CEffectCatalog::Get_EffectAssetIds())
    {
        if (ImGui::Selectable(EffectAssetId.c_str(),
            EffectAssetId == m_strSelectedDataFileAssetId))
        {
            m_strSelectedDataFileAssetId = EffectAssetId;
            Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), EffectAssetId);
        }
    }
    ImGui::EndChild();
    ImGui::SeparatorText("Active Effect Elements");
    ImGui::InputText("Element ID", m_NewElementId.data(),
        m_NewElementId.size());
    if (ImGui::Button("Add Element"))
        Try_AddElement();
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
        Try_DeleteSelectedElement();
    ImGui::SameLine();
    if (ImGui::Button("Clear All"))
        ImGui::OpenPopup("Confirm Clear All Elements");
    if (ImGui::BeginPopupModal(
        "Confirm Clear All Elements", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        const size_t elementCount = m_ActiveDocument.has_value() ?
            m_ActiveDocument->Elements.size() : 0u;
        ImGui::Text("Delete all %zu Elements from the active draft?", elementCount);
        if (ImGui::Button("Clear All Elements"))
        {
            if (Try_ClearElements())
                ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (m_ActiveDocument.has_value())
    {
        for (const EFFECT_ELEMENT_DESC& Element :
            m_ActiveDocument->Elements)
        {
            const bool_t bSelected =
                Element.strElementId == m_strSelectedElementId;
            const std::string Label = Element.strElementId + "  [" +
                Kind_Label(Element.eKind) + "]";
            if (ImGui::Selectable(Label.c_str(), bSelected))
            {
                if (!bSelected && m_bDetailDraftDirty)
                {
                    m_strElementStatus =
                        "Apply Detail or Revert Detail before selecting another Element.";
                    continue;
                }
                m_strSelectedElementId = Element.strElementId;
                m_eSelectedEffectType = Element.eKind;
                m_eSelectedResourceSlot =
                    EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
                    EFFECT_RESOURCE_SLOT::MESH_MODEL :
                    EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
            }
        }
    }
    if (!m_strElementStatus.empty())
        ImGui::TextWrapped("%s", m_strElementStatus.c_str());
    ImGui::End();
}

void Client::CEffect_Tool::Render_DataFilesWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10.f, 705.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1090.f, 260.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Data Files"))
    {
        ImGui::End();
        return;
    }
    ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
        m_NewAssetId.size());
    ImGui::InputText("Display Name", m_NewDisplayName.data(),
        m_NewDisplayName.size());
    if (ImGui::Button("Create Document"))
        Try_CreateDocument();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Try_SaveDocument();
    ImGui::SameLine();
    if (ImGui::Button("Load Selected") &&
        !m_strSelectedDataFileAssetId.empty())
        Try_LoadDocument(m_strSelectedDataFileAssetId);
    ImGui::SameLine();
    if (ImGui::Button("Discard"))
    {
        if (Has_UnsavedWork())
            m_bDiscardConfirmationRequested = true;
        else
            Discard_ActiveDocument();
    }
    if (m_bDiscardConfirmationRequested)
    {
        ImGui::OpenPopup("Discard unsaved Effect changes?");
        m_bDiscardConfirmationRequested = false;
    }
    if (ImGui::BeginPopupModal(
        "Discard unsaved Effect changes?", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(
            "The active Document and unapplied Detail draft will be lost.");
        if (ImGui::Button("Discard Changes"))
        {
            Discard_ActiveDocument();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (m_ActiveDocument.has_value())
    {
        ImGui::Text("Active: %s | %s | Elements %zu%s",
            m_ActiveDocument->strEffectAssetId.c_str(),
            m_ActiveDocument->strDisplayName.c_str(),
            m_ActiveDocument->Elements.size(),
            Has_UnsavedWork() ? " | DIRTY" : "");
    }

    const std::filesystem::path Directory =
        CProjectDataRoot::Resolve(L"Effects/Authored");
    std::error_code Error;
    std::vector<std::string> AssetIds;
    if (std::filesystem::is_directory(Directory, Error) && !Error)
    {
        for (const std::filesystem::directory_entry& Entry :
            std::filesystem::directory_iterator(Directory, Error))
        {
            if (!Entry.is_regular_file())
                continue;
            const std::string Name = Entry.path().filename().string();
            constexpr std::string_view Suffix = ".effect.json";
            if (Name.size() > Suffix.size() &&
                Name.ends_with(Suffix))
                AssetIds.push_back(Name.substr(0u, Name.size() - Suffix.size()));
        }
    }
    std::sort(AssetIds.begin(), AssetIds.end());
    ImGui::BeginChild("EffectDataFileList", ImVec2(0.f, 110.f), true);
    for (const std::string& AssetId : AssetIds)
    {
        if (ImGui::Selectable(AssetId.c_str(),
            AssetId == m_strSelectedDataFileAssetId))
        {
            m_strSelectedDataFileAssetId = AssetId;
            Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), AssetId);
        }
    }
    ImGui::EndChild();
    if (!m_strDocumentStatus.empty())
        ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
    ImGui::End();
}

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
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Document, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    m_ActiveDocument = std::move(Document);
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_bDocumentDirty = true;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration();
    Stage_WorldPreview();
    m_strDocumentStatus = "Created a v5 Effect Document in memory.";
    return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
    if (m_bDetailDraftDirty)
    {
        m_strElementStatus =
            "Apply Detail or Revert Detail before adding an Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value())
    {
        m_strElementStatus = "Create or load a Document first.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    EFFECT_ELEMENT_DESC Element;
    Element.strElementId = m_NewElementId.data();
    Element.eKind = m_eSelectedEffectType;
    Element.Material.eRenderProfile =
        EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
    if (Element.strElementId.empty())
    {
        Element.strElementId = "element_" +
            std::to_string(Staged.Elements.size() + 1u);
        Copy_Buffer(m_NewElementId.data(), m_NewElementId.size(),
            Element.strElementId);
    }
    Staged.Elements.push_back(Element);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strSelectedElementId = Element.strElementId;
    m_eSelectedResourceSlot =
        EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
        EFFECT_RESOURCE_SLOT::MESH_MODEL :
        EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
    m_strElementStatus = "Added one typed visual layer.";
    return true;
}

bool_t Client::CEffect_Tool::Try_DeleteSelectedElement()
{
    if (m_bDetailDraftDirty)
    {
        m_strElementStatus =
            "Apply Detail or Revert Detail before deleting an Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty())
    {
        m_strElementStatus = "Select one Element to delete.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    const auto NewEnd = std::remove_if(
        Staged.Elements.begin(), Staged.Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    if (NewEnd == Staged.Elements.end())
        return false;
    Staged.Elements.erase(NewEnd, Staged.Elements.end());
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strElementStatus = "Deleted the selected Element.";
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearElements()
{
    if (m_bDetailDraftDirty)
    {
        m_strElementStatus =
            "Apply Detail or Revert Detail before clearing Elements.";
        return false;
    }
    if (!m_ActiveDocument.has_value())
        return false;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.Elements.clear();
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strElementStatus = "Cleared all visual layers.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
    if (!m_ActiveDocument.has_value())
    {
        m_strDocumentStatus = "There is no active Document to save.";
        return false;
    }
    if (m_bDetailDraftDirty)
    {
        m_strDocumentStatus =
            "Apply Detail or Revert Detail before saving the Document.";
        return false;
    }
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(
            m_ActiveDocument->strEffectAssetId).wstring() +
            L".effect.json"));
    std::string Error;
    if (Path.empty() || !CEffectDocumentCodec::Save_Atomic(
        Path, *m_ActiveDocument, Error))
    {
        m_strDocumentStatus = Path.empty() ?
            "Effect authoring path escaped Data/Effects/Authored." : Error;
        return false;
    }
    m_bDocumentDirty = false;
    m_strSelectedDataFileAssetId =
        m_ActiveDocument->strEffectAssetId;
    m_strDocumentStatus = "Saved atomically: " + Path.string();
    return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument(
    const std::string& strAssetId)
{
    if (Has_UnsavedWork())
    {
        m_strDocumentStatus =
            "Save or explicitly discard the active Effect changes before loading.";
        return false;
    }
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strAssetId).wstring() + L".effect.json"));
    EFFECT_DOCUMENT_DESC Staged;
    std::string Error;
    if (Path.empty() || !CEffectDocumentCodec::Load(Path, Staged, Error))
    {
        m_strDocumentStatus = Path.empty() ?
            "Effect load path escaped Data/Effects/Authored." : Error;
        return false;
    }
    std::string PreviewStatus;
    const bool_t bDrawable =
        CEffectDocumentCodec::Validate_Drawable(Staged, PreviewStatus);
    bool_t bPreviewStaged = false;
    if (bDrawable)
    {
        bPreviewStaged = Stage_WorldPreview(Staged);
        if (!bPreviewStaged)
        {
            m_strDocumentStatus =
                "Load rejected; active Document and preview were preserved: " +
                m_strPreviewStatus;
            return false;
        }
    }
    else
        Release_WorldPreview(true);
    m_ActiveDocument = std::move(Staged);
    Reset_DetailDraft();
    m_strSelectedElementId = m_ActiveDocument->Elements.empty() ?
        std::string{} : m_ActiveDocument->Elements.front().strElementId;
    m_strSelectedDataFileAssetId = strAssetId;
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
        m_ActiveDocument->strEffectAssetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    m_bDocumentDirty = false;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration();
    if (const shared_ptr<CEffectObject> pObject =
        m_pWorldPreviewObject.lock())
        pObject->Set_SampleTime(0.f);
    m_strDocumentStatus = bPreviewStaged ?
        "Loaded and committed: " + Path.string() :
        "Loaded editable draft; preview is hidden until required resources bind: " +
            (PreviewStatus.empty() ? m_strPreviewStatus : PreviewStatus);
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
        m_strResourceStatus = "Resources/Effect is missing.";
        return false;
    }
    vector<EFFECT_RESOURCE_CATALOG_ENTRY> Staged;
    for (std::filesystem::recursive_directory_iterator Iterator(
        EffectRoot,
        std::filesystem::directory_options::skip_permission_denied,
        Error), End; Iterator != End; Iterator.increment(Error))
    {
        if (Error)
        {
            Error.clear();
            continue;
        }
        if (!Iterator->is_regular_file())
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
        if (!Relative.empty())
            Staged.push_back({ Relative.generic_string(), eKind });
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
    m_ResourceCatalog = std::move(Staged);
    m_pThumbnailCache->Invalidate(m_iFrameNumber);
    m_strResourceStatus = "Catalog refreshed: " +
        std::to_string(m_ResourceCatalog.size()) +
        " supported Resources/Effect files.";
    return true;
}

bool_t Client::CEffect_Tool::Try_BindResource(
    const std::string& strAssetId)
{
    if (!m_ActiveDocument.has_value() || nullptr == Find_SelectedElement())
    {
        m_strResourceStatus = "Select an Element before choosing a resource.";
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
    if (nullptr == pElement ||
        !Slot_Allowed(pElement->eKind, m_eSelectedResourceSlot))
    {
        m_strResourceStatus = "That resource slot is not allowed for this Element.";
        return false;
    }
    auto Iterator = std::find_if(
        pElement->ResourceBindings.begin(), pElement->ResourceBindings.end(),
        [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.eSlot == m_eSelectedResourceSlot;
        });
    if (Iterator == pElement->ResourceBindings.end())
        pElement->ResourceBindings.push_back(
            { m_eSelectedResourceSlot, strAssetId });
    else
        Iterator->strAssetId = strAssetId;
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strResourceStatus = "Bound " + strAssetId + " to " +
        Slot_Label(m_eSelectedResourceSlot) + ".";
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedSlot()
{
    if (!m_ActiveDocument.has_value() || nullptr == Find_SelectedElement())
        return false;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
        if (Element.strElementId != m_strSelectedElementId)
            continue;
        std::erase_if(Element.ResourceBindings,
            [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.eSlot == m_eSelectedResourceSlot;
            });
        break;
    }
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strResourceStatus = "Cleared the selected resource slot.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CommitDocument(
    EFFECT_DOCUMENT_DESC&& Staged)
{
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
        m_bDocumentDirty = true;
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
    m_bDocumentDirty = true;
    Recalculate_PreviewDuration();
    return true;
}

bool_t Client::CEffect_Tool::Ensure_WorldPreviewObject()
{
    if (nullptr != m_pWorldPreviewObject.lock())
        return true;
    const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
    CEffectObject::EFFECT_OBJECT_DESC Desc{};
    Desc.pDocument = nullptr;
    Desc.RootWorld = m_PreviewWorldRoot;
    Desc.bAutoPlay = false;
    shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
        iLevel, PREVIEW_LAYER, &Desc, &pGameObject)))
    {
        m_strPreviewStatus =
            "EffectObject prototype is not registered for world preview.";
        return false;
    }
    const shared_ptr<CEffectObject> pEffect =
        dynamic_pointer_cast<CEffectObject>(pGameObject);
    if (nullptr == pEffect)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevel, PREVIEW_LAYER, pGameObject);
        m_strPreviewStatus = "World preview clone returned the wrong type.";
        return false;
    }
    m_pWorldPreviewObject = pEffect;
    m_iWorldPreviewLevel = iLevel;
    return true;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview()
{
    return m_ActiveDocument.has_value() ?
        Stage_WorldPreview(*m_ActiveDocument) : false;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview(
    const EFFECT_DOCUMENT_DESC& Document)
{
    if (!Ensure_WorldPreviewObject())
        return false;
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    std::string Error;
    if (nullptr == pObject || !pObject->Stage_Document(
        Document, Error))
    {
        m_strPreviewStatus = "Document is editable but not drawable yet: " + Error;
        return false;
    }
    pObject->Set_Playing(false);
    pObject->Set_SampleTime(m_fPreviewTimeSeconds);
    m_strPreviewStatus = "World preview committed from the active Document.";
    return true;
}

bool_t Client::CEffect_Tool::Resolve_PreviewRoot(float4x4_t& OutRoot)
{
    switch (m_ePreviewPivotKind)
    {
    case EFFECT_PREVIEW_PIVOT_KIND::WORLD:
        OutRoot = m_PreviewWorldRoot;
        return true;
    case EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT:
        return CAnimationTargetService::Resolve_RootTransform(&OutRoot);
    case EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET:
    case EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE:
        return CAnimationTargetService::Resolve_AnchorTransform(
            m_strPreviewAnchorSlotId.c_str(), &OutRoot);
    case EFFECT_PREVIEW_PIVOT_KIND::END:
    default:
        return false;
    }
}

void Client::CEffect_Tool::Release_WorldPreview(
    const bool_t bRemoveFromLayer)
{
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (bRemoveFromLayer && nullptr != pObject &&
        m_iWorldPreviewLevel == CGameInstance::Get().Get_CurrentLevelID())
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            m_iWorldPreviewLevel, PREVIEW_LAYER, pObject);
    }
    m_pWorldPreviewObject.reset();
    m_iWorldPreviewLevel = UINT32_MAX;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
    m_ActiveDocument.reset();
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strSelectedResourceAssetId.clear();
    m_bDocumentDirty = false;
    m_bPreviewPlaying = false;
    m_fPreviewTimeSeconds = 0.f;
    Release_WorldPreview(true);
    m_strDocumentStatus = "Discarded the in-memory Effect Document.";
}

void Client::CEffect_Tool::Reset_DetailDraft()
{
    m_DetailDraft.reset();
    m_strDetailDraftElementId.clear();
    m_bDetailDraftDirty = false;
}

void Client::CEffect_Tool::Recalculate_PreviewDuration()
{
    m_fPreviewDurationSeconds = 1.f;
    if (!m_ActiveDocument.has_value())
        return;
    for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
    {
        const EFFECT_TIMING_DESC& Timing = Element.Detail.Timing;
        f32_t fElementTail = 0.f;
        if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
            fElementTail = Element.Detail.Particle.vLifeTimeSeconds.y;
        else if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
            fElementTail = Element.Detail.Trail.fPointLifeTimeSeconds;
        m_fPreviewDurationSeconds = (std::max)(m_fPreviewDurationSeconds,
            Timing.fStartDelaySeconds + Timing.fLifeTimeSeconds +
            Timing.fAfterImageSeconds + fElementTail);
    }
    m_fPreviewTimeSeconds = std::clamp(
        m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
}

bool_t Client::CEffect_Tool::Has_UnsavedWork() const
{
    return m_bDocumentDirty || m_bDetailDraftDirty;
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
