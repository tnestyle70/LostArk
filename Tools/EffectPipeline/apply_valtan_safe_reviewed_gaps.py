#!/usr/bin/env python3
"""Atomically apply the proven four-document Valtan safe-gap slice."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "Tools/EffectPipeline"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_safe_reviewed_gap_candidates as candidates
import build_valtan_safe_reviewed_gap_drawable_proof as drawable_proof
import build_valtan_trail_adapter_packets as trail_packets


RECEIPT_PATH = (
    candidates.OUTPUT_ROOT
    / "Valtan.safe-reviewed-gap-application-receipt.v1.json"
)
SCHEMA = "lostark.valtan-safe-reviewed-gap-application-receipt"
FORMAT_VERSION = 1


class ApplyError(RuntimeError):
    pass


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


def raw_sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def raw_sha256(path: Path) -> str:
    return raw_sha256_bytes(path.read_bytes())


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def read_json(path: Path) -> dict[str, Any]:
    try:
        result = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ApplyError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(result, dict):
        raise ApplyError(f"JSON root is not an object: {path}")
    return result


def seal(row: dict[str, Any], field: str) -> None:
    row.pop(field, None)
    row[field] = canonical_sha256(row)


def verify_seal(row: dict[str, Any], field: str, label: str) -> None:
    expected = row.get(field)
    clone = copy.deepcopy(row)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise ApplyError(f"{label} {field} is stale")


def relative(path: Path) -> str:
    return path.resolve().relative_to(ROOT.resolve()).as_posix()


def _load_inputs() -> tuple[dict[str, Any], dict[str, Any]]:
    manifest = read_json(candidates.MANIFEST_PATH)
    candidates.validate_manifest(manifest)
    proof = read_json(drawable_proof.PROOF_PATH)
    drawable_proof.validate_proof(proof)
    if (
        proof.get("candidateManifest", {}).get("rawSha256")
        != raw_sha256(candidates.MANIFEST_PATH)
        or proof.get("candidateManifest", {}).get("artifactSha256")
        != manifest.get("artifactSha256")
        or proof.get("drawableSweep", {}).get("rawSha256")
        != raw_sha256(drawable_proof.SWEEP_PATH)
    ):
        raise ApplyError("safe-gap proof no longer seals its candidate/sweep")
    trail_packets.load_whirlwind_canary(ROOT)
    return manifest, proof


def _index(rows: list[dict[str, Any]], field: str, label: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        key = str(row.get(field) or "")
        if not key or key in result:
            raise ApplyError(f"{label} identity is empty or duplicated")
        result[key] = row
    return result


def _state(
    manifest: dict[str, Any], cues: dict[str, Any], catalog: dict[str, Any]
) -> str:
    input_identity = manifest["inputIdentity"]
    cue_rows = _index(cues.get("cues", []), "bindingId", "cue")
    catalog_rows = _index(catalog.get("effects", []), "effectAssetId", "catalog")
    proposed_cues = _index(manifest["proposedCueRows"], "bindingId", "proposed cue")
    proposed_catalog = _index(
        manifest["proposedCatalogRows"], "effectAssetId", "proposed catalog"
    )
    cue_matches = {
        key: cue_rows.get(key) == value for key, value in proposed_cues.items()
    }
    catalog_matches = {
        key: catalog_rows.get(key) == value
        for key, value in proposed_catalog.items()
    }
    none_present = all(key not in cue_rows for key in proposed_cues) and all(
        key not in catalog_rows for key in proposed_catalog
    )
    preimage_exact = (
        none_present
        and len(cues.get("cues", [])) == input_identity["cueCount"] == 104
        and len(catalog.get("effects", [])) == input_identity["catalogCount"]
        and canonical_sha256(cues) == input_identity["cueCanonicalSha256"]
        and canonical_sha256(catalog) == input_identity["catalogCanonicalSha256"]
    )
    all_present = all(cue_matches.values()) and all(catalog_matches.values())
    applied_exact = (
        all_present
        and len(cues.get("cues", [])) == input_identity["cueCount"] + 4
        and len(catalog.get("effects", [])) == input_identity["catalogCount"] + 4
    )
    if preimage_exact:
        return "PREAPPLY_EXACT"
    if applied_exact:
        for row in manifest["candidateDocuments"]:
            canonical_path = ROOT / row["canonicalPath"]
            if (
                not canonical_path.is_file()
                or raw_sha256(canonical_path) != row["rawSha256"]
            ):
                raise ApplyError("applied canonical Effect document drifted")
        return "APPLIED_EXACT"
    raise ApplyError("canonical cue/catalog state is partial or diverged")


def _post_documents(
    manifest: dict[str, Any], state: str
) -> tuple[dict[Path, bytes], list[dict[str, Any]]]:
    writes: dict[Path, bytes] = {}
    rows: list[dict[str, Any]] = []
    for candidate_row in manifest["candidateDocuments"]:
        candidate_path = ROOT / candidate_row["candidatePath"]
        canonical_path = ROOT / candidate_row["canonicalPath"]
        payload = candidate_path.read_bytes()
        document = read_json(candidate_path)
        if (
            raw_sha256_bytes(payload) != candidate_row["rawSha256"]
            or canonical_sha256(document) != candidate_row["canonicalSha256"]
        ):
            raise ApplyError("candidate document identity changed")
        if state == "PREAPPLY_EXACT":
            if canonical_path.exists():
                raise ApplyError(f"new canonical path already exists: {canonical_path}")
            writes[canonical_path] = payload
        rows.append(
            {
                "sliceId": candidate_row["sliceId"],
                "effectAssetId": candidate_row["effectAssetId"],
                "canonicalPath": candidate_row["canonicalPath"],
                "rawSha256": candidate_row["rawSha256"],
                "canonicalSha256": candidate_row["canonicalSha256"],
                "elementCount": candidate_row["elementCount"],
                "elementIds": copy.deepcopy(candidate_row["elementIds"]),
            }
        )
    return writes, rows


def _receipt(
    manifest: dict[str, Any],
    proof: dict[str, Any],
    cue_payload: bytes,
    catalog_payload: bytes,
    canonical_documents: list[dict[str, Any]],
) -> dict[str, Any]:
    cue_document = json.loads(cue_payload.decode("utf-8"))
    catalog_document = json.loads(catalog_payload.decode("utf-8"))
    candidates_by_effect = {
        row["effectAssetId"]: row for row in manifest["candidateDocuments"]
    }
    trail_projections = []
    for projection in manifest["adapterProjections"]:
        candidate = candidates_by_effect[projection["effectAssetId"]]
        row = copy.deepcopy(projection)
        row["canonicalPath"] = candidate["canonicalPath"]
        row["cueBindingId"] = candidate["cue"]["bindingId"]
        row["cueOccurrenceId"] = candidate["cue"]["occurrenceId"]
        row["clipOccurrenceId"] = candidate["clipOccurrenceId"]
        row["applicationDisposition"] = "APPLIED_EXACT_MISSING_ONLY"
        seal(row, "applicationProjectionSha256")
        trail_projections.append(row)
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "disposition": "APPLIED_PROOF_GATED_IDEMPOTENT",
        "candidateManifest": {
            "path": relative(candidates.MANIFEST_PATH),
            "rawSha256": raw_sha256(candidates.MANIFEST_PATH),
            "artifactSha256": manifest["artifactSha256"],
        },
        "drawableProof": {
            "path": relative(drawable_proof.PROOF_PATH),
            "rawSha256": raw_sha256(drawable_proof.PROOF_PATH),
            "artifactSha256": proof["artifactSha256"],
        },
        "canonicalCueDocument": {
            "path": relative(candidates.CUES_PATH),
            "rawSha256": raw_sha256_bytes(cue_payload),
            "canonicalSha256": canonical_sha256(cue_document),
            "cueCount": len(cue_document["cues"]),
            "addedBindingIds": sorted(
                row["bindingId"] for row in manifest["proposedCueRows"]
            ),
        },
        "canonicalCatalogDocument": {
            "path": relative(candidates.CATALOG_PATH),
            "rawSha256": raw_sha256_bytes(catalog_payload),
            "canonicalSha256": canonical_sha256(catalog_document),
            "effectCount": len(catalog_document["effects"]),
            "addedEffectAssetIds": sorted(
                row["effectAssetId"] for row in manifest["proposedCatalogRows"]
            ),
        },
        "canonicalDocuments": sorted(
            canonical_documents, key=lambda row: row["effectAssetId"]
        ),
        "trailProjections": sorted(
            trail_projections, key=lambda row: row["projectionId"]
        ),
        "preservedUnresolvedTrailRows": copy.deepcopy(
            manifest["preservedUnresolvedTrailRows"]
        ),
        "summary": {
            "addedCueCount": 4,
            "addedCatalogCount": 4,
            "addedDocumentCount": 4,
            "addedElementCount": 167,
            "coreProjectionCount": 160,
            "animationTrailProjectionCount": 6,
            "cascadeRibbonProjectionCount": 1,
            "preservedUnresolvedTrailRowCount": 6,
            "canonicalMutationFileCount": 6,
        },
    }
    seal(receipt, "artifactSha256")
    return receipt


def validate_receipt(document: dict[str, Any]) -> None:
    if (
        document.get("schema") != SCHEMA
        or document.get("formatVersion") != FORMAT_VERSION
        or document.get("disposition") != "APPLIED_PROOF_GATED_IDEMPOTENT"
    ):
        raise ApplyError("safe-gap application receipt header is invalid")
    verify_seal(document, "artifactSha256", "safe-gap application receipt")
    summary = document.get("summary") or {}
    if (
        summary.get("addedElementCount") != 167
        or summary.get("animationTrailProjectionCount") != 6
        or summary.get("cascadeRibbonProjectionCount") != 1
        or len(document.get("trailProjections") or []) != 7
    ):
        raise ApplyError("safe-gap application receipt denominator changed")
    for row in document["trailProjections"]:
        verify_seal(row, "applicationProjectionSha256", "trail application projection")


def _atomic_replace(writes: dict[Path, bytes]) -> None:
    staged: dict[Path, Path] = {}
    originals: dict[Path, bytes | None] = {}
    replaced: list[Path] = []
    try:
        for path, payload in sorted(writes.items(), key=lambda item: str(item[0])):
            path.parent.mkdir(parents=True, exist_ok=True)
            temporary = path.with_suffix(path.suffix + f".safe-gap.{os.getpid()}.tmp")
            temporary.write_bytes(payload)
            staged[path] = temporary
            originals[path] = path.read_bytes() if path.is_file() else None
        for path in sorted(staged, key=str):
            os.replace(staged[path], path)
            replaced.append(path)
    except OSError as error:
        for path in reversed(replaced):
            original = originals[path]
            try:
                if original is None:
                    path.unlink(missing_ok=True)
                else:
                    restore = path.with_suffix(path.suffix + f".safe-gap.restore.{os.getpid()}.tmp")
                    restore.write_bytes(original)
                    os.replace(restore, path)
            except OSError:
                pass
        raise ApplyError(f"safe-gap atomic transaction failed: {error}") from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def expected_application() -> tuple[str, dict[Path, bytes], dict[str, Any]]:
    manifest, proof = _load_inputs()
    cues = read_json(candidates.CUES_PATH)
    catalog = read_json(candidates.CATALOG_PATH)
    state = _state(manifest, cues, catalog)
    writes, canonical_documents = _post_documents(manifest, state)
    if state == "PREAPPLY_EXACT":
        cues = copy.deepcopy(cues)
        catalog = copy.deepcopy(catalog)
        cues["cues"] = sorted(
            cues["cues"] + copy.deepcopy(manifest["proposedCueRows"]),
            key=lambda row: row["bindingId"],
        )
        catalog["effects"] = sorted(
            catalog["effects"] + copy.deepcopy(manifest["proposedCatalogRows"]),
            key=lambda row: row["effectAssetId"],
        )
        cue_payload = pretty_bytes(cues)
        catalog_payload = pretty_bytes(catalog)
        writes[candidates.CUES_PATH] = cue_payload
        writes[candidates.CATALOG_PATH] = catalog_payload
    else:
        cue_payload = candidates.CUES_PATH.read_bytes()
        catalog_payload = candidates.CATALOG_PATH.read_bytes()
    receipt = _receipt(
        manifest, proof, cue_payload, catalog_payload, canonical_documents
    )
    validate_receipt(receipt)
    writes[RECEIPT_PATH] = pretty_bytes(receipt)
    return state, writes, receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--apply", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    state, writes, receipt = expected_application()
    if args.apply:
        changed = {
            path: payload
            for path, payload in writes.items()
            if not path.is_file() or path.read_bytes() != payload
        }
        _atomic_replace(changed)
        final_state, final_writes, final_receipt = expected_application()
        if final_state != "APPLIED_EXACT" or any(
            not path.is_file() or path.read_bytes() != payload
            for path, payload in final_writes.items()
        ):
            raise ApplyError("safe-gap post-apply verification failed")
        print(
            "Valtan safe reviewed gaps applied: changed="
            + str(len(changed))
            + " "
            + json.dumps(final_receipt["summary"], sort_keys=True)
        )
        return 0
    if state != "APPLIED_EXACT":
        raise ApplyError("safe-gap canonical state has not been applied")
    drift = [
        str(path)
        for path, payload in writes.items()
        if not path.is_file() or path.read_bytes() != payload
    ]
    if drift:
        raise ApplyError("safe-gap applied state drifted: " + ", ".join(drift))
    print(
        "Valtan safe reviewed gaps checked: changed=0 "
        + json.dumps(receipt["summary"], sort_keys=True)
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (
        ApplyError,
        candidates.SafeGapError,
        drawable_proof.ProofError,
        trail_packets.AdapterError,
    ) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
