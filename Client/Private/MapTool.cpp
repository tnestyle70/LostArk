#include "imgui.h"
#include "MapTool_Internal.h"
#include "WorldSequenceToolPanel.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "MapEditorWorkspaceService.h"
#include "MapStaticBatchObject.h"
#include "MapAssetPreview.h"
#include "MapAssetObject.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "ValtanCinematicCameraController.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

#include "MapTool_NavigationResources.h"



Client::CMapTool::CMapTool() = default;
Client::CMapTool::~CMapTool() = default;

HRESULT Client::CMapTool::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto preview = std::make_unique<CMapAssetPreview>();
	if (FAILED(preview->Initialize(pDevice, pContext)))
		return E_FAIL;

	auto navigationResources =
		std::make_unique<NAVIGATION_RENDER_RESOURCES>();
	auto destructionSimulationController =
		std::make_unique<CDestructionSimulationController>();
	auto worldSequenceToolPanel =
		std::make_unique<CWorldSequenceToolPanel>();
	navigationResources->pBatch =
		make_shared<PrimitiveBatch<VertexPositionColor>>(pContext.Get());
	navigationResources->pEffect =
		make_shared<BasicEffect>(pDevice.Get());
	navigationResources->pEffect->SetVertexColorEnabled(true);

	const void* vertexShaderByteCode = nullptr;
	size_t byteCodeLength = {};
	navigationResources->pEffect->GetVertexShaderBytecode(
		&vertexShaderByteCode,
		&byteCodeLength);
	if (FAILED(pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		vertexShaderByteCode,
		byteCodeLength,
		navigationResources->pInputLayout.GetAddressOf())))
	{
		return E_FAIL;
	}

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_pAssetPreview = std::move(preview);
	m_pNavigationRenderResources = std::move(navigationResources);
	m_pDestructionSimulationController =
		std::move(destructionSimulationController);
	m_pWorldSequenceToolPanel = std::move(worldSequenceToolPanel);
	return S_OK;
}

void Client::CMapTool::Toggle()
{
	SetOpen(!m_bOpen);
}

void Client::CMapTool::SetOpen(const bool_t isOpen)
{
	if (m_bOpen && !isOpen)
	{
		Restore_DestructionPreview();
		Refresh_DestructionHighlight();
		m_bDestructionTimelinePlaying = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
		if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
		{
			m_pWorldSequenceToolPanel->Stop_AndRestore(
				m_iAuthoringLevelIndex, m_Catalog, m_Placements,
				m_DeployRuntime);
			if (m_pWorldSequenceToolPanel->Is_PreviewActive())
			{
				m_Status = m_pWorldSequenceToolPanel->Get_Status();
				return;
			}
		}
	}
	m_bOpen = isOpen;
	if (!m_bOpen)
		m_bDestructionSimulationClearRequested = true;
}

void Client::CMapTool::Update(
	f32_t fTimeDelta,
	const bool_t bAllowWorldInput)
{
	const uint32_t currentLevelIndex =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isMapAuthoringLevel =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelIndex &&
		CMapEditorWorkspaceService::Is_Active();
	Handle_LevelTransition(currentLevelIndex, isMapAuthoringLevel);
	Update_EditorAreaPreload();
	/* The editor is where this content is checked, so the same idle motion
	   the arena level plays runs here as well. Rebinding is keyed on the
	   area id, which is what changes when the workspace switches maps. */
	if (isMapAuthoringLevel)
	{
		const EDITOR_AREA_DESCRIPTOR* motionArea = Get_ActiveEditorArea();
		const std::string motionAreaId = nullptr != motionArea ?
			motionArea->areaId : m_Catalog.Get_AreaId();
		if (motionAreaId != m_strSelfMotionAreaId)
		{
			m_strSelfMotionAreaId = motionAreaId;
			m_fSelfMotionElapsedSeconds = 0.f;
			m_SelfMotionModels.clear();
			(void)CMapPlacementRuntime::Read_SelfMotions(
				motionAreaId, m_Placements, m_SelfMotions);
		}
		if (!m_SelfMotions.empty() && std::isfinite(fTimeDelta))
		{
			m_fSelfMotionElapsedSeconds += fTimeDelta;
			if (m_fSelfMotionElapsedSeconds >
				CMapPlacementRuntime::SELF_MOTION_WRAP_SECONDS)
			{
				m_fSelfMotionElapsedSeconds -=
					CMapPlacementRuntime::SELF_MOTION_WRAP_SECONDS;
			}
			CMapPlacementRuntime::Sample_SelfMotions(
				m_SelfMotions, m_fSelfMotionElapsedSeconds,
				m_iAuthoringLevelIndex, m_Catalog, m_SelfMotionModels,
				m_Placements);
		}
	}
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Update(
			fTimeDelta,
			isMapAuthoringLevel && m_bOpen &&
				TOOL_MODE::WORLD_SEQUENCE == m_eToolMode,
			m_iAuthoringLevelIndex,
			m_Catalog,
			m_Placements,
			m_DeployRuntime);
	}
	if (isMapAuthoringLevel && nullptr != m_pMapLightPresentation &&
		!m_pMapLightPresentation->Submit_Frame() &&
		!m_bMapLightSubmissionFailureReported)
	{
		m_bMapLightSubmissionFailureReported = true;
		OutputDebugStringA(("[MapTool][MapLight] " +
			m_pMapLightPresentation->Get_Status() + "\n").c_str());
	}
	Update_CutsceneArenaRise(fTimeDelta, isMapAuthoringLevel);
	if (isMapAuthoringLevel)
		Update_MarioIntro();
	Update_DestructionSimulation(fTimeDelta, isMapAuthoringLevel);
	Update_WorldInteraction(
		bAllowWorldInput && isMapAuthoringLevel &&
		!m_EditorAreaPreload.Is_Active());

	if (m_bOpen && TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode &&
		m_bDestructionTimelinePlaying)
	{
		const ENCOUNTER_PATTERN_REFERENCE* pattern =
			m_SelectedDestructionPatternId.empty() ? nullptr :
			m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
		if (nullptr == pattern || 0u == pattern->iTotalDurationMs)
		{
			m_bDestructionTimelinePlaying = false;
		}
		else
		{
			m_fDestructionTimelineMs += (std::max)(0.f, fTimeDelta) * 1000.f;
			const f32_t duration = static_cast<f32_t>(pattern->iTotalDurationMs);
			if (m_fDestructionTimelineMs >= duration)
			{
				if (m_bDestructionTimelineLoop)
					m_fDestructionTimelineMs = std::fmod(
						m_fDestructionTimelineMs, duration);
				else
				{
					m_fDestructionTimelineMs = duration;
					m_bDestructionTimelinePlaying = false;
				}
			}
		}
	}
}

