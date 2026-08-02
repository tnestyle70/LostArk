#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapAssetPreview.h"
#include "MapPlacementDocument.h"
#include "MapPlacementRuntime.h"
#include "DeployPropCatalog.h"
#include "NavGridBaker.h"
#include "NavGridPaintDocument.h"
#include "NavRuntimeBlockerDocument.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CMapStaticBatchObject;
class CDeployPropObject;
class CCamera_Free;
class CMapTool final
{
private:
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		NAVIGATION,
		CAMERA,
	};

	enum class NAVIGATION_MODE
	{
		BAKE,
		WALKABILITY,
		DESTRUCTION_AREA,
	};

	enum class NAVIGATION_EDIT_ACTION
	{
		APPLY,
		ERASE,
	};
	//bake 가이드 메시
	enum class NAV_BOUNDS_STATE
	{
		IDLE,
		PLACING,
	};

	struct NAVIGATION_RENDER_RESOURCES;

	enum class PLACEMENT_STATE
	{
		IDLE,
		ARMED,
	};

	using PLACED_ENTRY = MAP_RUNTIME_PLACED_ENTRY;
	using STATIC_BATCH_ENTRY = MAP_RUNTIME_STATIC_BATCH_ENTRY;

	enum class ENVIRONMENT_PHASE
	{
		BASELINE,
		SPACEHOLE,
		CHAOS_GATE,
	};

	struct DEPLOY_ENTRY
	{
		DEPLOY_PROP_PLACEMENT record;
		shared_ptr<CDeployPropObject> object;
	};

public:
	CMapTool();
	~CMapTool();

	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	void Toggle();
	void Update(f32_t fTimeDelta);
	void Render();

	bool IsOpen() const;
	bool_t ConsumesWorldLeftMouse() const;

private:
	/* Frame Update */
	void Update_EnvironmentShortcuts(bool_t isAssetTest);
	void Update_WorldInteraction(bool_t isAssetTest);
	void Handle_LevelTransition(bool_t isAssetTest);

	/* Frame Render */
	void Render_WorldOverlay(bool_t isAssetTest);
	void Render_ActiveMode(bool_t isAssetTest);
	void Render_MapAssetsPanel(bool_t isAssetTest);
	void Render_ModeBar();
	void Render_CameraPanel();
	void Render_NavigationPanel();
	void Render_DestructionAreaControls();
	void Render_NavigationDiagnostics();
	void Render_NavigationOverlay();
	void Render_NavigationBakeControls();
	void Render_NavigationBoundsOverlay();
	void Render_Toolbar();
	void Render_Palette(f32_t childHeight);
	void Render_Hierarchy(f32_t childHeight);
	void Render_Inspector();
	void Render_AssetPreview();
	void Render_DecoderReport() const;

	/* Camera Runtime */
	bool_t Find_AssetTestCamera();

	/* Navigation Document and Runtime */
	bool_t Load_NavigationDocument();
	bool_t Load_RuntimeBlockers();
	bool_t Register_RuntimeBlockers();
	bool_t Set_NavigationCondition(
		const std::string& conditionId,
		bool_t value);
	bool_t Save_Navigation();
	bool_t Try_PickNavigationCell(
		int32_t& outCellX,
		int32_t& outCellZ) const;
	bool_t Try_PaintNavigation();
	bool_t Try_PlaceNavigationBounds();
	bool_t Bake_Navigation();
	bool_t Collect_NavigationBakePlacements(
		std::vector<NAVGRID_BAKE_PLACEMENT>& outPlacements,
		std::string& outStatus) const;
	bool_t Is_CellInsideNavigationBounds(
		f32_t worldX,
		f32_t worldZ) const;
	static bool_t Is_ValidNavigationBakeDesc(
		const NAVGRID_BAKE_DESC& desc);

	/* Map Asset Document and Runtime */
	bool_t Try_PickPlacementPosition(float3_t& outPosition) const;
	bool_t Try_PlaceSelected();
	bool_t Create_Placement(const MAP_PLACEMENT_RECORD& record,
		PLACED_ENTRY& outEntry);
	bool_t Stage_PlacementRuntime(
		const vector<MAP_PLACEMENT_RECORD>& records,
		vector<PLACED_ENTRY>& outPlacements,
		vector<STATIC_BATCH_ENTRY>& outBatches);
	void Remove_PlacementRuntime(
		vector<PLACED_ENTRY>& placements,
		vector<STATIC_BATCH_ENTRY>& batches);
	static bool_t Set_RuntimeVisible(
		PLACED_ENTRY& entry, bool_t visible);
	bool_t Remove_Placement(uint64_t placementId);
	void Remove_AllPlacements();
	bool_t Save_Placements();
	bool_t Load_Placements();
	bool_t Load_DeployProps();
	void Remove_DeployProps();
	void Set_DeployPhase(DEPLOY_PROP_STATE state);
	void Set_EnvironmentPhase(ENVIRONMENT_PHASE phase);
	uint64_t Allocate_EditorPlacementId();
	std::wstring Make_LayerTag(const std::string& sourceLevel) const;
	void Select_Asset(const MAP_ASSET_ENTRY& asset);
	void Arm_SelectedAsset();

	/* Queries */
	PLACED_ENTRY* Find_Placement(uint64_t placementId);
	const MAP_ASSET_ENTRY* Get_SelectedAsset() const;

