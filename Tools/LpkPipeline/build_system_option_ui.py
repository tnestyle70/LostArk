#!/usr/bin/env python3
"""Build the 환경설정(system option) window's layout document and cut its art from the atlases.

Outputs
-------
  Client/Bin/Resources/UI/SystemOption/*.png
      window chrome, tab strip, slider and check box art, cut at the DefineSubImage
      regions systemoption.gfx and its shared componentsV2 / shareImageV2 dependencies
      resolve to. Nothing here is eyeballed -- every rect is one the gfx itself names.
  Data/UI/SystemOption/SystemOptionStrings.json
      the Korean labels, read from EFTable_GameMsg `sys.preferences.*` /
      `sys.systemoption.*`. C++ sources in this repo stay ASCII, so the window reads
      its text from here.
  Data/UI/SystemOption/SystemOption_Layout.json
      lostark.ui-layout slots for the fixed chrome of CSystemOptionWindowView: the retail
      systemOptionWnd frame (DefaultUIWindow_V2 at stage (381,47), content 876x655), the
      scroll bar and the button band, scaled onto the 1280x720 reference. The option rows
      themselves are not here: CSystemOptionWindowView builds them at runtime from
      SystemOptionRows.json (see build_system_option_rows.py).

Retail facts this encodes (see .md/TJ/09-17/2026-09-17_환경설정_조사.md)
  - systemOptionWnd = DefaultUIWindow_V2 at stage (381,47); SystemOptionWndContent is
    876x655, tab column 224 wide at x=13 y=53, content pane 571x582 at (272,14).
    Every panel shape in the content is alpha 0 -- the visible body is the shared V2
    window frame, not a fill systemoption.gfx owns.
  - EFToggleButton_Tab_renew_systemOption: base 210x36 (SystemOption_I15 517,42), the
    over / selected state adds a 166x35 skin (517,80) at x+4. Tabs pitch 40 within a
    group in SystemOptionTabGroupManager.
  - SystemOptionCustomSliderGroup: label at (2,2) w291, iconMc 26x20 at (122,2), slider
    at (151,1) 151x22, value right aligned at (299,2) w50. The slider's track draws the
    151x8 gradient (517,181) squeezed to 105x8 at y+7; the thumb is componentsV2's
    sliderThumb_V2 -> shareImageV2 V2SliderScrollBtn_* 17x30.
  - SystemOptionCheckBoxGroup: label at (2,2) w220, box V2CheckBox_* 23x21.

Inputs (all already extracted on this machine)
  D:/ClaudeWork/Extracted/SystemOptionGfx/tex/EFUI_SYSTEMOPTION/Texture2D/systemoption_i15.dds
  D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D/*.png
  D:/ClaudeWork/Extracted/LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db

Usage:
  python build_system_option_ui.py [--repo <LostArk root>]
"""

from __future__ import annotations

import argparse
import json
import sqlite3
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"
LOCAL_PAGE = EXTRACTED / "SystemOptionGfx/tex/EFUI_SYSTEMOPTION/Texture2D/systemoption_i15.dds"
SHARE = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"
# componentsv2.gfx image 0: the drop list's own opaque panel lives here, not in shareImageV2.
COMPONENTS_PAGE = EXTRACTED / "SystemOptionGfx/pages/componentsv2_i1.png"

# (output name, x, y, w, h) on COMPONENTS_PAGE
COMPONENTS = [
    # DefaultEFDropdownList_V2 (componentsv2 sprite 1346) = shape 1256 = bitmap fill of
    # DefineSubImage 1255: image 0 (753,328)-(925,442), the 172x114 dark panel under the rows.
    ("SystemOption_ListBg", 753, 328, 172, 114),
]

