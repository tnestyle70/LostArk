#include "imgui.h"
#include "WorldLevelTool.h"

#ifdef _DEBUG
#include "DataJson.h"
#include "DeployPropCatalog.h"
#include "MapAssetCatalog.h"
#include "MapPlacementDocument.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cfloat>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iterator>
#include <map>
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
    const auto selected = std::find_if(m_Areas.begin(), m_Areas.end(),
        [&](const AREA& area) { return area.id == m_SelectedAreaId; });
    if (selected == m_Areas.end()) return false;
    const AREA& area = *selected;
    std::vector<ROW> staged;
    CMapAssetCatalog catalog;
    std::vector<MAP_PLACEMENT_RECORD> placements;
    if (!catalog.Load_Source(area.catalog, area.placements, area.id, area.materials))
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
        row.status = placement.sourcePlacementId;
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
            if (!asset || asset->kind != DEPLOY_PROP_MODEL_KIND::ANIM)
            {
                row.canEdit = false;
                row.status += " Static Deploy placement: read-only inventory; Animated Props editor supports ANIM placements only.";
            }
            else if (placement.provenance == DEPLOY_PROP_PLACEMENT_PROVENANCE::SOURCE_EXACT)
                row.status += " Source-exact ANIM placement: the owning editor permits inspection but keeps this placement read-only.";
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
            row.key = prefix + box.strOccurrenceId;
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
            row.key = prefix + box.strOccurrenceId;
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

