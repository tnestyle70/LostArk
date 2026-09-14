#include "CombatAnalysisFrameView.h"

#include <DirectXColors.h>

#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <cmath>
#include <cstdio>
#include <fstream>

namespace
{
	/* combatanalysisframe.gfx is authored at 1920x1080; the layout document scales the frame
	onto the 1280x720 reference, so every retail px offset below goes through m_fRetailScale
	(CA_Bg slot width / FRAME_WIDTH). */
	constexpr f32_t FRAME_WIDTH = 470.f;
	constexpr f32_t TITLE_BAR_HEIGHT = 32.f;
	constexpr f32_t TEXT_BOOST = 1.15f;
	/* timeTick _defaultTF: YG760 14 px at (243+2, 5+2). Tiles: YG760 14 px centred in the
	116x26 tile (statTitleList y 33, userStatList y 60, RollingTileList pitch 470/4). */
	constexpr f32_t TIME_X = 247.f;
	constexpr f32_t TIME_Y = 8.f;
	constexpr f32_t TILE_PITCH = FRAME_WIDTH / 4.f;
	constexpr f32_t TILE_WIDTH = 116.f;
	constexpr f32_t TILE_HEIGHT = 26.f;
	constexpr f32_t TITLE_ROW_Y = 33.f;
	constexpr f32_t VALUE_ROW_Y = 60.f;
	constexpr f32_t TILE_FONT_PX = 14.f;
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr size_t STAT_COUNT = 4;

	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	const fvector_t COLOR_TIME = XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f);   // #FFD200
	const char* ART_CLOSE_NORMAL = "UI/CombatAnalysis/CA_Close_Normal.png";
	const char* ART_CLOSE_OVER = "UI/CombatAnalysis/CA_Close_Over.png";

	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}
}

Client::CCombatAnalysisFrameView::CCombatAnalysisFrameView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/CombatAnalysis/CombatAnalysis_Layout.json") }
{
	m_SlotIds = m_pView->Get_SlotIds();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("CA_Bg", fX, fY, fWidth, fHeight) && fWidth > 0.f)
		m_fRetailScale = fWidth / FRAME_WIDTH;
	Load_Data();
	/* LEVEL::STATIC sprites are visible from construction; the HUD gate shows them. */
	Hide();
}

Client::CCombatAnalysisFrameView::~CCombatAnalysisFrameView() = default;

void Client::CCombatAnalysisFrameView::Load_Data()
{
	m_Stats.clear();
	const filesystem::path DataPath =
		CProjectDataRoot::Resolve(L"UI/CombatAnalysis/CombatAnalysisUi.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[CombatAnalysis] CombatAnalysisUi.json missing -- tiles stay blank.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[CombatAnalysis] CombatAnalysisUi.json parse failed: " + Error + "\n").c_str());
		return;
	}
	const auto ReadText = [](const DATA_JSON_VALUE& Object, const char* pKey, wstring& outText)
	{
		const DATA_JSON_VALUE* pText = Object.Find(pKey);
		if (nullptr != pText && pText->Is_String())
			(void)ConvertUtf8ToWide(pText->Get_String(), outText);
	};
	if (const DATA_JSON_VALUE* pStrings = Root.Find("strings"); nullptr != pStrings && pStrings->Is_Object())
	{
		ReadText(*pStrings, "valueNull", m_strValueNull);
		if (const DATA_JSON_VALUE* pUnits = pStrings->Find("units"); nullptr != pUnits && pUnits->Is_Array())
		{
			for (const DATA_JSON_VALUE& Unit : pUnits->Get_Array())
			{
				wstring strUnit;
				if (Unit.Is_String())
					(void)ConvertUtf8ToWide(Unit.Get_String(), strUnit);
				m_Units.push_back(strUnit);
			}
		}
	}
	if (const DATA_JSON_VALUE* pStats = Root.Find("stats"); nullptr != pStats && pStats->Is_Array())
	{
		for (const DATA_JSON_VALUE& Value : pStats->Get_Array())
		{
			if (!Value.Is_Object() || m_Stats.size() >= STAT_COUNT)
				continue;
			STAT_TILE Tile{};
			if (const DATA_JSON_VALUE* pId = Value.Find("id"); nullptr != pId && pId->Is_String())
				Tile.strId = pId->Get_String();
			ReadText(Value, "title", Tile.strTitle);
			if (const DATA_JSON_VALUE* pFormat = Value.Find("format"); nullptr != pFormat && pFormat->Is_String())
				Tile.bCount = pFormat->Get_String() == "count";
			m_Stats.push_back(std::move(Tile));
		}
	}
}

