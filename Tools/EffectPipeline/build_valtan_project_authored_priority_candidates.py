#!/usr/bin/env python3
"""Build non-destructive project-authored Valtan priority overlays.

The output is deliberately not admitted into EffectCatalog and never edits an
Authored Effect document.  Each ordinary v13 document contains only elements
that are absent from its target document.  The accompanying patch-plan receipt
records existing elements as PRESERVE_EXISTING, so later reconciliation can
append missing stable identities without overwriting hand tuning.
"""

from __future__ import annotations

import argparse
from copy import deepcopy
from dataclasses import dataclass
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import sys
from typing import Any, Iterable, Mapping


RECEIPT_SCHEMA = "lostark.valtan-project-authored-priority-patch-plan"
RECEIPT_VERSION = 1
OWNER_ARCHETYPE_ID = "BOSS_VALTAN"
AUTHORING_SCHEMA = "lostark.effect-authoring"
AUTHORING_VERSION = 13
OUTPUT_RELATIVE_ROOT = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority"
)
RECEIPT_RELATIVE_PATH = OUTPUT_RELATIVE_ROOT / (
    "Valtan.project-authored-priority.patch-plan.v1.json"
)
STABLE_ID_PATTERN = re.compile(r"^[a-z0-9]+(?:[._:-][a-z0-9]+)*$")
SHA256_PATTERN = re.compile(r"^[0-9a-f]{64}$")
HAND_TUNABLE_FIELDS = (
    "detail.color.multiply",
    "detail.linearLerp.endScale",
    "detail.transform.scale",
    "detail.uv.speed",
)


ASSET_LINE_003 = (
    "Effect/Valtan/Textures/FX_TEX_01/fx_c_line_003_xcl.dds"
)
ASSET_ATYPICAL_042 = (
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_042_ycl.dds"
)
ASSET_RING_002 = (
    "Effect/Valtan/Textures/FX_TEX_01/fx_c_ring_002.dds"
)
ASSET_RING_004 = (
    "Effect/Valtan/Textures/FX_TEX_01/fx_c_ring_004_cl.dds"
)
ASSET_ATYPICAL_009 = (
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_009.dds"
)
ASSET_ATYPICAL_011 = (
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_011.dds"
)
ASSET_ATYPICAL_032 = (
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_032.dds"
)
ASSET_ATYPICAL_028 = (
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_028.dds"
)
ASSET_LINE_010 = (
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_line_010_ycl.dds"
)
ASSET_LANDING_ATYPICAL = (
    "Effect/Valtan/Textures/FX_TEX_04/fx_h_atypical_01_1.dds"
)
ASSET_GROUND_DECAL = (
    "Effect/Valtan/Textures/FX_TEX_HIGH_00/fx_b_decal_001.dds"
)
ASSET_HIGH_WAVE = (
    "Effect/Valtan/Textures/FX_H_W_01/fx_h_wave_04.dds"
)
ASSET_SHOCKWAVE_02 = (
    "Effect/Valtan/Textures/FX_TEX_04/fx_i_shockwave_02_ycl.dds"
)


class ContractError(RuntimeError):
    """Raised when an input or generated candidate violates the contract."""


@dataclass(frozen=True)
class TargetSpec:
    pattern_id: str
    stage_id: str
    action_id: str
    clip_occurrence_id: str
    effect_asset_id: str
    disposition: str
    presentation_only: bool
    elements: tuple[dict[str, Any], ...]
    excluded_source_occurrence_ids: tuple[str, ...] = ()
    notes: tuple[str, ...] = ()


@dataclass(frozen=True)
class BuildArtifacts:
    files: Mapping[PurePosixPath, bytes]
    receipt: dict[str, Any]
    documents: Mapping[str, dict[str, Any]]


def _json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False)
        + "\n"
    ).encode("utf-8")


def _sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _sha256_json(value: Any) -> str:
    return _sha256_bytes(_json_bytes(value))


