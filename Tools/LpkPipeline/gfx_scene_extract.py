#!/usr/bin/env python3
"""Scaleform GFX scene extractor for retail UI movies whose PlaceObject3 tags set the
"HasImage" bit (0x10 of the second flag byte) WITHOUT a class-name string.

gfx_native_parse.parse_place() follows the SWF spec and reads a class name when
(HasImage and HasCharacter); colosseumloadings3.gfx (and any movie exported by the same Scaleform
exporter) does not carry that string, so every such placement came out with a garbage character id
(35864, 43552, 16384 ...) and a wrong matrix. This module re-parses placements the way the bytes
actually lay out (class name only for HasClassName 0x08), decodes the PlaceObject3 filter list and
blend mode, and adds what the older parser skips:

  * DefineShape / DefineShape2/3 geometry (fill styles with solid / gradient / bitmap, edge records)
    -> a bounding rectangle, the bitmap fill (image id + fill matrix) and the fill list
  * DefineExternalImage2 (1009): bitmap id, width/height, format, atlas file name
  * DefineSubImage (1008)
  * ExporterInfo / SymbolClass / DefineSceneAndFrameLabelData
  * per-sprite frame timelines (placements per frame, labels, removes)
  * a resolved scene tree (sprite -> children) with full 2x3 matrices

  python gfx_scene_extract.py <movie.gfx> <out.json> [tree.txt]
"""
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from gfx_native_parse import Bits, read_gfx, tags  # noqa: E402


def cstr(b, p):
    e = b.index(b"\0", p)
    return b[p:e].decode("utf-8", "replace"), e + 1


FILTER_SIZES = {0: 23, 1: 9, 2: 15, 3: 27, 6: 80}


def skip_filters(body, p):
    n = body[p]
    p += 1
    out = []
    for _ in range(n):
        fid = body[p]
        p += 1
        if fid in FILTER_SIZES:
            out.append({"id": fid, "raw": body[p:p + FILTER_SIZES[fid]].hex()})
            p += FILTER_SIZES[fid]
        elif fid in (4, 7):
            cnt = body[p]
            size = 1 + cnt * 4 + cnt + 4 * 4 + 2 + 1
            out.append({"id": fid, "raw": body[p:p + size].hex()})
            p += size
        elif fid == 5:
            mx, my = body[p], body[p + 1]
            size = 2 + 4 + 4 + 4 * mx * my + 4 + 1
            out.append({"id": fid, "raw": body[p:p + size].hex()})
            p += size
        else:
            raise ValueError("unknown filter %d" % fid)
    return out, p


def parse_place(body, pos, ln, code):
    b = Bits(body, pos)
    end = pos + ln
    if code == 4:
        cid = b.u16()
        depth = b.u16()
        return {"op": "place", "depth": depth, "char": cid, "matrix": b.matrix()}
    f1 = b.u8()
    f2 = b.u8() if code == 70 else 0
    depth = b.u16()
    d = {"op": "place", "depth": depth, "move": bool(f1 & 1), "f1": f1, "f2": f2}
    if code == 70 and (f2 & 0x08):
        d["class"] = b.string()
    if f1 & 2:
        d["char"] = b.u16()
    if f1 & 4:
        d["matrix"] = b.matrix()
    if f1 & 8:
        d["cx"] = b.cxform(True)
    if f1 & 0x10:
        d["ratio"] = b.u16()
    if f1 & 0x20:
        d["name"] = b.string()
    if f1 & 0x40:
        d["clipDepth"] = b.u16()
    if code == 70:
        b.align()
        if f2 & 0x01:
            d["filters"], b.pos = skip_filters(body, b.pos)
        if f2 & 0x02:
            d["blend"] = body[b.pos]
            b.pos += 1
        if f2 & 0x04:
            d["cacheAsBitmap"] = body[b.pos]
            b.pos += 1
    d["_trailing"] = end - b.pos  # non-zero: unparsed bytes (clip actions or unknown flags)
    return d


def read_rgba(b, rgba):
    r, g, bl = b.u8(), b.u8(), b.u8()
    a = b.u8() if rgba else 255
    return [r, g, bl, a]


