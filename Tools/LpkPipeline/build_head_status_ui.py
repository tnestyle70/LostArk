#!/usr/bin/env python3
"""Build the over-head chat bubble art and layout from the retail headstatus.gfx.

The name plate itself is a LOA-font text pass (CWorldPlayerNameplateView) and needs no art; the
HeadStatusHPGauge of PcHeadStatusMc is not reproduced (the reference screen shows no gauge over a
player), so only the balloon is cut from the atlas.

Outputs
-------
  Client/Bin/Resources/UI/HeadStatus/HS_Balloon_Normal.png   72x50
      HeadStatus_Balloon normalBG (headStatus_I6 384,0), a 9-slice whose DefineScalingGrid centre is
      x 18.75..20.5 / y 17.75..19.75 px -- the fixed right/bottom slices carry the tail.
  Data/UI/HeadStatus/ChatBubble_Layout.json
      CB_<i>_<piece> slots (9 pieces x BUBBLE_SLOTS bubbles) for CWorldPlayerChatBubbleView; the view
      sizes/positions every piece and sets its UV window from BALLOON_GRID each frame.

Source: D:/ClaudeWork/Extracted/HeadStatus (ffdec XML + umodel texture); placement in
.md/TJ/09-14/2026-09-14_이름표_PLAN.md section 2.

Usage:
  python build_head_status_ui.py [--repo <LostArk root>]
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from PIL import Image

ATLAS = Path(r"D:/ClaudeWork/Extracted/HeadStatus/tex/EFUI_STATUS/Texture2D/headstatus_i6.dds")

RETAIL_SCALE = 2.0 / 3.0
BALLOON = (384, 0, 72, 50)                 # HeadStatus_Balloon normalBG
BALLOON_GRID = (18.75, 20.5, 17.75, 19.75) # scaling grid x0, x1, y0, y1 in art px (twips/20)
BUBBLE_SLOTS = 4
BUBBLE_PIECES = ("tl", "t", "tr", "l", "c", "r", "bl", "b", "br")


def slot(slot_id, w, h, path):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        # parked off screen; the view positions every visible one per frame
        "rect": {"x": -1000, "y": -1000, "width": round(w * RETAIL_SCALE, 4), "height": round(h * RETAIL_SCALE, 4)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": [1, 1, 1, 1], "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0}, "frames": [], "loop": True, "additive": False},
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo

    art_dir = repo / "Client/Bin/Resources/UI/HeadStatus"
    art_dir.mkdir(parents=True, exist_ok=True)
    atlas = Image.open(ATLAS).convert("RGBA")
    bx, by, bw, bh = BALLOON
    atlas.crop((bx, by, bx + bw, by + bh)).save(art_dir / "HS_Balloon_Normal.png")

    data_dir = repo / "Data/UI/HeadStatus"
    data_dir.mkdir(parents=True, exist_ok=True)
    bubble_slots = [slot("CB_%d_%s" % (i, piece), 8, 8, "UI/HeadStatus/HS_Balloon_Normal.png")
                    for i in range(BUBBLE_SLOTS) for piece in BUBBLE_PIECES]
    (data_dir / "ChatBubble_Layout.json").write_text(json.dumps({
        "schema": "lostark.ui-layout", "formatVersion": 1,
        "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": bubble_slots,
        "balloon": {"artWidth": bw, "artHeight": bh,
                    "grid": {"x0": BALLOON_GRID[0], "x1": BALLOON_GRID[1], "y0": BALLOON_GRID[2], "y1": BALLOON_GRID[3]}},
    }, ensure_ascii=False, indent=2), encoding="utf-8")
    print("art: balloon -> %s; layout: %d bubble slots" % (art_dir, len(bubble_slots)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
