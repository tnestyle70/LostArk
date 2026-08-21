from __future__ import annotations

import argparse
import copy
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TARGET_PATH = (
    ROOT
    / "Data/Effects/Authored"
    / "effect.dimensionmaster.skill.2050180.unified.effect.json"
)

EFFECT_ASSET_ID = "effect.dimensionmaster.skill.2050180.unified"
SOURCE_HELIX_ID = "authored.source-particle.98639f5f2e65e0f0193c09fe"
HELIX_OCCURRENCES = (
    (SOURCE_HELIX_ID, 0.50),
    (f"authored.copy.{SOURCE_HELIX_ID}.1", 0.70),
    (f"authored.copy.{SOURCE_HELIX_ID}.2", 1.05),
)
HELIX_MODEL_ASSET_ID = (
    "Effect/DimensionMaster/Meshes/fm_d_helix_015_1.wmodel"
)
HELIX_SOURCE_LIFETIME_SCALE = 0.1


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load_document(path: Path) -> tuple[str, dict[str, Any]]:
    text = path.read_text(encoding="utf-8-sig")
    return text, json.loads(text)


def element_by_id(document: dict[str, Any], element_id: str) -> dict[str, Any]:
    matches = [row for row in document["elements"] if row.get("id") == element_id]
    require(len(matches) == 1, f"expected one Element: {element_id}")
    return matches[0]


def spawn_count_literal(element: dict[str, Any]) -> dict[str, Any]:
    spawn_modules = [
        row
        for row in element["sourceRecipe"].get("modules", [])
        if str(row.get("className", "")).casefold() == "particlemodulespawn"
    ]
    require(len(spawn_modules) == 1, "helix spawn module cardinality changed")
    literals = [
        row
        for row in spawn_modules[0].get("literals", [])
        if str(row.get("propertyPath", "")).casefold()
        == "burstlist[0].count"
    ]
    require(len(literals) == 1, "helix burst literal cardinality changed")
    return literals[0]


def mutate_occurrence(
    template: dict[str, Any], element_id: str, start_seconds: float
) -> dict[str, Any]:
    occurrence = copy.deepcopy(template)
    occurrence["id"] = element_id
    if element_id != SOURCE_HELIX_ID:
        occurrence["sourceNode"] = f"authored-copy:{SOURCE_HELIX_ID}"
        occurrence["sourcePresentation"] = {"enabled": False}

    occurrence["detail"]["timing"]["startDelaySeconds"] = start_seconds
    source_scale = occurrence["detail"]["particle"].setdefault(
        "sourceScale",
        {
            "count": 1,
            "size": 1,
            "lifeTime": 1,
            "speed": 1,
            "rotation": 1,
            "alpha": 1,
            "spawnDelay": 1,
        },
    )
    source_scale["lifeTime"] = HELIX_SOURCE_LIFETIME_SCALE
    bursts = occurrence["sourceRecipe"].get("bursts", [])
    require(
        len(bursts) == 1 and float(bursts[0].get("timeSeconds", -1)) == 0.0,
        "helix source burst contract changed",
    )
    bursts[0]["countMinimum"] = 1
    bursts[0]["countMaximum"] = 1
    spawn_count_literal(occurrence)["value"] = 1.0
    return occurrence


def validate_occurrence(element: dict[str, Any], start_seconds: float) -> None:
    resources = {
        str(row.get("slotId", "")): str(row.get("assetId", ""))
        for row in element.get("resources", [])
    }
    require(
        resources.get("meshModel") == HELIX_MODEL_ASSET_ID,
        "helix mesh identity changed",
    )
    require(
        abs(
            float(element["detail"]["timing"]["startDelaySeconds"])
            - start_seconds
        )
        < 1.0e-6,
        f"helix start time changed: {element['id']}",
    )
    require(
        abs(float(element["detail"]["timing"]["lifeTimeSeconds"]) - 0.1)
        < 1.0e-6,
        f"helix authored lifetime changed: {element['id']}",
    )
    source_scale = element["detail"]["particle"].get("sourceScale") or {}
    require(
        abs(float(source_scale.get("lifeTime", -1)) -
            HELIX_SOURCE_LIFETIME_SCALE) < 1.0e-6,
        f"helix source lifetime trim changed: {element['id']}",
    )
    require(
        element["sourceRecipe"].get("bursts")
        == [{"timeSeconds": 0, "countMinimum": 1, "countMaximum": 1}],
        f"helix burst changed: {element['id']}",
    )
    require(
        float(spawn_count_literal(element).get("value")) == 1.0,
        f"helix spawn literal changed: {element['id']}",
    )


