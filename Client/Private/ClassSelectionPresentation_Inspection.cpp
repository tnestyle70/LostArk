#include "imgui.h"
#include "ClassSelectionPresentation.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "Model.h"
#include "WorldSequenceObject.h"
#include "UIInputRouter.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Client
{
namespace
{
std::string ItemId(const std::string& instance, const std::string& slot)
{ return instance + ".transform." + slot; }
bool Excluded(const CClassSelectionPresentation::SCENE& scene, const std::string& object)
{ return std::find(scene.excludedWorldObjectIds.begin(), scene.excludedWorldObjectIds.end(), object) != scene.excludedWorldObjectIds.end(); }
}

bool CClassSelectionPresentation::Validate_WorldExclusions(const std::vector<SCENE>& scenes,
    const CWorldSequenceDocument& document, std::string& status)
{
    for (const auto& scene : scenes)
    {
        std::set<std::string> objects;
        for (const auto* phase : {&scene.intro, &scene.loop})
            for (const auto& id : phase->instanceIds)
                if (const auto* instance = document.Find_Instance(id))
                    for (const auto& binding : instance->bindings)
                        if (document.Find_ObjectResource(binding.targetId)) objects.insert(binding.targetId);
        for (const auto& id : scene.excludedWorldObjectIds)
            if (!objects.contains(id))
            { status = "WORLD exclusion does not belong to this Movie: " + scene.classId + " / " + id; return false; }
    }
    return true;
}

CLASS_MOVIE_INSPECTION_STATE CClassSelectionPresentation::Get_WorldInspection(
    const std::string& classId, const bool loop) const
{
    CLASS_MOVIE_INSPECTION_STATE state;
    state.classId = classId; state.loop = loop; state.dirty = Has_AuthoringChanges();
    state.active = m_Active && m_Scene && m_Scene->classId == classId;
    state.freeCamera = state.active && Is_InspectionFreeCamera();
    const bool current = classId == m_InspectionClass;
    state.pickArmed = current && m_InspectionPickArmed;
    state.showBackground = !current || m_InspectionShowBackground;
    state.showEffects = !current || m_InspectionShowEffects;
    state.status = current ? m_InspectionStatus : std::string{};
    state.pickedMesh = current ? m_InspectionPickedMesh : UINT32_MAX;
    state.selectionGeneration = current ? m_InspectionSelectionGeneration : 0u;
    if (state.active) { state.movieMs = m_ElapsedMs; state.sourceMs = Get_SourceClockMs(); }
    if (const auto* camera = CGameInstance::Get().Get_CamPosition())
        state.cameraPosition = {camera->x, camera->y, camera->z};
    const auto& scenes = m_Authoring ? m_Authoring->scenes : m_Scenes;
    const auto scene = std::find_if(scenes.begin(), scenes.end(), [&](const auto& value) { return value.classId == classId; });
    if (scene == scenes.end()) return state;
    const auto& phase = loop ? scene->loop : scene->intro;
    const auto& document = m_Authoring ? m_Authoring->document : m_Resources.Get_Document();
    std::vector<CWorldSequencePlayer::OBJECT_INSPECTION_SAMPLE> samples;
    if (state.active && loop == m_Looping) m_Active->Collect_ObjectInspectionSamples(samples);
    for (const auto& instanceId : phase.instanceIds)
    {
        const auto* instance = document.Find_Instance(instanceId);
        const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
        if (!sequence) continue;
        for (const auto& track : sequence->tracks)
        {
            const auto binding = std::find_if(instance->bindings.begin(), instance->bindings.end(),
                [&](const auto& value) { return value.slotId == track.slotId; });
            const auto* resource = binding == instance->bindings.end() ? nullptr : document.Find_ObjectResource(binding->targetId);
            if (!resource) continue;
            CLASS_MOVIE_WORLD_ITEM item;
            item.id = ItemId(instanceId, track.slotId); item.instanceId = instanceId; item.slotId = track.slotId;
            item.objectId = resource->objectId; item.label = resource->displayName; item.modelAssetId = resource->modelAssetId;
            item.excluded = Excluded(*scene, item.objectId);
            item.selected = current && item.objectId == m_InspectionSelectedObject;
            item.solo = current && item.objectId == m_InspectionSoloObject;
            item.muted = current && m_InspectionMutedObjects.contains(item.objectId);
            if (item.selected) state.selectedId = item.id;
            for (const auto& sample : samples)
            {
                if (sample.instanceId != instanceId || sample.slotId != track.slotId || !sample.object) continue;
                item.sampled = true; item.authoredVisible |= sample.object->Is_Visible();
                item.drawn |= sample.object->Is_Visible() && sample.object->Is_InspectionDrawEnabled();
                const auto& world = sample.object->Get_SampledWorld();
                item.position = {world._41, world._42, world._43};
                if (const auto& model = sample.object->Get_Model())
                {
                    item.meshCount = model->Get_NumMeshes();
                    if (item.materials.empty())
                        for (uint32_t mesh = 0; mesh < item.meshCount; ++mesh)
                            item.materials.push_back(std::to_string(mesh) + ": " + model->Get_MaterialName(mesh));
                }
            }
            state.items.push_back(std::move(item));
        }
    }
    return state;
}

bool CClassSelectionPresentation::Is_InspectionBackgroundVisible() const
{ return !m_Scene || m_Scene->classId != m_InspectionClass || m_InspectionShowBackground; }

void CClassSelectionPresentation::Apply_WorldInspection()
{
    if (!m_Active || !m_Scene) return;
    const bool current = m_Scene->classId == m_InspectionClass;
    std::vector<CWorldSequencePlayer::OBJECT_INSPECTION_SAMPLE> samples;
    m_Active->Collect_ObjectInspectionSamples(samples);
    for (const auto& sample : samples)
    {
        if (!sample.object) continue;
        const bool enabled = !Excluded(*m_Scene, sample.objectId) && (!current ||
            (!m_InspectionMutedObjects.contains(sample.objectId) &&
             (m_InspectionSoloObject.empty() || m_InspectionSoloObject == sample.objectId)));
        sample.object->Set_InspectionState(enabled, current && m_InspectionSelectedObject == sample.objectId);
    }
    for (const auto& [id, effect] : m_Effects)
        CEffectPresentationService::Set_WorldRootInspectionVisible(effect.handle, !current || m_InspectionShowEffects);
}

bool CClassSelectionPresentation::Inspect_World(const std::string& classId, const bool loop,
    const CLASS_MOVIE_INSPECTION_COMMAND& command, std::string& status)
{
    const auto state = Get_WorldInspection(classId, loop);
    if (!Has_Class(classId)) { status = "This Movie is not available."; return false; }
    using Action = CLASS_MOVIE_INSPECTION_ACTION;
    if (command.action == Action::SAVE) return Save_Authoring(status, false);
    if (command.action == Action::RELOAD) return Reload_Authoring(status);
    const bool needsItem = command.action == Action::SELECT || command.action == Action::SOLO ||
        command.action == Action::MUTE || command.action == Action::EXCLUDE || command.action == Action::FOCUS;
    const auto item = std::find_if(state.items.begin(), state.items.end(),
        [&](const auto& value) { return value.id == command.itemId; });
    if (needsItem && item == state.items.end())
    { status = "The WORLD item changed. Refresh the Movie list; the current preview was preserved."; return false; }
    if (command.action == Action::EXCLUDE)
    {
        const bool changed = Set_WorldExcluded(classId, item->objectId, command.enabled, status);
        if (changed) m_InspectionStatus = status;
        return changed;
    }
    if (command.action == Action::FREE_CAMERA || command.action == Action::PICK_IN_SCENE || command.action == Action::FOCUS)
    {
        if (!state.active) { status = "Play or seek this Movie before inspecting its live camera."; return false; }
        if (command.action == Action::FOCUS && !item->sampled)
        { status = "The selected item has no pose in the playing phase. Seek that phase first."; return false; }
        if (!(command.action == Action::PICK_IN_SCENE && !command.enabled) &&
            !Set_InspectionFreeCamera(command.action == Action::FREE_CAMERA ? command.enabled : true, status)) return false;
    }
    if (m_InspectionClass != classId)
    {
        m_InspectionClass = classId; m_InspectionSelectedObject.clear(); m_InspectionSoloObject.clear();
        m_InspectionMutedObjects.clear(); m_InspectionPickedMesh = UINT32_MAX;
        m_InspectionShowBackground = m_InspectionShowEffects = true; m_InspectionPickArmed = false;
    }
    switch (command.action)
    {
    case Action::SELECT:
        m_InspectionSelectedObject = item->objectId; m_InspectionPickedMesh = UINT32_MAX; break;
    case Action::SOLO:
        m_InspectionSoloObject = command.enabled ? item->objectId : std::string{};
        m_InspectionSelectedObject = item->objectId; break;
    case Action::MUTE:
        if (command.enabled) m_InspectionMutedObjects.insert(item->objectId);
        else m_InspectionMutedObjects.erase(item->objectId);
        break;
    case Action::CLEAR_PREVIEW:
        m_InspectionSoloObject.clear(); m_InspectionMutedObjects.clear();
        m_InspectionShowBackground = m_InspectionShowEffects = true; m_InspectionPickArmed = false; break;
    case Action::BACKGROUND: m_InspectionShowBackground = command.enabled; break;
    case Action::EFFECTS: m_InspectionShowEffects = command.enabled; break;
    case Action::FREE_CAMERA: break;
    case Action::PICK_IN_SCENE:
        m_InspectionPickArmed = command.enabled; m_InspectionPickReleased = false;
        if (const auto camera = m_Camera.lock(); camera && command.enabled) camera->Set_MouseLookEnabled(false);
        break;
    case Action::FOCUS:
    {
        float3_t center = item->position; float radius = 2.f;
        std::vector<CWorldSequencePlayer::OBJECT_INSPECTION_SAMPLE> samples;
        m_Active->Collect_ObjectInspectionSamples(samples);
        for (const auto& sample : samples)
        {
            if (sample.instanceId != item->instanceId || sample.slotId != item->slotId || !sample.object) continue;
            const auto& model = sample.object->Get_Model(); float3_t minimum{}, maximum{};
            if (!model || !model->Try_GetCurrentPoseBounds(minimum, maximum)) break;
            const auto world = XMLoadFloat4x4(&sample.object->Get_SampledWorld());
            const auto localCenter = (XMLoadFloat3(&minimum) + XMLoadFloat3(&maximum)) * .5f;
            const auto worldCenter = XMVector3TransformCoord(localCenter, world);
            XMStoreFloat3(&center, worldCenter); radius = .1f;
            for (unsigned mask = 0; mask < 8; ++mask)
            {
                const auto corner = XMVectorSet(mask & 1 ? maximum.x : minimum.x,
                    mask & 2 ? maximum.y : minimum.y, mask & 4 ? maximum.z : minimum.z, 1.f);
                radius = (std::max)(radius, XMVectorGetX(XMVector3Length(XMVector3TransformCoord(corner, world) - worldCenter)));
            }
            break;
        }
        if (const auto camera = m_Camera.lock()) camera->Frame_Area(center, radius * 2.5f);
        m_InspectionSelectedObject = item->objectId;
        break;
    }
    default: status = "Unsupported Movie inspection command."; return false;
    }
    if (command.action == Action::SELECT || command.action == Action::SOLO || command.action == Action::FOCUS)
        ++m_InspectionSelectionGeneration;
    Apply_WorldInspection();
    status = command.action == Action::PICK_IN_SCENE && command.enabled ?
        "Click a WORLD model in the scene. Picking uses posed triangles; texture alpha is not sampled. TAB restores mouse look." :
        "Movie inspection updated. Solo/Mute and camera inspection are temporary; playback time is unchanged.";
    m_InspectionStatus = status;
    return true;
}

bool CClassSelectionPresentation::Pick_WorldInspection(const float3_t& origin,
    const float3_t& direction, std::string& status)
{
    if (!m_Active || !m_Scene) { status = "Play a Movie before picking its WORLD models."; return false; }
    std::vector<CWorldSequencePlayer::OBJECT_INSPECTION_SAMPLE> samples;
    m_Active->Collect_ObjectInspectionSamples(samples);
    const CWorldSequencePlayer::OBJECT_INSPECTION_SAMPLE* closest = nullptr;
    float distance = (std::numeric_limits<float>::max)(); uint32_t mesh = UINT32_MAX;
    for (const auto& sample : samples)
    {
        float candidate = 0.f; uint32_t candidateMesh = UINT32_MAX;
        if (sample.object && sample.object->Try_PickInspection(origin, direction, candidate, candidateMesh) && candidate < distance)
        { closest = &sample; distance = candidate; mesh = candidateMesh; }
    }
    if (!closest) { status = "No drawn Movie WORLD triangle was hit. Background and Effect elements are separate rows."; return false; }
    CLASS_MOVIE_INSPECTION_COMMAND select{CLASS_MOVIE_INSPECTION_ACTION::SELECT, ItemId(closest->instanceId, closest->slotId)};
    if (!Inspect_World(m_Scene->classId, m_Looping, select, status)) return false;
    m_InspectionPickedMesh = mesh;
    status = "Picked " + closest->objectId + " / mesh " + std::to_string(mesh);
    if (const auto& model = closest->object->Get_Model(); model && mesh < model->Get_NumMeshes())
        status += " / " + model->Get_MaterialName(mesh);
    m_InspectionStatus = status;
    return true;
}

void CClassSelectionPresentation::Update_InspectionPicking(const bool productPointerHovered)
{
#ifdef _DEBUG
    if (!m_InspectionPickArmed || !m_Active) return;
    auto& game = CGameInstance::Get();
    const bool down = (game.Get_DIMouseStateRaw(Engine::DIM::LB) & 0x80) != 0;
    if (!down) { m_InspectionPickReleased = true; return; }
    if (!m_InspectionPickReleased) return;
    m_InspectionPickReleased = false;
    if (GetForegroundWindow() != g_hWnd || productPointerHovered ||
        CUIInputRouter::Get().Is_MouseClaimedThisFrame() ||
        (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse)) return;
    ::POINT cursor{}; const auto viewport = game.Get_ViewportSize();
    if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor) || viewport.x <= 0.f || viewport.y <= 0.f ||
        cursor.x < 0 || cursor.y < 0 || cursor.x >= viewport.x || cursor.y >= viewport.y) return;
    const auto projection = XMLoadFloat4x4(game.Get_Transform(Engine::D3DTS::PROJ));
    const auto view = XMLoadFloat4x4(game.Get_Transform(Engine::D3DTS::VIEW));
    const auto nearPoint = XMVector3Unproject(XMVectorSet(float(cursor.x), float(cursor.y), 0.f, 1.f),
        0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f, projection, view, XMMatrixIdentity());
    const auto farPoint = XMVector3Unproject(XMVectorSet(float(cursor.x), float(cursor.y), 1.f, 1.f),
        0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f, projection, view, XMMatrixIdentity());
    float3_t origin{}, direction{}; XMStoreFloat3(&origin, nearPoint);
    XMStoreFloat3(&direction, XMVector3Normalize(farPoint - nearPoint));
    m_InspectionPickArmed = false;
    (void)Pick_WorldInspection(origin, direction, m_InspectionStatus);
    game.SetMouseButtonBlocked(Engine::DIM::LB, true);
#endif
}
}
