#!/usr/bin/env python3
"""Apply the Artist A slash-only SpriteWave noise semantic override.

The source ``noise`` texture remains bound.  Clearing that slot in the Effect
Tool means Reset to Source, so the authored product fix is the scalar contract
``uv_noise_velue = 0`` on exactly the eight slash occurrences.  The script is
layout preserving, idempotent, and refuses partial or unknown target states.
"""

from __future__ import annotations

import argparse
import codecs
import json
import os
import pathlib
import re
import tempfile
from typing import Any


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
TARGET_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31460.unified.effect.json"
)
TARGET_EFFECT_ID = "effect.artist.skill.31460.unified"
TARGET_ELEMENT_IDS = (
    "authored.source-particle.cb346af47371feedccf9b652",
    "authored.source-particle.32a4871cc934460404365309",
    "authored.source-particle.ab175ceab43d84d23b8a9efc",
    "authored.source-particle.a6b259e86343eae97f48c142",
    "authored.source-particle.9d090bbe905a7daa6215e39c",
    "authored.source-particle.64f6d0ab29d071e1a8d41dcb",
    "authored.source-particle.8c510113f8256fd62b31de3b",
    "authored.source-particle.4986a748ac0894912a52cc89",
)
PARAMETER_NAME = "uv_noise_velue"
COMPILER_VALUE = -1.10000002
AUTHORED_VALUE = 0.0
EXPECTED_MATERIAL_PATH = "fx_m_mi_o_00.fx_mi.fx_o_pa_spritewave_01_27_tr"
EXPECTED_PROFILE_ID = (
    "ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92"
)
EXPECTED_BINDINGS = {
    "base": "Effect/Artist/Textures/fx_m_trail_004_cl.dds",
    "dissolve": "Effect/Artist/Textures/fx_k_auraline_05_ycl.dds",
    "noise": "Effect/Artist/Textures/fx_a_cloud_022.dds",
}


class ArtistSlashOverrideError(RuntimeError):
    """Raised when the source-owned slash contract has drifted."""


def _load_document(text: str, label: str) -> dict[str, Any]:
    try:
        value = json.loads(text)
    except json.JSONDecodeError as error:
        raise ArtistSlashOverrideError(f"invalid JSON in {label}: {error}") from error
    if not isinstance(value, dict):
        raise ArtistSlashOverrideError(f"{label} root must be an object")
    return value


def _matching_brace(text: str, opening: int) -> int:
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
        elif character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return index + 1
    raise ArtistSlashOverrideError("unterminated element object")


def _element_spans(text: str) -> list[tuple[int, int, dict[str, Any]]]:
    marker = re.search(r'"elements"\s*:\s*\[', text)
    if marker is None:
        raise ArtistSlashOverrideError("elements array is missing")
    cursor = marker.end()
    spans: list[tuple[int, int, dict[str, Any]]] = []
    while cursor < len(text):
        while cursor < len(text) and text[cursor] in " \t\r\n,":
            cursor += 1
        if cursor >= len(text) or text[cursor] == "]":
            break
        if text[cursor] != "{":
            raise ArtistSlashOverrideError("elements array layout changed")
        end = _matching_brace(text, cursor)
        value = _load_document(text[cursor:end], f"element at {cursor}")
        spans.append((cursor, end, value))
        cursor = end
    return spans


def _scalar_rows(element: dict[str, Any]) -> list[dict[str, Any]]:
    try:
        rows = element["material"]["sourceProfile"]["scalars"]
    except (KeyError, TypeError) as error:
        raise ArtistSlashOverrideError(
            f"{element.get('id')} source scalar table is missing"
        ) from error
    return [row for row in rows if row.get("name") == PARAMETER_NAME]


def _validate_target(element: dict[str, Any]) -> bool:
    element_id = element.get("id")
    material = element.get("material", {})
    source_profile = material.get("sourceProfile", {})
    bindings = {
        row.get("slotId"): row.get("assetId")
        for row in element.get("resources", [])
        if isinstance(row, dict)
    }
    if (
        element.get("kind") != "particle"
        or material.get("sourceMaterialPath") != EXPECTED_MATERIAL_PATH
        or source_profile.get("profileId") != EXPECTED_PROFILE_ID
        or any(bindings.get(slot) != asset for slot, asset in EXPECTED_BINDINGS.items())
    ):
        raise ArtistSlashOverrideError(
            f"{element_id} SpriteWave material/resource identity changed"
        )
    scalars = _scalar_rows(element)
    if len(scalars) != 1 or scalars[0].get("group") != "uv_noise":
        raise ArtistSlashOverrideError(
            f"{element_id} must expose one uv_noise_velue scalar"
        )
    value = scalars[0].get("value")
    overrides = element.get("authoringOverrides")
    if overrides is None:
        if value != COMPILER_VALUE:
            raise ArtistSlashOverrideError(
                f"{element_id} compiler scalar changed without authored metadata"
            )
        return False
    if not isinstance(overrides, dict):
        raise ArtistSlashOverrideError(f"{element_id} override object is invalid")
    scalar_overrides = overrides.get("scalars")
    expected_override = {
        "name": PARAMETER_NAME,
        "value": AUTHORED_VALUE,
        "compilerValue": COMPILER_VALUE,
    }
    if (
        overrides.get("resources") != []
        or overrides.get("colors") != []
        or scalar_overrides != [expected_override]
        or value != AUTHORED_VALUE
    ):
        raise ArtistSlashOverrideError(
            f"{element_id} has a partial or foreign authoring override"
        )
    return True


