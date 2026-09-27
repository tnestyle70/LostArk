#include "imgui.h"
#include "ClassMovieInspector.h"

#include <algorithm>
#include <cctype>

namespace
{
std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
bool Matches(const Client::CLASS_MOVIE_WORLD_ITEM& item, const std::string& query)
{
    if (query.empty()) return true;
    std::string text = item.label + " " + item.id + " " + item.instanceId + " " +
        item.slotId + " " + item.objectId + " " + item.modelAssetId;
    for (const auto& material : item.materials) text += " " + material;
    return Lower(std::move(text)).find(query) != std::string::npos;
}
}

void Client::CClassMovieInspector::Render(const CLASS_MOVIE_INSPECTION_CALLBACKS& callbacks,
    const std::string& classId, const bool loop, const bool unappliedRowDraft)
{
    if (!callbacks.state || !callbacks.command || classId.empty()) return;
    ImGui::PushID(this);
    if (!ImGui::CollapsingHeader("Movie world models", ImGuiTreeNodeFlags_DefaultOpen))
    { ImGui::PopID(); return; }
    const auto state = callbacks.state(classId, loop);
    const auto submit = [&](const CLASS_MOVIE_INSPECTION_ACTION action,
        const std::string& id = std::string{}, const bool enabled = true)
    {
        CLASS_MOVIE_INSPECTION_COMMAND command;
        command.action = action; command.itemId = id; command.enabled = enabled;
        return callbacks.command(classId, loop, command, m_Status);
    };
    using ACTION = CLASS_MOVIE_INSPECTION_ACTION;
    ImGui::Text("%s | Movie %.3f s | Source %.3f s", loop ? "Loop" : "Intro",
        state.movieMs * .001, state.sourceMs * .001);
    ImGui::BeginDisabled(!state.active);
    if (ImGui::RadioButton("Cinematic camera", !state.freeCamera)) (void)submit(ACTION::FREE_CAMERA, {}, false);
    ImGui::SameLine();
    if (ImGui::RadioButton("Free camera (F6)", state.freeCamera)) (void)submit(ACTION::FREE_CAMERA);
    ImGui::Text("Camera XYZ: %.3f, %.3f, %.3f m", state.cameraPosition.x, state.cameraPosition.y, state.cameraPosition.z);
    if (ImGui::Button(state.pickArmed ? "Cancel scene pick" : "Pick in scene"))
        (void)submit(ACTION::PICK_IN_SCENE, {}, !state.pickArmed);
    if (state.pickArmed) ImGui::TextWrapped("Click one model in the scene outside the tool windows. Picking is used once.");
    bool background = state.showBackground, effects = state.showEffects;
    if (ImGui::Checkbox("Show background (preview)", &background)) (void)submit(ACTION::BACKGROUND, {}, background);
    if (ImGui::Checkbox("Show Effects (preview)", &effects)) (void)submit(ACTION::EFFECTS, {}, effects);
    ImGui::EndDisabled();
    if (ImGui::Button("Clear preview filters")) (void)submit(ACTION::CLEAR_PREVIEW);
    ImGui::TextWrapped("Solo isolates WORLD models. Background and Effects use their own preview checkboxes. These filters keep Movie time running and are not saved.");
    ImGui::InputTextWithHint("Search models", "Name, stable ID, WModel or material", m_Search.data(), m_Search.size());
    ImGui::Checkbox("Authored visible at cursor only", &m_VisibleAtCursorOnly);
    ImGui::Checkbox("Show deleted models (restore)", &m_ShowDeleted);
    const auto query = Lower(m_Search.data());
    size_t shown = 0;
    const auto tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("Models", 4, tableFlags, {0.f, 200.f}))
    {
        ImGui::TableSetupColumn("Model", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Authored", ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupColumn("Drawn", ImGuiTableColumnFlags_WidthFixed, 50.f);
        ImGui::TableSetupColumn("World XYZ (m)", ImGuiTableColumnFlags_WidthFixed, 155.f);
        ImGui::TableSetupScrollFreeze(0, 1); ImGui::TableHeadersRow();
        for (const auto& item : state.items)
        {
            if ((item.excluded && !m_ShowDeleted) ||
                (!item.excluded && m_VisibleAtCursorOnly && !(item.sampled && item.authoredVisible)) || !Matches(item, query)) continue;
            ++shown;
            ImGui::PushID(item.id.c_str());
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            std::string label = item.label.empty() ? item.id : item.label;
            if (item.excluded) label += " [deleted]";
            else if (item.solo) label += " [solo]";
            else if (item.muted) label += " [muted]";
            if (ImGui::Selectable((label + "##model").c_str(), item.selected,
                ImGuiSelectableFlags_SpanAllColumns)) (void)submit(ACTION::SELECT, item.id);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s", item.id.c_str(), item.modelAssetId.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(!item.sampled ? "No sample" : item.authoredVisible ? "Visible" : "Hidden");
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(item.drawn ? "Yes" : "No");
            ImGui::TableSetColumnIndex(3);
            if (item.sampled) ImGui::Text("%.2f, %.2f, %.2f", item.position.x, item.position.y, item.position.z);
            else ImGui::TextUnformatted("Not sampled");
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::TextDisabled("%zu / %zu models. Authored is the sampled source visibility; Drawn includes preview filters.", shown, state.items.size());
    const auto selected = std::find_if(state.items.begin(), state.items.end(),
        [&](const auto& item) { return item.id == state.selectedId; });
    if (selected != state.items.end())
    {
        const auto& item = *selected;
        ImGui::SeparatorText("Selected model");
        ImGui::TextWrapped("%s", item.label.c_str());
        ImGui::TextWrapped("Stable ID: %s", item.id.c_str());
        ImGui::TextWrapped("Instance: %s | Slot: %s | Object: %s", item.instanceId.c_str(), item.slotId.c_str(), item.objectId.c_str());
        ImGui::TextWrapped("WModel: %s", item.modelAssetId.c_str());
        if (item.sampled) ImGui::Text("World XYZ: %.3f, %.3f, %.3f m", item.position.x, item.position.y, item.position.z);
        ImGui::BeginDisabled(!state.active);
        if (ImGui::Button(item.solo ? "Unsolo WORLD" : "Solo WORLD")) (void)submit(ACTION::SOLO, item.id, !item.solo);
        ImGui::SameLine();
        if (ImGui::Button(item.muted ? "Unmute" : "Mute")) (void)submit(ACTION::MUTE, item.id, !item.muted);
        ImGui::SameLine();
        if (ImGui::Button("Focus selected")) (void)submit(ACTION::FOCUS, item.id);
        ImGui::EndDisabled();
        ImGui::Text("Meshes: %u | Materials: %zu", item.meshCount, item.materials.size());
        if (state.pickedMesh != UINT32_MAX) ImGui::Text("Picked mesh: %u", state.pickedMesh);
        if (ImGui::TreeNode("Mesh / material information"))
        {
            for (const auto& material : item.materials) ImGui::TextWrapped("%s", material.c_str());
            if (item.materials.empty()) ImGui::TextDisabled("No material information is available for this model.");
            ImGui::TreePop();
        }
        ImGui::BeginDisabled(unappliedRowDraft);
        if (ImGui::Button(item.excluded ? "Restore to Movie" : "Delete from Movie"))
            (void)submit(ACTION::EXCLUDE, item.id, !item.excluded);
        ImGui::EndDisabled();
    }
    ImGui::SeparatorText("Movie source");
    ImGui::TextWrapped("Delete / Restore changes the Movie draft immediately. Save Movie stores that change. Deleted models remain in this list for Restore; Clear preview filters does not restore them.");
    if (unappliedRowDraft)
        ImGui::TextWrapped("Apply the edited timeline row or use its Save movie button before deleting, saving or reloading here.");
    ImGui::BeginDisabled(unappliedRowDraft);
    if (ImGui::Button("Save Movie")) (void)submit(ACTION::SAVE);
    ImGui::SameLine();
    if (ImGui::Button("Reload saved Movie"))
    {
        if (state.dirty) ImGui::OpenPopup("Discard Movie draft?");
        else (void)submit(ACTION::RELOAD);
    }
    ImGui::EndDisabled();
    if (ImGui::BeginPopupModal("Discard Movie draft?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("Reload discards the unsaved Movie changes and reads the saved Movie sources.");
        if (ImGui::Button("Discard draft and reload"))
        { (void)submit(ACTION::RELOAD); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Keep editing")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (state.dirty) ImGui::TextUnformatted("Movie has unsaved source changes.");
    ImGui::TextWrapped("Save / Reload can stop playback. Press Play All again to replay the resulting Movie.");
    if (!state.status.empty()) ImGui::TextWrapped("%s", state.status.c_str());
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::PopID();
}
