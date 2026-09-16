#!/usr/bin/env python3
"""Quick-slot HUD pieces the base HUD layout did not carry yet, from the retail quickslot /
components / shareimage data.

Outputs
-------
  Client/Bin/Resources/UI/HUD/Buff/*.png
      Shared_BuffSlot_Common pieces (components.gfx): the 64x64 FCSlotBg slot plate
      (shareimage_i7 858,330), the 28x28 buff / debuff / party borders (components_i8
      453,300 / 483,300 / 363,300) and the status icons the project can actually show
      (EFTable_SkillBuff Buff_0 page: 침묵 1015 -> Buff_0, 속박 1022 -> Buff_37, 공포 13010
      -> Buff_38, plus the Warlord defence stance skill icon for the stance buff).
  Client/Bin/Resources/UI/HUD/Common/SkillType_{Combo,Holding,PerfectCombo}.png
      Shared_SkillSlotType_Icon marks (shareimagev2: SlotSkillIcon_Combo i6 66,1004 16x15,
      SlotSkillIcon_Holding ib 869,1005, SlotSkillIcon_PerfectCombo ib 887,1005).
  Client/Bin/Resources/UI/Skill/<Class>/<skillId>_Space.png
      the class move skills' icons (EFTable_Skill Icon/IconIndex -> IconInfo.loa) for the
      special slot above the emblem.
  Data/UI/HUD/SkillSlotMarks.json
      skillId -> mark for every Data/Balance/PlayerSkills.json skill whose EFTable_Skill Type
      is CHARGE(3)/HORDING(4) -> holding, COMBO(5)/CHAIN(6) -> combo, PERFECT_COMBO(13) ->
      perfectCombo (ARKNewSlot.set skillType / SkillSlotTypeMark.set frame).
  Data/UI/HUD/HudBuffSources.json
      which replicated player state feeds the buff / debuff bar and with which icon.
  Data/UI/HUD/HUD_Layout.json (append only, idempotent)
      Special_Space{,_Icon,_Cooldown,_Frame}: QuickSlotSpecialSkillBar slot0 -- BattleSlot art
        scaled 0.9 (43.2 px) centred on the emblem, top 105 retail px above the Q row.
      Buff_{0..3}_{Border,Bg,Icon,Cooldown} / Debuff_{0..3}_*: buffList (830,914) and
        deBuffList (1064,914) of QuickSlotFrame, perItemWidth 29, Shared_BuffSlot_Common
        (border 28 at -1, plate 24 at +1).
      Skill_<X>_TypeMark: skillTypeMc at (3,4) in the 48 px BattleSlot.
      Skill_<X>_Chain: chainSkillTimeEffectMc, 56 px pie at (-3,-2), black a180.
      Retail px -> reference: x through the emblem centre (retail 960 <-> HUD 673.5), y through
      the Q row top (retail 974 <-> HUD 644.7), 2/3 scale.

Sources: D:/ClaudeWork/Extracted/HudGfx_quickslot (quickslot.xml + ffdec scripts),
Components_Extracted, ShareImageGfx_Extracted, Vehicle/icons (icon atlas pages), LpkTables.
"""

from __future__ import annotations

import argparse
import glob
import json
import re
import sqlite3
import struct
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"
SKILLBUFF = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_SkillBuff.db"
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
ICON_PAGES = EXTRACTED / "Vehicle/icons"
COMPONENTS_I8 = EXTRACTED / "Components_Extracted/tex/EFUI_COMPONENTS/Texture2D/components_i8.tga"
SHARE_V1 = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"

# --- retail geometry (1920x1080 stage) -> HUD reference (1280x720) ---------------------------
HUD_EMBLEM_CENTER_X = 673.5          # Vehicle_Hud_Emblem / Kouku_Emblem / WarL_Id_Base centre
HUD_Q_ROW_TOP = 644.702026           # Skill_Q rect y
RETAIL_CENTER_X = 960.0
RETAIL_Q_ROW_TOP = 974.0             # skillSlotList (587,967) + slot0 (93,8) + art top -1
SCALE = 2.0 / 3.0
SLOT_ART = 48.0                      # QuickSlot_BattleSlot art (shape 83: 2..50 x 4..54)

def ref_x(stage_x): return HUD_EMBLEM_CENTER_X + (stage_x - RETAIL_CENTER_X) * SCALE
def ref_y(stage_y): return HUD_Q_ROW_TOP + (stage_y - RETAIL_Q_ROW_TOP) * SCALE

SPECIAL_SLOT_SCALE = 0.9             # QuickSlotSpecialSkillBar places slot0/1 at scale 0.8999
SPECIAL_BAR_Y = 869.0                # QuickSlotFrame.specialSlotList y
SPECIAL_SIZE = SLOT_ART * SPECIAL_SLOT_SCALE * SCALE

