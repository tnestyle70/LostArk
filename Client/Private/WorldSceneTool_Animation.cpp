#include "imgui.h"
#include "WorldSceneTool.h"

#ifdef _DEBUG
#include "MapAuthoringHost.h"

#include <algorithm>
#include <cmath>
#include <sstream>

NS_BEGIN(Client)

bool CWorldSceneTool::Begin_DeployPreview()
{
    if (m_bAnimationOwned) return true;
    if (m_bSelfMotionOwned)
    {
        m_Status = "Stop and restore map motions before starting a Deploy preview.";
        return false;
    }
    auto* host = Host();
    if (!host || !m_DeploySelection || !m_bDeploySelected) return false;
    std::string reason;
    if (!host->Can_ChangeMapAuthoringStructure(reason))
    {
        m_Status = reason;
        return false;
    }
    auto* runtime = host->Get_MapAuthoringDeployRuntime();
    auto object = runtime ? runtime->Find(m_DeploySelection->runtimePlacementId) : nullptr;
    if (!object || object->Is_AnimationAuthoringPreviewActive())
    {
        m_Status = "The selected Deploy object is missing or already belongs to another animation preview.";
        return false;
    }
    const auto clips = object->Get_AnimationClips();
    const auto selected = std::find_if(clips.begin(), clips.end(), [this](const auto& clip)
    { return clip.name == m_AnimationClip; });
    float3_t position{};
    float4_t rotation{};
    float uniformScale = 1.f;
    if (!object->Try_GetRenderedRootPose(position, rotation, uniformScale) || !object->Begin_AnimationAuthoringPreview())
    {
        m_Status = "The selected Deploy object could not begin its animation preview.";
        return false;
    }
    m_AnimationObject = object;
    m_AnimationPlacementId = m_DeploySelection->runtimePlacementId;
    m_AnimationAreaId = m_AreaId;
    m_AnimationLevelIndex = m_LevelIndex;
    m_bAnimationHasClip = !clips.empty();
    m_AnimationClip = clips.empty() ? std::string{} : (selected == clips.end() ? clips.front() : *selected).name;
    m_AnimationDuration = clips.empty() ? 0.f : (selected == clips.end() ? clips.front() : *selected).durationSeconds;
    m_AnimationTimeSeconds = 0.f;
    m_bAnimationOwned = true;
    m_bAnimationPlaying = false;
    m_DeployPreviewPosition = position;
    m_DeployPreviewBaseRotation = rotation;
    m_DeployPreviewRotationDegrees = {};
    m_bDeployPoseDirty = false;
    m_DeployPreviewUniformScale = uniformScale;
    m_DeployPreviewOpacity = object->Get_SurfacePresentation().fOpacity;
    m_bRevealHidden = false;
    if (m_bAnimationHasClip && !object->Sample_AnimationAuthoringPreview(m_AnimationClip, 0.f, m_bAnimationLoop, false))
    {
        Stop_AnimationPreview();
        m_Status = "The selected clip could not be sampled; its previous cursor was restored.";
        return false;
    }
    m_Status = m_bAnimationHasClip ? "Deploy animation preview ready." : "Deploy pose preview ready.";
    return true;
}

void CWorldSceneTool::Stop_AnimationPreview()
{
    if (m_bAnimationOwned)
    {
        if (auto object = m_AnimationObject.lock())
        {
            if (object->Is_AnimationAuthoringPreviewActive())
            {
                object->End_AnimationAuthoringPreview();
            }
        }
    }
    m_AnimationObject.reset();
    m_AnimationPlacementId = 0;
    m_AnimationAreaId.clear();
    m_AnimationLevelIndex = UINT32_MAX;
    m_bAnimationOwned = false;
    m_bAnimationPlaying = false;
    m_bAnimationHasClip = false;
    m_bDeployPoseDirty = false;
    m_AnimationTimeSeconds = 0.f;
    m_AnimationDuration = 0.f;
}

