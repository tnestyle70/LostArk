#!/usr/bin/env python3
"""Seal headless renderer evidence for the nine Valtan priority overlays.

The input sweep is an immutable headless-renderer capture for the exact
absolute candidate document paths.  This builder does not synthesize draw
results: it validates the recorded document/element denominator, counter
dispositions, resource root, and raw candidate hashes before projecting the
machine evidence into the narrower proof consumed by the canonical applicator.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import sys
from typing import Any, Iterable


PATCH_SCHEMA = "lostark.valtan-project-authored-priority-patch-plan"
SWEEP_SCHEMA = "lostark.effect-document-drawable-sweep"
PROOF_SCHEMA = "lostark.valtan-project-authored-priority-drawable-proof"
FORMAT_VERSION = 1
OWNER_ARCHETYPE_ID = "BOSS_VALTAN"
EXPECTED_TARGET_COUNT = 9

DEFAULT_PATCH_PLAN = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/"
    "Valtan.project-authored-priority.patch-plan.v1.json"
)
DEFAULT_DRAWABLE_SWEEP = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/DrawableProof/"
    "Valtan.project-authored-priority.drawable-sweep.v1.json"
)
DEFAULT_DRAWABLE_PROOF = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/DrawableProof/"
    "Valtan.project-authored-priority.drawable-proof.v1.json"
)


class DrawableProofError(RuntimeError):
    pass


def _reject_duplicate_pairs(pairs: Iterable[tuple[str, Any]]) -> dict[str, Any]:
    value: dict[str, Any] = {}
    for key, child in pairs:
        if key in value:
            raise DrawableProofError(f"duplicate JSON key: {key}")
        value[key] = child
    return value


def _load_json_bytes(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        payload = path.read_bytes()
        value = json.loads(
            payload.decode("utf-8-sig"),
            object_pairs_hook=_reject_duplicate_pairs,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise DrawableProofError(f"cannot parse JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise DrawableProofError(f"JSON root must be an object: {path}")
    return value, payload


def _sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, separators=(",", ": "))
        + "\n"
    ).encode("utf-8")


def _require_exact_keys(value: dict[str, Any], keys: tuple[str, ...], label: str) -> None:
    if tuple(value.keys()) != keys:
        raise DrawableProofError(f"{label} fields/order are invalid")


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise DrawableProofError(f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise DrawableProofError(f"{label} must be an array")
    return value


def _repo_input(root: Path, value: Path | PurePosixPath, label: str) -> tuple[PurePosixPath, Path]:
    raw = Path(*value.parts) if isinstance(value, PurePosixPath) else value
    candidate = raw if raw.is_absolute() else root / raw
    try:
        resolved = candidate.resolve(strict=True)
        relative = resolved.relative_to(root)
    except (OSError, ValueError) as exc:
        raise DrawableProofError(f"{label} must be an existing repository file") from exc
    if not resolved.is_file():
        raise DrawableProofError(f"{label} is not a file")
    return PurePosixPath(relative.as_posix()), resolved


def _output_path(root: Path, value: Path | PurePosixPath) -> tuple[PurePosixPath, Path]:
    raw = Path(*value.parts) if isinstance(value, PurePosixPath) else value
    candidate = raw if raw.is_absolute() else root / raw
    try:
        parent = candidate.parent.resolve(strict=True)
        relative_parent = parent.relative_to(root)
    except (OSError, ValueError) as exc:
        raise DrawableProofError("drawable proof output must have a repository parent") from exc
    relative = PurePosixPath(relative_parent.as_posix()) / candidate.name
    return relative, parent / candidate.name


def _counter(value: Any, label: str, minimum: int) -> int:
    if type(value) is not int or value < minimum:
        raise DrawableProofError(f"{label} is invalid")
    return value


def build_drawable_proof(
    repository_root: Path,
    *,
    patch_plan: Path | PurePosixPath = DEFAULT_PATCH_PLAN,
    drawable_sweep: Path | PurePosixPath = DEFAULT_DRAWABLE_SWEEP,
    expected_resource_root: Path,
) -> dict[str, Any]:
    root = repository_root.resolve(strict=True)
    _, patch_path = _repo_input(root, patch_plan, "patch plan")
    sweep_relative, sweep_path = _repo_input(root, drawable_sweep, "drawable sweep")
    plan, patch_payload = _load_json_bytes(patch_path)
    sweep, sweep_payload = _load_json_bytes(sweep_path)

    if (
        plan.get("schema") != PATCH_SCHEMA
        or plan.get("formatVersion") != FORMAT_VERSION
        or plan.get("ownerArchetypeId") != OWNER_ARCHETYPE_ID
    ):
        raise DrawableProofError("patch plan identity is invalid")
    targets = [
        _require_object(row, "patch-plan target")
        for row in _require_list(plan.get("targets"), "patch-plan.targets")
    ]
    if len(targets) != EXPECTED_TARGET_COUNT:
        raise DrawableProofError("patch plan must contain exactly nine targets")
    effect_ids = [row.get("targetEffectAssetId") for row in targets]
    if any(not isinstance(value, str) or not value for value in effect_ids):
        raise DrawableProofError("patch-plan target identity is invalid")
    if len(set(effect_ids)) != EXPECTED_TARGET_COUNT:
        raise DrawableProofError("patch-plan target identity is duplicated")

    _require_exact_keys(
        sweep,
        ("schema", "formatVersion", "resourceRoot", "sampleRateHz", "documents"),
        "drawable sweep root",
    )
    if (
        sweep.get("schema") != SWEEP_SCHEMA
        or sweep.get("formatVersion") != FORMAT_VERSION
        or sweep.get("sampleRateHz") != 60
    ):
        raise DrawableProofError("drawable sweep identity is invalid")
    resource_root_value = sweep.get("resourceRoot")
    if not isinstance(resource_root_value, str):
        raise DrawableProofError("drawable sweep resourceRoot is invalid")
    try:
        recorded_resource_root = Path(resource_root_value).resolve(strict=True)
        required_resource_root = expected_resource_root.resolve(strict=True)
    except OSError as exc:
        raise DrawableProofError("drawable sweep resource root does not exist") from exc
    if (
        not recorded_resource_root.is_dir()
        or not required_resource_root.is_dir()
        or recorded_resource_root != required_resource_root
    ):
        raise DrawableProofError("drawable sweep used the wrong resource root")

    documents = [
        _require_object(row, "drawable-sweep document")
        for row in _require_list(sweep.get("documents"), "drawable-sweep.documents")
    ]
    if len(documents) != EXPECTED_TARGET_COUNT:
        raise DrawableProofError("drawable sweep must contain exactly nine documents")
    if [row.get("effectAssetId") for row in documents] != effect_ids:
        raise DrawableProofError("drawable sweep target order/denominator changed")

    proof_targets: list[dict[str, Any]] = []
    seen_document_paths: set[Path] = set()
    for target, sweep_document in zip(targets, documents, strict=True):
        effect_id = target["targetEffectAssetId"]
        _require_exact_keys(
            sweep_document,
            (
                "documentPath",
                "effectAssetId",
                "durationSeconds",
                "sampleCount",
                "visibleElementCount",
                "preparedElementCount",
                "drawnElementCount",
                "disposition",
                "elements",
            ),
            f"drawable sweep document {effect_id}",
        )
        _, overlay_path = _repo_input(
            root,
            Path(target["overlayDocumentPath"]),
            f"overlay document {effect_id}",
        )
        document_path_value = sweep_document.get("documentPath")
        if not isinstance(document_path_value, str) or not Path(document_path_value).is_absolute():
            raise DrawableProofError(f"sweep document path is not absolute: {effect_id}")
        try:
            recorded_document_path = Path(document_path_value).resolve(strict=True)
        except OSError as exc:
            raise DrawableProofError(f"sweep document path is stale: {effect_id}") from exc
        if recorded_document_path != overlay_path or recorded_document_path in seen_document_paths:
            raise DrawableProofError(f"sweep document path/identity changed: {effect_id}")
        seen_document_paths.add(recorded_document_path)

        overlay, overlay_payload = _load_json_bytes(overlay_path)
        overlay_sha = _sha256(overlay_payload)
        if overlay_sha != target.get("overlayDocumentSha256"):
            raise DrawableProofError(f"overlay document SHA is stale: {effect_id}")
        if overlay.get("effectAssetId") != effect_id:
            raise DrawableProofError(f"overlay effect identity changed: {effect_id}")
        candidate_elements = [
            _require_object(row, f"overlay element {effect_id}")
            for row in _require_list(overlay.get("elements"), f"overlay.elements {effect_id}")
        ]
        candidate_ids = [row.get("id") for row in candidate_elements]
        if (
            not candidate_ids
            or any(not isinstance(value, str) or not value for value in candidate_ids)
            or len(set(candidate_ids)) != len(candidate_ids)
            or any(row.get("visible") is not True for row in candidate_elements)
        ):
            raise DrawableProofError(f"overlay visible element denominator is invalid: {effect_id}")

        duration = sweep_document.get("durationSeconds")
        if (
            not isinstance(duration, (int, float))
            or isinstance(duration, bool)
            or not math.isfinite(float(duration))
            or duration <= 0
            or duration > 60
        ):
            raise DrawableProofError(f"sweep duration is invalid: {effect_id}")
        sample_count = _counter(
            sweep_document.get("sampleCount"), f"sampleCount {effect_id}", 2
        )
        for count_key in (
            "visibleElementCount",
            "preparedElementCount",
            "drawnElementCount",
        ):
            if _counter(sweep_document.get(count_key), f"{count_key} {effect_id}", 1) != len(candidate_ids):
                raise DrawableProofError(f"sweep document closure changed: {effect_id}")
        if sweep_document.get("disposition") != "DRAWABLE_PROOF_PASS":
            raise DrawableProofError(f"sweep document did not pass: {effect_id}")

        element_rows = [
            _require_object(row, f"sweep element {effect_id}")
            for row in _require_list(
                sweep_document.get("elements"), f"sweep elements {effect_id}"
            )
        ]
        if [row.get("elementId") for row in element_rows] != candidate_ids:
            raise DrawableProofError(f"sweep element denominator changed: {effect_id}")
        proof_elements: list[dict[str, Any]] = []
        for row in element_rows:
            element_id = row.get("elementId")
            _require_exact_keys(
                row,
                (
                    "elementId",
                    "disposition",
                    "preparedSamples",
                    "attemptedSamples",
                    "submittedDraws",
                    "suppressedDraws",
                    "failedDraws",
                    "committedDraws",
                ),
                f"drawable sweep element {effect_id}/{element_id}",
            )
            if row.get("disposition") != "DRAWABLE_PROOF_PASS":
                raise DrawableProofError(f"sweep element did not pass: {effect_id}/{element_id}")
            prepared = _counter(
                row.get("preparedSamples"),
                f"preparedSamples {effect_id}/{element_id}",
                1,
            )
            attempted = _counter(
                row.get("attemptedSamples"),
                f"attemptedSamples {effect_id}/{element_id}",
                1,
            )
            submitted = _counter(
                row.get("submittedDraws"),
                f"submittedDraws {effect_id}/{element_id}",
                1,
            )
            suppressed = _counter(
                row.get("suppressedDraws"),
                f"suppressedDraws {effect_id}/{element_id}",
                0,
            )
            failed = _counter(
                row.get("failedDraws"),
                f"failedDraws {effect_id}/{element_id}",
                0,
            )
            committed = _counter(
                row.get("committedDraws"),
                f"committedDraws {effect_id}/{element_id}",
                1,
            )
            if prepared > sample_count or attempted > sample_count or failed != 0:
                raise DrawableProofError(
                    f"sweep element counters did not prove a clean draw: {effect_id}/{element_id}"
                )
            proof_elements.append(
                {
                    "elementId": element_id,
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "preparedSamples": prepared,
                    "attemptedSamples": attempted,
                    "submittedDraws": submitted,
                    "suppressedDraws": suppressed,
                    "failedDraws": failed,
                    "committedDraws": committed,
                }
            )
        proof_targets.append(
            {
                "effectAssetId": effect_id,
                "overlayDocumentSha256": overlay_sha,
                "disposition": "DRAWABLE_PROOF_PASS",
                "elements": proof_elements,
            }
        )

    return {
        "schema": PROOF_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "ownerArchetypeId": OWNER_ARCHETYPE_ID,
        "patchPlanSha256": _sha256(patch_payload),
        "drawableSweepPath": sweep_relative.as_posix(),
        "drawableSweepSha256": _sha256(sweep_payload),
        "resourceRoot": resource_root_value,
        "targets": proof_targets,
    }


def _write_atomic(path: Path, payload: bytes) -> None:
    staging = path.with_name(f"{path.name}.staging.{os.getpid()}")
    try:
        staging.write_bytes(payload)
        os.replace(staging, path)
    except OSError as exc:
        try:
            staging.unlink(missing_ok=True)
        except OSError:
            pass
        raise DrawableProofError(f"cannot atomically write drawable proof: {exc}") from exc


def _make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--repo-root", type=Path, default=Path(__file__).resolve().parents[2]
    )
    parser.add_argument(
        "--patch-plan", type=Path, default=Path(DEFAULT_PATCH_PLAN.as_posix())
    )
    parser.add_argument(
        "--drawable-sweep", type=Path, default=Path(DEFAULT_DRAWABLE_SWEEP.as_posix())
    )
    parser.add_argument("--expected-resource-root", type=Path, required=True)
    parser.add_argument(
        "--output", type=Path, default=Path(DEFAULT_DRAWABLE_PROOF.as_posix())
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _make_parser().parse_args(argv)
    try:
        root = args.repo_root.resolve(strict=True)
        _, output_path = _output_path(root, args.output)
        proof = build_drawable_proof(
            root,
            patch_plan=args.patch_plan,
            drawable_sweep=args.drawable_sweep,
            expected_resource_root=args.expected_resource_root,
        )
        payload = _json_bytes(proof)
        if args.check:
            if not output_path.is_file() or output_path.read_bytes() != payload:
                raise DrawableProofError("drawable proof output is stale")
            mode = "CHECK"
        else:
            _write_atomic(output_path, payload)
            mode = "WRITE"
    except (DrawableProofError, OSError) as exc:
        print(f"[FAILURE] {exc}", file=sys.stderr)
        return 1
    element_count = sum(len(row["elements"]) for row in proof["targets"])
    print(
        f"[PASS] {mode} Valtan drawable proof targets={len(proof['targets'])} "
        f"elements={element_count} sweepSha256={proof['drawableSweepSha256']} "
        f"proofSha256={_sha256(payload)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
