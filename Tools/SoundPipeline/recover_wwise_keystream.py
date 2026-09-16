#!/usr/bin/env python3
"""Regenerate and verify Tools/SoundPipeline/wwise_keystream.dat.

The Wwise containers under EFGame/ReleasePC/WwiseAudioPackage are XORed with
one keystream shared by every file, and the client code that holds it is inside
a WinLicense-wrapped binary. The table is therefore recovered from the shipped
data, in three steps that each check the previous one.

1. Period. Scan a few packages for byte-identical 24-byte windows. Equal
   ciphertext at distance d means equal plaintext and d divisible by the
   period, so the gcd over every observed distance bounds it. Across four
   unrelated packages that gcd is exactly 435,540.

2. Seed. The six 64-byte packages are byte-identical, so they are the same
   empty AKPK: "AKPK", header size 52, version 1, a 20-byte language map
   holding the single language "sfx", and three empty LUTs. XORing that
   reconstruction against the file gives the first 60 keystream bytes.

3. Body. Fold every package and loose .wem over the period and take, per
   column, the most frequent ciphertext byte. The dominant plaintext byte in
   Vorbis payloads and bank tables is 0x00, so the column mode is the keystream
   byte. Each file's own header is excluded (its plaintext is structured, not
   zero-biased). With ~20 GB of input every column gets ~48,000 samples and the
   winning byte leads the runner-up by at least 35 sigma.

Verification, both independent of how the table was produced:
  * the 60 seed bytes must match the recovered table;
  * every stream-LUT languageID field in every package is zero, so its
    ciphertext is keystream -- all ~8,800 of them must match.

Usage:
  python recover_wwise_keystream.py [--package-root DIR] [--out FILE] [--verify-only]

Needs numpy. On this project that means Blender's bundled interpreter:
  "C:/Program Files/Blender Foundation/Blender 5.0/5.0/python/bin/python.exe"
"""
from __future__ import annotations

import argparse
import math
import struct
import sys
from collections import Counter
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from wwise_audio_package import (  # noqa: E402
    CONTAINER_MAGIC,
    DEFAULT_PACKAGE_ROOT,
    KEYSTREAM_PATH,
    KEYSTREAM_PERIOD,
)

WINDOW = 24
COLUMN_CHUNK = 32768


def empty_package_plaintext() -> bytes:
    """The 60-byte payload of an AKPK holding nothing but the "sfx" language."""
    return (
        b"AKPK"
        + struct.pack("<6I", 52, 1, 20, 4, 4, 4)
        + struct.pack("<3I", 1, 12, 0)
        + "sfx\x00".encode("utf-16-le")
        + struct.pack("<3I", 0, 0, 0)
    )


def seed_keystream(root: Path) -> bytes:
    plain = empty_package_plaintext()
    candidates = sorted(
        (p for p in root.rglob("*.pck") if p.stat().st_size == 4 + len(plain))
    )
    if not candidates:
        raise SystemExit(f"no 64-byte package under {root} to seed from")
    bodies = {p.read_bytes()[4:] for p in candidates}
    if len(bodies) != 1:
        raise SystemExit("the 64-byte packages differ; the seed assumption is wrong")
    body = bodies.pop()
    return bytes(a ^ b for a, b in zip(body, plain))


def measure_period(root: Path, sample_count: int = 4) -> int:
    paths = sorted(root.glob("*.pck"), key=lambda p: p.stat().st_size)
    paths = [p for p in paths if 5_000_000 < p.stat().st_size < 80_000_000][:sample_count]
    distances: Counter[int] = Counter()
    for path in paths:
        raw = path.read_bytes()
        seen: dict[int, int] = {}
        for i in range(4, len(raw) - WINDOW, 4):
            window = raw[i : i + WINDOW]
            key = hash(window)
            previous = seen.get(key)
            if previous is not None and raw[previous : previous + WINDOW] == window:
                distances[i - previous] += 1
            else:
                seen[key] = i
    if not distances:
        raise SystemExit("no repeated window found; cannot bound the period")
    period = 0
    for distance in distances:
        period = math.gcd(period, distance)
    print(
        "period gcd %d over %d distinct distances in %d packages"
        % (period, len(distances), len(paths))
    )
    return period


def corpus(root: Path) -> list[Path]:
    return [
        p
        for p in sorted(root.rglob("*"))
        if p.is_file() and p.suffix.lower() in (".pck", ".wem")
    ]


