#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;

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
		uint64_t placementId = {};
		std::string assetId;
		shared_ptr<CMapAssetObject> object;
	};

public:
	void Toggle();
	void Update(f32_t fTimeDelta);
	void Render();

	bool IsOpen() const;

private:
	void Handle_LevelTransition(bool_t isAssetTest);
	bool_t Try_PickPlacementPosition(float3_t& outPosition) const;
	bool_t Try_PlaceSelected();
	bool_t Create_Placement(uint64_t placementId, const std::string& assetId,
		const float3_t& position, const float3_t& rotationDegrees,
		const float3_t& scale, bool_t visible, PLACED_ENTRY& outEntry);
	bool_t Remove_Placement(uint64_t placementId);
	void Remove_AllPlacements();
	bool_t Save_Placements();
	bool_t Load_Placements();

	void Render_Toolbar();
	void Render_Palette();
	void Render_Hierarchy();
	void Render_Inspector();
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
	std::string m_SelectedAssetId;
	std::string m_Status = "Enter AssetTest with F2";
	char m_Filter[128]{};

	vector<PLACED_ENTRY> m_Placements;
	uint64_t m_iSelectedPlacementId = {};
	uint64_t m_iNextPlacementId = 1;
};

NS_END
