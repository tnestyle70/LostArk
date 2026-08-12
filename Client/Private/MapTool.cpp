#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Camera_Free.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "MapAssetObject.h"
#include "MapEditorWorkspaceService.h"
#include "MapNavigationContract.h"
#include "MapStaticBatchObject.h"
#include "MapAssetPreview.h"
#include "DeployPropObject.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "Navigation.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Trigger_Box.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

namespace
{
	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t TryBuildCentralEditorFrame(
		const vector<float3_t>& positions,
		float3_t& outCenter,
		f32_t& outRadius)
	{
		vector<f32_t> xValues;
		vector<f32_t> yValues;
		vector<f32_t> zValues;
		xValues.reserve(positions.size());
		yValues.reserve(positions.size());
		zValues.reserve(positions.size());
		for (const float3_t& position : positions)
		{
			if (!IsFinite(position))
				continue;
			xValues.push_back(position.x);
			yValues.push_back(position.y);
			zValues.push_back(position.z);
		}
		if (xValues.empty())
			return false;

		std::sort(xValues.begin(), xValues.end());
		std::sort(yValues.begin(), yValues.end());
		std::sort(zValues.begin(), zValues.end());
		const auto sampleQuantile = [](const vector<f32_t>& values, f32_t quantile)
		{
			const size_t index = static_cast<size_t>(
				(values.size() - 1) * quantile + 0.5f);
			return values[index];
		};

		const float3_t center(
			sampleQuantile(xValues, 0.50f),
			sampleQuantile(yValues, 0.50f),
			sampleQuantile(zValues, 0.50f));
		const f32_t centralSpanX =
			sampleQuantile(xValues, 0.95f) - sampleQuantile(xValues, 0.05f);
		const f32_t centralSpanZ =
			sampleQuantile(zValues, 0.95f) - sampleQuantile(zValues, 0.05f);
		const f32_t radius = (std::max)(75.f,
			(std::max)(centralSpanX, centralSpanZ) * 0.35f);
		if (!IsFinite(center) || !std::isfinite(radius) || radius <= 0.f)
			return false;

		outCenter = center;
		outRadius = radius;
		return true;
	}

	bool_t TryBuildGameplaySpawnFrame(
		const CWorldGameplayDocument& document,
		const std::string& preferredPlacementId,
		float3_t& outCenter,
		f32_t& outRadius)
	{
		const WORLD_GAMEPLAY_PLACEMENT* spawn =
			document.Find(preferredPlacementId);
		if (nullptr == spawn ||
			WORLD_PLACEMENT_KIND::PLAYER_SPAWN != spawn->eKind ||
			!spawn->isEnabled || !IsFinite(spawn->position))
		{
			spawn = nullptr;
			for (const WORLD_GAMEPLAY_PLACEMENT& placement :
				document.Get_Placements())
			{
				if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled && IsFinite(placement.position))
				{
					spawn = &placement;
					break;
				}
			}
		}
		if (nullptr == spawn)
			return false;

		outCenter = spawn->position;
		outRadius = 35.f;
		return true;
	}

	bool_t IsBernLandscapePlacement(
		const CMapAssetCatalog& catalog,
		const MAP_PLACEMENT_RECORD& record)
	{
		if ("LV_BER_BERNCASTLE" != catalog.Get_AreaId())
			return false;

		const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
		return nullptr != asset && asset->groupId == "landscape";
	}

	bool_t IsBatchEligible(const MAP_ASSET_ENTRY& asset)
	{
		return CMapPlacementRuntime::Is_BatchEligible(asset);
	}

	HRESULT BuildStaticInstance(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		FMapStaticInstance& outInstance)
	{
		return CMapPlacementRuntime::Build_StaticInstance(
			asset, model, record, outInstance);
	}

	bool_t MatchesFilter(const std::string& text, const char* pFilter)
	{
		if (nullptr == pFilter || '\0' == *pFilter)
			return true;

		std::string haystack = text;
		std::string needle = pFilter;
		std::transform(haystack.begin(), haystack.end(), haystack.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		std::transform(needle.begin(), needle.end(), needle.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		return std::string::npos != haystack.find(needle);
	}

	uint64_t HashStableAuthoringId(const std::string& value)
	{
		uint64_t hash = 14695981039346656037ull;
		for (const unsigned char character : value)
		{
			hash ^= static_cast<uint64_t>(character);
			hash *= 1099511628211ull;
		}
		return hash;
	}

	std::string ToStableHex(const uint64_t value)
	{
		std::ostringstream output;
		output << std::hex << std::setfill('0') << std::setw(16) << value;
		return output.str();
	}

	bool_t IsSameNavigationFloat(f32_t left, f32_t right)
	{
		return std::fabs(left - right) <= 0.000001f;
	}

	bool_t HasSameNavigationGridIdentity(
		const NAVGRID_AUTHORING_DESC& left,
		const NAVGRID_AUTHORING_DESC& right)
	{
		return left.areaId == right.areaId &&
			left.width == right.width &&
			left.height == right.height &&
			IsSameNavigationFloat(left.cellSize, right.cellSize) &&
			IsSameNavigationFloat(left.originX, right.originX) &&
			IsSameNavigationFloat(left.originZ, right.originZ);
	}

	bool_t HasSameNavigationPath(
		const std::filesystem::path& left,
		const std::filesystem::path& right)
	{
		return left.lexically_normal() == right.lexically_normal();
	}

	bool_t ReadTextFile(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		outText.assign(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		return input.good() || input.eof();
	}

	bool_t ReadRequiredString(
		const Client::DATA_JSON_VALUE& object,
		const char_t* pName,
		std::string& outValue)
	{
		const Client::DATA_JSON_VALUE* value = object.Find(pName);
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	/* is_regular_file reports a missing file through the error_code, so a
	   bare "if (error)" cannot separate an absent optional document from a
	   real inspection failure. Only the latter may abort a load. */
	bool_t IsFileInspectionFailure(const std::error_code& error)
	{
		return error && error != std::errc::no_such_file_or_directory;
	}

	std::filesystem::path ResolveDataCatalogPath(
		const std::string& value)
	{
		const std::filesystem::path serialized(value);
		if (serialized.empty() || serialized.is_absolute() ||
			serialized.has_root_path())
		{
			return {};
		}
		auto part = serialized.begin();
		if (part == serialized.end() || *part != L"Data")
			return {};
		std::filesystem::path relative;
		for (++part; part != serialized.end(); ++part)
			relative /= *part;
		return CProjectDataRoot::Resolve(relative);
	}

	const char_t* SimulationPlaybackStateLabel(
		const DESTRUCTION_SIMULATION_PLAYBACK_STATE state)
	{
		switch (state)
		{
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::STOPPED:
			return "STOPPED";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING:
			return "PLAYING";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED:
			return "PAUSED";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED:
			return "FINISHED";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::END:
		default:
			return "INVALID";
		}
	}

	const char_t* SimulationElementStateLabel(
		const DESTRUCTION_SIMULATION_ELEMENT_STATE state)
	{
		switch (state)
		{
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING:
			return "WAITING";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE:
			return "ACTIVE";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED:
			return "EXPIRED";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED:
			return "FILTERED";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::END:
		default:
			return "INVALID";
		}
	}

	/* Two co-located Deploy walls that share every authored simulation value are
	   one wall to the author. They stay two document elements because an element
	   owns exactly one source placement, but the outliner, the authoring panel and
	   the Solo scope treat them as a single emitter. Profiles whose emitters were
	   tuned apart (the 3705102 wall ring) stay split, so a linked Apply can never
	   overwrite a direction the author set per wall. */
	bool_t HasSharedEmitterAuthoring(
		const Client::DESTRUCTION_SIMULATION_ELEMENT& left,
		const Client::DESTRUCTION_SIMULATION_ELEMENT& right)
	{
		return left.vSpawnOffset.x == right.vSpawnOffset.x &&
			left.vSpawnOffset.y == right.vSpawnOffset.y &&
			left.vSpawnOffset.z == right.vSpawnOffset.z &&
			left.vDirection.x == right.vDirection.x &&
			left.vDirection.y == right.vDirection.y &&
			left.vDirection.z == right.vDirection.z &&
			left.fSpeedMetersPerSecond == right.fSpeedMetersPerSecond &&
			left.fGravityScale == right.fGravityScale &&
			left.fLifetimeSeconds == right.fLifetimeSeconds &&
			left.Trigger.eKind == right.Trigger.eKind &&
			left.Trigger.fTimeSeconds == right.Trigger.fTimeSeconds &&
			left.Trigger.receiverCollisionId ==
				right.Trigger.receiverCollisionId;
	}

	bool_t AreEmittersAuthoredAsOneWall(
		const Client::DESTRUCTION_SIMULATION_PROFILE& profile)
	{
		if (2u > profile.Elements.size())
			return false;
		for (size_t index = 1u; index < profile.Elements.size(); ++index)
		{
			if (!HasSharedEmitterAuthoring(
				profile.Elements[0], profile.Elements[index]))
			{
				return false;
			}
		}
		return true;
	}

	const char_t* SimulationScopeLabel(
		const DESTRUCTION_SIMULATION_SCOPE scope)
	{
		switch (scope)
		{
		case DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS:
			return "ALL FRAGMENTS";
		case DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED:
			return "SOLO EMITTER";
		case DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT:
			return "SOLO FRAGMENT";
		case DESTRUCTION_SIMULATION_SCOPE::END:
		default:
			return "INVALID";
		}
	}

	bool_t NormalizeSimulationDirection(float3_t& direction)
	{
		const f32_t lengthSquared = direction.x * direction.x +
			direction.y * direction.y + direction.z * direction.z;
		if (!std::isfinite(lengthSquared) || lengthSquared <= 0.000001f)
			return false;
		const f32_t inverseLength = 1.f / std::sqrt(lengthSquared);
		direction.x *= inverseLength;
		direction.y *= inverseLength;
		direction.z *= inverseLength;
		return true;
	}

}

struct Client::CMapTool::NAVIGATION_RENDER_RESOURCES final
{
	shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch;
	shared_ptr<BasicEffect> pEffect;
	ComPtr<ID3D11InputLayout> pInputLayout;
};

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
	}
	m_bOpen = isOpen;
	if (!m_bOpen)
		m_bDestructionSimulationClearRequested = true;
}

void Client::CMapTool::Update(f32_t fTimeDelta)
{
	const uint32_t currentLevelIndex =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isMapAuthoringLevel =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelIndex &&
		CMapEditorWorkspaceService::Is_Active();
	Handle_LevelTransition(currentLevelIndex, isMapAuthoringLevel);
	if (isMapAuthoringLevel && nullptr != m_pMapLightPresentation &&
		!m_pMapLightPresentation->Submit_Frame() &&
		!m_bMapLightSubmissionFailureReported)
	{
		m_bMapLightSubmissionFailureReported = true;
		OutputDebugStringA(("[MapTool][MapLight] " +
			m_pMapLightPresentation->Get_Status() + "\n").c_str());
	}
	Update_DestructionSimulation(fTimeDelta, isMapAuthoringLevel);
	Update_WorldInteraction(isMapAuthoringLevel);

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

void Client::CMapTool::Update_DestructionSimulation(
	const f32_t fTimeDelta,
	const bool_t isMapAuthoringLevel)
{
	if (nullptr == m_pDestructionSimulationController)
		return;
	if (m_bDestructionSimulationClearRequested)
	{
		m_pDestructionSimulationController->Clear();
		m_bDestructionSimulationClearRequested = false;
	}

	const bool_t isAuditionVisible = isMapAuthoringLevel && m_bOpen &&
		TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode;
	if (!isAuditionVisible)
	{
		if (m_pDestructionSimulationController->Get_Runtime().Is_Staged() ||
			m_pDestructionSimulationController->Has_PendingCommand())
		{
			/* Leaving the audition must release its exclusive debug clock and
			   restore every DeployProp preview seam. Clear also discards a stage
			   command latched by the preceding Render frame. */
			m_pDestructionSimulationController->Clear();
		}
		return;
	}

	m_pDestructionSimulationController->Update(
		(std::max)(0.f, fTimeDelta));
	m_pDestructionSimulationController->Post_Physics_Update();

	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT& snapshot =
		m_pDestructionSimulationController->Get_Snapshot();
	if (isAuditionVisible && m_bDestructionSimulationLoop &&
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED == snapshot.eState &&
		!m_pDestructionSimulationController->Has_PendingCommand())
	{
		m_pDestructionSimulationController->Request_Reset();
		m_pDestructionSimulationController->Request_Play();
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
		m_bWorldTriggerTargetPickArmed = false;
		m_bSpawnAnchorPlacementArmed = false;
		m_bDestructionPickArmed = false;
		m_bDestructionAddMemberArmed = false;
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
		if (m_bWorldTriggerTargetPickArmed && mousePressed)
			Try_PickWorldTriggerTarget();
		else if (m_bSpawnAnchorPlacementArmed && mousePressed)
			Try_PlaceSpawnAnchor();
		else if (m_bWorldGameplayPlacementArmed && mousePressed)
			Try_PlaceWorldGameplay();
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
		Render_WorkspaceBar(isMapAuthoringLevel);
		ImGui::Separator();
		Render_ModeBar();
		ImGui::Separator();
		Render_ActiveMode(isMapAuthoringLevel);
	}

	ImGui::End();

	if (isOpen != m_bOpen)
		SetOpen(isOpen);
	Render_DestructionSimulationWindow(isMapAuthoringLevel);
}

void Client::CMapTool::Render_WorldOverlay(bool_t isAssetTest)
{
	if (!isAssetTest || TOOL_MODE::NAVIGATION != m_eToolMode)
		return;

	if (NAVIGATION_MODE::BAKE == m_eNavigationMode)
		Render_NavigationBoundsOverlay();
	else
		Render_NavigationOverlay();
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
	ImGui::SetNextItemWidth(320.f);
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
					Switch_EditorArea(index);
				}
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == active);
	if (ImGui::Button("Focus Area") && !Focus_ActiveEditorAreaCamera())
		m_Status = m_CameraStatus;
	ImGui::EndDisabled();
	if ("LV_BER_BERNCASTLE" == m_Catalog.Get_AreaId())
	{
		ImGui::SameLine();
		if (ImGui::Checkbox(
			"Show Bern Landscape",
			&m_bShowBernLandscape))
		{
			Set_EnvironmentPhase(m_EnvironmentPhase);
			m_Status = m_bShowBernLandscape ?
				"Bern Landscape preview enabled" :
				"Bern Landscape hidden; authoring data preserved";
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
					Switch_EditorArea(m_iPendingEditorArea);
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
				Switch_EditorArea(m_iPendingEditorArea);
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
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before saving";
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
	if (m_bDirty && !Save_Placements())
		return false;
	if (m_bSpawnGroupsDirty && !Save_SpawnGroups())
		return false;
	if (m_bWorldGameplayDirty && !Save_WorldGameplay())
		return false;
	if ((m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty()) && !Save_Navigation())
	{
		return false;
	}
	if ((m_DestructionDocument.Is_Dirty() ||
		m_DestructionSimulationDocument.Is_Dirty()) &&
		!Save_DestructionAuthoringPair())
	{
		return false;
	}
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

void Client::CMapTool::Render_WorldGameplayPanel(bool_t isAssetTest)
{
	ImGui::TextUnformatted("World Gameplay Authoring");
	ImGui::TextDisabled(
		"Player spawn, Trigger Box actions, and Collision Boxes are Server authority.");
	ImGui::TextDisabled(
		"Options: Player Spawn, NPC, Boss, Trigger Box, Collision Box.");
	ImGui::TextWrapped("%s", m_WorldGameplayStatus.c_str());
	ImGui::Separator();
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	if (nullptr == active ||
		EDITOR_GAMEPLAY_POLICY::REQUIRED != active->gameplayPolicy)
	{
		ImGui::TextDisabled(
			"This Area has no gameplay authoring document. No empty file will be created.");
		return;
	}

	ImGui::BeginDisabled(!isAssetTest || !m_Catalog.Is_Ready());
	if (ImGui::Button("Save Gameplay"))
		Save_WorldGameplay();
	ImGui::SameLine();
	if (ImGui::Button("Reload Gameplay"))
		Load_WorldGameplay();
	ImGui::SameLine();
	ImGui::Text("Revision: %u | Placements: %zu%s",
		m_WorldGameplayDocument.Get_Revision(),
		m_WorldGameplayDocument.Get_Placements().size(),
		m_bWorldGameplayDirty ? "  *unsaved" : "");
	ImGui::Separator();

	if (ImGui::RadioButton("Player Spawn",
		WORLD_PLACEMENT_KIND::PLAYER_SPAWN == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::PLAYER_SPAWN;
	ImGui::SameLine();
	if (ImGui::RadioButton("NPC",
		WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::NPC;
	ImGui::SameLine();
	if (ImGui::RadioButton("Boss",
		WORLD_PLACEMENT_KIND::BOSS == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::BOSS;
	ImGui::SameLine();
	if (ImGui::RadioButton("Trigger Box",
		WORLD_PLACEMENT_KIND::TRIGGER_BOX == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::TRIGGER_BOX;
	ImGui::SameLine();
	if (ImGui::RadioButton("Collision Box",
		WORLD_PLACEMENT_KIND::COLLISION_BOX == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::COLLISION_BOX;

	ImGui::InputText("Placement ID", m_WorldPlacementId,
		std::size(m_WorldPlacementId));
	const bool_t isTriggerPlacement =
		WORLD_PLACEMENT_KIND::TRIGGER_BOX == m_eWorldPlacementKind;
	const bool_t isCollisionPlacement =
		WORLD_PLACEMENT_KIND::COLLISION_BOX == m_eWorldPlacementKind;
	if (isTriggerPlacement || isCollisionPlacement)
	{
		ImGui::DragFloat3(
			"Default Box Scale (Half Extents)",
			&m_WorldTriggerHalfExtents.x,
			0.1f,
			0.1f,
			1000.f,
			"%.2f",
			ImGuiSliderFlags_AlwaysClamp);
		if (isTriggerPlacement)
		{
			ImGui::Checkbox("Trigger Once", &m_bWorldTriggerOnce);
			ImGui::TextDisabled(
				"Place the box first, then select it and choose one typed action below.");
		}
		else
		{
			ImGui::TextDisabled(
				"Collision Box blocks Server-authoritative player walking.");
		}
	}
	else
	{
		if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN == m_eWorldPlacementKind)
		{
			ImGui::TextDisabled(
				"Player Spawn owns position/yaw only; class is selected by the session.");
		}
		else if (WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind)
		{
			strcpy_s(m_WorldArchetypeId, "NPC_BEDA");
			m_WorldEncounterId[0] = '\0';
			ImGui::TextUnformatted("Archetype: NPC_BEDA (Npc_Beda)");
			ImGui::TextDisabled(
				"The current product path supports this one NpcCatalog presentation.");
		}
		else
		{
			ImGui::InputText("Archetype ID", m_WorldArchetypeId,
				std::size(m_WorldArchetypeId));
			ImGui::InputText("Encounter ID (optional)", m_WorldEncounterId,
				std::size(m_WorldEncounterId));
		}
	}
	if (ImGui::Button(m_bWorldGameplayPlacementArmed ?
		"Cancel World Placement" : "Arm World Placement"))
	{
		m_bWorldGameplayPlacementArmed =
			!m_bWorldGameplayPlacementArmed;
		if (m_bWorldGameplayPlacementArmed)
		{
			m_bWorldTriggerTargetPickArmed = false;
			m_bSpawnAnchorPlacementArmed = false;
		}
		m_WorldGameplayStatus = m_bWorldGameplayPlacementArmed ?
			"World placement armed: click a picked map surface; Esc cancels" :
			"World placement cancelled";
	}
	if (m_bWorldGameplayPlacementArmed)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PICKING: click the map surface to store the placement position");
	}

	ImGui::Separator();
	if (ImGui::BeginTable("WorldGameplayPlacements", 5,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY, ImVec2(0.f, 280.f)))
	{
		ImGui::TableSetupColumn("Placement ID");
		ImGui::TableSetupColumn("Kind");
		ImGui::TableSetupColumn("Archetype");
		ImGui::TableSetupColumn("Position");
		ImGui::TableSetupColumn("Enabled");
		ImGui::TableHeadersRow();
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			m_WorldGameplayDocument.Get_Placements())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected =
				m_SelectedWorldPlacementId == placement.placementId;
			if (ImGui::Selectable(placement.placementId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedWorldPlacementId = placement.placementId;
				m_WorldPlacementPositionDelta = {};
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::Kind_ToString(placement.eKind));
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(placement.archetypeId.c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%.2f, %.2f, %.2f", placement.position.x,
				placement.position.y, placement.position.z);
			ImGui::TableSetColumnIndex(4);
			ImGui::TextUnformatted(placement.isEnabled ? "yes" : "no");
		}
		ImGui::EndTable();
	}

	WORLD_GAMEPLAY_PLACEMENT* selected =
		m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
	if (nullptr != selected)
	{
		ImGui::SeparatorText("Selected Gameplay Placement");
		WORLD_GAMEPLAY_PLACEMENT staged = *selected;
		bool_t edited = false;
		edited |= ImGui::DragFloat3("Position", &staged.position.x,
			0.1f, -100000.f, 100000.f, "%.3f");
		edited |= ImGui::DragFloat("Yaw Degrees", &staged.yawDegrees,
			0.5f, -360.f, 360.f, "%.2f");
		if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN == staged.eKind)
		{
			ImGui::SeparatorText("Player Spawn Position Offset");
			ImGui::DragFloat3(
				"Position Delta",
				&m_WorldPlacementPositionDelta.x,
				0.1f,
				-100000.f,
				100000.f,
				"%+.3f");
			if (ImGui::Button("Apply Delta To Spawn Position"))
			{
				staged.position.x += m_WorldPlacementPositionDelta.x;
				staged.position.y += m_WorldPlacementPositionDelta.y;
				staged.position.z += m_WorldPlacementPositionDelta.z;
				m_WorldPlacementPositionDelta = {};
				edited = true;
			}
			ImGui::TextDisabled(
				"Example: +50, 0, 0 resolves and saves Position.x + 50; Server uses the saved result.");
		}
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == staged.eKind)
		{
			edited |= ImGui::DragFloat3(
				"Half Extents",
				&staged.halfExtents.x,
				0.1f,
				0.1f,
				1000.f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);
			edited |= ImGui::Checkbox("Trigger Once", &staged.isTriggerOnce);

			int actionOption = 0;
			if (1u == staged.triggerEvents.size())
			{
				switch (staged.triggerEvents.front().eKind)
				{
				case WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER:
					actionOption = 1;
					break;
				case WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL:
					actionOption = 2;
					break;
				case WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP:
					actionOption = 3;
					break;
				case WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER:
					actionOption = 4;
					break;
				default:
					break;
				}
			}
			const char_t* actionOptions[] =
			{
				"None", "Move Player", "Change Level",
				"Activate Spawn Group", "Activate Encounter"
			};
			if (ImGui::Combo("Action", &actionOption,
				actionOptions, static_cast<int>(std::size(actionOptions))))
			{
				m_bWorldTriggerTargetPickArmed = false;
				if (0 == actionOption)
				{
					staged.triggerEvents.clear();
					staged.isEnabled = false;
				}
				else
				{
					WORLD_TRIGGER_EVENT action{};
					if (1 == actionOption)
					{
						action.eKind = WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER;
						action.targetPosition = staged.position;
					}
					else if (2 == actionOption)
					{
						action.eKind = WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL;
						action.eTargetWorldId =
							LostArk::Shared::WORLD_ID::VALTAN_ARENA;
					}
					else
					{
						action.eKind = 3 == actionOption ?
							WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP :
							WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER;
						if (3 == actionOption &&
							!m_SpawnGroupDocument.Get_Groups().empty())
						{
							action.targetId =
								m_SpawnGroupDocument.Get_Groups().front().spawnGroupId;
						}
						else if (4 == actionOption)
						{
							const auto boss = std::find_if(
								m_WorldGameplayDocument.Get_Placements().begin(),
								m_WorldGameplayDocument.Get_Placements().end(),
								[](const WORLD_GAMEPLAY_PLACEMENT& value)
								{
									return WORLD_PLACEMENT_KIND::BOSS == value.eKind;
								});
							if (m_WorldGameplayDocument.Get_Placements().end() != boss)
								action.targetId = boss->placementId;
						}
					}
					staged.triggerEvents.assign(1u, action);
				}
				edited = true;
			}
			const bool_t hasMoveAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER ==
					staged.triggerEvents.front().eKind;
			if (hasMoveAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Move Player Action");
				edited |= ImGui::DragFloat3(
					"Target Position", &action.targetPosition.x,
					0.1f, -100000.f, 100000.f, "%.3f");
				edited |= ImGui::DragFloat(
					"Duration Seconds", &action.durationSeconds,
					0.05f, 0.05f, 10.f, "%.2f",
					ImGuiSliderFlags_AlwaysClamp);
				edited |= ImGui::DragFloat(
					"Arc Height", &action.arcHeight,
					0.1f, 0.f, 1000.f, "%.2f",
					ImGuiSliderFlags_AlwaysClamp);
				if (ImGui::Button(m_bWorldTriggerTargetPickArmed ?
					"Cancel Move Target Pick" : "Pick Move Target On Map"))
				{
					m_bWorldTriggerTargetPickArmed =
						!m_bWorldTriggerTargetPickArmed;
					m_bWorldGameplayPlacementArmed = false;
					m_WorldGameplayStatus = m_bWorldTriggerTargetPickArmed ?
						"Move target armed: click the destination surface; Esc cancels" :
						"Move target pick cancelled";
				}
			}
			const bool_t hasChangeLevelAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL ==
					staged.triggerEvents.front().eKind;
			if (hasChangeLevelAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Change Level Action");
				int targetOption =
					LostArk::Shared::WORLD_ID::BERN == action.eTargetWorldId ? 0 : 1;
				const char_t* targetOptions[] = { "BERN", "VALTAN_ARENA" };
				if (ImGui::Combo("Target World", &targetOption,
					targetOptions, static_cast<int>(std::size(targetOptions))))
				{
					action.eTargetWorldId = 0 == targetOption ?
						LostArk::Shared::WORLD_ID::BERN :
						LostArk::Shared::WORLD_ID::VALTAN_ARENA;
					edited = true;
				}
				ImGui::TextDisabled(
					"The Server changes rooms first; Client consumes S2C_ENTER_ACCEPTED.");
			}
			const bool_t hasSpawnGroupAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP ==
					staged.triggerEvents.front().eKind;
			if (hasSpawnGroupAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Activate Spawn Group Action");
				const char_t* preview = action.targetId.empty() ?
					"<select spawn group>" : action.targetId.c_str();
				if (ImGui::BeginCombo("Spawn Group", preview))
				{
					for (const SPAWN_GROUP_RECORD& group :
						m_SpawnGroupDocument.Get_Groups())
					{
						const bool_t isSelected = action.targetId == group.spawnGroupId;
						if (ImGui::Selectable(group.spawnGroupId.c_str(), isSelected))
						{
							action.targetId = group.spawnGroupId;
							edited = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::TextDisabled(
					"Server activates this stable group; prerequisite and once rules stay authoritative.");
			}
			const bool_t hasEncounterAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER ==
					staged.triggerEvents.front().eKind;
			if (hasEncounterAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Activate Encounter Action");
				const char_t* preview = action.targetId.empty() ?
					"<select disabled boss placement>" : action.targetId.c_str();
				if (ImGui::BeginCombo("Boss Placement", preview))
				{
					for (const WORLD_GAMEPLAY_PLACEMENT& placement :
						m_WorldGameplayDocument.Get_Placements())
					{
						if (WORLD_PLACEMENT_KIND::BOSS != placement.eKind)
							continue;
						const bool_t isSelected = action.targetId == placement.placementId;
						if (ImGui::Selectable(placement.placementId.c_str(), isSelected))
						{
							action.targetId = placement.placementId;
							edited = true;
						}
					}
					ImGui::EndCombo();
				}
			}

			const bool_t hasSupportedAction =
				hasMoveAction || hasChangeLevelAction ||
				hasSpawnGroupAction || hasEncounterAction;
			ImGui::BeginDisabled(!hasSupportedAction);
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
			ImGui::EndDisabled();
			if (!hasSupportedAction)
			{
				staged.isEnabled = false;
				ImGui::TextDisabled(
					"A Trigger Box needs exactly one supported action before it can be enabled.");
			}
		}
		else if (WORLD_PLACEMENT_KIND::COLLISION_BOX == staged.eKind)
		{
			edited |= ImGui::DragFloat3(
				"Half Extents",
				&staged.halfExtents.x,
				0.1f,
				0.1f,
				1000.f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
		}
		else
		{
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
		}
		if (edited)
		{
			if (CWorldGameplayDocument::Is_Valid(staged))
			{
				CWorldGameplayDocument previous = m_WorldGameplayDocument;
				*selected = staged;
				m_WorldGameplayDocument.Mark_Edited();
				vector<TRIGGER_BOX_ENTRY> stagedBoxes;
				if (Stage_WorldTriggerBoxes(
					m_WorldGameplayDocument, stagedBoxes))
				{
					Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
					m_WorldTriggerBoxes = std::move(stagedBoxes);
					m_bWorldGameplayDirty = true;
					m_WorldGameplayStatus = "Gameplay placement edited";
				}
				else
				{
					m_WorldGameplayDocument = std::move(previous);
				}
			}
			else
			{
				m_WorldGameplayStatus = "Gameplay edit rejected by validation";
			}
			selected = m_WorldGameplayDocument.Find(
				m_SelectedWorldPlacementId);
		}
		if (nullptr != selected &&
			ImGui::Button("Delete Gameplay Placement"))
		{
			const std::string deletedId = selected->placementId;
			CWorldGameplayDocument previous = m_WorldGameplayDocument;
			if (m_WorldGameplayDocument.Remove(deletedId))
			{
				vector<TRIGGER_BOX_ENTRY> stagedBoxes;
				if (Stage_WorldTriggerBoxes(
					m_WorldGameplayDocument, stagedBoxes))
				{
					Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
					m_WorldTriggerBoxes = std::move(stagedBoxes);
					m_SelectedWorldPlacementId.clear();
					m_bWorldGameplayDirty = true;
					m_WorldGameplayStatus =
						"Deleted gameplay placement: " + deletedId;
				}
				else
				{
					m_WorldGameplayDocument = std::move(previous);
				}
			}
		}
	}
	Render_SpawnGroupsPanel();
	ImGui::EndDisabled();
}

void Client::CMapTool::Render_SpawnGroupsPanel()
{
	ImGui::SeparatorText("Server Spawn Groups");
	ImGui::TextDisabled(
		"Anchors and waves stay in SpawnGroups.world.json; Trigger Boxes only reference a stable group ID.");
	if (ImGui::Button("Save Spawn Groups"))
		Save_SpawnGroups();
	ImGui::SameLine();
	if (ImGui::Button("Reload Spawn Groups"))
		Load_SpawnGroups();
	ImGui::SameLine();
	ImGui::Text("Revision: %u | Anchors: %zu | Groups: %zu%s",
		m_SpawnGroupDocument.Get_Revision(),
		m_SpawnGroupDocument.Get_Anchors().size(),
		m_SpawnGroupDocument.Get_Groups().size(),
		m_bSpawnGroupsDirty ? "  *unsaved" : "");

	ImGui::InputText("Anchor ID", m_SpawnAnchorId, std::size(m_SpawnAnchorId));
	if (ImGui::Button(m_bSpawnAnchorPlacementArmed ?
		"Cancel Spawn Anchor Pick" : "Place Spawn Anchor On Map"))
	{
		m_bSpawnAnchorPlacementArmed = !m_bSpawnAnchorPlacementArmed;
		m_bWorldGameplayPlacementArmed = false;
		m_bWorldTriggerTargetPickArmed = false;
		m_WorldGameplayStatus = m_bSpawnAnchorPlacementArmed ?
			"Spawn anchor armed: click a map surface" : "Spawn anchor pick cancelled";
	}
	if (m_bSpawnAnchorPlacementArmed)
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PICKING: click the exact monster spawn position");

	if (ImGui::BeginTable("SpawnAnchors", 2,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
		ImVec2(0.f, 130.f)))
	{
		ImGui::TableSetupColumn("Anchor ID");
		ImGui::TableSetupColumn("Position");
		ImGui::TableHeadersRow();
		for (const SPAWN_ANCHOR_RECORD& anchor :
			m_SpawnGroupDocument.Get_Anchors())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected = m_SelectedSpawnAnchorId == anchor.anchorId;
			if (ImGui::Selectable(anchor.anchorId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
				m_SelectedSpawnAnchorId = anchor.anchorId;
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f, %.2f, %.2f", anchor.position.x,
				anchor.position.y, anchor.position.z);
		}
		ImGui::EndTable();
	}
	SPAWN_ANCHOR_RECORD* selectedAnchor =
		m_SpawnGroupDocument.Find_Anchor(m_SelectedSpawnAnchorId);
	if (nullptr != selectedAnchor)
	{
		bool_t edited = false;
		edited |= ImGui::DragFloat3("Selected Anchor Position",
			&selectedAnchor->position.x, 0.1f, -100000.f, 100000.f, "%.3f");
		edited |= ImGui::DragFloat("Selected Anchor Yaw",
			&selectedAnchor->yawDegrees, 0.5f, -360.f, 360.f, "%.2f");
		if (edited)
		{
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
			vector<TRIGGER_BOX_ENTRY> stagedBoxes;
			if (Stage_SpawnAnchorBoxes(m_SpawnGroupDocument, stagedBoxes))
			{
				Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
				m_SpawnAnchorBoxes = std::move(stagedBoxes);
			}
		}
		if (ImGui::Button("Delete Selected Spawn Anchor"))
		{
			if (m_SpawnGroupDocument.Remove_Anchor(
				m_SelectedSpawnAnchorId, m_WorldGameplayStatus))
			{
				m_SelectedSpawnAnchorId.clear();
				m_bSpawnGroupsDirty = true;
				vector<TRIGGER_BOX_ENTRY> stagedBoxes;
				if (Stage_SpawnAnchorBoxes(m_SpawnGroupDocument, stagedBoxes))
				{
					Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
					m_SpawnAnchorBoxes = std::move(stagedBoxes);
				}
			}
		}
	}

	ImGui::SeparatorText("Spawn Group Definition");
	ImGui::InputText("New Spawn Group ID", m_SpawnGroupId,
		std::size(m_SpawnGroupId));
	int maxAlive = static_cast<int>(m_iSpawnGroupMaxAlive);
	if (ImGui::InputInt("New Group Max Alive", &maxAlive))
		m_iSpawnGroupMaxAlive = static_cast<uint32_t>((std::max)(1, (std::min)(64, maxAlive)));
	if (ImGui::Button("Add Spawn Group"))
	{
		SPAWN_GROUP_RECORD group;
		group.spawnGroupId = m_SpawnGroupId;
		group.maxAlive = m_iSpawnGroupMaxAlive;
		if (m_SpawnGroupDocument.Add_Group(group, m_WorldGameplayStatus))
		{
			m_SelectedSpawnGroupId = group.spawnGroupId;
			m_bSpawnGroupsDirty = true;
		}
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Add at least one wave before Save.");

	if (ImGui::BeginTable("SpawnGroups", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
		ImVec2(0.f, 150.f)))
	{
		ImGui::TableSetupColumn("Group ID");
		ImGui::TableSetupColumn("Prerequisite");
		ImGui::TableSetupColumn("Max Alive");
		ImGui::TableSetupColumn("Waves");
		ImGui::TableHeadersRow();
		for (const SPAWN_GROUP_RECORD& group :
			m_SpawnGroupDocument.Get_Groups())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected = m_SelectedSpawnGroupId == group.spawnGroupId;
			if (ImGui::Selectable(group.spawnGroupId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedSpawnGroupId = group.spawnGroupId;
				m_SelectedSpawnWaveId.clear();
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(group.requiredCompletedGroupId.empty() ?
				"<none>" : group.requiredCompletedGroupId.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%u", group.maxAlive);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%zu", group.waves.size());
		}
		ImGui::EndTable();
	}

	SPAWN_GROUP_RECORD* selectedGroup =
		m_SpawnGroupDocument.Find_Group(m_SelectedSpawnGroupId);
	if (nullptr == selectedGroup)
		return;

	bool_t groupEdited = false;
	int selectedMaxAlive = static_cast<int>(selectedGroup->maxAlive);
	if (ImGui::InputInt("Selected Group Max Alive", &selectedMaxAlive))
	{
		selectedGroup->maxAlive = static_cast<uint32_t>(
			(std::max)(1, (std::min)(64, selectedMaxAlive)));
		groupEdited = true;
	}
	const char_t* prerequisitePreview = selectedGroup->requiredCompletedGroupId.empty() ?
		"<none>" : selectedGroup->requiredCompletedGroupId.c_str();
	if (ImGui::BeginCombo("Required Completed Group", prerequisitePreview))
	{
		if (ImGui::Selectable("<none>", selectedGroup->requiredCompletedGroupId.empty()))
		{
			selectedGroup->requiredCompletedGroupId.clear();
			groupEdited = true;
		}
		for (const SPAWN_GROUP_RECORD& group : m_SpawnGroupDocument.Get_Groups())
		{
			if (group.spawnGroupId == selectedGroup->spawnGroupId)
				continue;
			if (ImGui::Selectable(group.spawnGroupId.c_str(),
				selectedGroup->requiredCompletedGroupId == group.spawnGroupId))
			{
				selectedGroup->requiredCompletedGroupId = group.spawnGroupId;
				groupEdited = true;
			}
		}
		ImGui::EndCombo();
	}
	if (groupEdited)
	{
		m_SpawnGroupDocument.Mark_Edited();
		m_bSpawnGroupsDirty = true;
	}

	ImGui::InputText("New Wave ID", m_SpawnWaveId, std::size(m_SpawnWaveId));
	int startDelay = static_cast<int>(m_iSpawnWaveStartDelayMs);
	if (ImGui::InputInt("Wave Start Delay Ms", &startDelay))
		m_iSpawnWaveStartDelayMs = static_cast<uint32_t>((std::max)(0, startDelay));
	if (ImGui::Button("Add Wave"))
	{
		if (selectedGroup->waves.size() < CSpawnGroupDocument::MAX_WAVE_COUNT &&
			CSpawnGroupDocument::Is_ValidStableId(m_SpawnWaveId) &&
			std::none_of(selectedGroup->waves.begin(), selectedGroup->waves.end(),
				[&](const SPAWN_WAVE_RECORD& value) { return value.waveId == m_SpawnWaveId; }))
		{
			SPAWN_WAVE_RECORD wave;
			wave.waveId = m_SpawnWaveId;
			wave.startDelayMs = m_iSpawnWaveStartDelayMs;
			selectedGroup->waves.push_back(std::move(wave));
			m_SelectedSpawnWaveId = m_SpawnWaveId;
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
		}
		else
			m_WorldGameplayStatus = "Wave ID is invalid, duplicate, or over limit";
	}

	for (const SPAWN_WAVE_RECORD& wave : selectedGroup->waves)
	{
		ImGui::SameLine();
		if (ImGui::RadioButton(wave.waveId.c_str(), m_SelectedSpawnWaveId == wave.waveId))
			m_SelectedSpawnWaveId = wave.waveId;
	}
	const auto waveIter = std::find_if(selectedGroup->waves.begin(), selectedGroup->waves.end(),
		[&](const SPAWN_WAVE_RECORD& value) { return value.waveId == m_SelectedSpawnWaveId; });
	if (selectedGroup->waves.end() == waveIter)
		return;
	SPAWN_WAVE_RECORD& selectedWave = *waveIter;

	static constexpr const char_t* ARCHETYPES[] =
	{
		"MONSTER_VALTAN_PADD_01",
		"MONSTER_VALTAN_SJFC_00_4",
		"MONSTER_VALTAN_0019_05",
		"MINIBOSS_LUGARU"
	};
	int archetypeOption = static_cast<int>(m_iSpawnArchetypeOption);
	if (ImGui::Combo("Entry Archetype", &archetypeOption, ARCHETYPES,
		static_cast<int>(std::size(ARCHETYPES))))
		m_iSpawnArchetypeOption = static_cast<uint32_t>(archetypeOption);
	const char_t* anchorPreview = m_SelectedSpawnAnchorId.empty() ?
		"<select anchor>" : m_SelectedSpawnAnchorId.c_str();
	if (ImGui::BeginCombo("Entry Anchor", anchorPreview))
	{
		for (const SPAWN_ANCHOR_RECORD& anchor : m_SpawnGroupDocument.Get_Anchors())
			if (ImGui::Selectable(anchor.anchorId.c_str(),
				m_SelectedSpawnAnchorId == anchor.anchorId))
				m_SelectedSpawnAnchorId = anchor.anchorId;
		ImGui::EndCombo();
	}
	int entryCount = static_cast<int>(m_iSpawnEntryCount);
	int initialDelay = static_cast<int>(m_iSpawnInitialDelayMs);
	int interval = static_cast<int>(m_iSpawnIntervalMs);
	if (ImGui::InputInt("Entry Count", &entryCount))
		m_iSpawnEntryCount = static_cast<uint32_t>((std::max)(1, entryCount));
	if (ImGui::InputInt("Entry Initial Delay Ms", &initialDelay))
		m_iSpawnInitialDelayMs = static_cast<uint32_t>((std::max)(0, initialDelay));
	if (ImGui::InputInt("Entry Spawn Interval Ms", &interval))
		m_iSpawnIntervalMs = static_cast<uint32_t>((std::max)(0, interval));
	if (ImGui::Button("Add Entry To Selected Wave"))
	{
		if (selectedWave.entries.size() < CSpawnGroupDocument::MAX_ENTRY_COUNT &&
			nullptr != m_SpawnGroupDocument.Find_Anchor(m_SelectedSpawnAnchorId))
		{
			SPAWN_WAVE_ENTRY_RECORD entry;
			entry.archetypeId = ARCHETYPES[m_iSpawnArchetypeOption];
			entry.count = m_iSpawnEntryCount;
			entry.anchorId = m_SelectedSpawnAnchorId;
			entry.initialDelayMs = m_iSpawnInitialDelayMs;
			entry.spawnIntervalMs = m_iSpawnIntervalMs;
			selectedWave.entries.push_back(std::move(entry));
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
		}
		else
			m_WorldGameplayStatus = "Select a valid anchor or remove an entry first";
	}
	for (size_t index = 0; index < selectedWave.entries.size(); ++index)
	{
		const auto& entry = selectedWave.entries[index];
		ImGui::PushID(static_cast<int>(index));
		ImGui::Text("%zu. %s x%u @ %s (+%ums, every %ums)", index + 1u,
			entry.archetypeId.c_str(), entry.count, entry.anchorId.c_str(),
			entry.initialDelayMs, entry.spawnIntervalMs);
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove"))
		{
			selectedWave.entries.erase(selectedWave.entries.begin() + index);
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
	}
	if (ImGui::Button("Delete Selected Wave"))
	{
		selectedGroup->waves.erase(waveIter);
		m_SelectedSpawnWaveId.clear();
		m_SpawnGroupDocument.Mark_Edited();
		m_bSpawnGroupsDirty = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Selected Spawn Group"))
	{
		if (m_SpawnGroupDocument.Remove_Group(
			m_SelectedSpawnGroupId, m_WorldGameplayStatus))
		{
			m_SelectedSpawnGroupId.clear();
			m_SelectedSpawnWaveId.clear();
			m_bSpawnGroupsDirty = true;
		}
	}
}

std::filesystem::path Client::CMapTool::Get_WorldGameplayPath() const
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	return nullptr != active &&
		EDITOR_GAMEPLAY_POLICY::REQUIRED == active->gameplayPolicy ?
		active->gameplayDocument : std::filesystem::path{};
}

std::filesystem::path Client::CMapTool::Get_SpawnGroupsPath() const
{
	const std::filesystem::path gameplayPath = Get_WorldGameplayPath();
	return gameplayPath.empty() ? std::filesystem::path{} :
		gameplayPath.parent_path() / L"SpawnGroups.world.json";
}

bool_t Client::CMapTool::Load_EncounterReference()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->encounterReference.empty())
	{
		m_EncounterReferenceStatus =
			"Active Area declares no encounter reference";
		return false;
	}

	CEncounterPatternReference staged;
	std::string status;
	if (!staged.Load(descriptor->encounterReference, status))
	{
		m_EncounterReferenceStatus = status;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			m_DestructionDocument,
			m_DeployRuntime,
			m_RuntimeBlockerDocument,
			m_WorldGameplayDocument,
			staged,
			status))
	{
		m_EncounterReferenceStatus = status;
		return false;
	}
	m_EncounterReference = std::move(staged);
	m_EncounterReferenceStatus = status;
	if (nullptr == m_EncounterReference.Find_Pattern(
		m_SelectedDestructionPatternId))
	{
		m_SelectedDestructionPatternId.clear();
	}
	return true;
}

bool_t Client::CMapTool::Reload_DestructionAuthoring()
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionStatus =
			"Apply or Revert the debris Detail draft before reloading";
		m_DestructionSimulationStatus = m_DestructionStatus;
		return false;
	}

	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->encounterReference.empty() ||
		descriptor->worldEventsDocument.empty())
	{
		m_DestructionStatus =
			"Active Area has no complete destruction authoring contract";
		return false;
	}

	CEncounterPatternReference stagedEncounter;
	std::string encounterStatus;
	if (!stagedEncounter.Load(
		descriptor->encounterReference, encounterStatus))
	{
		m_EncounterReferenceStatus = encounterStatus;
		m_DestructionStatus = encounterStatus;
		return false;
	}

	CWorldDestructionDocument stagedDestruction;
	std::error_code inspectError;
	const bool_t hasDocument = std::filesystem::is_regular_file(
		descriptor->worldEventsDocument, inspectError);
	if (IsFileInspectionFailure(inspectError))
	{
		m_DestructionStatus = "World events document is unreadable";
		return false;
	}

	std::string destructionStatus;
	if (!hasDocument)
	{
		stagedDestruction.Reset_Empty();
		destructionStatus =
			"No world events document yet. An empty draft was staged.";
	}
	else if (!stagedDestruction.Load(
		descriptor->worldEventsDocument,
		m_Catalog.Get_AreaId(),
		"ENCOUNTER_VALTAN",
		destructionStatus))
	{
		m_DestructionStatus = destructionStatus;
		return false;
	}

	std::string validationStatus;
	if (!Validate_DestructionExternalReferences(
		stagedDestruction,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		stagedEncounter,
		validationStatus))
	{
		m_EncounterReferenceStatus = encounterStatus;
		m_DestructionStatus = validationStatus;
		return false;
	}
	const uint64_t previousPlacementId = m_iSelectedDeployPlacementId;
	const std::string previousGroupId = m_SelectedDestructionGroupId;
	const std::string previousProfileId =
		m_SelectedDestructionSimulationProfileId;
	Restore_DestructionPreview();
	m_bDestructionPickArmed = false;
	m_bDestructionAddMemberArmed = false;
	m_bDestructionNewSettingArmed = false;
	m_bDestructionTimelinePlaying = false;
	m_fDestructionTimelineMs = 0.f;
	m_SelectedDestructionGroupId.clear();
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionPatternId.clear();
	m_SelectedDestructionStageId.clear();
	m_EncounterReference = std::move(stagedEncounter);
	m_DestructionDocument = std::move(stagedDestruction);
	m_EncounterReferenceStatus = encounterStatus;

	bool_t selectionRestored = false;
	if (0u != previousPlacementId)
	{
		selectionRestored = Select_DestructionWall(
			previousPlacementId, "authoring reload");
	}
	if (!selectionRestored && !previousGroupId.empty())
	{
		const DESTRUCTION_GROUP* previousGroup =
			m_DestructionDocument.Find_Group(previousGroupId);
		if (nullptr != previousGroup)
		{
			m_SelectedDestructionGroupId = previousGroup->groupId;
			selectionRestored = true;
		}
	}

	const DESTRUCTION_SIMULATION_PROFILE* previousProfile =
		previousProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(previousProfileId);
	if (nullptr != previousProfile &&
		previousProfile->groupId == m_SelectedDestructionGroupId)
	{
		Select_DestructionSimulationProfile(previousProfile->profileId);
	}
	else if (selectionRestored &&
		m_SelectedDestructionSimulationProfileId.empty())
	{
		const auto matchingProfile = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[this](const DESTRUCTION_SIMULATION_PROFILE& profile)
			{
				return profile.groupId == m_SelectedDestructionGroupId;
			});
		if (matchingProfile !=
			m_DestructionSimulationDocument.Get_Profiles().end())
		{
			Select_DestructionSimulationProfile(matchingProfile->profileId);
		}
	}
	if (selectionRestored)
		Sync_DestructionDraftFromSelection();
	Refresh_DestructionHighlight();
	m_DestructionStatus = destructionStatus + " " + validationStatus;
	return true;
}

bool_t Client::CMapTool::Load_WorldGameplay()
{
	const std::filesystem::path path = Get_WorldGameplayPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Gameplay load requires a ready map catalog";
		return false;
	}

	CWorldGameplayDocument stagedDocument;
	if (!stagedDocument.Load(
		path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
	{
		return false;
	}
	if (m_DestructionDocument.Is_Ready())
	{
		std::string status;
		if (!Validate_DestructionExternalReferences(
			m_DestructionDocument,
			m_DeployRuntime,
			m_RuntimeBlockerDocument,
			stagedDocument,
			m_EncounterReference,
			status))
		{
			m_WorldGameplayStatus = status;
			return false;
		}
	}
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_WorldTriggerBoxes(stagedDocument, stagedBoxes))
		return false;

	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	m_WorldGameplayDocument = std::move(stagedDocument);
	m_WorldTriggerBoxes = std::move(stagedBoxes);
	m_SelectedWorldPlacementId.clear();
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldTriggerTargetPickArmed = false;
	m_bWorldGameplayDirty = false;
	return true;
}

bool_t Client::CMapTool::Save_WorldGameplay()
{
	const std::filesystem::path path = Get_WorldGameplayPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Gameplay save requires a ready map catalog";
		return false;
	}
	if (m_DestructionDocument.Is_Ready())
	{
		std::string status;
		if (!Validate_CurrentDestructionReferences(status))
		{
			m_WorldGameplayStatus = status;
			m_DestructionStatus = status;
			return false;
		}
	}
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		m_WorldGameplayDocument.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind)
			continue;
		for (const WORLD_TRIGGER_EVENT& event : placement.triggerEvents)
		{
			if (WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP == event.eKind &&
				nullptr == m_SpawnGroupDocument.Find_Group(event.targetId))
			{
				m_WorldGameplayStatus =
					"Trigger references an unknown spawn group: " + event.targetId;
				return false;
			}
			if (WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER == event.eKind)
			{
				const WORLD_GAMEPLAY_PLACEMENT* target =
					m_WorldGameplayDocument.Find(event.targetId);
				if (nullptr == target || WORLD_PLACEMENT_KIND::BOSS != target->eKind ||
					target->isEnabled)
				{
					m_WorldGameplayStatus =
						"Encounter target must be a disabled boss placement: " + event.targetId;
					return false;
				}
			}
		}
	}
	if (!m_WorldGameplayDocument.Save(
		path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
	{
		return false;
	}
	m_bWorldGameplayDirty = false;
	return true;
}

bool_t Client::CMapTool::Load_SpawnGroups()
{
	const std::filesystem::path path = Get_SpawnGroupsPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Spawn group load requires a gameplay Area";
		return false;
	}
	CSpawnGroupDocument stagedDocument;
	if (!stagedDocument.Load(path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
		return false;
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_SpawnAnchorBoxes(stagedDocument, stagedBoxes))
		return false;
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	m_SpawnGroupDocument = std::move(stagedDocument);
	m_SpawnAnchorBoxes = std::move(stagedBoxes);
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_bSpawnAnchorPlacementArmed = false;
	m_bSpawnGroupsDirty = false;
	return true;
}

bool_t Client::CMapTool::Save_SpawnGroups()
{
	const std::filesystem::path path = Get_SpawnGroupsPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Spawn group save requires a gameplay Area";
		return false;
	}
	if (!m_SpawnGroupDocument.Save(
		path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
		return false;
	m_bSpawnGroupsDirty = false;
	return true;
}

bool_t Client::CMapTool::Try_PlaceSpawnAnchor()
{
	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_WorldGameplayStatus = "Spawn anchor placement failed: map pick missed";
		return false;
	}
	SPAWN_ANCHOR_RECORD anchor;
	anchor.anchorId = m_SpawnAnchorId;
	anchor.position = position;
	if (!m_SpawnGroupDocument.Add_Anchor(anchor, m_WorldGameplayStatus))
		return false;
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_SpawnAnchorBoxes(m_SpawnGroupDocument, stagedBoxes))
	{
		std::string removeStatus;
		m_SpawnGroupDocument.Remove_Anchor(anchor.anchorId, removeStatus);
		return false;
	}
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	m_SpawnAnchorBoxes = std::move(stagedBoxes);
	m_SelectedSpawnAnchorId = anchor.anchorId;
	m_bSpawnAnchorPlacementArmed = false;
	m_bSpawnGroupsDirty = true;
	return true;
}

bool_t Client::CMapTool::Stage_SpawnAnchorBoxes(
	const CSpawnGroupDocument& document,
	vector<TRIGGER_BOX_ENTRY>& outEntries)
{
	outEntries.clear();
	for (const SPAWN_ANCHOR_RECORD& anchor : document.Get_Anchors())
	{
		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = anchor.anchorId;
		desc.position = anchor.position;
		desc.halfExtents = float3_t(0.35f, 0.35f, 0.35f);
		desc.yawDegrees = anchor.yawDegrees;
		desc.isEnabled = true;
		desc.isCollisionBox = false;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_SpawnAnchors"),
			&desc,
			&gameObject)))
		{
			for (const TRIGGER_BOX_ENTRY& entry : outEntries)
				if (nullptr != entry.object)
					CGameInstance::Get().Remove_GameObject_from_Layer(
						m_iAuthoringLevelIndex, TEXT("Layer_SpawnAnchors"),
						static_pointer_cast<CGameObject>(entry.object));
			outEntries.clear();
			m_WorldGameplayStatus = "Spawn anchor presentation failed: " + anchor.anchorId;
			return false;
		}
		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex, TEXT("Layer_SpawnAnchors"), gameObject);
			return false;
		}
		triggerBox->Set_Selected(anchor.anchorId == m_SelectedSpawnAnchorId);
		triggerBox->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode);
		outEntries.push_back({ anchor.anchorId, std::move(triggerBox) });
	}
	return true;
}

