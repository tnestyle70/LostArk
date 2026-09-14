#!/usr/bin/env python3
"""Build the combat analyzer frame's art, layout and strings from the retail extraction.

Outputs
-------
  Client/Bin/Resources/UI/CombatAnalysis/*.png
      combatanalysisframe.gfx atlas regions (combatAnalysisFrame_I6, 1024x128) at the
      DefineSubImage rectangles the frame's own sprites reference, plus the V2 close button
      from shareImageV2 (the frame's closeBtn is the shared V2closeBtn_*).
  Data/UI/CombatAnalysis/CombatAnalysis_Layout.json
      lostark.ui-layout slots for CCombatAnalysisFrameView: the retail CombatAnalysisFrame
      (470x114 stage px) scaled 2/3 onto 1280x720 and parked bottom-right above the HUD's
      right-hand quick slots.
  Data/UI/CombatAnalysis/CombatAnalysisUi.json
      strings (GameMsg sys.analyzer.* / sys.trainingmode.*) and the four stat tiles the
      project shows (order and category colour index).

Source: D:/ClaudeWork/Extracted/CombatAnalysis (ffdec XML + AS3 + umodel texture), see
.md/TJ/09-14/2026-09-14_전투분석기_PLAN.md section 1 for how each region was found.

Usage:
  python build_combat_analysis_ui.py [--repo <LostArk root>]
"""

from __future__ import annotations

import argparse
import json
import sqlite3
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
ATLAS = EXTRACTED / "CombatAnalysis/tex/EFUI_COMBATANALYSISFRAME/Texture2D/combatanalysisframe_i6.dds"
SHARE_V2 = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"

# (output name, x, y, w, h) in combatAnalysisFrame_I6 -- DefineSubImage ids in comments.
FRAME_ART = [
    ("CA_ControlBox",     477, 0, 235, 32),    # sub 69  CombatAnalysisFrame_ControlBox_BGMc
    ("CA_TitleTile",      589, 63, 116, 26),   # sub 23  CombatAnalysisStatTitleListItem bg
    ("CA_Category_0",     471, 63, 116, 26),   # sub 4   categoryTypeMc frame 1 (default, dark)
    ("CA_Category_1",     714, 0, 116, 26),    # sub 8   frame 2  CUMULATIVE_DAMAGE  (green)
    ("CA_Category_2",     832, 0, 116, 26),    # sub 6   frame 3  CUMULATIVE_DPS     (orange)
    ("CA_Category_3",     707, 63, 116, 26),   # sub 10  frame 4  ONE_MINUTE_DPS     (purple)
    ("CA_Category_4",     825, 63, 116, 26),   # sub 12  frame 5  CRITICAL_PERCENTAGE(blue)
    ("CA_MaximumBtn",     997, 63, 25, 25),    # sub 47  AnimatedButton_EffectYellow_..._Maximum up
    ("CA_MaximumBtn_Over", 970, 63, 25, 25),   # sub 49
    ("CA_OrderIcon",      118, 93, 19, 17),    # sub 57  saveHistory / order arrows (grey)
    ("CA_OrderIcon_Gold", 139, 93, 19, 17),    # sub 59
]
SHARE_ART = [
    ("CA_Close_Normal", "shareimagev2_ib", 785, 1005, 19, 15),   # V2closeBtn_normal
    ("CA_Close_Over",   "shareimagev2_ib", 827, 1005, 19, 15),   # V2closeBtn_over
]

# --- layout -----------------------------------------------------------------------------
RETAIL_SCALE = 2.0 / 3.0
FRAME_W, FRAME_H = 470.0, 114.0
# Bottom-right, above SpecialSkill_1..6 (y 644.7) and right of the mana bar (x <= 932),
# in reference px.
FRAME_REF_X = 1280.0 - FRAME_W * RETAIL_SCALE - 10.0
FRAME_REF_Y = 644.7 - FRAME_H * RETAIL_SCALE - 8.0
TILE_W, TILE_H = 116.0, 26.0
TILE_PITCH = FRAME_W / 4.0          # RollingTileList: four tiles across the 470 px frame
TITLE_ROW_Y, VALUE_ROW_Y = 33.0, 60.0    # statTitleList / userStatList placement


def slot(slot_id, x, y, w, h, path, tint=(1, 1, 1, 1)):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round(FRAME_REF_X + x * RETAIL_SCALE, 4), "y": round(FRAME_REF_Y + y * RETAIL_SCALE, 4),
                 "width": round(w * RETAIL_SCALE, 4), "height": round(h * RETAIL_SCALE, 4)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": list(tint), "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0}, "frames": [], "loop": True, "additive": False},
    }


