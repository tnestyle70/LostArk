#!/usr/bin/env python3
"""Builds the Colosseum ("Proving Grounds") match-queue dialog UI: the 15 second accept / decline
offer and the "cleaning the arena floor" wait window that the Bern Colosseum NPC opens.

Retail art used (UModel TGA exports, see dump_upk_movie.py / gfx_scene_extract.py for how the packages
were opened):
  EFUI_DIALOG          dialog_i1.tga        the gold remaining-time bar of the generic timer dialog
                                            (DialogRemainProgress) -> remain_bar.png
  EFUI_COMMONOBJECT    globalobject_i4.tga  the blue and the white comet arcs of the client's
                                            loading circle -> ring_blue.png / ring_white.png, each
                                            re-centred so the arc's own circle centre is the image
                                            centre (the runtime turns the image about its centre)
Panel, buttons and the accept / decline icons are the existing UI/Common art (the same generic dialog
look CLevel_Bern's raid confirm already uses).

MEASURED, not retail data (no numbers for them were recoverable from the movie): every rect below is
measured off the two retail captures (Screenshots 215831 = offer, 405 x 204 px; 215917 = wait,
409 x 261 px) and scaled so the dialog is 571 reference px wide; the ring centres come from fitting
the arcs' bright ridge (blue) / skeleton (white).  The blue / white arcs sharing one ring centre and
the rotation speeds (CRaidEntryPreviewView) are assumptions.

Outputs: Client/Bin/Resources/UI/Colosseum/QueueDialog/*.png   (runtime input, Git-untracked)
         Data/UI/Colosseum/QueueDialog_Layout.json              (lostark.ui-layout v1, 1280x720)

usage: build_colosseum_queue_dialog_ui.py --repo <LostArk> --dialog-atlas <dialog_i1.tga> --global-atlas <globalobject_i4.tga>
"""
import argparse, json, os
import numpy as np
from PIL import Image

DLG_W = 571.0
OFFER_CAP = (405.0, 204.0)   # capture size of the offer dialog
WAIT_CAP = (409.0, 261.0)    # capture size of the wait dialog

# atlas pieces (atlas px)
BAR_BOX = (807, 164, 1018, 173)          # gold bar, alpha > 200 bbox is asserted below
BLUE_ARC_BOX = (322, 5, 437, 81)         # comet arc, bright head top-left, sweeping along the bottom
BLUE_CENTER = (390.0, 18.0)              # ring centre of the blue arc
WHITE_ARC_BOX = (847, 5, 899, 63)        # thinner comet arc
WHITE_CENTER = (903.5, 17.7)             # skeleton fit, residual 0.2 px
RING_CANVAS = 144                        # square canvas, ring centre at (72, 72)
RING_STROKE_RADIUS = 51.0                # atlas px, blue arc stroke centre (skeleton fit)


class Frame:
    """One dialog: capture-pixel coordinates -> 1280x720 reference coordinates."""

    def __init__(self, cap):
        self.s = DLG_W / cap[0]
        self.w = DLG_W
        self.h = cap[1] * self.s
        self.x = (1280.0 - self.w) / 2.0
        self.y = (720.0 - self.h) / 2.0

    def rect(self, x, y, w, h):
        return dict(x=round(self.x + x * self.s, 3), y=round(self.y + y * self.s, 3),
                    width=round(w * self.s, 3), height=round(h * self.s, 3))

    def whole(self, pad=0.0, height=None):
        return dict(x=round(self.x - pad, 3), y=round(self.y - pad, 3), width=round(self.w + 2 * pad, 3),
                    height=round((self.h if height is None else height) + 2 * pad, 3))

    def marker(self, cx, cy, w=320.0, h=24.0):
        """A position-only text anchor: centre (cx, cy) capture px, drawn nowhere (tint alpha 0)."""
        return self.rect(cx - w / 2.0 / self.s, cy - h / 2.0 / self.s, w / self.s, h / self.s)


def slot(sid, path, rect, tint=(1, 1, 1, 1)):
    return {
        "id": sid, "ownerClass": None, "type": 0, "rect": rect, "rotation": 0,
        "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": list(tint), "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1.0, "offset": {"x": 0, "y": 0}, "frames": [], "loop": True, "additive": False},
    }


def arc_canvas(atlas, box, center):
    """Crop the arc and put it on a square canvas whose centre is the arc's circle centre."""
    x0, y0, x1, y1 = box
    crop = atlas.crop(box)
    canvas = Image.new("RGBA", (RING_CANVAS, RING_CANVAS), (0, 0, 0, 0))
    ox = int(round(x0 - center[0] + RING_CANVAS / 2))
    oy = int(round(y0 - center[1] + RING_CANVAS / 2))
    assert 0 <= ox and 0 <= oy and ox + crop.width <= RING_CANVAS and oy + crop.height <= RING_CANVAS, (box, ox, oy)
    canvas.alpha_composite(crop, (ox, oy))
    return canvas


