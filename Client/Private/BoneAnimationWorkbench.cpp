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
BONE_ANIMATION_CLIP* CBoneAnimationWorkbench::Selected()
{
    const auto found = std::find_if(m_Document.clips.begin(), m_Document.clips.end(), [&](const auto& clip) { return clip.name == m_Selected; });
    return found == m_Document.clips.end() ? nullptr : &*found;
}
bool CBoneAnimationWorkbench::Select(const std::string& asset, const std::shared_ptr<Engine::CModel>& model)
{
    if (!model) return false;
    if (m_Document.asset == asset && m_Model.lock() == model) return true;
    if (m_Dirty) { m_Status = "Save bone clip changes before changing this skeleton"; return false; }
    CBoneAnimationDocument document;
    if (!document.Load(asset, *model, m_Status)) return false;
    Stop(); m_Model = model; m_Document = std::move(document); m_Selected.clear(); m_Bone.clear(); m_Source.clear();
    if (!m_Document.clips.empty()) m_Selected = m_Document.clips.front().name;
    return true;
}
void CBoneAnimationWorkbench::Stop()
{
    const bool ownedPose = m_Playing || m_Override;
    m_Playing = m_Override = false;
    if (ownedPose) if (const auto model = m_Model.lock()) model->Set_AnimPaused(true);
}
void CBoneAnimationWorkbench::Apply_Pose()
{
    const auto model = m_Model.lock(); const auto* clip = Selected();
    if (!model || !clip) return;
    std::vector<float4x4_t> pose;
    if (!m_Document.Sample(*model, clip->name, m_Clock, pose, m_Status)) { Stop(); return; }
    for (uint32_t bone = 0u; bone < pose.size(); ++bone)
        if (!model->Set_BoneLocalMatrix(bone, XMLoadFloat4x4(&pose[bone]))) { Stop(); return; }
    model->Refresh_BoneCombinedMatrices(); model->Set_AnimPaused(true);
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
    }
    Apply_Pose();
}
void CBoneAnimationWorkbench::Render()
{
    const auto model = m_Model.lock();
    if (!model) { ImGui::TextDisabled("Select a Character or mount preview first."); return; }
    ImGui::Text("Skeleton: %016llx | %zu bones", static_cast<unsigned long long>(model->Get_SkeletonHash()), model->Get_BoneNames().size());
    if (ImGui::BeginCombo("Authored clip", m_Selected.empty() ? "Create a clip" : m_Selected.c_str()))
    {
        for (const auto& clip : m_Document.clips)
            if (ImGui::Selectable(clip.name.c_str(), clip.name == m_Selected)) { Stop(); m_Selected = clip.name; m_Clock = 0.f; }
        ImGui::EndCombo();
    }
    ImGui::InputText("New clip name", m_NewName, sizeof(m_NewName));
    if (ImGui::BeginCombo("Source animation", m_Source.empty() ? "Rest pose" : m_Source.c_str()))
    {
        if (ImGui::Selectable("Rest pose", m_Source.empty())) m_Source.clear();
        for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
        {
            const char* name = model->Get_AnimationName(index);
            if (name && ImGui::Selectable(name, m_Source == name)) m_Source = name;
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Create authored clip"))
    {
        auto candidate = m_Document;
        BONE_ANIMATION_CLIP clip; clip.name = m_NewName;
        if (!m_Source.empty())
        {
            for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
                if (m_Source == model->Get_AnimationName(index))
                {
                    float position, duration;
                    const float rate = model->Get_AnimationTickPerSecond(index);
                    if (model->Get_AnimationProgress(index, position, duration) && rate > 0.f)
                        clip.durationMs = static_cast<uint32_t>((std::clamp)(std::round(duration / rate * 1000.f), 1.f, 60000.f));
                }
            clip.segments.push_back({CEffectEditingSession::New_Id("segment."), m_Source, clip.durationMs});
        }
        candidate.clips.push_back(clip);
        std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
        if (candidate.Compile(*model, compiled, m_Status))
        { Stop(); m_Document = std::move(candidate); m_Selected = clip.name; m_Dirty = true; m_Clock = 0.f; }
    }
    auto* clip = Selected();
    if (!clip) { ImGui::TextWrapped("%s", m_Status.c_str()); return; }
    if (ImGui::Button(m_Playing ? "Pause bone preview" : "Play bone preview")) { m_Playing = !m_Playing; m_Override = true; }
    ImGui::SameLine(); if (ImGui::Button("Stop bone preview")) Stop();
    ImGui::SameLine(); ImGui::Checkbox("Loop##Bone", &m_Loop);
    if (ImGui::SliderFloat("Bone time", &m_Clock, 0.f, static_cast<float>(clip->durationMs), "%.0f ms"))
    { m_Playing = false; m_Override = true; Apply_Pose(); }
    if (clip->segments.empty())
    {
        int duration = static_cast<int>(clip->durationMs);
        if (ImGui::DragInt("Rest clip duration", &duration, 10.f, 1, 60000))
        { clip->durationMs = static_cast<uint32_t>((std::clamp)(duration, 1, 60000)); m_Dirty = true; }
    }
    if (ImGui::CollapsingHeader("Animation stages / source trim", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextWrapped("Stages sample source clips in order. Source start and rate trim the source; duration controls this authored stage. Bone keys use the new clip's clock.");
        for (size_t index = 0u; index < clip->segments.size(); ++index)
        {
            auto& segment = clip->segments[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Stage %zu: %s", index + 1u, segment.sourceClip.c_str());
            int start = static_cast<int>(segment.sourceStartMs), duration = static_cast<int>(segment.durationMs);
            bool changed = ImGui::DragInt("Source start (ms)", &start, 1.f, 0, 60000);
            changed |= ImGui::DragInt("Stage duration (ms)", &duration, 1.f, 1, 60000);
            changed |= ImGui::DragFloat("Source rate", &segment.playRate, .01f, .01f, 16.f);
            changed |= ImGui::Checkbox("Loop source", &segment.loop);
            if (changed)
            {
                segment.sourceStartMs = static_cast<uint32_t>((std::clamp)(start, 0, 60000));
                segment.durationMs = static_cast<uint32_t>((std::clamp)(duration, 1, 60000));
                clip->durationMs = 0u; for (const auto& value : clip->segments) clip->durationMs += value.durationMs;
                m_Dirty = true;
            }
            if (ImGui::SmallButton("Remove stage"))
            {
                clip->segments.erase(clip->segments.begin() + index);
                if (!clip->segments.empty()) { clip->durationMs = 0u; for (const auto& value : clip->segments) clip->durationMs += value.durationMs; }
                m_Clock = (std::min)(m_Clock, static_cast<float>(clip->durationMs));
                m_Dirty = true; ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        if (!m_Source.empty() && ImGui::Button("Append source as Stage"))
        {
            clip->segments.push_back({CEffectEditingSession::New_Id("segment."), m_Source, 1000u});
            clip->durationMs = 0u; for (const auto& value : clip->segments) clip->durationMs += value.durationMs;
            m_Dirty = true;
        }
    }
    ImGui::SeparatorText("Bone / weapon socket keyframes");
    ImGui::InputTextWithHint("##BoneSearch", "Search bone or weapon socket", m_BoneSearch, sizeof(m_BoneSearch));
    if (ImGui::BeginCombo("Bone / anchor", m_Bone.empty() ? "Select bone" : m_Bone.c_str()))
    {
        for (const auto& name : model->Get_BoneNames())
            if ((!m_BoneSearch[0] || name.find(m_BoneSearch) != std::string::npos) && ImGui::Selectable(name.c_str(), m_Bone == name)) m_Bone = name;
        ImGui::EndCombo();
    }
    ImGui::DragFloat3("Local translation", &m_KeyPosition.x, .01f);
    ImGui::DragFloat3("Local rotation (degrees)", &m_KeyDegrees.x, .5f);
    ImGui::DragFloat3("Local scale", &m_KeyScale.x, .01f, .0001f, 100.f);
    ImGui::TextWrapped("Translation uses this installed bone's local units. Rotation is an additive local delta. Select b_wp / weapon socket bones to tune the held weapon's rotation without rotating the whole actor.");
    ImGui::BeginDisabled(m_Bone.empty());
    if (ImGui::Button("Set key at cursor"))
    {
        auto track = std::find_if(clip->tracks.begin(), clip->tracks.end(), [&](const auto& value) { return value.bone == m_Bone; });
        if (track == clip->tracks.end()) { clip->tracks.push_back({m_Bone, {}}); track = clip->tracks.end() - 1; }
        BONE_ANIMATION_KEY key; key.timeMs = static_cast<uint32_t>(std::round(m_Clock)); key.position = m_KeyPosition; key.scale = m_KeyScale;
        XMStoreFloat4(&key.rotation, XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(
            XMConvertToRadians(m_KeyDegrees.x), XMConvertToRadians(m_KeyDegrees.y), XMConvertToRadians(m_KeyDegrees.z))));
        auto found = std::find_if(track->keys.begin(), track->keys.end(), [&](const auto& value) { return value.timeMs == key.timeMs; });
        if (found == track->keys.end()) track->keys.push_back(key); else *found = key;
        std::sort(track->keys.begin(), track->keys.end(), [](const auto& a, const auto& b) { return a.timeMs < b.timeMs; });
        m_Dirty = true; m_Override = true; m_Playing = false; Apply_Pose();
    }
    ImGui::EndDisabled();
    for (auto& track : clip->tracks)
    {
        if (track.bone != m_Bone) continue;
        for (size_t index = 0u; index < track.keys.size(); ++index)
        {
            const auto& key = track.keys[index]; ImGui::PushID(static_cast<int>(index));
            if (ImGui::Selectable((std::to_string(key.timeMs) + " ms").c_str(), std::abs(m_Clock - key.timeMs) < .5f))
            { m_Clock = static_cast<float>(key.timeMs); m_KeyPosition = key.position; m_KeyScale = key.scale;
                float4x4_t rotation; XMStoreFloat4x4(&rotation, XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotation)));
                const float pitch = std::asin((std::clamp)(-rotation._32, -1.f, 1.f));
                if (std::abs(std::cos(pitch)) > .0001f)
                    m_KeyDegrees = {XMConvertToDegrees(pitch), XMConvertToDegrees(std::atan2(rotation._31, rotation._33)), XMConvertToDegrees(std::atan2(rotation._12, rotation._22))};
                else m_KeyDegrees = {XMConvertToDegrees(pitch), XMConvertToDegrees(std::atan2(-rotation._13, rotation._11)), 0.f};
                m_Playing = false; m_Override = true; Apply_Pose(); }
            ImGui::SameLine();
            if (ImGui::SmallButton("Delete key"))
            { track.keys.erase(track.keys.begin() + index); m_Dirty = true; ImGui::PopID(); break; }
            ImGui::PopID();
        }
    }
    clip->tracks.erase(std::remove_if(clip->tracks.begin(), clip->tracks.end(), [](const auto& track) { return track.keys.empty(); }), clip->tracks.end());
    ImGui::Separator();
    ImGui::BeginDisabled(!m_Dirty);
    if (ImGui::Button("Save Bone Clips"))
        if (m_Document.Save(*model, m_Status)) { m_Dirty = false; m_Installed = true; }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Dirty);
    if (ImGui::Button("Reload Bone Clips"))
    {
        CBoneAnimationDocument document;
        if (document.Load(m_Document.asset, *model, m_Status) && document.Install(*model, m_Status))
        { Stop(); m_Document = std::move(document); m_Installed = true; }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Discard Bone Draft"))
    {
        CBoneAnimationDocument document;
        if (document.Load(m_Document.asset, *model, m_Status))
        { Stop(); m_Document = std::move(document); m_Dirty = false; }
    }
    ImGui::TextWrapped("Saved authored.* clips appear in Animation Resources and use the existing CModel animation playback. Sequence bindings select when they play; Server movement and combat stay authoritative.");
    ImGui::TextWrapped("%s", m_Status.c_str());
}
}
