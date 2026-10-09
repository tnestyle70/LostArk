#include "imgui.h"
#include "SequencerTool.h"
#include "CompositionTimeline.h"
#include "SequenceCameraEditor.h"
#include "ClassMovieInspector.h"
#include <set>
#include <cmath>

#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <string_view>
#include <utility>

namespace
{
    using BOSS = Client::COMPOSITION_WORKBENCH_BOSS;
    using PANE = Client::COMPOSITION_WORKBENCH_PANE;
    using TARGET = Client::COMPOSITION_WORKBENCH_TARGET;

    constexpr std::array<PANE, 6u> PANES = {
        PANE::SEQUENCER, PANE::PATTERNS, PANE::RESOURCES,
        PANE::DETAILS, PANE::PREVIEW, PANE::BOSS_PATTERN };

    const char* BossLabel(const BOSS boss)
    {
        switch (boss)
        {
        case BOSS::VALTAN: return "Valtan";
        case BOSS::KOUKU_SAYDON: return "Saydon";
        case BOSS::KOUKU_SAYDON_GATE2: return "Large Saydon, Kouku";
        case BOSS::KOUKU_SAYDON_GATE3: return "Saydon (Gate 3)";
        case BOSS::KOUKU_SAYDON_ENCORE: return "Bingo / Encore Saydon";
        default: return "Unavailable boss";
        }
    }

    constexpr std::array<BOSS, 5u> BOSS_ENTRIES = {
        BOSS::VALTAN, BOSS::KOUKU_SAYDON, BOSS::KOUKU_SAYDON_GATE2,
        BOSS::KOUKU_SAYDON_GATE3, BOSS::KOUKU_SAYDON_ENCORE };

