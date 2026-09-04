# -*- coding: utf-8 -*-
"""Decrypt every EFGame/Movies/*.ipk and index resolution/frames to find the
commander-entrance backgrounds. Runs the repo's own ipk_to_bk2 recovery on each.
"""
import os, struct, sys, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ipk_to_bk2 import decrypt

MOV = r"D:\Games\LOSTARK\EFGame\Movies"
rows = []
files = [f for f in os.listdir(MOV) if f.lower().endswith(".ipk")]
for n, f in enumerate(files):
    try:
        dec = decrypt(open(os.path.join(MOV, f), "rb").read())
    except Exception as e:
        rows.append((f, "ERR", str(e)[:40]))
        continue
    if dec[:3] != b"KB2":
        rows.append((f, "NOTBINK", dec[:4].hex()))
        continue
    w = struct.unpack_from("<I", dec, 0x14)[0]
    h = struct.unpack_from("<I", dec, 0x18)[0]
    fr = struct.unpack_from("<I", dec, 0x08)[0]
    rows.append((f, "OK", w, h, fr))
    if n % 40 == 0:
        print(f"  {n}/{len(files)}", flush=True)

ok = [r for r in rows if r[1] == "OK"]
big = sorted([r for r in ok if r[2] * r[3] >= 700 * 700],
             key=lambda r: -(r[2] * r[3]))
with open("movies_index.txt", "w", encoding="utf-8") as out:
    out.write(f"총 {len(files)}  OK {len(ok)}  Bink아님 {sum(1 for r in rows if r[1]=='NOTBINK')}  ERR {sum(1 for r in rows if r[1]=='ERR')}\n\n")
    out.write("=== 대형 무비 (700x700+), 픽셀량 순 ===\n")
    for f, _, w, h, fr in big:
        out.write(f"{f:<40} {w}x{h}  frames={fr}\n")
json.dump([list(r) for r in rows], open("movies_index.json", "w"), ensure_ascii=False)
print(f"완료 OK={len(ok)} 대형={len(big)}")
