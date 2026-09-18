#pragma once

/* Shared by SystemOptionWindowView.cpp and SystemOptionWindowView_Rows.cpp only: the retail
geometry of systemoption.gfx (1920x1080 stage px, window-relative) and the art the window
swaps at runtime. Every number here is either read off the gfx symbol dumps or fitted against
the retail screenshots by template matching -- see .md/TJ/09-18/2026-09-18 system-option expansion PLAN
section 2b for the measurements. */

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <DirectXColors.h>

NS_BEGIN(Client)

namespace SystemOptionGeometry
{
	constexpr f32_t TEXT_BOOST = 1.15f;
	/* systemOptionWnd = DefaultUIWindow_V2, content 876 wide; the frame is 690 tall with the
	title band and the button band included. */
	constexpr f32_t WINDOW_WIDTH = 876.f;
	constexpr f32_t TITLE_BAR_HEIGHT = 52.f;
	constexpr f32_t TITLE_Y = 8.f;
	constexpr f32_t TITLE_PX = 18.f;

	/* Tab column: SystemOptionBaseTreeItem rows, 224 wide, pitch 39, first row at y 92. */
	constexpr f32_t TAB_X = 13.f;
	constexpr f32_t TAB_Y = 92.f;
	constexpr f32_t TAB_W = 224.f;
	constexpr f32_t TAB_ROW_H = 37.f;
	constexpr f32_t TAB_PITCH = 39.f;
	constexpr f32_t TAB_TEXT_X = 14.f;
	constexpr f32_t TAB_CHILD_TEXT_X = 27.f;
	constexpr f32_t TAB_TEXT_Y = 6.f;
	/* SystemOptionWndContent sits CONTENT_Y below the frame top (876x690 frame, 876x655
	content): every content-relative retail coordinate below carries that offset. */
	constexpr f32_t CONTENT_Y = 35.f;
	constexpr f32_t BOOKMARK_Y = CONTENT_Y + 18.f;
	constexpr f32_t BOOKMARK_W = 248.f;
	constexpr f32_t BOOKMARK_H = 30.f;

	/* Content pane (SystemOptionWndContent list at (272,14) 571x582) and its scroll bar
	(DefaultScrollBar_V2 at (845,14), the pane's full height). */
	constexpr f32_t PANE_X = 272.f;
	constexpr f32_t PANE_Y = CONTENT_Y + 14.f;
	constexpr f32_t PANE_W = 571.f;
	constexpr f32_t PANE_H = 582.f;
	constexpr f32_t TAB_TITLE_Y = 56.f;
	constexpr f32_t SCROLL_X = 847.f;
	constexpr f32_t SCROLL_Y = PANE_Y;
	constexpr f32_t SCROLL_H = PANE_H;
	constexpr f32_t SCROLL_W = 14.f;
	constexpr f32_t SCROLL_ARROW = 14.f;
	constexpr f32_t SCROLL_THUMB_MIN = 30.f;
	constexpr f32_t SCROLL_WHEEL_STEP = 41.f;

	/* Bottom band: reset-all left; apply / confirm / cancel right. */
	constexpr f32_t BUTTON_Y = 645.f;
	constexpr f32_t BUTTON_W = 103.f;
	constexpr f32_t BUTTON_H = 36.f;
	constexpr f32_t APPLY_BUTTON_X = WINDOW_WIDTH - 12.f - BUTTON_W * 3.f - 12.f;
	constexpr f32_t CONFIRM_BUTTON_X = WINDOW_WIDTH - 12.f - BUTTON_W * 2.f - 6.f;
	constexpr f32_t CANCEL_BUTTON_X = WINDOW_WIDTH - 12.f - BUTTON_W;
	constexpr f32_t BUTTON_PX = 16.f;

	/* Row text. */
	/* Measured against the retail capture: row / tab / value glyphs are 16 px of this font,
	the pane heading 14, group titles 16. */
	constexpr f32_t ROW_FONT_PX = 16.f;
	constexpr f32_t TITLE_ROW_PX = 16.f;
	constexpr f32_t PANE_TITLE_PX = 14.f;