bool_t Client::CMapTool::Try_PlaceWorldGameplay()
{
	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_WorldGameplayStatus = "Gameplay placement failed: map pick missed";
		return false;
	}

	WORLD_GAMEPLAY_PLACEMENT placement;
	placement.placementId = m_WorldPlacementId;
	placement.eKind = m_eWorldPlacementKind;
	placement.position = position;
	placement.yawDegrees = 0.f;
	if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == placement.eKind)
	{
		placement.isEnabled = false;
		placement.halfExtents = m_WorldTriggerHalfExtents;
		placement.isTriggerOnce = m_bWorldTriggerOnce;
	}
	else if (WORLD_PLACEMENT_KIND::COLLISION_BOX == placement.eKind)
	{
		placement.isEnabled = true;
		placement.halfExtents = m_WorldTriggerHalfExtents;
	}
	else
	{
		placement.archetypeId = m_WorldArchetypeId;
		placement.encounterId = m_WorldEncounterId;
		placement.isEnabled = true;
	}

	CWorldGameplayDocument previous = m_WorldGameplayDocument;
	if (!m_WorldGameplayDocument.Add(placement, m_WorldGameplayStatus))
		return false;
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_WorldTriggerBoxes(m_WorldGameplayDocument, stagedBoxes))
	{
		m_WorldGameplayDocument = std::move(previous);
		return false;
	}

	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	m_WorldTriggerBoxes = std::move(stagedBoxes);
	m_SelectedWorldPlacementId = placement.placementId;
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldGameplayDirty = true;
	return true;
}

bool_t Client::CMapTool::Try_PickWorldTriggerTarget()
{
	WORLD_GAMEPLAY_PLACEMENT* placement =
		m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
	if (nullptr == placement ||
		WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement->eKind)
	{
		m_bWorldTriggerTargetPickArmed = false;
		m_WorldGameplayStatus = "Move target pick requires a selected Trigger Box";
		return false;
	}

	float3_t target{};
	if (!Try_PickPlacementPosition(target))
	{
		m_WorldGameplayStatus = "Move target pick failed: map surface was not hit";
		return false;
	}

	WORLD_GAMEPLAY_PLACEMENT staged = *placement;
	if (1u != staged.triggerEvents.size() ||
		WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER != staged.triggerEvents.front().eKind)
	{
		WORLD_TRIGGER_EVENT action{};
		action.eKind = WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER;
		action.durationSeconds = 0.8f;
		staged.triggerEvents.assign(1u, action);
	}
	staged.triggerEvents.front().targetPosition = target;
	if (!CWorldGameplayDocument::Is_Valid(staged))
	{
		m_WorldGameplayStatus = "Move target pick produced an invalid trigger action";
		return false;
	}

	*placement = std::move(staged);
	m_WorldGameplayDocument.Mark_Edited();
	m_bWorldGameplayDirty = true;
	m_bWorldTriggerTargetPickArmed = false;
	m_WorldGameplayStatus = "Move target stored. Enable the Trigger Box and save gameplay.";
	return true;
}

bool_t Client::CMapTool::Stage_WorldTriggerBoxes(
	const CWorldGameplayDocument& document,
	vector<TRIGGER_BOX_ENTRY>& outEntries)
{
	outEntries.clear();
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind &&
			WORLD_PLACEMENT_KIND::COLLISION_BOX != placement.eKind)
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = placement.placementId;
		desc.position = placement.position;
		desc.halfExtents = placement.halfExtents;
		desc.yawDegrees = placement.yawDegrees;
		desc.isEnabled = placement.isEnabled;
		desc.isCollisionBox =
			WORLD_PLACEMENT_KIND::COLLISION_BOX == placement.eKind;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_TriggerBoxes"),
			&desc,
			&gameObject)))
		{
			Remove_WorldTriggerBoxes(outEntries);
			m_WorldGameplayStatus =
				"Trigger Box presentation stage failed: " + placement.placementId;
			return false;
		}

		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_TriggerBoxes"),
				gameObject);
			Remove_WorldTriggerBoxes(outEntries);
			m_WorldGameplayStatus =
				"Trigger Box clone type mismatch: " + placement.placementId;
			return false;
		}
		triggerBox->Set_Selected(
			m_SelectedWorldPlacementId == placement.placementId);
		triggerBox->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode);
		outEntries.push_back({ placement.placementId, std::move(triggerBox) });
	}
	return true;
}

void Client::CMapTool::Remove_WorldTriggerBoxes(
	vector<TRIGGER_BOX_ENTRY>& entries)
{
	for (const TRIGGER_BOX_ENTRY& entry : entries)
	{
		if (nullptr != entry.object &&
			m_iAuthoringLevelIndex < ETOUI(LEVEL::END))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_TriggerBoxes"),
				static_pointer_cast<CGameObject>(entry.object));
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_SpawnAnchors"),
				static_pointer_cast<CGameObject>(entry.object));
		}
	}
	entries.clear();
}

void Client::CMapTool::Update_WorldTriggerBoxPresentation(
	const bool_t isVisible)
{
	for (TRIGGER_BOX_ENTRY& entry : m_WorldTriggerBoxes)
	{
		if (nullptr == entry.object)
			continue;
		entry.object->Set_AuthoringVisible(isVisible);
		entry.object->Set_Selected(
			entry.placementId == m_SelectedWorldPlacementId);
	}
	for (TRIGGER_BOX_ENTRY& entry : m_SpawnAnchorBoxes)
	{
		if (nullptr == entry.object)
			continue;
		entry.object->Set_AuthoringVisible(isVisible);
		entry.object->Set_Selected(entry.placementId == m_SelectedSpawnAnchorId);
	}
	/* Destruction outlines follow their own mode instead of the gameplay
	   visibility flag the two loops above share. */
	const std::string pickedId =
		std::to_string(m_iSelectedDeployPlacementId);
	for (TRIGGER_BOX_ENTRY& entry : m_DestructionHighlightBoxes)
	{
		if (nullptr == entry.object)
			continue;
		entry.object->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode);
		entry.object->Set_Selected(entry.placementId == pickedId);
	}
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
				m_bSpawnAnchorPlacementArmed)) ||
		(TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode &&
			m_bDestructionPickArmed) ||
		PLACEMENT_STATE::ARMED == m_ePlacementState;
}

bool_t Client::CMapTool::Ensure_AuthoringPrototypes()
{
	return Ensure_AuthoringPrototypes(m_Catalog);
}

bool_t Client::CMapTool::Ensure_AuthoringPrototypes(
	const CMapAssetCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map authoring prototype admission is unavailable";
		return false;
	}

	const matrix_t modelTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	size_t admittedCount = 0;
	for (const MAP_ASSET_ENTRY& asset : catalog.Get_Entries())
	{
		const shared_ptr<CModel> existing =
			dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					m_iAuthoringLevelIndex,
					asset.prototypeTag));
		if (nullptr != existing)
		{
			const auto fingerprint =
				m_PrototypeModelPaths.find(asset.prototypeTag);
			if (fingerprint == m_PrototypeModelPaths.end() ||
				fingerprint->second.lexically_normal() !=
					asset.resolvedModelPath.lexically_normal())
			{
				m_Status =
					"Prototype tag already belongs to another model: " +
					asset.id;
				return false;
			}
			++admittedCount;
			continue;
		}

		auto model = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			asset.resolvedModelPath.string().c_str(),
			modelTransform);
		if (nullptr == model ||
			FAILED(CGameInstance::Get().Add_Prototype(
				m_iAuthoringLevelIndex,
				asset.prototypeTag,
				std::move(model))))
		{
			m_Status = "Map authoring model admission failed: " + asset.id;
			return false;
		}
		m_PrototypeModelPaths.emplace(
			asset.prototypeTag,
			asset.resolvedModelPath.lexically_normal());
		++admittedCount;
	}

	m_Status = "Map authoring admitted " +
		std::to_string(admittedCount) + " model prototypes";
	return admittedCount == catalog.Get_Entries().size();
}

bool_t Client::CMapTool::Ensure_DestructionDebrisAuthoringPrototypes()
{
	if (m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status =
			"PROJECT_AUTHORED debris model admission is unavailable";
		return false;
	}

	const auto& specs =
		CDestructionSimulationRuntime::Get_ProjectAuthoredDebrisModelSpecs();
	if (specs.empty())
	{
		m_Status = "PROJECT_AUTHORED debris model recipe is empty";
		return false;
	}

	std::unordered_set<wstring_t> uniqueTags;
	vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*> genericSpecs;
	vector<string> exactSourceAssetIds;
	for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& spec : specs)
	{
		if (spec.prototypeTag.empty() || spec.assetId.empty() ||
			!std::isfinite(spec.fUniformScale) || spec.fUniformScale <= 0.f ||
			!std::isfinite(spec.vSourceLocalPivotMeters.x) ||
			!std::isfinite(spec.vSourceLocalPivotMeters.y) ||
			!std::isfinite(spec.vSourceLocalPivotMeters.z) ||
			!uniqueTags.emplace(spec.prototypeTag).second)
		{
			m_Status =
				"PROJECT_AUTHORED debris model recipe is invalid or duplicated";
			return false;
		}
		if (spec.sourceDeployAssetId.empty())
		{
			genericSpecs.push_back(&spec);
		}
		else if (std::find(
			exactSourceAssetIds.begin(), exactSourceAssetIds.end(),
			spec.sourceDeployAssetId) == exactSourceAssetIds.end())
		{
			exactSourceAssetIds.push_back(spec.sourceDeployAssetId);
		}
	}
	if (genericSpecs.empty())
	{
		m_Status = "PROJECT_AUTHORED generic debris recipe is empty";
		return false;
	}

	auto admitBatch = [this](
		const vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>& batch,
		const bool_t required,
		const string& label,
		bool_t& outReady,
		string& outWarning)
	{
		outReady = false;
		vector<pair<wstring_t, unique_ptr<CPrototype>>> stagedPrototypes;
		vector<pair<wstring_t, std::filesystem::path>> stagedFingerprints;
		stagedPrototypes.reserve(batch.size());
		stagedFingerprints.reserve(batch.size());
		for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC* spec : batch)
		{
			const std::filesystem::path modelPath =
				CRuntimeAssetRoot::Resolve(spec->assetId).lexically_normal();
			std::error_code modelError;
			if (modelPath.empty() || !std::filesystem::is_regular_file(
				modelPath, modelError) || modelError)
			{
				const string message = label + " missing " + spec->assetId;
				if (required)
				{
					m_Status = message;
					return false;
				}
				outWarning = message;
				return true;
			}

			const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					m_iAuthoringLevelIndex, spec->prototypeTag));
			if (nullptr != existing)
			{
				const auto fingerprint =
					m_PrototypeModelPaths.find(spec->prototypeTag);
				if (fingerprint == m_PrototypeModelPaths.end() ||
					fingerprint->second.lexically_normal() != modelPath)
				{
					m_Status =
						"PROJECT_AUTHORED debris prototype tag collision: " +
						spec->assetId;
					return false;
				}
				continue;
			}

			const f32_t modelScale = 0.01f * spec->fUniformScale;
			auto model = CModel::Create(
				m_pDevice,
				m_pContext,
				MODEL::NONANIM,
				modelPath.string().c_str(),
				XMMatrixScaling(modelScale, modelScale, modelScale));
			if (nullptr == model)
			{
				const string message = label + " decode failed " + spec->assetId;
				if (required)
				{
					m_Status = message;
					return false;
				}
				outWarning = message;
				return true;
			}

			unique_ptr<CPrototype> prototype = std::move(model);
			stagedPrototypes.emplace_back(
				spec->prototypeTag, std::move(prototype));
			stagedFingerprints.emplace_back(spec->prototypeTag, modelPath);
		}

		if (!stagedPrototypes.empty() && FAILED(
			CGameInstance::Get().Add_Prototypes(
				m_iAuthoringLevelIndex, std::move(stagedPrototypes))))
		{
			const string message = label + " prototype batch commit failed";
			if (required)
			{
				m_Status = message;
				return false;
			}
			outWarning = message;
			return true;
		}
		for (const auto& [prototypeTag, modelPath] : stagedFingerprints)
			m_PrototypeModelPaths.emplace(prototypeTag, modelPath);
		outReady = true;
		return true;
	};

	bool_t genericReady = false;
	string warning;
	if (!admitBatch(
		genericSpecs, true, "Generic debris", genericReady, warning) ||
		!genericReady)
	{
		return false;
	}
	vector<string> unavailableExactRecipes;
	for (const string& sourceAssetId : exactSourceAssetIds)
	{
		vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*> exactSpecs;
		for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& spec : specs)
		{
			if (spec.sourceDeployAssetId == sourceAssetId)
				exactSpecs.push_back(&spec);
		}
		bool_t exactReady = false;
		warning.clear();
		if (!admitBatch(
			exactSpecs, false, "Exact debris " + sourceAssetId,
			exactReady, warning))
		{
			return false;
		}
		if (!exactReady)
			unavailableExactRecipes.push_back(warning);
	}

	m_Status = "PROJECT_AUTHORED generic debris models ready";
	if (unavailableExactRecipes.empty())
	{
		m_Status += "; exact wall recipes ready";
	}
	else
	{
		m_Status += "; exact wall recipe fallback: " +
			unavailableExactRecipes.front();
	}

	return true;
}

