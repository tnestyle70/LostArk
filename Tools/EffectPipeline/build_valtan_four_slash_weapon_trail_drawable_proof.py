#!/usr/bin/env python3
"""Seal the FourSlash stationary-root/moving-bone Trail renderer proof.

The runtime sweep is tracked immutable headless-renderer evidence. This builder
does not infer trajectory or draw results from JSON authoring data: it joins that
machine evidence to the exact isolated candidate, its preserved safe-gap row,
and the official Valtan weapon-bone identity before allowing canonical apply.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import sys
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
if str(SCRIPT_PATH.parent) not in sys.path:
    sys.path.insert(0, str(SCRIPT_PATH.parent))

import build_valtan_four_slash_weapon_trail_candidate as candidate_builder


SWEEP_SCHEMA = "lostark.valtan-four-slash-weapon-trail-runtime-sweep"
PROOF_SCHEMA = "lostark.valtan-four-slash-weapon-trail-drawable-proof"
FORMAT_VERSION = 1
OUTPUT_ROOT = candidate_builder.OUTPUT_DIRECTORY_RELATIVE_PATH
SWEEP_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "DrawableProof/Valtan.four-slash-weapon-trail.runtime-sweep.v1.json"
)
PROOF_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "DrawableProof/Valtan.four-slash-weapon-trail.drawable-proof.v1.json"
)
PROOF_SCHEMA_RELATIVE_PATH = PurePosixPath(
    "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-four-slash-weapon-trail-drawable-proof.schema.json"
)


class ProofError(RuntimeError):
    """The runtime evidence does not prove the bounded Trail candidate."""


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
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode(
        "utf-8"
    )


def read_json(path: Path) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
        value = json.loads(payload.decode("utf-8-sig"))
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


def verify_seal(value: dict[str, Any], field: str, label: str) -> None:
    expected = value.get(field)
    clone = copy.deepcopy(value)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise ProofError(f"{label} {field} is stale")


def seal(value: dict[str, Any], field: str) -> None:
    value.pop(field, None)
    value[field] = canonical_sha256(value)


def finite_vector(value: Any, size: int) -> bool:
    return (
        isinstance(value, list)
        and len(value) == size
        and all(
            isinstance(item, (int, float))
            and not isinstance(item, bool)
            and math.isfinite(float(item))
            for item in value
        )
    )


def _validate_manifest(
    root: Path, manifest: dict[str, Any]
) -> tuple[Path, dict[str, Any], dict[str, Any], Path, dict[str, Any]]:
    if (
        manifest.get("schema")
        != "lostark.valtan-four-slash-weapon-trail-candidate"
        or manifest.get("formatVersion") != 1
        or manifest.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise ProofError("FourSlash candidate manifest identity is invalid")
    verify_seal(manifest, "artifactSha256", "FourSlash candidate manifest")
    policy = manifest.get("policy") or {}
    if (
        policy.get("classification") != "PROJECT_TUNED"
        or policy.get("fidelity") != "BOUNDED_RECONSTRUCTION"
        or policy.get("reconcileMode") != "MISSING_ONLY"
        or policy.get("candidateOnly") is not True
        or policy.get("canonicalMutationPerformed") is not False
        or policy.get("cueBindingMutationPerformed") is not False
        or policy.get("catalogMutationPerformed") is not False
        or policy.get("sourceExactClaim") is not False
    ):
        raise ProofError("FourSlash candidate safety policy changed")

    target = manifest.get("target") or {}
    candidate_relative = PurePosixPath(str(target.get("candidatePath") or ""))
    canonical_relative = PurePosixPath(str(target.get("canonicalPath") or ""))
    if (
        target.get("patternId") != "VALTAN_FOUR_SLASH"
        or target.get("stageId") != "SLASHES"
        or target.get("actionId") != "valtan.attack.four-slash.active"
        or target.get("clipOccurrenceId")
        != "valtan.attack.four-slash.active.clip.02"
        or target.get("effectAssetId")
        != candidate_builder.SOURCE_EFFECT_ASSET_ID
        or target.get("candidateElementId")
        != candidate_builder.CANDIDATE_ELEMENT_ID
        or target.get("candidateSourceNode")
        != candidate_builder.CANDIDATE_SOURCE_NODE
        or candidate_relative.is_absolute()
        or canonical_relative.is_absolute()
        or ".." in candidate_relative.parts
        or ".." in canonical_relative.parts
    ):
        raise ProofError("FourSlash candidate target identity changed")

    candidate_path = repository_path(root, candidate_relative)
    candidate = read_json(candidate_path)
    elements = candidate.get("elements") or []
    if (
        candidate.get("schema") != "lostark.effect-authoring"
        or candidate.get("version") != 13
        or candidate.get("effectAssetId")
        != candidate_builder.SOURCE_EFFECT_ASSET_ID
        or len(elements) != 1
        or not isinstance(elements[0], dict)
    ):
        raise ProofError("FourSlash candidate Effect identity changed")
    element = elements[0]
    identity = manifest.get("candidateIdentity") or {}
    if (
        raw_sha256(candidate_path) != identity.get("rawSha256")
        or canonical_sha256(candidate) != identity.get("canonicalSha256")
        or canonical_sha256(element) != identity.get("elementSha256")
        or identity.get("elementCount") != 1
    ):
        raise ProofError("FourSlash candidate document hash closure changed")

    canonical_path = repository_path(root, canonical_relative)
    canonical = read_json(canonical_path)
    input_identity = manifest.get("inputIdentity") or {}
    if (
        raw_sha256(canonical_path)
        != input_identity.get("canonicalDocumentRawSha256")
        or canonical_sha256(canonical)
        != input_identity.get("canonicalDocumentCanonicalSha256")
    ):
        raise ProofError("FourSlash canonical source requires rebase")
    try:
        source = candidate_builder._source_element(canonical)
    except candidate_builder.CandidateError as error:
        raise ProofError(str(error)) from error
    if (
        source.get("id") != input_identity.get("preservedSourceElementId")
        or canonical_sha256(source)
        != input_identity.get("preservedSourceElementSha256")
        or source.get("actionCueAttachment")
        != input_identity.get("preservedSourceElementAttachment")
    ):
        raise ProofError("FourSlash safe-gap source row changed")
    return candidate_path, candidate, element, canonical_path, source


def _validate_runtime_sweep(
    root: Path,
    sweep_path: Path,
    sweep: dict[str, Any],
    candidate_path: Path,
    candidate: dict[str, Any],
    expected_resource_root: Path,
) -> None:
    if (
        sweep.get("schema") != SWEEP_SCHEMA
        or sweep.get("formatVersion") != FORMAT_VERSION
        or sweep.get("bossArchetypeId") != "BOSS_VALTAN"
        or sweep.get("effectAssetId")
        != candidate_builder.SOURCE_EFFECT_ASSET_ID
        or sweep.get("elementId") != candidate_builder.CANDIDATE_ELEMENT_ID
        or sweep.get("runtimeAnchorSlotId")
        != candidate_builder.RUNTIME_ANCHOR_SLOT_ID
        or sweep.get("runtimeBoneName") != candidate_builder.RUNTIME_BONE_NAME
        or sweep.get("sampleRateHz") != 60
        or sweep.get("disposition") != "DRAWABLE_PROOF_PASS"
    ):
        raise ProofError("FourSlash runtime sweep header is invalid")
    if Path(str(sweep.get("candidatePath") or "")).resolve() != candidate_path:
        raise ProofError("FourSlash runtime sweep candidate path changed")
    if sweep.get("candidateRawSha256") != raw_sha256(candidate_path):
        raise ProofError("FourSlash runtime sweep candidate SHA is stale")
    typed_codec_sha256 = sweep.get("candidateTypedCodecSha256")
    if not (
        isinstance(typed_codec_sha256, str)
        and len(typed_codec_sha256) == 64
        and all(character in "0123456789abcdef" for character in typed_codec_sha256)
    ):
        raise ProofError("FourSlash runtime sweep typed-codec SHA is invalid")
    if Path(str(sweep.get("resourceRoot") or "")).resolve() != (
        expected_resource_root.resolve()
    ):
        raise ProofError("FourSlash runtime sweep used the wrong resource root")
    if relative_path(root, sweep_path) != SWEEP_RELATIVE_PATH.as_posix():
        raise ProofError("FourSlash runtime sweep path is not canonical")

    positive = sweep.get("positiveMovingBone") or {}
    renderer = positive.get("renderer") or {}
    if (
        positive.get("rootWorldDistinctCount") != 1
        or int(positive.get("anchorWorldDistinctCount") or 0) < 2
        or int(positive.get("providerSampleCount") or 0) < 2
        or int(positive.get("trailPointCount") or 0) < 2
        or int(positive.get("distinctTrailPointCount") or 0) < 2
        or int(positive.get("finiteTrailPointCount") or 0)
        != int(positive.get("trailPointCount") or -1)
        or not isinstance(positive.get("cumulativeDistance"), (int, float))
        or not math.isfinite(float(positive.get("cumulativeDistance")))
        or float(positive.get("cumulativeDistance")) <= 0.0
        or not finite_vector(positive.get("firstWorldPosition"), 3)
        or not finite_vector(positive.get("lastWorldPosition"), 3)
        or int(renderer.get("preparedSamples") or 0) < 1
        or int(renderer.get("attemptedSamples") or 0) < 1
        or int(renderer.get("submittedDraws") or 0) < 1
        or int(renderer.get("committedDraws") or 0) < 1
        or int(renderer.get("failedDraws", -1)) != 0
        or renderer.get("transactionCommitted") is not True
    ):
        raise ProofError(
            "stationary-root/moving-bone Trail did not produce finite renderer output"
        )

    stationary = sweep.get("stationaryControl") or {}
    stationary_renderer = stationary.get("renderer") or {}
    if (
        stationary.get("rootWorldDistinctCount") != 1
        or stationary.get("anchorWorldDistinctCount") != 1
        or int(stationary.get("trailPointCount") or 0) > 1
        or int(stationary.get("distinctTrailPointCount") or 0) > 1
        or int(stationary_renderer.get("submittedDraws") or 0) != 0
        or int(stationary_renderer.get("committedDraws") or 0) != 0
        or int(stationary_renderer.get("failedDraws") or 0) != 0
        or stationary.get("segmentSuppressed") is not True
    ):
        raise ProofError("stationary root/anchor control was not suppressed")

    missing = sweep.get("missingAnchorControl") or {}
    if (
        missing.get("providerRejected") is not True
        or missing.get("playbackStatePreserved") is not True
        or missing.get("trailPointCountAfterReject") != 0
        or candidate_builder.RUNTIME_ANCHOR_SLOT_ID
        not in str(missing.get("status") or "")
    ):
        raise ProofError("missing-anchor control did not fail closed")


def validate_proof(proof: dict[str, Any]) -> None:
    if (
        proof.get("schema") != PROOF_SCHEMA
        or proof.get("formatVersion") != FORMAT_VERSION
        or proof.get("bossArchetypeId") != "BOSS_VALTAN"
        or proof.get("disposition") != "PROOF_PASS_CANONICAL_APPLY_ALLOWED"
    ):
        raise ProofError("FourSlash drawable proof header is invalid")
    verify_seal(proof, "artifactSha256", "FourSlash drawable proof")
    summary = proof.get("summary") or {}
    if summary != {
        "candidateElementCount": 1,
        "movingBoneTrailPointMinimum": 2,
        "stationaryControlMaximumTrailPoints": 1,
        "missingAnchorRejected": True,
        "rendererDrawMinimum": 1,
        "safeGapSourceRowsPreserved": 1,
        "failedDrawCount": 0,
    }:
        raise ProofError("FourSlash drawable proof summary changed")


def build_proof(
    repository_root: Path,
    *,
    manifest_path: Path | None = None,
    sweep_path: Path | None = None,
    expected_resource_root: Path,
) -> dict[str, Any]:
    root = repository_root.resolve()
    resolved_manifest = manifest_path or repository_path(
        root, OUTPUT_ROOT / candidate_builder.MANIFEST_FILENAME
    )
    resolved_sweep = sweep_path or repository_path(root, SWEEP_RELATIVE_PATH)
    manifest = read_json(resolved_manifest)
    candidate_path, candidate, element, canonical_path, source = (
        _validate_manifest(root, manifest)
    )
    sweep = read_json(resolved_sweep)
    _validate_runtime_sweep(
        root,
        resolved_sweep,
        sweep,
        candidate_path,
        candidate,
        expected_resource_root,
    )

    positive = sweep["positiveMovingBone"]
    stationary = sweep["stationaryControl"]
    missing = sweep["missingAnchorControl"]
    proof = {
        "schema": PROOF_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "generator": {
            "script": (
                "Tools/EffectPipeline/"
                "build_valtan_four_slash_weapon_trail_drawable_proof.py"
            ),
            "schemaPath": PROOF_SCHEMA_RELATIVE_PATH.as_posix(),
            "policyVersion": 1,
        },
        "candidateManifest": {
            "path": relative_path(root, resolved_manifest),
            "rawSha256": raw_sha256(resolved_manifest),
            "artifactSha256": manifest["artifactSha256"],
        },
        "candidateDocument": {
            "path": relative_path(root, candidate_path),
            "rawSha256": raw_sha256(candidate_path),
            "canonicalSha256": canonical_sha256(candidate),
            "elementId": candidate_builder.CANDIDATE_ELEMENT_ID,
            "elementSha256": canonical_sha256(element),
        },
        "runtimeSweep": {
            "path": relative_path(root, resolved_sweep),
            "rawSha256": raw_sha256(resolved_sweep),
            "canonicalSha256": canonical_sha256(sweep),
            "resourceRoot": str(expected_resource_root.resolve()),
        },
        "sourcePreservation": {
            "canonicalPath": relative_path(root, canonical_path),
            "canonicalRawSha256": raw_sha256(canonical_path),
            "sourceElementId": candidate_builder.SOURCE_ELEMENT_ID,
            "sourceElementSha256": canonical_sha256(source),
            "sourceElementAttachmentSha256": canonical_sha256(
                source["actionCueAttachment"]
            ),
            "preservedRowCount": 1,
        },
        "geometryProof": {
            "rootWorldPolicy": "IDENTITY_AND_STATIONARY_FOR_ALL_FIXED_STEPS",
            "runtimeAnchorSlotId": candidate_builder.RUNTIME_ANCHOR_SLOT_ID,
            "runtimeBoneName": candidate_builder.RUNTIME_BONE_NAME,
            "providerSampleCount": positive["providerSampleCount"],
            "rootWorldDistinctCount": positive["rootWorldDistinctCount"],
            "anchorWorldDistinctCount": positive["anchorWorldDistinctCount"],
            "trailPointCount": positive["trailPointCount"],
            "distinctTrailPointCount": positive["distinctTrailPointCount"],
            "finiteTrailPointCount": positive["finiteTrailPointCount"],
            "cumulativeDistance": positive["cumulativeDistance"],
            "firstWorldPosition": positive["firstWorldPosition"],
            "lastWorldPosition": positive["lastWorldPosition"],
        },
        "rendererProof": copy.deepcopy(positive["renderer"]),
        "negativeControls": {
            "stationary": {
                "trailPointCount": stationary["trailPointCount"],
                "distinctTrailPointCount": stationary[
                    "distinctTrailPointCount"
                ],
                "submittedDraws": stationary["renderer"]["submittedDraws"],
                "committedDraws": stationary["renderer"]["committedDraws"],
                "segmentSuppressed": stationary["segmentSuppressed"],
            },
            "missingAnchor": copy.deepcopy(missing),
        },
        "summary": {
            "candidateElementCount": 1,
            "movingBoneTrailPointMinimum": 2,
            "stationaryControlMaximumTrailPoints": 1,
            "missingAnchorRejected": True,
            "rendererDrawMinimum": 1,
            "safeGapSourceRowsPreserved": 1,
            "failedDrawCount": 0,
        },
        "disposition": "PROOF_PASS_CANONICAL_APPLY_ALLOWED",
    }
    seal(proof, "artifactSha256")
    validate_proof(proof)
    return proof


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staging = path.with_name(f"{path.name}.staging.{os.getpid()}")
    try:
        staging.write_bytes(payload)
        os.replace(staging, path)
    except OSError as error:
        staging.unlink(missing_ok=True)
        raise ProofError(f"cannot atomically write proof: {error}") from error


def check_committed_proof(
    repository_root: Path,
    *,
    output_path: Path,
    expected_resource_root: Path,
) -> dict[str, Any]:
    """Check the sealed pre-apply proof after its projection has committed."""

    root = repository_root.resolve()
    canonical_output = repository_path(root, PROOF_RELATIVE_PATH)
    if output_path.resolve() != canonical_output:
        raise ProofError("committed FourSlash proof must use its canonical path")
    try:
        candidate_builder.build_outputs(root)
    except candidate_builder.CandidateError as error:
        raise ProofError(str(error)) from error

    proof = read_json(canonical_output)
    validate_proof(proof)
    manifest_path = repository_path(
        root,
        OUTPUT_ROOT / candidate_builder.MANIFEST_FILENAME,
    )
    manifest = read_json(manifest_path)
    verify_seal(manifest, "artifactSha256", "FourSlash candidate manifest")
    target = manifest.get("target") or {}
    candidate_relative = PurePosixPath(str(target.get("candidatePath") or ""))
    candidate_path = repository_path(root, candidate_relative)
    candidate = read_json(candidate_path)
    sweep_path = repository_path(root, SWEEP_RELATIVE_PATH)
    sweep = read_json(sweep_path)
    _validate_runtime_sweep(
        root,
        sweep_path,
        sweep,
        candidate_path,
        candidate,
        expected_resource_root,
    )
    elements = candidate.get("elements") or []
    if len(elements) != 1 or not isinstance(elements[0], dict):
        raise ProofError("committed FourSlash candidate denominator changed")
    if proof.get("candidateManifest") != {
        "path": relative_path(root, manifest_path),
        "rawSha256": raw_sha256(manifest_path),
        "artifactSha256": manifest["artifactSha256"],
    }:
        raise ProofError("committed FourSlash proof/manifest join changed")
    if proof.get("candidateDocument") != {
        "path": relative_path(root, candidate_path),
        "rawSha256": raw_sha256(candidate_path),
        "canonicalSha256": canonical_sha256(candidate),
        "elementId": candidate_builder.CANDIDATE_ELEMENT_ID,
        "elementSha256": canonical_sha256(elements[0]),
    }:
        raise ProofError("committed FourSlash proof/candidate join changed")
    if proof.get("runtimeSweep") != {
        "path": relative_path(root, sweep_path),
        "rawSha256": raw_sha256(sweep_path),
        "canonicalSha256": canonical_sha256(sweep),
        "resourceRoot": str(expected_resource_root.resolve()),
    }:
        raise ProofError("committed FourSlash proof/runtime sweep join changed")
    receipt_path = repository_path(
        root, candidate_builder.APPLICATION_RECEIPT_RELATIVE_PATH
    )
    receipt = read_json(receipt_path)
    verify_seal(receipt, "artifactSha256", "FourSlash application receipt")
    if receipt.get("drawableProof") != {
        "path": relative_path(root, canonical_output),
        "rawSha256": raw_sha256(canonical_output),
        "artifactSha256": proof["artifactSha256"],
    }:
        raise ProofError("committed FourSlash receipt/proof join changed")
    return proof


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--dry-run", action="store_true")
    parser.add_argument(
        "--repository-root", type=Path, default=DEFAULT_REPOSITORY_ROOT
    )
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--runtime-sweep", type=Path)
    parser.add_argument("--expected-resource-root", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    arguments = parser.parse_args(argv)
    root = arguments.repository_root.resolve()
    output = arguments.output or repository_path(root, PROOF_RELATIVE_PATH)
    try:
        try:
            proof = build_proof(
                root,
                manifest_path=arguments.manifest,
                sweep_path=arguments.runtime_sweep,
                expected_resource_root=arguments.expected_resource_root,
            )
        except ProofError:
            if not arguments.check or arguments.manifest or arguments.runtime_sweep:
                raise
            proof = check_committed_proof(
                root,
                output_path=output,
                expected_resource_root=arguments.expected_resource_root,
            )
        payload = pretty_bytes(proof)
        if arguments.write:
            write_atomic(output, payload)
            label = "written"
        elif arguments.check:
            if not output.is_file() or output.read_bytes() != payload:
                raise ProofError(f"FourSlash drawable proof is stale: {output}")
            label = "checked"
        else:
            label = "dry-run"
        print(
            "FourSlash weapon-bone Trail drawable proof "
            f"{label}: points={proof['geometryProof']['trailPointCount']} "
            f"draws={proof['rendererProof']['committedDraws']}"
        )
        return 0
    except (ProofError, OSError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
