#include "imgui.h"

#include "EffectCompositionWorkbench.h"
#include "Effect_Tool_V2.h"
#include "CompositionTimeline.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

namespace Client
{
namespace
{
const char* Label(const std::string& name, const std::string& id) { return name.empty() ? id.c_str() : name.c_str(); }
bool Edit_Name(const char* label, std::string& name)
{
    char buffer[512]{};
    std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
    if (!ImGui::InputText(label, buffer, sizeof(buffer))) return false;
    name = buffer; return true;
}
bool Edit_Ms(const char* label, uint32_t& value, int minimum = 0)
{
    int edited = static_cast<int>(value);
    if (!ImGui::DragInt(label, &edited, 10.f, minimum, 600000, "%d ms")) return false;
    value = static_cast<uint32_t>((std::clamp)(edited, minimum, 600000)); return true;
}
}

CEffectCompositionWorkbench::CEffectCompositionWorkbench(ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context, CEffect_Tool_V2& resourceEditor)
    : m_Device(std::move(device)), m_Context(std::move(context)), m_ResourceEditor(resourceEditor)
{
    XMStoreFloat4x4(&m_WorldPivot, XMMatrixIdentity());
}
CEffectCompositionWorkbench::~CEffectCompositionWorkbench() { Stop(); }

void CEffectCompositionWorkbench::Deactivate()
{
    Stop();
    Reset_AnchorHistory();
}
void CEffectCompositionWorkbench::Stop()
{
    if (m_Handle) CEffectV2Runtime::Stop_Group(m_Handle);
    m_Handle = 0; m_Playing = false; m_Paused = false;
    m_Model.Stop(); m_PlaySnapshot.reset();
}
void CEffectCompositionWorkbench::Edited()
{
    m_Edit.Touch(); m_PreviewDirty = true;
}
uint32_t CEffectCompositionWorkbench::DurationMs() const
{
    uint32_t effectEnd = m_Edit.Group().iDurationMs;
    if (effectEnd == 0u)
        for (const auto& child : m_Edit.Group().Children)
        {
            uint32_t duration = child.iDurationMs; float tail = 0.f;
            if (const auto it = m_Edit.Leaves().find(child.strEffectId); it != m_Edit.Leaves().end())
            {
                const auto& document = it->second.document; const auto& params = document.Desc.Params;
                const float rate = (std::max)(.001f, params.fPlayRate);
                if (!duration) duration = params.fLifetime > 0.f ? static_cast<uint32_t>(std::ceil(params.fLifetime * 1000.f / rate)) : 3000u;
                if (child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE)
                {
                    if (document.eType == EFFECT_V2_TYPE::PARTICLE) tail = params.Particle.vLifetime.y / rate;
                    if (document.eType == EFFECT_V2_TYPE::TRAIL) tail = params.Trail.fPointLifetime / rate;
                }
            }
            effectEnd = (std::max)(effectEnd, child.iStartMs + duration + static_cast<uint32_t>(std::ceil(tail * 1000.f)));
        }
    return (std::min)(600000u, (std::max)(1u, (std::max)(effectEnd, m_Model.DurationMs())));
}
void CEffectCompositionWorkbench::Refresh_Inventory()
{
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> staged;
    std::string status;
    if (CEffectV2Catalog::Get().Read_Inventory(staged, status)) m_Inventory = std::move(staged);
    m_Status = status; m_InventoryLoaded = true;
}

void CEffectCompositionWorkbench::Load_WorldObjects()
{
    m_WorldLoaded = true;
    Read_EffectCompositionWorldResources("LV_LUT_MIDNIGHTC_ED", m_WorldObjects, m_WorldRevision, m_WorldStatus);
}

void CEffectCompositionWorkbench::Capture_Context()
{
    if (m_Edit.Empty()) return;
    EFFECT_V2_AUTHORING_PREVIEW context;
    context.strPatternId = m_Model.Selected_Id(); context.bBundle = m_Model.Selected_IsBundle();
    context.strAnchorKind = m_AnchorKind == 1 ? "CHARACTER_ROOT" : "WORLD";
    context.strMemberId = m_AnchorMember;
    context.vWorldPosition = {m_WorldPivot._41, m_WorldPivot._42, m_WorldPivot._43};
    m_Edit.Group().AuthoringPreview = context;
}
void CEffectCompositionWorkbench::Restore_Context()
{
    m_AnchorKind = 0; m_AnchorMember.clear(); m_Model.Clear_Selection(); Reset_AnchorHistory();
    if (!m_Edit.Group().AuthoringPreview) return;
    const auto context = *m_Edit.Group().AuthoringPreview;
    XMStoreFloat4x4(&m_WorldPivot, XMMatrixTranslation(context.vWorldPosition.x, context.vWorldPosition.y, context.vWorldPosition.z));
    m_AnchorKind = context.strAnchorKind == "CHARACTER_ROOT" ? 1 : 0;
    m_AnchorMember = context.strMemberId;
    if (!context.strPatternId.empty())
    {
        if (!m_Model.Is_Loaded()) m_Model.Reload();
        if (context.bBundle) m_Model.Select_Bundle(context.strPatternId);
        else m_Model.Select_Pattern(context.strPatternId);
    }
}

bool CEffectCompositionWorkbench::Resolve_Anchor(float4x4_t& pivot)
{
    if (m_AnchorKind == 0) { pivot = m_WorldPivot; return true; }
    EFFECT_V2_TARGET target; EFFECT_V2_TARGET_VIEW view;
    if (!m_Model.Resolve_Target(m_AnchorMember, target, view)) return false;
    return CEffectV2Object::Resolve_TargetPivot(view, "", CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, pivot);
}
void CEffectCompositionWorkbench::Reset_AnchorHistory()
{
    m_AnchorHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
    m_LastRecordedSeconds = -1.f;
}
void CEffectCompositionWorkbench::Record_Anchor()
{
    const float seconds = static_cast<float>(m_ClockMs * .001);
    if (seconds < m_LastRecordedSeconds) return; // Scrub preserves the recorded path.
    float4x4_t pivot;
    if (!Resolve_Anchor(pivot)) return;
    // Model-reference preview suppresses root motion and has a fixed actor root.
    // Seed its known initial pose for a first Play after seeking. Product actors
    // use their observed movement history instead of fabricating past poses.
    if (m_AnchorKind == 1 && m_Model.Is_Active() && m_LastRecordedSeconds < 0.f && seconds > 0.f)
    {
        std::string error;
        if (!m_AnchorHistory->Record(0.f, pivot, false, error)) { m_Status = error; return; }
        m_LastRecordedSeconds = 0.f; m_LastRecordedPivot = pivot;
    }
    const float dx = pivot._41 - m_LastRecordedPivot._41;
    const float dy = pivot._42 - m_LastRecordedPivot._42;
    const float dz = pivot._43 - m_LastRecordedPivot._43;
    const bool discontinuity = m_LastRecordedSeconds >= 0.f && dx*dx + dy*dy + dz*dz > 2500.f;
    std::string error;
    if (!m_AnchorHistory->Record(seconds, pivot, discontinuity, error)) { m_Status = error; return; }
    m_LastRecordedSeconds = seconds; m_LastRecordedPivot = pivot;
}
bool CEffectCompositionWorkbench::Sample_Anchor(const float seconds, float4x4_t& pivot, std::string& error) const
{
    if (m_AnchorKind == 0) { pivot = m_WorldPivot; return true; }
    return m_AnchorHistory->Sample(seconds, pivot, error);
}

bool CEffectCompositionWorkbench::Play(const bool reset)
{
    if (m_Edit.Empty()) { m_Status = "Create or open an Effect first."; return false; }
    std::vector<EFFECT_V2_DOCUMENT> documents;
    auto group = m_Edit.Group();
    std::erase_if(group.Children, [this](const auto& child) {
        return m_Muted.contains(child.strChildId) || (!m_Solo.empty() && child.strChildId != m_Solo); });
    std::set<std::string> included;
    for (const auto& child : group.Children)
        if (included.insert(child.strEffectId).second)
        {
            const auto leaf = m_Edit.Leaves().find(child.strEffectId);
            if (leaf == m_Edit.Leaves().end()) { m_Status = "Missing element draft: " + child.strEffectId; return false; }
            documents.push_back(leaf->second.document);
        }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
    if (!CEffectV2Catalog::Get().Create_ResourceSnapshot(documents, {group}, snapshot, m_Status)) return false;
    const auto* normalized = snapshot->Find_Group(group.strGroupId);
    if (!normalized) { m_Status = "Staged Effect group is unavailable."; return false; }
    group = *normalized;
    if (!CEffectV2Runtime::Prewarm_Group(group, snapshot, m_Device, m_Context))
    { m_Status = CEffectV2Runtime::Last_Error(); return false; }
    const double previousClock = m_ClockMs;
    const auto previousHistory = m_AnchorHistory;
    const float previousRecorded = m_LastRecordedSeconds;
    const auto previousPivot = m_LastRecordedPivot;
    const bool modelWasActive = m_Model.Is_Active();
    const auto rollback = [&]()
    {
        m_ClockMs = previousClock; m_AnchorHistory = previousHistory;
        m_LastRecordedSeconds = previousRecorded; m_LastRecordedPivot = previousPivot;
        if (modelWasActive && m_Model.Is_Active()) m_Model.Sample(static_cast<uint32_t>(previousClock), m_Paused);
        else if (!modelWasActive) m_Model.Stop();
    };
    if (reset) { m_ClockMs = 0.0; Reset_AnchorHistory(); }
    if (!m_Model.Selected_Id().empty() && !m_Model.Is_Active() &&
        !m_Model.Begin(static_cast<uint32_t>(m_ClockMs), true))
    { m_Status = m_Model.Status(); rollback(); return false; }
    if (m_Model.Is_Active()) m_Model.Sample(static_cast<uint32_t>(m_ClockMs), true);
    Record_Anchor();
    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
    if (!Resolve_Anchor(playback.PivotWorld)) { m_Status = "Select an available character anchor member."; rollback(); return false; }
    playback.fInitialAgeSeconds = static_cast<float>(m_ClockMs * 0.001);
    if (m_AnchorKind != 0)
    {
        const auto history = m_AnchorHistory;
        playback.PivotSampler = [history](float seconds, float4x4_t& pivot, std::string& error) { return history->Sample(seconds, pivot, error); };
    }
    playback.bExternalClock = true;
    const auto handle = CEffectV2Runtime::Play_Group(group, snapshot, playback, m_Device, m_Context);
    if (!handle) { m_Status = CEffectV2Runtime::Last_Error(); rollback(); return false; }
    std::string spawnFailure;
    if (!CEffectV2Runtime::Sample_Group(handle, static_cast<float>(m_ClockMs * .001), true, m_Device, m_Context) ||
        CEffectV2Runtime::Consume_GroupFailure(handle, spawnFailure))
    {
        if (spawnFailure.empty()) spawnFailure = CEffectV2Runtime::Last_Error();
        CEffectV2Runtime::Stop_Group(handle); rollback();
        m_Status = "Preview preserved: " + spawnFailure; return false;
    }
    if (m_Handle) CEffectV2Runtime::Stop_Group(m_Handle);
    m_Handle = handle; m_PlaySnapshot = std::move(snapshot);
    m_Playing = true; m_Paused = false; m_PreviewDirty = false;
    m_Status = "Playing Effect and model reference on one sequencer clock.";
    return true;
}
void CEffectCompositionWorkbench::Sample(const bool seek)
{
    if (m_Model.Is_Active() && !m_Model.Sample(static_cast<uint32_t>(m_ClockMs), m_Paused))
    { m_Status = m_Model.Status(); Stop(); return; }
    Record_Anchor();
    if (m_Handle)
    {
        if (m_AnchorKind == 0) CEffectV2Runtime::Set_GroupPivot(m_Handle, m_WorldPivot);
        const bool sampled = CEffectV2Runtime::Sample_Group(m_Handle, static_cast<float>(m_ClockMs * 0.001), m_Paused || seek, m_Device, m_Context);
        if (!sampled && !CEffectV2Runtime::Last_Error().empty()) m_Status = CEffectV2Runtime::Last_Error();
        std::string failure;
        if (CEffectV2Runtime::Consume_GroupFailure(m_Handle, failure)) { m_Status = failure; Stop(); }
    }
}
void CEffectCompositionWorkbench::Update(const float dt, const bool active)
{
    if (!active) { if (m_Playing) Stop(); return; }
    if (!m_Playing || m_Paused) return;
    if (!m_Model.Selected_Id().empty() && !m_Model.Is_Active())
    { m_Status = "Another preview took the model target. Press Play to take it back."; Stop(); return; }
    m_ClockMs = (std::min)(static_cast<double>(DurationMs()), m_ClockMs + dt * 1000.0);
    Sample(false);
    if (m_ClockMs >= DurationMs())
    {
        if (m_Loop) Play(true);
        else { m_Paused = true; if (m_Handle) CEffectV2Runtime::Set_GroupPaused(m_Handle, true); }
    }
}

void CEffectCompositionWorkbench::Render_Toolbar()
{
    ImGui::SetNextItemWidth(280.f);
    ImGui::InputTextWithHint("##EffectName", "Effect name", m_Name, sizeof(m_Name));
    ImGui::SameLine(); ImGui::SetNextItemWidth(140.f);
    ImGui::Combo("Type", &m_CreateType, "Mesh\0Texture\0Mesh / Sprite Particle\0Decal\0Trail\0Screen Post\0");
    ImGui::SameLine();
    if (ImGui::Button("Create Effect"))
    {
        if (m_Edit.Dirty()) m_Status = "Save or Revert the current Effect before creating another.";
        else if (!m_Name[0]) m_Status = "Enter an Effect name.";
        else { Stop(); m_ClockMs = 0; m_Muted.clear(); m_Solo.clear(); Reset_AnchorHistory(); m_AnchorMember.clear(); m_AnchorKind = 0; m_Model.Clear_Selection();
            m_Edit.Create(m_Name, static_cast<EFFECT_V2_TYPE>(m_CreateType));
            if (const auto* world = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))
            { const auto matrix = XMLoadFloat4x4(world); XMStoreFloat4x4(&m_WorldPivot, XMMatrixTranslationFromVector(matrix.r[3] + XMVector3Normalize(matrix.r[2]) * 5.f)); }
            m_Status = "Created. Bind resources and tune the selected element."; }
    }
    ImGui::SameLine(); ImGui::BeginDisabled(m_Edit.Empty());
    if (ImGui::Button("Save")) { Capture_Context(); if (m_Edit.Save(m_Status)) { Refresh_Inventory(); m_Status = "Saved Effect and elements; saved Object motions were not changed."; } }
    ImGui::SameLine();
    if (ImGui::Button("Revert"))
    {
        if (m_Edit.Revert(m_Status))
        {
            Stop(); m_ClockMs = 0; m_Muted.clear(); m_Solo.clear(); m_PreviewDirty = false;
            Restore_Context();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Element")) { m_Edit.Add_Element(static_cast<EFFECT_V2_TYPE>(m_CreateType), m_Name[0] ? m_Name : "Element"); m_PreviewDirty = true; }
    ImGui::EndDisabled();
    if (m_Edit.Dirty()) { ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, .8f, .3f, 1.f), "Unsaved"); }
}