bool_t Client::CMapTool::Ensure_DeployAuthoringPrototypes(
	const CDeployPropCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "DeployProp authoring prototype admission is unavailable";
		return false;
	}

	const matrix_t modelTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	const auto admitModel = [this, &modelTransform](
		const std::wstring& prototypeTag,
		const std::filesystem::path& modelPath,
		const MODEL modelKind,
		const std::string& assetId)
	{
		const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex, prototypeTag));
		if (nullptr != existing)
		{
			const auto fingerprint = m_PrototypeModelPaths.find(prototypeTag);
			return fingerprint != m_PrototypeModelPaths.end() &&
				fingerprint->second.lexically_normal() ==
					modelPath.lexically_normal();
		}

		auto model = CModel::Create(
			m_pDevice,
			m_pContext,
			modelKind,
			modelPath.string().c_str(),
			modelTransform);
		if (nullptr == model || FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			prototypeTag,
			std::move(model))))
		{
			m_Status = "DeployProp model admission failed: " + assetId;
			return false;
		}
		m_PrototypeModelPaths.emplace(
			prototypeTag, modelPath.lexically_normal());
		return true;
	};

	for (const DEPLOY_PROP_ASSET_ENTRY& asset : catalog.Get_Assets())
	{
		const MODEL modelKind =
			DEPLOY_PROP_MODEL_KIND::ANIM == asset.kind ?
			MODEL::ANIM : MODEL::NONANIM;
		if (!admitModel(
			asset.intactPrototypeTag,
			asset.intactResolvedPath,
			modelKind,
			asset.id))
		{
			return false;
		}
		if (DEPLOY_PROP_MODEL_KIND::STATIC == asset.kind &&
			!admitModel(
				asset.fracturedPrototypeTag,
				asset.fracturedResolvedPath,
				MODEL::NONANIM,
				asset.id))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CMapTool::Load_EditorAreaRegistry()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"Maps/MapCatalog.json");
	std::string text;
	std::string parseError;
	DATA_JSON_VALUE root;
	if (path.empty() || !ReadTextFile(path, text) ||
		!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_Status = "Map editor catalog parse failed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areas = root.Find("areas");
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != "lostark.map-catalog" ||
		nullptr == version || !version->Is_Number() ||
		version->Get_Number() != 1.0 ||
		nullptr == areas || !areas->Is_Array())
	{
		m_Status = "Map editor catalog header is invalid";
		return false;
	}

	const std::array<std::pair<const char_t*, const char_t*>, 4> targets =
	{{
		{ "LV_LOBBY_CLASSSELECT_SL00", "Character Select" },
		{ "LV_BER_BERNCASTLE", "Bern" },
		{ "LV_LUT_HEARTRB_ED", "Valtan" },
		{ "LV_SHS_RCARENA_D", "Training Map" },
	}};
	std::vector<EDITOR_AREA_DESCRIPTOR> staged;
	staged.reserve(targets.size());
	for (const auto& target : targets)
	{
		const DATA_JSON_VALUE* selected = nullptr;
		for (const DATA_JSON_VALUE& candidate : areas->Get_Array())
		{
			const DATA_JSON_VALUE* id = candidate.Find("id");
			if (candidate.Is_Object() && nullptr != id && id->Is_String() &&
				id->Get_String() == target.first)
			{
				if (nullptr != selected)
				{
					m_Status = "Duplicate MapCatalog area: " +
						std::string(target.first);
					return false;
				}
				selected = &candidate;
			}
		}
		if (nullptr == selected)
		{
			m_Status = "MapCatalog area is missing: " +
				std::string(target.first);
			return false;
		}

		EDITOR_AREA_DESCRIPTOR descriptor;
		descriptor.areaId = target.first;
		descriptor.label = target.second;
		std::string sourceCatalog;
		std::string sourcePlacements;
		if (!ReadRequiredString(*selected, "sourceCatalog", sourceCatalog) ||
			!ReadRequiredString(*selected, "sourcePlacements", sourcePlacements))
		{
			m_Status = "MapCatalog authoring source is missing: " +
				descriptor.areaId;
			return false;
		}
		descriptor.sourceCatalog = ResolveDataCatalogPath(sourceCatalog);
		descriptor.sourcePlacements =
			ResolveDataCatalogPath(sourcePlacements);
		const DATA_JSON_VALUE* sourceLights = selected->Find("sourceLights");
		if (nullptr != sourceLights)
		{
			if (!sourceLights->Is_String() ||
				sourceLights->Get_String().empty())
			{
				m_Status = "MapCatalog light authoring source is invalid: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceLights = ResolveDataCatalogPath(
				sourceLights->Get_String());
			if (descriptor.sourceLights.empty())
			{
				m_Status = "MapCatalog light path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceDeployCatalog =
			selected->Find("sourceDeployCatalog");
		const DATA_JSON_VALUE* sourceDeployPlacements =
			selected->Find("sourceDeployPlacements");
		if ((nullptr == sourceDeployCatalog) !=
			(nullptr == sourceDeployPlacements) ||
			(nullptr != sourceDeployCatalog &&
				(!sourceDeployCatalog->Is_String() ||
					sourceDeployCatalog->Get_String().empty() ||
					!sourceDeployPlacements->Is_String() ||
					sourceDeployPlacements->Get_String().empty())))
		{
			m_Status = "MapCatalog deploy authoring pair is invalid: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceDeployCatalog)
		{
			descriptor.sourceDeployCatalog = ResolveDataCatalogPath(
				sourceDeployCatalog->Get_String());
			descriptor.sourceDeployPlacements = ResolveDataCatalogPath(
				sourceDeployPlacements->Get_String());
			if (descriptor.sourceDeployCatalog.empty() ||
				descriptor.sourceDeployPlacements.empty())
			{
				m_Status = "MapCatalog deploy path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			std::string source;
			std::string paint;
			if (!ReadRequiredString(*selected, "navigationSource", source) ||
				!ReadRequiredString(*selected, "navigationPaint", paint))
			{
				m_Status = "MapCatalog navigation source is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.navigationSource = ResolveDataCatalogPath(source);
			descriptor.navigationPaint = ResolveDataCatalogPath(paint);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT;
			descriptor.allowNavigationBootstrap =
				descriptor.areaId == "LV_BER_BERNCASTLE";
		}
		if (descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			if (descriptor.sourceLights.empty())
			{
				m_Status = "Valtan source light presentation is missing";
				return false;
			}
			std::string blockers;
			if (!ReadRequiredString(*selected, "navigationBlockers", blockers))
			{
				m_Status = "Valtan navigation blocker source is missing";
				return false;
			}
			descriptor.navigationBlockers = ResolveDataCatalogPath(blockers);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;
			/* The editor area registry already hardcodes which four areas
			   the Map Tool opens, so the destruction reference path is
			   declared the same way instead of adding a field to the shared
			   MapCatalog schema. Render_DestructionEncounterSource cross
			   checks the loaded encounterId against the boss placement of
			   this Area. */
			descriptor.encounterReference = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json");
			descriptor.worldEventsDocument = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanWorldEvents.json");
			descriptor.destructionSimulationDocument = CProjectDataRoot::Resolve(
				L"Maps/Authoring/LV_LUT_HEARTRB_ED/"
				L"LV_LUT_HEARTRB_ED.destructionsimulation.json");
			if (descriptor.encounterReference.empty() ||
				descriptor.worldEventsDocument.empty() ||
				descriptor.destructionSimulationDocument.empty())
			{
				m_Status = "Valtan encounter reference path is invalid";
				return false;
			}
		}

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			std::string gameplay;
			if (!ReadRequiredString(*selected, "gameplayDocument", gameplay))
			{
				m_Status = "MapCatalog gameplay document is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.gameplayDocument = ResolveDataCatalogPath(gameplay);
			descriptor.gameplayPolicy =
				EDITOR_GAMEPLAY_POLICY::REQUIRED;
		}

		if (descriptor.sourceCatalog.empty() ||
			descriptor.sourcePlacements.empty() ||
			((!descriptor.sourceDeployCatalog.empty() ||
				!descriptor.sourceDeployPlacements.empty()) &&
				(descriptor.sourceDeployCatalog.empty() ||
					descriptor.sourceDeployPlacements.empty())) ||
			(EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy &&
				(descriptor.navigationSource.empty() ||
					descriptor.navigationPaint.empty())) ||
			(EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy &&
				descriptor.gameplayDocument.empty()))
		{
			m_Status = "MapCatalog path escapes Data root: " +
				descriptor.areaId;
			return false;
		}
		staged.push_back(std::move(descriptor));
	}

	m_EditorAreas = std::move(staged);
	return true;
}

const Client::CMapTool::EDITOR_AREA_DESCRIPTOR*
Client::CMapTool::Get_ActiveEditorArea() const
{
	return m_iActiveEditorArea < m_EditorAreas.size() ?
		&m_EditorAreas[m_iActiveEditorArea] : nullptr;
}

bool_t Client::CMapTool::Has_UnsavedAuthoring() const
{
	return m_bDirty || m_bWorldGameplayDirty || m_bSpawnGroupsDirty ||
		m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty() ||
		m_DestructionDocument.Is_Dirty() ||
		m_DestructionSimulationDocument.Is_Dirty() ||
		m_bDestructionSimulationElementDraftDirty;
}

bool_t Client::CMapTool::Switch_EditorArea(const size_t descriptorIndex)
{
	if (descriptorIndex >= m_EditorAreas.size() ||
		m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active())
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> records;
	std::string stagedStatus;
	if (!CMapPlacementDocument::Read(
		descriptor.sourcePlacements,
		stagedCatalog,
		records,
		stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}

	CWorldGameplayDocument stagedWorld;
	CSpawnGroupDocument stagedSpawnGroups;
	if (EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy)
	{
		error.clear();
		if (!std::filesystem::is_regular_file(
			descriptor.gameplayDocument, error) || error ||
			!stagedWorld.Load(
				descriptor.gameplayDocument,
				descriptor.areaId,
				stagedStatus))
		{
			m_Status = error ?
				"Could not inspect required gameplay document" : stagedStatus;
			return false;
		}
		const std::filesystem::path spawnGroupsPath =
			descriptor.gameplayDocument.parent_path() / L"SpawnGroups.world.json";
		if (!stagedSpawnGroups.Load(
			spawnGroupsPath, descriptor.areaId, stagedStatus))
		{
			m_Status = stagedStatus;
			return false;
		}
	}

	CNavGridPaintDocument stagedNavigation;
	CNavRuntimeBlockerDocument stagedBlockers;
	bool_t navigationLoaded = false;
	if (EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy)
	{
		error.clear();
		const bool_t hasSource = std::filesystem::is_regular_file(
			descriptor.navigationSource, error);
		if (IsFileInspectionFailure(error) ||
			(!hasSource && !descriptor.allowNavigationBootstrap))
		{
			m_Status = "Required navigation source is missing: " +
				descriptor.areaId;
			return false;
		}
		if (hasSource)
		{
			if (!stagedNavigation.Load(
				descriptor.navigationSource,
				descriptor.navigationPaint,
				stagedStatus) ||
				stagedNavigation.Get_Desc().areaId != descriptor.areaId ||
				!stagedBlockers.Load(
					descriptor.navigationBlockers,
					stagedNavigation.Get_Desc(),
					stagedStatus))
			{
				m_Status = stagedStatus;
				return false;
			}
			navigationLoaded = true;
		}
	}

	/* Pure data document: an Area without a world events path simply has no
	   destruction authoring, a missing file for Valtan starts empty so the
	   first Save can create it, and a corrupt file fails the switch like the
	   other authoring documents do. */
	CWorldDestructionDocument stagedDestruction;
	CEncounterPatternReference stagedEncounterReference;
	std::string stagedEncounterStatus;
	if (!descriptor.worldEventsDocument.empty())
	{
		if (descriptor.encounterReference.empty() ||
			!stagedEncounterReference.Load(
				descriptor.encounterReference, stagedEncounterStatus))
		{
			m_Status = descriptor.encounterReference.empty() ?
				"World destruction requires an encounter reference" :
				stagedEncounterStatus;
			return false;
		}
		std::error_code destructionError;
		const bool_t hasDocument = std::filesystem::is_regular_file(
			descriptor.worldEventsDocument, destructionError);
		if (IsFileInspectionFailure(destructionError))
		{
			m_Status = "World destruction document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string destructionStatus;
		if (!hasDocument)
		{
			stagedDestruction.Reset_Empty();
		}
		else if (!stagedDestruction.Load(
			descriptor.worldEventsDocument,
			descriptor.areaId,
			"ENCOUNTER_VALTAN",
			destructionStatus))
		{
			m_Status = destructionStatus;
			return false;
		}
	}

	CDestructionSimulationDocument stagedSimulation;
	if (!descriptor.destructionSimulationDocument.empty())
	{
		std::error_code simulationError;
		const bool_t hasSimulation = std::filesystem::is_regular_file(
			descriptor.destructionSimulationDocument, simulationError);
		if (IsFileInspectionFailure(simulationError))
		{
			m_Status = "Destruction simulation document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string simulationStatus;
		if (!hasSimulation)
		{
			stagedSimulation.Reset_Empty();
		}
		else if (!stagedSimulation.Load(
			descriptor.destructionSimulationDocument,
			descriptor.areaId,
			simulationStatus))
		{
			m_Status = simulationStatus;
			return false;
		}
	}
	if (stagedSimulation.Is_Ready() && stagedDestruction.Is_Ready() &&
		!stagedSimulation.Validate_GroupReferences(
			stagedDestruction, stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	shared_ptr<CMapLightPresentationRuntime> stagedMapLightPresentation;
	if (!descriptor.sourceLights.empty())
	{
		stagedMapLightPresentation =
			make_shared<CMapLightPresentationRuntime>();
		if (!stagedMapLightPresentation->Load(
			descriptor.sourceLights, descriptor.areaId))
		{
			m_Status = stagedMapLightPresentation->Get_Status();
			return false;
		}
	}

	if (!Ensure_AuthoringPrototypes(stagedCatalog))
		return false;
	const bool_t stagedDebrisPrototypesReady =
		!descriptor.destructionSimulationDocument.empty() &&
		Ensure_DestructionDebrisAuthoringPrototypes();
	const std::string stagedDebrisPrototypeStatus =
		descriptor.destructionSimulationDocument.empty() ?
		"PROJECT_AUTHORED debris is not declared for this Area" : m_Status;

	CMapAssetCatalog previousCatalog = m_Catalog;
	m_Catalog = stagedCatalog;
	std::vector<PLACED_ENTRY> stagedPlacements;
	std::vector<STATIC_BATCH_ENTRY> stagedBatches;
	if (!Stage_PlacementRuntime(records, stagedPlacements, stagedBatches))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		m_Status = "Map Area runtime stage rolled back: " + descriptor.areaId;
		return false;
	}
	CDeployPropRuntime stagedDeployRuntime;
	if (!Stage_DeployProps(descriptor, stagedDeployRuntime))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		return false;
	}
	if (stagedDestruction.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			stagedDestruction,
			stagedDeployRuntime,
			stagedBlockers,
			stagedWorld,
			stagedEncounterReference,
			stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedTriggerBoxes;
	if (!Stage_WorldTriggerBoxes(stagedWorld, stagedTriggerBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedSpawnAnchorBoxes;
	if (!Stage_SpawnAnchorBoxes(stagedSpawnGroups, stagedSpawnAnchorBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		m_Catalog = std::move(previousCatalog);
		return false;
	}

	/* The staged runtime owns preview seams into the current Deploy runtime.
	   Release them before the Area transaction move-assigns that owner. */
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	Remove_PlacementRuntime(m_Placements, m_StaticBatches);
	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	m_Placements = std::move(stagedPlacements);
	m_StaticBatches = std::move(stagedBatches);
	m_DeployRuntime = std::move(stagedDeployRuntime);
	m_WorldGameplayDocument = std::move(stagedWorld);
	m_WorldTriggerBoxes = std::move(stagedTriggerBoxes);
	m_SpawnGroupDocument = std::move(stagedSpawnGroups);
	m_SpawnAnchorBoxes = std::move(stagedSpawnAnchorBoxes);
	m_NavigationDocument = std::move(stagedNavigation);
	m_RuntimeBlockerDocument = std::move(stagedBlockers);
	m_DestructionDocument = std::move(stagedDestruction);
	m_EncounterReference = std::move(stagedEncounterReference);
	m_DestructionSimulationDocument = std::move(stagedSimulation);
	m_pMapLightPresentation = std::move(stagedMapLightPresentation);
	m_bMapLightSubmissionFailureReported = false;
	m_WorldEventsPath = descriptor.worldEventsDocument;
	m_NavigationSourcePath = descriptor.navigationSource;
	m_NavigationPaintPath = descriptor.navigationPaint;
	m_RuntimeBlockerPath = descriptor.navigationBlockers;
	m_NavigationRuntimePath.clear();
	m_NavigationBakeDesc = navigationLoaded ?
		m_NavigationDocument.Get_BakeDesc() : NAVGRID_BAKE_DESC{};
	m_iActiveEditorArea = descriptorIndex;
	m_bDestructionDebrisPrototypesReady = stagedDebrisPrototypesReady;
	m_DestructionDebrisPrototypeStatus = stagedDebrisPrototypeStatus;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_iSelectedPlacementId = 0;
	m_SelectedWorldPlacementId.clear();
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_SelectedAssetId.clear();
	Remove_WorldTriggerBoxes(m_DestructionHighlightBoxes);
	m_SelectedDestructionGroupId.clear();
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionStageId.clear();
	m_SelectedDestructionPatternId.clear();
	m_iSelectedDeployPlacementId = 0;
	m_DestructionPreviewPreviousStates.clear();
	m_bDestructionPickArmed = false;
	m_bDestructionAddMemberArmed = false;
	m_bDestructionNewSettingArmed = false;
	m_bDestructionTimelinePlaying = false;
	m_fDestructionTimelineMs = 0.f;
	m_EncounterReferenceStatus = descriptor.encounterReference.empty() ?
		"Active Area declares no encounter reference" :
		stagedEncounterStatus;
	Reset_DestructionSimulationUI();
	if (!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		const DESTRUCTION_SIMULATION_PROFILE& firstProfile =
			m_DestructionSimulationDocument.Get_Profiles().front();
		m_SelectedDestructionGroupId = firstProfile.groupId;
		Select_DestructionSimulationProfile(firstProfile.profileId);
		Refresh_DestructionHighlight();
	}
	if (!m_bDestructionDebrisPrototypesReady &&
		!descriptor.destructionSimulationDocument.empty())
	{
		m_DestructionSimulationStatus = m_DestructionDebrisPrototypeStatus;
	}
	m_DestructionStatus = descriptor.worldEventsDocument.empty() ?
		"World destruction authoring disabled for this Area" :
		"World destruction authoring ready";
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		if ((entry.record.transformSource == "editor" ||
			entry.record.transformSource == "legacy") &&
			entry.record.placementId <=
				CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
		}
	}
	m_bDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_WorldGameplayStatus =
		EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy ?
		"Gameplay authoring ready" : "Gameplay authoring disabled for this Area";
	m_NavigationStatus =
		EDITOR_NAVIGATION_POLICY::NONE == descriptor.navigationPolicy ?
		"Navigation authoring disabled for this Area" :
		(navigationLoaded ? "Navigation authoring ready" :
			"Navigation bootstrap: place Nav Bounds and Bake");
	m_Status = "Active editor Area: " + descriptor.label + " (" +
		descriptor.areaId + ") / " + std::to_string(m_Placements.size()) +
		" placements. Runtime publish is separate.";
	Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);

	if (!Focus_ActiveEditorAreaCamera())
		m_Status += " Camera focus unavailable: " + m_CameraStatus;
	return true;
}

void Client::CMapTool::Handle_LevelTransition(
	uint32_t currentLevelIndex,
	bool_t isMapAuthoringLevel)
{
	const uint32_t targetLevelIndex = isMapAuthoringLevel ?
		currentLevelIndex : ETOUI(LEVEL::END);
	if (targetLevelIndex == m_iAuthoringLevelIndex)
		return;
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	Reset_DestructionSimulationUI();

	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldTriggerTargetPickArmed = false;
	m_bSpawnAnchorPlacementArmed = false;
	m_SelectedWorldPlacementId.clear();
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	m_pAssetTestCamera.reset();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->Reset_LevelResources();

	m_Placements.clear();
	m_StaticBatches.clear();
	m_DeployRuntime.Reset_ClearedLevelTracking();
	m_WorldTriggerBoxes.clear();
	m_SpawnAnchorBoxes.clear();
	m_iNextPlacementId = 1;
	m_bDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_SpawnGroupDocument.Reset();
	m_DestructionSimulationDocument.Clear();
	m_pMapLightPresentation.reset();
	m_bMapLightSubmissionFailureReported = false;
	m_Catalog = CMapAssetCatalog{};
	m_EditorAreas.clear();
	m_iActiveEditorArea = SIZE_MAX;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_isEditorExitPending = false;
	m_PrototypeModelPaths.clear();
	m_bDestructionDebrisPrototypesReady = false;
	m_DestructionDebrisPrototypeStatus =
		"PROJECT_AUTHORED debris models are not admitted";
	m_iAuthoringLevelIndex = targetLevelIndex;

	if (!isMapAuthoringLevel)
	{
		m_Status = "Current level has no map Area to author";
		m_WorldGameplayStatus = "Current level has no gameplay Area";
		m_NavigationStatus = "Current level has no navigation Area";
		m_CameraStatus = "Current level has no authoring camera";
		return;
	}

	Find_AssetTestCamera();
	if (!Load_EditorAreaRegistry() || m_EditorAreas.empty())
		return;
	Switch_EditorArea(0);
}
bool_t Client::CMapTool::Find_AssetTestCamera()
{
	const shared_ptr<CGameObject> gameObject =
		CGameInstance::Get().Get_GameObject(
			m_iAuthoringLevelIndex,
			TEXT("Layer_Camera"),
			0);
	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		m_pAssetTestCamera.reset();
		m_CameraStatus = "ASSET_TEST camera is unavailable";
		return false;
	}

	m_pAssetTestCamera = camera;
	m_CameraStatus = "Camera ready";
	return true;
}

bool_t Client::CMapTool::Focus_ActiveEditorAreaCamera()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CameraStatus = "Select an Area before focusing the camera";
		return false;
	}

	float3_t center{};
	f32_t radius = 0.f;
	bool_t hasFrame = false;
	/* Product spawn data is the stable authoring focus. Distant backdrop
	   meshes must not pull the editor camera away from the playable entry. */
	if ("LV_BER_BERNCASTLE" == descriptor->areaId)
	{
		hasFrame = TryBuildGameplaySpawnFrame(
			m_WorldGameplayDocument,
			"player.spawn.bern.entry",
			center,
			radius);
	}
	else if ("LV_LUT_HEARTRB_ED" == descriptor->areaId)
	{
		hasFrame = TryBuildGameplaySpawnFrame(
			m_WorldGameplayDocument,
			"player.spawn.valtan.entry",
			center,
			radius);
	}

	if (!hasFrame && !m_Placements.empty())
	{
		float3_t minimum = m_Placements.front().record.position;
		float3_t maximum = minimum;
		for (const PLACED_ENTRY& entry : m_Placements)
		{
			minimum.x = (std::min)(minimum.x, entry.record.position.x);
			minimum.y = (std::min)(minimum.y, entry.record.position.y);
			minimum.z = (std::min)(minimum.z, entry.record.position.z);
			maximum.x = (std::max)(maximum.x, entry.record.position.x);
			maximum.y = (std::max)(maximum.y, entry.record.position.y);
			maximum.z = (std::max)(maximum.z, entry.record.position.z);
		}
		center = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		radius = (std::max)(50.f,
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z) * 0.35f);
		hasFrame = true;
	}
	if (!hasFrame)
	{
		m_CameraStatus = "Active Area has no valid camera focus target";
		return false;
	}

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	camera->Frame_Area(center, radius);
	m_CameraStatus = "Camera focused on " + descriptor->label;
	return true;
}

bool_t Client::CMapTool::Load_NavigationDocument()
{
	if (CMapEditorWorkspaceService::Is_Active())
	{
		const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
		if (nullptr == active ||
			EDITOR_NAVIGATION_POLICY::NONE == active->navigationPolicy)
		{
			m_NavigationDocument = CNavGridPaintDocument{};
			m_RuntimeBlockerDocument = CNavRuntimeBlockerDocument{};
			m_NavigationStatus =
				"Navigation authoring disabled for this Area";
			return true;
		}

		std::error_code sourceError;
		const bool_t hasSource = std::filesystem::is_regular_file(
			active->navigationSource, sourceError);
		if (IsFileInspectionFailure(sourceError) ||
			(!hasSource && !active->allowNavigationBootstrap))
		{
			m_NavigationStatus =
				"Required navigation source is missing: " + active->areaId;
			return false;
		}

		m_NavigationSourcePath = active->navigationSource;
		m_NavigationPaintPath = active->navigationPaint;
		m_RuntimeBlockerPath = active->navigationBlockers;
		m_NavigationRuntimePath.clear();
		if (!hasSource)
		{
			m_NavigationDocument = CNavGridPaintDocument{};
			m_RuntimeBlockerDocument = CNavRuntimeBlockerDocument{};
			m_NavigationBakeDesc = NAVGRID_BAKE_DESC{};
			m_NavigationStatus =
				"Navigation bootstrap: place Nav Bounds and Bake";
			return false;
		}

		std::string status;
		CNavGridPaintDocument stagedNavigation;
		CNavRuntimeBlockerDocument stagedBlockers;
		if (!stagedNavigation.Load(
			active->navigationSource,
			active->navigationPaint,
			status) ||
			stagedNavigation.Get_Desc().areaId != active->areaId ||
			!stagedBlockers.Load(
				active->navigationBlockers,
				stagedNavigation.Get_Desc(),
				status))
		{
			m_NavigationStatus = status;
			return false;
		}
		if (m_DestructionDocument.Is_Ready())
		{
			if (!Validate_DestructionExternalReferences(
				m_DestructionDocument,
				m_DeployRuntime,
				stagedBlockers,
				m_WorldGameplayDocument,
				m_EncounterReference,
				status))
			{
				m_NavigationStatus = status;
				return false;
			}
		}

		m_NavigationDocument = std::move(stagedNavigation);
		m_RuntimeBlockerDocument = std::move(stagedBlockers);
		m_NavigationBakeDesc = m_NavigationDocument.Get_BakeDesc();
		m_iSelectedRuntimeRegion = 0;
		m_NavigationStatus = "Navigation authoring ready";
		return true;
	}

	MAP_NAVIGATION_CONTRACT stagedContract;
	std::string stagedStatus;
	if (!CMapNavigationContract::Resolve_Area(
		m_Catalog.Get_AreaId(), stagedContract, stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}

	std::error_code sourceError;
	const bool_t hasSource = std::filesystem::is_regular_file(
		stagedContract.sourcePath, sourceError);
	if (IsFileInspectionFailure(sourceError))
	{
		m_NavigationStatus =
			"Could not inspect navigation source for " +
			stagedContract.areaId;
		return false;
	}
	if (!hasSource)
	{
		CNavGridPaintDocument stagedNavigationDocument;
		CNavRuntimeBlockerDocument stagedBlockerDocument;
		NAVGRID_BAKE_DESC stagedBakeDesc;

		m_NavigationSourcePath = stagedContract.sourcePath;
		m_NavigationPaintPath = stagedContract.paintPath;
		m_NavigationRuntimePath = stagedContract.runtimePath;
		m_RuntimeBlockerPath = stagedContract.blockerPath;
		m_NavigationDocument = std::move(stagedNavigationDocument);
		m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
		m_NavigationBakeDesc = stagedBakeDesc;
		m_iSelectedRuntimeRegion = 0;
		m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
		m_bNavigationStrokeActive = false;
		m_NavigationStatus =
			"Navigation bootstrap: place Nav Bounds and Bake for " +
			stagedContract.areaId;
		m_NavigationBakeStatus = "Create Nav Bounds";
		m_bNavigationBakeResetConfirmed = false;
		m_bNavigationBakeResetPending = false;
		return false;
	}

	CNavGridPaintDocument stagedNavigationDocument;
	if (!stagedNavigationDocument.Load(
		stagedContract.sourcePath,
		stagedContract.paintPath,
		stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}
	if (stagedNavigationDocument.Get_Desc().areaId !=
		stagedContract.areaId)
	{
		m_NavigationStatus =
			"NavGrid source area does not match active map area";
		return false;
	}

	CNavRuntimeBlockerDocument stagedBlockerDocument;
	if (!stagedBlockerDocument.Load(
		stagedContract.blockerPath,
		stagedNavigationDocument.Get_Desc(),
		stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			m_DestructionDocument,
			m_DeployRuntime,
			stagedBlockerDocument,
			m_WorldGameplayDocument,
			m_EncounterReference,
			stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}

	size_t stagedSelectedRuntimeRegion = m_iSelectedRuntimeRegion;
	if (stagedSelectedRuntimeRegion >=
		stagedBlockerDocument.Get_RegionCount())
	{
		stagedSelectedRuntimeRegion = 0;
	}
	const NAVGRID_BAKE_DESC stagedBakeDesc =
		stagedNavigationDocument.Get_BakeDesc();
	stagedStatus = stagedContract.runtimeGridAvailable ?
		"Saved" :
		"Authoring ready; save and re-enter ASSET_TEST for runtime navigation";

	m_NavigationSourcePath = stagedContract.sourcePath;
	m_NavigationPaintPath = stagedContract.paintPath;
	m_NavigationRuntimePath = stagedContract.runtimePath;
	m_RuntimeBlockerPath = stagedContract.blockerPath;
	m_NavigationDocument = std::move(stagedNavigationDocument);
	m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
	m_NavigationBakeDesc = stagedBakeDesc;
	m_iSelectedRuntimeRegion = stagedSelectedRuntimeRegion;
	m_NavigationStatus = stagedStatus;
	m_NavigationBakeStatus = "Baked source loaded";
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
	m_bNavigationStrokeActive = false;

	return !stagedContract.runtimeGridAvailable ||
		Register_RuntimeBlockers();
}

bool_t Client::CMapTool::Load_RuntimeBlockers()
{
	if (!m_NavigationDocument.Is_Ready())
	{
		m_NavigationStatus = "Load NavGrid source before runtime blockers";
		return false;
	}

	CNavRuntimeBlockerDocument stagedBlockerDocument;
	std::string stagedStatus;
	if (!stagedBlockerDocument.Load(
		m_RuntimeBlockerPath,
		m_NavigationDocument.Get_Desc(),
		stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}

	std::error_code runtimeError;
	const bool_t hasRuntime = std::filesystem::is_regular_file(
		m_NavigationRuntimePath, runtimeError);
	if (runtimeError)
	{
		m_NavigationStatus = "Could not inspect navigation runtime";
		return false;
	}
	size_t stagedSelectedRuntimeRegion = m_iSelectedRuntimeRegion;
	if (stagedSelectedRuntimeRegion >=
		stagedBlockerDocument.Get_RegionCount())
	{
		stagedSelectedRuntimeRegion = 0;
	}
	if (!hasRuntime)
	{
		m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
		m_iSelectedRuntimeRegion = stagedSelectedRuntimeRegion;
		m_NavigationStatus =
			"Authoring ready; save and re-enter ASSET_TEST for runtime navigation";
		return true;
	}

	m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
	m_iSelectedRuntimeRegion = stagedSelectedRuntimeRegion;
	m_NavigationStatus = stagedStatus;

	return Register_RuntimeBlockers();
}

bool_t Client::CMapTool::Register_RuntimeBlockers()
{
	if (CMapEditorWorkspaceService::Is_Active())
	{
		m_NavigationStatus =
			"Runtime blocker registration is disabled in authoring workspace";
		return false;
	}

	shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			CGameInstance::Get().Get_Component(
				m_iAuthoringLevelIndex,
				TEXT("Layer_Player"),
				TEXT("Com_Navigation"),
				0));
	if (nullptr == navigation)
	{
		m_NavigationStatus =
			"Character Navigation component is unavailable";
		return false;
	}

	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region)
			return false;

		const vector<uint32_t> cells =
			m_RuntimeBlockerDocument.Get_CellIndices(index);
		if (cells.empty())
			continue;

		const auto condition =
			m_NavigationConditions.find(region->conditionId);
		const bool_t conditionValue =
			condition != m_NavigationConditions.end() ?
			condition->second :
			false;
		const bool_t initiallyActive =
			conditionValue == region->activateWhenConditionTrue;
		if (!navigation->Register_RuntimeBlocker(
			region->id,
			cells,
			initiallyActive))
		{
			m_NavigationStatus =
				"Failed to register runtime blocker: " +
				region->id +
				"; re-enter AssetTest after editing region cells";
			return false;
		}
	}

	m_NavigationStatus =
		"Navigation ready: " +
		std::to_string(m_RuntimeBlockerDocument.Get_RegionCount()) +
		" runtime blocker regions";
	return true;
}

bool_t Client::CMapTool::Set_NavigationCondition(
	const std::string& conditionId,
	bool_t value)
{
	if (conditionId.empty())
		return false;

	m_NavigationConditions[conditionId] = value;
	if (CMapEditorWorkspaceService::Is_Active())
	{
		m_NavigationStatus =
			"Authoring preview condition changed; runtime Navigation was not mutated";
		return true;
	}
	shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			CGameInstance::Get().Get_Component(
				m_iAuthoringLevelIndex,
				TEXT("Layer_Player"),
				TEXT("Com_Navigation"),
				0));
	if (nullptr == navigation)
		return false;

	bool_t succeeded = true;
	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region ||
			region->conditionId != conditionId ||
			0 == m_RuntimeBlockerDocument.Get_RegionCellCount(index))
		{
			continue;
		}

		const bool_t active =
			value == region->activateWhenConditionTrue;
		succeeded =
			navigation->Set_RuntimeBlockerActive(region->id, active) &&
			succeeded;
	}
	return succeeded;
}

bool_t Client::CMapTool::Save_Navigation()
{
	if (CMapEditorWorkspaceService::Is_Active())
	{
		const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
		if (nullptr == active ||
			EDITOR_NAVIGATION_POLICY::NONE == active->navigationPolicy ||
			!m_NavigationDocument.Is_Ready())
		{
			m_NavigationStatus = "Navigation authoring is unavailable";
			return false;
		}
		if (!HasSameNavigationPath(
			m_NavigationSourcePath, active->navigationSource) ||
			!HasSameNavigationPath(
				m_NavigationPaintPath, active->navigationPaint) ||
			!HasSameNavigationPath(
				m_RuntimeBlockerPath, active->navigationBlockers))
		{
			m_NavigationStatus =
				"Navigation paths do not match the immutable active Area";
			return false;
		}
		if (m_DestructionDocument.Is_Ready())
		{
			std::string destructionStatus;
			if (!Validate_CurrentDestructionReferences(destructionStatus))
			{
				m_NavigationStatus = destructionStatus;
				m_DestructionStatus = destructionStatus;
				return false;
			}
		}

		std::string status;
		if (!m_NavigationDocument.Save_Paint(
			active->navigationPaint, status))
		{
			m_NavigationStatus = status;
			return false;
		}
		if (EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS ==
				active->navigationPolicy &&
			!m_RuntimeBlockerDocument.Save(
				active->navigationBlockers, status))
		{
			m_NavigationStatus = status;
			return false;
		}
		m_NavigationStatus =
			"Saved Data authoring only; publisher must build runtime navigation";
		return true;
	}

	m_NavigationStatus =
		"Navigation save is only available in the Map Editor workspace";
	return false;
}

bool_t Client::CMapTool::Try_PickNavigationCell(
	int32_t& outCellX,
	int32_t& outCellZ) const
{
	if (!m_NavigationDocument.Is_Ready())
		return false;

	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) ||
		!m_NavigationDocument.World_ToCell(
			XMLoadFloat4(&picked),
			outCellX,
			outCellZ))
	{
		return false;
	}

	return m_NavigationDocument.Has_ResolvedHeight(
		m_NavigationDocument.To_Index(
			outCellX,
			outCellZ));
}

bool_t Client::CMapTool::Try_PaintNavigation()
{
	int32_t cellX = {};
	int32_t cellZ = {};
	if (!Try_PickNavigationCell(cellX, cellZ))
		return false;

	bool_t changed = false;
	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		const bool_t erase =
			NAVIGATION_EDIT_ACTION::ERASE ==
			m_eNavigationEditAction;
		changed = m_RuntimeBlockerDocument.Paint(
			m_iSelectedRuntimeRegion,
			cellX,
			cellZ,
			m_iBrushRadius,
			!erase);
	}
	else
	{
		NAVGRID_PAINT_OVERRIDE overrideState =
			NAVGRID_PAINT_OVERRIDE::INHERIT;
		switch (m_eNavigationEditAction)
		{
		case NAVIGATION_EDIT_ACTION::APPLY:
			overrideState =
				NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED;
			break;
		case NAVIGATION_EDIT_ACTION::FORCE_WALKABLE:
			overrideState =
				NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE;
			break;
		case NAVIGATION_EDIT_ACTION::ERASE:
		default:
			overrideState = NAVGRID_PAINT_OVERRIDE::INHERIT;
			break;
		}

		changed = m_NavigationDocument.Paint(
			cellX,
			cellZ,
			m_iBrushRadius,
			overrideState);
	}

	if (changed)
		m_NavigationStatus = "Unsaved";
	return changed;
}

