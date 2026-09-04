# -*- coding: utf-8 -*-
"""Decrypt EFGame/Movies/*.ipk whose deobfuscated name matches a prefix and write
them as <LOGICAL_NAME>.bik|.bk2 into an output folder with a manifest.

  python dump_named_movies.py <out_dir> <prefix> [<prefix> ...]
  e.g. python dump_named_movies.py D:/ClaudeWork/movietest/esther_named ESTHER_SKILL_ COMBINEDSKILL_
"""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ipk_to_bk2 import _apply
from bink_key_solver import solve_key, SolveError
from deobfuscate_names import decode

MOV = r"D:\Games\LOSTARK\EFGame\Movies"


def main(argv):
    out_dir, prefixes = argv[0], [p.upper() for p in argv[1:]]
    os.makedirs(out_dir, exist_ok=True)
    lines = []
    for f in sorted(os.listdir(MOV)):
        if not f.lower().endswith(".ipk"):
            continue
        logical = decode(f.split(".")[0])
        if not any(logical.startswith(p) for p in prefixes):
            continue
        raw = open(os.path.join(MOV, f), "rb").read()
        try:
            key, info = solve_key(raw)
        except SolveError as e:
            line = f"{logical:<40} {f:<34} FAIL {e}"
        else:
            dec = _apply(raw, key)
            w, h = struct.unpack_from("<II", dec, 0x14)
            fps = struct.unpack_from("<II", dec, 0x1C)
            ext = ".bik" if info["magic"].startswith(b"BIK") else ".bk2"
            open(os.path.join(out_dir, logical + ext), "wb").write(dec)
            line = (f"{logical:<40} {f:<34} {info['magic'].decode('latin1')} {w}x{h} "
                    f"f={info['frames']} fps={fps[0]}/{fps[1]} size={len(raw)}")
        print(line, flush=True)
        lines.append(line)
    with open(os.path.join(out_dir, "_manifest.txt"), "w", encoding="utf-8") as m:
        m.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main(sys.argv[1:])