def find_json_object_span(text: str, element_id: str) -> tuple[int, int]:
    marker = f'"id": "{element_id}"'
    marker_index = text.find(marker)
    require(marker_index >= 0, f"Element text marker is missing: {element_id}")
    start = text.rfind("{", 0, marker_index)
    require(start >= 0, f"Element object start is missing: {element_id}")

    depth = 0
    in_string = False
    escaped = False
    for index in range(start, len(text)):
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
        elif character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return start, index + 1
    raise RuntimeError(f"Element object end is missing: {element_id}")


def replace_one(pattern: str, replacement: str, text: str, message: str) -> str:
    replaced, count = re.subn(pattern, replacement, text, count=1)
    require(count == 1, message)
    return replaced


def patch_element_block(
    source_block: str, element_id: str, start_seconds: float
) -> str:
    block = replace_one(
        rf'"id"\s*:\s*"{re.escape(SOURCE_HELIX_ID)}"',
        f'"id": "{element_id}"',
        source_block,
        "helix Element ID token changed",
    )
    if element_id != SOURCE_HELIX_ID:
        block = replace_one(
            r'"sourceNode"\s*:\s*"[^"]*"',
            f'"sourceNode": "authored-copy:{SOURCE_HELIX_ID}"',
            block,
            "helix sourceNode token changed",
        )

    block = replace_one(
        r'("timing"\s*:\s*\{\s*"startDelaySeconds"\s*:\s*)'
        r'[-+0-9.eE]+',
        rf'\g<1>{start_seconds:g}',
        block,
        "helix start-delay token changed",
    )
    block = replace_one(
        r'("bursts"\s*:\s*\[\s*\{\s*"timeSeconds"\s*:\s*0(?:\.0)?\s*,'
        r'\s*"countMinimum"\s*:\s*)3(\s*,\s*"countMaximum"\s*:\s*)3',
        r'\g<1>1\g<2>1',
        block,
        "helix burst token changed",
    )
    block = replace_one(
        r'("propertyPath"\s*:\s*"burstlist\[0\]\.count"\s*,'
        r'\s*"kind"\s*:\s*"number"\s*,\s*"value"\s*:\s*)3(?:\.0)?',
        r'\g<1>1.0',
        block,
        "helix burst literal token changed",
    )
    return patch_source_lifetime_scale(block)


def patch_source_lifetime_scale(block: str) -> str:
    source_scale_match = re.search(
        r'"sourceScale"\s*:\s*\{[^{}]*\}', block
    )
    if source_scale_match is not None:
        source_scale = source_scale_match.group(0)
        source_scale = replace_one(
            r'("lifeTime"\s*:\s*)[-+0-9.eE]+',
            rf'\g<1>{HELIX_SOURCE_LIFETIME_SCALE:g}',
            source_scale,
            "helix source lifetime scale token changed",
        )
        return (
            block[: source_scale_match.start()]
            + source_scale
            + block[source_scale_match.end() :]
        )

    marker = '"billboard": false }'
    require(block.count(marker) == 1, "helix particle sourceScale seam changed")
    source_scale = (
        '"billboard": false, "sourceScale": { "count": 1, "size": 1, '
        f'"lifeTime": {HELIX_SOURCE_LIFETIME_SCALE:g}, "speed": 1, '
        '"rotation": 1, "alpha": 1, "spawnDelay": 1 } }'
    )
    return block.replace(marker, source_scale, 1)


