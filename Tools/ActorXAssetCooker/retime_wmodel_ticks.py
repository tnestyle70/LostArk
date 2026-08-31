#!/usr/bin/env python3
"""Re-express every WModel clip on a different tick rate without changing timing.

The runtime ignores the ``ticksPerSecond`` stored in a ``.wmodel`` animation and
always advances cooked clips at ``CAnimation``'s ``COOKED_TICK_RATE`` (30). A
clip authored on any other rate therefore plays at the wrong speed: a 2.0 s clip
stored as 2000 ticks at 1000 ticks/second is replayed over 2000/30 = 66.7 s.

This tool rewrites the duration, the stored rate and every key time so the clip
describes the same wall-clock timing on the requested rate. Nothing else in the
package moves: the section sizes, key values and channel table are untouched, so
the file stays byte-compatible with every other reader.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import struct
import sys
from pathlib import Path
from typing import Any

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
ANIMATION_HEADER = struct.Struct("<4sIffIIB7s")
ANIMATION_CHANNEL = struct.Struct("<QIIIIIIiI")
VECTOR_KEY = struct.Struct("<4f")
QUATERNION_KEY = struct.Struct("<5f")

ANIMATION_SECTION_TYPE = 4
DURATION_FIELD_OFFSET = 8
TICK_RATE_FIELD_OFFSET = 12
MAX_WMODEL_BYTES = 512 * 1024 * 1024


class RetimeError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RetimeError(message)


def fixed_name(value: bytes) -> str:
    return value.split(b"\0", 1)[0].decode("ascii", "strict")


def read_bounded(path: Path) -> bytearray:
    try:
        size = path.stat().st_size
    except OSError as error:
        raise RetimeError(f"Could not stat the WModel: {error}") from error
    require(0 < size <= MAX_WMODEL_BYTES, "WModel size is outside the accepted range")
    data = bytearray(path.read_bytes())
    require(len(data) == size, "WModel size changed while it was being read")
    return data


def find_animation_sections(data: bytearray) -> list[tuple[int, str]]:
    """Return the absolute WANM header offset and clip name of every animation."""
    require(len(data) >= FILE_HEADER.size + MODEL_HEADER.size, "WModel is truncated")
    magic, major, _, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT" and major == 1 and flags == 0
        and content_size == len(data) - FILE_HEADER.size,
        "WModel WINT header is invalid",
    )
    content = FILE_HEADER.size
    model = MODEL_HEADER.unpack_from(data, content)
    # Only the section table and the animation count are this tool's business;
    # a mesh/skeleton floor belongs to the geometry validators, not here.
    require(model[0] == b"WMOD" and model[1] >= 1, "WModel WMOD header is invalid")
    section_count = model[1]
    animation_count = model[2]
    table = content + MODEL_HEADER.size
    require(
        table + section_count * SECTION_DESC.size <= len(data),
        "WModel section table is truncated",
    )

    found: list[tuple[int, str]] = []
    for index in range(section_count):
        type_id, _, offset, size, raw_name = SECTION_DESC.unpack_from(
            data, table + index * SECTION_DESC.size
        )
        if type_id != ANIMATION_SECTION_TYPE:
            continue
        section = content + offset
        require(
            section + size <= len(data) and size > FILE_HEADER.size,
            f"WModel section {index} is out of range",
        )
        nested_magic, nested_major, _, nested_flags, nested_size = (
            FILE_HEADER.unpack_from(data, section)
        )
        require(
            nested_magic == b"WINT" and nested_major == 1 and nested_flags == 0
            and nested_size == size - FILE_HEADER.size,
            f"WModel animation {index} WINT header is invalid",
        )
        header_offset = section + FILE_HEADER.size
        header = ANIMATION_HEADER.unpack_from(data, header_offset)
        require(header[0] == b"WANM", f"WModel animation {index} has no WANM header")
        found.append((header_offset, fixed_name(raw_name)))

    require(
        len(found) == animation_count,
        "WModel animation count differs from the section table",
    )
    return found


def scale_key_times(
    data: bytearray, header_offset: int, factor: float, label: str
) -> int:
    header = ANIMATION_HEADER.unpack_from(data, header_offset)
    channel_count = header[1]
    event_count = header[5]
    # Event records carry their own layout that this tool does not decode, so a
    # clip that owns events is refused instead of being silently mis-timed.
    require(event_count == 0, f"{label} carries {event_count} events; refusing to retime")

    channel_table = header_offset + ANIMATION_HEADER.size
    key_block = channel_table + channel_count * ANIMATION_CHANNEL.size
    require(key_block <= len(data), f"{label} channel table is truncated")

    scaled = 0
    for index in range(channel_count):
        row = ANIMATION_CHANNEL.unpack_from(
            data, channel_table + index * ANIMATION_CHANNEL.size
        )
        spans = (
            (row[2], row[1], VECTOR_KEY.size),
            (row[4], row[3], QUATERNION_KEY.size),
            (row[6], row[5], VECTOR_KEY.size),
        )
        for byte_offset, count, stride in spans:
            start = key_block + byte_offset
            require(
                start + count * stride <= len(data),
                f"{label} key span is outside the package",
            )
            for key in range(count):
                at = start + key * stride
                time = struct.unpack_from("<f", data, at)[0]
                require(math.isfinite(time) and time >= 0.0,
                        f"{label} has a non-finite or negative key time")
                struct.pack_into("<f", data, at, time * factor)
                scaled += 1
    return scaled


def retime(
    data: bytearray, target_rate: float, expected_rate: float | None
) -> list[dict[str, Any]]:
    report: list[dict[str, Any]] = []
    for header_offset, name in find_animation_sections(data):
        header = ANIMATION_HEADER.unpack_from(data, header_offset)
        duration = header[2]
        source_rate = header[3]
        label = f"clip '{name}'"
        require(
            math.isfinite(source_rate) and source_rate > 0.0,
            f"{label} has an invalid ticksPerSecond",
        )
        require(
            math.isfinite(duration) and duration > 0.0,
            f"{label} has an invalid duration",
        )
        if expected_rate is not None:
            require(
                abs(source_rate - expected_rate) <= 1e-6 * max(1.0, expected_rate),
                f"{label} is {source_rate} ticks/second, expected {expected_rate}",
            )
        seconds = duration / source_rate
        factor = target_rate / source_rate
        scaled = scale_key_times(data, header_offset, factor, label)
        struct.pack_into("<f", data, header_offset + DURATION_FIELD_OFFSET,
                         duration * factor)
        struct.pack_into("<f", data, header_offset + TICK_RATE_FIELD_OFFSET,
                         target_rate)
        report.append({
            "name": name,
            "sourceTicksPerSecond": source_rate,
            "sourceDurationTicks": duration,
            "ticksPerSecond": target_rate,
            "durationTicks": duration * factor,
            "durationSeconds": seconds,
            "scaledKeyTimes": scaled,
        })
    require(bool(report), "WModel contains no animation to retime")
    return report


def verify(data: bytearray, expected: list[dict[str, Any]], target_rate: float) -> None:
    sections = find_animation_sections(data)
    require(len(sections) == len(expected), "Retimed animation count changed")
    for (header_offset, name), row in zip(sections, expected):
        require(name == row["name"], "Retimed clip order changed")
        header = ANIMATION_HEADER.unpack_from(data, header_offset)
        require(abs(header[3] - target_rate) <= 1e-6,
                f"clip '{name}' did not adopt the target rate")
        seconds = header[2] / header[3]
        require(
            abs(seconds - row["durationSeconds"]) <= 1e-4 * max(1.0, row["durationSeconds"]),
            f"clip '{name}' wall-clock duration changed: "
            f"{row['durationSeconds']} -> {seconds}",
        )


def write_atomically(path: Path, data: bytes) -> None:
    temporary = path.with_name(path.name + ".retime.tmp")
    try:
        temporary.write_bytes(data)
        os.replace(temporary, path)
    except OSError as error:
        temporary.unlink(missing_ok=True)
        raise RetimeError(f"Could not replace the WModel: {error}") from error


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wmodel", required=True, type=Path)
    parser.add_argument("--ticks-per-second", required=True, type=float)
    parser.add_argument(
        "--expect-ticks-per-second",
        type=float,
        default=None,
        help="Refuse the package unless every clip currently uses this rate.",
    )
    parser.add_argument("--report", type=Path, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    require(
        math.isfinite(args.ticks_per_second) and args.ticks_per_second > 0.0,
        "--ticks-per-second must be a finite positive number",
    )
    if args.expect_ticks_per_second is not None:
        require(
            math.isfinite(args.expect_ticks_per_second)
            and args.expect_ticks_per_second > 0.0,
            "--expect-ticks-per-second must be a finite positive number",
        )
    data = read_bounded(args.wmodel)
    rows = retime(data, args.ticks_per_second, args.expect_ticks_per_second)
    verify(data, rows, args.ticks_per_second)
    write_atomically(args.wmodel, bytes(data))
    verify(read_bounded(args.wmodel), rows, args.ticks_per_second)
    if args.report is not None:
        args.report.write_text(
            json.dumps(
                {
                    "schema": "lostark.wmodel-tick-retime",
                    "formatVersion": 1,
                    "wmodel": str(args.wmodel),
                    "ticksPerSecond": args.ticks_per_second,
                    "clips": rows,
                },
                ensure_ascii=False,
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )
    print(
        "WMODEL_TICK_RETIME_OK "
        f"clips={len(rows)} ticksPerSecond={args.ticks_per_second} "
        f"wmodel={args.wmodel}"
    )


if __name__ == "__main__":
    try:
        main()
    except Exception as error:  # noqa: BLE001 - reported as one contract failure
        print(f"WMODEL_TICK_RETIME_ERROR {error}", file=sys.stderr)
        sys.exit(1)