    bool EditMovieValue(const char* label, Client::DATA_JSON_VALUE& value,
        std::map<std::string, int>& selections, const std::string& path, const bool readOnly = false)
    {
        using Json = Client::DATA_JSON_VALUE;
        bool changed = false;
        ImGui::PushID(path.c_str());
        ImGui::BeginDisabled(readOnly);
        if (value.Is_Number())
        {
            double number = value.Get_Number();
            if (ImGui::InputDouble(label, &number, 0., 0., "%.9g") && std::isfinite(number))
            { value = Json::Number(number, value.Was_FloatingPointToken()); changed = true; }
        }
        else if (value.Is_Boolean())
        {
            bool boolean = value.Get_Boolean();
            if (ImGui::Checkbox(label, &boolean)) { value = Json::Boolean(boolean); changed = true; }
        }
        else if (value.Is_String())
        {
            if (readOnly) ImGui::TextWrapped("%s: %s", label, value.Get_String().c_str());
            else
            {
                std::vector<char> text((std::max)(size_t(2048), value.Get_String().size() + 512), '\0');
                std::copy(value.Get_String().begin(), value.Get_String().end(), text.begin());
                if (ImGui::InputText(label, text.data(), text.size())) { value = Json::String(text.data()); changed = true; }
            }
        }
        else if (value.Is_Object())
        {
            if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto fields = value.Get_Object();
                for (auto& [name, child] : fields)
                {
                    const bool identity = name.ends_with("Id") || name == "slotId" || name == "source" ||
                        name == "family" || name == "materialName" || name == "parameter";
                    changed |= EditMovieValue(name.c_str(), child, selections, path + "/" + name, readOnly || identity);
                }
                if (changed) value = Json::Object(std::move(fields), value.Get_ObjectInsertionOrder());
                ImGui::TreePop();
            }
        }
        else if (value.Is_Array())
        {
            const auto& source = value.Get_Array();
            const bool vector = !source.empty() && source.size() <= 4u &&
                std::all_of(source.begin(), source.end(), [](const auto& item) { return item.Is_Number(); });
            if (vector)
            {
                std::array<double, 4> numbers{};
                for (size_t i = 0; i < source.size(); ++i) numbers[i] = source[i].Get_Number();
                if (ImGui::InputScalarN(label, ImGuiDataType_Double, numbers.data(), static_cast<int>(source.size()), nullptr, nullptr, "%.9g") &&
                    std::all_of(numbers.begin(), numbers.end(), [](double number) { return std::isfinite(number); }))
                {
                    Json::ARRAY items; for (size_t i = 0; i < source.size(); ++i) items.push_back(Json::Number(numbers[i]));
                    value = Json::Array(std::move(items)); changed = true;
                }
            }
            else if (ImGui::TreeNode(label, "%s (%zu)", label, source.size()))
            {
                if (!source.empty())
                {
                    int& index = selections[path];
                    ImGui::InputInt("Key / item", &index);
                    index = std::clamp(index, 0, static_cast<int>(source.size()) - 1);
                    Json edited = source[index];
                    changed = EditMovieValue("Values", edited, selections, path + "/" + std::to_string(index), readOnly);
                    auto items = source;
                    if (changed) items[index] = std::move(edited);
                    const bool key = items[index].Is_Object() && items[index].Find("timeMs");
                    if (key && !readOnly)
                    {
                        if (ImGui::Button("Duplicate key"))
                        {
                            auto duplicate = items[index]; auto fields = duplicate.Get_Object();
                            if (auto id = fields.find("keyId"); id != fields.end() && id->second.Is_String())
                            {
                                const std::string stem = id->second.Get_String() + ".edit";
                                size_t suffix = 1;
                                auto exists = [&](const std::string& candidate) { return std::any_of(items.begin(), items.end(), [&](const auto& item) {
                                    const auto* other = item.Find("keyId"); return other && other->Is_String() && other->Get_String() == candidate; }); };
                                while (exists(stem + std::to_string(suffix))) ++suffix;
                                id->second = Json::String(stem + std::to_string(suffix));
                            }
                            if (auto time = fields.find("timeMs"); time != fields.end() && time->second.Is_Number())
                                time->second = Json::Number(time->second.Get_Number() + 1.);
                            items.insert(items.begin() + index + 1, Json::Object(std::move(fields), duplicate.Get_ObjectInsertionOrder()));
                            ++index; changed = true;
                        }
                        ImGui::SameLine();
                        ImGui::BeginDisabled(items.size() <= 1u);
                        if (ImGui::Button("Delete key")) { items.erase(items.begin() + index); index = (std::max)(0, index - 1); changed = true; }
                        ImGui::EndDisabled();
                    }
                    if (changed) value = Json::Array(std::move(items));
                }
                ImGui::TreePop();
            }
        }
        else ImGui::Text("%s: empty", label);
        ImGui::EndDisabled(); ImGui::PopID(); return changed;
    }

    bool EditMovieModelPosition(const Client::DATA_JSON_VALUE& row, const std::size_t keyIndex,
        const std::array<double, 3>& position, const bool translatePhase,
        Client::DATA_JSON_VALUE& out, std::string& status)
    {
        using Json = Client::DATA_JSON_VALUE;
        const auto* source = row.Find("keys");
        if (!source || !source->Is_Array() || source->Get_Array().empty() ||
            (!translatePhase && keyIndex >= source->Get_Array().size()) ||
            !std::all_of(position.begin(), position.end(), [](double value) { return std::isfinite(value); }))
        { status = "The model position edit is invalid; the pending row was preserved."; return false; }
        auto keys = source->Get_Array();
        for (std::size_t i = 0u; i < keys.size(); ++i)
        {
            if (!translatePhase && i != keyIndex) continue;
            const auto* current = keys[i].Find("positionOffset");
            if (!current || !current->Is_Array() || current->Get_Array().size() != 3u)
            { status = "This model key has no editable position; the pending row was preserved."; return false; }
            Json::ARRAY values;
            for (std::size_t axis = 0u; axis < 3u; ++axis)
            {
                const auto& component = current->Get_Array()[axis];
                if (!component.Is_Number())
                { status = "This model key has an invalid position; the pending row was preserved."; return false; }
                const double value = translatePhase ? component.Get_Number() + position[axis] : position[axis];
                if (!std::isfinite(value) || std::abs(value) > 100000.)
                { status = "Model positions must be finite and within 100000 m; the pending row was preserved."; return false; }
                values.push_back(Json::Number(value));
            }
            auto fields = keys[i].Get_Object(); fields["positionOffset"] = Json::Array(std::move(values));
            keys[i] = Json::Object(std::move(fields), keys[i].Get_ObjectInsertionOrder());
        }
        auto fields = row.Get_Object(); fields["keys"] = Json::Array(std::move(keys));
        out = Json::Object(std::move(fields), row.Get_ObjectInsertionOrder());
        status = translatePhase ? "This phase's model position offset is staged. Apply row previews it; Save movie keeps it." :
            "Model key position staged. Apply row previews it; Save movie keeps it.";
        return true;
    }

    class CClassSelectionWorkbenchSession final : public Client::ICompositionWorkbenchSession
    {
    public:
        using CALLBACKS = Client::CSequencerTool::CLASS_SELECTION_PREVIEW_CALLBACKS;
        using STATE = Client::CSequencerTool::CLASS_SELECTION_PREVIEW_STATE;

        explicit CClassSelectionWorkbenchSession(CALLBACKS callbacks = {})
            : m_Callbacks(std::move(callbacks)) {}

        void On_WorkbenchDeactivated() override
        {
            const auto state = Read_State();
            if (m_OwnsPlayback && state.active && state.ownerToken == m_OwnerToken && m_Callbacks.stop)
                m_Callbacks.stop();
            m_OwnsPlayback = false;
            m_Scrubbing = false;
            m_Pending = COMMAND::NONE; m_Drag.reset(); m_DeleteWorldItem.clear();
            m_RequestedRepeatMovie.reset();
        }

        void Begin_WorkbenchFrame() override
        {
            m_State = Read_State();
            if (!m_State.available) m_OpenedAuthoring = false;
            else if (!m_OpenedAuthoring && m_Callbacks.beginAuthoring)
            {
                m_OpenedAuthoring = m_Callbacks.beginAuthoring(m_EditStatus);
                m_State = Read_State();
            }
            Refresh_RowSource();
            if (!m_State.active || m_State.ownerToken != m_OwnerToken) m_OwnsPlayback = false;
            if (!m_RowDirty && m_FollowPlayback && m_State.active && m_State.activeClassId == m_State.selectedClassId && m_ViewLoop != m_State.looping)
            {
                m_ViewLoop = m_State.looping; m_SelectedBox.clear(); m_SelectedKind.clear(); m_SelectedRow.clear();
                m_EditBox.reset(); m_CameraEditor = {}; m_Drag.reset();
            }
            m_Timeline = m_Callbacks.timeline ? m_Callbacks.timeline(m_State.selectedClassId, m_ViewLoop) : nullptr;
            if (!m_Scrubbing) m_EditMs = MatchesPlayback() ? static_cast<float>(m_State.clockMs) : 0.f;
            Sync_WorldSelection();
        }

        void Render_WorkbenchPane(const PANE pane) override
        {
            switch (pane)
            {
            case PANE::PATTERNS:
                ImGui::SeparatorText("World sequences");
                if (ImGui::BeginCombo("World##ClassSelection", "Character Select"))
                {
                    ImGui::Selectable("Character Select", true);
                    ImGui::EndCombo();
                }
                Render_CategorySelector();
                break;
            case PANE::TOOLBAR:
                Render_Transport();
                break;
            case PANE::SEQUENCER:
                Render_Timeline();
                break;
            case PANE::DETAILS:
                Render_Details();
                break;
            case PANE::RESOURCES:
                ImGui::Text("Character Select / %s", m_State.selectedLabel.c_str());
                if (m_Timeline)
                    for (const auto& lane : MOVIE_LANES)
                    {
                        size_t count = 0;
                        for (const auto& row : m_Timeline->rows) if (row.kind == lane.sourceKind) count += row.boxes.size();
                        ImGui::BulletText("%s: %zu", lane.label, count);
                    }
                break;
            case PANE::PREVIEW:
                ImGui::TextWrapped("Play opens the selected class movie on its original stage.");
                ImGui::TextWrapped("All rows use Movie time. Source time preserves the original slow-motion curve.");
                ImGui::TextWrapped("Select a box to edit its rows and keys. Apply, Save, Reload and Play use the same F1 movie owner.");
                m_WorldInspector.Render(m_Callbacks.inspection, m_State.selectedClassId, m_ViewLoop, m_RowDirty);
                break;
            default: break;
            }
        }

        void End_WorkbenchFrame() override
        {
            // Inspector source commands run after the row panes. Refresh clean
            // cached rows before the camera window can edit one this frame.
            m_State = Read_State();
            Refresh_RowSource();
            Render_CameraWindow();
            Render_VisibilityWindow();
            Sync_WorldSelection();
            if (!m_DeleteWorldItem.empty())
            {
                if (!m_RowDirty && !m_State.authoringPublishPending)
                    (void)Submit_Inspection(Client::CLASS_MOVIE_INSPECTION_ACTION::EXCLUDE, m_DeleteWorldItem);
                m_DeleteWorldItem.clear();
            }
            if (m_RequestedRepeatMovie)
            {
                // Submit before Save; a camera edit opened later this frame still
                // owns its pending row and must not be invalidated by this change.
                if (!m_RowDirty && m_OpenedAuthoring && !m_State.authoringPublishPending && m_Callbacks.setRepeatMovie)
                {
                    (void)m_Callbacks.setRepeatMovie(m_RequestedRepeatMovie->classId,
                        m_RequestedRepeatMovie->repeat, m_EditStatus);
                    m_State = Read_State();
                    Refresh_RowSource();
                }
                else m_EditStatus = "Apply or revert pending row edits before changing Movie repeat.";
                m_RequestedRepeatMovie.reset();
            }
            if (m_SaveRequested && m_RowDirty) m_ApplyRequested = true;
            if (m_RequestedRate && m_Callbacks.setPlaybackRate)
                (void)m_Callbacks.setPlaybackRate(*m_RequestedRate);
            m_RequestedRate.reset();
            if (m_ApplyRequested && m_EditBox && m_Callbacks.applyBox)
            {
                if (!Row_SourceChanged() && m_Callbacks.applyBox(*m_EditBox, m_EditValue, m_EditStatus))
                { m_EditBox.reset(); m_RowDirty = false; }
            }
            m_ApplyRequested = false;
            if (m_SaveRequested && !m_RowDirty && m_Callbacks.saveAuthoring && m_Callbacks.saveAuthoring(m_EditStatus)) m_EditBox.reset();
            if (m_PublishRequested && !m_RowDirty && m_Callbacks.publishAuthoring) m_Callbacks.publishAuthoring(m_EditStatus);
            m_PublishRequested = false;
            m_SaveRequested = false;
            if (m_ReloadRequested && m_Callbacks.reloadAuthoring && m_Callbacks.reloadAuthoring(m_EditStatus))
            { m_EditBox.reset(); m_RowDirty = false; }
            m_ReloadRequested = false;
            if (!m_OpenEffect.empty() && m_Callbacks.openEffectEditor)
            {
                // Opening V1 can deactivate this pane synchronously. Transfer
                // the movie first, and restore our claim if admission fails.
                const bool owned = std::exchange(m_OwnsPlayback, false);
                if (!m_Callbacks.openEffectEditor(m_State.selectedClassId, m_ViewLoop, m_OpenEffect, m_EditStatus))
                    m_OwnsPlayback = owned;
            }
            m_OpenEffect.clear();
            const auto command = std::exchange(m_Pending, COMMAND::NONE);
            switch (command)
            {
            case COMMAND::PLAY:
                if (m_Callbacks.play && m_Callbacks.play()) { m_FollowPlayback = true; Claim_Playback(); }
                break;
            case COMMAND::STOP:
                if (m_Callbacks.stop) m_Callbacks.stop();
                m_OwnsPlayback = false;
                break;
            case COMMAND::PAUSE:
                if (m_Callbacks.setPaused) { m_Callbacks.setPaused(m_RequestedPause); Claim_Playback(); }
                break;
            case COMMAND::SEEK:
                if (m_Callbacks.setPaused) m_Callbacks.setPaused(true);
                if (m_Callbacks.seek && m_Callbacks.seek(m_SeekLoop, m_SeekMs))
                {
                    if (m_Callbacks.setPaused) m_Callbacks.setPaused(true);
                    Claim_Playback();
                }
                break;
            default: break;
            }
        }

    private:
        enum class COMMAND { NONE, PLAY, STOP, PAUSE, SEEK };

        STATE Read_State() const
        {
            if (m_Callbacks.state) return m_Callbacks.state();
            STATE state;
            state.status = "Enter Character Select from the Lobby to play this sequence.";
            return state;
        }

        void Claim_Playback()
        {
            const auto state = Read_State();
            m_OwnerToken = state.ownerToken;
            m_OwnsPlayback = state.active;
        }

        void Queue_Seek(const bool loop, const double timeMs)
        {
            m_SeekLoop = loop;
            // The slider uses floats while the source slomo duration is double.
            // Its rounded-up final tick must still request the valid phase end.
            const double durationMs = loop ? m_State.loopDurationMs : m_State.introDurationMs;
            m_SeekMs = std::clamp(timeMs, 0., (std::max)(0., durationMs));
            m_Pending = COMMAND::SEEK;
        }

        void Render_CategorySelector()
        {
            ImGui::BeginDisabled(!m_Callbacks.selectCategory || m_State.options.empty() || m_RowDirty);
            if (ImGui::BeginCombo("Category##ClassMovie", m_State.selectedLabel.c_str()))
            {
                for (std::size_t i = 0u; i < m_State.options.size(); ++i)
                {
                    const auto& option = m_State.options[i];
                    ImGui::PushID(option.categoryId.c_str());
                    const bool selected = i == m_State.selectedCategory;
                    if (ImGui::Selectable(option.label.c_str(), selected) && m_Callbacks.selectCategory)
                    {
                        m_Callbacks.selectCategory(i);
                        m_Scrubbing = false;
                        m_Pending = COMMAND::NONE;
                        m_RequestedRepeatMovie.reset();
                        m_State = Read_State();
                        m_SelectedBox.clear(); m_SelectedRow.clear(); m_SelectedKind.clear(); m_ViewLoop = false;
                        m_EditBox.reset(); m_CameraEditor = {}; m_Drag.reset();
                        m_Timeline = m_Callbacks.timeline ? m_Callbacks.timeline(m_State.selectedClassId, false) : nullptr;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::EndDisabled();
        }

        void Render_PlaybackButtons(const bool sequencer)
        {
            if (sequencer)
            {
                ImGui::BeginDisabled(!m_OpenedAuthoring || m_State.authoringPublishPending);
                if (ImGui::Button("Save")) m_SaveRequested = true;
                ImGui::EndDisabled();
                ImGui::SameLine();
            }
            ImGui::BeginDisabled(!m_State.available || !m_Callbacks.play);
            const bool restarting = m_State.active && m_State.activeClassId == m_State.selectedClassId;
            if (ImGui::Button(sequencer ? "Play" : restarting ? "Play All (restart intro)" : "Play All")) m_Pending = COMMAND::PLAY;
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!m_State.active || m_State.completedHold || !m_Callbacks.setPaused);
            if (ImGui::Button(m_State.paused ? "Resume" : "Pause"))
            {
                m_RequestedPause = !m_State.paused;
                m_Pending = COMMAND::PAUSE;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!m_State.active || !m_Callbacks.stop);
            if (ImGui::Button("Stop")) m_Pending = COMMAND::STOP;
            ImGui::EndDisabled();
            if (sequencer)
            {
                ImGui::SameLine();
                ImGui::BeginDisabled(m_RowDirty || m_State.authoringPublishPending ||
                    !m_State.available || !m_Callbacks.seek);
                if (ImGui::Button("Reset")) Queue_Seek(m_ViewLoop, 0.);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Pause at the start of the selected Intro or Loop.");
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::Text("%s | %.0f / %.0f ms",
                    m_State.authoringDirty || m_RowDirty ? "Unsaved" : "Saved",
                    MatchesPlayback() ? m_State.clockMs : double(m_EditMs),
                    m_ViewLoop ? m_State.loopDurationMs : m_State.introDurationMs);
            }
        }

        void Render_Transport()
        {
            ImGui::TextUnformatted("World / Character Select");
            Render_CategorySelector();
            bool repeatMovie = m_State.repeatMovie;
            ImGui::BeginDisabled(!m_State.available || !m_OpenedAuthoring || !m_Callbacks.setRepeatMovie ||
                m_RowDirty || m_State.authoringPublishPending);
            if (ImGui::Checkbox("Repeat movie", &repeatMovie))
                m_RequestedRepeatMovie = REPEAT_MOVIE_REQUEST{m_State.selectedClassId, repeatMovie};
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Off: Play All runs the intro once and holds its last frame. An explicit Loop preview also plays once.\nOn: the intro continues into the repeating loop. Save movie keeps this class setting.");
            ImGui::EndDisabled();
            Render_PlaybackButtons(false);
            if (m_State.completedHold)
                ImGui::TextWrapped("Movie complete - holding last frame. Play All replays.");
            Render_RateControl();
            ImGui::BeginDisabled(!m_OpenedAuthoring || m_State.authoringPublishPending);
            if (ImGui::Button("Save movie")) m_SaveRequested = true;
            ImGui::SameLine();
            ImGui::BeginDisabled(m_RowDirty || m_State.authoringDirty || !m_Callbacks.publishAuthoring);
            if (ImGui::Button("Publish movie")) m_PublishRequested = true;
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Reload saved movie"))
            {
                if (m_State.authoringDirty || m_RowDirty) ImGui::OpenPopup("Discard movie draft?");
                else m_ReloadRequested = true;
            }
            ImGui::EndDisabled();
            if (ImGui::BeginPopupModal("Discard movie draft?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextUnformatted("Reload replaces this unsaved movie draft with the saved sources.");
                if (ImGui::Button("Reload")) { m_ReloadRequested = true; ImGui::CloseCurrentPopup(); }
                ImGui::SameLine(); if (ImGui::Button("Keep editing")) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            if (!m_State.authoringStatus.empty()) ImGui::TextWrapped("%s", m_State.authoringStatus.c_str());
            if (m_State.authoringDirty) ImGui::TextUnformatted("Movie has unsaved changes.");
            if (m_RowDirty)
            {
                ImGui::TextWrapped(Row_SourceChanged() ?
                    "Movie sources changed in another editor. This pending row is preserved; discard it and reopen the current source before applying or saving." :
                    "Save includes this pending row. Apply or revert before selecting another movie row.");
                if (m_EditBox && ImGui::Button("Return to edited row"))
                {
                    for (size_t i = 0; i < m_State.options.size(); ++i)
                        if (m_State.options[i].classId == m_EditBox->classId && m_Callbacks.selectCategory) m_Callbacks.selectCategory(i);
                    m_State = Read_State(); m_ViewLoop = m_EditBox->loop; m_FollowPlayback = false;
                    m_SelectedKind = m_EditBox->kind; m_SelectedBox = m_EditBox->boxId;
                    if (const auto timeline = m_Callbacks.timeline ? m_Callbacks.timeline(m_EditBox->classId, m_EditBox->loop) : nullptr)
                        for (const auto& row : timeline->rows)
                            for (const auto& box : row.boxes) if (row.kind == m_SelectedKind && box.id == m_SelectedBox) m_SelectedRow = row.id;
                }
                ImGui::SameLine();
                if (ImGui::Button(Row_SourceChanged() ? "Discard pending row and reopen" : "Revert pending row"))
                { m_RowDirty = false; m_EditBox.reset(); m_CameraEditor = {}; m_EditStatus.clear(); }
            }
            if (!m_EditStatus.empty()) ImGui::TextWrapped("%s", m_EditStatus.c_str());
            if (!m_State.status.empty()) ImGui::TextWrapped("%s", m_State.status.c_str());
        }


        struct MOVIE_LANE
        {
            const char* sourceKind;
            const char* label;
            ImU32 labelColor, boxColor;
        };
        // Display categories share the Boss/Sequence order. Source kinds and
        // actor/slot rows remain the identities used by movie edit callbacks.
        inline static constexpr std::array<MOVIE_LANE, 8> MOVIE_LANES = {{
            {"Animation", "Animation", Client::CompositionTimeline::AnimationLabelColor, Client::CompositionTimeline::AnimationColor},
            {"World Model", "World", Client::CompositionTimeline::WorldLabelColor, Client::CompositionTimeline::WorldColor},
            {"Material", "Material", Client::CompositionTimeline::SceneProfileLabelColor, Client::CompositionTimeline::SceneProfileColor},
            {"Effect", "Effect", Client::CompositionTimeline::PresentationLabelColor, Client::CompositionTimeline::PresentationColor},
            {"Sound", "Sound", Client::CompositionTimeline::PresentationLabelColor, Client::CompositionTimeline::PresentationColor},
            {"Camera", "Camera", Client::CompositionTimeline::PresentationLabelColor, Client::CompositionTimeline::PresentationColor},
            {"Light", "Light", Client::CompositionTimeline::PresentationLabelColor, Client::CompositionTimeline::PresentationColor},
            {"Time Control", "Time Control", Client::CompositionTimeline::LogicLabelColor, Client::CompositionTimeline::LogicColor}
        }};

        bool Row_SourceChanged() const
        { return m_EditBox && m_EditBox->generation != m_State.authoringGeneration; }

        void Refresh_RowSource()
        {
            if (m_Drag && m_Drag->before.generation != m_State.authoringGeneration) m_Drag.reset();
            if (!Row_SourceChanged()) return;
            if (m_RowDirty)
            {
                m_EditStatus = "Movie sources changed after this pending row was opened. Your edits are preserved. Discard the pending row and reopen it to use the current source.";
                return;
            }
            m_EditBox.reset(); m_CameraEditor = {}; m_KeySelections.clear();
        }

        bool MatchesPlayback() const
        { return m_State.active && m_State.activeClassId == m_State.selectedClassId && m_State.looping == m_ViewLoop; }

        void Sync_WorldSelection()
        {
            if (!m_Callbacks.inspection.state) return;
            const auto state = m_Callbacks.inspection.state(m_State.selectedClassId, m_ViewLoop);
            m_InspectedWorldId = state.selectedId;
            if (m_ObservedWorldClass != m_State.selectedClassId || m_ObservedWorldLoop != m_ViewLoop)
            {
                m_ObservedWorldClass = m_State.selectedClassId; m_ObservedWorldLoop = m_ViewLoop;
                m_ObservedWorldGeneration = 0u; m_ObservedWorldId.clear(); m_PendingWorldSelection.clear();
            }
            if (state.selectionGeneration != m_ObservedWorldGeneration || state.selectedId != m_ObservedWorldId)
            {
                m_ObservedWorldGeneration = state.selectionGeneration; m_ObservedWorldId = state.selectedId;
                m_PendingWorldSelection = state.selectedId;
            }
            if (m_PendingWorldSelection.empty()) return;
            if (m_SelectedKind == "World Model" && m_SelectedBox == m_PendingWorldSelection)
            { m_PendingWorldSelection.clear(); return; }
            if (m_RowDirty || m_Drag)
            {
                m_EditStatus = "A picked model is waiting. Apply, save or revert the pending row before changing selection.";
                return;
            }
            if (!m_Timeline) return;
            for (const auto& row : m_Timeline->rows) if (row.kind == "World Model")
                for (const auto& box : row.boxes) if (box.id == m_PendingWorldSelection)
                {
                    Select_Box(row, box);
                    m_PendingWorldSelection.clear();
                    return;
                }
            m_PendingWorldSelection.clear();
        }

        void Render_RateControl()
        {
            float rate = static_cast<float>(m_RequestedRate.value_or(m_State.playbackRate));
            ImGui::BeginDisabled(!m_Callbacks.setPlaybackRate || !m_State.available);
            ImGui::SetNextItemWidth(170.f);
            if (ImGui::SliderFloat("Movie speed", &rate, .05f, 2.f, "%.2fx")) m_RequestedRate = rate;
            ImGui::SameLine();
            if (ImGui::SmallButton("1x")) m_RequestedRate = 1.;
            ImGui::EndDisabled();
        }

        bool Submit_Inspection(const Client::CLASS_MOVIE_INSPECTION_ACTION action,
            const std::string& itemId = {}, const bool enabled = true)
        {
            if (!m_Callbacks.inspection.command) return false;
            Client::CLASS_MOVIE_INSPECTION_COMMAND command;
            command.action = action; command.itemId = itemId; command.enabled = enabled;
            return m_Callbacks.inspection.command(m_State.selectedClassId, m_ViewLoop, command, m_EditStatus);
        }

        void Render_VisibilityControls()
        {
            using ACTION = Client::CLASS_MOVIE_INSPECTION_ACTION;
            if (ImGui::Button("Movie visibility / models")) m_VisibilityWindow = true;
            ImGui::SameLine();
            if (ImGui::Button("Clear model Mute / Solo")) (void)Submit_Inspection(ACTION::CLEAR_PREVIEW);
            ImGui::SameLine();
            ImGui::BeginDisabled(!m_OpenedAuthoring || m_State.authoringPublishPending);
            if (ImGui::Button("Save movie##visibility")) m_SaveRequested = true;
            ImGui::EndDisabled();
            m_Inspection = m_Callbacks.inspection.state ?
                m_Callbacks.inspection.state(m_State.selectedClassId, m_ViewLoop) : Client::CLASS_MOVIE_INSPECTION_STATE{};
            m_InspectedWorldId = m_Inspection.selectedId;
            bool background = m_Inspection.showBackground, effects = m_Inspection.showEffects;
            ImGui::BeginDisabled(!m_Callbacks.inspection.command);
            if (ImGui::Checkbox("Background (preview)", &background)) (void)Submit_Inspection(ACTION::BACKGROUND, {}, background);
            ImGui::SameLine();
            if (ImGui::Checkbox("Effects (preview)", &effects)) (void)Submit_Inspection(ACTION::EFFECTS, {}, effects);
            ImGui::EndDisabled();
            const auto selected = std::find_if(m_Inspection.items.begin(), m_Inspection.items.end(),
                [this](const auto& item) { return item.id == m_SelectedBox && m_SelectedKind == "World Model"; });
            if (selected != m_Inspection.items.end())
            {
                const auto& item = *selected;
                ImGui::TextWrapped("Model: %s", item.label.c_str());
                if (ImGui::Button(item.muted ? "Unmute model" : "Mute model")) (void)Submit_Inspection(ACTION::MUTE, item.id, !item.muted);
                ImGui::SameLine();
                if (ImGui::Button(item.solo ? "Unsolo model" : "Solo model")) (void)Submit_Inspection(ACTION::SOLO, item.id, !item.solo);
                ImGui::SameLine();
                ImGui::BeginDisabled(m_RowDirty || m_State.authoringPublishPending);
                if (ImGui::Button(item.excluded ? "Restore to Movie" : "Delete from Movie"))
                {
                    if (item.excluded) (void)Submit_Inspection(ACTION::EXCLUDE, item.id, false);
                    else m_DeleteWorldItem = item.id;
                }
                ImGui::EndDisabled();
                ImGui::TextDisabled("Mute / Solo are temporary. Delete removes this model from the Movie; Save movie persists it. Restore keeps the resource available.");
            }
            else if (m_SelectedKind == "Effect")
            {
                for (const auto& row : m_Timeline->rows) if (row.kind == "Effect" && row.id == m_SelectedRow)
                    for (const auto& box : row.boxes) if (box.id == m_SelectedBox)
                    {
                        ImGui::BeginDisabled(m_RowDirty || !m_Callbacks.openEffectEditor);
                        if (ImGui::Button("Edit Elements / Mute / Hide")) m_OpenEffect = box.resource;
                        ImGui::EndDisabled();
                        ImGui::SameLine(); ImGui::TextWrapped("%s", box.resource.c_str());
                    }
            }
            else ImGui::TextDisabled("Select a World Model for Mute / Solo / Delete, or an Effect to edit its Elements.");
            if (m_State.authoringDirty) ImGui::TextUnformatted("Movie has unsaved changes.");
            if (!m_EditStatus.empty()) ImGui::TextWrapped("%s", m_EditStatus.c_str());
        }

        void Render_VisibilityWindow()
        {
            if (!m_VisibilityWindow) return;
            ImGui::SetNextWindowSize({820.f, 620.f}, ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Movie visibility / models###WorldMovieVisibility", &m_VisibilityWindow))
                m_WorldInspector.Render(m_Callbacks.inspection, m_State.selectedClassId, m_ViewLoop, m_RowDirty);
            ImGui::End();
        }

        void Render_Timeline()
        {
            Render_PlaybackButtons(true);
            ImGui::Text("%s / %s", m_State.selectedLabel.c_str(), m_ViewLoop ? "Loop" : "Intro");
            ImGui::BeginDisabled(m_RowDirty);
            if (ImGui::RadioButton("Intro", !m_ViewLoop)) { m_ViewLoop = false; m_FollowPlayback = false; m_EditBox.reset(); m_SelectedBox.clear(); m_SelectedKind.clear(); m_Drag.reset(); }
            ImGui::SameLine();
            if (ImGui::RadioButton("Loop", m_ViewLoop)) { m_ViewLoop = true; m_FollowPlayback = false; m_EditBox.reset(); m_SelectedBox.clear(); m_SelectedKind.clear(); m_Drag.reset(); }
            ImGui::SameLine(); ImGui::Checkbox("Follow playback", &m_FollowPlayback);
            ImGui::EndDisabled();
            m_Timeline = m_Callbacks.timeline ? m_Callbacks.timeline(m_State.selectedClassId, m_ViewLoop) : nullptr;
            if (!m_Timeline) { ImGui::TextUnformatted("Movie timeline is not loaded."); return; }
            ImGui::Text("Movie duration %.3f s / source duration %.3f s",
                m_Timeline->movieDurationMs * .001, m_Timeline->sourceDurationMs * .001);
            if (MatchesPlayback())
                ImGui::Text("Movie %.3f s -> source %.3f s | source rate %.3fx | movie speed %.2fx",
                    m_State.clockMs * .001, m_State.sourceClockMs * .001, m_State.sourceRate, m_State.playbackRate);
            const float duration = static_cast<float>(m_Timeline->movieDurationMs);
            Render_VisibilityControls();
            m_RowFilter.Draw("Filter boxes", 220.f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(155.f);
            ImGui::BeginDisabled(m_Drag.has_value() || m_Scrubbing);
            if (ImGui::SliderFloat("Zoom", &m_PixelsPerSecond, 1.f, 500.f, "%.1f px/s"))
                m_TimelineFitRequested = false;
            ImGui::SameLine();
            if (ImGui::Button("Fit##MovieTimeline")) m_TimelineFitRequested = true;
            ImGui::EndDisabled();
            ImGui::SameLine(); ImGui::Checkbox("Active at cursor", &m_ActiveOnly);
            ImGui::TextDisabled("Ctrl + mouse wheel: zoom at cursor | Drag the ruler to seek on release");
            if (ImGui::BeginChild("MovieRows", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders,
                ImGuiWindowFlags_HorizontalScrollbar))
            {
                using namespace Client::CompositionTimeline;
                constexpr float labelWidth = LabelWidth;
                const auto rowMetrics = GetCompactRowMetrics();
                auto* draw = ImGui::GetWindowDrawList();
                const auto origin = ImGui::GetCursorScreenPos();
                const float availableWidth = (std::max)(40.f, ImGui::GetWindowSize().x -
                    ImGui::GetStyle().WindowPadding.x * 2.f - ImGui::GetStyle().ScrollbarSize - labelWidth - 24.f);
                if (m_TimelineFitRequested)
                {
                    m_PixelsPerSecond = FitPixelsPerSecond(availableWidth, duration);
                    m_TimelineFitRequested = false;
                    ImGui::SetScrollX(0.f);
                }
                const auto& io = ImGui::GetIO();
                if (ImGui::IsWindowHovered() && io.KeyCtrl && io.MouseWheel != 0.f &&
                    !io.WantTextInput && !ImGui::IsAnyItemActive() && !m_Drag && !m_Scrubbing)
                {
                    const float oldScale = m_PixelsPerSecond;
                    m_PixelsPerSecond = std::clamp(m_PixelsPerSecond * std::pow(1.2f, io.MouseWheel), 1.f, 500.f);
                    ImGui::SetScrollX((std::max)(0.f, ImGui::GetScrollX() +
                        (io.MousePos.x - origin.x - labelWidth) * (m_PixelsPerSecond / oldScale - 1.f)));
                }
                // The canvas can fill the window without changing the authored time scale.
                const float width = (std::max)(availableWidth, duration * .001f * m_PixelsPerSecond);
                const float timeX = origin.x + labelWidth;
                DrawRuler(draw, {timeX, origin.y}, {timeX + width, origin.y + rowMetrics.rulerHeight},
                    static_cast<uint32_t>(duration), m_PixelsPerSecond);
                ImGui::SetCursorScreenPos({timeX, origin.y});
                const bool canRulerSeek = !m_RowDirty && !m_State.authoringPublishPending &&
                    m_State.available && bool(m_Callbacks.seek) && !m_Drag;
                ImGui::BeginDisabled(!canRulerSeek);
                ImGui::InvisibleButton("MovieRulerSeek", {width, rowMetrics.rulerHeight});
                if (ImGui::IsItemActivated())
                {
                    m_Scrubbing = true; m_ScrubLoop = m_ViewLoop;
                    m_RequestedPause = true; m_Pending = COMMAND::PAUSE;
                }
                if (ImGui::IsItemActive())
                    m_EditMs = std::clamp((io.MousePos.x - timeX) * 1000.f / m_PixelsPerSecond, 0.f, (std::max)(0.f, duration));
                if (ImGui::IsItemDeactivated())
                {
                    if (m_Scrubbing && canRulerSeek) Queue_Seek(m_ScrubLoop, m_EditMs);
                    m_Scrubbing = false;
                }
                ImGui::EndDisabled();
                float y = origin.y + rowMetrics.rulerHeight;
                using REF = std::pair<const Client::CLASS_MOVIE_TIMELINE_ROW*, const Client::CLASS_MOVIE_TIMELINE_BOX*>;
                const double cursorMs = m_Scrubbing ? m_EditMs : MatchesPlayback() ? m_State.clockMs : m_EditMs;
                for (const auto& lane : MOVIE_LANES)
                {
                    std::vector<REF> refs;
                    std::vector<DISPLAY_INTERVAL> intervals;
                    size_t total = 0, active = 0;
                    for (const auto& row : m_Timeline->rows) if (row.kind == lane.sourceKind)
                        for (const auto& box : row.boxes)
                        {
                            ++total;
                            const bool alive = cursorMs >= box.movieStartMs && cursorMs < (std::max)(box.movieEndMs, box.movieHoldEndMs);
                            if (alive) ++active;
                            if (m_ActiveOnly && !alive) continue;
                            if (!m_RowFilter.PassFilter((row.label + " " + box.label + " " + box.resource).c_str())) continue;
                            refs.push_back({&row, &box});
                            intervals.push_back({box.id, box.movieStartMs, box.movieEndMs - box.movieStartMs,
                                (std::max)(0., box.movieHoldEndMs - box.movieEndMs), {}});
                        }
                    // Display rows share a category; source actor/slot rows still
                    // own selection, overlap validation and all edit commands.
                    const auto layout = AllocateDisplayRows(std::move(intervals), MinimumBoxWidth * 1000. / m_PixelsPerSecond);
                    const auto geometry = DrawCategoryLane(draw, {origin.x, y}, labelWidth + width,
                        labelWidth, layout.rowCount, rowMetrics.laneHeight, lane.label, lane.labelColor,
                        (&lane - MOVIE_LANES.data()) % 2 != 0);
                    std::vector<std::vector<REF>> refsByLane(layout.rowCount);
                    for (const auto& ref : refs) refsByLane[layout.occurrenceRows.at(ref.second->id)].push_back(ref);
                    ImGui::PushID(lane.sourceKind);
                    for (size_t subrow = 0; subrow < refsByLane.size(); ++subrow)
                    {
                        const ImVec2 p{origin.x, geometry.SubrowOrigin(subrow).y};
                        ImGui::PushID(static_cast<int>(subrow));
                        ImGui::SetCursorScreenPos(p);
                        ImGui::InvisibleButton("lane", {labelWidth + width, geometry.rowHeight});
                        const bool hovered = ImGui::IsItemHovered();
                        if (hovered && io.MousePos.x < geometry.contentMin.x)
                            ImGui::SetTooltip("%s / %zu boxes / %zu active / %zu shown",
                                lane.label, total, active, refs.size());
                        for (const auto& [row, box] : refsByLane[subrow])
                            Render_TimelineBox(*row, *box, p, labelWidth, hovered, lane.boxColor, rowMetrics);
                        ImGui::PopID();
                    }
                    ImGui::PopID();
                    y = geometry.contentMax.y;
                }
                const double clockMs = m_Scrubbing ? m_EditMs : MatchesPlayback() ? m_State.clockMs : m_EditMs;
                const float cursorX = timeX + float(clockMs * .001 * m_PixelsPerSecond);
                draw->AddLine({cursorX, origin.y}, {cursorX, y}, IM_COL32(255,210,80,255), 2.f);
                ImGui::SetCursorScreenPos(origin);
                ImGui::Dummy({labelWidth + width + 24.f, y - origin.y});
            }
            ImGui::EndChild();
            const auto& io = ImGui::GetIO();
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                !io.WantTextInput && !ImGui::IsAnyItemActive() && !io.KeyCtrl && !io.KeyAlt && !io.KeySuper &&
                !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId) &&
                !ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::GetDragDropPayload() && !m_Drag &&
                !m_RowDirty && !m_State.authoringPublishPending && m_SelectedKind == "World Model" &&
                ImGui::IsKeyPressed(ImGuiKey_Delete, false))
            {
                const auto item = std::find_if(m_Inspection.items.begin(), m_Inspection.items.end(),
                    [this](const auto& value) { return value.id == m_SelectedBox && !value.excluded; });
                if (item != m_Inspection.items.end()) m_DeleteWorldItem = item->id;
            }
            Update_TimelineGesture();
        }

        void Select_Box(const Client::CLASS_MOVIE_TIMELINE_ROW& row, const Client::CLASS_MOVIE_TIMELINE_BOX& box)
        {
            if (m_RowDirty) { m_EditStatus = "Apply or save the edited camera before selecting another box."; return; }
            m_SelectedKind = row.kind; m_SelectedRow = row.id; m_SelectedBox = box.id;
            if (row.kind == "World Model" && m_Callbacks.inspection.command && m_InspectedWorldId != box.id)
            {
                Client::CLASS_MOVIE_INSPECTION_COMMAND command;
                command.action = Client::CLASS_MOVIE_INSPECTION_ACTION::SELECT;
                command.itemId = box.id;
                if (m_Callbacks.inspection.command(m_State.selectedClassId, m_ViewLoop, command, m_EditStatus))
                    m_InspectedWorldId = box.id;
            }
            m_CameraKey = 0; m_EditBox.reset(); m_CameraEditor = {}; m_FollowPlayback = false;
            m_ModelPositionDelta = {};
        }

        void Render_TimelineBox(const Client::CLASS_MOVIE_TIMELINE_ROW& row,
            const Client::CLASS_MOVIE_TIMELINE_BOX& box, const ImVec2 p, float labelWidth, bool hovered,
            const ImU32 boxColor, const Client::CompositionTimeline::ROW_METRICS& rowMetrics)
        {
            using namespace Client::CompositionTimeline;
            auto* draw = ImGui::GetWindowDrawList();
            double start = box.movieStartMs, end = box.movieEndMs;
            if (m_Drag && m_Drag->before.boxId == box.id && m_Drag->before.kind == row.kind)
            { start = m_Drag->startMovie; end = m_Drag->endMovie; }
            const float x = p.x + labelWidth + float(start * .001 * m_PixelsPerSecond);
            const float endX = (std::max)(x + MinimumBoxWidth, p.x + labelWidth + float(end * .001 * m_PixelsPerSecond));
            const ImVec2 min{x, p.y}, max{endX, p.y + rowMetrics.boxHeight};
            const bool selected = (m_SelectedKind == row.kind && m_SelectedBox == box.id) ||
                (row.kind == "World Model" && m_InspectedWorldId == box.id);
            const bool editable = row.kind == "Camera" || row.kind == "Effect" || row.kind == "Sound" ||
                (row.kind == "Animation" && !box.loopAnimation && box.nativeDurationMs > 0.);
            std::string label = row.kind == "Animation" ? row.label + " | " + box.label : box.label;
            ImU32 color = boxColor;
            if (row.kind == "World Model")
            {
                const auto item = std::find_if(m_Inspection.items.begin(), m_Inspection.items.end(),
                    [&](const auto& value) { return value.id == box.id; });
                if (item != m_Inspection.items.end())
                {
                    if (item->excluded) { label += " [excluded]"; color = IM_COL32(73,62,62,220); }
                    else if (item->muted) { label += " [muted]"; color = IM_COL32(67,70,77,220); }
                    else if (item->solo) { label += " [solo]"; color = IM_COL32(72,126,94,255); }
                }
            }
            DrawBox(draw, min, max, color, selected, label.c_str(), editable, editable);
            if (box.movieHoldEndMs > end)
                draw->AddRectFilled({endX, p.y + rowMetrics.boxHeight * .3f},
                    {p.x + labelWidth + float(box.movieHoldEndMs * .001 * m_PixelsPerSecond), p.y + rowMetrics.boxHeight * .7f}, IM_COL32(72,116,79,70));
            if (hovered && ImGui::IsMouseHoveringRect(min, max))
            {
                ImGui::SetTooltip("%s\n%s\n%s\nMovie %.3f - %.3f s | Source %.3f - %.3f s\n%s",
                    row.label.c_str(), box.label.c_str(), box.resource.c_str(),
                    start * .001, end * .001, box.sourceStartMs * .001, box.sourceEndMs * .001,
                    editable ? (row.kind == "Camera" ? "Drag to reorder cuts; drag an edge to move the shared cut boundary. Double-click to edit camera." : "Drag to move; drag an edge to trim. Double-click to open the resource.") : "This track spans its phase. Select it to edit its keys.");
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_RowDirty)
                {
                    Select_Box(row, box);
                    if (editable && m_Callbacks.editableBox && m_Callbacks.editTiming && !m_State.authoringPublishPending)
                    {
                        Client::CLASS_MOVIE_AUTHORING_BOX before;
                        if (m_Callbacks.editableBox(m_State.selectedClassId, m_ViewLoop, row.kind, box.id, before, m_EditStatus))
                        {
                            const auto hit = HitBoxGesture(ImGui::GetIO().MousePos.x, x, endX, 6.f, true, true);
                            m_Drag = DRAG{std::move(before), static_cast<Client::CLASS_MOVIE_TIMING_EDIT>(hit),
                                ImGui::GetIO().MousePos.x, start, end, start, end};
                        }
                    }
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && selected)
                {
                    m_Drag.reset();
                    if (row.kind == "Camera") m_CameraWindow = true;
                    if (row.kind == "Effect") m_OpenEffect = box.resource;
                }
            }
        }

        void Update_TimelineGesture()
        {
            if (!m_Drag || !m_Timeline) return;
            const double delta = (ImGui::GetIO().MousePos.x - m_Drag->mouseX) * 1000. / m_PixelsPerSecond;
            auto& drag = *m_Drag;
            drag.startMovie = drag.originalStart; drag.endMovie = drag.originalEnd;
            const double minimumSpan = (std::min)(.1, drag.originalEnd - drag.originalStart);
            if (drag.gesture == Client::CLASS_MOVIE_TIMING_EDIT::MOVE)
            {
                // Camera body motion chooses an insertion point, independent of
                // the moved cut's length. A long first cut can reach the last cut.
                const bool reorder = drag.before.kind == "Camera";
                const double limit = reorder ? m_Timeline->movieDurationMs - .1 : m_Timeline->movieDurationMs - (drag.originalEnd - drag.originalStart);
                drag.startMovie = std::clamp(drag.originalStart + delta, 0., (std::max)(0., limit));
                drag.endMovie = (std::min)(m_Timeline->movieDurationMs, drag.startMovie + drag.originalEnd - drag.originalStart);
            }
            else if (drag.gesture == Client::CLASS_MOVIE_TIMING_EDIT::TRIM_START)
                drag.startMovie = std::clamp(drag.originalStart + delta, 0., drag.originalEnd - minimumSpan);
            else drag.endMovie = std::clamp(drag.originalEnd + delta, drag.originalStart + minimumSpan, m_Timeline->movieDurationMs);
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                if (std::abs(delta) >= 3. * 1000. / m_PixelsPerSecond && m_Callbacks.mapTime)
                {
                    const double start = m_Callbacks.mapTime(drag.before.classId, drag.before.loop, drag.startMovie, true);
                    double end = m_Callbacks.mapTime(drag.before.classId, drag.before.loop, drag.endMovie, true);
                    // Visual tracks preserve source duration; Sound preserves its WAV duration.
                    if (drag.gesture == Client::CLASS_MOVIE_TIMING_EDIT::MOVE &&
                        drag.before.kind != "Camera" && drag.before.kind != "Sound")
                        end = start + m_Callbacks.mapTime(drag.before.classId, drag.before.loop, drag.originalEnd, true)
                            - m_Callbacks.mapTime(drag.before.classId, drag.before.loop, drag.originalStart, true);
                    if (m_Callbacks.editTiming(drag.before, start, end, drag.gesture, m_EditStatus))
                    { m_EditBox.reset(); m_RowDirty = false; }
                }
                m_Drag.reset();
            }
            else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) m_Drag.reset();
        }

        void Render_Details()
        {
            const Client::CLASS_MOVIE_TIMELINE_BOX* box = nullptr;
            if (m_Timeline) for (const auto& row : m_Timeline->rows)
                if (row.id == m_SelectedRow && row.kind == m_SelectedKind)
                    for (const auto& value : row.boxes) if (value.id == m_SelectedBox) box = &value;
            if (!box) { ImGui::TextWrapped("Select a timeline box to inspect its source and movie timing."); return; }
            ImGui::SeparatorText(m_SelectedKind.c_str());
            ImGui::TextWrapped("%s", box->label.c_str());
            ImGui::TextWrapped("Resource: %s", box->resource.c_str());
            ImGui::Text("Movie: %.3f - %.3f s", box->movieStartMs * .001, box->movieEndMs * .001);
            ImGui::Text("Source: %.3f - %.3f s", box->sourceStartMs * .001, box->sourceEndMs * .001);
            const bool canSeek = m_State.active && m_State.activeClassId == m_State.selectedClassId && m_Callbacks.seek;
            ImGui::BeginDisabled(!canSeek);
            if (ImGui::Button("Seek to box start")) Queue_Seek(m_ViewLoop, box->movieStartMs);
            ImGui::EndDisabled();
            if (m_SelectedKind == "Animation")
                ImGui::Text("Clip offset %.3f s / clip rate %.3fx", box->sourceOffsetMs * .001, box->playbackRate);
            if (m_SelectedKind == "Sound")
                ImGui::Text("Audio source in: %.3f s", box->sourceOffsetMs * .001);
            if (m_SelectedKind == "Time Control")
            {
                Render_RateControl();
                ImGui::Text("Source dilation: %.4fx", m_State.sourceRate);
                ImGui::Text("Effective speed: %.4fx", m_State.sourceRate * m_State.playbackRate);
                ImGui::TextWrapped("Movie speed multiplies the shared movie clock. Original time-dilation keys stay unchanged.");
            }
            Render_Authoring(*box);
            if (!box->camera) { ImGui::Text("Keys: %zu", box->keyMovieTimes.size()); return; }
            const auto& row = *box->camera;
            if (row.cue.Keyframes.empty()) return;
            ImGui::SeparatorText("Applied camera key");
            const int count = static_cast<int>(row.cue.Keyframes.size());
            m_CameraKey = (std::clamp)(m_CameraKey, 0, count - 1);
            const auto& key = row.cue.Keyframes[m_CameraKey];
            const double movieMs = box->keyMovieTimes[m_CameraKey];
            ImGui::Text("Key %d / %d | %s", m_CameraKey + 1, count, key.cutBefore ? "Cut" : "Continuous");
            ImGui::Text("Movie %.3f s / source %.3f s / box-local %.3f s", movieMs * .001,
                (row.startMs + key.iTimeMs) * .001, key.iTimeMs * .001);
            ImGui::Text("Eye (m): %.4f, %.4f, %.4f", key.vEye.x, key.vEye.y, key.vEye.z);
            ImGui::Text("Look at (m): %.4f, %.4f, %.4f", key.vLookAt.x, key.vLookAt.y, key.vLookAt.z);
            ImGui::Text("Saved %s FOV: %.4f deg", row.horizontalFov ? "horizontal" : "vertical", key.fFovYDegrees);
            ImGui::BeginDisabled(!canSeek);
            if (ImGui::Button("Seek to camera key")) Queue_Seek(m_ViewLoop, movieMs);
            ImGui::EndDisabled();
            const bool freeCamera = m_Callbacks.inspection.state &&
                m_Callbacks.inspection.state(m_State.selectedClassId, m_ViewLoop).freeCamera;
            ImGui::SeparatorText(freeCamera ? "Authored Movie camera sample (free view detached)" : "Applied camera sample");
            const auto& sample = m_State.cameraSample;
            if (!MatchesPlayback() || !sample.valid || sample.rowId != row.id)
            { ImGui::TextWrapped("This camera box is not currently applied. Seek to the box to compare."); return; }
            ImGui::Text("Movie %.3f s / sampled source %.3f s", sample.movieMs * .001, sample.sourceMs * .001);
            ImGui::Text("Eye (m): %.4f, %.4f, %.4f", sample.pose.vEye.x, sample.pose.vEye.y, sample.pose.vEye.z);
            ImGui::Text("Look at (m): %.4f, %.4f, %.4f", sample.pose.vLookAt.x, sample.pose.vLookAt.y, sample.pose.vLookAt.z);
            ImGui::Text("Up: %.4f, %.4f, %.4f", sample.up.x, sample.up.y, sample.up.z);
            ImGui::Text(freeCamera ? "Authored vertical FOV: %.4f deg | aspect %.4f" :
                "Applied vertical FOV: %.4f deg | aspect %.4f", sample.pose.fFovYDegrees, sample.aspect);
            ImGui::TextWrapped(freeCamera ?
                "These are the authored Movie camera values at this time. The detached view uses the actual Camera XYZ shown in Movie world models." :
                "Applied values come from the movie's camera submission after interpolation and FOV conversion.");
        }

        void Render_Authoring(const Client::CLASS_MOVIE_TIMELINE_BOX& box)
        {
            if (!m_Callbacks.editableBox || !m_OpenedAuthoring) return;
            const bool matches = m_EditBox && m_EditBox->classId == m_State.selectedClassId &&
                m_EditBox->loop == m_ViewLoop && m_EditBox->kind == m_SelectedKind && m_EditBox->boxId == box.id;
            if (!matches && !m_RowDirty)
            {
                Client::CLASS_MOVIE_AUTHORING_BOX edit;
                if (m_Callbacks.editableBox(m_State.selectedClassId, m_ViewLoop, m_SelectedKind, box.id, edit, m_EditStatus))
                { m_EditValue = edit.value; m_EditBox = std::move(edit); m_KeySelections.clear(); }
                else m_EditBox.reset();
            }
            if (!m_EditBox || (!matches && m_RowDirty)) return;
            ImGui::SeparatorText("Movie row editor");
            ImGui::TextWrapped(m_SelectedKind == "Camera" ?
                "Position and FOV edits preserve playback. Advanced box timing changes stop playback after full validation. Save movie keeps the changes." :
                "Times are source milliseconds; the timeline maps them through the movie clock. Apply validates the whole movie and stops playback.");
            if (m_SelectedKind == "Effect" && m_Callbacks.openEffectEditor)
            {
                if (ImGui::Button("Open Effect Editor")) m_OpenEffect = box.resource;
                ImGui::TextWrapped("The V1 editor keeps this movie's actors and animation. Play All previews its elements in the same movie.");
            }
            ImGui::BeginDisabled(!m_RowDirty);
            if (ImGui::Button(m_SelectedKind == "Camera" ? "Apply camera live" : "Apply row")) m_ApplyRequested = true;
            ImGui::SameLine();
            if (ImGui::Button("Revert row")) { m_EditValue = m_EditBox->value; m_RowDirty = false; }
            ImGui::EndDisabled();
            if (m_SelectedKind == "Camera")
            {
                if (ImGui::Button("Open Sequence Camera Tool")) m_CameraWindow = true;
                ImGui::TextWrapped("Edit the ordered camera cuts and keys in Sequence Camera Tool. Save movie includes pending key edits.");
            }
            if (m_SelectedKind == "Sound")
            {
                const auto* field = m_EditValue.Find("sourceStartMs");
                double sourceIn = field ? field->Get_Number() : 0.;
                if (ImGui::InputDouble("Audio source in (ms)", &sourceIn, 1., 100., "%.0f") && std::isfinite(sourceIn))
                {
                    auto fields = m_EditValue.Get_Object();
                    fields["sourceStartMs"] = Client::DATA_JSON_VALUE::Number(std::round(sourceIn));
                    m_EditValue = Client::DATA_JSON_VALUE::Object(std::move(fields), m_EditValue.Get_ObjectInsertionOrder());
                    m_RowDirty = true; m_FollowPlayback = false;
                }
                ImGui::TextWrapped("Source in skips the beginning of the audio without moving the box. The left edge trims audio; dragging the body only moves it.");
            }
            if (m_SelectedKind == "World Model") Render_ModelPosition();
            const bool showRaw = m_SelectedKind == "Camera" ? ImGui::CollapsingHeader("All camera row fields") :
                m_SelectedKind == "World Model" ? ImGui::CollapsingHeader("All model track fields") : true;
            if (showRaw)
                if (EditMovieValue("Row values", m_EditValue, m_KeySelections, "movie-row"))
                { m_RowDirty = true; m_FollowPlayback = false; }
        }

        void Render_ModelPosition()
        {
            const auto* source = m_EditValue.Find("keys");
            if (!source || !source->Is_Array() || source->Get_Array().empty()) return;
            const auto& keys = source->Get_Array();
            int& selected = m_KeySelections["movie-row/keys"];
            ImGui::SeparatorText("Model position");
            ImGui::Text("Editing %s only | %zu keys", m_ViewLoop ? "Loop" : "Intro", keys.size());
            ImGui::InputInt("Position key", &selected);
            selected = std::clamp(selected, 0, static_cast<int>(keys.size()) - 1);
            ImGui::SameLine();
            ImGui::BeginDisabled(!m_Callbacks.mapTime);
            if (ImGui::Button("Nearest key to cursor"))
            {
                const double clock = m_Scrubbing ? m_EditMs : MatchesPlayback() ? m_State.clockMs : m_EditMs;
                double distance = (std::numeric_limits<double>::max)();
                for (std::size_t i = 0u; i < keys.size(); ++i)
                    if (const auto* time = keys[i].Find("timeMs"); time && time->Is_Number())
                    {
                        const double movieMs = m_Callbacks.mapTime(m_State.selectedClassId, m_ViewLoop, time->Get_Number(), false);
                        const double candidate = std::abs(movieMs - clock);
                        if (std::isfinite(candidate) && candidate < distance) { distance = candidate; selected = static_cast<int>(i); }
                    }
            }
            ImGui::EndDisabled();
            const auto& key = keys[static_cast<std::size_t>(selected)];
            if (const auto* time = key.Find("timeMs"); time && time->Is_Number()) ImGui::Text("Source key: %.3f s", time->Get_Number() * .001);
            const auto* position = key.Find("positionOffset");
            if (!position || !position->Is_Array() || position->Get_Array().size() != 3u ||
                !std::all_of(position->Get_Array().begin(), position->Get_Array().end(), [](const auto& value) { return value.Is_Number(); })) return;
            std::array<double, 3> value{};
            for (std::size_t axis = 0u; axis < value.size(); ++axis) value[axis] = position->Get_Array()[axis].Get_Number();
            const auto stage = [&](const std::array<double, 3>& change, const bool all)
            {
                Client::DATA_JSON_VALUE candidate;
                if (!EditMovieModelPosition(m_EditValue, static_cast<std::size_t>(selected), change, all, candidate, m_EditStatus)) return false;
                m_EditValue = std::move(candidate); m_RowDirty = true; m_FollowPlayback = false; return true;
            };
            if (ImGui::InputScalarN("Position offset (m)", ImGuiDataType_Double, value.data(), 3, nullptr, nullptr, "%.6f")) (void)stage(value, false);
            ImGui::TextWrapped("Sequence-local position of this key, before the instance and anchor transform. World XYZ in the model inspector is read-only. Changing one key can blend back toward later keys.");
            ImGui::InputScalarN("Translate this phase (m)", ImGuiDataType_Double, m_ModelPositionDelta.data(), 3, nullptr, nullptr, "%.6f");
            if (ImGui::Button("Stage translation for all keys"))
                if (stage(m_ModelPositionDelta, true)) m_ModelPositionDelta = {};
            ImGui::TextWrapped("Moves every key in this Intro or Loop track by the same offset. The other phase and other models are unchanged. Apply row stops playback; Play or seek previews the result. Save movie and Publish keep World changes.");
        }

        void Render_CameraWindow()
        {
            if (!m_CameraWindow) return;
            ImGui::SetNextWindowSize({1000.f, 650.f}, ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Sequence Camera Tool###WorldMovieCamera", &m_CameraWindow)) { ImGui::End(); return; }
            ImGui::Text("%s / %s", m_State.selectedLabel.c_str(), m_ViewLoop ? "Loop" : "Intro");
            ImGui::BeginDisabled(!m_OpenedAuthoring || m_State.authoringPublishPending);
            if (ImGui::Button("Save")) m_SaveRequested = true;
            ImGui::SameLine();
            ImGui::BeginDisabled(m_RowDirty || m_State.authoringDirty || !m_Callbacks.publishAuthoring);
            if (ImGui::Button("Publish")) m_PublishRequested = true;
            ImGui::EndDisabled();
            ImGui::EndDisabled();
            ImGui::SameLine(); ImGui::Checkbox("Live preview", &m_LiveCamera);
            ImGui::TextWrapped("Camera keys are saved for Client playback. Publish applies saved World changes; server action timing is unchanged.");
            if (!m_EditStatus.empty()) ImGui::TextWrapped("%s", m_EditStatus.c_str());
            if (m_RowDirty && Row_SourceChanged() && ImGui::Button("Discard pending row and reopen"))
            { m_RowDirty = false; m_EditBox.reset(); m_CameraEditor = {}; m_EditStatus.clear(); }
            if (!m_State.authoringStatus.empty()) ImGui::TextWrapped("%s", m_State.authoringStatus.c_str());
            m_Timeline = m_Callbacks.timeline ? m_Callbacks.timeline(m_State.selectedClassId, m_ViewLoop) : nullptr;
            ImGui::BeginChild("CameraCuts", {260.f, 0.f}, ImGuiChildFlags_Borders);
            if (m_Timeline) for (const auto& row : m_Timeline->rows) if (row.kind == "Camera")
                for (size_t i = 0; i < row.boxes.size(); ++i)
                {
                    const auto& box = row.boxes[i];
                    ImGui::PushID(box.id.c_str());
                    const std::string label = std::to_string(i + 1u) + ". " + box.label;
                    if (ImGui::Selectable(label.c_str(), m_SelectedKind == "Camera" && m_SelectedBox == box.id)) Select_Box(row, box);
                    ImGui::TextDisabled("%.3f - %.3f s", box.movieStartMs * .001, box.movieEndMs * .001);
                    ImGui::PopID();
                }
            ImGui::EndChild(); ImGui::SameLine();
            ImGui::BeginChild("CameraKeys", {0.f, 0.f}, ImGuiChildFlags_Borders);
            if (m_SelectedKind != "Camera") ImGui::TextUnformatted("Select a camera cut on the left.");
            else
            {
                if (!m_EditBox && m_Callbacks.editableBox)
                {
                    Client::CLASS_MOVIE_AUTHORING_BOX box;
                    if (m_Callbacks.editableBox(m_State.selectedClassId, m_ViewLoop, "Camera", m_SelectedBox, box, m_EditStatus))
                    { m_EditValue = box.value; m_EditBox = std::move(box); }
                }
                if (m_EditBox && m_EditBox->kind == "Camera")
                {
                    using Json = Client::DATA_JSON_VALUE;
                    std::vector<Client::EFFECT_CAMERA_ROW> rows;
                    std::string error;
                    if (Client::CEffectRecoveryCamera::Parse(Json::Object({{"cameras", Json::Array({m_EditValue})}}), true, rows, error))
                    {
                        auto& row = rows.front();
                        double sourceCursor = MatchesPlayback() ? m_State.sourceClockMs :
                            m_Callbacks.mapTime ? m_Callbacks.mapTime(m_State.selectedClassId, m_ViewLoop, m_EditMs, true) : 0.;
                        const auto local = static_cast<uint32_t>(std::clamp(sourceCursor - row.startMs, 0., double(row.cue.iDurationMs)));
                        const auto result = Client::CSequenceCameraEditor::Render(row, m_CameraEditor, local, m_Callbacks.captureFreeCamera);
                        if (result.changed)
                        {
                            m_EditValue = Client::CEffectRecoveryCamera::Write_Row(row, &m_EditValue);
                            m_RowDirty = true; m_FollowPlayback = false;
                            if (m_LiveCamera) m_ApplyRequested = true;
                        }
                        for (size_t i = 0; i < row.cue.Keyframes.size(); ++i)
                            if (row.cue.Keyframes[i].strSceneId == m_CameraEditor.selectedKeyId) m_CameraKey = static_cast<int>(i);
                        if (result.seekLocalMs && m_Callbacks.mapTime)
                        {
                            const double movieMs = m_Callbacks.mapTime(m_State.selectedClassId, m_ViewLoop, row.startMs + *result.seekLocalMs, false);
                            if (m_RowDirty) m_ApplyRequested = true;
                            Queue_Seek(m_ViewLoop, movieMs);
                        }
                        ImGui::BeginDisabled(!m_RowDirty);
                        if (ImGui::Button("Apply camera")) m_ApplyRequested = true;
                        ImGui::SameLine();
                        if (ImGui::Button("Revert pending key edits")) { m_EditValue = m_EditBox->value; m_RowDirty = false; }
                        ImGui::EndDisabled();
                    }
                    else
                    {
                        ImGui::TextWrapped("%s", error.c_str());
                        ImGui::BeginDisabled(!m_RowDirty);
                        if (ImGui::Button("Revert invalid camera row")) { m_EditValue = m_EditBox->value; m_RowDirty = false; }
                        ImGui::EndDisabled();
                    }
                }
            }
            ImGui::EndChild(); ImGui::End();
        }

        struct DRAG
        {
            Client::CLASS_MOVIE_AUTHORING_BOX before;
            Client::CLASS_MOVIE_TIMING_EDIT gesture;
            float mouseX;
            double originalStart, originalEnd, startMovie, endMovie;
        };
        std::optional<DRAG> m_Drag;
        Client::SEQUENCE_CAMERA_EDITOR_STATE m_CameraEditor;
        bool m_CameraWindow = false, m_ActiveOnly = false, m_PublishRequested = false;
        CALLBACKS m_Callbacks;
        STATE m_State;
        COMMAND m_Pending = COMMAND::NONE;
        std::uint64_t m_OwnerToken = 0;
        bool m_OwnsPlayback = false;
        bool m_RequestedPause = false;
        bool m_Scrubbing = false;
        bool m_ScrubLoop = false;
        bool m_SeekLoop = false;
        float m_EditMs = 0.f;
        double m_SeekMs = 0.;
        bool m_ViewLoop = false, m_FollowPlayback = true;
        float m_PixelsPerSecond = 45.f;
        bool m_TimelineFitRequested = true;
        int m_CameraKey = 0;
        bool m_LiveCamera = true;
        ImGuiTextFilter m_RowFilter;
        std::optional<double> m_RequestedRate;
        struct REPEAT_MOVIE_REQUEST { std::string classId; bool repeat; };
        std::optional<REPEAT_MOVIE_REQUEST> m_RequestedRepeatMovie;
        std::shared_ptr<const Client::CLASS_MOVIE_TIMELINE> m_Timeline;
        std::string m_SelectedKind, m_SelectedRow, m_SelectedBox;
        Client::CClassMovieInspector m_WorldInspector;
        Client::CLASS_MOVIE_INSPECTION_STATE m_Inspection;
        bool m_VisibilityWindow = false;
        std::string m_InspectedWorldId;
        std::string m_ObservedWorldClass, m_ObservedWorldId, m_PendingWorldSelection;
        std::uint64_t m_ObservedWorldGeneration = 0u;
        bool m_ObservedWorldLoop = false;
        std::array<double, 3> m_ModelPositionDelta{};
        bool m_OpenedAuthoring = false, m_RowDirty = false;
        bool m_ApplyRequested = false, m_SaveRequested = false, m_ReloadRequested = false;
        std::optional<Client::CLASS_MOVIE_AUTHORING_BOX> m_EditBox;
        Client::DATA_JSON_VALUE m_EditValue;
        std::map<std::string, int> m_KeySelections;
        std::string m_EditStatus, m_OpenEffect, m_DeleteWorldItem;
    };

    const char* PaneLabel(const PANE pane, const bool sequenceWorkspace)
    {
        if (sequenceWorkspace && pane == PANE::PATTERNS)
            return "Sequences";
        if (sequenceWorkspace && pane == PANE::BOSS_PATTERN)
            return "Boss Sequences";
        if (sequenceWorkspace && pane == PANE::TOOLBAR)
            return "Sequencer Benchmark";
        switch (pane)
        {
        case PANE::SEQUENCER: return "Sequencer";
        case PANE::PATTERNS: return "Actions";
        case PANE::RESOURCES: return "Resources";
        case PANE::DETAILS: return "Box Detail";
        case PANE::PREVIEW: return "Preview";
        case PANE::BOSS_PATTERN: return "Boss Pattern";
        case PANE::TOOLBAR: return "Action Workbench";
        default: return "Unavailable pane";
        }
    }

    bool SameAnimationResource(const Client::COMPOSITION_ANIMATION_RESOURCE& left,
        const Client::COMPOSITION_ANIMATION_RESOURCE& right)
    {
        return left.strTargetAssetName == right.strTargetAssetName &&
            left.strModelAssetId == right.strModelAssetId &&
            left.strSourceAssetId == right.strSourceAssetId &&
            left.strRuntimeClip == right.strRuntimeClip;
    }

    bool ResourceTextMatches(const std::string_view text, const std::string_view query)
    {
        return query.empty() || std::search(text.begin(), text.end(), query.begin(), query.end(),
            [](const unsigned char left, const unsigned char right) {
                return std::tolower(left) == std::tolower(right);
            }) != text.end();
    }

    const char* PaneWindowId(const PANE pane, const bool sequenceWorkspace)
    {
        if (sequenceWorkspace)
        {
            switch (pane)
            {
            case PANE::SEQUENCER: return "Composition Sequencer###SequenceBenchmarkSequencerWindow";
            case PANE::PATTERNS: return "Composition Sequencer###SequenceBenchmarkSequencesWindow";
            case PANE::RESOURCES: return "Composition Resources###SequenceBenchmarkResourcesWindow";
            case PANE::DETAILS: return "Box Detail###SequenceBenchmarkDetailsWindow";
            case PANE::PREVIEW: return "Composition Preview###SequenceBenchmarkPreviewWindow";
            case PANE::BOSS_PATTERN: return "Composition Sequences###SequenceBenchmarkBossPatternWindow";
            case PANE::TOOLBAR: return "Sequencer Benchmark###SequenceBenchmarkSessionWindow";
            default: return "Unavailable pane###SequenceBenchmarkUnavailableWindow";
            }
        }
        switch (pane)
        {
        case PANE::SEQUENCER:
            return "Composition Sequencer###CompositionSequencerWindowResizableV3";
        case PANE::PATTERNS:
            return "Composition Actions###CompositionPatternsWindow";
        case PANE::RESOURCES:
            return "Composition Resources###CompositionResourcesWindowResizableV2";
        case PANE::DETAILS:
            return "Box Detail###CompositionDetailsWindow";
        case PANE::PREVIEW:
            return "Composition Preview###CompositionPreviewWindow";
        case PANE::BOSS_PATTERN:
            return "Boss Pattern###CompositionBossPatternWindow";
        case PANE::TOOLBAR:
            return "Action Workbench###CompositionSessionWindow";
        default: return "Unavailable pane###CompositionUnavailablePane";
        }
    }

    struct PANE_PLACEMENT final
    {
        ImVec2 position;
        ImVec2 size;
    };

    PANE_PLACEMENT PanePlacement(const PANE pane)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 origin = nullptr == viewport ? ImVec2(20.f, 20.f) : viewport->WorkPos;
        const ImVec2 available = nullptr == viewport ? ImVec2(1600.f, 900.f) : viewport->WorkSize;
        constexpr float margin = 8.f;
        constexpr float gap = 8.f;
        const float contentWidth = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
        const float contentHeight = (std::max)(1.f, available.y - margin * 2.f);
        const float leftWidth = contentWidth * 0.18f;
        const float rightWidth = contentWidth * 0.20f;
        const float centerWidth = contentWidth - leftWidth - rightWidth;
        const float leftTopHeight = contentHeight * 0.58f;
        const float previewHeight = contentHeight * 0.30f;
        const float toolbarHeight = contentHeight * 0.18f;
        const float sequencerHeight = (std::max)(1.f, contentHeight - previewHeight - toolbarHeight - gap * 2.f);
        const float leftX = origin.x + margin;
        const float centerX = leftX + leftWidth + gap;
        const float rightX = centerX + centerWidth + gap;
        const float topY = origin.y + margin;
        switch (pane)
        {
        case PANE::PATTERNS:
            return { { leftX, topY }, { leftWidth, leftTopHeight } };
        case PANE::RESOURCES:
            return { { leftX, topY + leftTopHeight + gap },
                { leftWidth, (std::max)(1.f, contentHeight - leftTopHeight - gap) } };
        case PANE::DETAILS:
            return { { rightX, topY }, { rightWidth, contentHeight } };
        case PANE::PREVIEW:
            return { { centerX, topY }, { centerWidth, previewHeight } };
        case PANE::SEQUENCER:
            return { { centerX, topY + previewHeight + gap }, { centerWidth, sequencerHeight } };
        case PANE::TOOLBAR:
            return { { centerX, topY + previewHeight + sequencerHeight + gap * 2.f },
                { centerWidth, toolbarHeight } };
        case PANE::BOSS_PATTERN:
            return { { centerX + 24.f, topY + 24.f }, { centerWidth, contentHeight * 0.65f } };
        default: return { origin, { 320.f, 200.f } };
        }
    }
}