private:
	/* Shared Tool State */
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	bool_t m_bOpen = false;
	bool_t m_bWasInAssetTest = false;
	TOOL_MODE m_eToolMode = TOOL_MODE::MAP_ASSETS;

	/* World Interaction State */
	bool_t m_bPreviousMouseDown = false;

	/* Map Asset State */
	bool_t m_bDirty = false;
	PLACEMENT_STATE m_ePlacementState = PLACEMENT_STATE::IDLE;

	CMapAssetCatalog m_Catalog;
	CDeployPropCatalog m_DeployCatalog;
	std::unique_ptr<CMapAssetPreview> m_pAssetPreview;
	std::string m_SelectedAssetId;
	std::string m_Status = "Enter AssetTest with F2";
	char m_Filter[128]{};
	std::unordered_set<std::string> m_FavoriteAssetIds;

	vector<PLACED_ENTRY> m_Placements;
	vector<STATIC_BATCH_ENTRY> m_StaticBatches;
	vector<DEPLOY_ENTRY> m_DeployProps;
	DEPLOY_PROP_STATE m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
	ENVIRONMENT_PHASE m_EnvironmentPhase = ENVIRONMENT_PHASE::BASELINE;
	uint64_t m_iSelectedPlacementId = {};
	uint64_t m_iNextPlacementId = 1;

	/* Navigation State */
	bool_t m_bNavigationStrokeActive = false;
	NAVIGATION_MODE m_eNavigationMode =
		NAVIGATION_MODE::BAKE;
	NAV_BOUNDS_STATE m_eNavigationBoundsState =
		NAV_BOUNDS_STATE::IDLE;
	NAVGRID_BAKE_DESC m_NavigationBakeDesc;
	std::string m_NavigationBakeStatus = "Create Nav Bounds";
	bool_t m_bNavigationBakeResetConfirmed = false;
	bool_t m_bNavigationBakeResetPending = false;

	NAVIGATION_EDIT_ACTION m_eNavigationEditAction =
		NAVIGATION_EDIT_ACTION::APPLY;
	uint32_t m_iBrushRadius = {};

	CNavGridPaintDocument m_NavigationDocument;
	CNavRuntimeBlockerDocument m_RuntimeBlockerDocument;
	std::filesystem::path m_NavigationSourcePath;
	std::filesystem::path m_NavigationPaintPath;
	std::filesystem::path m_NavigationRuntimePath;
	std::filesystem::path m_RuntimeBlockerPath;
	std::string m_NavigationStatus = "Open ASSET_TEST with F2";
	size_t m_iSelectedRuntimeRegion = {};
	char m_RuntimeBlockerId[128] =
		"VALTAN_OUTER_RING_COLLAPSE";
	char m_RuntimeConditionId[128] =
		"VALTAN_ARENA_DESTROYED";
	bool_t m_RuntimeActivateWhenConditionTrue = true;
	std::unordered_map<std::string, bool_t> m_NavigationConditions;
	std::unique_ptr<NAVIGATION_RENDER_RESOURCES>
		m_pNavigationRenderResources;

	/* Camera State */
	weak_ptr<CCamera_Free> m_pAssetTestCamera;
	std::string m_CameraStatus = "Open ASSET_TEST with F2";
};

NS_END
