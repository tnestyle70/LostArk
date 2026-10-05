#include "imgui.h"
#include "MainApp.h"

#ifdef _DEBUG
#include "Camera_Free.h"
#include "GameInstance.h"
#include "GuideAITool.h"
#include "KoukuSaydonActionWorkbench.h"
#include "LevelRegistry.h"
#include "Level_Bern.h"
#include "Level_CharacterSelect.h"
#include "Level_KakulSaydonArena.h"
#include "Level_ValtanArena.h"
#include "MapEditorWorkspaceService.h"
#include "MapTool.h"
#include "PlayerController.h"
#include "SequencerTool.h"
#include "UIInputRouter.h"
#include "WorldLevelTool.h"
#include "WorldSceneTool.h"
#include "MapAuthoringHost.h"
#include "WorldObjectTool.h"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <sstream>

using namespace Client;

bool CMainApp::UpdateWorldMeshInspectionInput()
{
    const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (!leftDown && !rightDown) m_bWorldMeshPickSuppressMouse = false;
    const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
    if (!m_bWorldMeshPickArmed) return m_bWorldMeshPickSuppressMouse;
    if (auto* controller = Find_ActivePlayerController(); controller && controller->Is_DebugPlayerPlacementArmed())
    {
        m_bWorldMeshPickArmed = false;
        if (m_pWorldSceneTool) m_pWorldSceneTool->Set_Status("Move Player owns the next world click; mesh selection was preserved.");
        return m_bWorldMeshPickSuppressMouse;
    }
    auto* host = Find_ActiveMapAuthoringHost();
    const HWND foreground = GetForegroundWindow();
    DWORD process = 0u;
    if (foreground) GetWindowThreadProcessId(foreground, &process);
    if (!m_pWorldSceneTool || !m_bDeveloperToolsVisible || !IsDebugToolVisible(DEBUG_TOOL::WORLD_SCENE) ||
        !m_pWorldSceneTool->Is_Open() || m_eDebugInputOwner != DEBUG_TOOL::WORLD_SCENE ||
        !host || currentLevel != m_iWorldMeshPickLevel || process != GetCurrentProcessId() ||
        rightDown || (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
    {
        m_bWorldMeshPickArmed = false;
        m_bWorldMeshPickSuppressMouse = leftDown || rightDown;
        if (m_pWorldSceneTool) m_pWorldSceneTool->Set_Status("World pick cancelled; previous selection preserved.");
        return true;
    }
    const bool pressed = leftDown && !m_bWorldMeshPickLeftDown;
    m_bWorldMeshPickLeftDown = leftDown;
    if (!pressed || foreground != g_hWnd || ImGui::GetIO().WantCaptureMouse ||
        ImGui::GetIO().WantTextInput || CUIInputRouter::Get().Is_MouseClaimedThisFrame() ||
        CUIInputRouter::Get().Was_MouseClaimedLastFrame()) return true;
    CUIInputRouter::Get().Claim_Mouse_This_Frame();
    m_bWorldMeshPickSuppressMouse = true;
    vector_t origin, direction;
    float3_t rayOrigin{}, rayDirection{};
    if (CPlayerController::Try_PickWorldRay(origin, direction))
    {
        XMStoreFloat3(&rayOrigin, origin); XMStoreFloat3(&rayDirection, direction);
        const auto inCameraDepth = [](const float3_t& point) {
            const auto* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
            const auto* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
            if (!view || !projection) return false;
            float4_t clip{};
            XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(point.x, point.y, point.z, 1.f),
                XMLoadFloat4x4(view) * XMLoadFloat4x4(projection)));
            return std::isfinite(clip.w) && std::isfinite(clip.z) && clip.w > .0001f &&
                clip.z >= 0.f && clip.z <= clip.w;
        };
        MAP_WORLD_MESH_PICK mapPick; DEPLOY_WORLD_MESH_PICK deployPick;
        const bool mapHit = host->Get_MapAuthoringRuntime().Try_PickInspectionSurface(rayOrigin, rayDirection, mapPick) && inCameraDepth(mapPick.hitPosition);
        auto* deploy = host->Get_MapAuthoringDeployRuntime();
        const bool deployHit = deploy && deploy->Try_PickInspectionSurface(rayOrigin, rayDirection, deployPick) && inCameraDepth(deployPick.hitPosition);
        if (mapHit || deployHit)
        {
            const float mapDistance = mapHit ? XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&mapPick.hitPosition) - origin)) : FLT_MAX;
            const float deployDistance = deployHit ? XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&deployPick.hitPosition) - origin)) : FLT_MAX;
            if (deployHit && (!mapHit || deployDistance < mapDistance)) m_pWorldSceneTool->Complete_DeployPick(std::move(deployPick));
            else m_pWorldSceneTool->Complete_MapPick(std::move(mapPick));
            m_bWorldMeshPickArmed = false;
            return true;
        }
    }
    m_pWorldSceneTool->Set_Status("No live map or Deploy triangle hit. Click again, or Esc / right-click to cancel.");
    return true;
}