Client::CSequencerTool::CSequencerTool(
    ICompositionWorkbenchSession* pValtanSession,
    ICompositionWorkbenchSession* pKoukuSaydonSession,
    const bool sequenceWorkspace)
    : m_pValtanSession(pValtanSession)
    , m_pKoukuSaydonSession(pKoukuSaydonSession)
    , m_eSelectedBoss(sequenceWorkspace ? BOSS::KOUKU_SAYDON : BOSS::VALTAN)
    , m_bSequenceWorkspace(sequenceWorkspace)
{
    m_pClassSelectionSession = std::make_unique<CClassSelectionWorkbenchSession>();
}

void Client::CSequencerTool::Set_ActionSessions(ICompositionWorkbenchSession* character,
    ICompositionWorkbenchSession* object, ICompositionWorkbenchSession* sequence)
{
    m_pCharacterSession = character;
    m_pObjectSession = object;
    m_pSequenceSession = sequence;
}

void Client::CSequencerTool::Set_TargetChangedCallback(std::function<void(TARGET)> callback)
{
    m_TargetChanged = std::move(callback);
}

void Client::CSequencerTool::Set_ClassSelectionPreviewCallbacks(CLASS_SELECTION_PREVIEW_CALLBACKS callbacks)
{
    if (m_pClassSelectionSession) m_pClassSelectionSession->On_WorkbenchDeactivated();
    m_pClassSelectionSession = std::make_unique<CClassSelectionWorkbenchSession>(std::move(callbacks));
}