void Client::CMapTool::Update_WorldInteraction(bool_t isAssetTest)
{
	const bool_t mouseDown = 0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;
	Update_WorldTriggerBoxPresentation(
		m_bOpen && isAssetTest &&
		TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode);

	if (!m_bOpen || !isAssetTest)
	{
		m_bNavigationStrokeActive = false;
		return;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bWorldNpcContinuousPlacement = false;
		m_bWorldTriggerTargetPickArmed = false;
		m_bWorldNpcWaypointPickArmed = false;
		m_bWorldNpcBatchCenterPickArmed = false;
		m_bSpawnAnchorPlacementArmed = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
		m_bAnimatedPropPlacementArmed = false;
		m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
		m_Status = "Placement cancelled";
		if (TOOL_MODE::NAVIGATION == m_eToolMode)
			m_NavigationBakeStatus = "Nav Bounds placement cancelled";
	}

	const bool_t canUseWorldMouse =
		GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantCaptureMouse;

	if (TOOL_MODE::NAVIGATION == m_eToolMode)
	{
		if (NAVIGATION_MODE::BAKE == m_eNavigationMode)
		{
			m_bNavigationStrokeActive = false;
			if (canUseWorldMouse &&
				mousePressed &&
				NAV_BOUNDS_STATE::PLACING ==
					m_eNavigationBoundsState)
			{
				Try_PlaceNavigationBounds();
			}
			return;
		}

		if (!mouseDown || !canUseWorldMouse)
		{
			m_bNavigationStrokeActive = false;
			return;
		}

		if (mousePressed)
			m_bNavigationStrokeActive = true;

		if (m_bNavigationStrokeActive)
			Try_PaintNavigation();
		return;
	}

	if (!canUseWorldMouse)
		return;
	if (TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode)
	{
		uint64_t pickedPlacementId = 0u;
		std::string pickFailure;
		if (m_bDestructionPickArmed && mousePressed)
		{
			if (Try_PickDeployProp(pickedPlacementId, pickFailure))
				Select_DestructionWall(pickedPlacementId, "viewport");
			else
				m_DestructionStatus = std::move(pickFailure);
		}
		return;
	}
	if (TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode)
	{
		if (m_bWorldNpcWaypointPickArmed && mousePressed)
			Try_PickWorldNpcWaypoint();
		else if (m_bWorldNpcBatchCenterPickArmed && mousePressed)
			Try_PickWorldNpcBatchCenter();
		else if (m_bWorldTriggerTargetPickArmed && mousePressed)
			Try_PickWorldTriggerTarget();
		else if (m_bSpawnAnchorPlacementArmed && mousePressed)
			Try_PlaceSpawnAnchor();
		else if (m_bWorldGameplayPlacementArmed && mousePressed)
			Try_PlaceWorldGameplay();
		return;
	}
	if (TOOL_MODE::WORLD_SEQUENCE == m_eToolMode)
	{
		/* Sequence targets are selected from the stable Map Objects list.
		   Only an explicitly armed Animated Prop placement consumes a
		   viewport click; ordinary map placement never falls through. */
		if (m_bAnimatedPropPlacementArmed && mousePressed)
			(void)Try_PlaceSelectedDeploy();
		return;
	}

	if (PLACEMENT_STATE::ARMED == m_ePlacementState && mousePressed)
		Try_PlaceSelected();
}

void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	const uint32_t currentLevelIndex =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isMapAuthoringLevel =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelIndex &&
		CMapEditorWorkspaceService::Is_Active();
	Render_WorldOverlay(isMapAuthoringLevel);

	ImGui::SetNextWindowSize(ImVec2(1180.f, 900.f), ImGuiCond_FirstUseEver);
	bool_t isOpen = m_bOpen;
	if (ImGui::Begin("LostArk Map Tool", &isOpen))
	{
#ifdef _DEBUG
		if (ImGui::Button("Complete Play (Server/Arena)##MapTool"))
		{
			if (CMainApp* const app = CMainApp::Get_Active())
				(void)app->Debug_CompletePlaySelected(m_CompletePlayStatus);
			else
				m_CompletePlayStatus = "Complete Play workspace is unavailable.";
		}
		ImGui::SameLine();
		ImGui::TextDisabled("%s", m_CompletePlayStatus.c_str());
#endif
		Render_WorkspaceBar(isMapAuthoringLevel);
		ImGui::Separator();
		Render_ModeBar();
		ImGui::Separator();
		ImGui::BeginDisabled(m_EditorAreaPreload.Is_Active());
		Render_ActiveMode(isMapAuthoringLevel);
		ImGui::EndDisabled();
	}

	ImGui::End();

	if (isOpen != m_bOpen)
		SetOpen(isOpen);
	Render_DestructionSimulationWindow(isMapAuthoringLevel);
}

void Client::CMapTool::Render_WorldOverlay(bool_t isAssetTest)
{
	if (!isAssetTest)
		return;
	if (TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode)
	{
		Render_WorldNpcRouteOverlay();
		Render_WorldNpcBatchOverlay();
		return;
	}
	if (TOOL_MODE::NAVIGATION != m_eToolMode)
		return;

	if (NAVIGATION_MODE::BAKE == m_eNavigationMode)
	{
		Render_NavigationBoundsOverlay();
		/* Bake mode used to draw the bounds alone, which is exactly the mode
		   a staged bake has to be visible in: the numbers say how many cells
		   were found, and this says where they landed. */
		if (m_bNavigationBakePreviewReady)
			Render_NavigationOverlay();
	}
	else
	{
		Render_NavigationOverlay();
	}
}

