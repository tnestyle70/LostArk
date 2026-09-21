#include "imgui.h"
#include "MainApp.h"

#ifdef _DEBUG
#include "Camera_Free.h"
#include "GameInstance.h"
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
#include "WorldObjectTool.h"

#include <algorithm>
#include <cmath>

using namespace Client;

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

/* The World Level Tool's map edit session arms one viewport click. The loop
   mirrors UpdateMapEffectPlacementInput so the click keeps exactly one
   consumer: same foreground / ImGui / UI-router checks, same Esc and
   right-click cancel, same one-pixel readback. */
bool CMainApp::UpdateWorldLevelPlacementPickInput()
{
    const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (!leftDown && !rightDown) m_bWorldLevelPickSuppressMouse = false;
    if (!m_bWorldLevelPickArmed) return m_bWorldLevelPickSuppressMouse;
    const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
    const bool valid = nullptr != m_pWorldLevelTool && m_pWorldLevelTool->Is_PlacementPickArmed() &&
        m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL) &&
        m_eDebugInputOwner == DEBUG_TOOL::WORLD_LEVEL && currentLevel == m_iWorldLevelPickLevel &&
        m_pWorldLevelTool->Get_PlacementPickAreaId() == GetWorldLevelAreaId();
    const HWND foreground = GetForegroundWindow();
    DWORD foregroundProcess = 0u;
    if (foreground) GetWindowThreadProcessId(foreground, &foregroundProcess);
    if (!valid || foregroundProcess != GetCurrentProcessId() || rightDown ||
        (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
    {
        m_bWorldLevelPickArmed = false;
        if (nullptr != m_pWorldLevelTool)
            m_pWorldLevelTool->Cancel_PlacementPick(
                "Pick cancelled; the previous selection was preserved.");
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
        m_pWorldLevelTool->Set_Status(
            "No visible mesh surface at this pixel. Click a surface again, or Esc to cancel.");
        return true;
    }
    m_bWorldLevelPickArmed = false;
    m_pWorldLevelTool->Complete_PlacementPick({picked.x, picked.y, picked.z});
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
    /* The map edit session keeps running while the window is closed: a publish
       still has to report and a Level change still has to end it. */
    m_pWorldLevelTool->Update(m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL));
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

void CMainApp::RenderWorldLevelTool()
{
    if (!IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL) || !m_pWorldLevelTool) return;
    if (m_eDebugWindowFocusPending == DEBUG_TOOL::WORLD_LEVEL)
    { ImGui::SetNextWindowFocus(); m_eDebugWindowFocusPending = DEBUG_TOOL::NONE; }
    m_pWorldLevelTool->Render();
    if (m_pWorldLevelTool->Consume_InteractionRequest()) m_eDebugInputOwner = DEBUG_TOOL::WORLD_LEVEL;
    if (!m_pWorldLevelTool->Is_Open()) { SetDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL, false); return; }
    WORLD_LEVEL_TOOL_REQUEST request;
    if (!m_pWorldLevelTool->Consume_Request(request)) return;
    std::string status;
    if (request.kind == WORLD_LEVEL_REQUEST_KIND::FOCUS)
    {
        if (request.areaId != GetWorldLevelAreaId()) status = "Enter this Area before focusing its world position.";
        else (void)FocusWorldLevelPosition(request.position, 8.f, status);
    }
    else if (request.kind == WORLD_LEVEL_REQUEST_KIND::PICK_PLACEMENT)
    {
        if (request.areaId.empty() || request.areaId != GetWorldLevelAreaId())
        {
            m_bWorldLevelPickArmed = false;
            m_pWorldLevelTool->Cancel_PlacementPick({});
            status = "Enter the Level that owns " + request.areaId + " before picking its map objects.";
        }
        else
        {
            // One viewport click has one authoring consumer, even if Move Player was armed earlier.
            if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
                arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
            if (auto* select = CLevel_CharacterSelect::Get_Active())
                select->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
            if (auto* bern = CLevel_Bern::Get_Active())
                bern->Get_PlayerController().Cancel_DebugPlayerPlacement();
            m_bWorldLevelPickArmed = true;
            m_iWorldLevelPickLevel = CGameInstance::Get().Get_CurrentLevelID();
            m_bWorldLevelPickLeftDown = true;
            m_eDebugInputOwner = DEBUG_TOOL::WORLD_LEVEL;
            status = "Click a map object in the viewport once. Esc / right-click cancels.";
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
        if (!CMapEditorWorkspaceService::Is_Active() ||
            CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::DEVELOPMENT))
            status = "Open Lobby > Test to edit this Area in Map Tool. Current Level and unsaved drafts are preserved.";
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
