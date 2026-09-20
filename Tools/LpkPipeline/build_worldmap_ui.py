#!/usr/bin/env python3
"""Build the M-key world map window (zone stage) art, layout and area labels from retail data.

Retail source: EFUI_WORLDMAP worldmap2.gfx (ffdec dump under D:/ClaudeWork/Extracted/MapGfx/worldmap2,
atlas pages exported with umodel under .../MapGfx/WorldmapTex/png). Placement: worldMapWnd2 =
DefaultUIWindow_V2 at stage (230,103) on 1920x1080, content WorldMapWndContent2 1440x768 with the
WorldMapRebornFrame plate 1448x776 at (-4,-4); the zone map itself is a host render target that we
replace with one CUI_Sprite whose UV window scrolls/zooms the area image CMinimapView already uses.
Bottom buttons theWholeWorld_btn (-2,781) / myLocation_btn (36,781) / playerMark_btn (74,781), 37x38.

Outputs
-------
  Client/Bin/Resources/UI/WorldMap/WorldMap_Frame.png        1448x776  (worldmap2_i1a 0,0)
  Client/Bin/Resources/UI/WorldMap/WorldMap_Bg.png           1440x768  (worldmap2_i8 0,0)
  Client/Bin/Resources/UI/WorldMap/WorldMap_Btn{World,MyLocation,PlayerMark}_{Normal,Over,Down}.png
  Data/UI/WorldMap/WorldMap_Layout.json   lostark.ui-layout 1280x720 (retail px * 2/3)
  Data/UI/WorldMap/WorldMapLabels.json    area-name labels per level in retail cm
      (EFTable_MapString X/Y_Location are zone-canvas px of the 1440x768 canvas; the zone image is
      assumed drawn at WorldmapScaleFactor x the 768 px overview, centred with WorldmapShiftFactor --
      px -> image uv -> cm through the MinimapAreas bounds. Calibrate against landmarks in game.)
"""
from __future__ import annotations

import argparse
import json
import math
import re
import sqlite3
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
PAGES = EXTRACTED / "MapGfx/WorldmapTex/png"
TABLES = EXTRACTED / "LpkTables/Map/EFGame_Extra/ClientData/TableData"
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"

RETAIL_SCALE = 2.0 / 3.0
WINDOW_STAGE_X, WINDOW_STAGE_Y = 230.0, 103.0
CONTENT_W, CONTENT_H = 1440.0, 768.0
FRAME_W, FRAME_H = 1448.0, 776.0
BTN_W, BTN_H = 37.0, 38.0
BTN_Y = 781.0
CLOSE_W, CLOSE_H = 19.0, 15.0

