# -*- coding: utf-8 -*-
"""Deobfuscate Lost Ark package/movie file names (EFGame/Movies/*.ipk,
ReleasePC/Packages/*.upk).

The on-disk name is a length-keyed affine substitution over the base-36
alphabet plus 2-char escapes for '_', '-', 'Q', 'X' and a '!' padding
terminator. Algorithm credit: gildor.org forum (Rust original), C# port
Twigzie/Fantality-LostArkRenamer.

Usage:
  python deobfuscate_names.py <name-or-dir> [...]
  python deobfuscate_names.py --encode <plain-name>
"""
import os, sys

ESC = {
    ("QP", 0): "Q", ("QD", 1): "Q", ("QW", 2): "Q", ("Q4", 3): "Q",
    ("QL", 0): "-", ("QB", 1): "-", ("QO", 2): "-", ("Q5", 3): "-",
    ("QC", 0): "_", ("QN", 1): "_", ("QT", 2): "_", ("Q9", 3): "_",
    ("XU", 0): "X", ("XN", 1): "X", ("XH", 2): "X", ("X3", 3): "X",
    ("XW", 0): "!", ("XS", 1): "!", ("XZ", 2): "!", ("X0", 3): "!",
}
ESC_ENC = {(v, k[1]): k[0] for k, v in ESC.items()}


def _unescape(s):
    out, i = [], 0
    while i < len(s):
        pair = s[i:i + 2]
        sub = ESC.get((pair, i % 4))
        if sub is not None:
            out.append(sub)
            i += 2
        else:
            out.append(s[i])
            i += 1
    return "".join(out)


def decode(stem):
    """stem = obfuscated name without extension (e.g. 'F9P2M1G2NIHG31G2P0P2DEF')."""
    s = stem.upper()
    n = len(s)
    out = []
    for c in s:
        x = ord(c)
        if "0" <= c <= "9":
            x += 43
        i = (31 * (x - n - 65) % 36 + 36) % 36 + 65
        if i >= 91:
            i -= 43
        out.append(chr(i))
    plain = _unescape("".join(out))
    return plain.split("!")[0] if "!" in plain else plain


def encode(plain):
    """Inverse of decode. Output length must be a multiple of 4 ('!' padded)."""
    p = plain.upper()
    esc, i = [], 0
    for c in p:
        pos = len("".join(esc)) % 4
        if c in "Q-_X":
            esc.append(ESC_ENC[(c, pos)])
        else:
            esc.append(c)
    s = "".join(esc)
    while len(s) % 4 != 0:
        s += ESC_ENC[("!", len(s) % 4)]
        s = s[:len(s)] if len(s) % 4 == 0 else s
    n = len(s)
    # inverse affine: i = 31*(x-n-65) mod 36 + 65  ->  x = 7*(i-65) mod 36 + n + 65
    # (31*7 = 217 = 1 mod 36)
    out = []
    for c in s:
        i = ord(c) + (43 if "0" <= c <= "9" else 0)
        x = (7 * (i - 65)) % 36 + n + 65
        x = (x - 65) % 36 + 65
        if x >= 91:
            x -= 43
        out.append(chr(x))
    return "".join(out)


def main(argv):
    if argv and argv[0] == "--encode":
        for p in argv[1:]:
            print(f"{p} -> {encode(p)}")
        return
    for a in argv or ["."]:
        if os.path.isdir(a):
            for f in sorted(os.listdir(a)):
                stem, ext = os.path.splitext(f)
                stem = stem.split(".")[0]
                print(f"{f:<40} {decode(stem)}")
        else:
            stem = os.path.splitext(os.path.basename(a))[0].split(".")[0]
            print(f"{a} -> {decode(stem)}")


if __name__ == "__main__":
    main(sys.argv[1:])
