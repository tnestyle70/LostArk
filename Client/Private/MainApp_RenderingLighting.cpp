#include "imgui.h"
#include "MainApp.h"
#include "CompositionTimeline.h"

#ifdef _DEBUG
#include "Character.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "Level_Bern.h"
#include "Level_CharacterSelect.h"
#include "Level_KakulSaydonArena.h"
#include "Level_ValtanArena.h"
#include "LightResourceCatalog.h"
#include "Npc.h"
#include "Presentation_Manager.h"
#include "Transform.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string_view>

namespace
{
    using namespace Client;
    using namespace Engine;

    shared_ptr<CCharacter> Lighting_LocalCharacter()
    {
        switch (static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()))
        {
        case LEVEL::BERN:
            if (auto* level = CLevel_Bern::Get_Active()) return level->Get_LocalCharacter();
            break;
        case LEVEL::CHARACTER_SELECT:
            if (auto* level = CLevel_CharacterSelect::Get_Active()) return level->Get_LocalCharacter();
            break;
        case LEVEL::VALTAN_ARENA:
            if (auto* level = CLevel_ValtanArena::Get_Active()) return level->Get_LocalCharacter();
            break;
        case LEVEL::KAKULSAYDON_ARENA:
            if (auto* level = CLevel_KakulSaydonArena::Get_Active()) return level->Get_LocalCharacter();
            break;
        default: break;
        }
        return nullptr;
    }

    bool Lighting_PlayerPivot(float4x4_t& pivot)
    {
        const auto character = Lighting_LocalCharacter();
        if (!character || !character->Get_Transform()) return false;
        const auto* world = character->Get_Transform()->Get_WorldMatrixPtr();
        XMStoreFloat4x4(&pivot, XMMatrixTranslation(world->_41, world->_42, world->_43));
        return true;
    }

    bool Lighting_BossPivot(float4x4_t& pivot)
    {
        auto* arena = CLevel_KakulSaydonArena::Get_Active();
        if (!arena) return false;
        const auto& gates = arena->Get_DebugGates();
        const size_t gate = arena->Get_ActiveDebugGate();
        if (gate >= gates.size() || !gates[gate].pHudFocusArchetypeId) return false;
        const auto boss = arena->Debug_FindArenaBossNpc(gates[gate].pHudFocusArchetypeId);
        if (!boss || !boss->Get_Transform()) return false;
        const auto* world = boss->Get_Transform()->Get_WorldMatrixPtr();
        XMStoreFloat4x4(&pivot, XMMatrixTranslation(world->_41, world->_42, world->_43));
        return true;
    }

    class CWorkbenchLightPreview final : public IPresentationProvider
    {
    public:
        LIGHT_DESC light{};
        shared_ptr<uint32_t> skipped;
        HRESULT Submit_Presentation() override
        {
            auto& presentation = CPresentation_Manager::Get();
            const bool available = presentation.Get_TransientLights().size() < 64u;
            presentation.Register_ProviderSubmissionExpectation(1u, available ? 1u : 0u, 0u, 0u);
            if (!available) { if (skipped) ++*skipped; return S_OK; }
            return presentation.Add_TransientLight(light);
        }
    };

    LIGHT_RESOURCE Lighting_MapResource(const MAP_POINT_LIGHT_RECORD& record)
    {
        LIGHT_RESOURCE resource;
        resource.strLightResourceId = record.lightId;
        resource.strDisplayName = record.displayName.empty() ? record.lightId : record.displayName;
        resource.strDefaultAnchorKind = "MAP";
        resource.eType = record.kind;
        resource.vLocalOffset = record.position;
        resource.vLocalRotationDegrees = record.rotationDegrees;
        resource.fRangeMeters = record.radiusMeters;
        resource.fFalloffExponent = record.falloffExponent;
        resource.fInnerConeDegrees = record.innerConeDegrees;
        resource.fOuterConeDegrees = record.outerConeDegrees;
        resource.vColor = record.color;
        resource.fBrightness = record.brightness;
        return resource;
    }

    bool Lighting_EditResource(LIGHT_RESOURCE& resource, bool worldPosition, float floorY)
    {
        bool changed = false;
        char name[129]{};
        strncpy_s(name, resource.strDisplayName.c_str(), _TRUNCATE);
        if (ImGui::InputText("Name##LightDetail", name, sizeof(name)))
        { resource.strDisplayName = name; changed = true; }
        ImGui::Text("Type: %s", CLightResourceCatalog::Kind_Name(resource.eType));
        if (resource.eType != LIGHT::DIRECTIONAL)
        {
            changed |= ImGui::DragFloat3(worldPosition ? "World position" : "Anchor offset", &resource.vLocalOffset.x,
                0.05f, -100000.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Range (m)", &resource.fRangeMeters, 0.1f, 0.01f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Falloff", &resource.fFalloffExponent, 0.05f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        }
        if (resource.eType != LIGHT::POINT)
            changed |= ImGui::DragFloat3("Rotation (degrees)", &resource.vLocalRotationDegrees.x,
                0.5f, -360.f, 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        if (resource.eType == LIGHT::SPOT)
        {
            if (ImGui::Button("Point Down"))
            { resource.vLocalRotationDegrees = {90.f, 0.f, 0.f}; changed = true; }
            changed |= ImGui::DragFloat("Inner cone (degrees)", &resource.fInnerConeDegrees,
                0.25f, 0.1f, 88.9f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Outer cone (degrees)", &resource.fOuterConeDegrees,
                0.25f, 0.2f, 89.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            resource.fOuterConeDegrees = (std::max)(resource.fInnerConeDegrees + 0.1f, resource.fOuterConeDegrees);
            const float height = (std::max)(0.01f, std::abs(resource.vLocalOffset.y - floorY));
            float radius = height * std::tan(XMConvertToRadians(resource.fOuterConeDegrees));
            if (ImGui::DragFloat("Floor circle radius (m)", &radius, 0.05f, 0.01f, 10000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
            {
                resource.fOuterConeDegrees = (std::clamp)(XMConvertToDegrees(std::atan(radius / height)), 0.2f, 89.f);
                resource.fInnerConeDegrees = (std::min)(resource.fInnerConeDegrees, resource.fOuterConeDegrees - 0.1f);
                changed = true;
            }
            ImGui::TextDisabled("Circle radius uses height %.2f m and a downward cone; range must reach the floor.", height);
        }
        changed |= ImGui::ColorEdit3("RGB##LightDetail", &resource.vColor.x);
        changed |= ImGui::DragFloat("Brightness##LightDetail", &resource.fBrightness, 0.05f, 0.f, 64.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        resource.vColor.w = 1.f;
        return changed;
    }

    void Lighting_DrawWire(const LIGHT_DESC& light)
    {
        if (light.eType == LIGHT::DIRECTIONAL) return;
        auto& game = CGameInstance::Get();
        const matrix_t view = XMLoadFloat4x4(game.Get_Transform(D3DTS::VIEW));
        const matrix_t projection = XMLoadFloat4x4(game.Get_Transform(D3DTS::PROJ));
        auto* viewport = ImGui::GetMainViewport();
        auto* draw = ImGui::GetBackgroundDrawList(viewport);
        const auto project = [&](fvector_t world, ImVec2& out)
        {
            const vector_t v = XMVector3TransformCoord(world, view);
            if (XMVectorGetZ(v) <= 0.1f) return false;
            const vector_t p = XMVector3TransformCoord(v, projection);
            out = { viewport->Pos.x + (XMVectorGetX(p) * .5f + .5f) * viewport->Size.x,
                viewport->Pos.y + (.5f - XMVectorGetY(p) * .5f) * viewport->Size.y };
            return true;
        };
        const auto line = [&](fvector_t a, fvector_t b)
        { ImVec2 pa{}, pb{}; if (project(a, pa) && project(b, pb)) draw->AddLine(pa, pb, IM_COL32(255, 224, 80, 220), 1.5f); };
        const vector_t origin = XMLoadFloat4(&light.vPosition);
        const vector_t direction = light.eType == LIGHT::SPOT ? XMVector3Normalize(XMLoadFloat4(&light.vDirection)) : XMVectorSet(0.f, 1.f, 0.f, 0.f);
        const vector_t up = std::abs(XMVectorGetY(direction)) > .99f ? XMVectorSet(0.f, 0.f, 1.f, 0.f) : XMVectorSet(0.f, 1.f, 0.f, 0.f);
        const vector_t right = XMVector3Normalize(XMVector3Cross(direction, up));
        const vector_t forward = XMVector3Normalize(XMVector3Cross(right, direction));
        const float angle = light.eType == LIGHT::SPOT ? std::acos(light.fSpotOuterCos) : 0.f;
        const float radius = light.eType == LIGHT::SPOT ? light.fRange * std::sin(angle) : light.fRange;
        const vector_t center = light.eType == LIGHT::SPOT ? origin + direction * (light.fRange * std::cos(angle)) : origin;
        for (int i = 0; i < 48; ++i)
        {
            const float a = XM_2PI * float(i) / 48.f, b = XM_2PI * float(i + 1) / 48.f;
            const vector_t p = center + (right * std::cos(a) + forward * std::sin(a)) * radius;
            line(p, center + (right * std::cos(b) + forward * std::sin(b)) * radius);
            if (light.eType == LIGHT::SPOT && i % 12 == 0) line(origin, p);
        }
    }
}

void Client::CMainApp::UpdateLightingPreview()
{
    const bool toolVisible = m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::RENDERING);
    if (!toolVisible || (!m_strLightPreviewSelectionId.empty() &&
        m_iLightPreviewLevel != CGameInstance::Get().Get_CurrentLevelID()))
        StopLightingPreview();
    if (!m_strLightPreviewSelectionId.empty())
    {
        const auto now = std::chrono::steady_clock::now();
        if (!m_bLightPreviewPaused)
            m_fLightPreviewCursorMs += std::chrono::duration<double, std::milli>(now - m_LightPreviewLastUpdate).count();
        m_LightPreviewLastUpdate = now;
        if (m_fLightPreviewCursorMs >= static_cast<double>(m_iLightPreviewLifetimeMs))
        {
            StopLightingPreview();
            m_fLightPreviewCursorMs = static_cast<double>(m_iLightPreviewLifetimeMs);
        }
        else if (m_strLightPreviewSelectionId.rfind("map:", 0u) == 0u)
            StageMapLightPreview();
    }
    if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
    {
        const bool selectedMap = toolVisible && m_eRenderingSelectedLevel == LEVEL::KAKULSAYDON_ARENA &&
            ETOUI(m_eRenderingSelectedLevel) == CGameInstance::Get().Get_CurrentLevelID() &&
            m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() == "LV_LUT_MIDNIGHTC_ED";
        arena->Set_MapLightAuthoringOverride(selectedMap ?
            (m_pLightSequencerMapPreview ? m_pLightSequencerMapPreview :
                m_bPreviewMapLightDraft ? m_AreaLightSession.Get_Preview() : nullptr) : nullptr);
    }
    if (!toolVisible || m_strLightPreviewResourceId.empty()) return;
    const auto* resource = m_LightResources.Find_Resource(m_strLightPreviewResourceId);
    if (!resource) { StopLightingPreview(); return; }
    float4x4_t pivot = m_LightPreviewMapPivot;
    if (resource->eType != LIGHT::DIRECTIONAL &&
        ((resource->strDefaultAnchorKind == "PLAYER" && !Lighting_PlayerPivot(pivot)) ||
        (resource->strDefaultAnchorKind == "BOSS" && !Lighting_BossPivot(pivot))))
    { m_strLightingStatus = "Preview anchor is unavailable."; return; }
    auto provider = std::make_shared<CWorkbenchLightPreview>();
    provider->skipped = m_pLightPreviewSkipped;
    if (!CLightResourceCatalog::Try_BuildLightDesc(*resource, pivot, 1.f, provider->light, m_strLightingStatus)) return;
    CPresentation_Manager::Get().Add_FrameProvider(provider);
}

void Client::CMainApp::SelectRenderingLight(const string& id)
{
        if (m_strSelectedRenderingLightId != id) StopLightingPreview();
        m_strSelectedRenderingLightId = id;
        if (id == "@default-directional")
            m_strRenderingSelectedProfileId = m_strRenderingQualityProfileId;
        else if (id.rfind("scene:", 0u) == 0u)
            m_strRenderingSelectedProfileId = id.substr(6u);
        if (id == "@default-directional" || id.rfind("scene:", 0u) == 0u)
            if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
            { m_SceneRenderingDraft = *profile; m_strRenderingDraftProfileId = profile->strProfileId; }
        m_bLightDetailWindowVisible = true;
}

void Client::CMainApp::RenderLightingWorkbench()
{
    const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel);
    const string areaId = descriptor && descriptor->pMapAreaId ? descriptor->pMapAreaId : "";
    if (m_strRenderingLightAreaAttempt != areaId)
    {
        m_strRenderingLightAreaAttempt = areaId;
        SelectRenderingLight("@default-directional");
        if (!areaId.empty()) m_AreaLightSession.Open(areaId, m_strLightingStatus);
    }
    const bool areaReady = !areaId.empty() && m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() == areaId;
    const bool currentMap = ETOUI(m_eRenderingSelectedLevel) == CGameInstance::Get().Get_CurrentLevelID();
    if (!areaId.empty() && ImGui::Button("Reload Authored Map Lights"))
    {
        if (m_AreaLightSession.Open(areaId, m_strLightingStatus)) m_bPreviewMapLightDraft = true;
    }
    if (!areaId.empty())
    {
        ImGui::SameLine();
        if (ImGui::Button("Reload Published Map Lights")) ReloadPublishedMapLights();
        ImGui::Checkbox("Preview authored map lights", &m_bPreviewMapLightDraft);
    }
    ImGui::SeparatorText("Create Light");
    const char* usages[] = { "Map Profile (persistent)", "Scene Profile (mood)", "Anchor Light (pattern)" };
    const char* anchors[] = { "Map (fixed world)", "Character", "Boss" };
    const char* types[] = { "Directional", "Point", "Spot" };
    ImGui::TextUnformatted("Usage type");
    ImGui::SetNextItemWidth(-1.f);
    ImGui::Combo("##CreateLightUsage", &m_iRenderingLightCreateUsage, usages, 3);
    if (m_iRenderingLightCreateUsage == 1)
    {
        ImGui::TextWrapped("Create a pattern scene mood from the selected scene profile.");
        ImGui::TextWrapped("Source: %s", m_SceneRenderingDraft.Get_DisplayName().c_str());
        ImGui::TextDisabled("Light type: Directional (scene profile)");
    }
    else
    {
        if (m_iRenderingLightCreateUsage == 2)
        {
            ImGui::TextWrapped("Pattern-only light. Its Action Workbench box controls when it appears and ends.");
            ImGui::TextUnformatted("Anchor type");
            ImGui::SetNextItemWidth(-1.f);
            ImGui::Combo("##CreateLightAnchor", &m_iRenderingLightCreateAnchor, anchors, 3);
            if (m_iRenderingLightCreateAnchor == 0)
                ImGui::TextWrapped("Map: fixed World position during the pattern.");
        }
        else ImGui::TextWrapped("Persistent map placement. Create near the player, tune its World position, then Save Light and Publish Light.");
        ImGui::TextUnformatted("Light type");
        ImGui::SetNextItemWidth(-1.f);
        ImGui::Combo("##CreateLightType", &m_iRenderingLightCreateType, types, 3);
    }
    const bool defaultDirectional = m_iRenderingLightCreateUsage == 0 && m_iRenderingLightCreateType == 0;
    char defaultName[] = "Default Directional Light";
    char* createName = defaultDirectional ? defaultName : m_iRenderingLightCreateUsage == 1 ?
        m_szRenderingNewProfileName : m_szRenderingLightName;
    const size_t nameCapacity = defaultDirectional ? sizeof(defaultName) : m_iRenderingLightCreateUsage == 1 ?
        sizeof(m_szRenderingNewProfileName) : sizeof(m_szRenderingLightName);
    ImGui::TextUnformatted("Light name");
    ImGui::BeginDisabled(defaultDirectional);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##CreateLightName", "Enter the light name", createName, nameCapacity);
    ImGui::EndDisabled();
    if (m_iRenderingLightCreateUsage == 1)
    {
        ImGui::TextUnformatted("New profile ID");
        ImGui::SetNextItemWidth(-1.f);
        ImGui::InputText("##CreateSceneProfileId", m_szRenderingNewProfileId, sizeof(m_szRenderingNewProfileId));
    }
    const bool hasName = std::string_view(createName).find_first_not_of(" \t\r\n") != std::string_view::npos;
    if (!hasName) ImGui::TextWrapped("Enter a light name before creating it.");
    if (defaultDirectional) ImGui::TextWrapped("This map already owns its default directional light. Select it to edit and save.");
    ImGui::BeginDisabled(!defaultDirectional && !hasName);
    if (ImGui::Button(defaultDirectional ? "Select Default Directional Light" : "Create Light"))
    {
        const LIGHT type = m_iRenderingLightCreateType == 0 ? LIGHT::DIRECTIONAL : m_iRenderingLightCreateType == 1 ? LIGHT::POINT : LIGHT::SPOT;
        string id;
        if (m_iRenderingLightCreateUsage == 0)
        {
            float4x4_t pivot{};
            if (type == LIGHT::DIRECTIONAL)
            {
                SelectRenderingLight("@default-directional");
                m_strLightingStatus = "Selected the existing Default Directional Light for this map profile.";
            }
            else if (!currentMap || !areaReady)
                m_strLightingStatus = "Enter the selected Level with an authored map light layer before creating a placement.";
            else if (!Lighting_PlayerPivot(pivot))
                m_strLightingStatus = "A current player is required to seed the map light position.";
            else if (m_AreaLightSession.Create(m_szRenderingLightName, type,
                float3_t(pivot._41, pivot._42 + 8.f, pivot._43), id, m_strLightingStatus))
                { SelectRenderingLight("map:" + id); m_bPreviewMapLightDraft = true; }
        }
        else if (m_iRenderingLightCreateUsage == 1)
        {
            if (m_RenderingProfiles.Duplicate_Profile(m_strRenderingSelectedProfileId, m_szRenderingNewProfileId, m_strLightingStatus, m_szRenderingNewProfileName))
                SelectRenderingLight("scene:" + string(m_szRenderingNewProfileId));
        }
        else
        {
            const char* anchor = m_iRenderingLightCreateAnchor == 0 ? "MAP" : m_iRenderingLightCreateAnchor == 1 ? "PLAYER" : "BOSS";
            if (m_LightResources.Create(m_szRenderingLightName, anchor, type, id, m_strLightingStatus))
            { SelectRenderingLight("resource:" + id); m_iRenderingLightCategory = m_iRenderingLightCreateAnchor + 1; }
        }
        m_iRenderingLightUsageCategory = m_iRenderingLightCreateUsage + 1;
    }
    ImGui::EndDisabled();
    ImGui::SeparatorText("All Lights");
    const char* usageFilters[] = { "All", "Map Profile (persistent)", "Scene Profile (mood)", "Anchor Light (pattern)" };
    ImGui::Combo("Category##LightUsage", &m_iRenderingLightUsageCategory, usageFilters, 4);
    ImGui::TextWrapped("Select an item for Light Detail and Light Sequencer.");
    const float listHeight = (std::max)(140.f, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing() * 3.f);
    if (ImGui::BeginChild("LightResourceList", ImVec2(0.f, listHeight), true))
    {
        if ((m_iRenderingLightUsageCategory == 0 || m_iRenderingLightUsageCategory == 1) &&
            ImGui::TreeNodeEx("Map Profile", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextWrapped("Persistent map lights: enabled placements remain active in this Area.");
            if (ImGui::Selectable("Default Directional Light", m_strSelectedRenderingLightId == "@default-directional"))
                SelectRenderingLight("@default-directional");
            if (areaReady)
                for (const auto& light : m_AreaLightSession.Get_Document().Get_Lights())
                {
                    const string id = "map:" + light.lightId;
                    const string label = (light.displayName.empty() ? light.lightId : light.displayName) + "##" + id;
                    if (ImGui::Selectable(label.c_str(), m_strSelectedRenderingLightId == id)) SelectRenderingLight(id);
                }
            ImGui::TreePop();
        }
        if ((m_iRenderingLightUsageCategory == 0 || m_iRenderingLightUsageCategory == 2) &&
            ImGui::TreeNodeEx("Scene Profile", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (const auto& profileId : m_RenderingProfiles.Collect_ProfileIds())
            {
                const string id = "scene:" + profileId;
                const auto* profile = m_RenderingProfiles.Find_Profile(profileId);
                const string label = (profile ? profile->Get_DisplayName() : profileId) + "##" + id;
                if (ImGui::Selectable(label.c_str(), m_strSelectedRenderingLightId == id)) SelectRenderingLight(id);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", profileId.c_str());
            }
            ImGui::TreePop();
        }
        if ((m_iRenderingLightUsageCategory == 0 || m_iRenderingLightUsageCategory == 3) &&
            ImGui::TreeNodeEx("Anchor Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextWrapped("Pattern resources: append to an Action Workbench Light box to set the lifetime.");
            const char* categories[] = { "All", "Map (fixed world)", "Character", "Boss" };
            ImGui::Combo("Anchor category", &m_iRenderingLightCategory, categories, 4);
            for (int category = 1; category <= 3; ++category)
            {
                if (m_iRenderingLightCategory != 0 && m_iRenderingLightCategory != category) continue;
                if (!ImGui::TreeNodeEx(categories[category], ImGuiTreeNodeFlags_DefaultOpen)) continue;
                for (const auto& resource : m_LightResources.Get_Resources())
                {
                    if (m_LightResources.Is_MapResource(resource.strLightResourceId)) continue;
                    const int resourceCategory = resource.strDefaultAnchorKind == "MAP" ? 1 : resource.strDefaultAnchorKind == "PLAYER" ? 2 : 3;
                    if (category != resourceCategory) continue;
                    const string id = "resource:" + resource.strLightResourceId;
                    const string label = resource.strDisplayName + " [" + CLightResourceCatalog::Kind_Name(resource.eType) + "]##" + id;
                    if (ImGui::Selectable(label.c_str(), m_strSelectedRenderingLightId == id)) SelectRenderingLight(id);
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
    ImGui::TextWrapped("%s", m_strLightingStatus.c_str());
}

bool Client::CMainApp::ReloadPublishedMapLights()
{
    StopLightingPreview();
    const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel);
    const string areaId = descriptor && descriptor->pMapAreaId ? descriptor->pMapAreaId : "";
    const bool currentMap = ETOUI(m_eRenderingSelectedLevel) == CGameInstance::Get().Get_CurrentLevelID();
    if (areaId.empty() || !m_LightResources.Refresh_MapResources(areaId, m_strLightingStatus)) return false;
    if (currentMap && m_eRenderingSelectedLevel == LEVEL::KAKULSAYDON_ARENA)
    {
        auto* arena = CLevel_KakulSaydonArena::Get_Active();
        if (!arena || !arena->Reload_MapLights())
        { m_strLightingStatus = "Published map lights could not be reloaded; previous runtime preserved. See Output log."; return false; }
        arena->Set_MapLightAuthoringOverride(nullptr);
    }
    m_bPreviewMapLightDraft = false;
    m_strLightingStatus = "Published map lights reloaded; authoring preview disabled.";
    return true;
}

void Client::CMainApp::RenderLightDetail()
{
    const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel);
    const string areaId = descriptor && descriptor->pMapAreaId ? descriptor->pMapAreaId : "";
    const bool areaReady = !areaId.empty() && m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() == areaId;
    const bool currentMap = ETOUI(m_eRenderingSelectedLevel) == CGameInstance::Get().Get_CurrentLevelID();
    ImGui::SeparatorText("Light Detail");
    if (m_strSelectedRenderingLightId == "@default-directional" || m_strSelectedRenderingLightId.rfind("scene:", 0u) == 0u)
    {
        const bool sceneProfile = m_strSelectedRenderingLightId.rfind("scene:", 0u) == 0u;
        ImGui::TextWrapped("%s | %s", sceneProfile ? "Scene Profile" : "Default Directional Light | Map Profile", m_SceneRenderingDraft.Get_DisplayName().c_str());
        ImGui::TextDisabled("ID: %s", m_strRenderingDraftProfileId.c_str());
        char displayName[257]{};
        strncpy_s(displayName, m_SceneRenderingDraft.strDisplayName.c_str(), _TRUNCATE);
        bool changed = ImGui::InputText("Display name", displayName, sizeof(displayName));
        if (changed) m_SceneRenderingDraft.strDisplayName = displayName;
        changed |= ImGui::DragFloat3("Direction##DefaultLight", &m_SceneRenderingDraft.Light.vDirection.x, .01f, -64.f, 64.f);
        changed |= ImGui::DragFloat3("RGB##DefaultLight", &m_SceneRenderingDraft.Light.vDiffuse.x, .005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Ambient RGB##DefaultLight", &m_SceneRenderingDraft.Light.vAmbient.x, .005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Specular RGB##DefaultLight", &m_SceneRenderingDraft.Light.vSpecular.x, .005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (changed)
        {
            m_SceneRenderingDraft.Light.vDirection.w = 0.f;
            m_SceneRenderingDraft.Light.vDiffuse.w = m_SceneRenderingDraft.Light.vAmbient.w = m_SceneRenderingDraft.Light.vSpecular.w = 1.f;
            if (!m_RenderingProfiles.Update_Profile(m_SceneRenderingDraft, m_strLightingStatus))
                if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingDraftProfileId)) m_SceneRenderingDraft = *profile;
        }
        if (ImGui::Button("Save Light")) m_RenderingProfiles.Save_Authored(m_strLightingStatus);
        ImGui::SameLine();
        if (ImGui::Button("Publish Light")) m_RenderingProfiles.Publish_Runtime(m_strLightingStatus);
        ImGui::SameLine();
        if (ImGui::Button("Reload Light"))
            if (m_RenderingProfiles.Reload_Runtime(m_strLightingStatus))
                if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingDraftProfileId)) m_SceneRenderingDraft = *profile;
        ImGui::TextWrapped("Saved in RenderingProfiles.json. Scene lighting uses this existing profile.");
        if (ImGui::Button("Activate Selected Scene"))
            m_RenderingProfiles.Activate_Profile(m_strRenderingSelectedProfileId, m_strLightingStatus);
        ImGui::SameLine();
        if (ImGui::Button("Delete Selected Profile"))
            if (m_RenderingProfiles.Delete_Profile(m_strRenderingSelectedProfileId, m_strLightingStatus))
            {
                SelectRenderingLight("@default-directional");
                m_strRenderingSelectedProfileId = m_strRenderingQualityProfileId;
                if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
                { m_SceneRenderingDraft = *profile; m_strRenderingDraftProfileId = profile->strProfileId; }
            }
        RenderSceneProfileDetail();
    }
    else if (m_strSelectedRenderingLightId.rfind("map:", 0u) == 0u && areaReady)
    {
        const string id = m_strSelectedRenderingLightId.substr(4u);
        const auto& lights = m_AreaLightSession.Get_Document().Get_Lights();
        const auto found = std::find_if(lights.begin(), lights.end(), [&](const auto& row) { return row.lightId == id; });
        if (found != lights.end())
        {
            MAP_POINT_LIGHT_RECORD record = *found;
            LIGHT_RESOURCE resource = Lighting_MapResource(record);
            float4x4_t player{};
            const float floorY = Lighting_PlayerPivot(player) ? player._42 : record.position.y - 8.f;
            ImGui::Text("Map placement: %s", id.c_str());
            ImGui::TextWrapped("Persistent map light. Save Light stores this placement; Publish Light applies it on map entry.");
            ImGui::BeginDisabled(m_AreaLightSession.Is_ReadOnly());
            bool changed = ImGui::Checkbox("Enabled##MapLight", &record.enabled);
            changed |= Lighting_EditResource(resource, true, floorY);
            if (changed)
            {
                record.displayName = resource.strDisplayName; record.position = resource.vLocalOffset;
                record.rotationDegrees = resource.vLocalRotationDegrees; record.radiusMeters = resource.fRangeMeters;
                record.falloffExponent = resource.fFalloffExponent; record.innerConeDegrees = resource.fInnerConeDegrees;
                record.outerConeDegrees = resource.fOuterConeDegrees; record.color = resource.vColor; record.brightness = resource.fBrightness;
                if (m_AreaLightSession.Update(record, m_strLightingStatus)) m_bPreviewMapLightDraft = true;
            }
            if (ImGui::Button("Save Light"))
                if (m_AreaLightSession.Save_Authored(m_strLightingStatus))
                {
                    string refreshed;
                    if (!m_LightResources.Refresh_MapResources(areaId, refreshed)) m_strLightingStatus += " Resource list: " + refreshed;
                }
            ImGui::SameLine();
            if (ImGui::Button("Publish Light"))
                if (m_AreaLightSession.Publish_Runtime(m_strLightingStatus)) ReloadPublishedMapLights();
            ImGui::SameLine();
            if (ImGui::Button("Delete Light"))
                if (m_AreaLightSession.Delete(id, m_strLightingStatus)) SelectRenderingLight("@default-directional");
            ImGui::EndDisabled();
            if (ImGui::Button("Reload Authored Light"))
                if (m_AreaLightSession.Reload(m_strLightingStatus)) m_bPreviewMapLightDraft = true;
            if (m_AreaLightSession.Is_ReadOnly()) ImGui::TextDisabled("Imported v1 map light data is read-only.");
            if (m_bLightingDebugWire && currentMap && record.enabled)
            {
                float4x4_t identity{}; XMStoreFloat4x4(&identity, XMMatrixIdentity());
                LIGHT_DESC light{}; string ignored;
                if (CLightResourceCatalog::Try_BuildLightDesc(resource, identity, 1.f, light, ignored)) Lighting_DrawWire(light);
            }
        }
    }
    else if (m_strSelectedRenderingLightId.rfind("resource:", 0u) == 0u)
    {
        const string id = m_strSelectedRenderingLightId.substr(9u);
        if (const auto* source = m_LightResources.Find_Resource(id))
        {
            LIGHT_RESOURCE resource = *source;
            ImGui::Text("Reusable resource: %s", id.c_str());
            ImGui::Text("Anchor: %s", resource.strDefaultAnchorKind == "PLAYER" ? "Character" : resource.strDefaultAnchorKind.c_str());
            const bool fixedWorld = resource.strDefaultAnchorKind == "MAP";
            ImGui::TextWrapped(fixedWorld ? "Pattern light at a fixed World position. The Light box controls its lifetime." :
                "Pattern light following the selected anchor. The Light box controls its lifetime.");
            float4x4_t player{};
            const bool hasPlayer = fixedWorld && Lighting_PlayerPivot(player);
            bool changed = false;
            if (fixedWorld && resource.eType != LIGHT::DIRECTIONAL)
            {
                ImGui::BeginDisabled(!hasPlayer);
                if (ImGui::Button("Place above player (+8m)"))
                { resource.vLocalOffset = {player._41, player._42 + 8.f, player._43}; changed = true; }
                ImGui::EndDisabled();
            }
            const float floorY = fixedWorld ? (hasPlayer ? player._42 : resource.vLocalOffset.y - 8.f) : 0.f;
            changed |= Lighting_EditResource(resource, fixedWorld, floorY);
            if (changed) m_LightResources.Update(resource, m_strLightingStatus);
            if (ImGui::Button("Save Light")) m_LightResources.Save_Authored(m_strLightingStatus);
            ImGui::SameLine();
            if (ImGui::Button("Publish Light"))
                if (m_LightResources.Publish_Runtime(m_strLightingStatus)) m_LightResources.Reload_Runtime(m_strLightingStatus);
            ImGui::SameLine();
            if (ImGui::Button("Reload Light")) m_LightResources.Load_Authored(m_strLightingStatus);
            ImGui::SameLine();
            if (ImGui::Button("Delete Light"))
                if (m_LightResources.Delete(id, m_strLightingStatus)) SelectRenderingLight("@default-directional");
            if (ImGui::Button("Play Preview")) StartLightingPreview();
            ImGui::SameLine();
            if (ImGui::Button("Stop Preview")) StopLightingPreview();
            ImGui::Text("Preview lifetime: %d ms (Light Sequencer)", m_iLightPreviewLifetimeMs);
        }
    }
    ImGui::Checkbox("Debug light wire", &m_bLightingDebugWire);
    if (m_bLightingDebugWire && !m_strLightPreviewResourceId.empty() && m_fLightPreviewCursorMs < static_cast<double>(m_iLightPreviewLifetimeMs))
    {
        if (const auto* resource = m_LightResources.Find_Resource(m_strLightPreviewResourceId))
        {
            float4x4_t pivot = m_LightPreviewMapPivot;
            const bool ready = resource->strDefaultAnchorKind == "MAP" ||
                (resource->strDefaultAnchorKind == "PLAYER" ? Lighting_PlayerPivot(pivot) : Lighting_BossPivot(pivot));
            LIGHT_DESC light{}; string ignored;
            if (ready && CLightResourceCatalog::Try_BuildLightDesc(*resource, pivot, 1.f, light, ignored)) Lighting_DrawWire(light);
        }
    }
    ImGui::Text("Frame lights: %u / 64 | Preview frames skipped by budget: %u",
        CPresentation_Manager::Get().Get_LastTransientLightCount(), *m_pLightPreviewSkipped);
    ImGui::TextWrapped("%s", m_strLightingStatus.c_str());
}
bool Client::CMainApp::StageMapLightPreview()
{
    if (m_strLightPreviewSelectionId.rfind("map:", 0u) != 0u || !m_AreaLightSession.Is_Open()) return false;
    CMapLightDocument document = m_AreaLightSession.Get_Document();
    if (document.Get_FormatVersion() != 2u)
    { m_strLightingStatus = "Imported map profiles remain continuous and read-only."; return false; }
    auto lights = document.Get_Lights();
    const string id = m_strLightPreviewSelectionId.substr(4u);
    const auto found = std::find_if(lights.begin(), lights.end(), [&](const auto& row) { return row.lightId == id; });
    if (found == lights.end()) { m_strLightingStatus = "Selected map light is unavailable."; return false; }
    found->enabled = true;
    if (!document.Replace_Authored(lights, document.Get_NextLightOrdinal(), m_strLightingStatus)) return false;
    if (!m_pLightSequencerMapPreview) m_pLightSequencerMapPreview = make_shared<CMapLightPresentationRuntime>();
    return m_pLightSequencerMapPreview->Replace_Document(document);
}

bool Client::CMainApp::StartLightingPreview()
{
    StopLightingPreview();
    if (m_strSelectedRenderingLightId.rfind("resource:", 0u) == 0u)
    {
        const string id = m_strSelectedRenderingLightId.substr(9u);
        const auto* resource = m_LightResources.Find_Resource(id);
        if (!resource) { m_strLightingStatus = "Selected light resource is unavailable."; return false; }
        float4x4_t pivot{};
        XMStoreFloat4x4(&pivot, XMMatrixIdentity());
        const bool ready = resource->eType == LIGHT::DIRECTIONAL || resource->strDefaultAnchorKind == "MAP" ||
            (resource->strDefaultAnchorKind == "PLAYER" && Lighting_PlayerPivot(pivot)) ||
            (resource->strDefaultAnchorKind == "BOSS" && Lighting_BossPivot(pivot));
        if (!ready)
        { m_strLightingStatus = "Selected preview anchor is unavailable: " + resource->strDefaultAnchorKind; return false; }
        m_LightPreviewMapPivot = pivot;
        m_strLightPreviewResourceId = id;
    }
    else if (m_strSelectedRenderingLightId.rfind("map:", 0u) == 0u)
    {
        if (m_eRenderingSelectedLevel != LEVEL::KAKULSAYDON_ARENA || !CLevel_KakulSaydonArena::Get_Active())
        { m_strLightingStatus = "Enter KoukuSaydon to preview this authored map light."; return false; }
        m_strLightPreviewSelectionId = m_strSelectedRenderingLightId;
        if (!StageMapLightPreview()) { StopLightingPreview(); return false; }
    }
    else if (m_strSelectedRenderingLightId == "@default-directional" || m_strSelectedRenderingLightId.rfind("scene:", 0u) == 0u)
    {
        m_strLightPreviewPreviousProfileId = m_RenderingProfiles.Get_ActiveProfileId();
        m_strLightPreviewSceneProfileId = m_strRenderingSelectedProfileId;
        if (!m_RenderingProfiles.Activate_Profile(m_strLightPreviewSceneProfileId, m_strLightingStatus))
        { m_strLightPreviewPreviousProfileId.clear(); m_strLightPreviewSceneProfileId.clear(); return false; }
    }
    else { m_strLightingStatus = "Select a light or scene profile first."; return false; }
    m_strLightPreviewSelectionId = m_strSelectedRenderingLightId;
    m_LightPreviewLastUpdate = std::chrono::steady_clock::now();
    m_fLightPreviewCursorMs = 0.0;
    m_bLightPreviewPaused = false;
    m_iLightPreviewLevel = CGameInstance::Get().Get_CurrentLevelID();
    m_bLightSequencerWindowVisible = true;
    m_strLightingStatus = "Preview started. Stop or lifetime end restores the previous preview state. Source values are unchanged.";
    return true;
}

void Client::CMainApp::StopLightingPreview()
{
    if (!m_strLightPreviewPreviousProfileId.empty() &&
        m_iLightPreviewLevel == CGameInstance::Get().Get_CurrentLevelID() &&
        m_RenderingProfiles.Get_ActiveProfileId() == m_strLightPreviewSceneProfileId)
    {
        string status;
        if (!m_RenderingProfiles.Activate_Profile(m_strLightPreviewPreviousProfileId, status)) m_strLightingStatus = status;
    }
    m_strLightPreviewPreviousProfileId.clear();
    m_strLightPreviewSceneProfileId.clear();
    m_strLightPreviewSelectionId.clear();
    m_strLightPreviewResourceId.clear();
    m_pLightSequencerMapPreview.reset();
    m_fLightPreviewCursorMs = 0.0;
    m_bLightPreviewPaused = false;
}

void Client::CMainApp::RenderSceneProfileDetail()
{
	const auto applyScene = [this]()
	{
		m_RenderingProfiles.Update_Profile(
			m_SceneRenderingDraft, m_strRenderingStatus);
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
		{
			m_SceneRenderingDraft = *pProfile;
		}
	};

	bool_t sceneChanged = false;
	ImGui::SeparatorText("Active Scene Artistic Profile");
	sceneChanged |= ImGui::DragFloat(
		"Exposure Multiplier", &m_SceneRenderingDraft.fExposureMultiplier,
		0.005f, 0.1f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Bloom Intensity Multiplier",
		&m_SceneRenderingDraft.fBloomIntensityMultiplier,
		0.005f, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::Checkbox(
		"Directional Shadow Enabled",
		&m_SceneRenderingDraft.ShadowSettings.bEnabled);
	ImGui::BeginDisabled(!m_SceneRenderingDraft.ShadowSettings.bEnabled);
	sceneChanged |= ImGui::DragFloat3(
		"Shadow Focus", &m_SceneRenderingDraft.vShadowFocus.x,
		0.1f, -100000.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Light Distance", &m_SceneRenderingDraft.fShadowDistance,
		0.1f, 0.1f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Coverage Width",
		&m_SceneRenderingDraft.ShadowSettings.fOrthographicWidth,
		0.1f, 0.1f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Coverage Height",
		&m_SceneRenderingDraft.ShadowSettings.fOrthographicHeight,
		0.1f, 0.1f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Near", &m_SceneRenderingDraft.ShadowSettings.fNear,
		0.01f, 0.0001f, 100000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Far", &m_SceneRenderingDraft.ShadowSettings.fFar,
		0.1f, 0.0001f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Depth Bias", &m_SceneRenderingDraft.ShadowSettings.fDepthBias,
		0.00005f, 0.f, 0.05f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Normal Bias", &m_SceneRenderingDraft.ShadowSettings.fNormalBias,
		0.001f, 0.f, 10.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Strength", &m_SceneRenderingDraft.ShadowSettings.fStrength,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Shadow uses a fixed 2048 depth map with 3x3 PCF; light eye is derived from focus and scene direction.");

	ImGui::SeparatorText("Height Fog");
	sceneChanged |= ImGui::Checkbox(
		"Height Fog Enabled", &m_SceneRenderingDraft.Fog.bEnabled);
	ImGui::BeginDisabled(!m_SceneRenderingDraft.Fog.bEnabled);
	sceneChanged |= ImGui::ColorEdit3(
		"Fog Color", &m_SceneRenderingDraft.Fog.vColor.x);
	sceneChanged |= ImGui::DragFloat(
		"Fog Top Height", &m_SceneRenderingDraft.Fog.fTopHeight,
		0.25f, -10000.f, 10000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Height Falloff", &m_SceneRenderingDraft.Fog.fHeightFalloff,
		0.002f, 0.0001f, 4.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Density", &m_SceneRenderingDraft.Fog.fDensity,
		0.01f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Start Distance", &m_SceneRenderingDraft.Fog.fStartDistance,
		0.25f, 0.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Maximum Opacity", &m_SceneRenderingDraft.Fog.fMaximumOpacity,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Drift Speed", &m_SceneRenderingDraft.Fog.fDriftSpeed,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Drift Height", &m_SceneRenderingDraft.Fog.fDriftHeightAmplitude,
		0.05f, 0.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Drift Density", &m_SceneRenderingDraft.Fog.fDriftDensityAmplitude,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::SeparatorText("Cloud Banks");
	/* The authored value is a fraction; the slider speaks percent because that
	   is how the map coverage is judged by eye. */
	f32_t fFogCoveragePercent =
		m_SceneRenderingDraft.Fog.fCoveragePercent * 100.f;
	if (ImGui::DragFloat("Map Coverage", &fFogCoveragePercent,
		0.5f, 0.f, 100.f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp))
	{
		m_SceneRenderingDraft.Fog.fCoveragePercent =
			fFogCoveragePercent * 0.01f;
		sceneChanged = true;
	}
	sceneChanged |= ImGui::DragFloat(
		"Wind Direction X", &m_SceneRenderingDraft.Fog.fWindDirectionX,
		0.01f, -1.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Wind Direction Z", &m_SceneRenderingDraft.Fog.fWindDirectionZ,
		0.01f, -1.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Wind Speed", &m_SceneRenderingDraft.Fog.fWindSpeed,
		0.05f, 0.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Patch Scale", &m_SceneRenderingDraft.Fog.fPatchScale,
		0.0005f, 0.0001f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Patch Softness", &m_SceneRenderingDraft.Fog.fPatchSoftness,
		0.005f, 0.001f, 0.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::TextDisabled(
		"Coverage 100%% is one blanket. Lower it and the fog breaks into banks that the wind walks across world XZ; Patch Scale sets their size.");
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Fog fills below Top Height and is applied in the deferred combine, so effects and the blend group stay clear of it.");
	if (sceneChanged)
	{
		m_SceneRenderingDraft.Light.vDirection.w = 0.f;
		m_SceneRenderingDraft.Light.vDiffuse.w = 1.f;
		m_SceneRenderingDraft.Light.vAmbient.w = 1.f;
		m_SceneRenderingDraft.Light.vSpecular.w = 1.f;
		m_SceneRenderingDraft.ShadowSettings.fFar = (std::max)(
			m_SceneRenderingDraft.ShadowSettings.fFar,
			m_SceneRenderingDraft.ShadowSettings.fNear + 0.0001f);
		applyScene();
	}
	ImGui::TextDisabled(
		"Effective Exposure/Bloom = selected quality base x scene multiplier.");

}

void Client::CMainApp::RenderLightSequencer()
{
    const bool reusable = m_strSelectedRenderingLightId.rfind("resource:", 0u) == 0u;
    const auto* selected = reusable ? m_LightResources.Find_Resource(m_strSelectedRenderingLightId.substr(9u)) : nullptr;
    string selectedName = selected ? selected->strDisplayName : m_strSelectedRenderingLightId == "@default-directional" ? "Default Directional Light" : m_SceneRenderingDraft.Get_DisplayName();
    bool hasSelection = selected != nullptr || m_strSelectedRenderingLightId == "@default-directional" || m_strSelectedRenderingLightId.rfind("scene:", 0u) == 0u;
    if (m_strSelectedRenderingLightId.rfind("map:", 0u) == 0u && m_AreaLightSession.Is_Open())
        for (const auto& light : m_AreaLightSession.Get_Document().Get_Lights())
            if ("map:" + light.lightId == m_strSelectedRenderingLightId)
            { selectedName = light.displayName.empty() ? light.lightId : light.displayName; hasSelection = true; break; }
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button(m_bLightPreviewPaused ? "Resume" : "Play"))
    {
        if (m_bLightPreviewPaused && !m_strLightPreviewSelectionId.empty())
        { m_bLightPreviewPaused = false; m_LightPreviewLastUpdate = std::chrono::steady_clock::now(); }
        else StartLightingPreview();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_strLightPreviewSelectionId.empty());
    if (ImGui::Button("Pause")) m_bLightPreviewPaused = true;
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Stop")) StopLightingPreview();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(130.f);
    if (ImGui::DragInt("Lifetime (ms)", &m_iLightPreviewLifetimeMs, 10.f, 1, 600000, "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLightPreviewCursorMs = (std::min)(m_fLightPreviewCursorMs, static_cast<double>(m_iLightPreviewLifetimeMs));
    const auto seek = [this](const double cursor)
    {
        if (m_strLightPreviewSelectionId.empty() && !StartLightingPreview()) return;
        m_fLightPreviewCursorMs = (std::clamp)(cursor, 0.0, static_cast<double>(m_iLightPreviewLifetimeMs));
        m_bLightPreviewPaused = true;
        m_LightPreviewLastUpdate = std::chrono::steady_clock::now();
    };
    int cursor = static_cast<int>(m_fLightPreviewCursorMs);
    ImGui::SetNextItemWidth((std::max)(120.f, ImGui::GetContentRegionAvail().x - 80.f));
    if (ImGui::SliderInt("Seek (ms)", &cursor, 0, m_iLightPreviewLifetimeMs)) seek(cursor);
    ImGui::Text("%s | %d / %d ms", m_strLightPreviewSelectionId.empty() ? "Stopped" : m_bLightPreviewPaused ? "Paused" : "Playing", cursor, m_iLightPreviewLifetimeMs);
    ImGui::EndDisabled();
    if (!hasSelection) ImGui::TextWrapped("Select a light or scene profile in Light Resources.");
    else if (selected)
        ImGui::TextWrapped("%s | Anchor: %s | Preview lifetime is local; the Action Workbench Light box stores pattern timing.",
            selectedName.c_str(), selected->strDefaultAnchorKind == "PLAYER" ? "Character" : selected->strDefaultAnchorKind.c_str());
    else
        ImGui::TextWrapped("%s | Scene / map preview. The original profile or map enabled state returns on Stop; no duplicate directional light is added.", selectedName.c_str());

    if (ImGui::BeginChild("LightTimelineCanvas", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float labelWidth = 76.f;
        const float width = (std::max)(180.f, ImGui::GetContentRegionAvail().x - labelWidth);
        const float scale = width * 1000.f / static_cast<float>(m_iLightPreviewLifetimeMs);
        auto* draw = ImGui::GetWindowDrawList();
        const ImVec2 rulerMin(origin.x + labelWidth, origin.y);
        const ImVec2 rulerMax(rulerMin.x + width, origin.y + 24.f);
        CompositionTimeline::DrawRuler(draw, rulerMin, rulerMax, static_cast<uint32_t>(m_iLightPreviewLifetimeMs), scale);
        draw->AddText(ImVec2(origin.x + 4.f, origin.y + 31.f), IM_COL32(221, 195, 110, 255), "Light");
        CompositionTimeline::DrawBox(draw, ImVec2(rulerMin.x, rulerMax.y + 4.f), ImVec2(rulerMax.x, rulerMax.y + 28.f),
            IM_COL32(160, 136, 57, 255), hasSelection, hasSelection ? selectedName.c_str() : "Select a light", false, false);
        const float cursorX = rulerMin.x + static_cast<float>(m_fLightPreviewCursorMs) * scale * .001f;
        draw->AddLine(ImVec2(cursorX, rulerMin.y), ImVec2(cursorX, rulerMax.y + 34.f), IM_COL32(255, 96, 96, 255), 2.f);
        ImGui::InvisibleButton("LightTimelineSeek", ImVec2(labelWidth + width, 62.f));
        if (hasSelection && ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::GetIO().MousePos.x >= rulerMin.x)
            seek((ImGui::GetIO().MousePos.x - rulerMin.x) * 1000.0 / scale);
    }
    ImGui::EndChild();
}

#endif