def build_layout(stats):
    A = "UI/CombatAnalysis/"
    slots = [
        # bg_mc (CombatAnalysisFrame_SizeMc) is a vector fill, not a bitmap: a tinted white.
        slot("CA_Bg", 0, 0, FRAME_W, FRAME_H, "UI/Common/White1x1.png", (0.04, 0.04, 0.05, 0.72)),
        slot("CA_ControlBox", 233, 0, 235, 32, A + "CA_ControlBox.png"),
        slot("CA_OrderIcon", 363 + 4, 7, 19, 17, A + "CA_OrderIcon.png"),
        slot("CA_MaximumBtn", 413, 3, 25, 25, A + "CA_MaximumBtn.png"),
        slot("CA_Close", 443, 9, 19, 15, A + "CA_Close_Normal.png"),
    ]
    for i, stat in enumerate(stats):
        x = TILE_PITCH * i
        slots.append(slot("CA_Title_%d" % i, x, TITLE_ROW_Y, TILE_W, TILE_H, A + "CA_TitleTile.png"))
        slots.append(slot("CA_Value_%d" % i, x, VALUE_ROW_Y, TILE_W, TILE_H, A + "CA_Category_%d.png" % stat["categoryType"]))
    return {"schema": "lostark.ui-layout", "formatVersion": 1,
            "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo

    art_dir = repo / "Client/Bin/Resources/UI/CombatAnalysis"
    art_dir.mkdir(parents=True, exist_ok=True)
    atlas = Image.open(ATLAS).convert("RGBA")
    for name, x, y, w, h in FRAME_ART:
        atlas.crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    for name, stem, x, y, w, h in SHARE_ART:
        Image.open(SHARE_V2 / (stem + ".png")).convert("RGBA").crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))

    g = sqlite3.connect(str(GAMEMSG))
    g.text_factory = bytes

    def msg(key):
        row = g.execute("select MSG from GameMsg where lower(KEY)=lower(?)", (key,)).fetchone()
        return row[0].decode("utf-8", "replace") if row else ""

    # The retail frame shows CUMULATIVE_DAMAGE / CUMULATIVE_DPS / ONE_MINUTE_DPS /
    # CRITICAL_PERCENTAGE (CombatAnalysisCategoryListItem.as). The project shows damage, DPS,
    # stagger dealt and counter successes (user decision), keeping the four category colours
    # in retail order. Titles come from GameMsg where retail has the same label.
    stats = [
        {"id": "damage", "title": msg("sys.trainingmode.damage"), "categoryType": 1, "format": "amount"},
        {"id": "dps", "title": msg("sys.trainingmode.damage_totaldps01"), "categoryType": 2, "format": "amount"},
        # GameMsg only has the bare "무력화" (timeline legend); the user asked for the
        # "dealt" wording on this tile.
        {"id": "stagger", "title": "준 " + msg("sys.analyzer.ui_timeline_legend_paralyze"), "categoryType": 3, "format": "amount"},
        {"id": "counter", "title": msg("sys.analyzer.ui_add_option_counter"), "categoryType": 4, "format": "count"},
    ]
    strings = {
        "title": msg("sys.analyzer.ui_main_title"),
        "time": msg("sys.analyzer.ui_simple_time"),
        "valueNull": msg("sys.analyzer.value_null"),
        "units": [msg("sys.analyzer.ui_unit_%d" % i) for i in (1, 2, 3, 4)],
    }
    data_dir = repo / "Data/UI/CombatAnalysis"
    data_dir.mkdir(parents=True, exist_ok=True)
    (data_dir / "CombatAnalysisUi.json").write_text(json.dumps({
        "schema": "lostark.combat-analysis-ui", "formatVersion": 1,
        "source": {"strings": "EFTable_GameMsg sys.analyzer.* / sys.trainingmode.*",
                   "layout": "combatanalysisframe.gfx CombatAnalysisFrame (470x114 stage px)",
                   "categoryColours": "CombatAnalysisCategoryMc_16 frames 1..5"},
        "strings": strings, "stats": stats,
    }, ensure_ascii=False, indent=1), encoding="utf-8")
    (data_dir / "CombatAnalysis_Layout.json").write_text(
        json.dumps(build_layout(stats), ensure_ascii=False, indent=2), encoding="utf-8")
    print("art: %d frame + %d shared -> %s" % (len(FRAME_ART), len(SHARE_ART), art_dir))
    print("stats:", [(s["id"], s["title"]) for s in stats])
    return 0


if __name__ == "__main__":
    sys.exit(main())
