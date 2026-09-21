"""Append the installed source-baked boss cinematics to their Character bodies.

Map donors are offline bake inputs only. Existing body sections and exact WANM
payloads are preserved; installation and authoring publication are separate.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))
from bake_valtan_original_cinematic_actors import append, sections, wm

RESOURCES = ROOT / "Client/Bin/Resources"
BODIES = {
    "kouku": "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel",
    "saydon": "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel",
    "large_saydon": "Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel",
}
# Explicit original inputs remain stable after WorldSequence starts using bodies.
SPECS = (
    ("kouku", "gate2.intro.kouku", "Map/KakulSaydon/Gate2Intro/Kouku/Kouku.wmodel", "gate2_intro_27s"),
    ("kouku", "gate2.clear.kouku", "Map/KakulSaydon/SourceSequences/kouku.gate2.clear/Kouku/Kouku.wmodel", "kouku.gate2.clear.kouku"),
    ("kouku", "gate2.maze.kouku", "Map/KakulSaydon/SourceSequences/kouku.gate2.maze/Kouku/Kouku.wmodel", "kouku.gate2.maze.kouku"),
    ("saydon", "gate2.intro.saydon", "Map/KakulSaydon/Gate2Intro/Saydon/Saydon.wmodel", "gate2_intro_27s"),
    ("saydon", "gate1.full.saydonbook", "Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonBook/SaydonBook.wmodel", "kouku.gate1.full.saydonbook"),
    ("saydon", "gate1.full.saydonstage", "Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonStage/SaydonStage.wmodel", "kouku.gate1.full.saydonstage"),
    ("saydon", "gate1.full.saydonfinale", "Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonFinale/SaydonFinale.wmodel", "kouku.gate1.full.saydonfinale"),
    ("saydon", "gate2.clear.saydonarrival", "Map/KakulSaydon/SourceSequences/kouku.gate2.clear/SaydonArrival/SaydonArrival.wmodel", "kouku.gate2.clear.saydonarrival"),
    ("saydon", "gate3.intro.saydonarrival", "Map/KakulSaydon/SourceSequences/kouku.gate3.intro/SaydonArrival/SaydonArrival.wmodel", "kouku.gate3.intro.saydonarrival"),
    ("saydon", "bingo.encore.saydon", "Map/KakulSaydon/SourceSequences/kouku.bingo.encore/Saydon/Saydon.wmodel", "kouku.bingo.encore.saydon"),
    ("large_saydon", "gate2.clear.largesaydon", "Map/KakulSaydon/SourceSequences/kouku.gate2.clear/LargeSaydon/LargeSaydon.wmodel", "kouku.gate2.clear.largesaydon"),
)
DONOR_TO_BODY = {donor: BODIES[kind] for kind, _, donor, _ in SPECS}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def animation_payloads(data):
    result = {}
    for kind, _, name, payload in sections(data):
        if kind == 4:
            name = wm.fixed_name(name)
            require(name not in result, "Duplicate animation name: " + name)
            result[name] = payload
    return result


def append_idempotent(original, additions):
    existing = animation_payloads(original)
    missing = []
    for name, payload in additions:
        require(0 < len(name.encode("ascii")) < 40, "Invalid clip name: " + name)
        if name in existing:
            require(existing[name] == payload, "Animation name/content collision: " + name)
        else:
            missing.append((name, payload))
            existing[name] = payload
    candidate = append(original, missing) if missing else original
    require(sections(candidate)[:len(sections(original))] == sections(original),
            "An existing body section changed")
    return candidate, len(missing)


def validate_keys(animation):
    require(np.isfinite(animation.duration_ticks) and animation.duration_ticks > 0
            and np.isfinite(animation.ticks_per_second) and animation.ticks_per_second > 0,
            "Invalid clip timing: " + animation.name)
    count = 0
    require(animation.channels, "No animation channels: " + animation.name)
    for channel in animation.channels:
        for keys in (channel.position_keys, channel.rotation_keys, channel.scale_keys):
            array = np.asarray(keys)
            require(len(array) > 0 and np.isfinite(array).all(), "Invalid keys: " + animation.name)
            require(np.all(np.diff(array[:, 0]) >= 0) and array[0, 0] >= 0
                    and array[-1, 0] <= animation.duration_ticks + .001,
                    "Invalid key time: " + animation.name)
            count += len(array)
        lengths = np.linalg.norm(np.asarray(channel.rotation_keys)[:, 1:], axis=1)
        require(np.max(np.abs(lengths - 1)) < .0001, "Unnormalized rotation: " + animation.name)
    return count


def canonical_baked_asset(resources, donor_asset, clip_name):
    """Source generators must not silently reintroduce a Map boss reference."""
    target = DONOR_TO_BODY.get(donor_asset)
    if target is None:
        return donor_asset
    source_clips = animation_payloads((resources / donor_asset).read_bytes())
    target_clips = animation_payloads((resources / target).read_bytes())
    require(clip_name in source_clips and target_clips.get(clip_name) == source_clips[clip_name],
            "Bake/install Character cinematic first: " + target + " / " + clip_name)
    return target


def install_baked_clip(resources, donor_asset, clip_name, *, donor_path=None):
    """Source bake tools append one new clip using backup, freshness and replace.

    The canonical path can be a hard link in a candidate Resources tree. Atomic
    replacement breaks that link instead of writing through it into live data.
    """
    asset = DONOR_TO_BODY.get(donor_asset)
    if asset is None:
        return donor_asset
    path = resources / asset
    donor_path = Path(donor_path) if donor_path is not None else resources / donor_asset
    original, donor = path.read_bytes(), donor_path.read_bytes()
    skeleton = lambda data: [p for k, _, _, p in sections(data) if k == 3]
    require(len(skeleton(original)) == 1 and skeleton(original) == skeleton(donor),
            "Donor skeleton/rest basis differs: " + donor_asset)
    payloads = animation_payloads(donor)
    require(clip_name in payloads, "Missing donor clip: " + clip_name)
    model = wm.read_wmodel(donor_path, include_geometry=False, animation_names=(clip_name,))
    validate_keys(next(a for a in model.animations if a.name == clip_name))
    candidate, count = append_idempotent(original, [(clip_name, payloads[clip_name])])
    if not count:
        return asset
    backup = ROOT / "out/KoukuCharacterCinematicBake/backup" / (sha(original) + ".wmodel")
    backup.parent.mkdir(parents=True, exist_ok=True)
    if backup.exists():
        require(backup.read_bytes() == original, "Cinematic backup differs: " + str(backup))
    else:
        backup.write_bytes(original)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix=path.name + ".", suffix=".tmp", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(candidate)
            stream.flush()
            os.fsync(stream.fileno())
        require(path.read_bytes() == original, "Concurrent Character model change: " + asset)
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    return asset


def build(resources, output):
    require(not output.resolve().is_relative_to(resources.resolve()),
            "Candidate output must be outside the installed Resources")
    receipt = {"tool": Path(__file__).relative_to(ROOT).as_posix(), "models": [],
               "objectMappings": [], "manualVisualValidation": "USER_PENDING"}
    for kind, asset in BODIES.items():
        original_path = resources / asset
        original = original_path.read_bytes()
        original_model = wm.read_wmodel(original_path, include_geometry=False, animation_names=())
        original_sections = sections(original)
        skeleton = [payload for section_kind, _, _, payload in original_sections if section_kind == 3]
        require(len(skeleton) == 1, "Body skeleton is not unique: " + asset)
        additions, details = [], []
        for model_kind, suffix, donor_asset, clip_name in SPECS:
            if model_kind != kind:
                continue
            donor_path = resources / donor_asset
            donor_bytes = donor_path.read_bytes()
            donor_sections = sections(donor_bytes)
            require([p for k, _, _, p in donor_sections if k == 3] == skeleton,
                    "Donor skeleton/rest basis differs: " + donor_asset)
            donor = wm.read_wmodel(donor_path, include_geometry=False, animation_names=(clip_name,))
            payloads = animation_payloads(donor_bytes)
            require(clip_name in payloads, "Missing donor clip: " + clip_name)
            animation = next(a for a in donor.animations if a.name == clip_name)
            finite_keys = validate_keys(animation)
            additions.append((clip_name, payloads[clip_name]))
            details.append({"clipName": clip_name, "sourceAssetId": donor_asset,
                            "sourceSha256": sha(donor_bytes), "animationSha256": sha(payloads[clip_name]),
                            "durationSeconds": animation.duration_ticks / animation.ticks_per_second,
                            "boneCount": len(donor.skeleton_bones), "finiteKeyCount": finite_keys,
                            "skeletonBytesIdentical": True, "unitConversion": "NONE_EXACT_SKELETON"})
            receipt["objectMappings"].append({"objectId": "world.object.kouku." + suffix,
                "sourceAssetId": donor_asset, "targetAssetId": asset, "clipName": clip_name})
        candidate_bytes, added_count = append_idempotent(original, additions)
        require(append_idempotent(candidate_bytes, additions) == (candidate_bytes, 0),
                "Second append is not byte-identical: " + asset)
        candidate = output / asset
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(candidate_bytes)
        checked = wm.read_wmodel(candidate, include_geometry=False, animation_names=())
        require(len(checked.animations) == len(original_model.animations) + added_count,
                "Animation count mismatch: " + asset)
        if kind == "saydon":
            require("rpct00_evt2_rpct_showtime_01" in animation_payloads(candidate_bytes),
                    "Existing Showtime clip missing")
        receipt["models"].append({"sourceAssetId": asset, "targetAssetId": asset,
            "sourceSha256": sha(original), "candidateSha256": sha(candidate_bytes),
            "preservedSectionCount": len(original_sections),
            "preservedAnimationCount": len(original_model.animations), "addedAnimationCount": added_count,
            "animationCount": len(checked.animations), "idempotent": True, "clips": details})
        print(f"{kind}: {len(original_model.animations)} -> {len(checked.animations)} ({candidate})", flush=True)
    output.mkdir(parents=True, exist_ok=True)
    (output / "receipt.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--resources", type=Path, default=RESOURCES)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    build(args.resources, args.output)


if __name__ == "__main__":
    main()
