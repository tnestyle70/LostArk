#!/usr/bin/env python3
"""Build the first-hit DimensionMaster A manual Mesh restoration candidate.

The candidate is derived from the canonical 2050210 source elements, but it is
saved under a separate Effect ID.  It never overwrites an existing candidate;
the Effect Tool owns all tuning after the initial seed is created.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = (
    REPOSITORY_ROOT
    / "Data/Effects/Authored/effect.dimensionmaster.skill.2050210.effect.json"
)
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Authored/"
    "effect.dimensionmaster.skill.2050210.a-restoration-candidate.effect.json"
)

SOURCE_EFFECT_ID = "effect.dimensionmaster.skill.2050210"
CANDIDATE_EFFECT_ID = (
    "effect.dimensionmaster.skill.2050210.a-restoration-candidate"
)
SOURCE_GROUP = "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1"

BODY_SOURCE_ID = f"{SOURCE_GROUP}.particlespriteemitter_15"
RIM_SOURCE_ID = f"{SOURCE_GROUP}.particlespriteemitter_3"
AFTERIMAGE_SOURCE_ID = f"{SOURCE_GROUP}.particlespriteemitter_20"
EXPECTED_HIT01_CANDIDATE_SHA256 = (
    "4F3487A9249D50F7B14686DD721FB7BFF471773F535C2FE3585D48E7A979C8FE"
)
EXPECTED_FOUR_HIT_Y_AXIS_SHA256 = (
    "22925981482F1DF413F39A722BFA54A1F2E4514714D240D5F14A0BF2455CBBA6"
)

HIT_SPECS: tuple[dict[str, Any], ...] = (
    {
        "id": "hit01",
        "sourceSuffix": "",
        "startDelay": 0.25,
        "position": [0.5, 0.15, -0.9],
    },
    {
        "id": "hit02",
        "sourceSuffix": ".event_source-event-030",
        "startDelay": 0.60,
        "position": [0.5, 0.15, 0.8],
    },
    {
        "id": "hit03",
        "sourceSuffix": ".event_source-event-045",
        "startDelay": 0.90,
        "position": [0.5, 0.3, -0.9],
    },
    {
        "id": "hit04",
        "sourceSuffix": ".event_source-event-060",
        "startDelay": 1.30,
        "position": [0.5, 0.6, -0.8],
    },
)


def _resources(**slots: str) -> list[dict[str, str]]:
    order = ("meshModel", "base", "noise", "mask", "emissive", "dissolve")
    return [
        {"slotId": slot_id, "assetId": slots[slot_id]}
        for slot_id in order
        if slots.get(slot_id)
    ]


LAYER_SPECS: tuple[dict[str, Any], ...] = (
    {
        "id": "manual.a.hit01.body",
        "sourceId": BODY_SOURCE_ID,
        "visible": True,
        "resources": _resources(
            meshModel="Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel",
            base=(
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "fx_j_mirnoise_02.dds"
            ),
            noise=(
                "Effect/DimensionMaster/Textures/FX_TEX_02/"
                "fx_d_noise_014.dds"
            ),
            mask=(
                "Effect/DimensionMaster/Textures/FX_TEX_06/"
                "fx_j_auraline_19_ycl.dds"
            ),
            dissolve=(
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "fx_h_atypical_01_1.dds"
            ),
        ),
        "renderProfile": "alpha_two_sided_depth_read",
        "rotation": [-18.0, 0.0, 0.0],
        "scale": [0.0341, 0.0341, 0.0341],
        "revolutionStart": [-280.0, 0.0, 0.0],
        "revolutionEnd": [-70.0, 0.0, 0.0],
        "lifeTime": 0.5,
        "colorMultiply": [1.8, 1.55, 2.6, 0.9],
        "emissiveIntensity": 0.0,
        "distortionIntensity": 0.02,
        "dissolveStart": 0.78,
        "uvSpeed": [0.0, 0.1],
    },
    {
        "id": "manual.a.hit01.rim",
        "sourceId": RIM_SOURCE_ID,
        "visible": True,
        "resources": _resources(
            meshModel="Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel",
            base=(
                "Effect/DimensionMaster/Textures/FX_TEX_05/"
                "fx_l_environment_001.dds"
            ),
            mask=(
                "Effect/DimensionMaster/Textures/FX_TEX_06/"
                "fx_j_line_01_xcl.dds"
            ),
            dissolve=(
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "fx_h_atypical_01_1.dds"
            ),
        ),
        "renderProfile": "alpha_two_sided_depth_read",
        "rotation": [-18.0, 0.0, 0.0],
        "scale": [0.035, 0.035, 0.035],
        "revolutionStart": [-280.0, 0.0, 0.0],
        "revolutionEnd": [-70.0, 0.0, 0.0],
        "lifeTime": 0.3,
        "colorMultiply": [0.55, 0.28, 1.6, 0.72],
        "emissiveIntensity": 0.0,
        "distortionIntensity": 0.0,
        "dissolveStart": 0.66,
        "uvSpeed": [0.0, 0.0],
    },
    {
        "id": "manual.a.hit01.highlight",
        "sourceId": BODY_SOURCE_ID,
        "visible": True,
        "resources": _resources(
            meshModel="Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel",
            base=(
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "fx_j_mirnoise_02.dds"
            ),
            noise=(
                "Effect/DimensionMaster/Textures/FX_TEX_02/"
                "fx_d_noise_014.dds"
            ),
            mask=(
                "Effect/DimensionMaster/Textures/FX_TEX_06/"
                "fx_j_auraline_19_ycl.dds"
            ),
            emissive=(
                "Effect/DimensionMaster/Textures/FX_TEX_06/"
                "fx_j_auraline_19_ycl.dds"
            ),
            dissolve=(
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "fx_h_atypical_01_1.dds"
            ),
        ),
        "renderProfile": "additive_two_sided_depth_read",
        "rotation": [-18.0, 0.0, 0.0],
        "scale": [0.032, 0.032, 0.032],
        "revolutionStart": [-280.0, 0.0, 0.0],
        "revolutionEnd": [-70.0, 0.0, 0.0],
        "lifeTime": 0.2,
        "colorMultiply": [2.3, 2.0, 3.2, 0.72],
        "emissiveIntensity": 2.0,
        "distortionIntensity": 0.0,
        "dissolveStart": 0.75,
        "uvSpeed": [0.0, 0.1],
    },
    {
        "id": "manual.a.hit01.afterimage",
        "sourceId": AFTERIMAGE_SOURCE_ID,
        "visible": False,
        "resources": _resources(
            meshModel="Effect/DimensionMaster/Meshes/fm_m_trail_01.wmodel",
            base=(
                "Effect/DimensionMaster/Textures/FX_TEX_06/"
                "fx_j_cloud_tile_01.dds"
            ),
            noise=(
                "Effect/DimensionMaster/Textures/FX_TEX_02/"
                "fx_d_noise_009.dds"
            ),
            mask=(
                "Effect/DimensionMaster/Textures/FX_TEX_06/"
                "fx_j_line_08_cl.dds"
            ),
            dissolve=(
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "fx_h_atypical_01_1.dds"
            ),
        ),
        "renderProfile": "alpha_two_sided_depth_read",
        "rotation": [-18.0, 0.0, 0.0],
        "scale": [0.077, 0.066, 0.077],
        "revolutionStart": [0.0, 0.0, 0.0],
        "revolutionEnd": [0.0, 0.0, 0.0],
        "lifeTime": 0.7,
        "colorMultiply": [0.7, 0.55, 1.25, 0.18],
        "emissiveIntensity": 0.0,
        "distortionIntensity": 0.02,
        "dissolveStart": 0.75,
        "uvSpeed": [0.0, 0.1],
    },
)


def _reset_action_cue() -> dict[str, Any]:
    return {
        "enabled": False,
        "follow": False,
        "sourceAnchorSlotId": "",
        "runtimeAnchorSlotId": "",
        "runtimeBoneName": "",
        "socketLocalTransform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }


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


def _make_layer(
    source: dict[str, Any],
    spec: dict[str, Any],
    hit: dict[str, Any],
) -> dict[str, Any]:
    layer = copy.deepcopy(source)
    layer_id = spec["id"].replace("hit01", hit["id"])
    layer["id"] = layer_id
    layer["displayName"] = layer_id
    layer["groupId"] = f"manual.a.{hit['id']}"
    layer["sourceNode"] = "dimensionmaster.a.manual-seed"
    layer["visible"] = spec["visible"]
    layer["kind"] = "mesh"
    layer["resources"] = copy.deepcopy(spec["resources"])
    layer["material"] = {
        "templateId": "effect.standard",
        "sourceMaterialPath": "",
        "renderProfile": spec["renderProfile"],
        "sourceProfile": {"enabled": False},
    }
    layer["actionCueAttachment"] = _reset_action_cue()
    layer["sourceRecipe"] = _reset_source_recipe()
    layer["sourcePresentation"] = {"enabled": False}

    detail = layer["detail"]
    transform = detail["transform"]
    transform["position"] = hit["position"]
    transform["rotationDegrees"] = spec["rotation"]
    transform["scale"] = spec["scale"]
    transform["revolutionDegreesPerSecond"] = spec["revolutionStart"]
    transform["velocityPerSecond"] = [0.0, 0.0, 0.0]

    timing = detail["timing"]
    timing["startDelaySeconds"] = hit["startDelay"]
    timing["lifeTimeSeconds"] = spec["lifeTime"]
    timing["afterImageSeconds"] = 0.0
    timing["dissolveStartNormalized"] = spec["dissolveStart"]

    color = detail["color"]
    color["offset"] = [0.0, 0.0, 0.0, 0.0]
    color["multiply"] = spec["colorMultiply"]
    color["clip"] = 0.0
    color["emissiveIntensity"] = spec["emissiveIntensity"]
    color["distortionIntensity"] = spec["distortionIntensity"]
    color["distortionOnBaseMaterial"] = False
    color["radialTime"] = 0.0
    color["radialIntensity"] = 0.0

    detail["uv"]["start"] = [0.0, 0.0]
    detail["uv"]["speed"] = spec["uvSpeed"]
    detail["mesh"]["useModelMaterial"] = False
    detail["sprite"]["billboard"] = False

    linear = detail["linearLerp"]
    linear["position"] = False
    linear["rotation"] = False
    linear["revolution"] = spec["revolutionStart"] != spec["revolutionEnd"]
    linear["endRevolutionDegreesPerSecond"] = spec["revolutionEnd"]
    linear["scale"] = False
    linear["velocity"] = False
    linear["colorOffset"] = False
    linear["colorMultiply"] = False
    linear["emissiveIntensity"] = False

    if "particle" in detail:
        detail["particle"]["lifeTimeSeconds"] = [
            spec["lifeTime"],
            spec["lifeTime"],
        ]
    return layer


def build_candidate(source_document: dict[str, Any]) -> dict[str, Any]:
    if source_document.get("schema") != "lostark.effect-authoring":
        raise ValueError("Source is not a LostArk Effect Authoring document.")
    if source_document.get("version") != 12:
        raise ValueError("DimensionMaster A candidate requires Effect v12.")
    if source_document.get("effectAssetId") != SOURCE_EFFECT_ID:
        raise ValueError("Source Effect ID is not canonical DimensionMaster A 2050210.")

    elements = source_document.get("elements")
    if not isinstance(elements, list):
        raise ValueError("Source Effect elements are missing.")
    by_id = {element.get("id"): element for element in elements}

    layers: list[dict[str, Any]] = []
    for hit in HIT_SPECS:
        for spec in LAYER_SPECS:
            source_id = spec["sourceId"] + hit["sourceSuffix"]
            source = by_id.get(source_id)
            if not isinstance(source, dict):
                raise ValueError(f"Required A source element is missing: {source_id}")
            layers.append(_make_layer(source, spec, hit))

    candidate = {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": CANDIDATE_EFFECT_ID,
        "displayName": "DimensionMaster A 2050210 Four Hit Restoration Candidate",
        "particleSystem": copy.deepcopy(source_document["particleSystem"]),
        "modelCues": [],
        "elements": layers,
    }
    candidate["particleSystem"] = {
        "uniformScaleMultiplier": 1.0,
        "yawOffsetDegrees": 0.0,
        "directionYawDegrees": 0.0,
        "initialSpeedMultiplier": 1.0,
    }
    return candidate


def write_candidate(output_path: Path, candidate: dict[str, Any]) -> None:
    output_path = output_path.resolve()
    authored_root = (REPOSITORY_ROOT / "Data/Effects/Authored").resolve()
    if output_path.parent != authored_root:
        raise ValueError("Candidate output must stay directly under Data/Effects/Authored.")
    if output_path.exists():
        raise FileExistsError(
            "Candidate already exists; load and tune it in Effect Tool instead of reseeding."
        )

    _write_candidate_atomic(output_path, candidate)


def _write_candidate_atomic(
    output_path: Path, candidate: dict[str, Any]
) -> None:

    output_path.parent.mkdir(parents=True, exist_ok=True)
    serialized = json.dumps(candidate, indent=2, ensure_ascii=False) + "\n"
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


def upgrade_hit01_candidate_to_four_hits(
    output_path: Path, candidate: dict[str, Any]
) -> None:
    output_path = output_path.resolve()
    authored_root = (REPOSITORY_ROOT / "Data/Effects/Authored").resolve()
    if output_path.parent != authored_root or not output_path.is_file():
        raise ValueError(
            "Four-hit upgrade requires the existing candidate directly under Authored."
        )
    current_hash = hashlib.sha256(output_path.read_bytes()).hexdigest().upper()
    if current_hash != EXPECTED_HIT01_CANDIDATE_SHA256:
        raise ValueError(
            "Four-hit upgrade refused a candidate changed by Effect Tool tuning: "
            f"{current_hash}"
        )
    _write_candidate_atomic(output_path, candidate)


def upgrade_four_hits_to_forward_tilt(
    output_path: Path, candidate: dict[str, Any]
) -> None:
    output_path = output_path.resolve()
    authored_root = (REPOSITORY_ROOT / "Data/Effects/Authored").resolve()
    if output_path.parent != authored_root or not output_path.is_file():
        raise ValueError(
            "Forward-tilt upgrade requires the existing candidate directly under Authored."
        )
    current_hash = hashlib.sha256(output_path.read_bytes()).hexdigest().upper()
    if current_hash != EXPECTED_FOUR_HIT_Y_AXIS_SHA256:
        raise ValueError(
            "Forward-tilt upgrade refused a candidate changed by Effect Tool tuning: "
            f"{current_hash}"
        )
    _write_candidate_atomic(output_path, candidate)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--write",
        action="store_true",
        help="Atomically create the candidate; existing output is never overwritten.",
    )
    parser.add_argument(
        "--upgrade-hit01-to-four-hits",
        action="store_true",
        help=(
            "Atomically replace only the untouched, known Hit01 candidate with "
            "the four-hit candidate."
        ),
    )
    parser.add_argument(
        "--upgrade-four-hits-to-forward-tilt",
        action="store_true",
        help=(
            "Atomically replace only the known four-hit Y-axis candidate with "
            "the source-evidenced X-axis forward-tilt candidate."
        ),
    )
    args = parser.parse_args()

    with args.source.open("r", encoding="utf-8") as stream:
        source = json.load(stream)
    candidate = build_candidate(source)
    if args.upgrade_four_hits_to_forward_tilt:
        upgrade_four_hits_to_forward_tilt(args.output, candidate)
        print(f"Upgraded {args.output} to player-forward X-axis tilt")
    elif args.upgrade_hit01_to_four_hits:
        upgrade_hit01_candidate_to_four_hits(args.output, candidate)
        print(f"Upgraded {args.output} to four Hit groups")
    elif args.write:
        write_candidate(args.output, candidate)
        print(f"Created {args.output}")
    else:
        print(json.dumps(candidate, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