void CMainApp::RenderWorldMeshInspection()
{
    if (!Find_ActiveMapAuthoringHost()) return;
    ImGui::SeparatorText("World Scene");
    if (ImGui::Button("Open World Scene Tool")) (void)EnsureDebugTool(DEBUG_TOOL::WORLD_SCENE);
    ImGui::SameLine(); ImGui::TextDisabled("Live mesh pick / transform / map animation");
}

void CMainApp::RenderWorldSceneTool()
{
    if (!IsDebugToolVisible(DEBUG_TOOL::WORLD_SCENE) || !m_pWorldSceneTool) return;
    if (m_eDebugWindowFocusPending == DEBUG_TOOL::WORLD_SCENE)
    { ImGui::SetNextWindowFocus(); m_eDebugWindowFocusPending = DEBUG_TOOL::NONE; }
    m_pWorldSceneTool->Render();
    if (m_pWorldSceneTool->Consume_InteractionRequest()) m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
    if (!m_pWorldSceneTool->Is_Open()) { SetDebugToolVisible(DEBUG_TOOL::WORLD_SCENE, false); return; }
    float3_t focus{}; float radius = 8.f;
    if (m_pWorldSceneTool->Consume_FocusRequest(focus, radius))
    {
        std::string status; (void)FocusWorldLevelPosition(focus, radius, status); m_pWorldSceneTool->Set_Status(std::move(status));
    }
    if (m_pWorldSceneTool->Consume_PickRequest())
    {
        if (m_bWorldMeshPickArmed)
        {
            m_bWorldMeshPickArmed = false;
            m_pWorldSceneTool->Set_Status("World pick cancelled; previous selection preserved.");
        }
        else
        {
            m_bWorldLevelPickArmed = false;
            if (m_pGuideAITool) m_pGuideAITool->Cancel_PlacementPick("World Scene Tool owns the next click.");
            if (auto* bern = CLevel_Bern::Get_Active()) bern->Get_PlayerController().Cancel_DebugPlayerPlacement();
            if (auto* select = CLevel_CharacterSelect::Get_Active()) select->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
            if (auto* valtan = CLevel_ValtanArena::Get_Active()) valtan->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
            if (auto* kouku = CLevel_KakulSaydonArena::Get_Active()) kouku->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
            if (m_pMapEffectPlacementRequest)
            {
                auto* owner = m_eMapEffectPlacementOwner == DEBUG_TOOL::SEQUENCER ?
                    m_pKoukuSaydonActionWorkbench.get() : m_pSequenceActionWorkbench.get();
                if (owner) owner->Cancel_MapEffectPlacementRequest(m_pMapEffectPlacementRequest->iRequestToken);
                m_pMapEffectPlacementRequest.reset(); m_eMapEffectPlacementOwner = DEBUG_TOOL::NONE;
            }
            m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
            m_bWorldMeshPickArmed = true; m_bWorldMeshPickLeftDown = true;
            m_iWorldMeshPickLevel = CGameInstance::Get().Get_CurrentLevelID();
            m_pWorldSceneTool->Set_Status("Click a world mesh outside the UI. Esc / right-click cancels.");
        }
    }
    float3_t hit{};
    if (!m_pWorldSceneTool->Try_GetSelectedHit(hit)) return;
    const auto* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
    const auto* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
    if (!view || !projection) return;
    float4_t clip;
    XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(hit.x, hit.y, hit.z, 1.f),
        XMLoadFloat4x4(view) * XMLoadFloat4x4(projection)));
    if (!std::isfinite(clip.w) || clip.w <= .0001f || clip.z < 0.f || clip.z > clip.w) return;
    const float nx = clip.x / clip.w, ny = clip.y / clip.w;
    if (!std::isfinite(nx) || !std::isfinite(ny) || std::abs(nx) > 1.f || std::abs(ny) > 1.f) return;
    const auto* viewport = ImGui::GetMainViewport();
    const ImVec2 center(viewport->Pos.x + (nx * .5f + .5f) * viewport->Size.x,
        viewport->Pos.y + (.5f - ny * .5f) * viewport->Size.y);
    auto* draw = ImGui::GetBackgroundDrawList(); const auto color = IM_COL32(255, 220, 65, 255);
    draw->AddCircle(center, 10.f, color, 24, 2.f);
    draw->AddLine({center.x - 15.f, center.y}, {center.x + 15.f, center.y}, color, 2.f);
    draw->AddLine({center.x, center.y - 15.f}, {center.x, center.y + 15.f}, color, 2.f);
}

std::string CMainApp::GetWorldLevelAreaId() const
{
    const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
    if (level == LEVEL::DEVELOPMENT && m_pMapTool && CMapEditorWorkspaceService::Is_Active())
        return m_pMapTool->Debug_GetActiveAreaId();
    const auto* descriptor = CLevelRegistry::Find(level);
    return descriptor && descriptor->pMapAreaId ? descriptor->pMapAreaId : "";
}

