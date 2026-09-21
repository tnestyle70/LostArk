#!/usr/bin/env python3
"""Cut the GuardianKnight (retail DragonKnight, `ddk`) combat-HUD art and lay out its slots.

Retail splits this class's HUD across two Scaleform documents, both of which are on screen at
once in the two stances this project has:

  EFUI_IDENTITYDRAGONKNIGHT               DragonKnightSkinFrame -- the horned identity frame left
                                          of the quick slots, its orb gauge, the Z key plate and
                                          the 8-dot skill status strip.
  EFUI_IDENTITYDRAGONKNIGHTEMBERETHGAUGE  IdentityDragonKnightEmberethGauge -- the 10-socket bar
                                          that stands where every other class shows a mana bar.

What the source says about each piece (read off the .as and the placement tree, not guessed):

  orbGauge      ark.controls.Progress: target.width = percent * trackLength, so the fill is a
                left-to-right reveal at native scale -- Set_SlotFillRatio, not a resize. Its
                `track` sprite has one frame per stance (1 human / 2 dragon / 3 liberatio), which
                is DragonKnightSkinFrame.inMarkGaugeState(). Only 1 and 2 exist for us.
  inMark        Progress.updateMark: x = target.x + target.width, y centred on the track, and
                useAutoHideMark hides it at exactly 0 and exactly maximum.
  skillKey_lb   DragonKnightSkinFrame.draw()/set defaultGaugeValue: white (0xFFFFFF) while the
                gauge is full or the dragon stance is held, otherwise 0x686C20.
  stanceMc      frame labels normal(1) / dragon(21) / dragon_off(73) / liberatio(93). This script
                installs the static `normal` pose only; the 52-frame transform is not baked.
  bloodGauge    socket_0..socket_9 at x -3,19,41,63,85,110,132,154,176,198 (y -4) inside a
                bloodGauge placed at stage (1063,953) -- note the 25 px step between 4 and 5,
                which groups them 5+5. Each socket is one DragonKnightBloodSocket sprite with
                show/hide/lock label ranges; we install the resting art of each state.
  slotAni_0/1   ark-passive slots. IDENTITY_STANCE_DRAGONKNIGHT_DRAGON_AP12 / _NORMAL_AP13 only,
                so retail itself hides them in both stances this project has. Not installed.

Retail px -> reference px is the same mapping the rest of the combat HUD already uses: x through
the emblem centre (retail 960 <-> HUD 673.5), y through the Q row top (retail 974 <-> HUD 644.7),
2/3 scale. Checked against the existing Warlord slots, whose identity frame starts at retail 846
and is authored at 597.0.

Writes:
  Client/Bin/Resources/UI/HUD/GuardianKnight/*.png   identity frame, orb, key plate, status strip
  Client/Bin/Resources/UI/HUD/GuardianKnight/Embereth/*.png
  Client/Bin/Resources/UI/Skill/GuardianKnight/*.png  18 quick-slot icons (EFTable_Skill ->
                                                      IconInfo -> ddk_skill_1 atlas page)
  Data/UI/HUD/HUD_Layout.json                         append only, idempotent
  Data/UI/HUD/GuardianKnightIdentity.json             what each slot means, for the HUD code

Sources: D:/ClaudeWork/Extracted/HudGfx_dragonknight (ffdec XML + scripts + umodel textures),
Vehicle/icons (EFUI_ICONATLAS_D), Vehicle/tables (EFTable_Skill), Data3 (IconInfo.loa).
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
DK = EXTRACTED / "HudGfx_dragonknight"
IDENTITY_PAGE = DK / "tex/EFUI_IDENTITYDRAGONKNIGHT/identitydragonknight_i16.png"
EMBERETH_PAGE = (DK / "tex_eb/EFUI_IDENTITYDRAGONKNIGHTEMBERETHGAUGE"
                 / "identitydragonknightemberethgauge_i4.png")
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
ICON_PAGES = EXTRACTED / "Vehicle/icons"

# --- retail -> reference resolution ------------------------------------------------------------
EMBLEM_RETAIL_X, EMBLEM_HUD_X = 960.0, 673.5
QROW_RETAIL_Y, QROW_HUD_Y = 974.0, 644.7
SCALE = 2.0 / 3.0


def hx(x: float) -> float:
    return EMBLEM_HUD_X + (x - EMBLEM_RETAIL_X) * SCALE


def hy(y: float) -> float:
    return QROW_HUD_Y + (y - QROW_RETAIL_Y) * SCALE


# --- crops (atlas page, x, y, w, h) ------------------------------------------------------------
# DragonKnightSkinFrame / DragonKnightStanceMc frame 1 ("normal"), in its own placement order.
IDENTITY_CROPS = {
    "Frame_Glow":        (400, 202, 224, 159),   # stanceMc depth 1, the soft backdrop
    "Frame_Body":        (202,   0, 190, 174),   # stanceMc depth 7, the horned frame
    "Frame_Inner":       (  0, 366, 112, 116),   # stanceMc depth 34, the orb bezel
    "Orb_Empty":         (396, 484,  70,  70),   # orbGauge base, unfilled
    "Orb_Fill_Human":    (324, 484,  70,  70),   # Gauge_Track frame 1
    "Orb_Fill_Dragon":   (468, 484,  70,  70),   # Gauge_Track frame 2
    "Orb_Mark":          (759, 484,  89,  37),   # inMark, rides the fill edge
    "SkillKey_Bg":       (963, 130,  30,  16),   # skillKey_bg, the Z plate
    "SkillStatus_Plate": (855, 130,  46,  22),   # skillStatusList backing
    "SkillStatus_Dot":   (1013, 10,   8,   8),   # one use_mc dot
}
EMBERETH_CROPS = {
    "Frame":        (487, 0, 257, 23),   # dragonKnightGaugeStance
    "Socket_Empty": (463, 0,  22, 25),   # bloodGauge track tile, one per socket
    "Socket_Full":  (433, 0,  28, 31),   # DragonKnightBloodSocket "show" gem
    "Socket_Lock":  (746, 0,  18, 23),   # "lock" padlock
}

# --- identity frame placement (retail stage px) ------------------------------------------------
STANCE_ORIGIN = (846.0, 922.0)           # DragonKnightSkinFrame.stanceMc
STANCE_PIECES = [                        # (crop, stage x, stage y) from the placement tree
    ("Frame_Glow",  846.0, 922.0),
    ("Frame_Body",  864.0, 906.0),
    ("Frame_Inner", 903.0, 933.0),
]
ORB = (924.0, 964.0, 70.0, 70.0)         # gaugeTooltip hit area == the orb; skillEffect sits at
                                         # (960,1000), i.e. its centre, which confirms this.
SKILLKEY_BG = (944.0, 1050.0, 30.0, 16.0)
STATUS_PLATE = (890.0, 1046.0, 46.0, 22.0)
STATUS_DOTS = [(897.0 + 8.0 * (i % 4), 1049.0 + 8.0 * (i // 4)) for i in range(8)]

# --- embereth placement (retail stage px) ------------------------------------------------------
EMBERETH_FRAME = (1046.0, 954.0, 257.0, 23.0)
BLOOD_GAUGE = (1063.0, 953.0)
SOCKET_OFFSETS = [-3.0, 19.0, 41.0, 63.0, 85.0, 110.0, 132.0, 154.0, 176.0, 198.0]
SOCKET_Y = -4.0

# --- quick-slot icons --------------------------------------------------------------------------
# skillId -> (english name for the file, EFTable_Skill IconIndex). LMB 49000/49001 carry no icon
# in the table either, and the HUD has no LMB slot, so they are absent on purpose.
SKILL_ICONS = [
    (49020, "Lunge"), (49021, "Glide"),
    (49040, "DragonAvatar"), (49041, "DragonAvatarRelease"),
    (49100, "Cleave"), (49110, "WildStrike"), (49120, "Ignite"), (49130, "ValiantCharge"),
    (49150, "QuakeSmash"),
    (49200, "DragonsBlow"), (49210, "DragonsSpear"),
    (49220, "BurningFlame"), (49230, "AbaddonFlame"),
    (49260, "InfernoStrike"), (49270, "InfernoFlame"),
    (49330, "FireBreath"),
    (49400, "DragonRampage"), (49420, "BreathOfDestruction"),
]


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
        raise SystemExit("icon page %s not extracted (umodel -export EFUI_ICONATLAS_D)" % stem)
    return Image.open(hits[0]).convert("RGBA")


def slot(slot_id: str, owner, x: float, y: float, w: float, h: float, path, type_id: int = 0,
         visible_note: str = ""):
    """One HUD_Layout slot in the document's own shape. `path` None = a slot the HUD code fills."""
    return {
        "id": slot_id,
        "ownerClass": owner,
        "type": type_id,
        "rect": {"x": round(x, 4), "y": round(y, 4),
                 "width": round(w, 4), "height": round(h, 4)},
        "rotation": 0,
        "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": [1, 1, 1, 1],
                    "additive": False, "flipX": False}] if path else [],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1.0, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def build_slots() -> list:
    res = "UI/HUD/GuardianKnight/"
    eb = res + "Embereth/"
    owner = "GuardianKnight"
    out = []

    # Identity frame: the stanceMc "normal" pose, in its authored depth order.
    for name, x, y in STANCE_PIECES:
        _, _, w, h = IDENTITY_CROPS[name]
        out.append(slot("GK_Id_" + name.replace("Frame_", ""), owner,
                        hx(x), hy(y), w * SCALE, h * SCALE, res + name + ".png"))

    ox, oy, ow, oh = ORB
    out.append(slot("GK_Id_OrbEmpty", owner, hx(ox), hy(oy), ow * SCALE, oh * SCALE,
                    res + "Orb_Empty.png"))
    # Fill: authored at full width, clipped by Set_SlotFillRatio. Texture swaps per stance.
    out.append(slot("GK_Id_OrbFill", owner, hx(ox), hy(oy), ow * SCALE, oh * SCALE,
                    res + "Orb_Fill_Human.png"))
    # Mark: authored at the orb's left edge, centred on it; the HUD moves it to the fill edge.
    mx, my, mw, mh = IDENTITY_CROPS["Orb_Mark"]
    out.append(slot("GK_Id_OrbMark", owner,
                    hx(ox) - mw * SCALE * 0.5, hy(oy + oh * 0.5) - mh * SCALE * 0.5,
                    mw * SCALE, mh * SCALE, res + "Orb_Mark.png"))

    kx, ky, kw, kh = SKILLKEY_BG
    out.append(slot("GK_Id_SkillKeyBg", owner, hx(kx), hy(ky), kw * SCALE, kh * SCALE,
                    res + "SkillKey_Bg.png"))

    px, py, pw, ph = STATUS_PLATE
    out.append(slot("GK_Id_StatusPlate", owner, hx(px), hy(py), pw * SCALE, ph * SCALE,
                    res + "SkillStatus_Plate.png"))
    dw, dh = IDENTITY_CROPS["SkillStatus_Dot"][2:]
    for i, (dx, dy) in enumerate(STATUS_DOTS):
        out.append(slot("GK_Id_StatusDot%d" % i, owner, hx(dx), hy(dy),
                        dw * SCALE, dh * SCALE, res + "SkillStatus_Dot.png"))

    # Embereth bar, where every other class draws its mana bar.
    fx, fy, fw, fh = EMBERETH_FRAME
    out.append(slot("GK_Embereth_Frame", owner, hx(fx), hy(fy), fw * SCALE, fh * SCALE,
                    eb + "Frame.png"))
    bx, by = BLOOD_GAUGE
    ew, eh = EMBERETH_CROPS["Socket_Empty"][2:]
    lw, lh = EMBERETH_CROPS["Socket_Full"][2:]
    for i, off in enumerate(SOCKET_OFFSETS):
        sx, sy = bx + off, by + SOCKET_Y
        out.append(slot("GK_Embereth_Socket%d" % i, owner, hx(sx), hy(sy),
                        ew * SCALE, eh * SCALE, eb + "Socket_Empty.png"))
        # The lit gem is bigger than the empty tile, so it hangs off the tile's own centre.
        out.append(slot("GK_Embereth_Fill%d" % i, owner,
                        hx(sx + (ew - lw) * 0.5), hy(sy + (eh - lh) * 0.5),
                        lw * SCALE, lh * SCALE, eb + "Socket_Full.png"))
        # A socket past the unlocked count wears the padlock (the "lock" label range).
        kw, kh = EMBERETH_CROPS["Socket_Lock"][2:]
        out.append(slot("GK_Embereth_Lock%d" % i, owner,
                        hx(sx + (ew - kw) * 0.5), hy(sy + (eh - kh) * 0.5),
                        kw * SCALE, kh * SCALE, eb + "Socket_Lock.png"))
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo
    res = repo / "Client/Bin/Resources"

    # --- identity + embereth art ---------------------------------------------------------------
    out_dir = res / "UI/HUD/GuardianKnight"
    (out_dir / "Embereth").mkdir(parents=True, exist_ok=True)
    page = Image.open(IDENTITY_PAGE).convert("RGBA")
    for name, (x, y, w, h) in IDENTITY_CROPS.items():
        page.crop((x, y, x + w, y + h)).save(out_dir / (name + ".png"))
    eb_page = Image.open(EMBERETH_PAGE).convert("RGBA")
    for name, (x, y, w, h) in EMBERETH_CROPS.items():
        eb_page.crop((x, y, x + w, y + h)).save(out_dir / "Embereth" / (name + ".png"))
    print("art: %d identity + %d embereth crops" % (len(IDENTITY_CROPS), len(EMBERETH_CROPS)))

    # --- quick-slot icons ----------------------------------------------------------------------
    icon_dir = res / "UI/Skill/GuardianKnight"
    icon_dir.mkdir(parents=True, exist_ok=True)
    icon_data = ICONINFO.read_bytes()
    skill_db = glob.glob(str(EXTRACTED / "Vehicle/tables/**/EFTable_Skill.db"), recursive=True)[0]
    conn = sqlite3.connect(skill_db)
    icon_rows = {}
    for skill_id, english in SKILL_ICONS:
        row = conn.execute(
            "select distinct Icon, IconIndex from Skill where PrimaryKey=?", (skill_id,)).fetchone()
        if row is None or not row[0]:
            raise SystemExit("EFTable_Skill has no icon for %d" % skill_id)
        key = "%s_%d" % (row[0], row[1])
        found = iconinfo_lookup(icon_data, key)
        if found is None:
            raise SystemExit("IconInfo has no %s" % key)
        src_page, x, y, w, h = found
        asset = "UI/Skill/GuardianKnight/%d_%s.png" % (skill_id, english)
        open_page(src_page.lower()).crop((x, y, x + w, y + h)).save(repo / "Client/Bin/Resources" / asset)
        icon_rows[skill_id] = {"asset": asset, "icon": key, "page": src_page}
    print("icons: %d" % len(icon_rows))

    # --- what the HUD code needs to know about these slots -------------------------------------
    (repo / "Data/UI/HUD/GuardianKnightIdentity.json").write_text(json.dumps({
        "schema": "lostark.guardianknight-identity-hud",
        "formatVersion": 1,
        "source": {
            "identity": "EFUI_IDENTITYDRAGONKNIGHT DragonKnightSkinFrame",
            "embereth": "EFUI_IDENTITYDRAGONKNIGHTEMBERETHGAUGE IdentityDragonKnightEmberethGauge",
            "mapping": "retail 1920x1080 stage -> 1280x720 reference: emblem 960<->673.5, "
                       "Q row top 974<->644.7, 2/3 scale",
        },
        "orb": {
            "fill": "Set_SlotFillRatio(GK_Id_OrbFill, identity/maximum) -- Progress.target.width",
            "stanceTexture": {
                "GUARDIANKNIGHT_HUMAN": "UI/HUD/GuardianKnight/Orb_Fill_Human.png",
                "GUARDIANKNIGHT_DRAGON": "UI/HUD/GuardianKnight/Orb_Fill_Dragon.png",
            },
            "markHiddenAt": ["minimum", "maximum"],
        },
        "skillKeyColor": {"usable": "#FFFFFF", "unusable": "#686C20",
                          "usableWhen": "gauge full, or the dragon stance is held"},
        "embereth": {
            "socketCount": 10,
            "note": "retail invokeDragonKnightBloodGauge(filled, unlocked): sockets [0,unlocked) "
                    "are lit or empty, sockets past `unlocked` wear the padlock.",
            "filled": "HUD_PLAYER_STATE.iEmberOrbs",
            "unlocked": "iEmberMaximumSockets - iEmberLockedSockets (the server's own capacity "
                        "expression in CPlayerSkillSystem)",
        },
        "skillIcons": {str(k): v for k, v in sorted(icon_rows.items())},
        "notInstalled": {
            "slotAni_0/slotAni_1": "ark-passive slots; retail hides them in both stances we have",
            "stanceMc transform": "the 52-frame dragon / 20-frame dragon_off clips are not baked; "
                                  "only the static normal pose is installed",
        },
    }, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")

    # --- HUD_Layout.json: append only -----------------------------------------------------------
    layout_path = repo / "Data/UI/HUD/HUD_Layout.json"
    raw = layout_path.read_bytes()
    crlf = b"\r\n" in raw
    layout = json.loads(raw.decode("utf-8"))
    text = raw.decode("utf-8").replace("\r\n", "\n")
    existing = {s["id"] for s in layout["slots"]}
    new_slots = [s for s in build_slots() if s["id"] not in existing]
    if new_slots:
        body = ",\n".join("    " + json.dumps(s, indent=2, ensure_ascii=False).replace("\n", "\n    ")
                          for s in new_slots)
        idx = text.rstrip().rfind("\n  ]")
        assert idx > 0, "unexpected layout tail"
        text = text[:idx] + ",\n" + body + text[idx:]
    if "GuardianKnight" not in layout["classes"]:
        # The array is one entry per line; append before its closing bracket rather than
        # reserialising it, so the rest of the document keeps its authored formatting.
        start = text.index('"classes": [')
        close = text.index("\n  ],", start)
        text = text[:close] + ',\n    "GuardianKnight"' + text[close:]
    if crlf:
        text = text.replace("\n", "\r\n")
    layout_path.write_bytes(text.encode("utf-8"))
    json.loads(layout_path.read_text(encoding="utf-8"))
    print("layout: appended %d slots" % len(new_slots))
    return 0


if __name__ == "__main__":
    sys.exit(main())