# (output name, page stem, x, y, w, h) -- worldmap2.gfx DefineSubImage rectangles.
CROPS = [
    ("WorldMap_Frame", "worldmap2_i1a", 0, 0, 1448, 776),           # WorldMapRebornFrame bitmap 26
    ("WorldMap_Bg", "worldmap2_i8", 0, 0, 1440, 768),               # WorldMapWndContent2 shape 9 bitmap 8
    ("WorldMap_BtnWorld_Normal", "worldmap2_i14", 383, 968, 37, 38),      # WorldMap2_World_btn up
    ("WorldMap_BtnWorld_Over", "worldmap2_i14", 617, 968, 37, 38),        # over
    ("WorldMap_BtnWorld_Down", "worldmap2_i14", 461, 968, 37, 38),        # down
    ("WorldMap_BtnMyLocation_Normal", "worldmap2_i14", 422, 968, 37, 38), # WorldMap2_myLocation_btn
    ("WorldMap_BtnMyLocation_Over", "worldmap2_i14", 500, 968, 37, 38),
    ("WorldMap_BtnMyLocation_Down", "worldmap2_i14", 539, 968, 37, 38),
    ("WorldMap_BtnPlayerMark_Normal", "worldmap2_i14", 578, 968, 37, 38), # WorldMap2_PlayerMarkIcon
    ("WorldMap_BtnPlayerMark_Over", "worldmap2_i14", 975, 368, 37, 38),
    ("WorldMap_BtnPlayerMark_Down", "worldmap2_i14", 344, 968, 37, 38),
    # zone stage chrome (worldmap2.gfx WorldMapWndContent2 children, see gfx_dump_symbol.py)
    ("WorldMap_ZoneNameBg", "worldmap2_i14", 823, 452, 168, 162),          # continentNameBG (compass plate)
    ("WorldMap_PanelHeader", "worldmap2_i14", 632, 825, 302, 81),          # WorldMap2_TreeList header
    ("WorldMap_PanelTile", "worldmap2_i14", 993, 452, 26, 34),             # WorldMap2_TreeList body tile
    ("WorldMap_ToggleSquareHole_Normal", "worldmap2_i14", 823, 798, 25, 24),   # worldMapCheckBox_squareHole
    ("WorldMap_ToggleSquareHole_Over", "worldmap2_i14", 850, 798, 25, 24),
    ("WorldMap_ToggleSquareHole_Selected", "worldmap2_i14", 877, 798, 25, 24),
    ("WorldMap_ToggleLegend_Normal", "worldmap2_i14", 931, 798, 25, 23),       # worldMapCheckBox_legend
    ("WorldMap_ToggleLegend_Over", "worldmap2_i14", 958, 798, 25, 23),
    ("WorldMap_ToggleLegend_Selected", "worldmap2_i14", 985, 798, 25, 23),
    ("WorldMap_CheckOn", "worldmap2_i14", 990, 968, 28, 26),              # legend row check
    ("WorldMap_CheckOff", "worldmap2_i14", 656, 968, 36, 36),             # empty slot frame
    ("WorldMap_Anchor", "worldmap2_i14", 1003, 671, 20, 24),              # SquareHoleRollingTreeItem rootSeaportMc
    ("WorldMap_IconLiner", "worldmap2_i14", 429, 943, 22, 22),            # WorldMap2_oceanBtn2 (voyageLinerBtn)
    ("WorldMap_IconOcean", "worldmap2_i14", 405, 943, 22, 22),            # WorldMap2_oceanBtn
    ("WorldMap_IconMemo", "worldmap2_i14", 453, 943, 22, 22),             # worldMapCheckBox_memoCheckBox
    # minimaplibrary.gfx icon_worldmap_pc: the green pin retail draws at the player's map position.
    ("WorldMap_PlayerPin", "minimaplibrary_i4", 744, 83, 60, 60),
]
# DefaultUIWindow_V2 chrome, the same shared regions the honor title / vehicle windows cut.
SHARE_V2 = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"
CHROME = [
    ("WorldMap_HeaderBg", "shareimagev2_i4", 0, 742, 330, 40),      # WindowBG_V2 top band (title bar)
    ("WorldMap_HeaderDeco", "shareimagev2_ib", 293, 1005, 326, 18),  # V2windowTopDeco_center
    # tree row chevrons (RollingTreeItem arrow_mc = V2step1ListArrow_normal / _selected, turned for open)
    ("WorldMap_TreeArrow_Down", "shareimagev2_ib", 866, 973, 20, 16),
    ("WorldMap_TreeArrow_Up", "shareimagev2_ib", 104, 1005, 23, 19),
    # dialog.gfx AnimatedButton_renew_confirm / _cancel icons (V2BtnIconConfirm_* / V2BtnIconCancel_*)
    ("WorldMap_DlgIconOk_Normal", "shareimagev2_i80", 858, 384, 27, 22),
    ("WorldMap_DlgIconOk_Over", "shareimagev2_ie", 343, 426, 33, 29),
    ("WorldMap_DlgIconCancel_Normal", "shareimagev2_ib", 667, 1005, 21, 18),
    ("WorldMap_DlgIconCancel_Over", "shareimagev2_ie", 157, 458, 29, 28),
    # shareiconimage emoticon_Icon_Silver: the shilling coin next to the square-hole fare
    ("WorldMap_CoinSilver", "shareiconimage_i2", 241, 737, 24, 24),
    # The search button. worldmap2's own search_btn is a vector shape, so there is nothing to cut
    # there; this shared V2 button carries the same plate with the magnifier baked in and matches
    # the retail render (normalised cross correlation 0.97 against the capture). Blue is its
    # pressed state, which is why the earlier crop looked permanently held down.
    ("WorldMap_SearchBtn_Normal", "shareimagev2_ie", 383, 359, 53, 32),
    ("WorldMap_SearchBtn_Over", "shareimagev2_ie", 326, 359, 53, 32),
    # WindowBG_V2 body: the flat dark fill under the title band (page 2,782 - 327,1010). The window
    # runs past the reborn frame to hold the bottom bar, which otherwise floats over the world.
    ("WorldMap_WindowBody", "shareimagev2_i4", 8, 800, 64, 64),
]
# The square-hole confirm dialog (dialog.gfx DialogWindow: 377 wide, YoonGasiIIM 18 #fff7e2 title,
# YG760 14 body, two renew buttons) drawn with the shared V2 window chrome the honor title window cut.
DLG_W, DLG_H = 377.0, 168.0
DLG_BTN_W, DLG_BTN_H = 103.0, 36.0
# Map symbols: EFTable_MapSymbol TextureName/IconIndex -> IconInfo.loa -> EFUI_ICONATLAS_M pages (64x64).
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
SYMBOL_PAGES = EXTRACTED / "MapGfx/MapSymbolAtlas/EFUI_ICONATLAS_M/Texture2D"
SYMBOLS = {
    "WorldMap_Sym_Npc": "Minimap_Symbol_175",        # sys.map.filter_npc_general
    "WorldMap_Sym_Portal": "Minimap_Symbol_7",       # sys.Map.filter_background_portal
    "WorldMap_Sym_SquareHole": "Minimap_Symbol_73",  # sys.Map.filter_background_squarehole
}
# Legend tree (zone map): MapLegend UseZonemap categories in MainCategorySortOrder, one row each.
# (row key, category name string key, icon) -- kind "npc" drives the NPC symbols, "background" the
# portal symbols; the rest are display toggles until their data exists.
LEGEND_ROWS = [
    ("quest", "sys.Map.filter_category_quest", "Minimap_Symbol_12"),
    ("background", "sys.Map.filter_category_background", "Minimap_Symbol_3"),
    ("entrance", "sys.map.filter_category_entrance", "Minimap_Symbol_5"),
    ("npc", "sys.Map.filter_category_npc", "Minimap_Symbol_175"),
    ("item", "sys.map.filter_category_item", "Minimap_Symbol_1_207"),
    ("life", "sys.Map.filter_category_life_level", "Minimap_Symbol_21"),
    ("etc", "sys.map.filter_category_etc", "Minimap_Symbol_1_121"),
    ("character", "sys.Map.filter_category_character", "Minimap_Symbol_1_564"),
]
LEVEL_CONTINENT = {"BERN": "BER", "VALTAN_ARENA": "LUT", "KAKULSAYDON_ARENA": "LUT"}
CONTINENT_PK = {"BER": 11}     # EFTable_ContinentBase row of "베른 북부" (the zone tree's continent title)
SQUAREHOLE_FARE_SHILLING = 38   # capture: 조화의 광장 38 shilling; no currency system yet, display only
PANEL_ROWS = 16
PORTAL_SLOTS = 4
SYMBOL_SLOTS = 40
SYMBOL_PX = 26.0          # WorldMap2_IconSlot content 26x26
ROW_H = 37.0              # WorldMap2_TreeRollingItem
PANEL_X, PANEL_Y = 1128.0, 46.0
PANEL_W, PANEL_H = 304.0, 709.0
PANEL_HEADER_H = 81.0
HEADER_H = 40.0

