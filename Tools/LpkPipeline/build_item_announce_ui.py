"""Cut the retail item acquisition announce (EFUI_ANNOUNCE ItemSlotAnnounceListItem) and write its
ui-layout document for CLevel_ValtanArena's item announce.

Retail source: EFUI_ANNOUNCE (OVSG0AWFFMSFAODR2YW6VV.upk, announce.gfx, 1920x1080 stage, 40 fps).
The host routes an item acquisition to AnnounceWnd.updateLuxuryItemCtrl (ANNOUNCE_UPDATE_LUXURY_ITEM,
list origin SetPositionUV 0.5 / 750 of 1080, align top, alignHorizontal center) with category
ItemSlotAnnounceListItem (sprite 360). The list centres an item on its marginMc width (610), so the
item origin sits at stage (960 - 305, 750). Placements inside sprite 360 at its settled frame:

  depth 3  bitmap 271  Announce_I55 (0,924)-(642,82)    at (61,-74)   band (slides in from x 51)
  depth 4  bitmap 273  Announce_I20 (729,881)-(105,107) at (0,-86)    slot frame
  depth 5  slot        ARKNewSlot 64x64 icon            at (22,-62)   scale 0.8283386
  depth 9  qualityProgress (sprite 293)                 at (92,-94):
             bitmap 275 Announce_I20 (643,561)-(228,23) at (0,-1)     quality plate
             text   276 "[$]item.option_quality"        at (14,2)     bounds x -2..72, 12 px centred
             bitmap 278 Announce_I23 (857,397)-(92,18)  at (108,2)    gauge well
             track  291 frames 1..6 (81x6 bars)          at (114,7)    revealed by the target mask
             text   292 targetText                      at (21.1,2)   bounds x 54.9..87.9, right
  depth 42 textField 95 16 px centred                  at (140,-44)  bounds -2..398 x -2..30
Gauge frames (ColorTransformSimpleProgress colorList "0,1 / 1,1 / 10,2 / 30,3 / 70,4 / 90,5 / 100,6"):
  1 = 280 (83,1016)  2 = 282 (0,1016)  3 = 284 (415,1016)  4 = 286 (249,1016)
  5 = 288 (332,1016) 6 = 290 (166,1016), all 81x6 on Announce_I5.

Light sweep (frames 10..28): depth 6 is shape 96, a 58x58 white square at (22,-62) placed with
clipDepth 8, masking depth 7 = shape 97, a 52x98 quad with a linear white gradient (alpha 0 / 210 /
0 at ratios 0 / 215 / 255) that each frame moves along a 45-degree matrix across the icon. The sweep
is vector art, so it is rasterised here per frame from the shape records, the gradient matrix and
the per-frame placement matrix, clipped to the mask, into one 58x58 image per frame.

Inputs:
  --pages  announce_i5 / announce_i20 / announce_i23 / announce_i55 (.dds/.png/.tga), exported with
           umodel_lostark_v7 -export -dds -game=lostark -kr, object announce_iN
  --xml    ffdec -swf2xml dump of announce.gfx
Writes Client/Bin/Resources/UI/ItemAnnounce/*.png and Data/UI/ItemAnnounce/ItemAnnounce_Layout.json.
"""

from __future__ import annotations

import argparse
import json
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

STAGE_TO_REF = 2.0 / 3.0
ORIGIN = (960.0 - 305.0, 750.0)
SLOT_ICON_SCALE = 0.8283386

# name -> (page, x, y, w, h)
CROPS = {
    "ItemAnnounce_Band": ("announce_i55", 0, 924, 642, 82),
    "ItemAnnounce_SlotFrame": ("announce_i20", 729, 881, 105, 107),
    "ItemAnnounce_QualityPlate": ("announce_i20", 643, 561, 228, 23),
    "ItemAnnounce_QualityWell": ("announce_i23", 857, 397, 92, 18),
}
GAUGE_FRAMES = [(83, 1016), (0, 1016), (415, 1016), (249, 1016), (332, 1016), (166, 1016)]

