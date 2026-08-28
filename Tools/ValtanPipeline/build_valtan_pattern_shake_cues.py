#!/usr/bin/env python3
"""Join Valtan's raw extracted SHAKE events to the authored pattern/stage/clip
join keys and emit Data/Animation/Authored/Valtan/Valtan.patternshakecues.json.

Same three exact-string joins as build_valtan_pattern_sound_cues.py:

  Valtan.animevents SHAKE row (clip, startMs, payload spec)
    -> Valtan.patternbindings.json bindings[].clips[].clip == clip
       (gives actionId, clipOccurrenceId, that clip's own sourceStartMs/playMs)
    -> ValtanEncounter.json patterns[].stages[].actionId == actionId
       (gives patternId, stageId, durationMs)

The payload is the source CEFCameraViewShake block emitted by
extract_action_loa.py ("dur=..;in=..;out=..;x=a,f;y=a,f;z=a,f;fov=a,f") and is
stored verbatim; CCameraShakeService::Parse_PayloadSpec is the single parser
for player and boss shakes. Rows with an empty payload (legacy extraction) are
reported and skipped.

No network/DB use. Pure filesystem read + deterministic JSON write.
"""
from __future__ import annotations

import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
ANIMEVENTS_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Reference" / "Valtan" / "Valtan.animevents"
)
PATTERNBINDINGS_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Authored" / "Valtan" / "Valtan.patternbindings.json"
)
ENCOUNTER_PATH = REPO_ROOT / "Data" / "Encounters" / "Valtan" / "ValtanEncounter.json"
OUT_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Authored" / "Valtan" / "Valtan.patternshakecues.json"
)

SHAKE_ROW_RE = re.compile(
    r'^"(?P<clip>[^"]+)"\s+SHAKE\s+(?P<fields>.*?)\s*src=\S+\s*$'
)
STARTMS_RE = re.compile(r'startms=(\d+)')
PAYLOAD_RE = re.compile(r'payload="([^"]*)"')
SPEC_KEYS = ("dur", "in", "out", "x", "y", "z", "fov")


def parse_shake_rows(path: Path) -> tuple[list[tuple[str, int, str]], int]:
    rows: list[tuple[str, int, str]] = []
    empty = 0
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or " SHAKE " not in line or "src=orig" not in line:
            continue
        match = SHAKE_ROW_RE.match(line)
        if not match:
            continue
        startms_match = STARTMS_RE.search(match.group("fields"))
        payload_match = PAYLOAD_RE.search(match.group("fields"))
        if not startms_match or not payload_match:
            continue
        payload = payload_match.group(1)
        if not payload:
            empty += 1
            continue
        keys = [field.split("=", 1)[0] for field in payload.split(";")]
        if keys != list(SPEC_KEYS):
            raise SystemExit(f"unexpected SHAKE payload keys on {match.group('clip')}: {payload}")
        rows.append((match.group("clip"), int(startms_match.group(1)), payload))
    return rows, empty


def build_clip_index(patternbindings: dict) -> dict[str, list[dict]]:
    index: dict[str, list[dict]] = defaultdict(list)
    for binding in patternbindings.get("bindings", []):
        action_id = binding.get("actionId")
        for clip in binding.get("clips", []):
            clip_name = clip.get("clip")
            if not action_id or not clip_name:
                continue
            index[clip_name].append({
                "actionId": action_id,
                "clipOccurrenceId": clip.get("clipOccurrenceId"),
                "sourceStartMs": clip.get("sourceStartMs", 0),
                "playMs": clip.get("playMs", 0),
            })
    return index


def build_action_index(encounter: dict) -> dict[str, dict]:
    index: dict[str, dict] = {}
    for pattern in encounter.get("patterns", []):
        pattern_id = pattern.get("patternId")
        for stage in pattern.get("stages", []):
            action_id = stage.get("actionId")
            if not pattern_id or not action_id:
                continue
            index[action_id] = {
                "patternId": pattern_id,
                "stageId": stage.get("stageId"),
                "durationMs": stage.get("durationMs", 0),
            }
    return index


def build_cues(rows, clip_index, action_index):
    cues: list[dict] = []
    occurrence_counters: dict[tuple[str, str], int] = defaultdict(int)
    stats = {"totalShakeRows": len(rows), "unmatchedClip": 0,
             "unmatchedAction": 0, "outsideClipSegment": 0, "matched": 0}
    for clip, start_ms, payload in rows:
        bindings = clip_index.get(clip)
        if not bindings:
            stats["unmatchedClip"] += 1
            continue
        for binding in bindings:
            action_id = binding["actionId"]
            clip_occurrence_id = binding["clipOccurrenceId"]
            action_entry = action_index.get(action_id)
            if not action_entry:
                stats["unmatchedAction"] += 1
                continue
            source_start_ms = binding["sourceStartMs"]
            play_ms = binding["playMs"]
            if start_ms < source_start_ms or (
                play_ms != 0 and start_ms >= source_start_ms + play_ms
            ):
                stats["outsideClipSegment"] += 1
                continue
            key = (action_id, clip_occurrence_id)
            occurrence_counters[key] += 1
            binding_id = f"cue.shake.{clip_occurrence_id}.{occurrence_counters[key]:02d}"
            cues.append({
                "bindingId": binding_id,
                "occurrenceId": f"{binding_id}.occurrence.01",
                "patternId": action_entry["patternId"],
                "stageId": action_entry["stageId"],
                "actionId": action_id,
                "clipOccurrenceId": clip_occurrence_id,
                "repeatPolicy": "once",
                "startMs": start_ms,
                "shake": payload,
            })
            stats["matched"] += 1
    return cues, stats


def main() -> int:
    rows, empty = parse_shake_rows(ANIMEVENTS_PATH)
    patternbindings = json.loads(PATTERNBINDINGS_PATH.read_text(encoding="utf-8"))
    encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
    cues, stats = build_cues(rows, build_clip_index(patternbindings), build_action_index(encounter))
    cues.sort(key=lambda c: (c["actionId"], c["clipOccurrenceId"], c["startMs"], c["occurrenceId"]))
    document = {
        "schema": "lostark.valtan-pattern-shake-cues",
        "formatVersion": 1,
        "ownerArchetypeId": "BOSS_VALTAN",
        "cues": cues,
    }
    OUT_PATH.write_text(json.dumps(document, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print(f"Total SHAKE rows in Valtan.animevents: {stats['totalShakeRows']} (empty payload skipped: {empty})")
    print(f"Unmatched (no patternbindings clip): {stats['unmatchedClip']}")
    print(f"Unmatched (clip matched but no encounter stage actionId): {stats['unmatchedAction']}")
    print(f"Skipped (startMs outside this occurrence's own clip segment): {stats['outsideClipSegment']}")
    print(f"Matched cue rows written: {stats['matched']}")
    print(f"Wrote {OUT_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