# (output name, shareImageV2 page stem, x, y, w, h)
SHARED = [
    # DefaultUIWindow_V2 -> WindowBG_V2 -> V2windowBackground
    ("SystemOption_WindowBg",        "shareimagev2_i4",  0, 742, 330, 276),
    # V2WindowDeco -> V2windowTopDeco_center
    ("SystemOption_TopDeco",         "shareimagev2_ib",  293, 1005, 326, 18),
    # s_closeBtn_V2 -> V2closeBtn_*
    ("SystemOption_Close_Normal",    "shareimagev2_ib",  785, 1005, 19, 15),
    ("SystemOption_Close_Over",      "shareimagev2_ib",  827, 1005, 19, 15),
    # AnimatedButton_renew_V2 / AnimatedButton_systemOption_FC -> V2btn_*
    ("SystemOption_Btn_Normal",      "shareimagev2_i46", 643, 988, 103, 36),
    ("SystemOption_Btn_Over",        "shareimagev2_i46", 328, 988, 103, 36),
    ("SystemOption_Btn_Down",        "shareimagev2_ie",  532, 173, 103, 36),
    ("SystemOption_Btn_Disabled",    "shareimagev2_i46", 853, 988, 103, 36),
    # sliderThumb_V2 -> V2SliderScrollBtn_*
    ("SystemOption_Thumb_Normal",    "shareimagev2_ie",  286, 426, 17, 30),
    ("SystemOption_Thumb_Over",      "shareimagev2_ie",  267, 426, 17, 30),
    ("SystemOption_Thumb_Down",      "shareimagev2_ie",  305, 426, 17, 30),
    # SystemOptionCheckBox -> V2CheckBox_*
    ("SystemOption_Check_Normal",    "shareimagev2_ie",  272, 545, 23, 21),
    ("SystemOption_Check_Over",      "shareimagev2_ie",  222, 545, 23, 21),
    ("SystemOption_Check_Selected",  "shareimagev2_ie",  172, 545, 23, 21),
    ("SystemOption_Check_Disabled",  "shareimagev2_ie",  197, 545, 23, 21),
    # DefaultRadioButton2_systemOption / SystemOptionRollingListItem_CheckBox -> V2RadioBtn_*
    ("SystemOption_Radio_Normal",    "shareimagev2_ie",  343, 545, 21, 21),
    ("SystemOption_Radio_Over",      "shareimagev2_ie",  320, 545, 21, 21),
    ("SystemOption_Radio_Selected",  "shareimagev2_ie",  366, 545, 21, 21),
    ("SystemOption_Radio_Disabled",  "shareimagev2_ie",  297, 545, 21, 21),
    # SystemOptionComboBox_Custom -> V2dropBox_* (172x32 box) + V2dropBtnOn/Off_* (28x28 arrow)
    ("SystemOption_Combo_Normal",    "shareimagev2_ie",  350, 323, 172, 32),
    ("SystemOption_Combo_Over",      "shareimagev2_ie",  698, 323, 172, 32),
    ("SystemOption_Combo_Down",      "shareimagev2_ie",  176, 323, 172, 32),
    ("SystemOption_Combo_Disabled",  "shareimagev2_ie",  524, 323, 172, 32),
    ("SystemOption_ComboArrow_Normal",   "shareimagev2_ie", 308, 458, 28, 28),
    ("SystemOption_ComboArrow_Over",     "shareimagev2_ie", 398, 458, 28, 28),
    ("SystemOption_ComboArrow_Open",     "shareimagev2_ie", 278, 458, 28, 28),
    ("SystemOption_ComboArrow_Disabled", "shareimagev2_ie", 548, 458, 28, 28),
    # combo drop list rows -> V2step2List_*
    ("SystemOption_ListRow_Normal",  "shareimagev2_ie",  0, 250, 206, 35),
    ("SystemOption_ListRow_Over",    "shareimagev2_ie",  790, 212, 206, 35),
    ("SystemOption_ListRow_Selected", "shareimagev2_i2", 771, 986, 208, 37),
    # tab-column tree: parent row V2step1List_*, its arrow V2step1ListArrow_*, child row = list row
    ("SystemOption_Tree_Normal",     "shareimagev2_i2",  618, 947, 209, 37),
    ("SystemOption_Tree_Over",       "shareimagev2_ie",  581, 212, 207, 35),
    ("SystemOption_Tree_Selected",   "shareimagev2_i2",  407, 947, 209, 37),
    ("SystemOption_TreeArrow_Closed", "shareimagev2_ib", 866, 973, 20, 16),
    ("SystemOption_TreeArrow_Open",  "shareimagev2_ib",  104, 1005, 23, 19),
    # SystemOptionTitle's bookMarkCheckBox -> shared_favoritesBtnM_*
    ("SystemOption_Star_Normal",     "shareimagev2_ie",  557, 488, 26, 26),
    ("SystemOption_Star_Over",       "shareimagev2_ie",  688, 517, 22, 22),
    ("SystemOption_Star_Selected",   "shareimagev2_ie",  585, 488, 26, 26),
    # SystemOptionLine -> V2Line_HDivision (stretched x3.84 in retail)
    ("SystemOption_Separator",       "shareimagev2_ib",  745, 992, 146, 5),
    # DefaultScrollBar_V2_video -> V2Scrollbar_* (18 wide track, 14x84 thumb, 14x14 arrows)
    ("SystemOption_Scroll_Track",    "shareimagev2_ib",  863, 225, 18, 198),
    ("SystemOption_Scroll_Thumb_Normal", "shareimagev2_i1d", 877, 352, 14, 84),
    ("SystemOption_Scroll_Thumb_Over",   "shareimagev2_i1d", 861, 352, 14, 84),
    ("SystemOption_Scroll_Thumb_Down",   "shareimagev2_i1d", 0, 440, 14, 84),
    ("SystemOption_Scroll_Up_Normal",    "shareimagev2_i6",  436, 1004, 14, 14),
    ("SystemOption_Scroll_Up_Over",      "shareimagev2_i6",  484, 1004, 14, 14),
    ("SystemOption_Scroll_Down_Normal",  "shareimagev2_i6",  500, 1004, 14, 14),
    ("SystemOption_Scroll_Down_Over",    "shareimagev2_i6",  404, 1004, 14, 14),
    # ARKSliderEx_systemOption (the FPS spinner) -> PlusButton_V2_02 / MinusButton_V2_02
    #   -> V2SliderPlusBtn2_* / V2SliderMinusBtn2_* (29x29, drawn x0.7586 in retail)
    ("SystemOption_Plus_Normal",     "shareimagev2_ie",  534, 426, 29, 29),
    ("SystemOption_Plus_Over",       "shareimagev2_ie",  565, 426, 29, 29),
    ("SystemOption_Plus_Down",       "shareimagev2_ie",  503, 426, 29, 29),
    ("SystemOption_Minus_Normal",    "shareimagev2_ie",  410, 426, 29, 29),
    ("SystemOption_Minus_Over",      "shareimagev2_ie",  441, 426, 29, 29),
    ("SystemOption_Minus_Down",      "shareimagev2_ie",  472, 426, 29, 29),
]