void Client::CCombatAnalysisFrameView::Update(const f32_t fTimeDelta, const HUD_PLAYER_STATE& Player)
{
	(void)fTimeDelta;
	const HUD_COMBAT_ANALYSIS& Analysis = CCombatHUDViewModel::Get().Get_CombatAnalysis();
	m_iServerTick = Player.iServerTick;
	/* X hides the frame for the raid it was closed in; the next raid's first hit (a new
	start tick after Reset_RuntimeState) brings it back. */
	if (m_bHidden && Analysis.isActive && Analysis.iStartTick != m_iHiddenFightStartTick)
		m_bHidden = false;
	if (m_bHidden)
	{
		Hide();
		m_Drag.Reset();
		return;
	}
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, true);
	m_bShown = true;

	m_Drag.Update(*m_pView, m_SlotIds, Ref_X(0.f), Ref_Y(0.f),
		FRAME_WIDTH * m_fRetailScale, TITLE_BAR_HEIGHT * m_fRetailScale, "CA_Close");
	Update_Chrome();

	/* Anything over the frame belongs to the frame -- no move command underneath. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("CA_Bg", fX, fY, fWidth, fHeight) &&
		Router.Is_Hovered(fX, fY, fWidth, fHeight,
			m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
	{
		Router.Claim_Mouse_This_Frame();
	}
}

void Client::CCombatAnalysisFrameView::Update_Chrome()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("CA_Close", fX, fY, fWidth, fHeight))
		return;
	const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("CA_Close", bHovered ? ART_CLOSE_OVER : ART_CLOSE_NORMAL);
	if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		m_bHidden = true;
		m_iHiddenFightStartTick = CCombatHUDViewModel::Get().Get_CombatAnalysis().iStartTick;
	}
}

wstring Client::CCombatAnalysisFrameView::Format_Amount(const uint64_t iValue) const
{
	wchar_t szBuffer[32] = {};
	if (iValue < 10000ull || m_Units.empty())
	{
		(void)swprintf_s(szBuffer, L"%llu", static_cast<unsigned long long>(iValue));
		return szBuffer;
	}
	/* 만 / 억 / 조 / 경: each unit is 10^4 of the previous; two decimals like the retail
	MoneyLabel ("11.48억", "249.08만"). */
	uint64_t iDivisor = 10000ull;
	size_t iUnit = 0;
	while (iUnit + 1 < m_Units.size() && iValue >= iDivisor * 10000ull)
	{
		iDivisor *= 10000ull;
		++iUnit;
	}
	const f64_t fScaled = static_cast<f64_t>(iValue) / static_cast<f64_t>(iDivisor);
	(void)swprintf_s(szBuffer, L"%.2f", fScaled);
	return wstring(szBuffer) + m_Units[iUnit];
}

