# Sequence Camera Tool 키 목록 높이 확대 계획

## G00. 현재 표시 구조와 이번 범위

사용자가 촬영과 카메라 키 편집을 다시 진행하면서 작은 키 목록을 최소 4배 확대하도록 요청했다.
현재 `CSequenceCameraEditor::Render`의 `Keys` ListBox는 폭을 가용 영역에 맞추지만 높이는
125px로 고정되어 있다. 이 높이를 500px로 바꾸어 세로 표시 영역을 정확히 4배 확보한다.
글꼴 크기, 개별 키 행 높이, 시간축과 실제 저장된 키 값은 유지한다.

관련 기존 구현은 [자유 시점 포즈 캡처 계획](../10-04/2026-10-04_CAMERA_POSE_CAPTURE_IMPLEMENTATION_PLAN.md)과
[결과](../10-04/2026-10-04_CAMERA_POSE_CAPTURE_RESULT.md)를 따른다. 기존 키 선택·이동·캡처·Apply·Save·Publish
계약은 변경하지 않는다. 현재 브랜치는 `codex/release-movie-profiler`이며 다른 기능의 미커밋 변경이 있다.
이번 작업은 아래 CPP 한 줄과 대응 PLAN/RESULT만 소유한다.

## G01. 공통 편집기와 실제 호출자

| 구분 | 절대 경로 | 책임 |
|---|---|---|
| 수정 | `C:/Users/tnest/Desktop/LostArk/Client/Private/SequenceCameraEditor.cpp` | `Render`의 `BeginListBox("Keys", ...)` 높이를 125에서 500으로 교체한다. |

새 선언·include·자료형·멤버 변수는 없다. `CSequenceCameraEditor::Render`는 기존 키 행의
복사본을 편집하고 검증에 성공한 변경만 호출자에 반환하는 공통 UI다. 목록의 viewport 높이만 바꾸며
선택 stable ID, `ImGuiListClipper`, `Selectable`, 입력 검증과 commit 흐름을 보존한다.

`SequencerTool.cpp`의 `Render_CameraWindow`는 공통 편집기를 `CameraKeys` BeginChild 안에서
호출한다. 이 child는 기본 세로 스크롤을 사용한다. `EffectAuthoringSequencer_Camera.cpp`의
`Render_RecoveryCameraTool`은 일반 ImGui 창 안에서 같은 함수를 호출하며 기본 세로 스크롤을
사용한다. 작은 창에서 아래 Time/Eye/Look at/Up/FOV/Apply가 창 높이를 넘으면 기존 스크롤로 접근한다.
기존 창 크기·배치·사용자 imgui.ini는 변경하지 않는다.

Sequence 저작 도구는 기존 Debug 전용 노출을 유지한다. Release 빌드는 수행하되 이 도구를
Release에 새로 열지 않는다. Server·Shared·카메라 저작 JSON·publisher 변경은 없다.

## G02. SequenceCameraEditor.cpp 전체 적용 코드

변경 종류: 기존 함수 한 줄 교체. 기준점은 `Render`의 키 개수 안내 바로 아래 `BeginListBox`다.
기존 UTF-8 BOM 없음과 CRLF를 유지한다.

```cpp
#include "imgui.h"
#include "SequenceCameraEditor.h"
#include <algorithm>
#include <cmath>

namespace Client
{
bool CSequenceCameraEditor::Set_KeyPose(EFFECT_CAMERA_ROW& row, const std::string& id,
    const VALTAN_CINEMATIC_CAMERA_POSE& pose, std::string& status)
{
    const auto found = std::find_if(row.cue.Keyframes.begin(), row.cue.Keyframes.end(),
        [&](const auto& key) { return key.strSceneId == id; });
    if (!pose.hasUp || found == row.cue.Keyframes.end() || row.upVectors.size() != row.cue.Keyframes.size())
    { status = "Select a valid camera key and capture a complete camera pose."; return false; }
    const auto index = static_cast<std::size_t>(found - row.cue.Keyframes.begin());
    auto candidate = row;
    candidate.cue.Keyframes[index].vEye = pose.vEye;
    candidate.cue.Keyframes[index].vLookAt = pose.vLookAt;
    candidate.upVectors[index] = pose.vUp;
    if (!CEffectRecoveryCamera::Validate({candidate}, status)) return false;
    // Commit only pose fields, keeping selection iterators and all authored timing/lens settings.
    found->vEye = pose.vEye; found->vLookAt = pose.vLookAt; row.upVectors[index] = pose.vUp;
    status.clear(); return true;
}
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
    SEQUENCE_CAMERA_EDITOR_STATE& state, const std::uint32_t cursor,
    const SEQUENCE_CAMERA_CAPTURE& captureFreeCamera)
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
    ImGui::TextDisabled("%zu keys | Delete key keeps the first key and interpolation settings.", row.cue.Keyframes.size());
    if (ImGui::BeginListBox("Keys", {-1.f, 500.f}))
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
    if (captureFreeCamera)
    {
        if (ImGui::Button("Use free cam pos"))
        {
            VALTAN_CINEMATIC_CAMERA_POSE pose;
            if (captureFreeCamera(pose, state.validationStatus))
                result.changed |= Set_KeyPose(row, state.selectedKeyId, pose, state.validationStatus);
        }
        ImGui::TextDisabled("Copies Eye, Look at and Up to this key. Time and FOV stay unchanged.");
    }
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

```

## G03. 등록과 검증

기존 `Client/Default/Client.vcxproj`와 `.vcxproj.filters`에 `SequenceCameraEditor.cpp`가
등록되어 있으므로 새 프로젝트 항목은 없다. XML·JSON 수정이 없다.

1. 위 한 줄을 적용하고 diff가 해당 높이만 변경했는지 확인한다.
2. 기존 UTF-8 BOM 없음·CRLF 유지와 대상 `git diff --check`를 확인한다.
3. 다음 정본 Product 빌드를 순서대로 수행한다. 실행 결과는 RESULT에 별도로 기록한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

4. 사용자가 Debug의 기존 Action Workbench → World → Character Select → 카메라 박스 →
   Open Sequence Camera Tool에서 늘어난 목록과 아래 키 편집 항목의 스크롤 접근을 확인한다.
   Effect의 기존 Sequence Camera Tool도 같은 높이를 사용한다.
5. Client/UI 실행·조작·화면 캡처는 수행하지 않는다. 빌드 성공과 사용자의 실제 화면 판정을 구분한다.
