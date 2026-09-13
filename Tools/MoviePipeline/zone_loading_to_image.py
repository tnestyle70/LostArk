#!/usr/bin/env python3
"""Decrypt a Lost Ark ExtRes/Loading/*.ipk into the illustration it holds.

Same container as EFGame/Movies -- a repeating-key XOR, period 48, one key per
file -- but the payload is a JPEG, not Bink, so bink_key_solver's header cribs
never fit and ipk_to_bk2 reports "no frame count fits".

Recovering the key without a usable header crib
-----------------------------------------------
Only lags that are multiples of 48 lift the byte-coincidence rate (3-5% against
a 0.4% baseline). A high-entropy payload cannot do that, so the lift comes from
the plaintext: somewhere in the file it holds a long run of one repeated byte.
Inside such a run the ciphertext IS the key XOR that constant, so the longest
48-periodic stretch hands back the key up to a single unknown byte, and trying
all 256 values of it against the JPEG signature settles which.

That run exists in every file checked so far because these JPEGs carry an Exif
block and large flat areas.

Usage:
  python zone_loading_to_image.py <input.ipk> <output.jpg>
  python zone_loading_to_image.py --all <zone-dir> <names.txt> <out-dir>
"""

from __future__ import annotations

import os
import sys

KEY_LEN = 48
JPEG_SIGNATURE = b"\xff\xd8\xff"
MIN_RUN = KEY_LEN * 2


class SolveError(Exception):
    pass


def _longest_periodic_run(raw: bytes) -> tuple[int, int]:
    """(start, length) of the longest stretch where raw[i] == raw[i + 48]."""
    best_start = best_length = 0
    start = None
    limit = len(raw) - KEY_LEN
    for i in range(limit):
        if raw[i] == raw[i + KEY_LEN]:
            if start is None:
                start = i
        elif start is not None:
            if i - start > best_length:
                best_start, best_length = start, i - start
            start = None
    if start is not None and limit - start > best_length:
        best_start, best_length = start, limit - start
    return best_start, best_length


def recover_key(raw: bytes) -> bytes:
    start, length = _longest_periodic_run(raw)
    if length < MIN_RUN:
        raise SolveError(
            "no constant plaintext run long enough (best %d bytes at %d)"
            % (length, start))

    # Inside the run the ciphertext repeats the key, offset by the run's phase.
    masked = bytes(raw[start + ((c - start) % KEY_LEN)] for c in range(KEY_LEN))

    for constant in range(256):
        key = bytes(b ^ constant for b in masked)
        head = bytes(raw[i] ^ key[i % KEY_LEN] for i in range(3))
        if head == JPEG_SIGNATURE:
            return key
    raise SolveError("run found but no constant yields a JPEG signature")


def decrypt(raw: bytes) -> bytes:
    key = recover_key(raw)
    plain = bytes(raw[i] ^ key[i % KEY_LEN] for i in range(len(raw)))
    if not plain.startswith(JPEG_SIGNATURE):
        raise SolveError("decrypted payload is not a JPEG")
    if not plain.endswith(b"\xff\xd9"):
        raise SolveError("decrypted payload has no JPEG end marker")
    return plain


def _one(source: str, destination: str) -> bool:
    raw = open(source, "rb").read()
    try:
        plain = decrypt(raw)
    except SolveError as error:
        print("  %-46s FAIL %s" % (os.path.basename(source), error))
        return False
    open(destination, "wb").write(plain)
    print("  %-46s -> %s  %d bytes"
          % (os.path.basename(source), os.path.basename(destination), len(plain)))
    return True


def main(argv: list[str]) -> int:
    if len(argv) == 5 and argv[1] == "--all":
        zone_dir, names_path, out_dir = argv[2], argv[3], argv[4]
        os.makedirs(out_dir, exist_ok=True)
        ok = bad = 0
        for line in open(names_path, encoding="utf-8", errors="replace"):
            parts = line.split()
            if len(parts) < 4 or not parts[-1].endswith(".ipk"):
                continue
            name, filename = parts[1], parts[-1]
            source = os.path.join(zone_dir, filename)
            if not os.path.isfile(source):
                continue
            if _one(source, os.path.join(out_dir, name + ".jpg")):
                ok += 1
            else:
                bad += 1
        print("%d ok, %d failed" % (ok, bad))
        return 0 if bad == 0 else 1

    if len(argv) != 3:
        print(__doc__)
        return 2
    return 0 if _one(argv[1], argv[2]) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
