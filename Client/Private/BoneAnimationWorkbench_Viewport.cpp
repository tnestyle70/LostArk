#include "imgui.h"
#include "BoneAnimationWorkbench.h"
#include "AnimationTargetService.h"
#include "GameInstance.h"
#include "Model.h"
#include "UIInputRouter.h"
#include <cmath>
#include <limits>
#include <vector>

namespace Client
{
void CBoneAnimationWorkbench::Render_Viewport()
{
    if (!m_ShowSkeleton || !ImGui::GetCurrentContext()) return;
    const auto model = m_Model.lock();
    ANIMATION_MODEL_TARGET_VIEW target;
    if (!model || !CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::BODY, target) ||
        target.Model != model) return;

    auto& game = Engine::CGameInstance::Get();
    const auto* view = game.Get_Transform(D3DTS::VIEW);
    const auto* projection = game.Get_Transform(D3DTS::PROJ);
    auto* viewport = ImGui::GetMainViewport();
    if (!view || !projection || !viewport || viewport->Size.x <= 0.f || viewport->Size.y <= 0.f) return;
    const auto names = model->Get_BoneNames();
    if (names.empty() || names.size() > 512u) return;

    // CModel's combined bone matrix already includes the model pre-transform.
    // Match the socket consumer: combined * actual preview root, exactly once.
    const matrix_t rootViewProjection = XMLoadFloat4x4(&target.BoneRoot) *
        XMLoadFloat4x4(view) * XMLoadFloat4x4(projection);
    struct PROJECTED_BONE final
    {
        ImVec2 position{};
        float depth = 0.f;
        int32_t parent = -1;
        bool visible = false;
    };
    std::vector<PROJECTED_BONE> points(names.size());
    for (uint32_t index = 0u; index < names.size(); ++index)
    {
        auto& point = points[index];
        point.parent = model->Get_BoneParentIndex(index);
        matrix_t combined;
        if (!model->Get_BoneCombinedMatrix(index, combined)) continue;
        float4_t clip{};
        XMStoreFloat4(&clip, XMVector4Transform(combined.r[3], rootViewProjection));
        if (!std::isfinite(clip.x) || !std::isfinite(clip.y) || !std::isfinite(clip.z) ||
            !std::isfinite(clip.w) || clip.w <= .00001f || clip.z < 0.f || clip.z > clip.w ||
            std::abs(clip.x) > clip.w || std::abs(clip.y) > clip.w) continue;
        point.position = {viewport->Pos.x + (.5f + .5f * clip.x / clip.w) * viewport->Size.x,
            viewport->Pos.y + (.5f - .5f * clip.y / clip.w) * viewport->Size.y};
        point.depth = clip.z / clip.w;
        point.visible = true;
    }

    const int32_t selected = model->Find_BoneIndex(m_Bone.c_str());
    auto* draw = ImGui::GetBackgroundDrawList(viewport);
    draw->PushClipRect(viewport->Pos,
        {viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y}, true);
    for (uint32_t index = 0u; index < points.size(); ++index)
    {
        const auto& point = points[index];
        if (!point.visible || point.parent < 0 || static_cast<size_t>(point.parent) >= points.size() ||
            !points[point.parent].visible) continue;
        const bool connected = static_cast<int32_t>(index) == selected || point.parent == selected;
        draw->AddLine(points[point.parent].position, point.position,
            connected ? IM_COL32(255, 205, 70, 245) : IM_COL32(90, 210, 245, 160), connected ? 2.5f : 1.f);
    }
    for (uint32_t index = 0u; index < points.size(); ++index)
    {
        if (!points[index].visible) continue;
        const bool chosen = static_cast<int32_t>(index) == selected;
        draw->AddCircleFilled(points[index].position, chosen ? 5.f : 2.5f,
            chosen ? IM_COL32(255, 225, 95, 255) : IM_COL32(120, 230, 255, 210));
        if (chosen) draw->AddCircle(points[index].position, 8.f, IM_COL32(255, 225, 95, 255), 16, 1.5f);
    }
    if (selected >= 0 && static_cast<size_t>(selected) < points.size() && points[selected].visible)
    {
        const auto position = points[selected].position;
        const auto size = ImGui::CalcTextSize(m_Bone.c_str());
        const float left = (std::max)(viewport->Pos.x, (std::min)(position.x + 11.f,
            viewport->Pos.x + viewport->Size.x - size.x - 8.f));
        const float top = (std::max)(viewport->Pos.y, (std::min)(position.y - size.y - 8.f,
            viewport->Pos.y + viewport->Size.y - size.y - 6.f));
        draw->AddRectFilled({left, top}, {left + size.x + 8.f, top + size.y + 6.f},
            IM_COL32(15, 20, 30, 220), 3.f);
        draw->AddText({left + 4.f, top + 3.f}, IM_COL32(255, 225, 95, 255), m_Bone.c_str());
    }
    draw->PopClipRect();

    // Passive labels only. Rendering happens after gameplay input, so a mouse
    // claim here could not prevent an already-issued attack on the same frame.
    // Selection stays in the bone hierarchy; this overlay never consumes clicks.
    const auto& io = ImGui::GetIO();
    const auto& ui = CUIInputRouter::Get();
    if (GetForegroundWindow() != g_hWnd || io.WantCaptureMouse || io.WantTextInput ||
        ui.Is_TextInputActive() || ui.Is_CinematicSuppressed() || ui.Is_MouseClaimedThisFrame() ||
        ui.Was_MouseClaimedLastFrame() || ImGui::IsAnyItemActive() ||
        ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::GetDragDropPayload() ||
        ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) return;
    const auto mouse = ImGui::GetMousePos();
    if (mouse.x < viewport->Pos.x || mouse.y < viewport->Pos.y ||
        mouse.x >= viewport->Pos.x + viewport->Size.x || mouse.y >= viewport->Pos.y + viewport->Size.y) return;
    size_t hovered = points.size();
    float nearestDistance = 81.f;
    float nearestDepth = (std::numeric_limits<float>::max)();
    for (size_t index = 0u; index < points.size(); ++index)
    {
        const auto& point = points[index];
        if (!point.visible) continue;
        const float x = point.position.x - mouse.x, y = point.position.y - mouse.y;
        const float distance = x * x + y * y;
        if (distance > nearestDistance || (distance == nearestDistance && point.depth >= nearestDepth)) continue;
        hovered = index; nearestDistance = distance; nearestDepth = point.depth;
    }
    if (hovered < names.size())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(names[hovered].c_str());
        const auto parent = points[hovered].parent;
        if (parent >= 0 && static_cast<size_t>(parent) < names.size()) ImGui::Text("Parent: %s", names[parent].c_str());
        ImGui::TextDisabled("Select this bone in the Bone hierarchy panel.");
        ImGui::EndTooltip();
    }
}
}
