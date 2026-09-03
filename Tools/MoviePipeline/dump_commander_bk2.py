# -*- coding: utf-8 -*-
"""Decrypt all 1200x848 commander-entrance backgrounds with the VERIFIED tool
(ipk_to_bk2.recover_key) and save each as .bk2 (Bink2) for viewing in RAD.

Writes a manifest so identification survives across runs.
"""
import struct, os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ipk_to_bk2 as T

MOV = r"D:\Games\LOSTARK\EFGame\Movies"
OUT = r"D:\ClaudeWork\movietest\cmd848_verified"
os.makedirs(OUT, exist_ok=True)
MANIFEST = os.path.join(OUT, "manifest.txt")


def fast_decrypt(raw):
    key = T.recover_key(raw)
    a = np.frombuffer(raw, dtype=np.uint8).copy()
    a ^= np.tile(np.frombuffer(key, dtype=np.uint8), len(a) // 48 + 1)[:len(a)]
    return a.tobytes()


def analyze(dec, n):
    frames = struct.unpack_from("<I", dec, 0x08)[0]
    if not (0 < frames < 100000) or 0x2c + (frames + 1) * 4 > n:
        return None
    w = struct.unpack_from("<I", dec, 0x14)[0]
    h = struct.unpack_from("<I", dec, 0x18)[0]
    offs = [struct.unpack_from("<I", dec, 0x2c + i * 4)[0] for i in range(frames + 1)]
    # entry[0] is a header field, real table is monotonic from index 1
    bad = [i for i in range(1, frames) if offs[i] > offs[i + 1]]
    return w, h, frames, dec[:4], bad


def main():
    lines = []
    for f in sorted(os.listdir(MOV)):
        if not f.endswith(".ipk"):
            continue
        p = os.path.join(MOV, f)
        if os.path.getsize(p) > 120 * 1024 * 1024:
            continue
        raw = open(p, "rb").read()
        n = len(raw)
        try:
            dec = fast_decrypt(raw)
        except Exception as e:
            continue
        info = analyze(dec, n)
        if not info:
            continue
        w, h, frames, magic, bad = info
        if (w, h) != (1200, 848):
            continue
        ver = chr(magic[3]) if magic[:3] == b"KB2" else "?"
        stem = f[:12]
        outp = os.path.join(OUT, stem + ".bk2")
        open(outp, "wb").write(dec)
        line = f"{stem}\tKB2{ver}\t{w}x{h}\tf={frames}\tbad_from1={len(bad)}\t{f}"
        lines.append(line)
        print(line, flush=True)
    open(MANIFEST, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"\n== {len(lines)}개 저장 -> {OUT}", flush=True)


if __name__ == "__main__":
    main()
