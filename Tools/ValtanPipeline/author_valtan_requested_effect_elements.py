#!/usr/bin/env python3
"""Project requested Valtan composites without erasing authored tuning.

This is deliberately an append-preserving authoring projection.  Existing
elements, including rows that were edited live in Effect Tool, are treated as
user-owned and remain byte-for-byte equivalent at the JSON value level.  A
stable generated element is added only while its id is absent; a later run
therefore preserves manual tuning performed on that generated row as well.
Each requested gameplay pattern owns one direct-authored Product Effect and
its typed runtime cue set.  Existing user-authored documents promoted to Product keep
their exact element values; only their redundant DRAFT_ATTACHED binding is
removed after the same file is catalogued as the Product source.  Unrelated
drafts remain untouched.  The existing exact STRUGGLING authored
document is promoted in place so the Tool shows one Product rather than a
second generated comparison asset.

Validate computes the same projection as Apply and reports any missing/stale
output.  Apply uses compare-and-swap guards for every donor and destination,
then promotes all changed JSON files as one rollback-capable transaction.
"""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
from typing import Any, Callable, Iterable, Mapping


ROOT = Path(__file__).resolve().parents[2]

AUTHORING_ROOT = Path("Data/Effects/Authored")
CATALOG_PATH = Path("Data/Effects/EffectCatalog.json")
PRESENTATION_PATH = Path("Data/Valtan/Valtan.presentation.json")
GAMEPLAY_PATH = Path("Data/Valtan/Valtan.gameplay.json")
PATTERN_AUTHORING_BINDINGS_PATH = Path(
    "Data/Effects/ValtanPatternAuthoringEffects.json"
)

DONOR_ARENA_109 = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01.effect.json"
)
DONOR_FLOOR_WIPE = AUTHORING_ROOT / "effect.valtan.floor-wipe-130.effect.json"
DONOR_FOUR_SLASH = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json"
)
DONOR_WHIRLWIND = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json"
)
DONOR_WHIRLWIND_NATIVE = AUTHORING_ROOT / (
    "effect.valtan.pattern.420633.active.v1.unified.effect.json"
)
DONOR_CONE = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.effect.json"
)
DONOR_SKY_AXE = AUTHORING_ROOT / "effect.valtan.sky-axe.active.effect.json"
DONOR_FRONT_BACK = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.front-back-front.active.clip-01.effect.json"
)
DONOR_PORTAL = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json"
)
DONOR_PORTAL_RUSH = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.portal-rush.rushes.clip-01.effect.json"
)
DONOR_HIGH_JUMP_TAKEOFF = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01.effect.json"
)
DONOR_HIGH_JUMP_LAND = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.high-jump.land.clip-01.effect.json"
)
DONOR_TARGET_CONE = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01.effect.json"
)
DONOR_HAND_CORE = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.charge-grab-roar.counter.clip-01.effect.json"
)
DONOR_HAND_RECOVERY = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.charge-grab-roar.recovery.clip-01.effect.json"
)
DONOR_HAND_ROAR = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.charge-grab-roar.roar.clip-01.effect.json"
)
DONOR_FIST_IN_OUT = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.effect.json"
)
DONOR_IMPRISON_ROAR = AUTHORING_ROOT / (
    "effect.valtan.carrier-v1.attack.imprison-roar.active.clip-01.effect.json"
)

RING_DARK_ID = "local_decal_3"
RING_CYAN_ID = "authored.copy.local_decal_3.2"
SECTOR_RED_ID = "authored.copy.authored.copy.sprite_particle_8.1.1"

FOUR_SLASH_FINISH_IDS = (
    "authored.copy.authored.copy.valtan.clip01.weapon-slash.01.1.1",
    "valtan.clip01.hit-spark.01",
    "impact.fragments.hit_007",
)
WHIRLWIND_IDS = (
    "whirlwind.mesh.10.cyan.phase000",
    "authored.copy.whirlwind.mesh.10.cyan.phase000.2",
    "whirlwind.trail.20.axe.main",
)
AXE_AURA_IDS = (
    "valtan.420633.notify005.emitter7034",
    "valtan.420633.notify005.emitter7035",
    "valtan.420633.notify006.emitter7336",
)
SKY_AXE_WAVE_ID = "sprite_particle_7"
CONE_ID = "source.242e5d84c3d400b2e284"
FRONT_BACK_IDS = (
    "source.fd7548be22f18052fa77",
    "source.9662e2ae68d0e8967f0c",
    "source.89720228446a2a858116",
    "source.1093f8f3ea54b148fae0",
    "source.dc17c4b1791e3d008215",
)
FRONT_BACK_SHORT_BLAST_IDS = (
    "source.f8d777d1df9ff25e7865",
    "source.fc574dec0694b30d912b",
)
HAND_CORE_IDS = (
    "source.acdbb3a0e8ab1346264d",
    "source.3185cea483c5e69c9450",
    "source.4130aafa4d8fa24da731",
    "source.fffd6d2e32bf885abba7",
    "source.d7e4b30dd66c86f61803",
    "source.0195edfc2014ce88d9cf",
)
HAND_RECOVERY_SMOKE_ID = "source.9c312f0c5add9f438229"
HAND_RECOVERY_FRAGMENT_ID = "source.fbb066e0115f95665a99"
HAND_RECOVERY_RING_ID = "source.88d8174c6663f929eea7"
HAND_ROAR_IDS = (
    "source.152d5f146482af4e2d14",
    "source.3ac3b1e29652000c0cc2",
)
TARGET_CONE_COMPOSITE_IDS = (
    "source.c0fe650144959a572dbb",
    "source.756fbb3603e8a548576e",
    "source.d8db179c65da128b5e46",
)
PORTAL_RUSH_FORWARD_MESH_ID = "source.6653e9d4443d28016cf4"
HIGH_JUMP_VERTICAL_ID = "authored.copy.authored.copy.donut.impact.wave.black.1.1"
FIST_RING_SPRITE_ID = "sprite_particle_6"
FIST_DONUT_GROW_ID = "donut.telegraph.inner.grow"
IMPRISON_RADIAL_ID = "source.9337c047a20560efa84a"

SECTOR_04 = "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_04.dds"
SECTOR_05 = "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_05.dds"

GENERATED_PREFIX = "requested.20260827."
CUE_PREFIX = "cue.valtan.requested.20260827."
PHASE_TWO_CUE_PREFIX = "cue.valtan.phase2."


class AuthoringError(RuntimeError):
    """The live inputs cannot be projected without risking authored work."""


JsonObject = dict[str, Any]
Mutator = Callable[[JsonObject], None]


@dataclass(frozen=True)
class Target:
    effect_asset_id: str
    display_name: str
    pattern_id: str | None
    catalog_direct: bool = True

    @property
    def relative_path(self) -> Path:
        return AUTHORING_ROOT / f"{self.effect_asset_id}.effect.json"

    @property
    def authoring_path(self) -> str:
        return f"Effects/Authored/{self.effect_asset_id}.effect.json"


TERRAIN_3 = Target(
    "effect.valtan.project-tuned.terrain-destruction-3.semicircle",
    "Requested / Terrain Destruction 3 O'Clock / Red Semicircle",
    "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
)
TERRAIN_9 = Target(
    "effect.valtan.project-tuned.terrain-destruction-9.semicircle",
    "Requested / Terrain Destruction 9 O'Clock / Red Semicircle",
    "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
)
SIX_PIZZA = Target(
    "effect.valtan.project-tuned.sequence.six-pizza-106",
    "Requested / Center Six Pizza / Composite",
    "VALTAN_SIX_PIZZA_106",
)
ATTACK_WHIRLWIND = Target(
    "effect.valtan.project-tuned.sequence.attack-whirlwind",
    "Requested / Jump Slam And Whirlwind / Composite",
    "VALTAN_ATTACK_WHIRLWIND",
)
CHARGE = Target(
    "effect.valtan.project-tuned.sequence.charge",
    "Requested / Gather Slash / Cyan Black Axe Follow",
    "VALTAN_CHARGE",
)
PROMOTED_CHARGE = Target(
    "effect.valtan.sequence.charge",
    "모아치기 | Pattern Effect",
    "VALTAN_CHARGE",
)
CHARGE_2 = Target(
    "effect.valtan.sequence.charge2",
    "Product / Gather Slash 2 / Red Fan",
    "VALTAN_CHARGE_2",
)
ROAR_CHARGE = Target(
    "effect.valtan.sequence.roar-charge",
    "Product / Roar Then Upward Gather / Composite",
    "VALTAN_ROAR_CHARGE",
)
THREE = Target(
    "effect.valtan.project-tuned.sequence.three",
    "Requested / Three Downward Smashes / Composite",
    "VALTAN_THREE",
)
FRONT_BACK_FRONT = Target(
    "effect.valtan.sequence.front-back-front",
    "Product / Front Back Front / Electric Fan",
    "VALTAN_SEQUENCE_FRONT_BACK_FRONT",
)
COUNTER = Target(
    "effect.valtan.project-tuned.sequence.counter",
    "Requested / Counter Downward Smash / Cyan Roar Ring",
    "VALTAN_COUNTER",
)
WARP_PORTAL = Target(
    "effect.valtan.project-tuned.sequence.warp.portal",
    "Requested / Warp / Single Portal And Rush Composite",
    "VALTAN_WARP",
)
WARP_PORTAL_ENTER = Target(
    "effect.valtan.project-tuned.sequence.warp.portal-enter",
    "Requested / Warp / Portal Enter",
    "VALTAN_WARP",
)
TRASH = Target(
    "effect.valtan.project-tuned.sequence.trash",
    "Project Tuned / Trash / Floor And Hand Composite",
    "VALTAN_TRASH",
)
TRASH_CATCH_SUCCESS = Target(
    "effect.valtan.project-tuned.sequence.trash-catch-success",
    "Product / Trash Catch Success / Grab Roar Release",
    "VALTAN_TRASH_CATCH_SUCCESS",
)
TRASH_CATCH_FAIL = Target(
    "effect.valtan.project-tuned.sequence.trash-catch-fail",
    "Product / Trash Catch Fail / Dark Release",
    "VALTAN_TRASH_CATCH_FAIL",
)
TRASH_CATCH_IF = Target(
    "effect.valtan.project-tuned.sequence.trash-catch-if",
    "Product / Trash Catch Branch / Hand Aura",
    "VALTAN_TRASH_CATCH_IF",
)
CATCH_BREATH = Target(
    "effect.valtan.project-tuned.sequence.catch-breath",
    "Product / Catch Breath / Yellow Black Forward Blast",
    "VALTAN_CATCH_BREATH",
)
STRUGGLING = Target(
    "effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead",
    "Product / Phase 3 Struggling / Portal Fan Smash Roar Composite",
    "VALTAN_STRUGGLING",
)

PRODUCT_TARGETS = (
    TERRAIN_3,
    TERRAIN_9,
    SIX_PIZZA,
    ATTACK_WHIRLWIND,
    CHARGE,
    CHARGE_2,
    ROAR_CHARGE,
    THREE,
    FRONT_BACK_FRONT,
    COUNTER,
    WARP_PORTAL,
    TRASH,
    TRASH_CATCH_SUCCESS,
    TRASH_CATCH_FAIL,
    TRASH_CATCH_IF,
    CATCH_BREATH,
    STRUGGLING,
)

PROMOTED_USER_TARGETS = (
    PROMOTED_CHARGE,
)

PRODUCT_CATALOG_TARGETS = (*PRODUCT_TARGETS, *PROMOTED_USER_TARGETS)

PROMOTED_DRAFT_BINDINGS = (
    ("VALTAN_CHARGE", PROMOTED_CHARGE.effect_asset_id),
    ("VALTAN_CHARGE_2", CHARGE_2.effect_asset_id),
    ("VALTAN_ROAR_CHARGE", ROAR_CHARGE.effect_asset_id),
    ("VALTAN_SEQUENCE_FRONT_BACK_FRONT", FRONT_BACK_FRONT.effect_asset_id),
    ("VALTAN_STRUGGLING", STRUGGLING.effect_asset_id),
)