# Project level -> retail zone. Bounds come from Data/UI/Minimap/MinimapAreas.json (MinimapData vol 0/1).
LEVEL_ZONE = {"BERN": 11102, "VALTAN_ARENA": 37051, "KAKULSAYDON_ARENA": 37081}
# EFTable_MapFactor WorldmapScaleFactor / WorldmapShiftFactorX,Y of the outdoor volume.
MAP_FACTOR = {11102: (1.4, 3.0, -9.0), 37051: (1.0, 0.0, 0.0), 37081: (1.0, 0.0, 0.0)}
OVERVIEW_PX = 768.0     # <Level>_<vol>_Full textures are 768 px on their long side
# Retail draws the zone picture turned 45 deg counter-clockwise on screen (camera forward = up), and the
# MapString px live in that turned canvas. Fitted on 2026-09-17 against a retail Bern capture (20 labels,
# two unambiguous picture features): overview px = 384 + texelsPerCanvasPx * R(+45) * (canvas px - center).
MAP_ROTATION_DEG = 45.0
MAP_FIT = {11102: (1.3457, 710.4, 397.5)}            # zone -> (texels per canvas px, canvas center x, y)
FIT_TEX_PER_PX_UNIT = 1.3457 / 1.4                    # other zones: scaled by their WorldmapScaleFactor


def slot(slot_id, x, y, w, h, path, hover=None):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round((WINDOW_STAGE_X + x) * RETAIL_SCALE, 4),
                 "y": round((WINDOW_STAGE_Y + y) * RETAIL_SCALE, 4),
                 "width": round(w * RETAIL_SCALE, 4), "height": round(h * RETAIL_SCALE, 4)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": hover, "tint": [1, 1, 1, 1],
                    "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def marker_slot(slot_id, size, path):
    """Parked off screen; the view positions it every frame (same as Minimap_*)."""
    s = slot(slot_id, 0, 0, size / RETAIL_SCALE, size / RETAIL_SCALE, path)
    s["rect"]["x"] = -1000
    s["rect"]["y"] = -1000
    return s


def read_strings(keys):
    c = sqlite3.connect(GAMEMSG)
    c.text_factory = bytes
    out = {}
    for k in keys:
        row = c.execute("select MSG from GameMsg where KEY=?", (k,)).fetchone()
        out[k] = row[0].decode("utf-8", "replace") if row else k
    return out


