#include "imgui.h"
#include "BoneAnimationWorkbench.h"
#include "CompositionTimeline.h"
#include "EffectEditingSession.h"
#include "Model.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace Client
{
namespace
{
BONE_ANIMATION_KEY KeyAt(const BONE_ANIMATION_CLIP& clip, const std::string& bone, float time)
{
    BONE_ANIMATION_KEY result;
    result.timeMs = static_cast<uint32_t>(std::round((std::clamp)(time, 0.f, static_cast<float>(clip.durationMs))));
    const auto track = std::find_if(clip.tracks.begin(), clip.tracks.end(), [&](const auto& t) { return t.bone == bone; });
    if (track == clip.tracks.end() || track->keys.empty()) return result;
    const auto upper = std::upper_bound(track->keys.begin(), track->keys.end(), time,
        [](float t, const auto& k) { return t < k.timeMs; });
    if (upper == track->keys.begin()) result = *upper;
    else if (upper == track->keys.end()) result = track->keys.back();
    else
    {
        const auto& a = *(upper - 1); const auto& b = *upper;
        const float weight = (time - a.timeMs) / static_cast<float>(b.timeMs - a.timeMs);
        XMStoreFloat3(&result.position, XMVectorLerp(XMLoadFloat3(&a.position), XMLoadFloat3(&b.position), weight));
        XMStoreFloat3(&result.scale, XMVectorLerp(XMLoadFloat3(&a.scale), XMLoadFloat3(&b.scale), weight));
        XMStoreFloat4(&result.rotation, XMQuaternionNormalize(XMQuaternionSlerp(XMLoadFloat4(&a.rotation), XMLoadFloat4(&b.rotation), weight)));
    }
    result.timeMs = static_cast<uint32_t>(std::round((std::clamp)(time, 0.f, static_cast<float>(clip.durationMs))));
    return result;
}
void PutKey(BONE_ANIMATION_CLIP& clip, const std::string& bone, const BONE_ANIMATION_KEY& key)
{
    auto track = std::find_if(clip.tracks.begin(), clip.tracks.end(), [&](const auto& t) { return t.bone == bone; });
    if (track == clip.tracks.end()) { clip.tracks.push_back({bone, {}}); track = clip.tracks.end() - 1; }
    const auto found = std::find_if(track->keys.begin(), track->keys.end(), [&](const auto& k) { return k.timeMs == key.timeMs; });
    if (found == track->keys.end()) track->keys.push_back(key); else *found = key;
    std::sort(track->keys.begin(), track->keys.end(), [](const auto& a, const auto& b) { return a.timeMs < b.timeMs; });
}
}
BONE_ANIMATION_CLIP* CBoneAnimationWorkbench::Selected()
{
    const auto found = std::find_if(m_Document.clips.begin(), m_Document.clips.end(), [&](const auto& clip) { return clip.name == m_Selected; });
    return found == m_Document.clips.end() ? nullptr : &*found;
}
bool CBoneAnimationWorkbench::Select(const std::string& asset, const std::shared_ptr<Engine::CModel>& model)
{
    if (!model) return false;
    if (m_Document.asset == asset && m_Model.lock() == model) return true;
    if (Is_Dirty()) { m_Status = "Save or discard bone clip / pose changes before changing this skeleton"; return false; }
    CBoneAnimationDocument document;
    if (!document.Load(asset, *model, m_Status)) return false;
    Stop(); m_Model = model; m_Document = std::move(document); m_Selected.clear(); m_Bone.clear(); m_Source.clear();
    m_Undo.clear(); m_Redo.clear(); m_PoseDirty = false; m_Clock = 0.f;
    if (!m_Document.clips.empty()) m_Selected = m_Document.clips.front().name;
    Read_Key();
    return true;
}
void CBoneAnimationWorkbench::Stop()
{
    const bool ownedPose = m_Playing || m_Override;
    m_Playing = m_Override = false;
    if (ownedPose) if (const auto model = m_Model.lock()) model->Set_AnimPaused(true);
}
void CBoneAnimationWorkbench::Apply_Pose(bool requestOwnership)
{
    const auto model = m_Model.lock(); const auto* clip = Selected();
    if (!model || !clip) return;
    if (requestOwnership) m_PreviewRequested = true;
    std::vector<float4x4_t> pose;
    // A dragged pose is session state until Set key. Replacing the sampled
    // track key avoids applying its previous delta twice.
    auto sample = &m_Document;
    CBoneAnimationDocument draft;
    if (m_PoseDirty && !m_Bone.empty())
    {
        draft = m_Document;
        auto& edited = *std::find_if(draft.clips.begin(), draft.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
        auto key = KeyAt(edited, m_Bone, m_Clock);
        key.position = m_KeyPosition; key.scale = m_KeyScale;
        XMStoreFloat4(&key.rotation, XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(
            XMConvertToRadians(m_KeyDegrees.x), XMConvertToRadians(m_KeyDegrees.y), XMConvertToRadians(m_KeyDegrees.z))));
        PutKey(edited, m_Bone, key); sample = &draft;
    }
    if (!sample->Sample(*model, clip->name, m_Clock, pose, m_Status)) { Stop(); return; }
    for (uint32_t bone = 0u; bone < pose.size(); ++bone)
        if (!model->Set_BoneLocalMatrix(bone, XMLoadFloat4x4(&pose[bone]))) { Stop(); return; }
    model->Refresh_BoneCombinedMatrices(); model->Set_AnimPaused(true);
}
void CBoneAnimationWorkbench::Read_Key()
{
    m_PoseDirty = false;
    const auto* clip = Selected();
    const auto key = clip ? KeyAt(*clip, m_Bone, m_Clock) : BONE_ANIMATION_KEY{};
    m_KeyPosition = key.position; m_KeyScale = key.scale;
    float4x4_t rotation; XMStoreFloat4x4(&rotation, XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotation)));
    const float pitch = std::asin((std::clamp)(-rotation._32, -1.f, 1.f));
    if (std::abs(std::cos(pitch)) > .0001f)
        m_KeyDegrees = {XMConvertToDegrees(pitch), XMConvertToDegrees(std::atan2(rotation._31, rotation._33)), XMConvertToDegrees(std::atan2(rotation._12, rotation._22))};
    else m_KeyDegrees = {XMConvertToDegrees(pitch), XMConvertToDegrees(std::atan2(-rotation._13, rotation._11)), 0.f};
}
void CBoneAnimationWorkbench::Select_Bone(const std::string& name)
{
    m_Bone = name; m_Playing = false; Read_Key();
    m_Override = true; Apply_Pose();
}
void CBoneAnimationWorkbench::Remember()
{
    if (m_Undo.size() >= 24u) m_Undo.erase(m_Undo.begin());
    m_Undo.push_back(m_Document); m_Redo.clear();
}
bool CBoneAnimationWorkbench::Commit(CBoneAnimationDocument candidate)
{
    const auto model = m_Model.lock();
    std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
    if (!model || !candidate.Compile(*model, compiled, m_Status)) return false;
    Remember(); m_Document = std::move(candidate); m_Dirty = true;
    if (!Selected()) m_Selected = m_Document.clips.empty() ? std::string{} : m_Document.clips.back().name;
    if (const auto* clip = Selected()) m_Clock = (std::min)(m_Clock, static_cast<float>(clip->durationMs));
    Read_Key(); m_Playing = false; m_Override = true; Apply_Pose();
    return true;
}
void CBoneAnimationWorkbench::Restore_History(bool redo)
{
    auto& source = redo ? m_Redo : m_Undo;
    auto& destination = redo ? m_Undo : m_Redo;
    if (source.empty()) return;
    destination.push_back(m_Document); m_Document = std::move(source.back()); source.pop_back();
    if (!Selected()) m_Selected = m_Document.clips.empty() ? std::string{} : m_Document.clips.back().name;
    if (const auto* clip = Selected()) m_Clock = (std::min)(m_Clock, static_cast<float>(clip->durationMs));
    m_Dirty = true; Read_Key(); m_Playing = false; m_Override = true; Apply_Pose();
}
void CBoneAnimationWorkbench::Render_Source()
{
    const auto model = m_Model.lock();
    if (!model || !ImGui::CollapsingHeader("New clip / source in-out", ImGuiTreeNodeFlags_DefaultOpen)) return;
    ImGui::InputText("New clip name", m_NewName, sizeof(m_NewName));
    if (ImGui::BeginCombo("Source animation", m_Source.empty() ? "Select source" : m_Source.c_str()))
    {
        for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
        {
            const char* name = model->Get_AnimationName(index);
            if (!name || std::string(name).starts_with("authored.")) continue;
            if (ImGui::Selectable(name, m_Source == name))
            {
                m_Source = name; m_SourceIn = 0;
                float position = 0.f, duration = 0.f;
                const auto rate = model->Get_AnimationTickPerSecond(index);
                if (model->Get_AnimationProgress(index, position, duration) && rate > 0.f)
                    m_SourceOut = static_cast<int>(std::floor(duration / rate * 1000.f));
            }
        }
        for (const auto& clip : m_Document.clips)
            if (ImGui::Selectable(clip.name.c_str(), m_Source == clip.name))
            { m_Source = clip.name; m_SourceIn = 0; m_SourceOut = static_cast<int>(clip.durationMs); }
        ImGui::EndCombo();
    }
    ImGui::DragInt("Source in (ms)", &m_SourceIn, 1.f, 0, 60000);
    ImGui::DragInt("Source out (ms)", &m_SourceOut, 1.f, 1, 60000);
    ImGui::DragFloat("New clip speed", &m_SourceRate, .01f, .01f, 16.f);
    ImGui::BeginDisabled(m_Source.empty());
    if (ImGui::Button("Create clip from in / out"))
    {
        auto candidate = m_Document;
        if (m_SourceIn >= 0 && m_SourceOut > 0 && candidate.Create_FromSourceRange(*model, m_NewName, m_Source,
            static_cast<uint32_t>(m_SourceIn), static_cast<uint32_t>(m_SourceOut), m_SourceRate, m_Status))
        { if (Commit(std::move(candidate))) { m_Selected = m_NewName; m_Clock = 0.f; Read_Key(); Apply_Pose(); } }
        else if (m_SourceIn < 0 || m_SourceOut <= 0) m_Status = "Source times must be positive and in < out.";
    }
    ImGui::DragInt("Held pose duration (ms)", &m_HoldDuration, 10.f, 1, 60000);
    if (ImGui::Button("Create held pose at source in"))
    {
        auto candidate = m_Document;
        if (m_SourceIn >= 0 && m_HoldDuration > 0 && candidate.Create_HeldPose(*model, m_NewName, m_Source,
            static_cast<uint32_t>(m_SourceIn), static_cast<uint32_t>(m_HoldDuration), m_Status))
        { if (Commit(std::move(candidate))) { m_Selected = m_NewName; m_Clock = 0.f; Read_Key(); Apply_Pose(); } }
        else if (m_SourceIn < 0 || m_HoldDuration <= 0) m_Status = "Held pose times must be positive.";
    }
    ImGui::EndDisabled();
    if (m_Document.asset == "Vehicle_9523")
    {
        if (ImGui::Button("Create dragon glide + ascent"))
        {
            auto candidate = m_Document;
            if (candidate.Create_AncientSeaFlightStudies(*model, m_Status) && Commit(std::move(candidate)))
            { m_Selected = "authored.dragon.glide"; m_Clock = 0.f; Read_Key(); Apply_Pose();
                m_Status = "Created glide (2 s) and ascent (967 ms) drafts from npc_sk_look. Save Bone Clips to keep them."; }
        }
        ImGui::TextWrapped("Glide holds the spread-wing pose at 2500 ms. Ascent loops the 1800-2767 ms wing beat with a 120 ms seam. Both stay in place; tune the pose before saving.");
    }
}
void CBoneAnimationWorkbench::Update(float deltaSeconds)
{
    if (!m_Override) return;
    const auto* clip = Selected();
    if (!clip || !std::isfinite(deltaSeconds)) { Stop(); return; }
    if (m_Playing)
    {
        m_Clock += (std::max)(0.f, deltaSeconds) * 1000.f;
        if (m_Clock >= clip->durationMs)
        {
            if (m_Loop) m_Clock = std::fmod(m_Clock, static_cast<float>(clip->durationMs));
            else { m_Clock = static_cast<float>(clip->durationMs); m_Playing = false; }
        }
        Read_Key();
    }
    Apply_Pose(false);
}
void CBoneAnimationWorkbench::Render()
{
    const auto model = m_Model.lock();
    if (!model) { ImGui::TextDisabled("Select a Character or mount preview first."); return; }
    ImGui::Text("Skeleton: %016llx | %zu bones", static_cast<unsigned long long>(model->Get_SkeletonHash()), model->Get_BoneNames().size());
    ImGui::BeginDisabled(m_PoseDirty);
    if (ImGui::BeginCombo("Authored clip", m_Selected.empty() ? "Create a clip" : m_Selected.c_str()))
    {
        for (const auto& clip : m_Document.clips)
            if (ImGui::Selectable(clip.name.c_str(), clip.name == m_Selected))
            { Stop(); m_Selected = clip.name; m_Clock = 0.f; Read_Key(); m_Override = true; Apply_Pose(); }
        ImGui::EndCombo();
    }
    Render_Source();
    ImGui::BeginDisabled(m_Undo.empty()); if (ImGui::Button("Undo bone edit")) Restore_History(false); ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::BeginDisabled(m_Redo.empty()); if (ImGui::Button("Redo bone edit")) Restore_History(true); ImGui::EndDisabled();
    ImGui::Checkbox("Show skeleton in scene", &m_ShowSkeleton);
    Render_Viewport();
    auto* clip = Selected();
    if (!clip) { ImGui::EndDisabled(); ImGui::TextWrapped("%s", m_Status.c_str()); return; }
    ImGui::BeginDisabled(m_PoseDirty);
    if (ImGui::Button(m_Playing ? "Pause bone preview" : "Play bone preview"))
    { m_Playing = !m_Playing; if (m_Playing && m_Clock >= clip->durationMs) m_Clock = 0.f; m_Override = true; m_PreviewRequested = true; }
    ImGui::EndDisabled();
    ImGui::SameLine(); if (ImGui::Button("Stop bone preview")) Stop();
    ImGui::SameLine(); ImGui::Checkbox("Loop##Bone", &m_Loop);
    if (ImGui::SliderFloat("Bone time", &m_Clock, 0.f, static_cast<float>(clip->durationMs), "%.0f ms"))
    { m_Playing = false; m_Override = true; Read_Key(); Apply_Pose(); }
    if (ImGui::Button("Go to start")) { m_Clock = 0.f; m_Playing = false; m_Override = true; Read_Key(); Apply_Pose(); }
    ImGui::SameLine();
    if (ImGui::Button("Go to end")) { m_Clock = static_cast<float>(clip->durationMs); m_Playing = false; m_Override = true; Read_Key(); Apply_Pose(); }
    if (clip->segments.empty())
    {
        int duration = static_cast<int>(clip->durationMs);
        if (ImGui::DragInt("Retime clip duration (ms)", &duration, 10.f, 1, 60000))
        {
            auto candidate = m_Document;
            if (candidate.Retime_Clip(*model, m_Selected, static_cast<uint32_t>((std::clamp)(duration, 1, 60000)), m_Status))
                Commit(std::move(candidate));
            clip = Selected();
        }
    }
    if (ImGui::CollapsingHeader("Animation stages / source trim", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextWrapped("Stages sample source clips in order. Source start and rate trim the source; duration controls this authored stage. Bone keys use the new clip's clock.");
        for (size_t index = 0u; index < clip->segments.size(); ++index)
        {
            const auto& segment = clip->segments[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Stage %zu: %s", index + 1u, segment.sourceClip.c_str());
            int start = static_cast<int>(segment.sourceStartMs), duration = static_cast<int>(segment.durationMs);
            float rate = segment.playRate; bool loop = segment.loop;
            bool changed = ImGui::DragInt("Source start (ms)", &start, 1.f, 0, 60000);
            changed |= ImGui::DragInt("Stage duration (ms)", &duration, 1.f, 1, 60000);
            changed |= ImGui::DragFloat("Source rate", &rate, .01f, .01f, 16.f);
            changed |= ImGui::Checkbox("Loop source to original end", &loop);
            ImGui::Text("Source out: %.2f ms%s", segment.sourceStartMs + segment.durationMs * segment.playRate, segment.loop ? " (before wrapping)" : "");
            if (changed)
            {
                auto candidate = m_Document;
                auto& edited = *std::find_if(candidate.clips.begin(), candidate.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
                auto& stage = edited.segments[index];
                stage.sourceStartMs = static_cast<uint32_t>((std::clamp)(start, 0, 60000));
                stage.durationMs = static_cast<uint32_t>((std::clamp)(duration, 1, 60000));
                stage.playRate = rate; stage.loop = loop;
                edited.durationMs = 0u; for (const auto& value : edited.segments) edited.durationMs += value.durationMs;
                Commit(std::move(candidate)); clip = Selected(); ImGui::PopID(); break;
            }
            if (ImGui::SmallButton("Remove stage"))
            {
                auto candidate = m_Document;
                auto& edited = *std::find_if(candidate.clips.begin(), candidate.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
                edited.segments.erase(edited.segments.begin() + index);
                if (!edited.segments.empty()) { edited.durationMs = 0u; for (const auto& value : edited.segments) edited.durationMs += value.durationMs; }
                Commit(std::move(candidate)); clip = Selected(); ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        if (!m_Source.empty() && ImGui::Button("Append source as Stage"))
        {
            auto candidate = m_Document;
            auto& edited = *std::find_if(candidate.clips.begin(), candidate.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
            edited.segments.push_back({CEffectEditingSession::New_Id("segment."), m_Source, 1000u});
            edited.durationMs = 0u; for (const auto& value : edited.segments) edited.durationMs += value.durationMs;
            Commit(std::move(candidate)); clip = Selected();
        }
    }
    ImGui::EndDisabled();
    ImGui::SeparatorText("Bone / weapon socket keyframes");
    Render_Keys();
    ImGui::Separator();
    ImGui::BeginDisabled(!m_Dirty || m_PoseDirty);
    if (ImGui::Button("Save Bone Clips"))
        if (m_Document.Save(*model, m_Status)) { m_Dirty = false; m_Installed = true; m_Undo.clear(); m_Redo.clear(); }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(Is_Dirty());
    if (ImGui::Button("Reload Bone Clips"))
    {
        CBoneAnimationDocument document;
        if (document.Load(m_Document.asset, *model, m_Status) && document.Install(*model, m_Status))
        { Stop(); m_Document = std::move(document); m_Installed = true; m_Undo.clear(); m_Redo.clear();
            if (!Selected()) m_Selected = m_Document.clips.empty() ? std::string{} : m_Document.clips.front().name;
            m_Clock = 0.f; Read_Key(); }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Discard Bone Draft"))
    {
        CBoneAnimationDocument document;
        if (document.Load(m_Document.asset, *model, m_Status))
        { Stop(); m_Document = std::move(document); m_Dirty = false; m_Undo.clear(); m_Redo.clear();
            if (!Selected()) m_Selected = m_Document.clips.empty() ? std::string{} : m_Document.clips.front().name;
            m_Clock = 0.f; Read_Key(); }
    }
    ImGui::TextWrapped("Save Bone Clips keeps authored animations and refreshes Animation Resources. Stop bone preview before playing an action sequence. Bindings decide when clips play; Server movement remains authoritative.");
    ImGui::TextWrapped("%s", m_Status.c_str());
}
void CBoneAnimationWorkbench::Render_Keys()
{
    const auto model = m_Model.lock(); auto* clip = Selected();
    if (!model || !clip) return;
    ImGui::InputTextWithHint("##BoneSearch", "Search bone or weapon socket", m_BoneSearch, sizeof(m_BoneSearch));
    const auto names = model->Get_BoneNames();
    ImGui::BeginDisabled(m_PoseDirty);
    if (ImGui::BeginChild("Bone hierarchy", ImVec2(0.f, 160.f), ImGuiChildFlags_Borders))
    {
        for (uint32_t index = 0u; index < names.size(); ++index)
        {
            const auto& name = names[index];
            if (m_BoneSearch[0] && name.find(m_BoneSearch) == std::string::npos) continue;
            int depth = 0, parent = model->Get_BoneParentIndex(index);
            while (parent >= 0 && depth < 16) { ++depth; parent = model->Get_BoneParentIndex(static_cast<uint32_t>(parent)); }
            const float indent = m_BoneSearch[0] ? 0.f : depth * 8.f;
            if (indent > 0.f) ImGui::Indent(indent);
            if (ImGui::Selectable(name.c_str(), m_Bone == name)) Select_Bone(name);
            if (indent > 0.f) ImGui::Unindent(indent);
        }
    }
    ImGui::EndChild();
    ImGui::EndDisabled();
    ImGui::Text("Selected bone: %s", m_Bone.empty() ? "Select from the hierarchy" : m_Bone.c_str());
    ImGui::BeginDisabled(m_Bone.empty());
    bool changed = ImGui::DragFloat3("Local translation", &m_KeyPosition.x, .01f);
    changed |= ImGui::DragFloat3("Local rotation (degrees)", &m_KeyDegrees.x, .5f);
    changed |= ImGui::DragFloat3("Local scale", &m_KeyScale.x, .01f, .0001f, 100.f);
    if (changed) { m_PoseDirty = true; m_Playing = false; m_Override = true; Apply_Pose(); }
    ImGui::TextWrapped("Live pose preview. Set key commits this pose at the cursor; Cancel pose restores its saved key values. Translation uses installed bone units; rotation and scale are bone-local deltas.");
    if (m_PoseDirty) ImGui::TextUnformatted("Unkeyed pose: Set key or Cancel pose before Save / Play.");
    if (ImGui::Button("Cancel pose")) { Read_Key(); m_Override = true; Apply_Pose(); }
    ImGui::SameLine();
    if (ImGui::Button("Reset delta"))
    { m_KeyPosition = {}; m_KeyDegrees = {}; m_KeyScale = {1.f, 1.f, 1.f}; m_PoseDirty = true; m_Playing = false; m_Override = true; Apply_Pose(); }
    if (ImGui::Button("Set key at cursor"))
    {
        auto candidate = m_Document;
        auto& edited = *std::find_if(candidate.clips.begin(), candidate.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
        BONE_ANIMATION_KEY key; key.timeMs = static_cast<uint32_t>(std::round(m_Clock)); key.position = m_KeyPosition; key.scale = m_KeyScale;
        XMStoreFloat4(&key.rotation, XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(
            XMConvertToRadians(m_KeyDegrees.x), XMConvertToRadians(m_KeyDegrees.y), XMConvertToRadians(m_KeyDegrees.z))));
        PutKey(edited, m_Bone, key); Commit(std::move(candidate)); clip = Selected();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_PoseDirty);
    if (ImGui::Button("Copy start key to end"))
    {
        auto candidate = m_Document;
        auto& edited = *std::find_if(candidate.clips.begin(), candidate.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
        auto key = KeyAt(edited, m_Bone, 0.f); key.timeMs = edited.durationMs;
        PutKey(edited, m_Bone, key); Commit(std::move(candidate)); clip = Selected();
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    uint32_t deleteTime = UINT32_MAX;
    ImGui::BeginDisabled(m_PoseDirty);
    for (const auto& track : clip->tracks)
    {
        if (track.bone != m_Bone) continue;
        for (size_t index = 0u; index < track.keys.size(); ++index)
        {
            const auto& key = track.keys[index]; ImGui::PushID(static_cast<int>(index));
            if (ImGui::Selectable((std::to_string(key.timeMs) + " ms").c_str(), std::abs(m_Clock - key.timeMs) < .5f))
            { m_Clock = static_cast<float>(key.timeMs); Read_Key(); m_Playing = false; m_Override = true; Apply_Pose(); }
            ImGui::SameLine();
            if (ImGui::SmallButton("Delete key"))
                deleteTime = key.timeMs;
            ImGui::PopID();
        }
    }
    ImGui::EndDisabled();
    if (deleteTime != UINT32_MAX)
    {
        auto candidate = m_Document;
        auto& edited = *std::find_if(candidate.clips.begin(), candidate.clips.end(), [&](const auto& c) { return c.name == m_Selected; });
        for (auto& track : edited.tracks) if (track.bone == m_Bone)
            track.keys.erase(std::remove_if(track.keys.begin(), track.keys.end(), [&](const auto& key) { return key.timeMs == deleteTime; }), track.keys.end());
        edited.tracks.erase(std::remove_if(edited.tracks.begin(), edited.tracks.end(), [](const auto& track) { return track.keys.empty(); }), edited.tracks.end());
        Commit(std::move(candidate));
    }
}
}