REDUNDANT_GENERATED_CATALOG_IDS = frozenset(
    {
        "effect.valtan.project-tuned.sequence.charge2",
        "effect.valtan.project-tuned.sequence.roar-charge",
        "effect.valtan.project-tuned.sequence.front-back-front",
        WARP_PORTAL_ENTER.effect_asset_id,
    }
)


@dataclass
class Projection:
    outputs: dict[Path, bytes]
    guards: dict[Path, bytes | None]
    appended_by_target: dict[str, int]

    @property
    def changed_paths(self) -> tuple[Path, ...]:
        changed = [
            path
            for path, expected in self.outputs.items()
            if self.guards.get(path) != expected
        ]
        return tuple(sorted(changed, key=lambda value: value.as_posix()))


def _absolute(relative: Path) -> Path:
    result = (ROOT / relative).resolve(strict=False)
    try:
        result.relative_to(ROOT.resolve())
    except ValueError as exc:
        raise AuthoringError(f"path escaped repository: {relative}") from exc
    return result


def _decode_json(payload: bytes, path: Path) -> JsonObject:
    if payload.startswith(b"\xef\xbb\xbf"):
        raise AuthoringError(f"JSON must be UTF-8 without BOM: {path}")

    def no_duplicates(pairs: list[tuple[str, Any]]) -> JsonObject:
        result: JsonObject = {}
        for key, value in pairs:
            if key in result:
                raise AuthoringError(f"duplicate JSON property {key!r}: {path}")
            result[key] = value
        return result

    try:
        value = json.loads(payload.decode("utf-8"), object_pairs_hook=no_duplicates)
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise AuthoringError(f"invalid JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise AuthoringError(f"JSON root must be an object: {path}")
    return value


def _load_required(relative: Path, guards: dict[Path, bytes | None]) -> JsonObject:
    path = _absolute(relative)
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise AuthoringError(f"cannot read required input {relative}: {exc}") from exc
    guards.setdefault(relative, payload)
    return _decode_json(payload, relative)


def _load_optional(
    relative: Path, guards: dict[Path, bytes | None]
) -> JsonObject | None:
    path = _absolute(relative)
    if not path.is_file():
        guards.setdefault(relative, None)
        return None
    payload = path.read_bytes()
    guards.setdefault(relative, payload)
    return _decode_json(payload, relative)


def _json_bytes(value: Any, original: bytes | None = None) -> bytes:
    newline = "\r\n" if original is not None and b"\r\n" in original else "\n"
    text = json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    if newline == "\r\n":
        text = text.replace("\n", "\r\n")
    return text.encode("utf-8")


def _output_bytes(
    relative: Path, value: JsonObject, guards: Mapping[Path, bytes | None]
) -> bytes:
    original = guards.get(relative)
    if original is not None and _decode_json(original, relative) == value:
        return original
    return _json_bytes(value, original)


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise AuthoringError(f"{label} must be an array")
    return value


def _require_object(value: Any, label: str) -> JsonObject:
    if not isinstance(value, dict):
        raise AuthoringError(f"{label} must be an object")
    return value


def _indexed(rows: Iterable[Any], field: str, label: str) -> dict[str, JsonObject]:
    result: dict[str, JsonObject] = {}
    for raw in rows:
        row = _require_object(raw, f"{label} row")
        identity = row.get(field)
        if not isinstance(identity, str) or not identity or identity in result:
            raise AuthoringError(f"invalid/duplicate {label}.{field}: {identity!r}")
        result[identity] = row
    return result


def _validate_effect_document(document: JsonObject, effect_id: str, label: str) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
    ):
        raise AuthoringError(f"{label} effect identity/version drift: {effect_id}")
    elements = _require_list(document.get("elements"), f"{label}.elements")
    _indexed(elements, "id", f"{label}.elements")


def _element(document: JsonObject, element_id: str, label: str) -> JsonObject:
    rows = _indexed(
        _require_list(document.get("elements"), f"{label}.elements"),
        "id",
        f"{label}.elements",
    )
    result = rows.get(element_id)
    if result is None:
        raise AuthoringError(f"missing donor element {label}/{element_id}")
    return result


def _new_effect_document(target: Target) -> JsonObject:
    return {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": target.effect_asset_id,
        "displayName": target.display_name,
        "particleSystem": {
            "uniformScaleMultiplier": 1,
            "yawOffsetDegrees": 0,
            "directionYawDegrees": 0,
            "initialSpeedMultiplier": 1,
        },
        "modelCues": [],
        "elements": [],
    }


def _clone(
    donor: JsonObject,
    donor_id: str,
    generated_id: str,
    role: str,
    *mutators: Mutator,
) -> JsonObject:
    source = _element(donor, donor_id, donor.get("effectAssetId", "donor"))
    result = copy.deepcopy(source)
    result["id"] = generated_id
    result["displayName"] = f"Requested 2026-08-27 / {role}"
    result["groupId"] = "requested.20260827"
    result["sourceNode"] = (
        f"authored-copy:{donor.get('effectAssetId')}:{donor_id}:{generated_id}"
    )
    for mutator in mutators:
        mutator(result)
    return result


def _clone_row(
    source: JsonObject,
    source_effect_asset_id: str,
    generated_id: str,
    role: str,
    *mutators: Mutator,
) -> JsonObject:
    """Clone an already selected Product row without re-resolving its id.

    This is used when the live Product itself is the donor.  It deliberately
    copies the exact current row, including Effect Tool tuning, before assigning
    a new stable id for the receiving composite.
    """

    source_id = source.get("id")
    if not isinstance(source_id, str) or not source_id:
        raise AuthoringError(f"source Product row has no stable id: {role}")
    result = copy.deepcopy(source)
    result["id"] = generated_id
    result["displayName"] = f"Requested 2026-08-27 / {role}"
    result["groupId"] = "requested.20260827"
    result["sourceNode"] = (
        f"authored-copy:{source_effect_asset_id}:{source_id}:{generated_id}"
    )
    for mutator in mutators:
        mutator(result)
    return result


def _detail(element: JsonObject) -> JsonObject:
    return _require_object(element.get("detail"), f"{element.get('id')}.detail")


def _set_resource(element: JsonObject, slot_id: str, asset_id: str) -> None:
    resources = _require_list(element.get("resources"), f"{element.get('id')}.resources")
    matches = [row for row in resources if isinstance(row, dict) and row.get("slotId") == slot_id]
    if len(matches) > 1:
        raise AuthoringError(f"duplicate resource slot {element.get('id')}/{slot_id}")
    compiler_asset_id = ""
    if matches:
        existing_asset_id = matches[0].get("assetId")
        if isinstance(existing_asset_id, str):
            compiler_asset_id = existing_asset_id
        matches[0]["assetId"] = asset_id
    else:
        resources.append({"slotId": slot_id, "assetId": asset_id})
    overrides = element.get("authoringOverrides")
    if isinstance(overrides, dict):
        override_resources = overrides.get("resources")
        if isinstance(override_resources, list):
            override_matches = [
                row
                for row in override_resources
                if isinstance(row, dict) and row.get("slotId") == slot_id
            ]
            if len(override_matches) > 1:
                raise AuthoringError(
                    f"duplicate override resource {element.get('id')}/{slot_id}"
                )
            if override_matches:
                override_matches[0].setdefault(
                    "compilerAssetId", compiler_asset_id
                )
                override_matches[0]["assetId"] = asset_id
            else:
                override_resources.append(
                    {
                        "slotId": slot_id,
                        "assetId": asset_id,
                        "compilerAssetId": compiler_asset_id,
                    }
                )


def _remove_resource(element: JsonObject, slot_id: str) -> None:
    resources = _require_list(element.get("resources"), f"{element.get('id')}.resources")
    resources[:] = [
        row
        for row in resources
        if not isinstance(row, dict) or row.get("slotId") != slot_id
    ]
    overrides = element.get("authoringOverrides")
    if isinstance(overrides, dict) and isinstance(overrides.get("resources"), list):
        overrides["resources"][:] = [
            row
            for row in overrides["resources"]
            if not isinstance(row, dict) or row.get("slotId") != slot_id
        ]


def _set_start(element: JsonObject, seconds: float) -> None:
    timing = _require_object(_detail(element).get("timing"), "detail.timing")
    timing["startDelaySeconds"] = round(float(seconds), 6)
    recipe = element.get("sourceRecipe")
    if isinstance(recipe, dict) and recipe.get("enabled") is True:
        recipe["emitterDelaySeconds"] = round(float(seconds), 6)


def _set_life(element: JsonObject, seconds: float) -> None:
    timing = _require_object(_detail(element).get("timing"), "detail.timing")
    timing["lifeTimeSeconds"] = round(float(seconds), 6)


def _set_yaw(element: JsonObject, degrees: float) -> None:
    transform = _require_object(_detail(element).get("transform"), "detail.transform")
    rotation = _require_list(transform.get("rotationDegrees"), "transform.rotationDegrees")
    if len(rotation) != 3:
        raise AuthoringError("rotationDegrees must contain three components")
    rotation[1] = float(degrees)


def _set_position_y(element: JsonObject, value: float) -> None:
    transform = _require_object(_detail(element).get("transform"), "detail.transform")
    position = _require_list(transform.get("position"), "transform.position")
    if len(position) != 3:
        raise AuthoringError("position must contain three components")
    position[1] = float(value)


def _set_position(element: JsonObject, x: float, y: float, z: float) -> None:
    transform = _require_object(_detail(element).get("transform"), "detail.transform")
    transform["position"] = [float(x), float(y), float(z)]


def _set_rotation(element: JsonObject, pitch: float, yaw: float, roll: float) -> None:
    transform = _require_object(_detail(element).get("transform"), "detail.transform")
    transform["rotationDegrees"] = [float(pitch), float(yaw), float(roll)]


def _set_particle_sizes(
    element: JsonObject,
    start: tuple[float, float],
    end: tuple[float, float],
) -> None:
    particle = _require_object(_detail(element).get("particle"), "detail.particle")
    particle["startSize"] = [float(start[0]), float(start[1])]
    particle["endSize"] = [float(end[0]), float(end[1])]


def _set_single_particle_burst(element: JsonObject, lifetime: float) -> None:
    particle = _require_object(_detail(element).get("particle"), "detail.particle")
    particle["maxParticles"] = 1
    particle["spawnRatePerSecond"] = 0.0
    particle["burstCount"] = 1
    particle["lifeTimeSeconds"] = [float(lifetime), float(lifetime)]
    recipe = _require_object(
        element.get("sourceRecipe"), f"{element.get('id')}.sourceRecipe"
    )
    recipe["enabled"] = False
    recipe["rendererShape"] = ""
    recipe["emitterDelaySeconds"] = 0.0
    recipe["emitterDurationSeconds"] = 0.0
    recipe["emitterLoopCount"] = 1
    recipe["bursts"] = []
    recipe["modules"] = []


def _set_color(
    element: JsonObject,
    multiply: list[float],
    *,
    end_multiply: list[float] | None = None,
    emissive: float | None = None,
    clip: float | None = None,
) -> None:
    detail = _detail(element)
    color = _require_object(detail.get("color"), "detail.color")
    color["multiply"] = list(multiply)
    if emissive is not None:
        color["emissiveIntensity"] = float(emissive)
    if clip is not None:
        color["clip"] = float(clip)
    if end_multiply is not None:
        lerp = _require_object(detail.get("linearLerp"), "detail.linearLerp")
        lerp["colorMultiply"] = True
        lerp["endColorMultiply"] = list(end_multiply)


def _set_render_profile(element: JsonObject, profile: str) -> None:
    material = _require_object(element.get("material"), f"{element.get('id')}.material")
    material["renderProfile"] = profile


def _set_decal_size(element: JsonObject, x: float, y: float | None = None) -> None:
    decal = _require_object(_detail(element).get("decal"), "detail.decal")
    decal["size"] = [float(x), float(x if y is None else y)]


