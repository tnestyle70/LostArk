#!/usr/bin/env python3
"""Recover Valtan's action definitions from the Lost Ark Action `.loa` lane.

The source of truth produced by this tool is the decoded object graph:

    CEFActionObject
      CEFActionStage
        CEFActionNotify_<kind>   (Anim / AKEvent / PlayParticleEffect / ...)

`CEFActionNotify_Anim` names the animation clip a stage plays, and the
`MonsterMoveNextStage` family of notifies is how one stage hands off to the
next.  That chain is the pattern order, which the cooked `.wanim` files do not
carry.

Transition notifies carry numbers that do vary between stages: the two floats
that follow the class string are 2.0 on the stage playing `Att_Battle_2_01` and
1.1 on the stages playing `Att_Battle_2_02`, and the conditional variants carry
an eight digit identifier in the `4206xxxx` range that Valtan's own NPC rows
also use.  Those values are recorded verbatim as `fields`, with convenience
accessors, but their UNITS AND MEANINGS ARE NOT VERIFIED and this tool does not
claim them to be seconds, frames, or probabilities.

Stage order is taken to be the order the stages appear in the file.  The
unconditional `MonsterMoveNextStage` notify carries no visible target, so the
"advance to the next stage in file order" reading is an ASSUMPTION recorded in
`contract.orderAssumption` rather than a decoded field.  Only the conditional
variants name an explicit identifier.

Strings use the same convention as `extract_deploydata_props.py`:

    <u32 length including the NUL><bytes including the NUL>

The source `.loa` is opened read-only and never modified.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import struct
import tempfile
from pathlib import Path
from typing import Any, Iterable


class ActionExtractError(RuntimeError):
    pass


ACTION_OBJECT = "CEFActionObject"
ACTION_STAGE = "CEFActionStage"
NOTIFY_PREFIX = "CEFActionNotify_"

MAX_STRING_BYTES = 512
MIN_STRING_BYTES = 2

OBJECT_REFERENCE = re.compile(r"^([A-Za-z][A-Za-z0-9_]*)'([^']+)'$")

# Clip names are bare identifiers; object references and package paths are not.
CLIP_PREFIXES = (
    "att_",
    "idle_",
    "run_",
    "walk_",
    "turn_",
    "dead_",
    "fast_",
    "sk_",
    "evt",
    "act_",
    "behit_",
    "stun_",
)

# Notify kinds that hand control to another stage.
TRANSITION_KINDS = ("MonsterMoveNextStage",)

# How many fields to decode after a transition notify's class string.  The
# observed records go quiet well before this, so it is a ceiling, not a size.
TRANSITION_FIELD_COUNT = 26

# Identifiers observed in conditional transitions share the range Valtan's own
# EFTable_Npc rows use, so plain counters and flags are excluded by magnitude.
MIN_IDENTIFIER = 1_000_000
MAX_IDENTIFIER = 1_000_000_000


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            json.dump(value, output, ensure_ascii=False, indent=1)
            output.write("\n")
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def scan_strings(blob: bytes) -> list[tuple[int, str]]:
    """Every length prefixed ASCII string, as (offset of the length field, text)."""
    found: list[tuple[int, str]] = []
    cursor = 0
    limit = len(blob) - 4
    while cursor < limit:
        length = struct.unpack_from("<I", blob, cursor)[0]
        if MIN_STRING_BYTES <= length <= MAX_STRING_BYTES and (
            cursor + 4 + length <= len(blob)
        ):
            payload = blob[cursor + 4 : cursor + 4 + length]
            if payload.endswith(b"\0") and all(
                32 <= byte < 127 for byte in payload[:-1]
            ):
                found.append((cursor, payload[:-1].decode("ascii")))
                cursor += 4 + length
                continue
        cursor += 1
    return found


def is_clip_name(text: str) -> bool:
    lowered = text.lower()
    if OBJECT_REFERENCE.match(text) or "." in text or "'" in text:
        return False
    return lowered.startswith(CLIP_PREFIXES)


def split_reference(text: str) -> tuple[str, str] | None:
    match = OBJECT_REFERENCE.match(text)
    if match is None:
        return None
    return match.group(1), match.group(2)


def segment(
    strings: list[tuple[int, str]], predicate
) -> list[tuple[int, int, list[tuple[int, str]]]]:
    """Cut `strings` at every entry matching `predicate`.

    Returns (start offset, end offset, contained strings) per segment.  The
    marker string itself is the first entry of its own segment.
    """
    marks = [index for index, (_, text) in enumerate(strings) if predicate(text)]
    segments: list[tuple[int, int, list[tuple[int, str]]]] = []
    for position, index in enumerate(marks):
        stop = marks[position + 1] if position + 1 < len(marks) else len(strings)
        body = strings[index:stop]
        start = body[0][0]
        end = strings[stop][0] if stop < len(strings) else -1
        segments.append((start, end, body))
    return segments


def decode_fields(
    blob: bytes, offset: int, count: int
) -> list[dict[str, Any]]:
    """Walk `count` fields from `offset`, treating length prefixed runs as strings."""
    fields: list[dict[str, Any]] = []
    cursor = offset
    while len(fields) < count and cursor + 4 <= len(blob):
        length = struct.unpack_from("<I", blob, cursor)[0]
        if MIN_STRING_BYTES <= length <= MAX_STRING_BYTES and (
            cursor + 4 + length <= len(blob)
        ):
            payload = blob[cursor + 4 : cursor + 4 + length]
            if payload.endswith(b"\0") and all(
                32 <= byte < 127 for byte in payload[:-1]
            ):
                fields.append(
                    {
                        "index": len(fields),
                        "offset": f"0x{cursor:X}",
                        "text": payload[:-1].decode("ascii"),
                    }
                )
                cursor += 4 + length
                continue
        unsigned = struct.unpack_from("<I", blob, cursor)[0]
        real = struct.unpack_from("<f", blob, cursor)[0]
        entry: dict[str, Any] = {
            "index": len(fields),
            "offset": f"0x{cursor:X}",
            "u32": unsigned,
        }
        # Only surface a float reading when it is a plausible authored value.
        if real == real and abs(real) < 1.0e6 and (abs(real) > 1.0e-4 or real == 0.0):
            entry["f32"] = round(real, 6)
        fields.append(entry)
        cursor += 4
    return fields


def summarize_transition(fields: list[dict[str, Any]]) -> dict[str, Any]:
    """Pull the values that vary between transition records.

    Field meanings are unverified; this only reports what is stored.
    """
    numbers = [field for field in fields if "u32" in field]
    floats = [
        field.get("f32")
        for field in numbers
        if "f32" in field and field["f32"] not in (0.0,)
    ]
    identifiers = sorted(
        {
            field["u32"]
            for field in numbers
            if MIN_IDENTIFIER <= field["u32"] <= MAX_IDENTIFIER
        }
    )
    return {
        "unverifiedValues": floats,
        "conditionIdCandidates": identifiers,
        "fields": fields,
    }


def build_notify(body: list[tuple[int, str]], blob: bytes) -> dict[str, Any]:
    kind = body[0][1][len(NOTIFY_PREFIX) :]
    notify: dict[str, Any] = {
        "kind": kind,
        "offset": f"0x{body[0][0]:X}",
        "clips": [],
        "references": [],
        "labels": [],
    }
    for _, text in body[1:]:
        reference = split_reference(text)
        if reference is not None:
            notify["references"].append(
                {"class": reference[0], "path": reference[1]}
            )
        elif is_clip_name(text):
            notify["clips"].append(text)
        elif text != "None":
            notify["labels"].append(text)

    if kind.startswith(TRANSITION_KINDS):
        class_offset = body[0][0]
        class_length = struct.unpack_from("<I", blob, class_offset)[0]
        notify["transition"] = summarize_transition(
            decode_fields(
                blob, class_offset + 4 + class_length, TRANSITION_FIELD_COUNT
            )
        )
    return notify


def build_stage(
    body: list[tuple[int, str]], index: int, blob: bytes
) -> dict[str, Any]:
    notify_segments = segment(
        body, lambda text: text.startswith(NOTIFY_PREFIX)
    )
    header = body[: len(body) - sum(len(part) for _, _, part in notify_segments)]
    stage: dict[str, Any] = {
        "index": index,
        "offset": f"0x{body[0][0]:X}",
        "headerStrings": [text for _, text in header[1:] if text != "None"],
        "notifies": [
            build_notify(part, blob) for _, _, part in notify_segments
        ],
    }

    clips: list[str] = []
    for notify in stage["notifies"]:
        if notify["kind"] != "Anim":
            continue
        for clip in notify["clips"]:
            if not clips or clips[-1] != clip:
                clips.append(clip)
    stage["clips"] = clips
    stage["transitions"] = [
        notify["kind"]
        for notify in stage["notifies"]
        if notify["kind"].startswith(TRANSITION_KINDS)
    ]
    return stage


def build_action(
    body: list[tuple[int, str]], index: int, end_offset: int, blob: bytes
) -> dict[str, Any]:
    stage_segments = segment(body, lambda text: text == ACTION_STAGE)
    header_length = len(body) - sum(len(part) for _, _, part in stage_segments)
    header = body[:header_length]
    stages = [
        build_stage(part, position, blob)
        for position, (_, _, part) in enumerate(stage_segments, 1)
    ]

    sound_families = sorted(
        {
            notify_reference["path"].split(".", 1)[0]
            for stage in stages
            for notify in stage["notifies"]
            for notify_reference in notify["references"]
            if notify_reference["class"] == "AkEvent"
        }
    )

    clip_sequence: list[str] = []
    for stage in stages:
        for clip in stage["clips"]:
            if not clip_sequence or clip_sequence[-1] != clip:
                clip_sequence.append(clip)

    start = body[0][0]
    return {
        "index": index,
        "offset": f"0x{start:X}",
        "byteLength": (end_offset - start) if end_offset >= 0 else None,
        "headerStrings": [text for _, text in header[1:] if text != "None"],
        "soundFamilies": sound_families,
        "stageCount": len(stages),
        "clipSequence": clip_sequence,
        "stages": stages,
    }


def extract(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    strings = scan_strings(data)
    if not strings:
        raise ActionExtractError(f"no length prefixed strings in {path}")

    object_segments = segment(strings, lambda text: text == ACTION_OBJECT)
    if not object_segments:
        raise ActionExtractError(f"{path} contains no {ACTION_OBJECT} records")

    actions = [
        build_action(body, index, end, data)
        for index, (_, end, body) in enumerate(object_segments)
    ]

    notify_kinds: dict[str, int] = {}
    for action in actions:
        for stage in action["stages"]:
            for notify in stage["notifies"]:
                notify_kinds[notify["kind"]] = notify_kinds.get(notify["kind"], 0) + 1

    clips = sorted(
        {clip for action in actions for clip in action["clipSequence"]}
    )
    ordered = [action for action in actions if len(action["clipSequence"]) > 1]

    condition_ids = sorted(
        {
            identifier
            for action in actions
            for stage in action["stages"]
            for notify in stage["notifies"]
            for identifier in notify.get("transition", {}).get(
                "conditionIdCandidates", []
            )
        }
    )

    return {
        "schemaVersion": 2,
        "source": {
            "path": path.as_posix(),
            "bytes": len(data),
            "sha256": sha256(path),
        },
        "contract": {
            "stringEncoding": "<u32 length including NUL><bytes including NUL>",
            "orderAssumption": (
                "stages run in the order they appear in the file; the "
                "unconditional MonsterMoveNextStage notify carries no visible "
                "target field, so this is an assumption and not a decoded value"
            ),
            "transitionValuesVerified": False,
            "transitionValuesNote": (
                "transition notifies store numbers that differ per stage "
                "(2.0 vs 1.1 within one attack) and conditional variants store "
                "an identifier, but the units and meanings are unverified"
            ),
        },
        "summary": {
            "stringCount": len(strings),
            "actionCount": len(actions),
            "stageCount": sum(action["stageCount"] for action in actions),
            "actionsWithClips": sum(
                1 for action in actions if action["clipSequence"]
            ),
            "actionsWithOrder": len(ordered),
            "uniqueClipCount": len(clips),
            "conditionIdCount": len(condition_ids),
            "notifyKindCounts": dict(sorted(notify_kinds.items())),
        },
        "conditionIds": condition_ids,
        "clips": clips,
        "actions": actions,
    }


def check_expectation(label: str, actual: int, expected: int | None) -> None:
    if expected is not None and actual != expected:
        raise ActionExtractError(f"{label} {actual} != expected {expected}")


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract Valtan action stage graphs from an Action .loa file"
    )
    parser.add_argument("--action-loa", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--expect-actions", type=int)
    parser.add_argument("--expect-stages", type=int)
    parser.add_argument("--expect-clips", type=int)
    parser.add_argument(
        "--require-clip",
        action="append",
        default=[],
        help="fail unless this clip name appears (case insensitive)",
    )
    return parser.parse_args(list(argv) if argv is not None else None)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    if not args.action_loa.is_file():
        raise ActionExtractError(f"action file is missing: {args.action_loa}")

    report = extract(args.action_loa)
    summary = report["summary"]
    check_expectation("action count", summary["actionCount"], args.expect_actions)
    check_expectation("stage count", summary["stageCount"], args.expect_stages)
    check_expectation("clip count", summary["uniqueClipCount"], args.expect_clips)

    available = {clip.casefold() for clip in report["clips"]}
    missing = [
        clip for clip in args.require_clip if clip.casefold() not in available
    ]
    if missing:
        raise ActionExtractError(f"required clips are absent: {missing}")

    atomic_write_json(args.output, report)

    print(json.dumps(summary, ensure_ascii=False, indent=1))
    print(f"written: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
