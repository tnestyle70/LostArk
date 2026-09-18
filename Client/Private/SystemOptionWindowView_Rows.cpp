/* The option rows of CSystemOptionWindowView: retail's relative layout rule, the runtime
widget sprites each control kind owns, their per-frame placement / interaction, and their text.
Chrome, tab column, scrolling and buttons live in SystemOptionWindowView.cpp. */

#include "SystemOptionWindowView.h"
#include "SystemOptionWindowView_Internal.h"

#include "GameInstance.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <set>

using namespace Client::SystemOptionGeometry;

namespace
{
	string Slot(const SYSTEM_OPTION_ROW& Row, const char* pPart)
	{
		return "SO_R" + std::to_string(Row.iPrimaryKey) + "_" + pPart;
	}

	string Slot(const SYSTEM_OPTION_ROW& Row, const char* pPart, const int32_t iIndex)
	{
		return Slot(Row, pPart) + std::to_string(iIndex);
	}

	bool_t Is_FixedCombo(const SYSTEM_OPTION_ROW& Row)
	{
		/* Resolution and screen mode are what this executable is; the rows show the fact. */
		return Row.strId == "combobox_resolution" || Row.strId == "combobox_monitor";
	}

	f32_t Snap(const f32_t fValue, const SYSTEM_OPTION_ROW& Row)
	{
		f32_t fSnapped = fValue;
		if (Row.iSnapInterval > 0)
			fSnapped = std::round(fValue / static_cast<f32_t>(Row.iSnapInterval)) * static_cast<f32_t>(Row.iSnapInterval);
		else
			fSnapped = std::round(fValue);
		return std::clamp(fSnapped, Row.fMinimum, Row.fMaximum);
	}

	wstring Format_Value(const f32_t fValue, const SYSTEM_OPTION_ROW& Row)
	{
		return std::to_wstring(static_cast<int32_t>(std::lround(fValue))) + Row.strUnit;
	}
}

/* Rows retail greys out because of another row's value -- the two rules the data makes
plain: the upscaling mode needs an upscaling type, and the mokoko cursor (preset 6) has no
outline variants in EFUI_CURSOR, so the outline combo has nothing to offer there. */
bool_t Client::CSystemOptionWindowView::Is_RetailDisabled(const SYSTEM_OPTION_ROW& Row) const
{
	if (Row.strId == "combobox_upscaling_mode")
		return 0 == static_cast<int32_t>(std::lround(m_Draft.Get("combobox_upscaling_type", 0.f)));
	if (Row.strId == SystemOptionRowId::CURSOR_PRESET_OUTLINE)
		return 6 == static_cast<int32_t>(std::lround(m_Draft.Get(SystemOptionRowId::CURSOR_PRESET, 0.f)));
	return false;
}

/* ---- layout --------------------------------------------------------------------------- */

f32_t Client::CSystemOptionWindowView::Row_Height(const SYSTEM_OPTION_ROW& Row) const
{
	switch (Row.eControl)
	{
	case SYSTEM_OPTION_CONTROL::TAB_TITLE:	return HEIGHT_TAB_TITLE;
	case SYSTEM_OPTION_CONTROL::GROUP_TITLE:return HEIGHT_GROUP_TITLE;
	case SYSTEM_OPTION_CONTROL::SEPARATOR:	return HEIGHT_SEPARATOR;
	case SYSTEM_OPTION_CONTROL::COMBOBOX:	return HEIGHT_COMBOBOX;
	case SYSTEM_OPTION_CONTROL::SLIDER:		return HEIGHT_SLIDER;
	case SYSTEM_OPTION_CONTROL::CHECKBOX:	return HEIGHT_CHECKBOX;
	case SYSTEM_OPTION_CONTROL::SPINNER:	return HEIGHT_SPINNER;
	case SYSTEM_OPTION_CONTROL::BUTTON:		return HEIGHT_BUTTON;
	case SYSTEM_OPTION_CONTROL::SUBTITLE:	return HEIGHT_SUBTITLE;
	case SYSTEM_OPTION_CONTROL::COLOR:		return HEIGHT_COLOR;
	case SYSTEM_OPTION_CONTROL::TEXT_INPUT:	return HEIGHT_TEXT_INPUT;
	case SYSTEM_OPTION_CONTROL::SCALE_LIST:
	case SYSTEM_OPTION_CONTROL::COUNT_LIST:
	case SYSTEM_OPTION_CONTROL::RADIO_PAIR:
	{
		const int32_t iLines = (static_cast<int32_t>(Row.Choices.size()) + RADIO_COLUMNS - 1) / RADIO_COLUMNS;
		return HEIGHT_RADIO_ROW * static_cast<f32_t>((std::max)(1, iLines)) - (HEIGHT_RADIO_ROW - HEIGHT_DEFAULT);
	}
	default:								return HEIGHT_DEFAULT;
	}
}

