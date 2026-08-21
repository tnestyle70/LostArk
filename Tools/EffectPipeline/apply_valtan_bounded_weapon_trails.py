#!/usr/bin/env python3
"""Apply the proved bounded Valtan weapon Trails as an atomic missing-only batch."""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
import hashlib
import importlib.util
import json
import os
from pathlib import Path, PurePosixPath
import sys
import tempfile
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
    "build_valtan_bounded_weapon_trail_candidates_for_apply",
    SCRIPT_PATH.with_name("build_valtan_bounded_weapon_trail_candidates.py"),
)
proof_builder = _load_sibling_module(
    "build_valtan_bounded_weapon_trail_drawable_proof_for_apply",
    SCRIPT_PATH.with_name("build_valtan_bounded_weapon_trail_drawable_proof.py"),
)

RECEIPT_SCHEMA = "lostark.valtan-bounded-weapon-trail-application-receipt"
RECEIPT_RELATIVE_PATH = candidate_builder.RECEIPT_RELATIVE_PATH
PROTECTED_EXTERNAL_PATHS = (
    candidate_builder.PATTERN_BINDINGS_RELATIVE_PATH,
    candidate_builder.PATTERN_CUES_RELATIVE_PATH,
    candidate_builder.EFFECT_CATALOG_RELATIVE_PATH,
    candidate_builder.SOURCE_DOCUMENT_RELATIVE_PATH,
)


class ApplicationError(RuntimeError):
    """The transactional projection could not be committed or rolled back."""


class SourceRebaseRequired(ApplicationError):
    """Candidate, proof, source family, or canonical target changed."""