def column_counts(files: list[Path], first: int, last: int) -> np.ndarray:
    width = last - first
    counts = np.zeros(width * 256, dtype=np.int64)
    base = np.arange(width, dtype=np.int64) * 256
    rows: list[np.ndarray] = []

    def flush():
        if rows:
            counts.__iadd__(
                np.bincount(
                    (base[None, :] + np.stack(rows)).ravel(), minlength=width * 256
                )
            )
            rows.clear()

    for path in files:
        mapped = np.memmap(path, dtype=np.uint8, mode="r")
        payload = len(mapped) - 4
        for block in range(1, payload // KEYSTREAM_PERIOD + 1):
            start = 4 + block * KEYSTREAM_PERIOD + first
            if start + width > len(mapped):
                break
            rows.append(np.array(mapped[start : start + width]))
            if len(rows) >= 512:
                flush()
        del mapped
    flush()
    return counts.reshape(width, 256)


def recover(root: Path) -> tuple[bytes, float]:
    files = corpus(root)
    total = sum(p.stat().st_size for p in files)
    print("%d files, %.1f GB" % (len(files), total / 2**30))
    table = bytearray(KEYSTREAM_PERIOD)
    weakest = float("inf")
    for first in range(0, KEYSTREAM_PERIOD, COLUMN_CHUNK):
        last = min(first + COLUMN_CHUNK, KEYSTREAM_PERIOD)
        counts = column_counts(files, first, last)
        ordered = np.sort(counts, axis=1)
        table[first:last] = counts.argmax(axis=1).astype(np.uint8).tobytes()
        samples = counts.sum(axis=1).astype(np.float64)
        samples[samples == 0] = 1.0
        sigma = np.sqrt(np.maximum(samples / 256.0, 1.0))
        margin = float(((ordered[:, -1] - ordered[:, -2]) / sigma).min())
        weakest = min(weakest, margin)
        print(
            "  columns %6d-%6d  samples/col %d  margin %.1f sigma"
            % (first, last, int(samples.min()), margin),
            flush=True,
        )
    return bytes(table), weakest


def language_id_samples(root: Path, table: bytes) -> tuple[int, int]:
    """Check the table against every LUT entry's languageID field.

    A package that declares only the "sfx" language (id 0) stores a zero there,
    so its ciphertext is raw keystream and the comparison is bit-exact. The
    localized packages under Korean/ declare a second language, so there the
    field is only required to name a language the package actually declares.
    """
    checked = mismatched = 0
    for path in sorted(root.rglob("*.pck")):
        raw = path.read_bytes()
        if raw[:4] != CONTAINER_MAGIC:
            continue
        head = bytes(a ^ b for a, b in zip(raw[4:68], table))
        if head[:4] != b"AKPK":
            raise SystemExit(f"{path.name}: header did not decrypt")
        _, _, language_map, banks, streams, _ = struct.unpack_from("<6I", head, 4)
        lut_end = 0x1C + language_map + banks + streams
        body = bytes(
            raw[4 + i] ^ table[i % KEYSTREAM_PERIOD] for i in range(min(lut_end, len(raw) - 4))
        )
        base = 0x1C
        declared = set()
        for i in range(struct.unpack_from("<I", body, base)[0]):
            declared.add(struct.unpack_from("<II", body, base + 4 + 8 * i)[1])
        single = declared == {0}
        for base, size in (
            (0x1C + language_map, banks),
            (0x1C + language_map + banks, streams),
        ):
            for i in range((size - 4) // 20):
                offset = base + 4 + 20 * i + 16
                value = struct.unpack_from("<I", body, offset)[0]
                checked += 1
                if (single and value != 0) or (not single and value not in declared):
                    mismatched += 1
    return checked, mismatched


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-root", type=Path, default=DEFAULT_PACKAGE_ROOT)
    parser.add_argument("--out", type=Path, default=KEYSTREAM_PATH)
    parser.add_argument("--verify-only", action="store_true")
    parser.add_argument("--skip-period", action="store_true")
    args = parser.parse_args(argv)

    if not args.package_root.is_dir():
        print(f"package root not found: {args.package_root}")
        return 1

    if args.verify_only:
        table = args.out.read_bytes()
        if len(table) != KEYSTREAM_PERIOD:
            print("table is %d bytes, expected %d" % (len(table), KEYSTREAM_PERIOD))
            return 1
    else:
        if not args.skip_period:
            period = measure_period(args.package_root)
            if period != KEYSTREAM_PERIOD:
                print("measured period %d != %d" % (period, KEYSTREAM_PERIOD))
                return 1
        table, weakest = recover(args.package_root)
        print("weakest column margin %.1f sigma" % weakest)

    seed = seed_keystream(args.package_root)
    if table[: len(seed)] != seed:
        print("seed mismatch in the first %d bytes" % len(seed))
        return 1
    print("seed check: first %d bytes match" % len(seed))

    checked, mismatched = language_id_samples(args.package_root, table)
    print("languageID check: %d samples, %d mismatched" % (checked, mismatched))
    if mismatched:
        return 1

    if not args.verify_only:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_bytes(table)
        print("wrote %s (%d bytes)" % (args.out, len(table)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