void Client::CMapTool::Render_WorkspaceBar(const bool_t isAssetTest)
{
	ImGui::TextUnformatted("Map Editor Workspace");
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Data authoring only; Client/Server runtime publish is separate");
	if (!isAssetTest)
	{
		ImGui::TextWrapped(
			"Waiting for the isolated Development editor shell.");
		return;
	}

	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const char_t* preview = nullptr != active ?
		active->label.c_str() : "<select Area>";
	const bool_t isAreaAdmissionBusy = m_EditorAreaPreload.Is_Active();
	ImGui::SetNextItemWidth(320.f);
	ImGui::BeginDisabled(isAreaAdmissionBusy);
	if (ImGui::BeginCombo("Area", preview))
	{
		for (size_t index = 0; index < m_EditorAreas.size(); ++index)
		{
			const bool_t selected = index == m_iActiveEditorArea;
			const std::string label = m_EditorAreas[index].label + "  [" +
				m_EditorAreas[index].areaId + "]";
			if (ImGui::Selectable(label.c_str(), selected))
			{
				if (selected)
				{
					if (!Focus_ActiveEditorAreaCamera())
						m_Status = m_CameraStatus;
				}
				else if (Has_UnsavedAuthoring())
				{
					m_iPendingEditorArea = index;
					m_isEditorAreaSwitchPending = true;
				}
				else
				{
					Begin_EditorAreaSwitch(index);
				}
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == active);
	if (ImGui::Button("Focus Area") && !Focus_ActiveEditorAreaCamera())
		m_Status = m_CameraStatus;
	ImGui::EndDisabled();

	/* Sublevel jumps. The shortcut poll lives here so the keys exist only
	   while this bar is on screen, which is only inside the isolated
	   Development editor shell. */
	Update_EditorSublevelJumpShortcuts();
	if (!m_EditorSublevelJumps.empty())
	{
		ImGui::TextDisabled(
			"Sublevel jump (number row or keypad):");
		for (size_t index = 0u; index < m_EditorSublevelJumps.size(); ++index)
		{
			const EDITOR_SUBLEVEL_JUMP& jump = m_EditorSublevelJumps[index];
			if (index < 9u)
				ImGui::SameLine();
			const std::string caption = index < 9u ?
				std::to_string(index + 1u) + ". " + jump.label :
				jump.label;
			if (ImGui::Button(caption.c_str()))
				(void)Jump_ToEditorSublevel(index);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"%s\n%zu placements\ncenter %.0f, %.0f, %.0f",
					jump.label.c_str(), jump.placementCount,
					jump.center.x, jump.center.y, jump.center.z);
			}
		}
	}
	if ("LV_BER_BERNCASTLE" == m_Catalog.Get_AreaId())
	{
		ImGui::SameLine();
		const bool_t sequenceOwnsPreviewTargets =
			nullptr != m_pWorldSequenceToolPanel &&
			m_pWorldSequenceToolPanel->Is_PreviewActive();
		ImGui::BeginDisabled(sequenceOwnsPreviewTargets);
		if (ImGui::Checkbox(
			"Show Bern Landscape",
			&m_bShowBernLandscape))
		{
			Set_EnvironmentPhase(m_EnvironmentPhase);
			m_Status = m_bShowBernLandscape ?
				"Bern Landscape preview enabled" :
				"Bern Landscape hidden; authoring data preserved";
		}
		ImGui::EndDisabled();
		if (sequenceOwnsPreviewTargets && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"연출 미리보기를 Stop / Restore한 뒤 Landscape 표시를 변경할 수 있습니다.");
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Exit to Lobby"))
	{
		if (Has_UnsavedAuthoring())
			m_isEditorExitPending = true;
		else if (!CLevelTransitionService::Request_Load(
			LEVEL::LOBBY, "debug.map-editor.exit"))
			m_Status = CLevelTransitionService::Get_Status();
	}
	if (isAreaAdmissionBusy)
	{
		const size_t iTotalEntries =
			m_EditorAreaPreload.Catalog.Get_Entries().size();
		const f32_t fProgress = 0 == iTotalEntries ? 0.f :
			static_cast<f32_t>(m_EditorAreaPreload.iNextEntry) /
			static_cast<f32_t>(iTotalEntries);
		ImGui::ProgressBar(fProgress, ImVec2(-1.f, 0.f));
	}
	if (!m_Status.empty())
	{
		ImGui::Separator();
		ImGui::TextWrapped("Workspace status: %s", m_Status.c_str());
	}

	if (m_isEditorAreaSwitchPending || m_isEditorExitPending)
		ImGui::OpenPopup("Unsaved map authoring");
	if (ImGui::BeginPopupModal(
		"Unsaved map authoring",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped(
			"The active Area has unsaved Data authoring changes.");
		if (ImGui::Button("Save and Continue"))
		{
			if (Save_AllAuthoring())
			{
				if (m_isEditorExitPending)
					CLevelTransitionService::Request_Load(
						LEVEL::LOBBY, "debug.map-editor.exit");
				else
					Begin_EditorAreaSwitch(m_iPendingEditorArea);
				m_isEditorAreaSwitchPending = false;
				m_isEditorExitPending = false;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Discard and Continue"))
		{
			if (m_isEditorExitPending)
				CLevelTransitionService::Request_Load(
					LEVEL::LOBBY, "debug.map-editor.exit");
			else
				Begin_EditorAreaSwitch(m_iPendingEditorArea);
			m_isEditorAreaSwitchPending = false;
			m_isEditorExitPending = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			m_iPendingEditorArea = SIZE_MAX;
			m_isEditorAreaSwitchPending = false;
			m_isEditorExitPending = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

bool_t Client::CMapTool::Save_AllAuthoring()
{
	if (nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready())
	{
		std::string sequenceStatus;
		if (!m_pWorldSequenceToolPanel->Validate(
			m_Catalog, m_Placements, m_DeployRuntime, sequenceStatus))
		{
			m_Status = sequenceStatus;
			return false;
		}
	}
	if (!m_WorldNpcBatchDraft.empty())
	{
		m_WorldGameplayStatus =
			"Confirm or Discard the staged NPC batch before saving";
		m_Status = m_WorldGameplayStatus;
		return false;
	}
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before saving";
		m_Status = m_DestructionSimulationStatus;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		(m_bWorldGameplayDirty || m_NavigationDocument.Is_Dirty() ||
			m_RuntimeBlockerDocument.Is_Dirty() ||
			m_DestructionDocument.Is_Dirty()))
	{
		std::string status;
		if (!Validate_CurrentDestructionReferences(status))
		{
			m_Status = status;
			m_DestructionStatus = status;
			return false;
		}
	}
	/* World sequence animation tracks reference Deploy placements by their
	   stable runtime ID, so the Deploy document is replaced first and the
	   linked map/sequence transaction only runs once it is on disk. */
	if (m_bDeployDirty && !Save_DeployPlacements())
		return false;
	bool_t savedLinkedMapAndSequences = false;
	if (nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready())
	{
		if (m_bDirty || m_pWorldSequenceToolPanel->Is_Dirty())
		{
			if (!Save_PlacementsAndWorldSequences())
				return false;
			savedLinkedMapAndSequences = true;
		}
	}
	if (!savedLinkedMapAndSequences && m_bDirty && !Save_Placements())
		return false;
	if (m_bSpawnGroupsDirty && !Save_SpawnGroups())
	{
		m_Status = m_WorldGameplayStatus;
		return false;
	}
	if ((m_bWorldGameplayDirty || m_bWorldNpcBehaviorDraftDirty) &&
		!Save_WorldGameplay())
	{
		m_Status = m_WorldGameplayStatus;
		return false;
	}
	if ((m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty()) && !Save_Navigation())
	{
		m_Status = m_NavigationStatus;
		return false;
	}
	if ((m_DestructionDocument.Is_Dirty() ||
		m_DestructionSimulationDocument.Is_Dirty()) &&
		!Save_DestructionAuthoringPair())
	{
		m_Status = m_DestructionStatus;
		return false;
	}
	if (!savedLinkedMapAndSequences &&
		nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Dirty())
	{
		std::string sequenceStatus;
		if (!m_pWorldSequenceToolPanel->Save(
			m_Catalog, m_Placements, m_DeployRuntime, sequenceStatus))
		{
			m_Status = sequenceStatus;
			return false;
		}
	}
	m_Status = "Saved all changed MapTool authoring documents";
	return true;
}

void Client::CMapTool::Render_ActiveMode(bool_t isAssetTest)
{
	switch (m_eToolMode)
	{
	case TOOL_MODE::MAP_ASSETS:
		Render_MapAssetsPanel(isAssetTest);
		break;

	case TOOL_MODE::WORLD_GAMEPLAY:
		Render_WorldGameplayPanel(isAssetTest);
		break;

	case TOOL_MODE::WORLD_DESTRUCTION:
		ImGui::BeginDisabled(!isAssetTest);
		Render_WorldDestructionPanel(isAssetTest);
		ImGui::EndDisabled();
		break;

	case TOOL_MODE::WORLD_SEQUENCE:
		ImGui::BeginDisabled(!isAssetTest);
		Render_WorldSequencePanel(isAssetTest);
		ImGui::EndDisabled();
		break;

	case TOOL_MODE::NAVIGATION:
		ImGui::BeginDisabled(!isAssetTest);
		Render_NavigationPanel();
		ImGui::EndDisabled();
		break;

	case TOOL_MODE::CAMERA:
		ImGui::BeginDisabled(!isAssetTest);
		Render_CameraPanel();
		ImGui::EndDisabled();
		break;
	}
}

void Client::CMapTool::Render_MapAssetsPanel(bool_t isAssetTest)
{
	ImGui::Text("Level: %u", m_iAuthoringLevelIndex);
	ImGui::SameLine();
	ImGui::Text("| Area: %s",
		isAssetTest && m_Catalog.Is_Ready() ?
			m_Catalog.Get_AreaId().c_str() : "NO MAP AREA");
	ImGui::SameLine();
	ImGui::Text("| Catalog: %s",
		m_Catalog.Is_Ready() ? "READY" : "NOT READY");
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Separator();

	ImGui::BeginDisabled(!isAssetTest || !m_Catalog.Is_Ready());
	Render_Toolbar();

	const f32_t availableHeight = ImGui::GetContentRegionAvail().y;
	const f32_t topPanelHeight = (std::max)(
		280.f, (std::min)(480.f, availableHeight * 0.48f));

	if (ImGui::BeginTable("MapEditorColumns", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Asset Palette", ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthStretch, 0.27f);
		ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch, 0.35f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_Palette(topPanelHeight);
		ImGui::TableSetColumnIndex(1);
		Render_Hierarchy(topPanelHeight);
		ImGui::TableSetColumnIndex(2);
		Render_Inspector();
		ImGui::EndTable();
	}
	Render_AssetPreview();
	ImGui::EndDisabled();
}

void Client::CMapTool::Render_WorldSequencePanel(const bool_t isAssetTest)
{
	if (nullptr == m_pWorldSequenceToolPanel)
	{
		ImGui::TextDisabled("World Sequence tool is unavailable.");
		return;
	}
	if (isAssetTest)
	{
		Render_AnimatedPropsAuthoring();
		ImGui::Separator();
		Render_CutsceneArenaPreview();
		ImGui::Separator();
	}
	m_pWorldSequenceToolPanel->Render(
		isAssetTest,
		m_iAuthoringLevelIndex,
		m_Catalog,
		m_Placements,
		m_iSelectedPlacementId,
		m_DeployRuntime);
	if (m_pWorldSequenceToolPanel->Consume_ReloadAllRequest())
	{
		const bool_t reloaded = Load_Placements();
		if (!reloaded && nullptr != m_pWorldSequenceToolPanel)
		{
			m_pWorldSequenceToolPanel->Report_ReloadAllResult(false, m_Status);
		}
		return;
	}
	if (m_pWorldSequenceToolPanel->Consume_SaveAllRequest())
	{
		const bool_t saved = Save_AllAuthoring();
		m_pWorldSequenceToolPanel->Report_SaveAllResult(saved, m_Status);
	}
}

bool_t Client::CMapTool::Sample_ShotCameraTrack(
	const EDITOR_CAMERA_SHOT& shot,
	const f32_t elapsedMs,
	VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	/* Built here rather than stored so every preview samples what the
	   editor currently shows, and so the product runtime keeps the single
	   cinematic sampler as the only implementation of this motion. */
	VALTAN_CINEMATIC_CAMERA_CUE cue{};
	cue.strCueId = shot.shotId;
	cue.iDurationMs = static_cast<uint32_t>((std::max)(1, shot.trackDurationMs));
	cue.eInterpolation = 0 == shot.interpolationIndex ?
		VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR :
		VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
	cue.eEasing = 0 == shot.easingIndex ?
		VALTAN_CINEMATIC_CAMERA_EASING::LINEAR :
		(1 == shot.easingIndex ?
			VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP :
			VALTAN_CINEMATIC_CAMERA_EASING::HOLD);
	cue.eTrackingMode = VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
	cue.Keyframes.reserve(shot.keyframes.size());
	for (const EDITOR_CAMERA_KEYFRAME& source : shot.keyframes)
	{
		VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe{};
		keyframe.strSceneId = source.sceneId;
		keyframe.iTimeMs = static_cast<uint32_t>((std::max)(0, source.timeMs));
		keyframe.vEye = source.eye;
		keyframe.vLookAt = source.lookAt;
		keyframe.fFovYDegrees = source.fovYDegrees;
		keyframe.vUp = source.up; keyframe.hasUp = source.hasUp;
		cue.Keyframes.push_back(std::move(keyframe));
	}
	outPose.vEye = shot.eye;
	outPose.vLookAt = shot.lookAt;
	outPose.fFovYDegrees = shot.fovYDegrees;
	if (cue.Keyframes.empty())
		return true;
	return CValtanCinematicCameraController::Sample_Cue(
		cue, elapsedMs / 1000.f, outPose);
}

int Client::CMapTool::Debug_SequenceViewer(const std::string& areaId,
	const std::string& sequenceId, const std::string& triggerId,
	const bool_t play, const bool_t stop, const float3_t* focus, std::string& status)
{
	if (m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) || !CMapEditorWorkspaceService::Is_Active())
	{ status = "Lobby > Test에서 편집 미리보기를 사용할 수 있습니다."; return -1; }
	if (m_EditorAreaPreload.Is_Active())
	{ status = m_Status; return 0; }
	const auto* area = Get_ActiveEditorArea();
	if (!area || area->areaId != areaId)
	{
		if (Has_UnsavedAuthoring())
		{ status = "Map Tool의 수정 사항을 저장한 뒤 다시 실행해 주세요."; return -1; }
		const auto found = std::find_if(m_EditorAreas.begin(), m_EditorAreas.end(),
			[&](const EDITOR_AREA_DESCRIPTOR& value) { return value.areaId == areaId; });
		if (found == m_EditorAreas.end() || !Begin_EditorAreaSwitch(static_cast<size_t>(found - m_EditorAreas.begin())))
		{ status = "Area 준비 실패: " + m_Status; return -1; }
		status = "Area를 준비하고 있습니다."; return 0;
	}
	if (!triggerId.empty())
	{
		if (!m_WorldGameplayDocument.Find(triggerId))
		{ status = "현재 편집 문서에 트리거가 없습니다: " + triggerId; return -1; }
		m_SelectedWorldPlacementId = triggerId;
		m_eToolMode = TOOL_MODE::WORLD_GAMEPLAY;
	}
	else
	{
		if (!m_pWorldSequenceToolPanel || !m_pWorldSequenceToolPanel->Select_Instance(sequenceId))
		{ status = "현재 편집 문서에 시퀀스가 없습니다: " + sequenceId; return -1; }
		m_eToolMode = TOOL_MODE::WORLD_SEQUENCE;
	}
	if (focus)
	{
		const auto camera = m_pAssetTestCamera.lock();
		if (!camera) { status = "편집 카메라가 준비되지 않았습니다."; return -1; }
		if (m_bCutsceneOriginalRunning || m_bMarioIntroRunning || 0.f <= m_fCutsceneScrubMs)
		{ status = "컷신 카메라가 재생/편집 중입니다. 해당 연출을 Stop하고 Go To를 다시 눌러 주세요."; return -1; }
		End_CutsceneCameraTrack();
		camera->Frame_Area(*focus, 8.f);
		status = "선택한 트리거 위치로 편집 카메라를 이동했습니다."; return 1;
	}
	if (!play && !stop) { status = "Map Tool에서 선택했습니다. 변경은 Save 후 Publish가 필요합니다."; return 1; }
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog; targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	targets.device = m_pDevice; targets.context = m_pContext;
	if (stop)
	{
		if (sequenceId == "world.sequence.instance.original_kouku")
		{
			for (const auto& instance : m_ArenaRisePlayer.Get_Document().Get_Instances())
				if (instance.instanceId.starts_with(KAKUL_ORIGINAL_INSTANCE_PREFIX))
					m_ArenaRisePlayer.Stop_Instance(instance.instanceId, targets, true);
			m_bCutsceneOriginalRunning = false;
			Hide_CutsceneSet(); Apply_CutsceneArenaVisibility(false); End_CutsceneCameraTrack();
		}
		m_ArenaRisePlayer.Stop_Instance(sequenceId, targets, true);
		Stop_MarioIntro();
		m_fCutsceneScrubMs = m_fCutsceneLoopStartMs = m_fCutsceneLoopEndMs = -1.f;
		m_ArenaRisePlayer.Set_Paused(false);
		End_CutsceneCameraTrack();
		status = "선택 시퀀스의 편집 미리보기를 정지했습니다."; return 1;
	}
	if (sequenceId.empty())
	{ status = "이 트리거는 서버 동작입니다. 아레나에서 Play를 사용해 주세요."; return -1; }
	if (!m_pWorldSequenceToolPanel || !m_ArenaRisePlayer.Set_Document(
		m_pWorldSequenceToolPanel->Get_Document(), targets, status)) return -1;
	m_bArenaRiseAreaLoaded = true;
	m_fCutsceneScrubMs = m_fCutsceneLoopStartMs = m_fCutsceneLoopEndMs = -1.f;
	m_ArenaRisePlayer.Set_Paused(false);
	if (sequenceId == "world.sequence.instance.original_kouku")
	{
		const bool_t started = Play_CutsceneOriginalRise();
		status = m_Status; return started ? 1 : -1;
	}
	if (!m_ArenaRisePlayer.Play(sequenceId, targets))
	{ status = m_ArenaRisePlayer.Get_Status(); return -1; }
	const auto camera = std::find_if(m_CameraShots.begin(), m_CameraShots.end(),
		[&](const EDITOR_CAMERA_SHOT& shot) { return shot.sequenceInstanceId == sequenceId; });
	Stop_MarioIntro();
	if (camera != m_CameraShots.end())
	{
		m_iSelectedCameraShot = static_cast<size_t>(camera - m_CameraShots.begin());
		m_MarioIntroInstanceId = sequenceId; m_bMarioIntroRunning = true;
	}
	status = "편집 시퀀스 미리보기 시작 (서버 상태는 바뀌지 않습니다).";
	return 1;
}

std::filesystem::path Client::CMapTool::Get_WorldSequencePath() const
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	return nullptr == active ? std::filesystem::path{} :
		active->sourcePlacements.parent_path() /
		std::filesystem::path(active->areaId + ".worldsequences.json");
}
bool Client::CMapTool::IsOpen() const
{
	return m_bOpen;
}

bool_t Client::CMapTool::ConsumesWorldLeftMouse() const
{
	if (!m_bOpen ||
		m_iAuthoringLevelIndex !=
		CGameInstance::Get().Get_CurrentLevelID())
	{
		return false;
	}

	return TOOL_MODE::NAVIGATION == m_eToolMode ||
		(TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode &&
			m_bDestructionPickArmed) ||
		(TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode &&
			(m_bWorldGameplayPlacementArmed ||
				m_bWorldTriggerTargetPickArmed ||
				m_bWorldNpcWaypointPickArmed ||
				m_bWorldNpcBatchCenterPickArmed ||
				m_bSpawnAnchorPlacementArmed)) ||
		(TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode &&
			m_bDestructionPickArmed) ||
		(TOOL_MODE::WORLD_SEQUENCE == m_eToolMode &&
			m_bAnimatedPropPlacementArmed) ||
		PLACEMENT_STATE::ARMED == m_ePlacementState;
}

bool_t Client::CMapTool::Has_UnsavedAuthoring() const
{
	return m_bDirty || m_bDeployDirty || m_bWorldGameplayDirty ||
		m_bSpawnGroupsDirty ||
		m_bWorldNpcBehaviorDraftDirty || !m_WorldNpcBatchDraft.empty() ||
		m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty() ||
		m_DestructionDocument.Is_Dirty() ||
		m_DestructionSimulationDocument.Is_Dirty() ||
		m_bDestructionSimulationElementDraftDirty ||
		(nullptr != m_pWorldSequenceToolPanel &&
			m_pWorldSequenceToolPanel->Is_Dirty());
}

void Client::CMapTool::Render_ModeBar()
{
	const TOOL_MODE previousMode = m_eToolMode;
	if (ImGui::RadioButton(
		"Map Assets",
		TOOL_MODE::MAP_ASSETS == m_eToolMode))
	{
		Restore_DestructionPreview();
		m_eToolMode = TOOL_MODE::MAP_ASSETS;
		m_bWorldGameplayPlacementArmed = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"World Gameplay",
		TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode))
	{
		Restore_DestructionPreview();
		m_eToolMode = TOOL_MODE::WORLD_GAMEPLAY;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bNavigationStrokeActive = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"World Destruction",
		TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::WORLD_DESTRUCTION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bWorldTriggerTargetPickArmed = false;
		m_bSpawnAnchorPlacementArmed = false;
		m_bNavigationStrokeActive = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"World Sequence",
		TOOL_MODE::WORLD_SEQUENCE == m_eToolMode))
	{
		Restore_DestructionPreview();
		m_eToolMode = TOOL_MODE::WORLD_SEQUENCE;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bWorldTriggerTargetPickArmed = false;
		m_bSpawnAnchorPlacementArmed = false;
		m_bNavigationStrokeActive = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Navigation",
		TOOL_MODE::NAVIGATION == m_eToolMode))
	{
		Restore_DestructionPreview();
		m_eToolMode = TOOL_MODE::NAVIGATION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Camera",
		TOOL_MODE::CAMERA == m_eToolMode))
	{
		Restore_DestructionPreview();
		m_eToolMode = TOOL_MODE::CAMERA;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bNavigationStrokeActive = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
	}
	if (TOOL_MODE::WORLD_DESTRUCTION == previousMode &&
		TOOL_MODE::WORLD_DESTRUCTION != m_eToolMode &&
		nullptr != m_pDestructionSimulationController)
	{
		m_bDestructionSimulationClearRequested = true;
	}
	if (TOOL_MODE::WORLD_SEQUENCE == previousMode &&
		TOOL_MODE::WORLD_SEQUENCE != m_eToolMode)
	{
		m_bAnimatedPropPlacementArmed = false;
	}
	if (TOOL_MODE::WORLD_SEQUENCE == previousMode &&
		TOOL_MODE::WORLD_SEQUENCE != m_eToolMode &&
		nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, m_Placements,
			m_DeployRuntime);
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			m_eToolMode = previousMode;
			m_Status = m_pWorldSequenceToolPanel->Get_Status();
		}
	}
}

