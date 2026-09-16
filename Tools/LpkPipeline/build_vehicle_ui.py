#!/usr/bin/env python3
"""Build the vehicle window's data document and crop its art from the retail atlases.

Outputs
-------
  Data/UI/Vehicle/VehicleUiCatalog.json
      one row per vehicle in Data/Actors/VehicleCatalog.json: id, Korean name and
      description (EFTable_GameMsg), move speed (EFTable_Vehicle), icon asset id.
  Client/Bin/Resources/UI/Vehicle/Icons/vehicle_<id>.png
      the vehicle's own icon, cut from the EFUI_ICONATLAS_{V,N} page IconInfo.loa
      names for `<Icon>_<IconIndex>` in EFTable_Vehicle.
  Client/Bin/Resources/UI/Vehicle/Skills/skill_<skillId>.png
      the vehicle's skill icons (Data/Vehicles/VehicleProfiles.json skills[] --
      SPACE/Q/W/E, the Server contract -- -> EFTable_Skill Icon/IconIndex ->
      IconInfo.loa), listed per vehicle in the catalog as
      `skills[{slot, skillId, cooldownMs, iconAsset}]` for the mounted HUD.
  Client/Bin/Resources/UI/Vehicle/*.png
      window chrome and list art, cut at the DefineSubImage regions vehicle.gfx and
      its shared componentsV2 / shareImageV2 dependencies resolve to.
  Data/UI/Vehicle/Vehicle_Layout.json
      lostark.ui-layout slots for CVehicleWindowView: the retail vehicleWnd placement
      (DefaultUIWindow_V2 at stage (1376,186), 480 px wide) with the four list rows,
      mount/close buttons and window chrome, scaled 2/3 onto the 1280x720 reference.
      The tabs / search / sort / summon-motion parts of VehicleWndContent are not
      placed (project scope: list, mount/dismount, mounted HUD).

Every crop below is a region the gfx itself names -- nothing is eyeballed. The
sources and how each was found are in
.md/TJ/09-14/2026-09-14_탈것UI_전수추출.md.

Inputs (all already extracted on this machine)
  D:/ClaudeWork/Extracted/Vehicle/tables/**/EFTable_Vehicle.db
  D:/ClaudeWork/Extracted/LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db
  D:/ClaudeWork/Extracted/Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa
  D:/ClaudeWork/Extracted/Vehicle/icons/EFUI_ICONATLAS_{V,N}/Texture2D/<page>.{dds,tga}
  D:/ClaudeWork/Extracted/Vehicle/tex/EFUI_VEHICLE/Texture2D/vehicle_i5.dds
  D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D/*.png

Usage:
  python build_vehicle_ui.py [--repo <LostArk root>]
"""

from __future__ import annotations

import argparse
import glob
import json
import sqlite3
import struct
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
ICON_PAGES = EXTRACTED / "Vehicle/icons"
VEHICLE_ATLAS = EXTRACTED / "Vehicle/tex/EFUI_VEHICLE/Texture2D/vehicle_i5.dds"
SHARE = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"
SHARE_V1 = EXTRACTED / "ShareImageGfx_Extracted/tex/EFUI_SHAREIMAGE"
# quickslot.gfx slot sprite 564 places lockMc (-> shareimage `icon_lock_01`, shareImage_I5
# (689,492) 21x27) at twips (316,220) = px (15.8,11) inside the 52x52 slot art; the mounted HUD
# shows that padlock on every key without a vehicle action. Composed onto a 52x52 canvas so it
# lands on the slot the way the retail one does when Set_SlotTexture stretches it to the slot.
LOCK_SLOT_SIZE = 52
LOCK_ICON = ("shareimage_i5", 689, 492, 21, 27, 16, 11)

