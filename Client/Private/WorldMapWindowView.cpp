/* WinSock2 before Windows.h (Client_Defines pulls the socket headers). */
#include <WinSock2.h>

#include "WorldMapWindowView.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	/* Zoom steps as multiples of "the whole area height fits the view" (index 1). Index 2 is
	the retail default for Bern (EFTable_MapFactor.WorldmapScaleFactor 1.4). */
	constexpr f32_t ZOOM_STEPS[] = { 0.7f, 1.f, 1.4f, 2.f, 2.8f, 4.f };
	constexpr int32_t ZOOM_COUNT = static_cast<int32_t>(sizeof(ZOOM_STEPS) / sizeof(ZOOM_STEPS[0]));
	constexpr int32_t ZOOM_DEFAULT = 2;
	constexpr int32_t ZOOM_FIT = 1;
	constexpr size_t PARTY_MARKER_COUNT = 4;
	constexpr size_t BOSS_MARKER_COUNT = 2;
	/* Same counts as build_worldmap_ui.py (HOLE_SLOTS / NPC_SYMBOL_SLOTS / PORTAL_SLOTS / PANEL_ROWS). */
	constexpr size_t HOLE_SLOT_COUNT = 4;
	/* Bern places 38 NPCs with an original map symbol; the extra slots leave room for a
	fuller placement set. Keep this equal to the WM_NpcSym_* slot count of WorldMap_Layout.json. */
	constexpr size_t NPC_SYMBOL_SLOT_COUNT = 48;
	constexpr size_t PORTAL_SLOT_COUNT = 4;
	constexpr size_t PANEL_ROW_COUNT = 16;
	/* Retail px on the 1440 canvas; everything scales with the layout (2/3). */
	constexpr f32_t CONTENT_W = 1440.f;
	/* Title band: YoonGasiIIM 18 px centred. titleTF (zone name) sits centred on the top of the
	content, continentNameTF (continent name) top-left, YoonGasiIIM 14 px drawn at 1.5x. Tree rows:
	$YG760 14 px, zone text at x+11, hole (child) text at x+20, legend text at x+63, 37 px pitch.
	Search hint: searchInput_txt placeholder, $YG760 14 px. Dialog (dialog.gfx DialogWindow):
	title YoonGasiIIM 18 px #fff7e2, body / buttons $YG760 14 px; a renew button's label is centred
	on its text field at (31,5) 67x25. */
	constexpr f32_t HEADER_TITLE_PX = 18.f;
	constexpr f32_t ZONE_TITLE_PX = 18.f;
	constexpr f32_t ZONE_TITLE_CENTER_Y = 27.f;
	constexpr f32_t CONTINENT_PX = 21.f;
	constexpr f32_t CONTINENT_X = 29.f;
	constexpr f32_t CONTINENT_CENTER_Y = 39.f;
	constexpr f32_t ROW_TEXT_PX = 14.f;
	constexpr f32_t ROW_TEXT_X_ZONE = 11.f;
	constexpr f32_t ROW_TEXT_X_HOLE = 20.f;
	constexpr f32_t ROW_TEXT_X_LEGEND = 63.f;
	constexpr f32_t SEARCH_HINT_PX = 14.f;
	constexpr f32_t SEARCH_HINT_INSET_X = 10.f;
	constexpr f32_t BOTTOM_BTN_PX = 14.f;
	constexpr f32_t DLG_TITLE_PX = 18.f;
	constexpr f32_t DLG_TITLE_CENTER_Y = 18.f;
	constexpr f32_t DLG_BODY_PX = 14.f;
	constexpr f32_t DLG_BODY_CENTER_Y = 60.f;
	constexpr f32_t DLG_FARE_CENTER_Y = 90.f;
	constexpr f32_t DLG_BTN_TEXT_X = 31.f + 67.f * 0.5f;
	constexpr f32_t DLG_BTN_TEXT_Y = 5.f + 25.f * 0.5f;
	/* Zone-name labels: retail WorldMapZoneNameRenderer textField = $YG760 280 twips (14 px on the
	1440 canvas), centered, white unless the MapString name carries its own <FONT COLOR>. The blue
	portal names get the portal symbol above their left end (capture). */
	constexpr f32_t LABEL_PX = 14.f;
	constexpr f32_t PORTAL_OFFSET_X = -12.f;
	constexpr f32_t PORTAL_OFFSET_Y = -24.f;
	constexpr f32_t TEXT_BOOST = 1.15f;
	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	constexpr float4_t COLOR_WHITE{ 1.f, 1.f, 1.f, 1.f };
	constexpr float4_t COLOR_DLG_TITLE{ 1.f, 247.f / 255.f, 226.f / 255.f, 1.f };	/* #fff7e2 */
	/* SquareHoleRollingTreeItem: default 0xFFFFFF, my zone 0xFFD700; the dialog's zone name is
	<FONT COLOR='#FFFF00'>. */
	constexpr float4_t COLOR_GOLD{ 1.f, 215.f / 255.f, 0.f, 1.f };
	constexpr float4_t COLOR_YELLOW{ 1.f, 1.f, 0.f, 1.f };
	constexpr float4_t COLOR_HINT{ 140.f / 255.f, 140.f / 255.f, 140.f / 255.f, 1.f };
	constexpr float4_t TINT_NORMAL{ 1.f, 1.f, 1.f, 1.f };
	constexpr float4_t TINT_HOVER{ 1.f, 1.f, 0.6f, 1.f };
	/* The selected toggle plate and the active voyage button are the gold button state. */
	constexpr float4_t TINT_GOLD_PLATE{ 1.f, 0.8f, 0.35f, 1.f };
	/* Retail draws the zone picture with the camera's forward direction up: every product camera
	(Data/Camera/*.camera.json) yaws 135 deg = client (+X, -Z), the image's up-right diagonal, so
	the picture turns 45 deg counter-clockwise on screen (measured on a retail capture: -43 deg at
	scale 1.40 = WorldmapScaleFactor). Positive = the clockwise turn CUI_Sprite::Set_UVRotation
	samples with; screen <-> texture offsets below use the matching inverse. */
	constexpr f32_t MAP_ROTATION_DEG = 45.f;

	const char* const PARTY_SLOTS[PARTY_MARKER_COUNT] =
		{ "WM_Party_0", "WM_Party_1", "WM_Party_2", "WM_Party_3" };
	const char* const BOSS_SLOTS[BOSS_MARKER_COUNT] = { "WM_Boss_0", "WM_Boss_1" };
	const char* const HOLE_SLOTS[HOLE_SLOT_COUNT] = { "WM_Hole_0", "WM_Hole_1", "WM_Hole_2", "WM_Hole_3" };
	const char* const PORTAL_SLOTS[PORTAL_SLOT_COUNT] = { "WM_Portal_0", "WM_Portal_1", "WM_Portal_2", "WM_Portal_3" };
	/* Slot ids are built once so the per-frame marker pass never allocates. */
	const std::array<string, NPC_SYMBOL_SLOT_COUNT>& Get_NpcSymbolSlots()
	{
		static const std::array<string, NPC_SYMBOL_SLOT_COUNT> Slots = []
		{
			std::array<string, NPC_SYMBOL_SLOT_COUNT> Ids;
			for (size_t i = 0; i < NPC_SYMBOL_SLOT_COUNT; ++i)
				Ids[i] = "WM_NpcSym_" + std::to_string(i);
			return Ids;
		}();
		return Slots;
	}
	const char* const DIALOG_SLOTS[] =
		{ "WM_DlgBg", "WM_DlgDeco", "WM_DlgCoin", "WM_DlgBtnOk", "WM_DlgIconOk", "WM_DlgBtnCancel", "WM_DlgIconCancel" };

	bool_t Convert_Utf8(const string& strUtf8, wstring_t& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int iLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iLength <= 0)
			return false;
		outWide.resize(static_cast<size_t>(iLength));
		return iLength == MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), outWide.data(), iLength);
	}

	LEVEL Parse_Level(const string& strName)
	{
		if ("BERN" == strName) return LEVEL::BERN;
		if ("VALTAN_ARENA" == strName) return LEVEL::VALTAN_ARENA;
		if ("KAKULSAYDON_ARENA" == strName) return LEVEL::KAKULSAYDON_ARENA;
		return LEVEL::END;
	}

	bool_t Read_Document(const wchar_t* pRelativePath, DATA_JSON_VALUE& outRoot, const char* pTag)
	{
		const filesystem::path DataPath = CProjectDataRoot::Resolve(pRelativePath);
		ifstream Stream(DataPath, ios::binary);
		if (!Stream.is_open())
		{
			OutputDebugStringA((string("[WorldMapWindow] ") + pTag + " missing.\n").c_str());
			return false;
		}
		const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
		string Error;
		if (!CDataJson::Parse(Text, outRoot, Error) || !outRoot.Is_Object())
		{
			OutputDebugStringA((string("[WorldMapWindow] ") + pTag + " parse failed: " + Error + "\n").c_str());
			return false;
		}
		return true;
	}

	bool_t Read_String(const DATA_JSON_VALUE& Object, const char* pKey, wstring_t& outText)
	{
		const DATA_JSON_VALUE* pValue = Object.Is_Object() ? Object.Find(pKey) : nullptr;
		return nullptr != pValue && pValue->Is_String() && Convert_Utf8(pValue->Get_String(), outText);
	}

	bool_t Read_Bool(const DATA_JSON_VALUE& Object, const char* pKey)
	{
		const DATA_JSON_VALUE* pValue = Object.Is_Object() ? Object.Find(pKey) : nullptr;
		return nullptr != pValue && pValue->Is_Boolean() && pValue->Get_Boolean();
	}

	bool_t Read_Cm(const DATA_JSON_VALUE& Object, f32_t& outX, f32_t& outY)
	{
		const DATA_JSON_VALUE* pWorld = Object.Is_Object() ? Object.Find("worldCm") : nullptr;
		if (nullptr == pWorld || !pWorld->Is_Array() || pWorld->Get_Array().size() < 2 ||
			!pWorld->Get_Array()[0].Is_Number() || !pWorld->Get_Array()[1].Is_Number())
		{
			return false;
		}
		outX = static_cast<f32_t>(pWorld->Get_Array()[0].Get_Number());
		outY = static_cast<f32_t>(pWorld->Get_Array()[1].Get_Number());
		return true;
	}

	bool_t Starts_With(const string& strText, const char* pPrefix)
	{
		return 0 == strText.rfind(pPrefix, 0);
	}

	/* Korean directional particle after a name: "-ro" after a vowel or final rieul, "-euro" after
	any other final consonant (the retail string ships the bare "-ro" and lets the host fix it). */
	wstring_t Particle_Ro(const wstring_t& strName)
	{
		if (strName.empty())
			return L"\xB85C";	/* -ro */
		const wchar_t c = strName.back();
		if (c < 0xAC00 || c > 0xD7A3)
			return L"\xB85C";
		const int32_t iFinal = (c - 0xAC00) % 28;
		return (0 == iFinal || 8 == iFinal) ? L"\xB85C" : L"\xC73C\xB85C";	/* -ro : -euro */
	}
}

