#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapNavigationContract.h"
#include "MapStaticBatchObject.h"
#include "MapAssetPreview.h"
#include "DeployPropObject.h"
#include "Model.h"
#include "Navigation.h"

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
	constexpr const wchar_t* MAP_BATCH_PROTOTYPE =
		TEXT("Prototype_GameObject_MapStaticBatch");
	constexpr const wchar_t* MAP_BATCH_LAYER =
		TEXT("Layer_MapStaticBatch");

	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t IsBatchEligible(const MAP_ASSET_ENTRY& asset)
	{
		/* Alpha/Additive는 인스턴스 간 정렬이 필요하고 Background는 카메라를
		   따라가므로 첫 단계에서는 고정된 Deferred 정적 배치만 허용한다. */
		return MAP_ASSET_RENDER_MODE::DEFERRED ==
			asset.renderProfile.renderMode;
	}

	HRESULT BuildStaticInstance(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		FMapStaticInstance& outInstance)
	{
		if (nullptr == model || !model->Has_LocalBounds())
			return E_FAIL;

		const float3_t& minimum = model->Get_LocalBoundsMin();
		const float3_t& maximum = model->Get_LocalBoundsMax();
		if (!IsFinite(minimum) || !IsFinite(maximum) ||
			minimum.x > maximum.x || minimum.y > maximum.y ||
			minimum.z > maximum.z)
			return E_FAIL;

		const float3_t localCenter(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		const vector_t halfExtents = XMVectorSet(
			(maximum.x - minimum.x) * 0.5f,
			(maximum.y - minimum.y) * 0.5f,
			(maximum.z - minimum.z) * 0.5f, 0.f);
		f32_t localRadius = XMVectorGetX(XMVector3Length(halfExtents));
		localRadius = localRadius > 0.05f ? localRadius : 0.05f;

		vector_t rotation = XMQuaternionNormalize(
			XMLoadFloat4(&record.rotationQuaternion));
		if (XMVectorGetW(rotation) < 0.f)
			rotation = XMVectorNegate(rotation);

		matrix_t world = XMMatrixScaling(
			record.signedScale.x,
			record.signedScale.y,
			record.signedScale.z) * XMMatrixRotationQuaternion(rotation);

		float3_t worldOrigin = record.position;
		if (MAP_ASSET_ANCHOR::BOTTOM_CENTER == asset.anchor)
		{
			const vector_t localAnchor = XMVectorSet(
				localCenter.x, minimum.y, localCenter.z, 1.f);
			float3_t anchorOffset{};
			XMStoreFloat3(&anchorOffset,
				XMVector3TransformCoord(localAnchor, world));
			worldOrigin.x -= anchorOffset.x;
			worldOrigin.y -= anchorOffset.y;
			worldOrigin.z -= anchorOffset.z;
		}

		world.r[3] = XMVectorSet(
			worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f);

		matrix_t linearWorld = world;
		linearWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		const f32_t determinant = XMVectorGetX(
			XMMatrixDeterminant(linearWorld));
		if (!std::isfinite(determinant) ||
			std::abs(determinant) < 0.000001f)
			return E_FAIL;

		const matrix_t worldInvTranspose = XMMatrixTranspose(
			XMMatrixInverse(nullptr, linearWorld));
		const f32_t maximumScale = (std::max)({
			std::abs(record.signedScale.x),
			std::abs(record.signedScale.y),
			std::abs(record.signedScale.z) });
		const f32_t worldRadius =
			localRadius * maximumScale * 1.02f + 0.05f;
		float3_t worldCenter{};
		XMStoreFloat3(&worldCenter,
			XMVector3TransformCoord(XMLoadFloat3(&localCenter), world));
		if (!IsFinite(worldCenter) || !std::isfinite(worldRadius) ||
			worldRadius <= 0.f)
			return E_FAIL;

		outInstance = {};
		outInstance.PlacementId = record.placementId;
		outInstance.Visible = record.visible;
		outInstance.WorldBoundsCenter = worldCenter;
		outInstance.WorldBoundsRadius = worldRadius;
		XMStoreFloat4x4(&outInstance.World, world);
		XMStoreFloat4x4(
			&outInstance.WorldInvTranspose, worldInvTranspose);
		return S_OK;
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

	m_pContext = pContext;
	m_pAssetPreview = std::move(preview);
	m_pNavigationRenderResources = std::move(navigationResources);
	return S_OK;
}

void Client::CMapTool::Toggle()
{
	m_bOpen = !m_bOpen;
}

void Client::CMapTool::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) == CGameInstance::Get().Get_CurrentLevelID();
	Handle_LevelTransition(isAssetTest);
	Update_EnvironmentShortcuts(isAssetTest);
	Update_WorldInteraction(isAssetTest);
}