def _set_transform_scale(element: JsonObject, value: float) -> None:
    transform = _require_object(_detail(element).get("transform"), "detail.transform")
    transform["scale"] = [float(value), float(value), float(value)]


def _set_uv_speed(element: JsonObject, u: float, v: float) -> None:
    uv = _require_object(_detail(element).get("uv"), "detail.uv")
    uv["speed"] = [float(u), float(v)]
    uv["loop"] = True


def _set_attachment(element: JsonObject, runtime_slot: str) -> None:
    attachment = _require_object(
        element.get("actionCueAttachment"), f"{element.get('id')}.actionCueAttachment"
    )
    attachment["enabled"] = True
    attachment["follow"] = True
    attachment["sourceAnchorSlotId"] = "b_wp_r_01"
    attachment["runtimeAnchorSlotId"] = runtime_slot
    attachment["runtimeBoneName"] = "b_wp_r_01"


def _set_left_hand_attachment(element: JsonObject, runtime_slot: str) -> None:
    attachment = _require_object(
        element.get("actionCueAttachment"), f"{element.get('id')}.actionCueAttachment"
    )
    attachment["enabled"] = True
    attachment["follow"] = True
    attachment["sourceAnchorSlotId"] = "bip001-l-hand"
    attachment["runtimeAnchorSlotId"] = runtime_slot
    attachment["runtimeBoneName"] = "bip001-l-hand"


def _avoid_dissolve(element: JsonObject) -> None:
    _remove_resource(element, "dissolve")
    timing = _require_object(_detail(element).get("timing"), "detail.timing")
    timing["dissolveStartNormalized"] = 1.0


def _mutate(*functions: Mutator) -> Mutator:
    def apply(element: JsonObject) -> None:
        for function in functions:
            function(element)

    return apply


def _append_preserving(
    document: JsonObject, generated: list[JsonObject], label: str
) -> int:
    elements = _require_list(document.get("elements"), f"{label}.elements")
    old_elements = copy.deepcopy(elements)
    existing = _indexed(elements, "id", f"{label}.elements")
    generated_index = _indexed(generated, "id", f"{label}.generated")
    appended = 0
    for generated_id, row in generated_index.items():
        if generated_id in existing:
            # Generated rows become user-owned as soon as they exist.  This is
            # what keeps Effect Tool tuning intact on later Apply/Refresh runs.
            continue
        elements.append(copy.deepcopy(row))
        existing[generated_id] = elements[-1]
        appended += 1
    if elements[: len(old_elements)] != old_elements:
        raise AuthoringError(f"append projection mutated an existing element: {label}")
    return appended


def _repair_generated_parser_metadata(document: JsonObject, label: str) -> None:
    """Apply the current executable's non-visual authored metadata contract.

    Early generated rows used ``project-authored-copy:``.  The executable
    intentionally recognizes only ``authored-copy:`` as a direct manual
    particle carrier, so ring/even spawn layouts were then rejected as an
    invalid range.  Resource override rows also require an explicit
    ``compilerAssetId`` field even when no compiler baseline exists.  This
    migration changes only those parser metadata fields and leaves every live
    Effect Detail/resource/color/timing value untouched.
    """

    elements = _require_list(document.get("elements"), f"{label}.elements")
    for raw in elements:
        row = _require_object(raw, f"{label} element")
        element_id = row.get("id")
        if not isinstance(element_id, str) or not element_id.startswith(
            GENERATED_PREFIX
        ):
            continue
        source_node = row.get("sourceNode")
        if isinstance(source_node, str) and source_node.startswith(
            "project-authored-copy:"
        ):
            row["sourceNode"] = "authored-copy:" + source_node.removeprefix(
                "project-authored-copy:"
            )
        overrides = row.get("authoringOverrides")
        if not isinstance(overrides, dict):
            continue
        override_resources = overrides.get("resources")
        if not isinstance(override_resources, list):
            continue
        for override_raw in override_resources:
            override = _require_object(
                override_raw, f"{label}/{element_id} resource override"
            )
            override.setdefault("compilerAssetId", "")


def _stage_target(
    target: Target,
    generated: list[JsonObject],
    guards: dict[Path, bytes | None],
) -> tuple[JsonObject, int]:
    current = _load_optional(target.relative_path, guards)
    document = _new_effect_document(target) if current is None else copy.deepcopy(current)
    _validate_effect_document(document, target.effect_asset_id, target.effect_asset_id)
    _repair_generated_parser_metadata(document, target.effect_asset_id)
    appended = _append_preserving(document, generated, target.effect_asset_id)
    return document, appended


def _sector_element(
    donor: JsonObject,
    generated_id: str,
    role: str,
    yaw: float,
    *,
    mask: str = SECTOR_04,
    yellow: bool = False,
    start: float = 0.0,
) -> JsonObject:
    def tune(row: JsonObject) -> None:
        _set_resource(row, "mask", mask)
        _set_yaw(row, yaw)
        _set_start(row, start)
        _set_life(row, 5.0)
        _avoid_dissolve(row)
        if yellow:
            _set_color(
                row,
                [0.82, 0.36, 0.015, 0.92],
                end_multiply=[1.25, 0.58, 0.02, 1.30],
                emissive=1.0,
            )

    return _clone(donor, SECTOR_RED_ID, generated_id, role, tune)


def _terrain_elements(
    donor: JsonObject,
    land: JsonObject,
    suffix: str,
    yaw_offset: float,
) -> list[JsonObject]:
    result = [
        _sector_element(
            donor,
            f"{GENERATED_PREFIX}terrain-{suffix}.semicircle.sector-{index + 1:02d}",
            f"terrain {suffix} red semicircle sector {index + 1}",
            yaw + yaw_offset,
            start=0.0,
        )
        for index, yaw in enumerate((-60.0, 0.0, 60.0))
    ]
    land_rows = _require_list(land.get("elements"), "terrain land.elements")
    for index, source in enumerate(land_rows):
        source_row = _require_object(source, "terrain land element")
        source_id = source_row.get("id")
        if not isinstance(source_id, str):
            raise AuthoringError("terrain land donor element has no stable id")
        original = float(_detail(source_row)["timing"]["startDelaySeconds"])
        result.append(
            _clone(
                land,
                source_id,
                f"{GENERATED_PREFIX}terrain-{suffix}.landing.{index + 1:02d}",
                f"terrain {suffix} folded landing layer {index + 1}",
                lambda row, value=original: _set_start(row, value),
            )
        )
    return result


def _six_pizza_elements(arena: JsonObject, floor: JsonObject) -> list[JsonObject]:
    roar_start = 6.2
    sector_start = 22.6
    landing_start = 28.6

    def tune_original_ring(row: JsonObject) -> None:
        _set_start(row, roar_start)

    result = [
        _clone(
            arena,
            RING_DARK_ID,
            f"{GENERATED_PREFIX}six-pizza.ring.original-dark",
            "six pizza original dark roar ring",
            tune_original_ring,
        ),
        _clone(
            arena,
            RING_CYAN_ID,
            f"{GENERATED_PREFIX}six-pizza.ring.original-cyan",
            "six pizza original cyan roar ring",
            tune_original_ring,
        ),
    ]

    def tune_separated_dark(row: JsonObject) -> None:
        _set_start(row, roar_start)
        _set_render_profile(row, "alpha_two_sided_depth_read")
        _set_decal_size(row, 32.0)
        _set_position_y(row, 0.021)
        _set_color(row, [0.008, 0.008, 0.008, 5.4], emissive=0.35, clip=0.15)

    def tune_separated_cyan(row: JsonObject) -> None:
        _set_start(row, roar_start)
        _set_render_profile(row, "additive_two_sided_depth_read")
        _set_decal_size(row, 24.5)
        _set_position_y(row, 0.037)
        _set_color(row, [0.0, 2.61, 1.72, 2.85], emissive=2.4, clip=0.03)

    result.extend(
        [
            _clone(
                arena,
                RING_DARK_ID,
                f"{GENERATED_PREFIX}six-pizza.ring.separated-dark",
                "six pizza separated dark comparison ring",
                tune_separated_dark,
            ),
            _clone(
                arena,
                RING_CYAN_ID,
                f"{GENERATED_PREFIX}six-pizza.ring.separated-cyan",
                "six pizza separated cyan comparison ring",
                tune_separated_cyan,
            ),
            _sector_element(
                floor,
                f"{GENERATED_PREFIX}six-pizza.sector.red-04",
                "six pizza red sector 04",
                0.0,
                mask=SECTOR_04,
                start=sector_start,
            ),
            _sector_element(
                floor,
                f"{GENERATED_PREFIX}six-pizza.sector.yellow-05",
                "six pizza yellow sector 05",
                0.0,
                mask=SECTOR_05,
                yellow=True,
                start=sector_start,
            ),
        ]
    )

    def tune_red_overlay(row: JsonObject) -> None:
        _set_start(row, sector_start)
        _set_resource(row, "mask", SECTOR_04)
        _set_render_profile(row, "alpha_two_sided_depth_read")
        _set_decal_size(row, 28.0)
        _set_position_y(row, 0.052)
        _set_color(row, [0.72, 0.012, 0.006, 1.05], emissive=1.25, clip=0.02)

    def tune_center_ring(row: JsonObject) -> None:
        _set_start(row, sector_start)
        _set_decal_size(row, 4.0)
        _set_position_y(row, 0.065)
        _set_transform_scale(row, 0.02)
        _set_color(row, [0.0, 3.8, 2.8, 4.8], emissive=5.0, clip=0.0)

    def tune_landing_ring(row: JsonObject) -> None:
        _set_start(row, landing_start)
        _set_decal_size(row, 30.0)
        _set_position_y(row, 0.045)
        _set_color(row, [0.0, 2.9, 2.05, 3.8], emissive=3.0, clip=0.0)

    result.extend(
        [
            _clone(
                arena,
                RING_CYAN_ID,
                f"{GENERATED_PREFIX}six-pizza.sector.red-roar-overlay",
                "six pizza red masked roar overlay",
                tune_red_overlay,
            ),
            _clone(
                arena,
                RING_CYAN_ID,
                f"{GENERATED_PREFIX}six-pizza.ring.center-emissive",
                "six pizza center emissive boundary ring",
                tune_center_ring,
            ),
            _clone(
                arena,
                RING_CYAN_ID,
                f"{GENERATED_PREFIX}six-pizza.ring.landing-cyan",
                "six pizza landing cyan ring",
                tune_landing_ring,
            ),
        ]
    )
    return result


def _target_cone_fan_elements(
    target_cone: JsonObject,
    id_prefix: str,
    role_prefix: str,
    *,
    start: float,
    life: float,
    scale: float,
) -> list[JsonObject]:
    tuning = (
        ("fan-mesh", [0.02, 0.10, 0.085, 1.8], 0.45),
        ("electric-core", [0.0, 3.4, 2.65, 4.0], 3.8),
        ("forward-shine", [0.1, 4.6, 3.7, 4.8], 4.4),
    )
    result: list[JsonObject] = []
    for index, (donor_id, (role, color, emissive)) in enumerate(
        zip(TARGET_CONE_COMPOSITE_IDS, tuning), start=1
    ):
        result.append(
            _clone(
                target_cone,
                donor_id,
                f"{GENERATED_PREFIX}{id_prefix}.{index:02d}",
                f"{role_prefix} {role}",
                _mutate(
                    lambda row, value=start: _set_start(row, value),
                    lambda row, value=life: _set_life(row, value),
                    lambda row, value=scale: _set_transform_scale(row, value),
                    lambda row, value=color, energy=emissive: _set_color(
                        row, value, emissive=energy, clip=0.0
                    ),
                ),
            )
        )
    return result


