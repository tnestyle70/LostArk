#!/usr/bin/env python3
"""Build the honor title (칭호) data document, the title window layout and its art.

Outputs
-------
  Data/Titles/HonorTitles.json
      the titles a player may wear: the chosen retail completed titles (EFTable_HonorTitle
      Type 2 rows, names from EFTable_GameMsg tip.name.honortitle_<pk>) plus the project's
      own titles (ids 100001+). Publish-HonorTitles.ps1 turns it into the Server bootstrap
      and CHonorTitleCatalog (Client) reads the names from it.
  Data/UI/HonorTitle/HonorTitle_Layout.json
      lostark.ui-layout slots for CHonorTitleWindowView: the retail honorTitleWnd
      (honortitle.gfx: DefaultUIWindow_V2 at stage (820,181.9), 452 px wide) reduced to
      the completed-title list, the current-title line and the apply / deselect buttons,
      scaled 2/3 x 1.2 onto the 1280x720 reference like the vehicle window.
  Client/Bin/Resources/UI/HonorTitle/*.png
      window chrome (shared componentsV2 / shareImageV2 regions, the same DefaultUIWindow_V2
      / AnimatedButton_renew_V2 / V2step2List crops the vehicle window uses) and the
      honortitle.gfx "사용중" badge (honortitle_i7 DefineSubImage 53).

Every crop is a region the gfx itself names. Placement source: honortitle.gfx
honorTitleWnd children -- completeTitleList (18,167) 395 px wide rows of 32
(HonorTitleRendererItem_big: text $YG760 14 px #FFFFFF at (27,6), useType_mc at (334,0)
whose "using" frame shows the 60x30 badge at (-62,5)), currentTitleTF (17,672),
applyTitleBtn (106,749) / applyDeselectBtn (227.95,749) 103x36.

Inputs (already extracted on this machine)
  D:/ClaudeWork/Extracted/HonorTitle/tables/EFGame_Extra/ClientData/TableData/EFTable_HonorTitle.db
  D:/ClaudeWork/Extracted/LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db
  D:/ClaudeWork/Extracted/HonorTitle/tex/honortitle_i7.png
  D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D/*.png

Usage:
  python build_honor_title_data.py [--repo <LostArk root>]
"""

from __future__ import annotations

import argparse
import json
import re
import sqlite3
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
HONOR_DB = EXTRACTED / "HonorTitle/tables/EFGame_Extra/ClientData/TableData/EFTable_HonorTitle.db"
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"
HONOR_ATLAS = EXTRACTED / "HonorTitle/tex/honortitle_i7.png"
SHARE = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"

# Retail completed titles (EFTable_HonorTitle Type 2, Hidden 0) the project ships, in list
# order: the story / continent titles first, then the Legion raid ones.
RETAIL_TITLE_IDS = [
    30001, 30004, 30007, 30082, 30022, 30078, 30088, 30090, 30118,
    39001, 39003, 39005, 39007, 39009, 39011, 30068, 39008,
]
# Project titles (user request). Ids start at 100001, far above the retail table (max 39xxx).
PROJECT_TITLES = [
    (100001, "161기 최후의 4인"),
    (100002, "최고의 팀장"),
    (100003, "이펙트의 천재"),
    (100004, "맵 제작의 신"),
    (100005, "161기 최고령"),
    (100006, "161기 반장"),
]

# (output name, page stem, x, y, w, h) -- DefineSubImage regions (see build_vehicle_ui.py for
# how each shared component resolves to its page).
CHROME = [
    ("HonorTitle_WindowBg",     "shareimagev2_i4",  0, 742, 330, 276),     # WindowBG_V2
    ("HonorTitle_TopDeco",      "shareimagev2_ib",  293, 1005, 326, 18),   # V2windowTopDeco_center
    ("HonorTitle_Close_Normal", "shareimagev2_ib",  785, 1005, 19, 15),    # V2closeBtn_*
    ("HonorTitle_Close_Over",   "shareimagev2_ib",  827, 1005, 19, 15),
    ("HonorTitle_Btn_Normal",   "shareimagev2_i46", 643, 988, 103, 36),    # AnimatedButton_renew_V2
    ("HonorTitle_Btn_Over",     "shareimagev2_i46", 328, 988, 103, 36),
    ("HonorTitle_Btn_Down",     "shareimagev2_ie",  532, 173, 103, 36),
    ("HonorTitle_Btn_Disabled", "shareimagev2_i46", 853, 988, 103, 36),
    ("HonorTitle_Row_Over",     "shareimagev2_ie",  790, 212, 206, 35),    # V2step2List_over
    ("HonorTitle_Row_Selected", "shareimagev2_i2",  771, 986, 208, 37),    # V2step2List_selected
]
# honortitle.gfx's own page honortitle_i7: DefineSubImage 53 = the "사용중" badge.
LOCAL = [
    ("HonorTitle_UsingBadge", 937, 51, 60, 30),
]

# --- HonorTitle_Layout.json ----------------------------------------------------------
WINDOW_SCALE = 1.2                                  # same enlargement as the vehicle window
RETAIL_SCALE = 2.0 / 3.0 * WINDOW_SCALE
WINDOW_W = 452.0                                    # WindowBG_V2 330 * 1.3697
VISIBLE_ROWS = 12                                   # retail list shows 15 of 32 px; 12 keeps the
                                                    # 1.2x window inside 720 px