void Client::CSequencerTool::Select_Target(const TARGET target)
{
    if (target == m_eSelectedTarget) return;
    if (auto* previous = Selected_Session()) previous->On_WorkbenchDeactivated();
    m_eSelectedTarget = target;
    m_bAnimationPreviewPending = false;
    m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
    m_AnimationPreviewState = {};
    if (m_TargetChanged) m_TargetChanged(target);
    if (target == TARGET::BOSS || target == TARGET::SEQUENCE || target == TARGET::OBJECT)
        if (auto* session = Selected_Session())
            session->Select_WorkbenchBoss(Get_SelectedBoss());
}

void Client::CSequencerTool::Open(const TARGET target)
{
    if (m_bInsideFrame)
    {
        m_ePendingTarget = target;
        m_bTargetChangePending = true;
        Open();
        return;
    }
    m_bTargetChangePending = false;
    Select_Target(target);
    Open();
}

void Client::CSequencerTool::Open(const TARGET target, const BOSS boss)
{
    if (m_bInsideFrame)
    {
        m_ePendingTarget = target; m_bTargetChangePending = true;
        m_ePendingBoss = boss; m_bBossChangePending = true;
        Open();
        return;
    }
    m_bTargetChangePending = m_bBossChangePending = false;
    if (target != m_eSelectedTarget)
    {
        // Stage the requested gate before entering the session. Never briefly
        // select the previous gate and discard an exact typed deep-link selection.
        if (target == TARGET::SEQUENCE) m_eSequenceBoss = boss;
        if (target == TARGET::BOSS || target == TARGET::OBJECT) m_eSelectedBoss = boss;
        Select_Target(target);
    }
    else Select_Boss(boss);
    Open();
}

