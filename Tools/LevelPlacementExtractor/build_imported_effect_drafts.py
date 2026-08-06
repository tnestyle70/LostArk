#!/usr/bin/env python3
"""Build reviewable Effect Document drafts from exact skill source graphs."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def suggested_slot(parameter: str | None, role: str) -> str:
    if role == "mesh":
        return "mesh_model"
    folded = str(parameter or "").casefold()
    if "dissolve" in folded:
        return "dissolve"
    if "noise" in folded:
        return "noise"
    if any(token in folded for token in ("mask", "alpha", "opacity")):
        return "mask"
    if any(token in folded for token in ("emiss", "glow", "bloom")):
        return "emissive"
    if any(token in folded for token in ("diffuse", "base", "color")):
        return "base"
    return "unclassified"


def candidate_kinds(system: dict[str, Any]) -> list[str]:
    feature_classes = [
        str(value).casefold()
        for value in system.get("summary", {}).get("featureClasses", [])
    ]
    resource_roles = {
        str(row.get("role", "")).casefold()
        for row in system.get("resourceBindings", [])
    }
    kinds = {"particle"}
    if "mesh" in resource_roles or any("mesh" in name for name in feature_classes):
        kinds.add("mesh")
    if any(token in name for name in feature_classes for token in ("ribbon", "trail")):
        kinds.add("trail")
    if any("decal" in name for name in feature_classes):
        kinds.add("decal")
    return [kind for kind in ("mesh", "sprite", "particle", "decal", "trail") if kind in kinds]


def runtime_asset_index(cook_receipt: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    index: dict[str, list[dict[str, Any]]] = {}
    for row in cook_receipt.get("assets", []):
        source = row.get("sourceAssetPath")
        if source and row.get("runtimeAssetId"):
            index.setdefault(str(source).casefold(), []).append(row)
    return index


def material_index(graph: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {
        str(row.get("sourceMaterialPath", "")).casefold(): row
        for row in graph.get("materialParameterBindings", [])
        if row.get("sourceMaterialPath")
    }


def add_resource_candidate(
    output: list[dict[str, Any]],
    seen: set[tuple[str, str, str]],
    runtime_index: dict[str, list[dict[str, Any]]],
    source_path: str,
    role: str,
    parameter: str | None,
) -> None:
    matches = runtime_index.get(source_path.casefold(), [])
    runtime_ids = sorted(
        {str(row["runtimeAssetId"]) for row in matches}, key=str.casefold
    )
    status = (
        "RESOLVED_RUNTIME_ASSET" if len(runtime_ids) == 1
        else "AMBIGUOUS_RUNTIME_ASSET" if runtime_ids
        else "MISSING_RUNTIME_ASSET"
    )
    key = (source_path.casefold(), role.casefold(), str(parameter).casefold())
    if key in seen:
        return
    seen.add(key)
    output.append(
        {
            "sourceObjectPath": source_path,
            "sourceRole": role,
            "parameterName": parameter,
            "suggestedSlot": suggested_slot(parameter, role),
            "slotMappingStatus": (
                "SOURCE_PARAMETER_HEURISTIC_NEEDS_REVIEW"
                if role == "texture" else "SOURCE_ROLE_EXACT"
            ),
            "runtimeAssetId": runtime_ids[0] if len(runtime_ids) == 1 else None,
            "runtimeCandidateAssetIds": runtime_ids,
            "resolutionStatus": status,
        }
    )


def build_draft(
    source_receipt: dict[str, Any],
    graph: dict[str, Any],
    cook_receipt: dict[str, Any],
) -> dict[str, Any]:
    if int(source_receipt["skillId"]) != int(graph["skillId"]):
        raise ValueError("source receipt and graph skillId differ")
    runtime_index = runtime_asset_index(cook_receipt)
    materials = material_index(graph)
    systems = []
    missing_runtime_count = 0
    for source_system in graph.get("sourceSystems", []):
        resources: list[dict[str, Any]] = []
        seen: set[tuple[str, str, str]] = set()
        for binding in source_system.get("resourceBindings", []):
            source_path = binding.get("objectPath")
            role = str(binding.get("role", "unknown"))
            if not source_path:
                continue
            if role in {"mesh", "texture"}:
                add_resource_candidate(
                    resources,
                    seen,
                    runtime_index,
                    str(source_path),
                    role,
                    str(binding.get("property") or "") or None,
                )
            if role != "material":
                continue
            material = materials.get(str(source_path).casefold())
            if material is None:
                continue
            for texture in material.get("textures", []):
                texture_path = texture.get("texture")
                if texture_path:
                    add_resource_candidate(
                        resources,
                        seen,
                        runtime_index,
                        str(texture_path),
                        "texture",
                        str(texture.get("name") or "") or None,
                    )
        missing_runtime_count += sum(
            row["resolutionStatus"] != "RESOLVED_RUNTIME_ASSET"
            for row in resources
        )
        systems.append(
            {
                "sourceSystemId": source_system["sourceSystemId"],
                "sourceAsset": source_system["sourceAsset"],
                "logicalPackage": source_system["logicalPackage"],
                "candidateKinds": candidate_kinds(source_system),
                "sourceGraphSummary": source_system.get("summary", {}),
                "resourceCandidates": resources,
                "conversionStatus": "NEEDS_EMITTER_PARTITION_AND_PARAMETER_MAPPING",
            }
        )

    timeline = source_receipt.get("timeline", {})
    events = timeline.get("events", [])
    return {
        "schema": "lostark.imported-effect-document-draft",
        "formatVersion": 1,
        "characterClass": source_receipt["characterClass"],
        "skillId": source_receipt["skillId"],
        "inputSlot": source_receipt.get("inputSlot"),
        "effectAssetIdCandidate": (
            f"effect.{str(source_receipt['characterClass']).lower()}."
            f"skill.{source_receipt['skillId']}"
        ),
        "runtimeDocumentStatus": "NOT_PUBLISHABLE_SOURCE_CONVERSION_DRAFT",
        "animationVisualContract": source_receipt.get(
            "animationVisualContract", {}
        ),
        "timeline": {
            "durationSeconds": timeline.get("durationSeconds", 0.0),
            "clips": timeline.get("clips", []),
            "events": events,
        },
        "sourceSystems": systems,
        "unsupportedOrUnresolved": source_receipt.get(
            "unsupportedUnresolved", []
        ),
        "summary": {
            "sourceSystemCount": len(systems),
            "sourceEventCount": len(events),
            "resolvedParticleEventCount": sum(
                row.get("resolutionStatus") == "RESOLVED_PARTICLE_GRAPH"
                for row in events
            ),
            "unsupportedSourceEventCount": sum(
                row.get("resolutionStatus") == "UNSUPPORTED_SOURCE_NOTIFY"
                for row in events
            ),
            "candidateElementPartitionCount": sum(
                len(row["candidateKinds"]) for row in systems
            ),
            "missingOrAmbiguousRuntimeResourceCount": missing_runtime_count,
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--imported-root", required=True, type=Path)
    parser.add_argument("--graphs-root", required=True, type=Path)
    parser.add_argument("--cook-receipt", required=True, type=Path)
    parser.add_argument("--output-root", required=True, type=Path)
    args = parser.parse_args()
    cook_receipt = json.loads(args.cook_receipt.read_text(encoding="utf-8-sig"))
    source_receipts = sorted(args.imported_root.glob("skill.*.source-receipt.json"))
    if not source_receipts:
        raise ValueError(f"no skill source receipts below {args.imported_root}")
    args.output_root.mkdir(parents=True, exist_ok=True)
    summaries = []
    for source_path in source_receipts:
        source_receipt = json.loads(source_path.read_text(encoding="utf-8-sig"))
        skill_id = int(source_receipt["skillId"])
        graph_matches = list(
            args.graphs_root.rglob(f"skill.{skill_id}.normalized-effect-graph.json")
        )
        if len(graph_matches) != 1:
            raise ValueError(
                f"skill {skill_id} normalized graph count is {len(graph_matches)}"
            )
        graph = json.loads(graph_matches[0].read_text(encoding="utf-8-sig"))
        draft = build_draft(source_receipt, graph, cook_receipt)
        output = args.output_root / f"skill.{skill_id}.imported-effect-draft.json"
        output.write_text(
            json.dumps(draft, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        summaries.append({"skillId": skill_id, **draft["summary"]})
    print(
        json.dumps(
            {
                "draftCount": len(summaries),
                "missingOrAmbiguousRuntimeResourceCount": sum(
                    row["missingOrAmbiguousRuntimeResourceCount"]
                    for row in summaries
                ),
            },
            ensure_ascii=False,
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