BUFF_LIST_X, BUFF_LIST_Y = 830.0, 914.0        # QuickSlotFrame.buffList
DEBUFF_LIST_X = 1064.0                          # QuickSlotFrame.deBuffList
BUFF_PITCH = 29.0                               # BuffSlotManagerEx.perItemWidth
BUFF_BORDER = 28.0                              # Shared_BuffSlot_Common_Buff shape 984 (-1..27)
BUFF_PLATE = 24.0                               # FCSlotBg 64 x 0.375, at (1,1)
BUFF_COUNT = 4

TYPE_MARK_W, TYPE_MARK_H = 16.0, 15.0           # SlotSkillIcon_* sub-images
TYPE_MARK_X, TYPE_MARK_Y = 3.0, 4.0             # skillTypeMc in BattleSlot
CHAIN_SIZE, CHAIN_X, CHAIN_Y = 56.0, -3.0, -2.0 # chainSkillTimeEffectMc

SKILL_SLOT_KEYS = ["Q", "W", "E", "R", "A", "S", "D", "F", "T", "V"]

WHITE = "UI/Common/White1x1.png"
SLOT_BG = "UI/HUD/Common/Slot Bg.png"
SLOT_FRAME = "UI/HUD/Common/Empty Slot.png"

# EFTable_Skill.Type -> mark. The table enum is the ARKNewSlot skillType enum plus one (checked on
# the project skills: 4 = charge 풀배럴 캐넌, 5 = holding 적룡포/적룡질풍격, 6 = chain 선풍참혼/공의연무/
# 대쉬 어퍼 파이어/필법 콩콩이 and the basic attacks), so: 4/5 -> holding, 6/7 -> combo, 14 -> perfect.
TYPE_TO_MARK = {4: "holding", 5: "holding", 6: "combo", 7: "combo", 14: "perfectCombo"}
# Project skillKind (PlayerSkills.json) -> mark; takes precedence over the retail Type.
KIND_TO_MARK = {"HOLD": "holding", "COMBO": "combo"}

# Status icons the project replicates today: SkillBuff row -> (icon name, buff/debuff, source).
STATUS_ICONS = [
    ("silence", 1015, "debuff"),        # iSilenceEndTick
    ("fetter", 1022, "debuff"),         # isPatternBound / iPatternBindEndTick
    ("fear", 13010, "debuff"),          # iFearEndTick
]
# Warlord Z (17810 = 방어 태세 전환) icon stands for the WARLORD_DEFENSE stance buff.
STANCE_BUFFS = [("WARLORD_DEFENSE", 17810, "Warlord")]

CLASS_DIR = {"LANCE_MASTER": "LanceMaster", "WARLORD": "Warlord", "ARTIST": "Artist",
             "DIMENSIONMASTER": "DimensionMaster", "GUNSLINGER": "Gunslinger", "SLAYER": "Slayer"}


def iconinfo_lookup(data: bytes, name: str):
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
    hits = [h for h in glob.glob(str(ICON_PAGES / "**" / (stem + ".*")), recursive=True)
            if h.lower().endswith((".dds", ".tga", ".png"))]
    if not hits:
        raise SystemExit("icon page %s not extracted (umodel -export EFUI_ICONATLAS_<letter>)" % stem)
    return Image.open(hits[0]).convert("RGBA")


def crop_icon(icon_data: bytes, name: str) -> Image.Image:
    found = iconinfo_lookup(icon_data, name)
    if found is None:
        raise SystemExit("IconInfo has no %s" % name)
    page, x, y, w, h = found
    return open_page(page.lower()).crop((x, y, x + w, y + h))


def slot(slot_id, x, y, w, h, path, tint=(1, 1, 1, 1)):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round(x, 4), "y": round(y, 4), "width": round(w, 4), "height": round(h, 4)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": list(tint), "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0}, "frames": [], "loop": True, "additive": False},
    }


