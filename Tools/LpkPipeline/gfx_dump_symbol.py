#!/usr/bin/env python3
"""Dump (and optionally composite) a Scaleform .gfx symbol from its ffdec XML dump.

The retail UI .gfx files keep their bitmaps in external atlas pages (DefineExternalImage2 ->
DefineSubImage rectangles), which ffdec cannot render. This walks a symbol's placement tree,
resolves every bitmap-filled shape to (atlas page, sub-image rect, destination px) and lists the
edit texts (font class / size / colour), so the crops and the layout can be lifted exactly.

  python gfx_dump_symbol.py <ffdec.xml> <page png dir> <symbol name>... [--png out_dir]

Positions are stage px (twips / 20) relative to the symbol origin, scale accumulated through the
tree. --png pastes the crops into one RGBA image per symbol for a quick visual check.
"""
from __future__ import annotations

import argparse
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


class Gfx:
    def __init__(self, xml_path: Path, page_dir: Path):
        self.root = ET.parse(xml_path).getroot()
        self.page_dir = page_dir
        self.chars: dict[str, ET.Element] = {}
        self.names: dict[str, str] = {}
        self.sub: dict[str, tuple[str, int, int, int, int]] = {}
        self.ext: dict[str, str] = {}
        for it in self.root.iter("item"):
            t = it.get("type", "")
            cid = it.get("characterID") or it.get("spriteId") or it.get("shapeId")
            if cid and t not in ("DefineSubImage", "DefineExternalImage2"):
                self.chars[cid] = it
            if t in ("SymbolClassTag", "ExportAssetsTag"):
                ids = [x.text or x.get("value") for x in it.find("tags")]
                nms = [x.text or x.get("value") for x in it.find("names")]
                for i, n in zip(ids, nms):
                    self.names.setdefault(i, n)
            elif t == "DefineSubImage":
                self.sub[it.get("characterID")] = (it.get("imageId"), int(it.get("x1")), int(it.get("y1")),
                                                   int(it.get("x2")), int(it.get("y2")))
            elif t == "DefineExternalImage2":
                self.ext[it.get("imageID")] = it.get("fileName")
        self.by_name = {n: i for i, n in self.names.items()}

    def page_png(self, image_id: str) -> Path | None:
        name = self.ext.get(image_id)
        if not name:
            return None
        stem = name.rsplit(".", 1)[0].lower()
        for cand in (self.page_dir / (stem + ".png"), self.page_dir / (stem.replace(".nopack", "") + ".png")):
            if cand.exists():
                return cand
        return None

    def walk(self, cid: str, ox: float, oy: float, sx: float, sy: float, depth: int, out: list, path: str):
        it = self.chars.get(cid)
        if it is None or depth > 8:
            return
        t = it.get("type", "")
        if t.startswith("DefineShape"):
            b = it.find("shapeBounds")
            bx, by = int(b.get("Xmin")) / 20.0, int(b.get("Ymin")) / 20.0
            bw, bh = (int(b.get("Xmax")) - int(b.get("Xmin"))) / 20.0, (int(b.get("Ymax")) - int(b.get("Ymin"))) / 20.0
            fills = it.find("shapes/fillStyles")
            bitmaps = []
            if fills is not None:
                # FILLSTYLEARRAY -> fillStyles -> item(FILLSTYLE); 65535 = no bitmap
                for f in fills.iter("item"):
                    if f.get("type") == "FILLSTYLE" and f.get("bitmapId") in self.sub:
                        m = f.find("bitmapMatrix")
                        bitmaps.append((f.get("bitmapId"), m))
            if not bitmaps:
                out.append({"kind": "shape", "path": path, "id": cid, "x": ox + bx * sx, "y": oy + by * sy,
                            "w": bw * sx, "h": bh * sy, "fill": "solid/gradient"})
            for bid, m in bitmaps:
                img, x1, y1, x2, y2 = self.sub[bid]
                # bitmap matrix: texture px -> shape twips; translate = where texel (0,0) lands
                mtx, mty = (int(m.get("translateX", 0)) / 20.0, int(m.get("translateY", 0)) / 20.0) if m is not None else (0.0, 0.0)
                msx = float(m.get("scaleX", 20.0)) / 20.0 if m is not None and m.get("hasScale") == "true" else 1.0
                msy = float(m.get("scaleY", 20.0)) / 20.0 if m is not None and m.get("hasScale") == "true" else 1.0
                out.append({"kind": "bitmap", "path": path, "id": cid, "page": self.ext.get(img), "rect": [x1, y1, x2 - x1, y2 - y1],
                            "x": ox + bx * sx, "y": oy + by * sy, "w": bw * sx, "h": bh * sy,
                            "texOrigin": [mtx, mty], "texScale": [msx * sx, msy * sy]})
            return
        if t == "DefineEditTextTag":
            b = it.find("bounds")
            out.append({"kind": "text", "path": path, "id": cid, "font": it.get("fontClass"), "px": int(it.get("fontHeight", 0)) / 20.0,
                        "align": it.get("align"), "color": it.get("textColor"), "leading": it.get("leading"),
                        "x": ox + int(b.get("Xmin")) / 20.0 * sx, "y": oy + int(b.get("Ymin")) / 20.0 * sy,
                        "w": (int(b.get("Xmax")) - int(b.get("Xmin"))) / 20.0 * sx, "h": (int(b.get("Ymax")) - int(b.get("Ymin"))) / 20.0 * sy,
                        "text": (it.get("initialText") or "")[:80]})
            return
        if t != "DefineSpriteTag":
            out.append({"kind": t, "path": path, "id": cid, "x": ox, "y": oy})
            return
        seen_depths = set()
        for sub in it.find("subTags"):
            st = sub.get("type", "")
            if not st.startswith("PlaceObject"):
                if st in ("ShowFrameTag",):
                    break  # first frame only
                continue
            c = sub.get("characterId")
            if not c or sub.get("depth") in seen_depths:
                continue
            seen_depths.add(sub.get("depth"))
            m = sub.find("matrix")
            tx, ty = (int(m.get("translateX", 0)) / 20.0, int(m.get("translateY", 0)) / 20.0) if m is not None else (0.0, 0.0)
            psx = float(m.get("scaleX", 1.0)) if m is not None and m.get("hasScale") == "true" else 1.0
            psy = float(m.get("scaleY", 1.0)) if m is not None and m.get("hasScale") == "true" else 1.0
            nm = sub.get("name") or self.names.get(c) or c
            self.walk(c, ox + tx * sx, oy + ty * sy, sx * psx, sy * psy, depth + 1, out, path + "/" + nm)

    def dump(self, name: str) -> list:
        cid = self.by_name.get(name) or (name if name in self.chars else None)
        if cid is None:
            print(f"!! unknown symbol {name}", file=sys.stderr)
            return []
        out: list = []
        self.walk(cid, 0.0, 0.0, 1.0, 1.0, 0, out, name)
        return out

    def composite(self, entries: list, out_png: Path):
        from PIL import Image
        bitmaps = [e for e in entries if e["kind"] == "bitmap"]
        if not bitmaps:
            return
        minx = min(e["x"] for e in bitmaps); miny = min(e["y"] for e in bitmaps)
        maxx = max(e["x"] + e["w"] for e in bitmaps); maxy = max(e["y"] + e["h"] for e in bitmaps)
        canvas = Image.new("RGBA", (max(1, int(round(maxx - minx))), max(1, int(round(maxy - miny)))), (0, 0, 0, 0))
        pages: dict[str, Image.Image] = {}
        for e in bitmaps:
            png = self.page_png(next((k for k, v in self.ext.items() if v == e["page"]), ""))
            if png is None:
                continue
            if png not in pages:
                pages[png] = Image.open(png).convert("RGBA")
            x, y, w, h = e["rect"]
            crop = pages[png].crop((x, y, x + w, y + h))
            dw, dh = max(1, int(round(e["w"]))), max(1, int(round(e["h"])))
            if (dw, dh) != crop.size:
                crop = crop.resize((dw, dh))
            canvas.alpha_composite(crop, (int(round(e["x"] - minx)), int(round(e["y"] - miny))))
        canvas.save(out_png)
        print(f"  -> {out_png} origin offset ({minx:.1f}, {miny:.1f}) size {canvas.size}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("xml", type=Path)
    ap.add_argument("pages", type=Path)
    ap.add_argument("symbols", nargs="+")
    ap.add_argument("--png", type=Path)
    a = ap.parse_args()
    g = Gfx(a.xml, a.pages)
    for s in a.symbols:
        entries = g.dump(s)
        print(f"== {s}")
        for e in entries:
            if e["kind"] == "bitmap":
                print(f"  BMP {e['path']}  page={e['page']} rect={e['rect']} at ({e['x']:.1f},{e['y']:.1f}) size ({e['w']:.1f}x{e['h']:.1f}) texScale={e['texScale']}")
            elif e["kind"] == "text":
                print(f"  TXT {e['path']}  {e['font']} {e['px']}px align={e['align']} color={e['color']} at ({e['x']:.1f},{e['y']:.1f}) size ({e['w']:.1f}x{e['h']:.1f}) {e['text']!r}")
            elif e["kind"] == "shape":
                print(f"  SHP {e['path']}  at ({e['x']:.1f},{e['y']:.1f}) size ({e['w']:.1f}x{e['h']:.1f}) {e['fill']}")
        if a.png:
            a.png.mkdir(parents=True, exist_ok=True)
            g.composite(entries, a.png / (s.replace(".", "_") + ".png"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
