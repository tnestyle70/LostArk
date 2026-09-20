/* WinSock2 ahead of everything else (Client_Defines pulls the socket headers). */
#include <WinSock2.h>

#include "HonorTitleWindowView.h"

#include <DirectXColors.h>

#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "HonorTitleCatalog.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UITextOcclusion.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>

namespace
{
	/* honortitle.gfx is authored at 1920x1080; the layout document scales it onto the 1280x720
	reference (2/3 x the document's own enlargement), so every retail px offset below goes
	through m_fRetailScale, read from the HT_WinBg slot against this retail width. */
	constexpr f32_t TEXT_BOOST = 1.15f;
	constexpr f32_t WINDOW_WIDTH = 452.f;
	constexpr f32_t TITLE_BAR_HEIGHT = 49.f;
	constexpr f32_t TITLE_Y = 12.f;
	constexpr f32_t TITLE_PX = 18.f;
	/* completeTitleList (18,167), HonorTitleRendererItem_big 395x32: text at (27,6) 14 px,
	useType_mc at (334,0) with the "using" badge at (-62,5) 60x30. */
	constexpr int32_t VISIBLE_ROWS = 12;
	constexpr f32_t ROW_X = 18.f;
	constexpr f32_t ROW_Y0 = 167.f;
	constexpr f32_t ROW_W = 395.f;
	constexpr f32_t ROW_H = 32.f;
	constexpr f32_t ROW_TEXT_X = 27.f;
	constexpr f32_t ROW_TEXT_Y = 6.f;
	constexpr f32_t ROW_FONT_PX = 14.f;
	constexpr f32_t BADGE_X = 334.f - 62.f;
	constexpr f32_t BADGE_Y = 5.f;
	constexpr f32_t BADGE_W = 60.f;
	constexpr f32_t BADGE_H = 30.f;
	/* currentTitleTF sits 22 px under the list, the button row 50 px under that
	(build_honor_title_data.py CURRENT_Y / BUTTON_Y). */
	constexpr f32_t CURRENT_X = 17.f;
	constexpr f32_t CURRENT_Y = ROW_Y0 + ROW_H * VISIBLE_ROWS + 22.f;
	constexpr f32_t CURRENT_PX = 14.f;
	constexpr f32_t BUTTON_Y = CURRENT_Y + 50.f;
	constexpr f32_t APPLY_X = 106.f;
	constexpr f32_t DESELECT_X = 227.95f;
	constexpr f32_t BUTTON_W = 103.f;
	constexpr f32_t BUTTON_H = 36.f;
	constexpr f32_t BUTTON_PX = 16.f;
	/* DefaultEFScrollBarSmall_V2 at (416,167), sized to the list: track 18 wide, 14x14 arrows 2 px
	in from each end, the thumb (14 wide) travelling between y 21 and h-19 of the component. */
	constexpr f32_t SCROLL_X = 416.f;
	constexpr f32_t SCROLL_THUMB_TOP = 21.f;
	constexpr f32_t SCROLL_THUMB_BOTTOM_PAD = 19.f;
	constexpr f32_t SCROLL_THUMB_MIN_H = 24.f;

	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");

	const fvector_t COLOR_TITLE = XMVectorSet(1.f, 247.f / 255.f, 226.f / 255.f, 1.f);        // #fff7e2
	const fvector_t COLOR_ROW = Colors::White;                                                 // textField #FFFFFF
	const fvector_t COLOR_USING = XMVectorSet(27.f / 255.f, 1.f, 140.f / 255.f, 1.f);          // #1BFF8C
	const fvector_t COLOR_LABEL = XMVectorSet(243.f / 255.f, 228.f / 255.f, 188.f / 255.f, 1.f); // #F3E4BC
	const fvector_t COLOR_MUTED = XMVectorSet(164.f / 255.f, 175.f / 255.f, 183.f / 255.f, 1.f); // #A4AFB7
	const fvector_t COLOR_DISABLED = XMVectorSet(120.f / 255.f, 120.f / 255.f, 120.f / 255.f, 1.f);

