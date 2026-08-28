#!/usr/bin/env python3
"""Bake the Valtan `b_root` travel into a per-pattern-stage root motion curve.

The player already moves this way: `Data/Animation/RootMotion/<Class>.rootmotion.json`
holds the displacement the animator baked into the clip, the balance publisher
turns it into `SKILLROOTMOTION` rows, and the server steps the body along that
curve instead of sliding it at a constant speed.  Valtan had no such document,
so its clips carried travel that nothing consumed: the server pushed the
transform at `distance / duration` while the mesh drifted along the baked root,
and the two never agreed.

This produces the boss side of that contract.  For every encounter pattern stage
it resolves the clip the stage actually plays, reads the `b_root` position keys
out of the cooked animation set, and writes the forward/lateral displacement
sampled across the stage window.

Chain, all of it already authored:

    ValtanEncounter.json  pattern -> stage -> actionId, durationMs
    Valtan.patternbindings.json  actionId -> clip, sourceStartMs, playRate
    MN_RPBF_01_AnimSet.wmodel  clip -> b_root position keys

Axis mapping is the one the player uses.  The rig sends local (x, y, z) through
an armature node that scales by 100 and the model prototype pre-transform scales
by 0.0001, so the surviving factor is 0.01:

    forward = 0.01 * raw.x
    lateral = -0.01 * raw.y
    up      = -0.01 * raw.z

`up` is written for shape parity with the player document but is always zero and
the publisher never packs it.  The pose already carries the leap, so moving the
server Y as well would double it - the same reason the player side drops it.

A stage that declares its own `motion.distance` is left alone.  Those two are
gameplay overrides authored to carry Valtan far past anything the bound clip
does (100 m and 20 m against a metre or less of baked travel), and quietly
replacing them with the curve would strand the charge short of the wall it has
to reach.  They are reported instead.
"""

from __future__ import annotations

import argparse
import json
import math
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

WMOD_MAGIC = b"WMOD"
WANM_MAGIC = b"WANM"
SECTION_ANIMATION = 4
SECTION_DESC_SIZE = 64
MODEL_META_SIZE = 32
ANIM_META_SIZE = 32
ANIM_CHANNEL_SIZE = 40
VECTOR_KEY_SIZE = 16

ROOT_BONE = "b_root"
# The armature node scales by 100 and the prototype pre-transform by 0.0001.
UNIT_TO_METRES = 0.01
# Below this the stage is standing still and an all-zero curve is noise.
MINIMUM_TRAVEL_METRES = 0.05
SAMPLE_INTERVAL_MS = 33


@dataclass(frozen=True)
class ExplicitClipSegment:
    """One finite source slice in a sequential pattern-action timeline."""

    curve: tuple[float, float, list]
    source_start_ms: float
    source_duration_ms: float
    wall_duration_ms: float
    play_rate: float
    loop: bool
    base_forward: float
    base_lateral: float
    delta_forward: float
    delta_lateral: float


