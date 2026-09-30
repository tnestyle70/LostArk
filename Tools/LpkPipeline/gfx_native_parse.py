"""Minimal Scaleform GFX/SWF tag parser (no ffdec / Java needed): sprites, placements, shapes with
bitmap fills, DefineExternalImage2 (tag 1009) / DefineSubImage (tag 1008), edit texts, symbol names.
Enough to lift a retail HUD layout the way gfx_dump_symbol.py does from an ffdec XML dump.

  python gfx_native_parse.py <movie.gfx> [byte offset of the GFX signature]      # summary
  from gfx_native_parse import parse_all; R = parse_all(open(path, "rb").read())  # as a module

parse_all() returns header / sprites / shapes / edits / names / ext (external atlas pages) / sub
(sub-image rectangles) / root (stage placements). Positions are stage px (twips / 20).
Feed json.dump(R) to gfx_native_tree.py. Get the .gfx from dump_upk_movie.py.
"""
import struct, zlib, sys, json


class Bits:
    def __init__(self, b, pos=0):
        self.b = b
        self.pos = pos
        self.bit = 0

    def align(self):
        if self.bit:
            self.pos += 1
            self.bit = 0

    def ub(self, n):
        v = 0
        for _ in range(n):
            byte = self.b[self.pos]
            v = (v << 1) | ((byte >> (7 - self.bit)) & 1)
            self.bit += 1
            if self.bit == 8:
                self.bit = 0
                self.pos += 1
        return v

    def sb(self, n):
        if n == 0:
            return 0
        v = self.ub(n)
        if v & (1 << (n - 1)):
            v -= 1 << n
        return v

    def fb(self, n):
        return self.sb(n) / 65536.0

    def u8(self):
        self.align()
        v = self.b[self.pos]
        self.pos += 1
        return v

    def u16(self):
        self.align()
        v = struct.unpack_from("<H", self.b, self.pos)[0]
        self.pos += 2
        return v

    def s16(self):
        self.align()
        v = struct.unpack_from("<h", self.b, self.pos)[0]
        self.pos += 2
        return v

    def u32(self):
        self.align()
        v = struct.unpack_from("<I", self.b, self.pos)[0]
        self.pos += 4
        return v

    def string(self):
        self.align()
        e = self.b.index(b"\0", self.pos)
        s = self.b[self.pos:e].decode("utf-8", "replace")
        self.pos = e + 1
        return s

    def rect(self):
        n = self.ub(5)
        r = [self.sb(n) for _ in range(4)]
        self.align()
        return r

    def matrix(self):
        sx = sy = 1.0
        r0 = r1 = 0.0
        if self.ub(1):
            n = self.ub(5)
            sx = self.fb(n)
            sy = self.fb(n)
        if self.ub(1):
            n = self.ub(5)
            r0 = self.fb(n)
            r1 = self.fb(n)
        n = self.ub(5)
        tx = self.sb(n)
        ty = self.sb(n)
        self.align()
        return {"sx": sx, "sy": sy, "r0": r0, "r1": r1, "tx": tx / 20.0, "ty": ty / 20.0}

    def cxform(self, alpha=True):
        add = self.ub(1)
        mul = self.ub(1)
        n = self.ub(4)
        m = a = None
        cnt = 4 if alpha else 3
        if mul:
            m = [self.sb(n) for _ in range(cnt)]
        if add:
            a = [self.sb(n) for _ in range(cnt)]
        self.align()
        return {"mul": m, "add": a}


def read_gfx(data):
    sig = data[:3]
    assert sig in (b"GFX", b"FWS", b"CFX", b"CWS"), sig
    version = data[3]
    length = struct.unpack_from("<I", data, 4)[0]
    body = data[8:]
    if sig in (b"CFX", b"CWS"):
        body = zlib.decompress(body)
    bits = Bits(body)
    frame = bits.rect()
    bits.align()
    rate = struct.unpack_from("<H", body, bits.pos)[0]
    count = struct.unpack_from("<H", body, bits.pos + 2)[0]
    return body, bits.pos + 4, {"version": version, "length": length, "frame": [v / 20 for v in frame], "rate": rate / 256, "frames": count}


def tags(body, pos, end=None):
    end = len(body) if end is None else end
    while pos < end:
        hdr = struct.unpack_from("<H", body, pos)[0]
        pos += 2
        code = hdr >> 6
        ln = hdr & 0x3F
        if ln == 0x3F:
            ln = struct.unpack_from("<I", body, pos)[0]
            pos += 4
        yield code, pos, ln
        pos += ln
        if code == 0:
            return


def skip_color(b, rgba):
    b.u8()
    b.u8()
    b.u8()
    if rgba:
        b.u8()


