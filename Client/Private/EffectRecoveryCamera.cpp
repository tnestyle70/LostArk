#include "EffectRecoveryCamera.h"
#include "EffectV2_Document.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <set>

namespace Client
{
namespace
{
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

bool CEffectRecoveryCamera::Validate(const std::vector<EFFECT_CAMERA_ROW>& rows, std::string& error)
{
    if (rows.size() > 64u) { error = "A sequence supports at most 64 camera rows."; return false; }
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
        { error = "Invalid camera row identity, duration or interpolation."; return false; }
        std::set<std::string> keyIds;
        std::uint32_t previous = 0u;
        for (std::size_t i = 0; i < row.cue.Keyframes.size(); ++i)
        {
            const auto& key = row.cue.Keyframes[i];
            if (!CEffectV2Document::Is_ValidEffectId(key.strSceneId) || !keyIds.insert(key.strSceneId).second ||
                key.iTimeMs > row.cue.iDurationMs || (i && key.iTimeMs <= previous))
            { error = "Camera keys need unique IDs and increasing times inside their row."; return false; }
            previous = key.iTimeMs;
            const float values[] = {key.vEye.x, key.vEye.y, key.vEye.z, key.vLookAt.x, key.vLookAt.y, key.vLookAt.z};
            if (std::any_of(std::begin(values), std::end(values), [](float value) { return !std::isfinite(value) || std::abs(value) > 100000.f; }))
            { error = "Camera coordinates must be finite and within 100000 m."; return false; }
            VALTAN_CINEMATIC_CAMERA_CUE single; single.Keyframes = {key};
            VALTAN_CINEMATIC_CAMERA_POSE pose;
            if (!CValtanCinematicCameraController::Sample_Cue(single, 0.f, pose) ||
                !Camera_ValidUp(key.vEye, key.vLookAt, row.upVectors[i]))
            { error = "Camera key has an invalid eye, target, up vector or FOV."; return false; }
        }
        if (!row.muted) spans.emplace_back(row.startMs, row.startMs + row.cue.iDurationMs);
    }
    std::sort(spans.begin(), spans.end());
    for (std::size_t i = 1; i < spans.size(); ++i)
        if (spans[i].first < spans[i - 1u].second)
        { error = "Unmuted camera rows overlap. Move or mute one row before playback."; return false; }
    return true;
}
bool CEffectRecoveryCamera::Parse(const DATA_JSON_VALUE& document, const bool required, std::vector<EFFECT_CAMERA_ROW>& out, std::string& error)
{
    const auto* cameras = document.Find("cameras");
    if (!cameras && !required) { out.clear(); return true; }
    if (!cameras || !cameras->Is_Array() || cameras->Get_Array().size() > 64u)
    { error = "Sequence cameras must be an array with at most 64 rows."; return false; }
    std::vector<EFFECT_CAMERA_ROW> staged;
    for (const auto& value : cameras->Get_Array())
    {
        EFFECT_CAMERA_ROW row; std::string space, interpolation, easing;
        const auto* axis = value.Find("fovAxis");
        if (axis && (!axis->Is_String() || (axis->Get_String() != "HORIZONTAL" && axis->Get_String() != "VERTICAL")))
        { error = "Unknown camera FOV axis."; return false; }
        row.horizontalFov = axis && axis->Get_String() == "HORIZONTAL";
        const auto* muted = value.Find("muted"); const auto* keys = value.Find("keys");
        if (!Camera_Text(value, "cameraId", row.id) || !Camera_Text(value, "displayName", row.label) ||
            !Camera_Text(value, "source", row.source) || !Camera_Text(value, "space", space) ||
            !Camera_Text(value, "interpolation", interpolation) || !Camera_Text(value, "easing", easing) ||
            !Camera_Ms(value, "startMs", row.startMs) || !Camera_Ms(value, "durationMs", row.cue.iDurationMs) ||
            !muted || !muted->Is_Boolean() || !keys || !keys->Is_Array() || keys->Get_Array().size() > 4096u)
        { error = "Malformed camera row; the current sequence is preserved."; return false; }
        if (space != "WORLD" && space != "MODEL_ROOT") { error = "Unknown camera coordinate space."; return false; }
        row.modelRelative = space == "MODEL_ROOT"; row.muted = muted->Get_Boolean(); row.cue.strCueId = row.id;
        if (interpolation == "LINEAR") row.cue.eInterpolation = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
        else if (interpolation == "CATMULL_ROM") row.cue.eInterpolation = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
        else { error = "Unknown camera interpolation."; return false; }
        if (easing == "LINEAR") row.cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
        else if (easing == "SMOOTHSTEP") row.cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
        else if (easing == "HOLD") row.cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
        else { error = "Unknown camera easing."; return false; }
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
            { error = "Malformed camera key; the current sequence is preserved."; return false; }
            key.fFovYDegrees = static_cast<float>(fov); row.cue.Keyframes.push_back(std::move(key)); row.upVectors.push_back(up);
        }
        staged.push_back(std::move(row));
    }
    if (!Validate(staged, error)) return false;
    out = std::move(staged); return true;
}
bool CEffectRecoveryCamera::Load(const std::string& effectId, EFFECT_RECOVERY_CAMERA_DOCUMENT& out, std::string& error)
{
    if (!CEffectV2Document::Is_ValidEffectId(effectId) || !effectId.ends_with(".restore"))
    { error = "Invalid recovery Effect identity."; return false; }
    const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Effects") / "Sequences" / (effectId + ".effectsequence.json"));
    std::error_code ec; const bool exists = std::filesystem::exists(path, ec);
    if (ec) { error = "Cannot inspect recovery camera sequence: " + ec.message(); return false; }
    if (!exists) { out = {}; return true; }
    const auto bytes = std::filesystem::file_size(path, ec);
    if (ec || bytes > 1024u * 1024u) { error = "Recovery camera sequence exceeds 1 MiB or is unreadable."; return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "Cannot read recovery camera sequence."; return false; }
    const std::string text(std::istreambuf_iterator<char>(input), {});
    EFFECT_RECOVERY_CAMERA_DOCUMENT staged; std::string schema, id;
    auto& root = staged.document;
    if (input.bad() || !CDataJson::Parse(text, root, error) || !Camera_Text(root, "schema", schema) ||
        schema != "lostark.effect-authoring-sequence" || !Camera_Ms(root, "formatVersion", staged.version) ||
        (staged.version != 3u && staged.version != 4u) || !Camera_Text(root, "sequenceId", id) || id != effectId)
    { error = "Invalid recovery camera sequence header; previous preview preserved."; return false; }
    const auto* model = root.Find("model"); const auto* effects = root.Find("effects");
    std::string kind, member, anchorMode; float3_t world;
    if (!model || !Camera_Text(*model, "kind", kind) || kind != "MODEL_SEQUENCE" ||
        !Camera_Text(*model, "assetName", staged.asset) || staged.asset.empty() ||
        !Camera_Text(*model, "sequenceId", staged.sequence) || staged.sequence.empty() ||
        !Camera_Text(*model, "anchorMemberId", member) || !member.empty() ||
        !Camera_Text(root, "anchorMode", anchorMode) || anchorMode != "MODEL_ROOT" ||
        !Camera_Vector(root, "worldPosition", world) || world.x != 0.f || world.y != 0.f || world.z != 0.f ||
        !effects || !effects->Is_Array() || effects->Get_Array().size() != 1u)
    { error = "Recovery camera preset must identify its character, skill and single root Effect."; return false; }
    const auto& occurrence = effects->Get_Array().front();
    std::string effect, owner, occurrenceId, anchor; float3_t offset; std::uint32_t start = 0u;
    const auto* muted = occurrence.Find("muted");
    if (!Camera_Text(occurrence, "effectId", effect) || effect != effectId ||
        !Camera_Text(occurrence, "owner", owner) || owner != "V1_DOCUMENT" ||
        !Camera_Text(occurrence, "occurrenceId", occurrenceId) || !CEffectV2Document::Is_ValidEffectId(occurrenceId) ||
        !Camera_Text(occurrence, "anchorSlotId", anchor) || anchor != "root" ||
        !Camera_Ms(occurrence, "startMs", start) || start != 0u ||
        !Camera_Ms(occurrence, "durationMs", staged.durationMs) || !staged.durationMs ||
        !Camera_Vector(occurrence, "offset", offset) || offset.x != 0.f || offset.y != 0.f || offset.z != 0.f ||
        !muted || !muted->Is_Boolean() || muted->Get_Boolean())
    { error = "Recovery camera preset has incompatible Effect timing, anchor or offset."; return false; }
    if (const auto* local = root.Find("localOnlyElementIds"))
    {
        if (!local->Is_Array() || local->Get_Array().size() > 4096u)
        { error = "Recovery local-only elements must be an array of stable IDs."; return false; }
        std::set<std::string> ids;
        for (const auto& id : local->Get_Array())
        {
            if (!id.Is_String() || !CEffectV2Document::Is_ValidEffectId(id.Get_String()) || !ids.insert(id.Get_String()).second)
            { error = "Recovery local-only element IDs must be valid and unique."; return false; }
            staged.localOnlyElementIds.push_back(id.Get_String());
        }
        std::sort(staged.localOnlyElementIds.begin(), staged.localOnlyElementIds.end());
    }
    if (!Parse(root, true, staged.rows, error)) return false;
    for (const auto& row : staged.rows)
        if (row.startMs + row.cue.iDurationMs > staged.durationMs)
        { error = "Recovery camera extends beyond its Effect occurrence."; return false; }
    staged.exists = true; out = std::move(staged); error.clear(); return true;
}