void CEffectCompositionWorkbench::Render_Resources()
{
    if (!ImGui::BeginTabBar("Resources")) return;
    if (ImGui::BeginTabItem("Effect Resources"))
    {
        if (auto* document = m_Edit.Selected_Document())
        {
            const auto mesh = document->Desc.strMeshAssetId;
            const auto textures = document->Desc.TextureAssetIds;
            m_ResourceEditor.Render_CompositionResources(*document);
            if (mesh != document->Desc.strMeshAssetId || textures != document->Desc.TextureAssetIds) Edited();
        }
        else ImGui::TextWrapped("Select an element to bind its resource slots. Create Effect creates a named composition and its first element.");
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("World Object"))
    {
        if (!m_WorldLoaded) Load_WorldObjects();
        if (ImGui::Button("Reload saved World Objects")) Load_WorldObjects();
        ImGui::TextWrapped("%s", m_WorldStatus.c_str());
        if (ImGui::BeginChild("Objects"))
        {
            for (const auto& entry : m_WorldObjects)
            {
                const auto& object = entry.resource;
                ImGui::PushID(object.objectId.c_str());
                const bool open = ImGui::TreeNodeEx(Label(object.displayName, object.objectId), ImGuiTreeNodeFlags_OpenOnArrow);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    auto* document = m_Edit.Selected_Document();
                    if (!document) m_Status = "Select an element first.";
                    else if (document->eType != EFFECT_V2_TYPE::MESH && document->eType != EFFECT_V2_TYPE::PARTICLE)
                        m_Status = "Select a Mesh or Particle element to bind a World Object mesh.";
                    else if (!entry.error.empty()) m_Status = entry.error;
                    else if (object.modelAssetId.empty()) m_Status = "This World Object is a sequence alias; it has no mesh resource.";
                    else
                    {
                        document->Desc.strMeshAssetId = object.modelAssetId;
                        if (!object.diffuseTextureAssetId.empty()) document->Desc.TextureAssetIds[0] = object.diffuseTextureAssetId;
                        document->Desc.Params.fMeshPreScale = object.modelPreScale;
                        document->Desc.Params.Scale.vStart = document->Desc.Params.Scale.vEnd = object.scale;
                        Edited(); m_Status = "Bound saved World Object mesh, base texture and scale. Select a child motion to import its timing and movement.";
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s\n%s", object.modelAssetId.c_str(), object.diffuseTextureAssetId.c_str(), entry.error.c_str());
                if (open)
                {
                    for (const auto& motion : entry.motions)
                    {
                        ImGui::PushID(motion.instanceId.c_str());
                        if (ImGui::Selectable(Label(motion.displayName, motion.instanceId)))
                        {
                            auto* document = m_Edit.Selected_Document(); auto* child = m_Edit.Selected_Child();
                            if (!document || !child) m_Status = "Select an element before importing a World Object motion.";
                            else if (Apply_WorldMotionToEffect(entry, motion, *document, *child, m_Status))
                            {
                                const float tail = document->eType == EFFECT_V2_TYPE::PARTICLE ? document->Desc.Params.Particle.vLifetime.y / (std::max)(.001f, document->Desc.Params.fPlayRate) : 0.f;
                                m_Edit.Group().iDurationMs = (std::max)(m_Edit.Group().iDurationMs, child->iStartMs + child->iDurationMs + static_cast<uint32_t>(tail * 1000.f));
                                Edited();
                            }
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%u ms | %.2fx | %zu clips\n%s", motion.durationMs, motion.playbackSpeed, motion.animationTracks.size(), motion.error.c_str());
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Saved Effects"))
    {
        if (!m_InventoryLoaded) Refresh_Inventory();
        if (ImGui::Button("Refresh list")) Refresh_Inventory();
        ImGui::SameLine(); ImGui::SetNextItemWidth(200.f);
        ImGui::InputTextWithHint("##SavedFilter", "Name or ID", m_Filter, sizeof(m_Filter));
        if (ImGui::BeginChild("Inventory"))
        {
            for (const auto& row : m_Inventory)
            {
                if (m_Filter[0] && row.strResourceId.find(m_Filter) == std::string::npos && row.strDisplayName.find(m_Filter) == std::string::npos) continue;
                ImGui::PushID(row.strResourceId.c_str());
                const std::string label = std::string(row.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "[Effect] " : "[Element] ") + Label(row.strDisplayName, row.strResourceId);
                if (ImGui::Selectable(label.c_str(), m_InventorySelection == row.strResourceId)) m_InventorySelection = row.strResourceId;
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s", row.strResourceId.c_str(), row.strStatus.c_str());
                if (m_InventorySelection == row.strResourceId)
                {
                    if (ImGui::SmallButton("Open"))
                    {
                        if (m_Edit.Load(row.eKind, row.strResourceId, m_Status)) { Stop(); m_ClockMs = 0; m_PreviewDirty = false; m_Muted.clear(); m_Solo.clear(); Restore_Context(); }
                    }
                    if (row.eKind == EFFECT_V2_RESOURCE_KIND::LEAF && !m_Edit.Empty())
                    { ImGui::SameLine(); if (ImGui::SmallButton("Append Element")) { if (m_Edit.Append(row.strResourceId, m_Status)) m_PreviewDirty = true; } }
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}

void CEffectCompositionWorkbench::Render_ModelResources()
{
    if (ImGui::Button("Reload saved Patterns")) m_Model.Reload();
    ImGui::SameLine();
    if (ImGui::Button("None")) { Stop(); m_Model.Clear_Selection(); m_AnchorKind = 0; Edited(); }
    if (!m_Model.Is_Loaded()) { ImGui::TextWrapped("Reload saved Patterns to choose a model animation reference. Effect creation is independent."); return; }
    const auto choosePattern = [this](const std::string& id)
    {
        Stop(); m_Model.Select_Pattern(id); m_AnchorMember.clear(); Reset_AnchorHistory(); Edited();
    };
    const auto& source = m_Model.Get_Document();
    std::map<std::string, size_t> folderCounts;
    std::map<std::string, const KOUKU_SAYDON_COMPOSITION_PATTERN*> patterns;
    std::set<std::string> bundledPatterns;
    for (const auto& folder : source.Folders) ++folderCounts[folder.strFolderId];
    for (const auto& pattern : source.Patterns) patterns.emplace(pattern.strPatternId, &pattern);
    for (const auto& bundle : source.Bundles)
        for (const auto& member : bundle.Members) bundledPatterns.insert(member.strPatternId);
    const auto validFolder = [&](const auto& folder)
    {
        return !folder.strFolderId.empty() && folder.strLoadError.empty() &&
            CKoukuSaydonCompositionDocument::Is_KnownGate(folder.strGateId) && folderCounts.at(folder.strFolderId) == 1u;
    };
    const auto findBundleFolder = [&](const auto& bundle)
    {
        return std::find_if(source.Folders.begin(), source.Folders.end(), [&](const auto& folder)
        { return validFolder(folder) && folder.strFolderId == bundle.strFolderId && folder.strGateId == bundle.strGateId; });
    };
    const auto renderPattern = [&](const auto& pattern)
    {
        ImGui::PushID(pattern.strPatternId.c_str());
        const std::string label = std::string(Label(pattern.strDisplayName, pattern.strPatternId)) + " / " + pattern.strActorProfileId;
        if (ImGui::Selectable(label.c_str(), !m_Model.Selected_IsBundle() && m_Model.Selected_Id() == pattern.strPatternId)) choosePattern(pattern.strPatternId);
        ImGui::PopID();
    };
    const auto renderBundle = [&](const auto& bundle, const bool orphaned)
    {
        ImGui::PushID(bundle.strBundleId.c_str());
        const auto flags = ImGuiTreeNodeFlags_OpenOnArrow |
            (m_Model.Selected_IsBundle() && m_Model.Selected_Id() == bundle.strBundleId ? ImGuiTreeNodeFlags_Selected : 0);
        const bool opened = ImGui::TreeNodeEx("Bundle", flags, "%s", Label(bundle.strDisplayName, bundle.strBundleId));
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        { Stop(); m_Model.Select_Bundle(bundle.strBundleId); m_AnchorMember.clear(); Reset_AnchorHistory(); Edited(); }
        if (orphaned)
            ImGui::TextColored(ImVec4(1.f, .7f, .3f, 1.f), "Saved folder unavailable: %s (Gate %s)", bundle.strFolderId.c_str(), bundle.strGateId.c_str());
        if (opened)
        {
            if (!bundle.strLoadError.empty()) ImGui::TextWrapped("%s", bundle.strLoadError.c_str());
            for (const auto& member : bundle.Members)
            {
                ImGui::PushID(member.strMemberId.c_str());
                const auto pattern = patterns.find(member.strPatternId);
                if (pattern == patterns.end()) ImGui::TextColored(ImVec4(1.f, .7f, .3f, 1.f), "Missing Pattern: %s", member.strPatternId.c_str());
                else renderPattern(*pattern->second);
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    };
    // Saved folders are direct Gate children; this schema has no parent-folder edge.
    for (const char* gate : {"GATE1", "GATE2", "GATE3", "BINGO"})
    {
        if (!ImGui::TreeNode(gate)) continue;
        for (const auto& folder : source.Folders)
        {
            if (folder.strGateId != gate || !validFolder(folder)) continue;
            ImGui::PushID(folder.strFolderId.c_str());
            if (ImGui::TreeNodeEx("Folder", ImGuiTreeNodeFlags_None, "%s", Label(folder.strDisplayName, folder.strFolderId)))
            {
                for (const auto& bundle : source.Bundles)
                    if (bundle.strGateId == gate && bundle.strFolderId == folder.strFolderId) renderBundle(bundle, false);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        for (const auto& bundle : source.Bundles)
            if (bundle.strGateId == gate && findBundleFolder(bundle) == source.Folders.end()) renderBundle(bundle, true);
        for (const auto& pattern : source.Patterns)
            if (pattern.strGateId == gate && !bundledPatterns.contains(pattern.strPatternId)) renderPattern(pattern);
        ImGui::TreePop();
    }
    // Invalid saved rows remain visible instead of disappearing from the inventory.
    for (const auto& folder : source.Folders)
        if (!validFolder(folder))
            ImGui::TextColored(ImVec4(1.f, .7f, .3f, 1.f), "Invalid saved folder: %s | %s | %s",
                Label(folder.strDisplayName, folder.strFolderId), folder.strGateId.c_str(),
                folder.strLoadError.empty() ? "Missing/duplicate ID or unknown Gate." : folder.strLoadError.c_str());
    for (const auto& bundle : source.Bundles)
        if (!CKoukuSaydonCompositionDocument::Is_KnownGate(bundle.strGateId)) renderBundle(bundle, true);
    for (const auto& pattern : source.Patterns)
        if (!CKoukuSaydonCompositionDocument::Is_KnownGate(pattern.strGateId) && !bundledPatterns.contains(pattern.strPatternId))
        {
            ImGui::TextColored(ImVec4(1.f, .7f, .3f, 1.f), "Unknown saved Gate: %s", pattern.strGateId.c_str());
            renderPattern(pattern);
        }
    ImGui::TextWrapped("%s", m_Model.Status().c_str());
    for (const auto& actor : m_Model.Actors())
        ImGui::TextWrapped("%s | %s | offset %u -> %.2f ms | %s", actor.displayName.c_str(), actor.actorProfileId.c_str(), actor.authoredOffsetMs, actor.effectiveOffsetMs, actor.status.c_str());
    ImGui::TextDisabled("Saved clip order is read-only here. Edit clips in Action Workbench.");
}

void CEffectCompositionWorkbench::Render_Detail()
{
    if (!ImGui::BeginTabBar("DetailTabs")) return;
    if (ImGui::BeginTabItem("Effect Detail"))
    {
        if (ImGui::BeginChild("EffectDetailScroll"))
        {
            if (!m_Edit.Empty())
            {
                if (Edit_Name("Effect name", m_Edit.Group().strDisplayName)) Edited();
                if (Edit_Ms("Effect duration (0 = auto)", m_Edit.Group().iDurationMs)) Edited();
                ImGui::TextDisabled("ID: %s", m_Edit.Group().strGroupId.c_str());
                if (ImGui::Combo("Anchor", &m_AnchorKind, "World\0Character root\0")) { Stop(); Reset_AnchorHistory(); Edited(); }
                if (m_AnchorKind == 1)
                {
                    if (ImGui::BeginCombo("Character", m_AnchorMember.empty() ? "Select model member" : m_AnchorMember.c_str()))
                    {
                        for (const auto& actor : m_Model.Actors())
                        {
                            ImGui::PushID(actor.memberId.c_str());
                            const std::string actorLabel = std::string(Label(actor.displayName, actor.memberId)) + "###AnchorMember";
                            if (ImGui::Selectable(actorLabel.c_str(), m_AnchorMember == actor.memberId))
                            { Stop(); m_AnchorMember = actor.memberId; Reset_AnchorHistory(); Edited(); }
                            ImGui::PopID();
                        }
                        ImGui::EndCombo();
                    }
                }
                else
                {
                    if (ImGui::DragFloat3("World position", &m_WorldPivot._41, .05f)) Edited();
                    if (ImGui::Button("Set in front of camera"))
                    {
                        const auto* world = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
                        if (world) { const auto matrix = XMLoadFloat4x4(world); XMStoreFloat4x4(&m_WorldPivot, XMMatrixTranslationFromVector(matrix.r[3] + XMVector3Normalize(matrix.r[2]) * 5.f)); Edited(); }
                    }
                }
                if (auto* child = m_Edit.Selected_Child())
                {
                    ImGui::SeparatorText("Element Box");
                    bool changed = Edit_Ms("Start", child->iStartMs);
                    changed |= Edit_Ms("Emission / element duration", child->iDurationMs, 1);
                    int stop = static_cast<int>(child->eStop);
                    if (ImGui::Combo("End", &stop, "Kill immediately\0Stop emission; keep remaining lifetime\0")) { child->eStop = static_cast<EFFECT_V2_CHILD_STOP>(stop); changed = true; }
                    changed |= ImGui::DragFloat3("Anchor offset", &child->vOffset.x, .01f);
                    changed |= ImGui::DragFloat("Yaw offset", &child->fYawDegrees, .5f);
                    changed |= ImGui::DragFloat("Pitch offset", &child->fPitchDegrees, .5f);
                    changed |= ImGui::DragFloat("Roll offset", &child->fRollDegrees, .5f);
                    changed |= ImGui::DragFloat3("Element scale", &child->vScale.x, .01f, .001f, 1000.f);
                    if (changed)
                    {
                        child->LocalTransform.vTranslation = child->vOffset;
                        child->LocalTransform.vRotation = {child->fPitchDegrees, child->fYawDegrees, child->fRollDegrees};
                        child->LocalTransform.vScale = child->vScale; Edited();
                    }
                    if (ImGui::Button("Remove Element")) { m_Edit.Remove_Selected(); m_PreviewDirty = true; }
                }
                if (auto* document = m_Edit.Selected_Document())
                {
                    if (Edit_Name("Element name", document->strDisplayName)) Edited();
                    ImGui::TextDisabled("Element ID: %s", document->strEffectId.c_str());
                    if (document->eType == EFFECT_V2_TYPE::PARTICLE)
                    {
                        auto& params = document->Desc.Params; auto& particle = params.Particle;
                        if (ImGui::CollapsingHeader("Finite emission count / random rotation", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            int count = static_cast<int>(particle.iBurstCount);
                            int interval = particle.fSpawnRate > 0.f ? static_cast<int>(std::lround(1000.f / particle.fSpawnRate)) : 0;
                            if (particle.fSpawnRate > 0.f && params.fLifetime > 0.f)
                                count += static_cast<int>(std::floor(params.fLifetime * particle.fSpawnRate + .00001f));
                            count = (std::clamp)(count, 1, 2048);
                            bool schedule = ImGui::DragInt("Emission count", &count, 1.f, 1, 2048);
                            schedule |= ImGui::DragInt("Interval (0 = together)", &interval, 1.f, 0, 600000, "%d ms");
                            if (schedule)
                            {
                                count = (std::clamp)(count, 1, 2048); interval = (std::clamp)(interval, 0, 600000);
                                const double emissionMs = count > 1 && interval > 0 ? double(count - 1) * interval + .01 : .01;
                                if (emissionMs > 600000.0) m_Status = "Emission schedule exceeds ten minutes; reduce count or interval.";
                                else
                                {
                                    particle.iBurstCount = interval > 0 && count > 1 ? 1u : static_cast<uint32_t>(count);
                                    particle.fSpawnRate = interval > 0 && count > 1 ? 1000.f / interval : 0.f;
                                    particle.iMaxParticles = (std::max)(particle.iMaxParticles, static_cast<uint32_t>(count));
                                    params.fLifetime = static_cast<float>(emissionMs * .001); params.bLoop = false;
                                    auto* child = m_Edit.Selected_Child();
                                    const float rate = (std::max)(.001f, params.fPlayRate);
                                    child->iDurationMs = static_cast<uint32_t>((std::max)(1.0, std::ceil(emissionMs / rate)));
                                    child->eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE;
                                    m_Edit.Group().iDurationMs = (std::max)(m_Edit.Group().iDurationMs,
                                        child->iStartMs + child->iDurationMs + static_cast<uint32_t>(std::ceil(particle.vLifetime.y * 1000.f / rate)));
                                    Edited();
                                }
                            }
                            ImGui::TextDisabled("Count controls births. Max Particles below is the live capacity limit.");
                            if (!document->Desc.strMeshAssetId.empty())
                            {
                                if (ImGui::Button("Random mesh rotation 0..360"))
                                { particle.vMeshRotationMin = {}; particle.vMeshRotationMax = {360.f, 360.f, 360.f}; Edited(); }
                                ImGui::SameLine();
                                if (ImGui::Button("Random mesh spin"))
                                { particle.vMeshSpinMin = {-180.f, -180.f, -180.f}; particle.vMeshSpinMax = {180.f, 180.f, 180.f}; Edited(); }
                            }
                        }
                    }
                    // Track CPU value edits without serialization or I/O per frame.
                    std::array<unsigned char, sizeof(CEffectV2Object::PARAMS)> before{};
                    std::memcpy(before.data(), &document->Desc.Params, before.size());
                    m_ResourceEditor.Render_DraftDetail(*document);
                    if (std::memcmp(before.data(), &document->Desc.Params, before.size()) != 0) Edited();
                }
                else ImGui::TextWrapped("Select an element row for its resource slots and tuning.");
            }
        }
        ImGui::EndChild(); ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Model Animation"))
    {
        if (ImGui::BeginChild("ModelResources")) Render_ModelResources();
        ImGui::EndChild(); ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}

void CEffectCompositionWorkbench::Render_Timeline()
{
    if (m_Edit.Empty())
    {
        ImGui::TextWrapped("Create an Effect or open a saved Effect to edit its sequencer.");
        return;
    }
    if (ImGui::Button("Play")) Play(m_ClockMs >= DurationMs());
    ImGui::SameLine();
    if (ImGui::Button(m_Paused ? "Resume" : "Pause")) { m_Paused = !m_Paused; Sample(true); }
    ImGui::SameLine(); if (ImGui::Button("Restart")) Play(true);
    ImGui::SameLine(); if (ImGui::Button("Stop")) { Stop(); m_ClockMs = 0; }
    ImGui::SameLine(); ImGui::Checkbox("Loop", &m_Loop);
    ImGui::SameLine();
    if (ImGui::Button("Apply tuning to preview")) { const bool paused = m_Paused; if (Play(false)) { m_Paused = paused; Sample(true); } }
    ImGui::SameLine(); ImGui::SetNextItemWidth(180.f);
    ImGui::SliderFloat("Zoom", &m_Zoom, 10.f, 300.f, "%.0f px/s");
    if (m_PreviewDirty) ImGui::TextColored(ImVec4(1.f, .8f, .3f, 1.f), "Draft changed. Apply tuning or Restart to replay the edited snapshot.");
    int cursor = static_cast<int>(m_ClockMs);
    if (ImGui::SliderInt("Time", &cursor, 0, static_cast<int>(DurationMs()), "%d ms"))
    { m_ClockMs = cursor; m_Paused = true; Sample(true); }
    constexpr float labels = 220.f, rowHeight = 30.f;
    const float width = labels + DurationMs() * m_Zoom * .001f + 20.f;
    ImGui::SetNextWindowContentSize(ImVec2(width, 0.f));
    if (ImGui::BeginChild("Timeline", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, {origin.x + labels, origin.y}, {origin.x + width, origin.y + rowHeight}, DurationMs(), m_Zoom);
        ImGui::Dummy({width, rowHeight});
        for (const auto& actor : m_Model.Actors())
        {
            const auto top = ImGui::GetCursorScreenPos();
            ImGui::Text("Model: %s", actor.displayName.c_str());
            for (const auto& clip : m_Model.Rows())
            {
                if (clip.memberId != actor.memberId) continue;
                const float x = top.x + labels + static_cast<float>(clip.startMs) * m_Zoom * .001f;
                const float end = x + clip.durationMs * m_Zoom * .001f;
                CompositionTimeline::DrawBox(draw, {x, top.y}, {end, top.y + 25.f}, IM_COL32(67, 100, 142, 255), false, clip.runtimeClip.c_str(), false, false);
                if (ImGui::IsMouseHoveringRect({x, top.y}, {end, top.y + 25.f}))
                    ImGui::SetTooltip("%s\n%s | source %u ms | %.2fx\n%s", clip.runtimeClip.c_str(), clip.occurrenceId.c_str(), clip.sourceStartMs, clip.playRate, clip.status.c_str());
            }
            ImGui::SetCursorScreenPos({top.x, top.y + rowHeight}); ImGui::Dummy({width, 1.f});
        }
        const std::string groupLabel = std::string(Label(
            m_Edit.Group().strDisplayName, m_Edit.Group().strGroupId)) + "###EffectCompositionGroup";
        if (ImGui::Selectable(groupLabel.c_str(), m_Edit.Selection().empty(), 0, {labels, 22.f})) m_Edit.Select({});
        for (auto& child : m_Edit.Group().Children)
        {
            const auto found = m_Edit.Leaves().find(child.strEffectId);
            const auto* document = found == m_Edit.Leaves().end() ? nullptr : &found->second.document;
            ImGui::PushID(child.strChildId.c_str());
            const auto top = ImGui::GetCursorScreenPos();
            bool muted = m_Muted.contains(child.strChildId);
            if (ImGui::Checkbox("M", &muted)) { if (muted) m_Muted.insert(child.strChildId); else m_Muted.erase(child.strChildId); m_PreviewDirty = true; }
            ImGui::SameLine(); bool solo = m_Solo == child.strChildId;
            if (ImGui::Checkbox("S", &solo)) { m_Solo = solo ? child.strChildId : ""; m_PreviewDirty = true; }
            ImGui::SameLine();
            if (ImGui::Selectable(document ? Label(document->strDisplayName, child.strEffectId) : child.strEffectId.c_str(), m_Edit.Selection() == child.strChildId, 0, {130, 24})) m_Edit.Select(child.strChildId);
            const float start = top.x + labels + child.iStartMs * m_Zoom * .001f;
            const float end = start + child.iDurationMs * m_Zoom * .001f;
            if (document && child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE)
            {
                float tail = 0.f;
                if (document->eType == EFFECT_V2_TYPE::PARTICLE) tail = document->Desc.Params.Particle.vLifetime.y;
                if (document->eType == EFFECT_V2_TYPE::TRAIL) tail = document->Desc.Params.Trail.fPointLifetime;
                tail /= (std::max)(.001f, document->Desc.Params.fPlayRate);
                const float limit = top.x + labels + (m_Edit.Group().iDurationMs ? m_Edit.Group().iDurationMs : DurationMs()) * m_Zoom * .001f;
                if (end < limit) draw->AddRectFilled({end, top.y + 4.f}, {(std::min)(limit, end + tail * m_Zoom), top.y + 22.f}, IM_COL32(114, 82, 50, 140));
            }
            CompositionTimeline::DrawBox(draw, {start, top.y}, {(std::max)(start + 3.f, end), top.y + 26.f}, IM_COL32(170, 105, 47, 255), m_Edit.Selection() == child.strChildId, document ? Label(document->strDisplayName, child.strEffectId) : "Element");
            if (ImGui::IsMouseHoveringRect({start, top.y}, {(std::max)(start + 5.f, end), top.y + 26.f}) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                m_Edit.Select(child.strChildId); m_DragChild = child.strChildId;
                m_DragKind = static_cast<int>(CompositionTimeline::HitBoxGesture(ImGui::GetIO().MousePos.x, start, end, 7.f, true, true));
                m_DragX = ImGui::GetIO().MousePos.x; m_DragStart = child.iStartMs; m_DragDuration = child.iDurationMs;
            }
            if (m_DragChild == child.strChildId && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                const int delta = static_cast<int>((ImGui::GetIO().MousePos.x - m_DragX) * 1000.f / m_Zoom);
                if (m_DragKind == 0) child.iStartMs = static_cast<uint32_t>((std::clamp)(static_cast<int>(m_DragStart) + delta, 0, 600000 - static_cast<int>(m_DragDuration)));
                if (m_DragKind == 1) { child.iStartMs = static_cast<uint32_t>((std::clamp)(static_cast<int>(m_DragStart) + delta, 0, static_cast<int>(m_DragStart + m_DragDuration) - 1)); child.iDurationMs = m_DragStart + m_DragDuration - child.iStartMs; }
                if (m_DragKind == 2) child.iDurationMs = static_cast<uint32_t>((std::clamp)(static_cast<int>(m_DragDuration) + delta, 1, 600000 - static_cast<int>(m_DragStart)));
                Edited();
            }
            ImGui::SetCursorScreenPos({top.x, top.y + rowHeight}); ImGui::Dummy({width, 1.f}); ImGui::PopID();
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) m_DragChild.clear();
        const float x = origin.x + labels + static_cast<float>(m_ClockMs) * m_Zoom * .001f;
        draw->AddLine({x, origin.y}, {x, ImGui::GetCursorScreenPos().y}, IM_COL32(255, 220, 80, 255), 2.f);
    }
    ImGui::EndChild();
}

void CEffectCompositionWorkbench::Render()
{
    if (!m_Open) return;
    m_ResourceEditor.Begin_CompositionFrame();
    ImGui::SetNextWindowSize({1450.f, 900.f}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Effect Composition Workbench", &m_Open))
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
            (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) m_Interaction = true;
        Render_Toolbar();
        ImGui::TextWrapped("%s", m_Status.c_str());
        const float height = (std::max)(240.f, ImGui::GetContentRegionAvail().y * .58f);
        if (ImGui::BeginTable("WorkbenchPanes", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Resources", ImGuiTableColumnFlags_WidthStretch, .53f);
            ImGui::TableSetupColumn("Detail / Model Animation", ImGuiTableColumnFlags_WidthStretch, .47f);
            ImGui::TableNextColumn();
            if (ImGui::BeginChild("ResourcesPane", {0, height})) Render_Resources();
            ImGui::EndChild(); ImGui::TableNextColumn();
            if (ImGui::BeginChild("DetailPane", {0, height})) Render_Detail();
            ImGui::EndChild(); ImGui::EndTable();
        }
        ImGui::SeparatorText("Sequencer - Model Animation / Effect Elements");
        Render_Timeline();
    }
    ImGui::End();
    if (!m_Open) Deactivate();
}
}
