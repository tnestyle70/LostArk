# -*- coding: utf-8 -*-
"""Decrypt a Lost Ark EFGame/Movies/*.ipk into its raw Bink stream (.bk2 / .bik).

The container is a repeating-key XOR stream, period 48 bytes, with a per-file key
(two different .ipk files do not share a keystream). The key is recovered exactly
by bink_key_solver from the Bink header and frame table -- see that module for the
cribs. Both Bink 2 ('KB2j' etc.) and Bink 1 ('BIKi', the Esther/skill cut-ins with
alpha) exist in EFGame/Movies; FFmpeg decodes BIKi and KB2i, RAD binkplay decodes
every version.

Verified against the team's reference: SMELT_LODING_LEVEL2
(F9P2M1G2NIHG31G2P0P2DEF.ipk) decrypts to the exact bk2 SHA in the note.

Usage:
    python ipk_to_bk2.py <input.ipk> <output.bk2|.bik>
"""
from __future__ import annotations

import struct
import sys

import numpy as np

from bink_key_solver import KEY_LEN, SolveError, solve_key


def _apply(raw: bytes, key: bytes) -> bytes:
    arr = np.frombuffer(raw, dtype=np.uint8)
    ks = np.tile(np.frombuffer(key, dtype=np.uint8), len(raw) // KEY_LEN + 1)[:len(raw)]
    return (arr ^ ks).tobytes()


def recover_key(raw: bytes) -> bytes:
    """Return the 48-byte XOR keystream for this file."""
    return solve_key(raw)[0]


def decrypt(raw: bytes) -> bytes:
    return _apply(raw, recover_key(raw))


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(__doc__)
        return 2
    raw = open(argv[1], "rb").read()
    try:
        key, info = solve_key(raw)
    except SolveError as e:
        print(f"복호 실패: {e}", file=sys.stderr)
        return 1
    dec = _apply(raw, key)
    open(argv[2], "wb").write(dec)
    w, h = struct.unpack_from("<II", dec, 0x14)
    print(f"OK  {argv[2]}  magic={dec[:4]!r} {w}x{h} frames={info['frames']} audio={info['audio']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
