#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "SystemOptionRowsDocument.h"
#include "UIWindowDrag.h"
#include "UserSettingsDocument.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Retail system option window (systemoption.gfx systemOptionWnd) rebuilt as CUI_Sprite slots
under LEVEL::STATIC, the same construction as CVehicleWindowView.

The window is the retail one, whole: every screen, group and row of the four tab sets this
project keeps (video / audio / accessibility / game play x4) comes from
Data/UI/SystemOption/SystemOptionRows.json, a projection of EFTable_SystemOption, and is laid
out with the table's own relative rule (SystemOptionWindowView_Rows.cpp). Only the fixed chrome
-- frame, scroll bar, buttons -- is authored in SystemOption_Layout.json; the row widgets are
runtime slots created on a screen's first visit.

Every row edits a value in CUserSettings keyed by its retail row id. Rows the engine acts on
(audio buses, brightness, bloom, FXAA, SSAO, mouse cursor) take effect as they change; the
rest look and behave like retail controls and simply store their value.

Edits are live -- the draft is committed as the control moves, so the change is audible and
visible immediately. Confirm writes the file; Cancel, the X and Esc restore the snapshot taken
when the window opened. apply writes without closing; reset / reset-all restore the retail
defaults of the screen / of everything. */
class CSystemOptionWindowView final
{
public:
	CSystemOptionWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CSystemOptionWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	/* Snapshots the live settings so Cancel has something exact to go back to. */
	void Open();
	/* Restores the snapshot -- the same thing Cancel does. */
	void Close();
	void Toggle();