void Client::CSystemOptionWindowView::Layout_Screen(const SYSTEM_OPTION_TAB& Tab, vector<ROW_LAYOUT>& OutRows) const
{
	/* Every row hangs off another row's primaryKey. The one key no row owns is the screen's
	own TAB_TITLE row, which the document folds into the tab entry: it sits at the pane
	heading. Hidden (retail Enabled=0) rows take their parent's origin with zero height, so a
	chain through them collapses the way retail's does. */
	struct PLACED { f32_t fX, fY, fHeight; };
	map<int32_t, PLACED> Placed;
	std::set<int32_t> Keys;
	for (const SYSTEM_OPTION_ROW& Row : Tab.Rows)
		Keys.insert(Row.iPrimaryKey);
	for (const SYSTEM_OPTION_ROW& Row : Tab.Rows)
		if (0 == Keys.count(Row.iAttachTarget))
			Placed[Row.iAttachTarget] = PLACED{ PANE_X + 3.f, TAB_TITLE_Y, HEIGHT_TAB_TITLE };

	vector<const SYSTEM_OPTION_ROW*> Pending;
	for (const SYSTEM_OPTION_ROW& Row : Tab.Rows)
		Pending.push_back(&Row);
	OutRows.clear();
	size_t iGuard = 0;
	while (!Pending.empty() && iGuard++ < Tab.Rows.size() * Tab.Rows.size() + 1)
	{
		const SYSTEM_OPTION_ROW* pRow = Pending.front();
		Pending.erase(Pending.begin());
		const auto parent = Placed.find(pRow->iAttachTarget);
		if (Placed.end() == parent)
		{
			Pending.push_back(pRow);
			continue;
		}
		const PLACED& Parent = parent->second;
		if (!pRow->bEnabled)
		{
			Placed[pRow->iPrimaryKey] = PLACED{ Parent.fX, Parent.fY, 0.f };
			continue;
		}
		const f32_t fHeight = Row_Height(*pRow);
		PLACED Mine{};
		if (2 == pRow->iDirection)
		{
			f32_t fAnchor = 0.f;
			switch (pRow->eControl)
			{
			case SYSTEM_OPTION_CONTROL::CHECKBOX:	fAnchor = RIGHT_ANCHOR_CHECKBOX; break;
			case SYSTEM_OPTION_CONTROL::BUTTON:		fAnchor = RIGHT_ANCHOR_BUTTON; break;
			case SYSTEM_OPTION_CONTROL::SUBTITLE:	fAnchor = RIGHT_ANCHOR_SUBTITLE; break;
			case SYSTEM_OPTION_CONTROL::COLOR:		fAnchor = RIGHT_ANCHOR_COLOR; break;
			default:								break;
			}
			Mine = PLACED{ Parent.fX + pRow->fMarginX + fAnchor, Parent.fY + pRow->fMarginY, fHeight };
		}
		else
		{
			Mine = PLACED{ Parent.fX + pRow->fMarginX, Parent.fY + Parent.fHeight + pRow->fMarginY, fHeight };
		}
		Placed[pRow->iPrimaryKey] = Mine;
		ROW_LAYOUT Layout{};
		Layout.pRow = pRow;
		Layout.fX = Mine.fX;
		Layout.fY = Mine.fY;
		Layout.fHeight = fHeight;
		OutRows.push_back(std::move(Layout));
	}
}

void Client::CSystemOptionWindowView::Build_Screen(const SYSTEM_OPTION_TAB& Tab)
{
	vector<ROW_LAYOUT> Rows;
	Layout_Screen(Tab, Rows);
	f32_t fBottom = Pane_Rect().fY;
	for (ROW_LAYOUT& Row : Rows)
	{
		Ensure_RowSlots(Row);
		fBottom = (std::max)(fBottom, Row.fY + Row.fHeight);
	}
	m_ContentHeightByTab[Tab.iTabId] = fBottom - Pane_Rect().fY + 12.f;
	m_Screens[Tab.iTabId] = std::move(Rows);
}