@dataclass(frozen=True)
class Projection:
    root: Path
    outputs: dict[PurePosixPath, bytes]
    changed_paths: tuple[PurePosixPath, ...]
    already_applied: bool
    protected_before: dict[PurePosixPath, bytes]


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def raw_sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def _path(root: Path, relative: PurePosixPath) -> Path:
    try:
        return candidate_builder.repository_path(root, relative)
    except candidate_builder.CandidateError as error:
        raise SourceRebaseRequired(str(error)) from error


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise SourceRebaseRequired(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise SourceRebaseRequired(f"JSON root must be an object: {path}")
    return value


def _verify_seal(value: dict[str, Any], field: str, label: str) -> None:
    expected = value.get(field)
    clone = copy.deepcopy(value)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise SourceRebaseRequired(f"{label} {field} is stale")


def _artifact_inputs(root: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    try:
        expected = candidate_builder.build_outputs(root)
    except candidate_builder.CandidateError as error:
        raise SourceRebaseRequired(str(error)) from error
    for relative, payload in expected.items():
        path = _path(root, relative)
        if not path.is_file() or path.read_bytes() != payload:
            raise SourceRebaseRequired(
                f"candidate artifact is stale: {relative.as_posix()}"
            )
    manifest_relative = candidate_builder.OUTPUT_ROOT / PurePosixPath(
        candidate_builder.MANIFEST_FILENAME
    )
    manifest = _read_json(_path(root, manifest_relative))
    _verify_seal(manifest, "artifactSha256", "candidate manifest")
    try:
        expected_proof = proof_builder.build_proof(
            root,
            expected_resource_root=(root / "Client/Bin/Resources"),
        )
    except proof_builder.ProofError as error:
        raise SourceRebaseRequired(str(error)) from error
    proof_path = _path(root, proof_builder.PROOF_RELATIVE_PATH)
    if (
        not proof_path.is_file()
        or proof_path.read_bytes() != proof_builder.pretty_json_bytes(expected_proof)
    ):
        raise SourceRebaseRequired("drawable proof is stale")
    proof = _read_json(proof_path)
    _verify_seal(proof, "artifactSha256", "drawable proof")
    return manifest, proof


def _protected_snapshot(root: Path) -> dict[PurePosixPath, bytes]:
    result: dict[PurePosixPath, bytes] = {}
    for relative in PROTECTED_EXTERNAL_PATHS:
        path = _path(root, relative)
        if not path.is_file():
            raise SourceRebaseRequired(
                f"protected external input is missing: {relative.as_posix()}"
            )
        result[relative] = path.read_bytes()
    if (
        raw_sha256_bytes(result[candidate_builder.SOURCE_DOCUMENT_RELATIVE_PATH])
        != candidate_builder.SOURCE_DOCUMENT_RAW_SHA256
    ):
        raise SourceRebaseRequired("protected Whirlwind canary bytes changed")
    return result


def _scan_candidate_identities(
    root: Path,
) -> dict[str, list[tuple[PurePosixPath, dict[str, Any]]]]:
    expected_ids = {
        element_id
        for target in candidate_builder.TARGETS
        for element_id in candidate_builder.candidate_element_ids(target)
    }
    expected_source_nodes = {
        (
            f"project-tuned:bounded-reconstruction:{target.adapter_target_id}:"
            f"{candidate_builder.SOURCE_FAMILY_NAME}:{source[0]}"
        )
        for target in candidate_builder.TARGETS
        for source in candidate_builder.SOURCE_ELEMENTS
    }
    expected_anchors = {
        target.runtime_anchor_slot_id for target in candidate_builder.TARGETS
    }
    matches = {"id": [], "sourceNode": [], "anchor": []}
    authored_root = _path(root, PurePosixPath("Data/Effects/Authored"))
    for path in sorted(authored_root.glob("effect.valtan.*.effect.json")):
        document = _read_json(path)
        relative = PurePosixPath(path.relative_to(root).as_posix())
        for element in document.get("elements") or []:
            if not isinstance(element, dict):
                continue
            if element.get("id") in expected_ids:
                matches["id"].append((relative, element))
            if element.get("sourceNode") in expected_source_nodes:
                matches["sourceNode"].append((relative, element))
            if (
                (element.get("actionCueAttachment") or {}).get(
                    "runtimeAnchorSlotId"
                )
                in expected_anchors
            ):
                matches["anchor"].append((relative, element))
    return matches


def _validate_committed_receipt(
    root: Path,
    receipt: dict[str, Any],
    manifest: dict[str, Any],
    proof: dict[str, Any],
) -> None:
    _verify_seal(receipt, "artifactSha256", "application receipt")
    manifest_relative = candidate_builder.OUTPUT_ROOT / PurePosixPath(
        candidate_builder.MANIFEST_FILENAME
    )
    manifest_path = _path(root, manifest_relative)
    proof_path = _path(root, proof_builder.PROOF_RELATIVE_PATH)
    if (
        receipt.get("schema") != RECEIPT_SCHEMA
        or receipt.get("formatVersion") != 1
        or receipt.get("transactionStatus") != "COMMITTED"
        or receipt.get("reconcileMode") != "MISSING_ONLY"
        or receipt.get("classification") != "PROJECT_TUNED"
        or receipt.get("reconstructionPolicy") != "BOUNDED_RECONSTRUCTION"
        or receipt.get("candidateManifest")
        != {
            "path": manifest_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(manifest_path.read_bytes()),
            "artifactSha256": manifest.get("artifactSha256"),
        }
        or receipt.get("drawableProof")
        != {
            "path": proof_builder.PROOF_RELATIVE_PATH.as_posix(),
            "rawSha256": raw_sha256_bytes(proof_path.read_bytes()),
            "artifactSha256": proof.get("artifactSha256"),
        }
        or (receipt.get("sourceCanary") or {}).get("rawSha256")
        != candidate_builder.SOURCE_DOCUMENT_RAW_SHA256
        or len(receipt.get("targets") or []) != 3
        or receipt.get("summary")
        != {
            "targetCount": 3,
            "appendedElementCount": 9,
            "preservedExistingElementCount": sum(
                target.baseline_element_count for target in candidate_builder.TARGETS
            ),
            "movingBoneDrawableCount": 9,
            "stationarySuppressedCount": 9,
            "missingAnchorRollbackCount": 3,
        }
    ):
        raise SourceRebaseRequired("committed application receipt contract changed")
    for target, row in zip(
        candidate_builder.TARGETS, receipt["targets"], strict=True
    ):
        candidate_path = _path(
            root,
            candidate_builder.OUTPUT_ROOT
            / PurePosixPath(target.candidate_filename),
        )
        candidate = _read_json(candidate_path)
        if (
            row.get("patternId") != target.pattern_id
            or row.get("effectAssetId") != target.effect_asset_id
            or row.get("canonicalPath")
            != target.canonical_relative_path.as_posix()
            or row.get("runtimeAnchorSlotId") != target.runtime_anchor_slot_id
            or row.get("appendedElementIds")
            != list(candidate_builder.candidate_element_ids(target))
            or row.get("initialElementSha256")
            != [canonical_sha256(element) for element in candidate["elements"]]
            or row.get("protectedElementContractSha256")
            != [
                canonical_sha256(
                    candidate_builder.protected_element_contract(element)
                )
                for element in candidate["elements"]
            ]
        ):
            raise SourceRebaseRequired(
                f"committed receipt target changed: {target.pattern_id}"
            )


def collect_projection(root: Path) -> Projection:
    root = root.resolve()
    manifest, proof = _artifact_inputs(root)
    protected_before = _protected_snapshot(root)
    identity_matches = _scan_candidate_identities(root)

    target_states: list[tuple[Any, dict[str, Any], dict[str, Any], bool]] = []
    for target in candidate_builder.TARGETS:
        try:
            current, _, applied = candidate_builder._target_base_document(root, target)
        except candidate_builder.CandidateError as error:
            raise SourceRebaseRequired(str(error)) from error
        candidate_relative = candidate_builder.OUTPUT_ROOT / PurePosixPath(
            target.candidate_filename
        )
        candidate = _read_json(_path(root, candidate_relative))
        target_states.append((target, current, candidate, applied))

    applied = [row[3] for row in target_states]
    if any(applied) and not all(applied):
        raise SourceRebaseRequired("bounded weapon Trail projection is partial")
    receipt_path = _path(root, RECEIPT_RELATIVE_PATH)
    expected_identity_count = len(candidate_builder.TARGETS) * len(
        candidate_builder.SOURCE_ELEMENTS
    )
    if all(applied):
        if (
            not receipt_path.is_file()
            or len(identity_matches["id"]) != expected_identity_count
            or len(identity_matches["sourceNode"]) != expected_identity_count
            or len(identity_matches["anchor"]) != expected_identity_count
        ):
            raise SourceRebaseRequired(
                "committed bounded weapon Trail identities are duplicated or orphaned"
            )
        receipt = _read_json(receipt_path)
        _validate_committed_receipt(root, receipt, manifest, proof)
        return Projection(root, {}, (), True, protected_before)

    if receipt_path.exists() or any(identity_matches.values()):
        raise SourceRebaseRequired(
            "candidate identity exists without a complete committed receipt"
        )

    outputs: dict[PurePosixPath, bytes] = {}
    receipt_targets: list[dict[str, Any]] = []
    for target, current, candidate, _ in target_states:
        before_payload = pretty_json_bytes(current)
        staged = copy.deepcopy(current)
        staged["elements"].extend(copy.deepcopy(candidate["elements"]))
        after_payload = pretty_json_bytes(staged)
        if before_payload == after_payload:
            raise SourceRebaseRequired(
                f"missing-only projection made no change: {target.pattern_id}"
            )
        outputs[target.canonical_relative_path] = after_payload
        receipt_targets.append(
            {
                "patternId": target.pattern_id,
                "stageId": target.stage_id,
                "effectAssetId": target.effect_asset_id,
                "canonicalPath": target.canonical_relative_path.as_posix(),
                "beforeRawSha256": raw_sha256_bytes(before_payload),
                "beforeCanonicalSha256": canonical_sha256(current),
                "afterRawSha256": raw_sha256_bytes(after_payload),
                "afterCanonicalSha256": canonical_sha256(staged),
                "preservedExistingElementCount": len(current["elements"]),
                "appendedElementIds": list(
                    candidate_builder.candidate_element_ids(target)
                ),
                "initialElementSha256": [
                    canonical_sha256(element) for element in candidate["elements"]
                ],
                "protectedElementContractSha256": [
                    canonical_sha256(
                        candidate_builder.protected_element_contract(element)
                    )
                    for element in candidate["elements"]
                ],
                "runtimeAnchorSlotId": target.runtime_anchor_slot_id,
                "runtimeBoneName": candidate_builder.RUNTIME_BONE_NAME,
                "sourceStartSeconds": target.source_time_seconds,
                "sourceDurationSeconds": target.source_duration_seconds,
            }
        )

    manifest_relative = candidate_builder.OUTPUT_ROOT / PurePosixPath(
        candidate_builder.MANIFEST_FILENAME
    )
    manifest_path = _path(root, manifest_relative)
    proof_path = _path(root, proof_builder.PROOF_RELATIVE_PATH)
    receipt: dict[str, Any] = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": 1,
        "transactionStatus": "COMMITTED",
        "reconcileMode": "MISSING_ONLY",
        "classification": "PROJECT_TUNED",
        "reconstructionPolicy": "BOUNDED_RECONSTRUCTION",
        "candidateManifest": {
            "path": manifest_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(manifest_path.read_bytes()),
            "artifactSha256": manifest.get("artifactSha256"),
        },
        "drawableProof": {
            "path": proof_builder.PROOF_RELATIVE_PATH.as_posix(),
            "rawSha256": raw_sha256_bytes(proof_path.read_bytes()),
            "artifactSha256": proof.get("artifactSha256"),
        },
        "sourceCanary": {
            "path": candidate_builder.SOURCE_DOCUMENT_RELATIVE_PATH.as_posix(),
            "rawSha256": candidate_builder.SOURCE_DOCUMENT_RAW_SHA256,
            "canonicalSha256": candidate_builder.SOURCE_DOCUMENT_CANONICAL_SHA256,
        },
        "protectedExternalInputs": [
            {
                "path": relative.as_posix(),
                "rawSha256AtCommit": raw_sha256_bytes(payload),
            }
            for relative, payload in protected_before.items()
        ],
        "targets": receipt_targets,
        "summary": {
            "targetCount": 3,
            "appendedElementCount": 9,
            "preservedExistingElementCount": sum(
                target.baseline_element_count for target in candidate_builder.TARGETS
            ),
            "movingBoneDrawableCount": 9,
            "stationarySuppressedCount": 9,
            "missingAnchorRollbackCount": 3,
        },
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    outputs[RECEIPT_RELATIVE_PATH] = pretty_json_bytes(receipt)
    changed = tuple(
        target.canonical_relative_path for target in candidate_builder.TARGETS
    ) + (RECEIPT_RELATIVE_PATH,)
    return Projection(root, outputs, changed, False, protected_before)


def check_projection(projection: Projection) -> None:
    if not projection.already_applied or projection.changed_paths or projection.outputs:
        raise SourceRebaseRequired("bounded weapon Trail projection is not committed")
    for relative, payload in projection.protected_before.items():
        if _path(projection.root, relative).read_bytes() != payload:
            raise SourceRebaseRequired(
                f"protected external input changed during check: {relative.as_posix()}"
            )


def _atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".staging.", dir=path.parent
    )
    os.close(descriptor)
    temporary = Path(temporary_name)
    try:
        temporary.write_bytes(payload)
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def commit_projection(
    projection: Projection, *, failure_after_promote: int | None = None
) -> None:
    if projection.already_applied:
        check_projection(projection)
        return
    originals: dict[PurePosixPath, bytes | None] = {}
    promoted: list[PurePosixPath] = []
    staged: list[tuple[Path, PurePosixPath]] = []
    try:
        for relative in projection.changed_paths:
            target = _path(projection.root, relative)
            originals[relative] = target.read_bytes() if target.is_file() else None
            target.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=target.name + ".staging.", dir=target.parent
            )
            os.close(descriptor)
            temporary = Path(temporary_name)
            temporary.write_bytes(projection.outputs[relative])
            staged.append((temporary, relative))
        for temporary, relative in staged:
            os.replace(temporary, _path(projection.root, relative))
            promoted.append(relative)
            if (
                failure_after_promote is not None
                and len(promoted) >= failure_after_promote
            ):
                raise ApplicationError("injected transaction failure")
        for relative in projection.changed_paths:
            if _path(projection.root, relative).read_bytes() != projection.outputs[relative]:
                raise ApplicationError(
                    f"committed payload mismatch: {relative.as_posix()}"
                )
        for relative, payload in projection.protected_before.items():
            if _path(projection.root, relative).read_bytes() != payload:
                raise ApplicationError(
                    f"protected external input changed: {relative.as_posix()}"
                )
    except Exception as error:
        rollback_errors: list[str] = []
        for relative in reversed(promoted):
            target = _path(projection.root, relative)
            try:
                original = originals[relative]
                if original is None:
                    target.unlink(missing_ok=True)
                else:
                    _atomic_write(target, original)
            except OSError as rollback_error:
                rollback_errors.append(f"{relative.as_posix()}: {rollback_error}")
        if rollback_errors:
            raise ApplicationError(
                "transaction failed and rollback was incomplete: "
                + "; ".join(rollback_errors)
            ) from error
        raise ApplicationError(f"transaction failed and rolled back: {error}") from error
    finally:
        for temporary, _ in staged:
            temporary.unlink(missing_ok=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--apply", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--repository-root", type=Path, default=DEFAULT_REPOSITORY_ROOT
    )
    arguments = parser.parse_args(argv)
    try:
        projection = collect_projection(arguments.repository_root)
        if arguments.check:
            check_projection(projection)
            print("Valtan bounded weapon Trails: check PASS (changed=0, 9/9)")
        elif arguments.dry_run:
            print(
                "Valtan bounded weapon Trails: dry-run PASS "
                f"(changed={len(projection.changed_paths)}, "
                f"alreadyApplied={str(projection.already_applied).lower()})"
            )
            for relative in projection.changed_paths:
                print(relative.as_posix())
        else:
            commit_projection(projection)
            committed = collect_projection(arguments.repository_root)
            check_projection(committed)
            print("Valtan bounded weapon Trails: apply PASS (appended=9, changed=4)")
        return 0
    except ApplicationError as error:
        print(f"Valtan bounded weapon Trails: FAILURE: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
