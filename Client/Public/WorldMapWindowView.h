#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ClientReplication.h"
#include "UIWindowDrag.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Retail world map window (worldmap2.gfx worldMapWnd2, DefaultUIWindow_V2 at stage (230,103),
1440x768 content) reduced to its zone stage, rebuilt as CUI_Sprite slots
(Data/UI/WorldMap/WorldMap_Layout.json, written by Tools/LpkPipeline/build_worldmap_ui.py) under
LEVEL::STATIC, the same construction as CVehicleWindowView.

Retail draws the zone map into a host render target; here the map is one sprite whose UV window
scrolls, zooms and turns (camera forward = up) the area image CMinimapView already uses
(Data/UI/Minimap/MinimapAreas.json: image + retail world bounds in cm). Markers: the local player's
pin (minimaplibrary icon_worldmap_pc), party members and live BOSS entities from
CClientReplication::MINIMAP_MARKER_SNAPSHOT; area-name labels from WorldMapLabels.json
(EFTable_MapString, portal labels blue with the portal symbol); square holes from
WorldMapSquareHoles.json (retail symbol 73); NPC function symbols from WorldMapNpcSymbols.json keyed
by the replicated placement id (EFTable_Npc.MapSymbolIndex). Chrome per the in-game captures:
title band, zone name centred on top (titleTF), continent name top-left (continentNameTF), search
box (display only), the square-hole / legend toggles and their tree panel (WorldMapPanels.json:
ZoneBase rows in SortOrder with the current zone expanded to its square holes, MapLegend
categories), the voyage / memo bottom buttons and the square-hole confirm dialog (dialog.gfx
DialogWindow strings). The Set Sail button and a confirmed square hole are only handed out as
requests; the level controller owns the Server round trip and this view never moves a player.