void Client::CSystemOptionWindowView::Ensure_RowSlots(ROW_LAYOUT& Row)
{
	const SYSTEM_OPTION_ROW& R = *Row.pRow;
	const auto Add = [&](const string& strId, const char* pArt)
		{
			m_pView->Ensure_RuntimeSlot(strId, 0.f, 0.f, 1.f, 1.f, pArt);
			Row.SlotIds.push_back(strId);
		};
	switch (R.eControl)
	{
	case SYSTEM_OPTION_CONTROL::GROUP_TITLE:
		if (R.bFavourite)
			Add(Slot(R, "star"), ART_STAR_NORMAL);
		Add(Slot(R, "bullet"), ART_TITLE_BULLET);
		break;
	case SYSTEM_OPTION_CONTROL::SEPARATOR:
		Add(Slot(R, "sep"), ART_SEPARATOR);
		break;
	case SYSTEM_OPTION_CONTROL::BUTTON:
		Add(Slot(R, "btn"), ART_BTN_NORMAL);
		break;
	case SYSTEM_OPTION_CONTROL::COMBOBOX:
	case SYSTEM_OPTION_CONTROL::TEXT_INPUT:
		Add(Slot(R, "combo"), ART_COMBO_NORMAL);
		if (SYSTEM_OPTION_CONTROL::COMBOBOX == R.eControl)
			Add(Slot(R, "arrow"), ART_COMBOARROW_NORMAL);
		break;
	case SYSTEM_OPTION_CONTROL::SLIDER:
		Add(Slot(R, "track"), ART_TRACK);
		Add(Slot(R, "thumb"), ART_THUMB_NORMAL);
		break;
	case SYSTEM_OPTION_CONTROL::SPINNER:
		Add(Slot(R, "spinbg"), ART_SPINNER_BG);
		Add(Slot(R, "minus"), ART_MINUS_NORMAL);
		Add(Slot(R, "track"), ART_TRACK);
		Add(Slot(R, "thumb"), ART_THUMB_NORMAL);
		Add(Slot(R, "plus"), ART_PLUS_NORMAL);
		break;
	case SYSTEM_OPTION_CONTROL::CHECKBOX:
		Add(Slot(R, "check"), ART_CHECK_NORMAL);
		break;
	case SYSTEM_OPTION_CONTROL::COLOR:
		Add(Slot(R, "color"), ART_COLOR_FRAME);
		break;
	case SYSTEM_OPTION_CONTROL::SCALE_LIST:
	case SYSTEM_OPTION_CONTROL::COUNT_LIST:
	case SYSTEM_OPTION_CONTROL::RADIO_PAIR:
		for (int32_t i = 0; i < static_cast<int32_t>(R.Choices.size()); ++i)
			Add(Slot(R, "radio", i), ART_RADIO_NORMAL);
		break;
	default:
		break;
	}
	/* The speaker icon: retail's SliderIcon column marks the audio volume rows. */
	if (SYSTEM_OPTION_CONTROL::SLIDER == R.eControl && R.strId.rfind("slider_", 0) == 0 &&
		Row.pRow->strUnit == L"%" && R.strId != SystemOptionRowId::BRIGHTNESS &&
		R.strId != "slider_pointer_speed" && R.strId != "slider_pointer_accelerate" &&
		R.strId != "CursorVisibility")
	{
		Add(Slot(R, "icon"), ART_ICON_VOLUME);
	}
}

/* ---- per frame ----------------------------------------------------------------------- */

