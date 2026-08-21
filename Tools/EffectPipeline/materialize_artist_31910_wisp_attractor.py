#!/usr/bin/env python3
"""Restore Artist V's reviewed wisp cohort and add one explicit attractor."""

from __future__ import annotations

import argparse
import codecs
import copy
import hashlib
import json
import os
import pathlib
import tempfile
from typing import Any


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
DOCUMENT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31910.unified.effect.json"
)
DONOR_PATH = REPOSITORY_ROOT / (
    "Client/Bin/DataFiles/Effect/Authored/"
    "effect.artist.skill.31910.unified."
    "3fa387259fabe72d6855c02239850784f1ce4cabb2ff4bc4b4d34382a5449bee."
    "effect.json"
)
ROLE_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Artist/"
    "Artist.e-r-d-s-t-v-z.composition.json"
)
TRACK_A_RECEIPT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-restoration-receipt.json"
)
MATERIALIZATION_RECEIPT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Artist/"
    "effect.artist.skill.31910.wisp-attractor.materialization.receipt.json"
)

EFFECT_ID = "effect.artist.skill.31910.unified"
BASE_ROW_COUNT = 41
FINAL_ROW_COUNT = 46
BASE_ROWS_SHA256 = (
    "17cc6e00ac7fdf21ecc57f8d60683c5d8a92f38f74e43d08af687bf73bd969c8"
)
FINAL_ROWS_SHA256 = (
    "dd2bcf68364ceafeabf6e789b3e533c044f4ee07697844bd70170683cdbe1693"
)
DONOR_RAW_SHA256 = (
    "3fa387259fabe72d6855c02239850784f1ce4cabb2ff4bc4b4d34382a5449bee"
)

RESTORED_IDS = (
    "authored.source-particle.36087cc48e5c8db080571686",
    "authored.source-particle.c63d1b07360842840df9db42",
    "authored.source-particle.61ecad590318a01a22324657",
    "authored.source-particle.b637bf78a795c9584bfe596a",
    "authored.source-particle.8538a8b05418b8e42b779058",
)
EXISTING_COHORT_IDS = (
    "authored.source-particle.f81b385edfd495c8f9b0ee62",
    "authored.source-particle.f2a563a924c233629be7a601",
)
FULL_COHORT_IDS = RESTORED_IDS + EXISTING_COHORT_IDS
ATTRACTOR_ID = "authored.source-particle.b637bf78a795c9584bfe596a"

DONOR_CANONICAL_SHA256 = {
    RESTORED_IDS[0]: "8e43e8657bb1dc1b7a2a69b1cbbc326d1e4b035aa8228ac826d53a3be684b778",
    RESTORED_IDS[1]: "23a4bea25261c8a63a5eba109265e549170cb20bae704f0822ae98b52cbe492b",
    RESTORED_IDS[2]: "174399f2671a85f32600c35854d87abadb7a0c91f2c6f977f2b0d5ce175df45c",
    RESTORED_IDS[3]: "250a1573d5d83a370e7b2282ec27a7be1bd1a7d48cfb0ff4aea876d6d0c9e338",
    RESTORED_IDS[4]: "cd7846b8bcf85578b4826c6b22846e6ccd8aebc8f07142809857a6cf97feebcc",
}
TRACK_A_IDENTITIES = {
    RESTORED_IDS[0]: (
        "fx_bs_04.state.par_j_exmove_01.particlespriteemitter_8",
        1,
        "b61a115f1686c602e9413b1a344882b12f033ee920904469e0573a1526887ae9",
    ),
    RESTORED_IDS[1]: (
        "fx_bs_04.state.par_j_exmove_01.particlespriteemitter_12",
        2,
        "43f5fb12774b42519edc891b709f90ce2893926541731175617eb3a88c212970",
    ),
    RESTORED_IDS[2]: (
        "fx_bs_04.state.par_j_exmove_01.particlespriteemitter_1",
        3,
        "53e0afe5b47c6cdab2019669bd0300e7bb25436019d37c3e458ec71ffde9406e",
    ),
    RESTORED_IDS[3]: (
        "fx_bs_04.state.par_j_exmove_01.particlespriteemitter_17",
        5,
        "ba3f716296f85c6a4c262211354ffe30bb32e702a29d47e865d4c855fb8b4033",
    ),
    RESTORED_IDS[4]: (
        "fx_bs_04.state.par_j_exmove_01.particlespriteemitter_15",
        6,
        "7f738bd4aed7a975a617c02cb020b78e92eac771f2b062ff80afa0a28e1653bc",
    ),
}

