#include "MinimapView.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <fstream>

namespace
{
	/* Visible map width in world meters per zoom step (retail's zoom slider range, coarsened
	to five stops). Index 2 is the default. */
	constexpr f32_t ZOOM_VIEW_WIDTH_METERS[] = { 45.f, 70.f, 100.f, 145.f, 210.f };
	constexpr int32_t ZOOM_LEVEL_COUNT =
		static_cast<int32_t>(sizeof(ZOOM_VIEW_WIDTH_METERS) / sizeof(ZOOM_VIEW_WIDTH_METERS[0]));
	constexpr int32_t ZOOM_LEVEL_DEFAULT = 2;
	constexpr size_t PARTY_MARKER_COUNT = 4;
	constexpr size_t BOSS_MARKER_COUNT = 2;
	/* Retail draws every map picture with the camera's forward direction up. All four product
	cameras (Data/Camera/*.camera.json) yaw 135 deg = client (+X, -Z), the image's up-right
	diagonal, so the picture turns 45 deg counter-clockwise on screen (verified against a retail
	world map capture: -43 deg, scale 1.40 = WorldmapScaleFactor). Positive = the clockwise
	turn CUI_Sprite::Set_UVRotation samples with; screen offsets use the inverse. */
	constexpr f32_t MAP_ROTATION_DEG = 45.f;
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	const char* const PARTY_SLOTS[PARTY_MARKER_COUNT] =
		{ "Minimap_Party_0", "Minimap_Party_1", "Minimap_Party_2", "Minimap_Party_3" };
	const char* const BOSS_SLOTS[BOSS_MARKER_COUNT] = { "Minimap_Boss_0", "Minimap_Boss_1" };
	/* Must match the Minimap_Npc_* slot count in Data/UI/Minimap/Minimap_Layout.json. */
	constexpr size_t NPC_MARKER_COUNT = 40;

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
}

Client::CMinimapView::CMinimapView(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, uint32_t iGameObjectLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iGameObjectLevelIndex,
		TEXT("Layer_UI"), L"UI/Minimap/Minimap_Layout.json");
	m_pView->Set_AllSlotsVisible(false);
	if (FAILED(Load_Areas()))
		OutputDebugStringA("[Minimap] MinimapAreas.json missing or invalid -- minimap stays hidden.\n");
}

Client::CMinimapView::~CMinimapView() = default;

