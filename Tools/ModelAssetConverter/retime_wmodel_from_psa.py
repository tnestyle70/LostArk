#!/usr/bin/env python3
"""Restore per-clip PSA AnimRate values in a Winters WModel package.

The original DimensionMaster conversion preserved key times and frame spans but
wrote 24 ticks/second into every WANM section.  This tool validates the complete
PSA/WModel clip order and duration contract before atomically replacing only the
``ticksPerSecond`` fields.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import struct
from pathlib import Path
from collections.abc import Sequence
from typing import Any


FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
PSA_CHUNK = struct.Struct("<20siii")
PSA_ANIM_INFO_SIZE = 168


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def fixed_name(value: bytes) -> str:
    return value.split(b"\0", 1)[0].decode("ascii", "strict")


def read_psa_animation_infos(path: Path) -> list[dict[str, Any]]:
    data = path.read_bytes()
    offset = 0
    result: list[dict[str, Any]] = []
    while offset + PSA_CHUNK.size <= len(data):
        raw_id, _, size, count = PSA_CHUNK.unpack_from(data, offset)
        chunk_id = fixed_name(raw_id)
        offset += PSA_CHUNK.size
        if size < 0 or count < 0 or size * count > len(data) - offset:
            raise ValueError(f"PSA chunk {chunk_id} is truncated")
        if chunk_id == "ANIMINFO":
            if size != PSA_ANIM_INFO_SIZE:
                raise ValueError(f"PSA ANIMINFO size is {size}, expected 168")
            for index in range(count):
                entry = offset + index * size
                name = fixed_name(data[entry : entry + 64])
                (
                    total_bones,
                    _,
                    _,
                    _,
                    _,
                    track_time,
                    anim_rate,
                    _,
                    first_raw_frame,
                    num_raw_frames,
                ) = struct.unpack_from("<iiiifffiii", data, entry + 128)
                if (
                    not name
                    or total_bones <= 0
                    or not math.isfinite(track_time)
                    or not math.isfinite(anim_rate)
                    or anim_rate <= 0.0
                    or num_raw_frames <= 1
                ):
                    raise ValueError(f"PSA animation {index} metadata is invalid")
                result.append(
                    {
                        "index": index,
                        "name": name,
                        "trackTime": track_time,
                        "animRate": anim_rate,
                        "firstRawFrame": first_raw_frame,
                        "numRawFrames": num_raw_frames,
                    }
                )
        offset += size * count
    if offset != len(data) or not result:
        raise ValueError("PSA chunk stream is incomplete or has no ANIMINFO")
    return result


def read_wmodel_animation_sections(data: bytes) -> list[dict[str, Any]]:
    if len(data) < FILE_HEADER.size + MODEL_HEADER.size:
        raise ValueError("WModel is truncated")
    magic, major, _, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    if (
        magic != b"WINT"
        or major != 1
        or flags != 0
        or content_size != len(data) - FILE_HEADER.size
    ):
        raise ValueError("WModel WINT header is invalid")
    content = FILE_HEADER.size
    model_magic, section_count, animation_count, _, *_ = MODEL_HEADER.unpack_from(
        data, content
    )
    if model_magic != b"WMOD" or section_count < 2:
        raise ValueError("WModel WMOD header is invalid")
    table = content + MODEL_HEADER.size
    if table + section_count * SECTION_DESC.size > len(data):
        raise ValueError("WModel section table is truncated")
    result = []
    for row in range(section_count):
        type_id, index, offset, size, raw_name = SECTION_DESC.unpack_from(
            data, table + row * SECTION_DESC.size
        )
        if offset > content_size or size > content_size - offset:
            raise ValueError(f"WModel section {row} is out of range")
        if type_id != 4:
            continue
        section = content + offset
        nested_magic, nested_major, _, nested_flags, nested_size = (
            FILE_HEADER.unpack_from(data, section)
        )
        if (
            nested_magic != b"WINT"
            or nested_major != 1
            or nested_flags != 0
            or nested_size != size - FILE_HEADER.size
        ):
            raise ValueError(f"WModel animation section {index} is invalid")
        animation = section + FILE_HEADER.size
        if data[animation : animation + 4] != b"WANM":
            raise ValueError(f"WModel animation {index} has no WANM header")
        duration_ticks, ticks_per_second = struct.unpack_from(
            "<ff", data, animation + 8
        )
        if (
            not math.isfinite(duration_ticks)
            or duration_ticks <= 0.0
            or not math.isfinite(ticks_per_second)
            or ticks_per_second <= 0.0
        ):
            raise ValueError(f"WModel animation {index} timing is invalid")
        result.append(
            {
                "index": index,
                "name": fixed_name(raw_name),
                "durationTicks": duration_ticks,
                "ticksPerSecond": ticks_per_second,
                "ticksPerSecondOffset": animation + 12,
            }
        )
    result.sort(key=lambda value: value["index"])
    if len(result) != animation_count or [row["index"] for row in result] != list(
        range(animation_count)
    ):
        raise ValueError("WModel animation section indices are not contiguous")
    return result


def retime_wmodel(
    wmodel_path: Path,
    psa_paths: Sequence[Path],
    receipt_path: Path,
    runtime_prefix: str = "pc_sp_m_00_sk_",
    source_logical_path: str | None = None,
) -> dict[str, Any]:
    if isinstance(psa_paths, Path):
        psa_paths = [psa_paths]
    psa_paths = list(psa_paths)
    if not psa_paths:
        raise ValueError("At least one PSA is required")
    # An AnimSet can be assembled from several PSA files. Their sequences are
    # concatenated and renumbered so one clip keeps one stable source index.
    source: list[dict[str, Any]] = []
    for path in psa_paths:
        for entry in read_psa_animation_infos(path):
            entry = dict(entry)
            entry["index"] = len(source)
            entry["sourcePsa"] = str(path)
            source.append(entry)
    names = [str(entry["name"]) for entry in source]
    if len(set(names)) != len(names):
        raise ValueError("PSA sequence names collide across the supplied PSA files")
    before = wmodel_path.read_bytes()
    runtime = read_wmodel_animation_sections(before)
    if len(source) != len(runtime):
        raise ValueError(
            f"PSA/WModel animation count mismatch: {len(source)} != {len(runtime)}"
        )

    source_runtime_order = sorted(
        source,
        key=lambda value: (
            runtime_prefix + str(value["name"])
        ).casefold(),
    )
    patched = bytearray(before)
    clips = []
    for psa, wmodel in zip(source_runtime_order, runtime, strict=True):
        expected_name = runtime_prefix.casefold() + psa["name"].casefold()
        runtime_name = str(wmodel["name"]).casefold()
        if not expected_name.startswith(runtime_name):
            raise ValueError(
                f"PSA/WModel clip order mismatch at {psa['index']}: "
                f"{psa['name']} != {wmodel['name']}"
            )
        expected_duration_ticks = float(psa["numRawFrames"] - 1)
        if abs(float(wmodel["durationTicks"]) - expected_duration_ticks) > 0.001:
            raise ValueError(
                f"PSA/WModel frame span mismatch for {psa['name']}: "
                f"{expected_duration_ticks} != {wmodel['durationTicks']}"
            )
        struct.pack_into(
            "<f",
            patched,
            int(wmodel["ticksPerSecondOffset"]),
            float(psa["animRate"]),
        )
        clips.append(
            {
                "index": int(psa["index"]),
                "runtimeIndex": int(wmodel["index"]),
                "sourceClip": str(psa["name"]),
                "sourcePsa": str(psa["sourcePsa"]),
                "runtimeClip": str(wmodel["name"]),
                "durationTicks": float(wmodel["durationTicks"]),
                "sourceAnimRate": float(psa["animRate"]),
                "previousTicksPerSecond": float(wmodel["ticksPerSecond"]),
                "durationSeconds": float(wmodel["durationTicks"])
                / float(psa["animRate"]),
            }
        )

    after = bytes(patched)
    decoded_after = read_wmodel_animation_sections(after)
    for psa, wmodel in zip(source_runtime_order, decoded_after, strict=True):
        if abs(float(wmodel["ticksPerSecond"]) - float(psa["animRate"])) > 1e-5:
            raise ValueError(f"WModel retime verification failed for {psa['name']}")

    if after != before:
        temporary = wmodel_path.with_suffix(wmodel_path.suffix + ".retime.tmp")
        temporary.write_bytes(after)
        os.replace(temporary, wmodel_path)

    receipt = {
        "schema": "lostark.wmodel-animation-retime-receipt",
        "formatVersion": 1,
        "sourceContract": "PSA_ANIMINFO_ANIMRATE",
        "sourcePsa": source_logical_path or str(psa_paths[0]),
        "sourcePsaSha256": hashlib.sha256(psa_paths[0].read_bytes()).hexdigest(),
        "sourcePsaFiles": [
            {
                "path": str(path),
                "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            }
            for path in psa_paths
        ],
        "runtimeWModel": str(wmodel_path),
        "beforeSha256": sha256_bytes(before),
        "afterSha256": sha256_bytes(after),
        "animationCount": len(clips),
        "changedAnimationCount": sum(
            abs(row["previousTicksPerSecond"] - row["sourceAnimRate"]) > 1e-5
            for row in clips
        ),
        "sourceRateRestorationComplete": True,
        "clips": clips,
    }
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_receipt = receipt_path.with_suffix(receipt_path.suffix + ".tmp")
    temporary_receipt.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary_receipt.replace(receipt_path)
    return receipt


def validate_retime_receipt(
    wmodel_path: Path,
    psa_path: Path,
    receipt_path: Path,
    runtime_prefix: str = "pc_sp_m_00_sk_",
    expected_source_sha256: str | None = None,
    expected_before_sha256: str | None = None,
    expected_after_sha256: str | None = None,
    expected_ticks_per_second: float | None = None,
) -> dict[str, Any]:
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if (
        receipt.get("schema") != "lostark.wmodel-animation-retime-receipt"
        or receipt.get("formatVersion") != 1
        or receipt.get("sourceContract") != "PSA_ANIMINFO_ANIMRATE"
        or receipt.get("sourceRateRestorationComplete") is not True
    ):
        raise ValueError("WModel retime receipt header is invalid")

    source_bytes = psa_path.read_bytes()
    runtime_bytes = wmodel_path.read_bytes()
    source_sha256 = sha256_bytes(source_bytes)
    runtime_sha256 = sha256_bytes(runtime_bytes)
    receipt_source_sha256 = str(receipt.get("sourcePsaSha256") or "").lower()
    receipt_before_sha256 = str(receipt.get("beforeSha256") or "").lower()
    receipt_after_sha256 = str(receipt.get("afterSha256") or "").lower()
    for label, value in (
        ("sourcePsaSha256", receipt_source_sha256),
        ("beforeSha256", receipt_before_sha256),
        ("afterSha256", receipt_after_sha256),
    ):
        if len(value) != 64 or any(character not in "0123456789abcdef" for character in value):
            raise ValueError(f"WModel retime receipt {label} is invalid")
    if source_sha256 != receipt_source_sha256:
        raise ValueError("PSA bytes do not match the WModel retime receipt")
    if runtime_sha256 != receipt_after_sha256:
        raise ValueError("Deployed WModel bytes do not match the retimed receipt")
    if expected_source_sha256 is not None and source_sha256 != expected_source_sha256.lower():
        raise ValueError("PSA bytes do not match the expected source identity")
    if expected_before_sha256 is not None and receipt_before_sha256 != expected_before_sha256.lower():
        raise ValueError("WModel retime receipt before identity changed")
    if expected_after_sha256 is not None and runtime_sha256 != expected_after_sha256.lower():
        raise ValueError("Deployed WModel bytes do not match the expected after identity")

    source = read_psa_animation_infos(psa_path)
    runtime = read_wmodel_animation_sections(runtime_bytes)
    source_runtime_order = sorted(
        source,
        key=lambda value: (runtime_prefix + str(value["name"])).casefold(),
    )
    clips = receipt.get("clips")
    if (
        not isinstance(clips, list)
        or len(source_runtime_order) != len(runtime)
        or len(clips) != len(runtime)
        or receipt.get("animationCount") != len(runtime)
    ):
        raise ValueError("PSA/WModel/receipt animation count mismatch")

    changed_count = 0
    observed_rates: list[float] = []
    for psa, wmodel, clip in zip(source_runtime_order, runtime, clips, strict=True):
        expected_name = runtime_prefix.casefold() + str(psa["name"]).casefold()
        runtime_name = str(wmodel["name"]).casefold()
        expected_duration_ticks = float(psa["numRawFrames"] - 1)
        source_rate = float(psa["animRate"])
        if (
            not expected_name.startswith(runtime_name)
            or abs(float(wmodel["durationTicks"]) - expected_duration_ticks) > 0.001
            or abs(float(wmodel["ticksPerSecond"]) - source_rate) > 1e-5
        ):
            raise ValueError(f"Retimed WModel clip contract changed for {psa['name']}")
        if expected_ticks_per_second is not None and abs(source_rate - expected_ticks_per_second) > 1e-5:
            raise ValueError(f"PSA clip rate changed for {psa['name']}")
        if not isinstance(clip, dict) or (
            clip.get("index") != int(psa["index"])
            or clip.get("runtimeIndex") != int(wmodel["index"])
            or clip.get("sourceClip") != str(psa["name"])
            or clip.get("runtimeClip") != str(wmodel["name"])
            or abs(float(clip.get("durationTicks", -1.0)) - expected_duration_ticks) > 0.001
            or abs(float(clip.get("sourceAnimRate", -1.0)) - source_rate) > 1e-5
            or abs(float(clip.get("durationSeconds", -1.0)) - expected_duration_ticks / source_rate) > 1e-6
        ):
            raise ValueError(f"WModel retime receipt clip changed for {psa['name']}")
        previous_rate = float(clip.get("previousTicksPerSecond", -1.0))
        if not math.isfinite(previous_rate) or previous_rate <= 0.0:
            raise ValueError(f"WModel retime receipt previous rate is invalid for {psa['name']}")
        changed_count += abs(previous_rate - source_rate) > 1e-5
        observed_rates.append(float(wmodel["ticksPerSecond"]))

    if receipt.get("changedAnimationCount") != changed_count:
        raise ValueError("WModel retime receipt changed-animation count is invalid")
    return {
        "receiptVerified": True,
        "sourcePsaSha256": source_sha256,
        "beforeSha256": receipt_before_sha256,
        "afterSha256": runtime_sha256,
        "animationCount": len(runtime),
        "ticksPerSecond": observed_rates,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wmodel", required=True, type=Path)
    parser.add_argument(
        "--psa",
        required=True,
        type=Path,
        action="append",
        help="Source PSA; repeat for an AnimSet assembled from several PSA files",
    )
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--runtime-prefix", default="pc_sp_m_00_sk_")
    parser.add_argument("--source-logical-path")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--expected-source-sha256")
    parser.add_argument("--expected-before-sha256")
    parser.add_argument("--expected-after-sha256")
    parser.add_argument("--expected-ticks-per-second", type=float)
    args = parser.parse_args()
    if args.check:
        result = validate_retime_receipt(
            args.wmodel,
            args.psa,
            args.receipt,
            args.runtime_prefix,
            args.expected_source_sha256,
            args.expected_before_sha256,
            args.expected_after_sha256,
            args.expected_ticks_per_second,
        )
        print(json.dumps(result, sort_keys=True))
        return 0
    receipt = retime_wmodel(
        args.wmodel,
        args.psa,
        args.receipt,
        args.runtime_prefix,
        args.source_logical_path,
    )
    print(
        json.dumps(
            {
                key: receipt[key]
                for key in (
                    "animationCount",
                    "changedAnimationCount",
                    "beforeSha256",
                    "afterSha256",
                )
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