void Client::CMapTool::Update_EnvironmentShortcuts(bool_t isAssetTest)
{
	if (!isAssetTest || GetForegroundWindow() != g_hWnd)
		return;

	if (CGameInstance::Get().Get_DIKeyPressed(DIK_F7))
	{
		Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
		m_Status = "Sky phase: Baseline (F7)";
	}
	else if (CGameInstance::Get().Get_DIKeyPressed(DIK_F8))
	{
		Set_EnvironmentPhase(ENVIRONMENT_PHASE::SPACEHOLE);
		m_Status = "Sky phase: SpaceHole (F8)";
	}
	else if (CGameInstance::Get().Get_DIKeyPressed(DIK_F9))
	{
		Set_EnvironmentPhase(ENVIRONMENT_PHASE::CHAOS_GATE);
		m_Status = "Sky phase: ChaosGate (F9)";
	}
}

void Client::CMapTool::Update_WorldInteraction(bool_t isAssetTest)
{
	const bool_t mouseDown = 0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;

	if (!m_bOpen || !isAssetTest)
	{
		m_bNavigationStrokeActive = false;
		return;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		m_ePlacementState = PLACEMENT_STATE::IDLE;
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

	if (PLACEMENT_STATE::ARMED == m_ePlacementState && mousePressed)
		Try_PlaceSelected();
}

void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().Get_CurrentLevelID();
	Render_WorldOverlay(isAssetTest);

	ImGui::SetNextWindowSize(ImVec2(1180.f, 900.f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		Render_ModeBar();
		ImGui::Separator();
		Render_ActiveMode(isAssetTest);
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

void Client::CMapTool::Render_ActiveMode(bool_t isAssetTest)
{
	switch (m_eToolMode)
	{
	case TOOL_MODE::MAP_ASSETS:
		Render_MapAssetsPanel(isAssetTest);
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
	ImGui::Text("Level: %s",
		isAssetTest ? "ASSET_TEST" : "Open ASSET_TEST with F2");
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

bool Client::CMapTool::IsOpen() const
{
	return m_bOpen;
}

bool_t Client::CMapTool::ConsumesWorldLeftMouse() const
{
	if (!m_bOpen ||
		ETOUI(LEVEL::ASSET_TEST) !=
		CGameInstance::Get().Get_CurrentLevelID())
	{
		return false;
	}

	return TOOL_MODE::NAVIGATION == m_eToolMode ||
		PLACEMENT_STATE::ARMED == m_ePlacementState;
}

void Client::CMapTool::Handle_LevelTransition(bool_t isAssetTest)
{
	if (isAssetTest == m_bWasInAssetTest)
		return;

	m_bWasInAssetTest = isAssetTest;
	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	m_pAssetTestCamera.reset();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->Reset_LevelResources();

	if (!isAssetTest)
	{
		m_Placements.clear();
		m_StaticBatches.clear();
		m_DeployProps.clear();
		m_iNextPlacementId = 1;
		m_bDirty = false;
		m_Status = "Enter AssetTest with F2";
		m_NavigationStatus = "Enter AssetTest with F2";
		m_CameraStatus = "Open ASSET_TEST with F2";
		return;
	}

	if (!m_Catalog.Load_Default())
	{
		m_Status = m_Catalog.Get_Status();
	}
	else
	{
		m_Status = m_Catalog.Get_Status();
		if (Load_Placements())
			Load_DeployProps();
	}

	Load_NavigationDocument();
	Find_AssetTestCamera();
}

bool_t Client::CMapTool::Find_AssetTestCamera()
{
	const shared_ptr<CGameObject> gameObject =
		CGameInstance::Get().Get_GameObject(
			ETOUI(LEVEL::ASSET_TEST),
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

bool_t Client::CMapTool::Load_NavigationDocument()
{
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
	shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			CGameInstance::Get().Get_Component(
				ETOUI(LEVEL::ASSET_TEST),
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
	shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			CGameInstance::Get().Get_Component(
				ETOUI(LEVEL::ASSET_TEST),
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
	if (!m_NavigationDocument.Is_Ready() ||
		!m_RuntimeBlockerDocument.Is_Ready())
	{
		m_NavigationStatus = "Navigation is not loaded";
		return false;
	}

	std::string status;
	MAP_NAVIGATION_CONTRACT contract;
	if (!CMapNavigationContract::Resolve_Area(
		m_Catalog.Get_AreaId(), contract, status))
	{
		m_NavigationStatus = status;
		return false;
	}
	if (!HasSameNavigationPath(
			m_NavigationSourcePath, contract.sourcePath) ||
		!HasSameNavigationPath(
			m_NavigationPaintPath, contract.paintPath) ||
		!HasSameNavigationPath(
			m_NavigationRuntimePath, contract.runtimePath) ||
		!HasSameNavigationPath(
			m_RuntimeBlockerPath, contract.blockerPath))
	{
		m_NavigationStatus =
			"Navigation paths do not match the active map area; reload before saving";
		return false;
	}

	std::error_code sourceError;
	if (!std::filesystem::is_regular_file(
		contract.sourcePath, sourceError) || sourceError)
	{
		m_NavigationStatus =
			"Navigation source is missing or unavailable; bake before saving";
		return false;
	}

	const NAVGRID_AUTHORING_DESC& sourceDesc =
		m_NavigationDocument.Get_Desc();
	const NAVGRID_AUTHORING_DESC& blockerDesc =
		m_RuntimeBlockerDocument.Get_Desc();
	if (sourceDesc.areaId != contract.areaId ||
		!HasSameNavigationGridIdentity(sourceDesc, blockerDesc))
	{
		m_NavigationStatus =
			"Navigation documents do not match the active map grid; reload before saving";
		return false;
	}

	if (!m_NavigationDocument.Save_Paint(
		m_NavigationPaintPath,
		status))
	{
		m_NavigationStatus = status;
		return false;
	}

	if (!m_RuntimeBlockerDocument.Save(
		m_RuntimeBlockerPath,
		status))
	{
		m_NavigationStatus = status;
		return false;
	}

	if (!m_NavigationDocument.Export_Runtime(
		m_NavigationRuntimePath,
		status))
	{
		m_NavigationStatus = status;
		return false;
	}

	m_NavigationStatus =
		"Saved. Re-enter ASSET_TEST to reload runtime navigation.";
	return true;
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

	const bool_t erase =
		NAVIGATION_EDIT_ACTION::ERASE ==
		m_eNavigationEditAction;

	bool_t changed = false;
	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		changed = m_RuntimeBlockerDocument.Paint(
			m_iSelectedRuntimeRegion,
			cellX,
			cellZ,
			m_iBrushRadius,
			!erase);
	}
	else
	{
		changed = m_NavigationDocument.Paint(
			cellX,
			cellZ,
			m_iBrushRadius,
			erase);
	}

	if (changed)
		m_NavigationStatus = "Unsaved";
	return changed;
}

bool_t Client::CMapTool::Try_PickPlacementPosition(float3_t& outPosition) const
{
	float4_t picked{};
	if (CGameInstance::Get().Picking(picked))
	{
		outPosition = float3_t(picked.x, picked.y, picked.z);
		return IsFinite(outPosition);
	}

	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	const float2_t viewport = CGameInstance::Get().Get_ViewportSize();
	if (cursor.x < 0 || cursor.y < 0 || cursor.x >= static_cast<LONG>(viewport.x) ||
		cursor.y >= static_cast<LONG>(viewport.y))
		return false;

	const matrix_t view = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	const matrix_t projection = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	const vector_t nearPoint = XMVector3Unproject(
		XMVectorSet(static_cast<float>(cursor.x), static_cast<float>(cursor.y), 0.f, 1.f),
		0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f,
		projection, view, XMMatrixIdentity());
	const vector_t farPoint = XMVector3Unproject(
		XMVectorSet(static_cast<float>(cursor.x), static_cast<float>(cursor.y), 1.f, 1.f),
		0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f,
		projection, view, XMMatrixIdentity());
	const vector_t direction = farPoint - nearPoint;
	const float directionY = XMVectorGetY(direction);
	if (std::abs(directionY) < 0.00001f)
		return false;

	const float distance = -XMVectorGetY(nearPoint) / directionY;
	if (distance < 0.f)
		return false;

	XMStoreFloat3(&outPosition, nearPoint + direction * distance);
	outPosition.y = 0.f;
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
	std::wstring layerTag = L"Layer_MapAsset_";
	layerTag.append(sourceLevel.begin(), sourceLevel.end());
	return layerTag;
}

bool_t Client::CMapTool::Create_Placement(
	const MAP_PLACEMENT_RECORD& record, PLACED_ENTRY& outEntry)
{
	const MAP_ASSET_ENTRY* pAsset = m_Catalog.Find(record.assetId);
	if (nullptr == pAsset ||
		!CMapPlacementDocument::Is_Valid(record, m_Catalog))
		return false;

	const std::wstring layerTag = Make_LayerTag(record.sourceLevel);
	CMapAssetObject::MAP_ASSET_DESC desc{};
	desc.placementId = record.placementId;
	desc.assetId = pAsset->id;
	desc.modelPrototypeTag = pAsset->prototypeTag;
	desc.position = record.position;
	desc.rotationQuaternion = record.rotationQuaternion;
	desc.signedScale = record.signedScale;
	desc.applyBottomCenter = MAP_ASSET_ANCHOR::BOTTOM_CENTER == pAsset->anchor;
	desc.visible = record.visible;
	desc.renderProfile = pAsset->renderProfile;

	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST), L"Prototype_GameObject_MapAsset",
		ETOUI(LEVEL::ASSET_TEST), layerTag, &desc, &pGameObject)))
		return false;

	shared_ptr<CMapAssetObject> pMapObject =
		dynamic_pointer_cast<CMapAssetObject>(pGameObject);
	if (nullptr == pMapObject)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), layerTag, pGameObject);
		return false;
	}

	outEntry.record = record;
	outEntry.layerTag = layerTag;
	outEntry.object = std::move(pMapObject);
	return true;
}

bool_t Client::CMapTool::Stage_PlacementRuntime(
	const vector<MAP_PLACEMENT_RECORD>& records,
	vector<PLACED_ENTRY>& outPlacements,
	vector<STATIC_BATCH_ENTRY>& outBatches)
{
	using BATCH_KEY = std::pair<std::string, bool_t>;
	std::map<BATCH_KEY, vector<const MAP_PLACEMENT_RECORD*>> groups;

	for (const MAP_PLACEMENT_RECORD& record : records)
	{
		const MAP_ASSET_ENTRY* asset = m_Catalog.Find(record.assetId);
		if (nullptr == asset)
			return false;
		if (!IsBatchEligible(*asset))
			continue;

		const bool_t mirrored = record.signedScale.x *
			record.signedScale.y * record.signedScale.z < 0.f;
		groups[{ record.assetId, mirrored }].push_back(&record);
	}

	std::unordered_map<uint64_t, shared_ptr<CMapStaticBatchObject>>
		batchByPlacement;
	batchByPlacement.reserve(records.size());

	for (const auto& [key, placements] : groups)
	{
		const MAP_ASSET_ENTRY* asset = m_Catalog.Find(key.first);
		if (nullptr == asset)
			return false;

		shared_ptr<CModel> model = dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				ETOUI(LEVEL::ASSET_TEST), asset->prototypeTag));
		if (nullptr == model)
			return false;

		/* Bounds가 없는 모델은 기존 MapAssetObject 경로로 fail-open한다. */
		if (!model->Has_LocalBounds())
			continue;

		CMapStaticBatchObject::DESC desc{};
		desc.AssetId = asset->id;
		desc.ModelPrototypeTag = asset->prototypeTag;
		desc.RenderProfile = asset->renderProfile;
		desc.Mirrored = key.second;
		desc.Instances.reserve(placements.size());

		bool_t batchIsValid = true;
		for (const MAP_PLACEMENT_RECORD* record : placements)
		{
			FMapStaticInstance instance{};
			if (nullptr == record || FAILED(BuildStaticInstance(
				*asset, model, *record, instance)))
			{
				batchIsValid = false;
				break;
			}
			desc.Instances.push_back(instance);
		}
		if (!batchIsValid)
			continue;

		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_PROTOTYPE,
			ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_LAYER,
			&desc, &gameObject)))
			return false;

		shared_ptr<CMapStaticBatchObject> batch =
			dynamic_pointer_cast<CMapStaticBatchObject>(gameObject);
		if (nullptr == batch)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_LAYER, gameObject);
			return false;
		}

		outBatches.push_back({ asset->id, key.second, batch });
		for (const MAP_PLACEMENT_RECORD* record : placements)
		{
			const auto [iter, inserted] = batchByPlacement.emplace(
				record->placementId, batch);
			UNREFERENCED_PARAMETER(iter);
			if (!inserted)
				return false;
		}
	}

	outPlacements.reserve(records.size());
	for (const MAP_PLACEMENT_RECORD& record : records)
	{
		const auto batch = batchByPlacement.find(record.placementId);
		if (batch != batchByPlacement.end())
		{
			PLACED_ENTRY entry{};
			entry.record = record;
			entry.layerTag = MAP_BATCH_LAYER;
			entry.batch = batch->second;
			outPlacements.push_back(std::move(entry));
			continue;
		}

		PLACED_ENTRY fallback{};
		if (!Create_Placement(record, fallback))
			return false;
		outPlacements.push_back(std::move(fallback));
	}

	return outPlacements.size() == records.size();
}