bool CMainApp::FocusWorldLevelPosition(const float3_t& position, const float radius, std::string& status)
{
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
    { status = "The selected item has no finite world position."; return false; }
    const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
    shared_ptr<CCamera_Free> camera;
    if (level == LEVEL::KAKULSAYDON_ARENA)
    { if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) camera = arena->Get_DebugCamera(); }
    else if (level == LEVEL::VALTAN_ARENA)
    { if (auto* arena = CLevel_ValtanArena::Get_Active()) camera = arena->Get_DebugCamera(); }
    else if (level == LEVEL::CHARACTER_SELECT)
    { if (auto* arena = CLevel_CharacterSelect::Get_Active()) camera = arena->Get_DebugCamera(); }
    else if (level == LEVEL::BERN)
    { if (auto* bern = CLevel_Bern::Get_Active()) camera = bern->Get_DebugCamera(); }
    else if (level == LEVEL::DEVELOPMENT && m_pMapTool)
        camera = m_pMapTool->Debug_GetCamera();
    if (!camera) { status = "The current Level has no authoring camera ready."; return false; }
    if (camera->Is_PresentationOverrideActive())
    {
        if (m_eCompositionPreviewOwner != DEBUG_TOOL::NONE)
            StopCompositionPreview(m_eCompositionPreviewOwner);
        if (camera->Is_PresentationOverrideActive())
        { status = "Stop the active cinematic camera, then Focus this position."; return false; }
    }
    camera->Frame_Area(position, std::clamp(radius, 2.f, 200.f));
    status = "Camera focused on the selected world pivot. F6 returns to the player.";
    return true;
}