def build_labels(areas):
    c = sqlite3.connect(TABLES / "EFTable_MapString.db")
    c.text_factory = lambda b: b.decode("cp949", "replace")
    levels = []
    for area in areas:
        zone = LEVEL_ZONE.get(area["level"])
        if zone is None:
            continue
        rows = c.execute("select SecondaryKey, X_Location, Y_Location, Name from MapString "
                         "where PrimaryKey=? and UsableDepth=3 order by SecondaryKey", (zone,)).fetchall()
        names = read_strings([r[3] for r in rows])
        scale, shift_x, shift_y = MAP_FACTOR[zone]
        (min_x, min_y), (max_x, max_y) = area["worldMinCm"], area["worldMaxCm"]
        tex_per_px, cx, cy = MAP_FIT.get(zone, (FIT_TEX_PER_PX_UNIT * scale, CONTENT_W * 0.5 + shift_x,
                                                 CONTENT_H * 0.5 + shift_y))
        cos_a, sin_a = math.cos(math.radians(MAP_ROTATION_DEG)), math.sin(math.radians(MAP_ROTATION_DEG))
        labels = []
        for key, px, py, name in rows:
            dx, dy = px - cx, py - cy
            # canvas (turned 45 deg ccw on screen) -> unturned overview px: turn +45 deg (y down)
            tx = OVERVIEW_PX * 0.5 + tex_per_px * (cos_a * dx - sin_a * dy)
            ty = OVERVIEW_PX * 0.5 + tex_per_px * (sin_a * dx + cos_a * dy)
            u, v = tx / OVERVIEW_PX, ty / OVERVIEW_PX   # image right = +y (retail cm), down = -x
            plain, color = split_font_tag(names[name])
            row = {
                "id": int(key), "name": plain, "stringKey": name,
                "canvasPx": [px, py],
                "worldCm": [round(max_x - v * (max_x - min_x), 1), round(min_y + u * (max_y - min_y), 1)],
            }
            if color:
                row["color"] = color
            labels.append(row)
        levels.append({"level": area["level"], "zoneId": zone, "worldmapScaleFactor": scale,
                       "worldmapShiftFactor": [shift_x, shift_y], "mapRotationDegrees": MAP_ROTATION_DEG,
                       "canvasToOverview": {"texelsPerCanvasPx": tex_per_px, "canvasCenter": [cx, cy]},
                       "labels": labels})
    return levels


def split_font_tag(text):
    """'<FONT COLOR='#FF73D1FF'>name</FONT>' -> ('name', '#73D1FF'); plain names -> (name, None)."""
    m = re.match(r"<FONT COLOR='#([0-9A-Fa-f]{8})'>(.*?)</FONT>", text)
    if not m:
        return re.sub(r"<[^>]+>", "", text), None
    return m.group(2), "#" + m.group(1)[2:].upper()

# Square holes: no placement data exists in the project yet, so the three Bern holes are read off the
# retail world map capture (2026-09-17): capture px -> overview texture px through the same fit the
# labels use (texture (660,400) <-> capture (950,238), scale 1.0582, turn -43.19 deg), then -> cm.
# Replace with real placements once the map owner authors them.
SQUARE_HOLES_REF = {"BERN": [("\uc870\ud654\uc758 \uad11\uc7a5", 852, 294), ("\uc0c1\uc5c5 \uc9c0\uad6c", 465, 313),
                             ("\uc81c\uc791 \uc9c0\uad6c", 906, 650)]}
REF_FIT = {"scale": 1.0582, "rotDeg": -43.19, "texAnchor": (660.0, 400.0), "shotAnchor": (950.0, 238.0)}
# NPC placement id -> map symbol (retail EFTable_Npc.MapSymbolIndex -> MapSymbol icon). The item upgrade
# NPC (npc.bern.schmidt) is the project's own function NPC and keeps the retail item-enhance hammer
# (sys.map.filter_item_enhance). Every other Bern NPC with an original MapSymbolIndex is in the checked-in
# Data/UI/WorldMap/WorldMapNpcSymbols.json, which is the source of truth for the assignment (resolved from
# the original EFTable_Npc / EFTable_MapSymbol rows, 09-19); this tool keeps that file and only cuts the icons.
NPC_SYMBOLS_DOC = "Data/UI/WorldMap/WorldMapNpcSymbols.json"
NPC_SYMBOL_DEFAULT = {"npc.bern.schmidt": "Minimap_Symbol_1_207"}   # sys.map.filter_item_enhance


def load_npc_symbols(repo):
    """placement id -> icon stem, from the checked-in document; the hammer default if it is missing."""
    path = repo / NPC_SYMBOLS_DOC
    if not path.is_file():
        return dict(NPC_SYMBOL_DEFAULT)
    placements = json.loads(path.read_text(encoding="utf-8-sig")).get("placements", {})
    return {pid: Path(icon).stem for pid, icon in placements.items()}


NPC_SYMBOLS = dict(NPC_SYMBOL_DEFAULT)
# The player's action casting bar (song play, item use): commonobject.gfx CommonActionTimingBar, matched
# against a retail capture on 2026-09-17 -- dark pointed plate (bitmap 1058, 379x44, placed at (85,-11) in the
# symbol), orange-red progress fill (bitmap 1060, 352x22, `track` at (99,0), masked by the value) and the
# white $YG760 13 px caption `nameTF` centred over the track. The other children (perfect zone, marks,
# success/fail bursts) belong to the timing minigame and are not placed. Pages: umodel export of
# EFUI_COMMONOBJECT (D:/ClaudeWork/Extracted/CommonObjectGfx). interruptskill.gfx, tried before, is the
# boss interruptible-skill (stagger) gauge and must not be used here.
CAST_BAR_PAGES = EXTRACTED / "CommonObjectGfx/png"
CAST_BAR = [("CastBar_Plate", "commonobject_i12", 452, 616, 379, 44),   # DefineSubImage 1058
            ("CastBar_Fill", "commonobject_i12", 354, 775, 352, 22)]    # DefineSubImage 1060