ATTRACTOR_CONTRACT = {
    "enabled": True,
    "targetSpace": "rootLocal",
    "targetOffset": [0, 0, 0],
    "activeNormalized": [0, 1],
    "radialAcceleration": 120,
    "tangentialAcceleration": 4,
    "maximumSpeed": 12,
    "convergenceRadius": 0.025,
    "arrivalDamping": 8,
}


class ArtistWispAttractorError(RuntimeError):
    """Raised when the reviewed Artist V source or authored subset drifts."""


def _canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _load(path: pathlib.Path) -> tuple[bytes, str, dict[str, Any]]:
    raw = path.read_bytes()
    text = raw.decode("utf-8-sig")
    try:
        document = json.loads(text)
    except json.JSONDecodeError as error:
        raise ArtistWispAttractorError(f"invalid JSON in {path}: {error}") from error
    if not isinstance(document, dict):
        raise ArtistWispAttractorError(f"{path} root must be an object")
    return raw, text, document


def _indexed_rows(document: dict[str, Any], *, label: str) -> dict[str, dict]:
    elements = document.get("elements")
    if not isinstance(elements, list):
        raise ArtistWispAttractorError(f"{label} elements are missing")
    indexed: dict[str, dict] = {}
    for row in elements:
        if not isinstance(row, dict) or not isinstance(row.get("id"), str):
            raise ArtistWispAttractorError(f"{label} has an invalid element")
        if row["id"] in indexed:
            raise ArtistWispAttractorError(f"{label} duplicates {row['id']}")
        indexed[row["id"]] = row
    return indexed


def validate_evidence(
    *,
    donor_path: pathlib.Path = DONOR_PATH,
    role_path: pathlib.Path = ROLE_PATH,
    track_a_path: pathlib.Path = TRACK_A_RECEIPT_PATH,
) -> list[dict[str, Any]]:
    donor_raw, _, donor = _load(donor_path)
    if hashlib.sha256(donor_raw).hexdigest() != DONOR_RAW_SHA256:
        raise ArtistWispAttractorError("Artist V sealed donor identity changed")
    if donor.get("effectAssetId") != EFFECT_ID:
        raise ArtistWispAttractorError("Artist V sealed donor effect changed")
    donor_rows = _indexed_rows(donor, label="Artist V sealed donor")
    selected: list[dict[str, Any]] = []
    for element_id in RESTORED_IDS:
        row = donor_rows.get(element_id)
        if row is None or _canonical_sha256(row) != DONOR_CANONICAL_SHA256[element_id]:
            raise ArtistWispAttractorError(
                f"Artist V sealed donor row changed: {element_id}"
            )
        selected.append(copy.deepcopy(row))

    _, _, roles = _load(role_path)
    systems = roles.get("skills")
    role_ids: list[str] | None = None
    if isinstance(systems, list):
        for system in systems:
            if not isinstance(system, dict) or system.get("skillId") != 31910:
                continue
            for role in system.get("roles", []) if isinstance(system, dict) else []:
                if isinstance(role, dict) and role.get("role") == "PALE_YELLOW_RED_ORBIT":
                    role_ids = role.get("stableIds")
    if (
        not isinstance(role_ids, list)
        or len(role_ids) != len(FULL_COHORT_IDS)
        or len(set(role_ids)) != len(role_ids)
        or set(role_ids) != set(FULL_COHORT_IDS)
    ):
        raise ArtistWispAttractorError("Artist V reviewed wisp role identity changed")

    _, _, track_a = _load(track_a_path)
    matches: dict[str, dict] = {}

    def visit(value: Any) -> None:
        if isinstance(value, dict):
            element_id = value.get("targetElementId")
            if element_id in TRACK_A_IDENTITIES:
                if element_id in matches:
                    raise ArtistWispAttractorError(
                        f"Track A duplicates Artist V evidence: {element_id}"
                    )
                matches[element_id] = value
            for child in value.values():
                visit(child)
        elif isinstance(value, list):
            for child in value:
                visit(child)

    visit(track_a)
    for element_id, expected in TRACK_A_IDENTITIES.items():
        row = matches.get(element_id)
        if row is None or (
            row.get("sourceElementId"),
            row.get("sourceOrder"),
            row.get("sourceRecipeCanonicalSha256"),
        ) != expected:
            raise ArtistWispAttractorError(
                f"Track A Artist V evidence changed: {element_id}"
            )

    selected[RESTORED_IDS.index(ATTRACTOR_ID)]["detail"]["particle"][
        "targetAttractor"
    ] = copy.deepcopy(ATTRACTOR_CONTRACT)
    return selected


