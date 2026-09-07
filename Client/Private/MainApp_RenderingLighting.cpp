#include "imgui.h"
#include "MainApp.h"

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
    const bool toolVisible = m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::RENDERING) && m_bRenderingLightsTabActive;
    if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
    {
        const bool previewMap = toolVisible && m_bPreviewMapLightDraft &&
            m_eRenderingSelectedLevel == LEVEL::KAKULSAYDON_ARENA &&
            ETOUI(m_eRenderingSelectedLevel) == CGameInstance::Get().Get_CurrentLevelID() &&
            m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() == "LV_LUT_MIDNIGHTC_ED";
        arena->Set_MapLightAuthoringOverride(previewMap ? m_AreaLightSession.Get_Preview() : nullptr);
    }
    if (!toolVisible) { m_strLightPreviewResourceId.clear(); return; }
    if (m_strLightPreviewResourceId.empty()) return;
    if (std::chrono::steady_clock::now() >= m_LightPreviewEnd ||
        m_iLightPreviewLevel != CGameInstance::Get().Get_CurrentLevelID())
    { m_strLightPreviewResourceId.clear(); return; }
    const auto* resource = m_LightResources.Find_Resource(m_strLightPreviewResourceId);
    if (!resource) { m_strLightPreviewResourceId.clear(); return; }
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

