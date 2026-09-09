#include "EffectAuthoringSequencer.h"
#include "DataJson.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include <cmath>
#include <limits>
#include <ostream>

namespace Client
{
namespace
{
constexpr std::uint32_t TRACK_MAX_MS = 600000u;
constexpr std::size_t TRACK_MAX_ROWS = 256u;
bool Track_Text(const DATA_JSON_VALUE& object, const char* key, std::string& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_String()) return false;
    result = value->Get_String(); return true;
}
bool Track_Bool(const DATA_JSON_VALUE& object, const char* key, bool& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Boolean()) return false;
    result = value->Get_Boolean(); return true;
}
bool Track_Number(const DATA_JSON_VALUE& object, const char* key, double& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
    result = value->Get_Number(); return true;
}
bool Track_Ms(const DATA_JSON_VALUE& object, const char* key, std::uint32_t& result)
{
    double value = 0.0;
    if (!Track_Number(object, key, value) || value < 0.0 || value > TRACK_MAX_MS || std::floor(value) != value) return false;
    result = static_cast<std::uint32_t>(value); return true;
}
bool Track_Vector(const DATA_JSON_VALUE& object, const char* key, std::array<double, 3u>& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Array() || value->Get_Array().size() != 3u) return false;
    for (std::size_t i = 0u; i < result.size(); ++i)
    {
        const auto& component = value->Get_Array()[i];
        if (!component.Is_Number() || !std::isfinite(component.Get_Number()) || std::abs(component.Get_Number()) > 100000.0) return false;
        result[i] = component.Get_Number();
    }
    return true;
}
bool Track_Vector(const DATA_JSON_VALUE& object, const char* key, float3_t& result)
{
    std::array<double, 3u> value{};
    if (!Track_Vector(object, key, value)) return false;
    result = {static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2])}; return true;
}
void Track_WriteVector(std::ostream& out, const float3_t& value)
{ out << '[' << value.x << ", " << value.y << ", " << value.z << ']'; }
}

