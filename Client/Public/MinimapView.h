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
		/* Optional: the Data/Worlds area whose NPC placements carry world map symbols.
		Empty for an area with none, which then draws no NPC markers. */
		string		strWorldAreaId;
	};

	HRESULT Load_Areas();
	/* NPC placements of strWorldAreaId joined with Data/UI/WorldMap/WorldMapNpcSymbols.json,
	the same pair CWorldMapWindowView draws on the M map. Loaded once per area change; the
	symbols are static authored placements, so there is nothing to refresh per frame. */
	void Load_AreaNpcSymbols(const AREA& Area);
	const AREA* Find_Area(LEVEL eLevel) const;
	void Hide_All();

private:
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<AREA>					m_Areas;
	struct NPC_SYMBOL
	{
		string	strIconPath;
		f32_t	fWorldX = 0.f;
		f32_t	fWorldZ = 0.f;
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