bool_t Client::CMapTool::Try_PickPlacementPosition(float3_t& outPosition) const
{
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked))
		return false;

	/* Target_PickPos is written by the first depth-tested rendered triangle at
	   the cursor. Do not invent a Y=0 fallback when the view ray misses. */
	outPosition = float3_t(picked.x, picked.y, picked.z);
	return IsFinite(outPosition);
}

bool_t Client::CMapTool::Try_PlaceSelected()
{
	const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
	if (nullptr == pAsset)
	{
		m_Status = "Select an asset before placing";
		return false;
	}

	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_Status = "No valid surface under the cursor";
		return false;
	}

	const uint64_t placementId = Allocate_EditorPlacementId();
	if (0 == placementId)
	{
		m_Status = "No editor placement ID is available";
		return false;
	}

	MAP_PLACEMENT_RECORD record{};
	record.placementId = placementId;
	record.sourcePlacementId = "editor:" + m_Catalog.Get_AreaId() +
		":" + std::to_string(placementId);
	record.sourceLevel = "EDITOR";
	record.transformSource = "editor";
	record.assetId = pAsset->id;
	record.position = position;
	record.rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	record.signedScale = pAsset->defaultScale;
	record.visible = true;

	PLACED_ENTRY placed{};
	if (!Create_Placement(record, placed))
	{
		m_Status = "Failed to clone map object for " + pAsset->id;
		return false;
	}

	m_iSelectedPlacementId = placementId;
	m_Placements.push_back(std::move(placed));
	m_bDirty = true;
	m_Status = "Placed " + pAsset->label +
		"; placement remains armed (Esc cancels).";
	return true;
}

uint64_t Client::CMapTool::Allocate_EditorPlacementId()
{
	for (size_t attempt = 0; attempt <= m_Placements.size(); ++attempt)
	{
		if (0 == m_iNextPlacementId ||
			m_iNextPlacementId > CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
			m_iNextPlacementId = 1;

		const uint64_t candidate = m_iNextPlacementId++;
		if (nullptr == Find_Placement(candidate))
			return candidate;
	}

	return 0;
}

std::wstring Client::CMapTool::Make_LayerTag(
	const std::string& sourceLevel) const
{
	return CMapPlacementRuntime::Make_LayerTag(sourceLevel);
}

bool_t Client::CMapTool::Create_Placement(
	const MAP_PLACEMENT_RECORD& record, PLACED_ENTRY& outEntry)
{
	return CMapPlacementRuntime::Create_Placement(
		m_iAuthoringLevelIndex, m_Catalog, record, outEntry);
}

bool_t Client::CMapTool::Stage_PlacementRuntime(
	const vector<MAP_PLACEMENT_RECORD>& records,
	vector<PLACED_ENTRY>& outPlacements,
	vector<STATIC_BATCH_ENTRY>& outBatches)
{
	return CMapPlacementRuntime::Stage_PlacementRuntime(
		m_iAuthoringLevelIndex,
		m_Catalog,
		records,
		outPlacements,
		outBatches);
}

void Client::CMapTool::Remove_PlacementRuntime(
	vector<PLACED_ENTRY>& placements,
	vector<STATIC_BATCH_ENTRY>& batches)
{
	CMapPlacementRuntime::Remove_PlacementRuntime(
		m_iAuthoringLevelIndex, placements, batches);
}

bool_t Client::CMapTool::Set_RuntimeVisible(
	PLACED_ENTRY& entry, bool_t visible)
{
	return CMapPlacementRuntime::Set_RuntimeVisible(entry, visible);
}

bool_t Client::CMapTool::Remove_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	if (iter == m_Placements.end())
		return false;

	if (nullptr != iter->object)
	{
		if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iAuthoringLevelIndex, iter->layerTag,
			static_pointer_cast<CGameObject>(iter->object))))
			return false;
	}
	else if (nullptr != iter->batch)
	{
		if (FAILED(iter->batch->Set_InstanceVisible(placementId, false)))
			return false;
	}
	else
		return false;

	m_Placements.erase(iter);
	if (m_iSelectedPlacementId == placementId)
		m_iSelectedPlacementId = 0;
	m_bDirty = true;
	return true;
}

void Client::CMapTool::Remove_AllPlacements()
{
	Remove_PlacementRuntime(m_Placements, m_StaticBatches);
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	m_bDirty = true;
}

bool_t Client::CMapTool::Save_Placements()
{
	vector<MAP_PLACEMENT_RECORD> document;
	document.reserve(m_Placements.size());
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		const bool_t hasObject = nullptr != entry.object;
		const bool_t hasBatch = nullptr != entry.batch;
		if (hasObject == hasBatch ||
			nullptr == m_Catalog.Find(entry.record.assetId))
		{
			m_Status = "Save aborted: runtime representation is invalid";
			return false;
		}

		MAP_PLACEMENT_RECORD stored = entry.record;
		if (entry.record.sourceLevel.starts_with("VALTAN_PHASE_"))
			stored.visible = false;
		if (!CMapPlacementDocument::Is_Valid(stored, m_Catalog))
		{
			m_Status = "Save aborted: a transform is invalid";
			return false;
		}
		document.push_back(std::move(stored));
	}

	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const std::filesystem::path authoringPath = nullptr != active ?
		active->sourcePlacements : std::filesystem::path{};
	if (authoringPath.empty() ||
		!CMapPlacementDocument::Write(
		authoringPath, m_Catalog.Get_AreaId(),
		document, m_Catalog, m_Status))
		return false;

	m_bDirty = false;
	m_Status += " (authoring only; publish step required)";
	return true;
}

bool_t Client::CMapTool::Load_Placements()
{
	if (!m_Catalog.Is_Ready())
		return false;

	vector<MAP_PLACEMENT_RECORD> document;
	std::string loadStatus;
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const std::filesystem::path authoringPath = nullptr != active ?
		active->sourcePlacements : std::filesystem::path{};
	std::error_code authoringError;
	const bool_t hasAuthoring =
		!authoringPath.empty() &&
		std::filesystem::is_regular_file(
			authoringPath, authoringError);
	if (authoringError || !hasAuthoring ||
		!CMapPlacementDocument::Read(
			authoringPath, m_Catalog, document, loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}

	vector<PLACED_ENTRY> stagedPlacements;
	vector<STATIC_BATCH_ENTRY> stagedBatches;
	if (!Stage_PlacementRuntime(
		document, stagedPlacements, stagedBatches))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Status = "Map runtime staging rolled back";
		return false;
	}

	Remove_PlacementRuntime(m_Placements, m_StaticBatches);
	m_Placements = std::move(stagedPlacements);
	m_StaticBatches = std::move(stagedBatches);
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		if ((entry.record.transformSource == "editor" ||
			entry.record.transformSource == "legacy") &&
			entry.record.placementId <=
			CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
		}
	}
	m_bDirty = false;
	Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
	const size_t fallbackCount = static_cast<size_t>(std::count_if(
		m_Placements.begin(), m_Placements.end(),
		[](const PLACED_ENTRY& entry)
		{
			return nullptr != entry.object;
		}));
	m_Status = "Loaded " + std::to_string(m_Placements.size()) +
		" placements / " + std::to_string(m_StaticBatches.size()) +
		" batches / " + std::to_string(fallbackCount) + " fallbacks";
	return true;
}

bool_t Client::CMapTool::Load_DeployProps()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_Status = "DeployProp reload requires an active editor Area";
		return false;
	}

	CDeployPropRuntime stagedRuntime;
	if (!Stage_DeployProps(*descriptor, stagedRuntime))
		return false;
	m_DeployRuntime = std::move(stagedRuntime);
	m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
	m_Status = "Loaded " + std::to_string(m_Placements.size()) +
		" map placements + " +
		std::to_string(m_DeployRuntime.Get_Entries().size()) +
		" gameplay DeployProps";
	return true;
}

bool_t Client::CMapTool::Stage_DeployProps(
	const EDITOR_AREA_DESCRIPTOR& descriptor,
	CDeployPropRuntime& outRuntime)
{
	if (descriptor.sourceDeployCatalog.empty() &&
		descriptor.sourceDeployPlacements.empty())
	{
		return true;
	}
	if (descriptor.sourceDeployCatalog.empty() ||
		descriptor.sourceDeployPlacements.empty())
	{
		m_Status = "DeployProp authoring file pair is incomplete: " +
			descriptor.areaId;
		return false;
	}

	std::error_code catalogError;
	std::error_code placementError;
	const bool_t catalogExists = std::filesystem::is_regular_file(
		descriptor.sourceDeployCatalog, catalogError);
	const bool_t placementExists = std::filesystem::is_regular_file(
		descriptor.sourceDeployPlacements, placementError);
	if (catalogError || placementError || !catalogExists || !placementExists)
	{
		m_Status = "DeployProp authoring source pair is unavailable: " +
			descriptor.areaId;
		return false;
	}

	CDeployPropCatalog catalog;
	if (!catalog.Load(
		descriptor.sourceDeployCatalog,
		descriptor.sourceDeployPlacements,
		descriptor.areaId))
	{
		m_Status = catalog.Get_Status();
		return false;
	}
	if (!Ensure_DeployAuthoringPrototypes(catalog))
		return false;
	if (!outRuntime.Load(m_iAuthoringLevelIndex, std::move(catalog)))
	{
		m_Status = outRuntime.Get_Status();
		return false;
	}
	return true;
}

void Client::CMapTool::Remove_DeployProps()
{
	m_DeployRuntime.Clear();
}

void Client::CMapTool::Set_DeployPhase(DEPLOY_PROP_STATE state)
{
	if (!m_DeployRuntime.Set_State_All(state))
	{
		m_Status = m_DeployRuntime.Get_Status();
		return;
	}
	m_DeployPhase = state;

	const bool_t arenaDestroyed =
		DEPLOY_PROP_STATE::INTACT != state;
	if (!Set_NavigationCondition(
		"VALTAN_ARENA_DESTROYED",
		arenaDestroyed) &&
		0 != m_RuntimeBlockerDocument.Get_RegionCount())
	{
		m_NavigationStatus =
			"Deploy visual state changed, but Nav blocker condition failed";
	}
}

void Client::CMapTool::Set_EnvironmentPhase(ENVIRONMENT_PHASE phase)
{
	m_EnvironmentPhase = phase;
	for (PLACED_ENTRY& entry : m_Placements)
	{
		bool_t visible = entry.record.visible;
		if (entry.record.sourceLevel == "VALTAN_PHASE_SPACEHOLE")
			visible = phase != ENVIRONMENT_PHASE::BASELINE;
		else if (entry.record.sourceLevel == "VALTAN_PHASE_CHAOSGATE")
			visible = phase == ENVIRONMENT_PHASE::CHAOS_GATE;
		if (!m_bShowBernLandscape &&
			IsBernLandscapePlacement(m_Catalog, entry.record))
		{
			visible = false;
		}
		Set_RuntimeVisible(entry, visible);
	}
}

std::filesystem::path Client::CMapTool::Get_WorldDestructionPath() const
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	return nullptr != descriptor ? descriptor->worldEventsDocument :
		std::filesystem::path{};
}

bool_t Client::CMapTool::Load_WorldDestruction()
{
	const std::filesystem::path path = Get_WorldDestructionPath();
	if (path.empty())
	{
		m_DestructionStatus = "This Area has no world destruction document";
		return false;
	}
	/* The document only exists after the first Save, so a missing file is a
	   normal starting state and not a read failure. */
	std::error_code inspectError;
	if (!std::filesystem::is_regular_file(path, inspectError))
	{
		m_DestructionStatus = IsFileInspectionFailure(inspectError) ?
			"World events document is unreadable" :
			"No world events document yet. Create a group and press Save.";
		return false;
	}
	CWorldDestructionDocument staged;
	std::string status;
	if (!staged.Load(path, m_Catalog.Get_AreaId(), "ENCOUNTER_VALTAN", status))
	{
		m_DestructionStatus = status;
		return false;
	}
	if (!Validate_DestructionExternalReferences(
		staged,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		m_EncounterReference,
		status))
	{
		m_DestructionStatus = status;
		return false;
	}
	if (m_DestructionSimulationDocument.Is_Ready() &&
		!m_DestructionSimulationDocument.Validate_GroupReferences(
			staged, status))
	{
		m_DestructionStatus = status;
		return false;
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	const std::string previousGroupId = m_SelectedDestructionGroupId;
	const std::string previousProfileId =
		m_SelectedDestructionSimulationProfileId;
	m_DestructionDocument = std::move(staged);
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionStageId.clear();
	Reset_DestructionSimulationUI();
	m_bDestructionSimulationClearRequested = false;
	const DESTRUCTION_GROUP* selectedGroup = previousGroupId.empty() ? nullptr :
		m_DestructionDocument.Find_Group(previousGroupId);
	const DESTRUCTION_SIMULATION_PROFILE* selectedProfile =
		previousProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(previousProfileId);
	if (nullptr != selectedProfile && nullptr != selectedGroup &&
		selectedProfile->groupId != selectedGroup->groupId)
	{
		selectedProfile = nullptr;
	}
	if (nullptr == selectedProfile && nullptr != selectedGroup)
	{
		const auto found = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[selectedGroup](const DESTRUCTION_SIMULATION_PROFILE& profile)
			{
				return profile.groupId == selectedGroup->groupId;
			});
		if (found != m_DestructionSimulationDocument.Get_Profiles().end())
			selectedProfile = &*found;
	}

	if (nullptr != selectedGroup)
		m_SelectedDestructionGroupId = selectedGroup->groupId;
	else if (nullptr == selectedProfile &&
		!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		selectedProfile =
			&m_DestructionSimulationDocument.Get_Profiles().front();
	}
	else if (nullptr == selectedProfile)
		m_SelectedDestructionGroupId.clear();

	if (nullptr != selectedProfile)
		Select_DestructionSimulationProfile(selectedProfile->profileId);
	Refresh_DestructionHighlight();
	m_DestructionStatus = status;
	return true;
}

bool_t Client::CMapTool::Save_WorldDestruction()
{
	return Save_DestructionAuthoringPair();
}

bool_t Client::CMapTool::Save_DestructionAuthoringPair()
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		const std::string status =
			"Apply or Revert the debris Detail draft before saving";
		m_DestructionStatus = status;
		m_DestructionSimulationStatus = status;
		return false;
	}
	if (m_bWorldGameplayDirty || m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty())
	{
		m_DestructionStatus =
			"Save World Gameplay and Navigation first, or use Save All";
		return false;
	}
	std::string status;
	if (!Validate_DestructionExternalReferences(
		m_DestructionDocument,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		m_EncounterReference,
		status))
	{
		m_DestructionStatus = status;
		return false;
	}
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	const std::filesystem::path destructionPath =
		Get_WorldDestructionPath();
	const std::filesystem::path simulationPath =
		Get_DestructionSimulationPath();
	if (nullptr == descriptor || destructionPath.empty() ||
		simulationPath.empty() || !m_DestructionDocument.Is_Ready() ||
		!m_DestructionSimulationDocument.Is_Ready())
	{
		const std::string status =
			"Destruction pair save requires a ready Valtan Area";
		m_DestructionStatus = status;
		m_DestructionSimulationStatus = status;
		return false;
	}

	if (!CDestructionSimulationDocument::Save_AuthoringPair(
		m_DestructionDocument, destructionPath,
		m_DestructionSimulationDocument, simulationPath,
		descriptor->areaId, "ENCOUNTER_VALTAN", status))
	{
		m_DestructionStatus = status;
		m_DestructionSimulationStatus = status;
		return false;
	}

	m_DestructionDocument.Clear_Dirty();
	m_DestructionSimulationDocument.Clear_Dirty();
	m_DestructionStatus = status;
	m_DestructionSimulationStatus = status;
	return true;
}

std::filesystem::path Client::CMapTool::Get_DestructionSimulationPath() const
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	return nullptr != descriptor ? descriptor->destructionSimulationDocument :
		std::filesystem::path{};
}

bool_t Client::CMapTool::Load_DestructionSimulation()
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before reloading";
		return false;
	}

	const std::string selectedGroupId = m_SelectedDestructionGroupId;
	const std::string selectedProfileId =
		m_SelectedDestructionSimulationProfileId;

	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	const std::filesystem::path path = Get_DestructionSimulationPath();
	if (nullptr == descriptor || path.empty())
	{
		m_DestructionSimulationStatus =
			"This Area has no destruction simulation document";
		return false;
	}

	CDestructionSimulationDocument staged;
	std::error_code inspectError;
	const bool_t exists = std::filesystem::is_regular_file(path, inspectError);
	if (IsFileInspectionFailure(inspectError))
	{
		m_DestructionSimulationStatus =
			"Destruction simulation document is unreadable";
		return false;
	}

	std::string status;
	if (!exists)
	{
		staged.Reset_Empty();
		status = "No simulation document yet; create a default profile";
	}
	else if (!staged.Load(path, descriptor->areaId, status))
	{
		m_DestructionSimulationStatus = status;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		!staged.Validate_GroupReferences(m_DestructionDocument, status))
	{
		m_DestructionSimulationStatus = status;
		return false;
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	m_DestructionSimulationDocument = std::move(staged);
	Reset_DestructionSimulationUI();
	const DESTRUCTION_GROUP* selectedGroup = selectedGroupId.empty() ? nullptr :
		m_DestructionDocument.Find_Group(selectedGroupId);
	const DESTRUCTION_SIMULATION_PROFILE* selectedProfile =
		selectedProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(selectedProfileId);
	if (nullptr != selectedProfile && nullptr != selectedGroup &&
		selectedProfile->groupId != selectedGroup->groupId)
	{
		selectedProfile = nullptr;
	}
	if (nullptr == selectedProfile && nullptr != selectedGroup)
	{
		const auto found = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[selectedGroup](const DESTRUCTION_SIMULATION_PROFILE& profile)
			{
				return profile.groupId == selectedGroup->groupId;
			});
		if (found != m_DestructionSimulationDocument.Get_Profiles().end())
			selectedProfile = &*found;
	}

	if (nullptr != selectedGroup)
		m_SelectedDestructionGroupId = selectedGroup->groupId;
	else if (nullptr == selectedProfile &&
		!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		selectedProfile =
			&m_DestructionSimulationDocument.Get_Profiles().front();
	}
	else
		m_SelectedDestructionGroupId.clear();

	if (nullptr != selectedProfile)
		Select_DestructionSimulationProfile(selectedProfile->profileId);
	else if (nullptr != selectedGroup)
		status += "; selected group has no simulation profile; create default";
	Refresh_DestructionHighlight();
	m_DestructionSimulationStatus = status;
	return true;
}

bool_t Client::CMapTool::Save_DestructionSimulation()
{
	return Save_DestructionAuthoringPair();
}

const DESTRUCTION_SIMULATION_PROFILE*
Client::CMapTool::Get_SelectedDestructionSimulationProfile() const
{
	return m_SelectedDestructionSimulationProfileId.empty() ? nullptr :
		m_DestructionSimulationDocument.Find_Profile(
			m_SelectedDestructionSimulationProfileId);
}

void Client::CMapTool::Reset_DestructionSimulationUI()
{
	m_SelectedDestructionSimulationProfileId.clear();
	m_SelectedDestructionSimulationElementId.clear();
	m_SelectedDestructionSimulationFragmentId.clear();
	m_DestructionSimulationElementDraft.reset();
	m_bDestructionSimulationElementDraftDirty = false;
	m_DestructionSimulationReceiverId[0] = '\0';
	m_bDestructionSimulationClearRequested = true;
}

void Client::CMapTool::Select_DestructionSimulationProfile(
	const std::string& profileId)
{
	if (m_bDestructionSimulationElementDraftDirty &&
		profileId != m_SelectedDestructionSimulationProfileId)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before changing profile";
		return;
	}
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		m_DestructionSimulationDocument.Find_Profile(profileId);
	if (nullptr == profile)
	{
		m_DestructionSimulationStatus =
			"Selected destruction simulation profile is missing";
		return;
	}

	const bool_t profileChanged =
		m_SelectedDestructionSimulationProfileId != profile->profileId;
	m_SelectedDestructionSimulationProfileId = profile->profileId;
	m_SelectedDestructionGroupId = profile->groupId;
	m_SelectedDestructionSimulationElementId.clear();
	m_SelectedDestructionSimulationFragmentId.clear();
	m_DestructionSimulationElementDraft.reset();
	m_bDestructionSimulationElementDraftDirty = false;
	m_DestructionSimulationReceiverId[0] = '\0';
	if (!profile->Elements.empty())
		Select_DestructionSimulationElement(profile->Elements.front().elementId);
	if (nullptr != m_pDestructionSimulationController)
	{
		const bool_t hasRuntime =
			m_pDestructionSimulationController->Get_Runtime().Is_Staged() ||
			m_pDestructionSimulationController->Has_PendingCommand();
		if (profileChanged && hasRuntime)
		{
			m_bDestructionSimulationClearRequested = true;
		}
		else if (hasRuntime)
		{
			m_pDestructionSimulationController->Request_Reset();
		}
	}
	m_DestructionSimulationStatus = "Selected simulation profile: " +
		profile->profileId;
}

void Client::CMapTool::Select_DestructionSimulationElement(
	const std::string& elementId)
{
	if (m_bDestructionSimulationElementDraftDirty &&
		elementId != m_SelectedDestructionSimulationElementId)
	{
		m_DestructionSimulationStatus =
			"Apply or Revert the debris Detail draft before changing element";
		return;
	}
	const DESTRUCTION_SIMULATION_ELEMENT* element =
		m_DestructionSimulationDocument.Find_Element(
			m_SelectedDestructionSimulationProfileId, elementId);
	if (nullptr == element)
	{
		m_DestructionSimulationStatus =
			"Selected debris element is missing";
		return;
	}

	m_SelectedDestructionSimulationElementId = element->elementId;
	m_SelectedDestructionSimulationFragmentId.clear();
	m_DestructionSimulationElementDraft = *element;
	m_bDestructionSimulationElementDraftDirty = false;
	strncpy_s(m_DestructionSimulationReceiverId,
		element->Trigger.receiverCollisionId.c_str(), _TRUNCATE);

	if (nullptr != m_pDestructionSimulationController)
	{
		const DESTRUCTION_SIMULATION_SCOPE scope =
			m_pDestructionSimulationController->Get_Snapshot().eScope;
		if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == scope ||
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope)
		{
			const DESTRUCTION_SIMULATION_PROFILE* selectedProfile =
				Get_SelectedDestructionSimulationProfile();
			if (nullptr != selectedProfile &&
				AreEmittersAuthoredAsOneWall(*selectedProfile))
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
			}
			else
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
					element->elementId);
			}
		}
	}
}

void Client::CMapTool::Select_DestructionSimulationFragment(
	const std::string& elementId,
	const std::string& fragmentId)
{
	if (nullptr == m_pDestructionSimulationController || fragmentId.empty())
	{
		m_DestructionSimulationStatus =
			"Stage a destruction profile before selecting a fragment";
		return;
	}
	const DESTRUCTION_SIMULATION_FRAME& frame =
		m_pDestructionSimulationController->Get_Runtime().Get_Frame();
	if (frame.profileId != m_SelectedDestructionSimulationProfileId)
	{
		m_DestructionSimulationStatus =
			"Selected fragment is not in the active staged profile";
		return;
	}
	const auto runtimeElement = std::find_if(
		frame.Elements.begin(), frame.Elements.end(),
		[&elementId](const DESTRUCTION_SIMULATION_ELEMENT_FRAME& value)
		{
			return value.elementId == elementId;
		});
	if (runtimeElement == frame.Elements.end())
	{
		m_DestructionSimulationStatus =
			"Selected fragment emitter is missing from the runtime frame";
		return;
	}
	const auto fragment = std::find_if(
		runtimeElement->Fragments.begin(), runtimeElement->Fragments.end(),
		[&fragmentId](const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& value)
		{
			return value.fragmentId == fragmentId;
		});
	if (fragment == runtimeElement->Fragments.end())
	{
		m_DestructionSimulationStatus =
			"Selected fragment is missing from the runtime frame";
		return;
	}

	const bool_t keepSoloFragment =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT ==
			m_pDestructionSimulationController->Get_Snapshot().eScope;
	if (m_SelectedDestructionSimulationElementId != elementId)
		Select_DestructionSimulationElement(elementId);
	if (m_SelectedDestructionSimulationElementId != elementId)
		return;
	m_SelectedDestructionSimulationFragmentId = fragment->fragmentId;
	if (keepSoloFragment)
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
			fragment->fragmentId);
	}
	m_DestructionSimulationStatus = "Selected mesh fragment: " +
		fragment->fragmentId;
}

bool_t Client::CMapTool::Create_DefaultDestructionSimulationProfile()
{
	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr == group)
	{
		m_DestructionSimulationStatus =
			"Select a non-empty destruction group before creating a profile";
		return false;
	}
	const auto existingProfile = std::find_if(
		m_DestructionSimulationDocument.Get_Profiles().begin(),
		m_DestructionSimulationDocument.Get_Profiles().end(),
		[group](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.profileId == group->groupId + ".preview";
		});
	if (existingProfile != m_DestructionSimulationDocument.Get_Profiles().end())
	{
		Select_DestructionSimulationProfile(existingProfile->profileId);
		m_DestructionSimulationStatus =
			"Selected the existing simulation profile for group " +
			group->groupId;
		return true;
	}

	DESTRUCTION_SIMULATION_PROFILE profile;
	std::string status;
	if (!CDestructionSimulationDocument::Create_DefaultForGroup(
		*group, m_DeployRuntime, profile, status) ||
		!m_DestructionSimulationDocument.Add_Profile(profile, status))
	{
		m_DestructionSimulationStatus = status;
		return false;
	}
	Select_DestructionSimulationProfile(profile.profileId);
	m_DestructionSimulationStatus = status;
	return true;
}

bool_t Client::CMapTool::Modify_DestructionGroupMember(
	const uint64_t placementId,
	const bool_t addMember)
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionStatus =
			"Apply or Revert the debris Detail draft before editing group members";
		return false;
	}
	if (0u == placementId || m_SelectedDestructionGroupId.empty())
	{
		m_DestructionStatus = "A selected group and Deploy placement are required";
		return false;
	}

	CWorldDestructionDocument stagedDestruction = m_DestructionDocument;
	CDestructionSimulationDocument stagedSimulation =
		m_DestructionSimulationDocument;
	std::string status;
	const bool_t changed = addMember ? stagedDestruction.Add_Member(
		m_SelectedDestructionGroupId, placementId, status) :
		stagedDestruction.Remove_Member(
			m_SelectedDestructionGroupId, placementId);
	if (!changed)
	{
		m_DestructionStatus = status.empty() ?
			"Destruction group member change was rejected" : status;
		return false;
	}

	const DESTRUCTION_GROUP* stagedGroup = stagedDestruction.Find_Group(
		m_SelectedDestructionGroupId);
	if (nullptr == stagedGroup ||
		!stagedSimulation.Synchronize_Group(
			*stagedGroup, m_DeployRuntime, status) ||
		!stagedSimulation.Validate_GroupReferences(
			stagedDestruction, status))
	{
		m_DestructionStatus = status.empty() ?
			"Destruction group/simulation transaction was rejected" : status;
		return false;
	}

	const std::string selectedGroupId = m_SelectedDestructionGroupId;
	const std::string selectedProfileId =
		m_SelectedDestructionSimulationProfileId;
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_DestructionDocument = std::move(stagedDestruction);
	m_DestructionSimulationDocument = std::move(stagedSimulation);
	Reset_DestructionSimulationUI();
	m_bDestructionSimulationClearRequested = false;
	m_SelectedDestructionGroupId = selectedGroupId;
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		m_DestructionSimulationDocument.Find_Profile(selectedProfileId);
	if (nullptr == profile)
	{
		const auto found = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[&selectedGroupId](const DESTRUCTION_SIMULATION_PROFILE& candidate)
			{
				return candidate.groupId == selectedGroupId;
			});
		if (found != m_DestructionSimulationDocument.Get_Profiles().end())
			profile = &*found;
	}
	if (nullptr != profile)
		Select_DestructionSimulationProfile(profile->profileId);
	Refresh_DestructionHighlight();
	m_DestructionStatus = addMember ?
		"Added group member and synchronized simulation elements" :
		"Removed group member and synchronized simulation elements";
	return true;
}

bool_t Client::CMapTool::Request_StageDestructionSimulation(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const bool_t preserveSampleTime,
	const bool_t playAfterStage)
{
	if (!m_bDestructionDebrisPrototypesReady)
	{
		m_DestructionSimulationStatus = m_DestructionDebrisPrototypeStatus;
		return false;
	}
	if (nullptr == m_pDestructionSimulationController ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		profile.groupId != m_SelectedDestructionGroupId ||
		nullptr == m_DestructionDocument.Find_Group(profile.groupId))
	{
		m_DestructionSimulationStatus =
			"Stage requires the selected profile and destruction group to match";
		return false;
	}

	std::string validationStatus;
	if (!CDestructionSimulationDocument::Validate_Profile(
		profile, validationStatus))
	{
		m_DestructionSimulationStatus = validationStatus;
		return false;
	}

	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT previous =
		m_pDestructionSimulationController->Get_Snapshot();
	/* Stage_Profile replaces the staged actors through the controller-owned
	   runtime, so it supersedes a UI-only clear requested by profile selection. */
	m_bDestructionSimulationClearRequested = false;
	m_pDestructionSimulationController->Request_StageProfile(
		profile,
		m_SelectedDestructionGroupId,
		m_DestructionDocument,
		m_DeployRuntime,
		m_iAuthoringLevelIndex);
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == previous.eScope &&
		!m_SelectedDestructionSimulationFragmentId.empty())
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
			m_SelectedDestructionSimulationFragmentId);
	}
	else if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == previous.eScope &&
		!m_SelectedDestructionSimulationElementId.empty())
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
			m_SelectedDestructionSimulationElementId);
	}
	else
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
	}
	if (preserveSampleTime)
		m_pDestructionSimulationController->Request_Seek(
			(std::min)(previous.fSampleTimeSeconds, profile.fDurationSeconds));
	if (playAfterStage)
		m_pDestructionSimulationController->Request_Play();
	m_DestructionSimulationStatus = playAfterStage ?
		"Simulation stage + play requested: " + profile.profileId :
		"Simulation staged paused at 0 s; press Play: " + profile.profileId;
	return true;
}

bool_t Client::CMapTool::Stage_DestructionElementDraftPreview()
{
	const DESTRUCTION_SIMULATION_PROFILE* activeProfile =
		Get_SelectedDestructionSimulationProfile();
	if (nullptr == activeProfile ||
		!m_DestructionSimulationElementDraft.has_value())
	{
		return false;
	}

	DESTRUCTION_SIMULATION_PROFILE staged = *activeProfile;
	const auto element = std::find_if(staged.Elements.begin(),
		staged.Elements.end(),
		[this](const DESTRUCTION_SIMULATION_ELEMENT& value)
		{
			return value.elementId ==
				m_DestructionSimulationElementDraft->elementId;
		});
	if (element == staged.Elements.end())
		return false;
	*element = *m_DestructionSimulationElementDraft;

	const bool_t wasPlaying = nullptr != m_pDestructionSimulationController &&
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING ==
			m_pDestructionSimulationController->Get_Snapshot().eState;
	return Request_StageDestructionSimulation(staged, true, wasPlaying);
}

bool_t Client::CMapTool::Try_PickDeployProp(
	uint64_t& outRuntimePlacementId,
	std::string& outFailure) const
{
	outFailure.clear();
	if (!m_DeployRuntime.Is_Loaded() ||
		m_DeployRuntime.Get_Entries().empty())
	{
		outFailure = "Wall pick unavailable: DeployProp runtime is not ready";
		return false;
	}

	float3_t picked{};
	if (!Try_PickPlacementPosition(picked))
	{
		outFailure =
			"Wall pick missed: no rendered surface under the cursor";
		return false;
	}

	/* The depth pick returns the surface point of whatever was drawn, so the
	   owning prop is the one whose world bounds contain that point. Ties go to
	   the nearest centre so overlapping props stay selectable. */
	constexpr f32_t tolerance = 0.05f;
	uint64_t bestId = 0u;
	f32_t bestDistance = 0.f;
	size_t missingBounds = 0u;
	for (const DEPLOY_RUNTIME_ENTRY& entry : m_DeployRuntime.Get_Entries())
	{
		float3_t center{};
		float3_t halfExtents{};
		if (nullptr == entry.object ||
			!entry.object->Get_WorldBounds(center, halfExtents))
		{
			++missingBounds;
			continue;
		}
		if (
			std::abs(picked.x - center.x) > halfExtents.x + tolerance ||
			std::abs(picked.y - center.y) > halfExtents.y + tolerance ||
			std::abs(picked.z - center.z) > halfExtents.z + tolerance)
		{
			continue;
		}
		const f32_t offsetX = picked.x - center.x;
		const f32_t offsetY = picked.y - center.y;
		const f32_t offsetZ = picked.z - center.z;
		const f32_t distance =
			offsetX * offsetX + offsetY * offsetY + offsetZ * offsetZ;
		if (0u == bestId || distance < bestDistance)
		{
			bestId = entry.placement.runtimePlacementId;
			bestDistance = distance;
		}
	}
	if (0u == bestId)
	{
		outFailure =
			"Wall pick missed: the rendered surface is not a loaded DeployProp";
		if (0u != missingBounds)
			outFailure += " (" + std::to_string(missingBounds) +
				" props have no model bounds)";
		return false;
	}
	outRuntimePlacementId = bestId;
	return true;
}