void Client::CMapTool::Remove_PlacementRuntime(
	vector<PLACED_ENTRY>& placements,
	vector<STATIC_BATCH_ENTRY>& batches)
{
	for (PLACED_ENTRY& entry : placements)
	{
		if (nullptr != entry.object)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::ASSET_TEST), entry.layerTag,
				static_pointer_cast<CGameObject>(entry.object));
		}
	}

	for (STATIC_BATCH_ENTRY& entry : batches)
	{
		if (nullptr != entry.object)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_LAYER,
				static_pointer_cast<CGameObject>(entry.object));
		}
	}

	placements.clear();
	batches.clear();
}

bool_t Client::CMapTool::Set_RuntimeVisible(
	PLACED_ENTRY& entry, bool_t visible)
{
	if (nullptr != entry.object)
	{
		entry.object->Set_Visible(visible);
		return true;
	}
	if (nullptr != entry.batch)
	{
		return SUCCEEDED(entry.batch->Set_InstanceVisible(
			entry.record.placementId, visible));
	}
	return false;
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
			ETOUI(LEVEL::ASSET_TEST), iter->layerTag,
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
	if (m_Catalog.Is_Sharded())
	{
		m_Status = "Save disabled: shard-set placement documents are read-only";
		return false;
	}

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

	if (!CMapPlacementDocument::Write(
		m_Catalog.Get_PlacementPath(), m_Catalog.Get_AreaId(),
		document, m_Catalog, m_Status))
		return false;

	m_bDirty = false;
	return true;
}