QUALITY_ORIGIN = (92.0, -94.0)
# slot id -> (stage x, stage y relative to the item origin, width, height, texture or None)
SLOTS = [
    ("ItemAnnounce_Band", 61.0, -74.0, 642.0, 82.0, "UI/ItemAnnounce/ItemAnnounce_Band.png"),
    ("ItemAnnounce_SlotFrame", 0.0, -86.0, 105.0, 107.0, "UI/ItemAnnounce/ItemAnnounce_SlotFrame.png"),
    ("ItemAnnounce_Icon", 22.0, -62.0, 64.0 * SLOT_ICON_SCALE, 64.0 * SLOT_ICON_SCALE,
     "UI/Items/Common/necklace_a.png"),
    ("ItemAnnounce_Sweep", 22.0, -62.0, 58.0, 58.0, "UI/ItemAnnounce/ItemAnnounce_Sweep_10.png"),
    ("ItemAnnounce_QualityPlate", QUALITY_ORIGIN[0] + 0.0, QUALITY_ORIGIN[1] - 1.0, 228.0, 23.0,
     "UI/ItemAnnounce/ItemAnnounce_QualityPlate.png"),
    ("ItemAnnounce_QualityWell", QUALITY_ORIGIN[0] + 108.0, QUALITY_ORIGIN[1] + 2.0, 92.0, 18.0,
     "UI/ItemAnnounce/ItemAnnounce_QualityWell.png"),
    ("ItemAnnounce_QualityGauge", QUALITY_ORIGIN[0] + 114.0, QUALITY_ORIGIN[1] + 7.0, 81.0, 6.0,
     "UI/ItemAnnounce/ItemAnnounce_QualityGauge_4.png"),
    # Text boxes: markers only, the text pass draws into their rects.
    ("ItemAnnounce_QualityLabelBox", QUALITY_ORIGIN[0] + 14.0 - 2.0, QUALITY_ORIGIN[1] + 2.0 - 2.0,
     74.0, 20.0, None),
    ("ItemAnnounce_QualityValueBox", QUALITY_ORIGIN[0] + 21.1 + 54.9, QUALITY_ORIGIN[1] + 2.0 - 2.0,
     33.0, 20.0, None),
    ("ItemAnnounce_TextBox", 140.0 - 2.0, -44.0 - 2.0, 400.0, 32.0, None),
]


def layout_slot(slot_id, x, y, w, h, path):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round(x, 2), "y": round(y, 2), "width": round(w, 2), "height": round(h, 2)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1 if path else 0},
        "layers": ([{"path": path, "hoverPath": None, "tint": [1, 1, 1, 1],
                     "additive": False, "flipX": False}] if path else []),
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def find_page(folder: Path, stem: str) -> Image.Image:
    for ext in (".dds", ".png", ".tga"):
        for name in (stem, stem.lower(), stem.capitalize()):
            p = folder / (name + ext)
            if p.exists():
                return Image.open(p).convert("RGBA")
    raise FileNotFoundError(folder / stem)


SWEEP_SPRITE, SWEEP_MASK_DEPTH, SWEEP_DEPTH = "360", "6", "7"
SUPERSAMPLE = 4
SHAPE_TAGS = ("DefineShapeTag", "DefineShape2Tag", "DefineShape3Tag", "DefineShape4Tag")


def read_sweep(xml_path: Path):
    """Shapes by id, the mask placement, and each frame's sweep placement in sprite 360."""
    shapes: dict[str, ET.Element] = {}
    frames: list[tuple[int, dict]] = []
    mask = None
    for _, el in ET.iterparse(xml_path, events=("end",)):
        if el.tag != "item":
            continue
        kind = el.get("type", "")
        if kind in SHAPE_TAGS:
            shapes[el.get("shapeId")] = el
            continue
        if kind == "DefineSpriteTag" and el.get("spriteId") == SWEEP_SPRITE:
            frame = 1
            for tag in el.find("subTags"):
                if tag.get("type") == "ShowFrameTag":
                    frame += 1
                    continue
                if not tag.get("type", "").startswith("PlaceObject"):
                    continue
                m = tag.find("matrix")
                if tag.get("depth") == SWEEP_MASK_DEPTH and m is not None:
                    mask = (tag.get("characterId"), dict(m.attrib))
                if tag.get("depth") == SWEEP_DEPTH and m is not None:
                    char = tag.get("characterId") or (frames[-1][1]["char"] if frames else None)
                    frames.append((frame, {"char": char, **m.attrib}))
            break
    return shapes, mask, frames


def matrix(attrs: dict):
    scaled = attrs.get("hasScale") == "true"
    rotated = attrs.get("hasRotate") == "true"
    a = float(attrs.get("scaleX", 1)) if scaled else 1.0
    d = float(attrs.get("scaleY", 1)) if scaled else 1.0
    b = float(attrs.get("rotateSkew0", 0)) if rotated else 0.0
    c = float(attrs.get("rotateSkew1", 0)) if rotated else 0.0
    return a, b, c, d, float(attrs.get("translateX", 0)), float(attrs.get("translateY", 0))


def invert(m):
    a, b, c, d, tx, ty = m
    det = a * d - b * c
    return (d / det, -b / det, -c / det, a / det,
            (c * ty - d * tx) / det, (b * tx - a * ty) / det)


def apply(m, x, y):
    a, b, c, d, tx, ty = m
    return a * x + c * y + tx, b * x + d * y + ty


def shape_polygon(shape: ET.Element):
    """The single straight-edged outline of a shape, in twips."""
    x = y = 0.0
    points = []
    for rec in shape.find("shapes").find("shapeRecords"):
        kind = rec.get("type")
        if kind == "StyleChangeRecord" and rec.get("stateMoveTo") == "true":
            x, y = float(rec.get("moveDeltaX")), float(rec.get("moveDeltaY"))
            points.append((x, y))
        elif kind == "StraightEdgeRecord":
            if rec.get("generalLineFlag") == "true":
                x += float(rec.get("deltaX", 0))
                y += float(rec.get("deltaY", 0))
            elif rec.get("vertLineFlag") == "true":
                y += float(rec.get("deltaY", 0))
            else:
                x += float(rec.get("deltaX", 0))
            points.append((x, y))
        elif kind == "CurvedEdgeRecord":
            raise ValueError("curved sweep outlines are not handled")
    return points