# systemoption.gfx's own page SystemOption_I15 (1024x256).
LOCAL = [
    ("SystemOption_Tab",             517, 42, 210, 36),   # EFToggleButton_Tab_renew base
    ("SystemOption_Tab_Selected",    517, 80, 166, 35),   # its over / selected skin
    ("SystemOption_SliderTrack",     517, 181, 151, 8),   # sliderTrack_V2_systemOption
    ("SystemOption_SpinnerBg",       816, 149, 149, 28),  # ARKSliderEx_systemOption inset bar
    ("SystemOption_Icon_Volume",     563, 192, 26, 20),   # SystemOptionCustomSliderGroup iconMc
    ("SystemOption_TitleBullet",     610, 192, 3, 16),    # SystemOptionTitle
    ("SystemOption_ColorFrame",      591, 192, 17, 17),   # SystemOptionColorComponent frame
    ("SystemOption_RowHighlight",    685, 80, 271, 30),   # SystemOptionHighlightMc
]

# GameMsg keys the window shows. Values land in SystemOptionStrings.json under these ids.
STRINGS = {
    "window.title":          "sys.preferences.window_title",
    "tab.gameplay":          "sys.preferences.window_tap_gameplay",
    "tab.community":         "sys.preferences.window_tap_cummunity",
    "tab.hotkey":            "sys.preferences.window_tap_hotkey",
    "tab.controller":        "sys.preferences.window_tap_controller",
    "button.confirm":        "sys.preferences.window_button_confirm",
    "button.cancel":         "sys.preferences.window_button_cancel",
    "button.apply":          "sys.preferences.window_button_save",
    "button.reset":          "sys.preferences.window_button_reset_defaults",
    "button.resetAll":       "sys.preferences.window_button_defaults",
    "button.bookmark":       "sys.preferences.window_btn_bookmark",
}

