#!/usr/bin/env python3
"""Restore Artist R compiler resources and append one true symbol LocalDecal."""

from __future__ import annotations

import argparse
import codecs
import hashlib
import json
import os
import pathlib
import tempfile
from typing import Any


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
BA1_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31210.ba1.unified.effect.json"
)
BA4_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31210.ba4.unified.effect.json"
)
BA1_EFFECT_ID = "effect.artist.skill.31210.ba1.unified"
BA4_EFFECT_ID = "effect.artist.skill.31210.ba4.unified"
SYMBOL_ASSET_ID = "Effect/Artist/Textures/fx_o_symbol_14.dds"
SYMBOL_DECAL_ID = "decal.artist.31210.ba4.symbol14.v1"
EXPECTED_BA4_BASE_ROWS_SHA256 = (
    "7d9043a115f4e800d98f9fbf5f3f4ee7ac5811a70ece078b7d63c7316e863ad9"
)
BA1_COMPILER_BINDINGS = {
    "authored.source-particle.98ba7ae504cce7d359b4fffe": {
        "base": "Effect/Artist/Textures/fx_k_auraline_02.dds",
        "noise": "Effect/Artist/Textures/fx_a_noise_014.dds",
    },
    "authored.source-particle.f544ac943110e3d7d9f2e768": {
        "base": "Effect/Artist/Textures/fx_m_atypical_013_yclamp.dds",
        "noise": "Effect/Artist/Textures/fx_m_flow_04_n.dds",
    },
}


class ArtistSymbolDecalError(RuntimeError):
    """Raised when the reviewed Artist R composition has drifted."""


def _canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _load_text(path: pathlib.Path) -> tuple[bytes, str, dict[str, Any]]:
    raw = path.read_bytes()
    text = raw.decode("utf-8-sig")
    try:
        document = json.loads(text)
    except json.JSONDecodeError as error:
        raise ArtistSymbolDecalError(f"invalid JSON in {path}: {error}") from error
    if not isinstance(document, dict):
        raise ArtistSymbolDecalError(f"{path} root must be an object")
    return raw, text, document


def validate_ba1(document: dict[str, Any]) -> None:
    if document.get("effectAssetId") != BA1_EFFECT_ID:
        raise ArtistSymbolDecalError("Artist R BA1 identity changed")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) != 4:
        raise ArtistSymbolDecalError("Artist R BA1 must preserve exactly four rows")
    indexed = {row.get("id"): row for row in elements if isinstance(row, dict)}
    if len(indexed) != 4:
        raise ArtistSymbolDecalError("Artist R BA1 element IDs are duplicated")
    for element_id, expected in BA1_COMPILER_BINDINGS.items():
        element = indexed.get(element_id)
        if element is None:
            raise ArtistSymbolDecalError(f"Artist R BA1 row is missing: {element_id}")
        bindings = {
            row.get("slotId"): row.get("assetId")
            for row in element.get("resources", [])
            if isinstance(row, dict)
        }
        if any(bindings.get(slot) != asset for slot, asset in expected.items()):
            raise ArtistSymbolDecalError(
                f"Artist R BA1 compiler bindings were not restored: {element_id}"
            )
        if "authoringOverrides" in element:
            raise ArtistSymbolDecalError(
                f"Artist R BA1 stale symbol override remains: {element_id}"
            )