CAST_PLATE_W, CAST_PLATE_H = 379.0, 44.0
CAST_FILL_W, CAST_FILL_H = 352.0, 22.0
CAST_FILL_OFFSET = (99.0 - 85.0, 0.0 + 11.0)   # fill origin inside the plate
HOLE_SLOTS = 4
NPC_SYMBOL_SLOTS = 48   # Bern places 38 NPCs with an original symbol; keep equal to NPC_SYMBOL_SLOT_COUNT in WorldMapWindowView.cpp


def msg(conn, key):
    row = conn.execute("select MSG from GameMsg where lower(KEY)=lower(?)", (key,)).fetchone()
    return row[0].decode("utf-8", "replace") if row else key


def shot_to_world(px, py, area):
    s, a = REF_FIT["scale"], math.radians(-REF_FIT["rotDeg"])
    dx, dy = (px - REF_FIT["shotAnchor"][0]) / s, (py - REF_FIT["shotAnchor"][1]) / s
    tx = REF_FIT["texAnchor"][0] + math.cos(a) * dx - math.sin(a) * dy
    ty = REF_FIT["texAnchor"][1] + math.sin(a) * dx + math.cos(a) * dy
    u, v = tx / OVERVIEW_PX, ty / OVERVIEW_PX
    (min_x, min_y), (max_x, max_y) = area["worldMinCm"], area["worldMaxCm"]
    return [round(max_x - v * (max_x - min_x), 1), round(min_y + u * (max_y - min_y), 1)]


def build_square_holes(areas):
    levels = []
    for area in areas:
        refs = SQUARE_HOLES_REF.get(area["level"])
        if not refs:
            continue
        holes = []
        for index, (name, px, py) in enumerate(refs, start=1):
            cm = shot_to_world(px, py, area)
            holes.append({"id": index, "name": name, "worldCm": cm,
                          "clientM": [round(cm[0] / 100.0, 2), round(-cm[1] / 100.0, 2)], "capturePx": [px, py],
                          "fareShilling": SQUAREHOLE_FARE_SHILLING})
        levels.append({"level": area["level"], "holes": holes})
    return levels


def build_panels(areas):
    g = sqlite3.connect(GAMEMSG)
    g.text_factory = bytes
    legend = [{"key": key, "name": msg(g, name_key), "icon": "UI/WorldMap/Symbols/%s.png" % icon}
              for key, name_key, icon in LEGEND_ROWS]
    z = sqlite3.connect(TABLES / "EFTable_ZoneBase.db")
    z.text_factory = lambda b: b.decode("cp949", "replace")
    cb = sqlite3.connect(TABLES / "EFTable_ContinentBase.db")
    cb.text_factory = lambda b: b.decode("cp949", "replace")
    zones = {}
    for area in areas:
        continent = LEVEL_CONTINENT.get(area["level"])
        if continent is None:
            continue
        # The retail tree lists the continent's world-map zones in ZoneBase.SortOrder; ship decks (Type 16,
        # StreamingParent = their port) are not rows but mark their port with the anchor icon.
        rows = z.execute("select PrimaryKey, Name, SortOrder from ZoneBase where Continent=? and AbleToUseWorldMap=1 "
                         "and UI_Xlocation!=0 and Type!=16 order by SortOrder, PrimaryKey", (continent,)).fetchall()
        ports = {r[0] for r in z.execute("select StreamingParent from ZoneBase where Continent=? and Type=16", (continent,))}
        continent_name = ""
        cont_row = cb.execute("select Name from ContinentBase where PrimaryKey=?", (CONTINENT_PK.get(continent, -1),)).fetchone()
        if cont_row:
            continent_name = msg(g, cont_row[0])
        zones[area["level"]] = {"continent": continent, "continentName": continent_name,
                                "currentZoneId": LEVEL_ZONE.get(area["level"]),
                                "rows": [{"zoneId": pk, "name": msg(g, name), "isPort": pk in ports} for pk, name, _ in rows]}
    strings = {"windowTitle": "\uc6d4\ub4dc\ub9f5", "legendTitle": msg(g, "sys.map.title_legend"),
               "squareHoleTitle": msg(g, "sys.map.filter_background_squarehole"),
               "searchHint": msg(g, "sys.map.search_inputbox_desc"),
               "linerButton": msg(g, "sys.voyage.ui_worldmap_liner_btn"),
               "oceanButton": msg(g, "sys.squarehole.direct_departure_btn_worldmap"),
               "memoButton": msg(g, "sys.map.memo_btn"),
               "dialogTitle": msg(g, "sys.squarehole.fla_squire_hall"),
               # "<FONT COLOR='#FFFF00'>{0}</FONT>로 이동하시겠습니까?" -- the view colours {0} gold and picks 로/으로.
               "confirmFormat": re.sub(r"<[^>]+>", "", msg(g, "sys.squarehole.go_questions")),
               "confirmOk": "\ud655\uc778", "confirmCancel": "\ucde8\uc18c",
               "songCasting": "\uc2a4\ud018\uc5b4\ud640\uc758 \ub178\ub798\ub97c \uc5f0\uc8fc\ud558\ub294 \uc911"}
    return {"strings": strings, "legend": legend, "zones": zones}


