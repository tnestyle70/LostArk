#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "Camera.h"
#include "CameraTool.h"
#include "CompositionTimeline.h"
#include "DataJson.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "ValtanCinematicCameraController.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <set>

namespace Client
{
namespace
{
constexpr std::uint64_t CAMERA_OWNER = 0x4546465345514341ull;
constexpr std::uint32_t MAX_CAMERA_MS = 600000u;
bool Camera_Text(const DATA_JSON_VALUE& object, const char* key, std::string& out)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_String()) return false;
    out = value->Get_String(); return true;
}
bool Camera_Number(const DATA_JSON_VALUE& object, const char* key, double& out)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
    out = value->Get_Number(); return true;
}
bool Camera_Ms(const DATA_JSON_VALUE& object, const char* key, std::uint32_t& out)
{
    double value = 0;
    if (!Camera_Number(object, key, value) || value < 0 || value > MAX_CAMERA_MS || std::floor(value) != value) return false;
    out = static_cast<std::uint32_t>(value); return true;
}
bool Camera_Vector(const DATA_JSON_VALUE& object, const char* key, float3_t& out)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Array() || value->Get_Array().size() != 3u) return false;
    float* parts[] = {&out.x, &out.y, &out.z};
    for (std::size_t i = 0; i < 3u; ++i)
    {
        const auto& part = value->Get_Array()[i];
        if (!part.Is_Number() || !std::isfinite(part.Get_Number()) || std::abs(part.Get_Number()) > 100000.0) return false;
        *parts[i] = static_cast<float>(part.Get_Number());
    }
    return true;
}
bool Camera_ValidUp(const float3_t& eye, const float3_t& target, const float3_t& up)
{
    const float values[] = {up.x, up.y, up.z};
    if (std::any_of(std::begin(values), std::end(values), [](float value) { return !std::isfinite(value) || std::abs(value) > 100000.f; })) return false;
    const auto direction = XMLoadFloat3(&target) - XMLoadFloat3(&eye);
    const float directionSq = XMVectorGetX(XMVector3LengthSq(direction));
    const float upSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&up)));
    if (!std::isfinite(directionSq) || directionSq <= .000001f || !std::isfinite(upSq) || upSq <= .000001f) return false;
    const float crossSq = XMVectorGetX(XMVector3LengthSq(XMVector3Cross(
        XMVector3Normalize(XMLoadFloat3(&up)), XMVector3Normalize(direction))));
    return std::isfinite(crossSq) && crossSq > .000001f;
}
bool Camera_ConvertFov(float& fov, const float aspect, const bool toHorizontal)
{
    if (!std::isfinite(fov) || fov <= 1.f || fov >= 179.f || !std::isfinite(aspect) || aspect <= 0.f) return false;
    const float tangent = std::tan(XMConvertToRadians(fov) * .5f);
    const float converted = XMConvertToDegrees(2.f * std::atan(toHorizontal ? tangent * aspect : tangent / aspect));
    if (!std::isfinite(converted) || converted <= 1.f || converted >= 179.f) return false;
    fov = converted; return true;
}
bool Camera_CurrentAspect(float& aspect)
{
    const auto* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
    if (!projection || !std::isfinite(projection->_11) || projection->_11 <= 0.f ||
        !std::isfinite(projection->_22) || projection->_22 <= 0.f) return false;
    aspect = projection->_22 / projection->_11;
    return std::isfinite(aspect) && aspect > 0.f;
}

}