Client::CWorldMapWindowView::CWorldMapWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/WorldMap/WorldMap_Layout.json") }
	, m_iZoomLevel{ ZOOM_DEFAULT }
{
	m_SlotIds = m_pView->Get_SlotIds();
	for (const string& strId : m_SlotIds)
	{
		const bool_t bParked = "WM_Player" == strId ||
			Starts_With(strId, "WM_Party_") || Starts_With(strId, "WM_Boss_") ||
			Starts_With(strId, "WM_Hole_") || Starts_With(strId, "WM_NpcSym_") ||
			Starts_With(strId, "WM_Portal_");
		if (!bParked)
			m_DragSlotIds.push_back(strId);
	}
	Load_Areas();
	Load_Labels();
	Load_SquareHoles();
	Load_NpcSymbols();
	Load_Panels();
	/* LEVEL::STATIC sprites are visible from construction. */
	Hide();
}

Client::CWorldMapWindowView::~CWorldMapWindowView() = default;

void Client::CWorldMapWindowView::Load_Areas()
{
	m_Areas.clear();
	DATA_JSON_VALUE Root;
	if (!Read_Document(L"UI/Minimap/MinimapAreas.json", Root, "MinimapAreas.json"))
		return;
	const DATA_JSON_VALUE* pAreas = Root.Find("areas");
	if (nullptr == pAreas || !pAreas->Is_Array())
		return;
	for (const DATA_JSON_VALUE& Value : pAreas->Get_Array())
	{
		if (!Value.Is_Object())
			continue;
		const DATA_JSON_VALUE* pLevel = Value.Find("level");
		const DATA_JSON_VALUE* pName = Value.Find("areaName");
		const DATA_JSON_VALUE* pImage = Value.Find("image");
		const DATA_JSON_VALUE* pMin = Value.Find("worldMinCm");
		const DATA_JSON_VALUE* pMax = Value.Find("worldMaxCm");
		if (nullptr == pLevel || !pLevel->Is_String() || nullptr == pImage || !pImage->Is_String() ||
			nullptr == pMin || !pMin->Is_Array() || pMin->Get_Array().size() < 2 ||
			nullptr == pMax || !pMax->Is_Array() || pMax->Get_Array().size() < 2)
		{
			continue;
		}
		AREA Area;
		Area.eLevel = Parse_Level(pLevel->Get_String());
		if (LEVEL::END == Area.eLevel)
			continue;
		Area.strImage = pImage->Get_String();
		if (nullptr != pName && pName->Is_String())
			(void)Convert_Utf8(pName->Get_String(), Area.strAreaName);
		const auto& Min = pMin->Get_Array();
		const auto& Max = pMax->Get_Array();
		if (!Min[0].Is_Number() || !Min[1].Is_Number() || !Max[0].Is_Number() || !Max[1].Is_Number())
			continue;
		Area.fWorldMinX = static_cast<f32_t>(Min[0].Get_Number());
		Area.fWorldMinY = static_cast<f32_t>(Min[1].Get_Number());
		Area.fWorldMaxX = static_cast<f32_t>(Max[0].Get_Number());
		Area.fWorldMaxY = static_cast<f32_t>(Max[1].Get_Number());
		if (Area.fWorldMaxX <= Area.fWorldMinX || Area.fWorldMaxY <= Area.fWorldMinY)
			continue;
		m_Areas.push_back(std::move(Area));
	}
}

void Client::CWorldMapWindowView::Load_Labels()
{
	m_LabelSets.clear();
	DATA_JSON_VALUE Root;
	if (!Read_Document(L"UI/WorldMap/WorldMapLabels.json", Root, "WorldMapLabels.json"))
		return;
	const DATA_JSON_VALUE* pLevels = Root.Find("levels");
	if (nullptr == pLevels || !pLevels->Is_Array())
		return;
	for (const DATA_JSON_VALUE& Value : pLevels->Get_Array())
	{
		const DATA_JSON_VALUE* pLevel = Value.Is_Object() ? Value.Find("level") : nullptr;
		const DATA_JSON_VALUE* pLabels = Value.Is_Object() ? Value.Find("labels") : nullptr;
		if (nullptr == pLevel || !pLevel->Is_String() || nullptr == pLabels || !pLabels->Is_Array())
			continue;
		LABEL_SET Set;
		Set.eLevel = Parse_Level(pLevel->Get_String());
		if (LEVEL::END == Set.eLevel)
			continue;
		for (const DATA_JSON_VALUE& Row : pLabels->Get_Array())
		{
			LABEL Label;
			if (!Read_String(Row, "name", Label.strName) || Label.strName.empty() ||
				!Read_Cm(Row, Label.fWorldX, Label.fWorldY))
			{
				continue;
			}
			/* Retail tables carry inline <FONT> markup on a few names; keep the plain text. */
			if (wstring_t::npos != Label.strName.find(L'<'))
			{
				wstring_t strPlain;
				bool_t bInTag = false;
				for (const wchar_t c : Label.strName)
				{
					if (L'<' == c) { bInTag = true; continue; }
					if (L'>' == c) { bInTag = false; continue; }
					if (!bInTag) strPlain.push_back(c);
				}
				Label.strName = std::move(strPlain);
			}
			/* Optional "#RRGGBB" (the generator keeps the MapString <FONT COLOR> here). */
			if (const DATA_JSON_VALUE* pColor = Row.Find("color"))
			{
				if (pColor->Is_String() && 7u == pColor->Get_String().size() && '#' == pColor->Get_String()[0])
				{
					const uint32_t iRGB = static_cast<uint32_t>(
						std::strtoul(pColor->Get_String().c_str() + 1, nullptr, 16));
					Label.vColor = float4_t(
						static_cast<f32_t>((iRGB >> 16) & 0xFF) / 255.f,
						static_cast<f32_t>((iRGB >> 8) & 0xFF) / 255.f,
						static_cast<f32_t>(iRGB & 0xFF) / 255.f, 1.f);
					Label.bPortal = true;
				}
			}
			Set.Labels.push_back(std::move(Label));
		}
		m_LabelSets.push_back(std::move(Set));
	}
}

