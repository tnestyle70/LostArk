#include "imgui.h"
#include "WorldLevelTool.h"

#ifdef _DEBUG
#include "DataJson.h"
#include "DeployPropCatalog.h"
#include "MapAssetCatalog.h"
#include "MapAuthoringHost.h"
#include "MapPlacementDocument.h"
#include "MapPlacementEditSession.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cfloat>
#include <cctype>
#include <cmath>
#include <fstream>
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
/* Degrees are friendlier than a raw quaternion; both edit the same pose. The
   Map Tool keeps an identical pair for its own inspector, so a placement
   edited in either tool reads back the same angles. */
float3_t PlacementQuaternionToEulerDegrees(const float4_t& value)
{
    /* Exact inverse of XMQuaternionRotationRollPitchYaw (M = Mz*Mx*My,
       row-vector). Yaw and roll use atan2 so Y keeps the full +-180 range;
       only pitch stays asin-limited to +-90. */
    constexpr float radiansToDegrees = 180.f / DirectX::XM_PI;
    float4x4_t rotation{};
    XMStoreFloat4x4(&rotation, XMMatrixRotationQuaternion(XMLoadFloat4(&value)));
    const float sinPitch = (std::max)(-1.f, (std::min)(1.f, -rotation._32));
    const float pitch = std::asin(sinPitch);
    float yaw = 0.f, roll = 0.f;
    if (std::abs(sinPitch) < 0.99999f)
    { yaw = std::atan2(rotation._31, rotation._33); roll = std::atan2(rotation._12, rotation._22); }
    else yaw = std::atan2(-rotation._13, rotation._11);
    return float3_t(pitch * radiansToDegrees, yaw * radiansToDegrees, roll * radiansToDegrees);
}
float4_t PlacementEulerDegreesToQuaternion(const float3_t& value)
{
    constexpr float degreesToRadians = DirectX::XM_PI / 180.f;
    vector_t quaternion = XMQuaternionRotationRollPitchYaw(
        value.x * degreesToRadians, value.y * degreesToRadians, value.z * degreesToRadians);
    quaternion = XMQuaternionNormalize(quaternion);
    if (XMVectorGetW(quaternion) < 0.f) quaternion = XMVectorNegate(quaternion);
    float4_t result{};
    XMStoreFloat4(&result, quaternion);
    return result;
}
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

CMapPlacementEditSession& CWorldLevelTool::Session()
{
    if (!m_pSession) m_pSession = std::make_unique<CMapPlacementEditSession>();
    return *m_pSession;
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
        row.status = placement.sourcePlacementId + " | source level " + placement.sourceLevel;
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
    Apply_SessionRows();
    m_RowsDirty = false;
    m_FilterDirty = true;
}

void CWorldLevelTool::Apply_SessionRows()
{
    if (!m_pSession || !m_pSession->Is_Bound() || m_pSession->Get_AreaId() != m_SelectedAreaId) return;
    const auto& catalog = m_pSession->Get_Catalog();
    const auto& draft = m_pSession->Get_Draft();
    std::unordered_map<uint64_t, size_t> existing;
    for (size_t i = 0u; i < m_Rows.size(); ++i)
        if (m_Rows[i].kind == "Map") existing.emplace(m_Rows[i].target.placementId, i);
    std::unordered_set<uint64_t> present;
    present.reserve(draft.size());
    for (const auto& record : draft)
    {
        present.insert(record.placementId);
        const auto found = existing.find(record.placementId);
        if (found == existing.end()) continue;
        ROW& row = m_Rows[found->second];
        row.target.position = record.position;
        row.visible = record.visible;
        row.status = record.sourcePlacementId + " | source level " + record.sourceLevel;
    }
    for (const auto& record : draft)
    {
        if (existing.contains(record.placementId)) continue;
        ROW row;
        row.key = "map:" + std::to_string(record.placementId);
        row.kind = "Map";
        const auto* asset = catalog.Find(record.assetId);
        row.name = asset ? asset->label : record.assetId;
        row.assetId = record.assetId;
        row.anchor = "MAP";
        row.hasPosition = true;
        row.visible = record.visible;
        row.target.areaId = m_SelectedAreaId;
        row.target.placementId = record.placementId;
        row.target.position = record.position;
        row.status = record.sourcePlacementId + " | source level " + record.sourceLevel +
            " (created in this editing session)";
        m_Rows.push_back(std::move(row));
    }
    std::erase_if(m_Rows, [&](const ROW& row)
        { return row.kind == "Map" && !present.contains(row.target.placementId); });
}

