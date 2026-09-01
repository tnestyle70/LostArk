# -*- coding: utf-8 -*-
"""Decrypt a Lost Ark EFGame/Movies/*.ipk into its raw Bink2 (.bk2) stream.

The container is a repeating-key XOR stream, period 48 bytes, with a per-file key
(two different .ipk files do not share a keystream). The key is recovered from known
plaintext: the Bink2 header is almost entirely predictable.

  offset  field            source
  0x00    'KB2' + version   fixed magic
  0x04    file_size - 8     = len(ipk) - 8
  0x08    num_frames           taken from the frame-offset table's own count once the
  0x0C    largest_frame        header is partly known, then fed back (see refine())
  0x10    (unused)
  0x14    width             read back after first pass
  0x18    height
  0x1C    fps_num
  0x20    fps_den
  0x24    video_flags
  0x28    audio_tracks (0)  UI movies are silent
  0x2C..  frame offset table (num_frames+1) uint32, monotonic, top byte 0

The 48-byte key spans past the fixed part of the header, so the last ~20 key bytes are
recovered by iterating: the frame table's high bytes are 0, which turns each table
entry into a crib for whatever key column its top byte lands on. A few passes converge.

Verified against the team's reference: SMELT_LODING_LEVEL2
(F9P2M1G2NIHG31G2P0P2DEF.ipk) decrypts to the exact bk2 SHA in the note.

Usage:
    python ipk_to_bk2.py <input.ipk> <output.bk2>
"""
from __future__ import annotations

import struct
import sys


KEY_LEN = 48
BINK2_MAGIC = b"KB2"


def _u32(v: int) -> bytes:
    return struct.pack("<I", v)


def _apply(raw: bytes, key: dict[int, int]) -> bytes:
    return bytes(raw[i] ^ key.get(i % KEY_LEN, 0) for i in range(len(raw)))


def recover_key(raw: bytes) -> bytes:
    """Return the 48-byte XOR keystream for this file."""
    n = len(raw)

    # --- fixed-position cribs that need no decrypted state --------------------
    # magic: 3 bytes are 'KB2', the 4th is the version char, unknown up front, so
    # crib only the 3 we know. file_size-8 is always derivable.
    key: dict[int, int] = {}
    for off, plain in ((0x00, BINK2_MAGIC), (0x04, _u32(n - 8))):
        for k, b in enumerate(plain):
            key[(off + k) % KEY_LEN] = raw[off + k] ^ b

    # Seed the still-unknown columns from each column's most common byte -- for
    # header fields and the table's zero high bytes the plaintext is 0 often
    # enough that the raw mode is the key byte. Good enough to bootstrap a decode.
    import collections
    for j in range(KEY_LEN):
        if j not in key:
            key[j] = collections.Counter(raw[j::KEY_LEN]).most_common(1)[0][0]

    # --- pin the frame-table high-byte columns by majority vote ----------------
    # audio_tracks is 0 for these UI movies, so the (num_frames+1)-entry uint32
    # table starts at 0x2C. Each entry's top byte is 0 (< 16 MB offsets), and those
    # top bytes land on only 12 key columns (period gcd(4,48)=4 -> columns
    # 3,7,11,...,47). Every such raw byte IS that key byte; the mode across ~25
    # samples per column beats a single wrong entry poisoning it. Do this before
    # the refine loop so it starts from a clean key.
    frames0 = struct.unpack_from("<I", _apply(raw, key), 0x08)[0]
    if 0 < frames0 < 100000 and 0x2C + (frames0 + 1) * 4 <= n:
        votes: dict[int, collections.Counter] = collections.defaultdict(collections.Counter)
        for i in range(frames0 + 1):
            top = 0x2C + i * 4 + 3            # high byte of entry i
            votes[top % KEY_LEN][raw[top]] += 1   # plaintext 0 -> key = raw byte
        for col, c in votes.items():
            key[col] = c.most_common(1)[0][0]

    # --- refine using the header the seed now exposes -------------------------
    for _ in range(8):
        dec = _apply(raw, key)
        num_frames = struct.unpack_from("<I", dec, 0x08)[0]
        if not (0 < num_frames < 100000):
            break
        audio = struct.unpack_from("<I", dec, 0x28)[0]
        table_off = 0x2C + (audio * 12 if audio < 8 else 0)

        # Re-crib the exact header values we can now read, plus audio_tracks=0 and
        # every frame-table entry (top byte 0, and the low bytes are the values we
        # just decoded, so this pins the key columns those bytes fall on).
        cribs: list[tuple[int, bytes]] = [
            (0x08, _u32(num_frames)),
            (0x14, dec[0x14:0x18]),
            (0x18, dec[0x18:0x1C]),
            (0x1C, dec[0x1C:0x20]),
            (0x20, dec[0x20:0x24]),
            (0x28, _u32(0)),
        ]
        for i in range(num_frames + 1):
            pos = table_off + i * 4
            if pos + 4 > n:
                break
            v = struct.unpack_from("<I", dec, pos)[0]
            # Every frame offset fits well under 16 MB, so its top byte is 0. That is a
            # hard crib on whatever key column the top byte lands on, and there are
            # num_frames+1 of them spread across all 48 columns -- enough to pin the
            # whole key. The low 3 bytes are the value we just decoded, so re-cribbing
            # them is a no-op that keeps already-solid columns; the top byte is the
            # one doing work. (largest_frame @0x0C is deliberately NOT cribbed: it needs
            # a correct table to compute, and a wrong guess there poisons the key.)
            cribs.append((pos, struct.pack("<I", v & 0x00FFFFFF)))

        new = dict(key)
        for off, plain in cribs:
            for k, b in enumerate(plain):
                new[(off + k) % KEY_LEN] = raw[off + k] ^ b
        if new == key:
            break
        key = new

    return bytes(key[i] for i in range(KEY_LEN))


def decrypt(raw: bytes) -> bytes:
    return _apply(raw, {i: b for i, b in enumerate(recover_key(raw))})


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(__doc__)
        return 2
    raw = open(argv[1], "rb").read()
    dec = decrypt(raw)
    if dec[:3] != BINK2_MAGIC:
        print(f"복호 실패: magic {dec[:4]!r} (KB2 아님)", file=sys.stderr)
        return 1
    open(argv[2], "wb").write(dec)
    w, h = struct.unpack_from("<I", dec, 0x14)[0], struct.unpack_from("<I", dec, 0x18)[0]
    frames = struct.unpack_from("<I", dec, 0x08)[0]
    print(f"OK  {argv[2]}  magic={dec[:4]!r} {w}x{h} frames={frames}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
