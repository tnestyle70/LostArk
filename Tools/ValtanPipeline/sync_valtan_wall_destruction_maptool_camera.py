#!/usr/bin/env python3
"""Synchronize the original Valtan Event_02 wall-destruction camera into Map Tool."""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
import sys


AREA_ID = "LV_LUT_HEARTRB_ED"
SHOT_ID = "source.phase2-wall-destruction.cut01"
CUTSCENE_ID = "editor.cutscene.valtan.phase2-wall-destruction"
INSTANCE_ID = "world.sequence.instance.valtan.source-preview.phase2"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def build_original_event02_keys(camera_document: dict) -> list[dict]:
    cues = {
        cue["stageId"]: cue
        for cue in camera_document["cues"]
        if cue.get("patternId") == "VALTAN_ARENA_BREAK_109"
    }
    required = {"IMPACT_HOLD", "WIDE_REVEAL", "RECOVERY"}
    if not required.issubset(cues):
        raise ValueError(f"missing Valtan Event_02 source cues: {sorted(required - set(cues))}")

    segments = (
        (cues["IMPACT_HOLD"], 600, 0),
        (cues["WIDE_REVEAL"], 0, 500),
        (cues["RECOVERY"], 0, 2800),
    )
    keys: list[dict] = []
    for cue, trim_before_ms, output_offset_ms in segments:
        for source_key in cue["keyframes"]:
            source_time = int(source_key["timeMs"])
            if source_time < trim_before_ms:
                continue
            time_ms = source_time - trim_before_ms + output_offset_ms
            if keys and time_ms == keys[-1]["timeMs"]:
                continue
            key = copy.deepcopy(source_key)
            key.pop("cutBefore", None)
            key["timeMs"] = time_ms
            key["sceneId"] = f"{SHOT_ID}.k{len(keys) + 1:03d}"
            keys.append(key)

    times = [key["timeMs"] for key in keys]
    if len(keys) != 104 or times[0] != 0 or times[-1] != 5500:
        raise ValueError(f"unexpected Event_02 key contract: count={len(keys)}, range={times[0]}..{times[-1]}")
    if any(left >= right for left, right in zip(times, times[1:])):
        raise ValueError("Event_02 camera key times are not strictly increasing")
    return keys


def expected_shot(keys: list[dict]) -> dict:
    first = keys[0]
    return {
        "shotId": SHOT_ID,
        "displayName": "원본 발탄 벽·기둥 파괴 (Event_02, 0.0~5.5초)",
        "defaultHoldMs": 5500,
        "transitionEasing": "LINEAR",
        "activation": "PATTERN_ONLY",
        "sequenceInstanceId": INSTANCE_ID,
        "box": {
            "center": copy.deepcopy(first["eye"]),
            "halfExtents": [0.25, 0.25, 0.25],
            "yawDegrees": 0,
        },
        "eye": copy.deepcopy(first["eye"]),
        "lookAt": copy.deepcopy(first["lookAt"]),
        "fovYDegrees": first["fovYDegrees"],
        "blendInMs": 0,
        "blendOutMs": 0,
        "priority": 0,
        "cameraTrack": {
            "durationMs": 5500,
            "interpolation": "LINEAR",
            "easing": "LINEAR",
            "keyframes": keys,
        },
    }


def expected_cutscene() -> dict:
    return {
        "cutsceneId": CUTSCENE_ID,
        "displayName": "발탄 벽·기둥 파괴 (원본 Event_02)",
        "durationMs": 5500,
        "cameraCuts": [{"cutId": "cut01", "shotId": SHOT_ID, "startMs": 0}],
        "worldInstanceIds": [INSTANCE_ID],
    }


def replace_by_id(items: list[dict], key: str, identity: str, value: dict) -> bool:
    matches = [index for index, item in enumerate(items) if item.get(key) == identity]
    if len(matches) > 1:
        raise ValueError(f"duplicate {key}: {identity}")
    if matches:
        index = matches[0]
        if items[index] == value:
            return False
        items[index] = value
        return True
    items.append(value)
    return True


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="update authoring camera data")
    args = parser.parse_args()

    repo = Path(__file__).resolve().parents[2]
    camera_source_path = repo / "Data/Encounters/Valtan/ValtanCinematicCamera.json"
    authoring_path = repo / f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.camerashots.json"
    world_sequence_path = repo / f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.worldsequences.json"

    source = load_json(camera_source_path)
    document = load_json(authoring_path)
    world_sequences = load_json(world_sequence_path)
    keys = build_original_event02_keys(source)

    instances = world_sequences.get("instances", [])
    if not any(item.get("instanceId") == INSTANCE_ID for item in instances):
        raise ValueError(f"missing paired world sequence instance: {INSTANCE_ID}")

    before_roar = copy.deepcopy(next(item for item in document["cutscenes"] if item.get("cutsceneId") == "editor.cutscene.valtan.roar"))
    changed = replace_by_id(document["shots"], "shotId", SHOT_ID, expected_shot(keys))
    changed |= replace_by_id(document["cutscenes"], "cutsceneId", CUTSCENE_ID, expected_cutscene())
    after_roar = next(item for item in document["cutscenes"] if item.get("cutsceneId") == "editor.cutscene.valtan.roar")
    if before_roar != after_roar:
        raise ValueError("existing phase-2 red-cloud roar cutscene changed")

    if args.write:
        if changed:
            document["revision"] = int(document.get("revision", 0)) + 1
            authoring_path.write_text(
                json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            print(f"updated: {authoring_path}")
        else:
            print(f"already synchronized: {authoring_path}")
        return 0

    if changed:
        print("Map Tool wall-destruction camera is missing or stale; run with --write", file=sys.stderr)
        return 1
    print("ok: Valtan Event_02 Map Tool camera (104 keys, 0..5500 ms)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