def _render_target(block: str, element: dict[str, Any], newline: str) -> str:
    already_applied = _validate_target(element)
    if already_applied:
        return block
    scalar_pattern = re.compile(
        r'("name"\s*:\s*"uv_noise_velue"\s*,\s*'
        r'"group"\s*:\s*"uv_noise"\s*,\s*"value"\s*:\s*)'
        r'-1\.10000002'
    )
    block, scalar_count = scalar_pattern.subn(r"\g<1>0", block)
    if scalar_count != 1:
        raise ArtistSlashOverrideError(
            f"{element.get('id')} scalar layout changed"
        )
    terminal_pattern = re.compile(
        r'(      "sourcePresentation"\s*:\s*\{\s*'
        r'"enabled"\s*:\s*false\s*\})(\s*\})\s*$'
    )
    override_lines = (
        ',{nl}      "authoringOverrides": {{ "resources": [], '
        '"scalars": [ {{ "name": "uv_noise_velue", "value": 0, '
        '"compilerValue": -1.10000002 }} ], "colors": [] }}'
    ).format(nl=newline)
    block, terminal_count = terminal_pattern.subn(
        lambda match: match.group(1) + override_lines + match.group(2), block
    )
    if terminal_count != 1:
        raise ArtistSlashOverrideError(
            f"{element.get('id')} sourcePresentation terminal layout changed"
        )
    staged = _load_document(block, str(element.get("id")))
    if not _validate_target(staged):
        raise ArtistSlashOverrideError(
            f"{element.get('id')} override did not reach the admitted state"
        )
    return block


def build_text(original_text: str) -> str:
    document = _load_document(original_text, "Artist A target")
    if document.get("effectAssetId") != TARGET_EFFECT_ID:
        raise ArtistSlashOverrideError("Artist A effect identity changed")
    spans = _element_spans(original_text)
    indexed: dict[str, tuple[int, int, dict[str, Any]]] = {}
    for span in spans:
        element_id = span[2].get("id")
        if not isinstance(element_id, str) or element_id in indexed:
            raise ArtistSlashOverrideError("element ID is missing or duplicated")
        indexed[element_id] = span
    if any(element_id not in indexed for element_id in TARGET_ELEMENT_IDS):
        raise ArtistSlashOverrideError("one or more Artist A slash IDs are missing")
    newline = "\r\n" if "\r\n" in original_text else "\n"
    replacements: list[tuple[int, int, str]] = []
    for element_id in TARGET_ELEMENT_IDS:
        start, end, element = indexed[element_id]
        block = original_text[start:end]
        rendered = _render_target(block, element, newline)
        replacements.append((start, end, rendered))
    result = original_text
    for start, end, rendered in sorted(replacements, reverse=True):
        result = result[:start] + rendered + result[end:]
    staged = _load_document(result, "staged Artist A target")
    target_rows = {
        row.get("id"): row
        for row in staged.get("elements", [])
        if row.get("id") in TARGET_ELEMENT_IDS
    }
    if tuple(target_rows) != TARGET_ELEMENT_IDS:
        raise ArtistSlashOverrideError("staged target order/cardinality changed")
    if not all(_validate_target(target_rows[element_id]) for element_id in TARGET_ELEMENT_IDS):
        raise ArtistSlashOverrideError("staged target is not fully overridden")
    return result


def _atomic_replace(path: pathlib.Path, text: str, *, bom: bool) -> None:
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    temporary = pathlib.Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            if bom:
                stream.write(codecs.BOM_UTF8)
            stream.write(text.encode("utf-8"))
            stream.flush()
            os.fsync(stream.fileno())
        staged = temporary.read_text(encoding="utf-8-sig")
        if build_text(staged) != staged:
            raise ArtistSlashOverrideError("temporary round-trip is not idempotent")
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def run(*, write: bool, target_path: pathlib.Path = TARGET_PATH) -> bool:
    raw = target_path.read_bytes()
    bom = raw.startswith(codecs.BOM_UTF8)
    original = raw.decode("utf-8-sig")
    rendered = build_text(original)
    changed = rendered != original
    if changed and not write:
        raise ArtistSlashOverrideError(
            "Artist A slash overrides are missing; rerun with --write"
        )
    if changed:
        _atomic_replace(target_path, rendered, bom=bom)
    return changed


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Apply Artist 31460 slash-only SpriteWave noise overrides"
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    try:
        changed = run(write=arguments.write)
    except (ArtistSlashOverrideError, OSError) as error:
        print(f"ERROR: {error}")
        return 1
    print(
        ("updated" if changed else "check passed")
        + ": Artist A slash noise influence is 0 on exactly 8 rows"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