bool_t Client::CMapTool::Validate_DestructionExternalReferences(
	const CWorldDestructionDocument& destruction,
	const CDeployPropRuntime& deployRuntime,
	const CNavRuntimeBlockerDocument& blockers,
	const CWorldGameplayDocument& worldGameplay,
	const CEncounterPatternReference& encounter,
	std::string& outStatus) const
{
	const bool_t hasEnabledBinding = std::any_of(
		destruction.Get_Bindings().begin(),
		destruction.Get_Bindings().end(),
		[](const DESTRUCTION_BINDING& binding)
		{
			return binding.isEnabled;
		});
	if (hasEnabledBinding)
	{
		if (!encounter.Is_Ready() ||
			encounter.Get_EncounterId() != "ENCOUNTER_VALTAN" ||
			encounter.Get_BossArchetypeId() != "BOSS_VALTAN")
		{
			outStatus =
				"Save blocked: enabled break settings require the Valtan encounter";
			return false;
		}

		size_t matchingBossCount = 0u;
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			worldGameplay.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::BOSS == placement.eKind &&
				placement.archetypeId == encounter.Get_BossArchetypeId() &&
				placement.encounterId == encounter.Get_EncounterId())
			{
				++matchingBossCount;
			}
		}
		if (1u != matchingBossCount)
		{
			outStatus =
				"Save blocked: enabled break settings require exactly one "
				"BOSS_VALTAN placement bound to ENCOUNTER_VALTAN";
			return false;
		}
	}

	std::vector<std::string> bindingSemanticKeys;
	std::vector<std::pair<std::string, std::string>> navigationOwners;
	for (const DESTRUCTION_GROUP& group : destruction.Get_Groups())
	{
		for (const uint64_t placementId : group.memberPlacementIds)
		{
			const auto entry = std::find_if(
				deployRuntime.Get_Entries().begin(),
				deployRuntime.Get_Entries().end(),
				[placementId](const DEPLOY_RUNTIME_ENTRY& value)
				{
					return value.placement.runtimePlacementId == placementId;
				});
			if (deployRuntime.Get_Entries().end() == entry ||
				!entry->placement.destructible)
			{
				outStatus = "Save blocked: group " + group.groupId +
					" references an unknown or non-destructible DeployProp " +
					std::to_string(placementId);
				return false;
			}
		}

		for (const std::string& regionId : group.navigationRegionIds)
		{
			const auto claimed = std::find_if(navigationOwners.begin(),
				navigationOwners.end(), [&regionId](const auto& value)
				{
					return value.first == regionId;
				});
			if (navigationOwners.end() != claimed &&
				claimed->second != group.groupId)
			{
				outStatus = "Save blocked: navigation region " + regionId +
					" is already owned by group " + claimed->second;
				return false;
			}
			if (navigationOwners.end() == claimed)
				navigationOwners.push_back({ regionId, group.groupId });

			bool_t found = false;
			for (size_t index = 0u;
				index < blockers.Get_RegionCount(); ++index)
			{
				const NAV_RUNTIME_BLOCKER_REGION* region =
					blockers.Get_Region(index);
				if (nullptr != region && region->id == regionId)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				outStatus = "Save blocked: group " + group.groupId +
					" references an unknown navigation region " + regionId;
				return false;
			}
		}
	}

	for (const DESTRUCTION_BINDING& binding :
		destruction.Get_Bindings())
	{
		const DESTRUCTION_MUTATION* mutation =
			destruction.Find_Mutation(binding.mutationId);
		const DESTRUCTION_GROUP* group = nullptr == mutation ? nullptr :
			destruction.Find_Group(mutation->groupId);
		if (nullptr == mutation || nullptr == group)
		{
			outStatus = "Save blocked: binding " + binding.bindingId +
				" has a missing mutation or group";
			return false;
		}
		if (binding.isEnabled && group->navigationRegionIds.empty())
		{
			outStatus = "Save blocked: enabled binding " + binding.bindingId +
				" requires a navigation blocker region";
			return false;
		}
		if (binding.isEnabled && group->memberPlacementIds.empty())
		{
			outStatus = "Save blocked: enabled binding " + binding.bindingId +
				" has no wall members";
			return false;
		}
		if (binding.isEnabled)
		{
			for (const std::string& regionId : group->navigationRegionIds)
			{
				bool_t hasCells = false;
				for (size_t index = 0u; index < blockers.Get_RegionCount(); ++index)
				{
					const NAV_RUNTIME_BLOCKER_REGION* region = blockers.Get_Region(index);
					if (nullptr != region && region->id == regionId)
					{
						hasCells = 0u != blockers.Get_RegionCellCount(index);
						break;
					}
				}
				if (!hasCells)
				{
					outStatus = "Save blocked: enabled binding " +
						binding.bindingId + " uses an empty navigation region " +
						regionId;
					return false;
				}
			}
		}

		const ENCOUNTER_PATTERN_REFERENCE* pattern =
			encounter.Find_Pattern(binding.patternId);
		if (nullptr == pattern)
		{
			outStatus = "Save blocked: binding " + binding.bindingId +
				" references an unknown Valtan pattern " + binding.patternId;
			return false;
		}
		const auto stage = std::find_if(pattern->stages.begin(),
			pattern->stages.end(), [&binding](const ENCOUNTER_STAGE_REFERENCE& value)
			{
				return value.stageId == binding.stageId;
			});
		if (pattern->stages.end() == stage ||
			(DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind &&
				binding.iOffsetMs > stage->iDurationMs))
		{
			outStatus = "Save blocked: binding " + binding.bindingId +
				" has an unknown stage or an out-of-range time";
			return false;
		}
		if (DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT ==
			binding.eTriggerKind)
		{
			const WORLD_GAMEPLAY_PLACEMENT* receiver =
				worldGameplay.Find(binding.receiverCollisionId);
			if (nullptr == receiver ||
				WORLD_PLACEMENT_KIND::COLLISION_BOX != receiver->eKind ||
				(binding.isEnabled && !receiver->isEnabled))
			{
				outStatus = "Save blocked: binding " + binding.bindingId +
					" references an unknown Collision Box " +
					binding.receiverCollisionId;
				return false;
			}
		}

		const std::string semanticKey = group->groupId + "|" +
			binding.patternId + "|" + binding.stageId + "|" +
			CWorldDestructionDocument::TriggerKind_ToString(
				binding.eTriggerKind) + "|" +
			std::to_string(binding.iOffsetMs) + "|" +
			binding.receiverCollisionId;
		if (bindingSemanticKeys.end() != std::find(
			bindingSemanticKeys.begin(), bindingSemanticKeys.end(), semanticKey))
		{
			outStatus = "Save blocked: duplicate semantic break setting " +
				binding.bindingId;
			return false;
		}
		bindingSemanticKeys.push_back(semanticKey);
	}

	outStatus = "World destruction external references validated";
	return true;
}

bool_t Client::CMapTool::Validate_CurrentDestructionReferences(
	std::string& outStatus) const
{
	if (!m_DestructionDocument.Is_Ready())
	{
		outStatus = "World destruction document is not active";
		return true;
	}
	return Validate_DestructionExternalReferences(
		m_DestructionDocument,
		m_DeployRuntime,
		m_RuntimeBlockerDocument,
		m_WorldGameplayDocument,
		m_EncounterReference,
		outStatus);
}

bool_t Client::CMapTool::Select_DestructionWall(
	const uint64_t runtimePlacementId,
	const char_t* source)
{
	if (m_bDestructionSimulationElementDraftDirty)
	{
		m_DestructionStatus =
			"Apply or Revert the debris Detail draft before selecting another wall";
		return false;
	}

	Restore_DestructionPreview();
	const auto entry = std::find_if(
		m_DeployRuntime.Get_Entries().begin(),
		m_DeployRuntime.Get_Entries().end(),
		[runtimePlacementId](const DEPLOY_RUNTIME_ENTRY& value)
		{
			return value.placement.runtimePlacementId == runtimePlacementId;
		});
	if (m_DeployRuntime.Get_Entries().end() == entry)
	{
		m_DestructionStatus = "Wall selection failed: unknown placement " +
			std::to_string(runtimePlacementId);
		return false;
	}
	if (!entry->placement.destructible)
	{
		m_DestructionStatus =
			"Wall selection rejected: this DeployProp is not destructible";
		return false;
	}

	if (m_bDestructionAddMemberArmed)
	{
		const DESTRUCTION_GROUP* target =
			m_SelectedDestructionGroupId.empty() ? nullptr :
			m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
		if (nullptr == target)
		{
			m_bDestructionAddMemberArmed = false;
			m_DestructionStatus =
				"Add wall cancelled: the target group no longer exists";
			return false;
		}

		const DESTRUCTION_GROUP* owner =
			m_DestructionDocument.Find_GroupOfMember(runtimePlacementId);
		if (nullptr != owner && owner->groupId != target->groupId)
		{
			m_DestructionStatus = "Add wall failed: it already belongs to " +
				owner->groupId;
			return false;
		}
		if (nullptr == owner)
		{
			if (!Modify_DestructionGroupMember(runtimePlacementId, true))
				return false;
		}
		m_bDestructionAddMemberArmed = false;
	}

	m_iSelectedDeployPlacementId = runtimePlacementId;
	const DESTRUCTION_GROUP* owner =
		m_DestructionDocument.Find_GroupOfMember(runtimePlacementId);
	if (nullptr != owner)
	{
		m_SelectedDestructionGroupId = owner->groupId;
		const auto profile = std::find_if(
			m_DestructionSimulationDocument.Get_Profiles().begin(),
			m_DestructionSimulationDocument.Get_Profiles().end(),
			[owner](const DESTRUCTION_SIMULATION_PROFILE& value)
			{
				return value.groupId == owner->groupId;
			});
		if (profile != m_DestructionSimulationDocument.Get_Profiles().end())
			Select_DestructionSimulationProfile(profile->profileId);
		else
		{
			Reset_DestructionSimulationUI();
			m_SelectedDestructionGroupId = owner->groupId;
			m_DestructionSimulationStatus =
				"Selected wall group has no simulation profile; create default";
		}
	}
	else if (!m_bDestructionAdvancedMode)
	{
		m_SelectedDestructionGroupId.clear();
		Reset_DestructionSimulationUI();
		m_DestructionSimulationStatus =
			"Selected wall is not assigned to a destruction group";
	}

	Sync_DestructionDraftFromSelection();
	if (!Refresh_DestructionHighlight())
	{
		m_DestructionStatus =
			"Wall selected and its group data was updated, but the authoring outline could not be created";
		m_bDestructionPickArmed = false;
		return true;
	}

	m_bDestructionPickArmed = false;
	m_DestructionStatus = "Selected wall " +
		std::to_string(runtimePlacementId) + " | " +
		entry->placement.assetId + " | " +
		(nullptr != source ? source : "unknown source");
	return true;
}

void Client::CMapTool::Sync_DestructionDraftFromSelection()
{
	m_SelectedDestructionBindingId.clear();
	m_bDestructionNewSettingArmed = false;
	if (0u == m_iSelectedDeployPlacementId)
		return;

	const DESTRUCTION_GROUP* owner =
		m_DestructionDocument.Find_GroupOfMember(
			m_iSelectedDeployPlacementId);
	if (nullptr == owner)
	{
		m_SelectedDestructionPatternId.clear();
		m_SelectedDestructionStageId.clear();
		m_iDestructionTriggerKind = 1;
		m_iDestructionOffsetMs = 0;
		m_iDestructionBreakingMs = 1900;
		m_bDestructionBindingEnabled = false;
		m_DestructionReceiverId[0] = '\0';
		m_fDestructionTimelineMs = 0.f;
		m_bDestructionTimelinePlaying = false;
		m_bDestructionNewSettingArmed = true;
		return;
	}
	m_SelectedDestructionGroupId = owner->groupId;

	const DESTRUCTION_BINDING* onlyBinding = nullptr;
	size_t bindingCount = 0u;
	for (const DESTRUCTION_BINDING& binding :
		m_DestructionDocument.Get_Bindings())
	{
		const DESTRUCTION_MUTATION* mutation =
			m_DestructionDocument.Find_Mutation(binding.mutationId);
		if (nullptr != mutation && mutation->groupId == owner->groupId &&
			WORLD_DESTROYABLE_STATE::FRACTURED == mutation->eTargetState)
		{
			onlyBinding = &binding;
			++bindingCount;
		}
	}
	if (1u == bindingCount && nullptr != onlyBinding)
		Load_DestructionDraftFromBinding(*onlyBinding);
	else
	{
		m_SelectedDestructionPatternId.clear();
		m_SelectedDestructionStageId.clear();
		m_iDestructionTriggerKind = 1;
		m_iDestructionOffsetMs = 0;
		m_iDestructionBreakingMs = 1900;
		m_bDestructionBindingEnabled = false;
		m_DestructionReceiverId[0] = '\0';
		m_fDestructionTimelineMs = 0.f;
		m_bDestructionTimelinePlaying = false;
		m_bDestructionNewSettingArmed = 0u == bindingCount;
	}
}

void Client::CMapTool::Load_DestructionDraftFromBinding(
	const DESTRUCTION_BINDING& binding)
{
	m_SelectedDestructionBindingId = binding.bindingId;
	m_bDestructionNewSettingArmed = false;
	m_SelectedDestructionPatternId = binding.patternId;
	m_SelectedDestructionStageId = binding.stageId;
	m_iDestructionTriggerKind =
		DESTRUCTION_TRIGGER_KIND::STAGE_ENTER == binding.eTriggerKind ? 0 :
		DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind ? 1 :
		DESTRUCTION_TRIGGER_KIND::STAGE_EXIT == binding.eTriggerKind ? 2 : 3;
	m_iDestructionOffsetMs = static_cast<int32_t>(binding.iOffsetMs);
	m_bDestructionBindingEnabled = binding.isEnabled;
	strncpy_s(m_DestructionReceiverId, binding.receiverCollisionId.c_str(),
		_TRUNCATE);

	const DESTRUCTION_MUTATION* mutation =
		m_DestructionDocument.Find_Mutation(binding.mutationId);
	if (nullptr != mutation)
		m_iDestructionBreakingMs =
			static_cast<int32_t>(mutation->iBreakingDurationMs);

	const ENCOUNTER_STAGE_REFERENCE* stage =
		Find_SelectedDestructionStage();
	if (nullptr != stage)
	{
		m_fDestructionTimelineMs = static_cast<f32_t>(stage->iStartOffsetMs);
		if (DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind)
			m_fDestructionTimelineMs += static_cast<f32_t>(binding.iOffsetMs);
		else if (DESTRUCTION_TRIGGER_KIND::STAGE_EXIT == binding.eTriggerKind)
			m_fDestructionTimelineMs += static_cast<f32_t>(stage->iDurationMs);
	}
}

const Client::ENCOUNTER_STAGE_REFERENCE*
Client::CMapTool::Find_SelectedDestructionStage() const
{
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern)
		return nullptr;
	const auto stage = std::find_if(pattern->stages.begin(), pattern->stages.end(),
		[this](const ENCOUNTER_STAGE_REFERENCE& value)
		{
			return value.stageId == m_SelectedDestructionStageId;
		});
	return pattern->stages.end() == stage ? nullptr : &(*stage);
}

void Client::CMapTool::Use_DestructionTimelineTime()
{
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern || pattern->stages.empty())
	{
		m_DestructionStatus = "Choose a pattern before using the playhead";
		return;
	}

	const f32_t clamped = (std::clamp)(m_fDestructionTimelineMs, 0.f,
		static_cast<f32_t>(pattern->iTotalDurationMs));
	const ENCOUNTER_STAGE_REFERENCE* selected = &pattern->stages.back();
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
	{
		if (clamped < static_cast<f32_t>(
			stage.iStartOffsetMs + stage.iDurationMs))
		{
			selected = &stage;
			break;
		}
	}

	m_SelectedDestructionStageId = selected->stageId;
	m_iDestructionTriggerKind = 1;
	m_iDestructionOffsetMs = static_cast<int32_t>((std::clamp)(
		clamped - static_cast<f32_t>(selected->iStartOffsetMs),
		0.f, static_cast<f32_t>(selected->iDurationMs)));
	m_DestructionStatus = "Break time copied from the pattern playhead";
}

bool_t Client::CMapTool::Apply_SimpleDestructionAuthoring()
{
	const auto entry = std::find_if(
		m_DeployRuntime.Get_Entries().begin(),
		m_DeployRuntime.Get_Entries().end(),
		[this](const DEPLOY_RUNTIME_ENTRY& value)
		{
			return value.placement.runtimePlacementId ==
				m_iSelectedDeployPlacementId;
		});
	if (m_DeployRuntime.Get_Entries().end() == entry ||
		!entry->placement.destructible)
	{
		m_DestructionStatus = "Choose a destructible wall first";
		return false;
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	const ENCOUNTER_STAGE_REFERENCE* stage =
		Find_SelectedDestructionStage();
	if (nullptr == pattern || nullptr == stage)
	{
		m_DestructionStatus = "Choose a valid Valtan pattern and stage";
		return false;
	}

	const DESTRUCTION_TRIGGER_KIND trigger =
		0 == m_iDestructionTriggerKind ?
			DESTRUCTION_TRIGGER_KIND::STAGE_ENTER :
		1 == m_iDestructionTriggerKind ?
			DESTRUCTION_TRIGGER_KIND::STAGE_TIME :
		2 == m_iDestructionTriggerKind ?
			DESTRUCTION_TRIGGER_KIND::STAGE_EXIT :
			DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT;
	const uint32_t offsetMs =
		DESTRUCTION_TRIGGER_KIND::STAGE_TIME == trigger ?
			static_cast<uint32_t>((std::max)(0, m_iDestructionOffsetMs)) : 0u;
	if (offsetMs > stage->iDurationMs)
	{
		m_DestructionStatus = "Break time exceeds the selected stage duration";
		return false;
	}

	std::string receiverId;
	if (DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT == trigger)
	{
		receiverId = m_DestructionReceiverId;
		const WORLD_GAMEPLAY_PLACEMENT* receiver =
			m_WorldGameplayDocument.Find(receiverId);
		if (nullptr == receiver ||
			WORLD_PLACEMENT_KIND::COLLISION_BOX != receiver->eKind)
		{
			m_DestructionStatus =
				"Choose an existing Collision Box for impact destruction";
			return false;
		}
	}
	if (m_iDestructionBreakingMs < 0 ||
		m_iDestructionBreakingMs >
			static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS))
	{
		m_DestructionStatus = "Breaking duration is outside the supported range";
		return false;
	}

	CWorldDestructionDocument staged = m_DestructionDocument;
	std::string status;
	const uint64_t wallId = entry->placement.runtimePlacementId;
	const DESTRUCTION_GROUP* owner = staged.Find_GroupOfMember(wallId);
	std::string groupId;
	if (nullptr == owner)
	{
		groupId = "destroyable.group.valtan.deploy." + std::to_string(wallId);
		if (!staged.Add_Group(groupId, status) ||
			!staged.Add_Member(groupId, wallId, status))
		{
			m_DestructionStatus = status;
			return false;
		}
	}
	else
	{
		groupId = owner->groupId;
	}

	const std::string groupHash = ToStableHex(HashStableAuthoringId(groupId));
	std::string mutationId;
	const DESTRUCTION_BINDING* selectedBinding =
		m_SelectedDestructionBindingId.empty() ? nullptr :
		staged.Find_Binding(m_SelectedDestructionBindingId);
	if (nullptr != selectedBinding)
	{
		const DESTRUCTION_MUTATION* selectedMutation =
			staged.Find_Mutation(selectedBinding->mutationId);
		if (nullptr != selectedMutation && selectedMutation->groupId == groupId &&
			WORLD_DESTROYABLE_STATE::FRACTURED ==
				selectedMutation->eTargetState)
			mutationId = selectedMutation->mutationId;
		else
		{
			m_DestructionStatus =
				"Easy Wall Editor can only edit FRACTURED wall settings";
			return false;
		}
	}
	if (mutationId.empty())
	{
		for (const DESTRUCTION_MUTATION& mutation : staged.Get_Mutations())
		{
			if (mutation.groupId != groupId ||
				WORLD_DESTROYABLE_STATE::FRACTURED != mutation.eTargetState)
			{
				continue;
			}
			if (!mutationId.empty())
			{
				m_DestructionStatus =
					"This group has multiple fracture mutations. Choose one in Advanced mode.";
				return false;
			}
			mutationId = mutation.mutationId;
		}
	}
	if (mutationId.empty())
	{
		mutationId = "mutation.valtan.group." + groupHash + ".fracture";
		DESTRUCTION_MUTATION mutation;
		mutation.mutationId = mutationId;
		mutation.groupId = groupId;
		mutation.eTargetState = WORLD_DESTROYABLE_STATE::FRACTURED;
		mutation.iBreakingDurationMs =
			static_cast<uint32_t>(m_iDestructionBreakingMs);
		if (!staged.Add_Mutation(mutation, status))
		{
			m_DestructionStatus = status;
			return false;
		}
	}
	else
	{
		const DESTRUCTION_MUTATION* current = staged.Find_Mutation(mutationId);
		if (nullptr == current)
		{
			m_DestructionStatus = "Selected fracture mutation is missing";
			return false;
		}
		DESTRUCTION_MUTATION updated = *current;
		updated.iBreakingDurationMs =
			static_cast<uint32_t>(m_iDestructionBreakingMs);
		if (!staged.Update_Mutation(updated, status))
		{
			m_DestructionStatus = status;
			return false;
		}
	}

	auto hasSameSemantic = [&staged, &groupId, &pattern, &stage, trigger,
		offsetMs, &receiverId](const DESTRUCTION_BINDING& value)
	{
		const DESTRUCTION_MUTATION* valueMutation =
			staged.Find_Mutation(value.mutationId);
		return nullptr != valueMutation && valueMutation->groupId == groupId &&
			value.patternId == pattern->patternId &&
			value.stageId == stage->stageId &&
			value.eTriggerKind == trigger &&
			value.iOffsetMs == offsetMs &&
			value.receiverCollisionId == receiverId;
	};
	std::vector<const DESTRUCTION_BINDING*> semanticMatches;
	for (const DESTRUCTION_BINDING& value : staged.Get_Bindings())
	{
		if (hasSameSemantic(value))
			semanticMatches.push_back(&value);
	}

	std::string bindingId;
	if (nullptr != selectedBinding && selectedBinding->mutationId == mutationId)
	{
		for (const DESTRUCTION_BINDING* match : semanticMatches)
		{
			if (match->bindingId != selectedBinding->bindingId)
			{
				m_DestructionStatus =
					"This group already has the same break setting";
				return false;
			}
		}
		bindingId = selectedBinding->bindingId;
	}
	else if (semanticMatches.size() > 1u)
	{
		m_DestructionStatus =
			"Duplicate break settings already exist. Resolve them in Advanced mode.";
		return false;
	}
	else if (1u == semanticMatches.size())
	{
		bindingId = semanticMatches.front()->bindingId;
	}
	else
	{
		const std::string semantic = pattern->patternId + "|" + stage->stageId +
			"|" + CWorldDestructionDocument::TriggerKind_ToString(trigger) +
			"|" + std::to_string(offsetMs) + "|" + receiverId;
		bindingId = "binding.valtan.group." + groupHash + "." +
			ToStableHex(HashStableAuthoringId(semantic));
	}

	DESTRUCTION_BINDING binding;
	binding.bindingId = bindingId;
	binding.mutationId = mutationId;
	binding.patternId = pattern->patternId;
	binding.stageId = stage->stageId;
	binding.eTriggerKind = trigger;
	binding.iOffsetMs = offsetMs;
	binding.receiverCollisionId = receiverId;
	binding.isEnabled = m_bDestructionBindingEnabled;
	const DESTRUCTION_BINDING* existing = staged.Find_Binding(bindingId);
	if (nullptr != existing &&
		(nullptr == selectedBinding ||
			existing->bindingId != selectedBinding->bindingId) &&
		!hasSameSemantic(*existing))
	{
		m_DestructionStatus =
			"Generated break setting ID collides with a different setting";
		return false;
	}
	const bool_t bindingApplied = nullptr == existing ?
		staged.Add_Binding(binding, status) :
		staged.Update_Binding(binding, status);
	if (!bindingApplied)
	{
		m_DestructionStatus = status;
		return false;
	}

	m_DestructionDocument = std::move(staged);
	m_SelectedDestructionGroupId = groupId;
	m_SelectedDestructionBindingId = bindingId;
	m_bDestructionNewSettingArmed = false;
	strncpy_s(m_DestructionGroupId, groupId.c_str(), _TRUNCATE);
	strncpy_s(m_DestructionMutationId, mutationId.c_str(), _TRUNCATE);
	strncpy_s(m_DestructionBindingId, bindingId.c_str(), _TRUNCATE);
	Refresh_DestructionHighlight();
	m_DestructionStatus = "Wall break setting staged. Press Save to write Data.";
	return true;
}

bool_t Client::CMapTool::Refresh_DestructionHighlight()
{
	Remove_WorldTriggerBoxes(m_DestructionHighlightBoxes);
	if (m_iAuthoringLevelIndex >= ETOUI(LEVEL::END))
		return false;

	/* Outline every member of the selected group, plus the single picked prop
	   while it has no group yet. The wire box is authoring presentation only. */
	std::vector<uint64_t> targets;
	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr != group)
		targets = group->memberPlacementIds;
	if (0u != m_iSelectedDeployPlacementId &&
		targets.end() == std::find(targets.begin(), targets.end(),
			m_iSelectedDeployPlacementId))
	{
		targets.push_back(m_iSelectedDeployPlacementId);
	}

	vector<TRIGGER_BOX_ENTRY> staged;
	for (const uint64_t placementId : targets)
	{
		const shared_ptr<CDeployPropObject> prop =
			m_DeployRuntime.Find(placementId);
		float3_t center{};
		float3_t halfExtents{};
		if (nullptr == prop || !prop->Get_WorldBounds(center, halfExtents))
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = std::to_string(placementId);
		desc.position = center;
		desc.halfExtents = halfExtents;
		desc.yawDegrees = 0.f;
		desc.isEnabled = true;
		desc.isCollisionBox = false;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_TriggerBoxes"),
			&desc,
			&gameObject)))
		{
			Remove_WorldTriggerBoxes(staged);
			return false;
		}
		shared_ptr<CTrigger_Box> box =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == box)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_TriggerBoxes"),
				gameObject);
			Remove_WorldTriggerBoxes(staged);
			return false;
		}
		box->Set_Selected(placementId == m_iSelectedDeployPlacementId);
		box->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode);
		staged.push_back({ desc.placementId, std::move(box) });
	}
	m_DestructionHighlightBoxes = std::move(staged);
	return true;
}

void Client::CMapTool::Apply_DestructionPreview(const DEPLOY_PROP_STATE state)
{
	Restore_DestructionPreview();
	std::vector<uint64_t> targets;
	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr != group)
		targets = group->memberPlacementIds;
	else if (0u != m_iSelectedDeployPlacementId)
		targets.push_back(m_iSelectedDeployPlacementId);
	if (targets.empty())
	{
		m_DestructionStatus = "Select a group or a wall before previewing";
		return;
	}

	/* Presentation only. The Server owns the real state and nothing here is
	   written to a document, so the preview never marks the document dirty. */
	size_t applied = 0u;
	for (const uint64_t placementId : targets)
	{
		const shared_ptr<CDeployPropObject> prop =
			m_DeployRuntime.Find(placementId);
		if (nullptr == prop)
			continue;
		m_DestructionPreviewPreviousStates.push_back(
			{ placementId, prop->Get_State() });
		if (m_DeployRuntime.Set_State(placementId, state))
			++applied;
	}
	m_DestructionStatus = "Preview applied to " + std::to_string(applied) +
		" of " + std::to_string(targets.size()) + " props";
	Refresh_DestructionHighlight();
}

void Client::CMapTool::Restore_DestructionPreview()
{
	for (const auto& previous : m_DestructionPreviewPreviousStates)
		m_DeployRuntime.Set_State(previous.first, previous.second);
	m_DestructionPreviewPreviousStates.clear();
}