# --- SystemOption_Layout.json --------------------------------------------------------
# Fixed chrome only. The 279 option rows are laid out at runtime from SystemOptionRows.json
# (the retail AttachTargetIndex / Direction / Margin rule), so this document stops at the
# window frame, the tab column area, the scroll bar and the bottom buttons.
#
# Retail: systemOptionWnd = DefaultUIWindow_V2 at stage (381,47); SystemOptionWndContent is
# 876x655 with the tab column 224 wide at (13,53) and the content pane 571x582 at (272,14),
# scroll bar 18x582 at (845,14). Drawn 1.2x larger than the plain 2/3 stage fit, the same
# call CVehicleWindowView's layout makes; the view reads the effective scale back from the
# SO_WinBg slot width.
WINDOW_SCALE = 1.2
RETAIL_SCALE = 2.0 / 3.0 * WINDOW_SCALE
WINDOW_W = 876.0
WINDOW_H = 690.0                          # frame incl. title band and button band
CONTENT_Y = 0.0                           # SystemOptionWndContent origin == frame origin
TAB_X, TAB_Y, TAB_W, TAB_H = 13.0, 92.0, 224.0, 522.0   # first tree row at y 92, pitch 39
PANE_X, PANE_Y, PANE_W, PANE_H = 272.0, CONTENT_Y + 14.0, 571.0, 582.0
SCROLL_X, SCROLL_Y, SCROLL_W, SCROLL_H = 845.0, PANE_Y, 18.0, PANE_H
BUTTON_Y = 645.0
BUTTON_W, BUTTON_H = 103.0, 36.0
WINDOW_STAGE_X = (1920.0 - WINDOW_W * WINDOW_SCALE) * 0.5
WINDOW_STAGE_Y = 47.0


def layout_slot(slot_id, x, y, w, h, path):
    """One lostark.ui-layout type-0 slot; x/y/w/h are retail px relative to the window."""
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round(WINDOW_STAGE_X * 2.0 / 3.0 + x * RETAIL_SCALE, 4),
                 "y": round(WINDOW_STAGE_Y * 2.0 / 3.0 + y * RETAIL_SCALE, 4),
                 "width": round(w * RETAIL_SCALE, 4), "height": round(h * RETAIL_SCALE, 4)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": [1, 1, 1, 1],
                    "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def build_layout():
    a = "UI/SystemOption/"
    slots = [
        layout_slot("SO_WinBg", 0, 0, WINDOW_W, WINDOW_H, a + "SystemOption_WindowBg.png"),
        layout_slot("SO_TopDeco", (WINDOW_W - 326.0) * 0.5, 40, 326, 18, a + "SystemOption_TopDeco.png"),
        layout_slot("SO_Close", WINDOW_W - 19 - 12, 10, 19, 15, a + "SystemOption_Close_Normal.png"),
        # SystemOptionWndContent/bookMarkButton at (12,18) -- 환경설정 즐겨찾기, retail-disabled here
        layout_slot("SO_BookmarkBtn", TAB_X, CONTENT_Y + 18, 248, 30, a + "SystemOption_Btn_Disabled.png"),
        # scroll bar chrome (DefaultScrollBar_V2_video at (845,14))
        layout_slot("SO_ScrollTrack", SCROLL_X, SCROLL_Y, SCROLL_W, SCROLL_H, a + "SystemOption_Scroll_Track.png"),
        layout_slot("SO_ScrollUp", SCROLL_X + 2, SCROLL_Y + 2, 14, 14, a + "SystemOption_Scroll_Up_Normal.png"),
        layout_slot("SO_ScrollDown", SCROLL_X + 2, SCROLL_Y + SCROLL_H - 16, 14, 14, a + "SystemOption_Scroll_Down_Normal.png"),
        layout_slot("SO_ScrollThumb", SCROLL_X + 2, SCROLL_Y + 17, 14, 84, a + "SystemOption_Scroll_Thumb_Normal.png"),
        # bottom band: 전체초기화 left, 적용 / 확인 / 취소 right (retail order)
        layout_slot("SO_ResetAllBtn", TAB_X, BUTTON_Y, BUTTON_W, BUTTON_H, a + "SystemOption_Btn_Normal.png"),
        layout_slot("SO_ApplyBtn", WINDOW_W - 12 - BUTTON_W * 3 - 12, BUTTON_Y, BUTTON_W, BUTTON_H, a + "SystemOption_Btn_Disabled.png"),
        layout_slot("SO_ConfirmBtn", WINDOW_W - 12 - BUTTON_W * 2 - 6, BUTTON_Y, BUTTON_W, BUTTON_H, a + "SystemOption_Btn_Normal.png"),
        layout_slot("SO_CancelBtn", WINDOW_W - 12 - BUTTON_W, BUTTON_Y, BUTTON_W, BUTTON_H, a + "SystemOption_Btn_Normal.png"),
    ]
    return {"schema": "lostark.ui-layout", "formatVersion": 1,
            "resolution": {"width": 1280, "height": 720}, "classes": ["Default"],
            "systemOption": {
                "retailScale": RETAIL_SCALE,
                "tabColumn": {"x": TAB_X, "y": TAB_Y, "width": TAB_W, "height": TAB_H},
                "pane": {"x": PANE_X, "y": PANE_Y, "width": PANE_W, "height": PANE_H},
            },
            "slots": slots}