bool CWorldSceneTool::Begin_SelfMotionPreview()
{
    if (m_bSelfMotionOwned) return true;
    if (m_bAnimationOwned)
    {
        m_Status = "Stop and restore the Deploy preview before starting map motions.";
        return false;
    }
    auto* host = Host();
    if (!host) return false;
    std::string reason;
    if (!host->Can_ChangeMapAuthoringStructure(reason))
    {
        m_Status = reason;
        return false;
    }
    auto& runtime = host->Get_MapAuthoringRuntime();
    if (!runtime.Get_SelfMotionCount())
    {
        m_Status = "This Area has no bound map self-motions.";
        return false;
    }
    m_SavedSelfMotionTime = runtime.Debug_GetSelfMotionTime();
    m_SavedSelfMotionRate = runtime.Debug_GetSelfMotionRate();
    m_bSavedSelfMotionPaused = runtime.Debug_IsSelfMotionPaused();
    m_SelfMotionAreaId = runtime.Get_Catalog().Get_AreaId();
    m_SelfMotionLevelIndex = host->Get_MapAuthoringLevelIndex();
    m_SelfMotionGeneration = runtime.Debug_GetRuntimeGeneration();
    m_pSelfMotionHost = host;
    m_SelfMotionTime = m_SavedSelfMotionTime;
    m_SelfMotionRate = 1.f;
    runtime.Debug_SetSelfMotionPlayback(true, m_SavedSelfMotionRate);
    m_bSelfMotionOwned = true;
    m_bSelfMotionPlaying = false;
    m_Status = "Map self-motion preview ready.";
    return true;
}

void CWorldSceneTool::Stop_SelfMotionPreview()
{
    if (m_bSelfMotionOwned)
    {
        auto* host = Host();
        if (host && host == m_pSelfMotionHost && host->Get_MapAuthoringLevelIndex() == m_SelfMotionLevelIndex)
        {
            auto& runtime = host->Get_MapAuthoringRuntime();
            if (runtime.Get_Catalog().Get_AreaId() == m_SelfMotionAreaId &&
                runtime.Debug_GetRuntimeGeneration() == m_SelfMotionGeneration)
            {
                std::string reason;
                if (host->Can_ChangeMapAuthoringStructure(reason))
                    runtime.Debug_SeekSelfMotions(m_SavedSelfMotionTime);
                runtime.Debug_SetSelfMotionPlayback(m_bSavedSelfMotionPaused, m_SavedSelfMotionRate);
            }
        }
    }
    m_bSelfMotionOwned = false;
    m_bSelfMotionPlaying = false;
    m_SelfMotionAreaId.clear();
    m_SelfMotionLevelIndex = UINT32_MAX;
    m_SelfMotionGeneration = UINT64_MAX;
    m_pSelfMotionHost = nullptr;
}

void CWorldSceneTool::Update_AnimationPreview(const float deltaSeconds)
{
    const float delta = std::isfinite(deltaSeconds) && deltaSeconds > 0.f ? deltaSeconds : 0.f;
    auto* host = Host();
    if (m_bSelfMotionOwned)
    {
        if (!host || host != m_pSelfMotionHost || host->Get_MapAuthoringLevelIndex() != m_SelfMotionLevelIndex ||
            host->Get_MapAuthoringRuntime().Get_Catalog().Get_AreaId() != m_SelfMotionAreaId ||
            host->Get_MapAuthoringRuntime().Debug_GetRuntimeGeneration() != m_SelfMotionGeneration)
            Stop_SelfMotionPreview();
        else
        {
            std::string reason;
            if (!host->Can_ChangeMapAuthoringStructure(reason))
            {
                Stop_SelfMotionPreview();
                m_Status = reason.empty() ? "Map motion preview ended because another presentation owns the live map." : reason;
            }
            else
            {
                if (m_bSelfMotionPlaying)
                    m_SelfMotionTime = std::fmod(m_SelfMotionTime + delta * m_SelfMotionRate,
                        CMapPlacementRuntime::SELF_MOTION_WRAP_SECONDS);
                host->Get_MapAuthoringRuntime().Debug_SeekSelfMotions(m_SelfMotionTime);
            }
        }
    }
    if (!m_bAnimationOwned) return;
    auto object = m_AnimationObject.lock();
    auto* runtime = host ? host->Get_MapAuthoringDeployRuntime() : nullptr;
    if (!host || host->Get_MapAuthoringLevelIndex() != m_AnimationLevelIndex ||
        host->Get_MapAuthoringCatalog().Get_AreaId() != m_AnimationAreaId || !runtime ||
        !object || runtime->Find(m_AnimationPlacementId) != object ||
        !object->Is_AnimationAuthoringPreviewActive())
    {
        Stop_AnimationPreview();
        return;
    }
    if (m_bAnimationPlaying && m_bAnimationHasClip)
    {
        m_AnimationTimeSeconds += delta * m_AnimationRate;
        if (m_AnimationDuration > 0.f)
        {
            if (m_bAnimationLoop)
                m_AnimationTimeSeconds = std::fmod(m_AnimationTimeSeconds, m_AnimationDuration);
            else if (m_AnimationTimeSeconds >= m_AnimationDuration)
            {
                m_AnimationTimeSeconds = m_AnimationDuration;
                m_bAnimationPlaying = false;
            }
        }
        else m_bAnimationPlaying = false;
    }
    const float normalized = m_AnimationDuration > 0.f ?
        (std::clamp)(m_AnimationTimeSeconds / m_AnimationDuration, 0.f, 1.f) : 0.f;
    if (m_bAnimationHasClip && !object->Sample_AnimationAuthoringPreview(m_AnimationClip, normalized, m_bAnimationLoop, m_bRevealHidden))
    {
        Stop_AnimationPreview();
        m_Status = "Animation sampling failed; the borrowed preview was restored.";
        return;
    }
    if (m_bDeployPoseDirty)
    {
        float4_t rotation{};
        const auto offset = XMQuaternionRotationRollPitchYaw(
            XMConvertToRadians(m_DeployPreviewRotationDegrees.x),
            XMConvertToRadians(m_DeployPreviewRotationDegrees.y),
            XMConvertToRadians(m_DeployPreviewRotationDegrees.z));
        XMStoreFloat4(&rotation, XMQuaternionNormalize(XMQuaternionMultiply(
            XMLoadFloat4(&m_DeployPreviewBaseRotation), offset)));
        if (!object->Apply_AnimationAuthoringPose(m_DeployPreviewPosition, rotation, m_DeployPreviewUniformScale))
        {
            Stop_AnimationPreview();
            m_Status = "Preview root pose was rejected; the previous pose was restored.";
            return;
        }
    }
    if (!object->Apply_AnimationAuthoringVisibility(m_DeployPreviewOpacity, m_bRevealHidden))
    {
        Stop_AnimationPreview();
        m_Status = "Preview visibility was rejected; the borrowed preview was restored.";
    }
}