bool CEffectAuthoringSequencer::Parse_AdditionalRows(const DATA_JSON_VALUE& document, const std::uint32_t version,
    std::vector<CLIP>& animations, bool& customAnimation,
    std::vector<SOUND_ROW>& sounds, std::vector<COLLIDER_ROW>& colliders)
{
    animations.clear(); sounds.clear(); colliders.clear(); customAnimation = false;
    if (version < 4u) return true;
    const auto* animationValues = document.Find("animationRows");
    const auto* soundValues = document.Find("soundRows");
    const auto* colliderValues = document.Find("colliderRows");
    if (!Track_Bool(document, "customAnimation", customAnimation) ||
        !animationValues || !animationValues->Is_Array() || animationValues->Get_Array().size() > TRACK_MAX_ROWS ||
        !soundValues || !soundValues->Is_Array() || soundValues->Get_Array().size() > TRACK_MAX_ROWS ||
        !colliderValues || !colliderValues->Is_Array() || colliderValues->Get_Array().size() > TRACK_MAX_ROWS ||
        (!customAnimation && !animationValues->Get_Array().empty()))
    { m_Status = "Invalid v4 track arrays or custom animation mode; current timeline preserved."; return false; }
    for (const auto& value : animationValues->Get_Array())
    {
        CLIP row; double rate = 0.0;
        if (!Track_Text(value, "occurrenceId", row.id) || !Track_Text(value, "displayName", row.label) ||
            !Track_Text(value, "memberId", row.memberId) || !Track_Text(value, "clipName", row.clipName) ||
            !Track_Ms(value, "startMs", row.startMs) || !Track_Ms(value, "durationMs", row.durationMs) ||
            !Track_Ms(value, "sourceStartMs", row.sourceStartMs) || !Track_Ms(value, "sourcePlayMs", row.sourcePlayMs) ||
            !Track_Number(value, "playRate", rate) || rate <= 0.0 || rate > 1000.0 ||
            !Track_Bool(value, "loop", row.loop) || !Track_Bool(value, "muted", row.muted))
        { m_Status = "Invalid Animation occurrence; current timeline preserved."; return false; }
        row.playRate = static_cast<float>(rate); animations.push_back(std::move(row));
    }
    for (const auto& value : soundValues->Get_Array())
    {
        SOUND_ROW row; double volume = 0.0;
        if (!Track_Text(value, "occurrenceId", row.id) || !Track_Text(value, "displayName", row.label) ||
            !Track_Text(value, "assetId", row.assetId) || !Track_Ms(value, "startMs", row.startMs) ||
            !Track_Ms(value, "durationMs", row.durationMs) || !Track_Ms(value, "sourceStartMs", row.sourceStartMs) ||
            !Track_Number(value, "volume", volume) || std::abs(volume) > 1000.0 || !Track_Bool(value, "muted", row.muted))
        { m_Status = "Invalid Sound occurrence; current timeline preserved."; return false; }
        row.volume = static_cast<float>(volume); sounds.push_back(std::move(row));
    }
    for (const auto& value : colliderValues->Get_Array())
    {
        COLLIDER_ROW row; std::string kind;
        const auto* resource = value.Find("resource");
        if (!Track_Text(value, "occurrenceId", row.id) || !Track_Text(value, "displayName", row.label) ||
            !Track_Ms(value, "startMs", row.startMs) || !Track_Ms(value, "durationMs", row.durationMs) ||
            !Track_Vector(value, "offset", row.offset) || !Track_Vector(value, "rotationDegrees", row.rotation) ||
            !Track_Vector(value, "scale", row.scale) || !Track_Text(value, "anchorSlotId", row.anchorSlotId) ||
            !Track_Bool(value, "muted", row.muted) || !Track_Bool(value, "debugRender", row.debugRender) ||
            !resource || !resource->Is_Object() || !Track_Text(*resource, "resourceId", row.resource.strResourceId) ||
            !Track_Text(*resource, "displayName", row.resource.strDisplayName) ||
            !Track_Text(*resource, "kind", kind) || kind != "COLLIDER" ||
            !Track_Text(*resource, "assetId", row.resource.strAssetId) ||
            !Track_Text(*resource, "resourceKind", row.resource.strResourceKind) ||
            !Track_Text(*resource, "defaultAnchorKind", row.resource.strDefaultAnchorKind) ||
            !Track_Ms(*resource, "durationMs", row.resource.iDurationMs) ||
            !Track_Text(*resource, "shape", row.resource.strShape) ||
            !Track_Text(*resource, "colliderKind", row.resource.strColliderKind) ||
            !Track_Vector(*resource, "halfExtents", row.resource.HalfExtents) ||
            !Track_Number(*resource, "radiusM", row.resource.fRadiusM) ||
            !Track_Number(*resource, "halfAngleDegrees", row.resource.fHalfAngleDegrees))
        { m_Status = "Invalid Collider occurrence or resource; current timeline preserved."; return false; }
        row.resource.eKind = KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
        colliders.push_back(std::move(row));
    }
    return Validate_AnimationRows(animations) && Validate_SoundRows(sounds) && Validate_ColliderRows(colliders);
}

void CEffectAuthoringSequencer::Write_AdditionalRows(std::ostream& out) const
{
    const auto precision = out.precision(std::numeric_limits<double>::max_digits10);
    const auto quote = [&](const std::string& value) { out << '"' << CDataJson::Escape(value) << '"'; };
    out << ",\n  \"customAnimation\": " << (m_CustomAnimation ? "true" : "false") << ",\n  \"animationRows\": [";
    for (std::size_t i = 0u; i < m_AnimationRows.size(); ++i)
    {
        const auto& row = m_AnimationRows[i];
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": "; quote(row.id);
        out << ", \"displayName\": "; quote(row.label); out << ", \"memberId\": "; quote(row.memberId);
        out << ", \"clipName\": "; quote(row.clipName);
        out << ", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs
            << ", \"sourceStartMs\": " << row.sourceStartMs << ", \"sourcePlayMs\": " << row.sourcePlayMs
            << ", \"playRate\": " << row.playRate << ", \"loop\": " << (row.loop ? "true" : "false")
            << ", \"muted\": " << (row.muted ? "true" : "false") << '}';
    }
    out << "\n  ],\n  \"soundRows\": [";
    for (std::size_t i = 0u; i < m_Sounds.size(); ++i)
    {
        const auto& row = m_Sounds[i];
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": "; quote(row.id);
        out << ", \"displayName\": "; quote(row.label); out << ", \"assetId\": "; quote(row.assetId);
        out << ", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs
            << ", \"sourceStartMs\": " << row.sourceStartMs << ", \"volume\": " << row.volume
            << ", \"muted\": " << (row.muted ? "true" : "false") << '}';
    }
    out << "\n  ],\n  \"colliderRows\": [";
    for (std::size_t i = 0u; i < m_Colliders.size(); ++i)
    {
        const auto& row = m_Colliders[i]; const auto& resource = row.resource;
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": "; quote(row.id);
        out << ", \"displayName\": "; quote(row.label);
        out << ", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs << ", \"offset\": "; Track_WriteVector(out, row.offset);
        out << ", \"rotationDegrees\": "; Track_WriteVector(out, row.rotation); out << ", \"scale\": "; Track_WriteVector(out, row.scale);
        out << ", \"anchorSlotId\": "; quote(row.anchorSlotId);
        out << ", \"muted\": " << (row.muted ? "true" : "false") << ", \"debugRender\": " << (row.debugRender ? "true" : "false");
        out << ", \"resource\": {\"resourceId\": "; quote(resource.strResourceId);
        out << ", \"displayName\": "; quote(resource.strDisplayName); out << ", \"kind\": \"COLLIDER\", \"assetId\": "; quote(resource.strAssetId);
        out << ", \"resourceKind\": "; quote(resource.strResourceKind); out << ", \"defaultAnchorKind\": "; quote(resource.strDefaultAnchorKind);
        out << ", \"durationMs\": " << resource.iDurationMs << ", \"shape\": "; quote(resource.strShape);
        out << ", \"colliderKind\": "; quote(resource.strColliderKind);
        out << ", \"halfExtents\": [" << resource.HalfExtents[0] << ", " << resource.HalfExtents[1] << ", " << resource.HalfExtents[2]
            << "], \"radiusM\": " << resource.fRadiusM << ", \"halfAngleDegrees\": " << resource.fHalfAngleDegrees << "}}";
    }
    out << "\n  ]";
    out.precision(precision);
}