void Client::CCombatAnalysisFrameView::Render_Text()
{
	if (m_bHidden || !m_bShown || nullptr == m_pView)
		return;
	f32_t fOriginX = 0.f, fOriginY = 0.f;
	if (!Get_FrameOrigin(fOriginX, fOriginY))
		return;
	const HUD_COMBAT_ANALYSIS& Analysis = CCombatHUDViewModel::Get().Get_CombatAnalysis();
	const uint32_t iElapsedTicks = Analysis.Get_ElapsedTicks(m_iServerTick);
	const uint32_t iElapsedSeconds = iElapsedTicks / static_cast<uint32_t>(SERVER_TICK_HZ);

	wchar_t szTime[16] = {};
	(void)swprintf_s(szTime, L"%02u : %02u", iElapsedSeconds / 60u, iElapsedSeconds % 60u);
	const float2_t vTopLeft(0.f, 0.f);
	const float2_t vCenter(0.5f, 0.5f);
	Draw_Label(FONT_YG760, szTime, TIME_X, TIME_Y, TILE_FONT_PX, COLOR_TIME, vTopLeft);

	for (size_t i = 0; i < m_Stats.size(); ++i)
	{
		const STAT_TILE& Tile = m_Stats[i];
		const f32_t fCenterX = TILE_PITCH * static_cast<f32_t>(i) + TILE_WIDTH * 0.5f;
		Draw_Label(FONT_YG760, Tile.strTitle, fCenterX, TITLE_ROW_Y + TILE_HEIGHT * 0.5f,
			TILE_FONT_PX, Colors::White, vCenter);

		wstring strValue = m_strValueNull;
		if (Analysis.isActive)
		{
			if (Tile.strId == "damage")
				strValue = Format_Amount(Analysis.iTotalDamage);
			else if (Tile.strId == "dps")
				strValue = Format_Amount(static_cast<uint64_t>(
					static_cast<f64_t>(Analysis.iTotalDamage) * SERVER_TICK_HZ /
					static_cast<f64_t>((std::max)(iElapsedTicks, 1u))));
			else if (Tile.strId == "stagger")
				strValue = Format_Amount(Analysis.iTotalStagger);
			else if (Tile.strId == "counter")
				strValue = Format_Amount(Analysis.iCounterSuccesses);
		}
		Draw_Label(FONT_YG760, strValue, fCenterX, VALUE_ROW_Y + TILE_HEIGHT * 0.5f,
			TILE_FONT_PX, Colors::White, vCenter);
	}
}

void Client::CCombatAnalysisFrameView::Hide()
{
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, false);
	m_bShown = false;
}

bool_t Client::CCombatAnalysisFrameView::Get_FrameOrigin(f32_t& fX, f32_t& fY) const
{
	f32_t fWidth = 0.f, fHeight = 0.f;
	return m_pView->Get_SlotRect("CA_Bg", fX, fY, fWidth, fHeight);
}

f32_t Client::CCombatAnalysisFrameView::Ref_X(const f32_t fRetailX) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_FrameOrigin(fX, fY);
	return fX + fRetailX * m_fRetailScale;
}

f32_t Client::CCombatAnalysisFrameView::Ref_Y(const f32_t fRetailY) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_FrameOrigin(fX, fY);
	return fY + fRetailY * m_fRetailScale;
}

void Client::CCombatAnalysisFrameView::Draw_Label(const wstring_t& strFont, const wstring& strText,
	const f32_t fRetailX, const f32_t fRetailY, const f32_t fRetailPx, const fvector_t vColor,
	const float2_t& vOrigin)
{
	if (strText.empty())
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	/* Same rule as the other runtime windows' Draw_Label: nearest baked size, whole-pixel
	top-left anchoring, drop shadow. */
	f32_t fScale = 1.f;
	const wstring_t strUseFont = UILabelFont::Resolve(strFont,
		fRetailPx * m_fRetailScale * fUiScale * TEXT_BOOST, fScale);
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(strUseFont, strText.c_str());
	const float2_t vPosition(
		std::round(Ref_X(fRetailX) * fScaleX - vMeasured.x * fScale * vOrigin.x),
		std::round(Ref_Y(fRetailY) * fScaleY - vMeasured.y * fScale * vOrigin.y));
	const float2_t vTopLeft(0.f, 0.f);
	if (CUIInputRouter::Get().Is_UnderTopWindow(
		vPosition.x + vMeasured.x * fScale * 0.5f, vPosition.y + vMeasured.y * fScale * 0.5f))
		return;
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, vTopLeft, fScale);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(), vPosition, vColor, 0.f, vTopLeft, fScale);
}