void CWorldSceneTool::Render_AnimationControls()
{
    auto* host = Host();
    if (!host || !ImGui::CollapsingHeader("Map self-motions", ImGuiTreeNodeFlags_DefaultOpen)) return;
    auto& runtime = host->Get_MapAuthoringRuntime();
    ImGui::Text("Bound motions: %zu", runtime.Get_SelfMotionCount());
    if (!m_bSelfMotionOwned)
    {
        ImGui::BeginDisabled(runtime.Get_SelfMotionCount() == 0 || m_Edit.Is_Publishing() || m_bAnimationOwned);
        if (ImGui::Button("Preview map motions"))
        {
            m_bInteraction = true;
            (void)Begin_SelfMotionPreview();
        }
        ImGui::EndDisabled();
        if (m_bAnimationOwned) ImGui::TextDisabled("Stop the Deploy preview before previewing map motions.");
        return;
    }
    if (ImGui::Button(m_bSelfMotionPlaying ? "Pause motions" : "Play motions"))
    {
        m_bSelfMotionPlaying = !m_bSelfMotionPlaying;
        m_bInteraction = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop and restore motions"))
    {
        Stop_SelfMotionPreview();
        m_bInteraction = true;
        return;
    }
    if (ImGui::SliderFloat("Motion time (seconds)", &m_SelfMotionTime, 0.f,
        CMapPlacementRuntime::SELF_MOTION_WRAP_SECONDS, "%.3f"))
    {
        m_bInteraction = true;
        Update_AnimationPreview(0.f);
    }
    if (ImGui::SliderFloat("Motion speed", &m_SelfMotionRate, 0.05f, 8.f, "%.2fx"))
        m_bInteraction = true;
}

void CWorldSceneTool::Render_DeployDetails()
{
    auto* host = Host();
    auto* runtime = host ? host->Get_MapAuthoringDeployRuntime() : nullptr;
    auto object = runtime && m_DeploySelection ? runtime->Find(m_DeploySelection->runtimePlacementId) : nullptr;
    if (!object)
    {
        ImGui::TextUnformatted("The selected Deploy object is no longer loaded.");
        return;
    }
    const auto clips = object->Get_AnimationClips();
    const auto& selected = *m_DeploySelection;
    ImGui::Text("Deploy placement: %llu", static_cast<unsigned long long>(object->Get_RuntimePlacementId()));
    ImGui::TextWrapped("Area: %s\nSource: %s\nAsset: %s\nWModel: %s",
        selected.areaId.c_str(), selected.sourcePlacementId.c_str(), selected.assetId.c_str(), selected.modelAssetId.c_str());
    if (selected.meshIndex == UINT32_MAX)
        ImGui::TextUnformatted("Placement selected. Use Pick in world for the exact mesh and material.");
    else
        ImGui::TextWrapped("Mesh %u | Material: %s", selected.meshIndex, selected.materialName.c_str());
    ImGui::Text("Hit XYZ: %.4f / %.4f / %.4f m", selected.hitPosition.x, selected.hitPosition.y, selected.hitPosition.z);
    if (ImGui::Button("Copy Deploy source selection"))
    {
        std::ostringstream text;
        text << "Area: " << selected.areaId << "\nPlacement: " << selected.runtimePlacementId
            << "\nSource: " << selected.sourcePlacementId << "\nAsset: " << selected.assetId
            << "\nWModel: " << selected.modelAssetId;
        if (selected.meshIndex == UINT32_MAX) text << "\nMesh: placement selected; exact mesh not picked";
        else text << "\nMesh: " << selected.meshIndex << "\nMaterial: " << selected.materialName;
        text << "\nHit XYZ (metres): " << selected.hitPosition.x << " / " << selected.hitPosition.y << " / " << selected.hitPosition.z;
        ImGui::SetClipboardText(text.str().c_str());
        m_bInteraction = true;
    }
    ImGui::Text("Animation clips: %zu", clips.size());
    if (clips.empty())
    {
        ImGui::TextUnformatted(object->Is_AnimBindPoseOnly() ?
            "This source model has a bind pose only; root pose can be previewed." : "This source model has no animation clips; root pose can be previewed.");
    }
    if (!clips.empty() && std::none_of(clips.begin(), clips.end(), [this](const auto& clip) { return clip.name == m_AnimationClip; }))
        m_AnimationClip = clips.front().name;
    if (!clips.empty() && ImGui::BeginCombo("Clip", m_AnimationClip.c_str()))
    {
        for (const auto& clip : clips)
        {
            const bool selected = clip.name == m_AnimationClip;
            if (ImGui::Selectable(clip.name.c_str(), selected))
            {
                m_AnimationClip = clip.name;
                m_AnimationDuration = clip.durationSeconds;
                m_AnimationTimeSeconds = 0.f;
                m_bInteraction = true;
                if (m_bAnimationOwned) Update_AnimationPreview(0.f);
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (!m_bAnimationOwned)
    {
        ImGui::BeginDisabled(m_Edit.Is_Publishing() || m_bSelfMotionOwned);
        if (ImGui::Button(clips.empty() ? "Preview Deploy pose" : "Preview selected clip"))
        {
            m_bInteraction = true;
            (void)Begin_DeployPreview();
        }
        ImGui::EndDisabled();
        if (m_bSelfMotionOwned) ImGui::TextDisabled("Stop map motions before previewing this Deploy object.");
        return;
    }
    if (m_bAnimationHasClip && ImGui::Button(m_bAnimationPlaying ? "Pause clip" : "Play clip"))
    {
        m_bAnimationPlaying = !m_bAnimationPlaying;
        m_bInteraction = true;
    }
    if (m_bAnimationHasClip) ImGui::SameLine();
    if (ImGui::Button("Stop and restore Deploy preview"))
    {
        Stop_AnimationPreview();
        m_bInteraction = true;
        return;
    }
    if (m_bAnimationHasClip && ImGui::Checkbox("Loop clip", &m_bAnimationLoop))
    {
        m_bInteraction = true;
        Update_AnimationPreview(0.f);
    }
    if (m_bAnimationHasClip && ImGui::SliderFloat("Clip time (seconds)", &m_AnimationTimeSeconds, 0.f,
        (std::max)(m_AnimationDuration, 0.0001f), "%.3f"))
    {
        m_bInteraction = true;
        Update_AnimationPreview(0.f);
    }
    if (m_bAnimationHasClip && ImGui::SliderFloat("Clip speed", &m_AnimationRate, 0.05f, 8.f, "%.2fx"))
        m_bInteraction = true;
    if (ImGui::Checkbox("Reveal hidden source object", &m_bRevealHidden))
    {
        m_bInteraction = true;
        Update_AnimationPreview(0.f);
    }
    bool poseChanged = ImGui::DragFloat3("Preview position (metres)", &m_DeployPreviewPosition.x, 0.01f);
    poseChanged |= ImGui::DragFloat3("Preview rotation offset (degrees)", &m_DeployPreviewRotationDegrees.x, 0.25f);
    poseChanged |= ImGui::DragFloat("Preview uniform scale", &m_DeployPreviewUniformScale,
        0.005f, 0.00001f, 1000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
    if (poseChanged)
    {
        m_bDeployPoseDirty = true;
        m_bInteraction = true;
        Update_AnimationPreview(0.f);
    }
    if (ImGui::SliderFloat("Preview opacity", &m_DeployPreviewOpacity, 0.f, 1.f))
    {
        m_bInteraction = true;
        Update_AnimationPreview(0.f);
    }
}

NS_END
#endif