HRESULT Client::CMinimapView::Load_Areas()
{
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"UI/Minimap/MinimapAreas.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
		return E_FAIL;
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
		return E_FAIL;
	const DATA_JSON_VALUE* pAreas = Root.Find("areas");
	if (nullptr == pAreas || !pAreas->Is_Array())
		return E_FAIL;

	for (const DATA_JSON_VALUE& Value : pAreas->Get_Array())
	{
		if (!Value.Is_Object())
			continue;
		AREA Area{};
		const DATA_JSON_VALUE* pLevel = Value.Find("level");
		const DATA_JSON_VALUE* pName = Value.Find("areaName");
		const DATA_JSON_VALUE* pImage = Value.Find("image");
		const DATA_JSON_VALUE* pMin = Value.Find("worldMinCm");
		const DATA_JSON_VALUE* pMax = Value.Find("worldMaxCm");
		if (nullptr == pLevel || !pLevel->Is_String() ||
			nullptr == pImage || !pImage->Is_String() ||
			nullptr == pMin || !pMin->Is_Array() || pMin->Get_Array().size() < 2 ||
			nullptr == pMax || !pMax->Is_Array() || pMax->Get_Array().size() < 2)
			continue;
		Area.eLevel = Parse_Level(pLevel->Get_String());
		if (LEVEL::END == Area.eLevel)
			continue;
		Area.strImage = pImage->Get_String();
		if (nullptr != pName && pName->Is_String())
			(void)Convert_Utf8(pName->Get_String(), Area.strAreaName);
		const DATA_JSON_VALUE* pWorldAreaId = Value.Find("worldAreaId");
		if (nullptr != pWorldAreaId && pWorldAreaId->Is_String())
			Area.strWorldAreaId = pWorldAreaId->Get_String();
		const DATA_JSON_VALUE* pDefault = Value.Find("default");
		Area.bDefault = nullptr != pDefault && pDefault->Is_Boolean() && pDefault->Get_Boolean();
		const auto Number = [](const DATA_JSON_VALUE& V, size_t i)
		{
			const DATA_JSON_VALUE& E = V.Get_Array()[i];
			return E.Is_Number() ? static_cast<f32_t>(E.Get_Number()) : 0.f;
		};
		Area.fWorldMinX = Number(*pMin, 0); Area.fWorldMinY = Number(*pMin, 1);
		Area.fWorldMaxX = Number(*pMax, 0); Area.fWorldMaxY = Number(*pMax, 1);
		const DATA_JSON_VALUE* pSelMin = Value.Find("selectMinCm");
		const DATA_JSON_VALUE* pSelMax = Value.Find("selectMaxCm");
		Area.fSelectMinX = Area.fWorldMinX; Area.fSelectMinY = Area.fWorldMinY;
		Area.fSelectMaxX = Area.fWorldMaxX; Area.fSelectMaxY = Area.fWorldMaxY;
		if (nullptr != pSelMin && pSelMin->Is_Array() && pSelMin->Get_Array().size() >= 2 &&
			nullptr != pSelMax && pSelMax->Is_Array() && pSelMax->Get_Array().size() >= 2)
		{
			const auto& SMin = pSelMin->Get_Array();
			const auto& SMax = pSelMax->Get_Array();
			if (SMin[0].Is_Number() && SMin[1].Is_Number() && SMax[0].Is_Number() && SMax[1].Is_Number() &&
				SMax[0].Get_Number() > SMin[0].Get_Number() && SMax[1].Get_Number() > SMin[1].Get_Number())
			{
				Area.fSelectMinX = static_cast<f32_t>(SMin[0].Get_Number());
				Area.fSelectMinY = static_cast<f32_t>(SMin[1].Get_Number());
				Area.fSelectMaxX = static_cast<f32_t>(SMax[0].Get_Number());
				Area.fSelectMaxY = static_cast<f32_t>(SMax[1].Get_Number());
			}
		}
		const DATA_JSON_VALUE* pExtraBoxes = Value.Find("extraSelectBoxesCm");
		if (nullptr != pExtraBoxes && pExtraBoxes->Is_Array())
		{
			for (const DATA_JSON_VALUE& Box : pExtraBoxes->Get_Array())
			{
				if (!Box.Is_Array() || Box.Get_Array().size() < 4)
					continue;
				AREA::SELECT_BOX Extra{};
				Extra.fMinX = Number(Box, 0); Extra.fMinY = Number(Box, 1);
				Extra.fMaxX = Number(Box, 2); Extra.fMaxY = Number(Box, 3);
				if (Extra.fMaxX > Extra.fMinX && Extra.fMaxY > Extra.fMinY)
					Area.ExtraSelect.push_back(Extra);
			}
		}
		const DATA_JSON_VALUE* pLandmarks = Value.Find("landmarks");
		if (nullptr != pLandmarks && pLandmarks->Is_Array())
		{
			for (const DATA_JSON_VALUE& Landmark : pLandmarks->Get_Array())
			{
				const DATA_JSON_VALUE* pLandmarkId = Landmark.Is_Object() ? Landmark.Find("placementId") : nullptr;
				const DATA_JSON_VALUE* pLandmarkIcon = Landmark.Is_Object() ? Landmark.Find("icon") : nullptr;
				if (nullptr == pLandmarkId || !pLandmarkId->Is_String() ||
					nullptr == pLandmarkIcon || !pLandmarkIcon->Is_String())
				{
					continue;
				}
				Area.Landmarks.push_back(AREA::LANDMARK{ pLandmarkId->Get_String(), pLandmarkIcon->Get_String() });
			}
		}
		if (Area.fWorldMaxX - Area.fWorldMinX <= 1.f || Area.fWorldMaxY - Area.fWorldMinY <= 1.f)
			continue;
		m_Areas.push_back(std::move(Area));
	}
	return m_Areas.empty() ? E_FAIL : S_OK;
}