ROW_X, ROW_Y0, ROW_W, ROW_H = 18.0, 167.0, 395.0, 32.0
LIST_H = ROW_H * VISIBLE_ROWS
CURRENT_Y = ROW_Y0 + LIST_H + 22.0                  # currentTitleTF sat 25 px under the list
BUTTON_Y = CURRENT_Y + 50.0                         # applyTitleBtn sat 77 px under it
WINDOW_H = BUTTON_Y + 36.0 + 20.0
WINDOW_STAGE_X, WINDOW_STAGE_Y = 820.0, 181.9
APPLY_X, DESELECT_X = 106.0, 227.95
BADGE_X, BADGE_Y = 334.0 - 62.0, 5.0                # useType_mc (334,0) + badge (-62,5)


def layout_slot(slot_id, x, y, w, h, path):
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
    A = "UI/HonorTitle/"
    slots = [
        layout_slot("HT_WinBg", 0, 0, WINDOW_W, WINDOW_H, A + "HonorTitle_WindowBg.png"),
        layout_slot("HT_TopDeco", (WINDOW_W - 326) * 0.5, 40, 326, 18, A + "HonorTitle_TopDeco.png"),
        layout_slot("HT_Close", WINDOW_W - 19 - 12, 10, 19, 15, A + "HonorTitle_Close_Normal.png"),
    ]
    for i in range(VISIBLE_ROWS):
        y = ROW_Y0 + ROW_H * i
        slots += [
            layout_slot("HT_Row%d_Over" % i, ROW_X, y, ROW_W, ROW_H, A + "HonorTitle_Row_Over.png"),
            layout_slot("HT_Row%d_Selected" % i, ROW_X, y, ROW_W, ROW_H, A + "HonorTitle_Row_Selected.png"),
            layout_slot("HT_Row%d_Using" % i, ROW_X + BADGE_X, y + BADGE_Y, 60, 30, A + "HonorTitle_UsingBadge.png"),
        ]
    slots += [
        layout_slot("HT_ApplyBtn", APPLY_X, BUTTON_Y, 103, 36, A + "HonorTitle_Btn_Normal.png"),
        layout_slot("HT_DeselectBtn", DESELECT_X, BUTTON_Y, 103, 36, A + "HonorTitle_Btn_Normal.png"),
    ]
    return {"schema": "lostark.ui-layout", "formatVersion": 1,
            "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}


def strip_markup(text: str) -> str:
    """GameMsg names may carry Scaleform <img> emoticons (39008); the plate draws plain text."""
    return re.sub(r"<[^>]+>", "", text).strip()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo

    hconn = sqlite3.connect(str(HONOR_DB))
    gconn = sqlite3.connect(str(GAMEMSG))
    gconn.text_factory = bytes

    def msg(key: str) -> str:
        row = gconn.execute("select MSG from GameMsg where lower(KEY)=lower(?)", (key,)).fetchone()
        if row is None:
            raise SystemExit("GameMsg has no %s" % key)
        return row[0].decode("utf-8")

    titles = []
    for pk in RETAIL_TITLE_IDS:
        row = hconn.execute("select Name, Type, Hidden from HonorTitle where PrimaryKey=?", (pk,)).fetchone()
        if row is None:
            raise SystemExit("EFTable_HonorTitle has no row %d" % pk)
        name_key, kind, hidden = row
        if kind != 2:
            raise SystemExit("title %d is not a completed (Type 2) title" % pk)
        raw = msg(name_key)
        titles.append({"titleId": pk, "name": strip_markup(raw), "origin": "retail", "nameKey": name_key})
        print("  %d %s" % (pk, titles[-1]["name"]))
    for pk, name in PROJECT_TITLES:
        titles.append({"titleId": pk, "name": name, "origin": "project", "nameKey": None})

    strings = {
        "windowTitle": msg("sys.honortitle.ui_title"),                       # 칭호
        "apply": msg("sys.honortitle.ui_button_apply"),                      # 적용
        "remove": msg("sys.honortitle.ui_button_remove"),                    # 해제
        "current": msg("sys.honortitle.ui_rabel_using_honortitle"),          # 현재 칭호
        "using": "사용중",                                                    # useType_mc "using" tf
        "change": msg("sys.characterinfo.button_change_honortitle"),         # 칭호변경
        "none": msg("sys.honortitle.ui_filter_empty_info"),                  # -
    }

    out = repo / "Data/Titles/HonorTitles.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps({
        "schema": "lostark.honor-titles", "formatVersion": 1,
        "source": {"table": "EFTable_HonorTitle (Type 2 completed titles, Hidden 0)",
                   "names": "EFTable_GameMsg tip.name.honortitle_<pk> (markup stripped)",
                   "strings": "EFTable_GameMsg sys.honortitle.* / sys.characterinfo.*",
                   "project": "ids 100001+ are the project's own titles (no retail row)"},
        "strings": strings,
        "titles": titles,
    }, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print("wrote", out, "(%d titles)" % len(titles))

    layout_out = repo / "Data/UI/HonorTitle/HonorTitle_Layout.json"
    layout_out.parent.mkdir(parents=True, exist_ok=True)
    layout = build_layout()
    layout_out.write_text(json.dumps(layout, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print("wrote", layout_out, "(%d slots)" % len(layout["slots"]))

    art_dir = repo / "Client/Bin/Resources/UI/HonorTitle"
    art_dir.mkdir(parents=True, exist_ok=True)
    for name, stem, x, y, w, h in CHROME:
        Image.open(SHARE / (stem + ".png")).convert("RGBA").crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    atlas = Image.open(HONOR_ATLAS).convert("RGBA")
    for name, x, y, w, h in LOCAL:
        atlas.crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    print("art: %d chrome + %d local -> %s" % (len(CHROME), len(LOCAL), art_dir))
    return 0


if __name__ == "__main__":
    sys.exit(main())
