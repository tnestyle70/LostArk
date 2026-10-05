#include "imgui.h"
#include "WorldSceneTool.h"

#ifdef _DEBUG
#include "DataJson.h"
#include "GameInstance.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#include "MainApp.h"
#include "MapAuthoringHost.h"
#include "MapAssetObject.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
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
std::shared_ptr<Engine::CModel> MapModel(const MAP_RUNTIME_PLACED_ENTRY& entry)
{
    if (entry.object) return std::dynamic_pointer_cast<Engine::CModel>(entry.object->Get_Component(L"Com_Model"));
    if (entry.batch) return std::dynamic_pointer_cast<Engine::CModel>(entry.batch->Get_Component(L"Com_Model"));
    return nullptr;
}
bool BoundedPosition(const float3_t& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
        std::abs(value.x) <= 1e6f && std::abs(value.y) <= 1e6f && std::abs(value.z) <= 1e6f;
}
bool FocusBounds(const float3_t& minimum, const float3_t& maximum, float3_t& center, float& radius)
{
    if (!BoundedPosition(minimum) || !BoundedPosition(maximum) ||
        minimum.x > maximum.x || minimum.y > maximum.y || minimum.z > maximum.z) return false;
    const double x = (double(maximum.x) - minimum.x) * .5;
    const double y = (double(maximum.y) - minimum.y) * .5;
    const double z = (double(maximum.z) - minimum.z) * .5;
    const double length = std::sqrt(x * x + y * y + z * z);
    if (!std::isfinite(length)) return false;
    center = {float(double(minimum.x) + x), float(double(minimum.y) + y), float(double(minimum.z) + z)};
    radius = float(std::clamp(length, 2., 200.));
    return true;
}
const char* SurfaceFamily(Engine::MODEL_SURFACE_FAMILY family)
{
    using F = Engine::MODEL_SURFACE_FAMILY;
    switch (family)
    {
    case F::LEGACY: return "Legacy";
    case F::SPECULAR_TEXTURE_REFLECTION: return "Specular texture reflection";
    case F::DIFFUSE_SPECULAR_REFLECTION: return "Diffuse specular reflection";
    case F::PBR_SEAMLESS_OPAQUE: return "PBR seamless opaque";
    case F::PBR_OPAQUE: return "PBR opaque";
    case F::SOURCE_SPECULAR_OPAQUE: return "Source specular opaque";
    case F::SOURCE_CHARACTER: return "Source character";
    case F::SOURCE_OVERLAY_OPAQUE: return "Source overlay opaque";
    case F::SOURCE_BG_OPAQUE_MASKED: return "Source BG opaque / masked";
    case F::SOURCE_FOLIAGE_MASKED: return "Source foliage masked";
    case F::SOURCE_GRASS_MASKED: return "Source grass masked";
    case F::SOURCE_SNOWICE_OPAQUE: return "Source snow / ice opaque";
    case F::SOURCE_VERTEXBLEND_OPAQUE: return "Source vertex blend opaque";
    case F::SOURCE_WET_OPAQUE: return "Source wet opaque";
    case F::SOURCE_LANDSCAPE_OPAQUE: return "Source landscape opaque";
    default: return "Unknown";
    }
}
const char* RenderMode(MAP_ASSET_RENDER_MODE mode)
{
    switch (mode)
    {
    case MAP_ASSET_RENDER_MODE::DEFERRED: return "Deferred";
    case MAP_ASSET_RENDER_MODE::TRANSLUCENT: return "Translucent";
    case MAP_ASSET_RENDER_MODE::BACKGROUND: return "Background";
    case MAP_ASSET_RENDER_MODE::ADDITIVE: return "Additive";
    case MAP_ASSET_RENDER_MODE::WATER: return "Water";
    default: return "Unknown";
    }
}
const char* SurfaceRenderMode(Engine::MODEL_SURFACE_RENDER_MODE mode)
{
    using M = Engine::MODEL_SURFACE_RENDER_MODE;
    switch (mode)
    {
    case M::INHERIT: return "Inherit catalog";
    case M::DEFERRED: return "Deferred";
    case M::TRANSLUCENT: return "Translucent";
    case M::BACKGROUND: return "Background";
    case M::ADDITIVE: return "Additive";
    case M::WATER: return "Water";
    default: return "Unknown";
    }
}
std::string ResourceId(const std::filesystem::path& path)
{
    if (path.empty()) return "unknown / no typed override";
    const auto relative = path.is_absolute() ? path.lexically_normal().lexically_relative(
        CRuntimeAssetRoot::Get_ResourceRoot().lexically_normal()) : path.lexically_normal();
    if (relative.empty() || relative.is_absolute() || relative.has_root_name()) return "unknown (outside Resources)";
    for (const auto& part : relative) if (part == L"..") return "unknown (outside Resources)";
    return relative.generic_string();
}
void TextureId(const char* label, const std::filesystem::path& path)
{
    ImGui::TextWrapped("%s: %s", label, ResourceId(path).c_str());
}
}