void Client::CSequencerTool::Deactivate()
{
    if (auto* session = Selected_Session()) session->On_WorkbenchDeactivated();
    m_bAnimationPreviewPending = false;
    m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
}

void Client::CSequencerTool::Open()
{
    m_bOpen = true;
    m_bSequencerMaximized = false;
    m_PaneVisible[static_cast<std::size_t>(PANE::PATTERNS)] = true;
    m_PaneVisible[static_cast<std::size_t>(PANE::RESOURCES)] = true;
    m_PaneVisible[static_cast<std::size_t>(PANE::DETAILS)] = true;
    m_bRestoreAuthoringPanesRequested = true;
}

void Client::CSequencerTool::Open(const COMPOSITION_WORKBENCH_BOSS boss)
{
    Open(TARGET::BOSS, boss);
}

void Client::CSequencerTool::Select_Boss(const COMPOSITION_WORKBENCH_BOSS boss)
{
    auto& selectedBoss = m_eSelectedTarget == TARGET::SEQUENCE ? m_eSequenceBoss : m_eSelectedBoss;
    if (selectedBoss != boss)
    {
        if (auto* previous = Selected_Session()) previous->On_WorkbenchDeactivated();
        selectedBoss = boss;
        if (m_TargetChanged) m_TargetChanged(m_eSelectedTarget);
    }
    if (ICompositionWorkbenchSession* session = Selected_Session())
        session->Select_WorkbenchBoss(boss);
}

