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
bool Camera_SampleUp(const VALTAN_CINEMATIC_CAMERA_CUE& cue, const std::vector<float3_t>& ups,
    const float elapsedSeconds, float3_t& out)
{
    if (cue.Keyframes.empty() || ups.size() != cue.Keyframes.size()) return false;
    // Match Sample_Cue's time, segment, easing and control-point selection.
    const float elapsedMs = elapsedSeconds * 1000.f;
    const auto upper = std::upper_bound(cue.Keyframes.begin(), cue.Keyframes.end(), elapsedMs,
        [](float value, const auto& key) { return value < static_cast<float>(key.iTimeMs); });
    if (upper == cue.Keyframes.begin()) { out = ups.front(); return true; }
    if (upper == cue.Keyframes.end()) { out = ups.back(); return true; }
    const auto right = static_cast<std::size_t>(upper - cue.Keyframes.begin()), left = right - 1u;
    float alpha = (std::clamp)((elapsedMs - cue.Keyframes[left].iTimeMs) /
        static_cast<float>(cue.Keyframes[right].iTimeMs - cue.Keyframes[left].iTimeMs), 0.f, 1.f);
    if (cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP) alpha = alpha * alpha * (3.f - 2.f * alpha);
    else if (cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::HOLD) alpha = 0.f;
    else if (cue.eEasing != VALTAN_CINEMATIC_CAMERA_EASING::LINEAR) return false;
    if (cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR)
        XMStoreFloat3(&out, XMVectorLerp(XMLoadFloat3(&ups[left]), XMLoadFloat3(&ups[right]), alpha));
    else if (cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM)
        XMStoreFloat3(&out, XMVectorCatmullRom(XMLoadFloat3(&ups[left ? left - 1u : left]),
            XMLoadFloat3(&ups[left]), XMLoadFloat3(&ups[right]),
            XMLoadFloat3(&ups[(std::min)(right + 1u, ups.size() - 1u)]), alpha));
    else return false;
    return true;
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
{
    if (rows.size() > 64u) { m_Status = "A sequence supports at most 64 camera rows."; return false; }
    std::set<std::string> ids;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> spans;
    for (const auto& row : rows)
    {
        if (!CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second || row.label.size() > 512u ||
            row.source.size() > 2048u || !row.cue.iDurationMs || row.cue.iDurationMs > MAX_CAMERA_MS ||
            row.startMs > MAX_CAMERA_MS - row.cue.iDurationMs || row.upVectors.size() != row.cue.Keyframes.size() || row.cue.Keyframes.empty() || row.cue.Keyframes.size() > 4096u ||
            row.cue.Keyframes.front().iTimeMs != 0u ||
            (row.cue.eInterpolation != VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR &&
             row.cue.eInterpolation != VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM) ||
            (row.cue.eEasing != VALTAN_CINEMATIC_CAMERA_EASING::LINEAR &&
             row.cue.eEasing != VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP && row.cue.eEasing != VALTAN_CINEMATIC_CAMERA_EASING::HOLD))
        { m_Status = "Invalid camera row identity, duration or interpolation."; return false; }
        std::set<std::string> keyIds;
        std::uint32_t previous = 0u;
        for (std::size_t i = 0; i < row.cue.Keyframes.size(); ++i)
        {
            const auto& key = row.cue.Keyframes[i];
            if (!CEffectV2Document::Is_ValidEffectId(key.strSceneId) || !keyIds.insert(key.strSceneId).second ||
                key.iTimeMs > row.cue.iDurationMs || (i && key.iTimeMs <= previous))
            { m_Status = "Camera keys need unique IDs and increasing times inside their row."; return false; }
            previous = key.iTimeMs;
            const float values[] = {key.vEye.x, key.vEye.y, key.vEye.z, key.vLookAt.x, key.vLookAt.y, key.vLookAt.z};
            if (std::any_of(std::begin(values), std::end(values), [](float value) { return !std::isfinite(value) || std::abs(value) > 100000.f; }))
            { m_Status = "Camera coordinates must be finite and within 100000 m."; return false; }
            VALTAN_CINEMATIC_CAMERA_CUE single; single.Keyframes = {key};
            VALTAN_CINEMATIC_CAMERA_POSE pose;
            if (!CValtanCinematicCameraController::Sample_Cue(single, 0.f, pose) ||
                !Camera_ValidUp(key.vEye, key.vLookAt, row.upVectors[i]))
            { m_Status = "Camera key has an invalid eye, target, up vector or FOV."; return false; }
        }
        if (!row.muted) spans.emplace_back(row.startMs, row.startMs + row.cue.iDurationMs);
    }
    std::sort(spans.begin(), spans.end());
    for (std::size_t i = 1; i < spans.size(); ++i)
        if (spans[i].first < spans[i - 1u].second)
        { m_Status = "Unmuted camera rows overlap. Move or mute one row before playback."; return false; }
    return true;
}
bool CEffectAuthoringSequencer::Parse_CameraRows(const DATA_JSON_VALUE& document, const bool required, std::vector<CAMERA_ROW>& out)
{
    const auto* cameras = document.Find("cameras");
    if (!cameras && !required) { out.clear(); return true; }
    if (!cameras || !cameras->Is_Array() || cameras->Get_Array().size() > 64u)
    { m_Status = "Sequence cameras must be an array with at most 64 rows."; return false; }
    std::vector<CAMERA_ROW> staged;
    for (const auto& value : cameras->Get_Array())
    {
        CAMERA_ROW row; std::string space, interpolation, easing;
        const auto* axis = value.Find("fovAxis");
        if (axis && (!axis->Is_String() || (axis->Get_String() != "HORIZONTAL" && axis->Get_String() != "VERTICAL")))
        { m_Status = "Unknown camera FOV axis."; return false; }
        row.horizontalFov = axis && axis->Get_String() == "HORIZONTAL";
        const auto* muted = value.Find("muted"); const auto* keys = value.Find("keys");
        if (!Camera_Text(value, "cameraId", row.id) || !Camera_Text(value, "displayName", row.label) ||
            !Camera_Text(value, "source", row.source) || !Camera_Text(value, "space", space) ||
            !Camera_Text(value, "interpolation", interpolation) || !Camera_Text(value, "easing", easing) ||
            !Camera_Ms(value, "startMs", row.startMs) || !Camera_Ms(value, "durationMs", row.cue.iDurationMs) ||
            !muted || !muted->Is_Boolean() || !keys || !keys->Is_Array() || keys->Get_Array().size() > 4096u)
        { m_Status = "Malformed camera row; the current sequence is preserved."; return false; }
        if (space != "WORLD" && space != "MODEL_ROOT") { m_Status = "Unknown camera coordinate space."; return false; }
        row.modelRelative = space == "MODEL_ROOT"; row.muted = muted->Get_Boolean(); row.cue.strCueId = row.id;
        if (interpolation == "LINEAR") row.cue.eInterpolation = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
        else if (interpolation == "CATMULL_ROM") row.cue.eInterpolation = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
        else { m_Status = "Unknown camera interpolation."; return false; }
        if (easing == "LINEAR") row.cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
        else if (easing == "SMOOTHSTEP") row.cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
        else if (easing == "HOLD") row.cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
        else { m_Status = "Unknown camera easing."; return false; }
        for (const auto& entry : keys->Get_Array())
        {
            VALTAN_CINEMATIC_CAMERA_KEYFRAME key; double fov = 0;
            float3_t up{0.f, 1.f, 0.f};
            const bool legacyFov = !axis && !entry.Find("fovDegrees");
            if (!Camera_Text(entry, "keyId", key.strSceneId) || !Camera_Ms(entry, "timeMs", key.iTimeMs) ||
                !Camera_Vector(entry, "eye", key.vEye) || !Camera_Vector(entry, "lookAt", key.vLookAt) ||
                (legacyFov ? !Camera_Number(entry, "fovYDegrees", fov) :
                    (!axis || entry.Find("fovYDegrees") || !Camera_Number(entry, "fovDegrees", fov))) ||
                ((axis || entry.Find("up")) && !Camera_Vector(entry, "up", up)))
            { m_Status = "Malformed camera key; the current sequence is preserved."; return false; }
            key.fFovYDegrees = static_cast<float>(fov); row.cue.Keyframes.push_back(std::move(key)); row.upVectors.push_back(up);
        }
        staged.push_back(std::move(row));
    }
    if (!Validate_CameraRows(staged)) return false;
    out = std::move(staged); return true;
}
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
bool CEffectAuthoringSequencer::Read_RecoveryCameras(const EFFECT_RESOURCE_KEY& key, std::vector<CAMERA_ROW>& rows)
{
    if (key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT || !key.strStableId.ends_with(".restore"))
    { rows.clear(); return true; }
    if (!key.Is_Valid()) { m_Status = "Invalid recovery Effect identity."; return false; }
    const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Effects") / "Sequences" / (key.strStableId + ".effectsequence.json"));
    std::error_code ec; const bool exists = std::filesystem::exists(path, ec);
    if (ec) { m_Status = "Cannot inspect recovery camera sequence: " + ec.message(); return false; }
    if (!exists) { rows.clear(); return true; }
    const auto bytes = std::filesystem::file_size(path, ec);
    if (ec || bytes > 1024u * 1024u) { m_Status = "Recovery camera sequence exceeds 1 MiB or is unreadable."; return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { m_Status = "Cannot read recovery camera sequence."; return false; }
    const std::string text(std::istreambuf_iterator<char>(input), {});
    DATA_JSON_VALUE root; std::string schema, id, asset, sequence; std::uint32_t version = 0u;
    if (input.bad() || !CDataJson::Parse(text, root, m_Status) || !Camera_Text(root, "schema", schema) ||
        schema != "lostark.effect-authoring-sequence" || !Camera_Ms(root, "formatVersion", version) || version != 3u ||
        !Camera_Text(root, "sequenceId", id) || id != key.strStableId)
    { m_Status = "Invalid recovery camera sequence header; previous preview preserved."; return false; }
    const auto* model = root.Find("model"); const auto* effects = root.Find("effects");
    std::string kind, member, anchorMode; float3_t world;
    if (!model || !Camera_Text(*model, "kind", kind) || kind != "MODEL_SEQUENCE" ||
        !Camera_Text(*model, "assetName", asset) || asset != m_AssetName ||
        !Camera_Text(*model, "sequenceId", sequence) || sequence != m_SelectedSequence ||
        !Camera_Text(*model, "anchorMemberId", member) || !member.empty() ||
        !Camera_Text(root, "anchorMode", anchorMode) || anchorMode != "MODEL_ROOT" ||
        !Camera_Vector(root, "worldPosition", world) || world.x != 0.f || world.y != 0.f || world.z != 0.f ||
        !effects || !effects->Is_Array() || effects->Get_Array().size() != 1u)
    { m_Status = "Recovery camera preset must match its character, skill and single root Effect."; return false; }
    const auto& occurrence = effects->Get_Array().front();
    std::string effect, owner, occurrenceId, anchor; float3_t offset; std::uint32_t start = 0u, duration = 0u;
    const auto* muted = occurrence.Find("muted");
    if (!Camera_Text(occurrence, "effectId", effect) || effect != key.strStableId ||
        !Camera_Text(occurrence, "owner", owner) || owner != "V1_DOCUMENT" ||
        !Camera_Text(occurrence, "occurrenceId", occurrenceId) || !CEffectV2Document::Is_ValidEffectId(occurrenceId) ||
        !Camera_Text(occurrence, "anchorSlotId", anchor) || anchor != "root" ||
        !Camera_Ms(occurrence, "startMs", start) || start != 0u || !Camera_Ms(occurrence, "durationMs", duration) || !duration ||
        !Camera_Vector(occurrence, "offset", offset) || offset.x != 0.f || offset.y != 0.f || offset.z != 0.f ||
        !muted || !muted->Is_Boolean() || muted->Get_Boolean())
    { m_Status = "Recovery camera preset has incompatible Effect timing, anchor or offset."; return false; }
    std::vector<CAMERA_ROW> staged;
    if (!Parse_CameraRows(root, true, staged)) return false;
    for (const auto& row : staged)
        if (row.startMs + row.cue.iDurationMs > duration)
        { m_Status = "Recovery camera extends beyond its Effect occurrence."; return false; }
    if (!staged.empty() && (m_UseKouku || !m_ModelRoot || m_DefaultAnchorSlotId != "root"))
    { m_Status = "Select Model root / root anchor to preview this character camera preset."; return false; }
    rows = std::move(staged); return true;
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
    const float seconds = (clockMs - active->startMs) * .001f;
    if (!CValtanCinematicCameraController::Sample_Cue(active->cue, seconds, pose) ||
        !Camera_SampleUp(active->cue, active->upVectors, seconds, up))
    { m_Status = "The shared camera sampler rejected the current row."; return false; }
    if (active->modelRelative)
    {
        XMStoreFloat3(&pose.vEye, XMVector3TransformCoord(XMLoadFloat3(&pose.vEye), XMLoadFloat4x4(&root)));
        XMStoreFloat3(&pose.vLookAt, XMVector3TransformCoord(XMLoadFloat3(&pose.vLookAt), XMLoadFloat4x4(&root)));
        XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), XMLoadFloat4x4(&root)));
    }
    if (!Camera_ValidUp(pose.vEye, pose.vLookAt, up) ||
        (active->horizontalFov && !Camera_ConvertFov(pose.fFovYDegrees, camera->Get_AspectRatio(), false)))
    { m_Status = "Camera up vector or projection FOV is invalid; the prior pose is preserved."; return false; }
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
            auto cameras = m_CameraRows;
            for (auto row : m_TransientCameraRows)
            { row.id = CEffectEditingSession::New_Id("camera.row."); row.cue.strCueId = row.id; cameras.push_back(std::move(row)); }
            if (m_Effects.size() >= 256u) m_Status = "The sequence already has 256 Effect rows.";
            else if (Validate_CameraRows(cameras))
            {
                auto row = std::move(*m_Transient); row.id = CEffectEditingSession::New_Id("effect.occurrence.");
                m_Effects.push_back(std::move(row)); m_CameraRows = std::move(cameras);
                m_Transient.reset(); m_TransientCameraRows.clear(); m_SelectedCamera.clear(); m_Dirty = true;
                m_Status = "Preview and camera rows appended. Save sequence preserves this arrangement.";
                return;
            }
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
