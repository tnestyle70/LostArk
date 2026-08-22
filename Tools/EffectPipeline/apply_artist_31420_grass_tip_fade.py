#!/usr/bin/env python3
"""Append the reviewed Artist S grass-coverage and emissive-tip sprites."""

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
DOCUMENT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31420.unified.effect.json"
)
DONOR_ROLE_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Artist/"
    "effect.artist.skill.31460.role-manifest.json"
)
EFFECT_ID = "effect.artist.skill.31420.unified"
BASE_ROW_COUNT = 1
RUNTIME_MATERIAL_OPCODE = 21
EXPECTED_BASE_ROWS_SHA256 = (
    "591ed3e084abce3ffff0f9931ae7c483a8951620b1e257477840fd069b712c5d"
)

GRASS_COVERAGE_ID = "sprite.artist.31420.grass-coverage.v1"
GRASS_TIP_ID = "sprite.artist.31420.grass-tip-emissive.v1"
PROJECT_ROW_IDS = (GRASS_COVERAGE_ID, GRASS_TIP_ID)

LINE_ASSET_ID = "Effect/Artist/Textures/fx_a_line_003.dds"
GRASS_03_ASSET_ID = "Effect/Artist/Textures/fx_o_grass_03.dds"
GRASS_04_ASSET_ID = "Effect/Artist/Textures/fx_o_grass_04.dds"
EMISSIVE_ASSET_ID = "Effect/Artist/Textures/fx_d_fluid_007.dds"
DONOR_RESOURCE_IDENTITIES = {
    LINE_ASSET_ID: "c876e05fecbf89ffe02d054851e5b14582539ee3dd99c5e1322d50a613240911",
    GRASS_03_ASSET_ID: "30d69eaa2789941301a874662fb4532d1c2e46475543f63410b7c9be2fd8a337",
    GRASS_04_ASSET_ID: "b3aff65943beb5bf2429da6dae72c931db99932d9c43bfaf830b0f82247b87d0",
}

BODY_START_SECONDS = 0.4318
BODY_LIFE_SECONDS = 1.1
TIP_START_SECONDS = 0.4618
TIP_LIFE_SECONDS = 1.07
COHORT_END_SECONDS = BODY_START_SECONDS + BODY_LIFE_SECONDS


class ArtistGrassTipError(RuntimeError):
    """Raised when the reviewed Artist S composition has drifted."""


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
        raise ArtistGrassTipError(f"invalid JSON in {path}: {error}") from error
    if not isinstance(document, dict):
        raise ArtistGrassTipError(f"{path} root must be an object")
    return raw, text, document


def validate_donor_resources(path: pathlib.Path = DONOR_ROLE_PATH) -> None:
    _, _, manifest = _load_text(path)
    if manifest.get("effectAssetId") != "effect.artist.skill.31460.unified":
        raise ArtistGrassTipError("Artist grass donor role manifest identity changed")
    rows = manifest.get("sourceResourceRepairs")
    if not isinstance(rows, list):
        raise ArtistGrassTipError("Artist grass donor resources are missing")
    indexed = {
        row.get("assetId"): row.get("sha256")
        for row in rows
        if isinstance(row, dict)
    }
    for asset_id, expected_sha256 in DONOR_RESOURCE_IDENTITIES.items():
        if indexed.get(asset_id) != expected_sha256:
            raise ArtistGrassTipError(
                f"Artist grass donor resource identity changed: {asset_id}"
            )


def _typed_sampler() -> dict[str, Any]:
    return {
        "filter": "linear",
        "addressU": "wrap",
        "addressV": "wrap",
        "addressW": "wrap",
        "mipLodBias": 0,
        "maxAnisotropy": 1,
        "comparison": "never",
        "borderColor": [0, 0, 0, 0],
        "minLod": 0,
        "maxLod": 3.40282347e38,
    }