bool CMainApp::UpdateMapEffectPlacementInput()
{
    const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (!leftDown && !rightDown) m_bMapEffectPlacementSuppressMouse = false;
    const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
    const auto workbenchFor = [this](DEBUG_TOOL owner) -> CKoukuSaydonActionWorkbench* {
        if (owner == DEBUG_TOOL::SEQUENCER) return m_pKoukuSaydonActionWorkbench.get();
        if (owner == DEBUG_TOOL::SEQUENCER_BENCHMARK) return m_pSequenceActionWorkbench.get();
        return nullptr;
    };
    const auto cancel = [&]() {
        if (m_pMapEffectPlacementRequest)
            if (auto* workbench = workbenchFor(m_eMapEffectPlacementOwner))
                workbench->Cancel_MapEffectPlacementRequest(m_pMapEffectPlacementRequest->iRequestToken);
        m_pMapEffectPlacementRequest.reset();
        m_eMapEffectPlacementOwner = DEBUG_TOOL::NONE;
    };
    for (const auto owner : {DEBUG_TOOL::SEQUENCER, DEBUG_TOOL::SEQUENCER_BENCHMARK})
    {
        auto* workbench = workbenchFor(owner);
        KOUKU_MAP_EFFECT_PLACEMENT_REQUEST request;
        if (!workbench || !workbench->Consume_MapEffectPlacementRequest(request)) continue;
        cancel();
        if (!m_bDeveloperToolsVisible || !IsDebugToolVisible(owner) ||
            !workbench->Is_MapEffectPlacementRequestCurrent(request) ||
            workbench->Get_Composition().strAreaId != GetWorldLevelAreaId())
        {
            workbench->Cancel_MapEffectPlacementRequest(request.iRequestToken);
            m_strMapEffectPlacementStatus = "Placement requires this box's Area to be the active Level.";
            continue;
        }
        m_eDebugInputOwner = owner;
        if (request.eAction == KOUKU_MAP_EFFECT_PLACEMENT_ACTION::FOCUS)
        {
            const auto& p = request.Selection.Position;
            (void)FocusWorldLevelPosition({float(p[0]), float(p[1]), float(p[2])}, 8.f,
                m_strMapEffectPlacementStatus);
            workbench->Cancel_MapEffectPlacementRequest(request.iRequestToken);
            continue;
        }
        // One viewport click has one authoring consumer, even if Move Player was armed earlier.
        if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
            arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
        if (auto* arena = CLevel_ValtanArena::Get_Active())
            arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
        if (auto* bern = CLevel_Bern::Get_Active())
            bern->Get_PlayerController().Cancel_DebugPlayerPlacement();
        m_pMapEffectPlacementRequest = std::make_unique<KOUKU_MAP_EFFECT_PLACEMENT_REQUEST>(request);
        m_eMapEffectPlacementOwner = owner;
        m_iMapEffectPlacementLevel = currentLevel;
        m_bMapEffectPlacementLeftDown = true;
        m_strMapEffectPlacementStatus = "Click a visible surface once. Esc / right-click cancels. Apply and Save keep the position.";
    }
    if (!m_pMapEffectPlacementRequest) return m_bMapEffectPlacementSuppressMouse;
    auto* workbench = workbenchFor(m_eMapEffectPlacementOwner);
    const bool valid = workbench && m_bDeveloperToolsVisible && IsDebugToolVisible(m_eMapEffectPlacementOwner) &&
        m_eDebugInputOwner == m_eMapEffectPlacementOwner && currentLevel == m_iMapEffectPlacementLevel &&
        workbench->Get_Composition().strAreaId == GetWorldLevelAreaId() &&
        workbench->Is_MapEffectPlacementRequestCurrent(*m_pMapEffectPlacementRequest);
    const HWND foreground = GetForegroundWindow();
    DWORD foregroundProcess = 0u;
    if (foreground) GetWindowThreadProcessId(foreground, &foregroundProcess);
    if (!valid || foregroundProcess != GetCurrentProcessId() || rightDown ||
        (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
    {
        cancel();
        m_bMapEffectPlacementSuppressMouse = leftDown || rightDown;
        m_strMapEffectPlacementStatus = "Placement cancelled; the previous box position was preserved.";
        return true;
    }
    const bool pressed = leftDown && !m_bMapEffectPlacementLeftDown;
    m_bMapEffectPlacementLeftDown = leftDown;
    if (!pressed || foreground != g_hWnd || ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput ||
        CUIInputRouter::Get().Is_MouseClaimedThisFrame() || CUIInputRouter::Get().Was_MouseClaimedLastFrame())
        return true;
    CUIInputRouter::Get().Claim_Mouse_This_Frame();
    m_bMapEffectPlacementSuppressMouse = true;
    float4_t picked{};
    if (!CGameInstance::Get().Picking(picked) || !std::isfinite(picked.x) ||
        !std::isfinite(picked.y) || !std::isfinite(picked.z))
    {
        m_strMapEffectPlacementStatus = "No visible mesh surface at this pixel. Click a surface again, or Esc to cancel.";
        return true;
    }
    const bool completed = workbench->Complete_MapEffectPlacementRequest(*m_pMapEffectPlacementRequest,
        {picked.x, picked.y, picked.z}, m_strMapEffectPlacementStatus);
    cancel();
    if (completed) m_strMapEffectPlacementStatus = "Picked world position staged in Box Detail. Preview now; Apply and Save keep it.";
    return true;
}

/* Guide trigger boxes consume a visible world position. Map mesh identity
   uses UpdateWorldMeshInspectionInput and never this GPU position readback. */
bool CMainApp::UpdateWorldLevelPlacementPickInput()
{
    const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (!leftDown && !rightDown) m_bWorldLevelPickSuppressMouse = false;
    if (!m_bWorldLevelPickArmed) return m_bWorldLevelPickSuppressMouse;
    const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
    const bool targetReady = m_eWorldLevelPickOwner == DEBUG_TOOL::GUIDE_AI &&
        m_pGuideAITool && m_pGuideAITool->Is_PlacementPickArmed() &&
        m_pGuideAITool->Get_PlacementPickAreaId() == GetWorldLevelAreaId();
    const bool valid = targetReady && m_bDeveloperToolsVisible && IsDebugToolVisible(m_eWorldLevelPickOwner) &&
        m_eDebugInputOwner == m_eWorldLevelPickOwner && currentLevel == m_iWorldLevelPickLevel;
    const HWND foreground = GetForegroundWindow();
    DWORD foregroundProcess = 0u;
    if (foreground) GetWindowThreadProcessId(foreground, &foregroundProcess);
    if (!valid || foregroundProcess != GetCurrentProcessId() || rightDown ||
        (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
    {
        m_bWorldLevelPickArmed = false;
        if (m_pGuideAITool)
            m_pGuideAITool->Cancel_PlacementPick("Pick cancelled; the previous Guide box position was preserved.");
        m_bWorldLevelPickSuppressMouse = leftDown || rightDown;
        return true;
    }
    const bool pressed = leftDown && !m_bWorldLevelPickLeftDown;
    m_bWorldLevelPickLeftDown = leftDown;
    if (!pressed || foreground != g_hWnd || ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput ||
        CUIInputRouter::Get().Is_MouseClaimedThisFrame() || CUIInputRouter::Get().Was_MouseClaimedLastFrame())
        return true;
    CUIInputRouter::Get().Claim_Mouse_This_Frame();
    m_bWorldLevelPickSuppressMouse = true;
    float4_t picked{};
    if (!CGameInstance::Get().Picking(picked) || !std::isfinite(picked.x) ||
        !std::isfinite(picked.y) || !std::isfinite(picked.z))
    {
        /* No surface under the pixel: stay armed so the next click can hit. */
        const std::string status = "No visible mesh surface at this pixel. Click a surface again, or Esc to cancel.";
        m_pGuideAITool->Set_Status(status);
        return true;
    }
    m_bWorldLevelPickArmed = false;
    m_pGuideAITool->Complete_PlacementPick({picked.x, picked.y, picked.z});
    return true;
}

void CMainApp::RenderMapEffectPlacementMarker()
{
    if (!m_bDeveloperToolsVisible) return;
    auto* workbench = m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER ? m_pKoukuSaydonActionWorkbench.get() :
        m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK ? m_pSequenceActionWorkbench.get() : nullptr;
    KOUKU_MAP_EFFECT_PLACEMENT selection;
    if (!workbench || !IsDebugToolVisible(m_eDebugInputOwner) ||
        workbench->Get_Composition().strAreaId != GetWorldLevelAreaId() ||
        !workbench->Get_SelectedMapEffectPlacement(selection)) return;
    const auto* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
    const auto* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
    if (!view || !projection) return;
    const auto viewProjection = XMLoadFloat4x4(view) * XMLoadFloat4x4(projection);
    const auto* viewport = ImGui::GetMainViewport();
    const auto project = [&](double x, double y, double z, ImVec2& screen) {
        float4_t clip;
        XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(float(x), float(y), float(z), 1.f), viewProjection));
        if (!std::isfinite(clip.w) || clip.w <= 0.0001f || clip.z < 0.f || clip.z > clip.w) return false;
        const float nx = clip.x / clip.w, ny = clip.y / clip.w;
        if (!std::isfinite(nx) || !std::isfinite(ny) || std::abs(nx) > 1.f || std::abs(ny) > 1.f) return false;
        screen = {viewport->Pos.x + (nx * .5f + .5f) * viewport->Size.x,
            viewport->Pos.y + (.5f - ny * .5f) * viewport->Size.y};
        return true;
    };
    auto* draw = ImGui::GetBackgroundDrawList();
    const auto& p = selection.Position;
    ImVec2 center;
    const bool onScreen = project(p[0], p[1], p[2], center);
    const ImU32 color = IM_COL32(255, 220, 65, 255);
    if (onScreen)
    {
        draw->AddCircle(center, 10.f, color, 24, 2.f);
        draw->AddLine({center.x - 16.f, center.y}, {center.x + 16.f, center.y}, color, 2.f);
        draw->AddLine({center.x, center.y - 16.f}, {center.x, center.y + 16.f}, color, 2.f);
        const ImU32 colors[] = {IM_COL32(255, 80, 80, 255), IM_COL32(80, 255, 110, 255), IM_COL32(85, 155, 255, 255)};
        const char* axes[] = {"X", "Y", "Z"};
        for (size_t axis = 0; axis < 3; ++axis)
        {
            auto end = p; end[axis] += 1.;
            ImVec2 endpoint;
            if (project(end[0], end[1], end[2], endpoint))
            { draw->AddLine(center, endpoint, colors[axis], 2.f); draw->AddText(endpoint, colors[axis], axes[axis]); }
        }
    }
    char label[256]{};
    sprintf_s(label, "MAP Effect pivot%s | XYZ %.3f / %.3f / %.3f m | box %.3f - %.3f s",
        onScreen ? "" : " OFF SCREEN - Focus (F) in Box Detail", p[0], p[1], p[2],
        selection.iStartMs / 1000., (double(selection.iStartMs) + selection.iDurationMs) / 1000.);
    const ImVec2 textPosition = onScreen ? ImVec2(center.x + 18.f, center.y + 18.f) :
        ImVec2(viewport->Pos.x + 20.f, viewport->Pos.y + viewport->Size.y - 85.f);
    draw->AddText({textPosition.x + 1.f, textPosition.y + 1.f}, IM_COL32(0, 0, 0, 255), label);
    draw->AddText(textPosition, color, label);
    if (!m_strMapEffectPlacementStatus.empty())
        draw->AddText({viewport->Pos.x + 20.f, viewport->Pos.y + viewport->Size.y - 60.f}, color,
            m_strMapEffectPlacementStatus.c_str());
}

void CMainApp::UpdateWorldLevelTool()
{
    if (!m_pWorldLevelTool) return;
    m_pWorldLevelTool->Set_ActiveArea(GetWorldLevelAreaId());
    for (const auto owner : {WORLD_LEVEL_COMPOSITION_OWNER::ACTION, WORLD_LEVEL_COMPOSITION_OWNER::SEQUENCE})
    {
        auto* workbench = owner == WORLD_LEVEL_COMPOSITION_OWNER::ACTION ?
            m_pKoukuSaydonActionWorkbench.get() : m_pSequenceActionWorkbench.get();
        m_pWorldLevelTool->Set_CompositionView(owner,
            workbench && workbench->Has_Composition() ? &workbench->Get_Composition() : nullptr,
            workbench ? workbench->Get_DraftGeneration() : 0u);
    }
    if (!m_pWorldLevelPendingMapRequest) return;
    if (!m_pMapTool || !m_bDeveloperToolsVisible || !IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL) ||
        std::chrono::steady_clock::now() > m_WorldLevelMapDeadline)
    {
        m_pWorldLevelPendingMapRequest.reset();
        m_pWorldLevelTool->Set_Status("Map selection ended; existing owner drafts were preserved.");
        return;
    }
    const auto request = *m_pWorldLevelPendingMapRequest;
    std::string status;
    const int result = !request.sequenceInstanceId.empty() || !request.sourceItemId.empty() ?
        m_pMapTool->Debug_SequenceViewer(request.areaId, request.sequenceInstanceId, request.sourceItemId, false, false, nullptr, status) :
        m_pMapTool->Debug_WorldLevelSelection(request.areaId, request.placementId, request.deploy, status);
    m_pWorldLevelTool->Set_Status(status);
    if (result != 0)
    {
        m_pWorldLevelPendingMapRequest.reset();
        if (result > 0) { m_eDebugInputOwner = DEBUG_TOOL::MAP; m_eDebugWindowFocusPending = DEBUG_TOOL::MAP; }
    }
}

namespace
{
bool ClipChunkOverlayEdge(float4_t& a, float4_t& b)
{
    const auto finite = [](const float4_t& p) { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) && std::isfinite(p.w); };
    if (!finite(a) || !finite(b)) return false;
    const double av[] = {double(a.w) - .0001, double(a.x) + a.w, double(a.w) - a.x,
        double(a.y) + a.w, double(a.w) - a.y, a.z, double(a.w) - a.z};
    const double bv[] = {double(b.w) - .0001, double(b.x) + b.w, double(b.w) - b.x,
        double(b.y) + b.w, double(b.w) - b.y, b.z, double(b.w) - b.z};
    double first = 0., last = 1.;
    for (size_t plane = 0; plane < 7; ++plane)
    {
        if (av[plane] < 0. && bv[plane] < 0.) return false;
        if ((av[plane] < 0.) != (bv[plane] < 0.))
        {
            const double t = av[plane] / (av[plane] - bv[plane]);
            if (av[plane] < 0.) first = (std::max)(first, t); else last = (std::min)(last, t);
        }
    }
    if (first > last) return false;
    const auto lerp = [&](double t) { return float4_t{float(a.x + (double(b.x) - a.x) * t),
        float(a.y + (double(b.y) - a.y) * t), float(a.z + (double(b.z) - a.z) * t), float(a.w + (double(b.w) - a.w) * t)}; };
    const float4_t begin = lerp(first), end = lerp(last); a = begin; b = end;
    return finite(a) && finite(b) && a.w > 0.f && b.w > 0.f;
}

void DrawWorldLevelChunkBounds(const CWorldLevelTool& tool)
{
    if (!tool.Show_ChunkBounds()) return;
    auto& game = CGameInstance::Get();
    const auto* view = game.Get_Transform(D3DTS::VIEW);
    const auto* projection = game.Get_Transform(D3DTS::PROJ);
    if (!view || !projection) return;
    const matrix_t vp = XMLoadFloat4x4(view) * XMLoadFloat4x4(projection);
    auto* viewport = ImGui::GetMainViewport();
    if (!viewport || viewport->Size.x <= 0.f || viewport->Size.y <= 0.f) return;
    auto* draw = ImGui::GetBackgroundDrawList(viewport);
    const auto screen = [viewport](const float4_t& p) { return ImVec2(viewport->Pos.x + (p.x / p.w * .5f + .5f) * viewport->Size.x,
        viewport->Pos.y + (.5f - p.y / p.w * .5f) * viewport->Size.y); };
    constexpr uint8_t edges[12][2] = {{0,1},{2,3},{4,5},{6,7},{0,2},{1,3},{4,6},{5,7},{0,4},{1,5},{2,6},{3,7}};
    for (const auto& row : tool.Get_ChunkView())
    {
        const bool selected = row.chunkId == tool.Get_SelectedChunk();
        if (!tool.Show_AllChunkBounds() && !selected) continue;
        const auto& lo = row.minimum; const auto& hi = row.maximum;
        if (!std::isfinite(lo.x) || !std::isfinite(lo.y) || !std::isfinite(lo.z) ||
            !std::isfinite(hi.x) || !std::isfinite(hi.y) || !std::isfinite(hi.z) ||
            lo.x > hi.x || lo.y > hi.y || lo.z > hi.z) continue;
        float4_t corners[8];
        for (size_t i = 0; i < 8; ++i) XMStoreFloat4(&corners[i], XMVector4Transform(
            XMVectorSet(i & 1 ? hi.x : lo.x, i & 2 ? hi.y : lo.y, i & 4 ? hi.z : lo.z, 1.f), vp));
        const ImU32 color = selected ? IM_COL32(255, 220, 65, 240) : !row.valid ? IM_COL32(255, 95, 95, 150) :
            row.farSelected && row.active ? IM_COL32(80, 230, 150, 150) : IM_COL32(90, 175, 255, 140);
        for (const auto& edge : edges)
        {
            auto a = corners[edge[0]], b = corners[edge[1]];
            if (ClipChunkOverlayEdge(a, b)) draw->AddLine(screen(a), screen(b), color, selected ? 2.f : 1.f);
        }
    }
}
}