void Client::CWorldMapWindowView::Load_SquareHoles()
{
	m_HoleSets.clear();
	DATA_JSON_VALUE Root;
	if (!Read_Document(L"UI/WorldMap/WorldMapSquareHoles.json", Root, "WorldMapSquareHoles.json"))
		return;
	const DATA_JSON_VALUE* pLevels = Root.Find("levels");
	if (nullptr == pLevels || !pLevels->Is_Array())
		return;
	for (const DATA_JSON_VALUE& Value : pLevels->Get_Array())
	{
		const DATA_JSON_VALUE* pLevel = Value.Is_Object() ? Value.Find("level") : nullptr;
		const DATA_JSON_VALUE* pHoles = Value.Is_Object() ? Value.Find("holes") : nullptr;
		if (nullptr == pLevel || !pLevel->Is_String() || nullptr == pHoles || !pHoles->Is_Array())
			continue;
		HOLE_SET Set;
		Set.eLevel = Parse_Level(pLevel->Get_String());
		if (LEVEL::END == Set.eLevel)
			continue;
		for (const DATA_JSON_VALUE& Row : pHoles->Get_Array())
		{
			const DATA_JSON_VALUE* pId = Row.Is_Object() ? Row.Find("id") : nullptr;
			SQUARE_HOLE Hole;
			if (nullptr == pId || !pId->Is_Number() || pId->Get_Number() < 1.0 ||
				!Read_Cm(Row, Hole.fWorldX, Hole.fWorldY))
			{
				continue;
			}
			Hole.iId = static_cast<uint16_t>(pId->Get_Number());
			(void)Read_String(Row, "name", Hole.strName);
			if (const DATA_JSON_VALUE* pFare = Row.Find("fareShilling"))
				if (pFare->Is_Number())
					Hole.iFare = static_cast<int32_t>(pFare->Get_Number());
			Set.Holes.push_back(std::move(Hole));
		}
		m_HoleSets.push_back(std::move(Set));
	}
}

void Client::CWorldMapWindowView::Load_NpcSymbols()
{
	m_NpcSymbols.clear();
	DATA_JSON_VALUE Root;
	if (!Read_Document(L"UI/WorldMap/WorldMapNpcSymbols.json", Root, "WorldMapNpcSymbols.json"))
		return;
	const DATA_JSON_VALUE* pPlacements = Root.Find("placements");
	if (nullptr == pPlacements || !pPlacements->Is_Object())
		return;
	for (const auto& [strPlacementId, Icon] : pPlacements->Get_Object())
	{
		if (Icon.Is_String() && !Icon.Get_String().empty())
			m_NpcSymbols[strPlacementId] = Icon.Get_String();
	}
}

void Client::CWorldMapWindowView::Load_Panels()
{
	m_LegendRows.clear();
	m_ZoneSets.clear();
	DATA_JSON_VALUE Root;
	if (!Read_Document(L"UI/WorldMap/WorldMapPanels.json", Root, "WorldMapPanels.json"))
		return;
	if (const DATA_JSON_VALUE* pStrings = Root.Find("strings"))
	{
		(void)Read_String(*pStrings, "windowTitle", m_strWindowTitle);
		(void)Read_String(*pStrings, "searchHint", m_strSearchHint);
		(void)Read_String(*pStrings, "dialogTitle", m_strDialogTitle);
		(void)Read_String(*pStrings, "confirmFormat", m_strConfirmFormat);
		(void)Read_String(*pStrings, "confirmOk", m_strConfirmOk);
		(void)Read_String(*pStrings, "confirmCancel", m_strConfirmCancel);
		(void)Read_String(*pStrings, "linerButton", m_strLinerButton);
		(void)Read_String(*pStrings, "oceanButton", m_strOceanButton);
		(void)Read_String(*pStrings, "memoButton", m_strMemoButton);
	}
	if (const DATA_JSON_VALUE* pLegend = Root.Find("legend"))
	{
		if (pLegend->Is_Array())
		{
			for (const DATA_JSON_VALUE& Row : pLegend->Get_Array())
			{
				const DATA_JSON_VALUE* pKey = Row.Is_Object() ? Row.Find("key") : nullptr;
				const DATA_JSON_VALUE* pIcon = Row.Is_Object() ? Row.Find("icon") : nullptr;
				LEGEND_ROW Legend;
				if (nullptr == pKey || !pKey->Is_String() || !Read_String(Row, "name", Legend.strName))
					continue;
				Legend.strKey = pKey->Get_String();
				if (nullptr != pIcon && pIcon->Is_String())
					Legend.strIcon = pIcon->Get_String();
				m_LegendRows.push_back(std::move(Legend));
			}
		}
	}
	if (const DATA_JSON_VALUE* pZones = Root.Find("zones"))
	{
		if (pZones->Is_Object())
		{
			for (const auto& [strLevel, Value] : pZones->Get_Object())
			{
				ZONE_SET Set;
				Set.eLevel = Parse_Level(strLevel);
				const DATA_JSON_VALUE* pCurrent = Value.Is_Object() ? Value.Find("currentZoneId") : nullptr;
				const DATA_JSON_VALUE* pRows = Value.Is_Object() ? Value.Find("rows") : nullptr;
				if (LEVEL::END == Set.eLevel || nullptr == pRows || !pRows->Is_Array())
					continue;
				if (nullptr != pCurrent && pCurrent->Is_Number())
					Set.iCurrentZoneId = static_cast<int32_t>(pCurrent->Get_Number());
				(void)Read_String(Value, "continentName", Set.strContinentName);
				for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
				{
					const DATA_JSON_VALUE* pZoneId = Row.Is_Object() ? Row.Find("zoneId") : nullptr;
					ZONE_ROW Zone;
					if (nullptr == pZoneId || !pZoneId->Is_Number() || !Read_String(Row, "name", Zone.strName))
						continue;
					Zone.iZoneId = static_cast<int32_t>(pZoneId->Get_Number());
					Zone.bPort = Read_Bool(Row, "isPort");
					Set.Rows.push_back(std::move(Zone));
				}
				m_ZoneSets.push_back(std::move(Set));
			}
		}
	}
}

const Client::CWorldMapWindowView::AREA* Client::CWorldMapWindowView::Find_Area(const LEVEL eLevel) const
{
	for (const AREA& Area : m_Areas)
		if (Area.eLevel == eLevel)
			return &Area;
	return nullptr;
}

const Client::CWorldMapWindowView::LABEL_SET* Client::CWorldMapWindowView::Find_Labels(const LEVEL eLevel) const
{
	for (const LABEL_SET& Set : m_LabelSets)
		if (Set.eLevel == eLevel)
			return &Set;
	return nullptr;
}

const Client::CWorldMapWindowView::HOLE_SET* Client::CWorldMapWindowView::Find_Holes(const LEVEL eLevel) const
{
	for (const HOLE_SET& Set : m_HoleSets)
		if (Set.eLevel == eLevel)
			return &Set;
	return nullptr;
}

const Client::CWorldMapWindowView::ZONE_SET* Client::CWorldMapWindowView::Find_Zones(const LEVEL eLevel) const
{
	for (const ZONE_SET& Set : m_ZoneSets)
		if (Set.eLevel == eLevel)
			return &Set;
	return nullptr;
}

const Client::CWorldMapWindowView::SQUARE_HOLE* Client::CWorldMapWindowView::Find_Hole(
	const LEVEL eLevel, const uint16_t iHoleId) const
{
	const HOLE_SET* pSet = Find_Holes(eLevel);
	if (nullptr == pSet)
		return nullptr;
	for (const SQUARE_HOLE& Hole : pSet->Holes)
		if (Hole.iId == iHoleId)
			return &Hole;
	return nullptr;
}