	/* Row heights per control (retail pitch minus the table's own MarginY). */
	constexpr f32_t HEIGHT_TAB_TITLE = 24.f;
	constexpr f32_t HEIGHT_GROUP_TITLE = 24.f;
	constexpr f32_t HEIGHT_SEPARATOR = 5.f;
	constexpr f32_t HEIGHT_COMBOBOX = 34.f;
	constexpr f32_t HEIGHT_SLIDER = 24.f;
	constexpr f32_t HEIGHT_CHECKBOX = 31.f;
	constexpr f32_t HEIGHT_SPINNER = 28.f;
	constexpr f32_t HEIGHT_BUTTON = 30.f;
	constexpr f32_t HEIGHT_SUBTITLE = 24.f;
	constexpr f32_t HEIGHT_COLOR = 24.f;
	constexpr f32_t HEIGHT_RADIO_ROW = 30.f;
	constexpr f32_t HEIGHT_TEXT_INPUT = 34.f;
	constexpr f32_t HEIGHT_DEFAULT = 24.f;

	/* Direction 2 (attach to the right) registration offsets, fitted. */
	constexpr f32_t RIGHT_ANCHOR_CHECKBOX = 242.f;
	constexpr f32_t RIGHT_ANCHOR_BUTTON = 106.f;
	constexpr f32_t RIGHT_ANCHOR_SUBTITLE = 324.f;
	constexpr f32_t RIGHT_ANCHOR_COLOR = 260.f;

	/* Component-internal offsets (gfx symbol dumps). */
	constexpr f32_t CHECK_BOX_DX = 117.f;     // V2CheckBox inside SystemOptionCheckBox
	constexpr f32_t CHECK_BOX_DY = 2.f;
	constexpr f32_t CHECK_W = 23.f;
	constexpr f32_t CHECK_H = 21.f;
	constexpr f32_t SLIDER_ICON_DX = 122.f;
	constexpr f32_t SLIDER_ICON_W = 26.f;
	constexpr f32_t SLIDER_ICON_H = 20.f;
	/* Track 466..612 of the label's 314 origin in the retail capture. */
	constexpr f32_t SLIDER_DX = 154.f;
	constexpr f32_t SLIDER_TRACK_W = 146.f;
	constexpr f32_t SLIDER_TRACK_H = 8.f;
	constexpr f32_t SLIDER_TRACK_DY = 8.f;
	constexpr f32_t SLIDER_THUMB_W = 17.f;
	constexpr f32_t SLIDER_THUMB_H = 30.f;
	constexpr f32_t SLIDER_THUMB_DY = -3.f;
	constexpr f32_t SLIDER_VALUE_RIGHT = 345.f;
	constexpr f32_t COMBO_H = 32.f;
	constexpr f32_t COMBO_ARROW = 28.f;
	constexpr f32_t COMBO_ARROW_INSET = 30.f;
	constexpr f32_t COMBO_TEXT_DX = 9.f;
	constexpr f32_t COMBO_TEXT_DY = 8.f;
	constexpr f32_t COMBO_ROW_H = 35.f;        // V2step2List row in the drop list
	constexpr f32_t SPINNER_BG_W = 149.f;
	constexpr f32_t SPINNER_BG_H = 28.f;
	constexpr f32_t SPINNER_BTN = 22.f;        // 29 px art at retail's x0.7586
	constexpr f32_t SPINNER_TRACK_DY = 10.f;
	constexpr f32_t SPINNER_THUMB_W = 11.f;
	constexpr f32_t SPINNER_THUMB_H = 19.f;
	constexpr f32_t SPINNER_VALUE_DX = 155.f;
	constexpr f32_t SPINNER_STEP = 10.f;       // -/+ click on the FPS limit
	constexpr f32_t GROUP_STAR_DX = -2.f;
	constexpr f32_t GROUP_STAR_DY = -1.f;
	constexpr f32_t GROUP_BULLET_DX = 18.f;
	constexpr f32_t GROUP_BULLET_DY = 4.f;
	constexpr f32_t GROUP_TEXT_DX = 22.f;
	constexpr f32_t GROUP_TEXT_DY = 2.f;
	constexpr f32_t SEPARATOR_W = 561.f;
	constexpr f32_t RADIO_PITCH_X = 85.f;
	constexpr f32_t RADIO_PITCH_Y = 30.f;
	constexpr f32_t RADIO_TEXT_TO_BUTTON = 50.f;
	constexpr f32_t RADIO_W = 21.f;
	constexpr f32_t COLOR_FRAME = 17.f;
	constexpr f32_t HIGHLIGHT_H = 30.f;
	constexpr int32_t MAX_COMBO_CHOICES = 11;
	constexpr int32_t RADIO_COLUMNS = 5;