	const char* ART_BTN_NORMAL = "UI/HonorTitle/HonorTitle_Btn_Normal.png";
	const char* ART_BTN_OVER = "UI/HonorTitle/HonorTitle_Btn_Over.png";
	const char* ART_BTN_DOWN = "UI/HonorTitle/HonorTitle_Btn_Down.png";
	const char* ART_BTN_DISABLED = "UI/HonorTitle/HonorTitle_Btn_Disabled.png";
	const char* ART_CLOSE_NORMAL = "UI/HonorTitle/HonorTitle_Close_Normal.png";
	const char* ART_CLOSE_OVER = "UI/HonorTitle/HonorTitle_Close_Over.png";
	const char* ART_THUMB_NORMAL = "UI/HonorTitle/HonorTitle_ScrollThumb_Normal.png";
	const char* ART_THUMB_OVER = "UI/HonorTitle/HonorTitle_ScrollThumb_Over.png";
	const char* ART_UP_NORMAL = "UI/HonorTitle/HonorTitle_ScrollUp_Normal.png";
	const char* ART_UP_OVER = "UI/HonorTitle/HonorTitle_ScrollUp_Over.png";
	const char* ART_DOWN_NORMAL = "UI/HonorTitle/HonorTitle_ScrollDown_Normal.png";
	const char* ART_DOWN_OVER = "UI/HonorTitle/HonorTitle_ScrollDown_Over.png";

	string Row_Slot(const int32_t iRow, const char* pSuffix)
	{
		return "HT_Row" + std::to_string(iRow) + "_" + pSuffix;
	}
}

Client::CHonorTitleWindowView::CHonorTitleWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/HonorTitle/HonorTitle_Layout.json") }
{
	/* Draw order for this window's panel; the same number orders its labels
	(Register_UITextOccluders) and its clicks. */
	m_pView->Set_UISortLayer(UI_TEXT_LAYER::WINDOW_HONOR_TITLE);
	m_SlotIds = m_pView->Get_SlotIds();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("HT_WinBg", fX, fY, fWidth, fHeight) && fWidth > 0.f)
		m_fRetailScale = fWidth / WINDOW_WIDTH;
	/* Same reason as CInventoryView: LEVEL::STATIC sprites are visible from construction. */
	Hide();
}

Client::CHonorTitleWindowView::~CHonorTitleWindowView() = default;

void Client::CHonorTitleWindowView::Toggle()
{
	m_bOpen = !m_bOpen;
	m_bSelectWornOnUpdate = m_bOpen;
}

int32_t Client::CHonorTitleWindowView::Row_Count() const
{
	return static_cast<int32_t>(CHonorTitleCatalog::Get_Titles().size());
}

int32_t Client::CHonorTitleWindowView::Max_Scroll() const
{
	return (std::max)(0, Row_Count() - VISIBLE_ROWS);
}