void Client::CMapTool::Render_DestructionSimpleEditor()
{
	if (ImGui::Button(m_bDestructionPickArmed ?
		"Cancel Wall Picking" : "Pick Wall In Viewport"))
	{
		m_bDestructionPickArmed = !m_bDestructionPickArmed;
		if (!m_bDestructionPickArmed)
			m_bDestructionAddMemberArmed = false;
		m_DestructionStatus = m_bDestructionPickArmed ?
			"Wall picking armed. Click a wall in the uncovered game viewport." :
			"Wall picking cancelled";
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Authoring Data"))
	{
		Reload_DestructionAuthoring();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_DestructionDocument.Is_Dirty());
	if (ImGui::Button("Save Pending Changes"))
		Save_WorldDestruction();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_DestructionDocument.Is_Dirty() ? "[unsaved]" : "[saved]");

	if (m_bDestructionPickArmed)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PICKING: click the world view, not this ImGui window. Escape cancels.");
	}
	ImGui::TextWrapped("%s", m_DestructionStatus.c_str());
	ImGui::Separator();

	if (ImGui::BeginTable("SimpleDestructionLayout", 2,
		ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("Walls", ImGuiTableColumnFlags_WidthFixed, 360.f);
		ImGui::TableSetupColumn("Selected Wall", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_DestructionSimpleWallList();
		ImGui::TableSetColumnIndex(1);
		Render_DestructionSimpleInspector();
		ImGui::EndTable();
	}
}

void Client::CMapTool::Render_DestructionSimpleWallList()
{
	ImGui::TextUnformatted("1. Choose a wall");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##SimpleWallFilter", "Search asset or placement ID",
		m_DestructionDeployFilter, IM_ARRAYSIZE(m_DestructionDeployFilter));
	ImGui::Checkbox("Only walls without a break setting",
		&m_bDestructionOnlyUnassigned);

	const std::vector<DEPLOY_RUNTIME_ENTRY>& entries =
		m_DeployRuntime.Get_Entries();
	const size_t destructibleCount = static_cast<size_t>(std::count_if(
		entries.begin(), entries.end(), [](const DEPLOY_RUNTIME_ENTRY& entry)
		{
			return entry.placement.destructible;
		}));
	ImGui::Text("%zu destructible walls loaded", destructibleCount);
	const f32_t height = (std::max)(360.f, ImGui::GetContentRegionAvail().y);
	if (!ImGui::BeginChild("SimpleWallRows", ImVec2(0.f, height), true))
	{
		ImGui::EndChild();
		return;
	}

	const std::string filter = m_DestructionDeployFilter;
	for (const DEPLOY_RUNTIME_ENTRY& entry : entries)
	{
		if (!entry.placement.destructible)
			continue;
		const DESTRUCTION_GROUP* owner =
			m_DestructionDocument.Find_GroupOfMember(
				entry.placement.runtimePlacementId);
		if (m_bDestructionOnlyUnassigned && nullptr != owner)
			continue;
		const std::string idText =
			std::to_string(entry.placement.runtimePlacementId);
		if (!filter.empty() &&
			!MatchesFilter(entry.placement.assetId, filter.c_str()) &&
			!MatchesFilter(idText, filter.c_str()))
		{
			continue;
		}

		const std::string label = std::string(nullptr != owner ? "[SET] " :
			"[NEW] ") + entry.placement.assetId + " / " + idText;
		const bool_t selected = m_iSelectedDeployPlacementId ==
			entry.placement.runtimePlacementId;
		ImGui::PushID(idText.c_str());
		if (ImGui::Selectable(label.c_str(), selected))
			Select_DestructionWall(entry.placement.runtimePlacementId, "wall list");
		if (nullptr != owner && ImGui::IsItemHovered())
			ImGui::SetTooltip("Group: %s", owner->groupId.c_str());
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_DestructionSimpleInspector()
{
	ImGui::TextUnformatted("2. Set when this wall breaks");
	const auto entry = std::find_if(
		m_DeployRuntime.Get_Entries().begin(),
		m_DeployRuntime.Get_Entries().end(),
		[this](const DEPLOY_RUNTIME_ENTRY& value)
		{
			return value.placement.runtimePlacementId ==
				m_iSelectedDeployPlacementId;
		});
	if (m_DeployRuntime.Get_Entries().end() == entry)
	{
		ImGui::TextWrapped(
			"Select a wall from the list or press Pick Wall In Viewport.");
		return;
	}

	const DEPLOY_PROP_ASSET_ENTRY* asset =
		m_DeployRuntime.Get_Catalog().Find(entry->placement.assetId);
	const DESTRUCTION_GROUP* owner =
		m_DestructionDocument.Find_GroupOfMember(
			entry->placement.runtimePlacementId);
	ImGui::SeparatorText("Selected Wall");
	ImGui::Text("Asset: %s", entry->placement.assetId.c_str());
	ImGui::Text("Placement: %llu",
		static_cast<unsigned long long>(entry->placement.runtimePlacementId));
	ImGui::Text("Position: %.2f, %.2f, %.2f",
		entry->placement.position.x, entry->placement.position.y,
		entry->placement.position.z);
	ImGui::Text("Source off action: %u | trigger evidence: %u",
		entry->placement.stateOffActionId,
		entry->placement.triggerBinaryOccurrenceCount);
	if (nullptr != asset)
	{
		ImGui::Text("Model: %s | fractured mesh: %s",
			DEPLOY_PROP_MODEL_KIND::ANIM == asset->kind ? "ANIM" : "STATIC",
			asset->fracturedRelativePath.empty() ? "NO" : "YES");
	}
	ImGui::Text("Break group: %s", nullptr != owner ?
		owner->groupId.c_str() : "not assigned (created automatically on Save)");
	if (nullptr != owner)
		ImGui::Text("Walls that break together: %zu",
			owner->memberPlacementIds.size());

	const char_t* previewScope = nullptr != owner ? "Group" : "Wall";
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" Original").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::INTACT);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" Broken").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::FRACTURED);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" Hidden").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::DESPAWNED);

	if (nullptr != owner)
	{
		if (!m_bDestructionAddMemberArmed)
		{
			if (ImGui::Button("Add Another Wall To This Group"))
			{
				m_bDestructionAddMemberArmed = true;
				m_bDestructionPickArmed = true;
				m_DestructionStatus =
					"Choose the next wall in the viewport or wall list";
			}
		}
		else
		{
			ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
				"Choose one more wall for group %s", owner->groupId.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Cancel Add"))
			{
				m_bDestructionAddMemberArmed = false;
				m_bDestructionPickArmed = false;
			}
		}
	}

	std::vector<const DESTRUCTION_BINDING*> groupBindings;
	if (nullptr != owner)
	{
		for (const DESTRUCTION_BINDING& binding :
			m_DestructionDocument.Get_Bindings())
		{
			const DESTRUCTION_MUTATION* mutation =
				m_DestructionDocument.Find_Mutation(binding.mutationId);
			if (nullptr != mutation && mutation->groupId == owner->groupId &&
				WORLD_DESTROYABLE_STATE::FRACTURED == mutation->eTargetState)
				groupBindings.push_back(&binding);
		}
	}
	if (!groupBindings.empty())
	{
		if (groupBindings.size() > 1u &&
			m_SelectedDestructionBindingId.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
				"This group has %zu break settings. Choose one explicitly.",
				groupBindings.size());
		}
		const char_t* preview = m_SelectedDestructionBindingId.empty() ?
			"Choose an existing setting" :
			m_SelectedDestructionBindingId.c_str();
		if (ImGui::BeginCombo("Existing Break Setting", preview))
		{
			for (const DESTRUCTION_BINDING* binding : groupBindings)
			{
				const bool_t selected = binding->bindingId ==
					m_SelectedDestructionBindingId;
				const std::string label = binding->patternId + " / " +
					binding->stageId + "##" + binding->bindingId;
				if (ImGui::Selectable(label.c_str(), selected))
					Load_DestructionDraftFromBinding(*binding);
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("New Setting"))
		{
			m_SelectedDestructionBindingId.clear();
			m_SelectedDestructionPatternId.clear();
			m_SelectedDestructionStageId.clear();
			m_iDestructionTriggerKind = 1;
			m_iDestructionOffsetMs = 0;
			m_iDestructionBreakingMs = 1900;
			m_bDestructionBindingEnabled = false;
			m_DestructionReceiverId[0] = '\0';
			m_fDestructionTimelineMs = 0.f;
			m_bDestructionTimelinePlaying = false;
			m_bDestructionNewSettingArmed = true;
		}
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	const char_t* patternPreview = nullptr != pattern ?
		pattern->displayName.c_str() : "Choose a Valtan pattern";
	if (ImGui::BeginCombo("Valtan Pattern", patternPreview))
	{
		for (const ENCOUNTER_PATTERN_REFERENCE& candidate :
			m_EncounterReference.Get_Patterns())
		{
			const bool_t selected = candidate.patternId ==
				m_SelectedDestructionPatternId;
			const std::string label = candidate.displayName + "##" +
				candidate.patternId;
			if (ImGui::Selectable(label.c_str(), selected))
			{
				m_SelectedDestructionPatternId = candidate.patternId;
				m_SelectedDestructionStageId = candidate.stages.empty() ? "" :
					candidate.stages.front().stageId;
				m_iDestructionOffsetMs = 0;
				m_fDestructionTimelineMs = 0.f;
				m_bDestructionTimelinePlaying = false;
			}
		}
		ImGui::EndCombo();
	}

	pattern = m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	const ENCOUNTER_STAGE_REFERENCE* stage = Find_SelectedDestructionStage();
	const char_t* stagePreview = nullptr != stage ?
		stage->stageId.c_str() : "Choose a stage";
	ImGui::BeginDisabled(nullptr == pattern);
	if (ImGui::BeginCombo("Animation Stage", stagePreview))
	{
		for (const ENCOUNTER_STAGE_REFERENCE& candidate : pattern->stages)
		{
			const bool_t selected = candidate.stageId ==
				m_SelectedDestructionStageId;
			const std::string label = candidate.stageId + " / " +
				candidate.stageKind + " / " + candidate.actionId;
			if (ImGui::Selectable(label.c_str(), selected))
			{
				m_SelectedDestructionStageId = candidate.stageId;
				m_iDestructionOffsetMs = 0;
				m_fDestructionTimelineMs =
					static_cast<f32_t>(candidate.iStartOffsetMs);
			}
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	stage = Find_SelectedDestructionStage();
	if (nullptr != stage)
	{
		ImGui::Text("Stage actionId: %s | %s | %u ms",
			stage->actionId.c_str(), stage->stageKind.c_str(),
			stage->iDurationMs);
		const char_t* genericClip = "patternRecovery";
		if ("WINDUP" == stage->stageKind)
			genericClip = "patternWindup";
		else if ("ACTIVE" == stage->stageKind)
			genericClip = "patternActive";
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Runtime currently resolves the generic %s clip; exact actionId clip is not authored.",
			genericClip);
	}

	Render_DestructionSimpleTimeline();
	ImGui::SeparatorText("Break Condition");
	if (ImGui::RadioButton("At stage start", 0 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("At selected time", 1 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("At stage end", 2 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 2;
	ImGui::SameLine();
	if (ImGui::RadioButton("On boss collision", 3 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 3;

	if (1 == m_iDestructionTriggerKind && nullptr != stage)
	{
		m_iDestructionOffsetMs = (std::clamp)(m_iDestructionOffsetMs, 0,
			static_cast<int32_t>(stage->iDurationMs));
		if (ImGui::SliderInt("Break point inside stage (ms)",
			&m_iDestructionOffsetMs, 0,
			static_cast<int32_t>(stage->iDurationMs)))
		{
			m_fDestructionTimelineMs = static_cast<f32_t>(
				stage->iStartOffsetMs + m_iDestructionOffsetMs);
		}
	}
	if (3 == m_iDestructionTriggerKind)
	{
		const char_t* receiverPreview = '\0' != m_DestructionReceiverId[0] ?
			m_DestructionReceiverId : "Choose a Collision Box";
		if (ImGui::BeginCombo("Boss Collision Receiver", receiverPreview))
		{
			for (const WORLD_GAMEPLAY_PLACEMENT& placement :
				m_WorldGameplayDocument.Get_Placements())
			{
				if (WORLD_PLACEMENT_KIND::COLLISION_BOX != placement.eKind)
					continue;
				if (ImGui::Selectable(placement.placementId.c_str(),
					placement.placementId == m_DestructionReceiverId))
				{
					strncpy_s(m_DestructionReceiverId,
						placement.placementId.c_str(), _TRUNCATE);
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::InputInt("Group breaking presentation duration (ms)",
		&m_iDestructionBreakingMs, 50, 250);
	m_iDestructionBreakingMs = (std::clamp)(m_iDestructionBreakingMs, 0,
		static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS));
	ImGui::Checkbox("Enable this break setting", &m_bDestructionBindingEnabled);
	if (!m_bDestructionBindingEnabled)
		ImGui::TextDisabled("Disabled is the safe authoring default until the Server path is published.");
	const bool_t navigationMissing = nullptr == owner ||
		owner->navigationRegionIds.empty();
	if (navigationMissing)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Navigation blocker region: not linked (use Advanced Graph Editor after authoring one)." );
		if (m_bDestructionBindingEnabled)
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"Enabled settings cannot be saved until a navigation region is linked.");
	}

	const bool_t settingChoiceRequired = groupBindings.size() > 1u &&
		m_SelectedDestructionBindingId.empty() &&
		!m_bDestructionNewSettingArmed;
	if (settingChoiceRequired)
		ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Choose an existing setting or press New Setting before applying.");
	const bool_t canSave = nullptr != pattern && nullptr != stage &&
		!settingChoiceRequired &&
		!(m_bDestructionBindingEnabled && navigationMissing);
	ImGui::BeginDisabled(!canSave);
	if (ImGui::Button("Apply And Save This Wall Setting"))
	{
		if (Apply_SimpleDestructionAuthoring())
			Save_WorldDestruction();
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"This writes Data authoring. Product Server destruction is still gated.");
}

void Client::CMapTool::Render_DestructionSimpleTimeline()
{
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern || 0u == pattern->iTotalDurationMs)
		return;

	ImGui::SeparatorText("Pattern Timing Preview");
	if (ImGui::Button(m_bDestructionTimelinePlaying ? "Pause" : "Play"))
		m_bDestructionTimelinePlaying = !m_bDestructionTimelinePlaying;
	ImGui::SameLine();
	if (ImGui::Button("Restart"))
	{
		m_fDestructionTimelineMs = 0.f;
		m_bDestructionTimelinePlaying = false;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_bDestructionTimelineLoop);

	const f32_t duration = static_cast<f32_t>(pattern->iTotalDurationMs);
	m_fDestructionTimelineMs = (std::clamp)(
		m_fDestructionTimelineMs, 0.f, duration);
	ImGui::SliderFloat("Pattern time", &m_fDestructionTimelineMs,
		0.f, duration, "%.0f ms");
	ImGui::SameLine();
	if (ImGui::Button("Use Current Time"))
		Use_DestructionTimelineTime();

	const f32_t width = (std::max)(220.f, ImGui::GetContentRegionAvail().x);
	constexpr f32_t barHeight = 28.f;
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
	{
		const f32_t left = origin.x + width *
			static_cast<f32_t>(stage.iStartOffsetMs) / duration;
		const f32_t right = origin.x + width *
			static_cast<f32_t>(stage.iStartOffsetMs + stage.iDurationMs) /
			duration;
		const bool_t selected = stage.stageId == m_SelectedDestructionStageId;
		const ImU32 fill = selected ? IM_COL32(65, 125, 205, 230) :
			("ACTIVE" == stage.stageKind ? IM_COL32(170, 85, 55, 210) :
				IM_COL32(65, 75, 100, 210));
		draw->AddRectFilled(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight), fill);
		draw->AddRect(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight),
			IM_COL32(220, 220, 220, 160));
		if (right - left > 48.f)
			draw->AddText(ImVec2(left + 4.f, origin.y + 6.f),
				IM_COL32(245, 245, 245, 255), stage.stageId.c_str());
	}
	const f32_t playheadX = origin.x + width *
		m_fDestructionTimelineMs / duration;
	draw->AddLine(ImVec2(playheadX, origin.y - 5.f),
		ImVec2(playheadX, origin.y + barHeight + 5.f),
		IM_COL32(255, 230, 70, 255), 2.f);
	ImGui::Dummy(ImVec2(width, barHeight + 10.f));
	ImGui::TextDisabled(
		"Timing preview only. It does not claim an exact Valtan animation clip.");
}

void Client::CMapTool::Render_DestructionGroupEditor()
{
	if (!ImGui::CollapsingHeader("Destruction Groups",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::BeginDisabled(!m_DestructionDocument.Is_Dirty());
	if (ImGui::Button("Save World Events"))
		Save_AllAuthoring();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Reload World Events"))
		Load_WorldDestruction();
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_DestructionDocument.Is_Dirty() ? "[unsaved]" : "[saved]");
	ImGui::TextWrapped("%s", m_DestructionStatus.c_str());
	ImGui::Separator();

	ImGui::SetNextItemWidth(320.f);
	ImGui::InputText("New group ID", m_DestructionGroupId,
		IM_ARRAYSIZE(m_DestructionGroupId));
	ImGui::SameLine();
	if (ImGui::Button("Create Group"))
	{
		std::string status;
		if (m_DestructionDocument.Add_Group(m_DestructionGroupId, status))
			m_SelectedDestructionGroupId = m_DestructionGroupId;
		m_DestructionStatus = status;
		Refresh_DestructionHighlight();
	}

	if (ImGui::BeginTable("DestructionGroupList", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 140.f)))
	{
		ImGui::TableSetupColumn("groupId");
		ImGui::TableSetupColumn("members");
		ImGui::TableSetupColumn("navPolarity");
		ImGui::TableSetupColumn("initial");
		ImGui::TableHeadersRow();
		for (const DESTRUCTION_GROUP& entry : m_DestructionDocument.Get_Groups())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t isSelected =
				m_SelectedDestructionGroupId == entry.groupId;
			if (ImGui::Selectable(entry.groupId.c_str(), isSelected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedDestructionGroupId = entry.groupId;
				const auto profile = std::find_if(
					m_DestructionSimulationDocument.Get_Profiles().begin(),
					m_DestructionSimulationDocument.Get_Profiles().end(),
					[&entry](const DESTRUCTION_SIMULATION_PROFILE& value)
					{
						return value.groupId == entry.groupId;
					});
				if (profile !=
					m_DestructionSimulationDocument.Get_Profiles().end())
				{
					Select_DestructionSimulationProfile(profile->profileId);
				}
				Refresh_DestructionHighlight();
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%zu", entry.memberPlacementIds.size());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldDestructionDocument::NavPolarity_ToString(
					entry.eNavPolarity));
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					entry.eInitialState));
		}
		ImGui::EndTable();
	}

	/* Picking starts from one wall. Preview affects its whole selected group
	   when a group exists, or the single wall before its first group exists. */
	ImGui::Separator();
	ImGui::Checkbox("Pick walls in the world", &m_bDestructionPickArmed);
	ImGui::SameLine();
	ImGui::TextDisabled("Left click a wall in the viewport; Escape cancels");
	ImGui::Text("picked: %llu",
		static_cast<unsigned long long>(m_iSelectedDeployPlacementId));
	ImGui::SameLine();
	ImGui::TextDisabled("| preview is presentation only, never saved");
	const char_t* previewScope = m_SelectedDestructionGroupId.empty() ?
		"Wall" : "Group";
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" INTACT").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::INTACT);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" FRACTURED").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::FRACTURED);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" DESPAWNED").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::DESPAWNED);

	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr == group)
	{
		ImGui::TextUnformatted(
			"Create a group above, then select it to add the picked wall.");
		return;
	}

	ImGui::Separator();
	ImGui::Text("Editing %s", group->groupId.c_str());
	if (ImGui::Button("Delete Group"))
	{
		CWorldDestructionDocument stagedDestruction = m_DestructionDocument;
		CDestructionSimulationDocument stagedSimulation =
			m_DestructionSimulationDocument;
		std::string status;
		const std::string removedGroupId = m_SelectedDestructionGroupId;
		if (stagedDestruction.Remove_Group(removedGroupId, status) &&
			stagedSimulation.Remove_ProfilesForGroup(removedGroupId, status) &&
			stagedSimulation.Validate_GroupReferences(
				stagedDestruction, status))
		{
			if (nullptr != m_pDestructionSimulationController)
				m_pDestructionSimulationController->Clear();
			m_DestructionDocument = std::move(stagedDestruction);
			m_DestructionSimulationDocument = std::move(stagedSimulation);
			Reset_DestructionSimulationUI();
			m_bDestructionSimulationClearRequested = false;
			m_SelectedDestructionGroupId.clear();
		}
		m_DestructionStatus = status;
		Refresh_DestructionHighlight();
		return;
	}

	/* The baked grid leaves every wall footprint walkable, so a wall blocks
	   while INTACT and a collapsing floor blocks while FRACTURED. */
	int32_t polarity = DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED ==
		group->eNavPolarity ? 1 : 0;
	ImGui::TextUnformatted("Navigation polarity");
	if (ImGui::RadioButton("Block while INTACT (wall)", 0 == polarity))
		polarity = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("Block while FRACTURED (floor)", 1 == polarity))
		polarity = 1;
	m_DestructionDocument.Set_NavPolarity(group->groupId,
		1 == polarity ? DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED :
		DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_INTACT);

	int32_t initialState =
		WORLD_DESTROYABLE_STATE::FRACTURED == group->eInitialState ? 1 : 0;
	ImGui::TextUnformatted("Initial state");
	if (ImGui::RadioButton("INTACT", 0 == initialState))
		initialState = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("FRACTURED", 1 == initialState))
		initialState = 1;
	m_DestructionDocument.Set_InitialState(group->groupId,
		1 == initialState ? WORLD_DESTROYABLE_STATE::FRACTURED :
		WORLD_DESTROYABLE_STATE::INTACT);

	ImGui::Separator();
	ImGui::BeginDisabled(0u == m_iSelectedDeployPlacementId);
	if (ImGui::Button("Add Picked Wall"))
	{
		if (Modify_DestructionGroupMember(
			m_iSelectedDeployPlacementId, true))
		{
			return;
		}
	}
	ImGui::EndDisabled();

	if (ImGui::BeginTable("DestructionMembers", 3,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 140.f)))
	{
		ImGui::TableSetupColumn("runtimePlacementId");
		ImGui::TableSetupColumn("assetId");
		ImGui::TableSetupColumn("");
		ImGui::TableHeadersRow();
		uint64_t removeRequest = 0u;
		for (const uint64_t placementId : group->memberPlacementIds)
		{
			ImGui::TableNextRow();
			ImGui::PushID(static_cast<int32_t>(placementId));
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%llu",
				static_cast<unsigned long long>(placementId));
			ImGui::TableSetColumnIndex(1);
			const shared_ptr<CDeployPropObject> prop =
				m_DeployRuntime.Find(placementId);
			ImGui::TextUnformatted(nullptr != prop ? "loaded" : "missing");
			ImGui::TableSetColumnIndex(2);
			if (ImGui::SmallButton("Remove"))
				removeRequest = placementId;
			ImGui::PopID();
		}
		ImGui::EndTable();
		if (0u != removeRequest &&
			Modify_DestructionGroupMember(removeRequest, false))
		{
			return;
		}
	}

	ImGui::SetNextItemWidth(280.f);
	ImGui::InputText("Nav region ID", m_DestructionRegionId,
		IM_ARRAYSIZE(m_DestructionRegionId));
	ImGui::SameLine();
	if (ImGui::Button("Link Region"))
	{
		std::string status;
		m_DestructionDocument.Add_NavigationRegion(
			group->groupId, m_DestructionRegionId, status);
		m_DestructionStatus = status;
	}
	std::string regionRemoveRequest;
	for (const std::string& regionId : group->navigationRegionIds)
	{
		ImGui::BulletText("%s", regionId.c_str());
		ImGui::SameLine();
		ImGui::PushID(regionId.c_str());
		if (ImGui::SmallButton("Unlink"))
			regionRemoveRequest = regionId;
		ImGui::PopID();
	}
	if (!regionRemoveRequest.empty())
	{
		m_DestructionDocument.Remove_NavigationRegion(
			m_SelectedDestructionGroupId, regionRemoveRequest);
	}

}