def patch_preserving_layout(original: str) -> str:
    start, end = find_json_object_span(original, SOURCE_HELIX_ID)
    source_block = original[start:end]
    require(json.loads(source_block)["id"] == SOURCE_HELIX_ID, "wrong helix block")
    blocks = [
        patch_element_block(source_block, element_id, start_seconds)
        for element_id, start_seconds in HELIX_OCCURRENCES
    ]
    replacement = (",\n    ").join(blocks)
    return original[:start] + replacement + original[end:]


def patch_materialized_lifetime_scales(original: str) -> str:
    patched = original
    spans = [
        (*find_json_object_span(patched, element_id), element_id)
        for element_id, _ in HELIX_OCCURRENCES
    ]
    for start, end, _ in sorted(spans, reverse=True):
        block = patched[start:end]
        patched = (
            patched[:start]
            + patch_source_lifetime_scale(block)
            + patched[end:]
        )
    return patched


def validate_document(document: dict[str, Any]) -> None:
    require(
        document.get("effectAssetId") == EFFECT_ASSET_ID,
        "DimensionMaster R document identity changed",
    )
    for element_id, start_seconds in HELIX_OCCURRENCES:
        validate_occurrence(element_by_id(document, element_id), start_seconds)


def atomic_write(path: Path, text: str) -> None:
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary_path = Path(stream.name)
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
        json.loads(temporary_path.read_text(encoding="utf-8"))
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()

    original, document = load_document(TARGET_PATH)
    occurrence_ids = {element_id for element_id, _ in HELIX_OCCURRENCES}
    present_ids = occurrence_ids.intersection(
        {str(row.get("id", "")) for row in document.get("elements", [])}
    )

    if present_ids == occurrence_ids:
        staged = copy.deepcopy(document)
        for element_id, start_seconds in HELIX_OCCURRENCES:
            occurrence = element_by_id(staged, element_id)
            occurrence["detail"]["particle"].setdefault(
                "sourceScale",
                {
                    "count": 1,
                    "size": 1,
                    "lifeTime": 1,
                    "speed": 1,
                    "rotation": 1,
                    "alpha": 1,
                    "spawnDelay": 1,
                },
            )["lifeTime"] = HELIX_SOURCE_LIFETIME_SCALE
        patched = patch_materialized_lifetime_scales(original)
        parsed = json.loads(patched)
        validate_document(parsed)
        require(parsed == staged, "layout-preserving lifetime patch drifted")
    else:
        require(
            present_ids == {SOURCE_HELIX_ID},
            "helix occurrence set is partial; refusing to rewrite",
        )
        before = copy.deepcopy(document)
        template = element_by_id(document, SOURCE_HELIX_ID)
        template_index = document["elements"].index(template)
        staged = [
            mutate_occurrence(template, element_id, start_seconds)
            for element_id, start_seconds in HELIX_OCCURRENCES
        ]
        document["elements"][template_index : template_index + 1] = staged

        for old_element in before["elements"]:
            if old_element.get("id") == SOURCE_HELIX_ID:
                continue
            require(
                old_element == element_by_id(document, str(old_element["id"])),
                f"unrelated Element changed: {old_element['id']}",
            )

        patched = patch_preserving_layout(original)
        parsed = json.loads(patched)
        validate_document(parsed)
        require(parsed == document, "layout-preserving helix patch drifted")

    changed = patched != original
    if args.write and changed:
        atomic_write(TARGET_PATH, patched)

    print(
        json.dumps(
            {
                "status": (
                    "updated"
                    if args.write and changed
                    else "would-update"
                    if changed
                    else "stable"
                ),
                "document": TARGET_PATH.relative_to(ROOT).as_posix(),
                "occurrences": [
                    {"elementId": element_id, "startSeconds": start_seconds}
                    for element_id, start_seconds in HELIX_OCCURRENCES
                ],
                "burstCountPerOccurrence": 1,
            },
            ensure_ascii=False,
            indent=2,
        )
    )
    return 0 if args.write or not changed else 1


if __name__ == "__main__":
    raise SystemExit(main())
