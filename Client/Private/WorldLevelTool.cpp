#include "imgui.h"
#include "WorldLevelTool.h"

#ifdef _DEBUG
#include "DataJson.h"
#include "DeployPropCatalog.h"
#include "MapAssetCatalog.h"
#include "MapAuthoringHost.h"
#include "MapPlacementDocument.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cfloat>
#include <cctype>
#include <cmath>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Client
{
namespace
{
std::string ReadString(const DATA_JSON_VALUE& value, const char* key)
{
    const auto* field = value.Find(key);
    return field && field->Is_String() ? field->Get_String() : std::string{};
}
uint32_t ReadTime(const DATA_JSON_VALUE& value, const char* key)
{
    const auto* field = value.Find(key);
    if (!field || !field->Is_Number() || !std::isfinite(field->Get_Number()) ||
        field->Get_Number() < 0. || field->Get_Number() > UINT32_MAX) return 0u;
    return static_cast<uint32_t>(field->Get_Number());
}
bool ReadPosition(const DATA_JSON_VALUE& value, const char* key, float3_t& position)
{
    const auto* field = value.Find(key);
    if (!field || !field->Is_Array() || field->Get_Array().size() != 3u) return false;
    float coordinates[3]{};
    for (size_t i = 0; i < 3u; ++i)
    {
        const auto& element = field->Get_Array()[i];
        if (!element.Is_Number() || !std::isfinite(element.Get_Number()) ||
            std::abs(element.Get_Number()) > FLT_MAX) return false;
        coordinates[i] = static_cast<float>(element.Get_Number());
    }
    position = {coordinates[0], coordinates[1], coordinates[2]};
    return true;
}
bool ReadJson(const std::filesystem::path& path, DATA_JSON_VALUE& root, std::string& status)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot read " + path.filename().string(); return false; }
    const std::string bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (!CDataJson::Parse(bytes, root, status) || !root.Is_Object())
    { status = path.filename().string() + ": " + status; return false; }
    return true;
}
bool SourcePath(const DATA_JSON_VALUE& row, const char* key, std::filesystem::path& result)
{
    const auto value = ReadString(row, key);
    if (value.empty()) { result.clear(); return true; }
    const auto relative = std::filesystem::path(value).lexically_normal();
    if (relative.is_absolute() || relative.has_root_name()) return false;
    auto part = relative.begin();
    if (part == relative.end() || *part != L"Data") return false;
    std::filesystem::path inside;
    for (++part; part != relative.end(); ++part)
    {
        if (*part == L"..") return false;
        inside /= *part;
    }
    if (inside.empty()) return false;
    result = CProjectDataRoot::Resolve(inside);
    return !result.empty();
}
std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return text;
}
float3_t Position(const std::array<double, 3u>& value)
{ return {static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2])}; }
const char* PresentationName(KOUKU_SAYDON_PRESENTATION_KIND kind)
{
    switch (kind)
    {
    case KOUKU_SAYDON_PRESENTATION_KIND::EFFECT: return "Effect";
    case KOUKU_SAYDON_PRESENTATION_KIND::LIGHT: return "Light";
    case KOUKU_SAYDON_PRESENTATION_KIND::CAMERA: return "Camera";
    case KOUKU_SAYDON_PRESENTATION_KIND::SOUND: return "Sound";
    case KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER: return "Collider";
    default: return "Presentation";
    }
}
}

CWorldLevelTool::CWorldLevelTool() = default;
CWorldLevelTool::~CWorldLevelTool() = default;

void CWorldLevelTool::Open(const std::string& activeAreaId)
{
    Set_ActiveArea(activeAreaId);
    m_Open = true;
    if (m_Areas.empty() && !Load_Areas()) return;
    if (m_SelectedAreaId.empty())
    {
        const auto found = std::find_if(m_Areas.begin(), m_Areas.end(),
            [&](const AREA& area) { return area.id == activeAreaId; });
        m_SelectedAreaId = found != m_Areas.end() ? found->id : m_Areas.front().id;
        Refresh();
    }
}

void CWorldLevelTool::Set_ActiveArea(const std::string& areaId)
{ m_ActiveAreaId = areaId; }

void CWorldLevelTool::Set_CompositionView(WORLD_LEVEL_COMPOSITION_OWNER owner,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT* document, uint64_t generation)
{
    auto& view = m_Compositions[static_cast<size_t>(owner)];
    if (view.source == document && view.generation == generation) return;
    view.source = document;
    view.generation = generation;
    if (document) view.document = *document;
    else view.document.reset();
    m_RowsDirty = true;
}

bool CWorldLevelTool::Load_Areas()
{
    DATA_JSON_VALUE root;
    if (!ReadJson(CProjectDataRoot::Resolve("Maps/MapCatalog.json"), root, m_Status)) return false;
    const auto* areas = root.Find("areas");
    if (ReadString(root, "schema") != "lostark.map-catalog" || !areas || !areas->Is_Array())
    { m_Status = "MapCatalog has no valid areas."; return false; }
    std::vector<AREA> staged;
    std::unordered_set<std::string> ids;
    for (const auto& item : areas->Get_Array())
    {
        AREA area;
        area.id = ReadString(item, "id");
        if (area.id.empty() || !ids.insert(area.id).second)
        { m_Status = "MapCatalog contains an empty or duplicate Area ID."; return false; }
        area.label = area.id;
        if (area.id == "LV_LUT_MIDNIGHTC_ED") area.label = "KoukuSaydon";
        else if (area.id == "LV_LOBBY_CLASSSELECT_SL00") area.label = "Character Select";
        else if (area.id == "LV_LUT_HEARTRB_ED") area.label = "Valtan";
        else if (area.id == "LV_BER_BERNCASTLE") area.label = "Bern";
        if (!SourcePath(item, "sourceCatalog", area.catalog) ||
            !SourcePath(item, "sourcePlacements", area.placements) ||
            !SourcePath(item, "sourceMaterials", area.materials) ||
            !SourcePath(item, "sourceDeployCatalog", area.deployCatalog) ||
            !SourcePath(item, "sourceDeployPlacements", area.deployPlacements) ||
            !SourcePath(item, "sourceSequences", area.sequences) ||
            !SourcePath(item, "gameplayDocument", area.gameplay) ||
            !SourcePath(item, "sourceLights", area.lights) ||
            !SourcePath(item, "sourceEffects", area.effects) || area.catalog.empty() || area.placements.empty())
        { m_Status = "MapCatalog source path is invalid: " + area.id; return false; }
        staged.push_back(std::move(area));
    }
    if (staged.empty()) { m_Status = "MapCatalog contains no areas."; return false; }
    m_Areas = std::move(staged);
    return true;
}