void Client::CHonorTitleWindowView::Update(const f32_t fTimeDelta, const HUD_PLAYER_STATE& Player)
{
	/* Hit tests below belong to this window; the router refuses a press that lands on the
	window in front and lets only one widget take any one press. */
	CUIPointerScope PointerScope(this);
	(void)fTimeDelta;
	if (!m_bOpen)
	{
		Hide();
		m_bDraggingThumb = false;
		m_Drag.Reset();
		return;
	}

	const vector<HONOR_TITLE_ENTRY>& Titles = CHonorTitleCatalog::Get_Titles();
	m_iWornTitleId = Player.iHonorTitleId;
	if (m_bSelectWornOnUpdate)
	{
		m_bSelectWornOnUpdate = false;
		m_iSelectedTitle = -1;
		for (size_t i = 0; i < Titles.size(); ++i)
		{
			if (Titles[i].iTitleId == Player.iHonorTitleId)
				m_iSelectedTitle = static_cast<int32_t>(i);
		}
		if (m_iSelectedTitle < 0 && !Titles.empty())
			m_iSelectedTitle = 0;
		/* Scroll so the selected row is inside the visible window. */
		m_iScroll = std::clamp(m_iSelectedTitle - VISIBLE_ROWS / 2, 0, Max_Scroll());
	}
	if (m_iSelectedTitle >= Row_Count())
		m_iSelectedTitle = Titles.empty() ? -1 : 0;

	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, true);

	/* Header-band drag first: while a drag is live the rows / buttons below see no click. */
	m_Drag.Update(*m_pView, m_SlotIds, Ref_X(0.f), Ref_Y(0.f),
		WINDOW_WIDTH * m_fRetailScale, TITLE_BAR_HEIGHT * m_fRetailScale, "HT_Close");
	Update_Chrome();
	Update_Scroll();
	Update_Rows(Player);
	Update_Buttons(Player);

	/* Anything over the window belongs to the window; while open it is the topmost runtime UI
	(it opens over the info window), so the other windows' text passes skip its rect. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("HT_WinBg", fX, fY, fWidth, fHeight))
	{
		if (Router.Is_Hovered(fX, fY, fWidth, fHeight,
			m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
			Router.Claim_Mouse_This_Frame();
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		if (vViewport.x > 0.f && vViewport.y > 0.f)
		{
			const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
			const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
			Router.Set_TopWindowRect(this, fX * fScaleX, fY * fScaleY, fWidth * fScaleX, fHeight * fScaleY);
		}
	}
}

void Client::CHonorTitleWindowView::Update_Chrome()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("HT_Close", fX, fY, fWidth, fHeight))
		return;
	const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("HT_Close", bHovered ? ART_CLOSE_OVER : ART_CLOSE_NORMAL);
	if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		Close();
	}
}

void Client::CHonorTitleWindowView::Update_Scroll()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	const int32_t iMaxScroll = Max_Scroll();
	const f32_t fListX = Ref_X(ROW_X), fListY = Ref_Y(ROW_Y0);
	const f32_t fListW = (SCROLL_X + 18.f - ROW_X) * m_fRetailScale;
	const f32_t fListH = ROW_H * VISIBLE_ROWS * m_fRetailScale;

	/* Wheel anywhere over the list or the bar: one row per notch (WM_MOUSEWHEEL through the
	router -- DirectInput's wheel is blocked while this window claims the mouse). */
	if (Router.Is_Hovered(fListX, fListY, fListW, fListH, fRefWidth, fRefHeight))
		m_iScroll -= Router.Get_MouseWheelNotches();

	/* Arrows: a row per click. */
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (m_pView->Get_SlotRect("HT_ScrollUp", fX, fY, fW, fH))
	{
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture("HT_ScrollUp", bHovered ? ART_UP_OVER : ART_UP_NORMAL);
		if (bHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
			--m_iScroll;
	}
	if (m_pView->Get_SlotRect("HT_ScrollDown", fX, fY, fW, fH))
	{
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture("HT_ScrollDown", bHovered ? ART_DOWN_OVER : ART_DOWN_NORMAL);
		if (bHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
			++m_iScroll;
	}

	/* Thumb geometry in reference px: the travel is the component's y 21 .. h-19 band, the thumb
	takes the visible share of it (never thinner than the art's own minimum). */
	const f32_t fTravelTop = Ref_Y(ROW_Y0 + SCROLL_THUMB_TOP);
	const f32_t fTravelH = (ROW_H * VISIBLE_ROWS - SCROLL_THUMB_TOP - SCROLL_THUMB_BOTTOM_PAD) * m_fRetailScale;
	const int32_t iRows = (std::max)(Row_Count(), 1);
	const f32_t fThumbH = (std::max)(SCROLL_THUMB_MIN_H * m_fRetailScale,
		fTravelH * static_cast<f32_t>((std::min)(VISIBLE_ROWS, iRows)) / static_cast<f32_t>(iRows));
	const f32_t fThumbRange = (std::max)(fTravelH - fThumbH, 0.f);
	const f32_t fThumbX = Ref_X(SCROLL_X + 2.f);
	const f32_t fThumbW = 14.f * m_fRetailScale;

	/* Thumb drag: press on the thumb, then the cursor's Y delta maps back onto rows. */
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHaveMouse = Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY);
	const f32_t fThumbY = fTravelTop + (iMaxScroll > 0 ?
		fThumbRange * static_cast<f32_t>(std::clamp(m_iScroll, 0, iMaxScroll)) / static_cast<f32_t>(iMaxScroll) : 0.f);
	const bool_t bThumbHovered = Router.Is_Hovered(fThumbX, fThumbY, fThumbW, fThumbH, fRefWidth, fRefHeight);
	if (m_bDraggingThumb)
	{
		if (!Router.Is_LeftDown())
			m_bDraggingThumb = false;
		else if (bHaveMouse && fThumbRange > 0.f && iMaxScroll > 0)
		{
			const f32_t fRowsPerPx = static_cast<f32_t>(iMaxScroll) / fThumbRange;
			m_iScroll = m_iThumbDragScroll +
				static_cast<int32_t>(std::lround((fMouseY - m_fThumbDragMouseY) * fRowsPerPx));
			Router.Claim_Mouse_This_Frame();
		}
	}
	else if (bThumbHovered && iMaxScroll > 0 && bHaveMouse &&
		Router.Is_Clicked(fThumbX, fThumbY, fThumbW, fThumbH, fRefWidth, fRefHeight))
	{
		m_bDraggingThumb = true;
		m_fThumbDragMouseY = fMouseY;
		m_iThumbDragScroll = m_iScroll;
	}
	else if (iMaxScroll > 0 && bHaveMouse &&
		Router.Is_Hovered(fThumbX, fTravelTop, fThumbW, fTravelH, fRefWidth, fRefHeight) &&
		Router.Is_Clicked(fThumbX, fTravelTop, fThumbW, fTravelH, fRefWidth, fRefHeight))
	{
		/* Track click outside the thumb pages towards the cursor. */
		m_iScroll += fMouseY < fThumbY ? -VISIBLE_ROWS : VISIBLE_ROWS;
	}

	m_iScroll = std::clamp(m_iScroll, 0, iMaxScroll);
	const f32_t fFinalThumbY = fTravelTop + (iMaxScroll > 0 ?
		fThumbRange * static_cast<f32_t>(m_iScroll) / static_cast<f32_t>(iMaxScroll) : 0.f);
	m_pView->Set_SlotRect("HT_ScrollThumb", fThumbX, fFinalThumbY, fThumbW, fThumbH);
	m_pView->Set_SlotTexture("HT_ScrollThumb",
		(bThumbHovered || m_bDraggingThumb) ? ART_THUMB_OVER : ART_THUMB_NORMAL);
}