def _attack_whirlwind_elements(
    four_slash: JsonObject,
    whirlwind: JsonObject,
    arena: JsonObject,
    target_cone: JsonObject,
    takeoff: JsonObject,
) -> list[JsonObject]:
    finish_starts = {
        FOUR_SLASH_FINISH_IDS[0]: 1.250,
        FOUR_SLASH_FINISH_IDS[1]: 1.364,
        FOUR_SLASH_FINISH_IDS[2]: 1.330,
    }
    result = [
        _clone(
            four_slash,
            donor_id,
            f"{GENERATED_PREFIX}attack-whirlwind.slam.{index + 1:02d}",
            f"jump slam finish layer {index + 1}",
            lambda row, start=finish_starts[donor_id]: _set_start(row, start),
        )
        for index, donor_id in enumerate(FOUR_SLASH_FINISH_IDS)
    ]
    for index, donor_id in enumerate(WHIRLWIND_IDS):
        donor_row = _element(whirlwind, donor_id, "whirlwind")
        original_start = float(_detail(donor_row)["timing"]["startDelaySeconds"])
        result.append(
            _clone(
                whirlwind,
                donor_id,
                f"{GENERATED_PREFIX}attack-whirlwind.spin.{index + 1:02d}",
                f"whirlwind copied layer {index + 1}",
                lambda row, start=3.833 + original_start: _set_start(row, start),
            )
        )

    decal_layout = (
        (1.180, 2.8, -2.3, 6.8),
        (1.220, 4.0, 0.0, 7.5),
        (1.260, 2.8, 2.3, 6.8),
    )
    for index, (start, x, z, size) in enumerate(decal_layout, start=1):
        result.append(
            _clone(
                arena,
                RING_CYAN_ID,
                f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.decal-{index:02d}",
                f"jump slam cyan fan decal {index}",
                _mutate(
                    lambda row, value=start: _set_start(row, value),
                    lambda row: _set_life(row, 0.8),
                    lambda row, px=x, pz=z: _set_position(row, px, 0.055, pz),
                    lambda row, value=size: _set_decal_size(row, value),
                    lambda row: _set_transform_scale(row, 0.03),
                    lambda row: _set_color(
                        row, [0.0, 3.5, 2.7, 4.3], emissive=4.0, clip=0.0
                    ),
                ),
            )
        )

    result.extend(
        _target_cone_fan_elements(
            target_cone,
            "attack-whirlwind.jump-fan.core",
            "jump slam electric fan",
            start=1.30,
            life=1.10,
            scale=1.90,
        )
    )
    result.append(
        _clone(
            takeoff,
            HIGH_JUMP_VERTICAL_ID,
            f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.vertical-core",
            "jump slam very large vertical cyan sprite",
            _mutate(
                lambda row: _set_start(row, 1.32),
                lambda row: _set_life(row, 0.95),
                lambda row: _set_color(
                    row, [0.12, 5.2, 4.15, 5.4], emissive=5.2, clip=0.0
                ),
                lambda row: _set_uv_speed(row, 0.0, -0.18),
            ),
        )
    )
    return result


def _axe_follow(row: JsonObject, slot: str, uv: tuple[float, float]) -> None:
    _set_attachment(row, slot)
    _set_uv_speed(row, *uv)


def _set_source_continuous_emission(
    element: JsonObject,
    *,
    duration: float,
    rate: float,
    initial_burst: int,
    max_particles: int,
    source_size_scale: float,
    source_lifetime_scale: float,
) -> None:
    """Turn an extracted Cascade burst into one bounded continuous emitter.

    Source-recipe playback evaluates ``particlemodulespawn.rate`` instead of
    the reconstructed Effect Detail fallback.  Keep both views synchronized so
    the Tool and runtime describe the same authored result.
    """

    particle = _require_object(_detail(element).get("particle"), "detail.particle")
    particle["maxParticles"] = int(max_particles)
    particle["spawnRatePerSecond"] = float(rate)
    particle["burstCount"] = int(initial_burst)
    particle["localSpace"] = True
    particle["billboard"] = True
    particle["sourceScale"] = {
        "count": 1.0,
        "size": float(source_size_scale),
        "lifeTime": float(source_lifetime_scale),
        "speed": 1.0,
        "rotation": 1.0,
        "alpha": 1.0,
        "spawnDelay": 1.0,
    }

    recipe = _require_object(
        element.get("sourceRecipe"), f"{element.get('id')}.sourceRecipe"
    )
    if recipe.get("enabled") is not True:
        raise AuthoringError(
            f"continuous source emission requires an enabled recipe: {element.get('id')}"
        )
    recipe["emitterDurationSeconds"] = round(float(duration), 6)
    recipe["emitterLoopCount"] = 1
    recipe["bursts"] = [
        {
            "timeSeconds": 0.0,
            "countMinimum": int(initial_burst),
            "countMaximum": int(initial_burst),
        }
    ]

    modules = _require_list(recipe.get("modules"), "sourceRecipe.modules")
    spawn_modules = [
        _require_object(raw, "sourceRecipe spawn module")
        for raw in modules
        if isinstance(raw, dict)
        and str(raw.get("className", "")).lower() == "particlemodulespawn"
    ]
    if len(spawn_modules) != 1:
        raise AuthoringError(
            f"expected one particlemodulespawn: {element.get('id')}, "
            f"got {len(spawn_modules)}"
        )
    spawn_module = spawn_modules[0]
    distributions = _require_list(
        spawn_module.get("distributions"), "particlemodulespawn.distributions"
    )
    rate_distributions = [
        _require_object(raw, "particlemodulespawn rate distribution")
        for raw in distributions
        if isinstance(raw, dict) and raw.get("propertyPath") == "rate"
    ]
    if len(rate_distributions) != 1:
        raise AuthoringError(
            f"expected one particlemodulespawn.rate distribution: "
            f"{element.get('id')}, got {len(rate_distributions)}"
        )
    rate_distribution = rate_distributions[0]
    lookup_table = _require_list(
        rate_distribution.get("lookupTable"), "particlemodulespawn.rate.lookupTable"
    )
    if len(lookup_table) != 4:
        raise AuthoringError(
            f"particlemodulespawn.rate must have one float4 lookup entry: "
            f"{element.get('id')}"
        )
    rate_distribution["lookupTable"] = [float(rate)] * 4
    rate_distribution["defaultMinimum"] = [float(rate)] * 4
    rate_distribution["defaultMaximum"] = [float(rate)] * 4

    literals = _require_list(
        spawn_module.get("literals"), "particlemodulespawn.literals"
    )
    for raw in literals:
        literal = _require_object(raw, "particlemodulespawn literal")
        if literal.get("propertyPath") == "burstlist[0].count":
            literal["value"] = float(initial_burst)


def _retune_charge_axe_particles(document: JsonObject) -> None:
    dark = _element(
        document,
        f"{GENERATED_PREFIX}charge.axe-aura.dark-smoke",
        CHARGE.effect_asset_id,
    )
    _set_life(dark, 1.067)
    _set_position(dark, 0.0, 0.12, -0.10)
    _set_color(dark, [0.015, 0.015, 0.015, 3.8], emissive=0.25, clip=0.0)
    _set_source_continuous_emission(
        dark,
        duration=0.82,
        rate=18.0,
        initial_burst=4,
        max_particles=28,
        source_size_scale=0.62,
        source_lifetime_scale=0.55,
    )

    cyan = _element(
        document,
        f"{GENERATED_PREFIX}charge.axe-aura.cyan-cloud",
        CHARGE.effect_asset_id,
    )
    _set_life(cyan, 1.067)
    _set_position(cyan, 0.0, 0.18, 0.10)
    _set_color(cyan, [0.0, 2.6, 2.0, 3.2], emissive=2.6, clip=0.0)
    _set_source_continuous_emission(
        cyan,
        duration=0.82,
        rate=14.0,
        initial_burst=6,
        max_particles=32,
        source_size_scale=0.55,
        source_lifetime_scale=0.35,
    )

    bright = _element(
        document,
        f"{GENERATED_PREFIX}charge.axe-aura.bright-flame",
        CHARGE.effect_asset_id,
    )
    _set_life(bright, 1.067)


def _charge_elements(
    whirlwind: JsonObject, native: JsonObject, target_cone: JsonObject
) -> list[JsonObject]:
    result = [
        _clone(
            whirlwind,
            WHIRLWIND_IDS[0],
            f"{GENERATED_PREFIX}charge.slash.dark",
            "gather slash dark axe-follow mesh",
            lambda row: _axe_follow(row, "requested.charge.slash.dark", (0.18, -0.06)),
        ),
        _clone(
            whirlwind,
            WHIRLWIND_IDS[1],
            f"{GENERATED_PREFIX}charge.slash.bright-cyan",
            "gather slash bright cyan axe-follow mesh",
            _mutate(
                lambda row: _axe_follow(
                    row, "requested.charge.slash.bright", (-0.22, 0.10)
                ),
                lambda row: _set_color(
                    row, [0.35, 5.4, 4.35, 6.0], emissive=5.5, clip=0.0
                ),
            ),
        ),
    ]
    native_tuning = {
        AXE_AURA_IDS[0]: ("dark-smoke", (0.12, -0.08), [0.01, 0.025, 0.02, 4.8]),
        AXE_AURA_IDS[1]: ("cyan-cloud", (-0.16, 0.11), [0.0, 2.8, 2.1, 3.6]),
        AXE_AURA_IDS[2]: ("bright-flame", (0.30, 0.0), [0.3, 5.2, 4.2, 5.5]),
    }
    for index, donor_id in enumerate(AXE_AURA_IDS):
        role, uv, color = native_tuning[donor_id]
        result.append(
            _clone(
                native,
                donor_id,
                f"{GENERATED_PREFIX}charge.axe-aura.{role}",
                f"gather slash {role} axe-follow sprite",
                _mutate(
                    lambda row, slot=f"requested.charge.aura.{index + 1:02d}", speed=uv: _axe_follow(
                        row, slot, speed
                    ),
                    lambda row, value=color: _set_color(row, value, emissive=3.0),
                ),
            )
        )
    cone_tuning = (
        ("fan-mesh", [0.02, 0.10, 0.085, 1.8], 0.45),
        ("electric-core", [0.0, 3.4, 2.65, 4.0], 3.8),
        ("forward-shine", [0.1, 4.6, 3.7, 4.8], 4.4),
    )
    for index, (donor_id, (role, color, emissive)) in enumerate(
        zip(TARGET_CONE_COMPOSITE_IDS, cone_tuning)
    ):
        result.append(
            _clone(
                target_cone,
                donor_id,
                f"{GENERATED_PREFIX}charge.target-cone.{index + 1:02d}",
                f"gather slash folded target cone {role}",
                _mutate(
                    lambda row: _set_start(row, 0.12),
                    lambda row: _set_life(row, 0.9),
                    lambda row, value=color, energy=emissive: _set_color(
                        row, value, emissive=energy, clip=0.0
                    ),
                ),
            )
        )
    return result


def _charge2_elements(floor: JsonObject, whirlwind: JsonObject) -> list[JsonObject]:
    result = [
        _sector_element(
            floor,
            f"{GENERATED_PREFIX}charge2.red-fan.sector-{index + 1:02d}",
            f"charge2 red fan sector {index + 1}",
            yaw,
        )
        for index, yaw in enumerate((-60.0, 0.0, 60.0))
    ]

    def tune_red_slash(row: JsonObject) -> None:
        _axe_follow(row, "requested.charge2.slash.red", (-0.22, 0.10))
        _set_color(row, [5.2, 0.10, 0.025, 5.8], emissive=5.0, clip=0.0)

    result.append(
        _clone(
            whirlwind,
            WHIRLWIND_IDS[1],
            f"{GENERATED_PREFIX}charge2.slash.bright-red",
            "charge2 bright red axe-follow slash",
            tune_red_slash,
        )
    )
    return result


