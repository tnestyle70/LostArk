#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "CompositionTimeline.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "RuntimeAssetRoot.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>

namespace Client
{
namespace
{
constexpr std::uint32_t LIMIT_MS = 600000u;
template<class Rows> auto FindRow(Rows& rows, const std::string& id)
{
    return std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.id == id; });
}
bool FinitePosition(const float3_t& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
        std::abs(value.x) <= 100000.f && std::abs(value.y) <= 100000.f && std::abs(value.z) <= 100000.f;
}
}

void CEffectAuthoringSequencer::Select_TimelineRow(const TRACK_KIND kind, const std::string& id)
{
    m_SelectedTrack = kind; m_SelectedRowId = id; m_BoxDetailOpen = true;
    m_SelectedEffect = kind == TRACK_KIND::EFFECT || kind == TRACK_KIND::SCREEN_POST ? id : "";
    m_SelectedCamera = kind == TRACK_KIND::CAMERA ? id : "";
}

bool CEffectAuthoringSequencer::Apply_AnimationRow(const CLIP& row)
{
    if (m_UseKouku) { m_Status = "Edit the saved boss composition in Action Workbench."; return false; }
    auto candidate = m_CustomAnimation ? m_AnimationRows :
        (Selected_Sequence() ? Selected_Sequence()->clips : std::vector<CLIP>{});
    auto found = FindRow(candidate, row.id);
    if (found == candidate.end()) { m_Status = "The selected Animation occurrence is unavailable."; return false; }
    *found = row;
    if (!Validate_AnimationRows(candidate)) return false;
    auto previous = m_AnimationRows; const bool wasCustom = m_CustomAnimation, wasDirty = m_Dirty;
    m_AnimationRows = std::move(candidate); m_CustomAnimation = true;
    if (!Refresh_AnimationTiming())
    {
        const auto error = m_Status; m_AnimationRows = std::move(previous); m_CustomAnimation = wasCustom;
        Refresh_AnimationTiming(); m_Dirty = wasDirty; m_Status = error; return false;
    }
    m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Animation occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Apply_EffectRow(const EFFECT_ROW& value)
{
    auto found = FindRow(m_Effects, value.id);
    if (found == m_Effects.end()) { m_Status = "Append this preview before editing its occurrence."; return false; }
    if (!FinitePosition(value.offset)) { m_Status = "Effect offset must be finite and within 100000 m."; return false; }
    auto candidate = value;
    candidate.v1.reset(); candidate.v2 = 0u; candidate.history.reset(); candidate.anchorHistory.reset();
    candidate.sampledAge = candidate.recordedAge = -1.f;
    float4x4_t root = m_WorldRoot;
    if (m_Active && !Resolve_Root(root)) return false;
    if (!Stage_Row(candidate, root) || (m_Active && !m_Transient && !Sample_Row(candidate, root)))
    { Release_Row(candidate); return false; }
    Release_Row(*found); *found = std::move(candidate); m_Dirty = true;
    m_BoxDetailDraft.reset(); Preserve_ClockDuringAuthoring(); m_Status = "Effect occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Apply_SoundRow(const SOUND_ROW& value)
{
    auto candidate = m_Sounds; auto found = FindRow(candidate, value.id);
    if (found == candidate.end()) { m_Status = "The selected Sound occurrence is unavailable."; return false; }
    const auto index = static_cast<std::size_t>(found - candidate.begin());
    *found = value;
    if (!Validate_SoundRows(candidate)) return false;
    auto staged = value; staged.handle = 0u; staged.sampledAge = -1;
    auto& sound = CGameInstance::Get();
    if (m_Active && !m_Transient && !staged.muted && ClockMs() >= staged.startMs &&
        std::uint64_t(ClockMs()) < std::uint64_t(staged.startMs) + staged.durationMs)
    {
        const auto path = CRuntimeAssetRoot::Resolve(std::filesystem::path(
            std::u8string(staged.assetId.begin(), staged.assetId.end())));
        std::uint32_t sourceDuration = 0u;
        if (path.empty() || !sound.Get_SoundDurationMs(path.wstring(), sourceDuration))
        { m_Status = "Sound source preparation failed; the previous occurrence is preserved."; return false; }
        const auto age = staged.sourceStartMs + ClockMs() - staged.startMs;
        if (age < sourceDuration)
        {
            staged.handle = sound.Play_SoundCue(path.wstring(), staged.volume, age, true);
            if (!staged.handle)
            { m_Status = "Sound channel preparation failed; the previous occurrence is preserved."; return false; }
        }
        staged.sampledAge = age;
    }
    if (m_Sounds[index].handle) sound.Stop_SoundCue(m_Sounds[index].handle);
    m_Sounds[index] = std::move(staged);
    if (m_Sounds[index].handle) sound.Pause_SoundCue(m_Sounds[index].handle, m_Paused);
    m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Sound occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Apply_ColliderRow(const COLLIDER_ROW& value)
{
    auto candidate = m_Colliders; auto found = FindRow(candidate, value.id);
    if (found == candidate.end()) { m_Status = "The selected Collider occurrence is unavailable."; return false; }
    *found = value;
    if (!Validate_ColliderRows(candidate) || !Validate_Anchor(value.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    m_Colliders = std::move(candidate); m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Collider occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Remove_SelectedRow()
{
    if (m_SelectedRowId.empty()) return false;
    switch (m_SelectedTrack)
    {
    case TRACK_KIND::ANIMATION:
    {
        if (m_UseKouku) return false;
        auto candidate = m_CustomAnimation ? m_AnimationRows :
            (Selected_Sequence() ? Selected_Sequence()->clips : std::vector<CLIP>{});
        auto row = FindRow(candidate, m_SelectedRowId); if (row == candidate.end()) return false;
        auto previous = m_AnimationRows; const bool wasCustom = m_CustomAnimation, wasDirty = m_Dirty;
        candidate.erase(row); m_AnimationRows = std::move(candidate); m_CustomAnimation = true;
        if (!Refresh_AnimationTiming())
        {
            const auto error = m_Status; m_AnimationRows = std::move(previous); m_CustomAnimation = wasCustom;
            Refresh_AnimationTiming(); m_Dirty = wasDirty; m_Status = error; return false;
        }
        break;
    }
    case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
    {
        auto row = FindRow(m_Effects, m_SelectedRowId); if (row == m_Effects.end()) return false;
        Release_Row(*row); m_Effects.erase(row); break;
    }
    case TRACK_KIND::SOUND:
    {
        auto row = FindRow(m_Sounds, m_SelectedRowId); if (row == m_Sounds.end()) return false;
        if (row->handle) CGameInstance::Get().Stop_SoundCue(row->handle);
        m_Sounds.erase(row); break;
    }
    case TRACK_KIND::COLLIDER:
    {
        auto row = FindRow(m_Colliders, m_SelectedRowId); if (row == m_Colliders.end()) return false;
        m_Colliders.erase(row); break;
    }
    case TRACK_KIND::CAMERA:
    {
        auto row = FindRow(m_CameraRows, m_SelectedRowId); if (row == m_CameraRows.end()) return false;
        m_CameraRows.erase(row);
        if (m_Active) { float4x4_t root; if (Resolve_Root(root)) Sample_Camera(ClockMs(), root); }
        break;
    }
    }
    m_SelectedRowId.clear(); m_SelectedEffect.clear(); m_SelectedCamera.clear();
    m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Occurrence removed."; return true;
}

bool CEffectAuthoringSequencer::Duplicate_SelectedRow()
{
    const auto nextStart = [](std::uint32_t start, std::uint32_t duration)
        { return duration <= LIMIT_MS && start <= LIMIT_MS - duration && start + duration <= LIMIT_MS - duration; };
    switch (m_SelectedTrack)
    {
    case TRACK_KIND::ANIMATION:
    {
        if (m_UseKouku) break;
        auto candidate = m_CustomAnimation ? m_AnimationRows :
            (Selected_Sequence() ? Selected_Sequence()->clips : std::vector<CLIP>{});
        auto found = FindRow(candidate, m_SelectedRowId);
        if (found == candidate.end() || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("animation.occurrence."); row.startMs += row.durationMs;
        candidate.push_back(row); if (!Validate_AnimationRows(candidate)) return false;
        auto previous = m_AnimationRows; const bool wasCustom = m_CustomAnimation, wasDirty = m_Dirty;
        m_AnimationRows = std::move(candidate); m_CustomAnimation = true;
        if (!Refresh_AnimationTiming())
        {
            const auto error = m_Status; m_AnimationRows = std::move(previous); m_CustomAnimation = wasCustom;
            Refresh_AnimationTiming(); m_Dirty = wasDirty; m_Status = error; return false;
        }
        Select_TimelineRow(TRACK_KIND::ANIMATION, row.id); m_Dirty = true; return true;
    }
    case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
    {
        auto found = FindRow(m_Effects, m_SelectedRowId);
        if (found == m_Effects.end() || m_Effects.size() >= 256u || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("effect.occurrence."); row.startMs += row.durationMs;
        row.v1.reset(); row.v2 = 0u; row.history.reset(); row.anchorHistory.reset(); row.sampledAge = row.recordedAge = -1.f;
        float4x4_t root = m_WorldRoot;
        if (m_Active && !Resolve_Root(root)) return false;
        if (!Stage_Row(row, root) || (m_Active && !m_Transient && !Sample_Row(row, root))) { Release_Row(row); return false; }
        Select_TimelineRow(m_SelectedTrack, row.id); m_Effects.push_back(std::move(row)); m_Dirty = true; return true;
    }
    case TRACK_KIND::SOUND:
    {
        auto found = FindRow(m_Sounds, m_SelectedRowId);
        if (found == m_Sounds.end() || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("sound.occurrence."); row.startMs += row.durationMs;
        row.handle = 0u; row.sampledAge = -1;
        auto candidate = m_Sounds; candidate.push_back(row); if (!Validate_SoundRows(candidate)) return false;
        m_Sounds = std::move(candidate); Select_TimelineRow(TRACK_KIND::SOUND, row.id); m_Dirty = true; return true;
    }
    case TRACK_KIND::COLLIDER:
    {
        auto found = FindRow(m_Colliders, m_SelectedRowId);
        if (found == m_Colliders.end() || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("collider.occurrence."); row.startMs += row.durationMs;
        auto candidate = m_Colliders; candidate.push_back(row); if (!Validate_ColliderRows(candidate)) return false;
        m_Colliders = std::move(candidate); Select_TimelineRow(TRACK_KIND::COLLIDER, row.id); m_Dirty = true; return true;
    }
    case TRACK_KIND::CAMERA:
    {
        auto found = FindRow(m_CameraRows, m_SelectedRowId);
        if (found == m_CameraRows.end() || !nextStart(found->startMs, found->cue.iDurationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("camera.row."); row.cue.strCueId = row.id;
        row.startMs += row.cue.iDurationMs;
        auto candidate = m_CameraRows; candidate.push_back(row); if (!Validate_CameraRows(candidate)) return false;
        m_CameraRows = std::move(candidate); Select_TimelineRow(TRACK_KIND::CAMERA, row.id); m_Dirty = true; return true;
    }
    }
    m_Status = "The selected occurrence cannot be duplicated at its end time."; return false;
}

void CEffectAuthoringSequencer::Render_Sequencer(const char* title)
{
    Engine::CProfilerScope Profile(CGameInstance::Get().Get_Profiler(), "EffectSequencer.Render");
    ImGui::SetNextWindowSize({1180.f, 420.f}, ImGuiCond_FirstUseEver);
    const bool expanded = ImGui::Begin(title, nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Composition Resources", nullptr, &m_ResourcesOpen);
            ImGui::MenuItem("Box Detail", nullptr, &m_BoxDetailOpen);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    if (expanded)
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
        const auto restartMs = Is_ElementPreview() ? m_Transient->previewStartMs : 0u;
        if (ImGui::Button("Play")) { if (m_ClockMs >= DurationMs()) m_ClockMs = restartMs; Play(); }
        ImGui::SameLine(); if (ImGui::Button(m_Paused ? "Resume" : "Pause")) Pause(!m_Paused);
        ImGui::SameLine(); if (ImGui::Button("Restart")) { m_ClockMs = restartMs; Play(); }
        ImGui::SameLine(); if (ImGui::Button("Stop")) { Stop(); m_ClockMs = 0; }
        ImGui::SameLine();
        ImGui::Checkbox("Loop", Uses_TransientLoop() ? &m_Transient->previewLoop : &m_Loop);
        ImGui::SameLine(); if (ImGui::Button("Refresh Effects")) Refresh_Effects();
        ImGui::SameLine(); if (ImGui::Button("Resources")) m_ResourcesOpen = true;
        ImGui::SameLine(); if (ImGui::Button("Details")) m_BoxDetailOpen = true;
        ImGui::SetNextItemWidth(255.f); ImGui::InputText("Sequence ID", m_SequenceId, sizeof(m_SequenceId));
        ImGui::SameLine(); if (ImGui::Button("Save")) Save_Sequence();
        ImGui::SameLine(); if (ImGui::Button("Load")) Load_Sequence();
        ImGui::SameLine(); if (ImGui::Button("Revert")) Load_Sequence(true);
        ImGui::SameLine(); if (ImGui::Button("New"))
        {
            if (m_Dirty) m_Status = "Save or Revert this sequence before creating another.";
            else
            {
                Stop(); m_Effects.clear(); m_CameraRows.clear(); m_Sounds.clear(); m_Colliders.clear();
                m_AnimationRows.clear(); m_CustomAnimation = false; m_SelectedSequence.clear(); m_UseKouku = false;
                m_SelectedCamera.clear(); m_SelectedEffect.clear(); m_SelectedRowId.clear(); m_BoxDetailDraft.reset();
                m_ClockMs = 0; m_NextEffectOrdinal = 1u;
                std::snprintf(m_SequenceId, sizeof(m_SequenceId), "%s", CEffectEditingSession::New_Id("effect.sequence.").c_str());
                m_SequenceBaseline.clear(); m_PersistedSequenceId.clear(); m_SequenceExisted = false; m_Dirty = true;
            }
        }
        if (m_Dirty) { ImGui::SameLine(); ImGui::TextDisabled("Unsaved"); }
        int clock = static_cast<int>(ClockMs());
        ImGui::SetNextItemWidth(300.f);
        if (ImGui::SliderInt("Time", &clock, 0, static_cast<int>(DurationMs()), "%d ms")) Seek(clock);
        ImGui::SameLine(); ImGui::SetNextItemWidth(130.f); ImGui::SliderFloat("Zoom", &m_Zoom, 10.f, 300.f, "%.0f px/s");
        ImGui::SameLine(); if (ImGui::Button("Duplicate")) Duplicate_SelectedRow();
        ImGui::SameLine(); if (ImGui::Button("Remove")) Remove_SelectedRow();
        const auto firstLine = m_Status.substr(0, m_Status.find('\n'));
        ImGui::TextUnformatted(firstLine.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_Status.c_str());

        struct BOX final
        {
            TRACK_KIND kind; std::string id, label; std::uint32_t start, duration;
            bool muted, editable;
        };
        std::array<std::vector<BOX>, 6> lanes;
        auto add = [&](TRACK_KIND kind, const std::string& id, const std::string& label,
            std::uint32_t start, std::uint32_t duration, bool muted, bool editable = true)
        { lanes[static_cast<std::size_t>(kind)].push_back({kind, id, label, start, duration, muted, editable}); };
        if (m_UseKouku)
            for (const auto& clip : m_Kouku.Rows())
                add(TRACK_KIND::ANIMATION, clip.memberId + "." + clip.occurrenceId, clip.runtimeClip,
                    clip.startMs, clip.durationMs, false, false);
        else
        {
            const auto* sequence = Selected_Sequence();
            const bool recoveryAnimation = !m_TransientAnimationRows.empty();
            const auto* clips = recoveryAnimation ? &m_TransientAnimationRows :
                (m_CustomAnimation ? &m_AnimationRows : (sequence ? &sequence->clips : nullptr));
            if (clips) for (const auto& clip : *clips)
                add(TRACK_KIND::ANIMATION, clip.id, clip.label.empty() ? clip.clipName : clip.label,
                    clip.startMs, clip.durationMs, clip.muted, !recoveryAnimation);
        }
        const bool elementPreview = m_Transient && !m_Transient->previewElementIds.empty();
        auto addEffect = [&](const EFFECT_ROW& row, bool transient)
        {
            const auto resource = std::find_if(m_CompositionResources.begin(), m_CompositionResources.end(),
                [&](const auto& entry) { return entry.key == row.key; });
            const auto label = row.previewElementIds.empty() ?
                (resource == m_CompositionResources.end() ? row.key.strStableId : resource->label) :
                (row.previewElementLabel.empty() ? row.previewElementIds.front() : row.previewElementLabel);
            add(row.screenPost ? TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT, row.id,
                transient ? (row.previewElementIds.empty() ? "Preview / " : "Element / ") + label : label,
                row.startMs, row.durationMs, row.muted, !transient);
        };
        if (!elementPreview) for (const auto& row : m_Effects) addEffect(row, false);
        if (m_Transient) addEffect(*m_Transient, true);
        if (!elementPreview)
        {
            for (const auto& row : m_Colliders) add(TRACK_KIND::COLLIDER, row.id, row.label, row.startMs, row.durationMs, row.muted);
            for (const auto& row : m_Sounds) add(TRACK_KIND::SOUND, row.id, row.label, row.startMs, row.durationMs, row.muted);
        }
        const auto& cameras = m_Transient ? m_TransientCameraRows : m_CameraRows;
        for (const auto& row : cameras) add(TRACK_KIND::CAMERA, row.id, row.label, row.startMs, row.cue.iDurationMs, row.muted, !m_Transient);
        constexpr float labels = 160.f, rowHeight = 29.f;
        const auto canvasMs = (std::min)(LIMIT_MS, (std::max)(10000u, DurationMs() + 1000u));
        const float width = (std::max)(ImGui::GetContentRegionAvail().x - 5.f, labels + canvasMs * m_Zoom * .001f + 30.f);
        ImGui::SetNextWindowContentSize({width, 0.f});
        if (ImGui::BeginChild("Timeline", {0.f, 0.f}, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
        {
            const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
            ImGui::TextDisabled("0  Timeline");
            CompositionTimeline::DrawRuler(draw, {origin.x + labels, origin.y}, {origin.x + width, origin.y + rowHeight}, canvasMs, m_Zoom);
            ImGui::SetCursorScreenPos({origin.x + labels, origin.y});
            ImGui::InvisibleButton("Timeline cursor", {width - labels, rowHeight});
            if (ImGui::IsItemActive())
            {
                const auto ms = static_cast<std::uint32_t>((std::clamp)(
                    (ImGui::GetIO().MousePos.x - origin.x - labels) * 1000.f / m_Zoom, 0.f, float(DurationMs())));
                if (!m_Active || ms != ClockMs()) Seek(ms);
                m_Interaction = true;
            }
            const char* names[] = {"Animation", "Effect", "Collider", "Sound", "Camera", "Screen Post"};
            const ImU32 colors[] = {IM_COL32(68,125,177,230), IM_COL32(173,107,48,230), IM_COL32(65,171,165,230),
                IM_COL32(130,176,83,230), IM_COL32(118,85,184,230), IM_COL32(190,90,144,230)};
            float y = origin.y + rowHeight;
            for (std::size_t lane = 0; lane < lanes.size(); ++lane)
            {
                auto& boxes = lanes[lane];
                std::vector<std::uint32_t> rowEnds;
                std::vector<std::size_t> positions;
                {
                    Engine::CProfilerScope LayoutProfile(CGameInstance::Get().Get_Profiler(), "EffectSequencer.LaneLayout");
                    std::stable_sort(boxes.begin(), boxes.end(), [](const auto& a, const auto& b) { return a.start < b.start; });
                    for (const auto& box : boxes)
                    {
                        std::size_t row = 0;
                        while (row < rowEnds.size() && rowEnds[row] > box.start) ++row;
                        if (row == rowEnds.size()) rowEnds.push_back(0u);
                        rowEnds[row] = box.start + box.duration; positions.push_back(row);
                    }
                }
                const auto rowCount = (std::max)(std::size_t{1}, rowEnds.size());
                const float height = rowHeight * static_cast<float>(rowCount);
                draw->AddRectFilled({origin.x, y}, {origin.x + width, y + height},
                    lane % 2 ? IM_COL32(31,34,41,255) : IM_COL32(38,41,49,255));
                draw->AddLine({origin.x, y}, {origin.x + width, y}, IM_COL32(64,68,76,255));
                draw->AddText({origin.x + 8.f, y + 6.f}, colors[lane], names[lane]);
                if (boxes.empty()) draw->AddText({origin.x + labels + 8.f, y + 6.f}, IM_COL32(115,118,127,255), "Add from Composition Resources");
                for (std::size_t i = 0; i < boxes.size(); ++i)
                {
                    const auto& box = boxes[i];
                    const bool dragging = m_DragRowId == box.id && m_DragTrack == box.kind;
                    const auto startMs = dragging ? m_DragPreviewStartMs : box.start;
                    const auto durationMs = dragging ? m_DragPreviewDurationMs : box.duration;
                    const float top = y + positions[i] * rowHeight + 2.f;
                    const float left = origin.x + labels + startMs * m_Zoom * .001f;
                    const float right = (std::max)(left + 6.f, left + durationMs * m_Zoom * .001f);
                    // Keep the active item alive while dragging beyond the viewport.
                    // The final Dummy below still owns the full scrollable layout.
                    if (!dragging && !ImGui::IsRectVisible({left - 2.f, top - 2.f}, {right + 2.f, top + rowHeight - 3.f}))
                        continue;
                    CompositionTimeline::DrawBox(draw, {left, top}, {right, top + rowHeight - 5.f},
                        box.muted ? IM_COL32(73,75,81,200) : colors[lane],
                        m_SelectedTrack == box.kind && m_SelectedRowId == box.id, box.label.c_str(), box.editable, box.editable);
                    ImGui::SetCursorScreenPos({left, top}); ImGui::PushID(static_cast<int>(lane)); ImGui::PushID(box.id.c_str());
                    ImGui::InvisibleButton("Occurrence", {right - left, rowHeight - 5.f});
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%u ms / %u ms%s", box.label.c_str(), startMs, durationMs, box.muted ? " / Muted" : "");
                    if (ImGui::IsItemActivated())
                    {
                        Select_TimelineRow(box.kind, box.id); m_Interaction = true;
                        if (box.editable)
                        {
                            m_DragRowId = box.id; m_DragTrack = box.kind; m_DragMouseX = ImGui::GetIO().MousePos.x;
                            m_DragStartMs = m_DragPreviewStartMs = box.start; m_DragDurationMs = m_DragPreviewDurationMs = box.duration;
                            m_DragWasPaused = m_Paused; Pause(true);
                            m_DragKind = static_cast<int>(CompositionTimeline::HitBoxGesture(m_DragMouseX, left, right, 6.f, true, true));
                        }
                    }
                    if (box.editable && dragging && ImGui::IsItemActive())
                    {
                        const auto delta = static_cast<std::int64_t>(std::llround((ImGui::GetIO().MousePos.x - m_DragMouseX) * 1000. / m_Zoom));
                        std::int64_t start = m_DragStartMs, duration = m_DragDurationMs;
                        if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::MOVE))
                            start = (std::clamp)(start + delta, std::int64_t{0}, std::int64_t{LIMIT_MS - m_DragDurationMs});
                        else if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START))
                        { start = (std::clamp)(start + delta, std::int64_t{0}, std::int64_t{m_DragStartMs} + m_DragDurationMs - 1); duration = m_DragStartMs + m_DragDurationMs - start; }
                        else duration = (std::clamp)(duration + delta, std::int64_t{1}, std::int64_t{LIMIT_MS - m_DragStartMs});
                        m_DragPreviewStartMs = static_cast<std::uint32_t>(start); m_DragPreviewDurationMs = static_cast<std::uint32_t>(duration);
                    }
                    ImGui::PopID(); ImGui::PopID();
                }
                y += height;
            }
            const float cursorX = origin.x + labels + float(m_ClockMs) * m_Zoom * .001f;
            draw->AddLine({cursorX, origin.y}, {cursorX, y}, IM_COL32(255,222,90,255), 2.f);
            draw->AddTriangleFilled({cursorX - 6.f, origin.y}, {cursorX + 6.f, origin.y}, {cursorX, origin.y + 8.f}, IM_COL32(255,222,90,255));
            ImGui::SetCursorScreenPos({origin.x, y}); ImGui::Dummy({width, 2.f});
        }
        ImGui::EndChild();
        if (!m_DragRowId.empty() && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (m_DragPreviewStartMs != m_DragStartMs || m_DragPreviewDurationMs != m_DragDurationMs)
            {
                switch (m_DragTrack)
                {
                case TRACK_KIND::ANIMATION:
                {
                    const auto* sequence = Selected_Sequence();
                    const auto* clips = m_CustomAnimation ? &m_AnimationRows : (sequence ? &sequence->clips : nullptr);
                    if (clips)
                    {
                        auto found = FindRow(*clips, m_DragRowId);
                        if (found != clips->end())
                        {
                            auto row = *found; bool valid = true;
                            if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START))
                            {
                                const auto sourceDelta = static_cast<std::int64_t>(std::llround(
                                    (std::int64_t(m_DragPreviewStartMs) - row.startMs) * double(row.playRate)));
                                const auto sourceStart = std::int64_t(row.sourceStartMs) + sourceDelta;
                                const auto sourcePlay = row.sourcePlayMs ? std::int64_t(row.sourcePlayMs) - sourceDelta : 0;
                                valid = sourceStart >= 0 && sourceStart <= LIMIT_MS &&
                                    (!row.sourcePlayMs || (sourcePlay > 0 && sourcePlay <= LIMIT_MS));
                                if (valid) { row.sourceStartMs = static_cast<std::uint32_t>(sourceStart); row.sourcePlayMs = static_cast<std::uint32_t>(sourcePlay); }
                            }
                            if (valid) { row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_AnimationRow(row); }
                            else m_Status = "Animation trim extends outside its source window; the occurrence is preserved.";
                        }
                    }
                    break;
                }
                case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
                { auto found = FindRow(m_Effects, m_DragRowId); if (found != m_Effects.end()) { auto row = *found; row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_EffectRow(row); } break; }
                case TRACK_KIND::SOUND:
                {
                    auto found = FindRow(m_Sounds, m_DragRowId);
                    if (found != m_Sounds.end())
                    {
                        auto row = *found; const auto sourceStart = std::int64_t(row.sourceStartMs) +
                            (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START) ?
                                std::int64_t(m_DragPreviewStartMs) - row.startMs : 0);
                        if (sourceStart >= 0 && sourceStart <= LIMIT_MS)
                        {
                            row.sourceStartMs = static_cast<std::uint32_t>(sourceStart);
                            row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_SoundRow(row);
                        }
                        else m_Status = "Sound trim extends before its source; the occurrence is preserved.";
                    }
                    break;
                }
                case TRACK_KIND::COLLIDER:
                { auto found = FindRow(m_Colliders, m_DragRowId); if (found != m_Colliders.end()) { auto row = *found; row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_ColliderRow(row); } break; }
                case TRACK_KIND::CAMERA:
                {
                    auto candidate = m_CameraRows; auto found = FindRow(candidate, m_DragRowId);
                    if (found != candidate.end())
                    {
                        for (auto& key : found->cue.Keyframes) key.iTimeMs = static_cast<std::uint32_t>(
                            std::llround(double(key.iTimeMs) * m_DragPreviewDurationMs / found->cue.iDurationMs));
                        found->startMs = m_DragPreviewStartMs; found->cue.iDurationMs = m_DragPreviewDurationMs;
                        if (Validate_CameraRows(candidate))
                        { m_CameraRows = std::move(candidate); m_Dirty = true; if (m_Active) Sample(); }
                    }
                    break;
                }
                }
            }
            m_DragRowId.clear(); Pause(m_DragWasPaused);
        }
    }
    ImGui::End();
    if (m_ResourcesOpen) Render_CompositionResources();
    if (m_BoxDetailOpen) Render_BoxDetail();
    Render_Colliders();
}
}
