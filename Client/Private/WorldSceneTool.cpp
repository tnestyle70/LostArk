#include "imgui.h"
#include "WorldSceneTool.h"

#ifdef _DEBUG
#include "DataJson.h"
#include "MapAuthoringHost.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>

namespace Client
{
namespace
{
std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
std::string String(const DATA_JSON_VALUE& row, const char* name)
{
    const auto* value = row.Find(name);
    return value && value->Is_String() ? value->Get_String() : std::string{};
}
bool SourcePath(const DATA_JSON_VALUE& row, const char* name, std::filesystem::path& out)
{
    const auto value = String(row, name);
    if (value.empty()) { out.clear(); return true; }
    const auto relative = std::filesystem::path(value).lexically_normal();
    if (relative.is_absolute() || relative.has_root_name()) return false;
    auto part = relative.begin();
    if (part == relative.end() || *part != L"Data") return false;
    std::filesystem::path inside;
    for (++part; part != relative.end(); ++part)
    { if (*part == L"..") return false; inside /= *part; }
    if (inside.empty()) return false;
    out = CProjectDataRoot::Resolve(inside);
    return !out.empty();
}
float3_t EulerDegrees(const float4_t& quaternion)
{
    float4x4_t matrix{};
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion)));
    const float sine = std::clamp(-matrix._32, -1.f, 1.f);
    const float pitch = std::asin(sine);
    float yaw = 0.f, roll = 0.f;
    if (std::abs(sine) < .99999f)
    { yaw = std::atan2(matrix._31, matrix._33); roll = std::atan2(matrix._12, matrix._22); }
    else yaw = std::atan2(-matrix._13, matrix._11);
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}
float4_t Quaternion(const float3_t& degrees)
{
    vector_t value = XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(degrees.x), XMConvertToRadians(degrees.y), XMConvertToRadians(degrees.z)));
    if (XMVectorGetW(value) < 0.f) value = XMVectorNegate(value);
    float4_t out{}; XMStoreFloat4(&out, value); return out;
}
}