void Client::CHonorTitleWindowView::Update_Rows(const HUD_PLAYER_STATE& Player)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	const vector<HONOR_TITLE_ENTRY>& Titles = CHonorTitleCatalog::Get_Titles();

	const f32_t fListX = Ref_X(ROW_X);
	const f32_t fListW = ROW_W * m_fRetailScale;
	m_iScroll = std::clamp(m_iScroll, 0, Max_Scroll());

	m_iHoveredTitle = -1;
	for (int32_t iRow = 0; iRow < VISIBLE_ROWS; ++iRow)
	{
		const int32_t iTitle = m_iScroll + iRow;
		const bool_t bHasRow = iTitle < static_cast<int32_t>(Titles.size());
		if (!bHasRow)
		{
			for (const char* pSuffix : { "Over", "Selected", "Using" })
				m_pView->Set_SlotVisible(Row_Slot(iRow, pSuffix), false);
			continue;
		}
		const HONOR_TITLE_ENTRY& Entry = Titles[static_cast<size_t>(iTitle)];
		const f32_t fY = Ref_Y(ROW_Y0 + ROW_H * static_cast<f32_t>(iRow));
		const bool_t bHovered = Router.Is_Hovered(fListX, fY, fListW, ROW_H * m_fRetailScale, fRefWidth, fRefHeight);
		if (bHovered)
		{
			m_iHoveredTitle = iTitle;
			if (Router.Is_Clicked(fListX, fY, fListW, ROW_H * m_fRetailScale, fRefWidth, fRefHeight) &&
				iTitle != m_iSelectedTitle)
			{
				CMainApp::Play_UIButtonClickSound();
				m_iSelectedTitle = iTitle;
			}
		}
		const bool_t bSelected = iTitle == m_iSelectedTitle;
		m_pView->Set_SlotVisible(Row_Slot(iRow, "Over"), bHovered && !bSelected);
		m_pView->Set_SlotVisible(Row_Slot(iRow, "Selected"), bSelected);
		m_pView->Set_SlotVisible(Row_Slot(iRow, "Using"),
			0u != Player.iHonorTitleId && Entry.iTitleId == Player.iHonorTitleId);
	}
}