bool CWorldLevelTool::Refresh()
{
    if (m_Hierarchy.Is_Dirty())
    { m_Status = "Save the Parent hierarchy or Undo its changes before refreshing or changing Area."; return false; }
    const auto selected = std::find_if(m_Areas.begin(), m_Areas.end(),
        [&](const AREA& area) { return area.id == m_SelectedAreaId; });
    if (selected == m_Areas.end()) return false;
    const AREA& area = *selected;
    std::vector<ROW> staged;
    CMapAssetCatalog catalog;
    std::vector<MAP_PLACEMENT_RECORD> placements;
    if (!catalog.Load_SourceMetadata(area.catalog, area.placements, area.id, area.materials))
    { m_Status = catalog.Get_Status() + "; previous inventory preserved."; return false; }
    if (!CMapPlacementDocument::Read(area.placements, catalog, placements, m_Status)) return false;
    std::map<std::string, float3_t> mapPositions, deployPositions;
    for (const auto& placement : placements)
    {
        ROW row;
        row.key = "map:" + std::to_string(placement.placementId);
        row.kind = "Map";
        const auto* asset = catalog.Find(placement.assetId);
        row.name = asset ? asset->label : placement.assetId;
        row.assetId = placement.assetId;
        row.anchor = "MAP";
        row.hasPosition = true;
        row.visible = placement.visible;
        row.target.areaId = area.id;
        row.target.placementId = placement.placementId;
        row.target.position = placement.position;
        row.status = placement.sourcePlacementId + " | source level " + placement.sourceLevel;
        row.sourceLevel = placement.sourceLevel;
        mapPositions.emplace(std::to_string(placement.placementId), placement.position);
        staged.push_back(std::move(row));
    }
    if (!area.deployCatalog.empty() && !area.deployPlacements.empty())
    {
        CDeployPropCatalog deploy;
        if (!deploy.Load(area.deployCatalog, area.deployPlacements, area.id))
        { m_Status = deploy.Get_Status() + "; previous inventory preserved."; return false; }
        for (const auto& placement : deploy.Get_Placements())
        {
            ROW row;
            row.key = "deploy:" + std::to_string(placement.runtimePlacementId);
            row.kind = "Deploy";
            const auto* asset = deploy.Find(placement.assetId);
            row.name = asset ? asset->label : placement.assetId;
            row.assetId = placement.assetId;
            row.anchor = "MAP";
            row.hasPosition = true;
            row.target.areaId = area.id;
            row.target.placementId = placement.runtimePlacementId;
            row.target.deploy = true;
            row.target.position = placement.position;
            row.status = placement.sourcePlacementId;
            row.sourceLevel = placement.sourcePlacementId.substr(0, placement.sourcePlacementId.find(':'));
            row.canEdit = asset != nullptr;
            if (placement.provenance == DEPLOY_PROP_PLACEMENT_PROVENANCE::SOURCE_EXACT)
                row.status += " Source-exact placement: scene inspection and reversible preview; permanent placement is preserved.";
            deployPositions.emplace(std::to_string(placement.runtimePlacementId), placement.position);
            staged.push_back(std::move(row));
        }
    }
    // Inventory projection only. The owning document/tool performs validation
    // and runtime admission when the user opens or plays an item.
    if (!area.sequences.empty())
    {
        DATA_JSON_VALUE root;
        if (!ReadJson(area.sequences, root, m_Status)) return false;
        if (ReadString(root, "areaId") != area.id || ReadString(root, "schema") != "lostark.world-sequences")
        { m_Status = "World sequence inventory Area/schema mismatch."; return false; }
        std::map<std::string, const DATA_JSON_VALUE*> templates, objects;
        if (const auto* list = root.Find("templates"); list && list->Is_Array())
            for (const auto& item : list->Get_Array()) templates.emplace(ReadString(item, "sequenceId"), &item);
        if (const auto* list = root.Find("objectResources"); list && list->Is_Array())
            for (const auto& item : list->Get_Array()) objects.emplace(ReadString(item, "objectId"), &item);
        const auto* instances = root.Find("instances");
        if (!instances || !instances->Is_Array()) { m_Status = "World sequence instances are invalid."; return false; }
        for (const auto& item : instances->Get_Array())
        {
            ROW row;
            row.target.areaId = area.id;
            row.target.sequenceInstanceId = ReadString(item, "instanceId");
            row.key = "world:" + row.target.sequenceInstanceId;
            row.kind = "World Sequence";
            row.name = row.target.sequenceInstanceId;
            row.anchor = ReadString(item, "anchorKind");
            row.startMs = ReadTime(item, "startDelayMs");
            row.timed = true;
            if (const auto* enabled = item.Find("enabled"); enabled && enabled->Is_Boolean()) row.visible = enabled->Get_Boolean();
            const auto motion = templates.find(ReadString(item, "templateId"));
            if (motion != templates.end())
            { row.name = ReadString(*motion->second, "displayName"); row.durationMs = ReadTime(*motion->second, "durationMs"); }
            const auto* bindings = item.Find("bindings");
            size_t positionCount = 0u;
            if (bindings && bindings->Is_Array()) for (const auto& binding : bindings->Get_Array())
            {
                const auto kind = ReadString(binding, "targetKind");
                const auto id = ReadString(binding, "targetId");
                if (kind == "OBJECT_RESOURCE")
                {
                    row.target.objectId = id;
                    row.target.kind = WORLD_LEVEL_REQUEST_KIND::OPEN_WORLD_OBJECT;
                    const auto object = objects.find(id);
                    if (object != objects.end()) row.assetId = ReadString(*object->second, "modelAssetId");
                    if (row.anchor == "WORLD" && ReadPosition(item, "position", row.target.position)) ++positionCount;
                }
                else if (kind == "MAP_PLACEMENT" || kind == "DEPLOY_PLACEMENT")
                {
                    const auto& positions = kind == "MAP_PLACEMENT" ? mapPositions : deployPositions;
                    const auto position = positions.find(id);
                    if (position != positions.end()) { row.target.position = position->second; ++positionCount; }
                }
            }
            row.hasPosition = positionCount == 1u;
            row.status = positionCount > 1u ? "Multiple bound placements; open the sequence to select a target." :
                (row.hasPosition ? "Saved target origin; animation may move it." : "Dynamic anchor; open the owning editor to resolve its current position.");
            staged.push_back(std::move(row));
        }
    }
    const auto appendJsonLayer = [&](const std::filesystem::path& path, const char* listName,
        const char* idName, const char* kind) -> bool
    {
        if (path.empty()) return true;
        DATA_JSON_VALUE root;
        if (!ReadJson(path, root, m_Status)) return false;
        if (ReadString(root, "areaId") != area.id) { m_Status = "Layer Area mismatch: " + path.filename().string(); return false; }
        const auto* list = root.Find(listName);
        if (!list || !list->Is_Array()) { m_Status = "Invalid inventory layer: " + path.filename().string(); return false; }
        for (const auto& item : list->Get_Array())
        {
            ROW row;
            row.target.areaId = area.id;
            row.target.sourceItemId = ReadString(item, idName);
            row.key = std::string(kind) + ":" + row.target.sourceItemId;
            row.name = ReadString(item, "displayName");
            if (row.name.empty()) row.name = row.target.sourceItemId;
            row.kind = kind;
            if (row.kind == "Map Light") row.target.kind = WORLD_LEVEL_REQUEST_KIND::OPEN_LIGHT;
            if (row.kind == "Map Effect") row.canEdit = false;
            row.assetId = ReadString(item, "archetypeId");
            row.anchor = "MAP";
            row.hasPosition = ReadPosition(item, "position", row.target.position);
            if (const auto* enabled = item.Find("enabled"); enabled && enabled->Is_Boolean()) row.visible = enabled->Get_Boolean();
            row.status = ReadString(item, "kind");
            if (!row.hasPosition) row.status += " Owner-relative surface; open Map Tool for its binding.";
            if (!row.canEdit) row.status = "Read-only surface binding inventory. This layer has no placement editor; its source remains owned by the map effect publisher.";
            staged.push_back(std::move(row));
        }
        return true;
    };
    if (!appendJsonLayer(area.gameplay, "placements", "placementId", "Gameplay") ||
        !appendJsonLayer(area.lights, "lights", "lightId", "Map Light") ||
        !appendJsonLayer(area.effects, "presentations", "independentEffectId", "Map Effect")) return false;
    std::array<std::optional<KOUKU_SAYDON_COMPOSITION_DOCUMENT>, 2u> compositions;
    if (area.id == "LV_LUT_MIDNIGHTC_ED")
    {
        for (size_t i = 0u; i < compositions.size(); ++i)
        {
            const auto path = i == 0u ? CKoukuSaydonCompositionDocument::Resolve_Path() : CKoukuSaydonCompositionDocument::Resolve_SequencePath();
            CKoukuSaydonCompositionDocument document(path);
            if (!document.Reload(m_Status)) return false;
            compositions[i] = document.Get_LastGood();
        }
    }
    CWorldLevelHierarchy hierarchy;
    const auto hierarchyPath = CProjectDataRoot::Resolve("Maps/Authoring/WorldHierarchy/" + area.id + ".json");
    if (!hierarchy.Load(hierarchyPath, area.id, m_Status)) return false;
    m_Hierarchy = std::move(hierarchy);
    m_SavedRows = std::move(staged);
    m_SavedCompositions = std::move(compositions);
    m_RowsDirty = true;
    Rebuild_Rows();
    m_Status = "Saved inventory refreshed. Owner drafts and runtime placements were not reloaded.";
    return true;
}