void CWorldLevelTool::Update(const bool visible)
{
    if (!m_pSession) return;
    m_pSession->Update(visible && m_Open);
    if (m_pSession->Consume_RowsDirty()) m_RowsDirty = true;
    const uint64_t selected = m_pSession->Get_SelectedPlacementId();
    if (selected == m_SessionSelectionMirror) return;
    m_SessionSelectionMirror = selected;
    if (0u != selected && m_pSession->Get_AreaId() == m_SelectedAreaId)
        m_SelectedKey = "map:" + std::to_string(selected);
}

bool CWorldLevelTool::Is_PlacementPickArmed() const
{ return m_pSession && m_pSession->Is_PickArmed(); }
const std::string& CWorldLevelTool::Get_PlacementPickAreaId() const
{
    static const std::string empty;
    return m_pSession ? m_pSession->Get_AreaId() : empty;
}
void CWorldLevelTool::Cancel_PlacementPick(std::string reason)
{ if (m_pSession) m_pSession->Cancel_Pick(std::move(reason)); }
void CWorldLevelTool::Complete_PlacementPick(const float3_t& worldPoint)
{ if (m_pSession) m_pSession->Complete_Pick(worldPoint); }
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
    /* Browsing another Area while placements are bound would show a document
       the draft does not describe, so the inventory stays on the edited Area. */
    const bool sessionBound = m_pSession && m_pSession->Is_Bound();
    ImGui::BeginDisabled(sessionBound);
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
    ImGui::EndDisabled();
    if (sessionBound)
        ImGui::TextWrapped("Map placements of %s are bound for editing; End editing to browse another Area.",
            m_pSession->Get_AreaId().c_str());
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
            if (ImGui::Selectable(label.c_str(), m_SelectedKey == row.key, ImGuiSelectableFlags_SpanAllColumns))
            {
                m_SelectedKey = row.key;
                if (sessionBound && row.kind == "Map" && row.target.areaId == m_pSession->Get_AreaId())
                {
                    m_pSession->Select_Placement(row.target.placementId);
                    m_SessionSelectionMirror = row.target.placementId;
                }
            }
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
    Render_MapEditing();
    ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::End();
}