# (output name, page stem, x, y, w, h) -- DefineSubImage regions, see the doc.
CHROME = [
    # DefaultUIWindow_V2 -> WindowBG_V2 -> V2windowBackground
    ("Vehicle_WindowBg",       "shareimagev2_i4",  0, 742, 330, 276),
    # V2WindowDeco -> V2windowTopDeco_center
    ("Vehicle_TopDeco",        "shareimagev2_ib",  293, 1005, 326, 18),
    # s_closeBtn_V2 -> V2closeBtn_*
    ("Vehicle_Close_Normal",   "shareimagev2_ib",  785, 1005, 19, 15),
    ("Vehicle_Close_Over",     "shareimagev2_ib",  827, 1005, 19, 15),
    ("Vehicle_Close_Down",     "shareimagev2_ib",  848, 1005, 19, 15),
    # AnimatedButton_renew_V2 -> V2btn_*
    ("Vehicle_Btn_Normal",     "shareimagev2_i46", 643, 988, 103, 36),
    ("Vehicle_Btn_Over",       "shareimagev2_i46", 328, 988, 103, 36),
    ("Vehicle_Btn_Down",       "shareimagev2_ie",  532, 173, 103, 36),
    ("Vehicle_Btn_Disabled",   "shareimagev2_i46", 853, 988, 103, 36),
    # V2step2List_* -- list row hover/selected skins vehicle.gfx places directly
    ("Vehicle_Row_Over",       "shareimagev2_ie",  790, 212, 206, 35),
    ("Vehicle_Row_Selected",   "shareimagev2_i2",  771, 986, 208, 37),
    # arkSlot_renew_basic_V2 -> arkSlot_renew_basic_Frame_V2 -> V2Slot_border
    ("Vehicle_SlotBorder",     "shareimagev2_i6",  863, 555, 70, 70),
    # Vehicle_bookMarkBtn -> shared_favoritesBtnM_*
    ("Vehicle_Star_Normal",    "shareimagev2_ie",  614, 517, 24, 22),
    ("Vehicle_Star_Over",      "shareimagev2_ie",  640, 517, 22, 22),
    ("Vehicle_Star_Selected",  "shareimagev2_ie",  585, 488, 26, 26),
]
# vehicle.gfx's own page Vehicle_I5 (1024x128), DefineSubImage ids 77 / 63 / 60.
LOCAL = [
    ("Vehicle_RowBg",          0,   0, 352, 60),   # VehicleListItem depth 1
    ("Vehicle_EquipRibbon",    354, 0, 163, 60),   # equipMc -- "(탑승중)" ribbon
    ("Vehicle_BookmarkBg",     519, 0, 276, 58),   # bookmarkBG_mc
]


# --- Vehicle_Layout.json -------------------------------------------------------------
# Retail px relative to the window's top-left (VehicleWndContent origin == window origin).
# The window is drawn 1.2x larger than the plain 2/3 stage fit (user request: too small at
# 1280x720); CVehicleWindowView reads the effective scale back from the VH_WinBg slot width.
WINDOW_SCALE = 1.2
RETAIL_SCALE = 2.0 / 3.0 * WINDOW_SCALE
WINDOW_W = 480.0                                   # WindowBG_V2 330 * 1.455
WINDOW_H = 60.0 + 60.0 * 6 + 10.0 + 30.0 + 50.0    # header + rows + hint + buttons (= 510)
# vehicleWnd sits at stage (1376,186): keep that top edge and its right margin (1920-1376-480)
# so the enlarged window still ends where the retail one did.
WINDOW_STAGE_Y = 186.0
WINDOW_STAGE_X = 1920.0 - (1920.0 - 1376.0 - 480.0) - WINDOW_W * WINDOW_SCALE
# vehicleList (14,165), item 402x60 in retail; rows widened to the window's right padding
# (user request) so the list fills the panel.
ROW_X, ROW_Y0, ROW_PITCH, ROW_H = 14.0, 60.0, 60.0, 60.0
ROW_W = WINDOW_W - ROW_X * 2
ROW_COUNT = 6                                      # every Data/Actors/VehicleCatalog.json vehicle
HINT_Y = ROW_Y0 + ROW_PITCH * ROW_COUNT + 10.0     # desc_lb, under the last row
BUTTON_Y = HINT_Y + 30.0                           # confirmBtn / closeBtnDummy row


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


