#!/usr/bin/env python3
"""Materialize DimensionMaster A hit occurrences inside one authored document.

The Product animation must own one cast-start cue.  Repeating the whole Effect
document as four outer cues makes Tool Play All and gameplay observe different
owner-root histories.  This migration keeps the two one-shot swing-deco rows and
expands each admitted SwingHit row to the four source-proven occurrence poses.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import pathlib
import re
import sys


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
TARGET = REPOSITORY_ROOT / "Data/Effects/Authored/effect.dimensionmaster.skill.2050210.unified.effect.json"
SOURCE = REPOSITORY_ROOT / "Data/Effects/Authored/effect.dimensionmaster.skill.2050210.effect.json"
ANIMATION_EVENTS = REPOSITORY_ROOT / "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents"
TARGET_EFFECT_ID = "effect.dimensionmaster.skill.2050210.unified"
SOURCE_EFFECT_ID = "effect.dimensionmaster.skill.2050210"
SOURCE_NODE_PREFIX = (
    f"authored-source-particle:{TARGET_EFFECT_ID}|"
    f"source:{SOURCE_EFFECT_ID}.imported|element:"
)
SWING_HIT_PREFIX = "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
VORONOI_FAMILY = SWING_HIT_PREFIX + "particlespriteemitter_30"
SLASH_FAMILY = SWING_HIT_PREFIX + "particlespriteemitter_15"
TARGET_STARTS = (0.25, 0.60, 0.90, 1.30)


def occurrence_base(element_id: str) -> str:
    return re.sub(r"\.event_source-event-\d+$", "", element_id)


def format_number(value: float) -> str:
    if abs(value) < 0.0000000005:
        return "0"
    return format(value, ".9g")


def variant_id(current_id: str, source_id: str, keep_current: bool) -> str:
    if keep_current:
        return current_id
    digest = hashlib.sha256(f"{current_id}|{source_id}".encode("utf-8")).hexdigest()[:24]
    return f"authored.source-particle.{digest}"


def validate_inputs(target: dict, source: dict) -> None:
    if target.get("effectAssetId") != TARGET_EFFECT_ID:
        raise RuntimeError(
            f"target Effect identity changed: {target.get('effectAssetId')!r}"
        )
    if source.get("effectAssetId") != SOURCE_EFFECT_ID:
        raise RuntimeError(
            f"source Effect identity changed: {source.get('effectAssetId')!r}"
        )
    if not isinstance(target.get("elements"), list) or not isinstance(
        source.get("elements"), list
    ):
        raise RuntimeError("target/source elements arrays are missing")

    source_ids = {element.get("id") for element in source["elements"]}
    referenced_families: set[str] = set()
    for element in target["elements"]:
        source_node = element.get("sourceNode", "")
        if SWING_HIT_PREFIX not in source_node:
            continue
        if not source_node.startswith(SOURCE_NODE_PREFIX):
            raise RuntimeError(
                f"{element.get('id')}: non-canonical SwingHit sourceNode"
            )
        source_id = source_node[len(SOURCE_NODE_PREFIX):]
        if source_id not in source_ids or not source_id.startswith(SWING_HIT_PREFIX):
            raise RuntimeError(
                f"{element.get('id')}: sourceNode does not name the canonical source"
            )
        referenced_families.add(occurrence_base(source_id))

    if len(referenced_families) != 8:
        raise RuntimeError(
            f"expected eight referenced SwingHit families, found {len(referenced_families)}"
        )
    for family_id in referenced_families:
        family = [
            element for element in source["elements"]
            if occurrence_base(element.get("id", "")) == family_id
        ]
        starts = tuple(sorted(
            element["detail"]["timing"]["startDelaySeconds"]
            for element in family
        ))
        if len(family) != 4 or starts != TARGET_STARTS:
            raise RuntimeError(
                f"{family_id}: source occurrence timing changed: {starts}"
            )


def build_document(target: dict, source: dict) -> dict:
    validate_inputs(target, source)
    if len(target.get("elements", [])) == 34:
        source_order: dict[str, int] = {}
        for element in source["elements"]:
            source_id = element["id"]
            if not source_id.startswith(SWING_HIT_PREFIX):
                continue
            base = occurrence_base(source_id)
            family = [
                row for row in source["elements"]
                if occurrence_base(row["id"]) == base
            ]
            family.sort(key=lambda row: row["detail"]["timing"]["startDelaySeconds"])
            if len(family) != 4:
                raise RuntimeError(f"{base}: expected four exact source occurrences")
            source_order.update({row["id"]: index for index, row in enumerate(family)})
        for element in target["elements"]:
            source_node = element.get("sourceNode", "")
            marker = "|element:"
            source_id = source_node.split(marker, 1)[1] if marker in source_node else ""
            if source_id in source_order:
                element["detail"]["timing"]["startDelaySeconds"] = \
                    TARGET_STARTS[source_order[source_id]]
        validate_result(target)
        return target
    source_by_base: dict[str, list[dict]] = {}
    for element in source["elements"]:
        element_id = element["id"]
        if not element_id.startswith(SWING_HIT_PREFIX):
            continue
        source_by_base.setdefault(occurrence_base(element_id), []).append(element)

    output: list[dict] = []
    expanded = 0
    for current in target["elements"]:
        source_node = current.get("sourceNode", "")
        marker = "|element:"
        source_id = source_node.split(marker, 1)[1] if marker in source_node else ""
        base = occurrence_base(source_id)
        if not base.startswith(SWING_HIT_PREFIX):
            output.append(current)
            continue

        occurrences = source_by_base.get(base, [])
        occurrences.sort(key=lambda item: item["detail"]["timing"]["startDelaySeconds"])
        if len(occurrences) != 4:
            raise RuntimeError(f"{base}: expected four exact source occurrences, found {len(occurrences)}")

        reference = next((item for item in occurrences if item["id"] == source_id), None)
        if reference is None:
            raise RuntimeError(f"{current['id']}: sourceNode does not name one preserved occurrence")
        current_position = current["detail"]["transform"]["position"]
        reference_position = reference["detail"]["transform"]["position"]
        position_delta = [current_position[i] - reference_position[i] for i in range(3)]

        for index, occurrence in enumerate(occurrences):
            clone = copy.deepcopy(current)
            clone["id"] = variant_id(current["id"], occurrence["id"], occurrence is reference)
            clone["sourceNode"] = source_node.split(marker, 1)[0] + marker + occurrence["id"]
            clone["detail"]["timing"]["startDelaySeconds"] = TARGET_STARTS[index]
            source_position = occurrence["detail"]["transform"]["position"]
            clone["detail"]["transform"]["position"] = [
                source_position[i] + position_delta[i] for i in range(3)
            ]
            output.append(clone)
        expanded += 1

    if expanded != 8 or len(target["elements"]) != 10 or len(output) != 34:
        raise RuntimeError(
            f"unexpected A denominator: source rows={len(target['elements'])}, "
            f"expanded families={expanded}, output rows={len(output)}"
        )
    target["elements"] = output
    return target


def object_spans(array_text: str) -> list[tuple[int, int]]:
    spans: list[tuple[int, int]] = []
    depth = 0
    start = -1
    in_string = False
    escaped = False
    for index, character in enumerate(array_text):
        if in_string:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
            continue
        if character == '"':
            in_string = True
        elif character == "{":
            if depth == 0:
                start = index
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                spans.append((start, index + 1))
    if depth != 0 or in_string:
        raise RuntimeError("elements array has an unterminated object or string")
    return spans


def elements_array_bounds(text: str) -> tuple[int, int]:
    match = re.search(r'"elements"\s*:\s*\[', text)
    if match is None:
        raise RuntimeError("elements array is missing")
    opening = text.find("[", match.start())
    depth = 0
    in_string = False
    escaped = False
    for index in range(opening, len(text)):
        character = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
            continue
        if character == '"':
            in_string = True
        elif character == "[":
            depth += 1
        elif character == "]":
            depth -= 1
            if depth == 0:
                return opening + 1, index
    raise RuntimeError("elements array is unterminated")


def replace_one(block: str, pattern: str, replacement: str, label: str) -> str:
    result, count = re.subn(pattern, replacement, block, count=1)
    if count != 1:
        raise RuntimeError(f"could not replace exactly one {label}")
    return result


def render_variant(block: str, current: dict, variant: dict) -> str:
    result = replace_one(
        block,
        rf'"id"\s*:\s*"{re.escape(current["id"])}"',
        f'"id": "{variant["id"]}"',
        "element id",
    )
    result = replace_one(
        result,
        rf'"sourceNode"\s*:\s*"{re.escape(current["sourceNode"])}"',
        f'"sourceNode": "{variant["sourceNode"]}"',
        "sourceNode",
    )
    position = ", ".join(
        format_number(value) for value in variant["detail"]["transform"]["position"]
    )
    result = replace_one(
        result,
        r'("detail"\s*:\s*\{\s*"transform"\s*:\s*\{\s*"position"\s*:\s*)\[[^\]]+\]',
        rf'\g<1>[{position}]',
        "detail transform position",
    )
    start = format_number(variant["detail"]["timing"]["startDelaySeconds"])
    result = replace_one(
        result,
        r'("timing"\s*:\s*\{\s*"startDelaySeconds"\s*:\s*)[-+0-9.eE]+',
        rf'\g<1>{start}',
        "detail timing start",
    )
    return result


def layout_preserving_text(original_text: str, current: dict, result: dict) -> str:
    if (len(current["elements"]), len(result["elements"])) not in ((10, 34), (34, 34)):
        raise RuntimeError("layout-preserving migration accepts only 10-to-34 or 34-to-34")
    array_start, array_end = elements_array_bounds(original_text)
    array_text = original_text[array_start:array_end]
    spans = object_spans(array_text)
    if len(spans) != len(current["elements"]):
        raise RuntimeError(
            f"expected {len(current['elements'])} physical element blocks, found {len(spans)}"
        )

    if len(current["elements"]) == 34:
        rendered = [
            render_variant(array_text[begin:end], current_element, result_element)
            for current_element, result_element, (begin, end) in
            zip(current["elements"], result["elements"], spans)
        ]
        leading = array_text[:spans[0][0]]
        trailing = array_text[spans[-1][1]:]
        migrated_array = leading + ",\n    ".join(rendered) + trailing
        return original_text[:array_start] + migrated_array + original_text[array_end:]

    result_by_source: dict[str, list[dict]] = {}
    for element in result["elements"]:
        source_node = element.get("sourceNode", "")
        marker = "|element:"
        source_id = source_node.split(marker, 1)[1] if marker in source_node else ""
        result_by_source.setdefault(occurrence_base(source_id), []).append(element)

    rendered_existing: list[str] = []
    rendered_added: list[str] = []
    for current_element, (begin, end) in zip(current["elements"], spans):
        block = array_text[begin:end]
        source_node = current_element.get("sourceNode", "")
        marker = "|element:"
        source_id = source_node.split(marker, 1)[1] if marker in source_node else ""
        base = occurrence_base(source_id)
        variants = result_by_source.get(base, [current_element])
        variants.sort(key=lambda item: item["detail"]["timing"]["startDelaySeconds"])
        reference = next(
            (variant for variant in variants if variant["id"] == current_element["id"]),
            None,
        )
        if reference is None:
            raise RuntimeError(f"{current_element['id']}: stable reference variant is missing")
        rendered_existing.append(render_variant(block, current_element, reference))
        rendered_added.extend(
            render_variant(block, current_element, variant)
            for variant in variants
            if variant is not reference
        )

    leading = array_text[:spans[0][0]]
    trailing = array_text[spans[-1][1]:]
    migrated_array = leading + ",\n    ".join(rendered_existing + rendered_added) + trailing
    return original_text[:array_start] + migrated_array + original_text[array_end:]


def validate_result(document: dict) -> None:
    ids = [element["id"] for element in document["elements"]]
    if len(ids) != len(set(ids)):
        raise RuntimeError("materialized occurrence IDs are not unique")
    swing_rows = [
        element for element in document["elements"]
        if SWING_HIT_PREFIX in element.get("sourceNode", "")
    ]
    if len(swing_rows) != 32:
        raise RuntimeError(f"expected 32 SwingHit rows, found {len(swing_rows)}")
    for base in {occurrence_base(row["sourceNode"].split("|element:", 1)[1]) for row in swing_rows}:
        family = [
            row for row in swing_rows
            if occurrence_base(row["sourceNode"].split("|element:", 1)[1]) == base
        ]
        starts = sorted(row["detail"]["timing"]["startDelaySeconds"] for row in family)
        if starts != list(TARGET_STARTS):
            raise RuntimeError(f"{base}: materialized timing changed: {starts}")

    by_family: dict[str, list[dict]] = {}
    for row in swing_rows:
        source_id = row["sourceNode"].split("|element:", 1)[1]
        by_family.setdefault(occurrence_base(source_id), []).append(row)
    voronoi = sorted(
        by_family[VORONOI_FAMILY],
        key=lambda row: row["detail"]["timing"]["startDelaySeconds"],
    )
    slash = sorted(
        by_family[SLASH_FAMILY],
        key=lambda row: row["detail"]["timing"]["startDelaySeconds"],
    )
    expected_delta = None
    for voronoi_row, slash_row in zip(voronoi, slash):
        if voronoi_row["detail"]["timing"]["startDelaySeconds"] != slash_row["detail"]["timing"]["startDelaySeconds"]:
            raise RuntimeError("voronoi/slash paired occurrence times diverged")
        delta = tuple(
            round(voronoi_row["detail"]["transform"]["position"][index]
                  - slash_row["detail"]["transform"]["position"][index], 7)
            for index in range(3)
        )
        if expected_delta is None:
            expected_delta = delta
        elif delta != expected_delta:
            raise RuntimeError(
                f"voronoi/slash local-space pairing changed: {delta} != {expected_delta}"
            )


def validate_animation_event_contract() -> None:
    lines = ANIMATION_EVENTS.read_text(encoding="utf-8-sig").splitlines()
    if not lines:
        raise RuntimeError("DimensionMaster animevents is empty")
    header = re.fullmatch(r'LOSTARK_ANIM_EVENTS\s+\d+\s+"DimensionMaster"\s+(\d+)', lines[0])
    if header is None:
        raise RuntimeError("DimensionMaster animevents header is malformed")
    if int(header.group(1)) != len(lines) - 1:
        raise RuntimeError(
            f"DimensionMaster animevents count changed: header={header.group(1)}, rows={len(lines) - 1}"
        )
    cues = [
        line for line in lines[1:]
        if line.startswith('"pc_sp_m_00_sk_sk_willowrend" EFFECT ')
        and 'payload="effect.dimensionmaster.skill.2050210.unified"' in line
    ]
    if len(cues) != 1:
        raise RuntimeError(f"A must have one Product document cue, found {len(cues)}")
    cue = cues[0]
    for token in ("startms=0", 'anchor="root"', "follow=snapshot", "stop=natural"):
        if token not in cue:
            raise RuntimeError(f"A Product document cue is missing {token}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="replace the target atomically")
    args = parser.parse_args()

    original_text = TARGET.read_text(encoding="utf-8-sig")
    target = json.loads(original_text)
    source = json.loads(SOURCE.read_text(encoding="utf-8-sig"))
    original = copy.deepcopy(target)
    result = build_document(target, source)
    validate_result(result)
    validate_animation_event_contract()
    serialized = layout_preserving_text(original_text, original, result)
    validate_result(json.loads(serialized))
    if args.write:
        if serialized == original_text:
            print(f"already materialized {len(result['elements'])} rows: {TARGET}")
            return 0
        temporary = TARGET.with_suffix(TARGET.suffix + ".tmp")
        temporary.write_text(serialized, encoding="utf-8", newline="\n")
        temporary.replace(TARGET)
        print(f"materialized {len(result['elements'])} rows: {TARGET}")
    else:
        if serialized != original_text:
            raise RuntimeError("target requires layout-preserving --write materialization")
        print("check passed: 2 one-shot rows + 8 x 4 explicit SwingHit occurrences")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, RuntimeError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
