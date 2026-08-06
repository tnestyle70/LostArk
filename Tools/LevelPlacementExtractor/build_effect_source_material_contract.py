#!/usr/bin/env python3
"""Build a loss-aware UE3 source-material contract for one Effect document."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import copy
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def folded(value: Any) -> str:
    return str(value or "").strip().casefold()


def stable_profile_id(parent_path: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", ".", folded(parent_path)).strip(".")
    digest = hashlib.sha256(folded(parent_path).encode("utf-8")).hexdigest()[:12]
    return f"ue3.material.{slug[:72]}.{digest}"


def runtime_shader_profile_id(parent_path: str, source_path: str) -> str:
    identity = folded(parent_path or source_path)
    exact_profiles = {
        "bfx_m.bfx_d_pa_circ_01_ad": "effect.ue3.circle.v1",
        "fx_m.fx_j_pa_dot_ad_01": "effect.ue3.dot.v1",
        "fx_m.fx_d_pa_ring_11_tr": "effect.ue3.ring.v1",
        "fx_m.fx_c_pa_aura_02_tr": "effect.ue3.aura.v1",
        "fx_mm.fx_mm_onelayerdistortion_02_ad": (
            "effect.ue3.one-layer-distortion.v1"
        ),
    }
    if identity in exact_profiles:
        return exact_profiles[identity]
    source_identity = folded(source_path)
    for material_path, profile_id in exact_profiles.items():
        if source_identity.endswith("." + material_path):
            return profile_id
    return "effect.ue3.reconstructed-standard.v1"


def required_runtime_bindings(shader_profile_id: str) -> list[dict[str, str]]:
    bindings = {
        "effect.ue3.ring.v1": [
            {
                "slotId": "base",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_bg_waterspray_01.dds"
                ),
            },
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_02/"
                    "fx_d_noise_009.dds"
                ),
            },
        ],
        "effect.ue3.aura.v1": [
            {
                "slotId": "base",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_a_glow_009.dds"
                ),
            },
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_HIGH_00/"
                    "fx_a_cloud_026.dds"
                ),
            },
        ],
        "effect.ue3.one-layer-distortion.v1": [
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_a_noise_002.dds"
                ),
            },
        ],
    }
    return copy.deepcopy(bindings.get(shader_profile_id, []))


def material_candidates(
    material_map: dict[str, Any], source_path: str
) -> list[dict[str, Any]]:
    materials = material_map.get("materials", {})
    keys = [folded(source_path)]
    parts = source_path.split(".")
    if len(parts) > 1:
        keys.append(folded(".".join(parts[1:])))
    keys.append(folded(parts[-1]))
    matches: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()
    for key in keys:
        for row in materials.get(key, []):
            identity = (
                folded(row.get("source_file")),
                folded(row.get("material_path")),
            )
            if identity in seen:
                continue
            seen.add(identity)
            matches.append(row)
    return matches


def source_asset_packages(manifest: dict[str, Any]) -> dict[str, str]:
    packages: dict[str, str] = {}
    for row in manifest.get("assets", []):
        source_path = folded(row.get("sourceAssetPath"))
        physical = str(row.get("physicalPackage") or "")
        if source_path and physical:
            packages[source_path] = physical
    return packages


def count_status(value: Any, token: str) -> int:
    if isinstance(value, dict):
        return int(folded(value.get("status")) == folded(token)) + sum(
            count_status(child, token) for child in value.values()
        )
    if isinstance(value, list):
        return sum(count_status(child, token) for child in value)
    return 0


def normalize_vector_parameter(row: dict[str, Any]) -> dict[str, Any]:
    value = row.get("value")
    if isinstance(value, dict):
        value = [
            float(value.get("r", 0.0)),
            float(value.get("g", 0.0)),
            float(value.get("b", 0.0)),
            float(value.get("a", 0.0)),
        ]
    if not isinstance(value, list) or len(value) != 4:
        value = [0.0, 0.0, 0.0, 0.0]
    return {"name": str(row.get("name") or ""), "value": value}


def element_subuv_mode(element: dict[str, Any]) -> str:
    modules = (element.get("sourceRecipe") or {}).get("modules", [])
    if not any(folded(row.get("className")) == "particlemodulesubuv"
               for row in modules):
        return "none"
    literals = {
        folded(literal.get("propertyPath")): literal.get("value")
        for module in modules
        if folded(module.get("className")) == "particlemodulerequired"
        for literal in module.get("literals", [])
    }
    allow_flip = bool(literals.get("ballowimageflipping"))
    square_flip = bool(literals.get("bsquareimageflipping"))
    random_time = float(literals.get("randomimagetime") or 0.0)
    return (
        "psuvim_linear_blend_random_flip_square"
        if allow_flip and square_flip and random_time > 0.0
        else "psuvim_linear_blend"
    )


def classify_dynamic_parameter(name: str) -> str:
    value = folded(name)
    if "alpha" in value:
        return "opacity"
    if "dissolve" in value:
        return "dissolve"
    if any(token in value for token in ("emissive", "bright", "power")):
        return "emissive"
    if "noise" in value or "distort" in value:
        return "distortion"
    if "size" in value or "radius" in value:
        return "radial_size"
    if "pan" in value or "uv" in value:
        return "uv_pan"
    return "unbound"


def dynamic_parameter_semantics(element: dict[str, Any]) -> list[str]:
    semantics = ["unbound", "unbound", "unbound", "unbound"]
    modules = (element.get("sourceRecipe") or {}).get("modules", [])
    for module in modules:
        if folded(module.get("className")) != "particlemoduleparameterdynamic":
            continue
        for literal in module.get("literals", []):
            match = re.fullmatch(
                r"dynamicparams\[(\d+)\]\.paramname",
                folded(literal.get("propertyPath")),
            )
            if not match:
                continue
            index = int(match.group(1))
            if 0 <= index < len(semantics):
                semantics[index] = classify_dynamic_parameter(
                    str(literal.get("value") or "")
                )
    return semantics


def upgrade_effect_document(
    effect_document: dict[str, Any], contract: dict[str, Any]
) -> dict[str, Any]:
    identities = {
        folded(row.get("sourceMaterialPath")): row
        for row in contract.get("materialIdentities", [])
    }
    upgraded = copy.deepcopy(effect_document)
    upgraded["version"] = 11
    for element in upgraded.get("elements", []):
        material = element.setdefault("material", {})
        if folded(element.get("kind")) != "particle":
            material["sourceProfile"] = {"enabled": False}
            continue
        source_path = folded(material.get("sourceMaterialPath"))
        identity = identities.get(source_path)
        if identity is None:
            raise ValueError(
                f"Particle {element.get('id')} has no source material contract."
            )
        parameters = identity.get("sourceParameters") or {}
        material["sourceProfile"] = {
            "enabled": True,
            "profileId": identity["profileId"],
            "runtimeShaderProfileId": identity["runtimeShaderProfileId"],
            "parentMaterialPath": identity["parentMaterialPath"],
            "semanticStatus": "reconstructed_profile",
            "scalars": list(parameters.get("scalars") or []),
            "vectors": [
                normalize_vector_parameter(row)
                for row in parameters.get("vectors") or []
            ],
            "staticSwitches": list(parameters.get("staticSwitches") or []),
            "dynamicParameterSemantics": dynamic_parameter_semantics(element),
            "subUVMode": element_subuv_mode(element),
        }
        required = identity.get("requiredRuntimeBindings") or []
        required_slots = {str(row.get("slotId")) for row in required}
        element["resources"] = [
            row for row in element.get("resources", [])
            if str(row.get("slotId")) not in required_slots
        ] + copy.deepcopy(required)
    return upgraded


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staged = path.with_suffix(path.suffix + ".tmp")
    staged.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    staged.replace(path)


def build_contract(
    effect_document: dict[str, Any],
    source_graph: dict[str, Any],
    conversion_receipt: dict[str, Any],
    resource_manifest: dict[str, Any],
    material_map: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    particle_elements = [
        row for row in effect_document.get("elements", [])
        if folded(row.get("kind")) == "particle"
    ]
    if not particle_elements:
        raise ValueError("Effect document has no Particle elements.")

    occurrence_ids_by_material: dict[str, list[str]] = defaultdict(list)
    template_counts_by_material: dict[str, Counter[str]] = defaultdict(Counter)
    resources_by_material: dict[str, dict[str, str]] = defaultdict(dict)
    source_path_spelling: dict[str, str] = {}
    for element in particle_elements:
        material = element.get("material") or {}
        source_path = str(material.get("sourceMaterialPath") or "")
        key = folded(source_path)
        if not key:
            raise ValueError(
                f"Particle {element.get('id')} has no sourceMaterialPath."
            )
        source_path_spelling.setdefault(key, source_path)
        occurrence_ids_by_material[key].append(str(element.get("id") or ""))
        template_counts_by_material[key][
            str(material.get("templateId") or "")
        ] += 1
        for binding in element.get("resources", []):
            slot = str(binding.get("slotId") or "")
            asset = str(binding.get("assetId") or "")
            if slot and asset:
                resources_by_material[key].setdefault(slot, asset)

    graph_rows_by_path = {
        folded(row.get("sourceMaterialPath")): row
        for row in source_graph.get("materialParameterBindings", [])
        if row.get("sourceMaterialPath")
    }
    manifest_packages = source_asset_packages(resource_manifest)

    identities: list[dict[str, Any]] = []
    profile_members: dict[str, list[str]] = defaultdict(list)
    failures: list[dict[str, Any]] = []
    for key in sorted(occurrence_ids_by_material):
        source_path = source_path_spelling[key]
        graph_row = graph_rows_by_path.get(key, {})
        candidates = material_candidates(material_map, source_path)
        expected_package = folded(graph_row.get("sourcePhysicalPackage"))
        exact_package_candidates = [
            row for row in candidates
            if expected_package
            and folded(row.get("source_file")) == expected_package
        ]
        if len(candidates) == 1:
            selected = candidates[0]
        elif len(exact_package_candidates) == 1:
            # Material object paths are not globally unique across cooked UE3
            # packages.  The source receipt already records the package that
            # owned this occurrence, so use that evidence to disambiguate the
            # material map instead of selecting by path order.
            selected = exact_package_candidates[0]
        else:
            selected = graph_row
        parent = str(selected.get("parent") or graph_row.get("parent") or source_path)
        parent_package = manifest_packages.get(folded(parent))
        source_package = str(
            selected.get("source_file")
            or graph_row.get("sourcePhysicalPackage")
            or manifest_packages.get(key)
            or ""
        )
        if len(candidates) > 1 and len(exact_package_candidates) != 1:
            failures.append({
                "sourceMaterialPath": source_path,
                "status": "AMBIGUOUS_MATERIAL_CANDIDATE",
                "candidateCount": len(candidates),
            })
        if not source_package and key.startswith("enginematerials."):
            source_package = "ENGINE_BUILTIN"
            parent_package = parent_package or "ENGINE_BUILTIN"
        if not source_package:
            failures.append({
                "sourceMaterialPath": source_path,
                "status": "MISSING_SOURCE_PACKAGE",
            })
        profile_id = stable_profile_id(parent)
        shader_profile_id = runtime_shader_profile_id(parent, source_path)
        profile_members[profile_id].append(source_path)
        template_counts = template_counts_by_material[key]
        pending_count = template_counts.get("effect.source_material", 0)
        identities.append({
            "sourceMaterialPath": source_path,
            "sourcePhysicalPackage": source_package or None,
            "parentMaterialPath": parent,
            "parentSourcePhysicalPackage": parent_package,
            "profileId": profile_id,
            "runtimeShaderProfileId": shader_profile_id,
            "requiredRuntimeBindings": required_runtime_bindings(
                shader_profile_id
            ),
            "semanticStatus": "RECONSTRUCTED_PROFILE",
            "runtimeOccurrenceCount": len(occurrence_ids_by_material[key]),
            "runtimeOccurrenceElementIds": occurrence_ids_by_material[key],
            "pendingRuntimeOccurrenceCount": pending_count,
            "templateCounts": dict(sorted(template_counts.items())),
            "currentResourceBindings": [
                {"slotId": slot, "assetId": asset}
                for slot, asset in sorted(resources_by_material[key].items())
            ],
            "sourceParameters": {
                "textures": list(selected.get("textures") or graph_row.get("textures") or []),
                "scalars": list(selected.get("scalars") or graph_row.get("scalars") or []),
                "vectors": list(selected.get("vectors") or graph_row.get("vectors") or []),
                "staticSwitches": list(
                    selected.get("static_switches")
                    or selected.get("staticSwitches")
                    or []
                ),
            },
            "renderState": (selected.get("materialEvidence") or {}).get(
                "renderState"
            ),
            "referencedTextures": (selected.get("materialEvidence") or {}).get(
                "referencedTextures", []
            ),
            "expressionCoverage": (selected.get("materialEvidence") or {}).get(
                "expressionCoverage",
                {
                    "entryCount": None,
                    "nonNullCount": None,
                    "nullCount": None,
                    "topologyStatus": "NOT_CAPTURED",
                },
            ),
            "materialResourceDecodeStatus": "NOT_CAPTURED",
        })

    profiles = [
        {
            "profileId": profile_id,
            "parentMaterialPath": next(
                row["parentMaterialPath"]
                for row in identities if row["profileId"] == profile_id
            ),
            "semanticStatus": "RECONSTRUCTED_PROFILE",
            "runtimeShaderProfileId": next(
                row["runtimeShaderProfileId"]
                for row in identities if row["profileId"] == profile_id
            ),
            "materialIdentityCount": len(members),
            "materialIdentities": sorted(members, key=str.casefold),
            "runtimeOccurrenceCount": sum(
                row["runtimeOccurrenceCount"]
                for row in identities if row["profileId"] == profile_id
            ),
        }
        for profile_id, members in sorted(profile_members.items())
    ]

    particle_conversions = [
        row for row in conversion_receipt.get("elementConversions", [])
        if folded(row.get("targetKind")) == "particle"
    ]
    heuristic_count = sum(
        count_status(row.get("resourceMappings", []), "PARAMETER_NAME_HEURISTIC")
        for row in particle_conversions
    )
    pending_occurrence_count = sum(
        row["pendingRuntimeOccurrenceCount"] for row in identities
    )
    summary = {
        "particleElementCount": len(particle_elements),
        "materialIdentityCount": len(identities),
        "parentProfileGroupCount": len(profiles),
        "pendingRuntimeOccurrenceCount": pending_occurrence_count,
        "standardRuntimeOccurrenceCount": (
            len(particle_elements) - pending_occurrence_count
        ),
        "parameterNameHeuristicCount": heuristic_count,
        "failureCount": len(failures),
        "sourceExactIdentityCount": len(identities),
        "runtimeExactProfileCount": 0,
        "reconstructedProfileCount": len(profiles),
    }
    contract = {
        "schema": "lostark.effect-source-material-contract",
        "formatVersion": 1,
        "effectAssetId": effect_document.get("effectAssetId"),
        "summary": summary,
        "materialIdentities": identities,
        "profiles": profiles,
        "failures": failures,
    }
    receipt = {
        "schema": "lostark.effect-source-material-contract-receipt",
        "formatVersion": 1,
        "effectAssetId": effect_document.get("effectAssetId"),
        "summary": summary,
        "failures": failures,
    }
    return contract, receipt


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--effect-document", required=True, type=Path)
    parser.add_argument("--source-graph", required=True, type=Path)
    parser.add_argument("--conversion-receipt", required=True, type=Path)
    parser.add_argument("--resource-manifest", required=True, type=Path)
    parser.add_argument("--material-map", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--output-effect-document", type=Path)
    args = parser.parse_args()

    contract, receipt = build_contract(
        load_json(args.effect_document),
        load_json(args.source_graph),
        load_json(args.conversion_receipt),
        load_json(args.resource_manifest),
        load_json(args.material_map),
    )
    receipt["sources"] = [
        {"path": path.as_posix(), "sha256": sha256_file(path)}
        for path in (
            args.effect_document,
            args.source_graph,
            args.conversion_receipt,
            args.resource_manifest,
            args.material_map,
        )
    ]
    write_json_atomic(args.output, contract)
    write_json_atomic(args.receipt, receipt)
    if args.output_effect_document is not None:
        write_json_atomic(
            args.output_effect_document,
            upgrade_effect_document(
                load_json(args.effect_document), contract
            ),
        )
    print(json.dumps(receipt["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not receipt["failures"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