def build_layout(rows):
    A = "UI/Vehicle/"
    slots = [
        layout_slot("VH_WinBg", 0, 0, WINDOW_W, WINDOW_H, A + "Vehicle_WindowBg.png"),
        # V2WindowDeco (77,30) in retail; under the title in the shorter header band here.
        layout_slot("VH_TopDeco", 77, 40, 326, 18, A + "Vehicle_TopDeco.png"),
        layout_slot("VH_Close", WINDOW_W - 19 - 12, 10, 19, 15, A + "Vehicle_Close_Normal.png"),
    ]
    for i, row in enumerate(rows):
        y = ROW_Y0 + ROW_PITCH * i
        slots += [
            # VehicleListItem depth 1 / over_mc / selected_mc / equipMc / slot(36,6)x0.71 / star(4,16)
            layout_slot("VH_Row%d_Bg" % i, ROW_X, y, ROW_W, ROW_H, A + "Vehicle_RowBg.png"),
            layout_slot("VH_Row%d_Over" % i, ROW_X, y, ROW_W, ROW_H, A + "Vehicle_Row_Over.png"),
            layout_slot("VH_Row%d_Selected" % i, ROW_X, y, ROW_W, ROW_H, A + "Vehicle_Row_Selected.png"),
            layout_slot("VH_Row%d_Equip" % i, ROW_X, y + 1, 163, 60, A + "Vehicle_EquipRibbon.png"),
            layout_slot("VH_Row%d_Icon" % i, ROW_X + 39, y + 9, 44, 44, row["iconAsset"]),
            layout_slot("VH_Row%d_Slot" % i, ROW_X + 36, y + 6, 50, 50, A + "Vehicle_SlotBorder.png"),
            layout_slot("VH_Row%d_Star" % i, ROW_X + 4, y + 16, 24, 22, A + "Vehicle_Star_Normal.png"),
        ]
    slots += [
        layout_slot("VH_MountBtn", 259, BUTTON_Y, 103, 36, A + "Vehicle_Btn_Normal.png"),
        layout_slot("VH_CloseBtn", 365, BUTTON_Y, 103, 36, A + "Vehicle_Btn_Normal.png"),
    ]
    return {"schema": "lostark.ui-layout", "formatVersion": 1,
            "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}


def iconinfo_lookup(data: bytes, name: str):
    """(page, x, y, w, h) for `<name>.png` in IconInfo.loa, or None."""
    needle = (name + ".png").encode()
    i = data.lower().find(needle.lower())
    if i < 0:
        return None
    n = struct.unpack_from("<i", data, i - 4)[0]
    j = i + n
    plen = struct.unpack_from("<i", data, j)[0]
    page = data[j + 4:j + 4 + plen].split(b"\0")[0].decode()
    x, y, w, h = struct.unpack_from("<4i", data, j + 4 + plen)
    return page, x, y, w, h


def open_page(stem: str) -> Image.Image:
    hits = glob.glob(str(ICON_PAGES / "**" / (stem + ".*")), recursive=True)
    hits = [h for h in hits if h.lower().endswith((".dds", ".tga", ".png"))]
    if not hits:
        raise SystemExit("icon page %s not extracted" % stem)
    return Image.open(hits[0]).convert("RGBA")


def msg(conn, key: str) -> str:
    row = conn.execute("select MSG from GameMsg where lower(KEY)=lower(?)", (key,)).fetchone()
    return row[0].decode("utf-8", "replace") if row else ""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    repo = args.repo

    catalog = json.loads((repo / "Data/Actors/VehicleCatalog.json").read_text(encoding="utf-8"))
    profiles = {int(v["vehicleId"]): v for v in
                json.loads((repo / "Data/Vehicles/VehicleProfiles.json").read_text(encoding="utf-8"))["vehicles"]}
    vehicle_db = glob.glob(str(EXTRACTED / "Vehicle/tables/**/EFTable_Vehicle.db"), recursive=True)[0]
    vconn = sqlite3.connect(vehicle_db)
    sconn = sqlite3.connect(str(Path(vehicle_db).with_name("EFTable_Skill.db")))
    gconn = sqlite3.connect(str(GAMEMSG))
    gconn.text_factory = bytes
    icon_data = ICONINFO.read_bytes()

    icons_dir = repo / "Client/Bin/Resources/UI/Vehicle/Icons"
    icons_dir.mkdir(parents=True, exist_ok=True)
    skills_dir = repo / "Client/Bin/Resources/UI/Vehicle/Skills"
    skills_dir.mkdir(parents=True, exist_ok=True)
    rows = []
    for entry in catalog["vehicles"]:
        vid = int(entry["vehicleId"])
        icon, index = vconn.execute(
            "select Icon, IconIndex from Vehicle where PrimaryKey=?", (vid,)).fetchone()
        found = iconinfo_lookup(icon_data, "%s_%d" % (icon, index))
        if found is None:
            raise SystemExit("IconInfo has no %s_%d for vehicle %d" % (icon, index, vid))
        page, x, y, w, h = found
        crop = open_page(page.lower()).crop((x, y, x + w, y + h))
        icon_asset = "UI/Vehicle/Icons/vehicle_%d.png" % vid
        crop.save(repo / "Client/Bin/Resources" / icon_asset)
        skills = []
        if vid not in profiles:
            raise SystemExit("VehicleProfiles.json has no Server profile for vehicle %d" % vid)
        # The Server contract owns which key does what (SPACE dash, Q/W/E actions, R dismounts);
        # this only fetches the retail icon for each of those skills.
        for profile_skill in profiles[vid].get("skills", []):
            slot, skill_id = profile_skill["inputSlot"], int(profile_skill["skillId"])
            sicon, sindex = sconn.execute(
                "select distinct Icon, IconIndex from Skill where PrimaryKey=?", (skill_id,)).fetchone()
            sfound = iconinfo_lookup(icon_data, "%s_%d" % (sicon, sindex))
            if sfound is None:
                raise SystemExit("IconInfo has no %s_%d for skill %d" % (sicon, sindex, skill_id))
            spage, sx, sy, sw, sh = sfound
            skill_asset = "UI/Vehicle/Skills/skill_%d.png" % skill_id
            open_page(spage.lower()).crop((sx, sy, sx + sw, sy + sh)).save(repo / "Client/Bin/Resources" / skill_asset)
            skills.append({"slot": slot, "skillId": skill_id, "cooldownMs": int(profile_skill.get("cooldownMs", 0)),
                           "name": msg(gconn, "tip.name.skill_CommonAction_%d" % skill_id) or msg(gconn, "tip.name.skill_%d" % skill_id),
                           "iconAsset": skill_asset,
                           "iconSource": {"page": spage, "x": sx, "y": sy, "width": sw, "height": sh}})
        rows.append({
            "vehicleId": vid,
            "archetypeId": entry["archetypeId"],
            "name": msg(gconn, "tip.name.vehicle_%d" % vid),
            "description": msg(gconn, "tip.desc.vehicle_%d" % vid),
            "moveSpeed": profiles[vid]["moveSpeed"],
            "iconAsset": icon_asset,
            "iconSource": {"page": page, "x": x, "y": y, "width": w, "height": h},
            "skills": skills,
        })
        print("  %d %-14s <- %s (%d,%d %dx%d)" % (vid, rows[-1]["name"], page, x, y, w, h))

    strings = {
        # GameMsg has "탈것"; the retail window title itself reads "탈 것" (space), and
        # the user asked for that spelling.
        "title": "탈 것",
        "mount": msg(gconn, "sys.vehicle.window_accept"),
        "dismount": msg(gconn, "sys.vehicle.window_release"),
        "close": msg(gconn, "sys.vehicle.window_exit"),
        "mountedSuffix": msg(gconn, "sys.vehicle.window_equip"),
        "moveSpeed": msg(gconn, "sys.vehicle.window_movespeed"),
        "quickslotHint": msg(gconn, "sys.vehicle.window_quickslot_desc"),
        "cannotHere": msg(gconn, "sys.common.action_result_failure_cannot_spawn_vehicle_by_this_place"),
        "cannotNow": msg(gconn, "sys.common.action_result_failure_cannot_spawn_vehicle_by_self"),
    }
    out = repo / "Data/UI/Vehicle/VehicleUiCatalog.json"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps({
        "schema": "lostark.vehicle-ui-catalog", "formatVersion": 1,
        "source": {"names": "EFTable_GameMsg tip.name/desc.vehicle_<id>",
                   "icons": "EFTable_Vehicle Icon/IconIndex -> IconInfo.loa -> EFUI_ICONATLAS_{V,N}",
                   "strings": "EFTable_GameMsg sys.vehicle.* / sys.common.*",
                   "skills": "Data/Vehicles/VehicleProfiles.json skills[] (SPACE/Q/W/E) -> EFTable_Skill Icon/IconIndex -> IconInfo.loa; names tip.name.skill_CommonAction_<id>",
                   "hudEmblem": "quickslot.gfx quickSlotTypeMc vehicle frame (UI/KoukuSaydon/Hud/emblem_interaction_vehicle_saddle.png, cut for the Kouku HUD modes)"},
        "strings": strings,
        "vehicles": rows,
    }, ensure_ascii=False, indent=1), encoding="utf-8")
    print("wrote", out)

    layout_out = repo / "Data/UI/Vehicle/Vehicle_Layout.json"
    layout_out.write_text(json.dumps(build_layout(rows), ensure_ascii=False, indent=2), encoding="utf-8")
    print("wrote", layout_out, "(%d slots)" % len(build_layout(rows)["slots"]))

    art_dir = repo / "Client/Bin/Resources/UI/Vehicle"
    for name, stem, x, y, w, h in CHROME:
        page = Image.open(SHARE / (stem + ".png")).convert("RGBA")
        page.crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    stem, lx, ly, lw, lh, ox, oy = LOCK_ICON
    lock = Image.new("RGBA", (LOCK_SLOT_SIZE, LOCK_SLOT_SIZE), (0, 0, 0, 0))
    lock.paste(Image.open(SHARE_V1 / (stem + ".png")).convert("RGBA").crop((lx, ly, lx + lw, ly + lh)), (ox, oy))
    lock.save(art_dir / "Vehicle_LockIcon.png")
    atlas = Image.open(VEHICLE_ATLAS).convert("RGBA")
    for name, x, y, w, h in LOCAL:
        atlas.crop((x, y, x + w, y + h)).save(art_dir / (name + ".png"))
    print("art: %d chrome + %d local pieces + lock -> %s" % (len(CHROME), len(LOCAL), art_dir))
    return 0


if __name__ == "__main__":
    sys.exit(main())
