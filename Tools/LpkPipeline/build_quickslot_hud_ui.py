#!/usr/bin/env python3
"""원본 quickslot·components·shareimage에서 버프, 스킬 종류, Space 아이콘을 추출한다.
PlayerSkills의 skillKind를 우선해 SkillSlotMarks.json/HudBuffSources.json을 만들고
HUD_Layout.json에 없는 slot만 추가한다. PNG와 JSON은 단계별로 저장된다.
"""

from __future__ import annotations

# 저장소 경로 --repo를 받는다.
import argparse
# 추출된 atlas와 스킬 DB를 찾는다.
import glob
# PlayerSkills와 기존 HUD를 읽고 표시용 JSON을 저장한다.
import json
# 현재 본문에서는 사용하지 않는 기존 import다.
import re
# 원본 Skill·SkillBuff의 아이콘과 종류를 조회한다.
import sqlite3
# IconInfo.loa의 길이와 crop 좌표를 리틀엔디언 정수로 읽는다.
import struct
# main의 반환값을 프로세스 종료 코드로 전달한다.
import sys
# 추출 원본과 Resources·Data 출력 경로를 연결한다.
from pathlib import Path

# atlas crop과 PNG 저장을 수행한다.
from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"
SKILLBUFF = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_SkillBuff.db"
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
ICON_PAGES = EXTRACTED / "Vehicle/icons"
COMPONENTS_I8 = EXTRACTED / "Components_Extracted/tex/EFUI_COMPONENTS/Texture2D/components_i8.tga"
SHARE_V1 = EXTRACTED / "ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D"

# 원본 1920×1080 좌표를 문양 중심과 Q 행에 맞춘 HUD 1280×720 좌표로 변환한다.
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
# PlayerSkills의 skillKind를 원본 Type보다 우선해 실제 조작 방식에 맞는 표식을 선택한다.
KIND_TO_MARK = {"HOLD": "holding", "COMBO": "combo"}

# 복제 상태의 침묵·속박·공포에 대응할 SkillBuff 행과 표시 종류를 연결한다.
STATUS_ICONS = [
    ("silence", 1015, "debuff"),        # iSilenceEndTick
    ("fetter", 1022, "debuff"),         # isPatternBound / iPatternBindEndTick
    ("fear", 13010, "debuff"),          # iFearEndTick
]
# 방어 태세 전환(17810)·화신화(49040) 아이콘을 각 직업의 stance 버프로 사용한다.
STANCE_BUFFS = [("WARLORD_DEFENSE", 17810, "Warlord"), ("GUARDIANKNIGHT_DRAGON", 49040, "GuardianKnight")]

CLASS_DIR = {"LANCE_MASTER": "LanceMaster", "WARLORD": "Warlord", "ARTIST": "Artist",
             "DIMENSIONMASTER": "DimensionMaster", "GUNSLINGER": "Gunslinger", "SLAYER": "Slayer",
             "GUARDIANKNIGHT": "GuardianKnight"}


# IconInfo에서 이름을 찾아 atlas 이름과 crop 범위를 읽는다. 찾지 못하면 None을 반환한다.
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


# runtime이 ID로 찾을 HUD 영역과 Resources 상대 이미지를 묶는다.
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
    # 단일 Space 슬롯은 문양 중심에 맞추고 배경·아이콘·쿨다운·테두리를 같은 rect로 만든다.
    sx = HUD_EMBLEM_CENTER_X - SPECIAL_SIZE * 0.5
    sy = ref_y(SPECIAL_BAR_Y - 1.0 * SPECIAL_SLOT_SCALE)
    out += [
        slot("Special_Space", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, SLOT_BG),
        slot("Special_Space_Icon", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, WHITE),
        slot("Special_Space_Cooldown", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, WHITE, (0, 0, 0, 150 / 255)),
        slot("Special_Space_Frame", sx, sy, SPECIAL_SIZE, SPECIAL_SIZE, SLOT_FRAME),
    ]
    # 버프와 디버프의 배경·아이콘·쿨다운·테두리를 각각 배치한다.
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
    # 기존 스킬 slot의 rect를 기준으로 종류 표식과 chain 시간 표시를 붙인다.
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

    # 버프 이미지를 저장하고 복제 상태에 대응하는 HudBuffSources를 작성한다.
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

    # 스킬 표식 이미지를 만들고 PlayerSkills의 ID·slot과 연결한다.
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
        # HOLD/COMBO는 프로젝트의 조작 계약을 우선하고, 매핑이 없는 종류만 원본 Type을 사용한다.
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

    # ACTIVE Space 스킬만 골라 직업별 아이콘을 저장한다.
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

    # 기존 HUD에 없는 ID만 추가하고 원래 행끝을 유지한다.
    layout_path = repo / "Data/UI/HUD/HUD_Layout.json"
    raw = layout_path.read_bytes()
    crlf = b"\r\n" in raw
    layout = json.loads(raw.decode("utf-8"))
    existing = {s["id"] for s in layout["slots"]}
    new_slots = [s for s in build_slots(layout) if s["id"] not in existing]
    if new_slots:
        text = raw.decode("utf-8").replace("\r\n", "\n")
        # slots 배열의 닫는 위치 앞에 새 행을 넣고 기존 2칸 들여쓰기를 유지한다.
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
