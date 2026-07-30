#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapAssetPreview.h"
#include "MapPlacementDocument.h"
#include "DeployPropCatalog.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CMapStaticBatchObject;
class CDeployPropObject;
class CMapTool final
{
private:
	enum class PLACEMENT_STATE
	{
		IDLE,
		ARMED,
	};

	struct PLACED_ENTRY
	{
		MAP_PLACEMENT_RECORD record;
		std::wstring layerTag;
		shared_ptr<CMapAssetObject> object;
		shared_ptr<CMapStaticBatchObject> batch;
	};

	struct STATIC_BATCH_ENTRY
	{
		std::string assetId;
		bool_t mirrored = false;
		shared_ptr<CMapStaticBatchObject> object;
	};

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
	~CMapTool();

	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	void Toggle();
	void Update(f32_t fTimeDelta);
	void Render();

	bool IsOpen() const;

private:
	void Handle_LevelTransition(bool_t isAssetTest);
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

	void Render_Toolbar();
	void Render_Palette(f32_t childHeight);
	void Render_Hierarchy(f32_t childHeight);
	void Render_Inspector();
	void Render_AssetPreview();
	void Render_DecoderReport() const;

	PLACED_ENTRY* Find_Placement(uint64_t placementId);
	const MAP_ASSET_ENTRY* Get_SelectedAsset() const;

private:
	bool_t m_bOpen = false;
	bool_t m_bWasInAssetTest = false;
	bool_t m_bPreviousMouseDown = false;
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
};

NS_END