CWorldSceneTool::CWorldSceneTool() = default;
CWorldSceneTool::~CWorldSceneTool() { Stop_AnimationPreview(); Stop_SelfMotionPreview(); }
IMapAuthoringHost* CWorldSceneTool::Host() const
{
    auto* host = Find_ActiveMapAuthoringHost();
    return host && host->Get_MapAuthoringLevelIndex() == m_LevelIndex &&
        host->Get_MapAuthoringCatalog().Get_AreaId() == m_AreaId &&
        host->Get_MapAuthoringRuntime().Debug_GetRuntimeGeneration() == m_RuntimeGeneration ? host : nullptr;
}
void CWorldSceneTool::Open() { m_bOpen = true; m_bRowsDirty = true; }
void CWorldSceneTool::Hide()
{
    m_bOpen = false; m_bPickRequested = false; m_bFocusRequested = false;
    Stop_AnimationPreview(); Stop_SelfMotionPreview();
}
bool CWorldSceneTool::Release_PlacementEditing(std::string& status)
{
    if (m_Edit.Is_Publishing())
    {
        status = "Object Details is publishing placements. Wait for completion before opening Map Tool; its draft is preserved.";
        return false;
    }
    if (m_Edit.Is_Dirty())
    {
        status = "Object Details has unsaved placement edits. Save them before opening Map Tool; its draft is preserved.";
        return false;
    }
    m_Edit.End();
    m_SelectedOriginal.reset();
    Hide();
    status = "Clean Object Details placement session released; its preview was restored.";
    return true;
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
        m_InspectMeshIndex = UINT32_MAX;
        m_Edit.End_EditGesture();
        m_bPickRequested = false; m_bFocusRequested = false;
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
    if (!Host()) return false;
    position = m_FocusPosition; radius = m_FocusRadius; return true;
}
bool CWorldSceneTool::Get_SelectedPlacement(uint64_t& placementId, bool& deploy) const
{
    auto* host = Host(); if (!host) return false;
    if (m_bDeploySelected && m_DeploySelection)
    {
        const auto* runtime = host->Get_MapAuthoringDeployRuntime();
        const auto id = m_DeploySelection->runtimePlacementId;
        if (!runtime || !runtime->Find(id)) return false;
        placementId = id; deploy = true; return true;
    }
    if (!m_MapSelection) return false;
    const auto id = m_MapSelection->placementId;
    const auto& placements = host->Get_MapAuthoringPlacements();
    if (std::none_of(placements.begin(), placements.end(),
        [id](const auto& entry) { return entry.record.placementId == id; })) return false;
    placementId = id; deploy = false; return true;
}
bool CWorldSceneTool::Request_SelectedFocus()
{
    uint64_t id = 0u; bool deploy = false;
    if (!Get_SelectedPlacement(id, deploy)) return false;
    auto* host = Host(); if (!host) return false;
    float3_t center{}; float radius = 8.f; bool hasBounds = false;
    if (deploy)
    {
        const auto object = host->Get_MapAuthoringDeployRuntime()->Find(id);
        if (!object) return false;
        float3_t root{}, half{}; float4_t rotation{}; float scale = 1.f;
        const bool hasRoot = object->Try_GetRenderedRootPose(root, rotation, scale) && BoundedPosition(root);
        if (object->Get_WorldBounds(center, half) && BoundedPosition(center) && BoundedPosition(half) &&
            half.x >= 0.f && half.y >= 0.f && half.z >= 0.f)
        {
            // Get_WorldBounds retains the source scale. The preview root exposes
            // its actual positive uniform scale without changing that source.
            const auto& entries = host->Get_MapAuthoringDeployRuntime()->Get_Entries();
            const auto entry = std::find_if(entries.begin(), entries.end(),
                [id](const auto& value) { return value.placement.runtimePlacementId == id; });
            if (hasRoot && entry != entries.end() && std::isfinite(entry->placement.uniformScale) &&
                entry->placement.uniformScale > 0.f && std::isfinite(scale) && scale > 0.f)
            {
                const float factor = scale / entry->placement.uniformScale;
                center = {root.x + (center.x - root.x) * factor, root.y + (center.y - root.y) * factor,
                    root.z + (center.z - root.z) * factor};
                half = {half.x * factor, half.y * factor, half.z * factor};
            }
            hasBounds = FocusBounds({center.x - half.x, center.y - half.y, center.z - half.z},
                {center.x + half.x, center.y + half.y, center.z + half.z}, center, radius);
        }
        if (!hasBounds)
        {
            if (!hasRoot) return false;
            center = root;
        }
    }
    else
    {
        const auto& entries = host->Get_MapAuthoringPlacements();
        const auto found = std::find_if(entries.begin(), entries.end(),
            [id](const auto& entry) { return entry.record.placementId == id; });
        if (found == entries.end()) return false;
        center = found->record.position;
        const auto* asset = host->Get_MapAuthoringCatalog().Find(found->record.assetId);
        float3_t minimum{}, maximum{};
        // Apply_Transform writes the draft into this live record. Sequence and
        // self-motion sampling also update it; no stale triangle hit is reused.
        if (asset && CMapPlacementRuntime::Try_Get_PlacementWorldBounds(*asset, MapModel(*found),
            found->record, minimum, maximum)) hasBounds = FocusBounds(minimum, maximum, center, radius);
        if (!hasBounds && !BoundedPosition(center)) return false;
    }
    m_FocusPosition = center; m_FocusRadius = radius; m_bFocusRequested = true; m_bInteraction = true;
    return true;
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
    if (!m_bDeploySelected && m_MapSelection && m_MapSelection->placementId == id) return;
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
    m_InspectMeshIndex = UINT32_MAX;
    m_Edit.Select_Placement(id); m_Edit.End_EditGesture();
    const auto* draft = m_Edit.Is_Bound() ? m_Edit.Find_Draft(id) : nullptr;
    m_SelectedOriginal = draft ? *draft : record;
}
void CWorldSceneTool::Complete_MapPick(MAP_WORLD_MESH_PICK selected)
{
    if (selected.areaId != m_AreaId) return;
    Select_Map(selected.placementId);
    m_InspectMeshIndex = selected.meshIndex;
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
    if (m_bDeploySelected && m_DeploySelection && m_DeploySelection->runtimePlacementId == id) return;
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
    m_InspectMeshIndex = UINT32_MAX;
    m_Edit.Clear_Selection(); m_SelectedOriginal.reset();
}
void CWorldSceneTool::Complete_DeployPick(DEPLOY_WORLD_MESH_PICK selected)
{
    if (selected.areaId != m_AreaId) return;
    Select_Deploy(selected.runtimePlacementId);
    m_InspectMeshIndex = selected.meshIndex;
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
    auto* app = CMainApp::Get_Active();
    if (!app) { m_Status = "The active application is unavailable; placement editing was not enabled."; return false; }
    if (!app->Debug_PrepareWorldPlacementEditing(m_AreaId, m_Status)) return false;
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
        if (m_MapSelection)
        {
            m_Edit.Select_Placement(m_MapSelection->placementId);
            if (const auto* draft = m_Edit.Find_Draft(m_MapSelection->placementId)) m_SelectedOriginal = *draft;
            m_Edit.End_EditGesture();
        }
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
    if (!m_Edit.Is_Bound())
    {
        if (auto* host = Host())
        {
            const auto& entries = host->Get_MapAuthoringPlacements();
            const auto found = std::find_if(entries.begin(), entries.end(),
                [&selected](const auto& entry) { return entry.record.placementId == selected.placementId; });
            if (found != entries.end())
            {
                auto pose = found->record;
                auto angles = EulerDegrees(pose.rotationQuaternion);
                ImGui::BeginDisabled();
                ImGui::DragFloat3("Position (m)", &pose.position.x, .05f);
                ImGui::DragFloat3("Rotation (deg pitch/yaw/roll)", &angles.x, .5f);
                ImGui::DragFloat3("Signed scale", &pose.signedScale.x, .01f);
                ImGui::EndDisabled();
                ImGui::TextDisabled("Enable map placement editing above to edit and save this pose.");
            }
        }
        return;
    }
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
    if (activated) m_Edit.Begin_EditGesture();
    if (changed && m_Edit.Apply_Transform(selected.placementId, staged))
        m_MapSelection->hitPosition = staged.position;
    if (!active) m_Edit.End_EditGesture();
    if (ImGui::Button("Reset selection pose") && m_SelectedOriginal && m_Edit.Apply_Transform(selected.placementId, *m_SelectedOriginal))
        m_MapSelection->hitPosition = m_SelectedOriginal->position;
    if (ImGui::Button("Duplicate") && m_Edit.Duplicate_Selected())
    { Select_Map(m_Edit.Get_SelectedPlacementId()); m_bRowsDirty = true; }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_Edit.Is_SessionCreated(selected.placementId));
    if (ImGui::Button("Delete duplicate") && m_Edit.Delete_Selected())
    { m_MapSelection.reset(); m_bRowsDirty = true; }
    ImGui::EndDisabled(); ImGui::EndDisabled();
}
void CWorldSceneTool::Render_MaterialDetails()
{
    uint64_t id = 0u; bool deploy = false;
    if (!Get_SelectedPlacement(id, deploy) || !ImGui::CollapsingHeader("Rendering / materials (read only)",
        ImGuiTreeNodeFlags_DefaultOpen)) return;
    auto* host = Host(); if (!host) return;
    const MAP_ASSET_ENTRY* asset = nullptr;
    const MAP_PLACEMENT_RECORD* record = nullptr;
    std::shared_ptr<Engine::CModel> model;
    if (deploy)
    {
        const auto object = host->Get_MapAuthoringDeployRuntime()->Find(id);
        if (!object) return;
        if (object->Get_State() == DEPLOY_PROP_STATE::FRACTURED)
            model = std::dynamic_pointer_cast<Engine::CModel>(object->Get_Component(L"Com_Model_Fractured"));
        if (!model) model = std::dynamic_pointer_cast<Engine::CModel>(object->Get_Component(L"Com_Model_Intact"));
        ImGui::TextDisabled("Deploy live model; permanent source placement is preserved.");
    }
    else
    {
        const auto& entries = host->Get_MapAuthoringPlacements();
        const auto entry = std::find_if(entries.begin(), entries.end(),
            [id](const auto& value) { return value.record.placementId == id; });
        if (entry == entries.end()) return;
        record = &entry->record;
        asset = host->Get_MapAuthoringCatalog().Find(record->assetId);
        model = MapModel(*entry);
        ImGui::Text("Runtime carrier: %s", entry->object ? "Map object" : entry->batch ? "Map instance batch" : "unknown");
        if (asset)
        {
            ImGui::TextWrapped("WModel: %s", asset->modelRelativePath.generic_string().c_str());
            ImGui::Text("Catalog render mode: %s | Cull policy: %s", RenderMode(asset->renderProfile.renderMode),
                asset->renderProfile.cullMode == MAP_ASSET_CULL_MODE::TWO_SIDED ? "Two sided" :
                asset->renderProfile.cullMode == MAP_ASSET_CULL_MODE::CULL_FRONT ? "Front" : "Back");
            ImGui::Text("Catalog UV scale %.4f / %.4f | speed %.4f / %.4f",
                asset->renderProfile.uvScale.x, asset->renderProfile.uvScale.y,
                asset->renderProfile.uvSpeed.x, asset->renderProfile.uvSpeed.y);
        }
    }
    if (!model || model->Get_NumMeshes() == 0u)
    { ImGui::TextDisabled("Live mesh/material information: unknown (model unavailable)."); return; }
    auto& mesh = m_InspectMeshIndex;
    const bool exactPick = (deploy ? m_DeploySelection->meshIndex : m_MapSelection->meshIndex) != UINT32_MAX;
    if (mesh >= model->Get_NumMeshes()) mesh = 0u;
    const auto preview = "Mesh " + std::to_string(mesh) + " | " + model->Get_MaterialName(mesh);
    if (ImGui::BeginCombo("Material mesh", preview.c_str()))
    {
        for (uint32_t i = 0u; i < model->Get_NumMeshes(); ++i)
        {
            const auto label = "Mesh " + std::to_string(i) + " | " + model->Get_MaterialName(i);
            if (ImGui::Selectable(label.c_str(), i == mesh)) mesh = i;
            if (i == mesh) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (!exactPick) ImGui::TextDisabled("Placement selected. This mesh is a read-only inspection choice; Pick in world identifies a hit.");
    const auto& materialName = model->Get_MaterialName(mesh);
    uint32_t slot = 0u;
    if (model->Try_GetSourceMaterialIndex(mesh, slot)) ImGui::Text("WModel source material slot: %u", slot);
    else ImGui::TextDisabled("WModel source material slot: unknown");
    ImGui::TextWrapped("Live CMaterial name: %s", materialName.empty() ? "unknown" : materialName.c_str());
    const Engine::MODEL_MATERIAL_OVERRIDE* typed = nullptr;
    if (asset)
    {
        const auto match = std::find_if(asset->materialOverrides.begin(), asset->materialOverrides.end(),
            [&materialName](const auto& value) { return value.materialName == materialName; });
        if (match != asset->materialOverrides.end()) typed = &*match;
    }
    ImGui::TextWrapped("Material source: %s", typed ?
        "WModel slot plus typed catalog override matched by exact material name" :
        "Inherited WModel material; no matching typed catalog override exposed here");
    ImGui::TextDisabled("Original source shader name / material parent: unknown (not retained by live catalog).");
    const auto* surface = model->Get_MaterialSurface(mesh);
    if (surface)
    {
        ImGui::Text("Live surface family: %s", SurfaceFamily(surface->family));
        ImGui::Text("Scene source-material option (read only): %s",
            CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials ? "enabled" : "disabled");
        ImGui::Text("Surface render mode: %s | Shadow declaration: %s",
            SurfaceRenderMode(surface->renderMode), surface->castsShadow ? "yes" : "no");
        ImGui::Text("Surface cull policy: %s",
            surface->cullMode == Engine::MODEL_SURFACE_CULL_MODE::INHERIT ? "Inherit catalog" :
            surface->cullMode == Engine::MODEL_SURFACE_CULL_MODE::TWO_SIDED ? "Two sided" :
            surface->cullMode == Engine::MODEL_SURFACE_CULL_MODE::CULL_FRONT ? "Front" : "Back");
        ImGui::Text("Source program: %u | BG flags: 0x%X | Foliage flags: 0x%X",
            surface->sourceCharacter.program, surface->sourceBgFlags, surface->sourceFoliageFlags);
        ImGui::Text("Surface UV tiling %.4f / %.4f | BG panning %.4f / %.4f",
            surface->uvTiling.x, surface->uvTiling.y, surface->sourceBgPanning.x, surface->sourceBgPanning.y);
        ImGui::Text("Foliage wind input: %s | Program: %u", surface->sourceFoliageWind ? "enabled" : "disabled",
            surface->sourceFoliageWindProgram);
        if (surface->sourceFoliageWind)
        {
            const auto& wind = surface->sourceFoliageWindDirectionSpeed;
            ImGui::Text("Wind direction / speed: %.4f / %.4f / %.4f / %.4f", wind.x, wind.y, wind.z, wind.w);
            const auto* instance = record ? host->Get_MapAuthoringCatalog().Find_PlacementWind(record->sourcePlacementId) : nullptr;
            if (instance)
            {
                const auto& position = instance->inputs.actorPositionSourceCm;
                const auto& dimensions = instance->inputs.objectDimensionsAndRadiusSourceCm;
                ImGui::Text("Placement source wind actor (cm): %.3f / %.3f / %.3f | admitted %.0f",
                    position.x, position.y, position.z, position.w);
                ImGui::Text("Source dimensions / radius (cm): %.3f / %.3f / %.3f / %.3f",
                    dimensions.x, dimensions.y, dimensions.z, dimensions.w);
            }
            else ImGui::TextDisabled("Placement wind carrier: unknown / no sidecar; material fallback inputs retained.");
        }
        ImGui::Text("Lighting declarations: RNM %s | static shadow %s | environment cube %s | source indirect %s",
            surface->hasBakedLighting ? "yes" : "no", surface->hasStaticShadow ? "yes" : "no",
            surface->hasEnvironmentCube ? "yes" : "no", surface->hasSourceIndirect ? "yes" : "no");
    }
    else ImGui::TextDisabled("Live surface policy: unknown");
    if (model->Has_MaterialTextureOverrides(mesh))
        ImGui::TextWrapped("Runtime texture replacement is active; typed IDs below describe catalog inputs and may be replaced.");
    if (ImGui::TreeNode("Typed texture IDs (Resources relative)"))
    {
        if (typed)
        {
            TextureId("Diffuse", typed->surfaceDiffusePath); TextureId("Normal", typed->surfaceNormalPath);
            TextureId("Specular", typed->surfaceSpecularPath); TextureId("ORM", typed->surfaceORMPath);
            TextureId("Emissive", typed->surfaceEmissivePath); TextureId("Reflection", typed->reflectionPath);
            TextureId("Detail normal", typed->detailNormalPath); TextureId("Foliage mask", typed->sourceFoliageMaskPath);
            TextureId("Overlay diffuse", typed->overlayDiffusePath); TextureId("Overlay normal", typed->overlayNormalPath);
            TextureId("Special mask", typed->sourceSpecialMaskPath);
            TextureId("Blend G diffuse", typed->sourceBlendDiffuseGPath); TextureId("Blend B diffuse", typed->sourceBlendDiffuseBPath);
            TextureId("Blend G normal", typed->sourceBlendNormalGPath); TextureId("Blend B normal", typed->sourceBlendNormalBPath);
            TextureId("RNM average", typed->bakedAveragePath); TextureId("RNM directional", typed->bakedDirectionalPath);
            TextureId("Static shadow", typed->staticShadowPath); TextureId("Environment cube", typed->environmentCubePath);
            TextureId("Environment BRDF", typed->environmentBRDFPath); TextureId("Source indirect cube", typed->sourceIndirectCubePath);
            TextureId("Source indirect BRDF", typed->sourceIndirectBRDFPath);
            for (uint32_t i = 0u; i < typed->sourceCharacterTextures.size(); ++i)
                if (!typed->sourceCharacterTextures[i].path.empty())
                    TextureId(("Source character t" + std::to_string(i)).c_str(), typed->sourceCharacterTextures[i].path);
            for (uint32_t i = 0u; i < Engine::SOURCE_LANDSCAPE_LAYER_COUNT; ++i)
            {
                if (!typed->sourceLandscapeTextures.diffuse[i].empty())
                    TextureId(("Landscape diffuse " + std::to_string(i)).c_str(), typed->sourceLandscapeTextures.diffuse[i]);
                if (!typed->sourceLandscapeTextures.normal[i].empty())
                    TextureId(("Landscape normal " + std::to_string(i)).c_str(), typed->sourceLandscapeTextures.normal[i]);
            }
            for (uint32_t i = 0u; i < Engine::SOURCE_LANDSCAPE_WEIGHTMAP_COUNT; ++i)
                if (!typed->sourceLandscapeTextures.weightmaps[i].empty())
                    TextureId(("Landscape weightmap " + std::to_string(i)).c_str(), typed->sourceLandscapeTextures.weightmaps[i]);
            if (!typed->sourceLandscapeTextures.heightmap.empty()) TextureId("Landscape heightmap", typed->sourceLandscapeTextures.heightmap);
        }
        else ImGui::TextDisabled("Typed texture IDs: unknown / no matching override");
        ImGui::TextWrapped("Inherited WModel texture IDs are unknown through the live API; an absent override does not mean a missing texture.");
        ImGui::Text("Legacy WModel slots present: diffuse %s | normal %s | specular %s | opacity %s",
            model->Has_MaterialTexture(mesh, aiTextureType_DIFFUSE) ? "yes" : "no",
            model->Has_MaterialTexture(mesh, aiTextureType_NORMALS) ? "yes" : "no",
            model->Has_MaterialTexture(mesh, aiTextureType_SPECULAR) ? "yes" : "no",
            model->Has_MaterialTexture(mesh, aiTextureType_OPACITY) ? "yes" : "no");
        ImGui::TreePop();
    }
    const auto* water = asset ? host->Get_MapAuthoringCatalog().Find_Water(asset->id) : nullptr;
    if (water && ImGui::TreeNode("Declared map water carrier"))
    {
        ImGui::TextWrapped("Source water material: %s", water->materialName.c_str());
        ImGui::Text("Opacity %.4f | Fresnel %.4f / %.4f | distortion %.4f", water->opacity,
            water->fresnelIntensity, water->fresnelPower, water->screenDistortionIntensity);
        ImGui::Text("Normal tiling / panning: %.4f / %.4f / %.4f / %.4f", water->normalTilingPanning.x,
            water->normalTilingPanning.y, water->normalTilingPanning.z, water->normalTilingPanning.w);
        TextureId("Declared detail normal", water->detailNormalTexture);
        TextureId("Declared reflection", water->reflectionTexture); TextureId("Declared foam", water->foamTexture);
        ImGui::TextWrapped("These auxiliary water texture IDs are declared metadata. The current water binder uses the model diffuse/normal; auxiliary SRVs are not connected.");
        ImGui::TreePop();
    }
    else if (!water) ImGui::TextDisabled("Map water carrier: no catalog row (not inferred from the asset name).");
    ImGui::TextWrapped("Read-only live material inputs. Current GPU pass, alpha coverage and shader displacement are not sampled; scene options can disable source programs.");
}
void CWorldSceneTool::Render_DetailsBody()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive() &&
        !ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyAlt && ImGui::IsKeyPressed(ImGuiKey_F, false))
        (void)Request_SelectedFocus();
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
    const bool historyInput = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive() && ImGui::GetIO().KeyCtrl;
    ImGui::BeginDisabled(m_bSelfMotionOwned || !m_Edit.Can_Undo());
    const bool undo = ImGui::Button("Undo") || (historyInput && !ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false));
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::BeginDisabled(m_bSelfMotionOwned || !m_Edit.Can_Redo());
    const bool redo = ImGui::Button("Redo") || (historyInput && (ImGui::IsKeyPressed(ImGuiKey_Y, false) ||
        (ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false))));
    ImGui::EndDisabled();
    if (!m_bSelfMotionOwned && ((undo && m_Edit.Undo()) || (redo && m_Edit.Redo())))
    {
        const auto id = m_Edit.Get_SelectedPlacementId();
        m_MapSelection.reset(); m_DeploySelection.reset(); m_bDeploySelected = false;
        if (id) Select_Map(id);
        m_bRowsDirty = true;
    }
    ImGui::Separator();
    if (m_bDeploySelected) Render_DeployDetails(); else Render_MapDetails();
    Render_MaterialDetails();
    Render_AnimationControls();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    if (!m_Edit.Get_Status().empty()) ImGui::TextWrapped("%s", m_Edit.Get_Status().c_str());
}
void CWorldSceneTool::Render_WorldDetails()
{
    if (!m_bOpen) return;
    ImGui::SetNextWindowSize(ImVec2(540.f, 760.f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Object Details", &m_bOpen))
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_bInteraction = true;
        if (auto* host = Host())
        {
            ImGui::Text("%s | %s", host->Get_MapAuthoringLabel(), m_AreaId.c_str());
            uint64_t placement = 0u; bool deploy = false;
            const bool selected = Get_SelectedPlacement(placement, deploy);
            if (ImGui::Button("Pick in world")) m_bPickRequested = true;
            ImGui::SameLine(); ImGui::BeginDisabled(!selected);
            if (ImGui::Button("Focus (F)")) (void)Request_SelectedFocus();
            ImGui::EndDisabled();
            if (!selected) ImGui::TextWrapped("Select a live object in the World Tree or use Pick in world.");
            Render_DetailsBody();
        }
        else ImGui::TextWrapped("Enter Bern, Character Select, Valtan or KoukuSaydon to inspect live objects.");
    }
    ImGui::End();
    if (!m_bOpen) Hide();
}
void CWorldSceneTool::Render()
{
    if (!m_bOpen) return;
    if (m_bDetailsMode) { Render_WorldDetails(); return; }
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
    uint64_t selectedId = 0u; bool selectedDeploy = false;
    const bool selection = Get_SelectedPlacement(selectedId, selectedDeploy);
    ImGui::BeginDisabled(!selection);
    if (ImGui::Button("Focus (F)")) (void)Request_SelectedFocus();
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Refresh list")) { m_bRowsDirty = true; Refresh_Rows(); }
    ImGui::TextDisabled("LOD0 triangles / current skeletal pose. Alpha holes and shader displacement can differ.");
    Render_Rows();
    Render_DetailsBody();
    ImGui::End();
}
}
#endif