def _roar_charge_elements(
    arena: JsonObject, cone: JsonObject, sky_axe: JsonObject
) -> list[JsonObject]:
    roar_start = 0.74
    upward_impact_start = 7.063

    def tune_roar_ring(row: JsonObject) -> None:
        _set_start(row, roar_start)

    result = [
        _clone(
            arena,
            RING_DARK_ID,
            f"{GENERATED_PREFIX}roar-charge.ring.dark",
            "roar charge dark ring",
            tune_roar_ring,
        ),
        _clone(
            arena,
            RING_CYAN_ID,
            f"{GENERATED_PREFIX}roar-charge.ring.cyan",
            "roar charge cyan ring",
            tune_roar_ring,
        ),
    ]

    def tune_cone_dark(row: JsonObject) -> None:
        _set_start(row, roar_start)
        _set_render_profile(row, "alpha_two_sided_depth_read")
        _set_color(row, [0.008, 0.018, 0.016, 5.0], emissive=0.4, clip=0.10)

    def tune_cone_cyan(row: JsonObject) -> None:
        _set_start(row, roar_start)
        _set_render_profile(row, "additive_two_sided_depth_read")
        _set_color(row, [0.10, 4.2, 3.25, 4.5], emissive=4.0, clip=0.01)

    result.extend(
        [
            _clone(
                cone,
                CONE_ID,
                f"{GENERATED_PREFIX}roar-charge.cone.dark",
                "roar charge dark cone comparison",
                tune_cone_dark,
            ),
            _clone(
                cone,
                CONE_ID,
                f"{GENERATED_PREFIX}roar-charge.cone.cyan",
                "roar charge cyan cone comparison",
                tune_cone_cyan,
            ),
            _clone(
                sky_axe,
                SKY_AXE_WAVE_ID,
                f"{GENERATED_PREFIX}roar-charge.upward.wave",
                "roar charge upward gather wave",
                lambda row: _set_start(row, upward_impact_start),
            ),
        ]
    )
    return result


def _three_elements(arena: JsonObject, sky_axe: JsonObject) -> list[JsonObject]:
    starts = (1.617, 2.763, 4.191)
    result: list[JsonObject] = []
    for index, start in enumerate(starts[:2]):
        result.append(
            _clone(
                arena,
                RING_CYAN_ID,
                f"{GENERATED_PREFIX}three.impact-{index + 1:02d}.cyan-ring",
                f"three smash impact {index + 1} cyan ring",
                _mutate(
                    lambda row, value=start: _set_start(row, value),
                    lambda row: _set_decal_size(row, 11.0),
                ),
            )
        )
    for index, start in enumerate(starts):
        result.append(
            _clone(
                sky_axe,
                SKY_AXE_WAVE_ID,
                f"{GENERATED_PREFIX}three.impact-{index + 1:02d}.sky-wave",
                f"three smash impact {index + 1} sky-axe wave",
                lambda row, value=start: _set_start(row, value),
            )
        )
    return result


def _small_ring(row: JsonObject, start: float) -> None:
    _set_start(row, start)
    _set_decal_size(row, 4.2)
    _set_transform_scale(row, 0.04)
    _set_position_y(row, 0.055)
    _set_color(row, [0.0, 3.4, 2.5, 4.2], emissive=4.0, clip=0.0)


def _front_back_elements(
    front_back: JsonObject,
    arena: JsonObject,
    sky_axe: JsonObject,
    target_cone: JsonObject,
) -> list[JsonObject]:
    starts = {
        FRONT_BACK_IDS[0]: 2.25,
        FRONT_BACK_IDS[1]: 0.45,
        FRONT_BACK_IDS[2]: 0.45,
        FRONT_BACK_IDS[3]: 1.35,
        FRONT_BACK_IDS[4]: 1.35,
    }
    result = [
        _clone(
            front_back,
            donor_id,
            f"{GENERATED_PREFIX}front-back-front.source.{index + 1:02d}",
            f"front-back-front crack/electric source {index + 1}",
            lambda row, value=starts[donor_id]: _set_start(row, value),
        )
        for index, donor_id in enumerate(FRONT_BACK_IDS)
    ]
    blast_starts = (0.45, 1.35, 2.25)
    for index, start in enumerate(blast_starts):
        result.extend(
            [
                _clone(
                    arena,
                    RING_CYAN_ID,
                    f"{GENERATED_PREFIX}front-back-front.blast-{index + 1:02d}.ring",
                    f"front-back-front small ring blast {index + 1}",
                    lambda row, value=start: _small_ring(row, value),
                ),
                _clone(
                    sky_axe,
                    SKY_AXE_WAVE_ID,
                    f"{GENERATED_PREFIX}front-back-front.blast-{index + 1:02d}.wave-fallback",
                    f"front-back-front fallback wave {index + 1}",
                    lambda row, value=start: _set_start(row, value),
                ),
            ]
        )
        result.extend(
            [
                _clone(
                    front_back,
                    FRONT_BACK_SHORT_BLAST_IDS[0],
                    f"{GENERATED_PREFIX}front-back-front.blast-{index + 1:02d}.cyan-burst",
                    f"front-back-front large cyan sprite burst {index + 1}",
                    _mutate(
                        lambda row, value=start: _set_start(row, value),
                        lambda row: _set_life(row, 0.55),
                        lambda row: _set_transform_scale(row, 1.8),
                        lambda row: _set_color(
                            row,
                            [0.15, 5.2, 4.25, 5.6],
                            emissive=5.0,
                            clip=0.0,
                        ),
                    ),
                ),
                _clone(
                    front_back,
                    FRONT_BACK_SHORT_BLAST_IDS[1],
                    f"{GENERATED_PREFIX}front-back-front.blast-{index + 1:02d}.vertical-spark",
                    f"front-back-front vertical fallback sprite {index + 1}",
                    _mutate(
                        lambda row, value=start: _set_start(row, value),
                        lambda row: _set_life(row, 0.32),
                        lambda row: _set_transform_scale(row, 1.45),
                        lambda row: _set_color(
                            row,
                            [0.05, 4.5, 3.65, 4.8],
                            emissive=4.2,
                            clip=0.0,
                        ),
                    ),
                ),
            ]
        )
    result.extend(
        _target_cone_fan_elements(
            target_cone,
            "front-back-front.fan",
            "front-back-front broad electric fan",
            start=0.30,
            life=4.10,
            scale=2.05,
        )
    )
    return result


def _counter_elements(arena: JsonObject) -> list[JsonObject]:
    return [
        _clone(
            arena,
            RING_CYAN_ID,
            f"{GENERATED_PREFIX}counter.cyan-roar-ring",
            "counter downward smash cyan roar ring",
            _mutate(
                lambda row: _set_start(row, 0.9),
                lambda row: _set_decal_size(row, 24.0),
                lambda row: _set_color(
                    row,
                    [0.0, 3.35, 2.45, 4.15],
                    emissive=4.0,
                    clip=0.0,
                ),
            ),
        )
    ]


def _portal_rush_forward_element(portal_rush: JsonObject) -> JsonObject:
    return _clone(
        portal_rush,
        PORTAL_RUSH_FORWARD_MESH_ID,
        f"{GENERATED_PREFIX}warp.portal-rush.forward-mesh",
        "portal rush folded forward mesh after three rolls",
        _mutate(
            lambda row: _set_start(row, 0.0),
            lambda row: _set_life(row, 0.41),
            lambda row: _set_transform_scale(row, 1.15),
            lambda row: _set_color(
                row,
                [0.015, 3.4, 2.75, 3.9],
                emissive=3.2,
                clip=0.02,
            ),
            lambda row: _set_uv_speed(row, 0.22, -0.08),
        ),
    )


def _portal_elements(portal: JsonObject, portal_rush: JsonObject) -> list[JsonObject]:
    elements = _require_list(portal.get("elements"), "portal.elements")
    result: list[JsonObject] = []
    for index, source in enumerate(elements):
        source_id = _require_object(source, "portal element").get("id")
        if not isinstance(source_id, str):
            raise AuthoringError("portal donor element has no stable id")
        result.append(
            _clone(
                portal,
                source_id,
                f"{GENERATED_PREFIX}warp.portal.{index + 1:02d}",
                f"portal exact layer {index + 1:02d}",
            )
        )
    if len(result) != 14:
        raise AuthoringError(f"portal donor closure drifted: expected 14, got {len(result)}")
    result.append(_portal_rush_forward_element(portal_rush))
    return result


def _retune_warp_leg_composite(document: JsonObject) -> None:
    """Fold one portal opening and its forward rush mesh into one leg Product."""

    portal_rows = [
        _element(
            document,
            f"{GENERATED_PREFIX}warp.portal.{index:02d}",
            WARP_PORTAL.effect_asset_id,
        )
        for index in range(1, 15)
    ]
    starts = {
        round(float(_detail(row)["timing"]["startDelaySeconds"]), 6)
        for row in portal_rows
    }
    if starts == {0.750337, 0.813559}:
        offset = 0.750337
    elif starts == {0.0, 0.063222}:
        offset = 0.0
    else:
        raise AuthoringError(
            "warp portal timing drifted; refusing destructive retime: "
            + repr(sorted(starts))
        )
    for row in portal_rows:
        current = float(_detail(row)["timing"]["startDelaySeconds"])
        _set_start(row, round(current - offset, 6))

    forward = _element(
        document,
        f"{GENERATED_PREFIX}warp.portal-rush.forward-mesh",
        WARP_PORTAL.effect_asset_id,
    )
    _set_start(forward, 0.0)
    _set_life(forward, 0.41)