void CWorldLevelTool::Append_CompositionRows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    WORLD_LEVEL_COMPOSITION_OWNER owner, bool draft)
{
    if (document.strAreaId != m_SelectedAreaId) return;
    const std::string prefix = owner == WORLD_LEVEL_COMPOSITION_OWNER::ACTION ? "action:" : "sequence:";
    for (const auto& pattern : document.Patterns)
    {
        ROW patternRow;
        patternRow.key = prefix + pattern.strPatternId;
        patternRow.name = pattern.strDisplayName;
        patternRow.kind = owner == WORLD_LEVEL_COMPOSITION_OWNER::ACTION ? "Action Pattern" : "Sequence";
        patternRow.target.kind = WORLD_LEVEL_REQUEST_KIND::OPEN_COMPOSITION;
        patternRow.target.areaId = document.strAreaId;
        patternRow.target.compositionOwner = owner;
        patternRow.target.patternId = pattern.strPatternId;
        patternRow.timed = true;
        patternRow.durationMs = pattern.iDurationMs;
        patternRow.status = draft ? "Current owner draft (Apply edits to include them here)." : "Saved Composition.";
        if (!pattern.strLoadError.empty()) patternRow.status = pattern.strLoadError;
        m_Rows.push_back(patternRow);
        for (const auto& box : pattern.PresentationOccurrences)
        {
            const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& entry) { return entry.strResourceId == box.strResourceId; });
            ROW row = patternRow;
            row.key = prefix + pattern.strPatternId + ":presentation:" + box.strOccurrenceId;
            row.target.occurrenceId = box.strOccurrenceId;
            row.name = resource == document.PresentationResources.end() ? box.strResourceId : resource->strDisplayName;
            row.kind = resource == document.PresentationResources.end() ? "Presentation" : PresentationName(resource->eKind);
            row.assetId = resource == document.PresentationResources.end() ? "" : resource->strAssetId;
            row.anchor = box.strAnchorKind;
            row.startMs = box.iStartMs;
            row.durationMs = box.iDurationMs;
            // Only MAP Effects own an absolute box pivot. MAP lights add the
            // box offset to the light resource; cameras have their own tracks.
            row.hasPosition = box.strAnchorKind == "MAP" &&
                resource != document.PresentationResources.end() && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
            if (row.hasPosition) row.target.position = Position(box.PositionOffset);
            else row.status += " Position is an anchor offset, not an absolute world point.";
            m_Rows.push_back(std::move(row));
        }
        for (const auto& box : pattern.WorldOccurrences)
        {
            ROW row = patternRow;
            row.key = prefix + pattern.strPatternId + ":world:" + box.strOccurrenceId;
            row.kind = "World Box";
            row.target.occurrenceId = box.strOccurrenceId;
            row.name = box.strWorldId;
            const auto resource = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& entry) { return entry.strWorldId == box.strWorldId; });
            if (resource != document.Worlds.end()) row.name = resource->strDisplayName;
            row.startMs = box.iStartMs;
            row.durationMs = box.iDurationMs;
            const auto motion = resource == document.Worlds.end() ? m_SavedRows.end() :
                std::find_if(m_SavedRows.begin(), m_SavedRows.end(), [&](const ROW& entry) {
                    return entry.kind == "World Sequence" && entry.target.sequenceInstanceId == resource->strSequenceInstanceId;
                });
            row.anchor = motion == m_SavedRows.end() ? "BOUND" : motion->anchor;
            row.hasPosition = box.Placement.has_value() && row.anchor == "WORLD";
            if (row.hasPosition) row.target.position = Position(box.Placement->Position);
            else row.status += " Relative or unresolved World anchor; open the owning editor to resolve its position.";
            m_Rows.push_back(std::move(row));
        }
    }
}

void CWorldLevelTool::Rebuild_Rows()
{
    m_Rows = m_SavedRows;
    for (size_t i = 0u; i < m_Compositions.size(); ++i)
    {
        const auto& live = m_Compositions[i].document;
        const bool useLive = live && live->strAreaId == m_SelectedAreaId;
        const auto& source = useLive ? live : m_SavedCompositions[i];
        if (source) Append_CompositionRows(*source, static_cast<WORLD_LEVEL_COMPOSITION_OWNER>(i), useLive);
    }
    m_RowsDirty = false;
    m_FilterDirty = true;
    m_TreeDirty = true;
}

void CWorldLevelTool::Request_Edit(const ROW& row)
{
    if (!row.canEdit) return;
    m_Request = row.target; m_InteractionRequested = true;
}
void CWorldLevelTool::Request_Focus(const ROW& row)
{
    if (!row.hasPosition || row.target.areaId != m_ActiveAreaId) return;
    m_Request = row.target;
    m_Request->kind = WORLD_LEVEL_REQUEST_KIND::FOCUS;
    m_InteractionRequested = true;
}
bool CWorldLevelTool::Consume_InteractionRequest()
{ return std::exchange(m_InteractionRequested, false); }
bool CWorldLevelTool::Consume_Request(WORLD_LEVEL_TOOL_REQUEST& request)
{
    if (!m_Request) return false;
    request = std::move(*m_Request);
    m_Request.reset();
    return true;
}

bool CWorldLevelTool::Needs_ChunkView(const std::string& area, const void* source, uint64_t generation) const
{
    return m_Open && (area != m_ChunkArea || source != m_ChunkSource || generation != m_ChunkGeneration ||
        std::chrono::steady_clock::now() >= m_ChunkNextRefresh);
}

void CWorldLevelTool::Set_ChunkView(std::string area, const void* source, uint64_t generation,
    std::vector<MAP_CHUNK_DEBUG_ROW> rows, bool enabled, bool hlodEnabled)
{
    if (area != m_ChunkArea || source != m_ChunkSource || generation != m_ChunkGeneration)
        m_SelectedChunk = UINT32_MAX;
    m_ChunkArea = std::move(area); m_ChunkSource = source; m_ChunkGeneration = generation;
    m_ChunkRows = std::move(rows); m_ChunkEnabled = enabled; m_ChunkHlodEnabled = hlodEnabled;
    if (std::none_of(m_ChunkRows.begin(), m_ChunkRows.end(),
        [&](const auto& row) { return row.chunkId == m_SelectedChunk; })) m_SelectedChunk = UINT32_MAX;
    m_ChunkNextRefresh = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
}