void Client::CSystemOptionWindowView::Update_Rows()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const auto found = m_Screens.find(m_iActiveTabId);
	if (m_Screens.end() == found)
		return;

	/* Switching screens: hide the previous screen's widgets once. */
	if (m_iShownTabId != m_iActiveTabId)
	{
		const auto previous = m_Screens.find(m_iShownTabId);
		if (m_Screens.end() != previous)
			for (const ROW_LAYOUT& Row : previous->second)
				for (const string& strId : Row.SlotIds)
					m_pView->Set_SlotVisible(strId, false);
		m_iShownTabId = m_iActiveTabId;
		m_iSliderDragKey = 0;
		m_iComboOpenKey = 0;
	}

	const SYSTEM_OPTION_TAB* pTab = m_Document.Find_Tab(m_iActiveTabId);
	const RECT_RETAIL Pane = Pane_Rect();
	const f32_t fScroll = m_ScrollByTab[m_iActiveTabId];
	const bool_t bPopup = 0 != m_iComboOpenKey;
	if (!Router.Is_LeftDown())
		m_iSliderDragKey = 0;
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bMouse = Mouse_Retail(fMouseX, fMouseY);
	const bool_t bMouseInPane = bMouse && fMouseX >= Pane.fX && fMouseX < Pane.fX + Pane.fWidth &&
		fMouseY >= Pane.fY && fMouseY < Pane.fY + Pane.fHeight;
	m_iHoveredRowKey = 0;

	for (ROW_LAYOUT& Row : found->second)
	{
		const SYSTEM_OPTION_ROW& R = *Row.pRow;
		/* A header row (the reset button beside the title) neither scrolls nor clips. */
		const bool_t bHeader = Is_HeaderRow(Row);
		const f32_t fX = Row.fX;
		const f32_t fY = Row.fY - (bHeader ? 0.f : fScroll);
		const f32_t fValue = Read_Value(R);
		const bool_t bRowHovered = !bPopup && bMouseInPane && !bHeader &&
			fMouseY >= fY && fMouseY < fY + Row.fHeight && fMouseX >= fX;
		/* A click only counts inside the pane; the pane edges clip the widgets themselves. */
		const auto Hovered = [&](f32_t x, f32_t y, f32_t w, f32_t h)
			{
				return !bPopup && (bHeader || bMouseInPane) && Is_Hovered(x, y, w, h);
			};
		const auto Clicked = [&](f32_t x, f32_t y, f32_t w, f32_t h)
			{
				return !bPopup && bMouseInPane && Is_Clicked(x, y, w, h);
			};

		switch (R.eControl)
		{
		case SYSTEM_OPTION_CONTROL::GROUP_TITLE:
		{
			if (R.bFavourite)
			{
				const string strStar = Slot(R, "star");
				Place_Slot(strStar, fX + GROUP_STAR_DX, fY + GROUP_STAR_DY, 26.f, 26.f, true);
				m_pView->Set_SlotTexture(strStar, Hovered(fX + GROUP_STAR_DX, fY + GROUP_STAR_DY, 26.f, 26.f) ?
					ART_STAR_OVER : ART_STAR_NORMAL);
			}
			Place_Slot(Slot(R, "bullet"), fX + GROUP_BULLET_DX, fY + GROUP_BULLET_DY, 3.f, 16.f, true);
			break;
		}
		case SYSTEM_OPTION_CONTROL::SEPARATOR:
			Place_Slot(Slot(R, "sep"), fX, fY, SEPARATOR_W, HEIGHT_SEPARATOR, true);
			break;
		case SYSTEM_OPTION_CONTROL::BUTTON:
		{
			const string strBtn = Slot(R, "btn");
			Place_Slot(strBtn, fX, fY, BUTTON_W, HEIGHT_BUTTON, !bHeader);
			const bool_t bHovered = Hovered(fX, fY, BUTTON_W, HEIGHT_BUTTON);
			Set_Button(strBtn, ART_BTN_NORMAL, ART_BTN_OVER, ART_BTN_DOWN, ART_BTN_DISABLED,
				bHovered ? (Router.Is_LeftDown() ? BUTTON_STATE::DOWN : BUTTON_STATE::OVER) : BUTTON_STATE::NORMAL);
			if (bHovered && Clicked(fX, fY, BUTTON_W, HEIGHT_BUTTON))
			{
				CMainApp::Play_UIButtonClickSound();
				/* reset resets the screen; auto settings has nothing to measure here. */
				if (nullptr != pTab && R.strTitle == Find_String("button.reset"))
					Reset_Screen(*pTab);
			}
			break;
		}
		case SYSTEM_OPTION_CONTROL::COMBOBOX:
		case SYSTEM_OPTION_CONTROL::TEXT_INPUT:
		{
			const f32_t fBoxX = fX + R.fMarginLength;
			const f32_t fBoxW = R.fWidth > 0.f ? R.fWidth : 240.f;
			const string strBox = Slot(R, "combo");
			Place_Slot(strBox, fBoxX, fY + 1.f, fBoxW, COMBO_H, true);
			if (SYSTEM_OPTION_CONTROL::TEXT_INPUT == R.eControl)
			{
				m_pView->Set_SlotTexture(strBox, ART_COMBO_NORMAL);
				break;
			}
			const string strArrow = Slot(R, "arrow");
			Place_Slot(strArrow, fBoxX + fBoxW - COMBO_ARROW_INSET, fY + 3.f, COMBO_ARROW, COMBO_ARROW, true);
			const bool_t bDisabled = Is_RetailDisabled(R);
			const bool_t bFixed = bDisabled || Is_FixedCombo(R) || R.Choices.empty();
			const bool_t bOpen = m_iComboOpenKey == R.iPrimaryKey;
			const bool_t bHovered = !bFixed && Hovered(fBoxX, fY + 1.f, fBoxW, COMBO_H);
			m_pView->Set_SlotTexture(strBox, bDisabled ? ART_COMBO_DISABLED :
				(bOpen ? ART_COMBO_DOWN : (bHovered ? ART_COMBO_OVER : ART_COMBO_NORMAL)));
			m_pView->Set_SlotTexture(strArrow, bDisabled ? ART_COMBOARROW_DISABLED :
				(bOpen ? ART_COMBOARROW_OPEN : (bHovered ? ART_COMBOARROW_OVER : ART_COMBOARROW_NORMAL)));
			if (bHovered && Clicked(fBoxX, fY + 1.f, fBoxW, COMBO_H))
			{
				CMainApp::Play_UIButtonClickSound();
				m_iComboOpenKey = R.iPrimaryKey;
				m_iComboHover = -1;
			}
			break;
		}
		case SYSTEM_OPTION_CONTROL::SLIDER:
		{
			const f32_t fTrackX = fX + SLIDER_DX;
			const f32_t fTrackY = fY + SLIDER_TRACK_DY;
			Place_Slot(Slot(R, "track"), fTrackX, fTrackY, SLIDER_TRACK_W, SLIDER_TRACK_H, true);
			const string strIcon = Slot(R, "icon");
			if (std::find(Row.SlotIds.begin(), Row.SlotIds.end(), strIcon) != Row.SlotIds.end())
				Place_Slot(strIcon, fX + SLIDER_ICON_DX, fY + 2.f, SLIDER_ICON_W, SLIDER_ICON_H, true);
			/* The grab band is the whole row over the track; an 8 px line is no target. */
			const f32_t fBandX = fTrackX - SLIDER_THUMB_W * 0.5f;
			const f32_t fBandW = SLIDER_TRACK_W + SLIDER_THUMB_W;
			const bool_t bHovered = Hovered(fBandX, fY, fBandW, HEIGHT_SLIDER + 6.f);
			if (bHovered && Clicked(fBandX, fY, fBandW, HEIGHT_SLIDER + 6.f))
				m_iSliderDragKey = R.iPrimaryKey;
			const bool_t bDragging = m_iSliderDragKey == R.iPrimaryKey;
			if (bDragging && bMouse && R.fMaximum > R.fMinimum)
			{
				const f32_t fTravel = SLIDER_TRACK_W - SLIDER_THUMB_W;
				const f32_t fFraction = std::clamp((fMouseX - (fTrackX + SLIDER_THUMB_W * 0.5f)) / fTravel, 0.f, 1.f);
				const f32_t fNew = Snap(R.fMinimum + fFraction * (R.fMaximum - R.fMinimum), R);
				if (fNew != fValue)
				{
					Write_Value(R, fNew);
					Commit_Draft(R);
				}
			}
			const f32_t fShown = Read_Value(R);
			const f32_t fFraction = R.fMaximum > R.fMinimum ?
				std::clamp((fShown - R.fMinimum) / (R.fMaximum - R.fMinimum), 0.f, 1.f) : 0.f;
			const string strThumb = Slot(R, "thumb");
			Place_Slot(strThumb, fTrackX + fFraction * (SLIDER_TRACK_W - SLIDER_THUMB_W),
				fY + SLIDER_THUMB_DY, SLIDER_THUMB_W, SLIDER_THUMB_H, true);
			m_pView->Set_SlotTexture(strThumb, bDragging ? ART_THUMB_DOWN : (bHovered ? ART_THUMB_OVER : ART_THUMB_NORMAL));
			break;
		}
		case SYSTEM_OPTION_CONTROL::SPINNER:
		{
			/* ARKSliderEx: inset bar at x-22, minus at x-20, track at x, plus at x+103, thumb
			at 42 (x0.647), all relative to the group's MarginLength. */
			const f32_t fBase = fX + R.fMarginLength;
			Place_Slot(Slot(R, "spinbg"), fBase - 22.f, fY, SPINNER_BG_W, SPINNER_BG_H, true);
			Place_Slot(Slot(R, "track"), fBase, fY + SPINNER_TRACK_DY, SLIDER_TRACK_W, SLIDER_TRACK_H, true);
			const string strMinus = Slot(R, "minus");
			const string strPlus = Slot(R, "plus");
			Place_Slot(strMinus, fBase - 20.f, fY + 3.f, SPINNER_BTN, SPINNER_BTN, true);
			Place_Slot(strPlus, fBase + 103.f, fY + 3.f, SPINNER_BTN, SPINNER_BTN, true);
			const bool_t bMinusHovered = Hovered(fBase - 20.f, fY + 3.f, SPINNER_BTN, SPINNER_BTN);
			const bool_t bPlusHovered = Hovered(fBase + 103.f, fY + 3.f, SPINNER_BTN, SPINNER_BTN);
			Set_Button(strMinus, ART_MINUS_NORMAL, ART_MINUS_OVER, ART_MINUS_DOWN, ART_MINUS_NORMAL,
				bMinusHovered ? (Router.Is_LeftDown() ? BUTTON_STATE::DOWN : BUTTON_STATE::OVER) : BUTTON_STATE::NORMAL);
			Set_Button(strPlus, ART_PLUS_NORMAL, ART_PLUS_OVER, ART_PLUS_DOWN, ART_PLUS_NORMAL,
				bPlusHovered ? (Router.Is_LeftDown() ? BUTTON_STATE::DOWN : BUTTON_STATE::OVER) : BUTTON_STATE::NORMAL);
			f32_t fNew = fValue;
			if (bMinusHovered && Clicked(fBase - 20.f, fY + 3.f, SPINNER_BTN, SPINNER_BTN))
				fNew = Snap(fValue - SPINNER_STEP, R);
			if (bPlusHovered && Clicked(fBase + 103.f, fY + 3.f, SPINNER_BTN, SPINNER_BTN))
				fNew = Snap(fValue + SPINNER_STEP, R);
			const f32_t fBandY = fY + 3.f;
			const bool_t bTrackHovered = Hovered(fBase, fBandY, SLIDER_TRACK_W, SPINNER_BTN);
			if (bTrackHovered && Clicked(fBase, fBandY, SLIDER_TRACK_W, SPINNER_BTN))
				m_iSliderDragKey = R.iPrimaryKey;
			const bool_t bDragging = m_iSliderDragKey == R.iPrimaryKey;
			if (bDragging && bMouse && R.fMaximum > R.fMinimum)
			{
				const f32_t fTravel = SLIDER_TRACK_W - SPINNER_THUMB_W;
				const f32_t fFraction = std::clamp((fMouseX - (fBase + SPINNER_THUMB_W * 0.5f)) / fTravel, 0.f, 1.f);
				fNew = Snap(R.fMinimum + fFraction * (R.fMaximum - R.fMinimum), R);
			}
			if (fNew != fValue)
			{
				Write_Value(R, fNew);
				Commit_Draft(R);
			}
			const f32_t fShown = Read_Value(R);
			const f32_t fFraction = R.fMaximum > R.fMinimum ?
				std::clamp((fShown - R.fMinimum) / (R.fMaximum - R.fMinimum), 0.f, 1.f) : 0.f;
			const string strThumb = Slot(R, "thumb");
			Place_Slot(strThumb, fBase + fFraction * (SLIDER_TRACK_W - SPINNER_THUMB_W), fY + 4.f,
				SPINNER_THUMB_W, SPINNER_THUMB_H, true);
			m_pView->Set_SlotTexture(strThumb, bDragging ? ART_THUMB_DOWN : (bTrackHovered ? ART_THUMB_OVER : ART_THUMB_NORMAL));
			break;
		}
		case SYSTEM_OPTION_CONTROL::CHECKBOX:
		{
			const f32_t fBoxX = fX + R.fMarginLength + CHECK_BOX_DX;
			const f32_t fBoxY = fY + CHECK_BOX_DY;
			const string strBox = Slot(R, "check");
			Place_Slot(strBox, fBoxX, fBoxY, CHECK_W, CHECK_H, true);
			const bool_t bOn = fValue != 0.f;
			const bool_t bHovered = Hovered(fBoxX, fBoxY, CHECK_W, CHECK_H);
			m_pView->Set_SlotTexture(strBox, bOn ? ART_CHECK_SELECTED : (bHovered ? ART_CHECK_OVER : ART_CHECK_NORMAL));
			if (bHovered && Clicked(fBoxX, fBoxY, CHECK_W, CHECK_H))
			{
				CMainApp::Play_UIButtonClickSound();
				Write_Value(R, bOn ? 0.f : 1.f);
				Commit_Draft(R);
			}
			break;
		}
		case SYSTEM_OPTION_CONTROL::COLOR:
			Place_Slot(Slot(R, "color"), fX, fY + 3.f, COLOR_FRAME, COLOR_FRAME, true);
			break;
		case SYSTEM_OPTION_CONTROL::SCALE_LIST:
		case SYSTEM_OPTION_CONTROL::COUNT_LIST:
		case SYSTEM_OPTION_CONTROL::RADIO_PAIR:
		{
			const f32_t fBaseX = fX + R.fMarginLength;
			const int32_t iSelected = static_cast<int32_t>(std::lround(fValue));
			for (int32_t i = 0; i < static_cast<int32_t>(R.Choices.size()); ++i)
			{
				const f32_t fCellX = fBaseX + static_cast<f32_t>(i % RADIO_COLUMNS) * RADIO_PITCH_X;
				const f32_t fCellY = fY + static_cast<f32_t>(i / RADIO_COLUMNS) * RADIO_PITCH_Y;
				const f32_t fRadioX = fCellX + RADIO_TEXT_TO_BUTTON;
				const string strRadio = Slot(R, "radio", i);
				Place_Slot(strRadio, fRadioX, fCellY + 2.f, RADIO_W, RADIO_W, true);
				const bool_t bHovered = Hovered(fCellX, fCellY, RADIO_PITCH_X, RADIO_PITCH_Y - 4.f);
				m_pView->Set_SlotTexture(strRadio, i == iSelected ? ART_RADIO_SELECTED :
					(bHovered ? ART_RADIO_OVER : ART_RADIO_NORMAL));
				if (bHovered && Clicked(fCellX, fCellY, RADIO_PITCH_X, RADIO_PITCH_Y - 4.f) && i != iSelected)
				{
					CMainApp::Play_UIButtonClickSound();
					Write_Value(R, static_cast<f32_t>(i));
					Commit_Draft(R);
				}
			}
			break;
		}
		default:
			break;
		}

		if (bRowHovered && SYSTEM_OPTION_CONTROL::SEPARATOR != R.eControl &&
			SYSTEM_OPTION_CONTROL::GROUP_TITLE != R.eControl && SYSTEM_OPTION_CONTROL::SUBTITLE != R.eControl)
		{
			m_iHoveredRowKey = R.iPrimaryKey;
		}
	}

	/* SystemOptionHighlightMc: the band retail draws behind the hovered editable row. */
	const ROW_LAYOUT* pHovered = nullptr;
	for (const ROW_LAYOUT& Row : found->second)
		if (Row.pRow->iPrimaryKey == m_iHoveredRowKey)
			pHovered = &Row;
	if (nullptr != pHovered && 2 != pHovered->pRow->iDirection)
		Place_Slot("SO_HL", Pane.fX + 4.f, pHovered->fY - fScroll - 2.f, Pane.fWidth - 8.f, HIGHLIGHT_H, true);
	else
		m_pView->Set_SlotVisible("SO_HL", false);
}