def build_symbol_decal() -> dict[str, Any]:
    return {
        "id": SYMBOL_DECAL_ID,
        "displayName": "PROJECT_TUNED | R final symbol LocalDecal",
        "groupId": "project.artist.31210.ba4.symbol",
        "sourceNode": "project-authored:artist:31210:ba4:symbol14:v1",
        "visible": True,
        "kind": "decal",
        "resources": [{"slotId": "base", "assetId": SYMBOL_ASSET_ID}],
        "material": {
            "templateId": "effect.standard",
            "sourceMaterialPath": "",
            "renderProfile": "alpha_two_sided_depth_read",
            "sourceProfile": {"enabled": False},
        },
        "actionCueAttachment": {
            "enabled": True,
            "follow": False,
            "sourceAnchorSlotId": "root",
            "runtimeAnchorSlotId": "root",
            "runtimeBoneName": "",
            "snapshotRootSourceBasisYawDegrees": 0,
            "socketLocalTransform": {
                "position": [0, 0, 0],
                "rotationDegrees": [0, 0, 0],
                "scale": [1, 1, 1],
            },
        },
        "transformInheritance": {"enabled": False, "masterElementId": ""},
        "detail": {
            "transform": {
                "position": [0, 0.03, 0],
                "rotationDegrees": [0, 0, 0],
                "revolutionDegreesPerSecond": [0, -720, 0],
                "scale": [1, 1, 1],
                "velocityPerSecond": [0, 0, 0],
            },
            "color": {
                "offset": [0, 0, 0, 0],
                "multiply": [1, 1, 1, 1],
                "clip": 0,
                "emissiveIntensity": 1,
                "distortionIntensity": 0,
                "distortionOnBaseMaterial": False,
                "radialTime": 0,
                "radialIntensity": 0,
            },
            "uv": {
                "start": [0, 0],
                "speed": [0, 0],
                "wave": False,
                "waveAmplitude": [0, 0],
                "waveFrequency": 1,
                "sequence": False,
                "loop": True,
                "sequenceTerm": 0.1,
                "tileColumns": 1,
                "tileRows": 1,
                "tileIndex": 0,
            },
            "timing": {
                "startDelaySeconds": 0.5333000000000014,
                "lifeTimeSeconds": 0.8,
                "afterImageSeconds": 0,
                "dissolveStartNormalized": 1,
            },
            "mesh": {
                "useModelMaterial": False,
                "sourceTypeDataRotationDegrees": [0, 0, 0],
            },
            "sprite": {
                "billboard": True,
                "billboardRollDegrees": 0,
                "billboardRollDegreesPerSecond": 0,
            },
            "decal": {"size": [2.5, 1.55], "depth": 0.25},
            "linearLerp": {
                "position": False,
                "endPosition": [0, 0.03, 0],
                "rotation": False,
                "endRotationDegrees": [0, 0, 0],
                "revolution": False,
                "endRevolutionDegreesPerSecond": [0, -720, 0],
                "scale": True,
                "endScale": [0.01, 0.01, 0.01],
                "velocity": False,
                "endVelocityPerSecond": [0, 0, 0],
                "colorOffset": False,
                "endColorOffset": [0, 0, 0, 0],
                "colorMultiply": True,
                "endColorMultiply": [1, 1, 1, 0],
                "emissiveIntensity": False,
                "endEmissiveIntensity": 1,
            },
            "particle": {
                "maxParticles": 1,
                "spawnRatePerSecond": 0,
                "burstCount": 0,
                "randomSeed": 31210,
                "lifeTimeSeconds": [0.8, 0.8],
                "initialPositionMin": [0, 0, 0],
                "initialPositionMax": [0, 0, 0],
                "initialVelocityMin": [0, 0, 0],
                "initialVelocityMax": [0, 0, 0],
                "acceleration": [0, 0, 0],
                "startSize": [2.5, 1.55],
                "endSize": [0.025, 0.0155],
                "localSpace": True,
                "billboard": True,
            },
            "trail": {
                "maxPoints": 64,
                "pointLifeTimeSeconds": 0.35,
                "sampleIntervalSeconds": 0.0166667,
                "minimumDistance": 0.01,
                "startWidth": 0.2,
                "endWidth": 0,
                "tilingDistanceWorldUnits": 0,
                "distanceTessellationStepWorldUnits": 0,
                "faceCamera": True,
            },
            "afterImage": {
                "sampleIntervalSeconds": 0.05,
                "maxCopies": 16,
                "alphaExponent": 1,
            },
            "light": {"enabled": False},
            "screenPost": {"enabled": False},
        },
        "sourceRecipe": {
            "enabled": False,
            "rendererShape": "",
            "emitterDelaySeconds": 0,
            "emitterDurationSeconds": 0,
            "emitterLoopCount": 1,
            "bursts": [],
            "modules": [],
        },
        "sourcePresentation": {"enabled": False},
    }