bool_t Client::CWorldMapWindowView::Is_LegendChecked(const char* pKey) const
{
	for (const LEGEND_ROW& Row : m_LegendRows)
		if (Row.strKey == pKey)
			return Row.bChecked;
	return true;	/* a category without a legend row is always shown */
}

void Client::CWorldMapWindowView::Build_PanelRows(const LEVEL eLevel, std::vector<PANEL_ROW>& outRows) const
{
	outRows.clear();
	if (PANEL::LEGEND == m_ePanel)
	{
		for (size_t i = 0; i < m_LegendRows.size(); ++i)
		{
			PANEL_ROW Row;
			Row.strName = m_LegendRows[i].strName;
			Row.fTextX = ROW_TEXT_X_LEGEND;
			Row.bCheck = true;
			Row.bChecked = m_LegendRows[i].bChecked;
			Row.strIcon = m_LegendRows[i].strIcon;
			Row.iLegendIndex = static_cast<int32_t>(i);
			outRows.push_back(std::move(Row));
		}
		return;
	}
	if (PANEL::SQUARE_HOLE != m_ePanel)
		return;
	const ZONE_SET* pZones = Find_Zones(eLevel);
	if (nullptr == pZones)
		return;
	const HOLE_SET* pHoles = Find_Holes(eLevel);
	for (const ZONE_ROW& Zone : pZones->Rows)
	{
		const bool_t bCurrent = Zone.iZoneId == pZones->iCurrentZoneId;
		PANEL_ROW Row;
		Row.strName = Zone.strName;
		Row.fTextX = ROW_TEXT_X_ZONE;
		Row.bArrow = true;
		Row.bArrowUp = bCurrent;
		Row.bAnchor = Zone.bPort;
		Row.bHighlight = bCurrent;
		outRows.push_back(std::move(Row));
		/* The current zone is expanded to its square holes (the only zone with hole data). */
		if (bCurrent && nullptr != pHoles)
		{
			for (const SQUARE_HOLE& Hole : pHoles->Holes)
			{
				PANEL_ROW Child;
				Child.strName = Hole.strName;
				Child.vColor = COLOR_GOLD;
				Child.fTextX = ROW_TEXT_X_HOLE;
				Child.iHoleId = Hole.iId;
				outRows.push_back(std::move(Child));
			}
		}
	}
}

void Client::CWorldMapWindowView::Toggle()
{
	m_bOpen = !m_bOpen;
	m_iDialogHoleId = 0u;
	if (m_bOpen)
	{
		m_iZoomLevel = ZOOM_DEFAULT;
		m_bRecenterOnUpdate = true;
	}
	else
	{
		Hide();
	}
}

void Client::CWorldMapWindowView::Hide()
{
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, false);
	m_Texts.clear();
	m_bPanning = false;
	m_Drag.Reset();
}

bool_t Client::CWorldMapWindowView::Take_SquareHoleRequest(uint16_t& outHoleId)
{
	if (0u == m_iPendingHoleId)
		return false;
	outHoleId = m_iPendingHoleId;
	m_iPendingHoleId = 0u;
	return true;
}

void Client::CWorldMapWindowView::Center_On(const f32_t fClientX, const f32_t fClientZ)
{
	if (nullptr == m_pActiveArea)
		return;
	/* Retail cm: x = client X * 100, y = -client Z * 100. Image right = +y, image up = +x. */
	m_fCenterU = (-fClientZ * 100.f - m_pActiveArea->fWorldMinY) /
		(m_pActiveArea->fWorldMaxY - m_pActiveArea->fWorldMinY);
	m_fCenterV = (m_pActiveArea->fWorldMaxX - fClientX * 100.f) /
		(m_pActiveArea->fWorldMaxX - m_pActiveArea->fWorldMinX);
}

void Client::CWorldMapWindowView::Clamp_Center()
{
	/* Keep the visible window inside the image while it fits; once the view shows more than
	the whole image on an axis, hold that axis centered. */
	const auto ClampAxis = [](f32_t& fCenter, const f32_t fVisible)
	{
		if (fVisible >= 1.f)
			fCenter = 0.5f;
		else
			fCenter = std::clamp(fCenter, fVisible * 0.5f, 1.f - fVisible * 0.5f);
	};
	ClampAxis(m_fCenterU, m_fScaleU);
	ClampAxis(m_fCenterV, m_fScaleV);
}

void Client::CWorldMapWindowView::Register_TopWindow() const
{
	/* While open this is the topmost runtime UI (it opens over the HUD and the info window), so
	the other windows' text passes skip its rect -- same rule as the honor title window. The
	title band sits above the frame, so the union of both is registered. */
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	/* WM_WindowBg is the window itself -- title band, map frame and the bottom bar row. */
	if (!m_pView->Get_SlotRect("WM_WindowBg", fX, fY, fWidth, fHeight))
	{
		if (!m_pView->Get_SlotRect("WM_Frame", fX, fY, fWidth, fHeight))
			return;
		f32_t fHX = 0.f, fHY = 0.f, fHW = 0.f, fHH = 0.f;
		if (m_pView->Get_SlotRect("WM_HeaderBg", fHX, fHY, fHW, fHH))
		{
			fHeight += fY - fHY;
			fY = fHY;
		}
	}
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	CUIInputRouter::Get().Set_TopWindowRect(fX * fScaleX, fY * fScaleY, fWidth * fScaleX, fHeight * fScaleY);
}