void Client::CSystemOptionWindowView::Update_ComboPopup()
{
	const auto found = m_Screens.find(m_iActiveTabId);
	if (m_Screens.end() == found)
	{
		m_iComboOpenKey = 0;
		return;
	}
	const ROW_LAYOUT* pOpen = nullptr;
	for (const ROW_LAYOUT& Row : found->second)
		if (Row.pRow->iPrimaryKey == m_iComboOpenKey)
			pOpen = &Row;
	if (nullptr == pOpen)
	{
		m_iComboOpenKey = 0;
		return;
	}
	const SYSTEM_OPTION_ROW& R = *pOpen->pRow;
	const f32_t fScroll = m_ScrollByTab[m_iActiveTabId];
	const f32_t fBoxX = pOpen->fX + R.fMarginLength;
	const f32_t fBoxW = R.fWidth > 0.f ? R.fWidth : 240.f;
	const f32_t fListY = pOpen->fY - fScroll + 1.f + COMBO_H;
	const int32_t iCount = (std::min)(MAX_COMBO_CHOICES, static_cast<int32_t>(R.Choices.size()));
	const int32_t iSelected = static_cast<int32_t>(std::lround(Read_Value(R)));
	CUIInputRouter& Router = CUIInputRouter::Get();
	m_iComboHover = -1;
	bool_t bAnyHovered = false;
	/* The list's own opaque panel (DefaultEFDropdownList_V2) under the rows, so the rows it
	opens over do not show through their 31% alpha art. */
	Place_Slot(m_strPopupBgSlotId, fBoxX, fListY, fBoxW, static_cast<f32_t>(iCount) * COMBO_ROW_H, false);
	for (int32_t i = 0; i < MAX_COMBO_CHOICES; ++i)
	{
		const string& strId = m_PopupSlotIds[static_cast<size_t>(i)];
		if (i >= iCount)
		{
			m_pView->Set_SlotVisible(strId, false);
			continue;
		}
		const f32_t fRowY = fListY + static_cast<f32_t>(i) * COMBO_ROW_H;
		Place_Slot(strId, fBoxX, fRowY, fBoxW, COMBO_ROW_H, false);
		const bool_t bHovered = Is_Hovered(fBoxX, fRowY, fBoxW, COMBO_ROW_H);
		bAnyHovered |= bHovered;
		if (bHovered)
			m_iComboHover = i;
		m_pView->Set_SlotTexture(strId, i == iSelected ? ART_LISTROW_SELECTED :
			(bHovered ? ART_LISTROW_OVER : ART_LISTROW_NORMAL));
		if (bHovered && Is_Clicked(fBoxX, fRowY, fBoxW, COMBO_ROW_H))
		{
			CMainApp::Play_UIButtonClickSound();
			if (i != iSelected)
			{
				Write_Value(R, static_cast<f32_t>(i));
				Commit_Draft(R);
			}
			m_iComboOpenKey = 0;
			return;
		}
	}
	/* A click anywhere else closes the list and is consumed. */
	if (!bAnyHovered && Router.Is_LeftClickEdge())
	{
		Router.Claim_Mouse_This_Frame();
		m_iComboOpenKey = 0;
	}
}