Controls: M toggles (CMainApp), Esc / close X closes (Esc closes the dialog first), left-drag
pans, wheel zooms about the view center, bottom-left buttons = retail theWholeWorld_btn (fit the
whole area), myLocation_btn (center on the player), playerMark_btn (toggle markers). The
continent / world stages, memo, live search and voyage of the retail window are not placed. */
class CWorldMapWindowView final
{
public:
	/* Cancel transient gestures without closing or moving the window. */
	void Cancel_Interaction() { m_Drag.Reset(); m_bPanning = false; }
	CWorldMapWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CWorldMapWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	/* Screen-pixel rect of the panel while open: CMainApp clips the labels of the windows drawn
	underneath out of it, so no text ever shows through a window on top. */
	bool_t Get_ScreenRect(f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;
	/* Opening recenters on the local player at the default zoom. */
	void Toggle();
	void Close() { m_bOpen = false; m_iDialogHoleId = 0u; }
	/* One Esc press routed here by CMainApp: closes the hole dialog first, then the window. */
	void Handle_EscapeEdge();

	/* Per frame. pSnapshot == nullptr (no minimap-capable level / no local character yet) or an
	unknown eLevel hides the window. Hover / click / drag / wheel / Esc. No-op (hides) while
	closed. */
	void Update(f32_t fTimeDelta, LEVEL eLevel,
		const CClientReplication::MINIMAP_MARKER_SNAPSHOT* pSnapshot);
	/* LOA-font text pass -- after CImGuiLayer::EndFrame() like the other runtime windows. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen. */
	void Hide();
	/* One confirmed square hole since the last call (1-based id of WorldMapSquareHoles.json);
	the window closes itself on the confirm. */
	bool_t Take_SquareHoleRequest(uint16_t& outHoleId);
	/* One Set Sail click since the last call. The map closes before the controller starts the
	Server-owned song, blackout, and ship-trigger landing flow. */
	bool_t Take_ShipTravelRequest();

private:
	struct AREA
	{
		LEVEL		eLevel = LEVEL::END;
		wstring_t	strAreaName;
		string		strImage;
		f32_t		fWorldMinX = 0.f, fWorldMinY = 0.f;	/* retail cm */
		f32_t		fWorldMaxX = 0.f, fWorldMaxY = 0.f;
	};
	struct LABEL
	{
		wstring_t	strName;
		f32_t		fWorldX = 0.f;	/* retail cm */
		f32_t		fWorldY = 0.f;
		float4_t	vColor{ 1.f, 1.f, 1.f, 1.f };	/* MapString <FONT COLOR>, else white */
		bool_t		bPortal = false;				/* coloured names are zone transfers */
	};
	struct LABEL_SET
	{
		LEVEL				eLevel = LEVEL::END;
		std::vector<LABEL>	Labels;
	};
	struct SQUARE_HOLE
	{
		uint16_t	iId = 0;
		wstring_t	strName;
		f32_t		fWorldX = 0.f;	/* retail cm */
		f32_t		fWorldY = 0.f;
		int32_t		iFare = 0;		/* shilling, display only */
	};
	struct HOLE_SET
	{
		LEVEL						eLevel = LEVEL::END;
		std::vector<SQUARE_HOLE>	Holes;
	};
	struct LEGEND_ROW
	{
		string		strKey;
		wstring_t	strName;
		string		strIcon;
		bool_t		bChecked = true;
	};
	struct ZONE_ROW
	{
		int32_t		iZoneId = 0;
		wstring_t	strName;
		bool_t		bPort = false;
	};
	struct ZONE_SET
	{
		LEVEL					eLevel = LEVEL::END;
		int32_t					iCurrentZoneId = 0;
		wstring_t				strContinentName;
		std::vector<ZONE_ROW>	Rows;
	};
	/* One visible tree row this frame (the zone list expands the current zone to its holes). */
	struct PANEL_ROW
	{
		wstring_t	strName;
		float4_t	vColor{ 1.f, 1.f, 1.f, 1.f };
		f32_t		fTextX = 11.f;		/* canvas px from the row's left edge */
		bool_t		bArrow = false;
		bool_t		bArrowUp = false;
		bool_t		bAnchor = false;
		bool_t		bHighlight = false;
		bool_t		bCheck = false;		/* legend check box */
		bool_t		bChecked = false;
		string		strIcon;			/* legend category icon */
		int32_t		iLegendIndex = -1;	/* click: toggle this legend row */
		uint16_t	iHoleId = 0;		/* click: open the confirm dialog for this hole */
	};
	struct TEXT_ITEM
	{
		wstring_t	strFont;
		wstring_t	strText;
		f32_t		fRefX = 0.f;	/* layout reference px: center, left edge (iAlign < 0) or right edge (> 0) */
		f32_t		fRefY = 0.f;	/* text center */
		f32_t		fRefPx = 12.f;
		float4_t	vColor{ 1.f, 1.f, 1.f, 1.f };
		int32_t		iAlign = 0;
		/* Optional second segment drawn right after strText in its own colour (dialog body). */
		wstring_t	strText2;
		float4_t	vColor2{ 1.f, 1.f, 1.f, 1.f };
		/* The confirm dialog's own text. Everything else is the map underneath it, which the text
		pass would otherwise draw straight over the plate -- sprites and LOA-font text are two
		passes, so draw order inside this list cannot put the map behind the dialog. */
		bool_t		bDialog = false;
	};
	enum class PANEL { NONE, SQUARE_HOLE, LEGEND };

	void Load_Areas();
	void Load_Labels();
	void Load_SquareHoles();
	void Load_NpcSymbols();
	void Load_Panels();
	const AREA* Find_Area(LEVEL eLevel) const;
	const LABEL_SET* Find_Labels(LEVEL eLevel) const;
	const HOLE_SET* Find_Holes(LEVEL eLevel) const;
	const ZONE_SET* Find_Zones(LEVEL eLevel) const;
	const SQUARE_HOLE* Find_Hole(LEVEL eLevel, uint16_t iHoleId) const;
	bool_t Is_LegendChecked(const char* pKey) const;
	void Build_PanelRows(LEVEL eLevel, std::vector<PANEL_ROW>& outRows) const;
	void Center_On(f32_t fClientX, f32_t fClientZ);
	void Clamp_Center();
	void Register_TopWindow() const;

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	/* m_SlotIds minus the marker / symbol slots. CUIWindowDrag keeps the union of the slots it
	is given inside the screen every frame; those are parked off screen while hidden and are
	re-placed from the map rect each frame anyway, so they must not take part in that clamp. */
	vector<string>					m_DragSlotIds;
	CUIWindowDrag					m_Drag;
	vector<AREA>					m_Areas;
	vector<LABEL_SET>				m_LabelSets;
	vector<HOLE_SET>				m_HoleSets;
	std::map<string, string>		m_NpcSymbols;	/* placement id -> UI/... icon */
	vector<LEGEND_ROW>				m_LegendRows;
	vector<ZONE_SET>				m_ZoneSets;
	wstring_t						m_strWindowTitle;
	wstring_t						m_strSearchHint;
	wstring_t						m_strDialogTitle;
	wstring_t						m_strConfirmFormat;
	wstring_t						m_strConfirmOk;
	wstring_t						m_strConfirmCancel;
	/* Bottom bar captions (sys.voyage.ui_worldmap_liner_btn,
	sys.squarehole.direct_departure_btn_worldmap, sys.map.memo_btn). */
	wstring_t						m_strLinerButton;
	wstring_t						m_strOceanButton;
	wstring_t						m_strMemoButton;
	vector<TEXT_ITEM>				m_Texts;

	const AREA*	m_pActiveArea = nullptr;
	bool_t		m_bOpen = false;
	bool_t		m_bShowMarkers = true;
	bool_t		m_bRecenterOnUpdate = false;
	int32_t		m_iZoomLevel = 0;
	PANEL		m_ePanel = PANEL::NONE;
	uint16_t	m_iDialogHoleId = 0;	/* confirm dialog open for this hole (0 = closed) */
	/* Reference-resolution rect of the open dialog; Render_Text drops map text inside it. */
	f32_t		m_fDialogX = 0.f, m_fDialogY = 0.f, m_fDialogW = 0.f, m_fDialogH = 0.f;
	uint16_t	m_iPendingHoleId = 0;
	bool_t		m_bPendingShipTravel = false;
	/* Map point (uv of the area image) shown at the view center. */
	f32_t		m_fCenterU = 0.5f;
	f32_t		m_fCenterV = 0.5f;
	/* Visible uv extents of the last frame (Clamp_Center / marker placement). */
	f32_t		m_fScaleU = 1.f;
	f32_t		m_fScaleV = 1.f;
	bool_t		m_bPanning = false;
	f32_t		m_fLastMouseX = 0.f;
	f32_t		m_fLastMouseY = 0.f;
};

NS_END