Client::ICompositionWorkbenchSession* Client::CSequencerTool::Selected_Session() const noexcept
{
    switch (m_eSelectedTarget)
    {
    case TARGET::CHARACTER: return m_pCharacterSession;
    case TARGET::OBJECT: return m_eSelectedBoss == BOSS::VALTAN ? m_pValtanSession : m_pObjectSession;
    case TARGET::WORLD: return m_pClassSelectionSession.get();
    case TARGET::SEQUENCE:
        return m_eSequenceBoss == BOSS::VALTAN ? m_pValtanSession : m_pSequenceSession;
    case TARGET::BOSS: break;
    default: return nullptr;
    }
    switch (m_eSelectedBoss)
    {
    case BOSS::VALTAN: return m_pValtanSession;
    case BOSS::KOUKU_SAYDON:
    case BOSS::KOUKU_SAYDON_GATE2:
    case BOSS::KOUKU_SAYDON_GATE3:
    case BOSS::KOUKU_SAYDON_ENCORE: return m_pKoukuSaydonSession;
    default: return nullptr;
    }
}

void Client::CSequencerTool::Set_AnimationResources(
    std::vector<COMPOSITION_ANIMATION_RESOURCE> resources, std::string status)
{
    // The reader isolates missing packages and retains their last rows. Accept
    // that partial snapshot so healthy models remain available after a failure.
    if (!resources.empty())
        m_AnimationResources = std::move(resources);
    m_strAnimationResourceStatus = std::move(status);
    m_bAnimationResourceTreeDirty = true;
    if (m_bHasSelectedAnimationResource)
    {
        const auto selected = std::find_if(m_AnimationResources.begin(), m_AnimationResources.end(),
            [this](const auto& resource) { return SameAnimationResource(resource, m_SelectedAnimationResource); });
        if (selected != m_AnimationResources.end())
            m_SelectedAnimationResource = *selected;
        else
        {
            m_bHasSelectedAnimationResource = false;
            m_SelectedAnimationResource = {};
        }
    }
}