bool_t Client::CMapTool::Load_Placements()
{
	if (!m_Catalog.Is_Ready())
		return false;

	vector<MAP_PLACEMENT_RECORD> document;
	std::string loadStatus;
	if (!m_Catalog.Is_Sharded())
	{
		if (!CMapPlacementDocument::Read(
			m_Catalog.Get_PlacementPath(), m_Catalog, document, loadStatus))
		{
			m_Status = loadStatus;
			return false;
		}
	}
	else
	{
		size_t declaredTotal = {};
		for (const MAP_ASSET_SHARD& shard : m_Catalog.Get_Shards())
		{
			if (declaredTotal > (std::numeric_limits<size_t>::max)() -
				shard.placementCount)
			{
				m_Status = "Shard placement count overflow";
				return false;
			}
			declaredTotal += shard.placementCount;
		}

		document.reserve(declaredTotal);
		std::unordered_set<uint64_t> runtimeIds;
		std::unordered_set<std::string> sourceIds;
		runtimeIds.reserve(declaredTotal);
		sourceIds.reserve(declaredTotal);
		for (const MAP_ASSET_SHARD& shard : m_Catalog.Get_Shards())
		{
			std::error_code placementError;
			if (!std::filesystem::is_regular_file(
				shard.placementPath, placementError))
			{
				m_Status = "Shard placement file is missing: " +
					shard.shardId;
				return false;
			}

			vector<MAP_PLACEMENT_RECORD> shardDocument;
			if (!CMapPlacementDocument::Read(
				shard.placementPath, m_Catalog, shardDocument, loadStatus))
			{
				m_Status = "Shard " + shard.shardId + " failed: " +
					loadStatus;
				return false;
			}
			if (shardDocument.size() != shard.placementCount)
			{
				m_Status = "Shard placement count mismatch: " + shard.shardId;
				return false;
			}

			for (MAP_PLACEMENT_RECORD& record : shardDocument)
			{
				if (!runtimeIds.insert(record.placementId).second ||
					!sourceIds.insert(record.sourcePlacementId).second)
				{
					m_Status = "Duplicate placement ID across shards: " +
						shard.shardId;
					return false;
				}
				document.push_back(std::move(record));
			}
		}
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
	const std::filesystem::path root = CMapAssetCatalog::Get_MapDataRoot();
	if (root.empty())
	{
		m_Status = "DeployProp data root is unavailable";
		return false;
	}

	const std::filesystem::path areaPath(m_Catalog.Get_AreaId());
	const std::filesystem::path catalogPath =
		root / (areaPath.wstring() + L".deployassets");
	const std::filesystem::path placementPath =
		root / (areaPath.wstring() + L".deployplacements");
	std::error_code catalogError;
	std::error_code placementError;
	const bool_t catalogExists =
		std::filesystem::exists(catalogPath, catalogError);
	const bool_t placementExists =
		std::filesystem::exists(placementPath, placementError);
	if (catalogError)
	{
		m_Status = "DeployProp catalog inspection failed: " +
			catalogPath.string() + " (" + catalogError.message() + ")";
		return false;
	}
	if (placementError)
	{
		m_Status = "DeployProp placement inspection failed: " +
			placementPath.string() + " (" + placementError.message() + ")";
		return false;
	}
	if (!catalogExists && !placementExists)
	{
		Remove_DeployProps();
		return true;
	}
	if (catalogExists != placementExists)
	{
		const std::filesystem::path& missingPath =
			catalogExists ? placementPath : catalogPath;
		m_Status = "DeployProp file pair is incomplete; missing: " +
			missingPath.string();
		return false;
	}

	if (!m_DeployCatalog.Load_Default(m_Catalog.Get_AreaId()))
	{
		m_Status = m_DeployCatalog.Get_Status();
		return false;
	}

	vector<DEPLOY_ENTRY> staged;
	staged.reserve(m_DeployCatalog.Get_Placements().size());
	for (const DEPLOY_PROP_PLACEMENT& record :
		m_DeployCatalog.Get_Placements())
	{
		const DEPLOY_PROP_ASSET_ENTRY* asset =
			m_DeployCatalog.Find(record.assetId);
		if (nullptr == asset)
		{
			for (const DEPLOY_ENTRY& rollback : staged)
				CGameInstance::Get().Remove_GameObject_from_Layer(
					ETOUI(LEVEL::ASSET_TEST), TEXT("Layer_DeployProps"),
					static_pointer_cast<CGameObject>(rollback.object));
			m_Status = "DeployProp staging lost asset " + record.assetId;
			return false;
		}
		CDeployPropObject::DEPLOY_PROP_DESC desc{};
		desc.placement = record;
		desc.modelKind = asset->kind;
		desc.intactPrototypeTag = asset->intactPrototypeTag;
		desc.fracturedPrototypeTag = asset->fracturedPrototypeTag;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_DeployProp"),
			ETOUI(LEVEL::ASSET_TEST), TEXT("Layer_DeployProps"),
			&desc, &gameObject)))
		{
			for (const DEPLOY_ENTRY& rollback : staged)
				CGameInstance::Get().Remove_GameObject_from_Layer(
					ETOUI(LEVEL::ASSET_TEST), TEXT("Layer_DeployProps"),
					static_pointer_cast<CGameObject>(rollback.object));
			m_Status = "DeployProp stage rolled back at " +
				record.sourcePlacementId;
			return false;
		}
		shared_ptr<CDeployPropObject> object =
			dynamic_pointer_cast<CDeployPropObject>(gameObject);
		if (nullptr == object)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::ASSET_TEST), TEXT("Layer_DeployProps"), gameObject);
			for (const DEPLOY_ENTRY& rollback : staged)
				CGameInstance::Get().Remove_GameObject_from_Layer(
					ETOUI(LEVEL::ASSET_TEST), TEXT("Layer_DeployProps"),
					static_pointer_cast<CGameObject>(rollback.object));
			m_Status = "DeployProp clone type mismatch";
			return false;
		}
		staged.push_back({ record, std::move(object) });
	}

	Remove_DeployProps();
	m_DeployProps = std::move(staged);
	Set_DeployPhase(DEPLOY_PROP_STATE::INTACT);
	const size_t bindPoseOnly = static_cast<size_t>(std::count_if(
		m_DeployProps.begin(), m_DeployProps.end(),
		[](const DEPLOY_ENTRY& entry)
		{
			return entry.object->Is_AnimBindPoseOnly();
		}));
	m_Status = "Loaded " + std::to_string(m_Placements.size()) +
		" map placements + " + std::to_string(m_DeployProps.size()) +
		" gameplay DeployProps (ITR_02326 bind-pose-only: " +
		std::to_string(bindPoseOnly) + ")";
	return true;
}

