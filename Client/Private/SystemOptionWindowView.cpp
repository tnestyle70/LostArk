/* WinSock2 then dinput, ahead of everything else -- the order VehicleWindowView.cpp already
uses for the DIK_* constants (dinput.h drags in windows.h/winsock.h and declares POINT, which
Engine_Enum.h's Engine::POINT would make ambiguous). */
#include <WinSock2.h>
#include <dinput.h>

#include "SystemOptionWindowView.h"
#include "SystemOptionWindowView_Internal.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

using namespace Client::SystemOptionGeometry;

namespace
{
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

Client::CSystemOptionWindowView::CSystemOptionWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/SystemOption/SystemOption_Layout.json") }
{
	m_ChromeSlotIds = m_pView->Get_SlotIds();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("SO_WinBg", fX, fY, fWidth, fHeight) && fWidth > 0.f)
		m_fRetailScale = fWidth / WINDOW_WIDTH;
	Load_Strings();

	string strStatus;
	if (!m_Document.Load(strStatus))
		OutputDebugStringA(("[SystemOption] " + strStatus + "\n").c_str());
	/* The retail table's defaults are the fallback for every row the user never touched. */
	for (const SYSTEM_OPTION_TAB& Tab : m_Document.Get_Tabs())
	{
		for (const SYSTEM_OPTION_ROW& Row : Tab.Rows)
		{
			if (SYSTEM_OPTION_CONTROL::GROUP_TITLE != Row.eControl &&
				SYSTEM_OPTION_CONTROL::SEPARATOR != Row.eControl &&
				SYSTEM_OPTION_CONTROL::SUBTITLE != Row.eControl &&
				SYSTEM_OPTION_CONTROL::BUTTON != Row.eControl)
			{
				CUserSettings::Get().Set_Default(Row.strId, Effective_Default(Row));
			}
		}
	}

	/* Draw order is creation order: the hover highlight first (under everything), the tab
	column, then every screen's widgets (hidden until visited), and the drop list last so it
	covers whatever row it opens over. */
	m_pView->Ensure_RuntimeSlot("SO_HL", 0.f, 0.f, 1.f, 1.f, ART_ROW_HIGHLIGHT);
	Build_TabColumn();
	for (const SYSTEM_OPTION_TAB& Tab : m_Document.Get_Tabs())
		Build_Screen(Tab);
	m_strPopupBgSlotId = "SO_POP_BG";
	m_pView->Ensure_RuntimeSlot(m_strPopupBgSlotId, 0.f, 0.f, 1.f, 1.f, ART_LIST_BG);
	for (int32_t i = 0; i < MAX_COMBO_CHOICES; ++i)
	{
		const string strId = "SO_POP_" + std::to_string(i);
		m_pView->Ensure_RuntimeSlot(strId, 0.f, 0.f, 1.f, 1.f, ART_LISTROW_NORMAL);
		m_PopupSlotIds.push_back(strId);
	}
	if (!m_Document.Get_Tabs().empty())
		m_iActiveTabId = m_Document.Get_Tabs().front().iTabId;
	/* Same reason as CVehicleWindowView: LEVEL::STATIC sprites are visible from construction. */
	Hide();
}

Client::CSystemOptionWindowView::~CSystemOptionWindowView() = default;

void Client::CSystemOptionWindowView::Load_Strings()
{
	m_Strings.clear();
	const filesystem::path DataPath =
		CProjectDataRoot::Resolve(L"UI/SystemOption/SystemOptionStrings.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[SystemOption] SystemOptionStrings.json missing -- labels stay empty.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[SystemOption] SystemOptionStrings.json parse failed: " +
			Error + "\n").c_str());
		return;
	}
	const DATA_JSON_VALUE* pStrings = Root.Find("strings");
	if (nullptr == pStrings || !pStrings->Is_Object())
		return;
	for (const auto& [strKey, Value] : pStrings->Get_Object())
	{
		if (!Value.Is_String())
			continue;
		wstring strWide;
		if (ConvertUtf8ToWide(Value.Get_String(), strWide))
			m_Strings.emplace(strKey, std::move(strWide));
	}
}

const wstring& Client::CSystemOptionWindowView::Find_String(const char* pKey) const
{
	static const wstring s_Empty;
	const auto found = m_Strings.find(pKey);
	return m_Strings.end() == found ? s_Empty : found->second;
}

/* ---- tab column ------------------------------------------------------------------- */

