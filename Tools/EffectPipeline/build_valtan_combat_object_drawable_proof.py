#!/usr/bin/env python3
"""Seal executed WARP evidence for Valtan-owned combat-object visuals.

The runtime sweep is tracked immutable headless-renderer evidence. This builder
joins it to BossCatalog ownership, Server combat-object lifetime/movement,
the two immutable authored Effect documents, and the source occurrence
inventory.  It never edits cues, bindings, sequences, authored Effects, or the
color pipeline.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


RUNTIME_SWEEP_SCHEMA = "lostark.valtan-combat-object-runtime-sweep"
PROOF_SCHEMA = "lostark.valtan-combat-object-drawable-proof"
FORMAT_VERSION = 1
OUTPUT_ROOT = PurePosixPath(
    "Data/Effects/Imported/Valtan/CombatObjectDrawableProof"
)
SWEEP_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "Valtan.combat-object.runtime-sweep.v1.json"
)
PROOF_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "Valtan.combat-object.drawable-proof.v1.json"
)
BOSS_CATALOG_RELATIVE_PATH = PurePosixPath("Data/Actors/BossCatalog.json")
COMBAT_OBJECT_CATALOG_RELATIVE_PATH = PurePosixPath(
    "Data/Encounters/Valtan/ValtanCombatObjects.json"
)
SKY_EFFECT_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json"
)
RED_EFFECT_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Authored/effect.valtan.red-blade-wave.active.effect.json"
)
SOURCE_INVENTORY_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/Valtan.source-occurrence-inventory.v1.json"
)

LIFECYCLE_FACTS = [
    "LATE_INITIAL_AGE_RESOLUTION",
    "STOP_ONCE",
    "ATOMIC_SNAPSHOT_ROLLBACK",
    "BOUNDED_THREE_ATTEMPT_RETRY",
    "UPDATE_FAILURE_STOP_AND_RETRY",
]

SKY_ARCHETYPE = "combatobject.valtan.high-jump.target-axe"
SKY_VISUAL = "combatobject.visual.valtan.high-jump.target-axe.v1"
SKY_EFFECT = "effect.valtan.sky-axe.active"
SKY_ELEMENTS = [
    "mesh.valtan.sky-axe.descent",
    "decal.valtan.sky-axe.target",
    "particle.valtan.sky-axe.impact",
]
SKY_FAMILIES = {
    SKY_ELEMENTS[0]: "MESH",
    SKY_ELEMENTS[1]: "DECAL",
    SKY_ELEMENTS[2]: "SPRITE",
}
SKY_RESOURCES = {
    SKY_ELEMENTS[0]: {
        "meshModel": "Character/Valtan/ValtanWeapon.wmodel",
    },
    SKY_ELEMENTS[1]: {
        "base": "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_032.dds",
    },
    SKY_ELEMENTS[2]: {
        "base": "Effect/Valtan/Textures/FX_TEX_04/fx_i_shockwave_02_ycl.dds",
        "dissolve": "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_011.dds",
    },
}

RED_ARCHETYPE = "combatobject.valtan.red-blade-wave.projectile"
RED_VISUAL = "combatobject.visual.valtan.red-blade-wave.projectile.v1"
RED_EFFECT = "effect.valtan.red-blade-wave.active"
RED_SPEED_MPS = 24.444445
RED_ELEMENTS = [
    "source.055adf57f38ac8f25be3",
    "source.5cb81ed1867daa4c331a",
    "source.5d987e41721843a8d390",
    "source.94fe8920096dda4a4e73",
    "source.f997148180378f188150",
]
RED_RESOURCES = {
    RED_ELEMENTS[0]: {
        "base": "Effect/Valtan/Textures/FX_TEX_05/fx_m_atypical_n_001.dds",
        "mask": "Effect/Valtan/Textures/FX_TEX_HIGH_01/fx_f_electric_005.dds",
    },
    RED_ELEMENTS[1]: {
        "base": "Effect/Valtan/Textures/FX_TEX_04/fx_f_ring_001.dds",
    },
    RED_ELEMENTS[2]: {
        "base": "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_03.dds",
    },
    RED_ELEMENTS[3]: {
        "base": "Effect/Valtan/Textures/FX_TEX_00/fx_a_hit_008.dds",
    },
    RED_ELEMENTS[4]: {
        "base": "Effect/Valtan/Textures/FX_TEX_HIGH_00/fx_c_glow_008.dds",
    },
}


class ProofError(RuntimeError):
    """Runtime evidence or one of its authority joins is stale/invalid."""


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    try:
        return hashlib.sha256(path.read_bytes()).hexdigest()
    except OSError as error:
        raise ProofError(f"cannot hash {path}: {error}") from error


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode(
        "utf-8"
    )


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_bytes().decode("utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ProofError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ProofError(f"JSON root must be an object: {path}")
    return value


def repository_path(root: Path, relative: PurePosixPath) -> Path:
    resolved_root = root.resolve()
    result = root.joinpath(*relative.parts).resolve()
    if result != resolved_root and resolved_root not in result.parents:
        raise ProofError(f"repository-relative path escaped root: {relative}")
    return result


def relative_path(root: Path, path: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError as error:
        raise ProofError(f"path is outside repository: {path}") from error


def _is_sha256(value: Any) -> bool:
    return (
        isinstance(value, str)
        and len(value) == 64
        and all(character in "0123456789abcdef" for character in value)
    )


def _number(value: Any, label: str) -> float:
    if (
        not isinstance(value, (int, float))
        or isinstance(value, bool)
        or not math.isfinite(float(value))
    ):
        raise ProofError(f"{label} must be finite")
    return float(value)


def _integer(value: Any, label: str) -> int:
    if not isinstance(value, int) or isinstance(value, bool):
        raise ProofError(f"{label} must be an integer")
    return value


def _near(actual: Any, expected: float, label: str, tolerance: float = 1e-5) -> None:
    if abs(_number(actual, label) - expected) > tolerance:
        raise ProofError(f"{label} changed: expected {expected}, got {actual}")


def _vector3(value: Any, label: str) -> list[float]:
    if not isinstance(value, list) or len(value) != 3:
        raise ProofError(f"{label} must be a finite vec3")
    return [_number(component, f"{label}[{index}]") for index, component in enumerate(value)]


def _resources(element: dict[str, Any]) -> dict[str, str]:
    rows = element.get("resources")
    if not isinstance(rows, list):
        raise ProofError(f"Effect Element {element.get('id')} has no resources")
    result: dict[str, str] = {}
    for row in rows:
        if not isinstance(row, dict):
            raise ProofError("Effect resource row is not an object")
        slot = row.get("slotId")
        asset = row.get("assetId")
        if (
            not isinstance(slot, str)
            or not slot
            or not isinstance(asset, str)
            or not asset
            or slot in result
        ):
            raise ProofError("Effect resource identity is empty or duplicate")
        result[slot] = asset
    return result


def _elements(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    rows = document.get("elements")
    if not isinstance(rows, list):
        raise ProofError("Effect document elements are unavailable")
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        if not isinstance(row, dict):
            raise ProofError("Effect Element is not an object")
        element_id = row.get("id")
        if not isinstance(element_id, str) or not element_id or element_id in result:
            raise ProofError("Effect Element ID is empty or duplicate")
        result[element_id] = row
    return result


def _resource_path(resource_root: Path, asset_id: str) -> Path:
    pure = PurePosixPath(asset_id)
    if pure.is_absolute() or ".." in pure.parts:
        raise ProofError(f"runtime resource ID escaped root: {asset_id}")
    root = resource_root.resolve()
    result = resource_root.joinpath(*pure.parts).resolve()
    if root not in result.parents or not result.is_file():
        raise ProofError(f"runtime resource is missing: {asset_id}")
    return result


def _walk_dicts(value: Any) -> Iterable[dict[str, Any]]:
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from _walk_dicts(child)
    elif isinstance(value, list):
        for child in value:
            yield from _walk_dicts(child)


def _validate_effect_documents(
    sky: dict[str, Any],
    red: dict[str, Any],
    source_inventory: dict[str, Any],
    resource_root: Path,
) -> tuple[list[str], list[str]]:
    if (
        sky.get("schema") != "lostark.effect-authoring"
        or sky.get("effectAssetId") != SKY_EFFECT
        or red.get("schema") != "lostark.effect-authoring"
        or red.get("effectAssetId") != RED_EFFECT
    ):
        raise ProofError("combat-object authored Effect identity changed")
    sky_elements = _elements(sky)
    red_elements = _elements(red)
    if list(sky_elements) != SKY_ELEMENTS:
        raise ProofError("sky-axe Effect must contain exactly the audited three Elements")

    expected_sky_kinds = {
        SKY_ELEMENTS[0]: "mesh",
        SKY_ELEMENTS[1]: "decal",
        SKY_ELEMENTS[2]: "particle",
    }
    expected_sky_timing = {
        SKY_ELEMENTS[0]: (0.0, 1.2),
        SKY_ELEMENTS[1]: (0.0, 1.2),
        SKY_ELEMENTS[2]: (1.2, 0.65),
    }
    for element_id in SKY_ELEMENTS:
        element = sky_elements[element_id]
        if element.get("visible") is not True or element.get("kind") != expected_sky_kinds[element_id]:
            raise ProofError(f"sky-axe Element kind/visibility changed: {element_id}")
        if _resources(element) != SKY_RESOURCES[element_id]:
            raise ProofError(f"sky-axe Element resources changed: {element_id}")
        timing = ((element.get("detail") or {}).get("timing") or {})
        _near(timing.get("startDelaySeconds"), expected_sky_timing[element_id][0], f"{element_id} delay")
        _near(timing.get("lifeTimeSeconds"), expected_sky_timing[element_id][1], f"{element_id} life")

    inventory_module_occurrences: set[tuple[str, int, str, str]] = set()
    source_system_ids: set[str] = set()
    for row in _walk_dicts(source_inventory):
        source_system_id = row.get("sourceSystemId")
        if isinstance(source_system_id, str):
            source_system_ids.add(source_system_id)
        node_id = row.get("sourceNodeId")
        reference_index = row.get("referenceIndex")
        class_name = row.get("className")
        object_path = row.get("objectPath")
        if (
            isinstance(node_id, str)
            and isinstance(reference_index, int)
            and not isinstance(reference_index, bool)
            and isinstance(class_name, str)
            and isinstance(object_path, str)
        ):
            inventory_module_occurrences.add(
                (node_id, reference_index, class_name, object_path)
            )
    if "fx_mn_rpbf_00_o.par_o_rpbf_atk_08_05" not in source_system_ids:
        raise ProofError("red-blade source system is missing from occurrence inventory")

    for element_id in RED_ELEMENTS:
        element = red_elements.get(element_id)
        if not isinstance(element, dict):
            raise ProofError(f"red-blade source Element is missing: {element_id}")
        if (
            element.get("visible") is not True
            or element.get("kind") != "particle"
            or _resources(element) != RED_RESOURCES[element_id]
        ):
            raise ProofError(f"red-blade source Element changed: {element_id}")
        recipe = element.get("sourceRecipe") or {}
        modules = recipe.get("modules")
        if (
            recipe.get("enabled") is not True
            or recipe.get("rendererShape") != "sprite"
            or not isinstance(modules, list)
            or not modules
        ):
            raise ProofError(f"red-blade source recipe is not admitted: {element_id}")
        for module in modules:
            if not isinstance(module, dict):
                raise ProofError(f"red-blade source module is malformed: {element_id}")
            stable_id = module.get("stableId")
            class_name = module.get("className")
            object_path = module.get("objectPath")
            if (
                not isinstance(stable_id, str)
                or "@ref:" not in stable_id
                or not isinstance(class_name, str)
                or not isinstance(object_path, str)
            ):
                raise ProofError(f"red-blade source module identity is incomplete: {element_id}")
            node_id, reference_text = stable_id.rsplit("@ref:", 1)
            try:
                reference_index = int(reference_text)
            except ValueError as error:
                raise ProofError(f"red-blade source module reference is invalid: {stable_id}") from error
            if (node_id, reference_index, class_name, object_path) not in inventory_module_occurrences:
                raise ProofError(f"red-blade source module is absent from occurrence inventory: {stable_id}")

    sky_resources = sorted(
        {asset for resources in SKY_RESOURCES.values() for asset in resources.values()}
    )
    red_resources = sorted(
        {asset for resources in RED_RESOURCES.values() for asset in resources.values()}
    )
    for asset_id in sky_resources + red_resources:
        _resource_path(resource_root, asset_id)
    return sky_resources, red_resources


def _validate_catalogs(
    boss_catalog: dict[str, Any], combat_catalog: dict[str, Any]
) -> dict[str, dict[str, Any]]:
    bosses = boss_catalog.get("bosses")
    valtan = next(
        (
            row
            for row in bosses
            if isinstance(row, dict) and row.get("archetypeId") == "BOSS_VALTAN"
        ),
        None,
    ) if isinstance(bosses, list) else None
    expected_visuals = [
        {
            "combatObjectArchetypeId": SKY_ARCHETYPE,
            "clientVisualId": SKY_VISUAL,
            "effectAssetId": SKY_EFFECT,
        },
        {
            "combatObjectArchetypeId": RED_ARCHETYPE,
            "clientVisualId": RED_VISUAL,
            "effectAssetId": RED_EFFECT,
        },
    ]
    if not isinstance(valtan, dict) or valtan.get("combatObjectVisuals") != expected_visuals:
        raise ProofError("BossCatalog combatObjectVisuals ownership changed")

    objects = combat_catalog.get("objects")
    if (
        combat_catalog.get("schema") != "lostark.valtan-combat-objects"
        or combat_catalog.get("formatVersion") != 1
        or combat_catalog.get("encounterId") != "ENCOUNTER_VALTAN"
        or not isinstance(objects, list)
        or len(objects) != 2
    ):
        raise ProofError("Valtan combat-object catalog identity changed")
    by_id = {
        row.get("combatObjectArchetypeId"): row
        for row in objects
        if isinstance(row, dict)
    }
    sky = by_id.get(SKY_ARCHETYPE)
    red = by_id.get(RED_ARCHETYPE)
    if not isinstance(sky, dict) or not isinstance(red, dict):
        raise ProofError("Valtan combat-object targets changed")
    if (
        sky.get("clientVisualId") != SKY_VISUAL
        or sky.get("ownerPatternId") != "VALTAN_HIGH_JUMP"
        or sky.get("ownerStageActionId") != "valtan.attack.high-jump.airborne"
        or sky.get("kind") != "FIXED_AREA"
        or sky.get("originPolicy") != "LOCKED_TARGET_PER_ALIVE_PLAYER"
        or sky.get("directionPolicy") != "NONE"
        or _number(sky.get("speedMps"), "sky speed") != 0.0
        or _integer(sky.get("lifeMs"), "sky lifeMs") != 1900
    ):
        raise ProofError("HighJump target-axe authority changed")
    sky_hits = sky.get("hits")
    if (
        not isinstance(sky_hits, list)
        or len(sky_hits) != 1
        or sky_hits[0].get("trigger") != "TIMED"
        or sky_hits[0].get("atMs") != 1200
    ):
        raise ProofError("HighJump target-axe hit boundary changed")
    if (
        red.get("clientVisualId") != RED_VISUAL
        or red.get("ownerPatternId") != "VALTAN_RED_BLADE_WAVE"
        or red.get("ownerStageActionId") != "valtan.attack.red-blade-wave.active"
        or red.get("kind") != "MISSILE"
        or red.get("originPolicy") != "BOSS_POSITION"
        or red.get("directionPolicy") != "PATTERN_FACING_AT_SPAWN"
        or abs(_number(red.get("speedMps"), "red speed") - RED_SPEED_MPS) > 1e-6
        or abs(_number(red.get("maximumDistanceM"), "red maximum distance") - 22.0) > 1e-6
        or _integer(red.get("lifeMs"), "red lifeMs") != 900
    ):
        raise ProofError("RedBlade missile authority changed")
    red_hits = red.get("hits")
    if (
        not isinstance(red_hits, list)
        or len(red_hits) != 1
        or red_hits[0].get("trigger") != "CONTACT"
    ):
        raise ProofError("RedBlade missile hit boundary changed")
    return {SKY_ARCHETYPE: sky, RED_ARCHETYPE: red}


def _rows(sample: dict[str, Any], expected_ids: list[str], label: str) -> dict[str, dict[str, Any]]:
    rows = sample.get("rows")
    if not isinstance(rows, list) or len(rows) != len(expected_ids):
        raise ProofError(f"{label} renderer row denominator changed")
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        if not isinstance(row, dict):
            raise ProofError(f"{label} renderer row is malformed")
        element_id = row.get("elementId")
        if not isinstance(element_id, str) or element_id in result:
            raise ProofError(f"{label} renderer row identity is duplicate")
        result[element_id] = row
        for field in ("configured", "attempted", "submitted", "committed", "suppressed", "failed"):
            if _integer(row.get(field), f"{label}/{element_id}/{field}") < 0:
                raise ProofError(f"{label}/{element_id}/{field} is negative")
        _vector3(row.get("positionMin"), f"{label}/{element_id}/positionMin")
        _vector3(row.get("positionMax"), f"{label}/{element_id}/positionMax")
        if row.get("failed") != 0:
            raise ProofError(f"{label}/{element_id} recorded a failed draw")
        if row.get("committed") < row.get("submitted"):
            raise ProofError(f"{label}/{element_id} committed fewer draws than submitted")
        if row.get("submitted") > 0 and row.get("hasSubmittedPosition") is not True:
            raise ProofError(f"{label}/{element_id} submitted without finite position proof")
    if list(result) != expected_ids:
        raise ProofError(f"{label} renderer row order/identity changed")
    return result


def _has_draw(row: dict[str, Any]) -> bool:
    return (
        row.get("attempted", 0) > 0
        and row.get("submitted", 0) > 0
        and row.get("committed", 0) > 0
        and row.get("failed") == 0
        and row.get("hasSubmittedPosition") is True
    )


def _has_no_draw(row: dict[str, Any]) -> bool:
    return (
        row.get("submitted") == 0
        and row.get("committed") == 0
        and row.get("failed") == 0
    )


def _validate_sample_common(
    sample: dict[str, Any], expected_label: str, expected_time: float
) -> None:
    if sample.get("label") != expected_label or sample.get("transactionCommitted") is not True:
        raise ProofError(f"runtime sample identity/transaction changed: {expected_label}")
    _near(sample.get("requestedSeconds"), expected_time, f"{expected_label} requested")
    _near(sample.get("evaluatedSeconds"), expected_time, f"{expected_label} evaluated")
    _vector3(sample.get("rootPosition"), f"{expected_label} root")
    minimum = _vector3(sample.get("submittedPositionMin"), f"{expected_label} bounds min")
    maximum = _vector3(sample.get("submittedPositionMax"), f"{expected_label} bounds max")
    if sample.get("hasSubmittedBounds") is True and any(a > b for a, b in zip(minimum, maximum)):
        raise ProofError(f"runtime sample bounds are inverted: {expected_label}")


def _validate_totals(
    target: dict[str, Any],
    samples: list[dict[str, Any]],
    element_ids: list[str],
    families: dict[str, str],
) -> list[dict[str, Any]]:
    totals = target.get("elementTotals")
    if not isinstance(totals, list) or len(totals) != len(element_ids):
        raise ProofError("renderer element-total denominator changed")
    proof_rows: list[dict[str, Any]] = []
    for index, element_id in enumerate(element_ids):
        total = totals[index]
        if not isinstance(total, dict) or total.get("elementId") != element_id:
            raise ProofError(f"renderer total identity changed: {element_id}")
        sample_rows = [_rows(sample, element_ids, str(sample.get("label")))[element_id] for sample in samples]
        expected = {
            "preparedSamples": sum(row["configured"] > 0 for row in sample_rows),
            "attemptedSamples": sum(row["attempted"] > 0 for row in sample_rows),
            "submittedDraws": sum(row["submitted"] for row in sample_rows),
            "committedDraws": sum(row["committed"] for row in sample_rows),
            "suppressedDraws": sum(row["suppressed"] for row in sample_rows),
            "failedDraws": sum(row["failed"] for row in sample_rows),
        }
        if total.get("family") != families[element_id] or any(
            total.get(field) != value for field, value in expected.items()
        ):
            raise ProofError(f"renderer total is inconsistent: {element_id}")
        if (
            expected["preparedSamples"] < 1
            or expected["attemptedSamples"] < 1
            or expected["submittedDraws"] < 1
            or expected["committedDraws"] < 1
            or expected["failedDraws"] != 0
        ):
            raise ProofError(f"renderer did not draw audited Element: {element_id}")
        proof_rows.append(
            {
                "elementId": element_id,
                "family": families[element_id],
                "preparedSamples": expected["preparedSamples"],
                "attemptedSamples": expected["attemptedSamples"],
                "submittedDraws": expected["submittedDraws"],
                "committedDraws": expected["committedDraws"],
                "failedDraws": expected["failedDraws"],
            }
        )
    return proof_rows


def _validate_runtime_target(
    target: dict[str, Any],
    expected_document_path: Path,
    expected_document_sha: str,
    object_authority: dict[str, Any],
    resources: list[str],
) -> dict[str, Any]:
    archetype = target.get("combatObjectArchetypeId")
    if archetype == SKY_ARCHETYPE:
        expected_visual = SKY_VISUAL
        expected_effect = SKY_EFFECT
        expected_root_policy = "FIXED_WORLD_ROOT"
        element_ids = SKY_ELEMENTS
        families = SKY_FAMILIES
        sample_spec = [
            ("spawn", 0.0),
            ("pre-impact", 1.19),
            ("late-initial-seek-impact-start", 1.2),
            ("impact-midlife", 1.5),
            ("impact-life-end", 1.85),
        ]
        classification = "PROJECT_AUTHORED_OFFICIAL_GEOMETRY"
    elif archetype == RED_ARCHETYPE:
        expected_visual = RED_VISUAL
        expected_effect = RED_EFFECT
        expected_root_policy = "MOVING_WORLD_ROOT"
        element_ids = RED_ELEMENTS
        families = {element_id: "SPRITE" for element_id in RED_ELEMENTS}
        sample_spec = [
            ("spawn-zero-step", 0.0),
            ("first-fixed-step", 1.0 / 60.0),
            ("late-initial-seek-mid-flight", 0.45),
            ("pre-despawn", 0.899),
        ]
        classification = "SOURCE_RECIPE_IDENTITY_PRESERVED"
    else:
        raise ProofError(f"unknown combat-object runtime target: {archetype}")

    if (
        target.get("clientVisualId") != expected_visual
        or target.get("effectAssetId") != expected_effect
        or target.get("rootPolicy") != expected_root_policy
        or Path(str(target.get("documentPath") or "")).resolve() != expected_document_path.resolve()
        or target.get("documentRawSha256") != expected_document_sha
        or not _is_sha256(target.get("documentTypedCodecSha256"))
        or target.get("auditedElementIds") != element_ids
    ):
        raise ProofError(f"runtime target identity changed: {archetype}")
    _number(target.get("effectDurationSeconds"), f"{archetype} effect duration")
    samples = target.get("samples")
    if not isinstance(samples, list) or len(samples) != len(sample_spec):
        raise ProofError(f"runtime target sample denominator changed: {archetype}")
    for sample, (label, seconds) in zip(samples, sample_spec):
        if not isinstance(sample, dict):
            raise ProofError(f"runtime target sample is malformed: {archetype}")
        _validate_sample_common(sample, label, seconds)
        _rows(sample, element_ids, label)

    if archetype == SKY_ARCHETYPE:
        for sample in samples:
            root = _vector3(sample.get("rootPosition"), f"{sample.get('label')} root")
            if any(abs(actual - expected) > 1e-6 for actual, expected in zip(root, [2.0, 0.0, 4.0])):
                raise ProofError("HighJump target-axe root moved")
        rows = [_rows(sample, element_ids, sample["label"]) for sample in samples]
        if not (
            _has_draw(rows[0][SKY_ELEMENTS[0]])
            and _has_draw(rows[0][SKY_ELEMENTS[1]])
            and _has_no_draw(rows[0][SKY_ELEMENTS[2]])
            and _has_draw(rows[1][SKY_ELEMENTS[0]])
            and _has_draw(rows[1][SKY_ELEMENTS[1]])
            and _has_no_draw(rows[1][SKY_ELEMENTS[2]])
            and _has_no_draw(rows[2][SKY_ELEMENTS[0]])
            and _has_no_draw(rows[2][SKY_ELEMENTS[1]])
            and _has_draw(rows[2][SKY_ELEMENTS[2]])
            and _has_draw(rows[3][SKY_ELEMENTS[2]])
            and all(_has_no_draw(row) for row in rows[4].values())
            and target.get("rootWorldDistinctCount") == 1
        ):
            raise ProofError("HighJump target-axe draw boundary changed")
    else:
        rows = [_rows(sample, element_ids, sample["label"]) for sample in samples]
        for sample, (_, seconds) in zip(samples, sample_spec):
            root = _vector3(sample.get("rootPosition"), f"{sample.get('label')} root")
            _near(root[0], RED_SPEED_MPS * seconds, f"{sample.get('label')} root x", 2e-5)
            _near(root[1], 0.0, f"{sample.get('label')} root y")
            _near(root[2], 4.0, f"{sample.get('label')} root z")
        if (
            target.get("rootWorldDistinctCount") != 4
            or _integer(target.get("submittedBoundsDistinctCount"), "red bounds distinct") < 2
            or not all(_has_no_draw(row) for row in rows[0].values())
            or not all(_has_draw(row) for row in rows[2].values())
        ):
            raise ProofError("RedBlade moving-root/source-5 draw boundary changed")

    audited = _validate_totals(target, samples, element_ids, families)
    if _integer(target.get("submittedBoundsDistinctCount"), f"{archetype} bounds") < 1:
        raise ProofError(f"{archetype} has no submitted bounds proof")
    _integer(target.get("unauditedSubmittedDraws"), f"{archetype} unaudited draws")
    return {
        "combatObjectArchetypeId": archetype,
        "clientVisualId": expected_visual,
        "effectAssetId": expected_effect,
        "ownerPatternId": object_authority["ownerPatternId"],
        "ownerStageActionId": object_authority["ownerStageActionId"],
        "kind": object_authority["kind"],
        "rootPolicy": expected_root_policy,
        "classification": classification,
        "sourceExactClaim": False,
        "sampleLabels": [label for label, _ in sample_spec],
        "rootWorldDistinctCount": target["rootWorldDistinctCount"],
        "submittedBoundsDistinctCount": target["submittedBoundsDistinctCount"],
        "auditedElements": audited,
        "resources": resources,
        "failedDraws": sum(row["failedDraws"] for row in audited),
    }


def build_proof(
    repository_root: Path,
    expected_resource_root: Path,
    sweep_path: Path | None = None,
) -> dict[str, Any]:
    root = repository_root.resolve()
    resource_root = expected_resource_root.resolve()
    if not root.is_dir() or not resource_root.is_dir():
        raise ProofError("repository or runtime resource root is unavailable")
    canonical_sweep_path = repository_path(root, SWEEP_RELATIVE_PATH)
    actual_sweep_path = (sweep_path or canonical_sweep_path).resolve()
    if actual_sweep_path != canonical_sweep_path:
        raise ProofError("runtime sweep path is not canonical")

    boss_path = repository_path(root, BOSS_CATALOG_RELATIVE_PATH)
    combat_path = repository_path(root, COMBAT_OBJECT_CATALOG_RELATIVE_PATH)
    sky_path = repository_path(root, SKY_EFFECT_RELATIVE_PATH)
    red_path = repository_path(root, RED_EFFECT_RELATIVE_PATH)
    inventory_path = repository_path(root, SOURCE_INVENTORY_RELATIVE_PATH)
    sweep = read_json(actual_sweep_path)
    boss = read_json(boss_path)
    combat = read_json(combat_path)
    sky = read_json(sky_path)
    red = read_json(red_path)
    source_inventory = read_json(inventory_path)

    if (
        sweep.get("schema") != RUNTIME_SWEEP_SCHEMA
        or sweep.get("formatVersion") != FORMAT_VERSION
        or sweep.get("bossArchetypeId") != "BOSS_VALTAN"
        or sweep.get("renderer")
        != {
            "driver": "D3D_DRIVER_TYPE_WARP",
            "vendorId": 5140,
            "deviceId": 140,
        }
        or Path(str(sweep.get("resourceRoot") or "")).resolve() != resource_root
        or sweep.get("disposition") != "DRAWABLE_PROOF_PASS"
    ):
        raise ProofError("combat-object runtime sweep header changed")
    boss_identity = sweep.get("bossCatalog") or {}
    combat_identity = sweep.get("combatObjectCatalog") or {}
    boss_sha = raw_sha256(boss_path)
    combat_sha = raw_sha256(combat_path)
    sky_sha = raw_sha256(sky_path)
    red_sha = raw_sha256(red_path)
    inventory_sha = raw_sha256(inventory_path)
    if (
        Path(str(boss_identity.get("path") or "")).resolve() != boss_path
        or boss_identity.get("rawSha256") != boss_sha
        or Path(str(combat_identity.get("path") or "")).resolve() != combat_path
        or combat_identity.get("rawSha256") != combat_sha
    ):
        raise ProofError("combat-object runtime sweep catalog identity is stale")

    lifecycle = sweep.get("existingLifecycleHarness") or {}
    if lifecycle != {
        "mode": "--valtan-combat-object-presentation-fast",
        "assertionCount": 9,
        "failureCount": 0,
        "facts": LIFECYCLE_FACTS,
    }:
        raise ProofError("existing combat-object lifecycle proof changed")
    owner_boundary = sweep.get("ownerLifetimeBoundary") or {}
    if owner_boundary != {
        "combatObjectArchetypeId": SKY_ARCHETYPE,
        "seconds": 1.9,
        "rendererSampleAttempted": False,
        "stopAppliedByExistingLifecycleHarness": True,
    }:
        raise ProofError("HighJump owner stop boundary changed")

    authority = _validate_catalogs(boss, combat)
    sky_resources, red_resources = _validate_effect_documents(
        sky, red, source_inventory, resource_root
    )
    targets = sweep.get("targets")
    if not isinstance(targets, list) or len(targets) != 2:
        raise ProofError("runtime sweep target denominator changed")
    by_id = {
        target.get("combatObjectArchetypeId"): target
        for target in targets
        if isinstance(target, dict)
    }
    if list(by_id) != [SKY_ARCHETYPE, RED_ARCHETYPE]:
        raise ProofError("runtime sweep target order/identity changed")
    proof_targets = [
        _validate_runtime_target(
            by_id[SKY_ARCHETYPE], sky_path, sky_sha, authority[SKY_ARCHETYPE], sky_resources
        ),
        _validate_runtime_target(
            by_id[RED_ARCHETYPE], red_path, red_sha, authority[RED_ARCHETYPE], red_resources
        ),
    ]

    proof: dict[str, Any] = {
        "schema": PROOF_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "evidence": {
            "kind": "EXECUTED_WARP_DRAWABLE_PROOF",
            "runtimeSweepPath": relative_path(root, actual_sweep_path),
            "runtimeSweepRawSha256": raw_sha256(actual_sweep_path),
            "runtimeSweepCanonicalSha256": canonical_sha256(sweep),
            "renderer": copy.deepcopy(sweep["renderer"]),
            "resourceRoot": "Client/Bin/Resources",
        },
        "inputs": [
            {"role": "BOSS_CATALOG", "path": BOSS_CATALOG_RELATIVE_PATH.as_posix(), "rawSha256": boss_sha},
            {"role": "COMBAT_OBJECT_CATALOG", "path": COMBAT_OBJECT_CATALOG_RELATIVE_PATH.as_posix(), "rawSha256": combat_sha},
            {"role": "SKY_AXE_EFFECT", "path": SKY_EFFECT_RELATIVE_PATH.as_posix(), "rawSha256": sky_sha},
            {"role": "RED_BLADE_EFFECT", "path": RED_EFFECT_RELATIVE_PATH.as_posix(), "rawSha256": red_sha},
            {"role": "SOURCE_OCCURRENCE_INVENTORY", "path": SOURCE_INVENTORY_RELATIVE_PATH.as_posix(), "rawSha256": inventory_sha},
        ],
        "lifecycleReuse": {
            "mode": lifecycle["mode"],
            "assertionCount": lifecycle["assertionCount"],
            "failureCount": lifecycle["failureCount"],
            "facts": list(lifecycle["facts"]),
            "highJumpOwnerStopSeconds": 1.9,
        },
        "targets": proof_targets,
        "safety": {
            "canonicalAuthoringMutationPerformed": False,
            "cueBindingSequenceMutationPerformed": False,
            "colorPipelineMutationPerformed": False,
            "ordinaryDocumentPreviewIsolationUsed": False,
            "exactElementDiagnosticsUsed": True,
            "visualFidelityClaimed": False,
        },
        "disposition": "PROOF_PASS_NO_AUTHORED_MUTATION",
    }
    proof["artifactSha256"] = canonical_sha256(proof)
    return proof


def verify_proof_seal(proof: dict[str, Any]) -> None:
    expected = proof.get("artifactSha256")
    unsealed = copy.deepcopy(proof)
    unsealed.pop("artifactSha256", None)
    if not _is_sha256(expected) or canonical_sha256(unsealed) != expected:
        raise ProofError("combat-object proof artifactSha256 is stale")


def _atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staging = path.with_name(f"{path.name}.tmp.{os.getpid()}")
    try:
        with staging.open("wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(staging, path)
    finally:
        try:
            staging.unlink()
        except FileNotFoundError:
            pass


def write_proof(repository_root: Path, expected_resource_root: Path) -> Path:
    root = repository_root.resolve()
    proof = build_proof(root, expected_resource_root)
    verify_proof_seal(proof)
    output = repository_path(root, PROOF_RELATIVE_PATH)
    _atomic_write(output, pretty_bytes(proof))
    return output


def check_proof(repository_root: Path, expected_resource_root: Path) -> Path:
    root = repository_root.resolve()
    expected = build_proof(root, expected_resource_root)
    verify_proof_seal(expected)
    output = repository_path(root, PROOF_RELATIVE_PATH)
    actual = read_json(output)
    verify_proof_seal(actual)
    if pretty_bytes(actual) != pretty_bytes(expected):
        raise ProofError("combat-object drawable proof is stale")
    return output


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parent.parent.parent,
    )
    parser.add_argument(
        "--expected-resource-root",
        type=Path,
        required=True,
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        output = (
            write_proof(args.repository_root, args.expected_resource_root)
            if args.write
            else check_proof(args.repository_root, args.expected_resource_root)
        )
    except ProofError as error:
        print(f"[FAIL] {error}")
        return 1
    print(f"[PASS] Valtan combat-object drawable proof: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
