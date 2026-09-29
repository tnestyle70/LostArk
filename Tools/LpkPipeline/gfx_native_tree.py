"""Print the placement tree of a GFX symbol: names, accumulated stage position/scale,
bitmap shapes resolved to (atlas page file, sub-image rect), edit texts.

  python gfx_native_tree.py <parsed.json> <symbol name or sprite id> [maxdepth] [frame]

<parsed.json> is json.dump(gfx_native_parse.parse_all(...)). Sprite ids are the DefineSprite ids
the SymbolClass/ExportAssets tags name.
"""
import json, sys


def load(path):
    R = json.load(open(path, encoding="utf-8"))
    for k in ("sprites", "shapes", "edits", "names", "ext", "sub"):
        R[k] = {int(a): b for a, b in R[k].items()}
    return R


def mul(a, b):
    # a * b : apply b first, then a  (affine: x' = sx*x + r1*y + tx ; y' = r0*x + sy*y + ty)
    return {
        "sx": a["sx"] * b["sx"] + a["r1"] * b["r0"],
        "r1": a["sx"] * b["r1"] + a["r1"] * b["sy"],
        "r0": a["r0"] * b["sx"] + a["sy"] * b["r0"],
        "sy": a["r0"] * b["r1"] + a["sy"] * b["sy"],
        "tx": a["sx"] * b["tx"] + a["r1"] * b["ty"] + a["tx"],
        "ty": a["r0"] * b["tx"] + a["sy"] * b["ty"] + a["ty"],
    }


IDENT = {"sx": 1.0, "sy": 1.0, "r0": 0.0, "r1": 0.0, "tx": 0.0, "ty": 0.0}


def state_at(items, frame):
    """Resolve the display list of a sprite at a frame (depth -> placement)."""
    disp = {}
    for it in items:
        if it["frame"] > frame:
            break
        if it["op"] == "remove":
            disp.pop(it["depth"], None)
        elif it["op"] == "place":
            cur = disp.get(it["depth"])
            if it.get("move") and cur is not None:
                cur = dict(cur)
                for k in ("matrix", "cx", "name", "ratio", "clip"):
                    if k in it:
                        cur[k] = it[k]
                if "char" in it:
                    cur["char"] = it["char"]
                disp[it["depth"]] = cur
            else:
                disp[it["depth"]] = dict(it)
    return disp


def labels(items):
    return [(i["name"], i["frame"]) for i in items if i["op"] == "label"]


def walk(R, char, mat, depth, maxdepth, frame, out, indent, path):
    if char in R["shapes"]:
        sh = R["shapes"][char]
        b = sh["bounds"]
        w = (b[1] - b[0]) * abs(mat["sx"])
        h = (b[3] - b[2]) * abs(mat["sy"])
        x0 = mat["tx"] + b[0] * mat["sx"]
        y0 = mat["ty"] + b[2] * mat["sy"]
        img = ""
        for f in sh["fills"]:
            if f["type"] == "bitmap":
                sub = R["sub"].get(f["bitmap"])
                if sub:
                    ext = R["ext"].get(sub["image"], {})
                    img = "%s [%s] " % (ext.get("file", "?img%s" % sub["image"]), ",".join(str(v) for v in sub["rect"]))
                else:
                    img = "bitmap#%s " % f["bitmap"]
        out.append("%s- shape#%d %sat(%.1f,%.1f) size(%.1f x %.1f)" % (indent, char, img, x0, y0, w, h))
        return
    if char in R["edits"]:
        e = R["edits"][char]
        b = e["bounds"]
        out.append("%s- edit#%d font=%s size=%s color=%s text=%r at(%.1f,%.1f) size(%.1f x %.1f) var=%r" % (
            indent, char, e.get("fontClass", e.get("font")), e.get("size"), e.get("color"), e.get("text"),
            mat["tx"] + b[0] * mat["sx"], mat["ty"] + b[2] * mat["sy"], (b[1] - b[0]) * abs(mat["sx"]), (b[3] - b[2]) * abs(mat["sy"]), e.get("var")))
        return
    sp = R["sprites"].get(char)
    if sp is None:
        out.append("%s- ?char#%d" % (indent, char))
        return
    if depth >= maxdepth:
        out.append("%s- sprite#%d (%s) frames=%d [depth limit]" % (indent, char, R["names"].get(char, ""), sp["frames"]))
        return
    lb = labels(sp["items"])
    out.append("%s+ sprite#%d %s frames=%d labels=%s" % (indent, char, R["names"].get(char, ""), sp["frames"], lb[:14]))
    disp = state_at(sp["items"], frame)
    for d in sorted(disp):
        it = disp[d]
        if "char" not in it:
            continue
        m = mul(mat, it.get("matrix", IDENT))
        nm = it.get("name", "")
        c = it["char"]
        tag = "%s d%d name=%r scale(%.3f,%.3f) pos(%.1f,%.1f)" % (indent + "  ", d, nm, m["sx"], m["sy"], m["tx"], m["ty"])
        out.append(tag)
        walk(R, c, m, depth + 1, maxdepth, frame, out, indent + "    ", path + [nm])


if __name__ == "__main__":
    R = load(sys.argv[1])
    key = sys.argv[2]
    maxdepth = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    frame = int(sys.argv[4]) if len(sys.argv) > 4 else 1
    byname = {v: k for k, v in R["names"].items()}
    cid = int(key) if key.isdigit() else byname[key]
    out = []
    walk(R, cid, IDENT, 0, maxdepth, frame, out, "", [])
    print("\n".join(out))
