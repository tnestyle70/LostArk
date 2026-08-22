#!/usr/bin/env python3
"""Seal headless renderer evidence for four FRONT_BACK_FRONT source waves.

The input sweep is an immutable headless-renderer capture for the exact four
candidate document paths.  This builder never invents draw results:
it validates candidate hashes, document and element order, resource-root
identity, and every draw counter before producing the narrower proof consumed
by ``apply_valtan_front_back_front_source_waves.py``.
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


CANDIDATE_SCHEMA = "lostark.valtan-front-back-front-source-wave-candidates"
SWEEP_SCHEMA = "lostark.effect-document-drawable-sweep"
PROOF_SCHEMA = "lostark.valtan-front-back-front-source-wave-drawable-proof"
FORMAT_VERSION = 1
BOSS_ARCHETYPE_ID = "BOSS_VALTAN"
EXPECTED_TARGET_COUNT = 4
EXPECTED_ELEMENTS_PER_TARGET = 25
EXPECTED_ELEMENT_COUNT = 100

DEFAULT_CANDIDATE_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/"
    "Valtan.front-back-front-source-wave-candidates.v1.json"
)
DEFAULT_DRAWABLE_SWEEP = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/DrawableProof/"
    "Valtan.front-back-front-source-waves.drawable-sweep.v1.json"
)
DEFAULT_DRAWABLE_PROOF = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/DrawableProof/"
    "Valtan.front-back-front-source-waves.drawable-proof.v1.json"
)


class DrawableProofError(RuntimeError):
    """The machine sweep cannot prove the exact four candidate documents."""


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
        if payload.startswith(b"\xef\xbb\xbf"):
            raise DrawableProofError(f"JSON must be UTF-8 without BOM: {path}")
        value = json.loads(
            payload.decode("utf-8"), object_pairs_hook=_reject_duplicate_pairs
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
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
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


def _repository_input(
    root: Path, value: Path | PurePosixPath, label: str
) -> tuple[PurePosixPath, Path]:
    raw = Path(*value.parts) if isinstance(value, PurePosixPath) else value
    candidate = raw if raw.is_absolute() else root / raw
    try:
        resolved = candidate.resolve(strict=True)
        relative = resolved.relative_to(root)
    except (OSError, ValueError) as exc:
        raise DrawableProofError(
            f"{label} must be an existing repository file"
        ) from exc
    if not resolved.is_file():
        raise DrawableProofError(f"{label} is not a file")
    return PurePosixPath(relative.as_posix()), resolved


def _repository_output(
    root: Path, value: Path | PurePosixPath
) -> tuple[PurePosixPath, Path]:
    raw = Path(*value.parts) if isinstance(value, PurePosixPath) else value
    candidate = raw if raw.is_absolute() else root / raw
    try:
        parent = candidate.parent.resolve(strict=True)
        relative_parent = parent.relative_to(root)
    except (OSError, ValueError) as exc:
        raise DrawableProofError(
            "drawable proof output must have an existing repository parent"
        ) from exc
    relative = PurePosixPath(relative_parent.as_posix()) / candidate.name
    return relative, parent / candidate.name


def _counter(value: Any, label: str, minimum: int) -> int:
    if type(value) is not int or value < minimum:
        raise DrawableProofError(f"{label} is invalid")
    return value


def build_drawable_proof(
    repository_root: Path,
    *,
    candidate_receipt: Path | PurePosixPath = DEFAULT_CANDIDATE_RECEIPT,
    drawable_sweep: Path | PurePosixPath = DEFAULT_DRAWABLE_SWEEP,
    expected_resource_root: Path,
) -> dict[str, Any]:
    root = repository_root.resolve(strict=True)
    _, receipt_path = _repository_input(root, candidate_receipt, "candidate receipt")
    _, sweep_path = _repository_input(root, drawable_sweep, "drawable sweep")
    receipt, receipt_payload = _load_json_bytes(receipt_path)
    sweep, _ = _load_json_bytes(sweep_path)

    if (
        receipt.get("schema") != CANDIDATE_SCHEMA
        or receipt.get("formatVersion") != FORMAT_VERSION
        or receipt.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
    ):
        raise DrawableProofError("candidate receipt identity is invalid")
    candidates = [
        _require_object(row, "candidate")
        for row in _require_list(receipt.get("candidates"), "candidate receipt candidates")
    ]
    if len(candidates) != EXPECTED_TARGET_COUNT:
        raise DrawableProofError("candidate receipt must contain exactly four waves")
    effect_ids = [row.get("effectAssetId") for row in candidates]
    if (
        any(not isinstance(value, str) or not value for value in effect_ids)
        or len(set(effect_ids)) != EXPECTED_TARGET_COUNT
    ):
        raise DrawableProofError("candidate wave identity is invalid or duplicated")

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
    if not isinstance(resource_root_value, str) or not Path(resource_root_value).is_absolute():
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
        for row in _require_list(sweep.get("documents"), "drawable-sweep documents")
    ]
    if len(documents) != EXPECTED_TARGET_COUNT:
        raise DrawableProofError("drawable sweep must contain exactly four documents")
    if [row.get("effectAssetId") for row in documents] != effect_ids:
        raise DrawableProofError("drawable sweep target order/denominator changed")

    proof_targets: list[dict[str, Any]] = []
    global_pairs: set[tuple[str, str]] = set()
    seen_document_paths: set[Path] = set()
    for candidate, sweep_document in zip(candidates, documents, strict=True):
        effect_id = candidate["effectAssetId"]
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
        _, candidate_path = _repository_input(
            root, Path(candidate["candidateDocumentPath"]), f"candidate {effect_id}"
        )
        recorded_path_value = sweep_document.get("documentPath")
        if not isinstance(recorded_path_value, str) or not Path(recorded_path_value).is_absolute():
            raise DrawableProofError(f"sweep document path is not absolute: {effect_id}")
        try:
            recorded_path = Path(recorded_path_value).resolve(strict=True)
        except OSError as exc:
            raise DrawableProofError(f"sweep document path is stale: {effect_id}") from exc
        if recorded_path != candidate_path or recorded_path in seen_document_paths:
            raise DrawableProofError(f"sweep document path/identity changed: {effect_id}")
        seen_document_paths.add(recorded_path)

        candidate_document, candidate_payload = _load_json_bytes(candidate_path)
        candidate_sha = _sha256(candidate_payload)
        if candidate_sha != candidate.get("candidateDocumentSha256"):
            raise DrawableProofError(f"candidate document SHA is stale: {effect_id}")
        if candidate_document.get("effectAssetId") != effect_id:
            raise DrawableProofError(f"candidate effect identity changed: {effect_id}")
        candidate_elements = [
            _require_object(row, f"candidate element {effect_id}")
            for row in _require_list(
                candidate_document.get("elements"), f"candidate elements {effect_id}"
            )
        ]
        candidate_pairs = [(row.get("id"), row.get("sourceNode")) for row in candidate_elements]
        candidate_ids = [row[0] for row in candidate_pairs]
        if (
            len(candidate_elements) != EXPECTED_ELEMENTS_PER_TARGET
            or any(
                not isinstance(element_id, str)
                or not element_id
                or not isinstance(source_node, str)
                or not source_node
                for element_id, source_node in candidate_pairs
            )
            or len(set(candidate_pairs)) != EXPECTED_ELEMENTS_PER_TARGET
            or any(row.get("visible") is not True for row in candidate_elements)
        ):
            raise DrawableProofError(f"candidate element denominator is invalid: {effect_id}")
        if global_pairs.intersection(candidate_pairs):
            raise DrawableProofError("a source element occurs in more than one wave")
        global_pairs.update(candidate_pairs)

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
            if _counter(
                sweep_document.get(count_key), f"{count_key} {effect_id}", 1
            ) != EXPECTED_ELEMENTS_PER_TARGET:
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
        for source_element, row in zip(candidate_elements, element_rows, strict=True):
            element_id = source_element["id"]
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
                raise DrawableProofError(
                    f"sweep element did not pass: {effect_id}/{element_id}"
                )
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
                row.get("failedDraws"), f"failedDraws {effect_id}/{element_id}", 0
            )
            committed = _counter(
                row.get("committedDraws"),
                f"committedDraws {effect_id}/{element_id}",
                1,
            )
            if prepared > sample_count or attempted > sample_count or failed != 0:
                raise DrawableProofError(
                    f"sweep element counters did not prove a clean draw: "
                    f"{effect_id}/{element_id}"
                )
            proof_elements.append(
                {
                    "elementId": element_id,
                    "sourceNode": source_element["sourceNode"],
                    "disposition": "DRAWABLE_PROOF_PASS",
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
                "candidateDocumentSha256": candidate_sha,
                "disposition": "DRAWABLE_PROOF_PASS",
                "elements": proof_elements,
            }
        )

    if len(global_pairs) != EXPECTED_ELEMENT_COUNT:
        raise DrawableProofError("drawable proof must cover exactly 100 source elements")
    return {
        "schema": PROOF_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": BOSS_ARCHETYPE_ID,
        "candidateReceiptSha256": _sha256(receipt_payload),
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
        "--candidate-receipt", type=Path, default=Path(DEFAULT_CANDIDATE_RECEIPT.as_posix())
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
        _, output_path = _repository_output(root, args.output)
        proof = build_drawable_proof(
            root,
            candidate_receipt=args.candidate_receipt,
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
        f"[PASS] {mode} FRONT_BACK_FRONT drawable proof "
        f"targets={len(proof['targets'])} elements={element_count} "
        f"proofSha256={_sha256(payload)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
