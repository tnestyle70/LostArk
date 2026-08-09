#!/usr/bin/env python3
"""Seed the DimensionMaster A authored baseline.

The seed is a separate authored layer.  It reads the canonical Effect only as
a structural template and never mutates Imported data or canonical source
recipes.  The output is created once; subsequent tuning is owned by the Effect
Tool and this script refuses to overwrite it.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path
import tempfile
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
AUTHORED_ROOT = (REPOSITORY_ROOT / "Data/Effects/Authored").resolve()
CORRECTION_ROOT = (
    REPOSITORY_ROOT / "Data/Effects/AuthoredCorrections/DimensionMaster"
).resolve()
DEFAULT_SOURCE = AUTHORED_ROOT / "effect.dimensionmaster.skill.2050210.effect.json"
DEFAULT_CORRECTION = CORRECTION_ROOT / (
    "effect.dimensionmaster.skill.2050210.authored-baseline.correction.json"
)
DEFAULT_OUTPUT = AUTHORED_ROOT / (
    "effect.dimensionmaster.skill.2050210.authored-baseline.effect.json"
)

CORRECTION_SCHEMA = "lostark.effect-authored-correction"
CORRECTION_VERSION = 2
SOURCE_EFFECT_ID = "effect.dimensionmaster.skill.2050210"
TARGET_EFFECT_ID = "effect.dimensionmaster.skill.2050210.authored-baseline"
CORRECTION_ID = "dimensionmaster.skill.2050210.authored-baseline"
STANDALONE_SPRITE = "standaloneSprite"
STANDALONE_MESH = "standaloneMesh"
AUTHORED_SPRITE_BILLBOARD_ROLL_DEGREES = -90.0
RESOURCE_SLOT_ORDER = ("meshModel", "base", "noise", "mask", "emissive", "dissolve")
TEXTURE_RESOURCE_SLOTS = frozenset({"base", "noise", "mask", "emissive", "dissolve"})
HIT_SNAPSHOT_CONTRACT = (
    ("hit01", "", 0.25),
    ("hit02", ".event_source-event-030", 0.60),
    ("hit03", ".event_source-event-045", 0.90),
    ("hit04", ".event_source-event-060", 1.30),
)
LAYER_CONTRACT = (
    (
        "white-echo",
        STANDALONE_MESH,
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_2",
    ),
    (
        "flow",
        STANDALONE_MESH,
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_14",
    ),
    (
        "body",
        STANDALONE_MESH,
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_15",
    ),
    (
        "afterimage",
        STANDALONE_MESH,
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_20",
    ),
    (
        "rim",
        STANDALONE_MESH,
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_3",
    ),
    (
        "sprite",
        STANDALONE_SPRITE,
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_9",
    ),
)


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError(f"{label} must be an object.")
    return value


def _require_string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or len(value) > 256:
        raise ValueError(f"{label} must be a non-empty bounded string.")
    return value


def _require_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{label} must be a number.")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{label} must be finite.")
    return result


def _require_vector(
    value: Any,
    count: int,
    label: str,
    *,
    positive: bool = False,
) -> list[float]:
    if not isinstance(value, list) or len(value) != count:
        raise ValueError(f"{label} must contain exactly {count} numbers.")
    result = [_require_number(component, label) for component in value]
    if positive and any(component <= 0.0 for component in result):
        raise ValueError(f"{label} components must be positive.")
    return result


def _require_asset_id(value: Any, label: str) -> str:
    asset_id = _require_string(value, label)
    normalized = asset_id.replace("\\", "/")
    if (
        normalized != asset_id
        or not normalized.startswith("Effect/")
        or ".." in normalized.split("/")
        or ":" in normalized
    ):
        raise ValueError(f"{label} is not a Resources-relative Effect asset ID.")
    return asset_id


def _resolve_input(path: Path, root: Path, label: str) -> Path:
    resolved = path.resolve()
    try:
        resolved.relative_to(root)
    except ValueError as error:
        raise ValueError(f"{label} escaped its canonical root.") from error
    if not resolved.is_file():
        raise ValueError(f"{label} does not exist: {resolved}")
    return resolved


def _resolve_output(path: Path) -> Path:
    resolved = path.resolve()
    if resolved.parent != AUTHORED_ROOT or resolved != DEFAULT_OUTPUT:
        raise ValueError("Authored Baseline output must use the stable Authored path.")
    return resolved


def _load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            return _require_object(json.load(stream), label)
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"{label} could not be parsed: {error}") from error


def _sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _validate_source(source: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if (
        source.get("schema") != "lostark.effect-authoring"
        or source.get("version") != 12
        or source.get("effectAssetId") != SOURCE_EFFECT_ID
    ):
        raise ValueError("Canonical DimensionMaster A source contract is invalid.")
    elements = source.get("elements")
    if not isinstance(elements, list):
        raise ValueError("Canonical DimensionMaster A elements are missing.")
    by_id: dict[str, dict[str, Any]] = {}
    for element in elements:
        item = _require_object(element, "source element")
        element_id = _require_string(item.get("id"), "source element ID")
        if element_id in by_id:
            raise ValueError(f"Canonical source has a duplicate Element ID: {element_id}")
        by_id[element_id] = item
    return by_id


def _validate_correction(
    correction: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    if correction.get("schema") != CORRECTION_SCHEMA:
        raise ValueError("Authored correction schema is invalid.")
    if correction.get("version") != CORRECTION_VERSION:
        raise ValueError("Authored correction version is invalid.")
    expected_fields = {
        "correctionId": CORRECTION_ID,
        "sourceEffectAssetId": SOURCE_EFFECT_ID,
        "targetEffectAssetId": TARGET_EFFECT_ID,
    }
    for field, expected in expected_fields.items():
        if correction.get(field) != expected:
            raise ValueError(f"Authored correction {field} is invalid.")
    source_hash = correction.get("sourceEffectSha256")
    if (
        not isinstance(source_hash, str)
        or len(source_hash) != 64
        or any(character not in "0123456789abcdef" for character in source_hash)
    ):
        raise ValueError("Authored correction sourceEffectSha256 is invalid.")
    if correction.get("authoredSpriteBillboardRollDegrees") != (
        AUTHORED_SPRITE_BILLBOARD_ROLL_DEGREES
    ):
        raise ValueError("Authored Sprite billboard roll correction must be -90 degrees.")

    cue = _require_object(correction.get("playerCue"), "playerCue")
    if cue != {
        "anchorSlotId": "root",
        "outerFollowPolicy": "follow",
        "elementFollowPolicy": "snapshot",
    }:
        raise ValueError("Authored Baseline requires follow outer cue and root snapshots.")

    hit_snapshots = correction.get("hitSnapshots")
    if not isinstance(hit_snapshots, list) or len(hit_snapshots) != len(
        HIT_SNAPSHOT_CONTRACT
    ):
        raise ValueError("Authored correction requires exactly four hitSnapshots.")
    staged_hits: list[dict[str, Any]] = []
    seen_hit_ids: set[str] = set()
    for index, (raw, expected) in enumerate(
        zip(hit_snapshots, HIT_SNAPSHOT_CONTRACT, strict=True)
    ):
        hit = _require_object(raw, f"hitSnapshots[{index}]")
        hit_id = _require_string(hit.get("id"), "hit snapshot ID")
        expected_id, expected_suffix, expected_time = expected
        if hit_id in seen_hit_ids:
            raise ValueError(f"Duplicate Authored correction hit snapshot: {hit_id}")
        seen_hit_ids.add(hit_id)
        if hit_id != expected_id:
            raise ValueError("Authored hit snapshot order is invalid.")
        suffix = hit.get("sourceEventSuffix")
        if not isinstance(suffix, str) or len(suffix) > 128:
            raise ValueError("Authored hit snapshot sourceEventSuffix is invalid.")
        if suffix != expected_suffix:
            raise ValueError(
                f"Authored hit snapshot sourceEventSuffix is invalid: {hit_id}"
            )
        source_time = _require_number(
            hit.get("sourceTimeSeconds"), "sourceTimeSeconds"
        )
        if source_time != expected_time:
            raise ValueError(f"Authored hit snapshot time is invalid: {hit_id}")
        transform = _require_object(hit.get("transform"), "hit snapshot transform")
        position = _require_vector(
            transform.get("position"), 3, "hit snapshot position"
        )
        if position[0] >= 0.0 or position[2] <= 0.0:
            raise ValueError(
                "Authored hit snapshots must remain left/forward in Player-root space."
            )
        _require_vector(
            transform.get("rotationDegrees"), 3, "hit snapshot rotation"
        )
        _require_vector(
            transform.get("scale"), 3, "hit snapshot scale", positive=True
        )
        staged_hits.append(hit)

    layers = correction.get("layers")
    if not isinstance(layers, list) or len(layers) != len(LAYER_CONTRACT):
        raise ValueError("Authored correction requires exactly six layers.")
    staged_layers: list[dict[str, Any]] = []
    for index, (raw, expected) in enumerate(zip(layers, LAYER_CONTRACT, strict=True)):
        layer = _require_object(raw, f"layers[{index}]")
        role = _require_string(layer.get("role"), "layer role")
        carrier_kind = _require_string(
            layer.get("carrierKind"), "layer carrierKind"
        )
        source_element_base_id = _require_string(
            layer.get("sourceElementBaseId"), "layer sourceElementBaseId"
        )
        expected_role, expected_kind, expected_source_id = expected
        if role != expected_role:
            raise ValueError("Authored baseline layer draw order is invalid.")
        if carrier_kind != expected_kind or source_element_base_id != expected_source_id:
            raise ValueError(f"Authored baseline source layer is invalid: {role}")
        emissive_from_slot = _require_string(
            layer.get("emissiveFromSlot"), "layer emissiveFromSlot"
        )
        if emissive_from_slot not in TEXTURE_RESOURCE_SLOTS - {"emissive"}:
            raise ValueError(f"Authored layer emissiveFromSlot is invalid: {role}")

        transform = _require_object(layer.get("transform"), "layer transform")
        _require_vector(
            transform.get("positionOffset"), 3, "layer positionOffset"
        )
        _require_vector(
            transform.get("rotationDegrees"), 3, "layer rotation"
        )
        _require_vector(
            transform.get("scale"), 3, "layer scale", positive=True
        )
        _require_vector(
            transform.get("revolutionDegreesPerSecond"),
            3,
            "layer revolution",
        )

        timing = _require_object(layer.get("timing"), "layer timing")
        life_time = _require_number(timing.get("lifeTimeSeconds"), "lifeTimeSeconds")
        dissolve_start = _require_number(
            timing.get("dissolveStartNormalized"), "dissolveStartNormalized"
        )
        if life_time <= 0.0 or not 0.0 <= dissolve_start <= 1.0:
            raise ValueError("Authored layer timing is outside the admitted range.")

        color = _require_object(layer.get("color"), "layer color")
        multiply = _require_vector(color.get("multiply"), 4, "color multiply")
        emissive = _require_number(
            color.get("emissiveIntensity"), "emissiveIntensity"
        )
        distortion = _require_number(
            color.get("distortionIntensity"), "distortionIntensity"
        )
        if not 0.0 <= multiply[3] <= 1.0 or emissive < 0.0 or distortion < 0.0:
            raise ValueError("Authored layer color is outside the admitted range.")

        uv = _require_object(layer.get("uv"), "layer uv")
        _require_vector(uv.get("speed"), 2, "uv speed")
        staged_layers.append(layer)
    return staged_hits, staged_layers


def _reset_source_recipe() -> dict[str, Any]:
    return {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 0,
        "bursts": [],
        "modules": [],
    }


def _reset_source_presentation() -> dict[str, Any]:
    return {
        "enabled": False,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": "",
        "status": "unresolved",
        "sourceObjectPath": "",
        "sourceActionCueId": "",
        "sourceEventId": "",
        "sourceOccurrenceIndex": 0,
        "sourceTimeSeconds": 0.0,
        "parameters": [],
    }


def _copy_resources(
    source_element: dict[str, Any],
    carrier_kind: str,
    emissive_from_slot: str,
) -> list[dict[str, str]]:
    raw_resources = source_element.get("resources")
    if not isinstance(raw_resources, list):
        raise ValueError("Source layer resources are missing.")
    resources: dict[str, str] = {}
    for index, raw in enumerate(raw_resources):
        resource = _require_object(raw, f"source resource[{index}]")
        slot_id = _require_string(resource.get("slotId"), "source resource slotId")
        if slot_id not in RESOURCE_SLOT_ORDER:
            raise ValueError(f"Source layer has an unknown resource slot: {slot_id}")
        if slot_id in resources:
            raise ValueError(f"Source layer has a duplicate resource slot: {slot_id}")
        resources[slot_id] = _require_asset_id(
            resource.get("assetId"), f"source resource {slot_id}"
        )
    if carrier_kind == STANDALONE_MESH and "meshModel" not in resources:
        raise ValueError("Standalone Mesh source layer requires meshModel.")
    if carrier_kind == STANDALONE_SPRITE and (
        "meshModel" in resources or not ({"base", "mask"} & set(resources))
    ):
        raise ValueError("Standalone Sprite source layer requires texture resources only.")
    if emissive_from_slot not in resources:
        raise ValueError(
            f"Source layer does not provide emissiveFromSlot: {emissive_from_slot}"
        )
    resources["emissive"] = resources[emissive_from_slot]
    return [
        {"slotId": slot, "assetId": resources[slot]}
        for slot in RESOURCE_SLOT_ORDER
        if slot in resources
    ]


def _build_element(
    source_element: dict[str, Any],
    source_element_id: str,
    hit_snapshot: dict[str, Any],
    layer: dict[str, Any],
) -> dict[str, Any]:
    element = copy.deepcopy(source_element)
    hit_id = _require_string(hit_snapshot.get("id"), "hit snapshot ID")
    role = _require_string(layer.get("role"), "layer role")
    element_id = f"authored.baseline.a.{hit_id}.{role}"
    group_id = f"authored.baseline.a.{hit_id}"
    carrier_kind = _require_string(
        layer.get("carrierKind"), "layer carrierKind"
    )
    if carrier_kind not in {STANDALONE_SPRITE, STANDALONE_MESH}:
        raise ValueError(f"Unsupported authored carrier kind: {carrier_kind}")
    source_recipe = _require_object(
        source_element.get("sourceRecipe"), "source element SourceRecipe"
    )
    if carrier_kind == STANDALONE_SPRITE and (
        source_element.get("kind") != "particle"
        or not source_recipe.get("enabled")
        or source_recipe.get("rendererShape") != "sprite"
    ):
        raise ValueError("Standalone Sprite carrier must reference a Sprite Renderer particle.")
    if carrier_kind == STANDALONE_MESH and (
        source_element.get("kind") != "particle"
        or not source_recipe.get("enabled")
        or source_recipe.get("rendererShape") != "mesh"
    ):
        raise ValueError("Standalone Mesh carrier must reference a Mesh Renderer particle.")
    element["id"] = element_id
    element["displayName"] = element_id
    element["groupId"] = group_id
    element["sourceNode"] = (
        f"authored-correction:{CORRECTION_ID}|source:{source_element_id}"
    )
    element["visible"] = True
    element["kind"] = "sprite" if carrier_kind == STANDALONE_SPRITE else "mesh"
    emissive_from_slot = _require_string(
        layer.get("emissiveFromSlot"), "layer emissiveFromSlot"
    )
    element["resources"] = _copy_resources(
        source_element, carrier_kind, emissive_from_slot
    )

    source_material = _require_object(
        source_element.get("material"), "source layer material"
    )
    _require_string(source_material.get("templateId"), "source material templateId")
    _require_string(
        source_material.get("sourceMaterialPath"), "source material path"
    )
    _require_string(
        source_material.get("renderProfile"), "source material renderProfile"
    )
    source_profile = _require_object(
        source_material.get("sourceProfile"), "source material sourceProfile"
    )
    if source_profile.get("enabled") is not True:
        raise ValueError(f"Source material profile is not enabled: {source_element_id}")
    _require_string(source_profile.get("profileId"), "source material profileId")
    _require_string(
        source_profile.get("runtimeShaderProfileId"),
        "source material runtimeShaderProfileId",
    )
    element["material"] = copy.deepcopy(source_material)
    element["actionCueAttachment"] = {
        "enabled": True,
        "follow": False,
        "sourceAnchorSlotId": "root",
        "runtimeAnchorSlotId": "root",
        "runtimeBoneName": "",
        "socketLocalTransform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }
    element["sourceRecipe"] = _reset_source_recipe()
    element["sourcePresentation"] = _reset_source_presentation()

    detail = _require_object(element.get("detail"), "source element detail")
    hit_transform = _require_object(
        hit_snapshot.get("transform"), "hit snapshot transform"
    )
    layer_transform = _require_object(layer.get("transform"), "layer transform")
    detail_transform = _require_object(detail.get("transform"), "detail transform")
    hit_position = _require_vector(
        hit_transform.get("position"), 3, "hit snapshot position"
    )
    position_offset = _require_vector(
        layer_transform.get("positionOffset"), 3, "layer positionOffset"
    )
    hit_rotation = _require_vector(
        hit_transform.get("rotationDegrees"), 3, "hit snapshot rotation"
    )
    layer_rotation = _require_vector(
        layer_transform.get("rotationDegrees"), 3, "layer rotation"
    )
    hit_scale = _require_vector(
        hit_transform.get("scale"), 3, "hit snapshot scale", positive=True
    )
    layer_scale = _require_vector(
        layer_transform.get("scale"), 3, "layer scale", positive=True
    )
    detail_transform["position"] = [
        parent + offset for parent, offset in zip(hit_position, position_offset)
    ]
    detail_transform["rotationDegrees"] = [
        parent + local for parent, local in zip(hit_rotation, layer_rotation)
    ]
    detail_transform["scale"] = [
        parent * local for parent, local in zip(hit_scale, layer_scale)
    ]
    detail_transform["revolutionDegreesPerSecond"] = _require_vector(
        layer_transform.get("revolutionDegreesPerSecond"),
        3,
        "layer revolution",
    )
    detail_transform["velocityPerSecond"] = [0.0, 0.0, 0.0]

    timing = _require_object(layer.get("timing"), "layer timing")
    detail_timing = _require_object(detail.get("timing"), "detail timing")
    source_time = _require_number(
        hit_snapshot.get("sourceTimeSeconds"), "sourceTimeSeconds"
    )
    life_time = _require_number(timing.get("lifeTimeSeconds"), "lifeTimeSeconds")
    dissolve_start = _require_number(
        timing.get("dissolveStartNormalized"), "dissolveStartNormalized"
    )
    if source_time < 0.0 or life_time <= 0.0 or not 0.0 <= dissolve_start <= 1.0:
        raise ValueError("Occurrence timing is outside the admitted range.")
    detail_timing["startDelaySeconds"] = source_time
    detail_timing["lifeTimeSeconds"] = life_time
    detail_timing["afterImageSeconds"] = 0.0
    detail_timing["dissolveStartNormalized"] = dissolve_start

    color = _require_object(layer.get("color"), "layer color")
    detail_color = _require_object(detail.get("color"), "detail color")
    detail_color["offset"] = [0.0, 0.0, 0.0, 0.0]
    detail_color["multiply"] = _require_vector(
        color.get("multiply"), 4, "color multiply"
    )
    detail_color["clip"] = 0.0
    detail_color["emissiveIntensity"] = _require_number(
        color.get("emissiveIntensity"), "emissiveIntensity"
    )
    detail_color["distortionIntensity"] = _require_number(
        color.get("distortionIntensity"), "distortionIntensity"
    )
    detail_color["distortionOnBaseMaterial"] = False

    uv = _require_object(layer.get("uv"), "layer uv")
    detail_uv = _require_object(detail.get("uv"), "detail uv")
    detail_uv["start"] = [0.0, 0.0]
    detail_uv["speed"] = _require_vector(uv.get("speed"), 2, "uv speed")

    lerp = _require_object(detail.get("linearLerp"), "detail linearLerp")
    lerp["position"] = False
    lerp["rotation"] = False
    lerp["revolution"] = False
    lerp["endRevolutionDegreesPerSecond"] = [0.0, 0.0, 0.0]
    lerp["scale"] = False
    lerp["velocity"] = False
    lerp["colorOffset"] = False
    lerp["colorMultiply"] = False
    lerp["emissiveIntensity"] = False

    if carrier_kind == STANDALONE_MESH:
        detail["mesh"] = {"useModelMaterial": False}
    if isinstance(detail.get("sprite"), dict):
        detail["sprite"]["billboard"] = carrier_kind == STANDALONE_SPRITE
        detail["sprite"]["billboardRollDegrees"] = (
            AUTHORED_SPRITE_BILLBOARD_ROLL_DEGREES
            if carrier_kind == STANDALONE_SPRITE
            else 0.0
        )
    if isinstance(detail.get("particle"), dict):
        detail["particle"]["spawnRatePerSecond"] = 0.0
        detail["particle"]["burstCount"] = 0
        detail["particle"]["lifeTimeSeconds"] = [life_time, life_time]
    return element


def build_document(
    source: dict[str, Any], correction: dict[str, Any]
) -> dict[str, Any]:
    source_by_id = _validate_source(source)
    hit_snapshots, layers = _validate_correction(correction)
    staged_elements: list[dict[str, Any]] = []
    for hit_snapshot in hit_snapshots:
        suffix = hit_snapshot["sourceEventSuffix"]
        for layer in layers:
            source_base_id = layer["sourceElementBaseId"]
            source_id = f"{source_base_id}{suffix}"
            source_element = source_by_id.get(source_id)
            if source_element is None:
                raise ValueError(f"Correction source Element is missing: {source_id}")
            staged_elements.append(
                _build_element(source_element, source_id, hit_snapshot, layer)
            )

    particle_system = _require_object(
        correction.get("particleSystem"), "particleSystem"
    )
    staged_particle_system = {
        "uniformScaleMultiplier": _require_number(
            particle_system.get("uniformScaleMultiplier"),
            "uniformScaleMultiplier",
        ),
        "yawOffsetDegrees": _require_number(
            particle_system.get("yawOffsetDegrees"), "yawOffsetDegrees"
        ),
        "directionYawDegrees": _require_number(
            particle_system.get("directionYawDegrees"), "directionYawDegrees"
        ),
        "initialSpeedMultiplier": _require_number(
            particle_system.get("initialSpeedMultiplier"),
            "initialSpeedMultiplier",
        ),
    }
    if (
        staged_particle_system["uniformScaleMultiplier"] <= 0.0
        or staged_particle_system["initialSpeedMultiplier"] <= 0.0
    ):
        raise ValueError("Particle-system scale and speed multipliers must be positive.")

    return {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": TARGET_EFFECT_ID,
        "displayName": "DimensionMaster A 2050210 Authored Baseline",
        "particleSystem": staged_particle_system,
        "modelCues": [],
        "elements": staged_elements,
    }


def write_document_seed(output_path: Path, document: dict[str, Any]) -> None:
    output_path = _resolve_output(output_path)
    if output_path.exists():
        raise FileExistsError(
            "Authored Baseline already exists; edit and save it with the Effect Tool."
        )
    serialized = json.dumps(document, indent=2, ensure_ascii=False) + "\n"
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{output_path.name}.", suffix=".tmp", dir=output_path.parent
    )
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(serialized)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_name, output_path)
    except Exception:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def materialize(
    source_path: Path,
    correction_path: Path,
    output_path: Path,
    *,
    write: bool,
) -> dict[str, Any]:
    source_path = _resolve_input(source_path, AUTHORED_ROOT, "source path")
    correction_path = _resolve_input(
        correction_path, CORRECTION_ROOT, "correction path"
    )
    output_path = _resolve_output(output_path)
    source = _load_json(source_path, "canonical source")
    correction = _load_json(correction_path, "authored correction")
    expected_source_hash = correction.get("sourceEffectSha256")
    if _sha256_file(source_path) != expected_source_hash:
        raise ValueError("Canonical source changed after the Authored Baseline seed was approved.")
    document = build_document(source, correction)
    if write:
        write_document_seed(output_path, document)
    return document


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--correction", type=Path, default=DEFAULT_CORRECTION)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()
    document = materialize(
        args.source, args.correction, args.output, write=args.write
    )
    if args.write:
        print(f"Materialized {args.output}")
    else:
        print(json.dumps(document, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
