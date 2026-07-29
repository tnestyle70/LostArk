#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "MapAssetObject.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
	constexpr const char* PLACEMENT_MAGIC = "LOSTARK_MAP_PLACEMENTS";
	constexpr uint32_t PLACEMENT_VERSION = 1;
	constexpr uint32_t MAX_PLACEMENT_COUNT = 10000;
	constexpr const wchar_t* MAP_LAYER_TAG = L"Layer_MapAsset";

	struct STORED_PLACEMENT
	{
		uint64_t placementId = {};
		std::string assetId;
		float3_t position = {};
		float3_t rotationDegrees = {};
		float3_t scale = float3_t(1.f, 1.f, 1.f);
		bool_t visible = true;
	};

	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t IsValidStoredPlacement(const STORED_PLACEMENT& placement)
	{
		return 0 != placement.placementId && !placement.assetId.empty() &&
			IsFinite(placement.position) && IsFinite(placement.rotationDegrees) &&
			IsFinite(placement.scale) && placement.scale.x > 0.f &&
			placement.scale.y > 0.f && placement.scale.z > 0.f;
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

	bool_t ReadPlacementDocument(const std::filesystem::path& path,
		const CMapAssetCatalog& catalog, vector<STORED_PLACEMENT>& outPlacements,
		std::string& outStatus)
	{
		outPlacements.clear();
		std::error_code fileError;
		if (!std::filesystem::exists(path, fileError))
		{
			outStatus = "No saved placement file; starting with an empty map";
			return true;
		}

		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			outStatus = "Could not open placement file: " + path.string();
			return false;
		}

		std::string magic;
		std::string areaId;
		uint32_t version = {};
		uint32_t count = {};
		if (!(input >> magic >> version >> std::quoted(areaId) >> count) ||
			magic != PLACEMENT_MAGIC || version != PLACEMENT_VERSION ||
			areaId != catalog.Get_AreaId() || count > MAX_PLACEMENT_COUNT)
		{
			outStatus = "Placement header is invalid or belongs to another area";
			return false;
		}

		std::unordered_set<uint64_t> ids;
		outPlacements.reserve(count);
		for (uint32_t index = 0; index < count; ++index)
		{
			STORED_PLACEMENT placement{};
			int32_t visible = {};
			if (!(input >> placement.placementId >> std::quoted(placement.assetId) >>
				placement.position.x >> placement.position.y >> placement.position.z >>
				placement.rotationDegrees.x >> placement.rotationDegrees.y >> placement.rotationDegrees.z >>
				placement.scale.x >> placement.scale.y >> placement.scale.z >> visible))
			{
				outStatus = "Placement row is truncated at index " + std::to_string(index);
				outPlacements.clear();
				return false;
			}

			placement.visible = 0 != visible;
			if (!IsValidStoredPlacement(placement) ||
				nullptr == catalog.Find(placement.assetId) ||
				!ids.insert(placement.placementId).second)
			{
				outStatus = "Placement validation failed at index " + std::to_string(index);
				outPlacements.clear();
				return false;
			}
			outPlacements.push_back(std::move(placement));
		}

		std::string trailing;
		if (input >> trailing)
		{
			outStatus = "Placement file contains unexpected trailing data";
			outPlacements.clear();
			return false;
		}

		outStatus = "Placement document validated";
		return true;
	}

	bool_t CommitTemporaryFile(const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError))
		{
			if (ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
				return true;
		}

		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
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

	ImGui::SetNextWindowSize(ImVec2(900.f, 620.f), ImGuiCond_FirstUseEver);
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

	if (ImGui::BeginTable("MapEditorColumns", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Asset Palette", ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthStretch, 0.27f);
		ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch, 0.35f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_Palette();
		ImGui::TableSetColumnIndex(1);
		Render_Hierarchy();
		ImGui::TableSetColumnIndex(2);
		Render_Inspector();
		ImGui::EndTable();
	}
	ImGui::EndDisabled();

	Render_DecoderReport();
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

	if (!isAssetTest)
	{
		m_Placements.clear();
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
	Load_Placements();
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

	PLACED_ENTRY placed{};
	if (!Create_Placement(m_iNextPlacementId, pAsset->id, position,
		float3_t(0.f, 0.f, 0.f), pAsset->defaultScale, true, placed))
	{
		m_Status = "Failed to clone map object for " + pAsset->id;
		return false;
	}

	m_iSelectedPlacementId = m_iNextPlacementId++;
	m_Placements.push_back(std::move(placed));
	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_bDirty = true;
	m_Status = "Placed " + pAsset->label;
	return true;
}

bool_t Client::CMapTool::Create_Placement(uint64_t placementId,
	const std::string& assetId, const float3_t& position,
	const float3_t& rotationDegrees, const float3_t& scale,
	bool_t visible, PLACED_ENTRY& outEntry)
{
	const MAP_ASSET_ENTRY* pAsset = m_Catalog.Find(assetId);
	if (nullptr == pAsset)
		return false;

	CMapAssetObject::MAP_ASSET_DESC desc{};
	desc.placementId = placementId;
	desc.assetId = pAsset->id;
	desc.modelPrototypeTag = pAsset->prototypeTag;
	desc.position = position;
	desc.rotationDegrees = rotationDegrees;
	desc.scale = scale;
	desc.applyBottomCenter = MAP_ASSET_ANCHOR::BOTTOM_CENTER == pAsset->anchor;
	desc.visible = visible;

	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST), L"Prototype_GameObject_MapAsset",
		ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG, &desc, &pGameObject)))
		return false;

	shared_ptr<CMapAssetObject> pMapObject =
		dynamic_pointer_cast<CMapAssetObject>(pGameObject);
	if (nullptr == pMapObject)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG, pGameObject);
		return false;
	}

	outEntry.placementId = placementId;
	outEntry.assetId = assetId;
	outEntry.object = std::move(pMapObject);
	return true;
}