void Client::CMapTool::Render_DestructionBindingEditor()
{
	if (!ImGui::CollapsingHeader("Mutations and Bindings"))
		return;

	ImGui::SetNextItemWidth(300.f);
	ImGui::InputText("Mutation ID", m_DestructionMutationId,
		IM_ARRAYSIZE(m_DestructionMutationId));
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputInt("Breaking ms", &m_iDestructionBreakingMs, 50, 500);
	m_iDestructionBreakingMs = (std::clamp)(m_iDestructionBreakingMs, 0,
		static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS));
	ImGui::SameLine();
	ImGui::BeginDisabled(m_SelectedDestructionGroupId.empty());
	if (ImGui::Button("Create Mutation"))
	{
		DESTRUCTION_MUTATION mutation;
		mutation.mutationId = m_DestructionMutationId;
		mutation.groupId = m_SelectedDestructionGroupId;
		mutation.eTargetState = WORLD_DESTROYABLE_STATE::FRACTURED;
		mutation.iBreakingDurationMs =
			static_cast<uint32_t>(m_iDestructionBreakingMs);
		std::string status;
		m_DestructionDocument.Add_Mutation(mutation, status);
		m_DestructionStatus = status;
	}
	ImGui::EndDisabled();

	if (ImGui::BeginTable("DestructionMutations", 5,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("mutationId");
		ImGui::TableSetupColumn("groupId");
		ImGui::TableSetupColumn("target");
		ImGui::TableSetupColumn("breaking ms");
		ImGui::TableSetupColumn("");
		ImGui::TableHeadersRow();
		std::string removeRequest;
		for (const DESTRUCTION_MUTATION& entry :
			m_DestructionDocument.Get_Mutations())
		{
			ImGui::TableNextRow();
			ImGui::PushID(entry.mutationId.c_str());
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(entry.mutationId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(entry.groupId.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					entry.eTargetState));
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", entry.iBreakingDurationMs);
			ImGui::TableSetColumnIndex(4);
			if (ImGui::SmallButton("Remove"))
				removeRequest = entry.mutationId;
			ImGui::PopID();
		}
		ImGui::EndTable();
		if (!removeRequest.empty())
		{
			std::string status;
			m_DestructionDocument.Remove_Mutation(removeRequest, status);
			m_DestructionStatus = status;
		}
	}

	ImGui::Separator();
	ImGui::SetNextItemWidth(300.f);
	ImGui::InputText("Binding ID", m_DestructionBindingId,
		IM_ARRAYSIZE(m_DestructionBindingId));
	ImGui::TextUnformatted("Trigger kind");
	if (ImGui::RadioButton("STAGE_ENTER", 0 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("STAGE_TIME", 1 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("STAGE_EXIT", 2 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 2;
	ImGui::SameLine();
	if (ImGui::RadioButton("COLLISION_IMPACT", 3 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 3;

	ImGui::BeginDisabled(1 != m_iDestructionTriggerKind);
	ImGui::SetNextItemWidth(200.f);
	ImGui::InputInt("Offset ms", &m_iDestructionOffsetMs, 10, 100);
	m_iDestructionOffsetMs = (std::clamp)(m_iDestructionOffsetMs, 0,
		static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS));
	ImGui::EndDisabled();
	ImGui::BeginDisabled(3 != m_iDestructionTriggerKind);
	ImGui::SetNextItemWidth(300.f);
	ImGui::InputText("Receiver collision ID", m_DestructionReceiverId,
		IM_ARRAYSIZE(m_DestructionReceiverId));
	ImGui::EndDisabled();

	const bool_t canCreateBinding =
		!m_SelectedDestructionPatternId.empty() &&
		!m_SelectedDestructionStageId.empty() &&
		!m_DestructionDocument.Get_Mutations().empty();
	ImGui::BeginDisabled(!canCreateBinding);
	if (ImGui::Button("Create Binding for selected pattern stage"))
	{
		DESTRUCTION_BINDING binding;
		binding.bindingId = m_DestructionBindingId;
		binding.mutationId = m_DestructionMutationId;
		binding.patternId = m_SelectedDestructionPatternId;
		binding.stageId = m_SelectedDestructionStageId;
		binding.eTriggerKind =
			0 == m_iDestructionTriggerKind ?
				DESTRUCTION_TRIGGER_KIND::STAGE_ENTER :
			1 == m_iDestructionTriggerKind ?
				DESTRUCTION_TRIGGER_KIND::STAGE_TIME :
			2 == m_iDestructionTriggerKind ?
				DESTRUCTION_TRIGGER_KIND::STAGE_EXIT :
				DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT;
		binding.iOffsetMs = 1 == m_iDestructionTriggerKind ?
			static_cast<uint32_t>(m_iDestructionOffsetMs) : 0u;
		binding.receiverCollisionId = 3 == m_iDestructionTriggerKind ?
			m_DestructionReceiverId : "";
		binding.isEnabled = false;
		std::string status;
		m_DestructionDocument.Add_Binding(binding, status);
		m_DestructionStatus = status;
	}
	ImGui::EndDisabled();
	if (!canCreateBinding)
	{
		ImGui::TextDisabled(
			"Select a pattern stage above and create a mutation first.");
	}

	if (!ImGui::BeginTable("DestructionBindings", 7,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		return;
	}
	ImGui::TableSetupColumn("bindingId");
	ImGui::TableSetupColumn("pattern");
	ImGui::TableSetupColumn("stage");
	ImGui::TableSetupColumn("trigger");
	ImGui::TableSetupColumn("offset ms");
	ImGui::TableSetupColumn("enabled");
	ImGui::TableSetupColumn("");
	ImGui::TableHeadersRow();
	std::string bindingRemoveRequest;
	for (const DESTRUCTION_BINDING& entry :
		m_DestructionDocument.Get_Bindings())
	{
		ImGui::TableNextRow();
		ImGui::PushID(entry.bindingId.c_str());
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(entry.bindingId.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(entry.patternId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextUnformatted(entry.stageId.c_str());
		ImGui::TableSetColumnIndex(3);
		ImGui::TextUnformatted(
			CWorldDestructionDocument::TriggerKind_ToString(
				entry.eTriggerKind));
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%u", entry.iOffsetMs);
		ImGui::TableSetColumnIndex(5);
		bool_t enabled = entry.isEnabled;
		if (ImGui::Checkbox("##enabled", &enabled))
		{
			DESTRUCTION_BINDING updated = entry;
			updated.isEnabled = enabled;
			std::string status;
			m_DestructionDocument.Update_Binding(updated, status);
			m_DestructionStatus = status;
		}
		ImGui::TableSetColumnIndex(6);
		if (ImGui::SmallButton("Remove"))
			bindingRemoveRequest = entry.bindingId;
		ImGui::PopID();
	}
	ImGui::EndTable();
	if (!bindingRemoveRequest.empty())
		m_DestructionDocument.Remove_Binding(bindingRemoveRequest);
}

void Client::CMapTool::Render_DestructionTimeline()
{
	if (!ImGui::CollapsingHeader("Pattern Timeline",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern || 0u == pattern->iTotalDurationMs)
	{
		ImGui::TextUnformatted(
			"Select a pattern in Encounter Source to see its timeline.");
		return;
	}

	const f32_t width = (std::max)(240.f, ImGui::GetContentRegionAvail().x - 16.f);
	constexpr f32_t barHeight = 26.f;
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	const f32_t totalMs = static_cast<f32_t>(pattern->iTotalDurationMs);

	for (size_t index = 0; index < pattern->stages.size(); ++index)
	{
		const ENCOUNTER_STAGE_REFERENCE& stage = pattern->stages[index];
		const f32_t left = origin.x +
			width * static_cast<f32_t>(stage.iStartOffsetMs) / totalMs;
		const f32_t right = origin.x + width *
			static_cast<f32_t>(stage.iStartOffsetMs + stage.iDurationMs) /
			totalMs;
		const bool_t isActive = "ACTIVE" == stage.stageKind;
		const ImU32 fill = isActive ?
			IM_COL32(190, 90, 60, 210) : IM_COL32(70, 80, 110, 210);
		draw->AddRectFilled(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight), fill);
		draw->AddRect(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight),
			IM_COL32(220, 220, 220, 160));
		if (right - left > 44.f)
		{
			draw->AddText(ImVec2(left + 4.f, origin.y + 5.f),
				IM_COL32(240, 240, 240, 255), stage.stageId.c_str());
		}
		if (ImGui::IsMouseHoveringRect(ImVec2(left, origin.y),
			ImVec2(right, origin.y + barHeight)))
		{
			ImGui::SetTooltip("%s | %s | %u ms | tick %u",
				stage.stageId.c_str(), stage.stageKind.c_str(),
				stage.iDurationMs,
				CEncounterPatternReference::To_ServerTick(
					stage.iStartOffsetMs,
					m_EncounterReference.Get_FixedTickHz()));
		}
	}

	/* Draw every binding of this pattern as a marker at its absolute ms so the
	   author sees the requested moment against the real stage boundaries. */
	for (const DESTRUCTION_BINDING& binding :
		m_DestructionDocument.Get_Bindings())
	{
		if (binding.patternId != pattern->patternId)
			continue;
		const auto stage = std::find_if(
			pattern->stages.begin(), pattern->stages.end(),
			[&binding](const ENCOUNTER_STAGE_REFERENCE& value)
			{
				return value.stageId == binding.stageId;
			});
		if (pattern->stages.end() == stage)
			continue;
		uint32_t absoluteMs = stage->iStartOffsetMs;
		if (DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind)
			absoluteMs += (std::min)(binding.iOffsetMs, stage->iDurationMs);
		else if (DESTRUCTION_TRIGGER_KIND::STAGE_EXIT == binding.eTriggerKind)
			absoluteMs += stage->iDurationMs;
		const f32_t markerX = origin.x +
			width * static_cast<f32_t>(absoluteMs) / totalMs;
		const ImU32 color = binding.isEnabled ?
			IM_COL32(120, 255, 120, 255) : IM_COL32(255, 220, 90, 255);
		draw->AddLine(ImVec2(markerX, origin.y - 4.f),
			ImVec2(markerX, origin.y + barHeight + 4.f), color, 2.f);
	}

	ImGui::Dummy(ImVec2(width, barHeight + 10.f));
	ImGui::Text("total %u ms | %u Hz | green marker = enabled binding",
		pattern->iTotalDurationMs,
		m_EncounterReference.Get_FixedTickHz());

	ImGui::TextUnformatted("Stage for the next binding");
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
	{
		if (ImGui::RadioButton(stage.stageId.c_str(),
			m_SelectedDestructionStageId == stage.stageId))
		{
			m_SelectedDestructionStageId = stage.stageId;
			m_iDestructionOffsetMs = 0;
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

void Client::CMapTool::Render_WorldDestructionPanel(bool_t isAssetTest)
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	ImGui::Text("Area: %s",
		nullptr != descriptor ? descriptor->label.c_str() : "NONE");
	ImGui::SameLine();
	ImGui::TextUnformatted(
		"| Saves Data authoring only. The Server owns the real state.");
	ImGui::Separator();

	if (!isAssetTest)
	{
		ImGui::TextUnformatted(
			"Enter the Debug Map Editor workspace to inspect this Area.");
		return;
	}
	if (nullptr == descriptor || descriptor->encounterReference.empty())
	{
		ImGui::TextUnformatted(
			"This Area declares no destruction authoring source.");
		ImGui::TextUnformatted("Select Valtan in the workspace bar.");
		return;
	}

	if (ImGui::RadioButton("Easy Wall Editor", !m_bDestructionAdvancedMode))
		m_bDestructionAdvancedMode = false;
	ImGui::SameLine();
	if (ImGui::RadioButton("Advanced Graph Editor", m_bDestructionAdvancedMode))
		m_bDestructionAdvancedMode = true;
	ImGui::Separator();
	if (!m_bDestructionAdvancedMode)
	{
		Render_DestructionSimpleEditor();
		return;
	}

	Render_DestructionEncounterSource();
	Render_DestructionTimeline();
	Render_DestructionGroupEditor();
	Render_DestructionBindingEditor();
	Render_DestructionDeployList();
	Render_DestructionWorldRows();
	Render_DestructionNavigationRegions();
	Render_DestructionDiagnostics();
}

void Client::CMapTool::Render_DestructionSimulationWindow(
	const bool_t isAssetTest)
{
	if (!m_bOpen || !isAssetTest ||
		TOOL_MODE::WORLD_DESTRUCTION != m_eToolMode)
	{
		return;
	}
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->destructionSimulationDocument.empty())
		return;

	ImGui::SetNextWindowPos(ImVec2(450.f, 35.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(820.f, 760.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.f);
	if (!ImGui::Begin("Destruction Model View"))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Area %s | %zu profiles",
		descriptor->areaId.c_str(),
		m_DestructionSimulationDocument.Get_Profiles().size());
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Authoring preview only; Server state and collision are unchanged");
	ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.22f, 1.f),
		"PROJECT_AUTHORED destruction: %u PhysX pieces per wall",
		CDestructionSimulationRuntime::PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT);
	ImGui::TextWrapped(
		"DEPLOY_ITR_02316 uses 12 macro shards derived from every triangle of "
		"its exact fractured wall mesh. Other Deploy assets use the four generic "
		"Valtan stone fallbacks. Original chunk pivots and source particle %s "
		"were not recovered, so the flight remains PROJECT_AUTHORED.",
		CDestructionSimulationRuntime::PROJECT_AUTHORED_SOURCE_PARTICLE_ID);
	if (m_bDestructionDebrisPrototypesReady)
	{
		ImGui::TextColored(ImVec4(0.35f, 0.9f, 0.45f, 1.f), "%s",
			m_DestructionDebrisPrototypeStatus.c_str());
	}
	else
	{
		ImGui::TextColored(ImVec4(1.f, 0.35f, 0.3f, 1.f), "%s",
			m_DestructionDebrisPrototypeStatus.c_str());
	}
	ImGui::BeginDisabled(!m_DestructionSimulationDocument.Is_Dirty() ||
		m_bDestructionSimulationElementDraftDirty);
	if (ImGui::Button("Save Simulations"))
		Save_AllAuthoring();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Reload Simulations"))
		Load_DestructionSimulation();
	ImGui::SameLine();
	const bool_t hasSelectedDeployGroup =
		nullptr != m_DestructionDocument.Find_Group(
			m_SelectedDestructionGroupId);
	ImGui::BeginDisabled(!hasSelectedDeployGroup);
	if (ImGui::Button("Create Default for Selected Group"))
		Create_DefaultDestructionSimulationProfile();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_DestructionSimulationDocument.Is_Dirty() ? "[unsaved]" : "[saved]");
	ImGui::TextWrapped("%s", m_DestructionSimulationStatus.c_str());

	Render_DestructionSimulationTimeline();
	ImGui::Separator();
	if (ImGui::BeginTable("DestructionSimulationColumns", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Emitters / Fragments", ImGuiTableColumnFlags_WidthStretch,
			0.43f);
		ImGui::TableSetupColumn("Emitter / Fragment Detail", ImGuiTableColumnFlags_WidthStretch,
			0.57f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_DestructionSimulationOutliner();
		ImGui::TableSetColumnIndex(1);
		Render_DestructionSimulationDetail();
		ImGui::EndTable();
	}
	ImGui::End();
}

void Client::CMapTool::Render_DestructionSimulationTimeline()
{
	ImGui::SeparatorText("Physics Timeline");
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		Get_SelectedDestructionSimulationProfile();
	const bool_t hasProfile = nullptr != profile;
	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT snapshot =
		nullptr != m_pDestructionSimulationController ?
			m_pDestructionSimulationController->Get_Snapshot() :
			DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT{};

	ImGui::BeginDisabled(!hasProfile);
	if (ImGui::Button("Stage Selected (Paused)"))
		Request_StageDestructionSimulation(*profile, false, false);
	ImGui::SameLine();
	if (ImGui::Button("Play All Fragments"))
	{
		if (Request_StageDestructionSimulation(*profile, false, true))
		{
			m_pDestructionSimulationController->Request_SetScope(
				DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
		}
	}
	ImGui::SameLine();
	const bool_t isPlaying =
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING == snapshot.eState;
	if (ImGui::Button(isPlaying ? "Pause" : "Play"))
	{
		if (isPlaying)
			m_pDestructionSimulationController->Request_Pause();
		else if (DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED == snapshot.eState)
		{
			m_pDestructionSimulationController->Request_Reset();
			m_pDestructionSimulationController->Request_Play();
		}
		else if (m_pDestructionSimulationController->Get_Runtime().Is_Staged())
			m_pDestructionSimulationController->Request_Play();
		else
			Request_StageDestructionSimulation(*profile, false, true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Restart + Play"))
	{
		if (!m_pDestructionSimulationController->Get_Runtime().Is_Staged())
			Request_StageDestructionSimulation(*profile, false, true);
		else
		{
			m_pDestructionSimulationController->Request_Reset();
			m_pDestructionSimulationController->Request_Play();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Single 60 Hz Step"))
		m_pDestructionSimulationController->Request_SingleStep();
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_bDestructionSimulationLoop);

	const bool_t allDebris =
		DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == snapshot.eScope;
	const bool_t soloEmitter =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == snapshot.eScope;
	const bool_t soloFragment =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == snapshot.eScope;
	if (ImGui::RadioButton("All Fragments", allDebris))
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
	}
	ImGui::SameLine();
	const bool_t canSoloEmitter =
		!m_SelectedDestructionSimulationElementId.empty();
	ImGui::BeginDisabled(!canSoloEmitter);
	if (ImGui::RadioButton("Solo Emitter", soloEmitter))
	{
		if (hasProfile && AreEmittersAuthoredAsOneWall(*profile))
		{
			m_pDestructionSimulationController->Request_SetScope(
				DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
		}
		else
		{
			m_pDestructionSimulationController->Request_SetScope(
				DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
				m_SelectedDestructionSimulationElementId);
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool_t canSoloFragment =
		hasProfile && !m_SelectedDestructionSimulationFragmentId.empty();
	ImGui::BeginDisabled(!canSoloFragment);
	if (ImGui::RadioButton("Solo Fragment", soloFragment))
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
			m_SelectedDestructionSimulationFragmentId);
	}
	ImGui::EndDisabled();

	size_t fragmentCount = hasProfile ?
		profile->Elements.size() *
		CDestructionSimulationRuntime::PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT :
		0u;
	if (nullptr != m_pDestructionSimulationController)
	{
		const DESTRUCTION_SIMULATION_FRAME& frame =
			m_pDestructionSimulationController->Get_Runtime().Get_Frame();
		if (hasProfile && frame.profileId == profile->profileId)
		{
			fragmentCount = 0u;
			for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME& elementFrame :
				frame.Elements)
			{
				fragmentCount += elementFrame.Fragments.size();
			}
		}
	}
	ImGui::TextDisabled("Scope %s | %zu mesh emitters | %zu fragments",
		SimulationScopeLabel(snapshot.eScope),
		hasProfile ? profile->Elements.size() : 0u,
		fragmentCount);

	ImGui::Text("%s | %.3f / %.3f s | fixed %.3f ms",
		SimulationPlaybackStateLabel(snapshot.eState),
		snapshot.fSampleTimeSeconds,
		hasProfile ? profile->fDurationSeconds : snapshot.fDurationSeconds,
		CDestructionSimulationController::FIXED_DELTA_SECONDS * 1000.f);
	if (hasProfile)
	{
		f32_t sampleTime = (std::clamp)(snapshot.fSampleTimeSeconds,
			0.f, profile->fDurationSeconds);
		if (ImGui::SliderFloat("Sample Time", &sampleTime,
			0.f, profile->fDurationSeconds, "%.3f s"))
		{
			if (!m_pDestructionSimulationController->Get_Runtime().Is_Staged())
				Request_StageDestructionSimulation(*profile, false, false);
			m_pDestructionSimulationController->Request_Seek(sampleTime);
		}

		const f32_t width = (std::max)(240.f,
			ImGui::GetContentRegionAvail().x - 12.f);
		constexpr f32_t barHeight = 18.f;
		const ImVec2 origin = ImGui::GetCursorScreenPos();
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(origin,
			ImVec2(origin.x + width, origin.y + barHeight),
			IM_COL32(45, 52, 68, 210));
		for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile->Elements)
		{
			const f32_t triggerTime =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
					element.Trigger.eKind ? element.Trigger.fTimeSeconds : 0.f;
			const f32_t markerX = origin.x + width *
				triggerTime / profile->fDurationSeconds;
			const ImU32 markerColor =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
					element.Trigger.eKind ? IM_COL32(255, 95, 95, 255) :
				IM_COL32(120, 235, 150, 255);
			draw->AddLine(ImVec2(markerX, origin.y - 2.f),
				ImVec2(markerX, origin.y + barHeight + 2.f), markerColor, 2.f);
		}
		draw->AddLine(
			ImVec2(origin.x + width * sampleTime / profile->fDurationSeconds,
				origin.y - 4.f),
			ImVec2(origin.x + width * sampleTime / profile->fDurationSeconds,
				origin.y + barHeight + 4.f),
			IM_COL32(255, 220, 80, 255), 2.f);
		ImGui::Dummy(ImVec2(width, barHeight + 6.f));

		std::unordered_set<std::string> collisionReceivers;
		for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile->Elements)
		{
			if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
				element.Trigger.eKind &&
				!element.Trigger.receiverCollisionId.empty())
			{
				collisionReceivers.insert(element.Trigger.receiverCollisionId);
			}
		}
		for (const std::string& receiver : collisionReceivers)
		{
			ImGui::PushID(receiver.c_str());
			if (ImGui::SmallButton(("Fire Collision: " + receiver).c_str()))
				m_pDestructionSimulationController->Request_Collision(receiver);
			ImGui::PopID();
		}
	}
	ImGui::EndDisabled();
	if (nullptr != m_pDestructionSimulationController)
	{
		ImGui::TextDisabled("%s",
			m_pDestructionSimulationController->Get_Snapshot().status.c_str());
	}
}

void Client::CMapTool::Render_DestructionSimulationOutliner()
{
	ImGui::SeparatorText("Mesh Emitters / Fragments");
	ImGui::InputText("Filter", m_DestructionSimulationFilter,
		IM_ARRAYSIZE(m_DestructionSimulationFilter));
	const std::string filter = m_DestructionSimulationFilter;
	const DESTRUCTION_SIMULATION_FRAME* frame =
		nullptr != m_pDestructionSimulationController ?
			&m_pDestructionSimulationController->Get_Runtime().Get_Frame() : nullptr;

	if (ImGui::BeginChild("DestructionProfiles", ImVec2(0.f, 420.f), true))
	{
		for (const DESTRUCTION_SIMULATION_PROFILE& profile :
			m_DestructionSimulationDocument.Get_Profiles())
		{
			const bool_t profileSelected =
				profile.profileId == m_SelectedDestructionSimulationProfileId;
			const std::string profileLabel = profile.profileId + "##profile";
			const bool_t profileOpen = ImGui::TreeNodeEx(profileLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow |
				(profileSelected ? ImGuiTreeNodeFlags_DefaultOpen : 0) |
				(profileSelected ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				Select_DestructionSimulationProfile(profile.profileId);
				Refresh_DestructionHighlight();
			}
			if (!profileOpen)
				continue;

			ImGui::TextDisabled("group %s | %.2f s | %zu mesh emitters",
				profile.groupId.c_str(), profile.fDurationSeconds,
				profile.Elements.size());
			const bool_t linkedEmitters = AreEmittersAuthoredAsOneWall(profile);
			for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
			{
				/* Linked walls render once, under the first element's identity. */
				if (linkedEmitters && &element != &profile.Elements.front())
					continue;
				const DESTRUCTION_SIMULATION_ELEMENT_FRAME* runtimeElement = nullptr;
				if (nullptr != frame && frame->profileId == profile.profileId)
				{
					const auto found = std::find_if(
						frame->Elements.begin(), frame->Elements.end(),
						[&element](const DESTRUCTION_SIMULATION_ELEMENT_FRAME& value)
						{
							return value.elementId == element.elementId;
						});
					if (found != frame->Elements.end())
						runtimeElement = &*found;
				}

				bool_t matches = filter.empty() ||
					MatchesFilter(element.elementId, filter.c_str());
				if (!matches && nullptr != runtimeElement)
				{
					matches = std::any_of(
						runtimeElement->Fragments.begin(),
						runtimeElement->Fragments.end(),
						[&filter](const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& fragment)
						{
							return MatchesFilter(fragment.fragmentId, filter.c_str()) ||
								MatchesFilter(fragment.modelAssetId, filter.c_str());
						});
				}
				if (!matches)
					continue;

				ImGui::PushID(element.elementId.c_str());
				const bool_t elementSelected = profileSelected &&
					element.elementId == m_SelectedDestructionSimulationElementId;
				size_t linkedFragmentCount = 0u;
				if (linkedEmitters && nullptr != frame &&
					frame->profileId == profile.profileId)
				{
					for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME& linkedFrame :
						frame->Elements)
					{
						linkedFragmentCount += linkedFrame.Fragments.size();
					}
				}
				std::ostringstream emitterLabel;
				if (linkedEmitters)
				{
					emitterLabel << "Wall Emitters ("
						<< profile.Elements.size() << " walls";
					if (0u != linkedFragmentCount)
						emitterLabel << ": " << linkedFragmentCount;
					emitterLabel << ")";
				}
				else
				{
					emitterLabel << element.elementId << " (Emitter";
					if (nullptr != runtimeElement)
						emitterLabel << ": " << runtimeElement->Fragments.size();
					emitterLabel << ")";
				}
				emitterLabel << "##emitter";
				ImGuiTreeNodeFlags emitterFlags =
					ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_SpanAvailWidth;
				if (elementSelected)
					emitterFlags |= ImGuiTreeNodeFlags_Selected |
						ImGuiTreeNodeFlags_DefaultOpen;
				if (!filter.empty())
					emitterFlags |= ImGuiTreeNodeFlags_DefaultOpen;
				const bool_t emitterOpen = ImGui::TreeNodeEx(
					emitterLabel.str().c_str(), emitterFlags);
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					if (!profileSelected)
						Select_DestructionSimulationProfile(profile.profileId);
					Select_DestructionSimulationElement(element.elementId);
				}
				if (ImGui::IsItemHovered())
				{
					if (linkedEmitters)
					{
						ImGui::SetTooltip(
							"%zu linked walls authored as one\n"
							"velocity = direction x %.3f m/s",
							profile.Elements.size(),
							element.fSpeedMetersPerSecond);
					}
					else
					{
						ImGui::SetTooltip(
							"Deploy %llu\nvelocity = direction x %.3f m/s",
							static_cast<unsigned long long>(
								element.sourceRuntimePlacementId),
							element.fSpeedMetersPerSecond);
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(linkedEmitters ?
					"Play Linked Walls" : "Solo Emitter + Play"))
				{
					if (!profileSelected)
						Select_DestructionSimulationProfile(profile.profileId);
					Select_DestructionSimulationElement(element.elementId);
					const DESTRUCTION_SIMULATION_PROFILE* selected =
						Get_SelectedDestructionSimulationProfile();
					if (nullptr != selected)
					{
						const DESTRUCTION_SIMULATION_FRAME& activeFrame =
							m_pDestructionSimulationController->Get_Runtime().Get_Frame();
						const bool_t alreadyStaged =
							m_pDestructionSimulationController->Get_Runtime().Is_Staged() &&
							activeFrame.profileId == selected->profileId;
						if (alreadyStaged || Request_StageDestructionSimulation(
							*selected, false, true))
						{
							if (linkedEmitters)
							{
								m_pDestructionSimulationController->Request_SetScope(
									DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
							}
							else
							{
								m_pDestructionSimulationController->Request_SetScope(
									DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
									element.elementId);
							}
							m_pDestructionSimulationController->Request_Reset();
							m_pDestructionSimulationController->Request_Play();
							m_DestructionSimulationStatus = linkedEmitters ?
								"Linked wall audition requested: " + profile.profileId :
								"Solo mesh emitter audition requested: " +
									element.elementId;
						}
					}
				}

				if (emitterOpen)
				{
					if (nullptr == runtimeElement)
					{
						ImGui::TextDisabled(
							"Stage this profile to materialize its 12 fragment rows.");
					}
					else
					{
						std::vector<const DESTRUCTION_SIMULATION_ELEMENT_FRAME*>
							ownerFrames;
						if (linkedEmitters)
						{
							ownerFrames.reserve(frame->Elements.size());
							for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME& linkedFrame :
								frame->Elements)
							{
								ownerFrames.push_back(&linkedFrame);
							}
						}
						else
						{
							ownerFrames.push_back(runtimeElement);
						}
						for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME* ownerFrame :
							ownerFrames)
						{
						ImGui::TextDisabled(
								"Deploy %llu | %s | life %.2f",
								static_cast<unsigned long long>(
									ownerFrame->sourceRuntimePlacementId),
								SimulationElementStateLabel(ownerFrame->eState),
								ownerFrame->fNormalizedLife);
						for (const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& fragment :
							ownerFrame->Fragments)
						{
							if (!filter.empty() &&
								!MatchesFilter(fragment.fragmentId, filter.c_str()) &&
								!MatchesFilter(fragment.modelAssetId, filter.c_str()))
							{
								continue;
							}
							ImGui::PushID(fragment.fragmentId.c_str());
							const bool_t fragmentSelected = profileSelected &&
								fragment.fragmentId ==
									m_SelectedDestructionSimulationFragmentId;
							std::ostringstream fragmentLabel;
							fragmentLabel << fragment.fragmentId << " ["
								<< SimulationElementStateLabel(fragment.eState)
								<< " | life " << std::fixed << std::setprecision(2)
								<< fragment.fNormalizedLife << "]";
							const f32_t soloWidth = 88.f;
							const f32_t rowWidth = (std::max)(1.f,
								ImGui::GetContentRegionAvail().x - soloWidth);
							if (ImGui::Selectable(fragmentLabel.str().c_str(),
								fragmentSelected, 0, ImVec2(rowWidth, 0.f)))
							{
								Select_DestructionSimulationFragment(
									ownerFrame->elementId, fragment.fragmentId);
							}
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip("Piece %u\n%s",
									fragment.pieceIndex,
									fragment.modelAssetId.c_str());
							}
							ImGui::SameLine();
							if (ImGui::SmallButton("Solo + Play"))
							{
								Select_DestructionSimulationFragment(
									ownerFrame->elementId, fragment.fragmentId);
								if (m_SelectedDestructionSimulationFragmentId ==
									fragment.fragmentId)
								{
									m_pDestructionSimulationController->Request_SetScope(
										DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
										fragment.fragmentId);
									m_pDestructionSimulationController->Request_Reset();
									m_pDestructionSimulationController->Request_Play();
									m_DestructionSimulationStatus =
										"Solo fragment audition requested: " +
										fragment.fragmentId;
								}
							}
							ImGui::PopID();
						}
						}
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	ImGui::EndChild();
	ImGui::TextDisabled(
		"Profile = complete destruction; source wall = one 12-piece mesh emitter");
}

void Client::CMapTool::Render_DestructionSimulationDetail()
{
	ImGui::SeparatorText("Emitter / Fragment Detail");
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		Get_SelectedDestructionSimulationProfile();
	if (nullptr == profile ||
		!m_DestructionSimulationElementDraft.has_value())
	{
		ImGui::TextDisabled("Select one debris element in All Debris.");
		return;
	}

	DESTRUCTION_SIMULATION_ELEMENT& draft =
		*m_DestructionSimulationElementDraft;
	if (!m_SelectedDestructionSimulationFragmentId.empty())
	{
		const DESTRUCTION_SIMULATION_FRAGMENT_FRAME* selectedFragment = nullptr;
		if (nullptr != m_pDestructionSimulationController)
		{
			const DESTRUCTION_SIMULATION_FRAME& frame =
				m_pDestructionSimulationController->Get_Runtime().Get_Frame();
			if (frame.profileId == profile->profileId)
			{
				const auto runtimeElement = std::find_if(
					frame.Elements.begin(), frame.Elements.end(),
					[this](const DESTRUCTION_SIMULATION_ELEMENT_FRAME& value)
					{
						return value.elementId ==
							m_SelectedDestructionSimulationElementId;
					});
				if (runtimeElement != frame.Elements.end())
				{
					const auto fragment = std::find_if(
						runtimeElement->Fragments.begin(),
						runtimeElement->Fragments.end(),
						[this](const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& value)
						{
							return value.fragmentId ==
								m_SelectedDestructionSimulationFragmentId;
						});
					if (fragment != runtimeElement->Fragments.end())
						selectedFragment = &*fragment;
				}
			}
		}

		ImGui::SeparatorText("Selected Fragment (Read-only Runtime Sample)");
		if (nullptr == selectedFragment)
		{
			ImGui::TextDisabled(
				"The selected fragment is unavailable. Stage the selected profile again.");
		}
		else
		{
			const f32_t speed = std::sqrt(
				selectedFragment->vLinearVelocity.x *
					selectedFragment->vLinearVelocity.x +
				selectedFragment->vLinearVelocity.y *
					selectedFragment->vLinearVelocity.y +
				selectedFragment->vLinearVelocity.z *
					selectedFragment->vLinearVelocity.z);
			ImGui::TextWrapped("Stable ID: %s",
				selectedFragment->fragmentId.c_str());
			ImGui::Text("Piece index: %u", selectedFragment->pieceIndex);
			ImGui::TextWrapped("Model: %s",
				selectedFragment->modelAssetId.c_str());
			ImGui::Text("State: %s | life %.3f",
				SimulationElementStateLabel(selectedFragment->eState),
				selectedFragment->fNormalizedLife);
			ImGui::Text("Position: %.3f, %.3f, %.3f",
				selectedFragment->vWorldPosition.x,
				selectedFragment->vWorldPosition.y,
				selectedFragment->vWorldPosition.z);
			ImGui::Text("Velocity: %.3f, %.3f, %.3f m/s | speed %.3f",
				selectedFragment->vLinearVelocity.x,
				selectedFragment->vLinearVelocity.y,
				selectedFragment->vLinearVelocity.z,
				speed);
			if (ImGui::Button("Solo + Play Selected Fragment"))
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
					selectedFragment->fragmentId);
				m_pDestructionSimulationController->Request_Reset();
				m_pDestructionSimulationController->Request_Play();
				m_DestructionSimulationStatus =
					"Solo fragment audition requested: " +
					selectedFragment->fragmentId;
			}
			ImGui::TextDisabled(
				"Fragment values are generated deterministically from the emitter authoring below.");
		}
	}

	const bool_t linkedEmitters = AreEmittersAuthoredAsOneWall(*profile);
	if (linkedEmitters)
	{
		ImGui::SeparatorText(
			"Emitter Authoring (applies to every linked wall)");
		ImGui::TextWrapped("%zu walls authored as one emitter",
			profile->Elements.size());
		for (const DESTRUCTION_SIMULATION_ELEMENT& linked : profile->Elements)
		{
			ImGui::BulletText("%s | Deploy %llu",
				linked.elementId.c_str(),
				static_cast<unsigned long long>(
					linked.sourceRuntimePlacementId));
		}
	}
	else
	{
		ImGui::SeparatorText(
			"Emitter Authoring (applies to all 12 fragments)");
		ImGui::TextWrapped("%s", draft.elementId.c_str());
		ImGui::Text("Source Deploy placement: %llu",
			static_cast<unsigned long long>(draft.sourceRuntimePlacementId));
	}
	ImGui::TextDisabled(
		"These are authored simulation semantics. PhysX receives the derived velocity and gravity policy.");

	bool_t changed = false;
	changed |= ImGui::DragFloat3("Spawn Offset", &draft.vSpawnOffset.x,
		0.01f, -CDestructionSimulationDocument::MAX_ABSOLUTE_OFFSET,
		CDestructionSimulationDocument::MAX_ABSOLUTE_OFFSET, "%.3f");
	float3_t editedDirection = draft.vDirection;
	const bool_t directionChanged = ImGui::DragFloat3(
		"Direction (World Unit)", &editedDirection.x,
		0.01f, -1.f, 1.f, "%.4f");
	if (directionChanged)
	{
		if (!NormalizeSimulationDirection(editedDirection))
		{
			m_DestructionSimulationStatus =
				"Direction cannot be zero; the previous preview was preserved";
		}
		else
		{
			draft.vDirection = editedDirection;
			changed = true;
		}
	}
	changed |= ImGui::DragFloat("Speed (m/s)",
		&draft.fSpeedMetersPerSecond, 0.05f, 0.f,
		CDestructionSimulationDocument::MAX_SPEED_METERS_PER_SECOND,
		"%.3f", ImGuiSliderFlags_AlwaysClamp);
	changed |= ImGui::DragFloat("Gravity Scale",
		&draft.fGravityScale, 0.01f, 0.f,
		CDestructionSimulationDocument::MAX_GRAVITY_SCALE,
		"%.3f", ImGuiSliderFlags_AlwaysClamp);
	f32_t maximumLifetime = profile->fDurationSeconds;
	if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
		draft.Trigger.eKind)
	{
		maximumLifetime = (std::max)(
			CDestructionSimulationController::FIXED_DELTA_SECONDS,
			profile->fDurationSeconds - draft.Trigger.fTimeSeconds);
	}
	changed |= ImGui::DragFloat("Lifetime (s)",
		&draft.fLifetimeSeconds, 0.01f,
		CDestructionSimulationController::FIXED_DELTA_SECONDS,
		maximumLifetime, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);

	const float3_t velocity = {
		draft.vDirection.x * draft.fSpeedMetersPerSecond,
		draft.vDirection.y * draft.fSpeedMetersPerSecond,
		draft.vDirection.z * draft.fSpeedMetersPerSecond
	};
	ImGui::TextDisabled("Derived velocity: %.3f, %.3f, %.3f m/s",
		velocity.x, velocity.y, velocity.z);

	const char_t* triggerLabels[] = {
		"Immediate", "Timeline Time", "Collision Impact" };
	int32_t triggerKind =
		static_cast<int32_t>(draft.Trigger.eKind);
	if (ImGui::Combo("Trigger Condition", &triggerKind,
		triggerLabels, IM_ARRAYSIZE(triggerLabels)))
	{
		draft.Trigger.eKind =
			static_cast<DESTRUCTION_SIMULATION_TRIGGER_KIND>(triggerKind);
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME !=
			draft.Trigger.eKind)
		{
			draft.Trigger.fTimeSeconds = 0.f;
		}
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT !=
			draft.Trigger.eKind)
		{
			draft.Trigger.receiverCollisionId.clear();
			m_DestructionSimulationReceiverId[0] = '\0';
		}
		changed = true;
	}
	if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
		draft.Trigger.eKind)
	{
		const f32_t maximumTriggerTime = (std::max)(0.f,
			profile->fDurationSeconds - draft.fLifetimeSeconds);
		changed |= ImGui::DragFloat("Trigger Time (s)",
			&draft.Trigger.fTimeSeconds, 0.01f, 0.f,
			maximumTriggerTime, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
	}
	else if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
		draft.Trigger.eKind)
	{
		if (ImGui::InputText("Receiver Collision ID",
			m_DestructionSimulationReceiverId,
			IM_ARRAYSIZE(m_DestructionSimulationReceiverId)))
		{
			draft.Trigger.receiverCollisionId =
				m_DestructionSimulationReceiverId;
			changed = true;
		}
		if (!draft.Trigger.receiverCollisionId.empty() &&
			ImGui::Button("Fire This Collision"))
		{
			m_pDestructionSimulationController->Request_Collision(
				draft.Trigger.receiverCollisionId);
		}
	}

	if (changed)
	{
		m_bDestructionSimulationElementDraftDirty = true;
		m_DestructionSimulationStatus =
			"Live Detail draft staged; Apply commits it to the authoring document";
		Stage_DestructionElementDraftPreview();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bDestructionSimulationElementDraftDirty);
	if (ImGui::Button("Apply Detail"))
	{
		std::string status;
		const bool_t wasPlaying =
			DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING ==
				m_pDestructionSimulationController->Get_Snapshot().eState;
		/* Copy the identities first: Update_Element mutates the document that
		   `profile` points into. Each linked wall keeps its own elementId,
		   source placement and suppression aliases; only the authored
		   simulation values are shared. */
		bool_t committed = false;
		if (linkedEmitters)
		{
			std::vector<DESTRUCTION_SIMULATION_ELEMENT> linkedElements =
				profile->Elements;
			committed = true;
			for (DESTRUCTION_SIMULATION_ELEMENT& linked : linkedElements)
			{
				linked.vSpawnOffset = draft.vSpawnOffset;
				linked.vDirection = draft.vDirection;
				linked.fSpeedMetersPerSecond = draft.fSpeedMetersPerSecond;
				linked.fGravityScale = draft.fGravityScale;
				linked.fLifetimeSeconds = draft.fLifetimeSeconds;
				linked.Trigger = draft.Trigger;
				if (!m_DestructionSimulationDocument.Update_Element(
					m_SelectedDestructionSimulationProfileId, linked, status))
				{
					committed = false;
					break;
				}
			}
		}
		else
		{
			committed = m_DestructionSimulationDocument.Update_Element(
				m_SelectedDestructionSimulationProfileId, draft, status);
		}
		if (committed)
		{
			m_bDestructionSimulationElementDraftDirty = false;
			m_DestructionSimulationStatus = status + "; Save required";
			const DESTRUCTION_SIMULATION_PROFILE* committed =
				Get_SelectedDestructionSimulationProfile();
			if (nullptr != committed)
				Request_StageDestructionSimulation(
					*committed, true, wasPlaying);
		}
		else
		{
			m_DestructionSimulationStatus = status;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Detail"))
	{
		const DESTRUCTION_SIMULATION_ELEMENT* saved =
			m_DestructionSimulationDocument.Find_Element(
				m_SelectedDestructionSimulationProfileId,
				m_SelectedDestructionSimulationElementId);
		if (nullptr != saved)
		{
			m_DestructionSimulationElementDraft = *saved;
			m_bDestructionSimulationElementDraftDirty = false;
			strncpy_s(m_DestructionSimulationReceiverId,
				saved->Trigger.receiverCollisionId.c_str(), _TRUNCATE);
			Request_StageDestructionSimulation(*profile, true, false);
			m_DestructionSimulationStatus =
				"Reverted debris Detail to the saved authoring value";
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button(linkedEmitters ?
		"Play Linked Walls" : "Solo Emitter + Play"))
	{
		const bool_t staged = m_bDestructionSimulationElementDraftDirty ?
			Stage_DestructionElementDraftPreview() :
			Request_StageDestructionSimulation(*profile, false, true);
		if (staged)
		{
			if (linkedEmitters)
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
			}
			else
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
					draft.elementId);
			}
			m_pDestructionSimulationController->Request_Reset();
			m_pDestructionSimulationController->Request_Play();
		}
	}
	if (m_bDestructionSimulationElementDraftDirty)
		ImGui::TextDisabled("Detail draft is local until Apply Detail.");
}

void Client::CMapTool::Render_DestructionEncounterSource()
{
	if (!ImGui::CollapsingHeader("Encounter Source",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	if (ImGui::Button("Reload Encounter Reference"))
		Load_EncounterReference();
	ImGui::SameLine();
	ImGui::TextWrapped("%s", m_EncounterReferenceStatus.c_str());
	if (!m_EncounterReference.Is_Ready())
		return;

	const uint32_t fixedTickHz = m_EncounterReference.Get_FixedTickHz();
	ImGui::Text("Encounter %s | Boss %s | %u Hz | %zu patterns",
		m_EncounterReference.Get_EncounterId().c_str(),
		m_EncounterReference.Get_BossArchetypeId().c_str(),
		fixedTickHz,
		m_EncounterReference.Get_Patterns().size());

	/* The boss placement of this Area must reference the same encounter this
	   reference document declares, otherwise the two authoring layers drifted. */
	const WORLD_GAMEPLAY_PLACEMENT* bossPlacement = nullptr;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		m_WorldGameplayDocument.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::BOSS == placement.eKind)
		{
			bossPlacement = &placement;
			break;
		}
	}
	if (nullptr == bossPlacement)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Gameplay document has no boss placement to cross-check.");
	}
	else if (bossPlacement->encounterId !=
		m_EncounterReference.Get_EncounterId())
	{
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f),
			"Boss placement %s references %s, not %s.",
			bossPlacement->placementId.c_str(),
			bossPlacement->encounterId.c_str(),
			m_EncounterReference.Get_EncounterId().c_str());
	}

	if (ImGui::BeginTable("DestructionPatterns", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 180.f)))
	{
		ImGui::TableSetupColumn("patternId");
		ImGui::TableSetupColumn("HP bar");
		ImGui::TableSetupColumn("stages");
		ImGui::TableSetupColumn("total ms");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_PATTERN_REFERENCE& entry :
			m_EncounterReference.Get_Patterns())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t isSelected =
				m_SelectedDestructionPatternId == entry.patternId;
			if (ImGui::Selectable(entry.patternId.c_str(), isSelected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedDestructionPatternId = entry.patternId;
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%u", entry.iTriggerHealthBar);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%zu", entry.stages.size());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", entry.iTotalDurationMs);
		}
		ImGui::EndTable();
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern)
	{
		ImGui::TextUnformatted("Select a pattern to inspect its stages.");
		return;
	}

	ImGui::Text("%s | actionId %s | selection %s",
		pattern->displayName.c_str(),
		pattern->actionId.c_str(),
		pattern->selectionMode.c_str());
	if (ImGui::BeginTable("DestructionStages", 7,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("stageId");
		ImGui::TableSetupColumn("kind");
		ImGui::TableSetupColumn("start ms");
		ImGui::TableSetupColumn("duration ms");
		ImGui::TableSetupColumn("start tick");
		ImGui::TableSetupColumn("hitShape");
		ImGui::TableSetupColumn("actionId");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(stage.stageId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(stage.stageKind.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%u", stage.iStartOffsetMs);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", stage.iDurationMs);
			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%u", CEncounterPatternReference::To_ServerTick(
				stage.iStartOffsetMs, fixedTickHz));
			ImGui::TableSetColumnIndex(5);
			ImGui::TextUnformatted(stage.hitShape.c_str());
			ImGui::TableSetColumnIndex(6);
			ImGui::TextUnformatted(stage.actionId.c_str());
		}
		ImGui::EndTable();
	}
}

void Client::CMapTool::Render_DestructionDeployList()
{
	if (!ImGui::CollapsingHeader("Deploy Props",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	const std::vector<DEPLOY_RUNTIME_ENTRY>& entries =
		m_DeployRuntime.Get_Entries();
	ImGui::Text("Loaded placements: %zu | Runtime: %s",
		entries.size(),
		m_DeployRuntime.Is_Loaded() ? "READY" : "NOT LOADED");
	ImGui::SetNextItemWidth(220.f);
	ImGui::InputText("Asset filter", m_DestructionDeployFilter,
		IM_ARRAYSIZE(m_DestructionDeployFilter));
	ImGui::SameLine();
	ImGui::Checkbox("Only rows with stateOffActionId",
		&m_bDestructionOnlyWithOffAction);

	if (!ImGui::BeginTable("DestructionDeployRows", 6,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 240.f)))
	{
		return;
	}
	ImGui::TableSetupColumn("runtimePlacementId");
	ImGui::TableSetupColumn("assetId");
	ImGui::TableSetupColumn("stateOff");
	ImGui::TableSetupColumn("trig");
	ImGui::TableSetupColumn("position");
	ImGui::TableSetupColumn("state");
	ImGui::TableHeadersRow();

	const std::string filter = m_DestructionDeployFilter;
	for (const DEPLOY_RUNTIME_ENTRY& entry : entries)
	{
		if (m_bDestructionOnlyWithOffAction &&
			0u == entry.placement.stateOffActionId)
		{
			continue;
		}
		if (!filter.empty() &&
			std::string::npos == entry.placement.assetId.find(filter))
		{
			continue;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		const std::string idText =
			std::to_string(entry.placement.runtimePlacementId);
		const bool_t isSelected = m_iSelectedDeployPlacementId ==
			entry.placement.runtimePlacementId;
		if (ImGui::Selectable(idText.c_str(), isSelected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			Select_DestructionWall(
				entry.placement.runtimePlacementId, "advanced deploy list");
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(entry.placement.assetId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%u", entry.placement.stateOffActionId);
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%u", entry.placement.triggerBinaryOccurrenceCount);
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%.2f, %.2f, %.2f",
			entry.placement.position.x,
			entry.placement.position.y,
			entry.placement.position.z);
		ImGui::TableSetColumnIndex(5);
		ImGui::TextUnformatted(nullptr != entry.object ? "SPAWNED" : "NONE");
	}
	ImGui::EndTable();
}

void Client::CMapTool::Render_DestructionWorldRows()
{
	if (!ImGui::CollapsingHeader("World Gameplay destroyable rows"))
		return;

	size_t destroyableCount = 0;
	if (ImGui::BeginTable("DestructionWorldRows", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("placementId");
		ImGui::TableSetupColumn("deployRuntimePlacementId");
		ImGui::TableSetupColumn("initialState");
		ImGui::TableSetupColumn("enabled");
		ImGui::TableHeadersRow();
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			m_WorldGameplayDocument.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::DESTROYABLE != placement.eKind)
				continue;
			++destroyableCount;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(placement.placementId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%llu",
				static_cast<unsigned long long>(
					placement.deployRuntimePlacementId));
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					placement.eInitialState));
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(placement.isEnabled ? "true" : "false");
		}
		ImGui::EndTable();
	}
	ImGui::Text("destroyable rows: %zu", destroyableCount);
	ImGui::TextWrapped(
		"The Client document parser accepts this kind, but "
		"Publish-WorldGameplay.ps1 rejects every kind outside "
		"playerSpawn/npc/boss/triggerBox/collisionBox. G5 opens that gate.");
}

void Client::CMapTool::Render_DestructionNavigationRegions()
{
	if (!ImGui::CollapsingHeader("Navigation blocker regions"))
		return;

	ImGui::Text("Document: %s | regions: %zu",
		m_RuntimeBlockerDocument.Is_Ready() ? "READY" : "NOT READY",
		m_RuntimeBlockerDocument.Get_RegionCount());
	if (0 == m_RuntimeBlockerDocument.Get_RegionCount())
	{
		ImGui::TextWrapped(
			"No region is authored yet. The baked grid treats every wall "
			"footprint as walkable, so an INTACT wall has to add a blocker "
			"rather than remove one.");
		return;
	}

	if (!ImGui::BeginTable("DestructionNavRegions", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		return;
	}
	ImGui::TableSetupColumn("regionId");
	ImGui::TableSetupColumn("conditionId");
	ImGui::TableSetupColumn("activateWhenTrue");
	ImGui::TableSetupColumn("cells");
	ImGui::TableHeadersRow();
	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region)
			continue;
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(region->id.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(region->conditionId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextUnformatted(
			region->activateWhenConditionTrue ? "true" : "false");
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%u",
			m_RuntimeBlockerDocument.Get_RegionCellCount(index));
	}
	ImGui::EndTable();
}

void Client::CMapTool::Render_DestructionDiagnostics()
{
	if (!ImGui::CollapsingHeader("Diagnostics"))
		return;

	ImGui::TextWrapped(
		"Measured on 2026-08-07. These are locked notes, not runtime state.");
	ImGui::BulletText(
		"Server and Shared contain no destroyable state, message or receiver.");
	ImGui::BulletText(
		"CDeployPropRuntime::Set_State has no product caller; only the dead "
		"Set_DeployPhase calls Set_State_All.");
	ImGui::BulletText(
		"All 85 wall footprints are walkable in the published navgrid, so "
		"navigation must block while INTACT and open while FRACTURED.");
	ImGui::BulletText(
		"Arena floor collapse is the opposite polarity of a wall and must not "
		"share one boolean.");
	ImGui::BulletText(
		"CServerNavigation exposes only Load/Find_Path/Project_Point and "
		"cannot mutate the grid after load.");
	ImGui::BulletText(
		"CPlayerSkillSystem keeps the raw X/Z after Project_Point, so skill "
		"movement passes through any blocker until G10.");
	ImGui::BulletText(
		"DEPLOY_ITR_02326 has no fractured mesh and its ao_off clip holds "
		"still for 1.833s then scales to zero at 1.900s.");
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
}

void Client::CMapTool::Render_CameraPanel()
{
	ImGui::TextUnformatted("Player Camera");
	ImGui::Separator();

	const shared_ptr<CCamera_Free> camera =
		m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		ImGui::TextUnformatted("ASSET_TEST camera is unavailable.");
		if (ImGui::Button("Find Camera"))
			Find_AssetTestCamera();
		ImGui::TextWrapped("%s", m_CameraStatus.c_str());
		return;
	}

	ImGui::Text(
		"Mode: %s",
		camera->Is_FollowEnabled() ?
			"Follow Player" :
			"Free Camera");
	ImGui::SameLine();
	if (camera->Is_FollowEnabled())
	{
		if (ImGui::Button("Switch to Free Camera"))
		{
			camera->Set_FollowEnabled(false);
			m_CameraStatus = "Free Camera";
		}
	}
	else
	{
		if (ImGui::Button("Follow Player"))
		{
			camera->Set_FollowEnabled(true);
			m_CameraStatus = "Following Player";
		}
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!camera->Is_FollowEnabled());
	float3_t positionOffset = camera->Get_PositionOffset();
	if (ImGui::DragFloat3(
		"Position Offset",
		&positionOffset.x,
		0.1f,
		-100.f,
		100.f,
		"%.2f"))
	{
		camera->Set_PositionOffset(positionOffset);
		m_CameraStatus = "Position Offset applied";
	}
	ImGui::EndDisabled();
	if (!camera->Is_FollowEnabled())
		ImGui::TextDisabled("Follow Player mode previews Position Offset");

	if (!camera->Is_FollowEnabled())
	{
		ImGui::Text(
			"Mouse Look: %s",
			camera->Is_MouseLookEnabled() ? "Enabled" : "Locked");
	}

	ImGui::TextUnformatted(m_CameraStatus.c_str());
	ImGui::TextDisabled(
		"Tab: toggle Free Camera mouse look");
	ImGui::TextDisabled(
		"Free Camera: WASD move");
}