def build(args):
    res_dir = os.path.join(args.repo, "Client", "Bin", "Resources", "UI", "Colosseum", "QueueDialog")
    os.makedirs(res_dir, exist_ok=True)

    dialog = Image.open(args.dialog_atlas).convert("RGBA")
    a = np.array(dialog)[162:177, 805:1024, 3]
    ys, xs = np.nonzero(a > 200)
    got = (805 + int(xs.min()), 162 + int(ys.min()), 805 + int(xs.max()) + 1, 162 + int(ys.max()) + 1)
    assert got == BAR_BOX, "gold bar moved in the atlas: %r" % (got,)
    dialog.crop(BAR_BOX).save(os.path.join(res_dir, "remain_bar.png"))

    glob = Image.open(args.global_atlas).convert("RGBA")
    arc_canvas(glob, BLUE_ARC_BOX, BLUE_CENTER).save(os.path.join(res_dir, "ring_blue.png"))
    arc_canvas(glob, WHITE_ARC_BOX, WHITE_CENTER).save(os.path.join(res_dir, "ring_white.png"))

    WHITE = "UI/Common/White1x1.png"
    HIDDEN = (1, 1, 1, 0)
    o, w = Frame(OFFER_CAP), Frame(WAIT_CAP)
    slots = [
        # ---- offer: 405 x 204 capture ----------------------------------------------------
        slot("QueueOffer_Border", WHITE, o.whole(1.0), (0.30, 0.27, 0.22, 0.95)),
        slot("QueueOffer_Backdrop", WHITE, o.whole(), (0.045, 0.05, 0.06, 0.95)),
        slot("QueueOffer_Panel", "UI/Common/CreateCharacterModalPanel.png", o.whole(height=131.0)),
        slot("QueueOffer_BarTrack", WHITE, o.rect(59, 109.5, 287, 6), (0.11, 0.11, 0.12, 1.0)),
        slot("QueueOffer_BarFill", "UI/Colosseum/QueueDialog/remain_bar.png", o.rect(59, 109.5, 287, 6)),
        slot("QueueOffer_BarCap", WHITE, o.rect(59, 109.5, 2.2, 6), (1.0, 0.97, 0.82, 1.0)),
        slot("QueueOffer_AcceptButton", "UI/Common/NormalButton.png", o.rect(94, 158, 108, 36)),
        slot("QueueOffer_DeclineButton", "UI/Common/NormalButton.png", o.rect(206, 158, 110, 36)),
        slot("QueueOffer_AcceptIcon", "UI/Common/Accept.png", o.rect(100, 166, 20, 16)),
        slot("QueueOffer_DeclineIcon", "UI/Common/Decline.png", o.rect(217.5, 169, 15, 15)),
        slot("QueueOffer_TitleText", WHITE, o.marker(204, 22), HIDDEN),
        slot("QueueOffer_BodyText1", WHITE, o.marker(202.5, 67), HIDDEN),
        slot("QueueOffer_BodyText2", WHITE, o.marker(201.5, 88.5), HIDDEN),
        slot("QueueOffer_SecondsText", WHITE, o.marker(202, 132), HIDDEN),
        slot("QueueOffer_AcceptLabel", WHITE, o.marker(159.5, 175.5, 80, 24), HIDDEN),
        slot("QueueOffer_DeclineLabel", WHITE, o.marker(273.5, 175.5, 80, 24), HIDDEN),
        # ---- wait: 409 x 261 capture -----------------------------------------------------
        slot("QueueWait_Border", WHITE, w.whole(1.0), (0.30, 0.27, 0.22, 0.95)),
        slot("QueueWait_Backdrop", WHITE, w.whole(), (0.045, 0.05, 0.06, 0.95)),
        slot("QueueWait_Panel", "UI/Common/CreateCharacterModalPanel.png", w.whole(height=131.0)),
        slot("QueueWait_RingBlue", "UI/Colosseum/QueueDialog/ring_blue.png", None),
        slot("QueueWait_RingWhite", "UI/Colosseum/QueueDialog/ring_white.png", None),
        slot("QueueWait_TitleText", WHITE, w.marker(204, 22), HIDDEN),
        slot("QueueWait_BodyText", WHITE, w.marker(202.5, 67.5), HIDDEN),
    ]
    # ring: the blue arc's stroke radius (atlas px) maps onto the measured 40.1 capture-px radius
    ring_px = RING_CANVAS * 40.1 / RING_STROKE_RADIUS
    ring_rect = w.rect(207.7 - ring_px / 2.0, 166.9 - ring_px / 2.0, ring_px, ring_px)
    for s in slots:
        if s["id"].startswith("QueueWait_Ring"):
            s["rect"] = dict(ring_rect)
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}
    out = os.path.join(args.repo, "Data", "UI", "Colosseum", "QueueDialog_Layout.json")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        json.dump(doc, f, ensure_ascii=False, indent=2)
        f.write("\n")
    print("wrote", out, "and 3 PNGs in", res_dir)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", required=True)
    ap.add_argument("--dialog-atlas", required=True)
    ap.add_argument("--global-atlas", required=True)
    build(ap.parse_args())