void Client::CSystemOptionWindowView::Build_TabColumn()
{
	m_TabColumn.clear();
	string strLastParent;
	int32_t iIndex = 0;
	const auto Push = [&](TAB_ENTRY Entry)
		{
			Entry.strSlotId = "SO_TAB_" + std::to_string(iIndex);
			m_pView->Ensure_RuntimeSlot(Entry.strSlotId, 0.f, 0.f, 1.f, 1.f, ART_TREE_NORMAL);
			if (Entry.bParent)
			{
				Entry.strArrowSlotId = "SO_TABARROW_" + std::to_string(iIndex);
				m_pView->Ensure_RuntimeSlot(Entry.strArrowSlotId, 0.f, 0.f, 1.f, 1.f, ART_TREEARROW_CLOSED);
			}
			m_TabColumn.push_back(std::move(Entry));
			++iIndex;
		};
	/* A parent with a single screen of the same name (accessibility) is that screen: retail
	shows one plain tab there, not a fold with one child. */
	map<string, int32_t> ChildCount;
	for (const SYSTEM_OPTION_TAB& Tab : m_Document.Get_Tabs())
		if (!Tab.strParentId.empty())
			++ChildCount[Tab.strParentId];
	for (const SYSTEM_OPTION_TAB& Tab : m_Document.Get_Tabs())
	{
		const bool_t bLeaf = Tab.strParentId.empty() ||
			(1 == ChildCount[Tab.strParentId] && Tab.strParentLabel == Tab.strTitle);
		if (!bLeaf && Tab.strParentId != strLastParent)
		{
			strLastParent = Tab.strParentId;
			TAB_ENTRY Parent{};
			Parent.bParent = true;
			Parent.strParentId = Tab.strParentId;
			Parent.strLabel = Tab.strParentLabel;
			Push(std::move(Parent));
		}
		TAB_ENTRY Entry{};
		Entry.pTab = &Tab;
		Entry.strParentId = bLeaf ? string() : Tab.strParentId;
		Entry.strLabel = Tab.strTitle;
		Push(std::move(Entry));
	}
	/* Retail keeps the tab sets this project leaves out visible and folded. */
	for (const char* pKey : { "tab.community", "tab.hotkey", "tab.controller" })
	{
		TAB_ENTRY Parent{};
		Parent.bParent = true;
		Parent.bExcluded = true;
		Parent.strParentId = pKey;
		Parent.strLabel = Find_String(pKey);
		Push(std::move(Parent));
	}
}

void Client::CSystemOptionWindowView::Update_TabColumn()
{
	f32_t fY = TAB_Y;
	for (TAB_ENTRY& Entry : m_TabColumn)
	{
		const bool_t bChild = !Entry.bParent && !Entry.strParentId.empty();
		if (bChild && Entry.strParentId != m_strOpenParentId)
		{
			m_pView->Set_SlotVisible(Entry.strSlotId, false);
			continue;
		}
		const bool_t bActive = !Entry.bParent && nullptr != Entry.pTab && Entry.pTab->iTabId == m_iActiveTabId;
		const bool_t bOpenParent = Entry.bParent && Entry.strParentId == m_strOpenParentId;
		const bool_t bHovered = 0 == m_iComboOpenKey && Is_Hovered(TAB_X, fY, TAB_W, TAB_ROW_H);
		const char* pArt = bChild ?
			(bActive ? ART_LISTROW_SELECTED : (bHovered ? ART_LISTROW_OVER : ART_LISTROW_NORMAL)) :
			(bActive || bOpenParent ? ART_TREE_SELECTED : (bHovered ? ART_TREE_OVER : ART_TREE_NORMAL));
		Place_Slot(Entry.strSlotId, TAB_X, fY, TAB_W, TAB_ROW_H, false);
		m_pView->Set_SlotTexture(Entry.strSlotId, pArt);
		if (Entry.bParent)
		{
			Place_Slot(Entry.strArrowSlotId, TAB_X + TAB_W - 30.f, fY + 10.f, 20.f, 16.f, false);
			m_pView->Set_SlotTexture(Entry.strArrowSlotId, bOpenParent ? ART_TREEARROW_OPEN : ART_TREEARROW_CLOSED);
		}
		if (bHovered && Is_Clicked(TAB_X, fY, TAB_W, TAB_ROW_H))
		{
			CMainApp::Play_UIButtonClickSound();
			if (Entry.bParent)
			{
				if (!Entry.bExcluded)
					m_strOpenParentId = m_strOpenParentId == Entry.strParentId ? string() : Entry.strParentId;
			}
			else if (nullptr != Entry.pTab)
			{
				m_iActiveTabId = Entry.pTab->iTabId;
				m_strOpenParentId = Entry.pTab->strParentId;
				m_iSliderDragKey = 0;
			}
		}
		fY += TAB_PITCH;
	}
}