void Client::CWorldMapWindowView::Update(const f32_t fTimeDelta, const LEVEL eLevel,
	const CClientReplication::MINIMAP_MARKER_SNAPSHOT* pSnapshot)
{
	(void)fTimeDelta;
	m_Texts.clear();
	const AREA* pArea = Find_Area(eLevel);
	if (!m_bOpen || nullptr == pArea || nullptr == pSnapshot || !pSnapshot->hasLocal)
	{
		if (m_bOpen && nullptr == pArea)
			m_bOpen = false;		/* left the last map-capable level */
		Hide();
		return;
	}
	if (pArea != m_pActiveArea)
	{
		m_pActiveArea = pArea;
		m_pView->Set_SlotTexture("WM_Map", pArea->strImage);
		m_bRecenterOnUpdate = true;
	}
	if (m_bRecenterOnUpdate)
	{
		m_bRecenterOnUpdate = false;
		Center_On(pSnapshot->fLocalX, pSnapshot->fLocalZ);
	}
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, true);

	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fFrameX = 0.f, fFrameY = 0.f, fFrameW = 0.f, fFrameH = 0.f;
	(void)m_pView->Get_SlotRect("WM_Frame", fFrameX, fFrameY, fFrameW, fFrameH);
	f32_t fHeaderX = 0.f, fHeaderY = 0.f, fHeaderW = 0.f, fHeaderH = 0.f;
	(void)m_pView->Get_SlotRect("WM_HeaderBg", fHeaderX, fHeaderY, fHeaderW, fHeaderH);
	f32_t fMapX = 0.f, fMapY = 0.f, fMapW = 1.f, fMapH = 1.f;
	(void)m_pView->Get_SlotRect("WM_Map", fMapX, fMapY, fMapW, fMapH);
	const f32_t fUiScale = fMapW / CONTENT_W;		/* layout px per retail canvas px */
	const bool_t bDialog = 0u != m_iDialogHoleId;
	const SQUARE_HOLE* pDialogHole = bDialog ? Find_Hole(eLevel, m_iDialogHoleId) : nullptr;
	if (bDialog && nullptr == pDialogHole)
		m_iDialogHoleId = 0u;

	/* Title band drag first (not while the dialog is up): while a drag is live nothing below
	sees a click. */
	const bool_t bDragging = !bDialog && m_Drag.Update(*m_pView, m_DragSlotIds,
		fHeaderX, fHeaderY, fHeaderW, fHeaderH, "WM_Close");
	if (bDragging)
	{
		(void)m_pView->Get_SlotRect("WM_Frame", fFrameX, fFrameY, fFrameW, fFrameH);
		(void)m_pView->Get_SlotRect("WM_HeaderBg", fHeaderX, fHeaderY, fHeaderW, fHeaderH);
		(void)m_pView->Get_SlotRect("WM_Map", fMapX, fMapY, fMapW, fMapH);
		m_bPanning = false;
	}

	/* Anything over the window (frame or title band) belongs to the window. */
	const bool_t bOverFrame =
		Router.Is_Hovered(fFrameX, fFrameY, fFrameW, fFrameH, REF_WIDTH, REF_HEIGHT) ||
		Router.Is_Hovered(fHeaderX, fHeaderY, fHeaderW, fHeaderH, REF_WIDTH, REF_HEIGHT);
	if (bOverFrame)
		Router.Claim_Mouse_This_Frame();
	Register_TopWindow();

	const auto Hovered = [&](const char* pSlotId) -> bool_t
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		return !bDragging && m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH) &&
			Router.Is_Hovered(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT);
	};
	const auto Clicked = [&](const char* pSlotId) -> bool_t
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		return !bDragging && m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH) &&
			Router.Is_Hovered(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT) &&
			Router.Is_Clicked(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT);
	};
	/* --- buttons (texture swap on hover, click sound on press) --- */
	const auto Button = [&](const char* pSlotId, const string& strNormal, const string& strOver, bool_t bEnabled = true) -> bool_t
	{
		const bool_t bHovered = bEnabled && Hovered(pSlotId);
		if (!strNormal.empty())
			m_pView->Set_SlotTexture(pSlotId, bHovered ? strOver : strNormal);
		return bHovered && Clicked(pSlotId);
	};
	const bool_t bPanelOpen = PANEL::NONE != m_ePanel;
	/* The chrome the pan / wheel must not see through. */
	bool_t bOverChrome = bDialog;
	for (const char* pSlotId : { "WM_BtnWorld", "WM_BtnMyLocation", "WM_BtnPlayerMark", "WM_Close",
		"WM_SearchInput", "WM_SearchBtn", "WM_ToggleSquareHoleBg", "WM_ToggleLegendBg",
		"WM_BtnLiner", "WM_BtnOcean", "WM_BtnMemo" })
	{
		if (Hovered(pSlotId))
			bOverChrome = true;
	}
	if (bPanelOpen && Hovered("WM_PanelTile"))
		bOverChrome = true;

	/* --- square-hole confirm dialog (dialog.gfx DialogWindow strings) --- */
	for (const char* pSlotId : DIALOG_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, bDialog);
	if (bDialog && nullptr != pDialogHole)
	{
		f32_t fDX = 0.f, fDY = 0.f, fDW = 0.f, fDH = 0.f;
		(void)m_pView->Get_SlotRect("WM_DlgBg", fDX, fDY, fDW, fDH);
		m_fDialogX = fDX; m_fDialogY = fDY; m_fDialogW = fDW; m_fDialogH = fDH;
		const bool_t bOk = Button("WM_DlgBtnOk", "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png");
		const bool_t bCancel = Button("WM_DlgBtnCancel", "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png");
		m_pView->Set_SlotTexture("WM_DlgIconOk", Hovered("WM_DlgBtnOk") ?
			"UI/WorldMap/WorldMap_DlgIconOk_Over.png" : "UI/WorldMap/WorldMap_DlgIconOk_Normal.png");
		m_pView->Set_SlotTexture("WM_DlgIconCancel", Hovered("WM_DlgBtnCancel") ?
			"UI/WorldMap/WorldMap_DlgIconCancel_Over.png" : "UI/WorldMap/WorldMap_DlgIconCancel_Normal.png");
		if (bOk)
		{
			CMainApp::Play_UIButtonClickSound();
			m_iPendingHoleId = pDialogHole->iId;
			Close();
			Hide();
			return;
		}
		if (bCancel)
		{
			CMainApp::Play_UIButtonClickSound();
			m_iDialogHoleId = 0u;
		}
		else
		{
			m_Texts.push_back({ FONT_YOON, m_strDialogTitle, fDX + fDW * 0.5f, fDY + DLG_TITLE_CENTER_Y * fUiScale,
				DLG_TITLE_PX * fUiScale, COLOR_DLG_TITLE, 0, {}, { 1.f, 1.f, 1.f, 1.f }, true });
			/* "{0}-ro move there?": the name in yellow, then the particle and the rest in white. */
			wstring_t strRest = m_strConfirmFormat;
			const size_t iToken = strRest.find(L"{0}");
			if (wstring_t::npos != iToken)
			{
				strRest.erase(0, iToken + 3);
				if (!strRest.empty() && L'\xB85C' == strRest.front())	/* the shipped bare -ro */
					strRest.erase(0, 1);
				strRest = Particle_Ro(pDialogHole->strName) + strRest;
			}
			TEXT_ITEM Body{ FONT_YG760, pDialogHole->strName, fDX + fDW * 0.5f, fDY + DLG_BODY_CENTER_Y * fUiScale,
				DLG_BODY_PX * fUiScale, COLOR_YELLOW, 0 };
			Body.strText2 = strRest;
			Body.vColor2 = COLOR_WHITE;
			Body.bDialog = true;
			m_Texts.push_back(std::move(Body));
			if (pDialogHole->iFare > 0)
			{
				m_Texts.push_back({ FONT_YG760, std::to_wstring(pDialogHole->iFare), fDX + fDW * 0.5f + 4.f * fUiScale,
					fDY + DLG_FARE_CENTER_Y * fUiScale, DLG_BODY_PX * fUiScale, COLOR_WHITE, 1,
					{}, { 1.f, 1.f, 1.f, 1.f }, true });
			}
			else
			{
				m_pView->Set_SlotVisible("WM_DlgCoin", false);
			}
			for (const auto& [pSlotId, pText] : { std::pair<const char*, const wstring_t*>{ "WM_DlgBtnOk", &m_strConfirmOk },
				std::pair<const char*, const wstring_t*>{ "WM_DlgBtnCancel", &m_strConfirmCancel } })
			{
				f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
				if (m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
				{
					m_Texts.push_back({ FONT_YG760, *pText, fX + DLG_BTN_TEXT_X * fUiScale, fY + DLG_BTN_TEXT_Y * fUiScale,
						DLG_BODY_PX * fUiScale, COLOR_WHITE, 0, {}, { 1.f, 1.f, 1.f, 1.f }, true });
				}
			}
		}
	}

	if (!bDialog)
	{
		if (Button("WM_Close", "UI/HonorTitle/HonorTitle_Close_Normal.png", "UI/HonorTitle/HonorTitle_Close_Over.png"))
		{
			CMainApp::Play_UIButtonClickSound();
			Close();
			Hide();
			return;
		}
		if (Button("WM_BtnWorld", "UI/WorldMap/WorldMap_BtnWorld_Normal.png", "UI/WorldMap/WorldMap_BtnWorld_Over.png"))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iZoomLevel = ZOOM_FIT;
			m_fCenterU = 0.5f;
			m_fCenterV = 0.5f;
		}
		if (Button("WM_BtnMyLocation", "UI/WorldMap/WorldMap_BtnMyLocation_Normal.png", "UI/WorldMap/WorldMap_BtnMyLocation_Over.png"))
		{
			CMainApp::Play_UIButtonClickSound();
			Center_On(pSnapshot->fLocalX, pSnapshot->fLocalZ);
		}
		if (Button("WM_BtnPlayerMark", "UI/WorldMap/WorldMap_BtnPlayerMark_Normal.png", "UI/WorldMap/WorldMap_BtnPlayerMark_Over.png"))
		{
			CMainApp::Play_UIButtonClickSound();
			m_bShowMarkers = !m_bShowMarkers;
		}
		/* worldMapCheckBox_squareHole / worldMapCheckBox_legend: one tree panel at a time; the
		plate turns gold while selected. */
		const auto PanelToggle = [&](const char* pPlateId, const char* pIconId, const PANEL ePanel, const string& strBase)
		{
			const bool_t bSelected = m_ePanel == ePanel;
			const bool_t bHovered = Hovered(pPlateId) || Hovered(pIconId);
			m_pView->Set_SlotTexture(pIconId,
				strBase + (bSelected ? "_Selected.png" : bHovered ? "_Over.png" : "_Normal.png"));
			/* Retail lights the plate only while the panel is open or the pointer is on it. */
			m_pView->Set_SlotTexture(pPlateId, (bSelected || bHovered)
				? "UI/HonorTitle/HonorTitle_Btn_Over.png" : "UI/HonorTitle/HonorTitle_Btn_Normal.png");
			if (bHovered && (Clicked(pPlateId) || Clicked(pIconId)))
			{
				CMainApp::Play_UIButtonClickSound();
				m_ePanel = bSelected ? PANEL::NONE : ePanel;
			}
		};
		if (Button("WM_SearchBtn", "UI/WorldMap/WorldMap_SearchBtn_Normal.png",
			"UI/WorldMap/WorldMap_SearchBtn_Over.png"))
		{
			CMainApp::Play_UIButtonClickSound();
		}
		PanelToggle("WM_ToggleSquareHoleBg", "WM_ToggleSquareHole", PANEL::SQUARE_HOLE, "UI/WorldMap/WorldMap_ToggleSquareHole");
		PanelToggle("WM_ToggleLegendBg", "WM_ToggleLegend", PANEL::LEGEND, "UI/WorldMap/WorldMap_ToggleLegend");
		/* Bottom bar (voyage liner, set sail, memo): the renew plates hover; no system behind them. */
		for (const char* pSlotId : { "WM_BtnLiner", "WM_BtnOcean", "WM_BtnMemo" })
		{
			if (Button(pSlotId, "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png"))
				CMainApp::Play_UIButtonClickSound();
		}
		m_pView->Set_SlotTint("WM_BtnOcean", TINT_GOLD_PLATE);
	}
	else
	{
		for (const char* pSlotId : { "WM_Close", "WM_BtnWorld", "WM_BtnMyLocation", "WM_BtnPlayerMark",
			"WM_BtnLiner", "WM_BtnOcean", "WM_BtnMemo" })
			(void)pSlotId;
	}

	/* --- zoom (wheel over the window) and pan (left-drag inside the map) --- */
	const bool_t bOverMap = !bDragging && !bOverChrome &&
		Router.Is_Hovered(fMapX, fMapY, fMapW, fMapH, REF_WIDTH, REF_HEIGHT);
	if (bOverFrame && !bDialog && !(bPanelOpen && Hovered("WM_PanelTile")))
	{
		const int32_t iNotches = Router.Get_MouseWheelNotches();
		if (iNotches > 0)
			m_iZoomLevel = (std::min)(ZOOM_COUNT - 1, m_iZoomLevel + 1);
		else if (iNotches < 0)
			m_iZoomLevel = (std::max)(0, m_iZoomLevel - 1);
	}
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHaveMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);
	/* A hovered square hole takes the click; the pan below only starts otherwise. */
	bool_t bOverHole = false;

	/* --- map window: the image aspect follows the world cm box (square tiles), so pixels stay
	square: cm per layout px comes from the view height at the zoom step. --- */
	const f32_t fZoom = ZOOM_STEPS[m_iZoomLevel];
	const f32_t fWorldU = pArea->fWorldMaxY - pArea->fWorldMinY;
	const f32_t fWorldV = pArea->fWorldMaxX - pArea->fWorldMinX;
	const f32_t fCmPerPx = fWorldV / ((std::max)(fMapH, 1.f) * fZoom);
	m_fScaleV = 1.f / fZoom;
	m_fScaleU = fMapW * fCmPerPx / fWorldU;
	/* uv deltas turned between texture space and screen space (world units, so the turn is
	square even though u and v cover different cm spans). +MAP_ROTATION_DEG: screen -> texture
	(what the sprite samples); -MAP_ROTATION_DEG: texture -> screen (markers, labels). */
	const auto TurnDelta = [&](f32_t fDU, f32_t fDV, f32_t fDegrees, f32_t& outDU, f32_t& outDV)
	{
		const f32_t fSin = std::sin(XMConvertToRadians(fDegrees));
		const f32_t fCos = std::cos(XMConvertToRadians(fDegrees));
		const f32_t fWX = fDU * fWorldU, fWY = fDV * fWorldV;
		outDU = (fCos * fWX - fSin * fWY) / fWorldU;
		outDV = (fSin * fWX + fCos * fWY) / fWorldV;
	};
	const f32_t fCenterX = fMapX + fMapW * 0.5f;
	const f32_t fCenterY = fMapY + fMapH * 0.5f;
	const auto ToUV = [&](f32_t fClientX, f32_t fClientZ, f32_t& outU, f32_t& outV)
	{
		outU = (-fClientZ * 100.f - pArea->fWorldMinY) / fWorldU;
		outV = (pArea->fWorldMaxX - fClientX * 100.f) / fWorldV;
	};
	const auto CmToUV = [&](f32_t fWorldX, f32_t fWorldY, f32_t& outU, f32_t& outV)
	{
		outU = (fWorldY - pArea->fWorldMinY) / fWorldU;
		outV = (pArea->fWorldMaxX - fWorldX) / fWorldV;
	};
	/* Uses the current center, so the pan is applied before anything is placed. */
	const auto ToScreen = [&](f32_t fU, f32_t fV, f32_t& outX, f32_t& outY) -> bool_t
	{
		f32_t fDU = 0.f, fDV = 0.f;
		TurnDelta(fU - m_fCenterU, fV - m_fCenterV, -MAP_ROTATION_DEG, fDU, fDV);
		outX = fCenterX + fDU / m_fScaleU * fMapW;
		outY = fCenterY + fDV / m_fScaleV * fMapH;
		return outX >= fMapX && outX <= fMapX + fMapW && outY >= fMapY && outY <= fMapY + fMapH;
	};
	/* Points under the open tree panel are covered by it. */
	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	const bool_t bHavePanelRect = m_pView->Get_SlotRect("WM_PanelTile", fPanelX, fPanelY, fPanelW, fPanelH);
	const auto UnderPanel = [&](f32_t fX, f32_t fY) -> bool_t
	{
		return bPanelOpen && bHavePanelRect &&
			fX >= fPanelX && fX <= fPanelX + fPanelW && fY >= fPanelY && fY <= fPanelY + fPanelH;
	};

	/* Hover test of the square holes against the *previous* pan state is good enough for a
	click; it is re-placed after the pan below. A click opens the confirm dialog. */
	const HOLE_SET* pHoles = Find_Holes(eLevel);
	const bool_t bShowHoles = Is_LegendChecked("background");
	if (nullptr != pHoles && bShowHoles && bHaveMouse && !bDragging && !bOverChrome)
	{
		for (size_t i = 0; i < pHoles->Holes.size() && i < HOLE_SLOT_COUNT; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
			f32_t fU = 0.f, fV = 0.f, fSX = 0.f, fSY = 0.f;
			CmToUV(pHoles->Holes[i].fWorldX, pHoles->Holes[i].fWorldY, fU, fV);
			if (!m_pView->Get_SlotRect(HOLE_SLOTS[i], fX, fY, fW, fH) ||
				!ToScreen(fU, fV, fSX, fSY) || UnderPanel(fSX, fSY))
			{
				continue;
			}
			if (Router.Is_Hovered(fSX - fW * 0.5f, fSY - fH * 0.5f, fW, fH, REF_WIDTH, REF_HEIGHT))
			{
				bOverHole = true;
				if (Router.Is_Clicked(fSX - fW * 0.5f, fSY - fH * 0.5f, fW, fH, REF_WIDTH, REF_HEIGHT))
				{
					CMainApp::Play_UIButtonClickSound();
					m_iDialogHoleId = pHoles->Holes[i].iId;
				}
			}
		}
	}

	if (bOverMap && !bOverHole && bHaveMouse && Router.Is_LeftClickEdge())
	{
		m_bPanning = true;
		m_fLastMouseX = fMouseX;
		m_fLastMouseY = fMouseY;
	}
	if (m_bPanning && (!Router.Is_LeftDown() || !bHaveMouse || bDialog))
		m_bPanning = false;
	if (m_bPanning && bHaveMouse)
	{
		f32_t fDU = 0.f, fDV = 0.f;
		TurnDelta((fMouseX - m_fLastMouseX) / fMapW * m_fScaleU,
			(fMouseY - m_fLastMouseY) / fMapH * m_fScaleV, MAP_ROTATION_DEG, fDU, fDV);
		m_fCenterU -= fDU;
		m_fCenterV -= fDV;
		m_fLastMouseX = fMouseX;
		m_fLastMouseY = fMouseY;
		Router.Claim_Mouse_This_Frame();
	}
	Clamp_Center();
	m_pView->Set_SlotUVWindow("WM_Map",
		m_fCenterU - m_fScaleU * 0.5f, m_fCenterV - m_fScaleV * 0.5f, m_fScaleU, m_fScaleV);
	m_pView->Set_SlotUVRotation("WM_Map", XMConvertToRadians(MAP_ROTATION_DEG), fWorldU / fWorldV);

	/* --- symbols and markers: layout px from the uv delta to the view center. --- */
	const auto PlaceAt = [&](const char* pSlotId, f32_t fU, f32_t fV, bool_t bShow, f32_t& outSX, f32_t& outSY,
		f32_t fAnchorX = 0.5f, f32_t fAnchorY = 0.5f) -> bool_t
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			return false;
		const bool_t bInside = bShow && ToScreen(fU, fV, outSX, outSY) && !UnderPanel(outSX, outSY);
		m_pView->Set_SlotVisible(pSlotId, bInside);
		if (bInside)
			m_pView->Set_SlotPosition(pSlotId, outSX - fW * fAnchorX, outSY - fH * fAnchorY);
		return bInside;
	};
	/* Square holes (retail Minimap_Symbol_73), hover-tinted. */
	for (size_t i = 0; i < HOLE_SLOT_COUNT; ++i)
	{
		f32_t fU = 0.f, fV = 0.f, fSX = 0.f, fSY = 0.f;
		const bool_t bHave = nullptr != pHoles && i < pHoles->Holes.size();
		if (bHave)
			CmToUV(pHoles->Holes[i].fWorldX, pHoles->Holes[i].fWorldY, fU, fV);
		if (PlaceAt(HOLE_SLOTS[i], fU, fV, bHave && bShowHoles, fSX, fSY))
		{
			f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
			(void)m_pView->Get_SlotRect(HOLE_SLOTS[i], fX, fY, fW, fH);
			const bool_t bHovered = !bDragging && !bOverChrome &&
				Router.Is_Hovered(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT);
			m_pView->Set_SlotTint(HOLE_SLOTS[i], bHovered ? TINT_HOVER : TINT_NORMAL);
		}
	}
	/* NPC function symbols: only placements the symbol document knows. */
	{
		const bool_t bShowNpcs = Is_LegendChecked("npc");
		size_t iSlot = 0;
		if (bShowNpcs)
		{
			for (const CClientReplication::MINIMAP_MARKER_SNAPSHOT::NPC_MARKER& Npc : pSnapshot->Npcs)
			{
				if (iSlot >= NPC_SYMBOL_SLOT_COUNT)
					break;
				const auto Iter = m_NpcSymbols.find(Npc.strPlacementId);
				if (m_NpcSymbols.end() == Iter)
					continue;
				f32_t fU = 0.f, fV = 0.f, fSX = 0.f, fSY = 0.f;
				ToUV(Npc.fX, Npc.fZ, fU, fV);
				const char* const pSlotId = Get_NpcSymbolSlots()[iSlot].c_str();
				m_pView->Set_SlotTexture(pSlotId, Iter->second);
				(void)PlaceAt(pSlotId, fU, fV, true, fSX, fSY);
				++iSlot;
			}
		}
		for (; iSlot < NPC_SYMBOL_SLOT_COUNT; ++iSlot)
			m_pView->Set_SlotVisible(Get_NpcSymbolSlots()[iSlot].c_str(), false);
	}
	const auto PlaceMarker = [&](const char* pSlotId, const CClientReplication::MINIMAP_MARKER* pMarker)
	{
		f32_t fU = 0.f, fV = 0.f, fSX = 0.f, fSY = 0.f;
		if (nullptr != pMarker)
			ToUV(pMarker->fX, pMarker->fZ, fU, fV);
		(void)PlaceAt(pSlotId, fU, fV, nullptr != pMarker && m_bShowMarkers, fSX, fSY);
	};
	size_t iParty = 0;
	for (const CClientReplication::MINIMAP_MARKER& Marker : pSnapshot->Players)
	{
		if (!Marker.bParty || iParty >= PARTY_MARKER_COUNT)
			continue;
		PlaceMarker(PARTY_SLOTS[iParty++], &Marker);
	}
	for (; iParty < PARTY_MARKER_COUNT; ++iParty)
		PlaceMarker(PARTY_SLOTS[iParty], nullptr);
	for (size_t i = 0; i < BOSS_MARKER_COUNT; ++i)
		PlaceMarker(BOSS_SLOTS[i], i < pSnapshot->Bosses.size() ? &pSnapshot->Bosses[i] : nullptr);
	{
		/* The player pin (icon_worldmap_pc) stands on the position with its tip; no turn. */
		f32_t fU = 0.f, fV = 0.f, fSX = 0.f, fSY = 0.f;
		ToUV(pSnapshot->fLocalX, pSnapshot->fLocalZ, fU, fV);
		if (PlaceAt("WM_Player", fU, fV, m_bShowMarkers, fSX, fSY, 0.5f, 1.f))
			m_pView->Set_SlotRotation("WM_Player", 0.f);
	}

	/* --- texts: title band, zone name (titleTF, centred), continent name (continentNameTF,
	top-left), search hint, zone-name labels with portal symbols --- */
	if (!m_strWindowTitle.empty())
	{
		m_Texts.push_back({ FONT_YOON, m_strWindowTitle,
			fHeaderX + fHeaderW * 0.5f, fHeaderY + fHeaderH * 0.5f, HEADER_TITLE_PX * fUiScale, COLOR_WHITE, 0 });
	}
	m_Texts.push_back({ FONT_YOON, pArea->strAreaName,
		fMapX + CONTENT_W * 0.5f * fUiScale, fMapY + ZONE_TITLE_CENTER_Y * fUiScale, ZONE_TITLE_PX * fUiScale, COLOR_WHITE, 0 });
	if (const ZONE_SET* pZonesForTitle = Find_Zones(eLevel))
	{
		if (!pZonesForTitle->strContinentName.empty())
		{
			m_Texts.push_back({ FONT_YOON, pZonesForTitle->strContinentName,
				fMapX + CONTINENT_X * fUiScale, fMapY + CONTINENT_CENTER_Y * fUiScale, CONTINENT_PX * fUiScale, COLOR_WHITE, -1 });
		}
	}
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_strSearchHint.empty() && m_pView->Get_SlotRect("WM_SearchInput", fX, fY, fW, fH))
		{
			m_Texts.push_back({ FONT_YG760, m_strSearchHint,
				fX + SEARCH_HINT_INSET_X * fUiScale, fY + fH * 0.5f, SEARCH_HINT_PX * fUiScale, COLOR_HINT, -1 });
		}
	}
	size_t iPortal = 0;
	if (const LABEL_SET* pLabels = Find_Labels(eLevel))
	{
		for (const LABEL& Label : pLabels->Labels)
		{
			f32_t fU = 0.f, fV = 0.f, fSX = 0.f, fSY = 0.f;
			CmToUV(Label.fWorldX, Label.fWorldY, fU, fV);
			if (!ToScreen(fU, fV, fSX, fSY) || UnderPanel(fSX, fSY))
				continue;
			m_Texts.push_back({ FONT_YG760, Label.strName, fSX, fSY, LABEL_PX * fUiScale, Label.vColor, 0 });
			if (Label.bPortal && bShowHoles && iPortal < PORTAL_SLOT_COUNT)
			{
				f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
				if (m_pView->Get_SlotRect(PORTAL_SLOTS[iPortal], fX, fY, fW, fH))
				{
					m_pView->Set_SlotVisible(PORTAL_SLOTS[iPortal], true);
					m_pView->Set_SlotPosition(PORTAL_SLOTS[iPortal],
						fSX + PORTAL_OFFSET_X * fUiScale - fW * 0.5f, fSY + PORTAL_OFFSET_Y * fUiScale - fH * 0.5f);
					++iPortal;
				}
			}
		}
	}
	for (; iPortal < PORTAL_SLOT_COUNT; ++iPortal)
		m_pView->Set_SlotVisible(PORTAL_SLOTS[iPortal], false);
	/* Bottom bar captions, centred on their plate the way retail sets them; the icon sits at the
	plate's left edge and the label keeps the plate's own centre. */
	for (const auto& [pSlotId, pText] : {
		std::pair<const char*, const wstring_t*>{ "WM_BtnLiner", &m_strLinerButton },
		std::pair<const char*, const wstring_t*>{ "WM_BtnOcean", &m_strOceanButton },
		std::pair<const char*, const wstring_t*>{ "WM_BtnMemo", &m_strMemoButton } })
	{
		f32_t fBX = 0.f, fBY = 0.f, fBW = 0.f, fBH = 0.f;
		if (pText->empty() || !m_pView->Get_SlotRect(pSlotId, fBX, fBY, fBW, fBH))
			continue;
		m_Texts.push_back({ FONT_YG760, *pText, fBX + fBW * 0.5f, fBY + fBH * 0.5f,
			BOTTOM_BTN_PX * fUiScale, COLOR_WHITE, 0 });
	}

	/* --- tree panel (square holes = the continent's zones with the current one expanded,
	legend = symbol categories) --- */
	{
		for (const char* pSlotId : { "WM_PanelTile", "WM_PanelHeader" })
			m_pView->Set_SlotVisible(pSlotId, bPanelOpen);
		std::vector<PANEL_ROW> Rows;
		Build_PanelRows(eLevel, Rows);
		for (size_t i = 0; i < PANEL_ROW_COUNT; ++i)
		{
			const string strIndex = std::to_string(i);
			const string strOver = "WM_RowOver_" + strIndex;
			const string strCheck = "WM_RowCheck_" + strIndex;
			const string strIcon = "WM_RowIcon_" + strIndex;
			const string strAnchor = "WM_RowAnchor_" + strIndex;
			const string strArrow = "WM_RowArrow_" + strIndex;
			const bool_t bRow = bPanelOpen && i < Rows.size();
			f32_t fRX = 0.f, fRY = 0.f, fRW = 0.f, fRH = 0.f;
			const bool_t bHaveRow = m_pView->Get_SlotRect(strOver, fRX, fRY, fRW, fRH);
			if (!bRow || !bHaveRow)
			{
				for (const string& strId : { strOver, strCheck, strIcon, strAnchor, strArrow })
					m_pView->Set_SlotVisible(strId, false);
				continue;
			}
			PANEL_ROW& Row = Rows[i];
			const bool_t bHovered = !bDragging && !bDialog &&
				Router.Is_Hovered(fRX, fRY, fRW, fRH, REF_WIDTH, REF_HEIGHT);
			const bool_t bClicked = bHovered && Router.Is_Clicked(fRX, fRY, fRW, fRH, REF_WIDTH, REF_HEIGHT);
			m_pView->Set_SlotVisible(strOver, Row.bHighlight || (bHovered && (Row.iLegendIndex >= 0 || 0u != Row.iHoleId)));
			m_pView->Set_SlotVisible(strCheck, Row.bCheck);
			if (Row.bCheck)
			{
				m_pView->Set_SlotTexture(strCheck,
					Row.bChecked ? "UI/WorldMap/WorldMap_CheckOn.png" : "UI/WorldMap/WorldMap_CheckOff.png");
			}
			m_pView->Set_SlotVisible(strIcon, !Row.strIcon.empty());
			if (!Row.strIcon.empty())
				m_pView->Set_SlotTexture(strIcon, Row.strIcon);
			m_pView->Set_SlotVisible(strAnchor, Row.bAnchor);
			m_pView->Set_SlotVisible(strArrow, Row.bArrow);
			if (Row.bArrow)
			{
				m_pView->Set_SlotTexture(strArrow,
					Row.bArrowUp ? "UI/WorldMap/WorldMap_TreeArrow_Up.png" : "UI/WorldMap/WorldMap_TreeArrow_Down.png");
			}
			m_Texts.push_back({ FONT_YG760, Row.strName, fRX + Row.fTextX * fUiScale,
				fRY + fRH * 0.5f, ROW_TEXT_PX * fUiScale, Row.vColor, -1 });
			if (bClicked && Row.iLegendIndex >= 0 && static_cast<size_t>(Row.iLegendIndex) < m_LegendRows.size())
			{
				CMainApp::Play_UIButtonClickSound();
				m_LegendRows[Row.iLegendIndex].bChecked = !m_LegendRows[Row.iLegendIndex].bChecked;
			}
			else if (bClicked && 0u != Row.iHoleId)
			{
				CMainApp::Play_UIButtonClickSound();
				m_iDialogHoleId = Row.iHoleId;
			}
		}
	}
}