def _elements_array_bounds(text: str) -> tuple[int, int]:
    marker = text.find('"elements"')
    opening = text.find("[", marker)
    if marker < 0 or opening < 0:
        raise ArtistWispAttractorError("Artist V elements array is missing")
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
    raise ArtistWispAttractorError("Artist V elements array is unterminated")


def validate_document(
    document: dict[str, Any], restored_rows: list[dict[str, Any]]
) -> bool:
    if document.get("effectAssetId") != EFFECT_ID or document.get("version") != 13:
        raise ArtistWispAttractorError("Artist V document identity changed")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) not in (
        BASE_ROW_COUNT,
        FINAL_ROW_COUNT,
    ):
        raise ArtistWispAttractorError("Artist V row count is outside 41/46")
    ids = [row.get("id") if isinstance(row, dict) else None for row in elements]
    if len(set(ids)) != len(ids):
        raise ArtistWispAttractorError("Artist V element IDs are duplicated")

    if len(elements) == BASE_ROW_COUNT:
        if any(element_id in ids for element_id in RESTORED_IDS):
            raise ArtistWispAttractorError("Artist V missing cohort is partially inserted")
        if ids[:2] != list(EXISTING_COHORT_IDS):
            raise ArtistWispAttractorError("Artist V retained wisp rows moved")
        if _canonical_sha256(elements) != BASE_ROWS_SHA256:
            raise ArtistWispAttractorError("Artist V current 41-row tuning changed")
        return False

    if elements[: len(RESTORED_IDS)] != restored_rows:
        raise ArtistWispAttractorError("Artist V restored wisp rows changed or moved")
    retained = elements[len(RESTORED_IDS) :]
    if _canonical_sha256(retained) != BASE_ROWS_SHA256:
        raise ArtistWispAttractorError("Artist V retained 41 rows changed")
    if [row.get("id") for row in elements[:7]] != list(FULL_COHORT_IDS):
        raise ArtistWispAttractorError("Artist V seven-row wisp cohort order changed")
    attractors = [
        row
        for row in elements
        if row.get("detail", {}).get("particle", {}).get("targetAttractor")
    ]
    if len(attractors) != 1 or attractors[0].get("id") != ATTRACTOR_ID:
        raise ArtistWispAttractorError("Artist V attractor ownership changed")
    return True