def fnv1a64(text: str) -> int:
    value = 0xCBF29CE484222325
    for byte in text.encode("utf-8"):
        value ^= byte
        value = (value * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return value


def read_root_curves(animset_path: Path) -> dict[str, tuple[float, float, list]]:
    """clip name -> (durationMs, ticksPerSecond, [(tick, x, y, z), ...])."""
    data = animset_path.read_bytes()
    model_at = data.find(WMOD_MAGIC)
    if model_at < 0:
        raise SystemExit(f"No WMOD section in {animset_path}")
    _, section_count, _, _ = struct.unpack_from("<4sIII", data, model_at)
    root_hash = fnv1a64(ROOT_BONE)

    curves: dict[str, tuple[float, float, list]] = {}
    for index in range(section_count):
        desc_at = model_at + MODEL_META_SIZE + index * SECTION_DESC_SIZE
        section_type, _, offset, size = struct.unpack_from("<IIQQ", data, desc_at)
        if section_type != SECTION_ANIMATION:
            continue
        name = data[desc_at + 24:desc_at + SECTION_DESC_SIZE].split(b"\x00")[0]
        clip = name.decode("ascii", "replace")
        anim_at = data.find(WANM_MAGIC, offset, offset + 256)
        if anim_at < 0:
            continue
        _, channels, duration_ticks, ticks_per_second, _, _, _ = struct.unpack_from(
            "<4sIffIIB", data, anim_at)
        if channels == 0 or ticks_per_second <= 0.0:
            continue
        channel_at = anim_at + ANIM_META_SIZE
        for channel in range(channels):
            entry = channel_at + channel * ANIM_CHANNEL_SIZE
            bone_hash, key_count, key_offset = struct.unpack_from("<QII", data, entry)
            if bone_hash != root_hash:
                continue
            key_block = channel_at + channels * ANIM_CHANNEL_SIZE
            keys = [
                struct.unpack_from("<4f", data, key_block + key_offset + k * VECTOR_KEY_SIZE)
                for k in range(key_count)
            ]
            curves[clip] = (
                duration_ticks / ticks_per_second * 1000.0, ticks_per_second, keys)
            break
    return curves


def sample_clip(curve: tuple[float, float, list], time_ms: float) -> tuple[float, float]:
    """Forward and lateral metres the clip has travelled by `time_ms`."""
    duration_ms, ticks_per_second, keys = curve
    if not keys:
        return 0.0, 0.0
    tick = max(0.0, min(time_ms, duration_ms)) * 0.001 * ticks_per_second
    previous = keys[0]
    for key in keys:
        if key[0] >= tick:
            if key[0] == previous[0]:
                return key[1] * UNIT_TO_METRES, -key[2] * UNIT_TO_METRES
            ratio = (tick - previous[0]) / (key[0] - previous[0])
            x = previous[1] + (key[1] - previous[1]) * ratio
            y = previous[2] + (key[2] - previous[2]) * ratio
            return x * UNIT_TO_METRES, -y * UNIT_TO_METRES
        previous = key
    return keys[-1][1] * UNIT_TO_METRES, -keys[-1][2] * UNIT_TO_METRES


def build_explicit_clip_segments(
        clips: list[dict],
        curves: dict[str, tuple[float, float, list]],
) -> list[ExplicitClipSegment] | None:
    """Resolve a finite multi-clip chain using the Client timeline contract.

    `playMs` is source time, not wall time.  The Client divides that source
    slice by `playRate` before handing off to the next clip.  Natural
    (`playMs == 0`) chains intentionally stay on the legacy first-clip bake in
    this tool: changing those historical curves is outside the explicit-slice
    migration that introduced the Dash WINDUP repeats.
    """
    if len(clips) < 2:
        return None
    play_windows = [float(clip.get("playMs", 0)) for clip in clips]
    if any(play_ms <= 0.0 or not math.isfinite(play_ms)
           for play_ms in play_windows):
        return None

    segments: list[ExplicitClipSegment] = []
    for clip, play_ms in zip(clips, play_windows):
        curve = curves.get(clip["clip"])
        if curve is None:
            return None
        source_start_ms = float(clip.get("sourceStartMs", 0))
        play_rate = float(clip.get("playRate", 1.0))
        model_duration_ms = curve[0]
        if (not math.isfinite(source_start_ms) or source_start_ms < 0.0 or
                source_start_ms >= model_duration_ms or
                not math.isfinite(play_rate) or play_rate <= 0.0):
            return None
        source_duration_ms = min(
            play_ms, model_duration_ms - source_start_ms)
        wall_duration_ms = source_duration_ms / play_rate
        if source_duration_ms <= 0.0 or wall_duration_ms <= 0.0:
            return None
        base_forward, base_lateral = sample_clip(curve, source_start_ms)
        end_forward, end_lateral = sample_clip(
            curve, source_start_ms + source_duration_ms)
        segments.append(ExplicitClipSegment(
            curve=curve,
            source_start_ms=source_start_ms,
            source_duration_ms=source_duration_ms,
            wall_duration_ms=wall_duration_ms,
            play_rate=play_rate,
            loop=bool(clip.get("loop", False)),
            base_forward=base_forward,
            base_lateral=base_lateral,
            delta_forward=end_forward - base_forward,
            delta_lateral=end_lateral - base_lateral,
        ))
    return segments


def sample_explicit_clip_segments(
        segments: list[ExplicitClipSegment],
        time_ms: float,
) -> tuple[float, float]:
    """Accumulate local root travel across finite sequential source slices."""
    remaining_ms = max(0.0, time_ms)
    accumulated_forward = 0.0
    accumulated_lateral = 0.0
    for index, segment in enumerate(segments):
        if segment.loop:
            epochs = math.floor(remaining_ms / segment.wall_duration_ms)
            accumulated_forward += epochs * segment.delta_forward
            accumulated_lateral += epochs * segment.delta_lateral
            remaining_ms -= epochs * segment.wall_duration_ms
        elif index + 1 < len(segments) and \
                remaining_ms >= segment.wall_duration_ms:
            accumulated_forward += segment.delta_forward
            accumulated_lateral += segment.delta_lateral
            remaining_ms -= segment.wall_duration_ms
            continue

        source_elapsed_ms = min(
            segment.source_duration_ms,
            remaining_ms * segment.play_rate)
        forward, lateral = sample_clip(
            segment.curve,
            segment.source_start_ms + source_elapsed_ms)
        return (
            accumulated_forward + forward - segment.base_forward,
            accumulated_lateral + lateral - segment.base_lateral,
        )
    return accumulated_forward, accumulated_lateral


def build(
        repo_root: Path,
        resource_root: Path | None = None,
) -> tuple[dict, list[str]]:
    encounter = json.loads(
        (repo_root / "Data/Encounters/Valtan/ValtanEncounter.json").read_text("utf-8"))
    bindings_document = json.loads(
        (repo_root / "Data/Animation/Authored/Valtan"
                     "/Valtan.patternbindings.json").read_text("utf-8"))
    bindings = {entry["actionId"]: entry["clips"]
                for entry in bindings_document["bindings"]}
    resources = resource_root or repo_root / "Client/Bin/Resources"
    curves = read_root_curves(
        resources / "Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel")

    notes: list[str] = []
    patterns = []
    for pattern in encounter["patterns"]:
        stages = []
        for stage_index, stage in enumerate(pattern["stages"]):
            motion = stage.get("motion") or {}
            if motion.get("kind") == "PORTAL_CROSS_ARENA":
                notes.append(f"{pattern['patternId']}/{stage['stageId']}: kept portal transform motion")
                continue
            authored = float(motion.get("distance", 0.0)) if motion.get(
                "kind") == "FORWARD" else 0.0
            action_id = stage.get("actionId")
            clips = bindings.get(action_id)
            if not clips:
                continue
            clip_name = clips[0]["clip"]
            curve = curves.get(clip_name)
            if curve is None:
                continue
            explicit_segments = build_explicit_clip_segments(clips, curves)
            is_explicit_chain = len(clips) > 1 and all(
                float(clip.get("playMs", 0)) > 0.0 for clip in clips)
            if is_explicit_chain and explicit_segments is None:
                continue
            start_ms = float(clips[0].get("sourceStartMs", 0))
            play_rate = float(clips[0].get("playRate", 1.0)) or 1.0
            duration_ms = int(stage["durationMs"])
            if duration_ms <= 0:
                continue

            if explicit_segments is None:
                base_forward, base_lateral = sample_clip(curve, start_ms)

            def sample_stage_displacement(time_ms: float) -> tuple[float, float]:
                if explicit_segments is not None:
                    return sample_explicit_clip_segments(
                        explicit_segments, time_ms)
                sampled_forward, sampled_lateral = sample_clip(
                    curve, start_ms + time_ms * play_rate)
                return (
                    sampled_forward - base_forward,
                    sampled_lateral - base_lateral,
                )

            end_forward, end_lateral = sample_stage_displacement(duration_ms)
            if authored > 0.0:
                notes.append(
                    f"{pattern['patternId']}/{stage['stageId']}: kept authored "
                    f"{authored:.1f} m, clip {clip_name} bakes "
                    f"{end_forward:+.2f} m")
                continue

            samples = []
            admission_travel = max(abs(end_forward), abs(end_lateral))
            time_ms = 0
            while True:
                forward, lateral = sample_stage_displacement(time_ms)
                if explicit_segments is not None:
                    admission_travel = max(
                        admission_travel,
                        abs(forward),
                        abs(lateral),
                    )
                samples.append({
                    "timeMs": time_ms,
                    "forward": round(forward, 4),
                    "lateral": round(lateral, 4),
                    # Carried for shape parity with the player document and for
                    # review; the publisher packs forward and lateral only.
                    "up": 0.0,
                })
                if time_ms >= duration_ms:
                    break
                time_ms = min(time_ms + SAMPLE_INTERVAL_MS, duration_ms)
            if admission_travel < MINIMUM_TRAVEL_METRES:
                continue
            stages.append({
                "stageIndex": stage_index,
                "stageId": stage["stageId"],
                "clip": clip_name,
                "durationMs": duration_ms,
                "samples": samples,
            })
        if stages:
            patterns.append({"patternId": pattern["patternId"], "stages": stages})

    document = {
        "schema": "lostark.valtan-pattern-root-motion",
        "formatVersion": 1,
        "bossArchetypeId": encounter["bossArchetypeId"],
        "animationAssetId": "Valtan",
        "patterns": patterns,
    }
    return document, notes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument(
        "--resource-root", type=Path, default=None,
        help="Resources root; defaults to <repo-root>/Client/Bin/Resources.")
    parser.add_argument("--out", type=Path, default=None)
    parser.add_argument("--check", action="store_true",
                        help="Fail if the file on disk differs from a fresh bake.")
    arguments = parser.parse_args()

    document, notes = build(arguments.repo_root, arguments.resource_root)
    out_path = arguments.out or (
        arguments.repo_root / "Data/Animation/RootMotion/Valtan.rootmotion.json")
    text = json.dumps(document, ensure_ascii=False, indent=2) + "\n"

    stage_count = sum(len(p["stages"]) for p in document["patterns"])
    sample_count = sum(len(s["samples"])
                       for p in document["patterns"] for s in p["stages"])
    print(f"patterns={len(document['patterns'])} stages={stage_count} "
          f"samples={sample_count}")
    for note in notes:
        print(f"  authored override kept: {note}")

    if arguments.check:
        if not out_path.exists():
            print(f"MISSING {out_path}", file=sys.stderr)
            return 1
        if out_path.read_text("utf-8") != text:
            print(f"STALE {out_path}", file=sys.stderr)
            return 1
        print("check OK")
        return 0

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(text, encoding="utf-8", newline="\n")
    print(f"wrote {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