/* ---- open / close / draft --------------------------------------------------------- */

void Client::CSystemOptionWindowView::Open()
{
	if (m_bOpen)
		return;
	m_Snapshot = CUserSettings::Get().Get_Settings();
	m_Draft = m_Snapshot;
	if (!m_Document.Get_Tabs().empty())
		m_iActiveTabId = m_Document.Get_Tabs().front().iTabId;
	m_strOpenParentId.clear();
	m_iSliderDragKey = 0;
	m_iComboOpenKey = 0;
	m_bScrollDragging = false;
	m_bOpen = true;
}

void Client::CSystemOptionWindowView::Handle_EscapeEdge()
{
	if (!m_bOpen)
		return;
	if (0 != m_iComboOpenKey)
		m_iComboOpenKey = 0;
	else
		Close();
}

bool_t Client::CSystemOptionWindowView::Get_ScreenRect(
	f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	if (!m_bOpen || nullptr == m_pView)
		return false;
	f32_t fRefX = 0.f, fRefY = 0.f, fRefWidth = 0.f, fRefHeight = 0.f;
	if (!m_pView->Get_SlotRect("SO_WinBg", fRefX, fRefY, fRefWidth, fRefHeight))
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

void Client::CSystemOptionWindowView::Close()
{
	if (!m_bOpen)
		return;
	m_bOpen = false;
	m_iSliderDragKey = 0;
	m_iComboOpenKey = 0;
	m_bScrollDragging = false;
	m_Drag.Reset();
	/* Closing without Confirm is a cancel: put back exactly what was live on open. */
	if (!m_Draft.Has_SameValues(m_Snapshot))
	{
		m_Draft = m_Snapshot;
		string strStatus;
		if (CUserSettings::Get().Commit(m_Draft, strStatus))
			m_bVideoDirty = true;
	}
	Hide();
}

void Client::CSystemOptionWindowView::Toggle()
{
	if (m_bOpen)
		Close();
	else
		Open();
}

bool_t Client::CSystemOptionWindowView::Take_VideoDirty()
{
	const bool_t bDirty = m_bVideoDirty;
	m_bVideoDirty = false;
	return bDirty;
}

f32_t Client::CSystemOptionWindowView::Read_Value(const SYSTEM_OPTION_ROW& Row) const
{
	return m_Draft.Get(Row.strId, Row.fDefault);
}

void Client::CSystemOptionWindowView::Write_Value(const SYSTEM_OPTION_ROW& Row, const f32_t fValue)
{
	m_Draft.Values[Row.strId] = fValue;
}

bool_t Client::CSystemOptionWindowView::Is_VideoRow(const SYSTEM_OPTION_ROW& Row)
{
	return Row.strId == SystemOptionRowId::BRIGHTNESS || Row.strId == SystemOptionRowId::BLOOM ||
		Row.strId == SystemOptionRowId::ANTIALIASING || Row.strId == SystemOptionRowId::SSAO;
}

f32_t Client::CSystemOptionWindowView::Effective_Default(const SYSTEM_OPTION_ROW& Row)
{
	if (SYSTEM_OPTION_CONTROL::COMBOBOX != Row.eControl || Row.fDefault >= 0.f)
		return Row.fDefault;
	/* -1 in EFTable_SystemOption: cursor size follows the resolution (48 px normal for this
	1280x720 client), battle font size is the 100% entry, keyboard language the Korean one. */
	if (Row.strId == SystemOptionRowId::BATTLE_FONT_SIZE)
		return 1.f;
	if (Row.strId == "combobox_keyboard_language")
		return 2.f;
	return 0.f;
}

void Client::CSystemOptionWindowView::Commit_Draft(const SYSTEM_OPTION_ROW& Row)
{
	/* Retail's graphics preset row: a preset writes the quality rows it owns; touching one of
	those rows by hand turns the preset into "custom" (the last choice). */
	const bool_t bQualityRow = Row.strId == SystemOptionRowId::ANTIALIASING ||
		Row.strId == SystemOptionRowId::SSAO || Row.strId == SystemOptionRowId::BLOOM;
	if (Row.strId == SystemOptionRowId::GRAPHICS_PRESET)
	{
		const int32_t iPreset = static_cast<int32_t>(std::lround(Read_Value(Row)));
		if (iPreset >= 0 && iPreset <= 3)
		{
			/* best / high / medium / low -> anti-aliasing and SSAO high/high/low/off, bloom on
			except low. The renderer has those on/off, so high and low both mean "on". */
			m_Draft.Values[SystemOptionRowId::ANTIALIASING] = iPreset < 3 ? (iPreset < 2 ? 0.f : 1.f) : 2.f;
			m_Draft.Values[SystemOptionRowId::SSAO] = iPreset < 3 ? (iPreset < 2 ? 0.f : 1.f) : 2.f;
			m_Draft.Values[SystemOptionRowId::BLOOM] = iPreset < 3 ? 1.f : 0.f;
			m_bVideoDirty = true;
		}
	}
	else if (bQualityRow && 0 != m_Draft.Values.count(SystemOptionRowId::GRAPHICS_PRESET))
	{
		m_Draft.Values[SystemOptionRowId::GRAPHICS_PRESET] = 4.f;
	}
	string strStatus;
	if (!CUserSettings::Get().Commit(m_Draft, strStatus))
	{
		OutputDebugStringA(("[SystemOption] " + strStatus + "\n").c_str());
		m_Draft = CUserSettings::Get().Get_Settings();
		return;
	}
	if (Is_VideoRow(Row))
		m_bVideoDirty = true;
}

void Client::CSystemOptionWindowView::Reset_Screen(const SYSTEM_OPTION_TAB& Tab)
{
	bool_t bVideo = false;
	for (const SYSTEM_OPTION_ROW& Row : Tab.Rows)
	{
		if (Row.strId.empty() || SYSTEM_OPTION_CONTROL::BUTTON == Row.eControl)
			continue;
		if (0 != m_Draft.Values.count(Row.strId))
			m_Draft.Values[Row.strId] = Effective_Default(Row);
		bVideo |= Is_VideoRow(Row);
	}
	string strStatus;
	if (CUserSettings::Get().Commit(m_Draft, strStatus) && bVideo)
		m_bVideoDirty = true;
}

void Client::CSystemOptionWindowView::Reset_All()
{
	for (const SYSTEM_OPTION_TAB& Tab : m_Document.Get_Tabs())
		Reset_Screen(Tab);
}

void Client::CSystemOptionWindowView::Save_And_Close()
{
	/* Confirm keeps what is live for the rest of this run; nothing is written to disk. */
	m_Snapshot = m_Draft;
	m_bOpen = false;
	m_iSliderDragKey = 0;
	m_iComboOpenKey = 0;
	m_bScrollDragging = false;
	m_Drag.Reset();
	Hide();
}

/* ---- per frame ---------------------------------------------------------------------- */

void Client::CSystemOptionWindowView::Update(const f32_t fTimeDelta)
{
	(void)fTimeDelta;
	if (!m_bOpen)
	{
		if (-1 != m_iShownTabId)
			Hide();
		m_bDragging = false;
		m_Drag.Reset();
		return;
	}

	for (const string& strId : m_ChromeSlotIds)
		m_pView->Set_SlotVisible(strId, true);

	/* Header-band drag first. The chrome slots follow the mouse inside CUIWindowDrag; every
	runtime slot is re-placed from the moved window origin below, the same frame, so the
	widgets never trail the frame. While the drag is live Is_Hovered / Is_Clicked answer
	false, which is what keeps a drag from turning into clicks. */
	m_bDragging = m_Drag.Update(*m_pView, m_ChromeSlotIds, Ref_X(0.f), Ref_Y(0.f),
		WINDOW_WIDTH * m_fRetailScale, TITLE_BAR_HEIGHT * m_fRetailScale, "SO_Close");
	Update_Chrome();
	/* An open drop list eats every click of its frame -- the one on a row, the one that
	closes it, the one on the combo's own arrow -- so nothing underneath (the same combo,
	another row, a button) can act on that click as well. The list itself is updated first:
	its row hit test goes through the same Is_Clicked wrapper, so the flag must not be up yet
	when the list looks at the click. */
	m_bPopupAteClick = false;
	if (0 != m_iComboOpenKey)
	{
		const bool_t bClickEdge = CUIInputRouter::Get().Is_LeftClickEdge();
		Update_ComboPopup();
		if (bClickEdge)
		{
			CUIInputRouter::Get().Claim_Mouse_This_Frame();
			m_bPopupAteClick = true;
		}
	}
	Update_TabColumn();
	Update_Scroll();
	Update_Rows();
	Update_Buttons();
	if (0 == m_iComboOpenKey)
	{
		for (const string& strId : m_PopupSlotIds)
			m_pView->Set_SlotVisible(strId, false);
		m_pView->Set_SlotVisible(m_strPopupBgSlotId, false);
	}

	/* Anything over the window belongs to the window. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_bOpen && m_pView->Get_SlotRect("SO_WinBg", fX, fY, fWidth, fHeight) &&
		Router.Is_Hovered(fX, fY, fWidth, fHeight,
			m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
	{
		Router.Claim_Mouse_This_Frame();
	}
}

void Client::CSystemOptionWindowView::Update_Chrome()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("SO_Close", fX, fY, fWidth, fHeight))
		return;
	const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("SO_Close", bHovered ? ART_CLOSE_OVER : ART_CLOSE_NORMAL);
	if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		Close();
	}
}

void Client::CSystemOptionWindowView::Update_Scroll()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const RECT_RETAIL Pane = Pane_Rect();
	m_fContentHeight = m_ContentHeightByTab[m_iActiveTabId];
	f32_t& fScroll = m_ScrollByTab[m_iActiveTabId];
	const f32_t fMaxScroll = (std::max)(0.f, m_fContentHeight - Pane.fHeight);

	/* Wheel over the pane. */
	if (0 == m_iComboOpenKey && Is_Hovered(Pane.fX, Pane.fY, Pane.fWidth, Pane.fHeight))
		fScroll -= static_cast<f32_t>(Router.Get_MouseWheelNotches()) * SCROLL_WHEEL_STEP;

	/* Thumb: proportional to the visible fraction, dragged with the mouse. The bar is the
	retail one on the whole pane (DefaultScrollBar_V2 at content (845,14)), so the thumb
	travels between the two arrows of that bar, not inside the row band under the heading. */
	const f32_t fTrackY = SCROLL_Y + SCROLL_ARROW + 3.f;
	const f32_t fTrackH = SCROLL_H - (SCROLL_ARROW + 3.f) * 2.f;
	const f32_t fThumbH = fMaxScroll <= 0.f ? fTrackH :
		(std::max)(SCROLL_THUMB_MIN, fTrackH * Pane.fHeight / m_fContentHeight);
	const f32_t fThumbTravel = fTrackH - fThumbH;
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bMouse = Mouse_Retail(fMouseX, fMouseY);
	if (!Router.Is_LeftDown())
		m_bScrollDragging = false;
	const f32_t fThumbY = fTrackY + (fMaxScroll > 0.f ? fScroll / fMaxScroll : 0.f) * fThumbTravel;
	const bool_t bThumbHovered = Is_Hovered(SCROLL_X, fThumbY, SCROLL_W, fThumbH);
	if (bThumbHovered && bMouse && 0 == m_iComboOpenKey && Is_Clicked(SCROLL_X, fThumbY, SCROLL_W, fThumbH))
	{
		m_bScrollDragging = true;
		m_fScrollDragOffset = fMouseY - fThumbY;
	}
	if (m_bScrollDragging && bMouse && fThumbTravel > 0.f)
		fScroll = ((fMouseY - m_fScrollDragOffset) - fTrackY) / fThumbTravel * fMaxScroll;
	/* Arrow buttons step a row. */
	const f32_t fUpY = SCROLL_Y + 2.f;
	const f32_t fDownY = SCROLL_Y + SCROLL_H - SCROLL_ARROW - 2.f;
	if (0 == m_iComboOpenKey && Is_Clicked(SCROLL_X, fUpY, SCROLL_ARROW, SCROLL_ARROW))
		fScroll -= SCROLL_WHEEL_STEP;
	if (0 == m_iComboOpenKey && Is_Clicked(SCROLL_X, fDownY, SCROLL_ARROW, SCROLL_ARROW))
		fScroll += SCROLL_WHEEL_STEP;

	fScroll = std::clamp(fScroll, 0.f, fMaxScroll);
	const f32_t fPlacedThumbY = fTrackY + (fMaxScroll > 0.f ? fScroll / fMaxScroll : 0.f) * fThumbTravel;
	Place_Slot("SO_ScrollThumb", SCROLL_X, fPlacedThumbY, SCROLL_W, fThumbH, false);
	m_pView->Set_SlotTexture("SO_ScrollThumb", m_bScrollDragging ? ART_SCROLL_THUMB_DOWN :
		(bThumbHovered ? ART_SCROLL_THUMB_OVER : ART_SCROLL_THUMB_NORMAL));
	m_pView->Set_SlotTexture("SO_ScrollUp", Is_Hovered(SCROLL_X, fUpY, SCROLL_ARROW, SCROLL_ARROW) ?
		ART_SCROLL_UP_OVER : ART_SCROLL_UP_NORMAL);
	m_pView->Set_SlotTexture("SO_ScrollDown", Is_Hovered(SCROLL_X, fDownY, SCROLL_ARROW, SCROLL_ARROW) ?
		ART_SCROLL_DOWN_OVER : ART_SCROLL_DOWN_NORMAL);
	/* Retail shows no bar at all on a screen that fits. */
	const bool_t bScrollable = fMaxScroll > 0.f;
	for (const char* pId : { "SO_ScrollTrack", "SO_ScrollUp", "SO_ScrollDown", "SO_ScrollThumb" })
		m_pView->Set_SlotVisible(pId, bScrollable);
}

bool_t Client::CSystemOptionWindowView::Is_HeaderRow(const ROW_LAYOUT& Row) const
{
	/* The screen's own reset button attaches to the right of the title row: it lives in the
	heading band above the pane, does not scroll and is not clipped by it. */
	return Row.fY + Row.fHeight <= Pane_Rect().fY;
}

void Client::CSystemOptionWindowView::Update_Buttons()
{
	struct BUTTON final
	{
		const char* pSlotId;
		f32_t fX;
		bool_t bEnabled;
	};
	const bool_t bDirty = !m_Draft.Has_SameValues(m_Snapshot);
	const BUTTON Buttons[] = {
		{ "SO_ResetAllBtn", TAB_X, true },
		{ "SO_ApplyBtn", APPLY_BUTTON_X, bDirty },
		{ "SO_ConfirmBtn", CONFIRM_BUTTON_X, true },
		{ "SO_CancelBtn", CANCEL_BUTTON_X, true },
	};
	for (const BUTTON& Button : Buttons)
	{
		const bool_t bHovered = Button.bEnabled && 0 == m_iComboOpenKey &&
			Is_Hovered(Button.fX, BUTTON_Y, BUTTON_W, BUTTON_H);
		Set_Button(Button.pSlotId, ART_BTN_NORMAL, ART_BTN_OVER, ART_BTN_DOWN, ART_BTN_DISABLED,
			!Button.bEnabled ? BUTTON_STATE::DISABLED :
			(bHovered ? (CUIInputRouter::Get().Is_LeftDown() ? BUTTON_STATE::DOWN : BUTTON_STATE::OVER) :
				BUTTON_STATE::NORMAL));
		if (!bHovered || !Is_Clicked(Button.fX, BUTTON_Y, BUTTON_W, BUTTON_H))
			continue;
		CMainApp::Play_UIButtonClickSound();
		if (0 == strcmp(Button.pSlotId, "SO_ResetAllBtn"))
			Reset_All();
		else if (0 == strcmp(Button.pSlotId, "SO_ApplyBtn"))
			m_Snapshot = m_Draft;
		else if (0 == strcmp(Button.pSlotId, "SO_ConfirmBtn"))
		{
			Save_And_Close();
			return;
		}
		else
		{
			Close();
			return;
		}
	}
	/* system option bookmark is retail-disabled in this build. */
	Set_Button("SO_BookmarkBtn", ART_BTN_NORMAL, ART_BTN_OVER, ART_BTN_DOWN, ART_BTN_DISABLED,
		BUTTON_STATE::DISABLED);
}

/* ---- text ---------------------------------------------------------------------------- */

void Client::CSystemOptionWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;
	f32_t fOriginX = 0.f, fOriginY = 0.f;
	if (!Get_WindowOrigin(fOriginX, fOriginY))
		return;

	const float2_t vTopLeft(0.f, 0.f);
	const float2_t vTopCenter(0.5f, 0.f);
	const float2_t vCenter(0.5f, 0.5f);
	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");

	Draw_Label(FONT_YOON, Find_String("window.title"), WINDOW_WIDTH * 0.5f, TITLE_Y,
		TITLE_PX, COLOR_TITLE, vTopCenter);

	/* Tab column. */
	f32_t fY = TAB_Y;
	for (const TAB_ENTRY& Entry : m_TabColumn)
	{
		const bool_t bChild = !Entry.bParent && !Entry.strParentId.empty();
		if (bChild && Entry.strParentId != m_strOpenParentId)
			continue;
		const bool_t bActive = !Entry.bParent && nullptr != Entry.pTab && Entry.pTab->iTabId == m_iActiveTabId;
		Draw_Label(FONT_YG760, Entry.strLabel, TAB_X + (bChild ? TAB_CHILD_TEXT_X : TAB_TEXT_X),
			fY + TAB_TEXT_Y, ROW_FONT_PX,
			Entry.bExcluded ? COLOR_TAB_OFF :
			(bActive || (Entry.bParent && Entry.strParentId == m_strOpenParentId) ? COLOR_TAB_ON : COLOR_TAB_OFF),
			vTopLeft);
		fY += TAB_PITCH;
	}
	Draw_Label(FONT_YG760, Find_String("button.bookmark"), TAB_X + BOOKMARK_W * 0.5f,
		BOOKMARK_Y + BOOKMARK_H * 0.5f, ROW_FONT_PX, COLOR_DISABLED, vCenter);

	/* Pane heading and rows, clipped to the pane. */
	const SYSTEM_OPTION_TAB* pTab = m_Document.Find_Tab(m_iActiveTabId);
	if (nullptr != pTab)
	{
		Draw_Label(FONT_YOON, pTab->strTitle, PANE_X + 3.f, TAB_TITLE_Y, PANE_TITLE_PX, COLOR_TITLE, vTopLeft);
		const RECT_RETAIL Pane = Pane_Rect();
		const auto found = m_Screens.find(m_iActiveTabId);
		/* Header rows (the screen's reset button hangs off the title) sit above the pane:
		unscrolled and outside its clip. */
		if (m_Screens.end() != found)
			for (const ROW_LAYOUT& Row : found->second)
				if (Is_HeaderRow(Row))
					Render_RowText(Row, 0.f);
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
		const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
		const f32_t fClipTop = Ref_Y(Pane.fY) * fScaleY;
		CGameInstance::Get().Set_TextClipInRect(Ref_X(Pane.fX) * fScaleX, fClipTop,
			Pane.fWidth * m_fRetailScale * fScaleX,
			Ref_Y(Pane.fY + Pane.fHeight) * fScaleY - fClipTop);
		/* The drop list is drawn first and its rect then clips the row labels beneath it:
		text has no depth, so this is how the list covers the rows it opens over. */
		if (0 != m_iComboOpenKey)
		{
			Render_ComboPopupText();
			f32_t fPopX = 0.f, fPopY = 0.f, fPopWidth = 0.f, fPopHeight = 0.f;
			if (Get_ComboPopupRect(fPopX, fPopY, fPopWidth, fPopHeight))
				CGameInstance::Get().Add_TextClipOutRect(Ref_X(fPopX) * fScaleX, Ref_Y(fPopY) * fScaleY,
					fPopWidth * m_fRetailScale * fScaleX, fPopHeight * m_fRetailScale * fScaleY);
		}
		if (m_Screens.end() != found)
		{
			const f32_t fScroll = m_ScrollByTab[m_iActiveTabId];
			for (const ROW_LAYOUT& Row : found->second)
				if (!Is_HeaderRow(Row))
					Render_RowText(Row, -fScroll);
		}
		CGameInstance::Get().Clear_TextClipInRect();
		/* This window is the topmost runtime window, so the drop-list rect is the only
		clip-out in effect here and the bottom band draws with it gone. */
		if (0 != m_iComboOpenKey)
			CGameInstance::Get().Clear_TextClipOutRect();
	}

	/* Bottom band. */
	const bool_t bDirty = !m_Draft.Has_SameValues(m_Snapshot);
	Draw_Label(FONT_YOON, Find_String("button.resetAll"), TAB_X + BUTTON_W * 0.5f,
		BUTTON_Y + BUTTON_H * 0.5f, BUTTON_PX, Colors::White, vCenter);
	Draw_Label(FONT_YOON, Find_String("button.apply"), APPLY_BUTTON_X + BUTTON_W * 0.5f,
		BUTTON_Y + BUTTON_H * 0.5f, BUTTON_PX, bDirty ? Colors::White : COLOR_DISABLED, vCenter);
	Draw_Label(FONT_YOON, Find_String("button.confirm"), CONFIRM_BUTTON_X + BUTTON_W * 0.5f,
		BUTTON_Y + BUTTON_H * 0.5f, BUTTON_PX, Colors::White, vCenter);
	Draw_Label(FONT_YOON, Find_String("button.cancel"), CANCEL_BUTTON_X + BUTTON_W * 0.5f,
		BUTTON_Y + BUTTON_H * 0.5f, BUTTON_PX, Colors::White, vCenter);
}

