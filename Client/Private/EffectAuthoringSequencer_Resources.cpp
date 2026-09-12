#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "EffectAuthoringResourceTree.h"
#include "EffectEditingSession.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>

namespace Client
{
namespace
{
constexpr std::uint32_t MAX_RESOURCE_MS = 600000u;
constexpr const char* RESOURCE_TABS[] = { "Animation", "Effect", "Collider", "Sound", "Camera", "Screen Post" };

std::string ResourcePathUtf8(const std::filesystem::path& path)
{
    const auto text = path.generic_u8string();
    return std::string(text.begin(), text.end());
}

bool ResourceMatches(const std::string& text, const std::string& query)
{
    return query.empty() || std::search(text.begin(), text.end(), query.begin(), query.end(),
        [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }) != text.end();
}

std::vector<std::string> ResourceSegments(const std::string& category)
{
    std::vector<std::string> result;
    std::size_t start = 0;
    while (start < category.size())
    {
        const auto end = category.find('/', start);
        if (end != start) result.push_back(category.substr(start, end - start));
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return result;
}

template<class Rows, class Kind>
void ReadAnimationRows(Rows& rows, const Kind animationKind)
{
    const auto model = CAnimationTargetService::Resolve_Model();
    if (!model) return;
    const auto asset = CAnimationTargetService::Resolve_AssetName();
    std::set<std::string> names;
    for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
    {
        const auto* name = model->Get_AnimationName(i);
        if (!name || !*name) continue;
        typename Rows::value_type row; row.kind = animationKind; row.id = row.label = name;
        row.category = asset;
        float position = 0.f, duration = 0.f;
        const float rate = model->Get_AnimationTickPerSecond(i);
        if (!names.insert(row.id).second) row.status = "The model contains an ambiguous clip name.";
        if (!model->Get_AnimationProgress(i, position, duration) || !std::isfinite(duration) || duration <= 0.f ||
            !std::isfinite(rate) || rate <= 0.f || duration / rate > MAX_RESOURCE_MS * .001f)
            row.status = "The model clip has invalid duration metadata.";
        else row.durationMs = static_cast<std::uint32_t>(std::ceil(duration / rate * 1000.f));
        rows.push_back(std::move(row));
    }
}

bool ReadColliderDefinitions(std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE>& rows,
    std::string& error)
{
    const auto path = CKoukuSaydonCompositionDocument::Resolve_Path();
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size > 16u * 1024u * 1024u)
    { error = "Saved Collider composition is unavailable or exceeds 16 MiB."; return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "Cannot read saved Collider composition."; return false; }
    const std::string bytes((std::istreambuf_iterator<char>(input)), {});
    if (input.bad()) { error = "Saved Collider composition read failed."; return false; }
    KOUKU_SAYDON_COMPOSITION_DOCUMENT document;
    // Reuse the source codec without admitting the unrelated boss gameplay graph.
    if (!CKoukuSaydonCompositionDocument::Parse_Text(bytes, document, error)) return false;
    for (auto& resource : document.PresentationResources)
        if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
            rows.push_back(std::move(resource));
    return true;
}

bool DetailMs(const char* label, std::uint32_t& value, bool& commit, const int minimum = 0)
{
    int edit = static_cast<int>(value);
    const bool changed = ImGui::DragInt(label, &edit, 10.f, minimum, MAX_RESOURCE_MS, "%d ms");
    commit |= ImGui::IsItemDeactivatedAfterEdit();
    if (changed) value = static_cast<std::uint32_t>((std::clamp)(edit, minimum, int(MAX_RESOURCE_MS)));
    return changed;
}

bool DetailVector(const char* label, float3_t& value, bool& commit, const float minimum = 0.f,
    const float maximum = 0.f)
{
    const bool changed = ImGui::DragFloat3(label, &value.x, .01f, minimum, maximum);
    commit |= ImGui::IsItemDeactivatedAfterEdit();
    return changed;
}
}

struct CEffectAuthoringSequencer::BOX_DETAIL_DRAFT final
{
    TRACK_KIND kind = TRACK_KIND::EFFECT;
    std::string id;
    CLIP animation;
    EFFECT_ROW effect;
    SOUND_ROW sound;
    COLLIDER_ROW collider;
    bool dirty = false;
};

bool CEffectAuthoringSequencer::Refresh_CompositionResourceInventory()
{
    std::vector<RESOURCE_ENTRY> staged;
    std::string problems;
    bool complete = true;
    const auto preserve = [&](TRACK_KIND kind, const std::string& error)
    {
        complete = false;
        if (!problems.empty()) problems += "\n";
        problems += std::string(RESOURCE_TABS[static_cast<std::size_t>(kind)]) + ": " + error;
        for (const auto& previous : m_CompositionResources)
            if (previous.kind == kind) staged.push_back(previous);
    };
    ReadAnimationRows(staged, TRACK_KIND::ANIMATION);
    m_ResourceModelGeneration = CAnimationTargetService::Resolve_TargetGeneration();

    std::vector<CEffectAuthoringResourceTree::RESOURCE> authored;
    std::string error;
    if (CEffectAuthoringResourceTree::Read_V1Inventory(authored, error))
        for (const auto& source : authored)
        {
            RESOURCE_ENTRY row; row.kind = TRACK_KIND::EFFECT;
            row.key = {source.eKind, source.strAssetId}; row.id = "authored:" + source.strAssetId;
            row.label = source.strDisplayName.empty() ? source.strAssetId : source.strDisplayName;
            row.category = "Saved Effects"; row.status = source.strStatus;
            staged.push_back(std::move(row));
        }
    else
    {
        complete = false; problems += "Saved Effects: " + error;
        for (const auto& previous : m_CompositionResources)
            if (previous.kind == TRACK_KIND::EFFECT && previous.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
                staged.push_back(previous);
    }
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> typed;
    error.clear();
    if (CEffectV2Catalog::Get().Read_Inventory(typed, error))
        for (const auto& source : typed)
        {
            RESOURCE_ENTRY row;
            row.kind = source.eKind == EFFECT_V2_RESOURCE_KIND::LEAF && source.strCategory == "ScreenPost" ?
                TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT;
            row.key = {source.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? EFFECT_RESOURCE_OWNER_KIND::V2_GROUP :
                EFFECT_RESOURCE_OWNER_KIND::V2_LEAF, source.strResourceId};
            row.id = std::string(source.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "group:" : "effect:") + source.strResourceId;
            row.label = source.strDisplayName.empty() ? source.strResourceId : source.strDisplayName;
            row.category = source.strCategory; row.status = source.strStatus; row.durationMs = source.iDurationMs;
            staged.push_back(std::move(row));
        }
    else
    {
        complete = false; if (!problems.empty()) problems += "\n"; problems += "Effect catalog: " + error;
        for (const auto& previous : m_CompositionResources)
            if ((previous.kind == TRACK_KIND::EFFECT || previous.kind == TRACK_KIND::SCREEN_POST) &&
                previous.key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT) staged.push_back(previous);
    }

    std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> colliders;
    error.clear();
    if (ReadColliderDefinitions(colliders, error))
        for (const auto& source : colliders)
        {
            RESOURCE_ENTRY row; row.kind = TRACK_KIND::COLLIDER; row.id = source.strResourceId;
            row.label = source.strDisplayName.empty() ? row.id : source.strDisplayName;
            row.category = source.strShape; row.collider = source; row.durationMs = source.iDurationMs;
            if (source.strColliderKind != "GEOMETRY") row.status = "This resource requires its Action Workbench gameplay Logic.";
            staged.push_back(std::move(row));
        }
    else preserve(TRACK_KIND::COLLIDER, error);

    std::vector<RESOURCE_ENTRY> sounds;
    const auto resourceRoot = CRuntimeAssetRoot::Get_ResourceRoot();
    const auto soundRoot = CRuntimeAssetRoot::Resolve("Sound");
    std::error_code ec;
    if (!soundRoot.empty())
    {
        std::filesystem::recursive_directory_iterator it(soundRoot,
            std::filesystem::directory_options::skip_permission_denied, ec), end;
        for (; !ec && it != end; it.increment(ec))
        {
            if (!it->is_regular_file(ec)) continue;
            auto extension = it->path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            if (extension != ".wav" && extension != ".ogg" && extension != ".mp3") continue;
            RESOURCE_ENTRY row; row.kind = TRACK_KIND::SOUND;
            row.id = ResourcePathUtf8(it->path().lexically_relative(resourceRoot));
            if (CRuntimeAssetRoot::Resolve(std::filesystem::path(std::u8string(row.id.begin(), row.id.end()))).empty()) continue;
            row.label = ResourcePathUtf8(it->path().filename());
            row.category = ResourcePathUtf8(it->path().parent_path().lexically_relative(soundRoot));
            if (row.category == ".") row.category.clear();
            sounds.push_back(std::move(row));
        }
    }
    if (soundRoot.empty() || ec) preserve(TRACK_KIND::SOUND, ec ? ec.message() : "Sound resource root is unavailable.");
    else staged.insert(staged.end(), std::make_move_iterator(sounds.begin()), std::make_move_iterator(sounds.end()));

    RESOURCE_ENTRY camera; camera.kind = TRACK_KIND::CAMERA; camera.id = "camera.capture.current";
    camera.label = "Current camera view"; camera.category = "Capture"; camera.durationMs = 1000u;
    staged.push_back(std::move(camera));
    std::stable_sort(staged.begin(), staged.end(), [](const auto& a, const auto& b)
    {
        if (a.kind != b.kind) return a.kind < b.kind;
        if (a.category != b.category) return a.category < b.category;
        if (a.label != b.label) return a.label < b.label;
        return a.id < b.id;
    });
    m_CompositionResources = std::move(staged); m_ResourcesLoaded = true;
    m_ResourceStatus = std::move(problems);
    Rebuild_CompositionResourceTrees();
    return complete;
}

void CEffectAuthoringSequencer::Rebuild_CompositionResourceTrees()
{
    for (std::size_t family = 0; family < m_ResourceTrees.size(); ++family)
    {
        m_ResourceTrees[family] = {};
        m_ResourceQueries[family] = m_ResourceSearch[family].data();
        for (std::size_t i = 0; i < m_CompositionResources.size(); ++i)
        {
            const auto& row = m_CompositionResources[i];
            if (static_cast<std::size_t>(row.kind) != family) continue;
            const auto& query = m_ResourceQueries[family];
            if (!ResourceMatches(row.label, query) && !ResourceMatches(row.id, query) && !ResourceMatches(row.category, query)) continue;
            InsertResourceTree(m_ResourceTrees[family], ResourceSegments(row.category), i);
        }
        FinalizeResourceTree(m_ResourceTrees[family]);
    }
}

void CEffectAuthoringSequencer::Render_CompositionResources()
{
    if (!m_ResourcesOpen) return;
    const std::string title = "Composition Resources###EffectCompositionResources." + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    ImGui::SetNextWindowSize({470.f, 570.f}, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title.c_str(), &m_ResourcesOpen)) { ImGui::End(); return; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
    if (!m_ResourcesLoaded) Refresh_CompositionResourceInventory();
    if (ImGui::Button("Refresh Resources")) Refresh_CompositionResourceInventory();
    ImGui::SameLine(); ImGui::TextDisabled("Append at %u ms", ClockMs());
    if (m_ResourceModelGeneration != CAnimationTargetService::Resolve_TargetGeneration())
    {
        // A new rig only changes native clips; the saved catalogs keep their cache.
        std::erase_if(m_CompositionResources, [](const auto& row) { return row.kind == TRACK_KIND::ANIMATION; });
        ReadAnimationRows(m_CompositionResources, TRACK_KIND::ANIMATION);
        m_ResourceModelGeneration = CAnimationTargetService::Resolve_TargetGeneration();
        m_SelectedResourceIds[static_cast<std::size_t>(TRACK_KIND::ANIMATION)].clear();
        Rebuild_CompositionResourceTrees();
    }
    if (ImGui::BeginTabBar("CompositionResourceTabs"))
    {
        for (std::size_t family = 0; family < m_ResourceTrees.size(); ++family)
        {
            if (!ImGui::BeginTabItem(RESOURCE_TABS[family])) continue;
            m_ResourceTab = static_cast<int>(family);
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::InputTextWithHint("##ResourceSearch", "Search name, category or source...",
                m_ResourceSearch[family].data(), m_ResourceSearch[family].size())) Rebuild_CompositionResourceTrees();
            const float footer = ImGui::GetFrameHeightWithSpacing() * 3.f;
            if (ImGui::BeginChild("ResourceList", {0.f, (std::max)(120.f, ImGui::GetContentRegionAvail().y - footer)}, ImGuiChildFlags_Borders))
                RenderResourceTree(m_ResourceTrees[family], [&](std::size_t index)
                {
                    const auto& row = m_CompositionResources[index];
                    ImGui::PushID(row.id.c_str());
                    if (ImGui::Selectable(row.label.c_str(), m_SelectedResourceIds[family] == row.id)) m_SelectedResourceIds[family] = row.id;
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s%s", row.key.Is_Valid() ? row.key.strStableId.c_str() : row.id.c_str(),
                        row.status.empty() ? "" : "\n", row.status.c_str());
                    ImGui::PopID();
                });
            ImGui::EndChild();
            const auto selected = std::find_if(m_CompositionResources.begin(), m_CompositionResources.end(), [&](const auto& row)
                { return static_cast<std::size_t>(row.kind) == family && row.id == m_SelectedResourceIds[family]; });
            ImGui::BeginDisabled(selected == m_CompositionResources.end() || !selected->status.empty());
            if (ImGui::Button("Add / Append"))
            {
                bool added = false;
                switch (selected->kind)
                {
                case TRACK_KIND::ANIMATION: added = Append_Animation(selected->id); break;
                case TRACK_KIND::EFFECT:
                    added = Append(selected->key, selected->key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ? 0u : selected->durationMs);
                    break;
                case TRACK_KIND::SCREEN_POST: added = Append(selected->key, selected->durationMs, true); break;
                case TRACK_KIND::SOUND: added = Append_Sound(selected->id, 0u); break;
                case TRACK_KIND::COLLIDER: added = Append_Collider(selected->collider); break;
                case TRACK_KIND::CAMERA:
                {
                    CAMERA_ROW row; row.id = CEffectEditingSession::New_Id("camera.row."); row.label = "Camera";
                    row.startMs = (std::min)(ClockMs(), MAX_RESOURCE_MS - 1000u); row.cue.iDurationMs = 1000u;
                    row.cue.strCueId = row.id; row.modelRelative = m_ModelRoot;
                    if (Capture_CameraKey(row, 0u))
                    {
                        auto staged = m_CameraRows;
                        for (auto preview : m_TransientCameraRows)
                        {
                            preview.id = CEffectEditingSession::New_Id("camera.row."); preview.cue.strCueId = preview.id;
                            staged.push_back(std::move(preview));
                        }
                        staged.push_back(row);
                        if (Validate_CameraRows(staged) && Commit_TransientPreview())
                        {
                            // Promotion owns its new camera identities. Append to that
                            // committed list rather than overwriting it with this draft.
                            m_CameraRows.push_back(row); m_Dirty = added = true;
                            Select_TimelineRow(TRACK_KIND::CAMERA, row.id);
                        }
                    }
                    break;
                }
                }
                if (added) m_BoxDetailOpen = true;
            }
            ImGui::EndDisabled();
            if (selected != m_CompositionResources.end() && !selected->status.empty()) ImGui::TextWrapped("%s", selected->status.c_str());
            if (family == 0 && !CAnimationTargetService::Resolve_Model()) ImGui::TextDisabled("Select a character or boss in Model View.");
            if (!m_ResourceStatus.empty()) { ImGui::TextDisabled("Some resources could not refresh."); if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_ResourceStatus.c_str()); }
            if (!m_Status.empty())
            {
                const auto end = m_Status.find_first_of("\r\n");
                ImGui::TextUnformatted(m_Status.data(), m_Status.data() + (end == std::string::npos ? m_Status.size() : end));
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_Status.c_str());
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CEffectAuthoringSequencer::Render_BoxDetail(const bool embedded)
{
    if (!embedded && !m_BoxDetailOpen) return;
    const auto finish = [embedded]() { if (!embedded) ImGui::End(); };
    const std::string title = "Box Detail###EffectCompositionDetail." + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    if (!embedded)
    {
        ImGui::SetNextWindowSize({440.f, 470.f}, ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title.c_str(), &m_BoxDetailOpen)) { ImGui::End(); return; }
    }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
    if (m_SelectedTrack == TRACK_KIND::CAMERA)
    {
        m_SelectedCamera = m_SelectedRowId;
        const auto previous = m_SelectedCamera;
        Render_CameraEditor();
        if (previous != m_SelectedCamera) Select_TimelineRow(TRACK_KIND::CAMERA, m_SelectedCamera);
        finish(); return;
    }
    if (m_SelectedRowId.empty()) { ImGui::TextDisabled("Select a timeline box."); finish(); return; }
    if (m_Transient && !m_Transient->previewElementIds.empty() && m_SelectedRowId == m_Transient->id &&
        m_SelectedTrack == TRACK_KIND::EFFECT)
    {
        const auto& row = *m_Transient;
        ImGui::TextWrapped("%s", row.previewElementLabel.empty() ? row.previewElementIds.front().c_str() : row.previewElementLabel.c_str());
        ImGui::Text("Selected elements: %zu", row.previewElementIds.size());
        for (const auto& elementId : row.previewElementIds) ImGui::TextWrapped("%s", elementId.c_str());
        ImGui::TextWrapped("Document: %s", row.key.strStableId.c_str());
        ImGui::Text("Original document time: %u / %u ms", ClockMs(), row.durationMs);
        ImGui::TextWrapped("This temporary preview keeps the original element timing. Pause or scrub to inspect it; Stop returns to the saved sequence.");
        ImGui::TextDisabled("Edit the element in Effect Detail. This row is not saved or appended.");
        finish(); return;
    }
    if (!m_BoxDetailDraft || m_BoxDetailDraft->id != m_SelectedRowId || m_BoxDetailDraft->kind != m_SelectedTrack || !m_BoxDetailDraft->dirty)
    {
        auto draft = std::make_shared<BOX_DETAIL_DRAFT>(); draft->kind = m_SelectedTrack; draft->id = m_SelectedRowId;
        bool found = false;
        const auto copy = [&](const auto& rows, auto& destination)
        {
            const auto row = std::find_if(rows.begin(), rows.end(), [&](const auto& value) { return value.id == draft->id; });
            if (row != rows.end()) { destination = *row; found = true; }
        };
        switch (draft->kind)
        {
        case TRACK_KIND::ANIMATION:
            if (m_CustomAnimation) copy(m_AnimationRows, draft->animation);
            else if (const auto* sequence = Selected_Sequence()) copy(sequence->clips, draft->animation);
            break;
        case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST: copy(m_Effects, draft->effect); break;
        case TRACK_KIND::SOUND: copy(m_Sounds, draft->sound); break;
        case TRACK_KIND::COLLIDER: copy(m_Colliders, draft->collider); break;
        case TRACK_KIND::CAMERA: break;
        }
        if (!found) { ImGui::TextDisabled("The selected box is no longer available."); finish(); return; }
        draft->effect.v1.reset(); draft->effect.v2 = 0; draft->effect.snapshot.reset();
        draft->effect.history.reset(); draft->effect.anchorHistory.reset();
        draft->sound.handle = 0; draft->sound.sampledAge = -1;
        m_BoxDetailDraft = std::move(draft);
    }
    const auto draftOwner = m_BoxDetailDraft;
    auto& draft = *draftOwner;
    ImGui::TextUnformatted(RESOURCE_TABS[static_cast<std::size_t>(draft.kind)]);
    ImGui::TextDisabled("%s", draft.id.c_str());
    bool changed = false, commit = false;
    ImGui::PushItemWidth(-1.f);
    switch (draft.kind)
    {
    case TRACK_KIND::ANIMATION:
    {
        auto& row = draft.animation; ImGui::TextWrapped("%s", row.clipName.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        changed |= DetailMs("Source start", row.sourceStartMs, commit); changed |= DetailMs("Source play", row.sourcePlayMs, commit);
        changed |= ImGui::DragFloat("Playback rate", &row.playRate, .01f, .01f, 100.f); commit |= ImGui::IsItemDeactivatedAfterEdit();
        if (ImGui::Checkbox("Loop", &row.loop)) changed = commit = true;
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
    {
        auto& row = draft.effect; ImGui::TextWrapped("%s", row.key.strStableId.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        if (draft.kind == TRACK_KIND::EFFECT)
        {
            if (ImGui::Checkbox("World anchor", &row.worldAnchor))
            { row.anchorSlotId = "root"; changed = commit = true; }
            changed |= DetailVector(row.worldAnchor ? "World position (m)" : "Anchor offset (m)", row.offset, commit);
            changed |= DetailVector("Rotation (degrees)", row.rotation, commit);
            changed |= DetailVector("Scale", row.scale, commit, .001f, 1000.f);
            if (!row.worldAnchor && Render_AnchorChoice("Bone / socket", row.anchorSlotId)) changed = commit = true;
        }
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::SOUND:
    {
        auto& row = draft.sound; ImGui::TextWrapped("%s", row.assetId.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        changed |= DetailMs("Source start", row.sourceStartMs, commit);
        changed |= ImGui::SliderFloat("Volume", &row.volume, 0.f, 1.f); commit |= ImGui::IsItemDeactivatedAfterEdit();
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::COLLIDER:
    {
        auto& row = draft.collider; ImGui::TextWrapped("%s", row.label.c_str());
        ImGui::TextDisabled("%s", row.resource.strShape.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        changed |= DetailVector("Offset (m)", row.offset, commit); changed |= DetailVector("Rotation (degrees)", row.rotation, commit);
        changed |= DetailVector("Scale", row.scale, commit, .001f, 10000.f);
        if (Render_AnchorChoice("Bone / socket", row.anchorSlotId)) changed = commit = true;
        if (ImGui::Checkbox("Debug Render", &row.debugRender)) changed = commit = true;
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::CAMERA: break;
    }
    ImGui::PopItemWidth();
    draft.dirty |= changed;
    if (ImGui::Button("Apply")) commit = true;
    ImGui::SameLine();
    if (ImGui::Button("Revert")) { m_BoxDetailDraft.reset(); finish(); return; }
    if (commit && draft.dirty)
    {
        bool applied = false;
        switch (draft.kind)
        {
        case TRACK_KIND::ANIMATION: applied = Apply_AnimationRow(draft.animation); break;
        case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST: applied = Apply_EffectRow(draft.effect); break;
        case TRACK_KIND::SOUND: applied = Apply_SoundRow(draft.sound); break;
        case TRACK_KIND::COLLIDER: applied = Apply_ColliderRow(draft.collider); break;
        case TRACK_KIND::CAMERA: break;
        }
        if (applied) draft.dirty = false;
    }
    if (draft.dirty) ImGui::TextDisabled("Unapplied values");
    ImGui::BeginDisabled(draft.dirty);
    if (ImGui::Button("Duplicate")) { if (Duplicate_SelectedRow()) m_BoxDetailDraft.reset(); }
    ImGui::SameLine();
    if (ImGui::Button("Remove")) { if (Remove_SelectedRow()) m_BoxDetailDraft.reset(); }
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    finish();
}
}