def validate_materialization_receipt(
    path: pathlib.Path = MATERIALIZATION_RECEIPT_PATH,
) -> None:
    _, _, receipt = _load(path)
    expected = {
        "schema": "lostark.effect-artist-wisp-attractor-materialization-receipt",
        "formatVersion": 1,
        "effectAssetId": EFFECT_ID,
        "baselineRowCount": BASE_ROW_COUNT,
        "finalRowCount": FINAL_ROW_COUNT,
        "preservedBaselineRowsCanonicalSha256": BASE_ROWS_SHA256,
        "outputRowsCanonicalSha256": FINAL_ROWS_SHA256,
        "sealedDonorRawSha256": DONOR_RAW_SHA256,
        "restoredStableIds": list(RESTORED_IDS),
        "reviewedCohortStableIds": list(FULL_COHORT_IDS),
        "attractorStableId": ATTRACTOR_ID,
        "attractorContract": ATTRACTOR_CONTRACT,
        "restoredOccurrenceProvenance": "SOURCE_EVIDENCE_DONOR_TRANSPLANT",
        "attractorProvenance": "PROJECT_TUNED",
        "runtimeAdmission": "AUTHORED_NOT_PUBLISHED",
        "userReview": "PENDING",
    }
    if receipt != expected:
        raise ArtistWispAttractorError("Artist V wisp attractor receipt changed")


def build_document_text(
    original_text: str,
    document: dict[str, Any],
    restored_rows: list[dict[str, Any]],
) -> str:
    if validate_document(document, restored_rows):
        return original_text
    start, end = _elements_array_bounds(original_text)
    array_text = original_text[start:end]
    body_start = len(array_text) - len(array_text.lstrip())
    if body_start >= len(array_text) or array_text[body_start] != "{":
        raise ArtistWispAttractorError("Artist V elements array layout changed")
    newline = "\r\n" if "\r\n" in original_text else "\n"
    rendered = json.dumps(
        restored_rows, ensure_ascii=False, indent=2, allow_nan=False
    )
    rendered_lines = rendered.splitlines()[1:-1]
    if not rendered_lines or any(not line.startswith("  ") for line in rendered_lines):
        raise ArtistWispAttractorError("Artist V staged row indentation changed")
    row_indent = array_text[:body_start].rsplit(newline, 1)[-1]
    rendered = (newline + row_indent).join(line[2:] for line in rendered_lines)
    migrated_array = (
        array_text[:body_start]
        + rendered
        + ","
        + newline
        + row_indent
        + array_text[body_start:]
    )
    result = original_text[:start] + migrated_array + original_text[end:]
    try:
        staged = json.loads(result)
    except json.JSONDecodeError as error:
        raise ArtistWispAttractorError(
            f"Artist V staged JSON changed: {error}"
        ) from error
    if not validate_document(staged, restored_rows):
        raise ArtistWispAttractorError("Artist V staged cohort was not admitted")
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
        json.loads(temporary.read_text(encoding="utf-8-sig"))
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def run(
    *,
    write: bool,
    document_path: pathlib.Path = DOCUMENT_PATH,
    donor_path: pathlib.Path = DONOR_PATH,
    role_path: pathlib.Path = ROLE_PATH,
    track_a_path: pathlib.Path = TRACK_A_RECEIPT_PATH,
    receipt_path: pathlib.Path = MATERIALIZATION_RECEIPT_PATH,
) -> bool:
    validate_materialization_receipt(receipt_path)
    restored_rows = validate_evidence(
        donor_path=donor_path, role_path=role_path, track_a_path=track_a_path
    )
    raw, original, document = _load(document_path)
    rendered = build_document_text(original, document, restored_rows)
    changed = rendered != original
    if changed and not write:
        raise ArtistWispAttractorError(
            "Artist V wisp cohort is missing; rerun with --write"
        )
    if changed:
        _atomic_replace(
            document_path, rendered, bom=raw.startswith(codecs.BOM_UTF8)
        )
    return changed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    try:
        changed = run(write=arguments.write)
    except (ArtistWispAttractorError, OSError) as error:
        print(f"ERROR: {error}")
        return 1
    print(
        ("updated" if changed else "check passed")
        + ": Artist V seven-row wisp cohort + root-local attractor"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
