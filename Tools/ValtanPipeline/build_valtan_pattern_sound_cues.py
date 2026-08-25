#!/usr/bin/env python3
"""Join Valtan's raw extracted SOUND events to the authored pattern/stage/clip
join keys and emit Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json.

Three real, exact-string joins (see .md/TJ/08-25/2026-08-25_VALTAN_PATTERN_SOUND_CUE_PLAN.md
section 1 for the measured proof each key actually lines up):

  Valtan.animevents SOUND row (clip, startMs, bank, event)
    -> Valtan.patternbindings.json bindings[].clips[].clip == clip
       (gives actionId, clipOccurrenceId, that clip's own sourceStartMs/loop)
    -> ValtanEncounter.json patterns[].stages[].actionId == actionId
       (gives patternId, stageId, durationMs)

Rows whose bank is S_BGM_CommanderRaid or S_Systems are skipped (not monster
voice/impact sound -- see PLAN section 5) and reported separately, not counted
as unmatched. A row whose clip or actionId has no join match is also skipped,
not fatal -- Valtan's authored patterns don't cover every clip in the raw
animevents yet (phase 2/3 gaps), same reasoning as the player GunSlinger/
Slayer sound gaps.

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
    REPO_ROOT / "Data" / "Animation" / "Authored" / "Valtan" / "Valtan.patternsoundcues.json"
)

SKIPPED_BANKS = {"S_BGM_COMMANDERRAID", "S_SYSTEMS"}

SOUND_ROW_RE = re.compile(
    r'^"(?P<clip>[^"]+)"\s+SOUND\s+(?P<fields>.*?)\s*src=\S+\s*$'
)
STARTMS_RE = re.compile(r'startms=(\d+)')
PAYLOAD_RE = re.compile(r'payload="([^"]*)"')


def parse_sound_rows(path: Path) -> list[tuple[str, int, str]]:
    """Returns (clip, startMs, payload) for every real SOUND row, src=orig only."""
    rows: list[tuple[str, int, str]] = []
    text = path.read_text(encoding="utf-8")
    for line in text.splitlines():
        line = line.strip()
        if not line or " SOUND " not in line or "src=orig" not in line:
            continue
        match = SOUND_ROW_RE.match(line)
        if not match:
            continue
        startms_match = STARTMS_RE.search(match.group("fields"))
        payload_match = PAYLOAD_RE.search(match.group("fields"))
        if not startms_match or not payload_match or not payload_match.group(1):
            continue
        rows.append((match.group("clip"), int(startms_match.group(1)), payload_match.group(1)))
    return rows


def split_bank_event(payload: str) -> tuple[str, str]:
    bank, _, event = payload.partition(".")
    return bank, event if event else bank


def build_clip_index(patternbindings: dict) -> dict[str, list[dict]]:
    """clip string -> list of {actionId, clipOccurrenceId, sourceStartMs, playMs}.
    One binding's clip can theoretically repeat across bindings, so this is a
    list, not a single hit. sourceStartMs/playMs carry each clip occurrence's
    own segment window within the raw clip -- some occurrences (e.g. a
    "...project-tuned.prep-repeat..." variant) only cover a slice of the full
    clip's raw timeline, not the whole thing, so a SOUND row's clip-local
    startMs only belongs to occurrences whose window actually contains it."""
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
    """actionId -> {patternId, stageId, durationMs} (stage actionIds are unique --
    each stage belongs to exactly one pattern)."""
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


def build_cues(rows: list[tuple[str, int, str]],
               clip_index: dict[str, list[dict]],
               action_index: dict[str, dict]) -> tuple[list[dict], dict[str, int]]:
    cues: list[dict] = []
    occurrence_counters: dict[tuple[str, str], int] = defaultdict(int)
    stats = {
        "totalSoundRows": len(rows),
        "skippedBank": 0,
        "unmatchedClip": 0,
        "unmatchedAction": 0,
        "outsideClipSegment": 0,
        "matched": 0,
    }

    for clip, start_ms, payload in rows:
        bank, event = split_bank_event(payload)
        if bank.upper() in SKIPPED_BANKS:
            stats["skippedBank"] += 1
            continue

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

            # Same window check CValtanPatternSoundCueDocument::Parse_Text performs
            # at load time -- filtered here too so a row that only belongs to one
            # of several occurrences sharing this clip name doesn't get attached
            # to occurrences whose own declared segment doesn't contain it (e.g. a
            # "...project-tuned.prep-repeat..." slice occurrence of the same clip).
            source_start_ms = binding["sourceStartMs"]
            play_ms = binding["playMs"]
            if start_ms < source_start_ms or (
                play_ms != 0 and start_ms >= source_start_ms + play_ms
            ):
                stats["outsideClipSegment"] += 1
                continue

            key = (action_id, clip_occurrence_id)
            occurrence_counters[key] += 1
            occurrence_index = occurrence_counters[key]

            # clip_occurrence_id in full, not just its last dot-segment: two
            # clip occurrences on the same action can share a last segment
            # (e.g. "...windup.clip.01" vs "...windup.project-tuned.prep-repeat.clip.01"),
            # which previously collapsed to the same bindingId and got rejected
            # as a duplicate by CValtanPatternSoundCueDocument's uniqueness check.
            binding_id = f"cue.sound.{clip_occurrence_id}.{occurrence_index:02d}"
            cues.append({
                "bindingId": binding_id,
                "occurrenceId": f"{binding_id}.occurrence.01",
                "patternId": action_entry["patternId"],
                "stageId": action_entry["stageId"],
                "actionId": action_id,
                "clipOccurrenceId": clip_occurrence_id,
                "soundBank": bank,
                "soundEvent": event,
                "repeatPolicy": "once",
                "startMs": start_ms,
            })
            stats["matched"] += 1

    return cues, stats


def main() -> int:
    rows = parse_sound_rows(ANIMEVENTS_PATH)
    patternbindings = json.loads(PATTERNBINDINGS_PATH.read_text(encoding="utf-8"))
    encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))

    clip_index = build_clip_index(patternbindings)
    action_index = build_action_index(encounter)

    cues, stats = build_cues(rows, clip_index, action_index)
    cues.sort(key=lambda c: (c["actionId"], c["clipOccurrenceId"], c["startMs"], c["occurrenceId"]))

    document = {
        "schema": "lostark.valtan-pattern-sound-cues",
        "formatVersion": 1,
        "ownerArchetypeId": "BOSS_VALTAN",
        "cues": cues,
    }

    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUT_PATH.write_text(
        json.dumps(document, ensure_ascii=False, indent=1) + "\n",
        encoding="utf-8",
    )

    print(f"Total SOUND rows in Valtan.animevents: {stats['totalSoundRows']}")
    print(f"Skipped (S_BGM_CommanderRaid/S_Systems bank, out of scope): {stats['skippedBank']}")
    print(f"Unmatched (no patternbindings clip): {stats['unmatchedClip']}")
    print(f"Unmatched (clip matched but no encounter stage actionId): {stats['unmatchedAction']}")
    print(f"Skipped (startMs outside this occurrence's own clip segment): {stats['outsideClipSegment']}")
    print(f"Matched cue rows written: {stats['matched']}")
    print(f"Wrote {OUT_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
