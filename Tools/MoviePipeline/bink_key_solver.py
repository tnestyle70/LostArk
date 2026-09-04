# -*- coding: utf-8 -*-
"""Exact 48-byte XOR key recovery for Lost Ark *.ipk movie containers.

Replaces the statistical seed of the first ipk_to_bk2 version with a constraint
solver over the Bink header and frame table, which is enough to pin every key
column without guessing:

  * magic 'KB2' (Bink 2) or 'BIK' (Bink 1) -- both exist in EFGame/Movies; the
    so-called "KB2i" family is plain Bink 1 ('BIKi').
  * file_size-8 @0x04, every u32 header field's zero top byte, the KB2-only
    constant 0x02050F00 @0x2C, frame table entry 0 = header size | 1,
    last entry = file size.
  * the frame table is 4-byte aligned and strictly increasing, and every frame
    starts with one or more little-endian u32s whose top byte is 0 (Bink 2:
    frame flags + increasing slice offsets, Bink 1 with alpha: alpha plane
    size).

Key columns are grouped 4 per u32 lane (period 48 = 12 lanes). Header cribs
fix the top byte of every lane and the low bytes of lanes 0/1/2/4/10/11; the
remaining lanes are solved by candidate elimination against the table, and
the finished key must reproduce largest_frame @0x0C as the real maximum frame
size.
"""
from __future__ import annotations

import struct

import numpy as np

KEY_LEN = 48
LANES = KEY_LEN // 4


class SolveError(Exception):
    pass


def _u32(v: int) -> bytes:
    return struct.pack("<I", v)


class _Key:
    def __init__(self):
        self.k: list[int | None] = [None] * KEY_LEN

    def set(self, col: int, val: int):
        col %= KEY_LEN
        if self.k[col] is not None and self.k[col] != val:
            raise SolveError(f"key column {col} conflict {self.k[col]:02x} != {val:02x}")
        self.k[col] = val

    def crib(self, raw: bytes, off: int, plain: bytes):
        for i, b in enumerate(plain):
            self.set(off + i, raw[off + i] ^ b)

    def known(self, col: int) -> bool:
        return self.k[col % KEY_LEN] is not None

    def byte(self, raw: bytes, off: int) -> int:
        k = self.k[off % KEY_LEN]
        if k is None:
            raise SolveError("unknown column")
        return raw[off] ^ k

    def u32_known(self, off: int) -> bool:
        return all(self.known(off + i) for i in range(4))

    def u32(self, raw: bytes, off: int) -> int:
        return (self.byte(raw, off) | self.byte(raw, off + 1) << 8
                | self.byte(raw, off + 2) << 16 | self.byte(raw, off + 3) << 24)

    def copy(self) -> "_Key":
        c = _Key()
        c.k = list(self.k)
        return c

    def arrays(self):
        vals = np.array([0 if k is None else k for k in self.k], dtype=np.uint8)
        known = np.array([k is not None for k in self.k], dtype=bool)
        return vals, known


def _lane_of(off: int) -> int:
    return (off % KEY_LEN) // 4


# --------------------------------------------------------------------------- frames
def _frame_ok(raw: bytes, key: _Key, e: int, size: int | None, sig: int, is_kb2: bool) -> bool:
    """Frame at offset e starts with `sig` zero-top u32 words; Bink 2 slice offsets
    increase and stay inside the frame."""
    n = len(raw)
    if e + 4 * sig > n:
        return False
    words = []
    for s in range(sig):
        p = e + 4 * s
        if not key.u32_known(p):
            words.append(None)
            continue
        w = key.u32(raw, p)
        if w >> 24:
            return False
        words.append(w)
    if is_kb2:
        prev = 4 * sig - 1
        for w in words[1:]:
            if w is None:
                continue
            if w <= prev or (size is not None and w >= size):
                return False
            prev = w
    elif words[0] is not None and (words[0] == 0 or (size is not None and words[0] >= size)):
        return False
    return True


def _signature_words(raw: bytes, key: _Key, starts: list[int]) -> int:
    """Leading zero-top u32 words shared by the given frame starts (cap 4). Frame 0
    is a real keyframe, so it bounds the count from above; an empty later frame
    only ever lowers it."""
    s_min = 4
    for e in starts:
        s = 0
        while s < 4 and e + 4 * s + 3 < len(raw) and key.byte(raw, e + 4 * s + 3) == 0:
            s += 1
        s_min = min(s_min, s)
    if s_min == 0:
        raise SolveError("frames do not start with a zero-top word")
    return s_min