def inside(poly, x, y):
    hit = False
    for i in range(len(poly)):
        x1, y1 = poly[i]
        x2, y2 = poly[(i + 1) % len(poly)]
        if (y1 > y) != (y2 > y) and x < (x2 - x1) * (y - y1) / (y2 - y1) + x1:
            hit = not hit
    return hit


def gradient_alpha(shape: ET.Element, x, y):
    """Alpha of a linear-gradient fill (the gradient square spans -16384..16384 twips)."""
    style = shape.find("shapes").find("fillStyles").find("fillStyles")[0]
    inv = invert(matrix({"hasScale": "true", **style.find("gradientMatrix").attrib}))
    gx, _ = apply(inv, x, y)
    ratio = max(0.0, min(255.0, (gx + 16384.0) / 32768.0 * 255.0))
    recs = [(float(r.get("ratio")), float(r.find("color").get("alpha")))
            for r in style.find("gradient").find("gradientRecords")]
    for (r0, a0), (r1, a1) in zip(recs, recs[1:]):
        if r0 <= ratio <= r1:
            return a0 + (a1 - a0) * (0.0 if r1 == r0 else (ratio - r0) / (r1 - r0))
    return recs[-1][1]


def bake_sweep(xml_path: Path, out_dir: Path) -> list[int]:
    shapes, mask, frames = read_sweep(xml_path)
    mask_m = matrix(mask[1])
    mask_poly = [apply(mask_m, px, py) for px, py in shape_polygon(shapes[mask[0]])]
    left = min(p[0] for p in mask_poly)
    top = min(p[1] for p in mask_poly)
    width = round((max(p[0] for p in mask_poly) - left) / 20)
    height = round((max(p[1] for p in mask_poly) - top) / 20)
    baked = []
    for frame, attrs in frames:
        shape = shapes[attrs["char"]]
        poly = shape_polygon(shape)
        inv = invert(matrix(attrs))
        img = Image.new("RGBA", (width, height))
        pixels = img.load()
        for iy in range(height):
            for ix in range(width):
                acc = 0.0
                for sy in range(SUPERSAMPLE):
                    for sx in range(SUPERSAMPLE):
                        x = left + (ix + (sx + 0.5) / SUPERSAMPLE) * 20
                        y = top + (iy + (sy + 0.5) / SUPERSAMPLE) * 20
                        if not inside(mask_poly, x, y):
                            continue
                        lx, ly = apply(inv, x, y)
                        if inside(poly, lx, ly):
                            acc += gradient_alpha(shape, lx, ly)
                pixels[ix, iy] = (255, 255, 255, int(round(acc / (SUPERSAMPLE * SUPERSAMPLE))))
        img.save(out_dir / f"ItemAnnounce_Sweep_{frame}.png")
        baked.append(frame)
    return baked


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument("--pages", type=Path, required=True)
    ap.add_argument("--xml", type=Path, required=True)
    args = ap.parse_args()

    out_dir = args.repo / "Client/Bin/Resources/UI/ItemAnnounce"
    out_dir.mkdir(parents=True, exist_ok=True)
    pages: dict[str, Image.Image] = {}

    def page(stem: str) -> Image.Image:
        if stem not in pages:
            pages[stem] = find_page(args.pages, stem)
        return pages[stem]

    for name, (stem, x, y, w, h) in CROPS.items():
        page(stem).crop((x, y, x + w, y + h)).save(out_dir / f"{name}.png")
    for index, (x, y) in enumerate(GAUGE_FRAMES, start=1):
        page("announce_i5").crop((x, y, x + 81, y + 6)).save(out_dir / f"ItemAnnounce_QualityGauge_{index}.png")

    sweep_frames = bake_sweep(args.xml, out_dir)
    print(f"baked sweep frames {sweep_frames[0]}..{sweep_frames[-1]}")

    slots = []
    for slot_id, x, y, w, h, path in SLOTS:
        sx = (ORIGIN[0] + x) * STAGE_TO_REF
        sy = (ORIGIN[1] + y) * STAGE_TO_REF
        slots.append(layout_slot(slot_id, sx, sy, w * STAGE_TO_REF, h * STAGE_TO_REF, path))
    doc = {
        "schema": "lostark.ui-layout", "formatVersion": 1,
        "resolution": {"width": 1280, "height": 720},
        "classes": ["Default"],
        "slots": slots,
    }
    layout = args.repo / "Data/UI/ItemAnnounce/ItemAnnounce_Layout.json"
    layout.write_text(json.dumps(doc, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {len(CROPS) + len(GAUGE_FRAMES)} crops -> {out_dir}")
    print(f"wrote {layout}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