void Client::CSystemOptionWindowView::Hide()
{
	for (const string& strId : m_pView->Get_SlotIds())
		m_pView->Set_SlotVisible(strId, false);
	m_iShownTabId = -1;
}

/* ---- geometry helpers ---------------------------------------------------------------- */

bool_t Client::CSystemOptionWindowView::Get_WindowOrigin(f32_t& fX, f32_t& fY) const
{
	f32_t fWidth = 0.f, fHeight = 0.f;
	return m_pView->Get_SlotRect("SO_WinBg", fX, fY, fWidth, fHeight);
}

f32_t Client::CSystemOptionWindowView::Ref_X(const f32_t fRetailX) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fX + fRetailX * m_fRetailScale;
}

f32_t Client::CSystemOptionWindowView::Ref_Y(const f32_t fRetailY) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fY + fRetailY * m_fRetailScale;
}

Client::CSystemOptionWindowView::RECT_RETAIL Client::CSystemOptionWindowView::Pane_Rect() const
{
	/* Rows start under the pane heading; the retail list component is the pane minus that band. */
	const f32_t fTop = TAB_TITLE_Y + HEIGHT_TAB_TITLE + 8.f;
	return RECT_RETAIL{ PANE_X, fTop, PANE_W, PANE_Y + PANE_H - fTop };
}

