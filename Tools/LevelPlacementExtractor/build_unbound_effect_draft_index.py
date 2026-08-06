#!/usr/bin/env python3
"""Map unbound class ParticleSystems to runtime resource/Element candidates."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from build_imported_effect_drafts import (
    add_resource_candidate,
    candidate_kinds,
    material_index,
    runtime_asset_index,
)


def build_index(
    catalog: dict[str, Any], cook_receipt: dict[str, Any]
) -> dict[str, Any]:
    runtime_index = runtime_asset_index(cook_receipt)
    materials = material_index(catalog)
    systems = []
    missing_count = 0
    for source_system in catalog.get("sourceSystems", []):
        graph = source_system.get("graph", {})
        resources: list[dict[str, Any]] = []
        seen: set[tuple[str, str, str]] = set()
        for binding in graph.get("resourceBindings", []):
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
            if not material:
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
        missing_count += sum(
            row["resolutionStatus"] != "RESOLVED_RUNTIME_ASSET"
            for row in resources
        )
        candidate_input = {
            "summary": graph.get("summary", {}),
            "resourceBindings": graph.get("resourceBindings", []),
        }
        systems.append(
            {
                "sourceAsset": source_system["sourceAsset"],
                "logicalPackage": source_system["logicalPackage"],
                "objectName": source_system["objectName"],
                "bindingStatus": "UNBOUND_TO_SKILL_NOTIFY",
                "candidateKinds": candidate_kinds(candidate_input),
                "sourceGraphSummary": graph.get("summary", {}),
                "resourceCandidates": resources,
                "conversionStatus": (
                    "NEEDS_SKILL_NOTIFY_AND_EMITTER_PARTITION_MAPPING"
                ),
            }
        )
    return {
        "schema": "lostark.unbound-effect-document-draft-index",
        "formatVersion": 1,
        "characterClass": catalog.get("characterClass"),
        "runtimeDocumentStatus": "NOT_PUBLISHABLE_UNBOUND_SOURCE_DRAFTS",
        "sourceSystems": systems,
        "unsupportedMaterialBindings": catalog.get(
            "unresolvedMaterialBindings", []
        ),
        "summary": {
            "sourceSystemCount": len(systems),
            "candidateElementPartitionCount": sum(
                len(row["candidateKinds"]) for row in systems
            ),
            "missingOrAmbiguousRuntimeResourceCount": missing_count,
            "skillBoundSourceSystemCount": 0,
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--cook-receipt", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    catalog = json.loads(args.catalog.read_text(encoding="utf-8-sig"))
    cook = json.loads(args.cook_receipt.read_text(encoding="utf-8-sig"))
    document = build_index(catalog, cook)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(document["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