def _elements_array_bounds(text: str) -> tuple[int, int]:
    marker = text.find('"elements"')
    opening = text.find("[", marker)
    if marker < 0 or opening < 0:
        raise ArtistSymbolDecalError("BA4 elements array is missing")
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
    raise ArtistSymbolDecalError("BA4 elements array is unterminated")


def validate_ba4(document: dict[str, Any]) -> bool:
    if document.get("effectAssetId") != BA4_EFFECT_ID:
        raise ArtistSymbolDecalError("Artist R BA4 identity changed")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) not in (68, 69):
        raise ArtistSymbolDecalError("Artist R BA4 must contain 68 base rows + one decal")
    if _canonical_sha256(elements[:68]) != EXPECTED_BA4_BASE_ROWS_SHA256:
        raise ArtistSymbolDecalError("Artist R BA4 tuned 68-row baseline changed")
    matches = [row for row in elements if row.get("id") == SYMBOL_DECAL_ID]
    if len(elements) == 68:
        if matches:
            raise ArtistSymbolDecalError("Artist R symbol decal displaced a tuned row")
        return False
    if matches != [build_symbol_decal()] or elements[-1] != matches[0]:
        raise ArtistSymbolDecalError("Artist R symbol decal is duplicated or changed")
    return True


def build_ba4_text(original_text: str, document: dict[str, Any]) -> str:
    if validate_ba4(document):
        return original_text
    start, end = _elements_array_bounds(original_text)
    array_text = original_text[start:end]
    body_end = len(array_text.rstrip())
    if body_end == 0 or array_text[:body_end][-1] != "}":
        raise ArtistSymbolDecalError("Artist R BA4 array layout changed")
    newline = "\r\n" if "\r\n" in original_text else "\n"
    rendered = json.dumps(
        build_symbol_decal(), ensure_ascii=False, indent=2, allow_nan=False
    )
    rendered = newline.join("    " + line for line in rendered.splitlines())
    migrated_array = (
        array_text[:body_end] + "," + newline + rendered + array_text[body_end:]
    )
    result = original_text[:start] + migrated_array + original_text[end:]
    try:
        staged = json.loads(result)
    except json.JSONDecodeError as error:
        raise ArtistSymbolDecalError(
            f"Artist R staged BA4 JSON changed: {error}"
        ) from error
    if not validate_ba4(staged):
        raise ArtistSymbolDecalError("Artist R symbol decal was not admitted")
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
        staged = json.loads(temporary.read_text(encoding="utf-8-sig"))
        if not validate_ba4(staged):
            raise ArtistSymbolDecalError("temporary BA4 round-trip changed decal")
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def run(*, write: bool, ba1_path: pathlib.Path = BA1_PATH, ba4_path: pathlib.Path = BA4_PATH) -> bool:
    _, _, ba1 = _load_text(ba1_path)
    validate_ba1(ba1)
    raw, original, ba4 = _load_text(ba4_path)
    rendered = build_ba4_text(original, ba4)
    changed = rendered != original
    if changed and not write:
        raise ArtistSymbolDecalError(
            "Artist R true LocalDecal is missing; rerun with --write"
        )
    if changed:
        _atomic_replace(ba4_path, rendered, bom=raw.startswith(codecs.BOM_UTF8))
    return changed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    try:
        changed = run(write=arguments.write)
    except (ArtistSymbolDecalError, OSError) as error:
        print(f"ERROR: {error}")
        return 1
    print(
        ("updated" if changed else "check passed")
        + ": Artist R compiler rows + one centered symbol LocalDecal"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