void Client::CMainApp::RenderLightingWorkbench()
{
    const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel);
    const string areaId = descriptor && descriptor->pMapAreaId ? descriptor->pMapAreaId : "";
    if (m_strRenderingLightAreaAttempt != areaId)
    {
        m_strRenderingLightAreaAttempt = areaId;
        m_strSelectedRenderingLightId = "@default-directional";
        if (!areaId.empty()) m_AreaLightSession.Open(areaId, m_strLightingStatus);
    }
    const bool areaReady = !areaId.empty() && m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() == areaId;
    const bool currentMap = ETOUI(m_eRenderingSelectedLevel) == CGameInstance::Get().Get_CurrentLevelID();
    if (!areaId.empty() && ImGui::Button("Reload Authored Map Lights"))
    {
        if (m_AreaLightSession.Open(areaId, m_strLightingStatus)) m_bPreviewMapLightDraft = true;
    }
    const auto reloadPublishedMap = [&]()
    {
        if (!m_LightResources.Refresh_MapResources(areaId, m_strLightingStatus)) return false;
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
    };
    if (!areaId.empty())
    {
        ImGui::SameLine();
        if (ImGui::Button("Reload Published Map Lights")) reloadPublishedMap();
        ImGui::Checkbox("Preview authored map lights", &m_bPreviewMapLightDraft);
    }
    ImGui::SeparatorText("All Lights");
    const char* categories[] = { "All", "Map", "Character", "Boss" };
    ImGui::Combo("Category##Lights", &m_iRenderingLightCategory, categories, 4);
    ImGui::TextDisabled("Map placements persist in the Area. Reusable resources are appended in Action Workbench > Light.");
    if (ImGui::BeginChild("LightResourceList", ImVec2(560.f, 170.f), true))
    {
        if (m_iRenderingLightCategory == 0 || m_iRenderingLightCategory == 1)
        {
            if (ImGui::Selectable("[Map] Default Directional Light", m_strSelectedRenderingLightId == "@default-directional"))
                m_strSelectedRenderingLightId = "@default-directional";
            if (areaReady)
                for (const auto& light : m_AreaLightSession.Get_Document().Get_Lights())
                {
                    const string id = "map:" + light.lightId;
                    const string label = "[Map placement] " + (light.displayName.empty() ? light.lightId : light.displayName) + "##" + id;
                    if (ImGui::Selectable(label.c_str(), m_strSelectedRenderingLightId == id)) m_strSelectedRenderingLightId = id;
                }
        }
        for (const auto& resource : m_LightResources.Get_Resources())
        {
            if (m_LightResources.Is_MapResource(resource.strLightResourceId)) continue;
            const int category = resource.strDefaultAnchorKind == "MAP" ? 1 : resource.strDefaultAnchorKind == "PLAYER" ? 2 : 3;
            if (m_iRenderingLightCategory != 0 && category != m_iRenderingLightCategory) continue;
            const string id = "resource:" + resource.strLightResourceId;
            const string label = "[" + string(categories[category]) + " resource] " + resource.strDisplayName + "##" + id;
            if (ImGui::Selectable(label.c_str(), m_strSelectedRenderingLightId == id)) m_strSelectedRenderingLightId = id;
        }
    }
    ImGui::EndChild();

    ImGui::SeparatorText("Create Light");
    ImGui::InputText("Name##CreateLight", m_szRenderingLightName, sizeof(m_szRenderingLightName));
    const char* anchors[] = { "Map", "Character", "Boss" };
    const char* types[] = { "Directional", "Point", "Spot" };
    ImGui::Combo("Anchor type##CreateLight", &m_iRenderingLightCreateAnchor, anchors, 3);
    ImGui::Combo("Light type##CreateLight", &m_iRenderingLightCreateType, types, 3);
    if (m_iRenderingLightCreateAnchor == 0)
        ImGui::Checkbox("Reusable Map resource (pattern lifetime)", &m_bRenderingCreateReusableMapLight);
    if (ImGui::Button("Create Light"))
    {
        const LIGHT type = m_iRenderingLightCreateType == 0 ? LIGHT::DIRECTIONAL : m_iRenderingLightCreateType == 1 ? LIGHT::POINT : LIGHT::SPOT;
        string id;
        if (m_iRenderingLightCreateAnchor == 0 && !m_bRenderingCreateReusableMapLight)
        {
            float4x4_t pivot{};
            if (type == LIGHT::DIRECTIONAL)
            {
                m_strSelectedRenderingLightId = "@default-directional";
                m_strLightingStatus = "Selected the existing Default Directional Light for this profile.";
            }
            else if (!currentMap || !areaReady)
                m_strLightingStatus = "Enter the selected Level with an authored map light layer before creating a placement.";
            else if (!Lighting_PlayerPivot(pivot))
                m_strLightingStatus = "A current player is required to seed the map light position.";
            else if (m_AreaLightSession.Create(m_szRenderingLightName, type,
                float3_t(pivot._41, pivot._42 + 8.f, pivot._43), id, m_strLightingStatus))
                { m_strSelectedRenderingLightId = "map:" + id; m_bPreviewMapLightDraft = true; }
        }
        else
        {
            const char* anchor = m_iRenderingLightCreateAnchor == 0 ? "MAP" : m_iRenderingLightCreateAnchor == 1 ? "PLAYER" : "BOSS";
            if (m_LightResources.Create(m_szRenderingLightName, anchor, type, id, m_strLightingStatus))
                m_strSelectedRenderingLightId = "resource:" + id;
        }
    }

    ImGui::SeparatorText("Light Detail");
    if (m_strSelectedRenderingLightId == "@default-directional")
    {
        ImGui::Text("Default Directional Light | Map | %s", m_strRenderingDraftProfileId.c_str());
        bool changed = ImGui::DragFloat3("Direction##DefaultLight", &m_SceneRenderingDraft.Light.vDirection.x, .01f, -64.f, 64.f);
        changed |= ImGui::ColorEdit3("RGB##DefaultLight", &m_SceneRenderingDraft.Light.vDiffuse.x);
        changed |= ImGui::ColorEdit3("Ambient RGB##DefaultLight", &m_SceneRenderingDraft.Light.vAmbient.x);
        changed |= ImGui::ColorEdit3("Specular RGB##DefaultLight", &m_SceneRenderingDraft.Light.vSpecular.x);
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
        ImGui::TextDisabled("Saved in RenderingProfiles.json with the selected scene. This row edits the existing map directional light.");
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
                if (m_AreaLightSession.Publish_Runtime(m_strLightingStatus)) reloadPublishedMap();
            ImGui::SameLine();
            if (ImGui::Button("Delete Light"))
                if (m_AreaLightSession.Delete(id, m_strLightingStatus)) m_strSelectedRenderingLightId = "@default-directional";
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
            if (Lighting_EditResource(resource, false, 0.f)) m_LightResources.Update(resource, m_strLightingStatus);
            if (ImGui::Button("Save Light")) m_LightResources.Save_Authored(m_strLightingStatus);
            ImGui::SameLine();
            if (ImGui::Button("Publish Light"))
                if (m_LightResources.Publish_Runtime(m_strLightingStatus)) m_LightResources.Reload_Runtime(m_strLightingStatus);
            ImGui::SameLine();
            if (ImGui::Button("Reload Light")) m_LightResources.Load_Authored(m_strLightingStatus);
            ImGui::SameLine();
            if (ImGui::Button("Delete Light"))
                if (m_LightResources.Delete(id, m_strLightingStatus)) m_strSelectedRenderingLightId = "@default-directional";
            if (ImGui::Button("Preview 3 seconds"))
            {
                float4x4_t pivot{};
                const bool ready = resource.eType == LIGHT::DIRECTIONAL || (resource.strDefaultAnchorKind == "BOSS" ? Lighting_BossPivot(pivot) : Lighting_PlayerPivot(pivot));
                if (ready)
                {
                    if (resource.eType == LIGHT::DIRECTIONAL) XMStoreFloat4x4(&pivot, XMMatrixIdentity());
                    m_LightPreviewMapPivot = pivot; m_strLightPreviewResourceId = id;
                    m_LightPreviewEnd = std::chrono::steady_clock::now() + std::chrono::seconds(3);
                    m_iLightPreviewLevel = CGameInstance::Get().Get_CurrentLevelID();
                    m_strLightingStatus = "Preview started. Append this resource in Action Workbench > Light to set pattern lifetime.";
                }
                else m_strLightingStatus = "Preview needs the selected anchor: a current player or active Kouku gate boss.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop Preview")) m_strLightPreviewResourceId.clear();
        }
    }
    ImGui::Checkbox("Debug light wire", &m_bLightingDebugWire);
    if (m_bLightingDebugWire && !m_strLightPreviewResourceId.empty() && std::chrono::steady_clock::now() < m_LightPreviewEnd)
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
#endif