def parse_fill_style(b, code):
    rgba = code in (32, 83)
    t = b.u8()
    if t == 0:
        return {"type": "solid", "color": read_rgba(b, rgba)}
    if t in (0x10, 0x12, 0x13):
        m = b.matrix()
        b.align()
        flags = b.u8()
        num = flags & 0x0F
        stops = []
        for _ in range(num):
            ratio = b.u8()
            stops.append({"ratio": ratio, "color": read_rgba(b, rgba)})
        d = {"type": "gradient", "kind": {0x10: "linear", 0x12: "radial", 0x13: "focal"}[t], "matrix": m, "stops": stops}
        if t == 0x13:
            d["focal"] = b.u16() / 256.0
        return d
    if 0x40 <= t <= 0x43:
        bid = b.u16()
        m = b.matrix()
        return {"type": "bitmap", "bitmap": bid, "matrix": m, "smooth": t in (0x40, 0x41), "repeat": t in (0x40, 0x42)}
    raise ValueError("fill style %x" % t)


def parse_shape(body, pos, ln, code):
    """Return {id, bounds, fills, edges: bbox of all drawn edge points in twips/20}."""
    b = Bits(body, pos)
    sid = b.u16()
    bounds = [v / 20.0 for v in b.rect()]
    if code == 83:
        b.rect()
        b.u8()
    nfill = b.u8()
    if nfill == 0xFF and code != 2:
        nfill = b.u16()
    fills = [parse_fill_style(b, code) for _ in range(nfill)]
    nline = b.u8()
    if nline == 0xFF and code != 2:
        nline = b.u16()
    for _ in range(nline):
        b.u16()
        read_rgba(b, code in (32, 83))
    nfb = b.ub(4)
    nlb = b.ub(4)
    x = y = 0
    segs = []
    fill0 = fill1 = 0
    base = 0
    cur_fills = fills
    # collect (fill index -> polyline list) so callers can rasterize solids/gradients
    paths = []
    cur = None

    def flush():
        nonlocal cur
        if cur and len(cur["pts"]) > 1:
            paths.append(cur)
        cur = None

    while True:
        edge = b.ub(1)
        if edge == 0:
            flags = b.ub(5)
            if flags == 0:
                break
            if flags & 1:
                flush()
                n = b.ub(5)
                x = b.sb(n)
                y = b.sb(n)
            if flags & 2:
                fill0 = b.ub(nfb) + base if False else b.ub(nfb)
            if flags & 4:
                fill1 = b.ub(nfb)
            if flags & 8:
                b.ub(nlb)
            if flags & 16:
                b.align()
                nfill2 = b.u8()
                if nfill2 == 0xFF and code != 2:
                    nfill2 = b.u16()
                base = len(fills)
                for _ in range(nfill2):
                    fills.append(parse_fill_style(b, code))
                nline2 = b.u8()
                if nline2 == 0xFF and code != 2:
                    nline2 = b.u16()
                for _ in range(nline2):
                    b.u16()
                    read_rgba(b, code in (32, 83))
                nfb = b.ub(4)
                nlb = b.ub(4)
            if cur is None:
                cur = {"fill0": fill0 + (base if fill0 else 0), "fill1": fill1 + (base if fill1 else 0), "pts": [(x, y)]}
            else:
                cur["pts"].append((x, y))
            if flags & 1:
                pass
            # a style change starts a new path segment
            if flags & (2 | 4):
                flush()
                cur = {"fill0": fill0 + (base if fill0 else 0), "fill1": fill1 + (base if fill1 else 0), "pts": [(x, y)]}
        else:
            straight = b.ub(1)
            if cur is None:
                cur = {"fill0": fill0, "fill1": fill1, "pts": [(x, y)]}
            if straight:
                n = b.ub(4) + 2
                if b.ub(1):
                    dx = b.sb(n)
                    dy = b.sb(n)
                elif b.ub(1):
                    dx = 0
                    dy = b.sb(n)
                else:
                    dx = b.sb(n)
                    dy = 0
                x += dx
                y += dy
                cur["pts"].append((x, y))
            else:
                n = b.ub(4) + 2
                cx = x + b.sb(n)
                cy = y + b.sb(n)
                ax = cx + b.sb(n)
                ay = cy + b.sb(n)
                # flatten the quadratic curve
                for i in range(1, 7):
                    t = i / 6.0
                    px = (1 - t) ** 2 * x + 2 * (1 - t) * t * cx + t * t * ax
                    py = (1 - t) ** 2 * y + 2 * (1 - t) * t * cy + t * t * ay
                    cur["pts"].append((px, py))
                x, y = ax, ay
    flush()
    xs = [p[0] for pa in paths for p in pa["pts"]]
    ys = [p[1] for pa in paths for p in pa["pts"]]
    ebox = [min(xs) / 20.0, max(xs) / 20.0, min(ys) / 20.0, max(ys) / 20.0] if xs else None
    return {"id": sid, "bounds": bounds, "fills": fills, "edgeBox": ebox,
            "paths": [{"fill0": p["fill0"], "fill1": p["fill1"], "pts": [(q[0] / 20.0, q[1] / 20.0) for q in p["pts"]]} for p in paths]}