def parse_fills(body, pos, code):
    b = Bits(body, pos)
    sid = b.u16()
    bounds = b.rect()
    if code == 83:
        b.rect()
        b.u8()
    n = b.u8()
    if n == 0xFF and code != 2:
        n = b.u16()
    rgba = code in (32, 83)
    fills = []
    for _ in range(n):
        t = b.u8()
        if t == 0:
            skip_color(b, rgba)
            fills.append({"type": "solid"})
        elif t in (0x10, 0x12, 0x13):
            b.matrix()
            b.align()
            num = b.u8() & 0x0F
            for _ in range(num):
                b.u8()
                skip_color(b, rgba)
            if t == 0x13:
                b.u16()
            fills.append({"type": "gradient"})
        elif 0x40 <= t <= 0x43:
            bid = b.u16()
            m = b.matrix()
            fills.append({"type": "bitmap", "bitmap": bid, "matrix": m, "smooth": t in (0x40, 0x41)})
        else:
            fills.append({"type": "unknown%x" % t})
            break
    return sid, [v / 20 for v in bounds], fills


def parse_place(body, pos, ln, code):
    b = Bits(body, pos)
    if code == 4:
        cid = b.u16()
        depth = b.u16()
        m = b.matrix()
        return {"op": "place", "depth": depth, "char": cid, "matrix": m}
    f1 = b.u8()
    f2 = b.u8() if code == 70 else 0
    depth = b.u16()
    d = {"op": "place", "depth": depth, "move": bool(f1 & 1)}
    if code == 70 and ((f2 & 0x08) or ((f2 & 0x10) and (f1 & 2))):
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
        d["clip"] = b.u16()
    return d


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
    d["flags"] = {"html": bool(f2 & 2), "readonly": bool(f1 & 8), "multiline": bool(f1 & 0x20), "wordwrap": bool(f1 & 0x40), "autosize": bool(f2 & 0x40)}
    return d


def parse_sprite_tags(body, pos, ln):
    b = Bits(body, pos)
    sid = b.u16()
    frames = b.u16()
    out = {"id": sid, "frames": frames, "items": []}
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
            out["items"].append({"op": "label", "name": Bits(body, p).string(), "frame": frame})
    return out


def parse_all(data):
    body, pos, hdr = read_gfx(data)
    R = {"header": hdr, "sprites": {}, "shapes": {}, "edits": {}, "names": {}, "ext": {}, "sub": {}, "root": [], "tagcount": {}}
    frame = 1
    for code, p, l in tags(body, pos):
        R["tagcount"][code] = R["tagcount"].get(code, 0) + 1
        if code in (2, 22, 32, 83):
            sid, bounds, fills = parse_fills(body, p, code)
            R["shapes"][sid] = {"bounds": bounds, "fills": fills}
        elif code == 39:
            s = parse_sprite_tags(body, p, l)
            R["sprites"][s["id"]] = s
        elif code == 37:
            e = parse_edit(body, p)
            R["edits"][e["id"]] = e
        elif code in (56, 76):
            b = Bits(body, p)
            n = b.u16()
            for _ in range(n):
                cid = b.u16()
                R["names"][cid] = b.string()
        elif code == 1009:
            raw = body[p:p + l]
            cid = struct.unpack_from("<H", raw, 0)[0]
            import re as _re
            m = _re.search(rb"[ -~]*\.(?:tga|dds|png)$", raw)
            fn = m.group(0).decode("ascii") if m else ""
            # the length byte precedes the file name; drop it when it landed inside the match
            if fn and len(fn) > 1 and fn[0] == chr(len(fn) - 1):
                fn = fn[1:]
            R["ext"][cid] = {"file": fn, "raw_len": l}
        elif code == 1008:
            b = Bits(body, p)
            cid = b.u16()
            iid = b.u16()
            x1, y1, x2, y2 = b.u16(), b.u16(), b.u16(), b.u16()
            R["sub"][cid] = {"image": iid, "rect": [x1, y1, x2, y2]}
        elif code == 1:
            frame += 1
        elif code in (4, 26, 70):
            d = parse_place(body, p, l, code)
            d["frame"] = frame
            R["root"].append(d)
        elif code == 43:
            R["root"].append({"op": "label", "name": Bits(body, p).string(), "frame": frame})
    return R


if __name__ == "__main__":
    data = open(sys.argv[1], "rb").read()
    off = data.find(b"GFX") if len(sys.argv) < 3 else int(sys.argv[2])
    R = parse_all(data[off:])
    print(json.dumps(R["header"]), "sprites", len(R["sprites"]), "shapes", len(R["shapes"]), "edits", len(R["edits"]), "names", len(R["names"]), "ext", len(R["ext"]), "sub", len(R["sub"]))
    print("tag counts", dict(sorted(R["tagcount"].items())))