bool_t Client::CMapTool::Remove_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.placementId == placementId;
		});
	if (iter == m_Placements.end())
		return false;

	if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
		ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG,
		static_pointer_cast<CGameObject>(iter->object))))
		return false;

	m_Placements.erase(iter);
	if (m_iSelectedPlacementId == placementId)
		m_iSelectedPlacementId = 0;
	m_bDirty = true;
	return true;
}

void Client::CMapTool::Remove_AllPlacements()
{
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG,
			static_pointer_cast<CGameObject>(entry.object));
	}
	m_Placements.clear();
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	m_bDirty = true;
}

bool_t Client::CMapTool::Save_Placements()
{
	vector<STORED_PLACEMENT> document;
	document.reserve(m_Placements.size());
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		if (nullptr == entry.object || nullptr == m_Catalog.Find(entry.assetId))
		{
			m_Status = "Save aborted: placement references are invalid";
			return false;
		}

		STORED_PLACEMENT stored{};
		stored.placementId = entry.placementId;
		stored.assetId = entry.assetId;
		stored.position = entry.object->Get_Position();
		stored.rotationDegrees = entry.object->Get_RotationDegrees();
		stored.scale = entry.object->Get_Scale();
		stored.visible = entry.object->Is_Visible();
		if (!IsValidStoredPlacement(stored))
		{
			m_Status = "Save aborted: a transform is invalid";
			return false;
		}
		document.push_back(std::move(stored));
	}

	const std::filesystem::path destination = CMapAssetCatalog::Get_DefaultPlacementPath();
	const std::filesystem::path temporary = destination.wstring() + L".tmp";
	std::error_code directoryError;
	std::filesystem::create_directories(destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Could not create placement directory";
		return false;
	}

	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		m_Status = "Could not create temporary placement file";
		return false;
	}

	output << PLACEMENT_MAGIC << ' ' << PLACEMENT_VERSION << ' '
		<< std::quoted(m_Catalog.Get_AreaId()) << ' ' << document.size() << '\n';
	output << std::setprecision(9);
	for (const STORED_PLACEMENT& placement : document)
	{
		output << placement.placementId << ' ' << std::quoted(placement.assetId) << ' '
			<< placement.position.x << ' ' << placement.position.y << ' ' << placement.position.z << ' '
			<< placement.rotationDegrees.x << ' ' << placement.rotationDegrees.y << ' ' << placement.rotationDegrees.z << ' '
			<< placement.scale.x << ' ' << placement.scale.y << ' ' << placement.scale.z << ' '
			<< (placement.visible ? 1 : 0) << '\n';
	}
	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully || !CommitTemporaryFile(destination, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		m_Status = "Failed to commit placement file atomically";
		return false;
	}

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(document.size()) + " placements";
	return true;
}

bool_t Client::CMapTool::Load_Placements()
{
	if (!m_Catalog.Is_Ready())
		return false;

	vector<STORED_PLACEMENT> document;
	std::string loadStatus;
	if (!ReadPlacementDocument(CMapAssetCatalog::Get_DefaultPlacementPath(),
		m_Catalog, document, loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}

	vector<PLACED_ENTRY> created;
	created.reserve(document.size());
	for (const STORED_PLACEMENT& placement : document)
	{
		PLACED_ENTRY entry{};
		if (!Create_Placement(placement.placementId, placement.assetId,
			placement.position, placement.rotationDegrees, placement.scale,
			placement.visible, entry))
		{
			for (const PLACED_ENTRY& rollback : created)
			{
				CGameInstance::Get().Remove_GameObject_from_Layer(
					ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG,
					static_pointer_cast<CGameObject>(rollback.object));
			}
			m_Status = "Load rolled back: could not clone " + placement.assetId;
			return false;
		}
		created.push_back(std::move(entry));
	}

	for (const PLACED_ENTRY& old : m_Placements)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG,
			static_pointer_cast<CGameObject>(old.object));
	}
	m_Placements = std::move(created);
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : m_Placements)
		m_iNextPlacementId = (std::max)(m_iNextPlacementId, entry.placementId + 1);
	m_bDirty = false;
	m_Status = "Loaded " + std::to_string(m_Placements.size()) + " placements";
	return true;
}

