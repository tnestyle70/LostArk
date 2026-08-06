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
#include "Model.h"
#include "Navigation.h"
#include "ProjectDataRoot.h"
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
	return S_OK;
}

void Client::CMapTool::Toggle()
{
	m_bOpen = !m_bOpen;
}

void Client::CMapTool::SetOpen(const bool_t isOpen)
{
	m_bOpen = isOpen;
}

void Client::CMapTool::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	const uint32_t currentLevelIndex =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isMapAuthoringLevel =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelIndex &&
		CMapEditorWorkspaceService::Is_Active();
	Handle_LevelTransition(currentLevelIndex, isMapAuthoringLevel);
	Update_WorldInteraction(isMapAuthoringLevel);
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
	if (ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		Render_WorkspaceBar(isMapAuthoringLevel);
		ImGui::Separator();
		Render_ModeBar();
		ImGui::Separator();
		Render_ActiveMode(isMapAuthoringLevel);
	}
	ImGui::End();
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
		(TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode &&
			(m_bWorldGameplayPlacementArmed ||
				m_bWorldTriggerTargetPickArmed ||
				m_bSpawnAnchorPlacementArmed)) ||
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
			std::string blockers;
			if (!ReadRequiredString(*selected, "navigationBlockers", blockers))
			{
				m_Status = "Valtan navigation blocker source is missing";
				return false;
			}
			descriptor.navigationBlockers = ResolveDataCatalogPath(blockers);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;
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
		m_RuntimeBlockerDocument.Is_Dirty();
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
		if (error || (!hasSource && !descriptor.allowNavigationBootstrap))
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

	if (!Ensure_AuthoringPrototypes(stagedCatalog))
		return false;

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
	m_NavigationSourcePath = descriptor.navigationSource;
	m_NavigationPaintPath = descriptor.navigationPaint;
	m_RuntimeBlockerPath = descriptor.navigationBlockers;
	m_NavigationRuntimePath.clear();
	m_NavigationBakeDesc = navigationLoaded ?
		m_NavigationDocument.Get_BakeDesc() : NAVGRID_BAKE_DESC{};
	m_iActiveEditorArea = descriptorIndex;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_iSelectedPlacementId = 0;
	m_SelectedWorldPlacementId.clear();
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_SelectedAssetId.clear();
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
	m_Catalog = CMapAssetCatalog{};
	m_EditorAreas.clear();
	m_iActiveEditorArea = SIZE_MAX;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_isEditorExitPending = false;
	m_PrototypeModelPaths.clear();
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
		if (sourceError || (!hasSource && !active->allowNavigationBootstrap))
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
	if (sourceError)
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

void Client::CMapTool::Render_ModeBar()
{
	if (ImGui::RadioButton(
		"Map Assets",
		TOOL_MODE::MAP_ASSETS == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::MAP_ASSETS;
		m_bWorldGameplayPlacementArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"World Gameplay",
		TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::WORLD_GAMEPLAY;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bNavigationStrokeActive = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Navigation",
		TOOL_MODE::NAVIGATION == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::NAVIGATION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Camera",
		TOOL_MODE::CAMERA == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::CAMERA;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bNavigationStrokeActive = false;
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