void CWorldLevelTool::Render_MapEditing()
{
    const auto area = std::find_if(m_Areas.begin(), m_Areas.end(),
        [&](const AREA& entry) { return entry.id == m_SelectedAreaId; });
    if (area == m_Areas.end()) return;
    if (!ImGui::CollapsingHeader("Map placements", ImGuiTreeNodeFlags_DefaultOpen)) return;

    CMapPlacementEditSession& session = Session();
    const IMapAuthoringHost* host = Find_ActiveMapAuthoringHost();
    const bool ownsArea = nullptr != host &&
        host->Get_MapAuthoringCatalog().Get_AreaId() == m_SelectedAreaId;
    if (!session.Is_Bound())
    {
        ImGui::BeginDisabled(!ownsArea);
        if (ImGui::Button("Edit placements in this Level"))
        {
            CMapPlacementEditSession::BIND_DESC desc;
            desc.areaId = area->id;
            desc.sourceCatalog = area->catalog;
            desc.sourcePlacements = area->placements;
            desc.sourceMaterials = area->materials;
            desc.declaresLights = !area->lights.empty();
            std::string status;
            if (!session.Bind(desc, status)) session.Set_Status(status);
            m_SessionSelectionMirror = 0u;
            m_RowsDirty = true;
        }
        ImGui::EndDisabled();
        if (!ownsArea)
            ImGui::TextWrapped("%s is listed read-only: the current Level does not own it. Enter Character Select or the KoukuSaydon arena that owns this Area, then press Edit placements.",
                m_SelectedAreaId.c_str());
        ImGui::TextWrapped("%s", session.Get_Status().c_str());
        return;
    }

    ImGui::Text("Editing %s%s%s", session.Get_AreaId().c_str(),
        session.Is_Dirty() ? " | unsaved draft" : "",
        session.Is_Publishing() ? " | publishing" : "");
    if (session.Is_ReadOnly())
        ImGui::TextWrapped("Read-only: %s", session.Get_ReadOnlyReason().c_str());
    if (ImGui::Button("End editing"))
    {
        session.End();
        m_SessionSelectionMirror = 0u;
        m_RowsDirty = true;
        session.Set_Status("Editing ended. The draft is kept in memory until the tool is closed.");
        return;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(session.Is_ReadOnly() || session.Is_PickArmed());
    if (ImGui::Button("Pick World Object"))
    {
        session.Arm_Pick();
        WORLD_LEVEL_TOOL_REQUEST request;
        request.kind = WORLD_LEVEL_REQUEST_KIND::PICK_PLACEMENT;
        request.areaId = session.Get_AreaId();
        m_Request = std::move(request);
        m_InteractionRequested = true;
    }
    ImGui::EndDisabled();
    if (session.Is_PickArmed())
    {
        ImGui::SameLine();
        if (ImGui::Button("Cancel pick"))
            session.Cancel_Pick("Pick cancelled; the selection was preserved.");
    }

    const uint64_t selected = session.Get_SelectedPlacementId();
    const MAP_PLACEMENT_RECORD* draft = session.Find_Draft(selected);
    if (nullptr == draft)
        ImGui::TextDisabled("No placement selected. Pick one in the viewport, or select a Map row above.");
    else
    {
        ImGui::Text("Placement #%llu", static_cast<unsigned long long>(draft->placementId));
        ImGui::TextWrapped("Asset: %s | Source: %s | Level: %s | Transform: %s",
            draft->assetId.c_str(), draft->sourcePlacementId.c_str(),
            draft->sourceLevel.c_str(), draft->transformSource.c_str());
        const std::string blocked = session.Describe_EditBlock(selected);
        if (!blocked.empty()) ImGui::TextWrapped("%s", blocked.c_str());
        float3_t position = draft->position;
        float4_t quaternion = draft->rotationQuaternion;
        float3_t scale = draft->signedScale;
        ImGui::BeginDisabled(!blocked.empty());
        const bool positionChanged = ImGui::DragFloat3("Position", &position.x, 0.1f);
        float3_t eulerDegrees = PlacementQuaternionToEulerDegrees(quaternion);
        const bool eulerChanged = ImGui::DragFloat3("Rotation (deg pitch/yaw/roll)", &eulerDegrees.x, 0.5f);
        if (eulerChanged) quaternion = PlacementEulerDegreesToQuaternion(eulerDegrees);
        const bool scaleChanged = ImGui::DragFloat3("Signed scale", &scale.x, 0.01f, -1000.f, 1000.f);
        ImGui::EndDisabled();
        ImGui::Text("Rotation quaternion: %.4f, %.4f, %.4f, %.4f",
            quaternion.x, quaternion.y, quaternion.z, quaternion.w);
        if (blocked.empty() && (positionChanged || eulerChanged || scaleChanged))
        {
            const vector_t rawQuaternion = XMLoadFloat4(&quaternion);
            const float quaternionLength = XMVectorGetX(XMVector4Length(rawQuaternion));
            const bool scaleIsValid = std::abs(scale.x) >= 0.000001f &&
                std::abs(scale.y) >= 0.000001f && std::abs(scale.z) >= 0.000001f;
            if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
                !std::isfinite(quaternionLength) || quaternionLength < 0.000001f || !scaleIsValid)
                session.Set_Status("Transform edit rejected: non-finite value or zero quaternion/scale axis.");
            else
            {
                vector_t normalized = XMQuaternionNormalize(rawQuaternion);
                if (XMVectorGetW(normalized) < 0.f) normalized = XMVectorNegate(normalized);
                XMStoreFloat4(&quaternion, normalized);
                MAP_PLACEMENT_RECORD staged = *draft;
                staged.position = position;
                staged.rotationQuaternion = quaternion;
                staged.signedScale = scale;
                if (session.Apply_Transform(selected, staged))
                {
                    const std::string key = "map:" + std::to_string(selected);
                    for (ROW& row : m_Rows) if (row.key == key) { row.target.position = staged.position; break; }
                    session.Set_Status("Transform applied to placement #" + std::to_string(selected) +
                        "; Save writes it to the authoring document.");
                }
            }
        }
        ImGui::BeginDisabled(session.Is_ReadOnly());
        if (ImGui::Button("Duplicate")) { if (session.Duplicate_Selected()) m_RowsDirty = true; }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(session.Is_ReadOnly() || !session.Is_SessionCreated(selected));
        if (ImGui::Button("Delete duplicate")) { if (session.Delete_Selected()) m_RowsDirty = true; }
        ImGui::EndDisabled();
        ImGui::SameLine();
    }
    ImGui::BeginDisabled(session.Is_ReadOnly() || !session.Is_Dirty() || session.Is_Publishing());
    if (ImGui::Button("Save")) (void)session.Save();
    ImGui::EndDisabled();
    ImGui::TextWrapped("%s", session.Get_Status().c_str());
}
}
#endif