void Client::CHonorTitleWindowView::Update_Buttons(const HUD_PLAYER_STATE& Player)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	const vector<HONOR_TITLE_ENTRY>& Titles = CHonorTitleCatalog::Get_Titles();
	const bool_t bLive = Player.isValid && !Player.isPreview;
	const bool_t bHasSelection = m_iSelectedTitle >= 0 && m_iSelectedTitle < static_cast<int32_t>(Titles.size());
	m_bApplyEnabled = bLive && bHasSelection &&
		Titles[static_cast<size_t>(m_iSelectedTitle)].iTitleId != Player.iHonorTitleId;
	m_bRemoveEnabled = bLive && 0u != Player.iHonorTitleId;

	struct BUTTON { const char* pSlotId; bool_t bEnabled; uint32_t iRequest; };
	const BUTTON Buttons[2] = {
		{ "HT_ApplyBtn", m_bApplyEnabled, bHasSelection ? Titles[static_cast<size_t>(m_iSelectedTitle)].iTitleId : 0u },
		{ "HT_DeselectBtn", m_bRemoveEnabled, 0u },
	};
	for (const BUTTON& Button : Buttons)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture(Button.pSlotId,
			!Button.bEnabled ? ART_BTN_DISABLED : (bHovered ? ART_BTN_OVER : ART_BTN_NORMAL));
		if (bHovered && Button.bEnabled &&
			Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_pView->Set_SlotTexture(Button.pSlotId, ART_BTN_DOWN);
			m_bTitleRequested = true;
			m_iRequestedTitleId = Button.iRequest;
		}
	}
}

bool_t Client::CHonorTitleWindowView::Take_TitleRequest(uint32_t& outTitleId)
{
	if (!m_bTitleRequested)
		return false;
	m_bTitleRequested = false;
	outTitleId = m_iRequestedTitleId;
	return true;
}