void Client::CMapTool::Render_Toolbar()
{
	if (ImGui::Button("Save"))
		Save_Placements();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
		Load_Placements();
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		ImGui::OpenPopup("Clear all placements?");
	ImGui::SameLine();
	ImGui::Text("Objects: %zu%s", m_Placements.size(), m_bDirty ? "  *unsaved" : "");

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

void Client::CMapTool::Render_Palette()
{
	ImGui::TextUnformatted("Palette");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##AssetFilter", "filter 17 Valtan parts", m_Filter, sizeof(m_Filter));
	ImGui::BeginChild("AssetPaletteList", ImVec2(0.f, 440.f), true);
	for (const MAP_ASSET_ENTRY& asset : m_Catalog.Get_Entries())
	{
		if (!MatchesFilter(asset.label + " " + asset.id, m_Filter))
			continue;

		const bool_t selected = asset.id == m_SelectedAssetId;
		if (ImGui::Selectable(asset.label.c_str(), selected))
		{
			m_SelectedAssetId = asset.id;
			m_ePlacementState = PLACEMENT_STATE::ARMED;
			m_Status = "Selected " + asset.label + "; click the rendered surface or Y=0 plane";
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(asset.id.c_str());
			ImGui::TextWrapped("%s", asset.modelRelativePath.string().c_str());
			ImGui::EndTooltip();
		}
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_Hierarchy()
{
	ImGui::TextUnformatted("Hierarchy");
	ImGui::BeginChild("PlacementHierarchy", ImVec2(0.f, 475.f), true);
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		const MAP_ASSET_ENTRY* pAsset = m_Catalog.Find(entry.assetId);
		const std::string label = nullptr == pAsset ? entry.assetId : pAsset->label;
		ImGui::PushID(reinterpret_cast<void*>(static_cast<uintptr_t>(entry.placementId)));
		const bool_t selected = entry.placementId == m_iSelectedPlacementId;
		if (ImGui::Selectable(label.c_str(), selected))
			m_iSelectedPlacementId = entry.placementId;
		ImGui::SameLine();
		ImGui::TextDisabled("#%llu", static_cast<unsigned long long>(entry.placementId));
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_Inspector()
{
	ImGui::TextUnformatted("Inspector");
	PLACED_ENTRY* pEntry = Find_Placement(m_iSelectedPlacementId);
	if (nullptr == pEntry || nullptr == pEntry->object)
	{
		ImGui::TextDisabled("Select a placed object.");
		return;
	}

	CMapAssetObject& object = *pEntry->object;
	ImGui::Text("Placement #%llu", static_cast<unsigned long long>(pEntry->placementId));
	ImGui::TextWrapped("Asset: %s", pEntry->assetId.c_str());

	float3_t position = object.Get_Position();
	float3_t rotation = object.Get_RotationDegrees();
	float3_t scale = object.Get_Scale();
	bool_t visible = object.Is_Visible();
	bool_t changed = false;
	changed |= ImGui::DragFloat3("Position", &position.x, 0.1f);
	changed |= ImGui::DragFloat3("Rotation", &rotation.x, 0.5f);
	if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 1000.f))
	{
		scale.x = (std::max)(scale.x, 0.001f);
		scale.y = (std::max)(scale.y, 0.001f);
		scale.z = (std::max)(scale.z, 0.001f);
		changed = true;
	}
	if (changed)
	{
		object.Set_PlacementTransform(position, rotation, scale);
		m_bDirty = true;
	}
	if (ImGui::Checkbox("Visible", &visible))
	{
		object.Set_Visible(visible);
		m_bDirty = true;
	}

	if (ImGui::Button("Delete selected"))
	{
		const uint64_t deletedId = pEntry->placementId;
		if (Remove_Placement(deletedId))
			m_Status = "Deleted placement #" + std::to_string(deletedId);
	}
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
			return entry.placementId == placementId;
		});
	return iter == m_Placements.end() ? nullptr : &*iter;
}

const MAP_ASSET_ENTRY* Client::CMapTool::Get_SelectedAsset() const
{
	return m_Catalog.Find(m_SelectedAssetId);
}
