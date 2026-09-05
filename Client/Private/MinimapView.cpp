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
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	const char* const PARTY_SLOTS[PARTY_MARKER_COUNT] =
		{ "Minimap_Party_0", "Minimap_Party_1", "Minimap_Party_2", "Minimap_Party_3" };
	const char* const BOSS_SLOTS[BOSS_MARKER_COUNT] = { "Minimap_Boss_0", "Minimap_Boss_1" };

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
		const auto Number = [](const DATA_JSON_VALUE& V, size_t i)
		{
			const DATA_JSON_VALUE& E = V.Get_Array()[i];
			return E.Is_Number() ? static_cast<f32_t>(E.Get_Number()) : 0.f;
		};
		Area.fWorldMinX = Number(*pMin, 0); Area.fWorldMinY = Number(*pMin, 1);
		Area.fWorldMaxX = Number(*pMax, 0); Area.fWorldMaxY = Number(*pMax, 1);
		if (Area.fWorldMaxX - Area.fWorldMinX <= 1.f || Area.fWorldMaxY - Area.fWorldMinY <= 1.f)
			continue;
		m_Areas.push_back(std::move(Area));
	}
	return m_Areas.empty() ? E_FAIL : S_OK;
}

const Client::CMinimapView::AREA* Client::CMinimapView::Find_Area(LEVEL eLevel) const
{
	for (const AREA& Area : m_Areas)
		if (Area.eLevel == eLevel)
			return &Area;
	return nullptr;
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
	const AREA* pArea = Find_Area(eLevel);
	if (nullptr == pArea || nullptr == pSnapshot || !pSnapshot->hasLocal)
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
	m_pView->Set_SlotVisible("Minimap_Map", m_bMapVisible);

	/* --- markers: reference-resolution screen position from the UV delta to the local
	character; anything outside the map rect hides. --- */
	const f32_t fCenterX = fViewX + fViewW * 0.5f;
	const f32_t fCenterY = fViewY + fViewH * 0.5f;
	const auto PlaceMarker = [&](const char* pSlotId,
		const CClientReplication::MINIMAP_MARKER* pMarker)
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
		const f32_t fDX = (fU - fLocalU) / fScaleU * fViewW;
		const f32_t fDY = (fV - fLocalV) / fScaleV * fViewH;
		const bool_t bInside = std::fabs(fDX) <= fViewW * 0.5f - fW * 0.35f &&
			std::fabs(fDY) <= fViewH * 0.5f - fH * 0.35f;
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

	/* Local arrow: centered, rotated to the character's facing. Character yaw is measured from
	+Z (yaw 0 = +Z, 90 = +X); on this map +X is screen up and +Z is screen left, so the
	clockwise screen rotation is yaw - 90. */
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (m_pView->Get_SlotRect("Minimap_Player", fX, fY, fW, fH))
		{
			m_pView->Set_SlotPosition("Minimap_Player", fCenterX - fW * 0.5f, fCenterY - fH * 0.5f);
			m_pView->Set_SlotRotation("Minimap_Player", pSnapshot->fLocalYawDegrees - 90.f);
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