void Client::CWorldMapWindowView::Handle_EscapeEdge()
{
	/* The hole dialog first, then the window. */
	if (0u != m_iDialogHoleId)
	{
		m_iDialogHoleId = 0u;
		return;
	}
	Close();
	Hide();
}

void Client::CWorldMapWindowView::Render_Text()
{
	if (!m_bOpen || m_Texts.empty())
		return;
	CGameInstance& gameInstance = CGameInstance::Get();
	const float2_t vViewport = gameInstance.Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	const auto Draw = [&](const wstring_t& strFont, const wstring_t& strText, const float2_t& vPosition,
		const float4_t& vColor, f32_t fScale)
	{
		gameInstance.Draw_Text(strFont, strText.c_str(),
			float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 0.8f), 0.f, float2_t(0.f, 0.f), fScale);
		gameInstance.Draw_Text(strFont, strText.c_str(), vPosition,
			XMLoadFloat4(&vColor), 0.f, float2_t(0.f, 0.f), fScale);
	};
	/* The dialog is modal, so anything of the map that would land on its plate is dropped: the
	text pass runs after every sprite, so a zone name under the dialog would otherwise print on
	top of it. */
	const bool_t bDialog = 0u != m_iDialogHoleId && m_fDialogW > 0.f;
	const f32_t fDlgLeft = m_fDialogX * fScaleX;
	const f32_t fDlgTop = m_fDialogY * fScaleY;
	const f32_t fDlgRight = (m_fDialogX + m_fDialogW) * fScaleX;
	const f32_t fDlgBottom = (m_fDialogY + m_fDialogH) * fScaleY;
	for (const TEXT_ITEM& Item : m_Texts)
	{
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(Item.strFont, Item.fRefPx * fUiScale * TEXT_BOOST, fScale);
		const float2_t vMeasured = gameInstance.Measure_Text(strFont, Item.strText.c_str());
		f32_t fTotalWidth = vMeasured.x * fScale;
		float2_t vMeasured2(0.f, 0.f);
		if (!Item.strText2.empty())
		{
			vMeasured2 = gameInstance.Measure_Text(strFont, Item.strText2.c_str());
			fTotalWidth += vMeasured2.x * fScale;
		}
		const f32_t fLeft = Item.iAlign < 0 ? Item.fRefX * fScaleX :
			Item.iAlign > 0 ? Item.fRefX * fScaleX - fTotalWidth :
			Item.fRefX * fScaleX - fTotalWidth * 0.5f;
		const float2_t vPosition(
			std::round(fLeft),
			std::round(Item.fRefY * fScaleY - vMeasured.y * fScale * 0.5f));
		if (bDialog && !Item.bDialog)
		{
			const f32_t fTextRight = vPosition.x + fTotalWidth;
			const f32_t fTextBottom = vPosition.y + vMeasured.y * fScale;
			if (vPosition.x < fDlgRight && fTextRight > fDlgLeft &&
				vPosition.y < fDlgBottom && fTextBottom > fDlgTop)
			{
				continue;
			}
		}
		Draw(strFont, Item.strText, vPosition, Item.vColor, fScale);
		if (!Item.strText2.empty())
		{
			Draw(strFont, Item.strText2,
				float2_t(std::round(fLeft + vMeasured.x * fScale), vPosition.y), Item.vColor2, fScale);
		}
	}
}

bool_t Client::CWorldMapWindowView::Get_ScreenRect(
	f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	if (!Is_Open() || nullptr == m_pView)
		return false;
	f32_t fRefX = 0.f, fRefY = 0.f, fRefWidth = 0.f, fRefHeight = 0.f;
	if (!m_pView->Get_SlotRect("WM_Frame", fRefX, fRefY, fRefWidth, fRefHeight))
		return false;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	fX = fRefX * fScaleX;
	fY = fRefY * fScaleY;
	fWidth = fRefWidth * fScaleX;
	fHeight = fRefHeight * fScaleY;
	return true;
}