def _load_json(path: Path) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise ContractError(f"cannot read JSON {path}: {exc}") from exc
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ContractError(f"JSON must be UTF-8 without BOM: {path}")

    def no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            if key in result:
                raise ContractError(f"duplicate JSON property {key!r}: {path}")
            result[key] = value
        return result

    try:
        value = json.loads(
            payload.decode("utf-8"), object_pairs_hook=no_duplicates
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"invalid JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def _resource_binding(slot_id: str, asset_id: str) -> dict[str, str]:
    return {"slotId": slot_id, "assetId": asset_id}


def _disabled_action_attachment() -> dict[str, Any]:
    return {
        "enabled": False,
        "follow": False,
        "sourceAnchorSlotId": "",
        "runtimeAnchorSlotId": "",
        "runtimeBoneName": "",
        "snapshotRootSourceBasisYawDegrees": 0,
        "socketLocalTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }


def _detail(
    *,
    position: tuple[float, float, float] = (0.0, 0.05, 0.0),
    rotation: tuple[float, float, float] = (0.0, 0.0, 0.0),
    scale: tuple[float, float, float] = (1.0, 1.0, 1.0),
    color: tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    emissive: float = 1.0,
    start_delay: float = 0.0,
    life_time: float = 0.6,
    dissolve_start: float = 0.75,
    decal_size: tuple[float, float] = (1.0, 1.0),
    decal_depth: float = 1.0,
    lerp_scale: tuple[float, float, float] | None = None,
    uv_speed: tuple[float, float] = (0.0, 0.0),
    billboard: bool = True,
    billboard_roll: float = 0.0,
    particle_size: tuple[float, float] = (1.0, 1.0),
    particle_end_size: tuple[float, float] | None = None,
) -> dict[str, Any]:
    end_particle_size = particle_end_size or particle_size
    return {
        "transform": {
            "position": list(position),
            "rotationDegrees": list(rotation),
            "revolutionDegreesPerSecond": [0, 0, 0],
            "scale": list(scale),
            "velocityPerSecond": [0, 0, 0],
        },
        "color": {
            "offset": [0, 0, 0, 0],
            "multiply": list(color),
            "clip": 0,
            "emissiveIntensity": emissive,
            "distortionIntensity": 0,
            "distortionOnBaseMaterial": False,
            "radialTime": 0,
            "radialIntensity": 0,
        },
        "uv": {
            "start": [0, 0],
            "speed": list(uv_speed),
            "wave": False,
            "waveAmplitude": [0, 0],
            "waveFrequency": 1,
            "sequence": False,
            "loop": False,
            "sequenceTerm": 0.1,
            "tileColumns": 1,
            "tileRows": 1,
            "tileIndex": 0,
        },
        "timing": {
            "startDelaySeconds": start_delay,
            "lifeTimeSeconds": life_time,
            "afterImageSeconds": 0,
            "dissolveStartNormalized": dissolve_start,
        },
        "mesh": {
            "useModelMaterial": False,
            "sourceTypeDataRotationDegrees": [0, 0, 0],
        },
        "sprite": {
            "billboard": billboard,
            "billboardRollDegrees": billboard_roll,
        },
        "decal": {"size": list(decal_size), "depth": decal_depth},
        "linearLerp": {
            "position": False,
            "endPosition": [0, 0, 0],
            "rotation": False,
            "endRotationDegrees": [0, 0, 0],
            "revolution": False,
            "endRevolutionDegreesPerSecond": [0, 0, 0],
            "scale": lerp_scale is not None,
            "endScale": list(lerp_scale or (1.0, 1.0, 1.0)),
            "velocity": False,
            "endVelocityPerSecond": [0, 0, 0],
            "colorOffset": False,
            "endColorOffset": [0, 0, 0, 0],
            "colorMultiply": False,
            "endColorMultiply": [1, 1, 1, 1],
            "emissiveIntensity": False,
            "endEmissiveIntensity": 1,
        },
        "particle": {
            "maxParticles": 1,
            "spawnRatePerSecond": 0,
            "burstCount": 1,
            "randomSeed": 1,
            "lifeTimeSeconds": [life_time, life_time],
            "initialPositionMin": [0, 0, 0],
            "initialPositionMax": [0, 0, 0],
            "initialVelocityMin": [0, 0, 0],
            "initialVelocityMax": [0, 0, 0],
            "acceleration": [0, 0, 0],
            "startSize": list(particle_size),
            "endSize": list(end_particle_size),
            "localSpace": True,
            "billboard": billboard,
        },
        "trail": {
            "maxPoints": 64,
            "pointLifeTimeSeconds": 0.35,
            "sampleIntervalSeconds": 0.0166666675,
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
    }


def _element(
    *,
    element_id: str,
    display_name: str,
    group_id: str,
    source_node: str,
    kind: str,
    resources: Iterable[tuple[str, str]],
    render_profile: str,
    detail: dict[str, Any],
) -> dict[str, Any]:
    return {
        "id": element_id,
        "displayName": display_name,
        "groupId": group_id,
        "sourceNode": source_node,
        "visible": True,
        "kind": kind,
        "resources": [_resource_binding(*binding) for binding in resources],
        "material": {
            "templateId": "effect.standard",
            "sourceMaterialPath": "",
            "renderProfile": render_profile,
            "sourceProfile": {"enabled": False},
        },
        "actionCueAttachment": _disabled_action_attachment(),
        "transformInheritance": {"enabled": False, "masterElementId": ""},
        "detail": detail,
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


def _decal(
    *,
    element_id: str,
    display_name: str,
    group_id: str,
    source_node: str,
    base: str,
    size: tuple[float, float],
    color: tuple[float, float, float, float],
    start_delay: float,
    life_time: float,
    position: tuple[float, float, float] = (0.0, 0.05, 0.0),
    rotation: tuple[float, float, float] = (0.0, 0.0, 0.0),
    scale: tuple[float, float, float] = (1.0, 1.0, 1.0),
    end_scale: tuple[float, float, float] | None = None,
    mask: str | None = None,
    noise: str | None = None,
    dissolve: str | None = None,
    additive: bool = False,
    emissive: float = 1.5,
    uv_speed: tuple[float, float] = (0.0, 0.0),
) -> dict[str, Any]:
    resources: list[tuple[str, str]] = [("base", base)]
    if mask is not None:
        resources.append(("mask", mask))
    if noise is not None:
        resources.append(("noise", noise))
    if dissolve is not None:
        resources.append(("dissolve", dissolve))
    return _element(
        element_id=element_id,
        display_name=display_name,
        group_id=group_id,
        source_node=source_node,
        kind="decal",
        resources=resources,
        render_profile=(
            "additive_two_sided_depth_read"
            if additive
            else "alpha_two_sided_depth_read"
        ),
        detail=_detail(
            position=position,
            rotation=rotation,
            scale=scale,
            color=color,
            emissive=emissive,
            start_delay=start_delay,
            life_time=life_time,
            dissolve_start=0.72,
            decal_size=size,
            decal_depth=1.0,
            lerp_scale=end_scale,
            uv_speed=uv_speed,
            billboard=False,
        ),
    )


def _sprite_particle(
    *,
    element_id: str,
    display_name: str,
    group_id: str,
    source_node: str,
    base: str,
    color: tuple[float, float, float, float],
    start_delay: float,
    life_time: float,
    position: tuple[float, float, float],
    size: tuple[float, float],
    end_size: tuple[float, float],
    roll: float,
) -> dict[str, Any]:
    return _element(
        element_id=element_id,
        display_name=display_name,
        group_id=group_id,
        source_node=source_node,
        kind="particle",
        resources=(("base", base),),
        render_profile="additive_two_sided_depth_read",
        detail=_detail(
            position=position,
            color=color,
            emissive=1.7,
            start_delay=start_delay,
            life_time=life_time,
            dissolve_start=0.62,
            billboard=True,
            billboard_roll=roll,
            particle_size=size,
            particle_end_size=end_size,
        ),
    )


def _floor_axis_elements(*, impact: bool) -> tuple[dict[str, Any], ...]:
    phase = "impact" if impact else "telegraph"
    group = "six_direction_impact" if impact else "six_direction_telegraph"
    prefix = "six-direction-impact" if impact else "six-direction-telegraph"
    life_time = 0.8 if impact else 1.8
    color = (0.06, 0.95, 0.72, 0.78) if impact else (1.0, 0.12, 0.02, 0.58)
    return tuple(
        _decal(
            element_id=f"{prefix}.axis-{degrees:03d}",
            display_name=f"6-direction {phase} / {degrees} degree axis",
            group_id=group,
            source_node=(
                f"project-authored:valtan.floor-wipe-130.{phase}.axis-{degrees:03d}"
            ),
            base=ASSET_LINE_010,
            size=(4.4, 28.0),
            color=color,
            start_delay=0.0,
            life_time=life_time,
            rotation=(0.0, float(degrees), 0.0),
            additive=impact,
        )
        for degrees in (0, 60, 120)
    )


def _target_specs() -> tuple[TargetSpec, ...]:
    red = (1.0, 0.08, 0.02, 0.72)
    red_soft = (1.0, 0.12, 0.04, 0.48)
    teal = (0.02, 1.0, 0.82, 0.9)
    green = (0.12, 1.0, 0.35, 0.62)

    dash = (
        _decal(
            element_id="project-dash-forward-line",
            display_name="Dash forward line telegraph",
            group_id="forward_telegraph",
            source_node="project-authored:valtan.dash-charge.forward-line",
            base=ASSET_LINE_003,
            size=(3.2, 14.0),
            color=red,
            start_delay=0.0,
            life_time=0.6,
            position=(0.0, 0.055, 7.0),
            scale=(0.25, 1.0, 1.0),
            end_scale=(1.0, 1.0, 1.0),
            emissive=1.4,
        ),
        _decal(
            element_id="project-dash-forward-pulse",
            display_name="Dash forward atypical pulse",
            group_id="forward_telegraph",
            source_node="project-authored:valtan.dash-charge.forward-pulse",
            base=ASSET_ATYPICAL_042,
            size=(5.0, 10.0),
            color=red_soft,
            start_delay=0.05,
            life_time=0.5,
            position=(0.0, 0.06, 6.0),
            additive=True,
            emissive=1.2,
        ),
    )

    magic_windup = (
        _decal(
            element_id="project-donut-outer-boundary",
            display_name="Donut outer red boundary",
            group_id="donut_telegraph",
            source_node="project-authored:valtan.magic-choice.donut.outer-boundary",
            base=ASSET_RING_002,
            mask=ASSET_RING_004,
            size=(18.0, 18.0),
            color=red,
            start_delay=0.0,
            life_time=1.4,
            emissive=1.6,
            uv_speed=(0.08, 0.0),
        ),
        _decal(
            element_id="project-donut-inner-growing-boundary",
            display_name="Donut inner boundary growing to outer radius",
            group_id="donut_telegraph",
            source_node="project-authored:valtan.magic-choice.donut.inner-growing-boundary",
            base=ASSET_RING_002,
            mask=ASSET_RING_004,
            size=(7.0, 7.0),
            color=red,
            start_delay=0.0,
            life_time=1.4,
            end_scale=(18.0 / 7.0, 18.0 / 7.0, 1.0),
            emissive=1.7,
            uv_speed=(-0.12, 0.0),
        ),
    )
    magic_inner = (
        _decal(
            element_id="project-donut-inner-impact",
            display_name="Donut inner impact",
            group_id="donut_impact",
            source_node="project-authored:valtan.magic-choice.donut.inner-impact",
            base=ASSET_ATYPICAL_009,
            mask=ASSET_RING_002,
            dissolve=ASSET_ATYPICAL_011,
            size=(8.0, 8.0),
            color=(1.0, 0.18, 0.04, 0.9),
            start_delay=0.0,
            life_time=0.7,
            additive=True,
            emissive=2.0,
        ),
    )
    magic_outer = (
        _decal(
            element_id="project-donut-outer-impact",
            display_name="Donut outer ring impact",
            group_id="donut_impact",
            source_node="project-authored:valtan.magic-choice.donut.outer-impact",
            base=ASSET_ATYPICAL_009,
            mask=ASSET_RING_002,
            dissolve=ASSET_ATYPICAL_011,
            size=(18.0, 18.0),
            color=(1.0, 0.14, 0.02, 0.9),
            start_delay=0.0,
            life_time=0.8,
            additive=True,
            emissive=2.1,
        ),
    )

    floor_windup = _floor_axis_elements(impact=False) + (
        _decal(
            element_id="project-floor-six-direction-center-guide",
            display_name="Six-direction center guide",
            group_id="six_direction_telegraph",
            source_node="project-authored:valtan.floor-wipe-130.telegraph.center-guide",
            base=ASSET_ATYPICAL_032,
            size=(4.5, 4.5),
            color=red_soft,
            start_delay=0.0,
            life_time=1.8,
            emissive=1.4,
        ),
    )
    floor_impact = _floor_axis_elements(impact=True) + (
        _decal(
            element_id="project-floor-six-direction-center-impact",
            display_name="Six-direction center impact and dissolve",
            group_id="six_direction_impact",
            source_node="project-authored:valtan.floor-wipe-130.impact.center",
            base=ASSET_ATYPICAL_009,
            mask=ASSET_ATYPICAL_032,
            noise=ASSET_ATYPICAL_028,
            dissolve=ASSET_ATYPICAL_011,
            size=(6.0, 6.0),
            color=teal,
            start_delay=0.05,
            life_time=0.75,
            additive=True,
            emissive=2.0,
        ),
    )

    high_jump_airborne = tuple(
        _decal(
            element_id=f"project-axe-beat-{beat:02d}-target-decal",
            display_name=f"Axe beat {beat} target decal",
            group_id="axe_drop_presentation",
            source_node=(
                f"project-authored:valtan.high-jump.airborne.beat-{beat:02d}.target-decal"
            ),
            base=ASSET_ATYPICAL_032,
            size=(4.2, 4.2),
            color=green,
            start_delay=(beat - 1) * 0.55,
            life_time=0.62,
            position=((-4.0, 0.05, 4.0), (4.0, 0.05, 4.0), (0.0, 0.05, 8.0))[beat - 1],
            emissive=1.5,
        )
        for beat in (1, 2, 3)
    )
    high_jump_land = (
        _decal(
            element_id="project-high-jump-landing-wave",
            display_name="Valtan landing outward wave",
            group_id="valtan_landing",
            source_node="project-authored:valtan.high-jump.land.outward-wave",
            base=ASSET_LANDING_ATYPICAL,
            mask=ASSET_HIGH_WAVE,
            size=(14.0, 14.0),
            color=teal,
            start_delay=0.0,
            life_time=1.0,
            scale=(0.35, 0.35, 1.0),
            end_scale=(1.0, 1.0, 1.0),
            additive=True,
            emissive=2.2,
            uv_speed=(0.35, 0.0),
        ),
    )

    slash_delays = (0.270, 1.354, 2.324)
    slash_rolls = (-35.0, 35.0, 0.0)
    slash_positions = ((-1.8, 1.4, 2.2), (1.8, 1.4, 2.2), (0.0, 1.8, 3.0))
    front_slashes = tuple(
        _sprite_particle(
            element_id=f"project-three-hit-slash-{index:02d}",
            display_name=f"Three-hit teal slash {index}",
            group_id="three_hit_supplement",
            source_node=(
                f"project-authored:valtan.front-back-front.hit-{index:02d}.slash-wave"
            ),
            base=ASSET_HIGH_WAVE,
            color=teal,
            start_delay=slash_delays[index - 1],
            life_time=0.42,
            position=slash_positions[index - 1],
            size=(5.0, 1.15),
            end_size=(8.0, 1.7),
            roll=slash_rolls[index - 1],
        )
        for index in (1, 2, 3)
    )
    front_impact = (
        _decal(
            element_id="project-three-hit-down-smash-decal",
            display_name="Third-hit ground decal",
            group_id="three_hit_supplement",
            source_node="project-authored:valtan.front-back-front.hit-03.ground-decal",
            base=ASSET_GROUND_DECAL,
            size=(9.0, 9.0),
            color=(0.04, 0.9, 0.78, 0.72),
            start_delay=2.324,
            life_time=1.0,
            emissive=1.4,
        ),
        _decal(
            element_id="project-three-hit-down-smash-wave",
            display_name="Third-hit outward ground wave",
            group_id="three_hit_supplement",
            source_node="project-authored:valtan.front-back-front.hit-03.ground-wave",
            base=ASSET_HIGH_WAVE,
            size=(12.0, 12.0),
            color=teal,
            start_delay=2.35,
            life_time=0.9,
            scale=(0.35, 0.35, 1.0),
            end_scale=(1.0, 1.0, 1.0),
            additive=True,
            emissive=2.0,
        ),
        _decal(
            element_id="project-three-hit-down-smash-shockwave",
            display_name="Third-hit supplemental ground shockwave",
            group_id="three_hit_supplement",
            source_node=(
                "project-authored:valtan.front-back-front.hit-03.ground-shockwave"
            ),
            base=ASSET_SHOCKWAVE_02,
            size=(10.0, 10.0),
            color=(0.02, 1.0, 0.82, 0.88),
            start_delay=2.36,
            life_time=0.72,
            scale=(0.25, 0.25, 1.0),
            end_scale=(1.0, 1.0, 1.0),
            additive=True,
            emissive=2.1,
            uv_speed=(0.16, 0.0),
        ),
    )

    return (
        TargetSpec(
            "VALTAN_DASH_CHARGE", "WINDUP",
            "valtan.attack.dash-charge.windup",
            "valtan.attack.dash-charge.windup.clip.01",
            "effect.valtan.dash-charge.windup",
            "PROJECT_TUNED_OVERRIDE", False, dash,
            notes=(
                "fx_c_line_003_xcl is a physically verified PROJECT_TUNED red forward-path telegraph; its exact source evidence belongs to FLOOR_WIPE action 420630 / par_o_rpbf_atk_09_02 / Att_Battle_15_03, not Dash.",
                "The Dash source-exact par_s_rpbf_dash_01_1 core uses fm_h_halfsphere_01_1 and fm_d_hemisphere_001_1; this path overlay stays separate and does not duplicate or claim that shield core.",
                "fx_d_atypical_042_ycl remains a physically verified project-tuned pulse; no shield mesh is inferred by filename or silhouette.",
            ),
        ),
        TargetSpec(
            "VALTAN_MAGIC_CHOICE", "WINDUP",
            "valtan.attack.magic-choice.windup",
            "valtan.attack.magic-choice.windup.clip.01",
            "effect.valtan.magic-choice.windup",
            "PROJECT_TUNED_OVERRIDE", False, magic_windup,
            notes=(
                "Both red boundaries combine fx_c_ring_002 and fx_c_ring_004_cl as PROJECT_TUNED candidates; the inner ring grows to the outer boundary while UV speed drives an annulus sweep.",
                "fx_c_ring_002 has exact Magic Choice evidence only at outer-end par_o_rpbf_atk_03_03 / Att_Battle_5_02_End; it does not make both candidate boundaries source-exact.",
                "fx_c_ring_004_cl is an exact transition texture in the Dash halfsphere/hemisphere material context, not a Magic Choice source join.",
            ),
        ),
        TargetSpec(
            "VALTAN_MAGIC_CHOICE", "INNER",
            "valtan.attack.magic-choice.inner",
            "valtan.attack.magic-choice.inner.clip.01",
            "effect.valtan.magic-choice.inner",
            "PROJECT_TUNED_OVERRIDE", False, magic_inner,
        ),
        TargetSpec(
            "VALTAN_MAGIC_CHOICE", "OUTER",
            "valtan.attack.magic-choice.outer",
            "valtan.attack.magic-choice.outer.clip.01",
            "effect.valtan.magic-choice.outer",
            "PROJECT_TUNED_OVERRIDE", False, magic_outer,
        ),
        TargetSpec(
            "VALTAN_FLOOR_WIPE_130", "WINDUP",
            "valtan.mechanic.floor-wipe-130.windup",
            "valtan.mechanic.floor-wipe-130.windup.clip.01",
            "effect.valtan.floor-wipe-130.windup",
            "PROJECT_AUTHORED", False, floor_windup,
            excluded_source_occurrence_ids=(
                "source-only:mesh_att_battle_15_03:Atk09_02",
            ),
            notes=(
                "Existing project-authored 0/60/120 degree axis rows are preservation sentinels and form six directions without importing source-only mesh_att_battle_15_03.",
                "Color, UV speed, and scale remain direct-authored hand-tuning fields.",
            ),
        ),
        TargetSpec(
            "VALTAN_FLOOR_WIPE_130", "FIRST_SMASH",
            "valtan.mechanic.floor-wipe-130.first-smash",
            "valtan.mechanic.floor-wipe-130.first-smash.clip.01",
            "effect.valtan.floor-wipe-130.first-smash",
            "PROJECT_AUTHORED", False, floor_impact,
            excluded_source_occurrence_ids=(
                "source-only:mesh_att_battle_15_03:Atk09_02",
            ),
            notes=(
                "The center impact exposes fx_d_atypical_009 diffuse, fx_d_atypical_032 mask, and fx_d_atypical_028 noise as hand-tunable project-authored slots; all occur in exact Atk09_02 evidence, but this overlay does not promote the unreachable 15_03 occurrence.",
                "fx_c_line_003_xcl remains exact evidence for the excluded 15_03 source renderer while existing project-authored axis rows are preserved.",
                "Color, UV speed, and scale remain direct-authored hand-tuning fields.",
            ),
        ),
        TargetSpec(
            "VALTAN_HIGH_JUMP", "AIRBORNE",
            "valtan.attack.high-jump.airborne",
            "valtan.attack.high-jump.airborne.clip.01",
            "effect.valtan.high-jump.airborne",
            "PROJECT_AUTHORED", True, high_jump_airborne,
            notes=(
                "Three root-relative target positions are hand-tuning candidates, not Server target selection or damage actors.",
                "Axe projectile meshes are intentionally absent until a stable payload and Server/presentation authority join are proven.",
            ),
        ),
        TargetSpec(
            "VALTAN_HIGH_JUMP", "LAND",
            "valtan.attack.high-jump.land",
            "valtan.attack.high-jump.land.clip.01",
            "effect.valtan.high-jump.land",
            "PROJECT_TUNED_OVERRIDE", True, high_jump_land,
            notes=(
                "fx_h_atypical_01_1 is a physically verified landing-wave tuning candidate, not a source-exact claim.",
            ),
        ),
        TargetSpec(
            "VALTAN_FRONT_BACK_FRONT", "SMASHES",
            "valtan.attack.front-back-front.active",
            "valtan.attack.front-back-front.active.clip.01",
            "effect.valtan.front-back-front.active",
            "PROJECT_TUNED_OVERRIDE", False, front_slashes + front_impact,
            notes=(
                "fx_b_decal_001 and fx_h_wave_04 do not source-join action 420637 and remain explicit project-tuned supplements.",
                "fx_i_shockwave_02_ycl is physically verified but has no Front/Back/Front action 420637 source join, so the final ground shockwave remains PROJECT_TUNED.",
                "Only the three gameplay-aligned waves are authored; the fourth source visual wave remains outside this overlay.",
            ),
        ),
    )


def _resource_role_dispositions() -> list[dict[str, Any]]:
    """Record reviewed source context without upgrading candidates to exact."""
    rows = (
        (
            ASSET_LINE_003,
            "DASH_FORWARD_RED_PATH",
            "EXACT_OTHER_PATTERN",
            (
                "Exact evidence is FLOOR_WIPE action 420630 / "
                "FX_MN_RPBF_00_O.par_o_rpbf_atk_09_02 / Att_Battle_15_03 / "
                "fx_j_de_master_01_17_tr 01.heightmap; Dash use is not exact."
            ),
            ("effect.valtan.dash-charge.windup",),
        ),
        (
            ASSET_RING_002,
            "MAGIC_DONUT_RING_BOUNDARIES",
            "EXACT_SAME_PATTERN_LIMITED_STAGE",
            (
                "Exact Magic Choice evidence is limited to outer-end "
                "FX_MN_RPBF_00_O.par_o_rpbf_atk_03_03 / "
                "Att_Battle_5_02_End; both authored boundaries remain tuned."
            ),
            (
                "effect.valtan.magic-choice.inner",
                "effect.valtan.magic-choice.outer",
                "effect.valtan.magic-choice.windup",
            ),
        ),
        (
            ASSET_RING_004,
            "MAGIC_DONUT_ANNULUS_UV_SWEEP",
            "EXACT_OTHER_PATTERN",
            (
                "Exact context is the Dash par_s_rpbf_dash_01_1 "
                "halfsphere/hemisphere material fx_h_me_fd_01_1_ts_tr "
                "transition texture; Magic Choice use is not exact."
            ),
            ("effect.valtan.magic-choice.windup",),
        ),
        (
            ASSET_ATYPICAL_009,
            "FLOOR_WIPE_IMPACT_DIFFUSE",
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
            (
                "Exact Atk09_02 evidence binds fx_d_atypical_009 as diff_tex1 "
                "on fx_o_me_makeflow_02_22_tr; the reachable authored center "
                "impact is a tuned approximation."
            ),
            ("effect.valtan.floor-wipe-130.first-smash",),
        ),
        (
            ASSET_ATYPICAL_032,
            "FLOOR_WIPE_GUIDE_AND_IMPACT_MASK",
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
            (
                "Exact Atk09_02 evidence binds fx_d_atypical_032 as mask_tex2 "
                "on fx_o_pa_waterflow_01_14_tr; guide and center-mask uses "
                "remain independently hand tuned."
            ),
            (
                "effect.valtan.floor-wipe-130.first-smash",
                "effect.valtan.floor-wipe-130.windup",
            ),
        ),
        (
            ASSET_ATYPICAL_028,
            "FLOOR_WIPE_IMPACT_NOISE",
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
            (
                "Reviewed Atk09_02 evidence exists at fx_o_pa_ap_01_1_tr "
                "01.map_a, so this resource is an explicit tuned noise "
                "candidate rather than deferred."
            ),
            ("effect.valtan.floor-wipe-130.first-smash",),
        ),
        (
            ASSET_SHOCKWAVE_02,
            "FRONT_BACK_FRONT_FINAL_GROUND_IMPACT",
            "NO_TARGET_SOURCE_JOIN",
            (
                "The reviewed source catalog has no action 420637 join for "
                "fx_i_shockwave_02_ycl; the third-hit ground supplement is "
                "therefore project tuned."
            ),
            ("effect.valtan.front-back-front.active",),
        ),
    )
    return [
        {
            "assetId": asset_id,
            "requestedRole": requested_role,
            "evidenceStatus": evidence_status,
            "sourceEvidenceContext": source_context,
            "candidateDisposition": "PROJECT_TUNED_CANDIDATE",
            "candidateEffectAssetIds": list(candidate_effect_ids),
            "handTunableFields": list(HAND_TUNABLE_FIELDS),
        }
        for (
            asset_id,
            requested_role,
            evidence_status,
            source_context,
            candidate_effect_ids,
        ) in rows
    ]


def _validate_reviewed_source_context(repo_root: Path) -> None:
    """Fail closed if the catalog no longer supports the receipt wording."""
    catalog_path = (
        repo_root
        / "Data/Effects/Imported/Valtan/Valtan.action-particle-resource-catalog.json"
    )
    catalog = _load_json(catalog_path)
    assets = catalog.get("assets")
    systems = catalog.get("sourceSystems")
    materials = catalog.get("materialParameterBindings")
    if not all(isinstance(rows, list) for rows in (assets, systems, materials)):
        raise ContractError("Valtan source resource catalog structure changed")

    asset_by_path = {
        row.get("sourceAssetPath"): row
        for row in assets
        if isinstance(row, dict) and isinstance(row.get("sourceAssetPath"), str)
    }
    system_by_path = {
        row.get("sourceAsset"): row
        for row in systems
        if isinstance(row, dict) and isinstance(row.get("sourceAsset"), str)
    }
    material_by_path = {
        row.get("sourceMaterialPath"): row
        for row in materials
        if isinstance(row, dict)
        and isinstance(row.get("sourceMaterialPath"), str)
    }

    def require_asset_join(
        source_asset_path: str,
        *,
        action_id: int | None,
        source_system: str | None,
        forbidden_action_id: int | None = None,
    ) -> None:
        row = asset_by_path.get(source_asset_path)
        if not isinstance(row, dict):
            raise ContractError(f"reviewed source asset disappeared: {source_asset_path}")
        action_ids = row.get("actionIds")
        source_systems = row.get("sourceSystems")
        if (
            not isinstance(action_ids, list)
            or not isinstance(source_systems, list)
            or (action_id is not None and action_id not in action_ids)
            or (source_system is not None and source_system not in source_systems)
            or (
                forbidden_action_id is not None
                and forbidden_action_id in action_ids
            )
        ):
            raise ContractError(
                f"reviewed source join changed for {source_asset_path}"
            )

    floor_system = "FX_MN_RPBF_00_O.par_o_rpbf_atk_09_02"
    magic_outer_end_system = "FX_MN_RPBF_00_O.par_o_rpbf_atk_03_03"
    dash_system = "FX_MN_RPBF_00_S.par_s_rpbf_dash_01_1"
    require_asset_join(
        "fx_tex_01.fx_c_line_003_xcl",
        action_id=420630,
        source_system=floor_system,
    )
    require_asset_join(
        "fx_tex_01.fx_c_ring_002",
        action_id=420608,
        source_system=magic_outer_end_system,
    )
    require_asset_join(
        "fx_tex_01.fx_c_ring_004_cl",
        action_id=420604,
        source_system=dash_system,
    )
    for source_asset_path in (
        "fx_tex_02.fx_d_atypical_009",
        "fx_tex_02.fx_d_atypical_028",
        "fx_tex_02.fx_d_atypical_032",
    ):
        require_asset_join(
            source_asset_path,
            action_id=420630,
            source_system=floor_system,
        )
    require_asset_join(
        "fx_tex_04.fx_i_shockwave_02_ycl",
        action_id=None,
        source_system=None,
        forbidden_action_id=420637,
    )

    def require_system(
        source_system: str,
        *,
        action_id: int,
        clip_name: str,
        resource_bindings: tuple[str, ...] = (),
    ) -> None:
        row = system_by_path.get(source_system)
        graph = row.get("graph", {}) if isinstance(row, dict) else {}
        bound_paths = {
            binding.get("objectPath")
            for binding in graph.get("resourceBindings", [])
            if isinstance(binding, dict)
        }
        if (
            not isinstance(row, dict)
            or action_id not in row.get("actionIds", [])
            or clip_name not in row.get("clipNames", [])
            or any(path not in bound_paths for path in resource_bindings)
        ):
            raise ContractError(f"reviewed source system changed: {source_system}")

    require_system(
        floor_system,
        action_id=420630,
        clip_name="Att_Battle_15_03",
    )
    require_system(
        magic_outer_end_system,
        action_id=420608,
        clip_name="Att_Battle_5_02_End",
    )
    require_system(
        dash_system,
        action_id=420604,
        clip_name="Att_Battle_4_01",
        resource_bindings=(
            "fx_m_mi_01.fx_mi.fx_h_me_fd_01_1_ts_tr",
            "fx_sm_00.fm_h_halfsphere_01_1",
            "fx_sm_00.fm_d_hemisphere_001_1",
        ),
    )

    def require_texture_parameter(
        source_material_path: str,
        parameter_name: str,
        texture_path: str,
    ) -> None:
        row = material_by_path.get(source_material_path)
        textures = row.get("textures", []) if isinstance(row, dict) else []
        if not any(
            isinstance(texture, dict)
            and texture.get("name") == parameter_name
            and texture.get("texture") == texture_path
            for texture in textures
        ):
            raise ContractError(
                "reviewed source material parameter changed: "
                f"{source_material_path} {parameter_name}"
            )

    require_texture_parameter(
        "fx_m_mi_04.fx_mi.fx_j_de_master_01_17_tr",
        "01.heightmap",
        "fx_tex_01.fx_c_line_003_xcl",
    )
    require_texture_parameter(
        "fx_m_mi_01.fx_mi.fx_j_pa_circlelenz_02_ad",
        "emissive_tex",
        "fx_tex_01.fx_c_ring_002",
    )
    require_texture_parameter(
        "fx_m_mi_01.fx_mi.fx_h_me_fd_01_1_ts_tr",
        "transition texture",
        "fx_tex_01.fx_c_ring_004_cl",
    )
    require_texture_parameter(
        "fx_m_mi_o_00.fx_mi.fx_o_me_makeflow_02_22_tr",
        "diff_tex1",
        "fx_tex_02.fx_d_atypical_009",
    )
    require_texture_parameter(
        "fx_m_mi_o_00.fx_mi.fx_o_pa_ap_01_1_tr",
        "01.map_a",
        "fx_tex_02.fx_d_atypical_028",
    )
    require_texture_parameter(
        "fx_m_mi_o_00.fx_mi.fx_o_pa_waterflow_01_14_tr",
        "mask_tex2",
        "fx_tex_02.fx_d_atypical_032",
    )


def _canonical_authoring_path(effect_asset_id: str) -> PurePosixPath:
    return PurePosixPath(
        f"Data/Effects/Authored/{effect_asset_id}.effect.json"
    )


def _overlay_relative_path(effect_asset_id: str) -> PurePosixPath:
    return OUTPUT_RELATIVE_ROOT / (
        f"{effect_asset_id}.project-authored-overlay.effect.json"
    )


def _candidate_document(
    effect_asset_id: str, elements: list[dict[str, Any]]
) -> dict[str, Any]:
    short_name = effect_asset_id.removeprefix("effect.valtan.")
    return {
        "schema": AUTHORING_SCHEMA,
        "version": AUTHORING_VERSION,
        "effectAssetId": effect_asset_id,
        "displayName": f"Project overlay: {short_name}",
        "particleSystem": {
            "uniformScaleMultiplier": 1,
            "yawOffsetDegrees": 0,
            "directionYawDegrees": 0,
            "initialSpeedMultiplier": 1,
        },
        "modelCues": [],
        "elements": elements,
    }


def _find_resource_root(repo_root: Path, explicit: Path | None) -> Path:
    candidates: list[Path] = []
    if explicit is not None:
        candidates.append(explicit)
    environment_root = os.environ.get("LOSTARK_RESOURCE_ROOT")
    if environment_root:
        candidates.append(Path(environment_root))
    candidates.append(repo_root / "Client/Bin/Resources")
    candidates.extend(
        ancestor / "Client/Bin/Resources" for ancestor in repo_root.parents
    )
    seen: set[str] = set()
    for candidate in candidates:
        normalized = str(candidate.resolve(strict=False)).casefold()
        if normalized in seen:
            continue
        seen.add(normalized)
        if candidate.is_dir():
            return candidate.resolve()
    raise ContractError(
        "cannot resolve the user-managed Client/Bin/Resources root; pass --resource-root"
    )


def _safe_resource_path(resource_root: Path, asset_id: str) -> Path:
    relative = PurePosixPath(asset_id)
    if (
        relative.is_absolute()
        or "\\" in asset_id
        or ":" in asset_id
        or any(part in ("", ".", "..") for part in relative.parts)
    ):
        raise ContractError(f"unsafe Resources-relative asset ID: {asset_id}")
    candidate = resource_root.joinpath(*relative.parts)
    resolved = candidate.resolve(strict=False)
    try:
        resolved.relative_to(resource_root.resolve())
    except ValueError as exc:
        raise ContractError(f"asset escapes Resources root: {asset_id}") from exc
    return resolved


def _verify_resources(
    resource_root: Path, targets: Iterable[TargetSpec]
) -> list[dict[str, Any]]:
    asset_ids = sorted(
        {
            resource["assetId"]
            for target in targets
            for element in target.elements
            for resource in element["resources"]
        }
    )
    verified: list[dict[str, Any]] = []
    for asset_id in asset_ids:
        path = _safe_resource_path(resource_root, asset_id)
        if not path.is_file():
            raise ContractError(f"required candidate resource is missing: {asset_id}")
        try:
            payload = path.read_bytes()
        except OSError as exc:
            raise ContractError(f"cannot read candidate resource {asset_id}: {exc}") from exc
        if not payload:
            raise ContractError(f"candidate resource is empty: {asset_id}")
        verified.append(
            {
                "assetId": asset_id,
                "byteSize": len(payload),
                "sha256": _sha256_bytes(payload),
            }
        )
    return verified


def _validate_canonical_document(
    document: dict[str, Any], effect_asset_id: str, path: PurePosixPath
) -> None:
    if (
        document.get("schema") != AUTHORING_SCHEMA
        or type(document.get("version")) is not int
        or document.get("version") != AUTHORING_VERSION
        or document.get("effectAssetId") != effect_asset_id
        or not isinstance(document.get("elements"), list)
    ):
        raise ContractError(f"canonical Effect identity is invalid: {path}")


def _reconcile_target(
    target: TargetSpec,
    canonical_document: dict[str, Any] | None,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], list[str]]:
    by_id: dict[str, dict[str, Any]] = {}
    by_source: dict[str, dict[str, Any]] = {}
    if canonical_document is not None:
        for element in canonical_document["elements"]:
            if not isinstance(element, dict):
                raise ContractError(
                    f"canonical element is not an object: {target.effect_asset_id}"
                )
            element_id = element.get("id")
            source_node = element.get("sourceNode")
            if not isinstance(element_id, str) or not isinstance(source_node, str):
                raise ContractError(
                    f"canonical element identity is invalid: {target.effect_asset_id}"
                )
            if element_id in by_id:
                raise ContractError(
                    f"duplicate canonical element ID {element_id}: {target.effect_asset_id}"
                )
            by_id[element_id] = element
            if source_node:
                if source_node in by_source:
                    raise ContractError(
                        f"duplicate canonical sourceNode {source_node}: {target.effect_asset_id}"
                    )
                by_source[source_node] = element

    missing: list[dict[str, Any]] = []
    desired_rows: list[dict[str, Any]] = []
    preserved_ids: list[str] = []
    for desired in target.elements:
        element_id = desired["id"]
        source_node = desired["sourceNode"]
        existing_by_id = by_id.get(element_id)
        existing_by_source = by_source.get(source_node)
        if (
            existing_by_id is not None
            and existing_by_id.get("sourceNode") != source_node
        ):
            raise ContractError(
                f"stable element ID collision for {target.effect_asset_id}/{element_id}"
            )
        if (
            existing_by_source is not None
            and existing_by_source.get("id") != element_id
        ):
            raise ContractError(
                f"stable sourceNode collision for {target.effect_asset_id}/{source_node}"
            )
        existing = existing_by_id or existing_by_source
        if existing is None:
            missing.append(deepcopy(desired))
            desired_rows.append(
                {
                    "elementId": element_id,
                    "sourceNode": source_node,
                    "disposition": target.disposition,
                    "reconcileAction": "APPEND_MISSING",
                    "canonicalElementSha256": None,
                }
            )
        else:
            preserved_ids.append(element_id)
            desired_rows.append(
                {
                    "elementId": element_id,
                    "sourceNode": source_node,
                    "disposition": target.disposition,
                    "reconcileAction": "PRESERVE_EXISTING",
                    "canonicalElementSha256": _sha256_json(existing),
                }
            )
    return missing, desired_rows, preserved_ids


def _high_jump_airborne_patch() -> dict[str, Any]:
    effect_asset_id = "effect.valtan.high-jump.airborne"
    return {
        "catalogRow": {
            "effectAssetId": effect_asset_id,
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": (
                "Effects/Authored/effect.valtan.high-jump.airborne.effect.json"
            ),
        },
        "cueRow": {
            "bindingId": "cue.valtan.high-jump.airborne.project-authored",
            "occurrenceId": (
                "cue.valtan.high-jump.airborne.project-authored.occurrence.01"
            ),
            "patternId": "VALTAN_HIGH_JUMP",
            "stageId": "AIRBORNE",
            "actionId": "valtan.attack.high-jump.airborne",
            "clipOccurrenceId": "valtan.attack.high-jump.airborne.clip.01",
            "effectAssetId": effect_asset_id,
            "anchorSlotId": "root",
            "followPolicy": "snapshot",
            "stopPolicy": "natural",
            "repeatPolicy": "once",
            "sourceStartMs": 0,
            "sourceEndMs": None,
            "localTransform": {
                "position": [0, 0, 0],
                "rotationDegrees": [0, 0, 0],
                "scale": [1, 1, 1],
            },
        },
        "authoringDocument": {
            "effectAssetId": effect_asset_id,
            "targetAuthoringPath": (
                "Data/Effects/Authored/effect.valtan.high-jump.airborne.effect.json"
            ),
        },
        "authority": {
            "presentationOnly": True,
            "serverGameplayChange": False,
            "serverDamagePolicy": "UNCHANGED_EXISTING_LAND_STAGE_AUTHORITY",
            "projectileAuthorityStatus": "UNRESOLVED_PROJECTILE",
        },
    }


def _unresolved_projectiles() -> list[dict[str, Any]]:
    return [
        {
            "unresolvedId": (
                f"project-authored:valtan.high-jump.airborne.beat-{beat:02d}.axe"
            ),
            "patternId": "VALTAN_HIGH_JUMP",
            "stageId": "AIRBORNE",
            "disposition": "UNRESOLVED_PROJECTILE",
            "candidateAssetId": None,
            "searchedResourceDomain": "Effect/Valtan/Meshes/**/*.wmodel",
            "reason": (
                "The 52 available Valtan Effect WModels contain no filename-stable axe, weapon, or throw payload; anonymous rpbf meshes are not inferred as an axe."
            ),
            "authority": (
                "Presentation payload and Server projectile/damage authority remain unresolved; no fake mesh or Client damage actor is emitted."
            ),
        }
        for beat in (1, 2, 3)
    ]


def validate_candidate_document(document: dict[str, Any]) -> None:
    if tuple(document.keys()) != (
        "schema", "version", "effectAssetId", "displayName",
        "particleSystem", "modelCues", "elements",
    ):
        raise ContractError("ordinary v13 candidate root fields/order are invalid")
    if (
        document["schema"] != AUTHORING_SCHEMA
        or document["version"] != AUTHORING_VERSION
        or not isinstance(document["elements"], list)
        or not document["elements"]
    ):
        raise ContractError("ordinary v13 candidate identity is invalid")
    seen_ids: set[str] = set()
    seen_nodes: set[str] = set()
    for element in document["elements"]:
        element_id = element.get("id")
        source_node = element.get("sourceNode")
        if (
            not isinstance(element_id, str)
            or STABLE_ID_PATTERN.fullmatch(element_id) is None
            or element_id in seen_ids
        ):
            raise ContractError(f"candidate stable element ID is invalid: {element_id}")
        if (
            not isinstance(source_node, str)
            or not source_node.startswith("project-authored:")
            or STABLE_ID_PATTERN.fullmatch(source_node) is None
            or source_node in seen_nodes
        ):
            raise ContractError(f"candidate sourceNode is invalid: {source_node}")
        seen_ids.add(element_id)
        seen_nodes.add(source_node)
        if element.get("kind") == "light":
            raise ContractError("new standalone Light is forbidden")
        lowered = (element_id + " " + source_node).lower()
        if any(token in lowered for token in ("dust", "debris", "stone", "rock", "smoke")):
            raise ContractError("generic dust/debris family is forbidden")
        if element.get("sourceRecipe") != {
            "enabled": False,
            "rendererShape": "",
            "emitterDelaySeconds": 0,
            "emitterDurationSeconds": 0,
            "emitterLoopCount": 1,
            "bursts": [],
            "modules": [],
        }:
            raise ContractError("candidate sourceRecipe must stay disabled")
        if element.get("sourcePresentation") != {"enabled": False}:
            raise ContractError("candidate sourcePresentation must stay disabled")
        if element.get("material", {}).get("renderProfile") not in {
            "alpha_two_sided_depth_read",
            "additive_two_sided_depth_read",
        }:
            raise ContractError("candidate uses a non-established material profile")
        timing = element.get("detail", {}).get("timing", {})
        start = timing.get("startDelaySeconds")
        lifetime = timing.get("lifeTimeSeconds")
        if (
            not isinstance(start, (int, float))
            or not isinstance(lifetime, (int, float))
            or not math.isfinite(float(start))
            or not math.isfinite(float(lifetime))
            or start < 0
            or lifetime <= 0
            or start + lifetime > 60
        ):
            raise ContractError("candidate timing is invalid or unbounded")


def validate_receipt(receipt: dict[str, Any]) -> None:
    expected_root_keys = (
        "schema", "formatVersion", "ownerArchetypeId", "generator",
        "policy", "resourceVerification", "resourceRoleDispositions", "targets",
        "highJumpAirbornePatch", "unresolved",
    )
    if tuple(receipt.keys()) != expected_root_keys:
        raise ContractError("project-authored patch-plan root fields/order are invalid")
    if (
        receipt["schema"] != RECEIPT_SCHEMA
        or receipt["formatVersion"] != RECEIPT_VERSION
        or receipt["ownerArchetypeId"] != OWNER_ARCHETYPE_ID
    ):
        raise ContractError("project-authored patch-plan identity is invalid")
    policy = receipt["policy"]
    if policy != {
        "reconcileMode": "MISSING_ONLY",
        "sourceClaimPolicy": "NO_SOURCE_EXACT_CLAIMS",
        "portalRushExcluded": True,
        "newStandaloneLightAllowed": False,
        "genericDustDebrisAllowed": False,
        "sourceRecipeEnabled": False,
        "sourcePresentationEnabled": False,
        "canonicalMutationPerformed": False,
    }:
        raise ContractError("project-authored patch-plan policy changed")
    resources = receipt["resourceVerification"]
    if not isinstance(resources, list) or not resources:
        raise ContractError("resource verification is empty")
    resource_ids: list[str] = []
    for row in resources:
        if (
            not isinstance(row, dict)
            or tuple(row.keys()) != ("assetId", "byteSize", "sha256")
            or not isinstance(row["assetId"], str)
            or type(row["byteSize"]) is not int
            or row["byteSize"] <= 0
            or not isinstance(row["sha256"], str)
            or SHA256_PATTERN.fullmatch(row["sha256"]) is None
        ):
            raise ContractError("resource verification row is invalid")
        resource_ids.append(row["assetId"])
    if resource_ids != sorted(set(resource_ids)):
        raise ContractError("resource verification identities are not unique/sorted")
    targets = receipt["targets"]
    if not isinstance(targets, list) or len(targets) != 9:
        raise ContractError("priority target count must remain nine")
    effect_ids: set[str] = set()
    for target in targets:
        if target.get("patternId") == "VALTAN_PORTAL_RUSH":
            raise ContractError("Portal Rush must stay in the source-exact pipeline")
        effect_id = target.get("targetEffectAssetId")
        if not isinstance(effect_id, str) or effect_id in effect_ids:
            raise ContractError("duplicate or invalid priority target Effect ID")
        effect_ids.add(effect_id)
        desired = target.get("desiredElements")
        if not isinstance(desired, list) or not desired:
            raise ContractError("priority target has no desired element")
        for row in desired:
            action = row.get("reconcileAction")
            digest = row.get("canonicalElementSha256")
            if action == "APPEND_MISSING" and digest is not None:
                raise ContractError("missing element cannot have a canonical SHA")
            if action == "PRESERVE_EXISTING" and (
                not isinstance(digest, str)
                or SHA256_PATTERN.fullmatch(digest) is None
            ):
                raise ContractError("preserved element requires a canonical SHA")
            if action not in ("APPEND_MISSING", "PRESERVE_EXISTING"):
                raise ContractError("unknown reconcile action")
    roles = receipt["resourceRoleDispositions"]
    expected_roles = {
        "DASH_FORWARD_RED_PATH": (ASSET_LINE_003, "EXACT_OTHER_PATTERN"),
        "MAGIC_DONUT_RING_BOUNDARIES": (
            ASSET_RING_002,
            "EXACT_SAME_PATTERN_LIMITED_STAGE",
        ),
        "MAGIC_DONUT_ANNULUS_UV_SWEEP": (
            ASSET_RING_004,
            "EXACT_OTHER_PATTERN",
        ),
        "FLOOR_WIPE_IMPACT_DIFFUSE": (
            ASSET_ATYPICAL_009,
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
        ),
        "FLOOR_WIPE_GUIDE_AND_IMPACT_MASK": (
            ASSET_ATYPICAL_032,
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
        ),
        "FLOOR_WIPE_IMPACT_NOISE": (
            ASSET_ATYPICAL_028,
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
        ),
        "FRONT_BACK_FRONT_FINAL_GROUND_IMPACT": (
            ASSET_SHOCKWAVE_02,
            "NO_TARGET_SOURCE_JOIN",
        ),
    }
    if not isinstance(roles, list) or len(roles) != len(expected_roles):
        raise ContractError("priority resource role disposition count changed")
    seen_roles: set[str] = set()
    verified_resource_ids = set(resource_ids)
    for row in roles:
        role = row.get("requestedRole")
        if role in seen_roles or role not in expected_roles:
            raise ContractError("duplicate or unknown resource role disposition")
        seen_roles.add(role)
        expected_asset, expected_evidence = expected_roles[role]
        candidate_ids = row.get("candidateEffectAssetIds")
        if (
            tuple(row.keys())
            != (
                "assetId", "requestedRole", "evidenceStatus",
                "sourceEvidenceContext", "candidateDisposition",
                "candidateEffectAssetIds", "handTunableFields",
            )
            or row.get("assetId") != expected_asset
            or row.get("assetId") not in verified_resource_ids
            or row.get("evidenceStatus") != expected_evidence
            or not isinstance(row.get("sourceEvidenceContext"), str)
            or not row["sourceEvidenceContext"]
            or row.get("candidateDisposition") != "PROJECT_TUNED_CANDIDATE"
            or not isinstance(candidate_ids, list)
            or candidate_ids != sorted(set(candidate_ids))
            or not candidate_ids
            or any(effect_id not in effect_ids for effect_id in candidate_ids)
            or tuple(row.get("handTunableFields", ())) != HAND_TUNABLE_FIELDS
        ):
            raise ContractError("resource role disposition is invalid")
    if seen_roles != set(expected_roles):
        raise ContractError("priority resource role disposition is incomplete")
    patch = receipt["highJumpAirbornePatch"]
    cue = patch.get("cueRow", {})
    catalog = patch.get("catalogRow", {})
    if (
        catalog.get("effectAssetId") != "effect.valtan.high-jump.airborne"
        or catalog.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13"
        or cue.get("patternId") != "VALTAN_HIGH_JUMP"
        or cue.get("stageId") != "AIRBORNE"
        or cue.get("actionId") != "valtan.attack.high-jump.airborne"
        or cue.get("clipOccurrenceId")
        != "valtan.attack.high-jump.airborne.clip.01"
        or cue.get("effectAssetId") != catalog.get("effectAssetId")
        or cue.get("followPolicy") != "snapshot"
        or cue.get("stopPolicy") != "natural"
        or cue.get("repeatPolicy") != "once"
        or cue.get("sourceStartMs") != 0
        or cue.get("sourceEndMs") is not None
    ):
        raise ContractError("High Jump AIRBORNE canonical v2 patch is incomplete")
    unresolved = receipt["unresolved"]
    if (
        not isinstance(unresolved, list)
        or len(unresolved) != 3
        or any(
            row.get("disposition") != "UNRESOLVED_PROJECTILE"
            or row.get("candidateAssetId") is not None
            for row in unresolved
        )
    ):
        raise ContractError("High Jump unresolved projectile denominator changed")


def build_artifacts(
    repo_root: Path,
    *,
    resource_root: Path | None = None,
    canonical_overrides: Mapping[str, dict[str, Any] | None] | None = None,
) -> BuildArtifacts:
    repo_root = repo_root.resolve()
    _validate_reviewed_source_context(repo_root)
    targets = _target_specs()
    resources = _verify_resources(
        _find_resource_root(repo_root, resource_root), targets
    )
    overrides = canonical_overrides or {}
    files: dict[PurePosixPath, bytes] = {}
    documents: dict[str, dict[str, Any]] = {}
    target_rows: list[dict[str, Any]] = []
    for target in targets:
        canonical_relative = _canonical_authoring_path(target.effect_asset_id)
        if target.effect_asset_id in overrides:
            canonical = overrides[target.effect_asset_id]
        else:
            canonical_path = repo_root.joinpath(*canonical_relative.parts)
            canonical = _load_json(canonical_path) if canonical_path.is_file() else None
        if canonical is not None:
            _validate_canonical_document(
                canonical, target.effect_asset_id, canonical_relative
            )
        missing, desired_rows, preserved_ids = _reconcile_target(
            target, canonical
        )
        overlay_relative: PurePosixPath | None = None
        overlay_sha: str | None = None
        if missing:
            document = _candidate_document(target.effect_asset_id, missing)
            validate_candidate_document(document)
            overlay_relative = _overlay_relative_path(target.effect_asset_id)
            payload = _json_bytes(document)
            overlay_sha = _sha256_bytes(payload)
            files[overlay_relative] = payload
            documents[target.effect_asset_id] = document
        target_rows.append(
            {
                "patternId": target.pattern_id,
                "stageId": target.stage_id,
                "actionId": target.action_id,
                "clipOccurrenceId": target.clip_occurrence_id,
                "targetEffectAssetId": target.effect_asset_id,
                "targetAuthoringPath": canonical_relative.as_posix(),
                "canonicalState": (
                    "EXISTING_DOCUMENT" if canonical is not None else "MISSING_DOCUMENT"
                ),
                "disposition": target.disposition,
                "presentationOnly": target.presentation_only,
                "overlayDocumentPath": (
                    overlay_relative.as_posix() if overlay_relative else None
                ),
                "overlayDocumentSha256": overlay_sha,
                "desiredElements": desired_rows,
                "preservedCanonicalElementIds": preserved_ids,
                "excludedSourceOccurrenceIds": list(
                    target.excluded_source_occurrence_ids
                ),
                "notes": list(target.notes),
            }
        )

    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": RECEIPT_VERSION,
        "ownerArchetypeId": OWNER_ARCHETYPE_ID,
        "generator": {
            "script": (
                "Tools/EffectPipeline/build_valtan_project_authored_priority_candidates.py"
            ),
            "policyVersion": 1,
        },
        "policy": {
            "reconcileMode": "MISSING_ONLY",
            "sourceClaimPolicy": "NO_SOURCE_EXACT_CLAIMS",
            "portalRushExcluded": True,
            "newStandaloneLightAllowed": False,
            "genericDustDebrisAllowed": False,
            "sourceRecipeEnabled": False,
            "sourcePresentationEnabled": False,
            "canonicalMutationPerformed": False,
        },
        "resourceVerification": resources,
        "resourceRoleDispositions": _resource_role_dispositions(),
        "targets": target_rows,
        "highJumpAirbornePatch": _high_jump_airborne_patch(),
        "unresolved": _unresolved_projectiles(),
    }
    validate_receipt(receipt)
    files[RECEIPT_RELATIVE_PATH] = _json_bytes(receipt)
    return BuildArtifacts(files=files, receipt=receipt, documents=documents)


def write_artifacts(artifacts: BuildArtifacts, destination_root: Path) -> None:
    destination_root = destination_root.resolve()
    expected = {path.as_posix() for path in artifacts.files}
    output_root = destination_root.joinpath(*OUTPUT_RELATIVE_ROOT.parts)
    output_root.mkdir(parents=True, exist_ok=True)
    for existing in output_root.glob("*.json"):
        relative = existing.relative_to(destination_root).as_posix()
        if relative not in expected:
            existing.unlink()
    for relative, payload in sorted(
        artifacts.files.items(), key=lambda item: item[0].as_posix()
    ):
        path = destination_root.joinpath(*relative.parts)
        path.parent.mkdir(parents=True, exist_ok=True)
        if path.is_file() and path.read_bytes() == payload:
            continue
        path.write_bytes(payload)


def check_artifacts(artifacts: BuildArtifacts, destination_root: Path) -> None:
    destination_root = destination_root.resolve()
    expected = {path.as_posix() for path in artifacts.files}
    output_root = destination_root.joinpath(*OUTPUT_RELATIVE_ROOT.parts)
    actual = (
        {
            path.relative_to(destination_root).as_posix()
            for path in output_root.glob("*.json")
        }
        if output_root.is_dir()
        else set()
    )
    extra = sorted(actual - expected)
    missing = sorted(expected - actual)
    if extra or missing:
        raise ContractError(
            f"generated file set differs; missing={missing}, extra={extra}"
        )
    for relative, expected_payload in artifacts.files.items():
        path = destination_root.joinpath(*relative.parts)
        try:
            actual_payload = path.read_bytes()
        except OSError as exc:
            raise ContractError(f"cannot read generated artifact {relative}: {exc}") from exc
        if actual_payload != expected_payload:
            raise ContractError(f"generated artifact is stale: {relative}")


def _make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument("--resource-root", type=Path)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _make_parser().parse_args(argv)
    try:
        artifacts = build_artifacts(
            args.repo_root, resource_root=args.resource_root
        )
        if args.write:
            write_artifacts(artifacts, args.repo_root)
        else:
            check_artifacts(artifacts, args.repo_root)
    except ContractError as exc:
        print(f"[FAILURE] {exc}", file=sys.stderr)
        return 1
    desired_count = sum(
        len(target["desiredElements"]) for target in artifacts.receipt["targets"]
    )
    append_count = sum(
        row["reconcileAction"] == "APPEND_MISSING"
        for target in artifacts.receipt["targets"]
        for row in target["desiredElements"]
    )
    preserved_count = desired_count - append_count
    print(
        "[PASS] Valtan project-authored priority candidates: "
        f"documents={len(artifacts.documents)}, desired={desired_count}, "
        f"append={append_count}, preserved={preserved_count}, "
        f"unresolved={len(artifacts.receipt['unresolved'])}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
