#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)
class CUILayoutRuntime;

/* Top-right area minimap, rebuilt from retail EFUI_MAP minimap.gfx (MinimapFrame, 326x264 at
1920x1080 anchored top-right, authored here at 1280x720 in Data/UI/Minimap/Minimap_Layout.json).
The map itself is one CUI_Sprite whose UV window scrolls/zooms across the retail minimap tile
image of the current area (Data/UI/Minimap/MinimapAreas.json: image + retail world bounds in cm,
from MinimapData.loa), masked to the round window. Markers are the local player's arrow, party
members and live BOSS entities, all read from CClientReplication::MINIMAP_MARKER_SNAPSHOT --
this view never decides gameplay. */
class CMinimapView final
{
public:
	CMinimapView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	~CMinimapView();

public:
	/* pSnapshot == nullptr (no minimap-capable level active, or no local character yet) hides
	every slot. eLevel selects the area image/bounds; an unknown level hides the view too. */
	void Update(f32_t fTimeDelta, LEVEL eLevel,
		const CClientReplication::MINIMAP_MARKER_SNAPSHOT* pSnapshot);
	/* Area name in the frame header -- after EndFrame() like every other LOA-font label. */
	void RenderText();

private:
	struct AREA
	{
		LEVEL		eLevel = LEVEL::END;
		wstring_t	strAreaName;
		string		strImage;
		f32_t		fWorldMinX = 0.f, fWorldMinY = 0.f;	/* retail cm */
		f32_t		fWorldMaxX = 0.f, fWorldMaxY = 0.f;
		/* Optional selection box (retail cm): which positions pick this area. Defaults to the map box.
		A harbour that shares a wide image with unrelated ground selects by a narrower box. */
		f32_t		fSelectMinX = 0.f, fSelectMinY = 0.f;
		f32_t		fSelectMaxX = 0.f, fSelectMaxY = 0.f;
		/* Optional: the Data/Worlds area whose NPC placements carry world map symbols.
		Empty for an area with none, which then draws no NPC markers. */
		string		strWorldAreaId;
		/* Data "default": the entry a position outside every box of its level falls back to. */
		bool_t		bDefault = false;
		/* Optional extra boxes (retail cm) that also pick this area, for ground that is not one rectangle
		(the open sea around a harbour town). Data "extraSelectBoxesCm": [[minX, minY, maxX, maxY], ...]. */
		struct SELECT_BOX
		{
			f32_t	fMinX = 0.f, fMinY = 0.f, fMaxX = 0.f, fMaxY = 0.f;
		};
		vector<SELECT_BOX>	ExtraSelect;
		/* Optional landmarks: placements of strWorldAreaId (a trigger box, not an NPC) drawn with an icon that
		sticks to the window edge when the place is outside the visible part of the map, so the player can
		find its direction. Data "landmarks": [{ "placementId", "icon" }, ...]. */
		struct LANDMARK
		{
			string	strPlacementId;
			string	strIcon;
		};
		vector<LANDMARK>	Landmarks;
	};

	HRESULT Load_Areas();
	/* NPC placements of strWorldAreaId joined with Data/UI/WorldMap/WorldMapNpcSymbols.json,
	the same pair CWorldMapWindowView draws on the M map. Loaded once per area change; the
	symbols are static authored placements, so there is nothing to refresh per frame. */
	void Load_AreaNpcSymbols(const AREA& Area);
	/* A level may list several areas (Bern's indoor volumes, then its outdoor map). The first entry
	whose retail-cm box contains the local position wins, in file order; a position inside none of
	them falls back to the entry marked "default", else the level's first, which keeps every
	single-area level as before. */
	const AREA* Find_Area(LEVEL eLevel, f32_t fClientX, f32_t fClientZ) const;
	void Hide_All();

private:
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<AREA>					m_Areas;
	struct NPC_SYMBOL
	{
		string	strIconPath;
		f32_t	fWorldX = 0.f;
		f32_t	fWorldZ = 0.f;
		bool_t	bEdgeClamp = false;	/* landmark: keeps to the window edge when off the visible map */
	};
	vector<NPC_SYMBOL>				m_NpcSymbols;
	string							m_strLoadedNpcAreaId;
	const AREA*						m_pActiveArea = nullptr;
	int32_t							m_iZoomLevel = 2;
	bool_t							m_bSliderOpen = false;
	bool_t							m_bMapVisible = true;
	bool_t							m_bAnySlotVisible = false;
};

NS_END
