"""Cut the commander-raid gate progress widget out of the retail document and write its
lostark.ui-layout documents for CRaidGateProgressView.

Source: EFUI_EPICGATECOMMANDERPROCESS (epicgatecommanderprocess.gfx), sprite 172
EpicGateCommanderProgressFrame -- the top-left "<raid> [<difficulty>]" panel with one gate
icon per gate (frame labels check / active / inactive). Geometry is the frame's own:
  bg          shape 140 (abbreviation frame 2) = image 2 (0,747)-(302,1012); visible 302x173
  gateIcon0-2 sprite 153 at (59,74) (121,74) (183,74): 59x65 art
  gateIcon states: check = image 1 (370,899)-(429,964) crest with the green check,
                   active = image 1 (192,899)-(284,987) blue glow (92x88, drawn centred),
                   inactive = image 1 (561,899)-(606,951) blue door (45x52)
  dungeonName label at (11,7) 18 px centred over the frame, dungeonRank at (11,48)
  dungeonLine sprite 146 at (94,94): the gold divider, a standalone (nopack) 252x76 texture
      drawn at 124x29
  showToggleBtn sprite 136 at (143,73): the shared V2step1ListArrow_selected arrow
      (shareImageV2 image 0 = shareImageV2_IB, (104,1005)-(127,1024)), placed scale -1/-1 from
      (19,15), i.e. a 23x19 arrow at frame-local (139,69) turned 180 degrees (pointing down)
  setting sprite 171 is an authoring text sheet (Arial #333333 labels at x 225..685) and
      gateTooltipMc sprite 135 is a fully transparent 278x53 hit area -- neither is visible art
Image ids come from the .gfx DefineSubImage tags; umodel exports the pages as
epicgatecommanderprocess_i<hex>.dds (image 1 = _i5a, 2 = _i52, 3 = _i60, checked by alpha).
The host places the frame top-left; that position is not in the document, so the layout
puts it at reference (8,12) under the 2/3 stage->reference scale every runtime UI uses.

The vote prompt reuses the party invite modal's panel and buttons
(UI/ClassSelect/Common/*), same rects as Data/UI/Party/PartyInviteConfirm_Layout.json.

Inputs (already extracted on this machine)
  D:/ClaudeWork/Extracted/KoukuUI_Extracted/epicgatecommanderprocess/pages/*.png
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

PAGES = Path(r"D:/ClaudeWork/Extracted/KoukuUI_Extracted/epicgatecommanderprocess/pages")
PAGE_BY_IMAGE_ID = {1: "epicgatecommanderprocess_i5a.png",
                    2: "epicgatecommanderprocess_i52.png",
                    3: "epicgatecommanderprocess_i60.png"}
# The widget's restart / progress / exit button is AnimatedButton_renew_V2 -> shareImageV2
# V2btn_* (the same 103x36 button the system option window uses).
SHARE = Path(r"D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/tex_v2/EFUI_SHAREIMAGE/Texture2D")

# (output name, image id, x, y, w, h)
CROPS = [
    ("GateProgress_Bg",     2, 0, 747, 302, 173),
    ("GateIcon_Check",      1, 370, 899, 59, 65),
    ("GateIcon_Active",     1, 192, 899, 92, 88),
    ("GateIcon_Inactive",   1, 561, 899, 45, 52),
]
# (output name, shareImageV2 page stem, x, y, w, h)
SHARED_CROPS = [
    ("GateProgress_Btn_Normal", "shareimagev2_i46", 643, 988, 103, 36),
    ("GateProgress_Btn_Over",   "shareimagev2_i46", 328, 988, 103, 36),
    ("GateProgress_ToggleArrow", "shareimagev2_ib", 104, 1005, 23, 19),
]
# The divider is its own standalone texture, not an atlas rect.
LINE_PAGE = PAGES / "epicgatecommanderprocess_i1.png"

# Placement: the frame's own sprite 172 geometry, checked against the retail 1080p capture of
# the live widget (frame top-left at the screen's (0,52)): dungeonName 18 px centred at (141,28),
# "[difficulty]" at (141,52), gateIcon0..2 at (59,74) (121,74) (183,74) 59x65 (62 px pitch,
# inside the frame under the difficulty line), and the V2 button stretched to (56,128) 176x37
# at the frame's bottom. The party frame sits below the widget and is not part of it. All
# frame-local retail px, scaled by 2/3 onto the 1280x720 reference with the frame at (4,32).
STAGE_TO_REF = 2.0 / 3.0
WIDGET_X, WIDGET_Y = 4.0, 32.0
BUTTON_LOCAL = (56.0, 128.0, 176.0, 37.0)
ICON_LOCAL = [(59.0, 74.0), (121.0, 74.0), (183.0, 74.0)]
ICON_W, ICON_H = 59.0, 65.0
LINE_LOCAL = (94.0, 94.0, 124.0, 29.0)
# Flipped about (19,15) inside showToggleBtn at (143,73): the art ends up here, turned 180.
TOGGLE_LOCAL = (139.0, 69.0, 23.0, 19.0)


def layout_slot(slot_id, x, y, w, h, path, hover=None, rotation=0, additive=False):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": x, "y": y, "width": w, "height": h},
        "rotation": rotation, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": hover, "tint": [1, 1, 1, 1],
                    "additive": additive, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def document(slots):
    return {"schema": "lostark.ui-layout", "formatVersion": 1,
            "resolution": {"width": 1280, "height": 720}, "classes": ["Default"],
            "slots": slots}


def build_widget_layout():
    a = "UI/RaidGateProgress/"
    s = STAGE_TO_REF
    slots = [layout_slot("RGP_Bg", WIDGET_X, WIDGET_Y, 302 * s, 173 * s, a + "GateProgress_Bg.png")]
    bx, by, bw, bh = BUTTON_LOCAL
    slots.append(layout_slot("RGP_Button", WIDGET_X + bx * s, WIDGET_Y + by * s, bw * s, bh * s,
                             a + "GateProgress_Btn_Normal.png", a + "GateProgress_Btn_Over.png"))
    lx, ly, lw, lh = LINE_LOCAL
    # The divider texture is a glow baked on black with no alpha, so it draws additively.
    slots.append(layout_slot("RGP_Line", WIDGET_X + lx * s, WIDGET_Y + ly * s, lw * s, lh * s,
                             a + "GateProgress_Line.png", additive=True))
    tx, ty, tw, th = TOGGLE_LOCAL
    slots.append(layout_slot("RGP_Toggle", WIDGET_X + tx * s, WIDGET_Y + ty * s, tw * s, th * s,
                             a + "GateProgress_ToggleArrow.png", rotation=180))
    for i, (lx, ly) in enumerate(ICON_LOCAL):
        slots.append(layout_slot("RGP_Icon%d" % i, WIDGET_X + lx * s, WIDGET_Y + ly * s,
                                 ICON_W * s, ICON_H * s, a + "GateIcon_Inactive.png"))
    return document(slots)


def build_vote_layout():
    c = "UI/ClassSelect/Common/"
    return document([
        layout_slot("RGV_Panel", 440.15, 278.95, 399.7, 135.1, c + "CreateCharacterModalPanel.png"),
        layout_slot("RGV_ConfirmButton", 543.1, 363.9, 70, 23.8, c + "NormalButton.png", c + "NormalButtonHover.png"),
        layout_slot("RGV_CancelButton", 649.2, 363.9, 70, 23.8, c + "NormalButton.png", c + "NormalButtonHover.png"),
    ])


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    args = ap.parse_args()
    art_out = args.repo / "Client/Bin/Resources/UI/RaidGateProgress"
    data_out = args.repo / "Data/UI/RaidGateProgress"
    art_out.mkdir(parents=True, exist_ok=True)
    data_out.mkdir(parents=True, exist_ok=True)

    pages = {}
    for name, image_id, x, y, w, h in CROPS:
        if image_id not in pages:
            pages[image_id] = Image.open(PAGES / PAGE_BY_IMAGE_ID[image_id]).convert("RGBA")
        crop = pages[image_id].crop((x, y, x + w, y + h))
        if crop.getchannel("A").getbbox() is None:
            print("  ! empty crop", name)
            return 1
        crop.save(art_out / (name + ".png"))
    Image.open(LINE_PAGE).convert("RGBA").save(art_out / "GateProgress_Line.png")
    for name, stem, x, y, w, h in SHARED_CROPS:
        page = Image.open(SHARE / (stem + ".png")).convert("RGBA")
        page.crop((x, y, x + w, y + h)).save(art_out / (name + ".png"))
    print("wrote %d png to %s" % (len(CROPS) + len(SHARED_CROPS) + 1, art_out))

    for file_name, doc in (("RaidGateProgress_Layout.json", build_widget_layout()),
                           ("RaidGateVote_Layout.json", build_vote_layout())):
        (data_out / file_name).write_text(json.dumps(doc, indent=1, ensure_ascii=False) + "\n",
                                          encoding="utf-8")
        print("wrote", data_out / file_name, "(%d slots)" % len(doc["slots"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