# ------------------------------------------------------------------------ layout
def _solve_layout(raw: bytes, magic: bytes, audio: int) -> tuple[bytes, dict]:
    n = len(raw)
    key = _Key()
    key.crib(raw, 0x00, magic)
    key.crib(raw, 0x04, _u32(n - 8))
    hdr_len = 0x2C + 12 * audio
    is_kb2 = magic == b"KB2"
    table_off = hdr_len + (4 if is_kb2 else 0)
    if is_kb2:
        key.crib(raw, hdr_len, _u32(0x02050F00))
    # zero top bytes of num_frames, largest_frame, frames_again, width, height,
    # fps_num, fps_den, video_flags, audio_tracks
    for off in (0x0B, 0x0F, 0x13, 0x17, 0x1B, 0x1F, 0x23, 0x27, 0x2B):
        key.set(off, raw[off])
    # width and height are below 65536: their third byte is 0 as well
    for off in (0x16, 0x1A):
        key.set(off, raw[off])
    key.crib(raw, 0x28, _u32(audio))
    for t in range(audio):
        base = 0x2C + 12 * t
        key.set(base + 3, raw[base + 3])    # per-track max packet size
        key.set(base + 11, raw[base + 11])  # per-track id
    # remaining top-byte columns: the first table entries are far below 16 MB
    for col in range(3, KEY_LEN, 4):
        if key.known(col):
            continue
        first_i = ((col - (table_off + 3)) // 4) % LANES
        votes = [raw[table_off + 4 * i + 3] for i in range(first_i, first_i + 4 * LANES, LANES)
                 if table_off + 4 * i + 3 < n]
        if not votes:
            raise SolveError("file too small")
        key.set(col, max(set(votes), key=votes.count))

    frames = _find_frame_count(raw, key, table_off, is_kb2)
    key.crib(raw, 0x08, _u32(frames))
    key.crib(raw, 0x10, _u32(frames))
    key.crib(raw, table_off, _u32(table_off + (frames + 1) * 4 | 1))
    key.crib(raw, table_off + 4 * frames, _u32(n))

    entries = _entries_known(raw, key, table_off, frames)
    sig = _signature_words(raw, key, [entries[i] for i in (0, 1) if i in entries])
    solved = _solve_lanes(raw, key, table_off, frames, sig, is_kb2)
    kb = bytes(solved.k)  # type: ignore[arg-type]
    info = {"magic": magic + bytes([raw[3] ^ kb[3]]), "audio": audio, "frames": frames,
            "table_off": table_off, "signature_words": sig}
    return kb, info


def _entries_known(raw: bytes, key: _Key, table_off: int, frames: int) -> dict[int, int]:
    out = {}
    for i in range(frames + 1):
        off = table_off + 4 * i
        if key.u32_known(off):
            out[i] = key.u32(raw, off) & ~1
    return out


def _find_frame_count(raw: bytes, key: _Key, table_off: int, is_kb2: bool) -> int:
    """Enumerate num_frames: the last table entry must decode to the file size, and
    every entry that becomes fully known must keep the table aligned and strictly
    increasing and point at a frame start."""
    n = len(raw)
    top_n = (n >> 24) & 0xFF
    max_f = min(200000, (n - table_off) // 4 - 1)
    hits = []
    for f in range(1, max_f + 1):
        last = table_off + 4 * f
        if raw[last + 3] ^ key.k[(last + 3) % KEY_LEN] != top_n:
            continue
        cand = key.copy()
        try:
            cand.crib(raw, 0x08, _u32(f))
            cand.crib(raw, 0x10, _u32(f))
            cand.crib(raw, table_off, _u32(table_off + (f + 1) * 4 | 1))
            cand.crib(raw, last, _u32(n))
            starts = [table_off + (f + 1) * 4]
            if f > 1 and cand.u32_known(table_off + 4):
                starts.append(cand.u32(raw, table_off + 4) & ~1)
            sig = _signature_words(raw, cand, starts)
        except SolveError:
            continue
        ok = True
        prev = -1
        checked = 0
        for i in range(f + 1):
            off = table_off + 4 * i
            if not cand.u32_known(off):
                continue
            v = cand.u32(raw, off)
            e = v & ~1
            if v & 2 or e <= prev or v > n:
                ok = False
                break
            if i < f and not _frame_ok(raw, cand, e, None, sig, is_kb2):
                ok = False
                break
            prev = e
            checked += 1
        if ok and checked >= 3:
            hits.append((f, checked))
    if not hits:
        raise SolveError("no frame count fits")
    if len(hits) > 1:
        # a spurious small count only ever survives on a handful of entries; the real
        # one is confirmed by every fully-known entry of the whole table
        hits.sort(key=lambda h: -h[1])
        if hits[0][1] == hits[1][1]:
            raise SolveError(f"ambiguous frame count {[h[0] for h in hits[:6]]}")
    return hits[0][0]


# ------------------------------------------------------------------------- lanes
def _solve_lanes(raw: bytes, key: _Key, table_off: int, frames: int, sig: int, is_kb2: bool) -> _Key:
    lane_entries: dict[int, list[int]] = {}
    for i in range(frames + 1):
        lane_entries.setdefault(_lane_of(table_off + 4 * i), []).append(i)
    unknown = [g for g in range(LANES) if not all(key.known(4 * g + b) for b in range(3))]
    results: list[_Key] = []
    _search(raw, key, table_off, frames, sig, is_kb2, lane_entries, unknown, results)
    if not results:
        raise SolveError("no key satisfies the frame table")
    if len(results) > 1:
        # very short tables can leave one lane with two consistent keys; the header
        # fields living in that lane separate them
        scored = sorted(((_header_score(raw, k), i) for i, k in enumerate(results)), reverse=True)
        if scored[0][0] == scored[1][0]:
            raise SolveError(f"{len(results)} keys satisfy the frame table")
        return results[scored[0][1]]
    return results[0]


_KNOWN_FPS = {(30, 1), (60, 1), (24, 1), (25, 1), (30000, 1001), (60000, 1001),
              (24000, 1001), (10000000, 333333), (10000000, 416666)}


def _header_score(raw: bytes, key: _Key) -> int:
    width, height = key.u32(raw, 0x14), key.u32(raw, 0x18)
    fps = (key.u32(raw, 0x1C), key.u32(raw, 0x20))
    flags = key.u32(raw, 0x24)
    score = 0
    score += 2 if fps in _KNOWN_FPS else 0
    score += 1 if width % 2 == 0 and height % 2 == 0 else 0
    score += 1 if flags & 0xFFE0FF00 == 0 else 0  # observed flags: 0x0010001x
    return score


def _search(raw, key: _Key, table_off, frames, sig, is_kb2, lane_entries, unknown, results):
    if len(results) > 1:
        return
    if not unknown:
        if _verify(raw, key, table_off, frames, sig, is_kb2):
            results.append(key)
        return
    entries = _entries_known(raw, key, table_off, frames)
    best = None
    for g in unknown:
        cands = _lane_candidates(raw, key, table_off, frames, entries, lane_entries.get(g, []), g, sig, is_kb2)
        if best is None or len(cands) < len(best[1]):
            best = (g, cands)
        if len(cands) <= 1:
            break
    g, cands = best
    if not cands:
        return
    if len(cands) > 64:
        raise SolveError(f"lane {g}: {len(cands)} key candidates remain")
    rest = [u for u in unknown if u != g]
    for b0, b1, b2 in cands:
        nk = key.copy()
        nk.set(4 * g, b0)
        nk.set(4 * g + 1, b1)
        nk.set(4 * g + 2, b2)
        _search(raw, nk, table_off, frames, sig, is_kb2, lane_entries, rest, results)


def _verify(raw: bytes, key: _Key, table_off: int, frames: int, sig: int, is_kb2: bool) -> bool:
    n = len(raw)
    ents = [key.u32(raw, table_off + 4 * i) for i in range(frames + 1)]
    if ents[0] & 1 == 0 or (ents[-1] & ~1) != n:
        return False
    prev = -1
    largest = 0
    for v in ents:
        e = v & ~1
        if v & 2 or e <= prev:
            return False
        prev = e
    for i in range(frames):
        size = (ents[i + 1] & ~1) - (ents[i] & ~1)
        largest = max(largest, size)
        if not _frame_ok(raw, key, ents[i] & ~1, size, sig, is_kb2):
            return False
    if key.u32(raw, 0x0C) != largest:
        return False
    width, height = key.u32(raw, 0x14), key.u32(raw, 0x18)
    fps_num, fps_den = key.u32(raw, 0x1C), key.u32(raw, 0x20)
    return 16 <= width <= 8192 and 16 <= height <= 8192 and fps_num > 0 and fps_den > 0


def _bounds(entries: dict[int, int], i: int, frames: int, n: int) -> tuple[int, int]:
    lo, hi = 0, n
    for j in range(i - 1, -1, -1):
        if j in entries:
            lo = entries[j] + 4 * (i - j)
            break
    for j in range(i + 1, frames + 1):
        if j in entries:
            hi = entries[j] - 4 * (j - i)
            break
    return lo, hi


def _lane_candidates(raw, key: _Key, table_off, frames, entries, idx, g, sig, is_kb2):
    """All (b0, b1, b2) key bytes for lane g under which every table entry in the
    lane is aligned, inside its monotonic bounds and points at a frame signature.
    Bit 0 of an entry is the keyframe flag, so it is matched with bit 0 of b0
    masked out and settled afterwards by majority (keyframes are rare)."""
    n = len(raw)
    arr = np.frombuffer(raw, dtype=np.uint8)
    keyarr, knownarr = key.arrays()
    c0, c1, c2, c3 = 4 * g, 4 * g + 1, 4 * g + 2, 4 * g + 3
    fixed = (None if key.k[c0] is None else key.k[c0] & 0xFE, key.k[c1], key.k[c2])
    cands: set[tuple[int, int, int]] | None = None
    checked: list[int] = []
    for i in idx:
        if i in entries:
            continue
        off = table_off + 4 * i
        top = raw[off + 3] ^ key.k[c3]
        lo, hi = _bounds(entries, i, frames, n)
        lo = max(lo, top << 24, 4)
        hi = min(hi, (top << 24) | 0xFFFFFF, n - 4 * sig)
        if hi < lo:
            return []
        e = np.arange((lo + 3) & ~3, hi + 1, 4, dtype=np.int64)
        mask = np.ones(len(e), dtype=bool)
        words = []
        for s in range(sig):
            p = e + 4 * s
            mask &= (arr[p + 3] ^ keyarr[(p + 3) % KEY_LEN]) == 0
            wk = np.ones(len(e), dtype=bool)
            w = np.zeros(len(e), dtype=np.int64)
            for b in range(3):
                col = (p + b) % KEY_LEN
                wk &= knownarr[col]
                w |= (arr[p + b] ^ keyarr[col]).astype(np.int64) << (8 * b)
            words.append((w, wk))
        if is_kb2:
            prev = np.full(len(e), 4 * sig - 1, dtype=np.int64)
            for w, wk in words[1:]:
                mask &= ~wk | (w > prev)
                prev = np.where(wk, w, prev)
        else:
            w, wk = words[0]
            mask &= ~wk | (w > 0)
        e = e[mask]
        if len(e) == 0:
            return []
        k0 = (e & 0xFE).astype(np.uint8) ^ np.uint8(raw[off] & 0xFE)
        k1 = ((e >> 8) & 0xFF).astype(np.uint8) ^ np.uint8(raw[off + 1])
        k2 = ((e >> 16) & 0xFF).astype(np.uint8) ^ np.uint8(raw[off + 2])
        sel = np.ones(len(e), dtype=bool)
        for fx, kk in zip(fixed, (k0, k1, k2)):
            if fx is not None:
                sel &= kk == fx
        here = set(zip(k0[sel].tolist(), k1[sel].tolist(), k2[sel].tolist()))
        cands = here if cands is None else cands & here
        checked.append(off)
        if not cands:
            return []
    if cands is None:
        return []
    out = []
    for k0m, k1, k2 in sorted(cands):
        if key.k[c0] is not None:
            out.append((key.k[c0], k1, k2))
            continue
        # bit 0 of the raw byte is keyframe_flag ^ key_bit0; most frames are not keyframes
        ones = sum((raw[off] ^ k0m) & 1 for off in checked)
        out.append((k0m | (1 if ones * 2 > len(checked) else 0), k1, k2))
    return out


def solve_key(raw: bytes) -> tuple[bytes, dict]:
    errors = []
    for magic in (b"KB2", b"BIK"):
        for audio in (0, 1):
            try:
                return _solve_layout(raw, magic, audio)
            except SolveError as e:
                errors.append(f"{magic.decode()} audio={audio}: {e}")
    raise SolveError("; ".join(errors))