void Client::CSystemOptionWindowView::Place_Slot(const string& strSlotId, const f32_t fRetailX,
	const f32_t fRetailY, const f32_t fRetailWidth, const f32_t fRetailHeight, const bool_t bClipToPane)
{
	f32_t fTop = fRetailY, fBottom = fRetailY + fRetailHeight;
	f32_t fOffsetV = 0.f, fScaleV = 1.f;
	if (bClipToPane)
	{
		const RECT_RETAIL Pane = Pane_Rect();
		const f32_t fPaneBottom = Pane.fY + Pane.fHeight;
		if (fBottom <= Pane.fY || fTop >= fPaneBottom || fRetailHeight <= 0.f)
		{
			m_pView->Set_SlotVisible(strSlotId, false);
			return;
		}
		const f32_t fClippedTop = (std::max)(fTop, Pane.fY);
		const f32_t fClippedBottom = (std::min)(fBottom, fPaneBottom);
		fOffsetV = (fClippedTop - fTop) / fRetailHeight;
		fScaleV = (fClippedBottom - fClippedTop) / fRetailHeight;
		fTop = fClippedTop;
		fBottom = fClippedBottom;
	}
	m_pView->Set_SlotVisible(strSlotId, true);
	m_pView->Set_SlotRect(strSlotId, Ref_X(fRetailX), Ref_Y(fTop),
		fRetailWidth * m_fRetailScale, (fBottom - fTop) * m_fRetailScale);
	m_pView->Set_SlotUVWindow(strSlotId, 0.f, fOffsetV, 1.f, fScaleV);
}