void Client::CMapTool::Remove_DeployProps()
{
	for (const DEPLOY_ENTRY& entry : m_DeployProps)
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), TEXT("Layer_DeployProps"),
			static_pointer_cast<CGameObject>(entry.object));
	m_DeployProps.clear();
}

void Client::CMapTool::Set_DeployPhase(DEPLOY_PROP_STATE state)
{
	m_DeployPhase = state;
	for (DEPLOY_ENTRY& entry : m_DeployProps)
		entry.object->Set_State(state);

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
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Navigation",
		TOOL_MODE::NAVIGATION == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::NAVIGATION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Camera",
		TOOL_MODE::CAMERA == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::CAMERA;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
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
		"F6: toggle Follow / Free Camera");
	ImGui::TextDisabled(
		"Tab: toggle Free Camera mouse look");
	ImGui::TextDisabled(
		"Free Camera: WASD move");
}

void Client::CMapTool::Render_NavigationPanel()
{
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

	const char* applyLabel =
		NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode ?
		"Add Cells" :
		"Block";
	const char* eraseLabel =
		NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode ?
		"Remove Cells" :
		"Erase";

	if (ImGui::RadioButton(
		applyLabel,
		NAVIGATION_EDIT_ACTION::APPLY ==
		m_eNavigationEditAction))
	{
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		eraseLabel,
		NAVIGATION_EDIT_ACTION::ERASE ==
		m_eNavigationEditAction))
	{
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::ERASE;
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
		"Surface: %u | Excluded: %u | Blocked: %u",
		surfaceCells,
		m_NavigationDocument.Get_CellCount() - surfaceCells,
		m_NavigationDocument.Get_BlockedCount());
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
	ImGui::BeginDisabled(m_Catalog.Is_Sharded());
	if (ImGui::Button("Save"))
		Save_Placements();
	ImGui::EndDisabled();
	if (m_Catalog.Is_Sharded())
	{
		ImGui::SameLine();
		ImGui::TextDisabled("Shard set is read-only");
	}
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
	ImGui::Text("Gameplay DeployProps: %zu | phase:", m_DeployProps.size());
	ImGui::SameLine();
	if (ImGui::RadioButton("Intact##DeployPhase",
		m_DeployPhase == DEPLOY_PROP_STATE::INTACT))
		Set_DeployPhase(DEPLOY_PROP_STATE::INTACT);
	ImGui::SameLine();
	if (ImGui::RadioButton("Fractured##DeployPhase",
		m_DeployPhase == DEPLOY_PROP_STATE::FRACTURED))
		Set_DeployPhase(DEPLOY_PROP_STATE::FRACTURED);
	ImGui::SameLine();
	if (ImGui::RadioButton("Despawned##DeployPhase",
		m_DeployPhase == DEPLOY_PROP_STATE::DESPAWNED))
		Set_DeployPhase(DEPLOY_PROP_STATE::DESPAWNED);
	ImGui::TextUnformatted("Sky phase (F7 Baseline / F8 SpaceHole / F9 ChaosGate):");
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
								ETOUI(LEVEL::ASSET_TEST),
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
								ETOUI(LEVEL::ASSET_TEST), migrated.layerTag,
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
		FAILED(m_pAssetPreview->Select_Asset(asset)))
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
		"; click the rendered surface or Y=0 plane.";
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
						ETOUI(LEVEL::ASSET_TEST),
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
			outStatus =
				"Could not build bake transform for " +
				asset->id;
			return false;
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
			"No visible deferred map meshes overlap Nav Bounds";
		return false;
	}
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
		std::filesystem::remove(
			m_NavigationPaintPath,
			paintError);
		std::error_code blockerError;
		std::filesystem::remove(
			m_RuntimeBlockerPath,
			blockerError);
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
		"Baked source; Save Navigation to export runtime";
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
	changed =
		ImGui::DragFloat3(
			"Position",
			&m_NavigationBakeDesc.position.x,
			0.1f) ||
		changed;
	changed =
		ImGui::DragFloat3(
			"Size",
			&m_NavigationBakeDesc.size.x,
			0.1f,
			0.1f,
			10000.f) ||
		changed;
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
	if (ImGui::Button("Bake"))
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
