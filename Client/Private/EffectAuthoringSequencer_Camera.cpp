#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "Camera.h"
#include "Camera_Free.h"
#include "CameraTool.h"
#include "CompositionTimeline.h"
#include "DataJson.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
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
                << up.x << ", " << up.y << ", " << up.z << "], \"fovDegrees\": " << key.fFovYDegrees;
            if (key.cutBefore) out << ", \"cutBefore\": true";
            out << "}";
        }
        out << "]";
        if (!row.sourceEffectId.empty())
            out << ", \"sourceCamera\": {\"effectId\": \"" << CDataJson::Escape(row.sourceEffectId)
                << "\", \"cameraId\": \"" << CDataJson::Escape(row.sourceCameraId) << "\", \"clockAtStartMs\": "
                << row.sourceClockAtStartMs << ", \"playRate\": " << row.sourcePlayRate << "}";
        out << "}";
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
    for (auto& camera : staged)
    { camera.sourceEffectId = key.strStableId; camera.sourceCameraId = camera.id; camera.sourceClockAtStartMs = camera.startMs; }
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
    const auto followCamera = std::dynamic_pointer_cast<CCamera_Free>(camera);
    if (followCamera && !followCamera->Is_FollowEnabled()) { Release_Camera(); return true; }
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
    if (!found->sourceEffectId.empty())
    {
        if (ImGui::Button("Open Sequence Camera Tool")) (void)Open_RecoveryCameraTool(*found);
        ImGui::TextWrapped("Edit and save this camera in its original recovery sequence. Action arrangement Save does not replace the Product camera source.");
        return;
    }
    if (found->id.starts_with("character.camera.preview."))
        ImGui::TextWrapped("This saved arrangement has no exact original camera link. Reopen the Product action to edit its canonical camera; these arrangement keys remain separate.");
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
bool CEffectAuthoringSequencer::Ensure_RecoveryCameraSaved(std::string& status)
{
    if (!m_RecoveryCameraSession.Is_Dirty()) return true;
    m_RecoveryCameraOpen = true;
    status = "Save camera source in Sequence Camera Tool first. Arrangement Save does not commit the Product camera.";
    m_RecoveryCameraStatus = status; return false;
}
bool CEffectAuthoringSequencer::Open_RecoveryCameraTool(const CAMERA_ROW& row)
{
    if (row.sourceEffectId.empty()) { m_Status = "The selected row has no canonical recovery camera owner."; return false; }
    if (m_RecoveryCameraSession.EffectId() != row.sourceEffectId)
    {
        if (m_RecoveryCameraDraft && m_RecoveryCameraSession.Is_Dirty())
        { m_Status = "Save the current Sequence Camera Tool draft before opening another source."; return false; }
        if (!m_RecoveryCameraSession.Open(row.sourceEffectId, m_RecoveryCameraStatus)) { m_Status = m_RecoveryCameraStatus; return false; }
        m_RecoveryCameraDraft.reset(); m_RecoveryCameraPending = false;
    }
    if (!m_RecoveryCameraDraft || m_RecoveryCameraDraft->id != row.sourceCameraId)
    {
        if (m_RecoveryCameraDraft && !m_RecoveryCameraSession.Apply(*m_RecoveryCameraDraft, m_RecoveryCameraStatus)) return false;
        const auto& cameras = m_RecoveryCameraSession.Rows();
        const auto camera = std::find_if(cameras.begin(), cameras.end(), [&](const auto& value) { return value.id == row.sourceCameraId; });
        if (camera == cameras.end()) { m_Status = "Original camera cut is no longer present."; return false; }
        m_RecoveryCameraDraft = *camera; m_RecoveryCameraEditor = {};
    }
    m_RecoveryCameraOpen = true; return true;
}
bool CEffectAuthoringSequencer::Refresh_RecoveryCameraDraft()
{
    auto saved = m_CameraRows, transient = m_TransientCameraRows;
    const auto project = [&](std::vector<CAMERA_ROW>& rows, const bool directPreview) {
        for (auto& row : rows)
        {
            if (row.sourceEffectId != m_RecoveryCameraSession.EffectId()) continue;
            const auto& canonical = m_RecoveryCameraSession.Rows();
            const auto original = std::find_if(canonical.begin(), canonical.end(), [&](const auto& value) { return value.id == row.sourceCameraId; });
            if (original == canonical.end()) { m_RecoveryCameraStatus = "Projected camera source disappeared."; return false; }
            if (directPreview && row.id == row.sourceCameraId)
            {
                const auto effect = row.sourceEffectId; row = *original;
                row.sourceEffectId = effect; row.sourceCameraId = row.id; row.sourceClockAtStartMs = row.startMs; continue;
            }
            if (row.sourceClockAtStartMs < original->startMs || !std::isfinite(row.sourcePlayRate) || row.sourcePlayRate <= 0.f) return false;
            const auto begin = row.sourceClockAtStartMs - original->startMs;
            const auto end = (std::min)(original->cue.iDurationMs, begin + static_cast<std::uint32_t>(std::llround(row.cue.iDurationMs * row.sourcePlayRate)));
            auto sample = *original; sample.modelRelative = false; sample.horizontalFov = false; sample.startMs = 0u; sample.muted = false; ++sample.cue.iDurationMs;
            std::set<std::uint32_t> times{begin, end};
            for (const auto& key : original->cue.Keyframes) if (key.iTimeMs > begin && key.iTimeMs < end) times.insert(key.iTimeMs);
            row.cue.Keyframes.clear(); row.upVectors.clear(); row.cue.eInterpolation = original->cue.eInterpolation; row.cue.eEasing = original->cue.eEasing;
            row.horizontalFov = original->horizontalFov; row.modelRelative = original->modelRelative; row.source = original->source;
            float4x4_t identity; XMStoreFloat4x4(&identity, XMMatrixIdentity());
            for (const auto time : times)
            {
                VALTAN_CINEMATIC_CAMERA_POSE pose; float3_t up;
                if (!CEffectRecoveryCamera::Sample(sample, time, identity, 1.f, pose, up, m_RecoveryCameraStatus)) return false;
                VALTAN_CINEMATIC_CAMERA_KEYFRAME key;
                key.iTimeMs = time == end ? row.cue.iDurationMs : static_cast<std::uint32_t>((time - begin) / row.sourcePlayRate);
                key.strSceneId = row.id + ".key." + std::to_string(key.iTimeMs);
                key.vEye = pose.vEye; key.vLookAt = pose.vLookAt; key.fFovYDegrees = pose.fFovYDegrees;
                const auto exact = std::find_if(original->cue.Keyframes.begin(), original->cue.Keyframes.end(), [&](const auto& value) { return value.iTimeMs == time; });
                key.cutBefore = exact != original->cue.Keyframes.end() && exact->cutBefore;
                if (!row.cue.Keyframes.empty() && row.cue.Keyframes.back().iTimeMs == key.iTimeMs)
                { row.cue.Keyframes.back() = key; row.upVectors.back() = up; }
                else { row.cue.Keyframes.push_back(std::move(key)); row.upVectors.push_back(up); }
            }
        }
        return CEffectRecoveryCamera::Validate(rows, m_RecoveryCameraStatus);
    };
    if (!project(saved, false) || !project(transient, true)) return false;
    m_CameraRows = std::move(saved); m_TransientCameraRows = std::move(transient);
    if (m_Active && !Sample()) return false;
    return true;
}
void CEffectAuthoringSequencer::Render_RecoveryCameraTool()
{
    if (!m_RecoveryCameraOpen || !m_RecoveryCameraDraft) return;
    ImGui::SetNextWindowSize({820.f, 720.f}, ImGuiCond_FirstUseEver);
    ImGui::PushID(this);
    if (ImGui::Begin("Sequence Camera Tool##Recovery", &m_RecoveryCameraOpen))
    {
        ImGui::TextWrapped("%s", m_RecoveryCameraSession.EffectId().c_str());
        const auto apply = [&]() {
            if (!m_RecoveryCameraSession.Apply(*m_RecoveryCameraDraft, m_RecoveryCameraStatus)) return false;
            if (!Refresh_RecoveryCameraDraft()) m_RecoveryCameraStatus =
                "Camera draft is valid; current preview could not refresh: " + m_RecoveryCameraStatus;
            return true;
        };
        const auto save = [&]() {
            if (!apply() || !m_RecoveryCameraSession.Save(m_RecoveryCameraStatus)) return false;
            const auto& rows = m_RecoveryCameraSession.Rows();
            const auto selected = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.id == m_RecoveryCameraDraft->id; });
            if (selected != rows.end()) m_RecoveryCameraDraft = *selected;
            m_RecoveryCameraPending = true;
            const auto savedStatus = m_RecoveryCameraStatus;
            if (!Refresh_RecoveryCameraDraft()) m_RecoveryCameraStatus = savedStatus + " Preview refresh: " + m_RecoveryCameraStatus;
            return true;
        };
        if (ImGui::Button("Save camera source")) (void)save();
        ImGui::SameLine();
        if (ImGui::Button("Publish saved cameras"))
        {
            if (m_RecoveryCameraSession.Is_Dirty())
                m_RecoveryCameraStatus = "Save the camera source before Publish. Unsaved keys were preserved.";
            else if (CEffectPresentationService::Reload_ProductCamera(m_RecoveryCameraSession.EffectId(), m_RecoveryCameraStatus))
                m_RecoveryCameraPending = false;
        }
        ImGui::SameLine(); ImGui::TextDisabled("%s", m_RecoveryCameraSession.Is_Dirty() ? "Unsaved" : m_RecoveryCameraPending ? "Saved; Publish pending" : "Saved");
        ImGui::TextWrapped("Save keeps canonical camera keys. Publish updates Client camera playback; Server skill timing remains with the existing action owner.");
        if (ImGui::BeginCombo("Cut", m_RecoveryCameraDraft->label.c_str()))
        {
            // Copy the list because committing the current draft replaces it.
            const auto rows = m_RecoveryCameraSession.Rows();
            for (const auto& row : rows) if (ImGui::Selectable((row.label + "###" + row.id).c_str(), row.id == m_RecoveryCameraDraft->id))
            { if (apply()) { m_RecoveryCameraDraft = row; m_RecoveryCameraEditor = {}; } }
            ImGui::EndCombo();
        }
        const auto& previews = m_Transient ? m_TransientCameraRows : m_CameraRows;
        const auto preview = std::find_if(previews.begin(), previews.end(), [&](const auto& row) {
            return row.sourceEffectId == m_RecoveryCameraSession.EffectId() && row.sourceCameraId == m_RecoveryCameraDraft->id; });
        std::uint32_t localMs = 0u;
        if (preview != previews.end() && ClockMs() >= preview->startMs)
        {
            const auto sourceMs = preview->sourceClockAtStartMs + static_cast<std::uint32_t>((ClockMs() - preview->startMs) * preview->sourcePlayRate);
            if (sourceMs >= m_RecoveryCameraDraft->startMs) localMs = (std::min)(sourceMs - m_RecoveryCameraDraft->startMs, m_RecoveryCameraDraft->cue.iDurationMs);
        }
        const auto edit = CSequenceCameraEditor::Render(*m_RecoveryCameraDraft, m_RecoveryCameraEditor, localMs);
        // Resolve seek before Apply replaces the projected row vectors.
        std::optional<std::uint32_t> seek;
        if (edit.seekLocalMs && preview != previews.end())
        {
            const auto sourceMs = m_RecoveryCameraDraft->startMs + *edit.seekLocalMs;
            if (sourceMs >= preview->sourceClockAtStartMs)
            {
                const auto target = preview->startMs + static_cast<std::uint32_t>((sourceMs - preview->sourceClockAtStartMs) / preview->sourcePlayRate);
                if (target <= preview->startMs + preview->cue.iDurationMs) seek = (std::min)(target, DurationMs());
            }
            if (!seek) m_RecoveryCameraStatus = "This key is outside the selected Action clip window; its canonical value remains editable.";
        }
        if (edit.changed) { m_RecoveryCameraPending = true; (void)apply(); }
        if (seek) { Pause(true); (void)Seek(*seek); }
        ImGui::TextWrapped("%s", m_RecoveryCameraStatus.c_str());
    }
    ImGui::End(); ImGui::PopID();
}
void CEffectAuthoringSequencer::Draw_CameraRows(const float labels, const float rowHeight, const float width)
{
    auto& rows = m_Transient ? m_TransientCameraRows : m_CameraRows;
    std::vector<CompositionTimeline::DISPLAY_INTERVAL> intervals;
    for (const auto& row : rows) intervals.push_back({row.id, double(row.startMs), double(row.cue.iDurationMs), 0., {}});
    const auto layout = CompositionTimeline::AllocateDisplayRows(std::move(intervals), 8. * 1000. / (std::max)(m_Zoom, .001f));
    const auto origin = ImGui::GetCursorScreenPos();
    for (std::size_t lane = 0; lane < layout.rowCount; ++lane)
        ImGui::GetWindowDrawList()->AddText({origin.x + 5.f, origin.y + lane * rowHeight + 4.f}, IM_COL32(180,170,210,255), "Camera");
    for (auto& row : rows)
    {
        ImGui::PushID(row.id.c_str());
        const float top = origin.y + layout.occurrenceRows.at(row.id) * rowHeight;
        const float left = origin.x + labels + row.startMs * m_Zoom * .001f;
        const float right = (std::max)(left + 8.f, origin.x + labels + (row.startMs + row.cue.iDurationMs) * m_Zoom * .001f);
        CompositionTimeline::DrawBox(ImGui::GetWindowDrawList(), {left, top}, {right, top + rowHeight - 3.f},
            row.muted ? IM_COL32(73,75,81,200) : IM_COL32(115,80,185,230), m_SelectedCamera == row.id, row.label.c_str(), false, false);
        ImGui::SetCursorScreenPos({left, top}); ImGui::InvisibleButton("Camera cut", {right - left, rowHeight - 3.f});
        if (ImGui::IsItemActivated()) { Select_TimelineRow(TRACK_KIND::CAMERA, row.id); m_Interaction = true; }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) (void)Open_RecoveryCameraTool(row);
        if (ImGui::BeginPopupContextItem("Camera menu"))
        {
            if (ImGui::MenuItem("Open Sequence Camera Tool")) (void)Open_RecoveryCameraTool(row);
            if (row.sourceEffectId.empty() && ImGui::MenuItem(row.muted ? "Unmute" : "Mute"))
            {
                auto staged = rows; staged[static_cast<std::size_t>(&row - rows.data())].muted = !row.muted;
                if (Validate_CameraRows(staged)) { row.muted = !row.muted; m_Dirty = true; }
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::SetCursorScreenPos({origin.x, origin.y + layout.rowCount * rowHeight}); ImGui::Dummy({width, 1.f});
}
}
