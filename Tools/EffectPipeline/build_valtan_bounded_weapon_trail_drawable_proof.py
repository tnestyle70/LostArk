#!/usr/bin/env python3
"""Seal the actual renderer sweep for the bounded Valtan weapon Trail batch."""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib.util
import json
import math
from pathlib import Path, PurePosixPath
import re
import sys
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_REPOSITORY_ROOT = SCRIPT_PATH.parents[2]


def _load_sibling_module(name: str, path: Path):
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


candidate_builder = _load_sibling_module(
    "build_valtan_bounded_weapon_trail_candidates_for_proof",
    SCRIPT_PATH.with_name("build_valtan_bounded_weapon_trail_candidates.py"),
)

OUTPUT_ROOT = candidate_builder.OUTPUT_ROOT / PurePosixPath("DrawableProof")
SWEEP_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "Valtan.bounded-weapon-trails.runtime-sweep.v1.json"
)
PROOF_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "Valtan.bounded-weapon-trails.drawable-proof.v1.json"
)
SWEEP_SCHEMA = "lostark.valtan-bounded-weapon-trail-runtime-sweep"
PROOF_SCHEMA = "lostark.valtan-bounded-weapon-trail-drawable-proof"
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


class ProofError(RuntimeError):
    """Actual renderer evidence is stale, incomplete, or not fail-closed."""


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ProofError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ProofError(f"JSON root must be an object: {path}")
    return value


def _path(root: Path, relative: PurePosixPath) -> Path:
    try:
        return candidate_builder.repository_path(root, relative)
    except candidate_builder.CandidateError as error:
        raise ProofError(str(error)) from error


def _verify_seal(value: dict[str, Any], field: str, label: str) -> None:
    expected = value.get(field)
    clone = copy.deepcopy(value)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise ProofError(f"{label} {field} is stale")


def _renderer_is_positive(renderer: Any) -> bool:
    return (
        isinstance(renderer, dict)
        and int(renderer.get("preparedSamples", 0)) >= 1
        and int(renderer.get("attemptedSamples", 0)) >= 1
        and int(renderer.get("submittedDraws", 0)) >= 1
        and int(renderer.get("committedDraws", 0)) >= 1
        and int(renderer.get("failedDraws", -1)) == 0
        and renderer.get("transactionCommitted") is True
    )


def _renderer_is_stationary_suppressed(renderer: Any) -> bool:
    return (
        isinstance(renderer, dict)
        and int(renderer.get("submittedDraws", -1)) == 0
        and int(renderer.get("committedDraws", -1)) == 0
        and int(renderer.get("failedDraws", -1)) == 0
        and renderer.get("transactionCommitted") is True
    )


def _finite_vector3(value: Any) -> bool:
    return (
        isinstance(value, list)
        and len(value) == 3
        and all(isinstance(row, (int, float)) and math.isfinite(row) for row in value)
    )


def _validate_element_sweep(
    row: dict[str, Any], expected_id: str
) -> dict[str, Any]:
    positive = row.get("positiveMovingBone") or {}
    stationary = row.get("stationaryControl") or {}
    if row.get("elementId") != expected_id:
        raise ProofError(f"renderer sweep element order changed: {expected_id}")
    if (
        int(positive.get("providerSampleCount", 0)) < 2
        or int(positive.get("rootWorldDistinctCount", 0)) != 1
        or int(positive.get("anchorWorldDistinctCount", 0)) < 2
        or int(positive.get("trailPointCount", 0)) < 2
        or int(positive.get("distinctTrailPointCount", 0)) < 2
        or int(positive.get("finiteTrailPointCount", 0))
        != int(positive.get("trailPointCount", -1))
        or not isinstance(positive.get("cumulativeDistance"), (int, float))
        or not math.isfinite(positive["cumulativeDistance"])
        or positive["cumulativeDistance"] <= 0
        or not _finite_vector3(positive.get("firstWorldPosition"))
        or not _finite_vector3(positive.get("lastWorldPosition"))
        or positive.get("firstWorldPosition") == positive.get("lastWorldPosition")
        or not _renderer_is_positive(positive.get("renderer"))
    ):
        raise ProofError(f"moving-bone Trail proof is incomplete: {expected_id}")
    if (
        int(stationary.get("providerSampleCount", 0)) < 2
        or int(stationary.get("rootWorldDistinctCount", 0)) != 1
        or int(stationary.get("anchorWorldDistinctCount", 0)) != 1
        or int(stationary.get("trailPointCount", -1)) > 1
        or int(stationary.get("distinctTrailPointCount", -1)) > 1
        or stationary.get("segmentSuppressed") is not True
        or not _renderer_is_stationary_suppressed(stationary.get("renderer"))
    ):
        raise ProofError(f"stationary Trail control was not suppressed: {expected_id}")
    return {
        "elementId": expected_id,
        "positiveMovingBone": copy.deepcopy(positive),
        "stationaryControl": copy.deepcopy(stationary),
    }