CWorldSceneTool::CWorldSceneTool() = default;
CWorldSceneTool::~CWorldSceneTool() { Stop_AnimationPreview(); Stop_SelfMotionPreview(); }
IMapAuthoringHost* CWorldSceneTool::Host() const
{
    auto* host = Find_ActiveMapAuthoringHost();
    return host && host->Get_MapAuthoringLevelIndex() == m_LevelIndex &&
        host->Get_MapAuthoringCatalog().Get_AreaId() == m_AreaId ? host : nullptr;
}
void CWorldSceneTool::Open() { m_bOpen = true; m_bRowsDirty = true; }
void CWorldSceneTool::Hide()
{
    m_bOpen = false; m_bPickRequested = false;
    Stop_AnimationPreview(); Stop_SelfMotionPreview();
}
void CWorldSceneTool::Update(float deltaSeconds, bool visible)
{
    m_Edit.Update(visible && m_bOpen && !m_bDeploySelected);
    auto* current = Find_ActiveMapAuthoringHost();
    const auto level = current ? current->Get_MapAuthoringLevelIndex() : UINT32_MAX;
    const auto area = current ? current->Get_MapAuthoringCatalog().Get_AreaId() : std::string{};
    const uint64_t generation = current ? current->Get_MapAuthoringRuntime().Debug_GetRuntimeGeneration() : UINT64_MAX;
    if (level != m_LevelIndex || area != m_AreaId || generation != m_RuntimeGeneration)
    {
        Stop_AnimationPreview(); Stop_SelfMotionPreview();
        m_Edit.End();
        m_MapSelection.reset(); m_DeploySelection.reset(); m_SelectedOriginal.reset();
        m_Undo.clear(); m_bPickRequested = false;
        m_LevelIndex = level; m_AreaId = area; m_RuntimeGeneration = generation; m_bRowsDirty = true;
    }
    if (!visible || !m_bOpen)
    { Stop_AnimationPreview(); Stop_SelfMotionPreview(); return; }
    if (current)
    {
        auto* deploy = current->Get_MapAuthoringDeployRuntime();
        const size_t count = current->Get_MapAuthoringPlacements().size() +
            (deploy ? deploy->Get_Entries().size() : 0u);
        if (count != m_Rows.size()) m_bRowsDirty = true;
    }
    if (m_Edit.Consume_RowsDirty()) m_bRowsDirty = true;
    if (m_bRowsDirty) Refresh_Rows();
    Update_AnimationPreview(deltaSeconds);
}
bool CWorldSceneTool::Consume_PickRequest() { return std::exchange(m_bPickRequested, false); }
bool CWorldSceneTool::Consume_InteractionRequest() { return std::exchange(m_bInteraction, false); }
bool CWorldSceneTool::Consume_FocusRequest(float3_t& position, float& radius)
{
    if (!std::exchange(m_bFocusRequested, false)) return false;
    position = m_FocusPosition; radius = m_FocusRadius; return true;
}
void CWorldSceneTool::Refresh_Rows()
{
    m_Rows.clear(); m_FilteredRows.clear(); m_bRowsDirty = false;
    auto* host = Host(); if (!host) return;
    for (const auto& entry : host->Get_MapAuthoringPlacements())
    {
        const auto& record = entry.record;
        const auto* asset = host->Get_MapAuthoringCatalog().Find(record.assetId);
        ROW row; row.id = record.placementId;
        row.label = "Map #" + std::to_string(row.id) + " | " + (asset ? asset->label : record.assetId);
        row.search = Lower(row.label + " " + record.sourcePlacementId + " " + record.sourceLevel + " " + record.assetId);
        m_Rows.push_back(std::move(row));
    }
    if (auto* deploy = host->Get_MapAuthoringDeployRuntime())
        for (const auto& entry : deploy->Get_Entries())
        {
            ROW row; row.id = entry.placement.runtimePlacementId; row.deploy = true;
            row.label = "Deploy #" + std::to_string(row.id) + " | " + entry.placement.assetId;
            row.search = Lower(row.label + " " + entry.placement.sourcePlacementId);
            m_Rows.push_back(std::move(row));
        }
    const auto query = Lower(m_Search.data());
    for (size_t i = 0; i < m_Rows.size(); ++i)
        if (query.empty() || m_Rows[i].search.find(query) != std::string::npos) m_FilteredRows.push_back(i);
}
bool CWorldSceneTool::Inspect_Placement(uint64_t placementId, bool deploy)
{
    auto* host = Host();
    if (!host) return false;
    if (deploy)
    {
        const auto* runtime = host->Get_MapAuthoringDeployRuntime();
        if (!runtime || std::none_of(runtime->Get_Entries().begin(), runtime->Get_Entries().end(),
            [placementId](const auto& entry) { return entry.placement.runtimePlacementId == placementId; })) return false;
        Select_Deploy(placementId);
    }
    else
    {
        const auto& entries = host->Get_MapAuthoringPlacements();
        if (std::none_of(entries.begin(), entries.end(),
            [placementId](const auto& entry) { return entry.record.placementId == placementId; })) return false;
        Select_Map(placementId);
    }
    m_Status = "Selected the live placement. Pick in scene identifies its exact mesh and material.";
    return true;
}
void CWorldSceneTool::Select_Map(uint64_t id)
{
    auto* host = Host(); if (!host) return;
    const auto& entries = host->Get_MapAuthoringPlacements();
    const auto entry = std::find_if(entries.begin(), entries.end(),
        [id](const auto& value) { return value.record.placementId == id; });
    if (entry == entries.end()) return;
    Stop_AnimationPreview();
    MAP_WORLD_MESH_PICK selection;
    selection.meshIndex = UINT32_MAX;
    const auto& record = entry->record;
    selection.placementId = id; selection.areaId = m_AreaId;
    selection.sourcePlacementId = record.sourcePlacementId; selection.sourceLevel = record.sourceLevel;
    selection.assetId = record.assetId; selection.hitPosition = record.position;
    if (const auto* asset = host->Get_MapAuthoringCatalog().Find(record.assetId))
        selection.modelAssetId = asset->modelRelativePath.generic_string();
    m_MapSelection = std::move(selection); m_DeploySelection.reset(); m_bDeploySelected = false;
    m_Edit.Select_Placement(id); m_Undo.clear(); m_PendingUndo.reset(); m_bUndoGestureSaved = false;
    const auto* draft = m_Edit.Is_Bound() ? m_Edit.Find_Draft(id) : nullptr;
    m_SelectedOriginal = draft ? *draft : record;
}
void CWorldSceneTool::Complete_MapPick(MAP_WORLD_MESH_PICK selected)
{
    if (selected.areaId != m_AreaId) return;
    Select_Map(selected.placementId);
    m_MapSelection = std::move(selected);
    m_Status = "Selected the nearest map mesh triangle. Source and material are shown below.";
}
void CWorldSceneTool::Select_Deploy(uint64_t id)
{
    auto* host = Host(); auto* deploy = host ? host->Get_MapAuthoringDeployRuntime() : nullptr;
    if (!deploy) return;
    const auto& entries = deploy->Get_Entries();
    const auto entry = std::find_if(entries.begin(), entries.end(),
        [id](const auto& value) { return value.placement.runtimePlacementId == id; });
    if (entry == entries.end()) return;
    Stop_AnimationPreview();
    DEPLOY_WORLD_MESH_PICK selection;
    selection.meshIndex = UINT32_MAX;
    selection.runtimePlacementId = id; selection.areaId = m_AreaId;
    selection.sourcePlacementId = entry->placement.sourcePlacementId; selection.assetId = entry->placement.assetId;
    selection.hitPosition = entry->placement.position;
    if (entry->object) selection.state = entry->object->Get_State();
    if (const auto* asset = deploy->Get_Catalog().Find(selection.assetId))
    {
        selection.modelKind = asset->kind;
        selection.modelAssetId = (selection.state == DEPLOY_PROP_STATE::FRACTURED && asset->kind == DEPLOY_PROP_MODEL_KIND::STATIC ?
            asset->fracturedRelativePath : asset->intactRelativePath).generic_string();
    }
    m_DeploySelection = std::move(selection); m_MapSelection.reset(); m_bDeploySelected = true;
    m_Edit.Clear_Selection(); m_SelectedOriginal.reset(); m_Undo.clear();
}
void CWorldSceneTool::Complete_DeployPick(DEPLOY_WORLD_MESH_PICK selected)
{
    if (selected.areaId != m_AreaId) return;
    Select_Deploy(selected.runtimePlacementId);
    m_DeploySelection = std::move(selected);
    m_Status = "Selected the nearest Deploy mesh in its current rendered pose.";
}
bool CWorldSceneTool::Try_GetSelectedHit(float3_t& position) const
{
    if (!Host()) return false;
    if (m_bDeploySelected && m_DeploySelection) position = m_DeploySelection->hitPosition;
    else if (m_MapSelection) position = m_MapSelection->hitPosition;
    else return false;
    return true;
}
bool CWorldSceneTool::Begin_Editing()
{
    auto* host = Host(); if (!host) return false;
    Stop_AnimationPreview(); Stop_SelfMotionPreview();
    std::string reason;
    if (!host->Can_ChangeMapAuthoringStructure(reason)) { m_Status = reason; return false; }
    std::ifstream input(CProjectDataRoot::Resolve("Maps/MapCatalog.json"), std::ios::binary);
    if (!input) { m_Status = "Cannot read MapCatalog.json."; return false; }
    const std::string bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    DATA_JSON_VALUE root;
    if (!CDataJson::Parse(bytes, root, m_Status)) return false;
    const auto* areas = root.Find("areas");
    if (!areas || !areas->Is_Array()) { m_Status = "MapCatalog has no areas."; return false; }
    for (const auto& row : areas->Get_Array())
    {
        if (String(row, "id") != m_AreaId) continue;
        CMapPlacementEditSession::BIND_DESC desc;
        desc.areaId = m_AreaId; desc.allowPartialLive = true;
        std::filesystem::path lights;
        if (!SourcePath(row, "sourceCatalog", desc.sourceCatalog) ||
            !SourcePath(row, "sourcePlacements", desc.sourcePlacements) ||
            !SourcePath(row, "sourceMaterials", desc.sourceMaterials) || !SourcePath(row, "sourceLights", lights))
        { m_Status = "MapCatalog has an invalid Data source path."; return false; }
        desc.declaresLights = !lights.empty();
        if (!m_Edit.Bind(desc, m_Status)) return false;
        if (m_MapSelection) Select_Map(m_MapSelection->placementId);
        return true;
    }
    m_Status = "Active Area is absent from MapCatalog."; return false;
}
void CWorldSceneTool::Render_Rows()
{
    if (ImGui::InputTextWithHint("##search", "Source ID / asset / level / placement", m_Search.data(), m_Search.size()))
    { m_bRowsDirty = true; Refresh_Rows(); }
    ImGui::Text("%zu / %zu live placements", m_FilteredRows.size(), m_Rows.size());
    ImGui::BeginChild("Live placements", ImVec2(0.f, 240.f), ImGuiChildFlags_Borders);
    ImGuiListClipper clipper; clipper.Begin(static_cast<int>(m_FilteredRows.size()));
    while (clipper.Step()) for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
    {
        const auto& row = m_Rows[m_FilteredRows[static_cast<size_t>(index)]];
        const bool selected = row.deploy ? m_bDeploySelected && m_DeploySelection && m_DeploySelection->runtimePlacementId == row.id :
            !m_bDeploySelected && m_MapSelection && m_MapSelection->placementId == row.id;
        ImGui::PushID(row.deploy ? "deploy" : "map"); ImGui::PushID(static_cast<int>(index));
        if (ImGui::Selectable(row.label.c_str(), selected))
        { if (row.deploy) Select_Deploy(row.id); else Select_Map(row.id); }
        ImGui::PopID(); ImGui::PopID();
    }
    ImGui::EndChild();
}
void CWorldSceneTool::Render_MapDetails()
{
    if (!m_MapSelection) return;
    const auto& selected = *m_MapSelection;
    ImGui::Text("Map #%llu", static_cast<unsigned long long>(selected.placementId));
    if (selected.meshIndex != UINT32_MAX) ImGui::Text("Mesh %u", selected.meshIndex);
    else ImGui::TextDisabled("Placement selected. Pick in scene identifies its exact mesh and material.");
    ImGui::TextWrapped("Source: %s | Level: %s", selected.sourcePlacementId.c_str(), selected.sourceLevel.c_str());
    ImGui::TextWrapped("Asset: %s\nWModel: %s\nMaterial: %s", selected.assetId.c_str(), selected.modelAssetId.c_str(), selected.materialName.c_str());
    ImGui::Text("Hit XYZ: %.4f / %.4f / %.4f m", selected.hitPosition.x, selected.hitPosition.y, selected.hitPosition.z);
    if (ImGui::Button("Copy source selection"))
    {
        std::ostringstream out;
        out << "Area: " << selected.areaId << "\nPlacement: " << selected.placementId << "\nSource: " << selected.sourcePlacementId
            << "\nLevel: " << selected.sourceLevel << "\nAsset: " << selected.assetId << "\nWModel: " << selected.modelAssetId
            << "\nMesh: ";
        if (selected.meshIndex == UINT32_MAX) out << "placement selected; Pick in scene for exact mesh";
        else out << selected.meshIndex;
        out << "\nMaterial: " << selected.materialName << "\nHit XYZ (m): " << selected.hitPosition.x
            << ", " << selected.hitPosition.y << ", " << selected.hitPosition.z;
        ImGui::SetClipboardText(out.str().c_str());
    }
    if (!m_Edit.Is_Bound()) return;
    const auto* draft = m_Edit.Find_Draft(selected.placementId); if (!draft) return;
    const auto original = *draft;
    auto staged = original;
    auto angles = EulerDegrees(staged.rotationQuaternion);
    const auto blocked = m_Edit.Describe_EditBlock(selected.placementId);
    auto* host = Host(); std::string ownerReason;
    const bool ownerReady = host && host->Can_ChangeMapAuthoringStructure(ownerReason);
    if (!blocked.empty()) ImGui::TextWrapped("%s", blocked.c_str());
    if (!ownerReady) ImGui::TextWrapped("%s", ownerReason.c_str());
    if (m_bSelfMotionOwned) ImGui::TextDisabled("Stop and restore map motions before editing the authored pose.");
    ImGui::BeginDisabled(!blocked.empty() || !ownerReady || m_Edit.Is_Publishing() || m_bSelfMotionOwned);
    bool changed = ImGui::DragFloat3("Position (m)", &staged.position.x, .05f);
    bool activated = ImGui::IsItemActivated();
    bool active = ImGui::IsItemActive();
    const bool rotation = ImGui::DragFloat3("Rotation (deg pitch/yaw/roll)", &angles.x, .5f);
    activated |= ImGui::IsItemActivated(); changed |= rotation;
    active |= ImGui::IsItemActive();
    if (rotation) staged.rotationQuaternion = Quaternion(angles);
    changed |= ImGui::DragFloat3("Signed scale", &staged.signedScale.x, .01f, -1000.f, 1000.f);
    activated |= ImGui::IsItemActivated();
    active |= ImGui::IsItemActive();
    changed |= ImGui::Checkbox("Visible", &staged.visible);
    activated |= ImGui::IsItemActivated();
    active |= ImGui::IsItemActive();
    if (activated) { m_PendingUndo = original; m_bUndoGestureSaved = false; }
    if (changed && m_Edit.Apply_Transform(selected.placementId, staged))
    {
        if (!m_bUndoGestureSaved)
        {
            if (m_Undo.size() == 64u) m_Undo.erase(m_Undo.begin());
            m_Undo.push_back(m_PendingUndo ? *m_PendingUndo : original);
            m_bUndoGestureSaved = true; m_PendingUndo.reset();
        }
        m_MapSelection->hitPosition = staged.position;
    }
    if (!active && !changed) { m_PendingUndo.reset(); m_bUndoGestureSaved = false; }
    ImGui::BeginDisabled(m_Undo.empty());
    if (ImGui::Button("Undo transform") && m_Edit.Apply_Transform(selected.placementId, m_Undo.back()))
    { m_MapSelection->hitPosition = m_Undo.back().position; m_Undo.pop_back(); }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Reset selection pose") && m_SelectedOriginal && m_Edit.Apply_Transform(selected.placementId, *m_SelectedOriginal))
    { m_MapSelection->hitPosition = m_SelectedOriginal->position; m_Undo.clear(); }
    if (ImGui::Button("Duplicate") && m_Edit.Duplicate_Selected())
    { Select_Map(m_Edit.Get_SelectedPlacementId()); m_bRowsDirty = true; }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_Edit.Is_SessionCreated(selected.placementId));
    if (ImGui::Button("Delete duplicate") && m_Edit.Delete_Selected())
    { m_MapSelection.reset(); m_bRowsDirty = true; }
    ImGui::EndDisabled(); ImGui::EndDisabled();
}
void CWorldSceneTool::Render()
{
    if (!m_bOpen) return;
    ImGui::SetNextWindowSize(ImVec2(780.f, 820.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("World Scene Tool", &m_bOpen)) { ImGui::End(); return; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_bInteraction = true;
    auto* host = Host();
    if (!host)
    { ImGui::TextWrapped("Enter Bern, Character Select, Valtan or KoukuSaydon to inspect its live models."); ImGui::End(); return; }
    ImGui::Text("%s | %s", host->Get_MapAuthoringLabel(), m_AreaId.c_str());
    auto& runtime = host->Get_MapAuthoringRuntime();
    const auto& chunkStats = runtime.Get_ChunkBuildStats();
    if (chunkStats.chunks && ImGui::CollapsingHeader("Spatial chunks / HLOD"))
    {
        const auto& policy = runtime.Get_ChunkPolicy();
        if (policy)
        {
            ImGui::Checkbox("Merged chunk draws", &policy->enabled);
            ImGui::SameLine();
            ImGui::Checkbox("Distant HLOD", &policy->hlodEnabled);
        }
        ImGui::Text("Prepared %u chunks from %u source draws; %u support HLOD", chunkStats.chunks,
            chunkStats.sourceDraws, chunkStats.farChunks);
        ImGui::Text("Geometry %.1f MiB | preparation %.0f ms", double(chunkStats.gpuBytes) / 1048576., chunkStats.buildMilliseconds);
        ImGui::TextWrapped("Compare original draws, merged draws, then HLOD at the same camera. Reset F7 capture for each mode. Moving or hiding a placement restores its affected chunks to original draws until the map reloads.");
    }
    if (ImGui::Button("Pick in scene")) m_bPickRequested = true;
    ImGui::SameLine();
    float3_t selectedHit{}; const bool selection = Try_GetSelectedHit(selectedHit);
    ImGui::BeginDisabled(!selection);
    if (ImGui::Button("Focus")) { m_FocusPosition = selectedHit; m_FocusRadius = 8.f; m_bFocusRequested = true; }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Refresh list")) { m_bRowsDirty = true; Refresh_Rows(); }
    ImGui::TextDisabled("LOD0 triangles / current skeletal pose. Alpha holes and shader displacement can differ.");
    Render_Rows();
    if (!m_Edit.Is_Bound())
    {
        ImGui::BeginDisabled(m_Edit.Is_Dirty() || m_Edit.Is_Publishing());
        if (ImGui::Button("Enable map placement editing")) (void)Begin_Editing();
        ImGui::EndDisabled();
        if (m_Edit.Is_Dirty())
        {
            ImGui::TextWrapped("A detached unsaved draft for %s is preserved.", m_Edit.Get_AreaId().c_str());
            if (ImGui::Button("Discard detached draft...")) ImGui::OpenPopup("Discard detached draft");
            if (ImGui::BeginPopupModal("Discard detached draft", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped("Discard only the preserved in-memory placement draft? Saved Data stays unchanged.");
                if (ImGui::Button("Discard draft")) { (void)m_Edit.Discard_DetachedDraft(); ImGui::CloseCurrentPopup(); }
                ImGui::SameLine(); if (ImGui::Button("Keep draft")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
            }
        }
    }
    else
    {
        ImGui::BeginDisabled(!m_Edit.Is_Dirty() || m_Edit.Is_ReadOnly() || m_Edit.Is_Publishing());
        if (ImGui::Button("Save Data + publish placements")) (void)m_Edit.Save();
        ImGui::EndDisabled(); ImGui::SameLine();
        ImGui::TextUnformatted(m_Edit.Is_Publishing() ? "Publishing..." : m_Edit.Is_Dirty() ? "Unsaved changes" : "Saved baseline");
    }
    ImGui::Separator();
    if (m_bDeploySelected) Render_DeployDetails(); else Render_MapDetails();
    Render_AnimationControls();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    if (!m_Edit.Get_Status().empty()) ImGui::TextWrapped("%s", m_Edit.Get_Status().c_str());
    ImGui::End();
}
}
#endif
