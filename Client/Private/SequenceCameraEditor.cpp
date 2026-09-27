#include "imgui.h"
#include "SequenceCameraEditor.h"
#include <algorithm>
#include <cmath>

namespace Client
{
bool CSequenceCameraEditor::Set_KeyTime(EFFECT_CAMERA_ROW& row, const std::string& id, const std::uint32_t time)
{
    auto found = std::find_if(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), [&](const auto& key) { return key.strSceneId == id; });
    if (found == row.cue.Keyframes.end() || found == row.cue.Keyframes.begin() || time > row.cue.iDurationMs ||
        time <= (found - 1)->iTimeMs || (found + 1 != row.cue.Keyframes.end() && time >= (found + 1)->iTimeMs)) return false;
    found->iTimeMs = time; return true;
}
bool CSequenceCameraEditor::Add_Key(EFFECT_CAMERA_ROW& row, const std::uint32_t time, std::string& selected)
{
    if (row.cue.Keyframes.empty() || row.cue.Keyframes.size() >= 4096u || time > row.cue.iDurationMs ||
        row.cue.Keyframes.size() != row.upVectors.size()) return false;
    const auto next = std::lower_bound(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), time,
        [](const auto& key, auto value) { return key.iTimeMs < value; });
    if (next != row.cue.Keyframes.end() && next->iTimeMs == time) { selected = next->strSceneId; return false; }
    auto sample = row; sample.modelRelative = false; sample.horizontalFov = false; sample.startMs = 0; sample.muted = false;
    // Preserve the saved FOV axis; sample the scalar in that axis.
    ++sample.cue.iDurationMs;
    float4x4_t identity; XMStoreFloat4x4(&identity, XMMatrixIdentity());
    VALTAN_CINEMATIC_CAMERA_POSE pose; float3_t up; std::string status;
    if (!CEffectRecoveryCamera::Sample(sample, time, identity, 1.f, pose, up, status)) return false;
    VALTAN_CINEMATIC_CAMERA_KEYFRAME key; key.iTimeMs = time; key.vEye = pose.vEye;
    key.vLookAt = pose.vLookAt; key.fFovYDegrees = pose.fFovYDegrees;
    for (std::uint32_t ordinal = 1u;; ++ordinal)
    {
        key.strSceneId = row.id + ".key." + std::to_string(ordinal);
        if (std::none_of(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), [&](const auto& old) { return old.strSceneId == key.strSceneId; })) break;
    }
    const auto index = static_cast<std::size_t>(next - row.cue.Keyframes.begin());
    selected = key.strSceneId; row.cue.Keyframes.insert(next, std::move(key)); row.upVectors.insert(row.upVectors.begin() + index, up); return true;
}
bool CSequenceCameraEditor::Delete_Key(EFFECT_CAMERA_ROW& row, const std::string& id)
{
    const auto found = std::find_if(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), [&](const auto& key) { return key.strSceneId == id; });
    if (found == row.cue.Keyframes.end() || found == row.cue.Keyframes.begin() || row.upVectors.size() != row.cue.Keyframes.size()) return false;
    row.upVectors.erase(row.upVectors.begin() + (found - row.cue.Keyframes.begin())); row.cue.Keyframes.erase(found); return true;
}
bool CSequenceCameraEditor::Offset_Range(EFFECT_CAMERA_ROW& row, const std::uint32_t first,
    const std::uint32_t last, const float3_t& offset)
{
    if (first > last || !std::isfinite(offset.x) || !std::isfinite(offset.y) || !std::isfinite(offset.z)) return false;
    auto candidate = row; bool changed = false;
    for (auto& key : candidate.cue.Keyframes) if (key.iTimeMs >= first && key.iTimeMs <= last)
    {
        key.vEye.x += offset.x; key.vEye.y += offset.y; key.vEye.z += offset.z;
        key.vLookAt.x += offset.x; key.vLookAt.y += offset.y; key.vLookAt.z += offset.z; changed = true;
    }
    std::string status;
    if (!changed || !CEffectRecoveryCamera::Validate({candidate}, status)) return false;
    row = std::move(candidate); return true;
}
SEQUENCE_CAMERA_EDITOR_RESULT CSequenceCameraEditor::Render(EFFECT_CAMERA_ROW& destination,
    SEQUENCE_CAMERA_EDITOR_STATE& state, const std::uint32_t cursor)
{
    auto row = destination;
    SEQUENCE_CAMERA_EDITOR_RESULT result;
    if (row.cue.Keyframes.empty() || row.upVectors.size() != row.cue.Keyframes.size()) return result;
    ImGui::PushID(row.id.c_str());
    ImGui::TextDisabled("%s | %s FOV | time stored in ms", row.modelRelative ? "Model root" : "World", row.horizontalFov ? "Horizontal" : "Vertical");
    ImGui::SetNextItemWidth(110.f); ImGui::DragFloat("Display FPS", &state.framesPerSecond, 1.f, 1.f, 240.f, "%.3f");
    state.framesPerSecond = std::isfinite(state.framesPerSecond) ? (std::clamp)(state.framesPerSecond, 1.f, 240.f) : 30.f;
    int curve = static_cast<int>(row.cue.eInterpolation), easing = static_cast<int>(row.cue.eEasing);
    if (ImGui::Combo("Interpolation", &curve, "Linear\0Catmull-Rom\0")) { row.cue.eInterpolation = static_cast<VALTAN_CINEMATIC_CAMERA_INTERPOLATION>(curve); result.changed = true; }
    if (ImGui::Combo("Easing", &easing, "Linear\0Smoothstep\0Hold\0")) { row.cue.eEasing = static_cast<VALTAN_CINEMATIC_CAMERA_EASING>(easing); result.changed = true; }
    auto selected = std::find_if(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), [&](const auto& key) { return key.strSceneId == state.selectedKeyId; });
    if (selected == row.cue.Keyframes.end()) { state.selectedKeyId = row.cue.Keyframes.front().strSceneId; selected = row.cue.Keyframes.begin(); }
    int index = static_cast<int>(selected - row.cue.Keyframes.begin());
    if (ImGui::Button("Previous key") && index > 0) { --index; state.selectedKeyId = row.cue.Keyframes[index].strSceneId; result.seekLocalMs = row.cue.Keyframes[index].iTimeMs; }
    ImGui::SameLine(); if (ImGui::Button("Next key") && index + 1 < static_cast<int>(row.cue.Keyframes.size())) { ++index; state.selectedKeyId = row.cue.Keyframes[index].strSceneId; result.seekLocalMs = row.cue.Keyframes[index].iTimeMs; }
    if (ImGui::BeginListBox("Keys", {-1.f, 125.f}))
    {
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>(row.cue.Keyframes.size()));
        while (clipper.Step()) for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; ++item)
        {
            const auto& key = row.cue.Keyframes[static_cast<std::size_t>(item)];
            const std::string label = std::to_string(key.iTimeMs) + " ms | " + key.strSceneId;
            if (ImGui::Selectable(label.c_str(), key.strSceneId == state.selectedKeyId)) { state.selectedKeyId = key.strSceneId; result.seekLocalMs = key.iTimeMs; }
        }
        ImGui::EndListBox();
    }
    selected = std::find_if(row.cue.Keyframes.begin(), row.cue.Keyframes.end(), [&](const auto& key) { return key.strSceneId == state.selectedKeyId; });
    index = static_cast<int>(selected - row.cue.Keyframes.begin());
    int time = static_cast<int>(selected->iTimeMs);
    ImGui::BeginDisabled(index == 0);
    if (ImGui::InputInt("Time (ms)", &time)) result.changed |= Set_KeyTime(row, state.selectedKeyId, static_cast<std::uint32_t>((std::max)(0, time)));
    int frame = static_cast<int>(std::llround(selected->iTimeMs * state.framesPerSecond / 1000.0));
    if (ImGui::InputInt("Frame", &frame)) result.changed |= Set_KeyTime(row, state.selectedKeyId,
        static_cast<std::uint32_t>((std::clamp)(std::llround((std::max)(0, frame) * 1000.0 / state.framesPerSecond), 0ll, 600000ll)));
    ImGui::EndDisabled();
    result.changed |= ImGui::DragFloat3("Eye", &selected->vEye.x, .01f);
    result.changed |= ImGui::DragFloat3("Look at", &selected->vLookAt.x, .01f);
    result.changed |= ImGui::DragFloat3("Up", &row.upVectors[index].x, .01f);
    result.changed |= ImGui::DragFloat(row.horizontalFov ? "FOV X" : "FOV Y", &selected->fFovYDegrees, .1f, 1.01f, 178.99f);
    result.changed |= ImGui::Checkbox("Cut before key", &selected->cutBefore);
    if (ImGui::Button("Seek to key")) result.seekLocalMs = selected->iTimeMs;
    ImGui::SameLine(); if (ImGui::Button("Add key at cursor")) result.changed |= Add_Key(row, (std::min)(cursor, row.cue.iDurationMs), state.selectedKeyId);
    ImGui::SameLine(); ImGui::BeginDisabled(index == 0);
    if (ImGui::Button("Delete key")) result.changed |= Delete_Key(row, state.selectedKeyId);
    ImGui::EndDisabled();
    if (ImGui::CollapsingHeader("Offset a range"))
    {
        ImGui::InputScalar("From (ms)", ImGuiDataType_U32, &state.rangeStartMs);
        ImGui::InputScalar("Through (ms)", ImGuiDataType_U32, &state.rangeEndMs);
        ImGui::DragFloat3("Position offset", &state.rangeOffset.x, .01f);
        if (ImGui::Button("Offset Eye and Look at")) result.changed |= Offset_Range(row, state.rangeStartMs, state.rangeEndMs, state.rangeOffset);
    }
    if (result.changed)
    {
        if (CEffectRecoveryCamera::Validate({row}, state.validationStatus))
        { row.source = "PROJECT_TUNED"; destination = std::move(row); state.validationStatus.clear(); }
        else result.changed = false;
    }
    if (!state.validationStatus.empty()) ImGui::TextWrapped("%s Previous key values were preserved.", state.validationStatus.c_str());
    ImGui::PopID(); return result;
}
}