/* ---- text ------------------------------------------------------------------------------ */

void Client::CSystemOptionWindowView::Render_RowText(const ROW_LAYOUT& Row, const f32_t fOffsetY)
{
	const SYSTEM_OPTION_ROW& R = *Row.pRow;
	const f32_t fX = Row.fX;
	const f32_t fY = Row.fY + fOffsetY;
	const float2_t vTopLeft(0.f, 0.f);
	const float2_t vTopRight(1.f, 0.f);
	const float2_t vCenter(0.5f, 0.5f);
	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	const f32_t fValue = Read_Value(R);

	switch (R.eControl)
	{
	case SYSTEM_OPTION_CONTROL::GROUP_TITLE:
		Draw_Label(FONT_YOON, R.strTitle, fX + GROUP_TEXT_DX, fY + GROUP_TEXT_DY, TITLE_ROW_PX, COLOR_SUB, vTopLeft);
		break;
	case SYSTEM_OPTION_CONTROL::SUBTITLE:
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 2.f, ROW_FONT_PX, COLOR_SUB, vTopLeft);
		break;
	case SYSTEM_OPTION_CONTROL::BUTTON:
		Draw_Label(FONT_YG760, R.strTitle, fX + BUTTON_W * 0.5f, fY + HEIGHT_BUTTON * 0.5f, ROW_FONT_PX,
			COLOR_LABEL, vCenter);
		break;
	case SYSTEM_OPTION_CONTROL::COMBOBOX:
	{
		const fvector_t vColor = Is_RetailDisabled(R) ? COLOR_DISABLED : COLOR_LABEL;
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 6.f, ROW_FONT_PX, vColor, vTopLeft);
		wstring strShown;
		if (R.strId == "combobox_resolution")
			strShown = L"1280x720 (16:9)";
		else if (R.strId == "combobox_monitor" && R.Choices.size() > 1)
			strShown = R.Choices[1].strText;
		else
		{
			const int32_t iChoice = static_cast<int32_t>(std::lround(fValue));
			if (iChoice >= 0 && iChoice < static_cast<int32_t>(R.Choices.size()))
				strShown = R.Choices[static_cast<size_t>(iChoice)].strText;
		}
		Draw_Label(FONT_YG760, strShown, fX + R.fMarginLength + COMBO_TEXT_DX, fY + COMBO_TEXT_DY,
			ROW_FONT_PX, vColor, vTopLeft);
		break;
	}
	case SYSTEM_OPTION_CONTROL::TEXT_INPUT:
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 6.f, ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		break;
	case SYSTEM_OPTION_CONTROL::SLIDER:
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 4.f, ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		if (R.bShowSliderValue)
			Draw_Label(FONT_YG760, Format_Value(fValue, R), fX + SLIDER_VALUE_RIGHT, fY + 4.f,
				ROW_FONT_PX, COLOR_VALUE, vTopRight);
		break;
	case SYSTEM_OPTION_CONTROL::SPINNER:
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 6.f, ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		Draw_Label(FONT_YG760, Format_Value(fValue, R), fX + R.fMarginLength + SPINNER_VALUE_DX, fY + 6.f,
			ROW_FONT_PX, COLOR_VALUE, vTopLeft);
		break;
	case SYSTEM_OPTION_CONTROL::CHECKBOX:
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 4.f, ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		break;
	case SYSTEM_OPTION_CONTROL::COLOR:
		Draw_Label(FONT_YG760, R.strTitle, fX + 22.f, fY + 2.f, ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		break;
	case SYSTEM_OPTION_CONTROL::SCALE_LIST:
	case SYSTEM_OPTION_CONTROL::COUNT_LIST:
	case SYSTEM_OPTION_CONTROL::RADIO_PAIR:
	{
		Draw_Label(FONT_YG760, R.strTitle, fX + 2.f, fY + 4.f, ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		const f32_t fBaseX = fX + R.fMarginLength;
		for (int32_t i = 0; i < static_cast<int32_t>(R.Choices.size()); ++i)
		{
			const f32_t fCellX = fBaseX + static_cast<f32_t>(i % RADIO_COLUMNS) * RADIO_PITCH_X;
			const f32_t fCellY = fY + static_cast<f32_t>(i / RADIO_COLUMNS) * RADIO_PITCH_Y;
			Draw_Label(FONT_YG760, R.Choices[static_cast<size_t>(i)].strText, fCellX, fCellY + 4.f,
				ROW_FONT_PX, COLOR_LABEL, vTopLeft);
		}
		break;
	}
	default:
		break;
	}
}