def _struggling_elements(
    portal_layers: list[JsonObject],
    front_back_product: JsonObject,
    arena: JsonObject,
    fist: JsonObject,
    takeoff: JsonObject,
    imprison_roar: JsonObject,
) -> list[JsonObject]:
    """Build one 27.766s Product for the complete Phase 3 struggle sequence."""

    result: list[JsonObject] = []

    # STEP_01..03: preserve the exact tuned Portal Product closure, including
    # the forward mesh that appears after the three-roll rush lead-in.
    for index, source in enumerate(portal_layers, start=1):
        result.append(
            _clone_row(
                source,
                WARP_PORTAL.effect_asset_id,
                f"{GENERATED_PREFIX}struggling.portal.{index:02d}",
                f"phase 3 struggle portal layer {index:02d}",
            )
        )

    # STEP_04 begins at 4.167s.  Copy the five user-visible broad fan rows from
    # the already tuned one-Product front/back/front effect and offset them into
    # this absolute timeline.
    fan_offset = 4.167
    for index in range(1, 6):
        source_id = f"{GENERATED_PREFIX}front-back-front.source.{index:02d}"
        source = _element(
            front_back_product,
            source_id,
            FRONT_BACK_FRONT.effect_asset_id,
        )
        original = float(_detail(source)["timing"]["startDelaySeconds"])
        result.append(
            _clone_row(
                source,
                FRONT_BACK_FRONT.effect_asset_id,
                f"{GENERATED_PREFIX}struggling.fan.{index:02d}",
                f"phase 3 struggle broad fan layer {index:02d}",
                lambda row, value=fan_offset + original: _set_start(row, value),
            )
        )

    result.append(
        _clone(
            arena,
            RING_CYAN_ID,
            f"{GENERATED_PREFIX}struggling.fist-smash.cyan-ring",
            "phase 3 fist smash resized cyan roar decal",
            _mutate(
                lambda row: _set_start(row, 14.682),
                lambda row: _set_life(row, 1.40),
                lambda row: _set_decal_size(row, 14.0),
                lambda row: _set_position(row, 0.0, 0.055, 0.0),
                lambda row: _set_transform_scale(row, 0.035),
                lambda row: _set_color(
                    row, [0.0, 3.8, 2.9, 4.6], emissive=4.4, clip=0.0
                ),
            ),
        )
    )

    # The reference ultimately uses a ring of roughly ten upright emissions.
    # This requested revision intentionally authors one isolated prototype so
    # its orientation and growth can be tuned before multiplication.
    result.append(
        _clone(
            fist,
            FIST_RING_SPRITE_ID,
            f"{GENERATED_PREFIX}struggling.radial-burst.prototype-01",
            "phase 3 one-of-ten upright radial ring prototype",
            _mutate(
                lambda row: _set_start(row, 17.75),
                lambda row: _set_life(row, 1.05),
                lambda row: _set_position(row, 5.5, 1.5, 0.0),
                lambda row: _set_rotation(row, 0.0, 90.0, 0.0),
                lambda row: _set_transform_scale(row, 1.0),
                lambda row: _set_particle_sizes(row, (0.05, 0.08), (5.5, 7.5)),
                lambda row: _set_single_particle_burst(row, 0.85),
                lambda row: _set_render_profile(
                    row, "additive_two_sided_depth_read"
                ),
                lambda row: _set_color(
                    row, [0.05, 4.8, 3.75, 5.0], emissive=4.6, clip=0.0
                ),
            ),
        )
    )

    result.append(
        _clone(
            takeoff,
            HIGH_JUMP_VERTICAL_ID,
            f"{GENERATED_PREFIX}struggling.large-vertical-burst",
            "phase 3 large vertical cyan sprite burst",
            _mutate(
                lambda row: _set_start(row, 19.20),
                lambda row: _set_life(row, 1.30),
                lambda row: _set_color(
                    row, [0.15, 5.4, 4.25, 5.7], emissive=5.4, clip=0.0
                ),
                lambda row: _set_uv_speed(row, 0.0, -0.16),
            ),
        )
    )

    result.append(
        _clone(
            fist,
            FIST_DONUT_GROW_ID,
            f"{GENERATED_PREFIX}struggling.donut.cyan-grow",
            "phase 3 growing cyan donut boundary",
            _mutate(
                lambda row: _set_start(row, 20.40),
                lambda row: _set_life(row, 1.70),
                lambda row: _set_position(row, 0.0, 0.24, 0.0),
                lambda row: _set_render_profile(
                    row, "additive_two_sided_depth_read"
                ),
                lambda row: _set_color(
                    row, [0.02, 4.2, 3.25, 4.6], emissive=4.2, clip=0.0
                ),
            ),
        )
    )

    # STEP_10 roar-end wave occurs at 21.407s.  The exact imprison-roar donor
    # is a fifteen-row authored composition (gather, eye, shout, and wave), so
    # all rows stay in this same Product instead of being split into subeffects.
    roar_start = 21.407
    roar_rows = _require_list(imprison_roar.get("elements"), "imprison roar.elements")
    if len(roar_rows) != 15:
        raise AuthoringError(
            f"imprison roar donor closure drifted: expected 15, got {len(roar_rows)}"
        )
    for index, raw in enumerate(roar_rows, start=1):
        source = _require_object(raw, "imprison roar element")
        original = float(_detail(source)["timing"]["startDelaySeconds"])
        result.append(
            _clone_row(
                source,
                str(imprison_roar.get("effectAssetId", "imprison-roar")),
                f"{GENERATED_PREFIX}struggling.final-roar.{index:02d}",
                f"phase 3 final roar exact layer {index:02d}",
                lambda row, value=roar_start + original: _set_start(row, value),
            )
        )

    if len(result) != 39:
        raise AuthoringError(
            f"phase 3 struggle closure drifted: expected 39, got {len(result)}"
        )
    return result


def _trash_elements(
    hand: JsonObject, front_back: JsonObject, recovery: JsonObject
) -> list[JsonObject]:
    result: list[JsonObject] = []
    # One STEP_01 cue owns the entire composition.  6.4s is the cumulative
    # start of the split pattern's STEP_06 counter windup.
    for index, donor_id in enumerate(HAND_CORE_IDS):
        donor_row = _element(hand, donor_id, "trash hand donor")
        original = float(_detail(donor_row)["timing"]["startDelaySeconds"])
        result.append(
            _clone(
                hand,
                donor_id,
                f"{GENERATED_PREFIX}trash.hand-core.{index + 1:02d}",
                f"trash hand core layer {index + 1}",
                _mutate(
                    lambda row, value=6.4 + original: _set_start(row, value),
                    lambda row, slot=f"requested.trash.left-hand.{index + 1:02d}": _set_left_hand_attachment(
                        row, slot
                    ),
                ),
            )
        )
    floor_ids = FRONT_BACK_IDS[:4]
    # STEP_03 begins after STEP_01 + STEP_02 (2.2s).  These offsets leave the
    # copied floor/electric layers visibly sequential around that landing.
    floor_starts = (2.20, 2.20, 2.30, 2.40)
    for index, (donor_id, start) in enumerate(zip(floor_ids, floor_starts)):
        result.append(
            _clone(
                front_back,
                donor_id,
                f"{GENERATED_PREFIX}trash.floor.{index + 1:02d}",
                f"trash floor crack/electric layer {index + 1}",
                lambda row, value=start: _set_start(row, value),
            )
        )
    recovery_row = _element(recovery, HAND_RECOVERY_SMOKE_ID, "trash recovery donor")
    recovery_start = float(_detail(recovery_row)["timing"]["startDelaySeconds"])
    result.append(
        _clone(
            recovery,
            HAND_RECOVERY_SMOKE_ID,
            f"{GENERATED_PREFIX}trash.recovery-smoke",
            "trash capture/recovery smoke",
            _mutate(
                lambda row, value=10.5 + recovery_start: _set_start(row, value),
                lambda row: _set_left_hand_attachment(
                    row, "requested.trash.left-hand.recovery-smoke"
                ),
            ),
        )
    )
    return result


def _hand_core_sequence(
    hand: JsonObject,
    id_prefix: str,
    role_prefix: str,
    *,
    start_offset: float = 0.0,
) -> list[JsonObject]:
    result: list[JsonObject] = []
    for index, donor_id in enumerate(HAND_CORE_IDS):
        donor_row = _element(hand, donor_id, role_prefix)
        original = float(_detail(donor_row)["timing"]["startDelaySeconds"])
        result.append(
            _clone(
                hand,
                donor_id,
                f"{GENERATED_PREFIX}{id_prefix}.hand-core.{index + 1:02d}",
                f"{role_prefix} hand aura layer {index + 1}",
                _mutate(
                    lambda row, value=start_offset + original: _set_start(row, value),
                    lambda row, slot=f"requested.{id_prefix}.left-hand.{index + 1:02d}": _set_left_hand_attachment(
                        row, slot
                    ),
                    lambda row, direction=(-0.10 + index * 0.035, 0.08 - index * 0.02): _set_uv_speed(
                        row, *direction
                    ),
                ),
            )
        )
    return result


def _trash_catch_if_elements(hand: JsonObject) -> list[JsonObject]:
    return _hand_core_sequence(
        hand,
        "trash-catch-if",
        "trash catch branch",
    )


def _trash_catch_success_elements(
    hand: JsonObject, roar: JsonObject, recovery: JsonObject
) -> list[JsonObject]:
    result = _hand_core_sequence(
        hand,
        "trash-catch-success",
        "trash catch success",
    )
    for index, donor_id in enumerate(HAND_ROAR_IDS):
        result.append(
            _clone(
                roar,
                donor_id,
                f"{GENERATED_PREFIX}trash-catch-success.release-roar.{index + 1:02d}",
                f"trash catch success cyan release roar {index + 1}",
                _mutate(
                    lambda row: _set_start(row, 2.35),
                    lambda row: _set_life(row, 0.8),
                    lambda row: _set_color(
                        row,
                        [0.02, 4.3, 3.45, 4.8],
                        emissive=4.0,
                        clip=0.0,
                    ),
                ),
            )
        )
    result.append(
        _clone(
            recovery,
            HAND_RECOVERY_SMOKE_ID,
            f"{GENERATED_PREFIX}trash-catch-success.release-smoke",
            "trash catch success black cyan release smoke",
            _mutate(
                lambda row: _set_start(row, 3.0),
                lambda row: _set_life(row, 0.8),
                lambda row: _set_uv_speed(row, -0.16, 0.12),
                lambda row: _set_color(
                    row,
                    [0.01, 0.36, 0.31, 3.8],
                    emissive=0.9,
                    clip=0.06,
                ),
                lambda row: _set_left_hand_attachment(
                    row, "requested.trash-catch-success.left-hand.release-smoke"
                ),
            ),
        )
    )
    return result


def _trash_catch_fail_elements(recovery: JsonObject) -> list[JsonObject]:
    result: list[JsonObject] = []
    for index, donor_id in enumerate(
        (HAND_RECOVERY_SMOKE_ID, HAND_RECOVERY_FRAGMENT_ID, HAND_RECOVERY_RING_ID)
    ):
        result.append(
            _clone(
                recovery,
                donor_id,
                f"{GENERATED_PREFIX}trash-catch-fail.release.{index + 1:02d}",
                f"trash catch fail dark release {index + 1}",
                _mutate(
                    lambda row, value=0.08 + index * 0.03: _set_start(row, value),
                    lambda row: _set_life(row, 0.62),
                    lambda row: _set_color(
                        row,
                        [0.006, 0.10, 0.085, 2.8],
                        emissive=0.35,
                        clip=0.08,
                    ),
                ),
            )
        )
    return result


def _catch_breath_elements(
    hand: JsonObject,
    cone: JsonObject,
    roar: JsonObject,
    recovery: JsonObject,
) -> list[JsonObject]:
    result: list[JsonObject] = []
    for index, donor_id in enumerate(HAND_CORE_IDS[:2]):
        result.append(
            _clone(
                hand,
                donor_id,
                f"{GENERATED_PREFIX}catch-breath.hand-charge.{index + 1:02d}",
                f"catch breath left-hand charge {index + 1}",
                _mutate(
                    lambda row: _set_start(row, 0.0),
                    lambda row, slot=f"requested.catch-breath.left-hand.{index + 1:02d}": _set_left_hand_attachment(
                        row, slot
                    ),
                    lambda row, speed=(-0.18 + index * 0.34, 0.11 - index * 0.20): _set_uv_speed(
                        row, *speed
                    ),
                ),
            )
        )

    def tune_breath_dark(row: JsonObject) -> None:
        _set_start(row, 2.25)
        _set_life(row, 4.1)
        _set_transform_scale(row, 1.65)
        _set_render_profile(row, "alpha_two_sided_depth_read")
        _set_color(row, [0.004, 0.012, 0.011, 5.0], emissive=0.25, clip=0.08)
        _set_uv_speed(row, 0.24, -0.08)

    def tune_breath_yellow(row: JsonObject) -> None:
        _set_start(row, 2.25)
        _set_life(row, 4.1)
        _set_transform_scale(row, 1.55)
        _set_render_profile(row, "additive_two_sided_depth_read")
        _set_color(row, [4.8, 3.85, 0.05, 5.2], emissive=4.8, clip=0.0)
        _set_uv_speed(row, -0.30, 0.12)

    result.extend(
        [
            _clone(
                cone,
                CONE_ID,
                f"{GENERATED_PREFIX}catch-breath.forward-cone.dark",
                "catch breath forward black volume",
                tune_breath_dark,
            ),
            _clone(
                cone,
                CONE_ID,
                f"{GENERATED_PREFIX}catch-breath.forward-cone.yellow",
                "catch breath forward yellow volume",
                tune_breath_yellow,
            ),
        ]
    )
    for index, donor_id in enumerate(HAND_ROAR_IDS):
        result.append(
            _clone(
                roar,
                donor_id,
                f"{GENERATED_PREFIX}catch-breath.release-core.{index + 1:02d}",
                f"catch breath release core {index + 1}",
                _mutate(
                    lambda row: _set_start(row, 2.35),
                    lambda row: _set_life(row, 1.0),
                    lambda row: _set_transform_scale(row, 1.35),
                    lambda row: _set_color(
                        row,
                        [0.02, 4.2, 3.35, 4.6],
                        emissive=4.0,
                        clip=0.0,
                    ),
                ),
            )
        )
    result.append(
        _clone(
            recovery,
            HAND_RECOVERY_SMOKE_ID,
            f"{GENERATED_PREFIX}catch-breath.recovery-smoke",
            "catch breath recovery black smoke",
            _mutate(
                lambda row: _set_start(row, 6.55),
                lambda row: _set_life(row, 1.2),
                lambda row: _set_uv_speed(row, -0.12, 0.09),
                lambda row: _set_color(
                    row,
                    [0.006, 0.18, 0.15, 3.4],
                    emissive=0.55,
                    clip=0.07,
                ),
            ),
        )
    )
    return result