def cut_symbols(art_dir):
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from build_vehicle_ui import iconinfo_lookup  # noqa: E402
    data = ICONINFO.read_bytes()
    out_dir = art_dir / "Symbols"
    out_dir.mkdir(parents=True, exist_ok=True)
    names = set(SYMBOLS.values()) | {icon for _, _, icon in LEGEND_ROWS} | set(NPC_SYMBOLS.values())
    pages = {}
    for name in sorted(names):
        found = iconinfo_lookup(data, name)
        if not found:
            raise SystemExit("IconInfo has no %s" % name)
        page, x, y, w, h = found
        if page not in pages:
            pages[page] = Image.open(SYMBOL_PAGES / (page.lower() + ".dds")).convert("RGBA")
        pages[page].crop((x, y, x + w, y + h)).save(out_dir / (name + ".png"))
    return len(names)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo

    art_dir = repo / "Client/Bin/Resources/UI/WorldMap"
    art_dir.mkdir(parents=True, exist_ok=True)
    pages = {}
    for name, stem, x, y, w, h in CROPS:
        if stem not in pages:
            pages[stem] = Image.open(PAGES / (stem + ".png")).convert("RGBA")
        pages[stem].crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    for name, stem, x, y, w, h in CHROME:
        if stem not in pages:
            pages[stem] = Image.open(SHARE_V2 / (stem + ".png")).convert("RGBA")
        pages[stem].crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    for name, stem, x, y, w, h in CAST_BAR:
        if stem not in pages:
            pages[stem] = Image.open(CAST_BAR_PAGES / (stem + ".png")).convert("RGBA")
        pages[stem].crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    NPC_SYMBOLS.clear()
    NPC_SYMBOLS.update(load_npc_symbols(repo))
    symbol_count = cut_symbols(art_dir)

    a = "UI/WorldMap/"
    sym = a + "Symbols/"
    header_y = -4.0 - HEADER_H
    slots = [
        # The window itself: from the title band down past the bottom bar, so that row reads as part
        # of the window instead of floating over the world.
        slot("WM_WindowBg", -4, header_y, FRAME_W, BTN_Y + BTN_H + 6 - header_y,
             a + "WorldMap_WindowBody.png"),
        # DefaultUIWindow_V2 title band above the reborn frame; the close X sits in it.
        slot("WM_HeaderBg", -4, header_y, FRAME_W, HEADER_H, a + "WorldMap_HeaderBg.png"),
        slot("WM_HeaderDeco", (CONTENT_W - 326) * 0.5, header_y + (HEADER_H - 18) * 0.5, 326, 18, a + "WorldMap_HeaderDeco.png"),
        slot("WM_Frame", -4, -4, FRAME_W, FRAME_H, a + "WorldMap_Frame.png"),
        slot("WM_Bg", 0, 0, CONTENT_W, CONTENT_H, a + "WorldMap_Bg.png"),
        # The zone image: one sprite over the whole content rect, UV window set by the view.
        slot("WM_Map", 0, 0, CONTENT_W, CONTENT_H, "UI/Minimap/Maps/BernCastle.png"),
    ]
    # Map symbols (square holes, NPC functions): parked, placed by the view like the markers.
    for i in range(HOLE_SLOTS):
        slots.append(marker_slot("WM_Hole_%d" % i, SYMBOL_PX * RETAIL_SCALE, sym + SYMBOLS["WorldMap_Sym_SquareHole"] + ".png"))
    for i in range(NPC_SYMBOL_SLOTS):
        slots.append(marker_slot("WM_NpcSym_%d" % i, SYMBOL_PX * RETAIL_SCALE, sym + SYMBOLS["WorldMap_Sym_Npc"] + ".png"))
    slots += [
        marker_slot("WM_Boss_0", 22, "UI/Minimap/marker_boss.png"),
        marker_slot("WM_Boss_1", 22, "UI/Minimap/marker_boss.png"),
        marker_slot("WM_Party_0", 20, "UI/Minimap/marker_party.png"),
        marker_slot("WM_Party_1", 20, "UI/Minimap/marker_party.png"),
        marker_slot("WM_Party_2", 20, "UI/Minimap/marker_party.png"),
        marker_slot("WM_Party_3", 20, "UI/Minimap/marker_party.png"),
        marker_slot("WM_Player", 60 * RETAIL_SCALE, a + "WorldMap_PlayerPin.png"),
    ]
    # Portal (zone-transfer) symbols beside the blue zone labels.
    for i in range(PORTAL_SLOTS):
        slots.append(marker_slot("WM_Portal_%d" % i, SYMBOL_PX * RETAIL_SCALE, sym + SYMBOLS["WorldMap_Sym_Portal"] + ".png"))
    slots += [
        # continentNameBG compass rose: faint, lower right of the map in the captures (drawn small).
        slot("WM_Compass", 1225, 683, 100, 96, a + "WorldMap_ZoneNameBg.png"),
        # searchInput_txt (930,9) stretched to the search button at (1255,9); display only.
        slot("WM_SearchInput", 930, 9, 1255 - 930 - 4, 32, "UI/AvatarBook/textinput.png"),
        slot("WM_SearchBtn", 1255, 9, 53, 32, a + "WorldMap_SearchBtn_Normal.png",
             a + "WorldMap_SearchBtn_Over.png"),
        # worldMapCheckBox_squareHole (1323,9) / worldMapCheckBox_legend (1378,9): plate + icon toggles.
        # The toggles' own plates are vector too; the shared V2 renew button is the same slate plate.
        slot("WM_ToggleSquareHoleBg", 1323, 9, 53, 32, "UI/HonorTitle/HonorTitle_Btn_Normal.png",
             "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_ToggleLegendBg", 1378, 9, 53, 32, "UI/HonorTitle/HonorTitle_Btn_Normal.png",
             "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_ToggleSquareHole", 1337, 12, 25, 24, a + "WorldMap_ToggleSquareHole_Normal.png", a + "WorldMap_ToggleSquareHole_Over.png"),
        slot("WM_ToggleLegend", 1394, 13, 25, 23, a + "WorldMap_ToggleLegend_Normal.png", a + "WorldMap_ToggleLegend_Over.png"),
        # WorldMap2_TreeList / TreeListSquareHole at (1131,49): body tile + header, then the rows.
        slot("WM_PanelTile", PANEL_X, PANEL_Y, PANEL_W, PANEL_H, a + "WorldMap_PanelTile.png"),
        slot("WM_PanelHeader", PANEL_X + 1, PANEL_Y, 302, PANEL_HEADER_H, a + "WorldMap_PanelHeader.png"),
    ]
    for i in range(PANEL_ROWS):
        row_y = PANEL_Y + 3 + PANEL_HEADER_H + i * ROW_H
        slots.append(slot("WM_RowOver_%d" % i, PANEL_X + 3, row_y, 279, 35, "UI/HonorTitle/HonorTitle_Row_Over.png"))
        slots.append(slot("WM_RowCheck_%d" % i, PANEL_X + 3 + 6, row_y + 4, 26, 26, a + "WorldMap_CheckOn.png"))
        slots.append(slot("WM_RowIcon_%d" % i, PANEL_X + 3 + 34, row_y + 4, 26, 26, sym + SYMBOLS["WorldMap_Sym_Npc"] + ".png"))
        slots.append(slot("WM_RowAnchor_%d" % i, PANEL_X + 3 + 222, row_y + 6, 20, 24, a + "WorldMap_Anchor.png"))
        slots.append(slot("WM_RowArrow_%d" % i, PANEL_X + 3 + 257, row_y + 11, 20, 16, a + "WorldMap_TreeArrow_Down.png"))
    slots += [
        # bottom bar: voyageLinerBtn (464,781), oceanBtn (722,781), memoCheckBox (1252,781); the shared renew
        # button plate with each symbol's own icon. Display only -- there is no voyage or memo system.
        slot("WM_BtnLiner", 464, 781, 250, 36, "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_IconLiner", 475, 788, 22, 22, a + "WorldMap_IconLiner.png"),
        slot("WM_BtnOcean", 722, 781, 250, 36, "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_IconOcean", 733, 788, 22, 22, a + "WorldMap_IconOcean.png"),
        slot("WM_BtnMemo", 1252, 781, 166, 36, "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_IconMemo", 1262, 788, 22, 22, a + "WorldMap_IconMemo.png"),
        slot("WM_BtnWorld", -2, BTN_Y, BTN_W, BTN_H, a + "WorldMap_BtnWorld_Normal.png", a + "WorldMap_BtnWorld_Over.png"),
        slot("WM_BtnMyLocation", 36, BTN_Y, BTN_W, BTN_H, a + "WorldMap_BtnMyLocation_Normal.png", a + "WorldMap_BtnMyLocation_Over.png"),
        slot("WM_BtnPlayerMark", 74, BTN_Y, BTN_W, BTN_H, a + "WorldMap_BtnPlayerMark_Normal.png", a + "WorldMap_BtnPlayerMark_Over.png"),
        # DefaultUIWindow_V2 close X in the title band: same shared crop the honor title window uses.
        slot("WM_Close", CONTENT_W - CLOSE_W - 10, header_y + (HEADER_H - CLOSE_H) * 0.5, CLOSE_W, CLOSE_H,
             "UI/HonorTitle/HonorTitle_Close_Normal.png", "UI/HonorTitle/HonorTitle_Close_Over.png"),
    ]
    # Square-hole confirm dialog, centred on the content; drawn last so it sits over everything.
    dlg_x, dlg_y = (CONTENT_W - DLG_W) * 0.5, (CONTENT_H - DLG_H) * 0.5
    btn_y = dlg_y + DLG_H - DLG_BTN_H - 10
    slots += [
        slot("WM_DlgBg", dlg_x, dlg_y, DLG_W, DLG_H, "UI/HonorTitle/HonorTitle_WindowBg.png"),
        # The divider sits under the title, not through it: measured centre 35 of the 168-tall plate.
        slot("WM_DlgDeco", dlg_x + (DLG_W - 326) * 0.5, dlg_y + 26, 326, 18, "UI/HonorTitle/HonorTitle_TopDeco.png"),
        slot("WM_DlgCoin", dlg_x + DLG_W * 0.5 + 8, dlg_y + 78, 24, 24, a + "WorldMap_CoinSilver.png"),
        slot("WM_DlgBtnOk", dlg_x + DLG_W * 0.5 - DLG_BTN_W - 6, btn_y, DLG_BTN_W, DLG_BTN_H,
             "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_DlgIconOk", dlg_x + DLG_W * 0.5 - DLG_BTN_W - 6 + 10, btn_y + 7, 27, 22, a + "WorldMap_DlgIconOk_Normal.png", a + "WorldMap_DlgIconOk_Over.png"),
        slot("WM_DlgBtnCancel", dlg_x + DLG_W * 0.5 + 6, btn_y, DLG_BTN_W, DLG_BTN_H,
             "UI/HonorTitle/HonorTitle_Btn_Normal.png", "UI/HonorTitle/HonorTitle_Btn_Over.png"),
        slot("WM_DlgIconCancel", dlg_x + DLG_W * 0.5 + 6 + 13, btn_y + 9, 21, 18, a + "WorldMap_DlgIconCancel_Normal.png", a + "WorldMap_DlgIconCancel_Over.png"),
    ]
    data_dir = repo / "Data/UI/WorldMap"
    data_dir.mkdir(parents=True, exist_ok=True)

    def write(name, doc, indent=2):
        (data_dir / name).write_text(json.dumps(doc, ensure_ascii=False, indent=indent) + "\n", encoding="utf-8")

    write("WorldMap_Layout.json", {
        "schema": "lostark.ui-layout", "formatVersion": 1,
        "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots,
    })
    # The song gauge is its own small document: it shows in the world after the map closes.
    # CommonActionTimingBar at the retail default UI scale: the plate is centred horizontally and its
    # centre sits at 67.4% of the screen height (user's in-game capture); the fill is the plate-relative `track`.
    plate_x = (1920.0 - CAST_PLATE_W) * 0.5
    plate_y = 1080.0 * 0.674 - CAST_PLATE_H * 0.5
    plate_slot = slot("SC_Plate", 0, 0, CAST_PLATE_W, CAST_PLATE_H, a + "CastBar_Plate.png")
    plate_slot["rect"] = {"x": round(plate_x * RETAIL_SCALE, 4), "y": round(plate_y * RETAIL_SCALE, 4),
                          "width": round(CAST_PLATE_W * RETAIL_SCALE, 4), "height": round(CAST_PLATE_H * RETAIL_SCALE, 4)}
    fill_slot = slot("SC_Fill", 0, 0, CAST_FILL_W, CAST_FILL_H, a + "CastBar_Fill.png")
    fill_slot["rect"] = {"x": round((plate_x + CAST_FILL_OFFSET[0]) * RETAIL_SCALE, 4),
                         "y": round((plate_y + CAST_FILL_OFFSET[1]) * RETAIL_SCALE, 4),
                         "width": round(CAST_FILL_W * RETAIL_SCALE, 4), "height": round(CAST_FILL_H * RETAIL_SCALE, 4)}
    write("SongCastGauge_Layout.json", {
        "schema": "lostark.ui-layout", "formatVersion": 1,
        "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": [plate_slot, fill_slot],
    })

    areas = json.loads((repo / "Data/UI/Minimap/MinimapAreas.json").read_text(encoding="utf-8"))["areas"]
    labels = build_labels(areas)
    write("WorldMapLabels.json", {
        "schema": "lostark.worldmap-labels", "formatVersion": 1,
        "source": "EFTable_MapString UsableDepth 3 (zone stage) + GameMsg tip.name.area_*; worldCm derived from the "
                  "1440x768 zone canvas through WorldmapScaleFactor/Shift and the MinimapAreas bounds (calibrate in game)",
        "levels": labels,
    }, indent=1)
    holes = build_square_holes(areas)
    write("WorldMapSquareHoles.json", {
        "schema": "lostark.worldmap-squareholes", "formatVersion": 1,
        "source": "positions read off the retail Bern world map capture through the label fit (REF_FIT); "
                  "replace with authored placements when the map owner adds square holes",
        "symbol": "UI/WorldMap/Symbols/%s.png" % SYMBOLS["WorldMap_Sym_SquareHole"],
        "levels": holes,
    })
    write("WorldMapNpcSymbols.json", {
        "schema": "lostark.worldmap-npc-symbols", "formatVersion": 1,
        "source": "EFTable_Npc.MapSymbolIndex -> EFTable_MapSymbol -> IconInfo; keyed by Gameplay.world.json placementId",
        "placements": {pid: "UI/WorldMap/Symbols/%s.png" % icon for pid, icon in NPC_SYMBOLS.items()},
    })
    write("WorldMapPanels.json", {"schema": "lostark.worldmap-panels", "formatVersion": 1, **build_panels(areas)})
    print("art:", len(CROPS) + len(CHROME) + len(CAST_BAR), "+ symbols", symbol_count, "->", art_dir,
          "| layout slots:", len(slots), "| labels:", [(l["level"], len(l["labels"])) for l in labels],
          "| holes:", [(h["level"], len(h["holes"])) for h in holes])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