def build_proof(
    root: Path, *, expected_resource_root: Path | None = None
) -> dict[str, Any]:
    root = root.resolve()
    try:
        expected_outputs = candidate_builder.build_outputs(root)
    except candidate_builder.CandidateError as error:
        raise ProofError(f"candidate inputs require rebase: {error}") from error
    manifest_relative = candidate_builder.OUTPUT_ROOT / PurePosixPath(
        candidate_builder.MANIFEST_FILENAME
    )
    manifest_path = _path(root, manifest_relative)
    manifest = _read_json(manifest_path)
    manifest_payload = manifest_path.read_bytes()
    if expected_outputs.get(manifest_relative) != manifest_payload:
        raise ProofError("candidate manifest is stale")
    _verify_seal(manifest, "artifactSha256", "candidate manifest")

    sweep_path = _path(root, SWEEP_RELATIVE_PATH)
    sweep = _read_json(sweep_path)
    if (
        sweep.get("schema") != SWEEP_SCHEMA
        or sweep.get("formatVersion") != 1
        or sweep.get("bossArchetypeId") != "BOSS_VALTAN"
        or sweep.get("classification") != "PROJECT_TUNED"
        or sweep.get("reconstructionPolicy") != "BOUNDED_RECONSTRUCTION"
        or sweep.get("runtimeBoneName") != candidate_builder.RUNTIME_BONE_NAME
        or sweep.get("sampleRateHz") != 60
        or sweep.get("disposition") != "DRAWABLE_PROOF_PASS"
    ):
        raise ProofError("runtime sweep root contract changed")
    resource_root = Path(str(sweep.get("resourceRoot") or "")).resolve()
    if expected_resource_root is not None and resource_root != expected_resource_root.resolve():
        raise ProofError("runtime sweep resource root changed")
    summary = sweep.get("summary") or {}
    if summary != {
        "targetCount": 3,
        "elementCount": 9,
        "movingBoneDrawCount": 9,
        "stationarySuppressedCount": 9,
        "missingAnchorRollbackCount": 3,
    }:
        raise ProofError("runtime sweep denominator changed")

    sweep_targets = sweep.get("targets") or []
    manifest_targets = manifest.get("targets") or []
    if len(sweep_targets) != 3 or len(manifest_targets) != 3:
        raise ProofError("runtime/manifest target denominator changed")
    proof_targets: list[dict[str, Any]] = []
    for target, manifest_target, sweep_target in zip(
        candidate_builder.TARGETS, manifest_targets, sweep_targets, strict=True
    ):
        candidate_relative = candidate_builder.OUTPUT_ROOT / PurePosixPath(
            target.candidate_filename
        )
        candidate_path = _path(root, candidate_relative)
        candidate = _read_json(candidate_path)
        candidate_payload = candidate_path.read_bytes()
        expected_ids = list(candidate_builder.candidate_element_ids(target))
        if (
            expected_outputs.get(candidate_relative) != candidate_payload
            or manifest_target.get("patternId") != target.pattern_id
            or manifest_target.get("effectAssetId") != target.effect_asset_id
            or manifest_target.get("candidatePath") != candidate_relative.as_posix()
            or manifest_target.get("candidateRawSha256") != raw_sha256(candidate_path)
            or manifest_target.get("candidateCanonicalSha256")
            != canonical_sha256(candidate)
            or manifest_target.get("candidateElementIds") != expected_ids
            or sweep_target.get("effectAssetId") != target.effect_asset_id
            or Path(str(sweep_target.get("candidatePath") or "")).resolve()
            != candidate_path.resolve()
            or sweep_target.get("candidateRawSha256") != raw_sha256(candidate_path)
            or not SHA256_RE.fullmatch(
                str(sweep_target.get("candidateTypedCodecSha256") or "")
            )
            or sweep_target.get("runtimeAnchorSlotId")
            != target.runtime_anchor_slot_id
            or abs(
                float(sweep_target.get("sourceStartSeconds", -1))
                - target.source_time_seconds
            )
            > 1.0e-6
            or abs(
                float(sweep_target.get("sourceDurationSeconds", -1))
                - target.source_duration_seconds
            )
            > 1.0e-6
            or sweep_target.get("disposition") != "DRAWABLE_PROOF_PASS"
        ):
            raise ProofError(f"candidate/runtime hash closure changed: {target.pattern_id}")
        sweep_elements = sweep_target.get("elements") or []
        candidate_elements = candidate.get("elements") or []
        if len(sweep_elements) != 3 or len(candidate_elements) != 3:
            raise ProofError(f"candidate element denominator changed: {target.pattern_id}")
        element_proofs = [
            _validate_element_sweep(row, expected_id)
            for row, expected_id in zip(sweep_elements, expected_ids, strict=True)
        ]
        missing = sweep_target.get("missingAnchorControl") or {}
        if (
            missing.get("providerRejected") is not True
            or missing.get("playbackStatePreserved") is not True
            or missing.get("trailPointCountAfterReject") != 0
            or target.runtime_anchor_slot_id
            not in str(missing.get("status") or "")
        ):
            raise ProofError(
                f"missing-anchor control did not rollback: {target.pattern_id}"
            )
        proof_targets.append(
            {
                "patternId": target.pattern_id,
                "stageId": target.stage_id,
                "effectAssetId": target.effect_asset_id,
                "candidatePath": candidate_relative.as_posix(),
                "candidateRawSha256": raw_sha256(candidate_path),
                "candidateCanonicalSha256": canonical_sha256(candidate),
                "candidateTypedCodecSha256": sweep_target[
                    "candidateTypedCodecSha256"
                ],
                "runtimeAnchorSlotId": target.runtime_anchor_slot_id,
                "runtimeBoneName": candidate_builder.RUNTIME_BONE_NAME,
                "sourceStartSeconds": target.source_time_seconds,
                "sourceDurationSeconds": target.source_duration_seconds,
                "elements": [
                    {
                        **proof_row,
                        "initialElementSha256": canonical_sha256(candidate_element),
                        "protectedElementContractSha256": canonical_sha256(
                            candidate_builder.protected_element_contract(
                                candidate_element
                            )
                        ),
                        "resourceClosureSha256": canonical_sha256(
                            candidate_element.get("resources")
                        ),
                        "materialClosureSha256": canonical_sha256(
                            candidate_element.get("material")
                        ),
                        "trailGeometrySha256": canonical_sha256(
                            (candidate_element.get("detail") or {}).get("trail")
                        ),
                    }
                    for proof_row, candidate_element in zip(
                        element_proofs, candidate_elements, strict=True
                    )
                ],
                "missingAnchorControl": copy.deepcopy(missing),
                "disposition": "DRAWABLE_PROOF_PASS",
            }
        )

    proof: dict[str, Any] = {
        "schema": PROOF_SCHEMA,
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "classification": "PROJECT_TUNED",
        "reconstructionPolicy": "BOUNDED_RECONSTRUCTION",
        "candidateManifest": {
            "path": manifest_relative.as_posix(),
            "rawSha256": hashlib.sha256(manifest_payload).hexdigest(),
            "artifactSha256": manifest.get("artifactSha256"),
        },
        "runtimeSweep": {
            "path": SWEEP_RELATIVE_PATH.as_posix(),
            "rawSha256": raw_sha256(sweep_path),
            "resourceRoot": str(resource_root),
        },
        "sourceCanary": copy.deepcopy(manifest.get("sourceFamily")),
        "modelEvidence": copy.deepcopy(manifest.get("modelEvidence")),
        "targets": proof_targets,
        "summary": {
            "targetCount": 3,
            "candidateElementCount": 9,
            "movingBoneDrawableCount": 9,
            "stationarySuppressedCount": 9,
            "missingAnchorRollbackCount": 3,
            "failedDrawCount": 0,
        },
        "disposition": "DRAWABLE_PROOF_PASS",
    }
    proof["artifactSha256"] = canonical_sha256(proof)
    validate_proof(proof)
    return proof