def _append_catalog_rows(catalog: JsonObject, targets: Iterable[Target]) -> None:
    if catalog.get("formatVersion") != 1:
        raise AuthoringError("EffectCatalog formatVersion must remain 1")
    rows = _require_list(catalog.get("effects"), "EffectCatalog.effects")
    rows[:] = [
        row
        for row in rows
        if not (
            isinstance(row, dict)
            and row.get("effectAssetId") in REDUNDANT_GENERATED_CATALOG_IDS
        )
    ]
    index = _indexed(rows, "effectAssetId", "EffectCatalog.effects")
    for target in targets:
        if not target.catalog_direct:
            continue
        expected = {
            "effectAssetId": target.effect_asset_id,
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
            "authoringPath": target.authoring_path,
        }
        existing = index.get(target.effect_asset_id)
        if existing is None:
            rows.append(expected)
            index[target.effect_asset_id] = rows[-1]
        elif existing != expected:
            raise AuthoringError(
                f"catalog identity collision for {target.effect_asset_id}; refusing overwrite"
            )


def _remove_promoted_draft_bindings(document: JsonObject) -> None:
    if (
        document.get("schema") != "lostark.valtan-pattern-authoring-effects"
        or document.get("formatVersion") != 1
    ):
        raise AuthoringError("ValtanPatternAuthoringEffects identity/version drift")
    bindings = _require_list(
        document.get("bindings"), "ValtanPatternAuthoringEffects.bindings"
    )
    promoted = set(PROMOTED_DRAFT_BINDINGS)
    kept: list[Any] = []
    for raw in bindings:
        row = _require_object(raw, "ValtanPatternAuthoringEffects binding")
        identity = (str(row.get("patternId", "")), str(row.get("effectAssetId", "")))
        if identity in promoted:
            continue
        kept.append(raw)
    bindings[:] = kept


def _set_single_pattern_cue(
    pattern: JsonObject,
    stage_id: str,
    cue: JsonObject,
) -> None:
    stages = _require_list(pattern.get("stages"), f"{pattern.get('patternId')}.stages")
    for raw in stages:
        stage = _require_object(raw, f"{pattern.get('patternId')} stage")
        stage["effectCues"] = []
    _require_list(
        _stage(pattern, stage_id).get("effectCues"),
        f"{pattern.get('patternId')}/{stage_id}.effectCues",
    ).append(cue)


def _pattern(document: JsonObject, pattern_id: str) -> JsonObject:
    patterns = _indexed(
        _require_list(document.get("patterns"), "patterns"), "patternId", "patterns"
    )
    result = patterns.get(pattern_id)
    if result is None:
        raise AuthoringError(f"missing Valtan pattern: {pattern_id}")
    return result


def _stage(pattern: JsonObject, stage_id: str) -> JsonObject:
    stages = _indexed(
        _require_list(pattern.get("stages"), f"{pattern.get('patternId')}.stages"),
        "stageId",
        f"{pattern.get('patternId')}.stages",
    )
    result = stages.get(stage_id)
    if result is None:
        raise AuthoringError(f"missing stage {pattern.get('patternId')}/{stage_id}")
    return result


def _local_transform(
    yaw: float = 0.0,
    position: tuple[float, float, float] = (0.0, 0.0, 0.0),
) -> JsonObject:
    return {
        "position": [float(value) for value in position],
        "rotationDegrees": [0.0, float(yaw), 0.0],
        "scale": [1.0, 1.0, 1.0],
    }


def _cue(
    cue_id: str,
    effect_asset_id: str,
    presentation_stage: JsonObject,
    *,
    scale_kind: str,
    yaw: float = 0.0,
    position: tuple[float, float, float] = (0.0, 0.0, 0.0),
    follow_policy: str = "follow",
) -> JsonObject:
    animation = _require_object(presentation_stage.get("animation"), "stage.animation")
    occurrences = _require_list(animation.get("occurrences"), "animation.occurrences")
    if len(occurrences) != 1:
        raise AuthoringError(f"requested cue requires one occurrence: {cue_id}")
    occurrence = _require_object(occurrences[0], "animation occurrence")
    scale: JsonObject = {"kind": scale_kind}
    if scale_kind == "GAMEPLAY_FOOTPRINT":
        scale["worldScale"] = [1.5, 1.5, 1.5]
    return {
        "cueId": cue_id,
        "scalePolicy": scale,
        "occurrenceId": f"{cue_id}.occurrence.01",
        "effectAssetId": effect_asset_id,
        "clipOccurrenceId": occurrence["clipOccurrenceId"],
        "sourceStartMs": occurrence["sourceStartMs"],
        "sourceEndMs": None,
        "anchorSlotId": "root",
        "followPolicy": follow_policy,
        "stopPolicy": "natural",
        "repeatPolicy": "once",
        "localTransform": _local_transform(yaw, position),
        "mappingBasis": "PROJECT_AUTHORED",
    }


def _stage_presentation(presentation: JsonObject) -> None:
    for target, suffix in ((TERRAIN_3, "3"), (TERRAIN_9, "9")):
        pattern = _pattern(presentation, target.pattern_id or "")
        impact = _stage(pattern, "IMPACT")
        _set_single_pattern_cue(
            pattern,
            "IMPACT",
            _cue(
                f"{CUE_PREFIX}terrain-{suffix}.semicircle",
                target.effect_asset_id,
                impact,
                scale_kind="GAMEPLAY_FOOTPRINT",
                follow_policy="snapshot",
            ),
        )

    warp = _pattern(presentation, WARP_PORTAL.pattern_id or "")
    for raw in _require_list(warp.get("stages"), "VALTAN_WARP.stages"):
        warp_stage = _require_object(raw, "VALTAN_WARP stage")
        warp_stage["effectCues"] = [
            cue
            for cue in _require_list(
                warp_stage.get("effectCues"), "VALTAN_WARP stage.effectCues"
            )
            if not str(_require_object(cue, "VALTAN_WARP cue").get("cueId", ""))
            .startswith((CUE_PREFIX, PHASE_TWO_CUE_PREFIX))
        ]
    for leg in range(2, 10):
        leg_stage = _stage(warp, f"STEP_{leg:02d}")
        leg_stage["effectCues"].append(
            _cue(
                f"{PHASE_TWO_CUE_PREFIX}warp.step-{leg:02d}.composite",
                WARP_PORTAL.effect_asset_id,
                leg_stage,
                scale_kind="OWNER_RELATIVE",
                position=(0.0, 0.0, 3.0),
            )
        )

    assignments = (
        (
            SIX_PIZZA,
            "STEP_01",
            f"{CUE_PREFIX}six-pizza.composite",
            "GAMEPLAY_FOOTPRINT",
        ),
        (
            ATTACK_WHIRLWIND,
            "STEP_01",
            f"{CUE_PREFIX}attack-whirlwind.composite",
            "GAMEPLAY_FOOTPRINT",
        ),
        (
            PROMOTED_CHARGE,
            "STEP_01",
            f"{CUE_PREFIX}charge.axe-follow",
            "OWNER_RELATIVE",
        ),
        (
            CHARGE_2,
            "STEP_03",
            f"{CUE_PREFIX}charge2.red-fan",
            "GAMEPLAY_FOOTPRINT",
        ),
        (
            ROAR_CHARGE,
            "STEP_03",
            f"{CUE_PREFIX}roar-charge.composite",
            "OWNER_RELATIVE",
        ),
        (THREE, "STEP_01", f"{CUE_PREFIX}three.composite", "GAMEPLAY_FOOTPRINT"),
        (
            FRONT_BACK_FRONT,
            "STEP_01",
            f"{CUE_PREFIX}front-back-front.electric-fan",
            "GAMEPLAY_FOOTPRINT",
        ),
        (
            COUNTER,
            "STEP_03",
            f"{CUE_PREFIX}counter.cyan-roar-ring",
            "GAMEPLAY_FOOTPRINT",
        ),
        (TRASH, "STEP_01", f"{CUE_PREFIX}trash.composite", "OWNER_RELATIVE"),
        (
            TRASH_CATCH_SUCCESS,
            "STEP_01",
            f"{CUE_PREFIX}trash-catch-success.composite",
            "OWNER_RELATIVE",
        ),
        (
            TRASH_CATCH_FAIL,
            "STEP_01",
            f"{CUE_PREFIX}trash-catch-fail.composite",
            "OWNER_RELATIVE",
        ),
        (
            TRASH_CATCH_IF,
            "STEP_01",
            f"{CUE_PREFIX}trash-catch-if.composite",
            "OWNER_RELATIVE",
        ),
        (
            CATCH_BREATH,
            "STEP_01",
            f"{CUE_PREFIX}catch-breath.composite",
            "OWNER_RELATIVE",
        ),
        (
            STRUGGLING,
            "STEP_01",
            f"{CUE_PREFIX}struggling.composite",
            "OWNER_RELATIVE",
        ),
    )
    for target, stage_id, cue_id, scale_kind in assignments:
        pattern = _pattern(presentation, target.pattern_id or "")
        stage = _stage(pattern, stage_id)
        _set_single_pattern_cue(
            pattern,
            stage_id,
            _cue(
                cue_id,
                target.effect_asset_id,
                stage,
                scale_kind=scale_kind,
            ),
        )


def _stage_six_pizza_targeting(gameplay: JsonObject) -> None:
    pizza = _pattern(gameplay, "VALTAN_SIX_PIZZA_106")
    pizza["targetPolicy"] = "LOCK_RANDOM_ALIVE_ON_START"
    pizza["aimPolicy"] = "TRACK_TARGET_EACH_TICK"