	const fvector_t COLOR_TITLE = XMVectorSet(1.f, 247.f / 255.f, 226.f / 255.f, 1.f);         // #fff7e2
	const fvector_t COLOR_LABEL = XMVectorSet(235.f / 255.f, 235.f / 255.f, 235.f / 255.f, 1.f);
	const fvector_t COLOR_SUB = XMVectorSet(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.f);
	const fvector_t COLOR_TAB_ON = XMVectorSet(254.f / 255.f, 254.f / 255.f, 223.f / 255.f, 1.f); // #fefedf
	const fvector_t COLOR_TAB_OFF = XMVectorSet(164.f / 255.f, 175.f / 255.f, 183.f / 255.f, 1.f); // #a4afb7
	const fvector_t COLOR_VALUE = XMVectorSet(235.f / 255.f, 235.f / 255.f, 235.f / 255.f, 1.f);
	const fvector_t COLOR_DISABLED = XMVectorSet(120.f / 255.f, 120.f / 255.f, 120.f / 255.f, 1.f);

	constexpr const char* ART_BTN_NORMAL = "UI/SystemOption/SystemOption_Btn_Normal.png";
	constexpr const char* ART_BTN_OVER = "UI/SystemOption/SystemOption_Btn_Over.png";
	constexpr const char* ART_BTN_DOWN = "UI/SystemOption/SystemOption_Btn_Down.png";
	constexpr const char* ART_BTN_DISABLED = "UI/SystemOption/SystemOption_Btn_Disabled.png";
	constexpr const char* ART_CLOSE_NORMAL = "UI/SystemOption/SystemOption_Close_Normal.png";
	constexpr const char* ART_CLOSE_OVER = "UI/SystemOption/SystemOption_Close_Over.png";
	constexpr const char* ART_CHECK_NORMAL = "UI/SystemOption/SystemOption_Check_Normal.png";
	constexpr const char* ART_CHECK_OVER = "UI/SystemOption/SystemOption_Check_Over.png";
	constexpr const char* ART_CHECK_SELECTED = "UI/SystemOption/SystemOption_Check_Selected.png";
	constexpr const char* ART_CHECK_DISABLED = "UI/SystemOption/SystemOption_Check_Disabled.png";
	constexpr const char* ART_RADIO_NORMAL = "UI/SystemOption/SystemOption_Radio_Normal.png";
	constexpr const char* ART_RADIO_OVER = "UI/SystemOption/SystemOption_Radio_Over.png";
	constexpr const char* ART_RADIO_SELECTED = "UI/SystemOption/SystemOption_Radio_Selected.png";
	constexpr const char* ART_RADIO_DISABLED = "UI/SystemOption/SystemOption_Radio_Disabled.png";
	constexpr const char* ART_THUMB_NORMAL = "UI/SystemOption/SystemOption_Thumb_Normal.png";
	constexpr const char* ART_THUMB_OVER = "UI/SystemOption/SystemOption_Thumb_Over.png";
	constexpr const char* ART_THUMB_DOWN = "UI/SystemOption/SystemOption_Thumb_Down.png";
	constexpr const char* ART_TRACK = "UI/SystemOption/SystemOption_SliderTrack.png";
	constexpr const char* ART_ICON_VOLUME = "UI/SystemOption/SystemOption_Icon_Volume.png";
	constexpr const char* ART_COMBO_NORMAL = "UI/SystemOption/SystemOption_Combo_Normal.png";
	constexpr const char* ART_COMBO_OVER = "UI/SystemOption/SystemOption_Combo_Over.png";
	constexpr const char* ART_COMBO_DOWN = "UI/SystemOption/SystemOption_Combo_Down.png";
	constexpr const char* ART_COMBO_DISABLED = "UI/SystemOption/SystemOption_Combo_Disabled.png";
	constexpr const char* ART_COMBOARROW_NORMAL = "UI/SystemOption/SystemOption_ComboArrow_Normal.png";
	constexpr const char* ART_COMBOARROW_OVER = "UI/SystemOption/SystemOption_ComboArrow_Over.png";
	constexpr const char* ART_COMBOARROW_OPEN = "UI/SystemOption/SystemOption_ComboArrow_Open.png";
	constexpr const char* ART_COMBOARROW_DISABLED = "UI/SystemOption/SystemOption_ComboArrow_Disabled.png";
	constexpr const char* ART_LISTROW_NORMAL = "UI/SystemOption/SystemOption_ListRow_Normal.png";
	constexpr const char* ART_LISTROW_OVER = "UI/SystemOption/SystemOption_ListRow_Over.png";
	constexpr const char* ART_LISTROW_SELECTED = "UI/SystemOption/SystemOption_ListRow_Selected.png";
	/* DefaultEFDropdownList_V2's own opaque panel (componentsv2 bitmap 1255), under the rows. */
	constexpr const char* ART_LIST_BG = "UI/SystemOption/SystemOption_ListBg.png";
	constexpr const char* ART_TREE_NORMAL = "UI/SystemOption/SystemOption_Tree_Normal.png";
	constexpr const char* ART_TREE_OVER = "UI/SystemOption/SystemOption_Tree_Over.png";
	constexpr const char* ART_TREE_SELECTED = "UI/SystemOption/SystemOption_Tree_Selected.png";
	constexpr const char* ART_TREEARROW_CLOSED = "UI/SystemOption/SystemOption_TreeArrow_Closed.png";
	constexpr const char* ART_TREEARROW_OPEN = "UI/SystemOption/SystemOption_TreeArrow_Open.png";
	constexpr const char* ART_STAR_NORMAL = "UI/SystemOption/SystemOption_Star_Normal.png";
	constexpr const char* ART_STAR_OVER = "UI/SystemOption/SystemOption_Star_Over.png";
	constexpr const char* ART_STAR_SELECTED = "UI/SystemOption/SystemOption_Star_Selected.png";
	constexpr const char* ART_SEPARATOR = "UI/SystemOption/SystemOption_Separator.png";
	constexpr const char* ART_SCROLL_THUMB_NORMAL = "UI/SystemOption/SystemOption_Scroll_Thumb_Normal.png";
	constexpr const char* ART_SCROLL_THUMB_OVER = "UI/SystemOption/SystemOption_Scroll_Thumb_Over.png";
	constexpr const char* ART_SCROLL_THUMB_DOWN = "UI/SystemOption/SystemOption_Scroll_Thumb_Down.png";
	constexpr const char* ART_SCROLL_UP_NORMAL = "UI/SystemOption/SystemOption_Scroll_Up_Normal.png";
	constexpr const char* ART_SCROLL_UP_OVER = "UI/SystemOption/SystemOption_Scroll_Up_Over.png";
	constexpr const char* ART_SCROLL_DOWN_NORMAL = "UI/SystemOption/SystemOption_Scroll_Down_Normal.png";
	constexpr const char* ART_SCROLL_DOWN_OVER = "UI/SystemOption/SystemOption_Scroll_Down_Over.png";
	constexpr const char* ART_PLUS_NORMAL = "UI/SystemOption/SystemOption_Plus_Normal.png";
	constexpr const char* ART_PLUS_OVER = "UI/SystemOption/SystemOption_Plus_Over.png";
	constexpr const char* ART_PLUS_DOWN = "UI/SystemOption/SystemOption_Plus_Down.png";
	constexpr const char* ART_MINUS_NORMAL = "UI/SystemOption/SystemOption_Minus_Normal.png";
	constexpr const char* ART_MINUS_OVER = "UI/SystemOption/SystemOption_Minus_Over.png";
	constexpr const char* ART_MINUS_DOWN = "UI/SystemOption/SystemOption_Minus_Down.png";
	constexpr const char* ART_SPINNER_BG = "UI/SystemOption/SystemOption_SpinnerBg.png";
	constexpr const char* ART_TITLE_BULLET = "UI/SystemOption/SystemOption_TitleBullet.png";
	constexpr const char* ART_COLOR_FRAME = "UI/SystemOption/SystemOption_ColorFrame.png";
	constexpr const char* ART_ROW_HIGHLIGHT = "UI/SystemOption/SystemOption_RowHighlight.png";
	constexpr const char* ART_TAB = "UI/SystemOption/SystemOption_Tab.png";
	constexpr const char* ART_TAB_SELECTED = "UI/SystemOption/SystemOption_Tab_Selected.png";
}

NS_END