void CWorldLevelTool::Render_ChunkView()
{
    if (!ImGui::CollapsingHeader("Runtime chunks / HLOD")) return;
    if (!m_ChunkSource || m_ChunkArea != m_SelectedAreaId || m_ChunkArea != m_ActiveAreaId)
    { ImGui::TextDisabled("Enter this Area to inspect its live spatial chunks."); return; }
    bool modeChanged = ImGui::Checkbox("Merged chunk draws", &m_ChunkEnabled);
    ImGui::SameLine();
    modeChanged |= ImGui::Checkbox("Distant HLOD", &m_ChunkHlodEnabled);
    if (modeChanged)
    {
        WORLD_LEVEL_TOOL_REQUEST request;
        request.kind = WORLD_LEVEL_REQUEST_KIND::SET_CHUNK_MODE; request.areaId = m_ChunkArea;
        request.chunkEnabled = m_ChunkEnabled; request.chunkHlodEnabled = m_ChunkHlodEnabled;
        request.runtimeGeneration = m_ChunkGeneration; m_Request = std::move(request);
        m_InteractionRequested = true;
    }
    ImGui::TextWrapped("Performance controls above: Merged chunk draws changes rendering; Distant HLOD reduces distant geometry. Bounds controls below only draw debug lines.");
    ImGui::Checkbox("Show chunk bounds", &m_ShowChunkBounds); ImGui::SameLine();
    bool allBounds = Show_AllChunkBounds();
    ImGui::BeginDisabled(m_SelectedChunk == UINT32_MAX);
    if (ImGui::Checkbox("All chunks", &allBounds)) m_ShowAllChunkBounds = allBounds;
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Refresh chunks")) m_ChunkNextRefresh = {};
    if (m_ShowChunkBounds)
        ImGui::TextWrapped("Bounds: %s. Yellow = selected, green = HLOD, blue = other valid chunks, red = invalid. Close bounds and tools when capturing performance.",
            Show_AllChunkBounds() ? "all chunks (select a row to inspect one)" : "selected chunk only");
    uint32_t nearDraws = 0u, farDraws = 0u;
    uint64_t submittedIndices = 0u;
    for (const auto& row : m_ChunkRows) if (row.submitted)
    {
        if (row.farSelected) ++farDraws; else ++nearDraws;
        submittedIndices += row.farSelected ? row.farIndices : row.nearIndices;
    }
    ImGui::Text("Successful chunk draws: near %u / HLOD %u; indices %llu (one frame sampled every 250 ms)",
        nearDraws, farDraws, static_cast<unsigned long long>(submittedIndices));
    ImGui::TextWrapped("%zu prepared chunks. Each chunk groups sources with the same active shader inputs; the material name is representative, not the grouping key. Source groups are not measured saved draws. Not submitted includes camera culling. IDs last only for this loaded map. Bounds are a debug overlay, not an occlusion test.", m_ChunkRows.size());
    if (ImGui::BeginTable("WorldLevelChunks", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, 170.f)))
    {
        ImGui::TableSetupColumn("Chunk", ImGuiTableColumnFlags_WidthFixed, 65.f);
        ImGui::TableSetupColumn("Representative material");
        ImGui::TableSetupColumn("Source placements", ImGuiTableColumnFlags_WidthFixed, 125.f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 110.f);
        ImGui::TableSetupColumn("Near / far indices", ImGuiTableColumnFlags_WidthFixed, 150.f);
        ImGui::TableSetupScrollFreeze(0, 1); ImGui::TableHeadersRow();
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>(m_ChunkRows.size()));
        while (clipper.Step()) for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            const auto& row = m_ChunkRows[index]; ImGui::PushID(static_cast<int>(row.chunkId));
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            const std::string id = std::to_string(row.chunkId);
            if (ImGui::Selectable(id.c_str(), row.chunkId == m_SelectedChunk, ImGuiSelectableFlags_SpanAllColumns))
            { m_SelectedChunk = row.chunkId; m_ShowChunkBounds = true; }
            ImGui::TableNextColumn(); ImGui::TextUnformatted(row.materialName.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%zu (%u source groups)", row.placementIds.size(), row.sourceDraws);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(!row.valid ? "Invalid / source" : !row.active ? "Source fallback" : !row.submitted ? "Not submitted" : row.farSelected ? "HLOD" : "Merged near");
            ImGui::TableNextColumn(); ImGui::Text("%u / %u", row.nearIndices, row.farIndices);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    const auto selected = std::find_if(m_ChunkRows.begin(), m_ChunkRows.end(),
        [&](const auto& row) { return row.chunkId == m_SelectedChunk; });
    if (selected == m_ChunkRows.end()) return;
    const auto& row = *selected;
    ImGui::Text("Bounds: [%.2f, %.2f, %.2f] - [%.2f, %.2f, %.2f]", row.minimum.x, row.minimum.y,
        row.minimum.z, row.maximum.x, row.maximum.y, row.maximum.z);
    if (ImGui::Button("Focus selected chunk"))
    {
        WORLD_LEVEL_TOOL_REQUEST request; request.kind = WORLD_LEVEL_REQUEST_KIND::FOCUS;
        request.areaId = m_ChunkArea;
        request.position = {(row.minimum.x + row.maximum.x) * .5f,
            (row.minimum.y + row.maximum.y) * .5f, (row.minimum.z + row.maximum.z) * .5f};
        request.focusRadius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&row.maximum) - XMLoadFloat3(&request.position)));
        m_Request = std::move(request); m_InteractionRequested = true;
    }
    ImGui::TextWrapped("Sources in selected chunk: select a stable placement ID to inspect its original object in the existing owner.");
    if (ImGui::BeginTable("WorldLevelChunkSources", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, 105.f)))
    {
        ImGui::TableSetupColumn("Placement ID", ImGuiTableColumnFlags_WidthFixed, 180.f);
        ImGui::TableSetupColumn("Asset ID"); ImGui::TableHeadersRow();
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>(row.placementIds.size()));
        while (clipper.Step()) for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::PushID(index);
            const std::string id = std::to_string(row.placementIds[index]);
            if (ImGui::Selectable(id.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
            {
                WORLD_LEVEL_TOOL_REQUEST request; request.kind = WORLD_LEVEL_REQUEST_KIND::OPEN_MAP;
                request.areaId = m_ChunkArea; request.placementId = row.placementIds[index];
                m_Request = std::move(request); m_InteractionRequested = true;
            }
            ImGui::TableNextColumn();
            if (static_cast<size_t>(index) < row.assetIds.size()) ImGui::TextUnformatted(row.assetIds[index].c_str());
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}


const CWorldLevelTool::ROW* CWorldLevelTool::Selected_Row() const
{
    const auto found = m_RowIndex.find(m_SelectedKey);
    return found == m_RowIndex.end() ? nullptr : &m_Rows[found->second];
}

bool CWorldLevelTool::Row_Matches(const ROW& row, const std::string& search) const
{
    const bool kind = m_KindFilter == 0 || (m_KindFilter == 1 && row.kind == "Map") ||
        (m_KindFilter == 2 && row.kind == "Deploy") ||
        (m_KindFilter == 3 && (row.kind == "World Sequence" || row.kind == "Action Pattern" || row.kind == "Sequence" || row.kind == "World Box")) ||
        (m_KindFilter == 4 && (row.kind == "Effect" || row.kind == "Light" || row.kind == "Map Effect" || row.kind == "Map Light")) ||
        (m_KindFilter == 5 && row.kind == "Gameplay");
    return kind && (search.empty() || Lower(row.name + " " + row.key + " " + row.assetId + " " +
        row.sourceLevel + " " + row.target.patternId + " " + row.kind).find(search) != std::string::npos);
}

void CWorldLevelTool::Rebuild_Tree()
{
    m_Nodes.clear(); m_NodeIndex.clear(); m_RowIndex.clear();
    m_Nodes.reserve(m_Rows.size() + 256u); m_NodeIndex.reserve(m_Rows.size() + 256u);
    m_RowIndex.reserve(m_Rows.size());
    for (size_t i = 0; i < m_Rows.size(); ++i) m_RowIndex.emplace(m_Rows[i].key, i);
    const auto add = [&](const std::string& id, const std::string& label, size_t parent,
        size_t row = SIZE_MAX, const std::string& folder = std::string{}) -> size_t
    {
        const auto found = m_NodeIndex.find(id);
        if (found != m_NodeIndex.end()) return found->second;
        const size_t index = m_Nodes.size();
        TREE_NODE node; node.id = id; node.label = label; node.parent = parent; node.row = row; node.folderId = folder;
        m_Nodes.push_back(std::move(node)); m_NodeIndex.emplace(id, index);
        if (parent != SIZE_MAX) m_Nodes[parent].children.push_back(index);
        return index;
    };
    const auto area = std::find_if(m_Areas.begin(), m_Areas.end(),
        [&](const AREA& value) { return value.id == m_SelectedAreaId; });
    const std::string rootId = "area:" + m_SelectedAreaId;
    const size_t root = add(rootId, area == m_Areas.end() ? m_SelectedAreaId : area->label, SIZE_MAX);
    m_ExpandedNodes.insert(rootId);
    const size_t parents = add("parents", "User Parents", root);
    m_ExpandedNodes.insert("parents");
    // Create all folder IDs before linking: serialization order need not be parent-first.
    for (const auto& folder : m_Hierarchy.Get_Folders())
        add("folder:" + folder.id, folder.name, SIZE_MAX, SIZE_MAX, folder.id);
    for (const auto& folder : m_Hierarchy.Get_Folders())
    {
        const auto child = m_NodeIndex.at("folder:" + folder.id);
        const auto parent = m_NodeIndex.find("folder:" + folder.parent);
        const auto owner = parent == m_NodeIndex.end() ? parents : parent->second;
        m_Nodes[child].parent = owner; m_Nodes[owner].children.push_back(child);
    }
    const auto segment = [](const std::string& value) { return std::to_string(value.size()) + ":" + value; };
    const auto category = [](const ROW& row) -> std::string
    {
        if (!row.target.patternId.empty()) return row.target.compositionOwner == WORLD_LEVEL_COMPOSITION_OWNER::ACTION ? "Action compositions" : "Sequence compositions";
        if (row.kind == "World Sequence") return "World sequences";
        if (row.kind == "Map Light") return "Map lights";
        if (row.kind == "Map Effect") return "Map effects";
        return row.kind;
    };
    // Pattern nodes are installed first so their stable occurrence children also
    // follow a pattern that the user has organized under a Parent.
    const auto appendRow = [&](size_t rowIndex)
    {
        const ROW& row = m_Rows[rowIndex];
        const std::string assigned = m_Hierarchy.Get_Parent(row.key);
        const auto folder = m_NodeIndex.find("folder:" + assigned);
        size_t owner = SIZE_MAX;
        if (!assigned.empty() && folder != m_NodeIndex.end()) owner = folder->second;
        else
        {
            const auto domain = category(row);
            owner = add("category:" + domain, domain, root);
            if (row.kind == "Map" || row.kind == "Deploy")
            {
                const auto source = row.sourceLevel.empty() ? "Unspecified source level" : row.sourceLevel;
                const auto sourceId = "source:" + segment(domain) + segment(source);
                owner = add(sourceId, source, owner);
                if (!row.assetId.empty()) owner = add("asset:" + segment(sourceId) + segment(row.assetId),
                    row.name.empty() ? row.assetId : row.name, owner);
            }
            else if (!row.target.occurrenceId.empty())
            {
                const std::string prefix = row.target.compositionOwner == WORLD_LEVEL_COMPOSITION_OWNER::ACTION ? "action:" : "sequence:";
                const auto pattern = m_NodeIndex.find("row:" + prefix + row.target.patternId);
                if (pattern != m_NodeIndex.end()) owner = pattern->second;
            }
            else if (row.kind == "World Sequence")
            {
                const auto anchor = row.anchor.empty() ? "Unspecified anchor" : row.anchor;
                owner = add("anchor:" + segment(domain) + segment(anchor), anchor, owner);
            }
        }
        std::string label = row.name.empty() ? row.key : row.name;
        if (row.kind == "Map" || row.kind == "Deploy") label += "  #" + std::to_string(row.target.placementId);
        if (!row.visible) label += " [hidden]";
        add("row:" + row.key, label, owner, rowIndex);
    };
    for (size_t i = 0; i < m_Rows.size(); ++i)
        if (!m_Rows[i].target.patternId.empty() && m_Rows[i].target.occurrenceId.empty()) appendRow(i);
    for (size_t i = 0; i < m_Rows.size(); ++i)
        if (m_Rows[i].target.patternId.empty() || !m_Rows[i].target.occurrenceId.empty()) appendRow(i);
    for (auto& node : m_Nodes)
        std::stable_sort(node.children.begin(), node.children.end(), [&](size_t a, size_t b)
        {
            const auto& left = m_Nodes[a]; const auto& right = m_Nodes[b];
            const bool leftBranch = left.row == SIZE_MAX || !left.children.empty();
            const bool rightBranch = right.row == SIZE_MAX || !right.children.empty();
            return leftBranch != rightBranch ? leftBranch : left.label < right.label;
        });
    for (auto it = m_SelectedNodes.begin(); it != m_SelectedNodes.end();)
        if (!m_NodeIndex.contains(*it)) it = m_SelectedNodes.erase(it); else ++it;
    if (!m_NodeIndex.contains(m_SelectedNode)) { m_SelectedNode.clear(); m_SelectedKey.clear(); }
    m_TreeDirty = false; m_FilterDirty = true; m_VisibleDirty = true;
}

void CWorldLevelTool::Rebuild_VisibleTree()
{
    const auto search = Lower(std::string(m_Search.data()));
    if (m_FilterDirty)
    {
        for (auto& node : m_Nodes)
        {
            node.matches = node.row != SIZE_MAX ? Row_Matches(m_Rows[node.row], search) :
                (m_KindFilter == 0 && (search.empty() || Lower(node.label + " " + node.folderId).find(search) != std::string::npos));
            node.branchMatches = node.matches;
        }
        for (size_t i = 0; i < m_Nodes.size(); ++i)
            if (m_Nodes[i].matches)
                for (size_t parent = m_Nodes[i].parent, depth = 0; parent != SIZE_MAX && depth < 256u; ++depth)
                { m_Nodes[parent].branchMatches = true; parent = m_Nodes[parent].parent; }
        m_FilterSearch = m_Search.data(); m_FilterKind = m_KindFilter; m_FilterDirty = false;
    }
    m_VisibleNodes.clear();
    const bool filtered = !search.empty() || m_KindFilter != 0;
    struct VISIT { size_t index; uint32_t depth; bool ancestorMatches; };
    std::vector<VISIT> stack;
    if (!m_Nodes.empty()) stack.push_back({0u, 0u, false});
    while (!stack.empty())
    {
        const auto visit = stack.back(); stack.pop_back();
        const auto& node = m_Nodes[visit.index];
        if (filtered && !node.branchMatches && !visit.ancestorMatches) continue;
        m_VisibleNodes.push_back({visit.index, visit.depth});
        if (!m_ExpandedNodes.contains(node.id) && !filtered) continue;
        const bool inheritMatch = visit.ancestorMatches ||
            (!search.empty() && m_KindFilter == 0 && node.row == SIZE_MAX && node.matches);
        for (auto it = node.children.rbegin(); it != node.children.rend(); ++it)
            stack.push_back({*it, visit.depth + 1u, inheritMatch});
    }
    m_VisibleDirty = false;
}

void CWorldLevelTool::Reveal_Node(size_t node)
{
    m_ScrollToNode = m_Nodes[node].id;
    for (size_t parent = m_Nodes[node].parent, depth = 0; parent != SIZE_MAX && depth < 256u; ++depth)
    { m_ExpandedNodes.insert(m_Nodes[parent].id); parent = m_Nodes[parent].parent; }
    m_VisibleDirty = true;
}

void CWorldLevelTool::Sync_LiveSelection(const std::string& areaId, uint64_t placementId, bool deploy, bool force)
{
    if (areaId != m_SelectedAreaId) return;
    const auto key = std::string(deploy ? "deploy:" : "map:") + std::to_string(placementId);
    if (!force && areaId == m_LastLiveArea && key == m_LastLiveKey) return;
    m_LastLiveArea = areaId; m_LastLiveKey = key;
    if (m_RowsDirty) Rebuild_Rows();
    if (m_TreeDirty) Rebuild_Tree();
    if (!force && key == m_SelectedKey) return;
    const auto found = m_NodeIndex.find("row:" + key);
    if (found == m_NodeIndex.end()) return;
    const auto& selected = m_Nodes[found->second];
    if (selected.row != SIZE_MAX && !Row_Matches(m_Rows[selected.row], Lower(m_Search.data())))
    {
        m_Search.fill('\0'); m_KindFilter = 0;
        m_FilterDirty = true; m_VisibleDirty = true;
    }
    m_SelectedKey = key; m_SelectedNode = found->first;
    m_SelectedNodes.clear(); m_SelectedNodes.insert(m_SelectedNode); m_SelectionAnchor = m_SelectedNode;
    m_ShowMetadata = false; Reveal_Node(found->second);
}

void CWorldLevelTool::Select_Node(size_t index, bool control, bool shift)
{
    const auto& node = m_Nodes[index];
    if (node.row == SIZE_MAX && node.folderId.empty()) return;
    if (shift && !m_SelectionAnchor.empty())
    {
        size_t first = SIZE_MAX, last = SIZE_MAX;
        for (size_t i = 0; i < m_VisibleNodes.size(); ++i)
        {
            const auto& id = m_Nodes[m_VisibleNodes[i].node].id;
            if (id == m_SelectionAnchor) first = i;
            if (id == node.id) last = i;
        }
        if (!control) m_SelectedNodes.clear();
        if (first != SIZE_MAX && last != SIZE_MAX)
            for (size_t i = (std::min)(first, last); i <= (std::max)(first, last); ++i)
            {
                const auto& selected = m_Nodes[m_VisibleNodes[i].node];
                if (selected.row != SIZE_MAX || !selected.folderId.empty()) m_SelectedNodes.insert(selected.id);
            }
        else m_SelectedNodes.insert(node.id);
    }
    else
    {
        if (!control) m_SelectedNodes.clear();
        if (control && m_SelectedNodes.contains(node.id)) m_SelectedNodes.erase(node.id);
        else m_SelectedNodes.insert(node.id);
        m_SelectionAnchor = node.id;
    }
    m_SelectedNode = node.id; m_SelectedKey = node.row == SIZE_MAX ? "" : m_Rows[node.row].key;
    m_InteractionRequested = true;
    WORLD_LEVEL_TOOL_REQUEST request; request.areaId = m_SelectedAreaId;
    if (node.row != SIZE_MAX)
    {
        const ROW& row = m_Rows[node.row]; request = row.target;
        const bool placement = row.kind == "Map" || row.kind == "Deploy";
        request.kind = placement ? WORLD_LEVEL_REQUEST_KIND::INSPECT_PLACEMENT : WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA;
        m_ShowMetadata = !placement;
    }
    else
    {
        request.kind = WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA; request.sourceItemId = node.folderId;
        m_ShowMetadata = true;
    }
    m_MetadataOpen = m_ShowMetadata; m_Request = std::move(request);
}

bool CWorldLevelTool::Move_Selection(const std::string& parent, const std::string& newName)
{
    const auto& selection = m_ParentAction == PARENT_ACTION::CREATE || m_ParentAction == PARENT_ACTION::MOVE ?
        m_ParentSelection : m_SelectedNodes;
    CWorldLevelHierarchy staged = m_Hierarchy;
    std::unordered_set<std::string> folders;
    std::vector<std::string> rows;
    for (const auto& id : selection)
    {
        const auto found = m_NodeIndex.find(id); if (found == m_NodeIndex.end()) continue;
        const auto& node = m_Nodes[found->second];
        bool selectedAncestor = false;
        for (size_t parentIndex = node.parent, depth = 0; parentIndex != SIZE_MAX && depth < 256u; ++depth)
        {
            if (selection.contains(m_Nodes[parentIndex].id)) { selectedAncestor = true; break; }
            parentIndex = m_Nodes[parentIndex].parent;
        }
        if (selectedAncestor) continue;
        if (!node.folderId.empty()) folders.insert(node.folderId);
        else if (node.row != SIZE_MAX) rows.push_back(m_Rows[node.row].key);
    }
    std::unordered_map<std::string, std::string> folderParents;
    for (const auto& folder : staged.Get_Folders()) folderParents.emplace(folder.id, folder.parent);
    const auto nested = [&](std::string current)
    {
        for (size_t depth = 0; !current.empty() && depth <= 64u; ++depth)
        {
            if (folders.contains(current)) return true;
            const auto found = folderParents.find(current);
            if (found == folderParents.end()) break;
            current = found->second;
        }
        return false;
    };
    std::string destination = parent, created;
    if (!newName.empty())
    {
        if (!staged.Create_Folder(newName, parent, created, m_Status)) return false;
        destination = created;
    }
    if (nested(parent)) { m_Status = "A selected Parent cannot move into its own branch."; return false; }
    std::vector<std::string> rootFolders;
    for (const auto& folder : folders)
        if (!nested(folderParents[folder])) rootFolders.push_back(folder);
    std::vector<std::string> rootRows;
    for (const auto& key : rows) if (!nested(m_Hierarchy.Get_Parent(key))) rootRows.push_back(key);
    for (const auto& folder : rootFolders)
        if (!staged.Move_Folder(folder, destination, m_Status)) return false;
    if (!rootRows.empty() && !staged.Assign(rootRows, destination, m_Status)) return false;
    Remember_HierarchySelection();
    if (!m_Hierarchy.Commit_Edit(staged, m_Status)) return false;
    m_TreeDirty = true; m_InteractionRequested = true;
    if (!created.empty())
    {
        m_SelectedNode = "folder:" + created; m_SelectedKey.clear();
        m_SelectedNodes.clear(); m_SelectedNodes.insert(m_SelectedNode); m_SelectionAnchor = m_SelectedNode;
        m_ExpandedNodes.insert(m_SelectedNode); m_ShowMetadata = true; m_MetadataOpen = true;
        WORLD_LEVEL_TOOL_REQUEST request; request.kind = WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA;
        request.areaId = m_SelectedAreaId; request.sourceItemId = created; m_Request = std::move(request);
    }
    if (!destination.empty()) m_ExpandedNodes.insert("folder:" + destination);
    m_Status = "Parent organization changed. Save hierarchy to keep it; object transforms are unchanged.";
    return true;
}

void CWorldLevelTool::Begin_ParentAction(PARENT_ACTION action)
{
    m_ParentAction = action; m_ParentName.fill('\0'); m_ParentDestination.clear(); m_ParentDialogFolder.clear();
    m_ParentSelection = m_SelectedNodes;
    const auto node = m_NodeIndex.find(m_SelectedNode);
    if (node != m_NodeIndex.end())
    {
        const auto& selected = m_Nodes[node->second];
        if (!selected.folderId.empty())
        {
            m_ParentDialogFolder = selected.folderId;
            for (const auto& folder : m_Hierarchy.Get_Folders()) if (folder.id == selected.folderId)
            {
                m_ParentDestination = folder.parent;
                if (action == PARENT_ACTION::RENAME)
                    std::copy_n(folder.name.data(), (std::min)(folder.name.size(), m_ParentName.size() - 1u), m_ParentName.data());
                break;
            }
        }
        else if (selected.row != SIZE_MAX) m_ParentDestination = m_Hierarchy.Get_Parent(m_Rows[selected.row].key);
    }
    m_ParentDialogRequested = true; m_InteractionRequested = true;
}

void CWorldLevelTool::Render_ParentDialog()
{
    if (m_ParentDialogRequested) { ImGui::OpenPopup("Parent hierarchy"); m_ParentDialogRequested = false; }
    if (!ImGui::BeginPopupModal("Parent hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    const bool naming = m_ParentAction == PARENT_ACTION::CREATE || m_ParentAction == PARENT_ACTION::RENAME;
    if (naming) ImGui::InputText("Name", m_ParentName.data(), m_ParentName.size());
    if (m_ParentAction == PARENT_ACTION::CREATE || m_ParentAction == PARENT_ACTION::MOVE)
    {
        const char* label = "Root / automatic groups";
        for (const auto& folder : m_Hierarchy.Get_Folders()) if (folder.id == m_ParentDestination) { label = folder.name.c_str(); break; }
        if (ImGui::BeginCombo("Parent", label))
        {
            if (ImGui::Selectable("Root / automatic groups", m_ParentDestination.empty())) m_ParentDestination.clear();
            for (const auto& folder : m_Hierarchy.Get_Folders())
            {
                ImGui::PushID(folder.id.c_str());
                if (ImGui::Selectable(folder.name.c_str(), folder.id == m_ParentDestination)) m_ParentDestination = folder.id;
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", folder.id.c_str());
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::TextUnformatted("Parents organize rows only. Position, rotation and scale are not inherited.");
    }
    if (m_ParentAction == PARENT_ACTION::REMOVE)
        ImGui::TextUnformatted("Delete this Parent and move its contents to its parent? Objects are kept.");
    ImGui::BeginDisabled(naming && m_ParentName.front() == '\0');
    if (ImGui::Button("Apply"))
    {
        bool changed = false;
        Remember_HierarchySelection();
        if (m_ParentAction == PARENT_ACTION::CREATE) changed = Move_Selection(m_ParentDestination, m_ParentName.data());
        else if (m_ParentAction == PARENT_ACTION::MOVE) changed = Move_Selection(m_ParentDestination);
        else if (m_ParentAction == PARENT_ACTION::RENAME) changed = m_Hierarchy.Rename_Folder(m_ParentDialogFolder, m_ParentName.data(), m_Status);
        else if (m_ParentAction == PARENT_ACTION::REMOVE) changed = m_Hierarchy.Delete_Folder(m_ParentDialogFolder, m_Status);
        if (changed) { m_TreeDirty = true; m_ParentAction = PARENT_ACTION::NONE; ImGui::CloseCurrentPopup(); }
    }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Cancel")) { m_ParentAction = PARENT_ACTION::NONE; ImGui::CloseCurrentPopup(); }
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::EndPopup();
}

void CWorldLevelTool::Remember_HierarchySelection()
{
    CWorldLevelHierarchy::SELECTION selection;
    selection.nodes.assign(m_SelectedNodes.begin(), m_SelectedNodes.end());
    selection.primary = m_SelectedNode;
    m_Hierarchy.Set_Selection(std::move(selection));
}

bool CWorldLevelTool::Apply_HierarchyHistory(bool redo)
{
    Remember_HierarchySelection();
    if (redo ? !m_Hierarchy.Redo(m_Status) : !m_Hierarchy.Undo(m_Status)) return false;
    m_TreeDirty = true; Rebuild_Tree();
    const auto& selection = m_Hierarchy.Get_Selection();
    m_SelectedNodes.clear();
    for (const auto& id : selection.nodes) if (m_NodeIndex.contains(id)) m_SelectedNodes.insert(id);
    m_SelectedNode = m_NodeIndex.contains(selection.primary) ? selection.primary : std::string{};
    m_SelectionAnchor = m_SelectedNode; m_SelectedKey.clear();
    WORLD_LEVEL_TOOL_REQUEST request; request.areaId = m_SelectedAreaId;
    request.kind = WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA;
    const auto selected = m_NodeIndex.find(m_SelectedNode);
    m_ShowMetadata = true; m_MetadataOpen = true;
    if (selected != m_NodeIndex.end())
    {
        const auto& node = m_Nodes[selected->second];
        Reveal_Node(selected->second);
        if (node.row != SIZE_MAX)
        {
            const auto& row = m_Rows[node.row]; m_SelectedKey = row.key;
            request = row.target;
            m_ShowMetadata = row.kind != "Map" && row.kind != "Deploy";
            request.kind = m_ShowMetadata ? WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA : WORLD_LEVEL_REQUEST_KIND::INSPECT_PLACEMENT;
        }
    }
    m_Request = std::move(request); m_InteractionRequested = true;
    return true;
}

void CWorldLevelTool::Render_HierarchyControls()
{
    if (ImGui::Button("Create Parent")) Begin_ParentAction(PARENT_ACTION::CREATE);
    ImGui::SameLine(); ImGui::BeginDisabled(m_SelectedNodes.empty());
    if (ImGui::Button("Move to Parent")) Begin_ParentAction(PARENT_ACTION::MOVE);
    ImGui::SameLine();
    if (ImGui::Button("Unparent")) (void)Move_Selection({});
    ImGui::EndDisabled();
    ImGui::SameLine();
    const auto& io = ImGui::GetIO();
    const bool shortcut = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !io.WantTextInput && !ImGui::IsAnyItemActive() && io.KeyCtrl;
    ImGui::BeginDisabled(!m_Hierarchy.Can_Undo());
    if (ImGui::Button("Undo") || (shortcut && !io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false)))
        (void)Apply_HierarchyHistory(false);
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::BeginDisabled(!m_Hierarchy.Can_Redo());
    if (ImGui::Button("Redo") || (shortcut && (ImGui::IsKeyPressed(ImGuiKey_Y, false) ||
        (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false))))) (void)Apply_HierarchyHistory(true);
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::BeginDisabled(!m_Hierarchy.Is_Dirty());
    if (ImGui::Button("Save hierarchy") && m_Hierarchy.Save(m_Status)) m_TreeDirty = true;
    ImGui::EndDisabled();
    ImGui::TextDisabled("%zu selected | Parents organize only; no transform inheritance. %s",
        m_SelectedNodes.size(), m_Hierarchy.Is_Dirty() ? "Unsaved hierarchy" : "Saved hierarchy");
}

void CWorldLevelTool::Render_Tree()
{
    if (m_TreeDirty) Rebuild_Tree();
    if (m_FilterSearch != m_Search.data() || m_FilterKind != m_KindFilter) { m_FilterDirty = true; m_VisibleDirty = true; }
    if (m_FilterDirty || m_VisibleDirty) Rebuild_VisibleTree();
    ImGui::Text("%zu objects | %zu visible tree rows", m_Rows.size(), m_VisibleNodes.size());
    if (!ImGui::BeginChild("WorldLevelTree", ImVec2(0.f, -95.f), ImGuiChildFlags_Borders)) { ImGui::EndChild(); return; }
    ImGui::PushID(m_SelectedAreaId.c_str());
    const bool filtered = !m_FilterSearch.empty() || m_KindFilter != 0;
    const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
    if (!m_ScrollToNode.empty())
    {
        for (size_t i = 0; i < m_VisibleNodes.size(); ++i)
            if (m_Nodes[m_VisibleNodes[i].node].id == m_ScrollToNode)
            { ImGui::SetScrollY((std::max)(0.f, float(i) * rowHeight - ImGui::GetWindowHeight() * .4f)); break; }
        m_ScrollToNode.clear();
    }
    ImGuiListClipper clipper; clipper.Begin(static_cast<int>(m_VisibleNodes.size()), rowHeight);
    while (clipper.Step()) for (int display = clipper.DisplayStart; display < clipper.DisplayEnd; ++display)
    {
        const auto& visible = m_VisibleNodes[display]; const auto& node = m_Nodes[visible.node];
        const bool leaf = node.children.empty();
        const bool selectable = node.row != SIZE_MAX || !node.folderId.empty();
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (leaf) flags |= ImGuiTreeNodeFlags_Leaf;
        if (m_SelectedNodes.contains(node.id)) flags |= ImGuiTreeNodeFlags_Selected;
        const bool wasOpen = filtered || m_ExpandedNodes.contains(node.id);
        ImGui::SetNextItemOpen(wasOpen, ImGuiCond_Always);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + visible.depth * ImGui::GetFontSize() * .8f);
        const bool open = ImGui::TreeNodeEx(node.id.c_str(), flags, "%s%s", node.label.c_str(), node.folderId.empty() ? "" : " [Parent]");
        if (!leaf && open != wasOpen && !filtered)
        { if (open) m_ExpandedNodes.insert(node.id); else m_ExpandedNodes.erase(node.id); m_VisibleDirty = true; }
        if (selectable && ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
            Select_Node(visible.node, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
        if (node.row != SIZE_MAX && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            Request_Focus(m_Rows[node.row]);
        if (ImGui::IsItemHovered())
        {
            if (node.row != SIZE_MAX)
            {
                const auto& row = m_Rows[node.row];
                ImGui::SetTooltip("%s\n%s\n%s", row.kind.c_str(), row.key.c_str(), row.assetId.c_str());
            }
            else if (!node.folderId.empty()) ImGui::SetTooltip("Parent: %s", node.folderId.c_str());
        }
        if (selectable && ImGui::BeginDragDropSource())
        {
            const auto* activePayload = ImGui::GetDragDropPayload();
            if (!activePayload || !activePayload->IsDataType("WORLD_LEVEL_PARENT"))
            {
                if (!m_SelectedNodes.contains(node.id)) Select_Node(visible.node, false, false);
                m_DragSelection = m_SelectedNodes; ++m_DragToken;
            }
            ImGui::SetDragDropPayload("WORLD_LEVEL_PARENT", &m_DragToken, sizeof(m_DragToken));
            ImGui::Text("Move %zu selected rows", m_DragSelection.size()); ImGui::EndDragDropSource();
        }
        if ((!node.folderId.empty() || visible.node == 0u || node.id == "parents") && ImGui::BeginDragDropTarget())
        {
            if (const auto* payload = ImGui::AcceptDragDropPayload("WORLD_LEVEL_PARENT"))
                if (payload->DataSize == sizeof(m_DragToken) && *static_cast<const uint64_t*>(payload->Data) == m_DragToken)
                {
                    const auto saved = m_SelectedNodes; m_SelectedNodes = m_DragSelection;
                    if (!Move_Selection(node.folderId)) m_SelectedNodes = saved;
                }
            ImGui::EndDragDropTarget();
        }
        if (selectable && ImGui::BeginPopupContextItem(node.id.c_str()))
        {
            if (!m_SelectedNodes.contains(node.id)) Select_Node(visible.node, false, false);
            else if (m_SelectedNode != node.id)
            {
                const auto selection = m_SelectedNodes;
                Select_Node(visible.node, false, false);
                m_SelectedNodes = selection;
            }
            if (ImGui::MenuItem("Create Parent")) Begin_ParentAction(PARENT_ACTION::CREATE);
            if (ImGui::MenuItem("Move to Parent")) Begin_ParentAction(PARENT_ACTION::MOVE);
            if (ImGui::MenuItem("Unparent")) (void)Move_Selection({});
            if (!node.folderId.empty())
            {
                if (ImGui::MenuItem("Rename Parent")) Begin_ParentAction(PARENT_ACTION::RENAME);
                if (ImGui::MenuItem("Delete Parent (keep contents)")) Begin_ParentAction(PARENT_ACTION::REMOVE);
            }
            else if (node.row != SIZE_MAX)
            {
                const auto& row = m_Rows[node.row];
                if (ImGui::MenuItem("Focus", nullptr, false, row.hasPosition && row.target.areaId == m_ActiveAreaId)) Request_Focus(row);
                if (ImGui::MenuItem("Open in owning tool", nullptr, false, row.canEdit)) Request_Edit(row);
            }
            ImGui::EndPopup();
        }
    }
    ImGui::PopID(); ImGui::EndChild();
}

void CWorldLevelTool::Render_MetadataDetails()
{
    if (!m_ShowMetadata || !m_MetadataOpen) return;
    ImGui::SetNextWindowSize(ImVec2(490.f, 580.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Object Details", &m_MetadataOpen)) { ImGui::End(); return; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_InteractionRequested = true;
    const auto* row = Selected_Row();
    if (row)
    {
        ImGui::TextWrapped("%s", row->name.c_str());
        ImGui::TextWrapped("Type: %s\nArea: %s\nID: %s", row->kind.c_str(), m_SelectedAreaId.c_str(), row->key.c_str());
        if (!row->assetId.empty()) ImGui::TextWrapped("Asset: %s", row->assetId.c_str());
        ImGui::TextWrapped("Anchor: %s", row->anchor.c_str());
        if (row->hasPosition) ImGui::Text("Saved origin: %.3f, %.3f, %.3f m", row->target.position.x, row->target.position.y, row->target.position.z);
        if (row->timed) ImGui::Text("Start %.3f s | Duration %.3f s", row->startMs * .001, row->durationMs * .001);
        ImGui::TextWrapped("%s", row->status.c_str());
        if (row->kind == "Map Effect")
            ImGui::TextWrapped("Surface-effect binding. It has no independent editable object transform. Select the receiver mesh to inspect its material; this row keeps its existing publisher owner.");
        ImGui::BeginDisabled(!row->canEdit);
        if (ImGui::Button("Open in owning tool")) Request_Edit(*row);
        ImGui::EndDisabled(); ImGui::SameLine();
        ImGui::BeginDisabled(!row->hasPosition || row->target.areaId != m_ActiveAreaId);
        if (ImGui::Button("Focus (F)")) Request_Focus(*row);
        ImGui::EndDisabled();
        const auto& io = ImGui::GetIO();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput && !ImGui::IsAnyItemActive() &&
            !io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_F, false)) Request_Focus(*row);
    }
    else
    {
        const auto node = m_NodeIndex.find(m_SelectedNode);
        if (node != m_NodeIndex.end() && !m_Nodes[node->second].folderId.empty())
        {
            ImGui::TextWrapped("%s", m_Nodes[node->second].label.c_str());
            ImGui::TextWrapped("Parent: %s", m_Nodes[node->second].folderId.c_str());
            ImGui::TextUnformatted("Organization only. Child objects keep their own position, rotation and scale.");
            if (ImGui::Button("Rename Parent")) Begin_ParentAction(PARENT_ACTION::RENAME);
            if (ImGui::Button("Move Parent")) Begin_ParentAction(PARENT_ACTION::MOVE);
            if (ImGui::Button("Delete Parent (keep contents)")) Begin_ParentAction(PARENT_ACTION::REMOVE);
        }
        else ImGui::TextUnformatted("Select an object or Parent in the World Level tree.");
    }
    ImGui::End();
}

void CWorldLevelTool::Render()
{
    if (!m_Open) return;
    if (m_RowsDirty) Rebuild_Rows();
    if (m_TreeDirty) Rebuild_Tree();
    ImGui::SetNextWindowSize(ImVec2(1000.f, 720.f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Open World Level Tool", &m_Open))
    {
        const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        if (focused && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) m_InteractionRequested = true;
        ImGui::Text("Active level: %s", m_ActiveAreaId.empty() ? "No world loaded" : m_ActiveAreaId.c_str());
        ImGui::BeginDisabled(m_Hierarchy.Is_Dirty());
        ImGui::SetNextItemWidth(270.f);
        if (ImGui::BeginCombo("Area", m_SelectedAreaId.c_str()))
        {
            for (const auto& area : m_Areas) if (ImGui::Selectable(area.label.c_str(), area.id == m_SelectedAreaId) && area.id != m_SelectedAreaId)
            {
                const auto previous = m_SelectedAreaId; m_SelectedAreaId = area.id;
                if (!Refresh()) m_SelectedAreaId = previous;
                else
                {
                    m_SelectedKey.clear(); m_SelectedNode.clear(); m_SelectedNodes.clear(); m_SelectionAnchor.clear();
                    m_ExpandedNodes.clear(); m_ShowMetadata = false; m_TreeDirty = true;
                    WORLD_LEVEL_TOOL_REQUEST request; request.kind = WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA;
                    request.areaId = m_SelectedAreaId; m_Request = std::move(request);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh saved inventory")) (void)Refresh();
        ImGui::EndDisabled();
        if (m_Hierarchy.Is_Dirty()) ImGui::TextDisabled("Save hierarchy or Undo before changing Area or refreshing.");
        if (m_SelectedAreaId != m_ActiveAreaId) ImGui::TextDisabled("Browsing saved data. Enter this Area to focus or edit live objects.");
        const auto request = [&](WORLD_LEVEL_REQUEST_KIND kind, WORLD_LEVEL_COMPOSITION_OWNER owner)
        {
            WORLD_LEVEL_TOOL_REQUEST value; value.kind = kind; value.areaId = m_SelectedAreaId; value.compositionOwner = owner;
            m_Request = std::move(value); m_InteractionRequested = true;
        };
        const auto* host = Find_ActiveMapAuthoringHost();
        ImGui::BeginDisabled(!host || host->Get_MapAuthoringCatalog().Get_AreaId() != m_SelectedAreaId);
        if (ImGui::Button("Pick in world")) request(WORLD_LEVEL_REQUEST_KIND::PICK_IN_SCENE, WORLD_LEVEL_COMPOSITION_OWNER::ACTION);
        ImGui::EndDisabled(); ImGui::SameLine();
        if (ImGui::Button("Inspect / Edit Map Objects")) request(WORLD_LEVEL_REQUEST_KIND::OPEN_MAP, WORLD_LEVEL_COMPOSITION_OWNER::ACTION);
        ImGui::SameLine();
        if (ImGui::Button("More tools")) ImGui::OpenPopup("World level tools");
        if (ImGui::BeginPopup("World level tools"))
        {
            if (ImGui::MenuItem("DimensionMaster Guide")) request(WORLD_LEVEL_REQUEST_KIND::OPEN_GUIDE, WORLD_LEVEL_COMPOSITION_OWNER::ACTION);
            const bool kouku = m_SelectedAreaId == "LV_LUT_MIDNIGHTC_ED";
            if (ImGui::MenuItem("Create World Object", nullptr, false, kouku)) request(WORLD_LEVEL_REQUEST_KIND::OPEN_WORLD_OBJECT, WORLD_LEVEL_COMPOSITION_OWNER::ACTION);
            if (ImGui::MenuItem("Create Sequence / Effect Box", nullptr, false, kouku)) request(WORLD_LEVEL_REQUEST_KIND::OPEN_COMPOSITION, WORLD_LEVEL_COMPOSITION_OWNER::SEQUENCE);
            ImGui::EndPopup();
        }
        ImGui::SetNextItemWidth(390.f);
        ImGui::InputTextWithHint("##WorldLevelSearch", "Search object, source level, asset or stable ID", m_Search.data(), m_Search.size());
        ImGui::SameLine();
        const char* filters[] = {"All", "Map", "Deploy", "Sequences / Boxes", "Effects / Lights", "Gameplay"};
        ImGui::SetNextItemWidth(175.f); ImGui::Combo("##WorldLevelKind", &m_KindFilter, filters, IM_ARRAYSIZE(filters));
        Render_HierarchyControls();
        Render_Tree();
        if (const auto* selected = Selected_Row())
        {
            ImGui::BeginDisabled(!selected->canEdit);
            if (ImGui::Button("Open selected in owner")) Request_Edit(*selected);
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(!selected->hasPosition || selected->target.areaId != m_ActiveAreaId);
            if (ImGui::Button("Focus (F)")) Request_Focus(*selected);
            ImGui::EndDisabled();
            const auto& io = ImGui::GetIO();
            if (focused && !io.WantTextInput && !ImGui::IsAnyItemActive() &&
                !io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_F, false)) Request_Focus(*selected);
        }
        Render_ChunkView();
        if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    }
    // Keep one popup ID owner even when the tree is collapsed and the request
    // came from the independent Object Details window.
    Render_ParentDialog();
    ImGui::End();
}
}
#endif
