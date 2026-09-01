# -*- coding: utf-8 -*-
"""Index EFGame/Movies/*.ipk by recovering each key from a 128 KB head only.

Same key recovery as ipk_to_bk2 (magic + file_size-8 + frame-table zero-byte cribs),
but XORing only the head and using the real file size for the file_size crib, so a
600 MB movie costs one 128 KB read instead of a full decrypt. A file is judged Bink2
only if the recovered header actually parses: version byte in b'a'..'z', sane frames,
and a width/height in real pixel ranges.
"""
import os, struct, collections, json

MOV = r"D:\Games\LOSTARK\EFGame\Movies"
KEY_LEN = 48
HEAD = 131072


def probe(path):
    n = os.path.getsize(path)
    with open(path, "rb") as f:
        raw = f.read(HEAD)
    if len(raw) < 0x200:
        return None
    key = {}
    for off, pt in ((0, b"KB2"), (4, struct.pack("<I", n - 8))):
        for k, b in enumerate(pt):
            key[(off + k) % KEY_LEN] = raw[off + k] ^ b
    for j in range(KEY_LEN):
        if j not in key:
            key[j] = collections.Counter(raw[j::KEY_LEN]).most_common(1)[0][0]

    def dec():
        return bytes(raw[i] ^ key[i % KEY_LEN] for i in range(len(raw)))

    for _ in range(6):
        d = dec()
        frames = struct.unpack_from("<I", d, 8)[0]
        if not (0 < frames < 50000):
            return None
        cribs = [(8, struct.pack("<I", frames)), (0x28, b"\0\0\0\0")]
        pos = 0x2C
        good = True
        for i in range(frames + 1):
            if pos + 4 > len(raw):
                good = False
                break
            v = struct.unpack_from("<I", d, pos)[0]
            cribs.append((pos, struct.pack("<I", v & 0x00FFFFFF)))  # top byte 0
            pos += 4
        new = dict(key)
        for off, pt in cribs:
            for k, b in enumerate(pt):
                p = off + k
                if p < len(raw):
                    new[p % KEY_LEN] = raw[p] ^ b
        if new == key:
            break
        key = new

    d = dec()
    ver = d[3]
    frames = struct.unpack_from("<I", d, 8)[0]
    w = struct.unpack_from("<I", d, 0x14)[0]
    h = struct.unpack_from("<I", d, 0x18)[0]
    if not (ord("a") <= ver <= ord("z")):
        return None
    if not (0 < w <= 4096 and 0 < h <= 4096 and 0 < frames < 50000):
        return None
    return (w, h, frames, chr(ver))


rows = []
files = [f for f in os.listdir(MOV) if f.lower().endswith(".ipk")]
for f in files:
    try:
        r = probe(os.path.join(MOV, f))
    except Exception:
        r = None
    if r:
        rows.append((f,) + r)

res = collections.Counter((r[1], r[2]) for r in rows)
with open("movies_index.txt", "w", encoding="utf-8") as o:
    o.write(f"총 {len(files)}  Bink2 판정 {len(rows)}\n\n=== 해상도 분포 ===\n")
    for (w, h), c in res.most_common():
        o.write(f"  {w}x{h}: {c}\n")
    o.write("\n=== 대형(700x700+) 픽셀량순 ===\n")
    for f, w, h, fr, v in sorted([r for r in rows if r[1]*r[2] >= 700*700], key=lambda r: -(r[1]*r[2])):
        o.write(f"{f:<40} {w}x{h} frames={fr} KB2{v}\n")
json.dump([list(r) for r in rows], open("movies_index.json", "w"), ensure_ascii=False)
print(f"Bink2 {len(rows)}/{len(files)}")