void Client::CMapTool::Render_NavigationPanel()
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	if (nullptr == active ||
		EDITOR_NAVIGATION_POLICY::NONE == active->navigationPolicy)
	{
		ImGui::TextDisabled(
			"Navigation authoring is disabled for this Area.");
		return;
	}

	if (ImGui::RadioButton(
		"Bake",
		NAVIGATION_MODE::BAKE == m_eNavigationMode))
	{
		m_eNavigationMode = NAVIGATION_MODE::BAKE;
		m_bNavigationStrokeActive = false;
	}
	ImGui::SameLine();
	const bool_t navigationReady =
		m_NavigationDocument.Is_Ready() &&
		m_RuntimeBlockerDocument.Is_Ready();
	ImGui::BeginDisabled(!navigationReady);
	if (ImGui::RadioButton(
		"Walkability",
		NAVIGATION_MODE::WALKABILITY == m_eNavigationMode))
	{
		m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Destruction Area",
		NAVIGATION_MODE::DESTRUCTION_AREA ==
		m_eNavigationMode))
	{
		m_eNavigationMode =
			NAVIGATION_MODE::DESTRUCTION_AREA;
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::EndDisabled();

	if (NAVIGATION_MODE::BAKE == m_eNavigationMode)
	{
		ImGui::Separator();
		Render_NavigationBakeControls();
		return;
	}

	if (!navigationReady)
	{
		ImGui::Separator();
		ImGui::TextUnformatted(
			"Navigation source is unavailable. Bake Nav Bounds first.");
		if (ImGui::Button("Retry Load"))
			Load_NavigationDocument();
		ImGui::TextWrapped("%s", m_NavigationStatus.c_str());
		return;
	}

	ImGui::Separator();

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
		Render_DestructionAreaControls();

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		if (ImGui::RadioButton(
			"Add Cells",
			NAVIGATION_EDIT_ACTION::APPLY ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::APPLY;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(
			"Remove Cells",
			NAVIGATION_EDIT_ACTION::ERASE ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::ERASE;
		}
	}
	else
	{
		if (ImGui::RadioButton(
			"Block",
			NAVIGATION_EDIT_ACTION::APPLY ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::APPLY;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(
			"Force Walkable",
			NAVIGATION_EDIT_ACTION::FORCE_WALKABLE ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::FORCE_WALKABLE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(
			"Reset",
			NAVIGATION_EDIT_ACTION::ERASE ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::ERASE;
		}
		ImGui::TextDisabled(
			"Force Walkable overrides baked blocked cells. Reset restores the baked state.");
	}

	int32_t brushRadius =
		static_cast<int32_t>(m_iBrushRadius);
	if (ImGui::SliderInt(
		"Brush",
		&brushRadius,
		0,
		static_cast<int32_t>(
			CNavGridPaintDocument::MAX_BRUSH_RADIUS)))
	{
		m_iBrushRadius =
			static_cast<uint32_t>(brushRadius);
	}

	ImGui::Checkbox(
		"Show empty cells",
		&m_bShowUnresolvedCells);

	ImGui::Separator();
	if (ImGui::Button("Save Navigation"))
		Save_Navigation();
	ImGui::SameLine();

	const bool_t dirty =
		m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty();
	ImGui::TextUnformatted(
		dirty ? "Unsaved" : m_NavigationStatus.c_str());

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		ImGui::TextDisabled(
			"Magenta: selected destruction area");
	}
	else
	{
		ImGui::TextDisabled(
			"Cell %.2f x %.2f | Green: walkable | Yellow: blocked",
			m_NavigationDocument.Get_Desc().cellSize,
			m_NavigationDocument.Get_Desc().cellSize);
	}

	Render_NavigationDiagnostics();
}

void Client::CMapTool::Render_DestructionAreaControls()
{
	const NAV_RUNTIME_BLOCKER_REGION* selectedRegion =
		m_RuntimeBlockerDocument.Get_Region(
			m_iSelectedRuntimeRegion);
	const char* preview =
		nullptr != selectedRegion ?
		selectedRegion->id.c_str() :
		"<none>";

	ImGui::SetNextItemWidth(320.f);
	if (ImGui::BeginCombo("Region", preview))
	{
		for (size_t index = 0;
			index < m_RuntimeBlockerDocument.Get_RegionCount();
			++index)
		{
			const NAV_RUNTIME_BLOCKER_REGION* region =
				m_RuntimeBlockerDocument.Get_Region(index);
			if (nullptr == region)
				continue;

			const bool_t selected =
				index == m_iSelectedRuntimeRegion;
			if (ImGui::Selectable(
				region->id.c_str(),
				selected))
			{
				m_iSelectedRuntimeRegion = index;
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("New"))
		ImGui::OpenPopup("New Destruction Area");

	if (ImGui::BeginPopupModal(
		"New Destruction Area",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText(
			"Name",
			m_RuntimeBlockerId,
			sizeof(m_RuntimeBlockerId));

		if (ImGui::RadioButton(
			"Block after destruction",
			m_RuntimeActivateWhenConditionTrue))
		{
			m_RuntimeActivateWhenConditionTrue = true;
		}
		if (ImGui::RadioButton(
			"Open after destruction",
			!m_RuntimeActivateWhenConditionTrue))
		{
			m_RuntimeActivateWhenConditionTrue = false;
		}

		if (ImGui::CollapsingHeader("Advanced"))
		{
			ImGui::InputText(
				"Condition",
				m_RuntimeConditionId,
				sizeof(m_RuntimeConditionId));
		}

		const bool_t canCreate =
			'\0' != m_RuntimeBlockerId[0] &&
			'\0' != m_RuntimeConditionId[0];
		ImGui::BeginDisabled(!canCreate);
		if (ImGui::Button("Create") &&
			m_RuntimeBlockerDocument.Add_Region(
				m_RuntimeBlockerId,
				m_RuntimeConditionId,
				m_RuntimeActivateWhenConditionTrue,
				m_NavigationStatus))
		{
			m_iSelectedRuntimeRegion =
				m_RuntimeBlockerDocument.Get_RegionCount() - 1;
			m_NavigationStatus = "Unsaved";
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	selectedRegion = m_RuntimeBlockerDocument.Get_Region(
		m_iSelectedRuntimeRegion);
	if (nullptr != selectedRegion &&
		ImGui::CollapsingHeader("Test"))
	{
		bool_t conditionValue =
			m_NavigationConditions[selectedRegion->conditionId];
		if (ImGui::Checkbox(
			"Destroyed",
			&conditionValue))
		{
			if (!Set_NavigationCondition(
				selectedRegion->conditionId,
				conditionValue))
			{
				m_NavigationStatus =
					"Re-enter ASSET_TEST before testing this region.";
			}
		}
	}
}

void Client::CMapTool::Render_NavigationDiagnostics()
{
	if (!ImGui::CollapsingHeader("Diagnostics"))
		return;

	const uint32_t surfaceCells =
		m_NavigationDocument.Get_ResolvedHeightCount();
	ImGui::Text(
		"Surface: %u | Excluded: %u | Blocked overrides: %u | Walkable overrides: %u",
		surfaceCells,
		m_NavigationDocument.Get_CellCount() - surfaceCells,
		m_NavigationDocument.Get_BlockedCount(),
		m_NavigationDocument.Get_ForcedWalkableCount());
	ImGui::TextWrapped(
		"Source: %s",
		m_NavigationSourcePath.string().c_str());
	ImGui::TextWrapped(
		"Paint: %s",
		m_NavigationPaintPath.string().c_str());
	ImGui::TextWrapped(
		"Runtime: %s",
		m_NavigationRuntimePath.string().c_str());
	ImGui::TextWrapped(
		"Blockers: %s",
		m_RuntimeBlockerPath.string().c_str());

	if (ImGui::Button("Reload from Disk"))
	{
		const bool_t dirty =
			m_NavigationDocument.Is_Dirty() ||
			m_RuntimeBlockerDocument.Is_Dirty();
		if (dirty)
			ImGui::OpenPopup("Discard Navigation Changes?");
		else
			Load_NavigationDocument();
	}

	if (ImGui::BeginPopupModal(
		"Discard Navigation Changes?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Reload and discard unsaved navigation changes?");
		if (ImGui::Button("Discard and Reload"))
		{
			Load_NavigationDocument();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Client::CMapTool::Render_NavigationOverlay()
{
	if (!m_NavigationDocument.Is_Ready() ||
		nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const float4_t green(0.1f, 1.f, 0.2f, 1.f);
	const float4_t yellow(1.f, 0.85f, 0.05f, 1.f);
	const float4_t magenta(1.f, 0.15f, 0.85f, 1.f);
	const NAVGRID_AUTHORING_DESC& desc =
		m_NavigationDocument.Get_Desc();
	const f32_t halfCell = desc.cellSize * 0.5f;

	resources.pBatch->Begin();
	for (uint32_t index = 0;
		index < m_NavigationDocument.Get_CellCount();
		++index)
	{
		const NAVGRID_AUTHORING_CELL_STATE state =
			m_NavigationDocument.Get_CellState(index);
		if (!m_bShowUnresolvedCells &&
			NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE == state)
		{
			continue;
		}

		const uint32_t cellX = index % desc.width;
		const uint32_t cellZ = index / desc.width;
		const f32_t worldX =
			desc.originX +
			(static_cast<f32_t>(cellX) + 0.5f) * desc.cellSize;
		const f32_t worldZ =
			desc.originZ +
			(static_cast<f32_t>(cellZ) + 0.5f) * desc.cellSize;
		if (!Is_CellInsideNavigationBounds(worldX, worldZ))
			continue;

		const f32_t displayHeight =
			NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE == state ?
			desc.bake.position.y - desc.bake.size.y * 0.5f :
			m_NavigationDocument.Get_CellHeight(index);
		const float3_t center(
			worldX,
			displayHeight + 0.08f,
			worldZ);

		const bool_t isSelectedRuntimeCell =
			NAVIGATION_MODE::DESTRUCTION_AREA ==
				m_eNavigationMode &&
			m_RuntimeBlockerDocument.Is_CellInRegion(
				m_iSelectedRuntimeRegion,
				index);
		const float4_t& color = isSelectedRuntimeCell ?
			magenta :
			NAVGRID_AUTHORING_CELL_STATE::WALKABLE != state ?
			yellow :
			green;
		const VertexPositionColor leftTop(
			float3_t(
				center.x - halfCell,
				center.y,
				center.z + halfCell),
			color);
		const VertexPositionColor rightTop(
			float3_t(
				center.x + halfCell,
				center.y,
				center.z + halfCell),
			color);
		const VertexPositionColor rightBottom(
			float3_t(
				center.x + halfCell,
				center.y,
				center.z - halfCell),
			color);
		const VertexPositionColor leftBottom(
			float3_t(
				center.x - halfCell,
				center.y,
				center.z - halfCell),
			color);

		resources.pBatch->DrawLine(leftTop, rightTop);
		resources.pBatch->DrawLine(rightTop, rightBottom);
		resources.pBatch->DrawLine(rightBottom, leftBottom);
		resources.pBatch->DrawLine(leftBottom, leftTop);
	}
	resources.pBatch->End();
}

void Client::CMapTool::Render_Toolbar()
{
	if (ImGui::Button("Save"))
		Save_Placements();
	ImGui::TextDisabled("Data/Maps authoring; publish required");
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		if (Load_Placements())
			Load_DeployProps();
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
					/* Mirror parity가 바뀌면 기존 batch pass와 달라지므로
					   안전하게 standalone으로 이동하고 Reload 때 재배치한다. */
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

bool_t Client::CMapTool::Try_PlaceNavigationBounds()
{
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) ||
		!std::isfinite(picked.x) ||
		!std::isfinite(picked.y) ||
		!std::isfinite(picked.z))
	{
		m_NavigationBakeStatus =
			"No rendered surface was picked for Nav Bounds";
		return false;
	}

	m_NavigationBakeDesc.position =
		float3_t(
			picked.x,
			picked.y + m_NavigationBakeDesc.size.y * 0.5f,
			picked.z);
	m_NavigationBakeDesc.isReady = true;
	m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_NavigationBakeStatus =
		"Nav Bounds placed; adjust Transform and press Bake";
	return true;
	//기존 Grid 범위를 초기 Bounds로 사용한다
}

bool_t Client::CMapTool::Collect_NavigationBakePlacements(
	std::vector<NAVGRID_BAKE_PLACEMENT>& outPlacements,
	std::string& outStatus) const
{
	outPlacements.clear();
	size_t skippedInstances = 0;
	if (!m_Catalog.Is_Ready())
	{
		outStatus = "Map asset catalog is unavailable";
		return false;
	}

	const f32_t radians =
		XMConvertToRadians(m_NavigationBakeDesc.yawDegrees);
	const f32_t cosine = std::abs(std::cos(radians));
	const f32_t sine = std::abs(std::sin(radians));
	const float3_t halfExtents(
		cosine * m_NavigationBakeDesc.size.x * 0.5f +
			sine * m_NavigationBakeDesc.size.z * 0.5f,
		m_NavigationBakeDesc.size.y * 0.5f,
		sine * m_NavigationBakeDesc.size.x * 0.5f +
			cosine * m_NavigationBakeDesc.size.z * 0.5f);
	const float3_t minimum(
		m_NavigationBakeDesc.position.x - halfExtents.x,
		m_NavigationBakeDesc.position.y - halfExtents.y,
		m_NavigationBakeDesc.position.z - halfExtents.z);
	const float3_t maximum(
		m_NavigationBakeDesc.position.x + halfExtents.x,
		m_NavigationBakeDesc.position.y + halfExtents.y,
		m_NavigationBakeDesc.position.z + halfExtents.z);

	std::unordered_map<std::string, shared_ptr<CModel>> modelCache;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		bool_t visible = entry.record.visible;
		if (entry.record.sourceLevel == "VALTAN_PHASE_SPACEHOLE")
		{
			visible =
				m_EnvironmentPhase != ENVIRONMENT_PHASE::BASELINE;
		}
		else if (entry.record.sourceLevel ==
			"VALTAN_PHASE_CHAOSGATE")
		{
			visible =
				m_EnvironmentPhase == ENVIRONMENT_PHASE::CHAOS_GATE;
		}
		if (!visible)
			continue;

		const MAP_ASSET_ENTRY* asset =
			m_Catalog.Find(entry.record.assetId);
		if (nullptr == asset ||
			!IsBatchEligible(*asset) ||
			asset->groupLabel == "LV_NAVIMESH" ||
			std::string::npos != asset->id.find("CUL_BOX"))
		{
			continue;
		}

		auto model = modelCache.find(asset->id);
		if (model == modelCache.end())
		{
			shared_ptr<CModel> cloned =
				dynamic_pointer_cast<CModel>(
					CGameInstance::Get().Clone_Prototype(
						m_iAuthoringLevelIndex,
						asset->prototypeTag));
			if (nullptr == cloned || !cloned->Has_LocalBounds())
			{
				outStatus =
					"Could not inspect model bounds for " +
					asset->id;
				return false;
			}
			model = modelCache.emplace(
				asset->id,
				std::move(cloned)).first;
		}

		FMapStaticInstance instance{};
		if (FAILED(BuildStaticInstance(
			*asset,
			model->second,
			entry.record,
			instance)))
		{
			++skippedInstances;
			continue;
		}

		const f32_t closestX = (std::clamp)(
			instance.WorldBoundsCenter.x,
			minimum.x,
			maximum.x);
		const f32_t closestY = (std::clamp)(
			instance.WorldBoundsCenter.y,
			minimum.y,
			maximum.y);
		const f32_t closestZ = (std::clamp)(
			instance.WorldBoundsCenter.z,
			minimum.z,
			maximum.z);
		const f32_t offsetX =
			instance.WorldBoundsCenter.x - closestX;
		const f32_t offsetY =
			instance.WorldBoundsCenter.y - closestY;
		const f32_t offsetZ =
			instance.WorldBoundsCenter.z - closestZ;
		const f32_t distanceSquared =
			offsetX * offsetX +
			offsetY * offsetY +
			offsetZ * offsetZ;
		if (distanceSquared >
			instance.WorldBoundsRadius *
				instance.WorldBoundsRadius)
		{
			continue;
		}

		NAVGRID_BAKE_PLACEMENT bakePlacement;
		bakePlacement.assetId = asset->id;
		bakePlacement.modelPath = asset->resolvedModelPath;
		bakePlacement.world = instance.World;
		outPlacements.push_back(std::move(bakePlacement));
	}

	if (outPlacements.empty())
	{
		outStatus =
			"No visible deferred map meshes overlap Nav Bounds; skipped " +
			std::to_string(skippedInstances) + " unbuildable instances";
		return false;
	}
	outStatus = std::to_string(outPlacements.size()) +
		" bake meshes; skipped " + std::to_string(skippedInstances) +
		" unbuildable instances";
	return true;
}

bool_t Client::CMapTool::Bake_Navigation()
{
	if (!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc))
	{
		m_NavigationBakeStatus =
			"Nav Bounds or bake settings are invalid";
		return false;
	}

	NAVGRID_AUTHORING_DESC nextDesc;
	std::string status;
	if (!CNavGridBaker::Build_Desc(
		m_Catalog.Get_AreaId(),
		m_NavigationBakeDesc,
		nextDesc,
		status))
	{
		m_NavigationBakeStatus = status;
		return false;
	}

	const bool_t hasCurrentNavigation =
		m_NavigationDocument.Is_Ready();

	const NAVGRID_AUTHORING_DESC* currentDesc =
		hasCurrentNavigation ?
		&m_NavigationDocument.Get_Desc() :
		nullptr;

	const bool_t layoutChanged =
		nullptr == currentDesc ||
		currentDesc->areaId != nextDesc.areaId ||
		currentDesc->width != nextDesc.width ||
		currentDesc->height != nextDesc.height ||
		std::abs(currentDesc->cellSize - nextDesc.cellSize) >
			0.000001f ||
		std::abs(currentDesc->originX - nextDesc.originX) >
			0.000001f ||
		std::abs(currentDesc->originZ - nextDesc.originZ) >
			0.000001f;

	const bool_t hasAuthoredCells =
		hasCurrentNavigation &&
		(0 != m_NavigationDocument.Get_BlockedCount() ||
			0 != m_RuntimeBlockerDocument.Get_RegionCount());

	if (layoutChanged &&
		hasAuthoredCells &&
		!m_bNavigationBakeResetConfirmed)
	{
		m_bNavigationBakeResetPending = true;
		m_NavigationBakeStatus =
			"Grid layout changed; confirm reset of paint and regions";
		return false;
	}

	std::vector<NAVGRID_BAKE_PLACEMENT> placements;
	if (!Collect_NavigationBakePlacements(placements, status))
	{
		m_NavigationBakeStatus = status;
		return false;
	}

	NAVGRID_BAKE_RESULT result;
	if (!CNavGridBaker::Build(
		m_Catalog.Get_AreaId(),
		m_NavigationBakeDesc,
		placements,
		result,
		status))
	{
		m_NavigationBakeStatus = status;
		return false;
	}

	struct FILE_BACKUP final
	{
		std::filesystem::path path;
		std::filesystem::path backup;
		bool_t existed = false;
	};
	std::array<FILE_BACKUP, 3> backups =
	{{
		{ m_NavigationSourcePath, {}, false },
		{ m_NavigationPaintPath, {}, false },
		{ m_RuntimeBlockerPath, {}, false },
	}};

	auto cleanupBackups = [&backups]()
	{
		for (const FILE_BACKUP& file : backups)
		{
			if (file.backup.empty())
				continue;
			std::error_code error;
			std::filesystem::remove(file.backup, error);
		}
	};
	auto restoreBackups = [&backups]()
	{
		bool_t restored = true;
		for (const FILE_BACKUP& file : backups)
		{
			if (file.path.empty())
				continue;
			std::error_code error;
			if (file.existed)
			{
				std::filesystem::copy_file(
					file.backup,
					file.path,
					std::filesystem::copy_options::overwrite_existing,
					error);
			}
			else
			{
				std::filesystem::remove(file.path, error);
			}
			restored = !error && restored;
		}
		return restored;
	};

	for (FILE_BACKUP& file : backups)
	{
		if (file.path.empty())
			continue;
		file.backup = file.path;
		file.backup += L".bakebak";
		std::error_code error;
		std::filesystem::remove(file.backup, error);
		error.clear();
		file.existed =
			std::filesystem::exists(file.path, error);
		if (error)
		{
			cleanupBackups();
			m_NavigationBakeStatus =
				"Could not inspect existing navigation files";
			return false;
		}
		if (file.existed)
		{
			std::filesystem::copy_file(
				file.path,
				file.backup,
				std::filesystem::copy_options::overwrite_existing,
				error);
			if (error)
			{
				cleanupBackups();
				m_NavigationBakeStatus =
					"Could not create navigation rollback backup";
				return false;
			}
		}
	}

	if (!CNavGridBaker::Save_Source(
		result,
		m_NavigationSourcePath,
		status))
	{
		cleanupBackups();
		m_NavigationBakeStatus = status;
		return false;
	}

	if (layoutChanged)
	{
		std::error_code paintError;
		if (!m_NavigationPaintPath.empty())
		{
			std::filesystem::remove(
				m_NavigationPaintPath,
				paintError);
		}
		std::error_code blockerError;
		if (!m_RuntimeBlockerPath.empty())
		{
			std::filesystem::remove(
				m_RuntimeBlockerPath,
				blockerError);
		}
		if (paintError || blockerError)
		{
			const bool_t restored = restoreBackups();
			cleanupBackups();
			m_NavigationBakeStatus = restored ?
				"Could not reset incompatible navigation authoring files" :
				"Navigation rollback failed after reset error";
			return false;
		}
	}

	if (!Load_NavigationDocument())
	{
		const bool_t restored = restoreBackups();
		cleanupBackups();
		const bool_t reloaded =
			restored && Load_NavigationDocument();
		m_NavigationBakeStatus = reloaded ?
			"Bake validation failed; previous navigation was restored" :
			"Bake validation failed and navigation rollback did not reload";
		return false;
	}

	cleanupBackups();
	m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
	m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_NavigationBakeStatus = status;
	m_NavigationStatus =
		"Baked source; Save Navigation to persist authoring paint";
	return true;
}

void Client::CMapTool::Render_NavigationBakeControls()
{
	if (ImGui::Button("Place Nav Bounds"))
	{
		m_eNavigationBoundsState = NAV_BOUNDS_STATE::PLACING;
		m_bNavigationBakeResetConfirmed = false;
		m_bNavigationBakeResetPending = false;
		m_NavigationBakeStatus =
			"Click a rendered floor to place Nav Bounds";
	}
	if (NAV_BOUNDS_STATE::PLACING == m_eNavigationBoundsState)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("Picking... Esc cancels");
	}

	ImGui::SeparatorText("Nav Bounds");
	bool_t changed = false;
	f32_t positionXZ[2] =
	{
		m_NavigationBakeDesc.position.x,
		m_NavigationBakeDesc.position.z,
	};
	if (ImGui::DragFloat2("Position XZ", positionXZ, 0.1f))
	{
		m_NavigationBakeDesc.position.x = positionXZ[0];
		m_NavigationBakeDesc.position.z = positionXZ[1];
		changed = true;
	}

	f32_t bottomY =
		m_NavigationBakeDesc.position.y -
		m_NavigationBakeDesc.size.y * 0.5f;
	if (ImGui::DragFloat("Bottom Y", &bottomY, 0.1f))
	{
		m_NavigationBakeDesc.position.y =
			bottomY + m_NavigationBakeDesc.size.y * 0.5f;
		changed = true;
	}

	f32_t sizeXZ[2] =
	{
		m_NavigationBakeDesc.size.x,
		m_NavigationBakeDesc.size.z,
	};
	if (ImGui::DragFloat2(
		"Size XZ",
		sizeXZ,
		0.1f,
		0.1f,
		10000.f))
	{
		m_NavigationBakeDesc.size.x = sizeXZ[0];
		m_NavigationBakeDesc.size.z = sizeXZ[1];
		changed = true;
	}

	f32_t height = m_NavigationBakeDesc.size.y;
	if (ImGui::DragFloat(
		"Height from Bottom",
		&height,
		0.1f,
		0.1f,
		10000.f))
	{
		m_NavigationBakeDesc.size.y = height;
		m_NavigationBakeDesc.position.y = bottomY + height * 0.5f;
		changed = true;
	}
	ImGui::Text(
		"Top Y: %.3f",
		bottomY + m_NavigationBakeDesc.size.y);
	changed =
		ImGui::DragFloat(
			"Yaw",
			&m_NavigationBakeDesc.yawDegrees,
			0.25f,
			-360.f,
			360.f) ||
		changed;

	ImGui::SeparatorText("Bake Settings");
	changed =
		ImGui::DragFloat(
			"Cell Size",
			&m_NavigationBakeDesc.cellSize,
			0.01f,
			0.05f,
			10.f) ||
		changed;
	changed =
		ImGui::DragFloat(
			"Max Slope",
			&m_NavigationBakeDesc.maxSlopeDegrees,
			0.25f,
			0.f,
			89.f) ||
		changed;
	if (changed)
	{
		m_bNavigationBakeResetConfirmed = false;
		m_bNavigationBakeResetPending = false;
		m_NavigationBakeStatus =
			m_NavigationBakeDesc.isReady ?
			"Needs Bake" :
			"Place Nav Bounds first";
	}

	ImGui::BeginDisabled(
		!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc) ||
		NAV_BOUNDS_STATE::PLACING == m_eNavigationBoundsState);
	if (ImGui::Button("Bake##NavigationBakeCommand"))
		Bake_Navigation();
	ImGui::EndDisabled();

	if (m_bNavigationBakeResetPending)
	{
		ImGui::SameLine();
		if (ImGui::Button("Confirm Reset and Rebake"))
		{
			m_bNavigationBakeResetConfirmed = true;
			Bake_Navigation();
		}
		ImGui::TextDisabled(
			"Grid coordinates changed. Manual blockers and runtime regions will reset.");
	}

	ImGui::TextWrapped(
		"%s",
		m_NavigationBakeStatus.c_str());
	ImGui::TextDisabled(
		"White: Nav Bounds | Bake uses visible static map meshes");
}

void Client::CMapTool::Render_NavigationBoundsOverlay()
{
	if (!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc) ||
		nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const f32_t radians =
		XMConvertToRadians(m_NavigationBakeDesc.yawDegrees);
	const f32_t cosine = std::cos(radians);
	const f32_t sine = std::sin(radians);
	const float3_t half(
		m_NavigationBakeDesc.size.x * 0.5f,
		m_NavigationBakeDesc.size.y * 0.5f,
		m_NavigationBakeDesc.size.z * 0.5f);
	const std::array<float3_t, 8> local =
	{{
		{ -half.x, -half.y, -half.z },
		{ half.x, -half.y, -half.z },
		{ half.x, -half.y, half.z },
		{ -half.x, -half.y, half.z },
		{ -half.x, half.y, -half.z },
		{ half.x, half.y, -half.z },
		{ half.x, half.y, half.z },
		{ -half.x, half.y, half.z },
	}};
	std::array<VertexPositionColor, 8> vertices;
	const float4_t white(1.f, 1.f, 1.f, 1.f);
	for (size_t index = 0; index < local.size(); ++index)
	{
		const float3_t world(
			m_NavigationBakeDesc.position.x +
				cosine * local[index].x +
				sine * local[index].z,
			m_NavigationBakeDesc.position.y +
				local[index].y,
			m_NavigationBakeDesc.position.z -
				sine * local[index].x +
				cosine * local[index].z);
		vertices[index] =
			VertexPositionColor(world, white);
	}

	constexpr std::array<std::array<uint32_t, 2>, 12> edges =
	{{
		{{ 0, 1 }}, {{ 1, 2 }}, {{ 2, 3 }}, {{ 3, 0 }},
		{{ 4, 5 }}, {{ 5, 6 }}, {{ 6, 7 }}, {{ 7, 4 }},
		{{ 0, 4 }}, {{ 1, 5 }}, {{ 2, 6 }}, {{ 3, 7 }},
	}};
	resources.pBatch->Begin();
	for (const auto& edge : edges)
	{
		resources.pBatch->DrawLine(
			vertices[edge[0]],
			vertices[edge[1]]);
	}
	resources.pBatch->End();
}

bool_t Client::CMapTool::Is_CellInsideNavigationBounds(
	f32_t worldX,
	f32_t worldZ) const
{
	if (!m_NavigationDocument.Is_Ready())
		return false;

	const NAVGRID_BAKE_DESC& bakeDesc =
		m_NavigationDocument.Get_BakeDesc();
	if (!Is_ValidNavigationBakeDesc(bakeDesc))
		return false;
	const f32_t radians =
		XMConvertToRadians(bakeDesc.yawDegrees);
	const f32_t cosine = std::cos(radians);
	const f32_t sine = std::sin(radians);
	const f32_t offsetX =
		worldX - bakeDesc.position.x;
	const f32_t offsetZ =
		worldZ - bakeDesc.position.z;
	const f32_t localX =
		cosine * offsetX - sine * offsetZ;
	const f32_t localZ =
		sine * offsetX + cosine * offsetZ;
	return
		std::abs(localX) <=
			bakeDesc.size.x * 0.5f + 0.00001f &&
		std::abs(localZ) <=
			bakeDesc.size.z * 0.5f + 0.00001f;
}

bool_t Client::CMapTool::Is_ValidNavigationBakeDesc(
	const NAVGRID_BAKE_DESC& desc)
{
	//Bounds 값에 대한 검증을 하는 함수 - 호출을 누가 하지?
	//어떤 상태를 변경하고 보존하는 역할을 하는 거지?
	return
		desc.isReady &&

		std::isfinite(desc.position.x) &&
		std::isfinite(desc.position.y) &&
		std::isfinite(desc.position.z) &&

		std::isfinite(desc.size.x) &&
		std::isfinite(desc.size.y) &&
		std::isfinite(desc.size.z) &&

		desc.size.x >= 0.1f &&
		desc.size.y >= 0.1f &&
		desc.size.z >= 0.1f &&

		std::isfinite(desc.yawDegrees) &&

		std::isfinite(desc.cellSize) &&
		desc.cellSize >= 0.05f &&
		desc.cellSize <= 10.f &&

		std::isfinite(desc.maxSlopeDegrees) &&
		desc.maxSlopeDegrees >= 0.f &&
		desc.maxSlopeDegrees < 90.f;
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

Client::CMapTool::PLACED_ENTRY* Client::CMapTool::Find_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	return iter == m_Placements.end() ? nullptr : &*iter;
}

const MAP_ASSET_ENTRY* Client::CMapTool::Get_SelectedAsset() const
{
	return m_Catalog.Find(m_SelectedAssetId);
}