def read_strings():
    """GameMsg MSG is UTF-8 bytes; <FONT ...> wrappers the tab titles carry are stripped."""
    conn = sqlite3.connect(str(GAMEMSG))
    conn.text_factory = bytes
    cur = conn.cursor()
    out = {}
    for slot_id, key in STRINGS.items():
        cur.execute("select MSG from GameMsg where KEY=?", (key,))
        row = cur.fetchone()
        if row is None:
            print("  ! GameMsg has no %s (%s)" % (key, slot_id))
            continue
        text = row[0].decode("utf-8", "replace")
        while "<" in text and ">" in text:
            start = text.find("<")
            end = text.find(">", start)
            if end < 0:
                break
            text = text[:start] + text[end + 1:]
        out[slot_id] = text.strip()
    conn.close()
    return out


def crop(page: Image.Image, x, y, w, h) -> Image.Image:
    return page.crop((x, y, x + w, y + h))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    args = ap.parse_args()

    art_out = args.repo / "Client/Bin/Resources/UI/SystemOption"
    data_out = args.repo / "Data/UI/SystemOption"
    art_out.mkdir(parents=True, exist_ok=True)
    data_out.mkdir(parents=True, exist_ok=True)

    pages: dict[str, Image.Image] = {}
    for name, stem, x, y, w, h in SHARED:
        if stem not in pages:
            path = SHARE / (stem + ".png")
            if not path.exists():
                print("  ! missing page", path)
                return 1
            pages[stem] = Image.open(path).convert("RGBA")
        crop(pages[stem], x, y, w, h).save(art_out / (name + ".png"))

    local = Image.open(LOCAL_PAGE).convert("RGBA")
    for name, x, y, w, h in LOCAL:
        crop(local, x, y, w, h).save(art_out / (name + ".png"))
    components = Image.open(COMPONENTS_PAGE).convert("RGBA")
    for name, x, y, w, h in COMPONENTS:
        crop(components, x, y, w, h).save(art_out / (name + ".png"))
    print("wrote %d png to %s" % (len(SHARED) + len(LOCAL) + len(COMPONENTS), art_out))

    strings = read_strings()
    (data_out / "SystemOptionStrings.json").write_text(
        json.dumps({"schema": "lostark.ui-strings", "formatVersion": 1, "strings": strings},
                   ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print("wrote", data_out / "SystemOptionStrings.json", "(%d strings)" % len(strings))

    layout = build_layout()
    (data_out / "SystemOption_Layout.json").write_text(
        json.dumps(layout, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print("wrote", data_out / "SystemOption_Layout.json", "(%d slots)" % len(layout["slots"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