void Client::CMapTool::Render_Toolbar()
{
	if (ImGui::Button("Save"))
		Save_AllAuthoring();
	ImGui::TextDisabled("Data/Maps authoring; publish required");
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		if (m_bDeployDirty)
		{
			m_Status =
				"Save or discard the animated prop changes before reloading";
		}
		else if (Load_Placements())
		{
			Load_DeployProps();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		ImGui::OpenPopup("Clear all placements?");
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == Get_SelectedAsset() ||
		PLACEMENT_STATE::ARMED == m_ePlacementState);
	if (ImGui::Button("Arm placement"))
		Arm_SelectedAsset();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Text("Objects: %zu%s", m_Placements.size(),
		m_bDirty ? "  *unsaved" : "");
	ImGui::TextDisabled(
		"DeployProp authoring is excluded until its source/stage contract is complete.");
	ImGui::TextUnformatted("Sky phase:");
	ImGui::SameLine();
	if (ImGui::RadioButton("Baseline##EnvironmentPhase",
		m_EnvironmentPhase == ENVIRONMENT_PHASE::BASELINE))
		Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
	ImGui::SameLine();
	if (ImGui::RadioButton("SpaceHole##EnvironmentPhase",
		m_EnvironmentPhase == ENVIRONMENT_PHASE::SPACEHOLE))
		Set_EnvironmentPhase(ENVIRONMENT_PHASE::SPACEHOLE);
	ImGui::SameLine();
	if (ImGui::RadioButton("ChaosGate##EnvironmentPhase",
		m_EnvironmentPhase == ENVIRONMENT_PHASE::CHAOS_GATE))
		Set_EnvironmentPhase(ENVIRONMENT_PHASE::CHAOS_GATE);

	if (ImGui::BeginPopupModal("Clear all placements?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Remove every placed map object from this level?");
		if (ImGui::Button("Clear all"))
		{
			Remove_AllPlacements();
			m_Status = "Cleared all placements (not saved yet)";
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	if (PLACEMENT_STATE::ARMED == m_ePlacementState)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PLACEMENT ARMED: click the world (Esc cancels)");
	}
	ImGui::Separator();
}

void Client::CMapTool::Render_Palette(f32_t childHeight)
{
	ImGui::TextUnformatted("Palette");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##AssetFilter",
		"search name, id, group, evidence...", m_Filter, sizeof(m_Filter));

	const f32_t listHeight = (std::max)(120.f, childHeight -
		ImGui::GetTextLineHeightWithSpacing() * 2.f);
	ImGui::BeginChild("AssetPaletteList", ImVec2(0.f, listHeight), true);

	const auto renderAssetRow = [&](const MAP_ASSET_ENTRY& asset)
	{
		ImGui::PushID(asset.id.c_str());
		const bool_t favorite = m_FavoriteAssetIds.contains(asset.id);
		if (ImGui::SmallButton(favorite ? "*" : "+"))
		{
			if (favorite)
				m_FavoriteAssetIds.erase(asset.id);
			else
				m_FavoriteAssetIds.insert(asset.id);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(favorite ? "Remove from candidate board" :
				"Add to candidate board");
		ImGui::SameLine();
		const bool_t selected = asset.id == m_SelectedAssetId;
		if (ImGui::Selectable(asset.label.c_str(), selected))
			Select_Asset(asset);
		if (ImGui::IsItemHovered() &&
			ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (!selected)
				Select_Asset(asset);
			Arm_SelectedAsset();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(asset.id.c_str());
			ImGui::TextWrapped("%s", asset.modelRelativePath.string().c_str());
			ImGui::TextWrapped("Evidence: %s", asset.evidence.c_str());
			ImGui::TextDisabled(
				"Single click: preview | Double click: arm placement");
			ImGui::EndTooltip();
		}
		ImGui::PopID();
	};

	const bool_t hasFilter = '\0' != m_Filter[0];
	if (!m_FavoriteAssetIds.empty())
	{
		size_t visibleFavorites = 0;
		for (const MAP_ASSET_ENTRY& asset : m_Catalog.Get_Entries())
		{
			if (m_FavoriteAssetIds.contains(asset.id) &&
				MatchesFilter(asset.label + " " + asset.id + " " +
					asset.groupLabel + " " + asset.evidence, m_Filter))
				++visibleFavorites;
		}
		if (0 != visibleFavorites)
		{
			if (hasFilter)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			const std::string header = "Candidate Board (" +
				std::to_string(visibleFavorites) + ")###favorite-assets";
			if (ImGui::CollapsingHeader(header.c_str(),
				ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (const MAP_ASSET_ENTRY& asset : m_Catalog.Get_Entries())
				if (m_FavoriteAssetIds.contains(asset.id) &&
					MatchesFilter(asset.label + " " + asset.id + " " +
						asset.groupLabel + " " + asset.evidence, m_Filter))
					renderAssetRow(asset);
			}
		}
	}

	vector<std::string> groupOrder;
	std::unordered_map<std::string, vector<const MAP_ASSET_ENTRY*>> groups;
	for (const MAP_ASSET_ENTRY& asset : m_Catalog.Get_Entries())
	{
		if (!MatchesFilter(asset.label + " " + asset.id + " " +
			asset.groupLabel + " " + asset.evidence, m_Filter))
			continue;
		if (!groups.contains(asset.groupId))
			groupOrder.push_back(asset.groupId);
		groups[asset.groupId].push_back(&asset);
	}

	for (const std::string& groupId : groupOrder)
	{
		const vector<const MAP_ASSET_ENTRY*>& assets = groups[groupId];
		if (assets.empty())
			continue;
		if (hasFilter)
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		const std::string header = assets.front()->groupLabel + " (" +
			std::to_string(assets.size()) + ")###group-" + groupId;
		const ImGuiTreeNodeFlags flags =
			"valtan-confirmed" == groupId ?
			ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None;
		if (ImGui::CollapsingHeader(header.c_str(), flags))
			for (const MAP_ASSET_ENTRY* pAsset : assets)
				renderAssetRow(*pAsset);
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_Hierarchy(f32_t childHeight)
{
	ImGui::TextUnformatted("Hierarchy");
	const f32_t listHeight = (std::max)(120.f, childHeight -
		ImGui::GetTextLineHeightWithSpacing());
	ImGui::BeginChild("PlacementHierarchy", ImVec2(0.f, listHeight), true);
	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(m_Placements.size()));
	while (clipper.Step())
	{
		for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
		{
			const PLACED_ENTRY& entry = m_Placements[index];
			const MAP_ASSET_ENTRY* pAsset =
				m_Catalog.Find(entry.record.assetId);
			const std::string assetLabel = nullptr == pAsset ?
				entry.record.assetId : pAsset->label;
			const std::string label = "[" + entry.record.sourceLevel + "] " +
				assetLabel + "###placement-" +
				std::to_string(entry.record.placementId);
			ImGui::PushID(reinterpret_cast<void*>(
				static_cast<uintptr_t>(entry.record.placementId)));
			const bool_t selected =
				entry.record.placementId == m_iSelectedPlacementId;
			if (ImGui::Selectable(label.c_str(), selected))
				m_iSelectedPlacementId = entry.record.placementId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s",
					entry.record.sourcePlacementId.c_str());
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_Inspector()
{
	ImGui::TextUnformatted("Inspector");
	PLACED_ENTRY* pEntry = Find_Placement(m_iSelectedPlacementId);
	if (nullptr == pEntry)
	{
		ImGui::TextDisabled("Select a placed object.");
		return;
	}

	ImGui::Text("Placement #%llu",
		static_cast<unsigned long long>(pEntry->record.placementId));
	ImGui::TextWrapped("Source: %s",
		pEntry->record.sourcePlacementId.c_str());
	ImGui::Text("Level: %s | Transform: %s",
		pEntry->record.sourceLevel.c_str(),
		pEntry->record.transformSource.c_str());
	ImGui::TextWrapped("Asset: %s", pEntry->record.assetId.c_str());
	ImGui::Text("Runtime: %s",
		nullptr != pEntry->batch ? "Static Batch" : "Standalone Fallback");
	float3_t position = pEntry->record.position;
	float4_t quaternion = pEntry->record.rotationQuaternion;
	float3_t scale = pEntry->record.signedScale;
	bool_t visible = pEntry->record.visible;
	const bool_t positionChanged =
		ImGui::DragFloat3("Position", &position.x, 0.1f);
	const bool_t rotationChanged =
		ImGui::DragFloat4("Rotation quaternion", &quaternion.x, 0.0025f);
	const bool_t scaleChanged =
		ImGui::DragFloat3("Signed scale", &scale.x, 0.01f, -1000.f, 1000.f);
	if (positionChanged || rotationChanged || scaleChanged)
	{
		const vector_t rawQuaternion = XMLoadFloat4(&quaternion);
		const float quaternionLength =
			XMVectorGetX(XMVector4Length(rawQuaternion));
		const bool_t scaleIsValid =
			std::abs(scale.x) >= 0.000001f &&
			std::abs(scale.y) >= 0.000001f &&
			std::abs(scale.z) >= 0.000001f;
		if (!std::isfinite(quaternionLength) ||
			quaternionLength < 0.000001f || !scaleIsValid)
		{
			m_Status =
				"Transform edit rejected: zero quaternion/scale axis";
		}
		else
		{
			vector_t normalized = XMQuaternionNormalize(rawQuaternion);
			if (XMVectorGetW(normalized) < 0.f)
				normalized = XMVectorNegate(normalized);
			XMStoreFloat4(&quaternion, normalized);

			MAP_PLACEMENT_RECORD staged = pEntry->record;
			staged.position = position;
			staged.rotationQuaternion = quaternion;
			staged.signedScale = scale;
			bool_t applied = false;
			const bool_t isValidPlacement =
				CMapPlacementDocument::Is_Valid(staged, m_Catalog);

			if (!isValidPlacement)
			{
				m_Status = "Transform edit rejected by placement validation";
			}
			else if (nullptr != pEntry->object)
			{
				pEntry->object->Set_PlacementTransform(
					position, quaternion, scale);
				applied = true;
			}
			else if (nullptr != pEntry->batch)
			{
				const bool_t oldMirrored = pEntry->record.signedScale.x *
					pEntry->record.signedScale.y *
					pEntry->record.signedScale.z < 0.f;
				const bool_t newMirrored =
					scale.x * scale.y * scale.z < 0.f;

				if (oldMirrored == newMirrored)
				{
					const MAP_ASSET_ENTRY* asset =
						m_Catalog.Find(staged.assetId);
					shared_ptr<CModel> model;
					if (nullptr != asset)
					{
						model = dynamic_pointer_cast<CModel>(
							CGameInstance::Get().Clone_Prototype(
								m_iAuthoringLevelIndex,
								asset->prototypeTag));
					}

					FMapStaticInstance instance{};
					if (nullptr != asset && nullptr != model &&
						SUCCEEDED(BuildStaticInstance(
							*asset, model, staged, instance)) &&
						SUCCEEDED(pEntry->batch->Update_Instance(
							staged.placementId, instance)))
						applied = true;
				}
				else
				{
					/* Mirror parity�� �ٲ�� ���� batch pass�� �޶����Ƿ�
					   �����ϰ� standalone���� �̵��ϰ� Reload �� ���ġ�Ѵ�. */
					PLACED_ENTRY migrated{};
					if (Create_Placement(staged, migrated))
					{
						if (SUCCEEDED(pEntry->batch->Set_InstanceVisible(
							staged.placementId, false)))
						{
							pEntry->layerTag = std::move(migrated.layerTag);
							pEntry->object = std::move(migrated.object);
							pEntry->batch.reset();
							applied = true;
						}
						else
						{
							CGameInstance::Get().Remove_GameObject_from_Layer(
								m_iAuthoringLevelIndex, migrated.layerTag,
								static_pointer_cast<CGameObject>(migrated.object));
						}
					}
				}
			}

			if (applied)
			{
				pEntry->record = std::move(staged);
				m_bDirty = true;
			}
			else if (isValidPlacement)
			{
				m_Status = "Transform edit failed; previous state preserved";
			}
		}
	}

	if (ImGui::Checkbox("Visible", &visible))
	{
		if (Set_RuntimeVisible(*pEntry, visible))
		{
			pEntry->record.visible = visible;
			m_bDirty = true;
			Set_EnvironmentPhase(m_EnvironmentPhase);
		}
		else
			m_Status = "Visibility edit failed";
	}
	const bool_t mirrored = pEntry->record.signedScale.x *
		pEntry->record.signedScale.y * pEntry->record.signedScale.z < 0.f;
	ImGui::Text("Mirrored pass: %s",
		mirrored ? "YES" : "NO");

	if (ImGui::Button("Delete selected"))
	{
		const uint64_t deletedId = pEntry->record.placementId;
		if (Remove_Placement(deletedId))
			m_Status = "Deleted placement #" +
				std::to_string(deletedId);
	}
}

void Client::CMapTool::Select_Asset(const MAP_ASSET_ENTRY& asset)
{
	m_SelectedAssetId = asset.id;
	m_ePlacementState = PLACEMENT_STATE::IDLE;

	if (nullptr == m_pAssetPreview ||
		FAILED(m_pAssetPreview->Select_Asset(m_iAuthoringLevelIndex, asset)))
	{
		m_Status = nullptr == m_pAssetPreview ?
			"Preview service is not initialized." :
			m_pAssetPreview->Get_Status();
		return;
	}

	m_Status = "Previewing " + asset.label +
		"; use Arm placement or double-click the Palette row to place it.";
}

void Client::CMapTool::Arm_SelectedAsset()
{
	const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
	if (nullptr == pAsset)
	{
		m_Status = "Select an asset before arming placement.";
		return;
	}

	m_ePlacementState = PLACEMENT_STATE::ARMED;
	m_Status = "Placement armed for " + pAsset->label +
		"; click the first rendered triangle surface.";
}

void Client::CMapTool::Render_AssetPreview()
{
	ImGui::SeparatorText("Selected Asset Preview");

	const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
	if (nullptr == pAsset || nullptr == m_pAssetPreview ||
		!m_pAssetPreview->Has_Asset())
	{
		ImGui::BeginChild("AssetPreviewEmpty", ImVec2(0.f, 280.f), true);
		ImGui::TextDisabled(
			"Select an asset from a folder. No world placement is required.");
		if (nullptr != m_pAssetPreview)
			ImGui::TextWrapped("%s", m_pAssetPreview->Get_Status().c_str());
		Render_DecoderReport();
		ImGui::EndChild();
		return;
	}

	const f32_t panelHeight = (std::max)(320.f,
		ImGui::GetContentRegionAvail().y);
	ImGui::BeginChild("AssetPreviewPanel", ImVec2(0.f, panelHeight), true);
	if (ImGui::BeginTable("AssetPreviewColumns", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Preview Image",
			ImGuiTableColumnFlags_WidthStretch, 0.68f);
		ImGui::TableSetupColumn("Preview Information",
			ImGuiTableColumnFlags_WidthStretch, 0.32f);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		const ImVec2 available = ImGui::GetContentRegionAvail();
		const ImVec2 imageSize(
			(std::max)(256.f, available.x),
			(std::max)(256.f, panelHeight - 12.f));
		m_pAssetPreview->Render(
			static_cast<uint32_t>(imageSize.x),
			static_cast<uint32_t>(imageSize.y));

		ID3D11ShaderResourceView* pTexture =
			m_pAssetPreview->Get_TextureView();
		if (nullptr != pTexture)
		{
			const ImTextureID textureId =
				static_cast<ImTextureID>(
					reinterpret_cast<uintptr_t>(pTexture));
			ImGui::Image(ImTextureRef(textureId), imageSize,
				ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));
			if (ImGui::IsItemHovered())
			{
				const ImGuiIO& io = ImGui::GetIO();
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					m_pAssetPreview->Orbit(io.MouseDelta.x, io.MouseDelta.y);
				if (0.f != io.MouseWheel)
					m_pAssetPreview->Zoom(io.MouseWheel);
			}
		}
		else
			ImGui::TextWrapped("Preview texture is unavailable: %s",
				m_pAssetPreview->Get_Status().c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::TextWrapped("%s", m_pAssetPreview->Get_Label().c_str());
		ImGui::Separator();
		ImGui::TextWrapped("Group: %s", pAsset->groupLabel.c_str());
		ImGui::TextWrapped("Evidence: %s", pAsset->evidence.c_str());
		ImGui::TextWrapped("Asset ID: %s",
			m_pAssetPreview->Get_AssetId().c_str());
		ImGui::TextWrapped("Model: %s",
			m_pAssetPreview->Get_ModelPath().c_str());
		const float3_t dimensions = m_pAssetPreview->Get_Dimensions();
		ImGui::Text("Meshes: %u", m_pAssetPreview->Get_MeshCount());
		ImGui::Text("Bounds: %.3f x %.3f x %.3f",
			dimensions.x, dimensions.y, dimensions.z);
		ImGui::TextWrapped("Preview: %s",
			m_pAssetPreview->Get_Status().c_str());

		if (ImGui::Button("Reset view"))
			m_pAssetPreview->Reset_Camera();
		ImGui::SameLine();
		if (ImGui::Button("Arm placement"))
			Arm_SelectedAsset();
		ImGui::TextDisabled("LMB drag: orbit | Mouse wheel: zoom");
		Render_DecoderReport();
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_DecoderReport() const
{
	ImGui::SeparatorText("Last .wmodel decode");
	const MODEL_DECODE_REPORT report = CModelDecoderRegistry::Get().Get_LastReport();
	if (report.meshPath.empty())
	{
		ImGui::TextDisabled("No binary model decode has been requested yet.");
		return;
	}

	ImGui::Text("Status: %s | Decoder: %s", report.succeeded ? "LOADED" : "FAILED",
		report.decoderName.empty() ? "not recognized" : report.decoderName.c_str());
	ImGui::TextWrapped("Source: %s", report.meshPath.string().c_str());
	if (report.succeeded)
	{
		ImGui::Text("Meshes: %u | Materials: %u | Vertices: %llu | Indices: %llu",
			report.meshCount, report.materialCount,
			static_cast<unsigned long long>(report.vertexCount),
			static_cast<unsigned long long>(report.indexCount));
	}
	else
		ImGui::TextWrapped("Reason: %s", report.error.c_str());
}

const MAP_ASSET_ENTRY* Client::CMapTool::Get_SelectedAsset() const
{
	return m_Catalog.Find(m_SelectedAssetId);
}