def parse_edit(body, pos):
    b = Bits(body, pos)
    cid = b.u16()
    bounds = [v / 20 for v in b.rect()]
    f1 = b.u8()
    f2 = b.u8()
    d = {"id": cid, "bounds": bounds}
    if f1 & 1:
        d["font"] = b.u16()
    if f2 & 0x80:
        d["fontClass"] = b.string()
    if f1 & 1:
        d["size"] = b.u16() / 20
    if f1 & 4:
        d["color"] = [b.u8() for _ in range(4)]
    if f1 & 2:
        d["maxLen"] = b.u16()
    if f2 & 0x20:
        d["layout"] = {"align": b.u8(), "left": b.u16() / 20, "right": b.u16() / 20, "indent": b.u16() / 20, "leading": b.s16() / 20}
    d["var"] = b.string()
    if f1 & 0x80:
        d["text"] = b.string()
    d["flags"] = {"html": bool(f2 & 2), "readonly": bool(f1 & 8), "multiline": bool(f1 & 0x20), "wordwrap": bool(f1 & 0x40), "autosize": bool(f2 & 0x40), "noselect": bool(f1 & 0x10) if False else None}
    return d


def parse_sprite(body, pos, ln):
    b = Bits(body, pos)
    sid = b.u16()
    frames = b.u16()
    out = {"id": sid, "frames": frames, "items": [], "labels": {}}
    frame = 1
    for code, p, l in tags(body, pos + 4, pos + ln):
        if code == 1:
            frame += 1
        elif code in (4, 26, 70):
            d = parse_place(body, p, l, code)
            d["frame"] = frame
            out["items"].append(d)
        elif code in (5, 28):
            c = Bits(body, p)
            if code == 5:
                c.u16()
            out["items"].append({"op": "remove", "depth": c.u16(), "frame": frame})
        elif code == 43:
            nm = Bits(body, p).string()
            out["labels"][nm] = frame
            out["items"].append({"op": "label", "name": nm, "frame": frame})
    return out


def parse_all(data):
    body, pos, hdr = read_gfx(data)
    R = {"header": hdr, "sprites": {}, "shapes": {}, "edits": {}, "names": {}, "ext": {}, "sub": {}, "root": [],
         "tagcount": {}, "symbolClass": {}, "exporter": None, "abc": []}
    frame = 1
    for code, p, l in tags(body, pos):
        R["tagcount"][code] = R["tagcount"].get(code, 0) + 1
        if code in (2, 22, 32, 83):
            s = parse_shape(body, p, l, code)
            R["shapes"][s["id"]] = s
        elif code == 39:
            s = parse_sprite(body, p, l)
            R["sprites"][s["id"]] = s
        elif code == 37:
            e = parse_edit(body, p)
            R["edits"][e["id"]] = e
        elif code in (56, 76):
            b = Bits(body, p)
            n = b.u16()
            for _ in range(n):
                cid = b.u16()
                nm = b.string()
                (R["symbolClass"] if code == 76 else R["names"])[cid] = nm
        elif code == 1009:
            raw = body[p:p + l]
            cid = struct.unpack_from("<H", raw, 0)[0]
            # Scaleform DefineExternalImage2: id u16, bitmapFormat u16, targetWidth u16, targetHeight u16, name-length u8...
            fmt, w, h = struct.unpack_from("<HHH", raw, 2)
            import re as _re
            m = _re.search(rb"[ -~]*\.(?:tga|dds|png)$", raw)
            fn = m.group(0).decode("ascii") if m else ""
            if fn and len(fn) > 1 and fn[0] == chr(len(fn) - 1):
                fn = fn[1:]
            R["ext"][cid] = {"file": fn, "raw_len": l, "hdr": list(struct.unpack_from("<HHHHHH", raw, 0)) if l >= 12 else []}
        elif code == 1008:
            b = Bits(body, p)
            cid = b.u16()
            iid = b.u16()
            x1, y1, x2, y2 = b.u16(), b.u16(), b.u16(), b.u16()
            R["sub"][cid] = {"image": iid, "rect": [x1, y1, x2, y2]}
        elif code == 1000:
            R["exporter"] = body[p:p + l].hex()
        elif code == 1:
            frame += 1
        elif code in (4, 26, 70):
            d = parse_place(body, p, l, code)
            d["frame"] = frame
            R["root"].append(d)
        elif code == 43:
            R["root"].append({"op": "label", "name": Bits(body, p).string(), "frame": frame})
        elif code == 82:
            R["abc"].append({"offset": p, "length": l})
    return R, body