void Client::CMinimapView::Load_AreaNpcSymbols(const AREA& Area)
{
	/* An area with landmarks is keyed by its own image, because its landmark set differs from the other
	areas that share the same world document; every other area stays keyed by the world area as before. */
	const string strKey = Area.Landmarks.empty() ? Area.strWorldAreaId : Area.strWorldAreaId + "|" + Area.strImage;
	if (m_strLoadedNpcAreaId == strKey)
		return;
	m_strLoadedNpcAreaId = strKey;
	m_NpcSymbols.clear();
	if (Area.strWorldAreaId.empty())
		return;

	/* placementId -> symbol image, the document CWorldMapWindowView draws the M map from. An
	NPC without an entry has no service marker in retail either, so it is simply skipped. */
	const auto ReadJson = [](const filesystem::path& Path, DATA_JSON_VALUE& outRoot)
	{
		ifstream Stream(Path, ios::binary);
		if (!Stream.is_open())
			return false;
		const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
		string Error;
		return CDataJson::Parse(Text, outRoot, Error) && outRoot.Is_Object();
	};

	DATA_JSON_VALUE WorldRoot;
	const filesystem::path WorldPath = CProjectDataRoot::Resolve(
		filesystem::path("Worlds") / Area.strWorldAreaId / "Gameplay.world.json");
	if (!ReadJson(WorldPath, WorldRoot))
		return;
	const DATA_JSON_VALUE* pWorldPlacements = WorldRoot.Find("placements");
	if (nullptr == pWorldPlacements || !pWorldPlacements->Is_Array())
		return;

	/* Landmarks first, so they always own a marker slot: the position comes from the placement itself
	(the same Gameplay.world.json the Server publishes), never from a copy typed into this view. */
	for (const AREA::LANDMARK& Landmark : Area.Landmarks)
	{
		for (const DATA_JSON_VALUE& Placement : pWorldPlacements->Get_Array())
		{
			if (!Placement.Is_Object())
				continue;
			const DATA_JSON_VALUE* pId = Placement.Find("placementId");
			const DATA_JSON_VALUE* pPosition = Placement.Find("position");
			if (nullptr == pId || !pId->Is_String() || Landmark.strPlacementId != pId->Get_String() ||
				nullptr == pPosition || !pPosition->Is_Array() || pPosition->Get_Array().size() < 3)
			{
				continue;
			}
			const DATA_JSON_VALUE& X = pPosition->Get_Array()[0];
			const DATA_JSON_VALUE& Z = pPosition->Get_Array()[2];
			if (!X.Is_Number() || !Z.Is_Number())
				break;
			NPC_SYMBOL Symbol{};
			Symbol.strIconPath = Landmark.strIcon;
			Symbol.fWorldX = static_cast<f32_t>(X.Get_Number());
			Symbol.fWorldZ = static_cast<f32_t>(Z.Get_Number());
			Symbol.bEdgeClamp = true;
			m_NpcSymbols.push_back(std::move(Symbol));
			break;
		}
	}

	DATA_JSON_VALUE SymbolRoot;
	if (!ReadJson(CProjectDataRoot::Resolve(L"UI/WorldMap/WorldMapNpcSymbols.json"), SymbolRoot))
		return;
	const DATA_JSON_VALUE* pPlacements = SymbolRoot.Find("placements");
	if (nullptr == pPlacements || !pPlacements->Is_Object())
		return;

	for (const DATA_JSON_VALUE& Placement : pWorldPlacements->Get_Array())
	{
		if (m_NpcSymbols.size() >= NPC_MARKER_COUNT)
			break;
		if (!Placement.Is_Object())
			continue;
		const DATA_JSON_VALUE* pKind = Placement.Find("kind");
		const DATA_JSON_VALUE* pId = Placement.Find("placementId");
		const DATA_JSON_VALUE* pPosition = Placement.Find("position");
		if (nullptr == pKind || !pKind->Is_String() || "npc" != pKind->Get_String() ||
			nullptr == pId || !pId->Is_String() ||
			nullptr == pPosition || !pPosition->Is_Array() ||
			pPosition->Get_Array().size() < 3)
		{
			continue;
		}
		const DATA_JSON_VALUE* pIcon = pPlacements->Find(pId->Get_String());
		if (nullptr == pIcon || !pIcon->Is_String() || pIcon->Get_String().empty())
			continue;
		NPC_SYMBOL Symbol{};
		Symbol.strIconPath = pIcon->Get_String();
		const DATA_JSON_VALUE& X = pPosition->Get_Array()[0];
		const DATA_JSON_VALUE& Z = pPosition->Get_Array()[2];
		Symbol.fWorldX = X.Is_Number() ? static_cast<f32_t>(X.Get_Number()) : 0.f;
		Symbol.fWorldZ = Z.Is_Number() ? static_cast<f32_t>(Z.Get_Number()) : 0.f;
		m_NpcSymbols.push_back(std::move(Symbol));
	}
}