void CEffectAuthoringSequencer::Set_Camera(const std::shared_ptr<Engine::CCamera>& camera)
{
    if (m_Camera.lock() == camera) return;
    Stop(); m_Camera = camera;
}
void CEffectAuthoringSequencer::Release_Camera()
{
    if (const auto camera = m_Camera.lock(); camera && camera->Is_PresentationOverrideOwnedBy(CAMERA_OWNER))
        camera->End_PresentationOverride(CAMERA_OWNER);
    m_CameraOwned = false;
}
bool CEffectAuthoringSequencer::Validate_CameraRows(const std::vector<CAMERA_ROW>& rows)
{ return CEffectRecoveryCamera::Validate(rows, m_Status); }
bool CEffectAuthoringSequencer::Parse_CameraRows(const DATA_JSON_VALUE& document, const bool required, std::vector<CAMERA_ROW>& out)
{ return CEffectRecoveryCamera::Parse(document, required, out, m_Status); }
void CEffectAuthoringSequencer::Write_CameraRows(std::ostream& out) const
{
    out << ",\n  \"cameras\": [";
    for (std::size_t i = 0; i < m_CameraRows.size(); ++i)
    {
        const auto& row = m_CameraRows[i];
        out << (i ? ",\n" : "\n") << "    {\"cameraId\": \"" << CDataJson::Escape(row.id) << "\", \"displayName\": \"" << CDataJson::Escape(row.label)
            << "\", \"source\": \"" << CDataJson::Escape(row.source) << "\", \"space\": \"" << (row.modelRelative ? "MODEL_ROOT" : "WORLD")
            << "\", \"fovAxis\": \"" << (row.horizontalFov ? "HORIZONTAL" : "VERTICAL")
            << "\", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.cue.iDurationMs << ", \"muted\": " << (row.muted ? "true" : "false")
            << ", \"interpolation\": \"" << (row.cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM ? "CATMULL_ROM" : "LINEAR")
            << "\", \"easing\": \"" << (row.cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::HOLD ? "HOLD" : row.cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP ? "SMOOTHSTEP" : "LINEAR") << "\", \"keys\": [";
        for (std::size_t j = 0; j < row.cue.Keyframes.size(); ++j)
        {
            const auto& key = row.cue.Keyframes[j]; const auto& up = row.upVectors[j];
            out << (j ? ", " : "") << "{\"keyId\": \"" << CDataJson::Escape(key.strSceneId) << "\", \"timeMs\": " << key.iTimeMs
                << ", \"eye\": [" << key.vEye.x << ", " << key.vEye.y << ", " << key.vEye.z << "], \"lookAt\": ["
                << key.vLookAt.x << ", " << key.vLookAt.y << ", " << key.vLookAt.z << "], \"up\": ["
                << up.x << ", " << up.y << ", " << up.z << "], \"fovDegrees\": " << key.fFovYDegrees << "}";
        }
        out << "]}";
    }
    out << "\n  ]";
}
bool CEffectAuthoringSequencer::Read_RecoveryCameras(const EFFECT_RESOURCE_KEY& key, std::vector<CAMERA_ROW>& rows,
    std::vector<CLIP>& animations)
{
    animations.clear();
    if (key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT || !key.strStableId.ends_with(".restore"))
    { rows.clear(); return true; }
    EFFECT_RECOVERY_CAMERA_DOCUMENT recovery;
    if (!key.Is_Valid() || !CEffectRecoveryCamera::Load(key.strStableId, recovery, m_Status)) return false;
    if (!recovery.exists) { rows.clear(); return true; }
    if (recovery.asset != m_AssetName || recovery.sequence != m_SelectedSequence)
    { m_Status = "Recovery camera preset must match its character and skill."; return false; }
    const auto& root = recovery.document;
    const auto version = recovery.version, duration = recovery.durationMs;
    auto staged = std::move(recovery.rows);
    if (!staged.empty() && (m_UseKouku || !m_ModelRoot || m_DefaultAnchorSlotId != "root"))
    { m_Status = "Select Model root / root anchor to preview this character camera preset."; return false; }
    if (version == 4u)
    {
        bool customAnimation = false;
        std::vector<SOUND_ROW> sounds; std::vector<COLLIDER_ROW> colliders;
        if (!Parse_AdditionalRows(root, version, animations, customAnimation, sounds, colliders)) return false;
        for (const auto& row : animations)
            if (!row.memberId.empty() || row.startMs + row.durationMs > duration)
            { m_Status = "Recovery animation must use this character within its Effect occurrence."; return false; }
        if (!animations.empty() && (m_UseKouku || !m_ModelRoot || m_DefaultAnchorSlotId != "root"))
        { m_Status = "Select Model root / root anchor to preview this recovery animation."; return false; }
        if (!sounds.empty() || !colliders.empty())
            m_Status = "Recovery camera preview uses its Effect, Animation and Camera rows only.";
        else m_Status.clear();
    }
    else m_Status.clear();
    rows = std::move(staged);
    return true;
}
bool CEffectAuthoringSequencer::Sample_Camera(const std::uint32_t clockMs, const float4x4_t& root,
    const std::vector<CAMERA_ROW>* overrideRows)
{
    const auto& rows = overrideRows ? *overrideRows : (m_Transient ? m_TransientCameraRows : m_CameraRows);
    const CAMERA_ROW* active = nullptr;
    for (const auto& row : rows)
        if (!row.muted && clockMs >= row.startMs && clockMs < row.startMs + row.cue.iDurationMs)
        {
            if (active) { m_Status = "Overlapping camera rows cannot own the view simultaneously."; return false; }
            active = &row;
        }
    if (!active) { Release_Camera(); return true; }
    const auto camera = m_Camera.lock();
    if (!camera) { m_Status = "This Level has not supplied an authoring camera."; return false; }
    if (m_CameraOwned && !camera->Is_PresentationOverrideOwnedBy(CAMERA_OWNER))
    { m_CameraOwned = false; m_Status = "Effect camera was preempted by another presentation owner."; return false; }
    VALTAN_CINEMATIC_CAMERA_POSE pose; float3_t up;
    if (!CEffectRecoveryCamera::Sample(*active, clockMs, root, camera->Get_AspectRatio(), pose, up, m_Status)) return false;
    if (!camera->Begin_PresentationOverride(CAMERA_OWNER, Engine::CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW))
    { m_Status = "Effect camera is unavailable while another cinematic or preview owns the view."; return false; }
    m_CameraOwned = true;
    if (!camera->Apply_PresentationPoseWithUp(CAMERA_OWNER, pose.vEye, pose.vLookAt, up, pose.fFovYDegrees))
    { m_Status = "Effect camera pose was rejected."; Release_Camera(); return false; }
    return true;
}
bool CEffectAuthoringSequencer::Sort_CameraKeys(CAMERA_ROW& row)
{
    if (row.cue.Keyframes.size() != row.upVectors.size())
    { m_Status = "Camera key/up-vector count differs."; return false; }
    std::vector<std::size_t> order(row.cue.Keyframes.size());
    std::iota(order.begin(), order.end(), 0u);
    std::stable_sort(order.begin(), order.end(), [&](auto a, auto b) { return row.cue.Keyframes[a].iTimeMs < row.cue.Keyframes[b].iTimeMs; });
    auto keys = row.cue.Keyframes; auto ups = row.upVectors;
    for (std::size_t i = 0; i < order.size(); ++i)
    { row.cue.Keyframes[i] = std::move(keys[order[i]]); row.upVectors[i] = ups[order[i]]; }
    return true;
}
bool CEffectAuthoringSequencer::Capture_CameraKey(CAMERA_ROW& row, const std::uint32_t time)
{
    if (row.cue.Keyframes.size() != row.upVectors.size() || time > row.cue.iDurationMs)
    { m_Status = "Cannot capture into inconsistent camera keys or outside their duration."; return false; }
    VALTAN_CINEMATIC_CAMERA_POSE pose; float3_t up;
    const auto* view = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
    if (!view || !CCameraTool::Capture_ViewPose(pose)) { m_Status = "Current camera pose is unavailable."; return false; }
    XMStoreFloat3(&up, XMLoadFloat4x4(view).r[1]);
    if (row.horizontalFov)
    {
        float aspect = 0.f;
        if (!Camera_CurrentAspect(aspect) || !Camera_ConvertFov(pose.fFovYDegrees, aspect, true))
        { m_Status = "Cannot capture horizontal FOV from the current projection."; return false; }
    }
    if (row.modelRelative)
    {
        float4x4_t root; if (!Resolve_Root(root)) return false;
        const auto inverse = XMMatrixInverse(nullptr, XMLoadFloat4x4(&root));
        XMStoreFloat3(&pose.vEye, XMVector3TransformCoord(XMLoadFloat3(&pose.vEye), inverse));
        XMStoreFloat3(&pose.vLookAt, XMVector3TransformCoord(XMLoadFloat3(&pose.vLookAt), inverse));
        XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), inverse));
    }
    if (!Camera_ValidUp(pose.vEye, pose.vLookAt, up)) { m_Status = "Captured camera has a degenerate basis."; return false; }
    auto found = std::find_if(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), [&](const auto& value) { return value.iTimeMs == time; });
    VALTAN_CINEMATIC_CAMERA_KEYFRAME key;
    key.strSceneId = found == row.cue.Keyframes.end() ? CEffectEditingSession::New_Id("camera.key.") : found->strSceneId;
    key.iTimeMs = time; key.vEye = pose.vEye; key.vLookAt = pose.vLookAt; key.fFovYDegrees = pose.fFovYDegrees;
    if (found == row.cue.Keyframes.end()) { row.cue.Keyframes.push_back(key); row.upVectors.push_back(up); }
    else { row.upVectors[static_cast<std::size_t>(found - row.cue.Keyframes.begin())] = up; *found = key; }
    if (!Sort_CameraKeys(row)) return false;
    row.source = "PROJECT_TUNED"; return true;
}
void CEffectAuthoringSequencer::Render_CameraEditor()
{
    auto& rows = m_Transient ? m_TransientCameraRows : m_CameraRows;
    if (ImGui::Button("Add Camera from current view"))
    {
        CAMERA_ROW row; row.id = CEffectEditingSession::New_Id("camera.row."); row.label = "Camera";
        row.startMs = (std::min)(ClockMs(), MAX_CAMERA_MS - 1000u); row.cue.iDurationMs = 1000u;
        row.modelRelative = m_ModelRoot; row.cue.strCueId = row.id;
        if (Capture_CameraKey(row, 0u))
        {
            auto staged = rows; staged.push_back(row);
            if (Validate_CameraRows(staged)) { rows = std::move(staged); m_SelectedCamera = row.id; m_Dirty = true; }
        }
    }
    if (m_Transient)
    {
        ImGui::SameLine();
        if (ImGui::Button("Append preview and cameras"))
        {
            if (Commit_TransientPreview()) return;
        }
    }
    auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.id == m_SelectedCamera; });
    if (found == rows.end()) return;
    ImGui::PushID("CameraRowEditor");
    auto draft = *found; bool changed = false;
    int start = static_cast<int>(draft.startMs), duration = static_cast<int>(draft.cue.iDurationMs);
    ImGui::SetNextItemWidth(150.f); changed |= ImGui::DragInt("Camera start", &start, 10.f, 0, MAX_CAMERA_MS - duration, "%d ms");
    ImGui::SameLine(); ImGui::SetNextItemWidth(150.f); changed |= ImGui::DragInt("Camera duration", &duration, 10.f, 1, MAX_CAMERA_MS - start, "%d ms");
    draft.startMs = static_cast<std::uint32_t>((std::clamp)(start, 0, int(MAX_CAMERA_MS - 1u)));
    draft.cue.iDurationMs = static_cast<std::uint32_t>((std::clamp)(duration, 1, int(MAX_CAMERA_MS - draft.startMs)));
    int space = draft.modelRelative ? 1 : 0;
    if (ImGui::Combo("Camera space", &space, "World\0Model root\0")) { draft.modelRelative = space == 1; changed = true; }
    int axis = draft.horizontalFov ? 1 : 0;
    if (ImGui::Combo("Camera FOV axis", &axis, "Vertical\0Horizontal\0"))
    {
        float aspect = 0.f; auto converted = draft.cue.Keyframes;
        const bool valid = Camera_CurrentAspect(aspect) && std::all_of(converted.begin(), converted.end(),
            [&](auto& key) { return Camera_ConvertFov(key.fFovYDegrees, aspect, axis == 1); });
        if (valid) { draft.cue.Keyframes = std::move(converted); draft.horizontalFov = axis == 1; changed = true; }
        else m_Status = "The current projection cannot convert every camera key to that FOV axis.";
    }
    int interpolation = static_cast<int>(draft.cue.eInterpolation), easing = static_cast<int>(draft.cue.eEasing);
    if (ImGui::Combo("Camera curve", &interpolation, "Linear\0Catmull-Rom\0")) { draft.cue.eInterpolation = static_cast<VALTAN_CINEMATIC_CAMERA_INTERPOLATION>(interpolation); changed = true; }
    ImGui::SameLine(); if (ImGui::Combo("Camera easing", &easing, "Linear\0Smoothstep\0Hold\0")) { draft.cue.eEasing = static_cast<VALTAN_CINEMATIC_CAMERA_EASING>(easing); changed = true; }
    if (ImGui::Button("Capture key at cursor"))
    { const auto age = ClockMs() > draft.startMs ? (std::min)(ClockMs() - draft.startMs, draft.cue.iDurationMs) : 0u; changed |= Capture_CameraKey(draft, age); }
    ImGui::SameLine(); if (ImGui::Button("Remove camera row"))
    { rows.erase(found); m_SelectedCamera.clear(); m_Dirty = true; if (m_Active && !Sample()) Stop(); ImGui::PopID(); return; }
    ImGui::TextWrapped("%s", draft.source.c_str());
    if (ImGui::BeginChild("CameraKeys", {0.f, 155.f}, ImGuiChildFlags_Borders))
        for (std::size_t keyIndex = 0; keyIndex < draft.cue.Keyframes.size(); ++keyIndex)
        {
            auto& key = draft.cue.Keyframes[keyIndex];
            ImGui::PushID(key.strSceneId.c_str()); int time = static_cast<int>(key.iTimeMs);
            ImGui::SetNextItemWidth(110.f); if (ImGui::DragInt("Time", &time, 1.f, 0, int(draft.cue.iDurationMs), "%d ms"))
            { key.iTimeMs = static_cast<std::uint32_t>((std::clamp)(time, 0, int(draft.cue.iDurationMs))); changed = true; }
            ImGui::SameLine(); ImGui::SetNextItemWidth(190.f); changed |= ImGui::DragFloat3("Eye", &key.vEye.x, .01f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(190.f); changed |= ImGui::DragFloat3("Look at", &key.vLookAt.x, .01f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(110.f); changed |= ImGui::DragFloat(draft.horizontalFov ? "FOV X" : "FOV Y", &key.fFovYDegrees, .1f, 1.01f, 178.99f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(190.f); changed |= ImGui::DragFloat3("Up", &draft.upVectors[keyIndex].x, .01f);
            ImGui::PopID();
        }
    ImGui::EndChild();
    if (changed)
    {
        if (!Sort_CameraKeys(draft)) { ImGui::PopID(); return; }
        draft.source = "PROJECT_TUNED"; auto staged = rows; staged[static_cast<std::size_t>(found - rows.begin())] = draft;
        if (Validate_CameraRows(staged)) { rows = std::move(staged); m_Dirty = true; if (m_Active && !Sample()) Stop(); }
    }
    ImGui::PopID();
}
void CEffectAuthoringSequencer::Draw_CameraRows(const float labels, const float rowHeight, const float width)
{
    auto& rows = m_Transient ? m_TransientCameraRows : m_CameraRows;
    for (auto& row : rows)
    {
        const auto top = ImGui::GetCursorScreenPos(); ImGui::PushID(row.id.c_str());
        bool muted = row.muted;
        if (ImGui::Checkbox("M", &muted))
        {
            auto staged = rows; staged[static_cast<std::size_t>(&row - rows.data())].muted = muted;
            if (Validate_CameraRows(staged)) { row.muted = muted; m_Dirty = true; }
        }
        ImGui::SameLine(); const std::string label = "Camera / " + row.label;
        if (ImGui::Selectable(label.c_str(), m_SelectedCamera == row.id, 0, {labels - 45.f, rowHeight - 3.f}))
        { m_SelectedCamera = row.id; m_SelectedEffect.clear(); }
        CompositionTimeline::DrawBox(ImGui::GetWindowDrawList(), {top.x + labels + row.startMs * m_Zoom * .001f, top.y},
            {top.x + labels + (row.startMs + row.cue.iDurationMs) * m_Zoom * .001f, top.y + rowHeight - 3.f},
            IM_COL32(115, 80, 185, 230), m_SelectedCamera == row.id, label.c_str(), false, false);
        ImGui::SetCursorScreenPos({top.x, top.y + rowHeight}); ImGui::Dummy({width, 1.f}); ImGui::PopID();
    }
}
}
