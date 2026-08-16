#!/usr/bin/env python3
"""Promote UModel Material exports into fail-closed source-material evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import defaultdict
from pathlib import Path
from typing import Any


CATALOG_SCHEMAS = {
    "lostark.dimensionmaster-material-candidate-catalog": {"DIMENSIONMASTER"},
    # Parent Material facts are class-neutral: fx_m/bfx_m are shared FX
    # libraries that every playable class instances from.  A four-class
    # capture therefore carries one FOUR_CLASS evidence document and the
    # per-class admission boundary stays in the aggregate compiler.
    "lostark.four-class-material-candidate-catalog": {
        "ARTIST", "DIMENSIONMASTER", "LANCE_MASTER", "WARLORD", "FOUR_CLASS",
    },
}
RECEIPT_SCHEMAS = {
    "lostark.dimensionmaster-material-candidate-catalog":
        "lostark.dimensionmaster-source-material-evidence-receipt",
    "lostark.four-class-material-candidate-catalog":
        "lostark.four-class-source-material-evidence-receipt",
}


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def fallback_reason(candidate: dict[str, Any]) -> str | None:
    """Return a draw-blocking reason if parent Material props are not unique."""
    if candidate.get("fallbackBlockedReason"):
        return str(candidate["fallbackBlockedReason"])
    status = str(candidate.get("resolutionStatus") or "")
    if status == "UNSUPPORTED_DECAL_MATERIAL":
        return "UNSUPPORTED_DECAL_MATERIAL"
    if status and status not in {
        "RESOLVED_UMODEL_EXPORT", "RESOLVED_UMODEL_DUMP",
        # The exporter positively reported an empty parameter declaration.
        "RESOLVED_UMODEL_EMPTY_PARAMETERS",
    }:
        return "UNRESOLVED_OR_AMBIGUOUS_MATERIAL_EXPORT"
    if str(candidate.get("materialEvidenceStatus") or "") not in {
        # ``SOURCE_MATERIAL_EMPTY_PARAMETERS`` is the exporter positively
        # reporting that the parent Material declares no parameters.  That is
        # resolved evidence of an empty declaration, not a failed lookup, so
        # it must not block a draw the emitter's own bindings already prove.
        "SOURCE_MATERIAL_PROPS", "SOURCE_MATERIAL_DUMP",
        "SOURCE_MATERIAL_EMPTY_PARAMETERS",
    }:
        return "MISSING_OR_AMBIGUOUS_PARENT_MATERIAL_PROPS"
    if int(candidate.get("materialEvidencePropsCandidateCount", 0)) != 1:
        return "MISSING_OR_AMBIGUOUS_PARENT_MATERIAL_PROPS"
    if not str(
        candidate.get("materialEvidenceSha256")
        or candidate.get("materialEvidencePropsSha256")
        or ""
    ):
        return "MISSING_PARENT_MATERIAL_PROPS_PROVENANCE"
    if not str(
        candidate.get("sourceEvidenceSha256")
        or candidate.get("propsFileSha256")
        or ""
    ):
        return "MISSING_INSTANCE_MATERIAL_PROPS_PROVENANCE"
    return None


def canonical_candidate(
    candidate: dict[str, Any], source_material_path: str,
    parent_material_evidence: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    result = {
        "material_path": source_material_path,
        "object_name": str(candidate.get("object_name") or ""),
        "source_file": candidate.get("source_file"),
        "parent": candidate.get("parent"),
        "parent_source_file": candidate.get("parent_source_file"),
        "textures": list(candidate.get("textures") or []),
        "scalars": list(candidate.get("scalars") or []),
        "vectors": list(candidate.get("vectors") or []),
        "static_switches": list(candidate.get("static_switches") or []),
        "materialEvidenceStatus": str(
            candidate.get("materialEvidenceStatus") or ""
        ),
        "materialEvidencePropsFile": candidate.get("materialEvidencePropsFile"),
        "materialEvidencePropsSha256": candidate.get(
            "materialEvidencePropsSha256"
        ),
        "materialEvidencePropsCandidateCount": int(
            candidate.get("materialEvidencePropsCandidateCount", 0)
        ),
        "materialEvidenceFile": candidate.get("materialEvidenceFile"),
        "materialEvidenceSha256": candidate.get("materialEvidenceSha256"),
        "sourceEvidenceFile": candidate.get("sourceEvidenceFile"),
        "sourceEvidenceSha256": candidate.get("sourceEvidenceSha256"),
        "propsFile": candidate.get("propsFile"),
        "propsFileSha256": candidate.get("propsFileSha256"),
    }
    reason = fallback_reason(candidate)
    if reason:
        result["fallbackBlockedReason"] = reason
        return result
    parent_path = str(candidate.get("parent") or "")
    parent_hash = str(
        candidate.get("materialEvidenceSha256")
        or candidate.get("materialEvidencePropsSha256")
        or ""
    )
    evidence_ref = f"{parent_path.casefold()}@sha256:{parent_hash}"
    parent_entry = {
        "parentMaterialPath": parent_path,
        "propsFile": (
            candidate.get("materialEvidenceFile")
            or candidate.get("materialEvidencePropsFile")
        ),
        "propsFileSha256": parent_hash,
        "materialEvidence": dict(candidate.get("materialEvidence") or {}),
    }
    existing = parent_material_evidence.get(evidence_ref)
    if existing is not None and existing != parent_entry:
        raise ValueError(
            f"conflicting parent material evidence for {evidence_ref}"
        )
    parent_material_evidence[evidence_ref] = parent_entry
    result["materialEvidenceRef"] = evidence_ref
    return result


def receipt_candidates_by_path(
    extractor_receipt: dict[str, Any],
) -> dict[str, list[dict[str, Any]]]:
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for candidate in extractor_receipt.get("candidates", []):
        source_path = str(
            candidate.get("material_path")
            or candidate.get("sourceMaterialPath")
            or ""
        )
        if source_path:
            grouped[source_path.casefold()].append(candidate)
    return grouped


def build_evidence(
    catalog: dict[str, Any],
    extractor_map: dict[str, Any],
    extractor_receipt: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    schema = str(catalog.get("schema") or "")
    allowed_classes = CATALOG_SCHEMAS.get(schema)
    if allowed_classes is None:
        raise ValueError("DimensionMaster material candidate catalog is invalid")
    character_class = str(catalog.get("characterClass") or "").upper()
    if character_class not in allowed_classes:
        raise ValueError("material candidate catalog character class is invalid")
    map_rows = extractor_map.get("materials") or {}
    receipt_rows = receipt_candidates_by_path(extractor_receipt)
    materials: dict[str, list[dict[str, Any]]] = {}
    parent_material_evidence: dict[str, dict[str, Any]] = {}
    blocked = 0
    resolved_parent_props = 0
    missing_export = 0
    for request in catalog.get("unresolvedMaterialBindings", []):
        source_path = str(request.get("sourceMaterialPath") or "")
        if not source_path:
            raise ValueError("material candidate catalog has an empty source path")
        key = source_path.casefold()
        candidates = list(receipt_rows.get(key) or map_rows.get(key) or [])
        if not candidates:
            candidates = [{
                "material_path": source_path,
                "resolutionStatus": "MISSING_EXTRACTOR_CANDIDATE",
            }]
            missing_export += 1
        selected: list[dict[str, Any]] = []
        for candidate in candidates:
            row = canonical_candidate(
                candidate, source_path, parent_material_evidence
            )
            if row.get("fallbackBlockedReason"):
                blocked += 1
            else:
                resolved_parent_props += 1
            selected.append(row)
        materials[key] = selected

    evidence = {
        "schema": "lostark.effect-source-material-evidence",
        "formatVersion": 1,
        "characterClass": character_class,
        "captureMethod": "UModel Material3 props/dump",
        "checkpointSkillIds": [
            int(row["skillId"]) for row in catalog.get("skills", [])
        ],
        "parentMaterialEvidence": parent_material_evidence,
        "materials": materials,
    }
    receipt = {
        "schema": RECEIPT_SCHEMAS[schema],
        "formatVersion": 1,
        "characterClass": character_class,
        "summary": {
            "materialCandidateCount": len(materials),
            "resolvedParentPropsCandidateCount": resolved_parent_props,
            "failClosedCandidateCount": blocked,
            "missingExtractorCandidateCount": missing_export,
        },
    }
    return evidence, receipt


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--extractor-map", required=True, type=Path)
    parser.add_argument("--extractor-receipt", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    args = parser.parse_args()
    catalog = read_json(args.catalog)
    extractor_receipt = read_json(args.extractor_receipt)
    recorded_hash = str(extractor_receipt.get("sourceCatalogSha256") or "")
    if recorded_hash and recorded_hash != sha256_file(args.catalog):
        raise ValueError("extractor receipt does not belong to the supplied catalog")
    evidence, receipt = build_evidence(
        catalog, read_json(args.extractor_map), extractor_receipt
    )
    receipt["sources"] = [
        {"path": path.as_posix(), "sha256": sha256_file(path)}
        for path in (args.catalog, args.extractor_map, args.extractor_receipt)
    ]
    write_json_atomic(args.output, evidence)
    write_json_atomic(args.receipt, receipt)
    print(json.dumps(receipt["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
