#!/usr/bin/env python3
"""Append the reviewed Lance W cone carrier to Lance E clip3."""

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
DONOR_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.lancemaster.skill.34550.unified.effect.json"
)
TARGET_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.lancemaster.skill.34560.clip3.unified.effect.json"
)
DONOR_EFFECT_ID = "effect.lancemaster.skill.34550.unified"
TARGET_EFFECT_ID = "effect.lancemaster.skill.34560.clip3.unified"
DONOR_ELEMENT_ID = "authored.source-particle.622ad9be48bb849d36fa5b81"
TARGET_ELEMENT_ID = "authored.donor-particle.34560.clip3.w-cone.v1"
DONOR_SHA256 = "810e16e4ed6b300f6f82052e1f147df502a5981babc6577b1fe216ac23d244e1"
TARGET_BASE_SHA256 = "f7b8caae7bd922f0da0361c06b550277260a811b35afc242c43690ff4f257664"


class LanceConeDonorError(RuntimeError):
    """Raised when the reviewed donor or target composition drifts."""


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
        raise LanceConeDonorError(f"invalid JSON in {path}: {error}") from error
    if not isinstance(document, dict):
        raise LanceConeDonorError(f"{path} root must be an object")
    return raw, text, document


def build_donor_element(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("effectAssetId") != DONOR_EFFECT_ID:
        raise LanceConeDonorError("Lance W donor document identity changed")
    elements = document.get("elements")
    if not isinstance(elements, list):
        raise LanceConeDonorError("Lance W donor elements are missing")
    matches = [row for row in elements if row.get("id") == DONOR_ELEMENT_ID]
    if len(matches) != 1 or _canonical_sha256(matches[0]) != DONOR_SHA256:
        raise LanceConeDonorError("Lance W reviewed cone carrier changed")
    result = copy.deepcopy(matches[0])
    result["id"] = TARGET_ELEMENT_ID
    result["displayName"] = "DONOR_TRANSPLANT | W cone impact for E"
    result["groupId"] = "project.donor.lancemaster.34560.cone"
    return result


def validate_target(document: dict[str, Any], expected: dict[str, Any]) -> bool:
    if document.get("effectAssetId") != TARGET_EFFECT_ID:
        raise LanceConeDonorError("Lance E clip3 identity changed")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) not in (4, 5):
        raise LanceConeDonorError("Lance E clip3 must preserve four rows plus one donor")
    if _canonical_sha256(elements[:4]) != TARGET_BASE_SHA256:
        raise LanceConeDonorError("Lance E clip3 tuned four-row baseline changed")
    matches = [row for row in elements if row.get("id") == TARGET_ELEMENT_ID]
    if len(elements) == 4:
        if matches:
            raise LanceConeDonorError("Lance E donor displaced a tuned row")
        return False
    if len(matches) != 1 or matches[0] != expected or elements[-1] != expected:
        raise LanceConeDonorError("Lance E cone donor is duplicated or changed")
    resources = {
        row.get("slotId"): row.get("assetId")
        for row in expected.get("resources", [])
        if isinstance(row, dict)
    }
    detail = expected.get("detail", {})
    timing = detail.get("timing", {})
    transform = detail.get("transform", {})
    if (
        resources
        != {
            "meshModel": "Effect/LanceMaster/Meshes/fm_d_cone_005.wmodel",
            "base": "Effect/LanceMaster/Textures/fx_d_atypical_006.dds",
        }
        or expected.get("kind") != "particle"
        or expected.get("material", {}).get("sourceMaterialPath")
        != "fx_m_mi_03.fx_mi.fx_d_me_ringmaster_01_01_ad"
        or timing.get("startDelaySeconds") != 0.300000012
        or timing.get("lifeTimeSeconds") != 0.5
        or transform.get("rotationDegrees") != [0, 97.25, 0]
    ):
        raise LanceConeDonorError("Lance E cone carrier contract changed")
    return True


def _elements_array_bounds(text: str) -> tuple[int, int]:
    marker = text.find('"elements"')
    opening = text.find("[", marker)
    if marker < 0 or opening < 0:
        raise LanceConeDonorError("Lance E elements array is missing")
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
    raise LanceConeDonorError("Lance E elements array is unterminated")


def build_target_text(
    original_text: str, document: dict[str, Any], donor: dict[str, Any]
) -> str:
    if validate_target(document, donor):
        return original_text
    start, end = _elements_array_bounds(original_text)
    array_text = original_text[start:end]
    body_end = len(array_text.rstrip())
    if body_end == 0 or array_text[:body_end][-1] != "}":
        raise LanceConeDonorError("Lance E elements array layout changed")
    newline = "\r\n" if "\r\n" in original_text else "\n"
    rendered = json.dumps(donor, ensure_ascii=False, indent=2, allow_nan=False)
    rendered = newline.join("    " + line for line in rendered.splitlines())
    migrated = array_text[:body_end] + "," + newline + rendered + array_text[body_end:]
    result = original_text[:start] + migrated + original_text[end:]
    staged = json.loads(result)
    if not validate_target(staged, donor):
        raise LanceConeDonorError("Lance E staged cone donor was not admitted")
    return result


def _atomic_replace(
    path: pathlib.Path, text: str, *, bom: bool, donor: dict[str, Any]
) -> None:
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
        staged = json.loads(temporary.read_text(encoding="utf-8-sig"))
        if not validate_target(staged, donor):
            raise LanceConeDonorError("temporary Lance E round-trip changed donor")
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def run(
    *,
    write: bool,
    donor_path: pathlib.Path = DONOR_PATH,
    target_path: pathlib.Path = TARGET_PATH,
) -> bool:
    _, _, donor_document = _load(donor_path)
    donor = build_donor_element(donor_document)
    raw, original, target = _load(target_path)
    rendered = build_target_text(original, target, donor)
    changed = rendered != original
    if changed and not write:
        raise LanceConeDonorError("Lance E cone donor is missing; rerun with --write")
    if changed:
        _atomic_replace(
            target_path,
            rendered,
            bom=raw.startswith(codecs.BOM_UTF8),
            donor=donor,
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
    except (LanceConeDonorError, OSError) as error:
        print(f"ERROR: {error}")
        return 1
    print(("updated" if changed else "check passed") + ": Lance E W-cone donor")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