bool CEffectAuthoringSequencer::Commit_TransientPreview()
{
    if (!m_Transient) return true;
    if (!m_Transient->previewElementId.empty())
    { m_Status = "Element preview cannot become a saved sequence row. Stop it or preview the whole document before appending."; return false; }
    if (m_Effects.size() >= TRACK_MAX_ROWS)
    { m_Status = "The sequence already has 256 Effect rows."; return false; }
    auto cameras = m_CameraRows;
    for (auto row : m_TransientCameraRows)
    {
        row.id = CEffectEditingSession::New_Id("camera.row."); row.cue.strCueId = row.id;
        cameras.push_back(std::move(row));
    }
    if (!Validate_CameraRows(cameras)) return false;
    auto animations = m_AnimationRows;
    if (!m_TransientAnimationRows.empty())
    {
        // An implicit Product sequence is read-only; never silently replace one
        // already driving saved Effect rows with this clip-local recovery window.
        if (!m_CustomAnimation && !m_Effects.empty())
        { m_Status = "Recovery animation conflicts with the saved Product arrangement. Load its recovery sequence separately."; return false; }
        for (auto animation : m_TransientAnimationRows)
        {
            animation.id = CEffectEditingSession::New_Id("animation.occurrence.");
            animations.push_back(std::move(animation));
        }
        if (!Validate_AnimationRows(animations)) return false;
    }
    auto row = std::move(*m_Transient); row.id = CEffectEditingSession::New_Id("effect.occurrence.");
    Select_TimelineRow(row.screenPost ? TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT, row.id);
    m_Effects.push_back(std::move(row)); m_CameraRows = std::move(cameras);
    if (!m_TransientAnimationRows.empty())
    { m_AnimationRows = std::move(animations); m_CustomAnimation = true; }
    m_Transient.reset(); m_TransientCameraRows.clear(); m_TransientAnimationRows.clear();
    m_SelectedCamera.clear(); m_Dirty = true;
    m_Status = "Preview, recovery animation and camera rows appended. Save sequence preserves this arrangement.";
    return true;
}

bool CEffectAuthoringSequencer::Refresh_AnimationTiming()
{
    const auto clock = ClockMs(); const bool active = m_Active, paused = m_Paused;
    const auto reset = [](EFFECT_ROW& row)
    {
        row.history.reset(); row.anchorHistory.reset(); row.recordedAge = row.sampledAge = -1.f;
        if (row.v2) { CEffectV2Runtime::Stop_Group(row.v2); row.v2 = 0u; }
    };
    for (auto& row : m_Effects) reset(row);
    if (m_Transient) reset(*m_Transient);
    m_Dirty = true;
    if (!active) return true;
    // An Effect-only preview may not have acquired the model clock yet.
    if (!Begin_Model()) return false;
    if (!Seek(clock)) return false;
    Pause(paused); m_SkipNextPlaybackDelta = true;
    return true;
}
}