void CMainApp::RenderWorldLevelTool()
{
    if (!IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL) || !m_pWorldLevelTool) return;
    if (m_eDebugWindowFocusPending == DEBUG_TOOL::WORLD_LEVEL)
    { ImGui::SetNextWindowFocus(); m_eDebugWindowFocusPending = DEBUG_TOOL::NONE; }
    auto* chunkHost = Find_ActiveMapAuthoringHost();
    auto* chunkRuntime = chunkHost ? &chunkHost->Get_MapAuthoringRuntime() : nullptr;
    const std::string chunkArea = chunkHost ? chunkHost->Get_MapAuthoringCatalog().Get_AreaId() : GetWorldLevelAreaId();
    const uint64_t chunkGeneration = chunkRuntime ? chunkRuntime->Debug_GetRuntimeGeneration() : 0u;
    if (m_pWorldLevelTool->Needs_ChunkView(chunkArea, chunkRuntime, chunkGeneration))
    {
        const auto policy = chunkRuntime ? chunkRuntime->Get_ChunkPolicy() : nullptr;
        m_pWorldLevelTool->Set_ChunkView(chunkArea, chunkRuntime, chunkGeneration,
            chunkRuntime ? chunkRuntime->Get_ChunkDebugRows() : std::vector<MAP_CHUNK_DEBUG_ROW>{},
            policy && policy->enabled, policy && policy->hlodEnabled);
    }
    m_pWorldLevelTool->Render();
    DrawWorldLevelChunkBounds(*m_pWorldLevelTool);
    if (m_pWorldLevelTool->Consume_InteractionRequest()) m_eDebugInputOwner = DEBUG_TOOL::WORLD_LEVEL;
    if (!m_pWorldLevelTool->Is_Open()) { SetDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL, false); return; }
    WORLD_LEVEL_TOOL_REQUEST request;
    if (!m_pWorldLevelTool->Consume_Request(request)) return;
    std::string status;
    if (request.kind == WORLD_LEVEL_REQUEST_KIND::SET_CHUNK_MODE)
    {
        if (!chunkRuntime || request.areaId != chunkArea || request.runtimeGeneration != chunkGeneration)
            status = "Chunk source changed; refresh the live Area before changing its display mode.";
        else if (const auto& policy = chunkRuntime->Get_ChunkPolicy())
        {
            policy->enabled = request.chunkEnabled;
            policy->hlodEnabled = request.chunkHlodEnabled;
            status = "Runtime chunk display mode changed. Saved placements and materials are unchanged.";
        }
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_GUIDE)
    {
        status = SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::GUIDE_AI)) ? "DimensionMaster Guide opened." : "DimensionMaster Guide could not open.";
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::FOCUS)
    {
        if (request.areaId != GetWorldLevelAreaId()) status = "Enter this Area before focusing its world position.";
        else (void)FocusWorldLevelPosition(request.position, request.focusRadius, status);
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::PICK_IN_SCENE)
    {
        const auto* host = Find_ActiveMapAuthoringHost();
        if (!host || request.areaId != host->Get_MapAuthoringCatalog().Get_AreaId())
            status = "Enter this Area before selecting its live meshes.";
        else if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_SCENE))) status = "World Scene Tool could not open.";
        else
        {
            m_pWorldSceneTool->Update(0.f, true);
            m_pWorldSceneTool->Request_ScenePick();
            m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
            m_eDebugWindowFocusPending = DEBUG_TOOL::WORLD_SCENE;
            status = "Pick in scene opened. Click a visible map or Deploy object outside the UI.";
        }
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_LIGHT)
    {
        LEVEL renderingLevel = LEVEL::END;
        for (const auto level : {LEVEL::KAKULSAYDON_ARENA, LEVEL::VALTAN_ARENA, LEVEL::BERN, LEVEL::CHARACTER_SELECT})
            if (const auto* descriptor = CLevelRegistry::Find(level); descriptor && descriptor->pMapAreaId &&
                request.areaId == descriptor->pMapAreaId) renderingLevel = level;
        if (request.areaId != GetWorldLevelAreaId()) status = "Enter this Area before editing its live map lights.";
        else if (renderingLevel == LEVEL::END) status = "This Area has no Rendering Workbench Level binding; its lights are listed for inspection.";
        else if (m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() != request.areaId)
            status = "Rendering Workbench has another Area open. Finish its light edits before switching Areas.";
        else if (FAILED(EnsureDebugTool(DEBUG_TOOL::RENDERING))) status = "Rendering Workbench could not initialize.";
        else if (!m_AreaLightSession.Is_Open() && !m_AreaLightSession.Open(request.areaId, status)) {}
        else
        {
            const auto& lights = m_AreaLightSession.Get_Document().Get_Lights();
            if (std::none_of(lights.begin(), lights.end(), [&](const auto& light) { return light.lightId == request.sourceItemId; }))
                status = "The selected light is absent from the current Rendering Workbench draft.";
            else
            {
                m_iRenderingLastLevel = CGameInstance::Get().Get_CurrentLevelID();
                m_eRenderingSelectedLevel = renderingLevel;
                m_strRenderingLightAreaAttempt = request.areaId;
                SelectRenderingLight("map:" + request.sourceItemId);
                m_eDebugInputOwner = DEBUG_TOOL::RENDERING;
                m_eDebugWindowFocusPending = DEBUG_TOOL::RENDERING;
                status = "Selected the map light in Rendering Workbench. Its existing Save controls own changes.";
            }
        }
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_WORLD_OBJECT)
    {
        if (request.areaId != "LV_LUT_MIDNIGHTC_ED") status = "This Area's Object motions are edited in Map Tool.";
        else if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT))) status = "Object Tool could not initialize.";
        else if (request.objectId.empty() && request.sequenceInstanceId.empty()) status = "World Object Tool opened. Create an Object in its Resources pane.";
        else (void)m_pWorldObjectTool->Open_ObjectMotion(request.objectId, request.sequenceInstanceId, status);
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_COMPOSITION)
    {
        const bool sequence = request.compositionOwner == WORLD_LEVEL_COMPOSITION_OWNER::SEQUENCE;
        const auto owner = sequence ? DEBUG_TOOL::SEQUENCER_BENCHMARK : DEBUG_TOOL::SEQUENCER;
        auto* shell = m_pSequencerTool.get();
        if (!shell && FAILED(EnsureDebugTool(owner))) status = "Composition could not initialize.";
        else
        {
            auto* workbench = sequence ? m_pSequenceActionWorkbench.get() : m_pKoukuSaydonActionWorkbench.get();
            shell = m_pSequencerTool.get();
            if (!workbench->Has_Composition() && !workbench->Reload(status)) {}
            else if (workbench->Get_Composition().strAreaId != request.areaId)
                status = "This Area's sequences are edited in Map Tool; this Composition belongs to another Area.";
            else
            {
                const auto& patterns = workbench->Get_Composition().Patterns;
                const auto pattern = std::find_if(patterns.begin(), patterns.end(), [&](const auto& value) {
                    return value.strPatternId == request.patternId;
                });
                auto boss = shell->Get_SelectedBoss();
                if (pattern != patterns.end())
                {
                    const auto& gate = pattern->strGateId;
                    boss = gate == "GATE2" ? COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2 :
                        gate == "GATE3" ? COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE3 :
                        gate == "BINGO" ? COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_ENCORE :
                        COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON;
                }
                else if (boss == COMPOSITION_WORKBENCH_BOSS::VALTAN) boss = COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON;
                // Select the target first: opening the matching gate must not discard the same box's edits.
                shell->Open(sequence ? COMPOSITION_WORKBENCH_TARGET::SEQUENCE :
                    COMPOSITION_WORKBENCH_TARGET::BOSS, boss);
                bool selected = request.patternId.empty();
                if (!request.occurrenceId.empty())
                {
                    const bool worldBox = pattern != patterns.end() &&
                        std::any_of(pattern->WorldOccurrences.begin(), pattern->WorldOccurrences.end(), [&](const auto& value) {
                            return value.strOccurrenceId == request.occurrenceId;
                        });
                    selected = worldBox ? workbench->Select_WorldBoxById(request.patternId, request.occurrenceId, status) :
                        workbench->Select_PresentationBoxById(request.patternId, request.occurrenceId, status);
                }
                else if (!request.patternId.empty()) selected = workbench->Select_PatternById(request.patternId, status);
                else status = "Composition opened. Create a Sequence/Pattern using its existing authoring controls.";
                if (selected)
                {
                    SetDebugToolVisible(owner, true);
                    m_eDebugInputOwner = owner; m_eDebugWindowFocusPending = owner;
                }
            }
        }
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_MAP)
    {
        const auto* host = Find_ActiveMapAuthoringHost();
        if (host && request.areaId == host->Get_MapAuthoringCatalog().Get_AreaId() &&
            request.sequenceInstanceId.empty() && request.sourceItemId.empty())
        {
            if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_SCENE))) status = "World Scene Tool could not open.";
            else
            {
                m_pWorldSceneTool->Update(0.f, true);
                const bool selected = request.placementId == 0u ||
                    m_pWorldSceneTool->Inspect_Placement(request.placementId, request.deploy);
                status = selected ? "World Scene Tool opened on this Level. Pick in scene inspects meshes; enable editing only to change placements." :
                    "This saved placement is outside the current live map scope. Previous scene selection was preserved.";
                m_pWorldSceneTool->Set_Status(status);
                m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
                m_eDebugWindowFocusPending = DEBUG_TOOL::WORLD_SCENE;
            }
        }
        else if (!CMapEditorWorkspaceService::Is_Active() ||
            CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::DEVELOPMENT))
            status = "This item needs its owning Area/editor. Scene picking remains available for the current Level; existing drafts are preserved.";
        else if (FAILED(EnsureDebugTool(DEBUG_TOOL::MAP))) status = "Map Tool could not initialize.";
        else
        {
            m_pWorldLevelPendingMapRequest = std::make_unique<WORLD_LEVEL_TOOL_REQUEST>(request);
            m_WorldLevelMapDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
            status = "Preparing the selected Area in Map Tool...";
        }
    }
    m_pWorldLevelTool->Set_Status(std::move(status));
}
#endif