bool Client::CSequencerTool::Consume_ResourceRefreshRequest()
{
    const bool requested = m_bResourceRefreshRequested;
    m_bResourceRefreshRequested = false;
    return requested;
}

bool Client::CSequencerTool::Consume_AnimationPreviewRequest(COMPOSITION_ANIMATION_RESOURCE& resource)
{
    if (!m_bAnimationPreviewPending)
        return false;
    resource = std::move(m_PendingAnimationPreview);
    m_bAnimationPreviewPending = false;
    return true;
}

void Client::CSequencerTool::Set_AnimationPreviewStatus(std::string status)
{
    m_strAnimationBrowserStatus = std::move(status);
}

bool Client::CSequencerTool::Consume_AnimationPreviewTransportRequest(
    ANIMATION_PREVIEW_TRANSPORT& transport)
{
    transport = m_eAnimationPreviewTransport;
    m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
    return transport != ANIMATION_PREVIEW_TRANSPORT::NONE;
}

void Client::CSequencerTool::Set_AnimationPreviewState(ANIMATION_PREVIEW_STATE state)
{
    m_AnimationPreviewState = std::move(state);
}

void Client::CSequencerTool::Queue_AnimationPreview(const COMPOSITION_ANIMATION_RESOURCE& resource)
{
    m_PendingAnimationPreview = resource;
    m_bAnimationPreviewPending = true;
    m_strAnimationBrowserStatus = "Preview requested: " + resource.strRuntimeClip;
}

void Client::CSequencerTool::Render_PhysicalAnimationBrowser(ICompositionWorkbenchSession& session)
{
    if (!ImGui::CollapsingHeader("Animation Library##CompositionPhysicalAnimation",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    if (ImGui::Button("Refresh Animation Resources##CompositionPhysicalAnimation"))
        m_bResourceRefreshRequested = true;
    ImGui::SameLine();
    ImGui::TextDisabled("%zu models / %zu clips",
        COMPOSITION_ANIMATION_TARGET_ASSET_NAMES.size(), m_AnimationResources.size());
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##CompositionPhysicalAnimationSearch", "Search character, designer name or clip...",
        m_AnimationResourceSearch.data(), m_AnimationResourceSearch.size())) m_bAnimationResourceTreeDirty = true;
    if (m_bAnimationResourceTreeDirty || m_AnimationResourceQuery != m_AnimationResourceSearch.data())
    {
        m_AnimationResourceQuery = m_AnimationResourceSearch.data();
        m_AnimationResourceTree = {};
        for (std::size_t index = 0u; index < m_AnimationResources.size(); ++index)
        {
            const auto& resource = m_AnimationResources[index];
            const auto category = CompositionAnimationCategory(resource.strTargetAssetName);
            const auto& query = m_AnimationResourceQuery;
            if (!ResourceTextMatches(resource.strTargetAssetName, query) &&
                !ResourceTextMatches(resource.strRuntimeClip, query) &&
                !ResourceTextMatches(resource.strDisplayName, query) &&
                !ResourceTextMatches(resource.strSourceAssetId, query) &&
                std::none_of(category.begin(), category.end(), [&](const auto& name) { return ResourceTextMatches(name, query); })) continue;
            InsertResourceTree(m_AnimationResourceTree, category, index);
        }
        FinalizeResourceTree(m_AnimationResourceTree);
        m_bAnimationResourceTreeDirty = false;
    }
    if (ImGui::BeginChild("##CompositionPhysicalAnimationTree", ImVec2(0.f, 260.f), true))
    {
        m_bPhysicalAnimationFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
        RenderResourceTree(m_AnimationResourceTree, [&](const std::size_t index)
        {
            const auto& resource = m_AnimationResources[index];
            ImGui::PushID(resource.strTargetAssetName.c_str());
            ImGui::PushID(resource.strSourceAssetId.c_str());
            ImGui::PushID(resource.strRuntimeClip.c_str());
            const bool selected = m_bHasSelectedAnimationResource &&
                SameAnimationResource(resource, m_SelectedAnimationResource);
            const auto& label = resource.strDisplayName.empty() ? resource.strRuntimeClip : resource.strDisplayName;
            if (ImGui::Selectable(label.c_str(), selected))
            {
                m_SelectedAnimationResource = resource;
                m_bHasSelectedAnimationResource = true;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s\nModel: %s\nPackage: %s\nNative: %u ms\nDouble-click to preview",
                    resource.strRuntimeClip.c_str(), resource.strModelAssetId.c_str(),
                    resource.strSourceAssetId.c_str(), resource.iDurationMs);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) Queue_AnimationPreview(resource);
            }
            Offer_CompositionResourceDrag(label.c_str(), [&resource, &label]() -> COMPOSITION_TRANSFER
            {
                auto transfer = std::make_shared<COMPOSITION_ANIMATION_TRANSFER>();
                transfer->resource = resource; transfer->label = label;
                return transfer;
            });
            ImGui::PopID(); ImGui::PopID(); ImGui::PopID();
        });
        if (m_AnimationResourceTree.iRecursiveLeafCount == 0u)
            ImGui::TextDisabled("No clips match. Refresh reads every installed Character and Boss model.");
    }
    ImGui::EndChild();
    if (m_bHasSelectedAnimationResource)
    {
        const auto& resource = m_SelectedAnimationResource;
        ImGui::TextWrapped("%s / %s | %u ms", resource.strTargetAssetName.c_str(),
            resource.strRuntimeClip.c_str(), resource.iDurationMs);
        if (ImGui::Button("Play Preview##CompositionPhysicalAnimation"))
            Queue_AnimationPreview(resource);
        ImGui::SameLine();
        if (ImGui::Button("Copy Resource##CompositionPhysicalAnimation"))
        {
            auto transfer = std::make_shared<COMPOSITION_ANIMATION_TRANSFER>();
            transfer->resource = resource;
            transfer->label = resource.strDisplayName.empty() ? resource.strRuntimeClip : resource.strDisplayName;
            CCompositionClipboard::Get().Write(std::move(transfer));
            m_CompositionEditStatus = "Animation resource copied.";
        }
    }
    if (m_bHasSelectedAnimationResource || m_AnimationPreviewState.bPlaying)
    {
        ImGui::BeginDisabled(!m_AnimationPreviewState.bPlaying);
        if (ImGui::Button(m_AnimationPreviewState.bPaused ?
            "Resume##CompositionPhysicalAnimation" : "Pause##CompositionPhysicalAnimation"))
            m_eAnimationPreviewTransport = m_AnimationPreviewState.bPaused ?
                ANIMATION_PREVIEW_TRANSPORT::RESUME : ANIMATION_PREVIEW_TRANSPORT::PAUSE;
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Stop##CompositionPhysicalAnimation"))
            m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::STOP;
        if (!m_AnimationPreviewState.strPatternId.empty())
            ImGui::TextWrapped("%s: %.3f / %.3f s%s", m_AnimationPreviewState.strPatternId.c_str(),
                m_AnimationPreviewState.iClockMs / 1000.0, m_AnimationPreviewState.iDurationMs / 1000.0,
                m_AnimationPreviewState.bPaused ? " (paused)" : "");
        if (!m_AnimationPreviewState.strStatus.empty())
            ImGui::TextWrapped("%s", m_AnimationPreviewState.strStatus.c_str());
    }
    if (m_bHasSelectedAnimationResource)
    {
        const auto& resource = m_SelectedAnimationResource;
        std::string stageStatus;
        std::string rowStatus;
        const bool canStage = session.Can_AppendCompositionAnimationResource(resource, true, stageStatus);
        const bool canRow = session.Can_AppendCompositionAnimationResource(resource, false, rowStatus);
        const bool characterAction = m_eSelectedTarget == TARGET::CHARACTER;
        const bool valtanPattern = &session == m_pValtanSession;
        const char* stageLabel = characterAction ? "Replace selected Animation" :
            (valtanPattern ? "Append to Pattern End" : "Append as Stage");
        const char* rowLabel = characterAction ? "Append Animation" : "Add Animation Row";
        ImGui::BeginDisabled(!canStage);
        if (ImGui::Button((std::string(stageLabel) + "##CompositionPhysicalAnimation").c_str()))
            (void)session.Append_CompositionAnimationResource(resource, true, m_strAnimationBrowserStatus);
        ImGui::EndDisabled();
        if (!valtanPattern)
        {
            ImGui::BeginDisabled(!canRow);
            if (ImGui::Button((std::string(rowLabel) + "##CompositionPhysicalAnimation").c_str()))
                (void)session.Append_CompositionAnimationResource(resource, false, m_strAnimationBrowserStatus);
            ImGui::EndDisabled();
        }
        if (!canStage && !stageStatus.empty())
            ImGui::TextWrapped("%s: %s", stageLabel, stageStatus.c_str());
        if (!canRow && !rowStatus.empty() && rowStatus != stageStatus)
            ImGui::TextWrapped("%s: %s", rowLabel, rowStatus.c_str());
    }
    if (!m_strAnimationBrowserStatus.empty())
        ImGui::TextWrapped("%s", m_strAnimationBrowserStatus.c_str());
    if (!m_strAnimationResourceStatus.empty() && ImGui::TreeNode("Resource status##CompositionPhysicalAnimation"))
    {
        ImGui::TextWrapped("%s", m_strAnimationResourceStatus.c_str());
        ImGui::TreePop();
    }
    ImGui::Separator();
}