def build_execution(
    *, mask_asset_id: str, dissolve_asset_id: str
) -> dict[str, Any]:
    lane_rows = (
        ("base_radiance", LINE_ASSET_ID, "RGBA"),
        ("coverage", mask_asset_id, "R"),
        ("emissive_radiance", EMISSIVE_ASSET_ID, "RGB"),
        ("dissolve", dissolve_asset_id, "R"),
    )
    lanes = [
        {
            "laneId": f"lane.{index}",
            "role": role,
            "assetId": asset_id,
            "textureRegister": index,
            "samplerRegister": 5 + index,
            "sourceChannel": source_channel,
            "colorSpace": "linear",
            "sampler": _typed_sampler(),
        }
        for index, (role, asset_id, source_channel) in enumerate(lane_rows)
    ]
    return {
        "enabled": True,
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": RUNTIME_MATERIAL_OPCODE,
        "passIndex": 1,
        "renderState": {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "textureLaneCount": 4,
        "textureMask": 15,
        "textureLanes": lanes,
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 0,
        "particleColorPolicy": 0,
        "particleColorConsumedMask": 0,
        "particleColorSuppressedMask": 0,
        "scalarCount": 0,
        "vectorCount": 0,
        "inputCount": 0,
        "inputConsumedMask": [0, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [0, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 0,
        "renderConsumedMask": 0,
        "renderSuppressedMask": 0,
        "scalars": [],
        "vectors": [],
        "artistParameters": [],
        "colors": [],
    }


def _base_element(
    *,
    element_id: str,
    display_name: str,
    source_node: str,
    mask_asset_id: str,
    dissolve_asset_id: str,
    position: list[float],
    scale: list[float],
    end_scale: list[float],
    color: list[float],
    emissive_intensity: float,
    start_seconds: float,
    life_seconds: float,
    dissolve_start: float,
    uv_speed: list[float],
    random_seed: int,
    typed_execution: bool,
) -> dict[str, Any]:
    element = {
        "id": element_id,
        "displayName": display_name,
        "groupId": "project.artist.31420.grass-tip-fade",
        "sourceNode": source_node,
        "visible": True,
        "kind": "sprite",
        "resources": [
            {"slotId": "base", "assetId": LINE_ASSET_ID},
            {"slotId": "mask", "assetId": mask_asset_id},
            {"slotId": "emissive", "assetId": EMISSIVE_ASSET_ID},
            {"slotId": "dissolve", "assetId": dissolve_asset_id},
        ],
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
                "position": position,
                "rotationDegrees": [0, 0, 0],
                "revolutionDegreesPerSecond": [0, 0, 0],
                "scale": scale,
                "velocityPerSecond": [0, 0, 0],
            },
            "color": {
                "offset": [0, 0, 0, 0],
                "multiply": color,
                "clip": 0.02,
                "emissiveIntensity": emissive_intensity,
                "distortionIntensity": 0,
                "distortionOnBaseMaterial": False,
                "radialTime": 0,
                "radialIntensity": 0,
            },
            "uv": {
                "start": [0, 0],
                "speed": uv_speed,
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
                "startDelaySeconds": start_seconds,
                "lifeTimeSeconds": life_seconds,
                "afterImageSeconds": 0,
                "dissolveStartNormalized": dissolve_start,
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
            "decal": {"size": scale[:2], "depth": 0.25},
            "linearLerp": {
                "position": False,
                "endPosition": position,
                "rotation": False,
                "endRotationDegrees": [0, 0, 0],
                "revolution": False,
                "endRevolutionDegreesPerSecond": [0, 0, 0],
                "scale": scale != end_scale,
                "endScale": end_scale,
                "velocity": False,
                "endVelocityPerSecond": [0, 0, 0],
                "colorOffset": False,
                "endColorOffset": [0, 0, 0, 0],
                "colorMultiply": True,
                "endColorMultiply": color[:3] + [0],
                "emissiveIntensity": True,
                "endEmissiveIntensity": 0,
            },
            "particle": {
                "maxParticles": 1,
                "spawnRatePerSecond": 0,
                "burstCount": 0,
                "randomSeed": random_seed,
                "lifeTimeSeconds": [life_seconds, life_seconds],
                "initialPositionMin": [0, 0, 0],
                "initialPositionMax": [0, 0, 0],
                "initialVelocityMin": [0, 0, 0],
                "initialVelocityMax": [0, 0, 0],
                "acceleration": [0, 0, 0],
                "startSize": scale[:2],
                "endSize": end_scale[:2],
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
    if typed_execution:
        element["material"]["execution"] = build_execution(
            mask_asset_id=mask_asset_id,
            dissolve_asset_id=dissolve_asset_id,
        )
    return element


def build_project_rows(
    *, typed_execution: bool = True
) -> list[dict[str, Any]]:
    body = _base_element(
        element_id=GRASS_COVERAGE_ID,
        display_name="PROJECT_TUNED | S grass coverage dissolve",
        source_node="project-authored:artist:31420:s:grass-coverage:v1",
        mask_asset_id=GRASS_04_ASSET_ID,
        dissolve_asset_id=GRASS_03_ASSET_ID,
        position=[0, 0.72, 1.15],
        scale=[4.6, 1.5, 1],
        end_scale=[4.6, 1.5, 1],
        color=[0.28, 0.78, 0.38, 0.82],
        emissive_intensity=1.4,
        start_seconds=BODY_START_SECONDS,
        life_seconds=BODY_LIFE_SECONDS,
        dissolve_start=0.42,
        uv_speed=[0, 0.08],
        random_seed=31420,
        typed_execution=typed_execution,
    )
    tip = _base_element(
        element_id=GRASS_TIP_ID,
        display_name="PROJECT_TUNED | S grass-tip HDR emissive fade",
        source_node="project-authored:artist:31420:s:grass-tip-emissive:v1",
        mask_asset_id=GRASS_03_ASSET_ID,
        dissolve_asset_id=GRASS_04_ASSET_ID,
        position=[0, 1.32, 1.15],
        scale=[4.35, 0.5, 1],
        end_scale=[4.7, 0.18, 1],
        color=[0.95, 1, 0.42, 0.78],
        emissive_intensity=6,
        start_seconds=TIP_START_SECONDS,
        life_seconds=TIP_LIFE_SECONDS,
        dissolve_start=0.38,
        uv_speed=[0, -0.05],
        random_seed=31421,
        typed_execution=typed_execution,
    )
    return [body, tip]


def _elements_array_bounds(text: str) -> tuple[int, int]:
    marker = text.find('"elements"')
    opening = text.find("[", marker)
    if marker < 0 or opening < 0:
        raise ArtistGrassTipError("Artist S elements array is missing")
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
    raise ArtistGrassTipError("Artist S elements array is unterminated")


def validate_document(document: dict[str, Any]) -> bool:
    if document.get("effectAssetId") != EFFECT_ID:
        raise ArtistGrassTipError("Artist S document identity changed")
    if document.get("version") != 13:
        raise ArtistGrassTipError("Artist S document version changed")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) not in (1, 3):
        raise ArtistGrassTipError(
            "Artist S must contain one source row and two project sprites"
        )
    if _canonical_sha256(elements[:BASE_ROW_COUNT]) != EXPECTED_BASE_ROWS_SHA256:
        raise ArtistGrassTipError("Artist S source-owned row changed")
    source_ids = [row.get("id") for row in elements[:BASE_ROW_COUNT]]
    if source_ids != ["authored.source-particle.25b54f02bd198502d3b9f851"]:
        raise ArtistGrassTipError("Artist S source row identity changed")
    project_rows = build_project_rows()
    matches = [row for row in elements if row.get("id") in PROJECT_ROW_IDS]
    if len(elements) == BASE_ROW_COUNT:
        if matches:
            raise ArtistGrassTipError("Artist S project row displaced source data")
        return False
    if elements[BASE_ROW_COUNT:] == project_rows and matches == project_rows:
        if len({row.get("id") for row in elements}) != len(elements):
            raise ArtistGrassTipError("Artist S element IDs are duplicated")
        return True
    legacy_rows = build_project_rows(typed_execution=False)
    if elements[BASE_ROW_COUNT:] == legacy_rows and matches == legacy_rows:
        return False
    else:
        raise ArtistGrassTipError(
            "Artist S project sprites are duplicated, reordered, or changed"
        )


def _render_project_rows(
    rows: list[dict[str, Any]], newline: str
) -> str:
    rendered = json.dumps(rows, ensure_ascii=False, indent=2, allow_nan=False)
    return newline.join("    " + line for line in rendered.splitlines()[1:-1])


def build_document_text(original_text: str, document: dict[str, Any]) -> str:
    if validate_document(document):
        return original_text
    start, end = _elements_array_bounds(original_text)
    array_text = original_text[start:end]
    elements = document["elements"]
    body_end = len(array_text.rstrip())
    if body_end == 0 or array_text[:body_end][-1] != "}":
        raise ArtistGrassTipError("Artist S elements array layout changed")
    newline = "\r\n" if "\r\n" in original_text else "\n"
    rendered = _render_project_rows(build_project_rows(), newline)
    if len(elements) == BASE_ROW_COUNT:
        migrated_array = (
            array_text[:body_end] + "," + newline + rendered + array_text[body_end:]
        )
    else:
        legacy = _render_project_rows(
            build_project_rows(typed_execution=False), newline
        )
        if array_text.count(legacy) != 1:
            raise ArtistGrassTipError(
                "Artist S legacy project sprite text changed"
            )
        migrated_array = array_text.replace(legacy, rendered, 1)
    result = original_text[:start] + migrated_array + original_text[end:]
    try:
        staged = json.loads(result)
    except json.JSONDecodeError as error:
        raise ArtistGrassTipError(
            f"Artist S staged JSON changed: {error}"
        ) from error
    if not validate_document(staged):
        raise ArtistGrassTipError("Artist S project sprites were not admitted")
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
        if not validate_document(staged):
            raise ArtistGrassTipError("temporary Artist S round-trip changed rows")
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def run(
    *,
    write: bool,
    document_path: pathlib.Path = DOCUMENT_PATH,
    donor_role_path: pathlib.Path = DONOR_ROLE_PATH,
) -> bool:
    validate_donor_resources(donor_role_path)
    raw, original, document = _load_text(document_path)
    rendered = build_document_text(original, document)
    changed = rendered != original
    if changed and not write:
        raise ArtistGrassTipError(
            "Artist S grass-tip sprites are missing; rerun with --write"
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
    except (ArtistGrassTipError, OSError) as error:
        print(f"ERROR: {error}")
        return 1
    print(
        ("updated" if changed else "check passed")
        + ": Artist S source row + grass coverage/HDR tip fade cohort"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