bool CEffectRecoveryCamera::Sample(const EFFECT_CAMERA_ROW& row, const std::uint32_t clockMs,
    const float4x4_t& root, const float aspect, VALTAN_CINEMATIC_CAMERA_POSE& pose, float3_t& up, std::string& error)
{
    if (clockMs < row.startMs || clockMs >= row.startMs + row.cue.iDurationMs || row.muted)
    { error = "Camera row is inactive at this clock."; return false; }
    const float seconds = (clockMs - row.startMs) * .001f;
    if (!CValtanCinematicCameraController::Sample_Cue(row.cue, seconds, pose) ||
        !Camera_SampleUp(row.cue, row.upVectors, seconds, up))
    { error = "The shared camera sampler rejected the current row."; return false; }
    if (row.modelRelative)
    {
        XMStoreFloat3(&pose.vEye, XMVector3TransformCoord(XMLoadFloat3(&pose.vEye), XMLoadFloat4x4(&root)));
        XMStoreFloat3(&pose.vLookAt, XMVector3TransformCoord(XMLoadFloat3(&pose.vLookAt), XMLoadFloat4x4(&root)));
        XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), XMLoadFloat4x4(&root)));
    }
    if (!Camera_ValidUp(pose.vEye, pose.vLookAt, up) ||
        (row.horizontalFov && !Camera_ConvertFov(pose.fFovYDegrees, aspect, false)))
    { error = "Camera up vector or projection FOV is invalid; the prior pose is preserved."; return false; }
    return true;
}
}
