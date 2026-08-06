#include "imgui.h"

#include "Effect_Tool.h"

#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "EffectAuthoringTransfer.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_ThumbnailCache.h"
#include "GameInstance.h"
#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"
#include "Model.h"
#include "Profiler.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <limits>
#include <map>
#include <set>
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

    const char* Class_Label(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "Lance Master";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "Dimension Master";
        case CHARACTER_CLASS_ID::END:
        default: return "Invalid";
        }
    }

    const char* Resource_DomainId(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
        case CHARACTER_CLASS_ID::END:
        default: return nullptr;
        }
    }

    std::string EffectAsset_DomainId(const std::string& strEffectAssetId)
    {
        constexpr std::pair<std::string_view, std::string_view> Domains[] =
        {
            { "effect.lancemaster.", "LanceMaster" },
            { "effect.gunslinger.", "Gunslinger" },
            { "effect.slayer.", "Slayer" },
            { "effect.artist.", "Artist" },
            { "effect.dimensionmaster.", "DimensionMaster" },
            { "effect.warlord.", "Warlord" },
            { "effect.valtan.", "Valtan" }
        };
        for (const auto& [Prefix, DomainId] : Domains)
        {
            if (strEffectAssetId.starts_with(Prefix))
                return std::string(DomainId);
        }
        return "Uncategorized";
    }

    std::string First_PathComponent(const std::filesystem::path& Relative)
    {
        const auto Iterator = Relative.begin();
        if (Iterator == Relative.end())
            return {};
        return Iterator->generic_string();
    }

    const char* Animation_AssetName(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "GunSlinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
        case CHARACTER_CLASS_ID::END:
        default: return nullptr;
        }
    }

    std::vector<std::string> Collect_AnimationClipNames(
        const std::shared_ptr<Engine::CModel>& pModel)
    {
        std::vector<std::string> Clips;
        if (nullptr == pModel)
            return Clips;
        Clips.reserve(pModel->Get_NumAnimations());
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            if (nullptr != pName)
                Clips.emplace_back(pName);
        }
        return Clips;
    }

    const char* Source_Label(const Client::EFFECT_DOCUMENT_SOURCE eSource)
    {
        switch (eSource)
        {
        case Client::EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT: return "New";
        case Client::EFFECT_DOCUMENT_SOURCE::AUTHORED: return "Authored";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED: return "Imported";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE:
            return "Imported Draft";
        case Client::EFFECT_DOCUMENT_SOURCE::END:
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

    std::string Default_SlotId(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        return Client::EFFECT_ELEMENT_KIND::MESH == eKind ?
            std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID) :
            std::string(Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    }

    const Client::EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        const auto Iterator = std::find_if(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [strSlotId](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == strSlotId;
            });
        return Iterator == Element.ResourceBindings.end() ?
            nullptr : &*Iterator;
    }

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind;
        return nullptr != Client::Find_EffectMaterialInput(
            Element.Material.strTemplateId, strSlotId);
    }

    Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
        const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
            Client::Find_EffectMaterialInput(
                Element.Material.strTemplateId, strSlotId);
        return nullptr == pInput ? Client::EFFECT_RESOURCE_FILE_KIND::END :
            pInput->eAllowedResourceKind;
    }

    std::string Slot_Label(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return "Mesh Shape";
        const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
            Client::Find_EffectMaterialInput(
                Element.Material.strTemplateId, strSlotId);
        return nullptr == pInput ? std::string(strSlotId) :
            std::string(pInput->strDisplayName);
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

    bool DragFloat2(
        const char* Label,
        float2_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f")
    {
        return ImGui::DragFloat2(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    bool DragFloat3(
        const char* Label,
        float3_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f")
    {
        return ImGui::DragFloat3(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    bool DragFloat4(
        const char* Label,
        float4_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f")
    {
        return ImGui::DragFloat4(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
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
    Engine::CProfilerScope Profile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.Render");
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
    ImGui::TextUnformatted(
        "Edit the selected Authored Effect with typed visual Elements.");
    const ImGuiIO& IO = ImGui::GetIO();
    ImGui::TextDisabled("FPS %.1f | Frame %.2f ms",
        IO.Framerate,
        IO.DeltaTime > 0.f ? IO.DeltaTime * 1000.f : 0.f);
    Render_EffectTypeSelector();
    if (ImGui::Button("Restart Preview"))
        Start_WorldPreviewFromBeginning();
    ImGui::SameLine();
    if (ImGui::Button("Refresh Resources"))
        Refresh_ResourceCatalog();
    ImGui::TextDisabled(
        "Load existing data in All Effects or Data Files; Add Element is authoring only.");
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
            m_strSelectedResourceSlotId = Default_SlotId(eKind);
        }
    }
}

void Client::CEffect_Tool::Render_ResourceSlots()
{
    const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
    ImGui::SeparatorText("Selected Element Resources");
    if (nullptr == pElement)
    {
        ImGui::TextDisabled("Select an Element in All Effects.");
        return;
    }

    ImGui::Text("%s | %s", pElement->strDisplayName.c_str(),
        Kind_Label(pElement->eKind));
    const auto RenderSlotCard = [this, pElement](
        const std::string_view strSlotId,
        const std::string_view strLabel)
    {
        ImGui::PushID(strSlotId.data());
        ImGui::BeginGroup();
        const EFFECT_RESOURCE_BINDING_DESC* pBinding =
            Find_Binding(*pElement, strSlotId);
        const EFFECT_RESOURCE_FILE_KIND eFileKind =
            Slot_FileKind(*pElement, strSlotId);
        bool_t bClicked = false;
        if (nullptr != pBinding)
        {
            const CEffectThumbnailCache::RESULT Thumbnail =
                m_pThumbnailCache->Request(pBinding->strAssetId, eFileKind);
            if (nullptr != Thumbnail.pTextureView)
            {
                ImGui::Image(Thumbnail.pTextureView, ImVec2(64.f, 58.f));
                bClicked = ImGui::IsItemClicked();
            }
            else
            {
                bClicked = ImGui::Button(
                    EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind ?
                        "Mesh" : "DDS", ImVec2(64.f, 58.f));
                if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
                    ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
            }
        }
        else
        {
            bClicked = ImGui::Button("Empty", ImVec2(64.f, 58.f));
        }
        if (m_strSelectedResourceSlotId == strSlotId)
        {
            ImGui::GetWindowDrawList()->AddRect(
                ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 2.f);
        }
        if (bClicked)
        {
            m_strSelectedResourceSlotId = strSlotId;
            m_strSelectedResourceAssetId.clear();
        }
        ImGui::TextUnformatted(std::string(strLabel).c_str());
        if (nullptr != pBinding)
        {
            std::string Name = std::filesystem::path(
                pBinding->strAssetId).filename().string();
            if (Name.size() > 9u)
                Name = Name.substr(0u, 7u) + "..";
            ImGui::TextDisabled("%s", Name.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", pBinding->strAssetId.c_str());
        }
        ImGui::EndGroup();
        ImGui::PopID();
    };

    if (EFFECT_ELEMENT_KIND::MESH == pElement->eKind)
    {
        ImGui::SeparatorText("Mesh Shape");
        RenderSlotCard(EFFECT_MESH_SHAPE_SLOT_ID, "Mesh Shape");
    }
    ImGui::SeparatorText("Material Inputs");
    ImGui::TextDisabled("Template: %s",
        pElement->Material.strTemplateId.c_str());
    const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
        Find_EffectMaterialTemplate(pElement->Material.strTemplateId);
    if (nullptr == pTemplate)
    {
        ImGui::TextDisabled("Unknown Material Template.");
        return;
    }
    for (size_t iInput = 0u; iInput < pTemplate->iInputCount; ++iInput)
    {
        if (0u != iInput)
            ImGui::SameLine();
        RenderSlotCard(
            pTemplate->pInputs[iInput].strSlotId,
            pTemplate->pInputs[iInput].strDisplayName);
    }
}

void Client::CEffect_Tool::Render_ResourceGrid()
{
    Engine::CProfilerScope Profile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.ResourceGrid");
    if (!m_bResourceCatalogRefreshAttempted)
        Refresh_ResourceCatalog();
    const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
    if (nullptr == pElement ||
        !Slot_Allowed(*pElement, m_strSelectedResourceSlotId))
        return;
    const EFFECT_RESOURCE_FILE_KIND eWanted = Slot_FileKind(
        *pElement, m_strSelectedResourceSlotId);
    const std::string Filter = m_ResourceFilter.data();
    std::string BoundAssetId;
    if (const EFFECT_RESOURCE_BINDING_DESC* pBinding = Find_Binding(
        *pElement, m_strSelectedResourceSlotId))
        BoundAssetId = pBinding->strAssetId;

    ImGui::SeparatorText("Resource Browser");
    if (ImGui::BeginCombo("Authoring Category##ResourceDomain",
        m_strSelectedAuthoringDomainId.c_str()))
    {
        for (const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain : m_ResourceDomains)
        {
            if (ImGui::Selectable(Domain.strDomainId.c_str(),
                Domain.strDomainId == m_strSelectedAuthoringDomainId))
            {
                Select_AuthoringDomain(Domain.strDomainId);
            }
        }
        ImGui::EndCombo();
    }
    const auto DomainIterator = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [this](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
        {
            return Domain.strDomainId == m_strSelectedAuthoringDomainId;
        });
    if (DomainIterator == m_ResourceDomains.end())
    {
        ImGui::TextDisabled(
            "The selected authoring category has no Resources/Effect folder.");
        return;
    }

    const size_t iFileKind = static_cast<size_t>(eWanted);
    if (iFileKind >= DomainIterator->Categories.size())
        return;
    const vector<string>& Categories = DomainIterator->Categories[iFileKind];
    std::string Category = m_ResourceCategory.data();
    if (Category.empty() ||
        std::find(Categories.begin(), Categories.end(), Category) ==
            Categories.end())
    {
        Category = "All";
        Copy_Buffer(m_ResourceCategory.data(),
            m_ResourceCategory.size(), Category);
    }
    if (ImGui::BeginCombo("Resource Folder", Category.c_str()))
    {
        for (const std::string& Candidate : Categories)
        {
            if (ImGui::Selectable(Candidate.c_str(), Candidate == Category))
            {
                Category = Candidate;
                Copy_Buffer(m_ResourceCategory.data(),
                    m_ResourceCategory.size(), Category);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::TextDisabled("Binding: %s / %s",
        Slot_Label(*pElement, m_strSelectedResourceSlotId).c_str(),
        EFFECT_RESOURCE_FILE_KIND::MODEL == eWanted ? "WModel" : "DDS");
    ImGui::SameLine();
    ImGui::TextDisabled("| %s: %zu candidates",
        m_strSelectedAuthoringDomainId.c_str(),
        DomainIterator->ResourceCounts[iFileKind]);
    ImGui::BeginDisabled(m_strSelectedResourceAssetId.empty());
    if (ImGui::Button("Bind Selected"))
        Try_BindResource(m_strSelectedResourceAssetId);
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Clear Slot"))
        Try_ClearSelectedSlot();

    const float CardWidth = 92.f;
    const int32_t Columns = (std::max)(1,
        static_cast<int32_t>(ImGui::GetContentRegionAvail().x / CardWidth));
    Rebuild_ResourceBrowserView(eWanted, Filter,
        m_strSelectedAuthoringDomainId, Category);
    const int32_t Rows = static_cast<int32_t>(
        (m_VisibleResourceIndices.size() + Columns - 1u) / Columns);
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
                if (iEntry >= m_VisibleResourceIndices.size())
                    break;
                const EFFECT_RESOURCE_CATALOG_ENTRY& Entry =
                    m_ResourceCatalog[m_VisibleResourceIndices[iEntry]];
                ImGui::PushID(Entry.strAssetId.c_str());
                if (0 != iColumn)
                    ImGui::SameLine();
                ImGui::BeginGroup();
                bool_t bClicked = false;
                const CEffectThumbnailCache::RESULT Thumbnail =
                    m_pThumbnailCache->Request(
                        Entry.strAssetId, Entry.eFileKind);
                if (nullptr != Thumbnail.pTextureView)
                {
                    ImGui::Image(Thumbnail.pTextureView, ImVec2(80.f, 80.f));
                    bClicked = ImGui::IsItemClicked();
                }
                else
                {
                    bClicked = ImGui::Button(
                        EFFECT_RESOURCE_FILE_KIND::MODEL == Entry.eFileKind ?
                            "Mesh" : "DDS", ImVec2(80.f, 80.f));
                    if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
                        ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
                }
                if (Entry.strAssetId == BoundAssetId)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                        ImGui::GetColorU32(ImGuiCol_HeaderActive),
                        2.f, 0, 3.f);
                }
                else if (Entry.strAssetId == m_strSelectedResourceAssetId)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                        ImGui::GetColorU32(ImGuiCol_NavHighlight),
                        2.f, 0, 2.f);
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
                    m_strSelectedResourceAssetId = Entry.strAssetId;
                ImGui::PopID();
            }
        }
    }
}

void Client::CEffect_Tool::Rebuild_ResourceBrowserView(
    const EFFECT_RESOURCE_FILE_KIND eFileKind,
    const std::string& strFilter,
    const std::string& strDomainId,
    const std::string& strCategory)
{
    if (m_iResourceViewRevision == m_iResourceCatalogRevision &&
        m_eResourceViewFileKind == eFileKind &&
        m_strResourceViewFilter == strFilter &&
        m_strResourceViewDomainId == strDomainId &&
        m_strResourceViewCategory == strCategory)
    {
        return;
    }

    vector<size_t> Staged;
    Staged.reserve(m_ResourceCatalog.size());
    for (size_t iEntry = 0u; iEntry < m_ResourceCatalog.size(); ++iEntry)
    {
        const EFFECT_RESOURCE_CATALOG_ENTRY& Entry =
            m_ResourceCatalog[iEntry];
        if (Entry.eFileKind != eFileKind ||
            Entry.strDomainId != strDomainId ||
            !Contains_NoCase(Entry.strAssetId, strFilter) ||
            (strCategory != "All" && Entry.strCategory != strCategory))
        {
            continue;
        }
        Staged.push_back(iEntry);
    }

    m_VisibleResourceIndices = std::move(Staged);
    m_iResourceViewRevision = m_iResourceCatalogRevision;
    m_eResourceViewFileKind = eFileKind;
    m_strResourceViewFilter = strFilter;
    m_strResourceViewDomainId = strDomainId;
    m_strResourceViewCategory = strCategory;
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
    m_pCharacterPreviewPanel->Render_Selector(false, {}, false);
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
    const shared_ptr<const EFFECT_DOCUMENT_DESC> pRuntimeDocument =
        m_ActiveDocument.has_value() ?
        CEffectCatalog::Find(m_ActiveDocument->strEffectAssetId) : nullptr;
    const bool_t bRuntimeMatchesActive =
        nullptr != pRuntimeDocument && m_ActiveDocument.has_value() &&
        CEffectDocumentCodec::Serialize(*pRuntimeDocument) ==
            CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    const bool_t bCanTransfer = m_ActiveDocument.has_value() &&
        !Has_UnsavedWork() &&
        bRuntimeMatchesActive &&
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
            "Save the exact runtime-admitted Effect, then choose Player/Weapon/Bone pivot.");
    }

    ImGui::SeparatorText("Effect Preview");
    const auto SelectPreviewFilter = [this](
        const char* pLabel,
        const EFFECT_PREVIEW_FILTER eFilter)
    {
        if (ImGui::RadioButton(pLabel, m_ePreviewFilter == eFilter))
        {
            m_ePreviewFilter = eFilter;
            Stage_WorldPreview();
        }
    };
    SelectPreviewFilter("Complete Effect", EFFECT_PREVIEW_FILTER::COMPLETE);
    ImGui::SameLine();
    SelectPreviewFilter("Solo Element", EFFECT_PREVIEW_FILTER::SOLO_SELECTED);
    ImGui::SameLine();
    SelectPreviewFilter("Mute Element", EFFECT_PREVIEW_FILTER::MUTE_SELECTED);
    if (EFFECT_PREVIEW_FILTER::COMPLETE != m_ePreviewFilter &&
        m_strSelectedElementId.empty())
        ImGui::TextDisabled("Select an Element for Solo/Mute preview.");

    ImGui::SeparatorText("Timeline");
    if (ImGui::Button(m_bPreviewPlaying ? "Pause" : "Play"))
    {
        if (m_bPreviewPlaying)
            m_bPreviewPlaying = false;
        else if (nullptr != m_pWorldPreviewObject.lock())
        {
            if (m_fPreviewTimeSeconds >= m_fPreviewDurationSeconds)
                Start_WorldPreviewFromBeginning();
            else
                m_bPreviewPlaying = true;
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &m_bPreviewLoop);
    ImGui::SameLine();
    if (ImGui::Button("Restart + Play"))
        Start_WorldPreviewFromBeginning();
    ImGui::Text("World Preview: %s | %.3f / %.3f s",
        m_bPreviewPlaying ? "PLAYING" : "PAUSED",
        m_fPreviewTimeSeconds, m_fPreviewDurationSeconds);
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
    if (!m_strPreviewAnimationStatus.empty())
        ImGui::TextWrapped("%s", m_strPreviewAnimationStatus.c_str());
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
    ImGui::TextDisabled(
        "Drag numeric values for live world preview; Apply commits the draft.");
    Render_Detail(*m_DetailDraft, bChanged);
    if (bChanged)
    {
        m_bDetailDraftDirty = true;
        Stage_DetailDraftPreview();
    }
    ImGui::Separator();
    ImGui::BeginDisabled(!m_bDetailDraftDirty);
    if (ImGui::Button("Apply Detail"))
    {
        EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
        for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
        {
            if (Element.strElementId != m_strDetailDraftElementId)
                continue;
            Element.strDisplayName = m_DetailDraft->strDisplayName;
            Element.strGroupId = m_DetailDraft->strGroupId;
            Element.strSourceNode = m_DetailDraft->strSourceNode;
            Element.bVisible = m_DetailDraft->bVisible;
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
        Recalculate_PreviewDuration();
        if (Stage_WorldPreview())
            m_strPreviewStatus =
                "Detail draft reverted to the active Document preview.";
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
    ImGui::Text("Display Name: %s", Element.strDisplayName.c_str());
    ImGui::TextDisabled("Group: %s",
        Element.strGroupId.empty() ? "(none)" : Element.strGroupId.c_str());
    ImGui::TextDisabled("Source Node: %s",
        Element.strSourceNode.empty() ? "(authored)" :
            Element.strSourceNode.c_str());
    bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
    ImGui::TextDisabled("Material Template: %s",
        Element.Material.strTemplateId.c_str());
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
    bChanged |= DragFloat3(
        "Position", Detail.Transform.vPosition, 0.01f, -1000.f, 1000.f);
    bChanged |= DragFloat3("Rotation (Degrees)",
        Detail.Transform.vRotationDegrees, 0.25f, -360.f, 360.f);
    bChanged |= DragFloat3("Revolution (Degrees/Second)",
        Detail.Transform.vRevolutionDegreesPerSecond,
        0.5f, -3600.f, 3600.f);
    bChanged |= DragFloat3(
        "Scaling", Detail.Transform.vScale, 0.01f, 0.001f, 100.f);
    bChanged |= DragFloat3("Velocity",
        Detail.Transform.vVelocityPerSecond, 0.01f, -1000.f, 1000.f);
}

void Client::CEffect_Tool::Render_ColorDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= DragFloat4(
        "Color Offset", Detail.Color.vColorOffset, 0.01f, -10.f, 10.f);
    bChanged |= DragFloat4(
        "Color Multiply", Detail.Color.vColorMultiply, 0.01f, 0.f, 10.f);
    bChanged |= ImGui::SliderFloat("Color Clip",
        &Detail.Color.fColorClip, 0.f, 1.f);
    bChanged |= ImGui::DragFloat("Bloom Intensity",
        &Detail.Color.fEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Distortion Intensity",
        &Detail.Color.fDistortionIntensity, 0.01f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::Checkbox("Distortion On Base Material",
        &Detail.Color.bDistortionOnBaseMaterial);
    bChanged |= ImGui::DragFloat("Radial Time",
        &Detail.Color.fRadialTime, 0.01f, -100.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Radial Intensity",
        &Detail.Color.fRadialIntensity, 0.01f, -100.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
}

void Client::CEffect_Tool::Render_UVDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("UV", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= DragFloat2(
        "UV Start", Detail.UV.vStart, 0.001f, -100.f, 100.f);
    bChanged |= DragFloat2(
        "UV Speed", Detail.UV.vSpeed, 0.001f, -100.f, 100.f);
    bChanged |= ImGui::Checkbox("UV Wave", &Detail.UV.bWave);
    bChanged |= DragFloat2("Wave Amplitude",
        Detail.UV.vWaveAmplitude, 0.001f, -100.f, 100.f);
    bChanged |= ImGui::DragFloat("Wave Frequency",
        &Detail.UV.fWaveFrequency, 0.01f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::Checkbox("UV Sequence", &Detail.UV.bSequence);
    bChanged |= ImGui::Checkbox("UV Loop", &Detail.UV.bLoop);
    bChanged |= ImGui::DragFloat("Sequence Term",
        &Detail.UV.fSequenceTerm, 0.001f, 0.001f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bool_t bTileChanged = ImGui::DragInt(
        "UV Tile Columns", &Detail.UV.iTileColumns, 0.1f, 1, 64, "%d",
        ImGuiSliderFlags_AlwaysClamp);
    bTileChanged |= ImGui::DragInt(
        "UV Tile Rows", &Detail.UV.iTileRows, 0.1f, 1, 64, "%d",
        ImGuiSliderFlags_AlwaysClamp);
    const int32_t iMaximumTile = (std::max)(
        0, Detail.UV.iTileColumns * Detail.UV.iTileRows - 1);
    bTileChanged |= ImGui::DragInt(
        "UV Tile Index", &Detail.UV.iTileIndex, 0.1f, 0, iMaximumTile,
        "%d", ImGuiSliderFlags_AlwaysClamp);
    Detail.UV.iTileIndex = std::clamp(
        Detail.UV.iTileIndex, 0, iMaximumTile);
    bChanged |= bTileChanged;
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
            return Binding.strSlotId == "base";
        });
    if (Base == Element.ResourceBindings.end())
    {
        ImGui::TextDisabled("Keyframes require a Base texture.");
        return;
    }
    const CEffectThumbnailCache::RESULT Thumbnail =
        m_pThumbnailCache->Request(
            Base->strAssetId, EFFECT_RESOURCE_FILE_KIND::TEXTURE);
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
    bChanged |= ImGui::DragFloat("Life Time",
        &Detail.Timing.fLifeTimeSeconds, 0.01f, 0.001f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Start Delay Timer",
        &Detail.Timing.fStartDelaySeconds, 0.01f, 0.f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("After Image Timer",
        &Detail.Timing.fAfterImageSeconds, 0.01f, 0.f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
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
        bChanged |= DragFloat2(
            "Decal Size", Detail.Decal.vSize, 0.01f, 0.001f, 1000.f);
        bChanged |= ImGui::DragFloat("Decal Projection Depth",
            &Detail.Decal.fDepth, 0.01f, 0.001f, 1000.f, "%.3f",
            ImGuiSliderFlags_AlwaysClamp);
        break;
    case EFFECT_ELEMENT_KIND::PARTICLE:
    {
        bChanged |= ImGui::InputScalar("Max Particles",
            ImGuiDataType_U32, &Detail.Particle.iMaxParticles);
        bChanged |= ImGui::DragFloat("Spawn Rate / Second",
            &Detail.Particle.fSpawnRatePerSecond, 1.f, 0.f, 2048.f, "%.3f",
            ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::InputScalar("Burst Count",
            ImGuiDataType_U32, &Detail.Particle.iBurstCount);
        bChanged |= ImGui::InputScalar("Random Seed",
            ImGuiDataType_U32, &Detail.Particle.iRandomSeed);
		if (DragFloat2("Particle Life Min/Max",
            Detail.Particle.vLifeTimeSeconds, 0.01f, 0.001f, 30.f))
        {
            Detail.Particle.vLifeTimeSeconds.y = (std::max)(
                Detail.Particle.vLifeTimeSeconds.x,
                Detail.Particle.vLifeTimeSeconds.y);
			bChanged = true;
		}
		const bool_t bPositionMinChanged = DragFloat3(
			"Initial Position Min", Detail.Particle.vInitialPositionMin,
			0.01f, -1000.f, 1000.f);
		const bool_t bPositionMaxChanged = DragFloat3(
			"Initial Position Max", Detail.Particle.vInitialPositionMax,
			0.01f, -1000.f, 1000.f);
		if (bPositionMinChanged || bPositionMaxChanged)
		{
			Detail.Particle.vInitialPositionMax.x = (std::max)(
				Detail.Particle.vInitialPositionMin.x,
				Detail.Particle.vInitialPositionMax.x);
			Detail.Particle.vInitialPositionMax.y = (std::max)(
				Detail.Particle.vInitialPositionMin.y,
				Detail.Particle.vInitialPositionMax.y);
			Detail.Particle.vInitialPositionMax.z = (std::max)(
				Detail.Particle.vInitialPositionMin.z,
				Detail.Particle.vInitialPositionMax.z);
			bChanged = true;
		}
		const bool_t bVelocityMinChanged = DragFloat3("Initial Velocity Min",
            Detail.Particle.vInitialVelocityMin, 0.01f, -1000.f, 1000.f);
        const bool_t bVelocityMaxChanged = DragFloat3("Initial Velocity Max",
            Detail.Particle.vInitialVelocityMax, 0.01f, -1000.f, 1000.f);
        if (bVelocityMinChanged || bVelocityMaxChanged)
        {
            Detail.Particle.vInitialVelocityMax.x = (std::max)(
                Detail.Particle.vInitialVelocityMin.x,
                Detail.Particle.vInitialVelocityMax.x);
            Detail.Particle.vInitialVelocityMax.y = (std::max)(
                Detail.Particle.vInitialVelocityMin.y,
                Detail.Particle.vInitialVelocityMax.y);
            Detail.Particle.vInitialVelocityMax.z = (std::max)(
                Detail.Particle.vInitialVelocityMin.z,
                Detail.Particle.vInitialVelocityMax.z);
            bChanged = true;
        }
        bChanged |= DragFloat3("Acceleration",
            Detail.Particle.vAcceleration, 0.01f, -1000.f, 1000.f);
        bChanged |= DragFloat2(
            "Start Size", Detail.Particle.vStartSize, 0.01f, 0.001f, 100.f);
        bChanged |= DragFloat2(
            "End Size", Detail.Particle.vEndSize, 0.01f, 0.f, 100.f);
        bChanged |= ImGui::Checkbox("Particle Local Space",
            &Detail.Particle.bLocalSpace);
        bChanged |= ImGui::Checkbox("Particle Billboard",
            &Detail.Particle.bBillboard);
        break;
    }
    case EFFECT_ELEMENT_KIND::TRAIL:
        bChanged |= ImGui::InputScalar("Trail Max Points",
            ImGuiDataType_U32, &Detail.Trail.iMaxPoints);
        bChanged |= ImGui::DragFloat("Trail Point Life",
            &Detail.Trail.fPointLifeTimeSeconds,
            0.01f, 0.001f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Sample Interval",
            &Detail.Trail.fSampleIntervalSeconds,
            0.001f, 0.001f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Minimum Distance",
            &Detail.Trail.fMinimumDistance,
            0.001f, 0.f, 100.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Start Width",
            &Detail.Trail.fStartWidth,
            0.01f, 0.001f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail End Width",
            &Detail.Trail.fEndWidth,
            0.01f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
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
        bChanged |= ImGui::DragFloat("AfterImage Sample Interval",
            &Detail.AfterImage.fSampleIntervalSeconds,
            0.001f, 0.001f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::InputScalar("AfterImage Max Copies",
            ImGuiDataType_U32, &Detail.AfterImage.iMaxCopies);
        bChanged |= ImGui::DragFloat("AfterImage Alpha Exponent",
            &Detail.AfterImage.fAlphaExponent,
            0.01f, 0.001f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
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
        bChanged |= DragFloat3(
            "Position End", Lerp.vEndPosition, 0.01f, -1000.f, 1000.f);
    bChanged |= ImGui::Checkbox("Lerp Rotation", &Lerp.bRotation);
    if (Lerp.bRotation)
        bChanged |= DragFloat3(
            "Rotation End", Lerp.vEndRotationDegrees, 0.25f, -360.f, 360.f);
    bChanged |= ImGui::Checkbox("Lerp Revolution", &Lerp.bRevolution);
    if (Lerp.bRevolution)
        bChanged |= DragFloat3("Revolution End",
            Lerp.vEndRevolutionDegreesPerSecond, 0.5f, -3600.f, 3600.f);
    bChanged |= ImGui::Checkbox("Lerp Scaling", &Lerp.bScale);
    if (Lerp.bScale)
        bChanged |= DragFloat3(
            "Scaling End", Lerp.vEndScale, 0.01f, 0.001f, 100.f);
    bChanged |= ImGui::Checkbox("Lerp Velocity", &Lerp.bVelocity);
    if (Lerp.bVelocity)
        bChanged |= DragFloat3("Velocity End",
            Lerp.vEndVelocityPerSecond, 0.01f, -1000.f, 1000.f);
    bChanged |= ImGui::Checkbox("Lerp ColorOffset", &Lerp.bColorOffset);
    if (Lerp.bColorOffset)
        bChanged |= DragFloat4(
            "ColorOffset End", Lerp.vEndColorOffset, 0.01f, -10.f, 10.f);
    bChanged |= ImGui::Checkbox("Lerp Color Multiply", &Lerp.bColorMultiply);
    if (Lerp.bColorMultiply)
        bChanged |= DragFloat4("Color Multiply End",
            Lerp.vEndColorMultiply, 0.01f, 0.f, 10.f);
    bChanged |= ImGui::Checkbox("Lerp Bloom Intensity",
        &Lerp.bEmissiveIntensity);
    if (Lerp.bEmissiveIntensity)
        bChanged |= ImGui::DragFloat("Bloom Intensity End",
            &Lerp.fEndEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
            ImGuiSliderFlags_AlwaysClamp);
}

void Client::CEffect_Tool::Render_AllEffectsWindow()
{
    ImGui::SetNextWindowPos(ImVec2(1110.f, 705.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 420.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(360.f, 380.f), ImVec2(2000.f, 2000.f));
    if (!ImGui::Begin("All Effects"))
    {
        ImGui::End();
        return;
    }
    if (!m_bAllEffectsRefreshAttempted)
        Refresh_AllEffects();
    constexpr LostArk::Shared::CHARACTER_CLASS_ID Classes[] = {
        LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
        LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
        LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER };
    if (ImGui::BeginCombo("Class", Class_Label(m_eAllEffectsClass)))
    {
        for (const auto eClass : Classes)
        {
            if (ImGui::Selectable(Class_Label(eClass),
                eClass == m_eAllEffectsClass))
            {
                m_eAllEffectsClass = eClass;
                Select_AuthoringDomainForClass(eClass);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::InputText("Search", m_AllEffectsSearch.data(),
        m_AllEffectsSearch.size());
    ImGui::TextDisabled(
        "Click a skill row to load/restart its Complete Effect; Element rows edit one layer.");
    if (ImGui::Button("Refresh All Effects"))
        Refresh_AllEffects();
    ImGui::SameLine();
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
    const f32_t fStatusReserve = m_strElementStatus.empty() ? 1.f :
        ImGui::CalcTextSize(
            m_strElementStatus.c_str(), nullptr, false,
            ImGui::GetContentRegionAvail().x).y +
            ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild(
        "AllEffectsTree", ImVec2(0.f, -fStatusReserve), true);
    const std::string Search = m_AllEffectsSearch.data();
    bool_t bActiveAppearsInTree = false;
    for (const EFFECT_SKILL_TREE_ENTRY& Entry : m_AllEffects)
    {
        if (Entry.Skill.eCharacterClass != m_eAllEffectsClass ||
            (!Contains_NoCase(Entry.Skill.strInputSlot, Search) &&
             !Contains_NoCase(Entry.Skill.strDisplayName, Search) &&
             !Contains_NoCase(Entry.Skill.strEffectId, Search) &&
             !Contains_NoCase(Entry.Document.strDisplayName, Search)))
            continue;
        if (m_ActiveDocument.has_value() &&
            m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId)
            bActiveAppearsInTree = true;
        const EFFECT_DOCUMENT_DESC& TreeDocument =
            m_ActiveDocument.has_value() &&
            m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId ?
            *m_ActiveDocument : Entry.Document;
        ImGui::PushID(static_cast<int32_t>(Entry.Skill.iSkillId));
        const std::string SkillLabel = Entry.Skill.strInputSlot + " | " +
            Entry.Skill.strDisplayName + " (" + Entry.Skill.strEffectId + ")";
        const bool_t bActiveSkill = m_ActiveDocument.has_value() &&
            m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId;
        const ImGuiTreeNodeFlags SkillFlags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnArrow |
            (bActiveSkill ? ImGuiTreeNodeFlags_Selected : 0);
        const bool_t bSkillOpen = ImGui::TreeNodeEx(
            SkillLabel.c_str(), SkillFlags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            Try_SelectSkill(Entry.Skill.strEffectId);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(
                "%zu Elements in one complete Effect Document.\n"
                "Click the skill label to load or restart all of them.",
                TreeDocument.Elements.size());
        }
        if (bSkillOpen)
        {
            for (int32_t iKind = 0;
                iKind < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END);
                ++iKind)
            {
                const EFFECT_ELEMENT_KIND eKind =
                    static_cast<EFFECT_ELEMENT_KIND>(iKind);
                const size_t iKindCount = static_cast<size_t>(std::count_if(
                    TreeDocument.Elements.begin(), TreeDocument.Elements.end(),
                    [eKind](const EFFECT_ELEMENT_DESC& Element)
                    {
                        return Element.eKind == eKind;
                    }));
                const std::string KindLabel = std::string(Kind_Label(eKind)) +
                    " (" + std::to_string(iKindCount) + ")";
                if (0u == iKindCount || !ImGui::TreeNode(KindLabel.c_str()))
                    continue;
                for (const EFFECT_ELEMENT_DESC& Element : TreeDocument.Elements)
                {
                    if (Element.eKind != eKind)
                        continue;
                    const bool_t bSelected = m_ActiveDocument.has_value() &&
                        m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId &&
                        Element.strElementId == m_strSelectedElementId;
                    std::string Label = Element.strDisplayName + "##" +
                        Element.strElementId;
                    if (!Element.strGroupId.empty())
                        Label = "[" + Element.strGroupId + "] " + Label;
                    if (ImGui::Selectable(Label.c_str(), bSelected))
                        Try_SelectElement(
                            Entry.Skill.strEffectId, Element.strElementId);
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    if (m_ActiveDocument.has_value() && !bActiveAppearsInTree &&
        ImGui::TreeNode("Active / Imported Draft"))
    {
        for (int32_t iKind = 0;
            iKind < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iKind)
        {
            const EFFECT_ELEMENT_KIND eKind =
                static_cast<EFFECT_ELEMENT_KIND>(iKind);
            const size_t iKindCount = static_cast<size_t>(std::count_if(
                m_ActiveDocument->Elements.begin(),
                m_ActiveDocument->Elements.end(),
                [eKind](const EFFECT_ELEMENT_DESC& Element)
                {
                    return Element.eKind == eKind;
                }));
            const std::string KindLabel = std::string(Kind_Label(eKind)) +
                " (" + std::to_string(iKindCount) + ")";
            if (0u == iKindCount || !ImGui::TreeNode(KindLabel.c_str()))
                continue;
            for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
            {
                if (Element.eKind != eKind)
                    continue;
                if (ImGui::Selectable(Element.strDisplayName.c_str(),
                    Element.strElementId == m_strSelectedElementId))
                    Try_SelectElement(
                        m_ActiveDocument->strEffectAssetId,
                        Element.strElementId);
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::EndChild();
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
    if (!m_bDataFilesRefreshAttempted)
        Refresh_DataFiles();
    if (ImGui::BeginCombo("Authoring Category##DataFilesDomain",
        m_strSelectedAuthoringDomainId.c_str()))
    {
        for (const std::string& DomainId : m_DataFileDomains)
        {
            if (ImGui::Selectable(DomainId.c_str(),
                DomainId == m_strSelectedAuthoringDomainId))
            {
                Select_AuthoringDomain(DomainId);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
        m_NewAssetId.size());
    ImGui::InputText("Display Name", m_NewDisplayName.data(),
        m_NewDisplayName.size());
    if (ImGui::Button("New"))
        Try_CreateDocument();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Try_SaveDocument();
    ImGui::SameLine();
    if (ImGui::Button("Save As"))
        Try_SaveDocumentAs(m_NewAssetId.data());
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
        Try_ReloadActiveDocument();
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
        ImGui::Text("Active: %s | %s | %s | Elements %zu%s",
            m_ActiveDocument->strEffectAssetId.c_str(),
            m_ActiveDocument->strDisplayName.c_str(),
            Source_Label(m_eActiveDocumentSource),
            m_ActiveDocument->Elements.size(),
            Has_UnsavedWork() ? " | DIRTY" : "");
    }
    if (ImGui::Button("Refresh Files"))
        Refresh_DataFiles();
    ImGui::SameLine();
    const auto SelectedDataFile = std::find_if(
        m_DataFiles.begin(), m_DataFiles.end(),
        [this](const EFFECT_DATA_FILE_ENTRY& Entry)
        {
            return Entry.strAssetId == m_strSelectedDataFileAssetId;
        });
    const bool_t bSelectedFileLoadable =
        SelectedDataFile != m_DataFiles.end() &&
        EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE !=
            SelectedDataFile->eSource;
    ImGui::BeginDisabled(!bSelectedFileLoadable);
    if (ImGui::Button("Load Selected"))
    {
        if (SelectedDataFile != m_DataFiles.end())
            Try_LoadDocumentPath(
                SelectedDataFile->Path,
                SelectedDataFile->eSource,
                SelectedDataFile->strAssetId);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled(bSelectedFileLoadable ?
        "loads and immediately plays existing data" :
        "Imported Draft rows are extraction reference only");
    const size_t iVisibleDataFiles = static_cast<size_t>(std::count_if(
        m_DataFiles.begin(), m_DataFiles.end(),
        [this](const EFFECT_DATA_FILE_ENTRY& Entry)
        {
            return Entry.strDomainId == m_strSelectedAuthoringDomainId;
        }));
    ImGui::TextDisabled("%s: %zu data files",
        m_strSelectedAuthoringDomainId.c_str(), iVisibleDataFiles);
    ImGui::BeginChild("EffectDataFileList", ImVec2(0.f, 110.f), true);
    for (const EFFECT_DATA_FILE_ENTRY& Entry : m_DataFiles)
    {
        if (Entry.strDomainId != m_strSelectedAuthoringDomainId)
            continue;
        const std::string Label = std::string("[") +
            Source_Label(Entry.eSource) + "] " + Entry.strAssetId;
        if (ImGui::Selectable(Label.c_str(),
            Entry.strAssetId == m_strSelectedDataFileAssetId))
        {
            m_strSelectedDataFileAssetId = Entry.strAssetId;
            Select_AuthoringDomain(Entry.strDomainId);
            if (EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE == Entry.eSource)
            {
                m_strDocumentStatus =
                    "Selected extraction draft is not a runtime Effect "
                    "Document. Use the matching Authored row for playback.";
            }
            else
            {
                Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
                    Entry.strAssetId);
            }
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
    m_ActiveDocument = std::move(Document);
    m_ActiveDocumentPath.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_bDocumentDirty = true;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration();
    Stage_WorldPreview();
    m_strDocumentStatus = "Created a v6 Effect Document in memory.";
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
    Element.strDisplayName = Element.strElementId;
    Element.eKind = m_eSelectedEffectType;
    Element.Material.eRenderProfile =
        EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
    if (Element.strElementId.empty())
    {
        Element.strElementId = "element_" +
            std::to_string(Staged.Elements.size() + 1u);
        Element.strDisplayName = Element.strElementId;
        Copy_Buffer(m_NewElementId.data(), m_NewElementId.size(),
            Element.strElementId);
    }
    Staged.Elements.push_back(Element);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strSelectedElementId = Element.strElementId;
    m_strSelectedResourceSlotId = Default_SlotId(Element.eKind);
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
    if (EFFECT_DOCUMENT_SOURCE::IMPORTED == m_eActiveDocumentSource)
    {
        m_strDocumentStatus =
            "Imported Effect must use Save As to create a unique Authored ID.";
        return false;
    }
    if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource &&
        std::filesystem::is_regular_file(Path))
    {
        m_strDocumentStatus =
            "Save refuses to replace an existing Authored file from New; use Save As.";
        return false;
    }
    std::string Error;
    if (Path.empty() || !CEffectDocumentCodec::Save_Atomic(
        Path, *m_ActiveDocument, Error))
    {
        m_strDocumentStatus = Path.empty() ?
            "Effect authoring path escaped Data/Effects/Authored." : Error;
        return false;
    }
    m_bDocumentDirty = false;
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
    m_strSelectedDataFileAssetId =
        m_ActiveDocument->strEffectAssetId;
    m_strDocumentStatus = "Saved atomically: " + Path.string();
    Refresh_DataFiles();
    Refresh_AllEffects();
    return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocumentAs(
    const std::string& strAssetId)
{
    if (!m_ActiveDocument.has_value())
    {
        m_strDocumentStatus = "There is no active Document to save.";
        return false;
    }
    if (m_bDetailDraftDirty)
    {
        m_strDocumentStatus =
            "Apply Detail or Revert Detail before Save As.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.strEffectAssetId = strAssetId;
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
    if (!CEffectDocumentCodec::Save_Atomic(Path, Staged, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    m_ActiveDocument = std::move(Staged);
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
    m_bDocumentDirty = false;
    m_strSelectedDataFileAssetId = strAssetId;
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), strAssetId);
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strDocumentStatus = "Saved new Authored Effect atomically: " +
        Path.string();
    return true;
}

bool_t Client::CEffect_Tool::Try_ReloadActiveDocument()
{
    if (!m_ActiveDocument.has_value() || m_ActiveDocumentPath.empty())
    {
        m_strDocumentStatus = "The active Effect has no saved source to reload.";
        return false;
    }
    if (Has_UnsavedWork())
    {
        m_strDocumentStatus =
            "Save or Discard changes before Reload.";
        return false;
    }
    return Try_LoadDocumentPath(
        m_ActiveDocumentPath, m_eActiveDocumentSource,
        m_ActiveDocument->strEffectAssetId);
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
    const std::string& strSelectionId)
{
    if (EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE == eSource)
    {
        m_strDocumentStatus =
            "Extraction drafts are reference-only and cannot be played until "
            "they are converted to a validated Effect Document.";
        return false;
    }
    if (Has_UnsavedWork())
    {
        m_strDocumentStatus =
            "Save or explicitly discard the active Effect changes before loading.";
        return false;
    }
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
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = eSource;
    Reset_DetailDraft();
    m_strSelectedElementId = m_ActiveDocument->Elements.empty() ?
        std::string{} : m_ActiveDocument->Elements.front().strElementId;
    if (const EFFECT_ELEMENT_DESC* pSelected = Find_SelectedElement())
    {
        m_eSelectedEffectType = pSelected->eKind;
        m_strSelectedResourceSlotId = Default_SlotId(pSelected->eKind);
    }
    m_strSelectedDataFileAssetId = strSelectionId;
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
        m_ActiveDocument->strEffectAssetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    m_bDocumentDirty = false;
    m_fPreviewTimeSeconds = 0.f;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
    Synchronize_LoadedSkillPreview();
    float4x4_t TargetRoot{};
    if (bPreviewStaged &&
        CAnimationTargetService::Resolve_RootTransform(&TargetRoot))
    {
        m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
        if (const shared_ptr<CEffectObject> pObject =
            m_pWorldPreviewObject.lock())
        {
            pObject->Set_RootWorld(TargetRoot);
            pObject->Set_Visible(true);
        }
    }
    if (bPreviewStaged)
    {
        Stage_WorldPreview();
        Start_WorldPreviewFromBeginning();
    }
    else
        m_bPreviewPlaying = false;
    m_strDocumentStatus = bPreviewStaged ?
        "Loaded existing Effect and started the world preview: " +
            Path.string() :
        "Loaded editable draft; preview is hidden until required resources bind: " +
            (PreviewStatus.empty() ? m_strPreviewStatus : PreviewStatus);
    return true;
}

bool_t Client::CEffect_Tool::Refresh_AllEffects()
{
    m_bAllEffectsRefreshAttempted = true;
    std::string CatalogStatus;
    if (!CPlayerSkillCatalog::Load(CatalogStatus))
    {
        m_strElementStatus =
            "All Effects refresh preserved the previous tree: " +
            CatalogStatus;
        return false;
    }

    vector<EFFECT_SKILL_TREE_ENTRY> Staged;
    size_t iMissingAuthored = 0u;
    for (const PLAYER_SKILL_DEFINITION& Skill :
        CPlayerSkillCatalog::Get_Skills())
    {
        if (Skill.strEffectId.empty())
            continue;
        const std::filesystem::path Path = CProjectDataRoot::Resolve(
            std::filesystem::path(L"Effects") / L"Authored" /
            (std::filesystem::path(Skill.strEffectId).wstring() +
                L".effect.json"));
        if (Path.empty() || !std::filesystem::is_regular_file(Path))
        {
            ++iMissingAuthored;
            continue;
        }
        EFFECT_DOCUMENT_DESC Document;
        std::string Error;
        if (!CEffectDocumentCodec::Load(Path, Document, Error) ||
            Document.strEffectAssetId != Skill.strEffectId)
        {
            m_strElementStatus =
                "All Effects refresh preserved the previous tree; invalid " +
                Skill.strEffectId + ": " +
                (Error.empty() ? "file ID does not match effectId." : Error);
            return false;
        }
        Staged.push_back({ Skill, std::move(Document) });
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
    m_strElementStatus = "All Effects refreshed from PlayerSkills + Authored: " +
        std::to_string(m_AllEffects.size()) + " complete skills";
    if (0u != iMissingAuthored)
        m_strElementStatus += ", " + std::to_string(iMissingAuthored) +
            " mappings have no Authored document";
    m_strElementStatus += ".";
    return true;
}

bool_t Client::CEffect_Tool::Refresh_DataFiles()
{
    m_bDataFilesRefreshAttempted = true;
    vector<EFFECT_DATA_FILE_ENTRY> Staged;
    std::set<std::string> AssetIds;
    std::set<std::string> StagedDomainIds;
    for (const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain : m_ResourceDomains)
        StagedDomainIds.insert(Domain.strDomainId);

    std::map<std::string, std::string> SkillDomains;
    std::string SkillCatalogStatus;
    if (CPlayerSkillCatalog::Load(SkillCatalogStatus))
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
        &ResolveDocumentDomain](
        const std::filesystem::path& RelativeRoot,
        const EFFECT_DOCUMENT_SOURCE eSource) -> bool_t
    {
        const std::filesystem::path Root =
            CProjectDataRoot::Resolve(RelativeRoot);
        std::error_code Error;
        if (Root.empty() || !std::filesystem::exists(Root, Error))
            return true;
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
                m_strDocumentStatus =
                    "Data Files scan failed; previous list preserved.";
                return false;
            }
            if (!Iterator->is_regular_file())
                continue;
            const std::string Name = Iterator->path().filename().string();
            if (!Name.ends_with(".effect.json"))
                continue;
            EFFECT_DOCUMENT_DESC Document;
            std::string CodecError;
            if (!CEffectDocumentCodec::Load(
                Iterator->path(), Document, CodecError))
            {
                m_strDocumentStatus =
                    "Data Files refresh preserved the previous list: " +
                    Iterator->path().string() + ": " + CodecError;
                return false;
            }
            if (!AssetIds.insert(Document.strEffectAssetId).second)
            {
                m_strDocumentStatus =
                    "Data Files refresh rejected duplicate Effect ID: " +
                    Document.strEffectAssetId;
                return false;
            }
            const std::filesystem::path RelativePath =
                Iterator->path().lexically_relative(Root);
            const std::string DomainId = ResolveDocumentDomain(
                Document.strEffectAssetId, RelativePath, eSource);
            StagedDomainIds.insert(DomainId);
            Staged.push_back({
                Document.strEffectAssetId, DomainId,
                Iterator->path(), eSource });
        }
        return true;
    };
    if (!Scan(L"Effects/Authored", EFFECT_DOCUMENT_SOURCE::AUTHORED) ||
        !Scan(L"Effects/Imported", EFFECT_DOCUMENT_SOURCE::IMPORTED))
        return false;

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
    m_DataFiles = std::move(Staged);
    m_DataFileDomains.assign(
        StagedDomainIds.begin(), StagedDomainIds.end());
    if (m_DataFileDomains.end() == std::find(
        m_DataFileDomains.begin(), m_DataFileDomains.end(),
        m_strSelectedAuthoringDomainId) && !m_DataFileDomains.empty())
    {
        Select_AuthoringDomain(m_DataFileDomains.front());
    }
    m_strDocumentStatus = "Data Files refreshed: " +
        std::to_string(m_DataFiles.size() - iImportedReferenceCount) +
        " validated documents, " +
        std::to_string(iImportedReferenceCount) +
        " reference-only extraction drafts.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectSkill(
    const std::string& strEffectAssetId)
{
    if (!m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId)
    {
        return Try_LoadDocument(strEffectAssetId);
    }

    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
    Synchronize_LoadedSkillPreview();
    if (!Stage_WorldPreview())
    {
        m_strElementStatus =
            "Complete Effect preview could not be restarted: " +
            m_strPreviewStatus;
        return false;
    }
    Start_WorldPreviewFromBeginning();
    m_strElementStatus = m_bDetailDraftDirty ?
        "Complete Effect restarted from the active Document; Apply Detail to include the pending draft." :
        "Complete Effect restarted from All Effects.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectElement(
    const std::string& strEffectAssetId,
    const std::string& strElementId)
{
    const bool_t bChangesSelection =
        !m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
        (!strElementId.empty() &&
            m_strSelectedElementId != strElementId);
    if (bChangesSelection && m_bDetailDraftDirty)
    {
        m_strElementStatus =
            "Apply Detail or Revert Detail before selecting another Element.";
        return false;
    }
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
    Reset_DetailDraft();
    m_strSelectedElementId = strElementId;
    m_eSelectedEffectType = Iterator->eKind;
    m_strSelectedResourceSlotId = Default_SlotId(Iterator->eKind);
    m_strSelectedResourceAssetId.clear();
    if (EFFECT_PREVIEW_FILTER::COMPLETE != m_ePreviewFilter)
        Stage_WorldPreview();
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
        const std::filesystem::path EffectRelative =
            Iterator->path().lexically_relative(EffectRoot);
        if (Relative.empty() || EffectRelative.empty() ||
            EffectRelative.parent_path().empty())
            continue;
        const std::string DomainId = First_PathComponent(EffectRelative);
        if (DomainId.empty())
            continue;
        const std::filesystem::path DomainRelative =
            EffectRelative.lexically_relative(std::filesystem::path(DomainId));
        const std::filesystem::path CategoryPath = DomainRelative.parent_path();
        const string Category = CategoryPath.empty() ?
            "Root" : CategoryPath.generic_string();
        Staged.push_back({
            Relative.generic_string(), DomainId, Category, eKind });
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

    std::map<std::string, array<std::set<string>,
        static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)>>
        StagedCategorySets;
    std::map<std::string, array<size_t,
        static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)>>
        StagedResourceCounts;
    for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry : Staged)
    {
        const size_t iKind = static_cast<size_t>(Entry.eFileKind);
        StagedCategorySets[Entry.strDomainId][iKind].insert("All");
        StagedCategorySets[Entry.strDomainId][iKind].insert(Entry.strCategory);
        ++StagedResourceCounts[Entry.strDomainId][iKind];
    }

    vector<EFFECT_RESOURCE_DOMAIN_CATALOG> StagedDomains;
    StagedDomains.reserve(StagedCategorySets.size());
    for (const auto& [DomainId, CategorySets] : StagedCategorySets)
    {
        EFFECT_RESOURCE_DOMAIN_CATALOG Domain;
        Domain.strDomainId = DomainId;
        Domain.ResourceCounts = StagedResourceCounts[DomainId];
        for (size_t iKind = 0u; iKind < CategorySets.size(); ++iKind)
        {
            Domain.Categories[iKind].assign(
                CategorySets[iKind].begin(), CategorySets[iKind].end());
        }
        StagedDomains.push_back(std::move(Domain));
    }

    m_ResourceCatalog = std::move(Staged);
    m_ResourceDomains = std::move(StagedDomains);
    ++m_iResourceCatalogRevision;
    if (0u == m_iResourceCatalogRevision)
        m_iResourceCatalogRevision = 1u;
    m_iResourceViewRevision = UINT64_MAX;
    m_VisibleResourceIndices.clear();
    m_pThumbnailCache->Invalidate(m_iResourceCatalogRevision);
    const auto SelectedDomain = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [this](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
        {
            return Domain.strDomainId == m_strSelectedAuthoringDomainId;
        });
    if (SelectedDomain == m_ResourceDomains.end() && !m_ResourceDomains.empty())
    {
        const char* pPreferredDomain = Resource_DomainId(m_eAllEffectsClass);
        const auto Preferred = nullptr == pPreferredDomain ?
            m_ResourceDomains.end() : std::find_if(
                m_ResourceDomains.begin(), m_ResourceDomains.end(),
                [pPreferredDomain](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
                {
                    return Domain.strDomainId == pPreferredDomain;
                });
        Select_AuthoringDomain(Preferred == m_ResourceDomains.end() ?
            m_ResourceDomains.front().strDomainId : Preferred->strDomainId);
    }
    m_strResourceStatus = "Catalog refreshed: " +
        std::to_string(m_ResourceCatalog.size()) +
        " supported Resources/Effect files across " +
        std::to_string(m_ResourceDomains.size()) +
        " authoring categories.";
    return true;
}

void Client::CEffect_Tool::Select_AuthoringDomain(
    const std::string& strDomainId)
{
    if (strDomainId.empty() ||
        m_strSelectedAuthoringDomainId == strDomainId)
        return;
    m_strSelectedAuthoringDomainId = strDomainId;
    Copy_Buffer(m_ResourceCategory.data(),
        m_ResourceCategory.size(), "All");
    m_strSelectedResourceAssetId.clear();
    m_iResourceViewRevision = UINT64_MAX;
    m_strResourceStatus = "Authoring category selected: " + strDomainId + ".";
}

bool_t Client::CEffect_Tool::Select_AuthoringDomainForClass(
    const LostArk::Shared::CHARACTER_CLASS_ID eClass)
{
    const char* pDomainId = Resource_DomainId(eClass);
    if (nullptr == pDomainId)
        return false;
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
        !Slot_Allowed(*pElement, m_strSelectedResourceSlotId))
    {
        m_strResourceStatus = "That resource slot is not allowed for this Element.";
        return false;
    }
    const EFFECT_RESOURCE_FILE_KIND eExpectedKind =
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
    if (CatalogEntry == m_ResourceCatalog.end())
    {
        m_strResourceStatus =
            "Selected resource is outside the active authoring category or file kind.";
        return false;
    }
    auto Iterator = std::find_if(
        pElement->ResourceBindings.begin(), pElement->ResourceBindings.end(),
        [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == m_strSelectedResourceSlotId;
        });
    if (Iterator == pElement->ResourceBindings.end())
        pElement->ResourceBindings.push_back(
            { m_strSelectedResourceSlotId, strAssetId });
    else
        Iterator->strAssetId = strAssetId;
    const std::string strSlotLabel =
        Slot_Label(*pElement, m_strSelectedResourceSlotId);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strResourceStatus = "Bound " + strAssetId + " to " +
        strSlotLabel + ".";
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
                return Binding.strSlotId == m_strSelectedResourceSlotId;
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
    const EFFECT_DOCUMENT_DESC PreviewDocument =
        Build_PreviewDocument(Document);
    if (!Ensure_WorldPreviewObject())
        return false;
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    std::string Error;
    if (nullptr == pObject || !pObject->Stage_Document(
        PreviewDocument, Error))
    {
        m_strPreviewStatus = "Document is editable but not drawable yet: " + Error;
        return false;
    }
    pObject->Set_Playing(false);
    pObject->Set_SampleTime(m_fPreviewTimeSeconds);
    m_strPreviewStatus = EFFECT_PREVIEW_FILTER::COMPLETE == m_ePreviewFilter ?
        "Complete Effect preview committed from the active Document." :
        (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ?
            "Selected Element Solo preview committed." :
            "Selected Element Mute preview committed.");
    return true;
}

Client::EFFECT_DOCUMENT_DESC
Client::CEffect_Tool::Build_PreviewDocument(
    const EFFECT_DOCUMENT_DESC& Document) const
{
    EFFECT_DOCUMENT_DESC Preview = Document;
    if (EFFECT_PREVIEW_FILTER::COMPLETE == m_ePreviewFilter ||
        m_strSelectedElementId.empty())
        return Preview;
    const bool_t bSelectionExists = std::any_of(
        Preview.Elements.begin(), Preview.Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    if (!bSelectionExists)
        return Preview;
    std::erase_if(Preview.Elements,
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            const bool_t bSelected =
                Element.strElementId == m_strSelectedElementId;
            return EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ?
                !bSelected : bSelected;
        });
    return Preview;
}

bool_t Client::CEffect_Tool::Stage_DetailDraftPreview()
{
    if (!m_ActiveDocument.has_value() || !m_DetailDraft.has_value())
        return false;

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    bool_t bFound = false;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
        if (Element.strElementId != m_strDetailDraftElementId)
            continue;
        Element.strDisplayName = m_DetailDraft->strDisplayName;
        Element.strGroupId = m_DetailDraft->strGroupId;
        Element.strSourceNode = m_DetailDraft->strSourceNode;
        Element.bVisible = m_DetailDraft->bVisible;
        Element.Detail = m_DetailDraft->Detail;
        Element.Material = m_DetailDraft->Material;
        bFound = true;
        break;
    }
    if (!bFound)
    {
        m_strPreviewStatus =
            "Live Detail preview rejected: selected Element is missing.";
        return false;
    }

    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    Recalculate_PreviewDuration(Staged);
    if (!Stage_WorldPreview(Staged))
    {
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_fPreviewTimeSeconds = fPreviousTime;
        return false;
    }
    m_strPreviewStatus =
        "Live Detail draft staged; Apply Detail commits it to the active Document.";
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

void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()
{
    m_fPreviewTimeSeconds = 0.f;
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject)
    {
        m_bPreviewPlaying = false;
        return;
    }
    pObject->Reset();
    pObject->Set_SampleTime(0.f);
    m_bPreviewPlaying = true;
}

void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()
{
    m_strPreviewAnimationStatus.clear();
    if (!m_ActiveDocument.has_value())
        return;

    std::string CatalogStatus;
    const bool_t bCatalogReloaded = CPlayerSkillCatalog::Load(CatalogStatus);
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    const auto Skill = std::find_if(
        Skills.begin(), Skills.end(),
        [this](const PLAYER_SKILL_DEFINITION& Candidate)
        {
            return Candidate.strEffectId ==
                m_ActiveDocument->strEffectAssetId;
        });
    if (Skill == Skills.end())
    {
        m_strPreviewAnimationStatus = bCatalogReloaded ?
            "No PlayerSkills row owns this Effect; animation was left unchanged." :
            "PlayerSkills refresh failed; animation was left unchanged: " +
                CatalogStatus;
        return;
    }

    m_eAllEffectsClass = Skill->eCharacterClass;
    Select_AuthoringDomainForClass(Skill->eCharacterClass);
    const char* pAnimationAsset = Animation_AssetName(Skill->eCharacterClass);
    if (nullptr == pAnimationAsset)
    {
        m_strPreviewAnimationStatus =
            "The loaded Effect has no admitted playable class target.";
        return;
    }

    const std::string CurrentAsset =
        CAnimationTargetService::Resolve_AssetName();
    if (CurrentAsset != pAnimationAsset &&
        !m_pCharacterPreviewPanel->Select_TargetAsset(pAnimationAsset))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing on the current target; the matching class model "
            "could not be staged.";
        return;
    }

    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel)
    {
        m_strPreviewAnimationStatus =
            "Effect is loaded, but no animation model target is available.";
        return;
    }

    ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
    std::string BindingStatus;
    if (!CAnimationSkillBindingDocument::Load(
        pAnimationAsset,
        Skill->eCharacterClass,
        Skills,
        Collect_AnimationClipNames(pModel),
        Bindings,
        BindingStatus))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; skill animation binding was not applied: " +
            BindingStatus;
        return;
    }

    const auto Binding = std::find_if(
        Bindings.Bindings.begin(), Bindings.Bindings.end(),
        [&Skill](const ANIMATION_SKILL_BINDING& Candidate)
        {
            return Candidate.iSkillId == Skill->iSkillId;
        });
    if (Binding == Bindings.Bindings.end() || Binding->Clips.empty() ||
        !pModel->Start_Animation(Binding->Clips.front().c_str(), true))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
    pModel->Set_AnimPaused(false);
    m_strPreviewAnimationStatus =
        "Skill animation synced: " + Skill->strInputSlot + " | " +
        Skill->strDisplayName + " -> " + Binding->Clips.front();
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
    m_ActiveDocumentPath.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
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
    if (!m_ActiveDocument.has_value())
    {
        m_fPreviewDurationSeconds = 1.f;
        m_fPreviewTimeSeconds = std::clamp(
            m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
        return;
    }
    Recalculate_PreviewDuration(*m_ActiveDocument);
}

void Client::CEffect_Tool::Recalculate_PreviewDuration(
    const EFFECT_DOCUMENT_DESC& Document)
{
    m_fPreviewDurationSeconds = 1.f;
    for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (!Element.bVisible)
            continue;
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