def build_slots(layout):
    rects = {s["id"]: s["rect"] for s in layout["slots"]}
    out = []
    # Special (Space) slot: slot0 centred on the emblem (QuickSlotSpecialSlotManager
    # playingCoolDownLength case 1 puts the single drawn slot on the screen centre).
    sx = HUD_EMBLEM_CENTER_X - SPECIAL_SIZE * 0.5
    sy = ref_y(SPECIAL_BAR_Y - 1.0 * SPECIAL_SLOT_SCALE)
    out += [
        slot("Special_Space", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, SLOT_BG),
        slot("Special_Space_Icon", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, WHITE),
        slot("Special_Space_Cooldown", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, WHITE, (0, 0, 0, 150 / 255)),
        slot("Special_Space_Frame", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, SLOT_FRAME),
    ]
    # Buff / debuff bars.
    for prefix, list_x in (("Buff", BUFF_LIST_X), ("Debuff", DEBUFF_LIST_X)):
        border_path = "UI/HUD/Buff/HUD_Buff_Border_%s.png" % ("Buff" if prefix == "Buff" else "Debuff")
        for i in range(BUFF_COUNT):
            bx = ref_x(list_x + i * BUFF_PITCH - 1.0)      # border at (-1,-1) of the slot origin
            by = ref_y(BUFF_LIST_Y - 1.0)
            bw = BUFF_BORDER * SCALE
            px = ref_x(list_x + i * BUFF_PITCH + 1.0)      # plate / icon at (1,1)
            py = ref_y(BUFF_LIST_Y + 1.0)
            pw = BUFF_PLATE * SCALE
            out += [
                slot("%s_%d_Bg" % (prefix, i), px, py, pw, pw, "UI/HUD/Buff/HUD_Buff_SlotBg.png"),
                slot("%s_%d_Icon" % (prefix, i), px, py, pw, pw, WHITE),
                slot("%s_%d_Cooldown" % (prefix, i), px, py, pw, pw, WHITE, (0, 0, 0, 127 / 255)),
                slot("%s_%d_Border" % (prefix, i), bx, by, bw, bw, border_path),
            ]
    # Per skill slot: type mark and chain time pie.
    k = 34.5 / SLOT_ART                                    # HUD slot 34.5 ref px == 48 retail px
    for key in SKILL_SLOT_KEYS:
        r = rects.get("Skill_" + key)
        if r is None:
            continue
        out.append(slot("Skill_%s_TypeMark" % key, r["x"] + TYPE_MARK_X * k, r["y"] + TYPE_MARK_Y * k,
                        TYPE_MARK_W * k, TYPE_MARK_H * k, WHITE))
        out.append(slot("Skill_%s_Chain" % key, r["x"] + CHAIN_X * k, r["y"] + CHAIN_Y * k,
                        CHAIN_SIZE * k, CHAIN_SIZE * k, WHITE, (0, 0, 0, 180 / 255)))
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo
    res = repo / "Client/Bin/Resources"
    icon_data = ICONINFO.read_bytes()
    skill_db = glob.glob(str(EXTRACTED / "Vehicle/tables/**/EFTable_Skill.db"), recursive=True)[0]
    sconn = sqlite3.connect(skill_db)
    bconn = sqlite3.connect(str(SKILLBUFF))

    # --- buff slot art -------------------------------------------------------------------
    buff_dir = res / "UI/HUD/Buff"
    buff_dir.mkdir(parents=True, exist_ok=True)
    Image.open(SHARE_V1 / "shareimage_i7.png").convert("RGBA").crop((858, 330, 922, 394)).save(buff_dir / "HUD_Buff_SlotBg.png")
    comp = Image.open(COMPONENTS_I8).convert("RGBA")
    for name, (x, y) in {"Buff": (453, 300), "Debuff": (483, 300), "Party": (363, 300)}.items():
        comp.crop((x, y, x + 28, y + 28)).save(buff_dir / ("HUD_Buff_Border_%s.png" % name))
    sources = [{"source": "vehicle", "kind": "buff", "iconAsset": None,
                "note": "PLAYER_SNAPSHOT.iVehicleId; icon = the ridden vehicle's VehicleUiCatalog icon"}]
    for name, pk, kind in STATUS_ICONS:
        icon, index = bconn.execute("select Icon, IconIndex from SkillBuff where PrimaryKey=?", (pk,)).fetchone()
        asset = "UI/HUD/Buff/buff_%s.png" % name
        crop_icon(icon_data, "%s_%d" % (icon, index)).save(res / asset)
        sources.append({"source": name, "kind": kind, "iconAsset": asset,
                        "iconSource": {"table": "EFTable_SkillBuff", "primaryKey": pk, "icon": "%s_%d" % (icon, index)}})
    for stance, skill_id, class_dir in STANCE_BUFFS:
        icon, index = sconn.execute("select distinct Icon, IconIndex from Skill where PrimaryKey=?", (skill_id,)).fetchone()
        asset = "UI/HUD/Buff/buff_stance_%s.png" % stance.lower()
        crop_icon(icon_data, "%s_%d" % (icon, index)).save(res / asset)
        sources.append({"source": "stance", "stance": stance, "kind": "buff", "iconAsset": asset,
                        "iconSource": {"table": "EFTable_Skill", "primaryKey": skill_id, "icon": "%s_%d" % (icon, index)}})
    (repo / "Data/UI/HUD/HudBuffSources.json").write_text(json.dumps({
        "schema": "lostark.hud-buff-sources", "formatVersion": 1,
        "source": {"layout": "quickslot.gfx QuickSlotFrame buffList (830,914) / deBuffList (1064,914), BuffSlotManagerEx perItemWidth 29",
                   "slot": "components.gfx Shared_BuffSlot_Common: FCSlotBg 24 @(1,1), border 28 @(-1,-1), cooldownText YG760 10px",
                   "colors": {"buffText": "#9BD979", "debuffText": "#E2C87A", "buffStack": "#66FF99", "debuffStack": "#FF6600"}},
        "sources": sources}, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")

    # --- skill type marks ------------------------------------------------------------------
    common = res / "UI/HUD/Common"
    ib = Image.open(SHARE_V1 / "shareimagev2_ib.png").convert("RGBA")
    i6 = Image.open(SHARE_V1 / "shareimagev2_i6.png").convert("RGBA")
    i6.crop((66, 1004, 82, 1019)).save(common / "SkillType_Combo.png")
    ib.crop((869, 1005, 885, 1020)).save(common / "SkillType_Holding.png")
    ib.crop((887, 1005, 903, 1020)).save(common / "SkillType_PerfectCombo.png")

    skills = json.loads((repo / "Data/Balance/PlayerSkills.json").read_text(encoding="utf-8"))["skills"]
    marks = []
    for s in skills:
        row = sconn.execute("select distinct Type from Skill where PrimaryKey=?", (s["skillId"],)).fetchone()
        if row is None:
            continue
        # The project's skillKind is how the skill actually plays here (Server contract), so a
        # HOLD skill gets the holding mark even where the retail row says COMBO (34590), and a
        # COMBO skill the chain mark; only ACTIVE skills fall back to the retail Type.
        mark = KIND_TO_MARK.get(s["skillKind"]) or TYPE_TO_MARK.get(int(row[0]))
        if mark:
            marks.append({"skillId": s["skillId"], "characterClass": s["characterClass"], "inputSlot": s["inputSlot"],
                          "mark": mark, "skillKind": s["skillKind"], "retailType": int(row[0])})
    (repo / "Data/UI/HUD/SkillSlotMarks.json").write_text(json.dumps({
        "schema": "lostark.hud-skill-slot-marks", "formatVersion": 1,
        "source": "Data/Balance/PlayerSkills.json skillKind first (HOLD -> holding, COMBO -> combo), else EFTable_Skill.Type (table enum = ARKNewSlot skillType + 1): 4 charge / 5 holding -> holding, 6 / 7 chain -> combo, 14 -> perfectCombo",
        "assets": {"combo": "UI/HUD/Common/SkillType_Combo.png", "holding": "UI/HUD/Common/SkillType_Holding.png",
                   "perfectCombo": "UI/HUD/Common/SkillType_PerfectCombo.png"},
        "marks": marks}, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print("marks:", [(m["skillId"], m["mark"]) for m in marks])

    # --- move (Space) skill icons -----------------------------------------------------------
    space_icons = []
    for s in skills:
        if s["inputSlot"] != "SPACE" or s["skillKind"] != "ACTIVE":
            continue
        icon, index = sconn.execute("select distinct Icon, IconIndex from Skill where PrimaryKey=?", (s["skillId"],)).fetchone()
        class_dir = CLASS_DIR[s["characterClass"]]
        asset = "UI/Skill/%s/%d_Space.png" % (class_dir, s["skillId"])
        (res / "UI/Skill" / class_dir).mkdir(parents=True, exist_ok=True)
        crop_icon(icon_data, "%s_%d" % (icon, index)).save(res / asset)
        space_icons.append((s["skillId"], asset))
    print("space icons:", space_icons)

    # --- HUD layout append (idempotent) -----------------------------------------------------
    layout_path = repo / "Data/UI/HUD/HUD_Layout.json"
    raw = layout_path.read_bytes()
    crlf = b"\r\n" in raw
    layout = json.loads(raw.decode("utf-8"))
    existing = {s["id"] for s in layout["slots"]}
    new_slots = [s for s in build_slots(layout) if s["id"] not in existing]
    if new_slots:
        text = raw.decode("utf-8").replace("\r\n", "\n")
        # Append before the closing of the "slots" array, matching the file's 2-space formatting.
        tail = "\n  ]\n}"
        assert text.rstrip("\n").endswith(tail.strip("\n").replace("\n", "\n")) or text.rstrip().endswith("]\n}"), "unexpected layout tail"
        body = ",\n".join("    " + json.dumps(s, indent=2, ensure_ascii=False).replace("\n", "\n    ") for s in new_slots)
        idx = text.rstrip().rfind("\n  ]")
        text = text[:idx] + ",\n" + body + text[idx:]
        if crlf:
            text = text.replace("\n", "\r\n")
        layout_path.write_bytes(text.encode("utf-8"))
        json.loads(layout_path.read_text(encoding="utf-8"))
    print("layout: appended %d slots" % len(new_slots))
    return 0


if __name__ == "__main__":
    sys.exit(main())