def collect_projection() -> Projection:
    guards: dict[Path, bytes | None] = {}
    donors = {
        "arena": _load_required(DONOR_ARENA_109, guards),
        "floor": _load_required(DONOR_FLOOR_WIPE, guards),
        "four": _load_required(DONOR_FOUR_SLASH, guards),
        "whirlwind": _load_required(DONOR_WHIRLWIND, guards),
        "native": _load_required(DONOR_WHIRLWIND_NATIVE, guards),
        "cone": _load_required(DONOR_CONE, guards),
        "sky": _load_required(DONOR_SKY_AXE, guards),
        "front": _load_required(DONOR_FRONT_BACK, guards),
        "takeoff": _load_required(DONOR_HIGH_JUMP_TAKEOFF, guards),
        "land": _load_required(DONOR_HIGH_JUMP_LAND, guards),
        "target": _load_required(DONOR_TARGET_CONE, guards),
        "portal_rush": _load_required(DONOR_PORTAL_RUSH, guards),
        "hand": _load_required(DONOR_HAND_CORE, guards),
        "recovery": _load_required(DONOR_HAND_RECOVERY, guards),
        "roar": _load_required(DONOR_HAND_ROAR, guards),
        "fist": _load_required(DONOR_FIST_IN_OUT, guards),
        "imprison": _load_required(DONOR_IMPRISON_ROAR, guards),
    }
    expected_donor_ids = {
        "arena": "effect.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01",
        "floor": "effect.valtan.floor-wipe-130",
        "four": "effect.valtan.carrier-v1.attack.four-slash.active.clip-01",
        "whirlwind": "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
        "native": "effect.valtan.pattern.420633.active.v1.unified",
        "cone": "effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01",
        "sky": "effect.valtan.sky-axe.active",
        "front": "effect.valtan.carrier-v1.attack.front-back-front.active.clip-01",
        "takeoff": "effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01",
        "land": "effect.valtan.carrier-v1.attack.high-jump.land.clip-01",
        "target": "effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01",
        "portal_rush": "effect.valtan.carrier-v1.attack.portal-rush.rushes.clip-01",
        "hand": "effect.valtan.carrier-v1.attack.charge-grab-roar.counter.clip-01",
        "recovery": "effect.valtan.carrier-v1.attack.charge-grab-roar.recovery.clip-01",
        "roar": "effect.valtan.carrier-v1.attack.charge-grab-roar.roar.clip-01",
        "fist": "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
        "imprison": "effect.valtan.carrier-v1.attack.imprison-roar.active.clip-01",
    }
    for name, document in donors.items():
        _validate_effect_document(document, expected_donor_ids[name], f"donor.{name}")

    # The Tool may save the editable portal donor while this generated Product
    # remains open for comparison.  Once the exact fourteen-layer Product
    # exists, it is the stable source for subsequent Validate/Apply runs; do
    # not require or rewrite the concurrently edited donor.
    existing_portal_product = _load_optional(WARP_PORTAL.relative_path, guards)
    if existing_portal_product is None:
        portal_donor = _load_required(DONOR_PORTAL, guards)
        _validate_effect_document(
            portal_donor,
            "effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01",
            "donor.portal",
        )
        portal_generated = _portal_elements(portal_donor, donors["portal_rush"])
    else:
        _validate_effect_document(
            existing_portal_product,
            WARP_PORTAL.effect_asset_id,
            WARP_PORTAL.effect_asset_id,
        )
        portal_generated = [
            copy.deepcopy(_require_object(row, "portal Product element"))
            for row in _require_list(
                existing_portal_product.get("elements"),
                "portal Product elements",
            )
            if str(_require_object(row, "portal Product element").get("id", ""))
            .startswith(f"{GENERATED_PREFIX}warp.portal.")
        ]
        if len(portal_generated) != 14:
            raise AuthoringError(
                "portal Product closure drifted: expected 14 generated layers, "
                f"got {len(portal_generated)}"
            )
        portal_generated.append(_portal_rush_forward_element(donors["portal_rush"]))

    portal_layers = [
        row
        for row in portal_generated
        if str(row.get("id", "")).startswith(f"{GENERATED_PREFIX}warp.portal.")
    ]
    front_back_product = _load_required(FRONT_BACK_FRONT.relative_path, guards)
    _validate_effect_document(
        front_back_product,
        FRONT_BACK_FRONT.effect_asset_id,
        FRONT_BACK_FRONT.effect_asset_id,
    )
    for promoted_target in PROMOTED_USER_TARGETS:
        promoted_document = _load_required(promoted_target.relative_path, guards)
        _validate_effect_document(
            promoted_document,
            promoted_target.effect_asset_id,
            promoted_target.effect_asset_id,
        )

    direct_generated = {
        TERRAIN_3: _terrain_elements(
            donors["floor"], donors["land"], "3", 0.0
        ),
        TERRAIN_9: _terrain_elements(
            donors["floor"], donors["land"], "9", 180.0
        ),
        SIX_PIZZA: _six_pizza_elements(donors["arena"], donors["floor"]),
        ATTACK_WHIRLWIND: _attack_whirlwind_elements(
            donors["four"],
            donors["whirlwind"],
            donors["arena"],
            donors["target"],
            donors["takeoff"],
        ),
        CHARGE: _charge_elements(
            donors["whirlwind"], donors["native"], donors["target"]
        ),
        CHARGE_2: _charge2_elements(donors["floor"], donors["whirlwind"]),
        ROAR_CHARGE: _roar_charge_elements(
            donors["arena"], donors["cone"], donors["sky"]
        ),
        THREE: _three_elements(donors["arena"], donors["sky"]),
        FRONT_BACK_FRONT: _front_back_elements(
            donors["front"], donors["arena"], donors["sky"], donors["target"]
        ),
        COUNTER: _counter_elements(donors["arena"]),
        WARP_PORTAL: portal_generated,
        TRASH: _trash_elements(
            donors["hand"], donors["front"], donors["recovery"]
        ),
        TRASH_CATCH_SUCCESS: _trash_catch_success_elements(
            donors["hand"], donors["roar"], donors["recovery"]
        ),
        TRASH_CATCH_FAIL: _trash_catch_fail_elements(donors["recovery"]),
        TRASH_CATCH_IF: _trash_catch_if_elements(donors["hand"]),
        CATCH_BREATH: _catch_breath_elements(
            donors["hand"], donors["cone"], donors["roar"], donors["recovery"]
        ),
        STRUGGLING: _struggling_elements(
            portal_generated,
            front_back_product,
            donors["arena"],
            donors["fist"],
            donors["takeoff"],
            donors["imprison"],
        ),
    }

    outputs: dict[Path, bytes] = {}
    appended_by_target: dict[str, int] = {}
    for target, generated in direct_generated.items():
        document, appended = _stage_target(target, generated, guards)
        if target in (TERRAIN_3, TERRAIN_9):
            suffix = "3" if target is TERRAIN_3 else "9"
            elements = _require_list(document.get("elements"), target.effect_asset_id)
            takeoff_prefix = f"{GENERATED_PREFIX}terrain-{suffix}.takeoff."
            elements[:] = [
                row
                for row in elements
                if not str(_require_object(row, target.effect_asset_id).get("id", ""))
                .startswith(takeoff_prefix)
            ]
            for index in range(1, 4):
                _set_start(
                    _element(
                        document,
                        f"{GENERATED_PREFIX}terrain-{suffix}.semicircle.sector-{index:02d}",
                        target.effect_asset_id,
                    ),
                    0.0,
                )
            for index, start in enumerate((0.230894, 0.231), start=1):
                _set_start(
                    _element(
                        document,
                        f"{GENERATED_PREFIX}terrain-{suffix}.landing.{index:02d}",
                        target.effect_asset_id,
                    ),
                    start,
                )
        if target is CATCH_BREATH:
            cyan_id = f"{GENERATED_PREFIX}catch-breath.forward-cone.cyan"
            elements = _require_list(document.get("elements"), target.effect_asset_id)
            elements[:] = [
                row
                for row in elements
                if str(_require_object(row, target.effect_asset_id).get("id", ""))
                != cyan_id
            ]
            dark = _element(
                document,
                f"{GENERATED_PREFIX}catch-breath.forward-cone.dark",
                target.effect_asset_id,
            )
            yellow = _element(
                document,
                f"{GENERATED_PREFIX}catch-breath.forward-cone.yellow",
                target.effect_asset_id,
            )
            for cone in (dark, yellow):
                _set_start(cone, 2.25)
                _set_yaw(cone, 0.0)
            _set_color(yellow, [4.8, 3.85, 0.05, 5.2], emissive=4.8, clip=0.0)
        if target is COUNTER:
            counter = _element(
                document,
                f"{GENERATED_PREFIX}counter.cyan-roar-ring",
                target.effect_asset_id,
            )
            _set_start(counter, 0.9)
            _set_decal_size(counter, 24.0)
            _set_color(
                counter,
                [0.0, 3.35, 2.45, 4.15],
                emissive=4.0,
                clip=0.0,
            )
        if target is WARP_PORTAL:
            document["displayName"] = target.display_name
            _retune_warp_leg_composite(document)
        outputs[target.relative_path] = _output_bytes(target.relative_path, document, guards)
        appended_by_target[target.effect_asset_id] = appended

    catalog = copy.deepcopy(_load_required(CATALOG_PATH, guards))
    _append_catalog_rows(catalog, PRODUCT_CATALOG_TARGETS)
    outputs[CATALOG_PATH] = _output_bytes(CATALOG_PATH, catalog, guards)

    pattern_bindings = copy.deepcopy(
        _load_required(PATTERN_AUTHORING_BINDINGS_PATH, guards)
    )
    _remove_promoted_draft_bindings(pattern_bindings)
    outputs[PATTERN_AUTHORING_BINDINGS_PATH] = _output_bytes(
        PATTERN_AUTHORING_BINDINGS_PATH,
        pattern_bindings,
        guards,
    )

    presentation = copy.deepcopy(_load_required(PRESENTATION_PATH, guards))
    _stage_presentation(presentation)
    outputs[PRESENTATION_PATH] = _output_bytes(PRESENTATION_PATH, presentation, guards)

    gameplay_source = _load_required(GAMEPLAY_PATH, guards)
    gameplay = copy.deepcopy(gameplay_source)
    pizza_before = _pattern(gameplay_source, "VALTAN_SIX_PIZZA_106")
    _stage_six_pizza_targeting(gameplay)
    gameplay_restored = copy.deepcopy(gameplay)
    restored_pizza = _pattern(gameplay_restored, "VALTAN_SIX_PIZZA_106")
    for field in ("targetPolicy", "aimPolicy"):
        if field in pizza_before:
            restored_pizza[field] = pizza_before[field]
        else:
            restored_pizza.pop(field, None)
    if gameplay_restored != gameplay_source:
        raise AuthoringError(
            "gameplay projection changed fields outside SIX_PIZZA targetPolicy/aimPolicy"
        )
    outputs[GAMEPLAY_PATH] = _output_bytes(GAMEPLAY_PATH, gameplay, guards)

    # A target may also be a donor in future revisions.  The exact captured
    # bytes remain the CAS precondition, and each old element was prefix-checked
    # before serialization.
    return Projection(outputs=outputs, guards=guards, appended_by_target=appended_by_target)


def _verify_guards(projection: Projection) -> None:
    for relative, expected in projection.guards.items():
        path = _absolute(relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            raise AuthoringError(
                f"input changed after staging; no files were written: {relative.as_posix()}"
            )


def apply_projection(projection: Projection) -> None:
    _verify_guards(projection)
    changed = projection.changed_paths
    if not changed:
        return
    transaction_root = Path(tempfile.mkdtemp(prefix=".valtan-requested-effects.", dir=ROOT))
    staged_root = transaction_root / "staged"
    backup_root = transaction_root / "backup"
    promoted: list[Path] = []
    try:
        for relative in changed:
            staged = staged_root / relative
            staged.parent.mkdir(parents=True, exist_ok=True)
            staged.write_bytes(projection.outputs[relative])
            if staged.read_bytes() != projection.outputs[relative]:
                raise AuthoringError(f"staged output verification failed: {relative}")
        _verify_guards(projection)
        for relative in changed:
            target = _absolute(relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.is_file():
                backup = backup_root / relative
                backup.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(target, backup)
            os.replace(staged_root / relative, target)
            promoted.append(relative)
    except Exception as exc:
        rollback_errors: list[str] = []
        for relative in reversed(promoted):
            target = _absolute(relative)
            backup = backup_root / relative
            try:
                if backup.is_file():
                    os.replace(backup, target)
                elif target.exists():
                    target.unlink()
            except OSError as rollback_exc:
                rollback_errors.append(f"{relative}: {rollback_exc}")
        if rollback_errors:
            raise AuthoringError(
                "application failed and rollback was incomplete: "
                + "; ".join(rollback_errors)
            ) from exc
        if isinstance(exc, AuthoringError):
            raise
        raise AuthoringError(f"application failed; all outputs rolled back: {exc}") from exc
    finally:
        shutil.rmtree(transaction_root, ignore_errors=True)


def validate_projection(projection: Projection) -> None:
    stale: list[str] = []
    for relative, expected in projection.outputs.items():
        path = _absolute(relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            stale.append(relative.as_posix())
    if stale:
        raise AuthoringError(
            "requested Valtan effect projection is missing/stale: "
            + ", ".join(sorted(stale))
        )


def _summary(projection: Projection, mode: str) -> str:
    generated = sum(projection.appended_by_target.values())
    return (
        f"[PASS] requested Valtan effect elements {mode}: "
        f"targets={len(projection.appended_by_target)}, "
        f"newElements={generated}, outputs={len(projection.outputs)}, "
        f"changed={len(projection.changed_paths)}, "
        "existingElements=preserved, sixPizzaTargeting=locked"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("Validate", "Apply"), default="Validate")
    args = parser.parse_args(argv)
    try:
        projection = collect_projection()
        if args.mode == "Apply":
            apply_projection(projection)
        else:
            validate_projection(projection)
        print(_summary(projection, args.mode.upper()))
        return 0
    except AuthoringError as exc:
        print(f"[FAILURE] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