void Client::CSequencerTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar())
        return;
    if (ImGui::BeginMenu("Windows"))
    {
        for (const PANE pane : PANES)
            ImGui::MenuItem(PaneLabel(pane, m_bSequenceWorkspace), nullptr, &m_PaneVisible[static_cast<std::size_t>(pane)]);
        ImGui::MenuItem("Physical Animation Browser", nullptr, &m_bPhysicalAnimationBrowserVisible);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All"))
            m_PaneVisible.fill(true);
        if (ImGui::MenuItem("Expand Resources"))
        {
            m_PaneVisible[static_cast<std::size_t>(PANE::RESOURCES)] = true;
            m_bExpandResourcesRequested = m_bFocusResourcesRequested = true;
            m_bSequencerMaximized = false;
        }
        ImGui::MenuItem("Maximize Sequencer", nullptr, &m_bSequencerMaximized);
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_bResetLayoutRequested = true;
            m_bSequencerMaximized = false;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void Client::CSequencerTool::Render_ActionSelector()
{
    ImGui::SeparatorText("Composition Actions");
    constexpr std::array<const char*, 5> labels = { "Boss", "Character", "Object", "Sequence", "World" };
    for (std::size_t i = 0; i < labels.size(); ++i)
    {
        const auto target = static_cast<TARGET>(i);
        if (ImGui::Selectable(labels[i], m_eSelectedTarget == target))
        {
            m_ePendingTarget = target;
            m_bTargetChangePending = target != m_eSelectedTarget;
        }
    }
    ImGui::Separator();
    if (m_eSelectedTarget == TARGET::BOSS || m_eSelectedTarget == TARGET::SEQUENCE || m_eSelectedTarget == TARGET::OBJECT)
        Render_BossSelector();
}

void Client::CSequencerTool::Render_BossSelector()
{
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::BeginCombo("Boss##CompositionWorkbenchBoss", BossLabel(Get_SelectedBoss())))
    {
        for (const BOSS boss : BOSS_ENTRIES)
        {
            const bool selected = boss == Get_SelectedBoss();
            if (ImGui::Selectable(BossLabel(boss), selected))
            {
                m_ePendingBoss = boss;
                m_bBossChangePending = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void Client::CSequencerTool::Apply_ViewRequest(ICompositionWorkbenchSession& session)
{
    const COMPOSITION_WORKBENCH_VIEW_REQUEST request = session.Consume_WorkbenchViewRequest();
    if (request.showResources || request.focusResources || request.expandResources)
        m_PaneVisible[static_cast<std::size_t>(PANE::RESOURCES)] = true;
    if (request.showPatterns || request.focusPatterns)
        m_PaneVisible[static_cast<std::size_t>(PANE::PATTERNS)] = true;
    m_bFocusResourcesRequested |= request.focusResources;
    m_bExpandResourcesRequested |= request.expandResources;
    m_bFocusPatternsRequested |= request.focusPatterns;
    if (request.maximizeSequencer)
    {
        m_bSequencerMaximized = true;
        m_PaneVisible[static_cast<std::size_t>(PANE::SEQUENCER)] = true;
    }
    if (request.restoreSequencer) m_bSequencerMaximized = false;
    m_bResetLayoutRequested |= request.resetLayout;
}

void Client::CSequencerTool::Render_Pane(
    ICompositionWorkbenchSession& session, const PANE pane)
{
    bool& visible = m_PaneVisible[static_cast<std::size_t>(pane)];
    if (!visible)
        return;
    const PANE_PLACEMENT placement = PanePlacement(pane);
    const ImGuiCond condition = m_bApplyResetLayoutThisFrame ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(placement.position, condition);
    ImGui::SetNextWindowSize(placement.size, condition);
    if (m_bRestoreAuthoringPanesRequested &&
        (pane == PANE::PATTERNS || pane == PANE::RESOURCES || pane == PANE::DETAILS))
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (pane == PANE::RESOURCES && m_bExpandResourcesRequested && nullptr != viewport)
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.08f,
            viewport->WorkPos.y + viewport->WorkSize.y * 0.12f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.62f,
            viewport->WorkSize.y * 0.72f), ImGuiCond_Always);
    }
    if ((pane == PANE::RESOURCES && m_bFocusResourcesRequested) ||
        (pane == PANE::PATTERNS && m_bFocusPatternsRequested))
    {
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
        ImGui::SetNextWindowFocus();
    }
    if (pane == PANE::SEQUENCER && m_bSequencerMaximized && nullptr != viewport)
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.02f,
            viewport->WorkPos.y + viewport->WorkSize.y * 0.02f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.96f,
            viewport->WorkSize.y * 0.96f), ImGuiCond_Always);
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    }
    const bool expanded = ImGui::Begin(PaneWindowId(pane, m_bSequenceWorkspace), &visible, ImGuiWindowFlags_MenuBar);
    Render_WindowMenu();
    if (expanded)
    {
        if (pane == PANE::PATTERNS) Render_ActionSelector();
        if (pane == PANE::RESOURCES && m_bPhysicalAnimationBrowserVisible && m_eSelectedTarget != TARGET::WORLD)
            Render_PhysicalAnimationBrowser(session);
        const bool hasBossOwner = m_eSelectedTarget == TARGET::BOSS || m_eSelectedTarget == TARGET::SEQUENCE ||
            m_eSelectedTarget == TARGET::OBJECT;
        ImGui::PushID(static_cast<int>(m_eSelectedTarget) * 16 +
            (hasBossOwner ? static_cast<int>(Get_SelectedBoss()) : 0));
        session.Render_WorkbenchPane(pane);
        ImGui::PopID();
        if (pane == PANE::SEQUENCER || pane == PANE::RESOURCES ||
            pane == PANE::PATTERNS || pane == PANE::DETAILS)
            m_bCompositionEditFocused |= ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        if (pane == PANE::SEQUENCER || pane == PANE::DETAILS)
            if (auto transfer = Accept_CompositionResourceDropInWindow())
                m_PendingCompositionTransfer = std::move(transfer);
    }
    ImGui::End();
    if (pane == PANE::RESOURCES)
        m_bExpandResourcesRequested = m_bFocusResourcesRequested = false;
    if (pane == PANE::PATTERNS) m_bFocusPatternsRequested = false;
    Apply_ViewRequest(session);
}

void Client::CSequencerTool::Render()
{
    if (!m_bOpen)
        return;
    if (m_bTargetChangePending)
    {
        if (m_bBossChangePending) Open(m_ePendingTarget, m_ePendingBoss);
        else { m_bTargetChangePending = false; Select_Target(m_ePendingTarget); }
    }
    if (m_bBossChangePending)
    {
        m_bBossChangePending = false;
        Select_Boss(m_ePendingBoss);
    }
    m_bApplyResetLayoutThisFrame = m_bResetLayoutRequested;
    m_bResetLayoutRequested = false;
    const PANE_PLACEMENT placement = PanePlacement(PANE::TOOLBAR);
    const ImGuiCond condition = m_bApplyResetLayoutThisFrame ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(placement.position, condition);
    ImGui::SetNextWindowSize(placement.size, condition);
    if (m_bRestoreAuthoringPanesRequested)
    {
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
        ImGui::SetNextWindowFocus();
    }
    const bool expanded = ImGui::Begin(PaneWindowId(PANE::TOOLBAR, m_bSequenceWorkspace), &m_bOpen, ImGuiWindowFlags_MenuBar);
    Render_WindowMenu();
    if (expanded)
        ImGui::TextUnformatted("Select Boss, Character, Object, Sequence or World in Composition Actions.");
    ICompositionWorkbenchSession* const session = Selected_Session();
    if (!m_bOpen || nullptr == session)
    {
        if (expanded && nullptr == session)
            ImGui::TextDisabled("The selected action authoring session is unavailable.");
        if (!m_bOpen) Deactivate();
        ImGui::End();
        return;
    }
    /* The Map Tool may already own this session frame. Draw nothing that
       would open a second Begin/End pair for the same owner. */
    if (m_pExternallyHostedSession == session ||
        m_pExternallyHostedSessionAlt == session)
    {
        if (expanded)
            ImGui::TextDisabled("Map Tool is editing this Sequence this frame.");
        ImGui::End();
        return;
    }
    m_bInsideFrame = true;
    m_bCompositionEditFocused = false;
    m_bPhysicalAnimationFocused = false;
    m_PendingCompositionEdit.reset();
    m_PendingCompositionTransfer.reset();
    session->Select_WorkbenchTarget(m_eSelectedTarget);
    session->Begin_WorkbenchFrame();
    Apply_ViewRequest(*session);
    if (expanded)
    {
        ImGui::Separator();
        if (ImGui::Button("Copy##SharedCompositionEdit"))
            m_PendingCompositionEdit = COMPOSITION_EDIT_COMMAND::COPY;
        ImGui::SameLine();
        ImGui::BeginDisabled(!CCompositionClipboard::Get().Read());
        if (ImGui::Button("Paste##SharedCompositionEdit"))
            m_PendingCompositionEdit = COMPOSITION_EDIT_COMMAND::PASTE;
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Duplicate##SharedCompositionEdit"))
            m_PendingCompositionEdit = COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION;
        if (!m_CompositionEditStatus.empty()) ImGui::TextWrapped("%s", m_CompositionEditStatus.c_str());
        const bool hasBossOwner = m_eSelectedTarget == TARGET::BOSS || m_eSelectedTarget == TARGET::SEQUENCE ||
            m_eSelectedTarget == TARGET::OBJECT;
        ImGui::PushID(static_cast<int>(m_eSelectedTarget) * 16 +
            (hasBossOwner ? static_cast<int>(Get_SelectedBoss()) : 0));
        session->Render_WorkbenchPane(PANE::TOOLBAR);
        ImGui::PopID();
        Apply_ViewRequest(*session);
    }
    ImGui::End();
    for (const PANE pane : PANES)
    {
        if (!m_bSequencerMaximized || pane == PANE::SEQUENCER)
            Render_Pane(*session, pane);
    }
    session->End_WorkbenchFrame();
    // Defer all authoring changes until every pane has released its row views.
    const auto& io = ImGui::GetIO();
    const COMPOSITION_EDIT_INPUT editInput{m_bCompositionEditFocused, io.KeyCtrl, io.WantTextInput,
        ImGui::IsAnyItemActive(), ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId),
        ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGui::GetDragDropPayload() != nullptr,
        ImGui::IsKeyPressed(ImGuiKey_C, false), ImGui::IsKeyPressed(ImGuiKey_V, false),
        ImGui::IsKeyPressed(ImGuiKey_D, false)};
    if (const auto command = Resolve_CompositionShortcut(editInput))
    {
        if (m_bPhysicalAnimationFocused && *command == COMPOSITION_EDIT_COMMAND::COPY)
        {
            if (m_bHasSelectedAnimationResource)
            {
                auto transfer = std::make_shared<COMPOSITION_ANIMATION_TRANSFER>();
                transfer->resource = m_SelectedAnimationResource;
                transfer->label = m_SelectedAnimationResource.strDisplayName.empty() ?
                    m_SelectedAnimationResource.strRuntimeClip : m_SelectedAnimationResource.strDisplayName;
                CCompositionClipboard::Get().Write(std::move(transfer));
                m_CompositionEditStatus = "Animation resource copied.";
            }
            else m_CompositionEditStatus = "Select an Animation resource first.";
        }
        else m_PendingCompositionEdit = command;
    }
    if (m_PendingCompositionTransfer)
        session->Insert_CompositionTransfer(m_PendingCompositionTransfer, m_CompositionEditStatus);
    else if (m_PendingCompositionEdit)
        session->Execute_CompositionEdit(*m_PendingCompositionEdit, m_CompositionEditStatus);
    m_bInsideFrame = false;
    Apply_ViewRequest(*session);
    m_bApplyResetLayoutThisFrame = false;
    m_bRestoreAuthoringPanesRequested = false;
}