const Client::CMinimapView::AREA* Client::CMinimapView::Find_Area(
	LEVEL eLevel, const f32_t fClientX, const f32_t fClientZ) const
{
	/* Retail cm: x = client X * 100, y = -client Z * 100. */
	const f32_t fWorldX = fClientX * 100.f;
	const f32_t fWorldY = -fClientZ * 100.f;
	const AREA* pFirst = nullptr;
	const AREA* pDefault = nullptr;
	for (const AREA& Area : m_Areas)
	{
		if (Area.eLevel != eLevel)
			continue;
		if (nullptr == pFirst)
			pFirst = &Area;
		if (nullptr == pDefault && Area.bDefault)
			pDefault = &Area;
		if (fWorldX >= Area.fSelectMinX && fWorldX <= Area.fSelectMaxX &&
			fWorldY >= Area.fSelectMinY && fWorldY <= Area.fSelectMaxY)
		{
			return &Area;
		}
		for (const AREA::SELECT_BOX& Box : Area.ExtraSelect)
		{
			if (fWorldX >= Box.fMinX && fWorldX <= Box.fMaxX &&
				fWorldY >= Box.fMinY && fWorldY <= Box.fMaxY)
			{
				return &Area;
			}
		}
	}
	return nullptr != pDefault ? pDefault : pFirst;
}

void Client::CMinimapView::Hide_All()
{
	if (m_bAnySlotVisible)
	{
		m_pView->Set_AllSlotsVisible(false);
		m_bAnySlotVisible = false;
	}
	m_pActiveArea = nullptr;
}