def _mat(m):
    return m or {"sx": 1.0, "sy": 1.0, "r0": 0.0, "r1": 0.0, "tx": 0.0, "ty": 0.0}


def kind_of(R, cid):
    k = str(cid)
    if cid in R["sprites"]:
        return "sprite"
    if cid in R["shapes"]:
        return "shape"
    if cid in R["edits"]:
        return "edit"
    if cid in R["sub"]:
        return "subimage"
    if cid in R["ext"]:
        return "image"
    return "?"


def tree(R, items, depth=0, frame=1, out=None, indent=""):
    out = [] if out is None else out
    disp = {}
    for it in items:
        if it.get("frame", 1) > frame:
            continue
        if it["op"] == "remove":
            disp.pop(it["depth"], None)
        elif it["op"] == "place":
            if it.get("move") and it["depth"] in disp and "char" not in it:
                base = dict(disp[it["depth"]])
                base.update({k: v for k, v in it.items() if k in ("matrix", "cx", "name", "ratio")})
                disp[it["depth"]] = base
            else:
                disp[it["depth"]] = it
    for d in sorted(disp):
        it = disp[d]
        cid = it.get("char")
        m = _mat(it.get("matrix"))
        k = kind_of(R, cid) if cid is not None else "-"
        line = "%sd%d %s#%s name=%r pos(%.1f,%.1f) scale(%.3f,%.3f)" % (indent, d, k, cid, it.get("name", ""), m["tx"], m["ty"], m["sx"], m["sy"])
        if it.get("blend"):
            line += " blend=%d" % it["blend"]
        if it.get("filters"):
            line += " filters=%s" % [f["id"] for f in it["filters"]]
        if it.get("cx"):
            line += " cx=%s" % json.dumps(it["cx"])
        out.append(line)
        if k == "sprite":
            sp = R["sprites"][cid]
            out.append("%s   [frames=%d labels=%s]" % (indent, sp["frames"], sp["labels"]))
            tree(R, sp["items"], depth + 1, 1, out, indent + "      ")
        elif k == "shape":
            sh = R["shapes"][cid]
            fl = [f.get("type") + (":%d" % f["bitmap"] if f.get("type") == "bitmap" else "") for f in sh["fills"]]
            out.append("%s   shape bounds=%s edgeBox=%s fills=%s" % (indent, sh["bounds"], sh["edgeBox"], fl))
        elif k == "edit":
            e = R["edits"][cid]
            out.append("%s   edit font=%s size=%s color=%s text=%r bounds=%s var=%r layout=%s" % (indent, e.get("fontClass") or e.get("font"), e.get("size"), e.get("color"), e.get("text"), e["bounds"], e["var"], e.get("layout")))
        elif k in ("subimage", "image"):
            info = R["sub"][cid] if k == "subimage" else R["ext"][cid]
            out.append("%s   %s %s" % (indent, k, info))
    return out


if __name__ == "__main__":
    data = open(sys.argv[1], "rb").read()
    off = data.find(b"GFX")
    R, body = parse_all(data[off:])
    Path(sys.argv[2]).write_text(json.dumps(R, ensure_ascii=False), encoding="utf-8")
    print("sprites", len(R["sprites"]), "shapes", len(R["shapes"]), "edits", len(R["edits"]), "ext", len(R["ext"]), "sub", len(R["sub"]))
    bad = sum(1 for s in R["sprites"].values() for it in s["items"] if it.get("_trailing"))
    print("placements with unparsed trailing bytes:", bad)
    if len(sys.argv) > 3:
        root = R["root"]
        lines = tree(R, root)
        Path(sys.argv[3]).write_text("\n".join(lines), encoding="utf-8")
        print("tree lines", len(lines))