void CWorldLevelTool::Render()
{
    if (!m_Open) return;
    if (m_RowsDirty) Rebuild_Rows();
    ImGui::SetNextWindowSize(ImVec2(1040.f, 680.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Open World Level Tool", &m_Open)) { ImGui::End(); return; }
    const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (focused && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) m_InteractionRequested = true;
    ImGui::Text("Active level: %s", m_ActiveAreaId.empty() ? "No world loaded" : m_ActiveAreaId.c_str());
    ImGui::SetNextItemWidth(240.f);
    if (ImGui::BeginCombo("Area inventory", m_SelectedAreaId.c_str()))
    {
        for (const auto& area : m_Areas) if (ImGui::Selectable(area.label.c_str(), area.id == m_SelectedAreaId))
        {
            const auto previous = m_SelectedAreaId;
            m_SelectedAreaId = area.id;
            if (!Refresh()) m_SelectedAreaId = previous;
            else m_SelectedKey.clear();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh Saved Inventory")) Refresh();
    ImGui::TextWrapped("Map/deploy/world inventory reads saved authoring data. Open Composition owners contribute their current drafts after Apply. Editing and creation use each existing owner and its Save/Publish controls.");
    if (m_SelectedAreaId != m_ActiveAreaId)
        ImGui::TextWrapped("Browsing another Area. Camera focus is available after that Area is loaded; choosing an inventory does not change levels.");
    const auto createRequest = [&](WORLD_LEVEL_REQUEST_KIND kind, WORLD_LEVEL_COMPOSITION_OWNER owner)
    {
        WORLD_LEVEL_TOOL_REQUEST request;
        request.kind = kind; request.areaId = m_SelectedAreaId; request.compositionOwner = owner;
        m_Request = std::move(request); m_InteractionRequested = true;
    };
    if (ImGui::Button("Create / Edit Map Objects")) createRequest(WORLD_LEVEL_REQUEST_KIND::OPEN_MAP, WORLD_LEVEL_COMPOSITION_OWNER::ACTION);
    ImGui::SameLine();
    ImGui::BeginDisabled(m_SelectedAreaId != "LV_LUT_MIDNIGHTC_ED");
    if (ImGui::Button("Create World Object")) createRequest(WORLD_LEVEL_REQUEST_KIND::OPEN_WORLD_OBJECT, WORLD_LEVEL_COMPOSITION_OWNER::ACTION);
    ImGui::SameLine();
    if (ImGui::Button("Create Sequence / Effect Box")) createRequest(WORLD_LEVEL_REQUEST_KIND::OPEN_COMPOSITION, WORLD_LEVEL_COMPOSITION_OWNER::SEQUENCE);
    ImGui::EndDisabled();
    ImGui::SetNextItemWidth(340.f);
    ImGui::InputTextWithHint("##WorldLevelSearch", "Search name, asset, pattern or stable ID", m_Search.data(), m_Search.size());
    ImGui::SameLine();
    const char* filters[] = {"All", "Map", "Deploy", "Sequences / Boxes", "Effects / Lights", "Gameplay"};
    ImGui::SetNextItemWidth(190.f);
    ImGui::Combo("##WorldLevelKind", &m_KindFilter, filters, IM_ARRAYSIZE(filters));
    if (m_FilterDirty || m_FilterKind != m_KindFilter || m_FilterSearch != m_Search.data())
    {
        m_FilterSearch = m_Search.data();
        m_FilterKind = m_KindFilter;
        const auto search = Lower(m_FilterSearch);
        m_FilteredRows.clear();
        m_FilteredRows.reserve(m_Rows.size());
        for (size_t i = 0u; i < m_Rows.size(); ++i)
        {
            const auto& row = m_Rows[i];
            bool include = m_KindFilter == 0 || (m_KindFilter == 1 && row.kind == "Map") || (m_KindFilter == 2 && row.kind == "Deploy") ||
                (m_KindFilter == 3 && (row.kind == "World Sequence" || row.kind == "Action Pattern" || row.kind == "Sequence" || row.kind == "World Box")) ||
                (m_KindFilter == 4 && (row.kind == "Effect" || row.kind == "Light" || row.kind == "Map Effect" || row.kind == "Map Light")) ||
                (m_KindFilter == 5 && row.kind == "Gameplay");
            if (include && (search.empty() || Lower(row.name + " " + row.key + " " + row.assetId + " " + row.target.patternId).find(search) != std::string::npos)) m_FilteredRows.push_back(i);
        }
        m_FilterDirty = false;
    }
    const auto& visible = m_FilteredRows;
    ImGui::Text("%zu / %zu entries", visible.size(), m_Rows.size());
    if (ImGui::BeginTable("WorldLevelInventory", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, -190.f)))
    {
        ImGui::TableSetupColumn("Name"); ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 100.f);
        ImGui::TableSetupColumn("Anchor", ImGuiTableColumnFlags_WidthFixed, 62.f);
        ImGui::TableSetupColumn("World origin", ImGuiTableColumnFlags_WidthFixed, 195.f);
        ImGui::TableSetupColumn("Start / Length (s)", ImGuiTableColumnFlags_WidthFixed, 135.f);
        ImGui::TableSetupScrollFreeze(0, 1); ImGui::TableHeadersRow();
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(visible.size()));
        while (clipper.Step()) for (int display = clipper.DisplayStart; display < clipper.DisplayEnd; ++display)
        {
            const ROW& row = m_Rows[visible[display]];
            ImGui::PushID(row.key.c_str());
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            const auto label = row.name + (row.visible ? "" : " [disabled/hidden]");
            if (ImGui::Selectable(label.c_str(), m_SelectedKey == row.key, ImGuiSelectableFlags_SpanAllColumns)) m_SelectedKey = row.key;
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) Request_Edit(row);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(row.kind.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(row.anchor.c_str());
            ImGui::TableNextColumn();
            if (row.hasPosition) ImGui::Text("%.2f, %.2f, %.2f", row.target.position.x, row.target.position.y, row.target.position.z);
            else ImGui::TextDisabled("Dynamic / multiple / none");
            ImGui::TableNextColumn();
            if (row.timed) ImGui::Text("%.3f / %.3f", row.startMs * .001, row.durationMs * .001);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    const auto selected = std::find_if(m_Rows.begin(), m_Rows.end(), [&](const ROW& row) { return row.key == m_SelectedKey; });
    if (selected != m_Rows.end())
    {
        const ROW& row = *selected;
        ImGui::TextWrapped("ID: %s", row.key.c_str());
        if (!row.target.patternId.empty()) ImGui::TextWrapped("Pattern: %s", row.target.patternId.c_str());
        ImGui::TextWrapped("%s", row.status.c_str());
        ImGui::BeginDisabled(!row.canEdit);
        if (ImGui::Button("Open Selected in Owner")) Request_Edit(row);
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!row.hasPosition || row.target.areaId != m_ActiveAreaId);
        if (ImGui::Button("Focus Origin (F)")) Request_Focus(row);
        ImGui::EndDisabled();
        const auto& io = ImGui::GetIO();
        if (focused && !io.WantTextInput && !ImGui::IsAnyItemActive() &&
            !io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper &&
            ImGui::IsKeyPressed(ImGuiKey_F, false)) Request_Focus(row);
    }
    ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::End();
}
}
#endif