bool_t Client::CSystemOptionWindowView::Get_ComboPopupRect(
	f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	if (0 == m_iComboOpenKey)
		return false;
	const auto found = m_Screens.find(m_iActiveTabId);
	if (m_Screens.end() == found)
		return false;
	for (const ROW_LAYOUT& Row : found->second)
	{
		if (Row.pRow->iPrimaryKey != m_iComboOpenKey)
			continue;
		const SYSTEM_OPTION_ROW& R = *Row.pRow;
		const auto scroll = m_ScrollByTab.find(m_iActiveTabId);
		const f32_t fScroll = m_ScrollByTab.end() == scroll ? 0.f : scroll->second;
		const int32_t iCount = (std::min)(MAX_COMBO_CHOICES, static_cast<int32_t>(R.Choices.size()));
		fX = Row.fX + R.fMarginLength;
		fY = Row.fY - fScroll + 1.f + COMBO_H;
		fWidth = R.fWidth > 0.f ? R.fWidth : 240.f;
		fHeight = static_cast<f32_t>(iCount) * COMBO_ROW_H;
		return iCount > 0;
	}
	return false;
}

void Client::CSystemOptionWindowView::Render_ComboPopupText()
{
	const auto found = m_Screens.find(m_iActiveTabId);
	if (m_Screens.end() == found)
		return;
	for (const ROW_LAYOUT& Row : found->second)
	{
		if (Row.pRow->iPrimaryKey != m_iComboOpenKey)
			continue;
		const SYSTEM_OPTION_ROW& R = *Row.pRow;
		const f32_t fScroll = m_ScrollByTab[m_iActiveTabId];
		const f32_t fBoxX = Row.fX + R.fMarginLength;
		const f32_t fListY = Row.fY - fScroll + 1.f + COMBO_H;
		const int32_t iCount = (std::min)(MAX_COMBO_CHOICES, static_cast<int32_t>(R.Choices.size()));
		const float2_t vTopLeft(0.f, 0.f);
		for (int32_t i = 0; i < iCount; ++i)
		{
			Draw_Label(TEXT("Font_YG760"), R.Choices[static_cast<size_t>(i)].strText,
				fBoxX + COMBO_TEXT_DX, fListY + static_cast<f32_t>(i) * COMBO_ROW_H + 9.f,
				ROW_FONT_PX, i == m_iComboHover ? COLOR_TAB_ON : COLOR_LABEL, vTopLeft);
		}
		return;
	}
}