def validate_proof(proof: dict[str, Any]) -> None:
    _verify_seal(proof, "artifactSha256", "drawable proof")
    if (
        proof.get("schema") != PROOF_SCHEMA
        or proof.get("formatVersion") != 1
        or proof.get("bossArchetypeId") != "BOSS_VALTAN"
        or proof.get("classification") != "PROJECT_TUNED"
        or proof.get("reconstructionPolicy") != "BOUNDED_RECONSTRUCTION"
        or proof.get("disposition") != "DRAWABLE_PROOF_PASS"
        or proof.get("summary")
        != {
            "targetCount": 3,
            "candidateElementCount": 9,
            "movingBoneDrawableCount": 9,
            "stationarySuppressedCount": 9,
            "missingAnchorRollbackCount": 3,
            "failedDrawCount": 0,
        }
        or len(proof.get("targets") or []) != 3
    ):
        raise ProofError("drawable proof root contract changed")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--repository-root", type=Path, default=DEFAULT_REPOSITORY_ROOT
    )
    parser.add_argument("--expected-resource-root", type=Path)
    arguments = parser.parse_args(argv)
    try:
        proof = build_proof(
            arguments.repository_root,
            expected_resource_root=arguments.expected_resource_root,
        )
        output = _path(arguments.repository_root.resolve(), PROOF_RELATIVE_PATH)
        payload = pretty_json_bytes(proof)
        if arguments.check:
            if not output.is_file() or output.read_bytes() != payload:
                raise ProofError("drawable proof output is stale")
            print("Valtan bounded weapon Trail drawable proof: check PASS (9/9)")
        else:
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(payload)
            print("Valtan bounded weapon Trail drawable proof: write PASS (9/9)")
        return 0
    except ProofError as error:
        print(
            f"Valtan bounded weapon Trail drawable proof: FAILURE: {error}",
            file=sys.stderr,
        )
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