	/* Per frame from CMainApp while the window shows. No-op (hides) while closed. */
	void Update(f32_t fTimeDelta);
	/* CMainApp owns the Escape edge (one key source, no DirectInput lag): closes the open
	drop list first, then cancels the window. */
	void Handle_EscapeEdge();
	/* Screen-pixel rect of the frame while open, for the text passes of the windows drawn
	underneath (CMainApp clips their labels out of it). */
	bool_t Get_ScreenRect(f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;
	/* LOA-font text pass -- after CImGuiLayer::EndFrame() like the other runtime windows. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen. */
	void Hide();

	/* True once after a video field changed. CMainApp re-activates the current scene
	rendering profile so the new brightness / post-process reaches the renderer now instead
	of waiting for the next Level or region change to resolve quality again. */
	bool_t Take_VideoDirty();

private:
	/* Retail px -> reference px: everything below is authored in retail px (the 1920x1080
	stage systemoption.gfx was built for) and scaled through m_fRetailScale at draw time. */
	struct RECT_RETAIL { f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f; };

	/* One laid-out row of the active screen: retail px relative to the pane's scroll origin,
	plus the runtime slot ids of the sprites it owns. */
	struct ROW_LAYOUT
	{
		const SYSTEM_OPTION_ROW* pRow = nullptr;
		f32_t fX = 0.f;
		f32_t fY = 0.f;
		f32_t fHeight = 0.f;
		vector<string> SlotIds;
	};

	/* Tab column entry: a parent (game play) or a screen. */
	struct TAB_ENTRY
	{
		const SYSTEM_OPTION_TAB* pTab = nullptr;   // nullptr for a parent row
		string strParentId;
		wstring strLabel;
		bool_t bParent = false;
		bool_t bExcluded = false;                  // community / hotkey / gamepad: shown, inert
		string strSlotId;
		string strArrowSlotId;
	};

	enum class BUTTON_STATE { NORMAL, OVER, DOWN, DISABLED };

private:
	void Load_Strings();
	void Build_TabColumn();
	void Build_Screen(const SYSTEM_OPTION_TAB& Tab);
	void Layout_Screen(const SYSTEM_OPTION_TAB& Tab, vector<ROW_LAYOUT>& OutRows) const;
	f32_t Row_Height(const SYSTEM_OPTION_ROW& Row) const;
	void Ensure_RowSlots(ROW_LAYOUT& Row);
	void Update_Chrome();
	void Update_TabColumn();
	void Update_Scroll();
	void Update_Rows();
	void Update_ComboPopup();
	/* Retail-px rect (window-relative) of the open drop list; false when none is open. */
	bool_t Get_ComboPopupRect(f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;
	void Update_Buttons();
	void Render_RowText(const ROW_LAYOUT& Row, f32_t fPaneTop);
	void Render_ComboPopupText();

	/* The retail table marks a few combo defaults -1 ("decide at run time"); this is that
	decision for this client, used both when seeding and when resetting. */
	static f32_t Effective_Default(const SYSTEM_OPTION_ROW& Row);

	/* Draft editing. */
	f32_t Read_Value(const SYSTEM_OPTION_ROW& Row) const;
	void Write_Value(const SYSTEM_OPTION_ROW& Row, f32_t fValue);
	void Commit_Draft(const SYSTEM_OPTION_ROW& Row);
	void Reset_Screen(const SYSTEM_OPTION_TAB& Tab);
	void Reset_All();
	void Save_And_Close();
	static bool_t Is_VideoRow(const SYSTEM_OPTION_ROW& Row);
	/* Greyed by retail because of another row's value (upscaling mode, cursor outline). */
	bool_t Is_RetailDisabled(const SYSTEM_OPTION_ROW& Row) const;

	/* Geometry helpers. */
	bool_t Get_WindowOrigin(f32_t& fX, f32_t& fY) const;
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	RECT_RETAIL Pane_Rect() const;
	/* Rows laid out above the pane's row band (the screen reset button beside the title). */
	bool_t Is_HeaderRow(const ROW_LAYOUT& Row) const;
	/* Places a runtime slot at a retail-px rect relative to the window origin, clipping it
	to the pane when bClipToPane. Hidden entirely when nothing of it remains. */
	void Place_Slot(const string& strSlotId, f32_t fRetailX, f32_t fRetailY,
		f32_t fRetailWidth, f32_t fRetailHeight, bool_t bClipToPane);
	void Set_Button(const string& strSlotId, const char* pNormal, const char* pOver,
		const char* pDown, const char* pDisabled, BUTTON_STATE eState);
	bool_t Is_Hovered(f32_t fRetailX, f32_t fRetailY, f32_t fRetailWidth, f32_t fRetailHeight) const;
	bool_t Is_Clicked(f32_t fRetailX, f32_t fRetailY, f32_t fRetailWidth, f32_t fRetailHeight);
	bool_t Mouse_Retail(f32_t& fOutX, f32_t& fOutY) const;
	f32_t Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);
	const wstring& Find_String(const char* pKey) const;

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_ChromeSlotIds;
	f32_t							m_fRetailScale = 0.8f;
	CUIWindowDrag					m_Drag;

	map<string, wstring>			m_Strings;
	CSystemOptionRowsDocument		m_Document;

	vector<TAB_ENTRY>				m_TabColumn;
	string							m_strOpenParentId;      // which parent is unfolded
	int32_t							m_iActiveTabId = 0;
	/* Laid-out rows per screen, built on first visit and kept; only the active screen's
	slots are shown. */
	map<int32_t, vector<ROW_LAYOUT>>	m_Screens;
	map<int32_t, f32_t>				m_ScrollByTab;          // retail px scrolled, per screen
	map<int32_t, f32_t>				m_ContentHeightByTab;   // retail px, per screen
	f32_t							m_fContentHeight = 0.f;  // active screen, retail px
	int32_t							m_iShownTabId = -1;      // whose row slots are currently visible
	int32_t							m_iHoveredRowKey = 0;    // primaryKey under the mouse (row highlight)

	/* Widget interaction state. */
	int32_t							m_iSliderDragKey = 0;    // primaryKey of the slider being dragged
	int32_t							m_iComboOpenKey = 0;     // primaryKey of the open combo, 0 = none
	int32_t							m_iComboHover = -1;
	bool_t							m_bScrollDragging = false;
	f32_t							m_fScrollDragOffset = 0.f;
	vector<string>					m_PopupSlotIds;
	string							m_strPopupBgSlotId;      // the opaque panel under the drop list rows
	/* An open drop list owns every click of its frame: whatever it did with it (pick a row,
	close), nothing underneath may see the same click. */
	bool_t							m_bPopupAteClick = false;

	USER_SETTINGS					m_Draft{};
	USER_SETTINGS					m_Snapshot{};
	bool_t							m_bOpen = false;
	/* Title-band drag in progress: widgets keep following the frame but take no clicks. */
	bool_t							m_bDragging = false;
	bool_t							m_bVideoDirty = false;
};

NS_END
