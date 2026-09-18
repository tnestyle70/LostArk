#!/usr/bin/env python3
"""Repair the six DimensionMaster rider donors for the unit-basis body palette.

The legacy donor armature has scale 100; the product body's corresponding
armature and all 155 body clips have scale 1. Attach_AnimationSet copies only
animation channels, so these constant import-scale keys must match the body.
Only the three scale values of pc_sp_m_00_sk are changed. Donor carrier mesh,
rest pose, other channels, key times and clip metadata remain byte-identical.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
sys.path.insert(0, str(ROOT / "Tools/ActorXAssetCooker"))
import verify_dimensionmaster_summon_bind_pose as wm
from retime_wmodel_ticks import find_animation_sections

ARMATURE = "pc_sp_m_00_sk"
MODES = ("Dragon2", "HeavywalkerBm9", "Horse", "Hoverboard", "Swing", "Tube")
RELATIVE = Path("Character/DimensionMaster/AnimSets")
BODY = Path("Character/DimensionMaster/DimensionMaster_Character.wmodel")
SOURCE_SCALE = (100.0, 99.99999237060547, 99.99999237060547)
TARGET_SCALE = tuple(struct.unpack("<f", struct.pack("<f", value / 100.0))[0]
                     for value in SOURCE_SCALE)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def root_keys(data: bytes, sections: list[tuple[int, str]], bone_hash: int):
    result = []
    for header_at, name in sections:
        header = wm.ANIMATION_HEADER.unpack_from(data, header_at)
        table = header_at + wm.ANIMATION_HEADER.size
        keys = table + header[1] * wm.ANIMATION_CHANNEL.size
        matched = []
        for index in range(header[1]):
            channel = wm.ANIMATION_CHANNEL.unpack_from(
                data, table + index * wm.ANIMATION_CHANNEL.size)
            if channel[0] == bone_hash:
                matched.append(channel)
        require(len(matched) == 1 and matched[0][5] > 0,
                f"{name}: missing or duplicate armature scale channel")
        channel = matched[0]
        for index in range(channel[5]):
            offset = keys + channel[6] + index * wm.VECTOR_KEY.size
            require(offset + wm.VECTOR_KEY.size <= len(data), f"{name}: key out of range")
            value = wm.VECTOR_KEY.unpack_from(data, offset)
            require(all(math.isfinite(item) for item in value), f"{name}: nonfinite scale key")
            result.append((offset, value[1:]))
    require(result, "No armature keys")
    return result


def repair_keys(data: bytes, sections: list[tuple[int, str]], bone_hash: int):
    require(all(name.startswith("pc_sp_m_00_sk_ride_") for _, name in sections),
            "Only DimensionMaster riding clips are repairable")
    keys = root_keys(data, sections, bone_hash)
    values = {value for _, value in keys}
    require(values in ({SOURCE_SCALE}, {TARGET_SCALE}),
            f"Unexpected or animated armature scale: {sorted(values)}")
    corrected = bytearray(data)
    for offset, _ in keys:
        struct.pack_into("<3f", corrected, offset + 4, *TARGET_SCALE)
    # Restore only admitted fields to prove every other byte stayed intact.
    restored = bytearray(corrected)
    for offset, _ in keys:
        restored[offset + 4:offset + 16] = data[offset + 4:offset + 16]
    require(bytes(restored) == data, "Repair touched a field outside armature scale XYZ")
    return bytes(corrected), len(keys), values == {TARGET_SCALE}


def topology(model):
    return [(bone.name_hash, bone.name, bone.parent) for bone in model.skeleton_bones]


def read_body_skeleton(data: bytes):
    # Existing body WANM names include legacy truncation duplicates. Admission
    # uses those installed clips; this repair validates its WSKL and root keys
    # without imposing a new clip-name policy.
    model = wm.MODEL_HEADER.unpack_from(data, wm.FILE_HEADER.size)
    table = wm.FILE_HEADER.size + wm.MODEL_HEADER.size
    bones = []
    for index in range(model[1]):
        kind, _, offset, size, _ = wm.SECTION_DESC.unpack_from(
            data, table + index * wm.SECTION_DESC.size)
        if kind != 3:
            continue
        require(not bones, "Duplicate body skeleton")
        at = wm._read_nested_header(data, wm.FILE_HEADER.size + offset, size, "body skeleton")
        header = wm.SKELETON_HEADER.unpack_from(data, at)
        require(header[0] == b"WSKL", "Invalid body skeleton magic")
        first = at + wm.SKELETON_HEADER.size
        require(first + header[1] * wm.SKELETON_BONE.size <= at + size - wm.FILE_HEADER.size,
                "Body skeleton bone table is truncated")
        for bone in range(header[1]):
            row = wm.SKELETON_BONE.unpack_from(data, first + bone * wm.SKELETON_BONE.size)
            bones.append(wm.Bone(row[0], wm.fixed_name(row[1]), row[2], list(row[3:19])))
    require(bones, "Missing body skeleton")
    return wm.WModel([], None, [], bones, [])


def armature(model):
    matches = [bone for bone in model.skeleton_bones if bone.name == ARMATURE]
    require(len(matches) == 1, "DimensionMaster armature is missing or duplicated")
    return matches[0]


def axis_lengths(bone):
    return tuple(math.sqrt(sum(bone.transform[row * 4 + col] ** 2 for col in range(3)))
                 for row in range(3))


def run(resources: Path, out: Path, install: bool):
    body_path = resources / BODY
    body_bytes = body_path.read_bytes()
    body = read_body_skeleton(body_bytes)
    target = armature(body)
    require(all(abs(value - 1.0) < 1e-6 for value in axis_lengths(target)),
            "The target body no longer has the measured unit armature")
    body_keys = root_keys(body_bytes, find_animation_sections(bytearray(body_bytes)),
                          target.name_hash)
    require(all(value == TARGET_SCALE for _, value in body_keys),
            "The target body root scale contract changed")
    out.mkdir(parents=True, exist_ok=True)
    prepared = []
    for mode in MODES:
        relative = RELATIVE / f"DimensionMaster_Ride{mode}AnimSet.wmodel"
        path = resources / relative
        before = path.read_bytes()
        donor = wm.read_wmodel(path, include_geometry=False, animation_names=())
        require(topology(donor) == topology(body), f"{mode}: receiver topology differs")
        require(all(abs(value - 100.0) < 1e-4 for value in axis_lengths(armature(donor))),
                f"{mode}: legacy donor rest basis changed")
        sections = find_animation_sections(bytearray(before))
        after, count, already = repair_keys(before, sections, target.name_hash)
        candidate = out / "candidates" / relative
        backup = out / "backup" / relative
        candidate.parent.mkdir(parents=True, exist_ok=True)
        backup.parent.mkdir(parents=True, exist_ok=True)
        if backup.exists():
            backup_bytes = backup.read_bytes()
            restored_candidate, _, _ = repair_keys(
                backup_bytes, find_animation_sections(bytearray(backup_bytes)), target.name_hash)
            require(restored_candidate == after, f"{mode}: existing backup differs")
        else:
            backup.write_bytes(before)
        candidate.write_bytes(after)
        wm.read_wmodel(candidate, include_geometry=False, animation_names=())
        prepared.append((path, before, after, {
            "resource": relative.as_posix(), "bytes": len(before),
            "beforeSha256": digest(before), "afterSha256": digest(after),
            "clips": len(sections), "scaleKeys": count, "alreadyRepaired": already,
        }))
    installed = []
    try:
        if install:
            require(body_path.read_bytes() == body_bytes, "Target body changed during preparation")
            for path, before, after, _ in prepared:
                require(path.read_bytes() == before, f"Concurrent edit: {path}")
                if before == after:
                    continue
                temporary = path.with_name(path.name + ".rider-scale.tmp")
                require(not temporary.exists(), f"Temporary file already exists: {temporary}")
                try:
                    temporary.write_bytes(after)
                    require(path.read_bytes() == before, f"Concurrent edit: {path}")
                    os.replace(temporary, path)
                    installed.append((path, before, after))
                finally:
                    temporary.unlink(missing_ok=True)
    except Exception:
        for path, before, after in reversed(installed):
            # Never overwrite an external change while rolling back our own work.
            if path.read_bytes() == after:
                temporary = path.with_name(path.name + ".rider-scale.rollback.tmp")
                temporary.write_bytes(before)
                os.replace(temporary, path)
        raise
    report = {
        "schema": "lostark.dimensionmaster-rider-scale-repair", "formatVersion": 1,
        "bodyResource": BODY.as_posix(), "bodySha256": digest(body_bytes),
        "bodyArmatureScale": [1, 1, 1], "bodyRootScaleKeys": len(body_keys),
        "armatureBone": ARMATURE, "sourceScale": SOURCE_SCALE,
        "targetScale": TARGET_SCALE, "scaleDivisor": 100,
        "preserved": ["mesh", "materials", "skeleton", "otherBoneChannels",
                      "translation", "rotation", "keyTimes", "clipMetadata"],
        "installed": install, "clientUiExecuted": False,
        "files": [row for _, _, _, row in prepared],
    }
    (out / "repair-receipt.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--resources", type=Path, default=ROOT / "Client/Bin/Resources")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    receipt = run(args.resources.resolve(), args.out.resolve(), args.install)
    print(json.dumps({"files": len(receipt["files"]),
                      "clips": sum(row["clips"] for row in receipt["files"]),
                      "scaleKeys": sum(row["scaleKeys"] for row in receipt["files"]),
                      "installed": receipt["installed"]}))
