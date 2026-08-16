#!/usr/bin/env python3
"""Build deduplicated Valtan source material/resource evidence.

Valtan repeats the same particle system, material and DDS across many actions,
stages and clips. Copying the source payload per occurrence would multiply the
same facts thousands of times, so this builder separates the two axes:

    notify occurrences  ->  stable source definitions  ->  material families

`Valtan.actionbindings.json` already owns the occurrence axis. This document
owns the definition axis and is referenced by stable identity, never inlined.

Nothing is inferred. A material path, package, parent or resource that the
source evidence does not state is recorded with a typed reason instead of a
guess.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
EXTERNAL = (ROOT.parent / "Resource_LostArk").resolve()

CATALOG_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
ACTION_CATALOG_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.action-particle-resource-catalog.json"
)
ACTION_BINDINGS_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.actionbindings.json"
)
MATERIAL_MAP_PATH = (
    EXTERNAL / "05_Reports/EffectExtraction/VALTAN/materials/Valtan.material-map.json"
)
RUNTIME_ROOT = ROOT / "Client/Bin/Resources"

OUTPUT_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json"
)
RECEIPT_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.source-material-evidence.receipt.json"
)


class EvidenceError(RuntimeError):
    pass


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def folded(value: Any) -> str:
    return str(value or "").casefold()


def build() -> tuple[dict[str, Any], dict[str, Any]]:
    catalog = load_json(CATALOG_PATH)
    if catalog.get("characterClass") != "VALTAN":
        raise EvidenceError("Valtan effect catalog identity drifted")

    material_map: dict[str, Any] = {}
    material_map_status = "NOT_CAPTURED"
    if MATERIAL_MAP_PATH.is_file():
        material_map = (load_json(MATERIAL_MAP_PATH).get("materials") or {})
        material_map_status = "UMODEL_PROPS_CAPTURED"

    # ---- definition axis: material identities -------------------------------
    materials: list[dict[str, Any]] = []
    reasons: Counter[str] = Counter()
    parent_ids: set[str] = set()
    for row in catalog.get("materialParameterBindings", []):
        source_path = str(row.get("sourceMaterialPath") or "")
        if not source_path:
            raise EvidenceError("material binding has no sourceMaterialPath")
        resolution = str(row.get("resolutionStatus") or "")
        parent = row.get("parent")
        entry: dict[str, Any] = {
            "materialId": source_path,
            "sourceMaterialPath": source_path,
            "sourceLogicalPackage": row.get("sourceLogicalPackage"),
            "sourcePhysicalPackage": row.get("sourcePhysicalPackage"),
            "expectedPhysicalPackage": row.get("expectedPhysicalPackage"),
            "packageResolutionStatus": resolution,
            "packageCandidateCount": int(row.get("candidateCount") or 0),
            "className": row.get("className"),
            "objectName": row.get("objectName"),
            "parentMaterialPath": parent,
            "parentSourcePhysicalPackage": row.get("parentSourcePhysicalPackage"),
            # MaterialInstance overrides, exactly as the source states them.
            "instanceTextures": list(row.get("textures") or []),
            "instanceScalars": list(row.get("scalars") or []),
            "instanceVectors": list(row.get("vectors") or []),
        }
        if parent:
            parent_ids.add(folded(parent))

        blockers: list[str] = []
        if resolution not in {
            "RESOLVED_EXACT_SOURCE_PACKAGE",
            "RESOLVED_IDENTICAL_COPIES",
            "RESOLVED_UNIQUE_PATH",
        }:
            blockers.append("PACKAGE_" + (resolution or "UNRESOLVED"))
        if not parent and folded(row.get("className")) == "materialinstanceconstant":
            blockers.append("PARENT_MATERIAL_UNDECLARED")

        # Parent Material3 declarations (group tags, render state, static
        # switches) never live in the catalog. They come from a UModel props
        # capture; absence is recorded, never substituted.
        declaration = None
        captured = material_map.get(folded(parent)) if parent else None
        if isinstance(captured, list):
            captured = captured[0] if captured else None
        if isinstance(captured, dict):
            evidence = captured.get("materialEvidence")
            if isinstance(evidence, dict):
                declaration = {
                    "renderState": evidence.get("renderState"),
                    "collectedTextureParameters":
                        evidence.get("collectedTextureParameters") or [],
                    "collectedScalarParameters":
                        evidence.get("collectedScalarParameters") or [],
                    "collectedVectorParameters":
                        evidence.get("collectedVectorParameters") or [],
                    "collectedStaticSwitchParameters":
                        evidence.get("collectedStaticSwitchParameters") or [],
                    "referencedTextures": evidence.get("referencedTextures") or [],
                    "expressionCoverage": evidence.get("expressionCoverage"),
                    "evidenceStatus": captured.get("materialEvidenceStatus"),
                }
        if declaration is None:
            blockers.append(
                "PARENT_DECLARATION_NOT_CAPTURED" if parent
                else "PARENT_DECLARATION_NOT_APPLICABLE"
            )
        entry["parentDeclaration"] = declaration
        entry["blockers"] = blockers
        for blocker in blockers:
            reasons[blocker] += 1
        materials.append(entry)

    # ---- definition axis: resources ----------------------------------------
    resources: list[dict[str, Any]] = []
    physical_missing: list[str] = []
    role_counts: Counter[str] = Counter()
    for asset in catalog.get("assets", []):
        asset_path = str(asset.get("sourceAssetPath") or "")
        roles = list(asset.get("roles") or [])
        for role in roles:
            role_counts[role] += 1
        resolution = str(asset.get("resolutionStatus") or "")
        entry = {
            "resourceId": asset_path,
            "sourceAssetPath": asset_path,
            "logicalPackage": asset.get("logicalPackage"),
            "physicalPackage": asset.get("physicalPackage"),
            "roles": roles,
            "sourceResolutionStatus": resolution,
            # Occurrence linkage stays by reference: the action ids are the join
            # key into Valtan.actionbindings.json, not a copy of its payload.
            "referencedByActionIdCount": len(asset.get("actionIds") or []),
            "referencedBySourceSystemCount": len(asset.get("sourceSystems") or []),
        }
        if resolution != "RESOLVED_SOURCE_PACKAGE":
            entry["blockers"] = ["SOURCE_" + (resolution or "UNRESOLVED")]
            reasons["SOURCE_" + (resolution or "UNRESOLVED")] += 1
        resources.append(entry)

    # ---- definition axis: source particle systems ---------------------------
    systems: list[dict[str, Any]] = []
    shape_counts: Counter[str] = Counter()
    for system in catalog.get("sourceSystems", []):
        graph = system.get("graph") or {}
        entry = {
            "sourceSystemId": str(system.get("sourceAsset") or ""),
            "logicalPackage": system.get("logicalPackage"),
            "objectName": system.get("objectName"),
            "sourceResolutionStatus": system.get("resolutionStatus"),
            "profileIds": list(system.get("profileIds") or []),
            # Occurrence axis by reference only.
            "actionIds": list(system.get("actionIds") or []),
            "clipNames": list(system.get("clipNames") or []),
            "occurrenceCount": int(system.get("occurrenceCount") or 0),
            "graphRootNodeId": graph.get("rootNodeId"),
            "graphResourceBindings": graph.get("resourceBindings"),
        }
        status = str(system.get("resolutionStatus") or "")
        shape_counts[status] += 1
        if status != "ACTION_BOUND_SOURCE_SYSTEM":
            entry["blockers"] = ["SYSTEM_" + (status or "UNRESOLVED")]
            reasons["SYSTEM_" + (status or "UNRESOLVED")] += 1
        systems.append(entry)

    # ---- coverage against the 31 encounter patterns -------------------------
    coverage: dict[str, Any] = {"patternCount": 0, "patternsWithSystems": 0}
    if ACTION_BINDINGS_PATH.is_file():
        bindings = load_json(ACTION_BINDINGS_PATH)
        by_action: dict[int, list[str]] = defaultdict(list)
        for system in systems:
            for action_id in system["actionIds"]:
                by_action[int(action_id)].append(system["sourceSystemId"])
        rows = []
        for pattern in bindings.get("patterns", []):
            ids = [int(v) for v in (pattern.get("sourceActionIds") or [])]
            covering = sorted({s for a in ids for s in by_action.get(a, [])})
            rows.append({
                "patternId": pattern.get("patternId"),
                "sourceActionIds": ids,
                "sourceSystemCount": len(covering),
            })
        coverage = {
            "patternCount": len(rows),
            "patternsWithSystems": sum(1 for r in rows if r["sourceSystemCount"]),
            "patterns": rows,
        }

    document = {
        "schema": "lostark.valtan-source-material-evidence",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "axisContract": (
            "Occurrences live in Valtan.actionbindings.json and reference these "
            "definitions by stable id. Definitions are stated once."
        ),
        "parentDeclarationCaptureStatus": material_map_status,
        "materials": materials,
        "resources": resources,
        "sourceSystems": systems,
        "patternCoverage": coverage,
    }

    sources = [CATALOG_PATH, ACTION_CATALOG_PATH, ACTION_BINDINGS_PATH]
    receipt = {
        "schema": "lostark.valtan-source-material-evidence-receipt",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "summary": {
            "materialDefinitionCount": len(materials),
            "materialsWithParent": sum(1 for m in materials if m["parentMaterialPath"]),
            "materialsWithParentDeclaration": sum(
                1 for m in materials if m["parentDeclaration"]
            ),
            "distinctParentMaterialCount": len(parent_ids),
            "resourceDefinitionCount": len(resources),
            "resourceRoleCounts": dict(sorted(role_counts.items())),
            "sourceSystemDefinitionCount": len(systems),
            "sourceSystemStatusCounts": dict(sorted(shape_counts.items())),
            "parentDeclarationCaptureStatus": material_map_status,
            "patternCoverage": {
                "patternCount": coverage.get("patternCount", 0),
                "patternsWithSystems": coverage.get("patternsWithSystems", 0),
            },
            "blockerReasonCounts": dict(sorted(reasons.items())),
            "physicalMissingResourceCount": len(physical_missing),
        },
        "sources": [
            {"path": p.relative_to(ROOT).as_posix(), "sha256": sha256_file(p)}
            for p in sources if p.is_file()
        ],
    }
    if MATERIAL_MAP_PATH.is_file():
        receipt["sources"].append({
            "path": MATERIAL_MAP_PATH.as_posix(),
            "sha256": sha256_file(MATERIAL_MAP_PATH),
        })
    return document, receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--write", action="store_true")
    args = parser.parse_args()
    document, receipt = build()
    if args.write:
        OUTPUT_PATH.write_text(
            json.dumps(document, ensure_ascii=False, indent=1) + "\n",
            encoding="utf-8",
        )
        RECEIPT_PATH.write_text(
            json.dumps(receipt, ensure_ascii=False, indent=1) + "\n",
            encoding="utf-8",
        )
    print(f"Valtan source material evidence "
          f"{'written' if args.write else 'dry-run'}: "
          f"{json.dumps(receipt['summary'], ensure_ascii=False)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