void Client::CHonorTitleWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;
	f32_t fOriginX = 0.f, fOriginY = 0.f;
	if (!Get_WindowOrigin(fOriginX, fOriginY))
		return;

	const float2_t vTopLeft(0.f, 0.f);
	const float2_t vTopCenter(0.5f, 0.f);
	const float2_t vCenter(0.5f, 0.5f);
	const HONOR_TITLE_STRINGS& Strings = CHonorTitleCatalog::Get_Strings();
	const vector<HONOR_TITLE_ENTRY>& Titles = CHonorTitleCatalog::Get_Titles();

	Draw_Label(FONT_YOON, Strings.strWindowTitle, WINDOW_WIDTH * 0.5f, TITLE_Y, TITLE_PX, COLOR_TITLE, vTopCenter);

	for (int32_t iRow = 0; iRow < VISIBLE_ROWS; ++iRow)
	{
		const int32_t iTitle = m_iScroll + iRow;
		if (iTitle >= static_cast<int32_t>(Titles.size()))
			break;
		const HONOR_TITLE_ENTRY& Entry = Titles[static_cast<size_t>(iTitle)];
		const f32_t fRowY = ROW_Y0 + ROW_H * static_cast<f32_t>(iRow);
		/* Yoon family, not the retail $YG760: that face is a non-square design whose short
		   syllables sit above the baseline, which in a list reads as the wrong glyph size. */
		Draw_Label(FONT_YOON, Entry.strName, ROW_X + ROW_TEXT_X, fRowY + ROW_TEXT_Y,
			ROW_FONT_PX, COLOR_ROW, vTopLeft);
		if (0u != m_iWornTitleId && Entry.iTitleId == m_iWornTitleId)
		{
			Draw_Label(FONT_YOON, Strings.strUsing, ROW_X + BADGE_X + BADGE_W * 0.5f,
				fRowY + BADGE_Y + BADGE_H * 0.5f, ROW_FONT_PX, COLOR_USING, vCenter);
		}
	}

	/* "<current> : <name>" -- the worn title, or the retail empty marker. */
	const wstring* pWorn = CHonorTitleCatalog::Find_Name(m_iWornTitleId);
	const f32_t fLabelWidth = Draw_Label(FONT_YG760, Strings.strCurrent + L" : ",
		CURRENT_X, CURRENT_Y, CURRENT_PX, COLOR_LABEL, vTopLeft);
	Draw_Label(FONT_YG760, nullptr != pWorn ? *pWorn : Strings.strNone,
		CURRENT_X + fLabelWidth, CURRENT_Y, CURRENT_PX, nullptr != pWorn ? COLOR_ROW : COLOR_MUTED, vTopLeft);

	Draw_Label(FONT_YOON, Strings.strApply, APPLY_X + BUTTON_W * 0.5f, BUTTON_Y + BUTTON_H * 0.5f,
		BUTTON_PX, m_bApplyEnabled ? Colors::White : COLOR_DISABLED, vCenter);
	Draw_Label(FONT_YOON, Strings.strRemove, DESELECT_X + BUTTON_W * 0.5f, BUTTON_Y + BUTTON_H * 0.5f,
		BUTTON_PX, m_bRemoveEnabled ? Colors::White : COLOR_DISABLED, vCenter);
}

void Client::CHonorTitleWindowView::Hide()
{
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, false);
}

bool_t Client::CHonorTitleWindowView::Get_WindowOrigin(f32_t& fX, f32_t& fY) const
{
	f32_t fWidth = 0.f, fHeight = 0.f;
	return m_pView->Get_SlotRect("HT_WinBg", fX, fY, fWidth, fHeight);
}

f32_t Client::CHonorTitleWindowView::Ref_X(const f32_t fRetailX) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fX + fRetailX * m_fRetailScale;
}

f32_t Client::CHonorTitleWindowView::Ref_Y(const f32_t fRetailY) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fY + fRetailY * m_fRetailScale;
}

f32_t Client::CHonorTitleWindowView::Draw_Label(const wstring_t& strFont, const wstring& strText,
	const f32_t fRetailX, const f32_t fRetailY, const f32_t fRetailPx, const fvector_t vColor,
	const float2_t& vOrigin)
{
	if (strText.empty())
		return 0.f;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return 0.f;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	/* Baked size nearest the on-screen line height, drawn 1:1 when it matches (no blur) --
	same rule as CVehicleWindowView::Draw_Label, whole-pixel top-left anchoring included. */
	f32_t fScale = 1.f;
	const wstring_t strUseFont = UILabelFont::Resolve(strFont,
		fRetailPx * m_fRetailScale * fUiScale * TEXT_BOOST, fScale);
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(strUseFont, strText.c_str());
	const float2_t vPosition(
		std::round(Ref_X(fRetailX) * fScaleX - vMeasured.x * fScale * vOrigin.x),
		std::round(Ref_Y(fRetailY) * fScaleY - vMeasured.y * fScale * vOrigin.y));
	const float2_t vTopLeft(0.f, 0.f);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, vTopLeft, fScale);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(), vPosition, vColor, 0.f, vTopLeft, fScale);
	/* Screen px -> retail px so the caller can place a follow-up label right after this one. */
	return vMeasured.x * fScale / (fScaleX * m_fRetailScale);
}

bool_t Client::CHonorTitleWindowView::Get_ScreenRect(
	f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	if (!Is_Open() || nullptr == m_pView)
		return false;
	f32_t fRefX = 0.f, fRefY = 0.f, fRefWidth = 0.f, fRefHeight = 0.f;
	if (!m_pView->Get_SlotRect("HT_WinBg", fRefX, fRefY, fRefWidth, fRefHeight))
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
