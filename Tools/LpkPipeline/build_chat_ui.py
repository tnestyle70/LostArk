"""Cut the retail chat window (EFUI_CHATTING chatting.gfx) and write its ui-layout document for
CChatWindowView.

Source: chatting.gfx sprite 340 ChattingChatWidget, which ChattingDocument places at stage
(1,1080) -- the widget is anchored to the screen's bottom-left and every child sits at a negative
local y. The tree below is the frame-1 display list straight out of the ffdec XML (local px on the
1920x1080 stage, character ids in brackets):

  chattingTabGroup [339]     at (0,-186): background [337] 290x42 rgba(214,214,214,0), a hit area.
                             The tab buttons themselves are ChattingTabButton [133] instances the
                             host adds per channel.
  resizeRectangle_mc [116]   at (0,-150) scale (0.95,0.335): shape [112] 397x254 rgba(0,0,0,51),
                             the drag/resize surface under the panel.
  chattingControl [111]      at (0,-149), ChattingControlComponent:
      background [85]        scale (1,0.374) of shape [84], bitmap [83] = page (0,0)-(378,227).
                             This is the panel people actually see: ~75% black art, not the 20%
                             rectangle above it.
      textField [86]         at (26,3), 352x83, $YG760 fontHeight 280 (14 px) leading 40 (2 px).
      chatLogTopScroll [110] at (3,1.1): bg [88] (910,175)-(928,191), arrow [101] (396,202)-(410,216)
                             at +(0,3).
      chatLogBottomScroll [99] at (3,82.2) scaleY -1: same bg, arrow [90] (460,202)-(474,216).
  chattingInputbar [336]     at (0.2,-39), ChattingInput:
      chattingInputTypeCombobox [326] at (0,-25): bg [318] 101x32 from bitmap [317]
                             (851,37)-(944,65), arrow [321] (458,175)-(483,199) at (1,4),
                             its label textField [319] 60x23 at (30,6), align centre.
      inputBoxBG [331]       at (93,8) scale (0.9595,0.9211): outer [328] 297x38 at (0,-39) from
                             bitmap [327] (554,37)-(849,69), inner [330] 292x30 at (4,-34) from
                             bitmap [329] (554,71)-(846,101).
      inputType_txt [332]    at (110,-20) 118x23, inputMessage_txt [333] at (110,-20) 235x23.
      languageIcon_mc [316]  at (341,-21): frame 1 is an empty 34x24 box (Latin input shows
                             nothing), frame 2 is the Korean glyph [303] (962,175)-(977,189) at
                             local (10,5). Frames 3..8 are 中/РУС/あ/DE/FR/ES, which this client
                             does not use. setLanguageType(n) does gotoAndStop(n+1).

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
# The retail widget reads too small on this project's screens, so every local offset and size is
# scaled about the widget's own bottom-left anchor. 1.0 reproduces the retail size exactly.
WIDGET_SCALE = 1.25
# ChattingDocument places mainChattingWidget here; local y is negative from the screen bottom.
WIDGET_STAGE = (1.0, 1080.0)
TAB_PITCH = 96.0

# name -> (x1, y1, x2, y2) on chatting_i1, from the DefineSubImage of that shape's bitmap fill
CROPS = {
    "Chat_LogPanelArt": (0, 0, 378, 227),
    "Chat_InputBoxOuter": (554, 37, 849, 69),
    "Chat_InputBoxInner": (554, 71, 846, 101),
    "Chat_ComboBg": (851, 37, 944, 65),
    "Chat_ComboArrow": (458, 175, 483, 199),
    "Chat_LangKorean": (962, 175, 977, 189),
    "Chat_ScrollButton": (910, 175, 928, 191),
    "Chat_ScrollArrowUp": (396, 202, 410, 216),
    "Chat_ScrollArrowDown": (460, 202, 474, 216),
    "Chat_LockButton": (917, 146, 941, 171),
    "Chat_OptionButton": (749, 146, 775, 171),
    "Chat_MinimizeButton": (665, 146, 691, 171),
    "Chat_TabBg": (842, 0, 936, 32),
    "Chat_TabAddBg": (938, 0, 967, 32),
    "Chat_TabAddPlus": (872, 175, 889, 192),
}
# resizeRectangle_mc's own shape: a plain 20% black rectangle under the panel art.
PANEL = ("Chat_LogPanel", (0, 0, 0, 51), 377, 85)

# id -> (local x, local y, w, h, art or None for a text/marker slot, flipY)
SLOTS = [
    ("Chat_LogPanel", 0.0, -150.0, 377.0, 85.0, "Chat_LogPanel", False),
    ("Chat_LogPanelArt", 0.0, -149.0, 378.0, 84.9, "Chat_LogPanelArt", False),
    ("Chat_LogTextBox", 24.0, -148.0, 352.0, 83.0, None, False),
    ("Chat_ScrollUp", 3.0, -147.9, 18.0, 16.0, "Chat_ScrollButton", False),
    ("Chat_ScrollUpArrow", 3.0, -144.9, 14.0, 14.0, "Chat_ScrollArrowUp", False),
    ("Chat_ScrollDown", 3.0, -82.8, 18.0, 16.0, "Chat_ScrollButton", True),
    ("Chat_ScrollDownArrow", 3.0, -79.8, 14.0, 14.0, "Chat_ScrollArrowDown", False),
    # chattingInputbar (0.2,-39) + combobox (0,-25)
    ("Chat_ComboBg", 0.2, -64.0, 101.0, 32.0, "Chat_ComboBg", False),
    ("Chat_ComboArrow", 1.2, -60.0, 25.0, 24.0, "Chat_ComboArrow", False),
    ("Chat_ChannelTextBox", 28.2, -60.0, 60.0, 23.0, None, False),
    # inputBoxBG (93,8) scale (0.9595,0.9211) applied to its two children
    ("Chat_InputBoxOuter", 93.2, -66.9, 285.0, 35.0, "Chat_InputBoxOuter", False),
    ("Chat_InputBoxInner", 97.0, -62.3, 280.2, 27.6, "Chat_InputBoxInner", False),
    ("Chat_InputTextBox", 110.2, -59.0, 225.0, 23.0, None, False),
    ("Chat_LangIcon", 351.2, -55.0, 15.0, 14.0, "Chat_LangKorean", False),
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
    for name, (x1, y1, x2, y2) in CROPS.items():
        crop = page.crop((x1, y1, x2, y2))
        if crop.getchannel("A").getbbox() is None:
            print("  ! empty crop", name)
            return 1
        crop.save(out_dir / (name + ".png"))
    name, rgba, w, h = PANEL
    Image.new("RGBA", (w, h), rgba).save(out_dir / (name + ".png"))

    slots = []
    for slot_id, x, y, w, h, path, flip_y in SLOTS:
        sx = (WIDGET_STAGE[0] + x * WIDGET_SCALE) * STAGE_TO_REF
        sy = (WIDGET_STAGE[1] + y * WIDGET_SCALE) * STAGE_TO_REF
        slots.append(layout_slot(slot_id, sx, sy, w * WIDGET_SCALE * STAGE_TO_REF,
                                 h * WIDGET_SCALE * STAGE_TO_REF, path, flip_y))
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}
    layout = args.repo / "Data/UI/Chat/ChatWindow_Layout.json"
    layout.write_text(json.dumps(doc, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {len(CROPS) + 1} png -> {out_dir}")
    print(f"wrote {layout} ({len(slots)} slots)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
