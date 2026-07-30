#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapStaticBatchObject.h"
#include "MapAssetPreview.h"
#include "DeployPropObject.h"
#include "Model.h"

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
}

Client::CMapTool::~CMapTool() = default;

HRESULT Client::CMapTool::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto preview = std::make_unique<CMapAssetPreview>();
	if (FAILED(preview->Initialize(pDevice, pContext)))
		return E_FAIL;

	m_pAssetPreview = std::move(preview);
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
	if (isAssetTest && GetForegroundWindow() == g_hWnd)
	{
		if (0 != (GetAsyncKeyState(VK_F7) & 1))
		{
			Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
			m_Status = "Sky phase: Baseline (F7)";
		}
		else if (0 != (GetAsyncKeyState(VK_F8) & 1))
		{
			Set_EnvironmentPhase(ENVIRONMENT_PHASE::SPACEHOLE);
			m_Status = "Sky phase: SpaceHole (F8)";
		}
		else if (0 != (GetAsyncKeyState(VK_F9) & 1))
		{
			Set_EnvironmentPhase(ENVIRONMENT_PHASE::CHAOS_GATE);
			m_Status = "Sky phase: ChaosGate (F9)";
		}
	}

	const bool_t mouseDown = 0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;

	if (!m_bOpen || !isAssetTest || PLACEMENT_STATE::ARMED != m_ePlacementState)
		return;

	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_Status = "Placement cancelled";
		return;
	}

	if (mousePressed && GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantCaptureMouse)
		Try_PlaceSelected();
}

void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(1180.f, 900.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) == CGameInstance::Get().Get_CurrentLevelID();
	ImGui::Text("Level: %s", isAssetTest ? "ASSET_TEST" : "Open ASSET_TEST with F2");
	ImGui::SameLine();
	ImGui::Text("| Catalog: %s", m_Catalog.Is_Ready() ? "READY" : "NOT READY");
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

	ImGui::End();
}

bool Client::CMapTool::IsOpen() const
{
	return m_bOpen;
}

void Client::CMapTool::Handle_LevelTransition(bool_t isAssetTest)
{
	if (isAssetTest == m_bWasInAssetTest)
		return;

	m_bWasInAssetTest = isAssetTest;
	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
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
		return;
	}

	if (!m_Catalog.Load_Default())
	{
		m_Status = m_Catalog.Get_Status();
		return;
	}

	m_Status = m_Catalog.Get_Status();
	if (Load_Placements())
		Load_DeployProps();
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
	if (!CMapPlacementDocument::Read(
		m_Catalog.Get_PlacementPath(), m_Catalog, document, loadStatus))
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

void Client::CMapTool::Render_Toolbar()
{
	if (ImGui::Button("Save"))
		Save_Placements();
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