void Client::CSystemOptionWindowView::Set_Button(const string& strSlotId, const char* pNormal,
	const char* pOver, const char* pDown, const char* pDisabled, const BUTTON_STATE eState)
{
	switch (eState)
	{
	case BUTTON_STATE::OVER:		m_pView->Set_SlotTexture(strSlotId, pOver); break;
	case BUTTON_STATE::DOWN:		m_pView->Set_SlotTexture(strSlotId, pDown); break;
	case BUTTON_STATE::DISABLED:	m_pView->Set_SlotTexture(strSlotId, pDisabled); break;
	default:						m_pView->Set_SlotTexture(strSlotId, pNormal); break;
	}
}

bool_t Client::CSystemOptionWindowView::Is_Hovered(const f32_t fRetailX, const f32_t fRetailY,
	const f32_t fRetailWidth, const f32_t fRetailHeight) const
{
	return !m_bDragging && CUIInputRouter::Get().Is_Hovered(Ref_X(fRetailX), Ref_Y(fRetailY),
		fRetailWidth * m_fRetailScale, fRetailHeight * m_fRetailScale,
		m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight());
}

bool_t Client::CSystemOptionWindowView::Is_Clicked(const f32_t fRetailX, const f32_t fRetailY,
	const f32_t fRetailWidth, const f32_t fRetailHeight)
{
	return !m_bDragging && !m_bPopupAteClick && CUIInputRouter::Get().Is_Clicked(Ref_X(fRetailX), Ref_Y(fRetailY),
		fRetailWidth * m_fRetailScale, fRetailHeight * m_fRetailScale,
		m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight());
}

bool_t Client::CSystemOptionWindowView::Mouse_Retail(f32_t& fOutX, f32_t& fOutY) const
{
	f32_t fRefX = 0.f, fRefY = 0.f;
	if (!CUIInputRouter::Get().Get_MousePosition(m_pView->Get_ResolutionWidth(),
		m_pView->Get_ResolutionHeight(), fRefX, fRefY))
	{
		return false;
	}
	f32_t fOriginX = 0.f, fOriginY = 0.f;
	(void)Get_WindowOrigin(fOriginX, fOriginY);
	fOutX = (fRefX - fOriginX) / m_fRetailScale;
	fOutY = (fRefY - fOriginY) / m_fRetailScale;
	return true;
}

f32_t Client::CSystemOptionWindowView::Draw_Label(const wstring_t& strFont, const wstring& strText,
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
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(), vPosition, vColor, 0.f,
		vTopLeft, fScale);
	return vMeasured.x * fScale / (fScaleX * m_fRetailScale);
}
