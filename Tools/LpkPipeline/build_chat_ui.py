"""Cut the retail chat window (EFUI_CHATTING chatting.gfx) and write its ui-layout document for
CChatWindowView.

Source: chatting.gfx sprite 340 ChattingChatWidget, which ChattingDocument places at stage
(1,1080) -- the widget is anchored to the screen's bottom-left and every child sits at a negative
local y. Geometry straight out of the document (local px, 1920x1080 stage):

  resizeRectangle_mc  shape 112 at (0,-150) 377x85   black 20% (rgba 0,0,0,51): the log panel
  chattingControl     textField YG760 14 px at (24,-148) 352x83
                      chatLogTopScroll    bg (910,175)-(18,16) at (1,-146.8), arrow
                                          (396,202)-(14,14) at (3,-144.8)
                      chatLogBottomScroll same art flipped: bg at (1,-83.8), arrow at (3,-83.8)
  chattingInputbar    chattingInputTypeCombobox: arrow (458,175)-(25,24) at (1.2,-60),
                                          channel label YG760 14 px centred at (28.2,-60) 60x23
                      inputMessage_txt    YG760 14 px at (108.2,-61) 235x23
  lockButton_mc       (917,146)-(24,25) at (299,-178)
  optionButton_mc     (749,146)-(26,25) at (323,-178)
  minimum_btn         (665,146)-(26,25) at (351,-177)
  chattingTabGroup    tab button bg (842,0)-(94,32) at (0,-180), 96 px pitch;
                      add-tab bg (938,0)-(29,32) with the plus (872,175)-(17,17) at (+6,+8)
  slideNoticeCanvas   bg (380,116)-(374,28) at (0,-28)
The input box, tab-group and combobox "backgrounds" are alpha-0 hit areas in the document, so the
only panel art is the 20% black rectangle above; it is generated here rather than cropped.

Input: --page  chatting_i1 (.png/.dds), exported with
       umodel_lostark_v7 -export -dds -game=lostark -kr -obj=chatting_i1 EFUI_CHATTING.upk
Writes Client/Bin/Resources/UI/Chat/*.png and Data/UI/Chat/ChatWindow_Layout.json.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

STAGE_TO_REF = 2.0 / 3.0
# ChattingDocument places mainChattingWidget here; local y is negative from the screen bottom.
WIDGET_STAGE = (1.0, 1080.0)
TAB_PITCH = 96.0

# name -> (x, y, w, h) on chatting_i1
CROPS = {
    "Chat_ScrollButton": (910, 175, 18, 16),
    "Chat_ScrollArrowUp": (396, 202, 14, 14),
    "Chat_ScrollArrowDown": (460, 202, 14, 14),
    "Chat_ComboArrow": (458, 175, 25, 24),
    "Chat_LockButton": (917, 146, 24, 25),
    "Chat_OptionButton": (749, 146, 26, 25),
    "Chat_MinimizeButton": (665, 146, 26, 25),
    "Chat_TabBg": (842, 0, 94, 32),
    "Chat_TabAddBg": (938, 0, 29, 32),
    "Chat_TabAddPlus": (872, 175, 17, 17),
    "Chat_NoticeBg": (380, 116, 374, 28),
}
# The log panel is a plain 20% black rectangle in the document (shape 112).
PANEL = ("Chat_LogPanel", (0, 0, 0, 51), 377, 85)

# id -> (local x, local y, w, h, art or None for a text/marker slot, flipY)
SLOTS = [
    ("Chat_LogPanel", 0.0, -150.0, 377.0, 85.0, "Chat_LogPanel", False),
    ("Chat_LogTextBox", 24.0, -148.0, 352.0, 83.0, None, False),
    ("Chat_ScrollUp", 1.0, -146.8, 18.0, 16.0, "Chat_ScrollButton", False),
    ("Chat_ScrollUpArrow", 3.0, -144.8, 14.0, 14.0, "Chat_ScrollArrowUp", False),
    ("Chat_ScrollDown", 1.0, -83.8, 18.0, 16.0, "Chat_ScrollButton", True),
    ("Chat_ScrollDownArrow", 3.0, -83.8, 14.0, 14.0, "Chat_ScrollArrowDown", False),
    ("Chat_ComboArrow", 1.2, -60.0, 25.0, 24.0, "Chat_ComboArrow", False),
    ("Chat_ChannelTextBox", 28.2, -60.0, 60.0, 23.0, None, False),
    ("Chat_InputTextBox", 108.2, -61.0, 235.0, 23.0, None, False),
    ("Chat_LockButton", 299.0, -178.0, 24.0, 25.0, "Chat_LockButton", False),
    ("Chat_OptionButton", 323.0, -178.0, 26.0, 25.0, "Chat_OptionButton", False),
    ("Chat_MinimizeButton", 351.0, -177.0, 26.0, 25.0, "Chat_MinimizeButton", False),
    ("Chat_Tab0", 0.0, -180.0, 94.0, 32.0, "Chat_TabBg", False),
    ("Chat_Tab0TextBox", 0.0, -173.0, 94.0, 25.0, None, False),
    ("Chat_TabAdd", TAB_PITCH, -180.0, 29.0, 32.0, "Chat_TabAddBg", False),
    ("Chat_TabAddPlus", TAB_PITCH + 6.0, -172.0, 17.0, 17.0, "Chat_TabAddPlus", False),
]


def layout_slot(slot_id, x, y, w, h, path, flip_y=False):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round(x, 2), "y": round(y, 2), "width": round(w, 2), "height": round(h, 2)},
        # The bottom scroll button is the top one turned over, as the document flips it.
        "rotation": 180 if flip_y else 0,
        "stages": {"baseFrom": 0, "shineFrom": 1 if path else 0},
        "layers": ([{"path": "UI/Chat/" + path + ".png", "hoverPath": None, "tint": [1, 1, 1, 1],
                     "additive": False, "flipX": False}] if path else []),
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument("--page", type=Path, required=True)
    args = ap.parse_args()

    out_dir = args.repo / "Client/Bin/Resources/UI/Chat"
    out_dir.mkdir(parents=True, exist_ok=True)
    page = Image.open(args.page).convert("RGBA")
    for name, (x, y, w, h) in CROPS.items():
        crop = page.crop((x, y, x + w, y + h))
        if crop.getchannel("A").getbbox() is None:
            print("  ! empty crop", name)
            return 1
        crop.save(out_dir / (name + ".png"))
    name, rgba, w, h = PANEL
    Image.new("RGBA", (w, h), rgba).save(out_dir / (name + ".png"))

    slots = []
    for slot_id, x, y, w, h, path, flip_y in SLOTS:
        sx = (WIDGET_STAGE[0] + x) * STAGE_TO_REF
        sy = (WIDGET_STAGE[1] + y) * STAGE_TO_REF
        slots.append(layout_slot(slot_id, sx, sy, w * STAGE_TO_REF, h * STAGE_TO_REF, path, flip_y))
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}
    layout = args.repo / "Data/UI/Chat/ChatWindow_Layout.json"
    layout.write_text(json.dumps(doc, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {len(CROPS) + 1} png -> {out_dir}")
    print(f"wrote {layout} ({len(slots)} slots)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