void Client::CMinimapView::Update(const f32_t fTimeDelta, LEVEL eLevel,
	const CClientReplication::MINIMAP_MARKER_SNAPSHOT* pSnapshot)
{
	(void)fTimeDelta;
	const AREA* pArea = nullptr != pSnapshot && pSnapshot->hasLocal ?
		Find_Area(eLevel, pSnapshot->fLocalX, pSnapshot->fLocalZ) : nullptr;
	if (nullptr == pArea)
	{
		Hide_All();
		return;
	}
	if (pArea != m_pActiveArea)
	{
		m_pActiveArea = pArea;
		m_pView->Set_SlotTexture("Minimap_Map", pArea->strImage);
		m_iZoomLevel = ZOOM_LEVEL_DEFAULT;
		m_bSliderOpen = false;
	}
	if (!m_bAnySlotVisible)
	{
		m_pView->Set_AllSlotsVisible(true);
		m_bAnySlotVisible = true;
	}

	/* --- buttons (retail: zoom icon toggles the slider group; +/- step the zoom; reset
	returns to the default; the header toggle collapses the map) --- */
	CUIInputRouter& Router = CUIInputRouter::Get();
	const auto Hover = [&](const char* pSlotId, bool_t& outClicked)
	{
		outClicked = false;
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			return false;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT);
		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			outClicked = Router.Is_Clicked(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT);
		}
		return bHovered;
	};
	bool_t bClicked = false;
	Hover("Minimap_ZoomBtn", bClicked);
	if (bClicked)
		m_bSliderOpen = !m_bSliderOpen;
	Hover("Minimap_VisBtn", bClicked);
	if (bClicked)
		m_bMapVisible = !m_bMapVisible;
	Hover("Minimap_ResetBtn", bClicked);
	if (bClicked)
		m_iZoomLevel = ZOOM_LEVEL_DEFAULT;
	if (m_bSliderOpen)
	{
		const bool_t bInHover = Hover("Minimap_ZoomIn", bClicked);
		if (bClicked)
			m_iZoomLevel = (std::max)(0, m_iZoomLevel - 1);
		m_pView->Set_SlotTexture("Minimap_ZoomIn",
			bInHover ? "UI/Minimap/btn_zoomin_down.png" : "");
		const bool_t bOutHover = Hover("Minimap_ZoomOut", bClicked);
		if (bClicked)
			m_iZoomLevel = (std::min)(ZOOM_LEVEL_COUNT - 1, m_iZoomLevel + 1);
		m_pView->Set_SlotTexture("Minimap_ZoomOut",
			bOutHover ? "UI/Minimap/btn_zoomout_down.png" : "");
	}
	/* Decorative retail controls (opacity, troop filter, channel dropdown) only hover. */
	Hover("Minimap_AlphaBtn", bClicked);
	Hover("Minimap_TroopBtn", bClicked);
	Hover("Minimap_ChannelBox", bClicked);
	for (const char* pSlotId : { "Minimap_SliderBg", "Minimap_ZoomIn", "Minimap_ZoomOut" })
		m_pView->Set_SlotVisible(pSlotId, m_bSliderOpen);

	/* --- map window: the local character sits at the view center; the visible width is the
	zoom stop in meters. Retail world cm: x = client X * 100, y = -client Z * 100. The area
	image keeps retail's minimap orientation (verified against the packaged overview image and
	the walkable navigation cells): image right = +y (client -Z), image up = +x (client +X).
	So u spans the y range and v spans the x range, descending. --- */
	f32_t fViewX = 0.f, fViewY = 0.f, fViewW = 1.f, fViewH = 1.f;
	(void)m_pView->Get_SlotRect("Minimap_Map", fViewX, fViewY, fViewW, fViewH);
	const f32_t fWorldU = pArea->fWorldMaxY - pArea->fWorldMinY;
	const f32_t fWorldV = pArea->fWorldMaxX - pArea->fWorldMinX;
	const f32_t fViewWidthCm = ZOOM_VIEW_WIDTH_METERS[m_iZoomLevel] * 100.f;
	const f32_t fScaleU = fViewWidthCm / fWorldU;
	const f32_t fScaleV = fViewWidthCm * (fViewH / (std::max)(fViewW, 1.f)) / fWorldV;
	const auto ToUV = [&](f32_t fClientX, f32_t fClientZ, f32_t& outU, f32_t& outV)
	{
		outU = (-fClientZ * 100.f - pArea->fWorldMinY) / fWorldU;
		outV = (pArea->fWorldMaxX - fClientX * 100.f) / fWorldV;
	};
	f32_t fLocalU = 0.f, fLocalV = 0.f;
	ToUV(pSnapshot->fLocalX, pSnapshot->fLocalZ, fLocalU, fLocalV);
	m_pView->Set_SlotUVWindow("Minimap_Map",
		fLocalU - fScaleU * 0.5f, fLocalV - fScaleV * 0.5f, fScaleU, fScaleV);
	m_pView->Set_SlotUVRotation("Minimap_Map", XMConvertToRadians(MAP_ROTATION_DEG), fWorldU / fWorldV);
	/* Texture-space uv delta -> screen uv delta: world units, turned by -MAP_ROTATION_DEG. */
	const f32_t fRotSin = std::sin(XMConvertToRadians(-MAP_ROTATION_DEG));
	const f32_t fRotCos = std::cos(XMConvertToRadians(-MAP_ROTATION_DEG));
	const auto ToScreenDelta = [&](f32_t fDU, f32_t fDV, f32_t& outDU, f32_t& outDV)
	{
		const f32_t fWX = fDU * fWorldU, fWY = fDV * fWorldV;
		outDU = (fRotCos * fWX - fRotSin * fWY) / fWorldU;
		outDV = (fRotSin * fWX + fRotCos * fWY) / fWorldV;
	};
	m_pView->Set_SlotVisible("Minimap_Map", m_bMapVisible);

	/* --- markers: reference-resolution screen position from the UV delta to the local
	character; anything outside the map rect hides. --- */
	const f32_t fCenterX = fViewX + fViewW * 0.5f;
	const f32_t fCenterY = fViewY + fViewH * 0.5f;
	/* bClampToEdge: a landmark keeps to the window edge in its direction when the place is off the visible
	map, instead of hiding like a party member or NPC does. */
	const auto PlaceMarker = [&](const char* pSlotId,
		const CClientReplication::MINIMAP_MARKER* pMarker, const bool_t bClampToEdge = false)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			return;
		if (nullptr == pMarker || !m_bMapVisible)
		{
			m_pView->Set_SlotVisible(pSlotId, false);
			return;
		}
		f32_t fU = 0.f, fV = 0.f;
		ToUV(pMarker->fX, pMarker->fZ, fU, fV);
		f32_t fDU = 0.f, fDV = 0.f;
		ToScreenDelta(fU - fLocalU, fV - fLocalV, fDU, fDV);
		f32_t fDX = fDU / fScaleU * fViewW;
		f32_t fDY = fDV / fScaleV * fViewH;
		const f32_t fHalfW = fViewW * 0.5f - fW * 0.35f;
		const f32_t fHalfH = fViewH * 0.5f - fH * 0.35f;
		if (bClampToEdge && fHalfW > 0.f && fHalfH > 0.f)
		{
			const f32_t fNorm = std::sqrt((fDX / fHalfW) * (fDX / fHalfW) + (fDY / fHalfH) * (fDY / fHalfH));
			if (fNorm > 1.f)
			{
				fDX /= fNorm;
				fDY /= fNorm;
			}
		}
		const bool_t bInside = std::fabs(fDX) <= fHalfW && std::fabs(fDY) <= fHalfH;
		m_pView->Set_SlotVisible(pSlotId, bInside);
		if (bInside)
			m_pView->Set_SlotPosition(pSlotId, fCenterX + fDX - fW * 0.5f, fCenterY + fDY - fH * 0.5f);
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

	/* Service NPC symbols: the same authored placements and icons the M map shows, so the two
	agree instead of only the full map carrying them. These are static world positions, not
	replicated entities, so they go through the same projection as a marker but are read from
	the area document rather than the snapshot. */
	Load_AreaNpcSymbols(*m_pActiveArea);
	for (size_t i = 0; i < NPC_MARKER_COUNT; ++i)
	{
		const string strSlotId = "Minimap_Npc_" + std::to_string(i);
		if (i >= m_NpcSymbols.size())
		{
			m_pView->Set_SlotVisible(strSlotId, false);
			continue;
		}
		const NPC_SYMBOL& Symbol = m_NpcSymbols[i];
		CClientReplication::MINIMAP_MARKER Marker{};
		Marker.fX = Symbol.fWorldX;
		Marker.fZ = Symbol.fWorldZ;
		m_pView->Set_SlotTexture(strSlotId, Symbol.strIconPath);
		PlaceMarker(strSlotId.c_str(), &Marker, Symbol.bEdgeClamp);
	}

	/* Local arrow: centered, rotated to the character's facing. Character yaw is measured from
	+Z (yaw 0 = +Z, 90 = +X); on the unturned image +X is up and +Z is left (yaw - 90), and the
	picture itself is turned MAP_ROTATION_DEG counter-clockwise. */
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (m_pView->Get_SlotRect("Minimap_Player", fX, fY, fW, fH))
		{
			m_pView->Set_SlotPosition("Minimap_Player", fCenterX - fW * 0.5f, fCenterY - fH * 0.5f);
			m_pView->Set_SlotRotation("Minimap_Player",
				pSnapshot->fLocalYawDegrees - 90.f - MAP_ROTATION_DEG);
			m_pView->Set_SlotVisible("Minimap_Player", m_bMapVisible);
		}
	}
	m_pView->Update(fTimeDelta);
}

void Client::CMinimapView::RenderText()
{
	if (!m_bAnySlotVisible || nullptr == m_pActiveArea || m_pActiveArea->strAreaName.empty())
		return;
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (!m_pView->Get_SlotRect("Minimap_AreaName", fX, fY, fW, fH))
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / REF_WIDTH;
	const f32_t fScaleY = vViewport.y / REF_HEIGHT;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	const wchar_t* pText = m_pActiveArea->strAreaName.c_str();
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), pText);
	if (vMeasured.y <= 0.f)
		return;
	/* Retail header label is $YG760 14px at 1080p (9.3 reference units); font kept, sized up
	for legibility at our resolution. Left-aligned. */
	f32_t fScale = 13.f / vMeasured.y;
	if (vMeasured.x * fScale > fW)
		fScale = fW / vMeasured.x;
	CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), pText,
		float2_t(fX * fScaleX, (fY + fH * 0.5f) * fScaleY),
		XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f, float2_t(0.f, 0.5f), fScale * fUiScale);
}
