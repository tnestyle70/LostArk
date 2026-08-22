#!/usr/bin/env python3
"""Build the fail-closed family inventory for Valtan FRONT_BACK_FRONT.

The report is deliberately not a Product materializer.  It joins the exact
Cascade carrier, child/parent material evidence, current Product cue timing,
and the finite typed-family witnesses without pretending that a blend mode is
a material equation.  Every row therefore records the remaining source
adapter and material-admission boundary before an authored document can be
changed.
"""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
SOURCE_OCCURRENCE_INVENTORY = (
    ROOT
    / "Data/Effects/Imported/Valtan/Valtan.source-occurrence-inventory.v1.json"
)
SOURCE_RESOURCE_CATALOG = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
SOURCE_MATERIAL_EVIDENCE = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json"
)
VALTAN_CUES = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
MATERIAL_TEMPLATE = ROOT / "Client/Public/Effect_MaterialTemplate.h"
DOCUMENT_RENDERER = ROOT / "Client/Private/Effect_DocumentRenderer.cpp"
FAMILY_SHADER = (
    ROOT / "Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli"
)
OUTPUT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/"
    / "Valtan.front-back-front-family-inventory.v1.json"
)

SCHEMA = "lostark.valtan-front-back-front-family-inventory"
FORMAT_VERSION = 1
PATTERN_ID = "VALTAN_FRONT_BACK_FRONT"
STAGE_ID = "SMASHES"
ACTION_ID = "valtan.attack.front-back-front.active"
CLIP_OCCURRENCE_ID = "valtan.attack.front-back-front.active.clip.01"
SOURCE_SYSTEM_ID = "fx_mn_rpbf_00_n.par_n_rpbf_atk_01_02"
EXPECTED_CARRIER_COUNT = 20
EXPECTED_SHAPE_COUNTS = {"sprite": 15, "mesh": 3, "decal": 2}
EXPECTED_SOURCE_WAVE_STARTS_MS = [1169, 2253, 3224, 4220]
EXPECTED_SOURCE_WAVE_ELEMENT_COUNT = 100
EXPECTED_EXTERNAL_MODULE_REFERENCE_COUNT = 149
EXPECTED_EXTERNAL_MODULE_PACKAGE_COUNT = 31

SOURCE_WAVE_DOCUMENTS = (
    PurePosixPath(
        "Data/Effects/Authored/"
        "effect.valtan.front-back-front.source-wave-01.effect.json"
    ),
    PurePosixPath(
        "Data/Effects/Authored/"
        "effect.valtan.front-back-front.source-wave-02.effect.json"
    ),
    PurePosixPath(
        "Data/Effects/Authored/"
        "effect.valtan.front-back-front.source-wave-03.effect.json"
    ),
    PurePosixPath(
        "Data/Effects/Authored/"
        "effect.valtan.front-back-front.auxiliary-source-wave.effect.json"
    ),
)
AGGREGATE_DOCUMENT = PurePosixPath(
    "Data/Effects/Authored/effect.valtan.front-back-front.active.effect.json"
)

# These are only finite evaluator witnesses.  A row remains blocked until its
# exact child/parent/static-set/carrier/pass/packet admission is proven.  In
# particular, SpriteWave and MakeFlow require a v2 UV-phase contract because
# the current bounded evaluators conflate absolute curve offsets and rates.
FAMILY_WITNESSES = {
    "bfx_m_mi_00.bfx_m.bfx_d_pa_spla_05_tr": {
        "family": "SPLA05",
        "disposition": "EXISTING_BOUNDED_EVALUATOR_REVIEW_REQUIRED",
        "codeToken": "EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA05",
    },
    "fx_m.fx_k_me_makeflow_03_tr": {
        "family": "MAKEFLOW_03_V2",
        "disposition": "UV_PHASE_V2_EVALUATOR_REQUIRED",
        "codeToken": "EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03",
    },
    "fx_m.fx_f_pa_shine_01_0_tr": {
        "family": "SHINE",
        "disposition": "EXISTING_BOUNDED_EVALUATOR_REVIEW_REQUIRED",
        "codeToken": "effect.ue3.shine.v1",
    },
    "fx_m.fx_m_pa_spritewave_01_tr": {
        "family": "SPRITEWAVE_01_V2",
        "disposition": "UV_PHASE_V2_EVALUATOR_REQUIRED",
        "codeToken": "EFFECT_STRICT_TYPED_SOURCE_PROFILE::SPRITEWAVE_01",
    },
    "fx_mastermaterial.fx_mm.fx_mm_simple_01_ad": {
        "family": "SIMPLE01",
        "disposition": "EXISTING_BOUNDED_EVALUATOR_REVIEW_REQUIRED",
        "codeToken": "EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE01",
    },
    "fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr": {
        "family": "WATERTRAIL",
        "disposition": "EXISTING_BOUNDED_EVALUATOR_REVIEW_REQUIRED",
        "codeToken": "EFFECT_STRICT_TYPED_SOURCE_PROFILE::WATERTRAIL",
    },
}

NEW_FAMILY_BY_CHILD = {
    "fx_m_mi_n_00.fx_n_me_dissolve_04_011_ma": {
        "family": "MASKED_DISSOLVE_STONE",
        "disposition": "NEW_MASKED_MESH_EVALUATOR_REQUIRED",
    },
    "fx_m_mi_n_00.fx_mi.fx_n_de_ground_04_30_tr": {
        "family": "GROUND_DECAL_04",
        "disposition": "NEW_TYPED_LOCAL_DECAL_EVALUATOR_REQUIRED",
    },
    "fx_m_mi_04.fx_mi.fx_d_de_unlit_01_02_tr": {
        "family": "UNLIT_LOCAL_DECAL",
        "disposition": "FAIL_CLOSE_PARENT_MATERIAL_UNDECLARED",
    },
}


class InventoryError(RuntimeError):
    """The exact source/Product join no longer satisfies the sealed slice."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise InventoryError(message)


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise InventoryError(f"cannot read JSON: {path}: {error}") from error
    require(isinstance(value, dict), f"JSON root is not an object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def pretty_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_file(path: Path) -> str:
    try:
        return sha256_bytes(path.read_bytes())
    except OSError as error:
        raise InventoryError(f"cannot hash input: {path}: {error}") from error


def folded(value: Any) -> str:
    return str(value or "").strip().casefold()


def repository_path(relative: PurePosixPath) -> Path:
    path = ROOT.joinpath(*relative.parts)
    try:
        path.resolve(strict=False).relative_to(ROOT.resolve())
    except ValueError as error:
        raise InventoryError(f"path escaped repository: {relative}") from error
    return path


def resource_bindings_for_system(
    catalog: dict[str, Any], system_id: str
) -> dict[str, list[dict[str, Any]]]:
    matches = [
        row
        for row in catalog.get("sourceSystems", [])
        if folded(row.get("sourceAsset")) == folded(system_id)
    ]
    require(len(matches) == 1, f"expected one source catalog system: {system_id}")
    result: dict[str, list[dict[str, Any]]] = {}
    for row in (matches[0].get("graph") or {}).get("resourceBindings", []):
        source_node_id = str(row.get("sourceNodeId") or "")
        require(source_node_id, "source resource binding has no node ID")
        result.setdefault(source_node_id, []).append(row)
    return result


def external_module_dependencies_for_system(
    catalog: dict[str, Any], system_id: str
) -> dict[str, list[dict[str, Any]]]:
    """Index unresolved external LOD modules without inventing defaults.

    Cascade can borrow Lifetime, Spawn, DynamicParameter, and other modules
    from a different package.  The local graph therefore looks incomplete
    until those exact packages are loaded.  Treating the missing rows as
    generic defaults would change both simulation and the material packet, so
    this inventory records the dependency closure as a first-class blocker.
    """

    matches = [
        row
        for row in catalog.get("sourceSystems", [])
        if folded(row.get("sourceAsset")) == folded(system_id)
    ]
    require(len(matches) == 1, f"expected one source catalog system: {system_id}")
    result: dict[str, list[dict[str, Any]]] = {}
    seen: set[tuple[str, int, str, str]] = set()
    for row in (matches[0].get("graph") or {}).get(
        "unresolvedExternalReferences", []
    ):
        if row.get("reason") != "external_graph_package_not_loaded":
            continue
        object_path = str(row.get("objectPath") or "").strip()
        if not object_path:
            continue
        property_name = folded(row.get("property"))
        require(
            property_name in {"modules", "spawnmodule"},
            "unexpected external particle-module property: "
            f"{row.get('property')}",
        )
        source_node_id = str(row.get("sourceNodeId") or "")
        require(source_node_id, "external module dependency has no LOD node ID")
        package_name, separator, _ = object_path.partition(".")
        require(separator == ".", f"external module path has no package: {object_path}")
        reference_index = int(row.get("referenceIndex", -1))
        require(reference_index >= 0, f"invalid external reference index: {row}")
        identity = (
            folded(source_node_id),
            reference_index,
            property_name,
            folded(object_path),
        )
        require(identity not in seen, f"duplicate external module reference: {identity}")
        seen.add(identity)
        leaf_name = object_path.rsplit(".", 1)[-1]
        module_class = re.sub(r"_\d+$", "", leaf_name.casefold())
        result.setdefault(folded(source_node_id), []).append(
            {
                "referenceIndex": reference_index,
                "property": property_name,
                "objectPath": object_path,
                "packageName": package_name.casefold(),
                "moduleClassCandidate": module_class,
                "reason": "EXTERNAL_GRAPH_PACKAGE_NOT_LOADED",
            }
        )
    for rows in result.values():
        rows.sort(
            key=lambda row: (
                row["referenceIndex"],
                row["property"],
                folded(row["objectPath"]),
            )
        )
    return result


def material_evidence_index(evidence: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in evidence.get("materials", []):
        source_path = folded(row.get("sourceMaterialPath"))
        require(source_path, "source material evidence has no child identity")
        require(source_path not in result, f"duplicate material evidence: {source_path}")
        result[source_path] = row
    return result


def binding_paths(
    carrier: dict[str, Any],
    bindings_by_node: dict[str, list[dict[str, Any]]],
    role: str,
) -> list[str]:
    node_ids = {
        str(row.get("sourceNodeId") or "")
        for row in carrier.get("orderedModuleOccurrences", [])
    }
    paths = [
        str(binding.get("objectPath") or "")
        for node_id in node_ids
        for binding in bindings_by_node.get(node_id, [])
        if folded(binding.get("role")) == role and binding.get("objectPath")
    ]
    return sorted(set(paths), key=str.casefold)


def normalized_parent_candidates(parent_path: str) -> list[str]:
    """Return evidence identity plus safe package-qualified equivalences.

    The source extractor can spell a parent as ``fx_m.object`` while the
    finite evaluator records ``fx_m_mi_x_00.fx_m.object``.  The short identity
    is not silently rewritten; this list makes the normalization review
    explicit in the generated report.
    """

    parent = folded(parent_path)
    candidates = [parent] if parent else []
    for witness_parent in FAMILY_WITNESSES:
        if (
            parent
            and witness_parent != parent
            and witness_parent.endswith("." + parent)
        ):
            candidates.append(witness_parent)
    return candidates


def family_candidate(
    child_path: str, parent_path: str, renderer_shape: str
) -> dict[str, Any]:
    child = folded(child_path)
    parent_candidates = normalized_parent_candidates(parent_path)
    direct = NEW_FAMILY_BY_CHILD.get(child)
    if direct is not None:
        return {
            **direct,
            "matchBasis": "EXACT_CHILD_MATERIAL",
            "matchedParentIdentity": folded(parent_path) or None,
        }
    for candidate in parent_candidates:
        witness = FAMILY_WITNESSES.get(candidate)
        if witness is not None:
            return {
                "family": witness["family"],
                "disposition": witness["disposition"],
                "matchBasis": (
                    "EXACT_PARENT_MATERIAL"
                    if candidate == folded(parent_path)
                    else "PACKAGE_QUALIFIED_PARENT_EQUIVALENCE_REVIEW"
                ),
                "matchedParentIdentity": candidate,
                "codeToken": witness["codeToken"],
            }
    if renderer_shape == "decal":
        return {
            "family": "UNRESOLVED_LOCAL_DECAL",
            "disposition": "FAIL_CLOSE_LOCAL_DECAL_VARIANT_UNRESOLVED",
            "matchBasis": "NO_TYPED_DECAL_WITNESS",
            "matchedParentIdentity": folded(parent_path) or None,
        }
    return {
        "family": "UNRESOLVED_MATERIAL_VARIANT",
        "disposition": "NEW_EVALUATOR_OR_EXACT_REUSE_PROOF_REQUIRED",
        "matchBasis": "NO_FINITE_EVALUATOR_WITNESS",
        "matchedParentIdentity": folded(parent_path) or None,
    }


def source_profile_enabled(element: dict[str, Any]) -> bool:
    return (element.get("material") or {}).get("sourceProfile", {}).get(
        "enabled"
    ) is True


def execution_enabled(element: dict[str, Any]) -> bool:
    return (element.get("material") or {}).get("execution", {}).get(
        "enabled"
    ) is True


def authored_document_summary(relative: PurePosixPath) -> dict[str, Any]:
    path = repository_path(relative)
    document = load_json(path)
    elements = document.get("elements")
    require(isinstance(elements, list), f"authored elements missing: {relative}")
    return {
        "path": relative.as_posix(),
        "sha256": sha256_file(path),
        "effectAssetId": str(document.get("effectAssetId") or ""),
        "elementCount": len(elements),
        "sourceProfileEnabledCount": sum(source_profile_enabled(row) for row in elements),
        "runtimeMaterialExecutionEnabledCount": sum(
            execution_enabled(row) for row in elements
        ),
        "kindCounts": dict(sorted(Counter(str(row.get("kind") or "") for row in elements).items())),
    }


def selected_source_system(inventory: dict[str, Any]) -> dict[str, Any]:
    matches = [
        row
        for row in inventory.get("sourceSystems", [])
        if folded(row.get("sourceSystemId")) == SOURCE_SYSTEM_ID
    ]
    require(len(matches) == 1, "Atk_01_02 source system is not singular")
    carriers = matches[0].get("carriers")
    require(isinstance(carriers, list), "Atk_01_02 carrier rows are missing")
    require(len(carriers) == EXPECTED_CARRIER_COUNT, "Atk_01_02 carrier count drifted")
    shape_counts = Counter(str(row.get("rendererShape") or "") for row in carriers)
    require(
        dict(shape_counts) == EXPECTED_SHAPE_COUNTS,
        f"Atk_01_02 carrier shape denominator drifted: {dict(shape_counts)}",
    )
    return matches[0]


def impact_occurrences(inventory: dict[str, Any]) -> list[dict[str, Any]]:
    rows = [
        row
        for row in inventory.get("occurrences", [])
        if row.get("patternId") == PATTERN_ID
        and row.get("gameplayActionId") == ACTION_ID
        and row.get("clipOccurrenceId") == CLIP_OCCURRENCE_ID
        and folded(row.get("sourceSystemId")) == SOURCE_SYSTEM_ID
        and row.get("timingDisposition") == "REACHABLE"
    ]
    rows.sort(key=lambda row: float(row.get("sourceTimeSeconds", 0.0)))
    require(len(rows) == 4, f"Atk_01_02 reachable occurrence count drifted: {len(rows)}")
    return [
        {
            "occurrenceId": str(row.get("occurrenceId") or ""),
            "fullKey": str(row.get("fullKey") or ""),
            "notifyId": str(row.get("notifyId") or ""),
            "sourceTimeSeconds": float(row["sourceTimeSeconds"]),
            "sourceDurationSeconds": float(row.get("sourceDurationSeconds", 0.0)),
            "expandedCarrierCount": int(row.get("expandedCarrierCount", 0)),
            "currentDisposition": str(row.get("disposition") or ""),
        }
        for row in rows
    ]


def source_wave_cues(cues: dict[str, Any]) -> list[dict[str, Any]]:
    effect_ids = {
        "effect.valtan.front-back-front.source-wave-01",
        "effect.valtan.front-back-front.source-wave-02",
        "effect.valtan.front-back-front.source-wave-03",
        "effect.valtan.front-back-front.auxiliary-source-wave",
    }
    rows = [
        row
        for row in cues.get("cues", [])
        if row.get("patternId") == PATTERN_ID
        and row.get("clipOccurrenceId") == CLIP_OCCURRENCE_ID
        and row.get("effectAssetId") in effect_ids
    ]
    rows.sort(key=lambda row: int(row["sourceStartMs"]))
    starts = [int(row["sourceStartMs"]) for row in rows]
    require(starts == EXPECTED_SOURCE_WAVE_STARTS_MS, f"source wave cue timing drifted: {starts}")
    return [
        {
            "occurrenceId": str(row.get("occurrenceId") or ""),
            "effectAssetId": str(row.get("effectAssetId") or ""),
            "sourceStartMs": int(row["sourceStartMs"]),
            "presentationRole": (
                "AUXILIARY_SOURCE_WAVE"
                if row.get("effectAssetId")
                == "effect.valtan.front-back-front.auxiliary-source-wave"
                else "PRIMARY_SOURCE_WAVE"
            ),
        }
        for row in rows
    ]


def verify_code_witnesses() -> list[dict[str, Any]]:
    paths = (MATERIAL_TEMPLATE, DOCUMENT_RENDERER, FAMILY_SHADER)
    text_by_path = {
        path: path.read_text(encoding="utf-8-sig", errors="strict") for path in paths
    }
    witnesses = []
    for parent, witness in sorted(FAMILY_WITNESSES.items()):
        token = witness["codeToken"]
        matches = [path for path, text in text_by_path.items() if token in text]
        require(matches, f"finite evaluator witness disappeared: {token}")
        witnesses.append(
            {
                "family": witness["family"],
                "parentMaterialIdentity": parent,
                "disposition": witness["disposition"],
                "codeToken": token,
                "witnessPaths": [
                    path.relative_to(ROOT).as_posix() for path in matches
                ],
            }
        )
    return witnesses


def carrier_rows(
    source_system: dict[str, Any],
    bindings_by_node: dict[str, list[dict[str, Any]]],
    external_dependencies_by_lod: dict[str, list[dict[str, Any]]],
    evidence_by_material: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    result = []
    for ordinal, carrier in enumerate(source_system["carriers"]):
        material_paths = binding_paths(carrier, bindings_by_node, "material")
        mesh_paths = binding_paths(carrier, bindings_by_node, "mesh")
        require(
            len(material_paths) == 1,
            f"carrier {ordinal} material binding is not singular: {material_paths}",
        )
        renderer_shape = str(carrier.get("rendererShape") or "")
        require(
            (renderer_shape == "mesh" and len(mesh_paths) == 1)
            or (renderer_shape != "mesh" and not mesh_paths),
            f"carrier {ordinal} mesh binding does not match shape: {mesh_paths}",
        )
        child_path = folded(material_paths[0])
        evidence = evidence_by_material.get(child_path)
        require(evidence is not None, f"carrier {ordinal} has no material evidence: {child_path}")
        parent_path = str(evidence.get("parentMaterialPath") or "")
        parent_declaration = evidence.get("parentDeclaration") or {}
        static_switches = parent_declaration.get("collectedStaticSwitchParameters") or []
        module_counts = Counter(
            str(row.get("className") or "")
            for row in carrier.get("orderedModuleOccurrences", [])
        )
        missing_canonical_modules = []
        if module_counts.get("particlemodulerequired", 0) != 1:
            missing_canonical_modules.append("particlemodulerequired")
        if module_counts.get("particlemodulelifetime", 0) == 0:
            missing_canonical_modules.append("particlemodulelifetime")
        if module_counts.get("particlemodulespawn", 0) != 1:
            missing_canonical_modules.append("particlemodulespawn")
        if renderer_shape == "mesh" and module_counts.get(
            "particlemoduletypedatamesh", 0
        ) != 1:
            missing_canonical_modules.append("particlemoduletypedatamesh")
        family = family_candidate(child_path, parent_path, renderer_shape)
        selected_lod_node_id = str(carrier.get("selectedLodNodeId") or "")
        require(selected_lod_node_id, f"carrier {ordinal} has no selected LOD node ID")
        external_dependencies = external_dependencies_by_lod.get(
            folded(selected_lod_node_id), []
        )
        require(
            external_dependencies,
            f"carrier {ordinal} unexpectedly has no external module dependency",
        )
        dependency_package_counts = Counter(
            row["packageName"] for row in external_dependencies
        )
        render_state = parent_declaration.get("renderState") or {}
        variant_key = {
            "childMaterialPath": child_path,
            "parentMaterialPath": folded(parent_path) or None,
            "parentStaticSwitchDeclarationSha256": sha256_bytes(
                canonical_bytes(static_switches)
            ),
            "staticSetEvidenceStatus": "PARENT_DECLARATION_ONLY_NOT_EFFECTIVE_STATIC_SET",
            "rendererShape": renderer_shape,
            "meshObjectPath": folded(mesh_paths[0]) if mesh_paths else None,
            "parentRenderState": render_state,
        }
        result.append(
            {
                "sourceOrder": int(carrier.get("sourceOrder", ordinal)),
                "carrierKey": str(carrier.get("carrierKey") or ""),
                "sourceEmitterPath": str(carrier.get("sourceEmitterPath") or ""),
                "selectedLodNodeId": selected_lod_node_id,
                "rendererShape": renderer_shape,
                "kind": str(carrier.get("kind") or ""),
                "childMaterialPath": child_path,
                "parentMaterialPath": folded(parent_path) or None,
                "meshObjectPath": folded(mesh_paths[0]) if mesh_paths else None,
                "moduleClassCounts": dict(sorted(module_counts.items())),
                "missingCanonicalPortableModules": missing_canonical_modules,
                "externalModuleClosure": {
                    "status": "BLOCKED_EXTERNAL_GRAPH_PACKAGES_NOT_LOADED",
                    "referenceCount": len(external_dependencies),
                    "packageCount": len(dependency_package_counts),
                    "packageCounts": dict(sorted(dependency_package_counts.items())),
                    "dependencies": external_dependencies,
                },
                "currentSourceAdapterDisposition": str(
                    carrier.get("disposition") or ""
                ),
                "currentSourceAdapterBlockers": list(
                    carrier.get("conversionBlockers") or []
                ),
                "materialEvidence": {
                    "packageResolutionStatus": str(
                        evidence.get("packageResolutionStatus") or ""
                    ),
                    "blockers": list(evidence.get("blockers") or []),
                    "instanceTextureCount": len(evidence.get("instanceTextures") or []),
                    "instanceScalarCount": len(evidence.get("instanceScalars") or []),
                    "instanceVectorCount": len(evidence.get("instanceVectors") or []),
                    "parentExpressionCoverage": (
                        parent_declaration.get("expressionCoverage") or {}
                    ),
                },
                "familyCandidate": family,
                "provisionalVariantKey": variant_key,
                "provisionalVariantKeySha256": sha256_bytes(
                    canonical_bytes(variant_key)
                ),
                "productAdmission": "BLOCKED_PENDING_SOURCE_AND_FAMILY_CLOSURE",
            }
        )
    require(
        [row["sourceOrder"] for row in result] == list(range(EXPECTED_CARRIER_COUNT)),
        "Atk_01_02 source order is not contiguous",
    )
    return result


def build_inventory(
    *,
    source_inventory: dict[str, Any] | None = None,
    source_catalog: dict[str, Any] | None = None,
    material_evidence: dict[str, Any] | None = None,
    cues: dict[str, Any] | None = None,
) -> dict[str, Any]:
    source_inventory = source_inventory or load_json(SOURCE_OCCURRENCE_INVENTORY)
    source_catalog = source_catalog or load_json(SOURCE_RESOURCE_CATALOG)
    material_evidence = material_evidence or load_json(SOURCE_MATERIAL_EVIDENCE)
    cues = cues or load_json(VALTAN_CUES)
    require(
        source_inventory.get("schema") == "lostark.valtan-source-occurrence-inventory",
        "source occurrence inventory schema changed",
    )
    require(
        source_catalog.get("schema")
        == "lostark.unbound-class-particle-resource-catalog",
        "source resource catalog schema changed",
    )
    require(
        material_evidence.get("schema") == "lostark.valtan-source-material-evidence",
        "source material evidence schema changed",
    )
    require(
        cues.get("schema") == "lostark.valtan-pattern-effect-cues",
        "Valtan cue schema changed",
    )

    system = selected_source_system(source_inventory)
    bindings = resource_bindings_for_system(source_catalog, SOURCE_SYSTEM_ID)
    external_dependencies = external_module_dependencies_for_system(
        source_catalog, SOURCE_SYSTEM_ID
    )
    evidence_index = material_evidence_index(material_evidence)
    carriers = carrier_rows(system, bindings, external_dependencies, evidence_index)
    wave_documents = [authored_document_summary(path) for path in SOURCE_WAVE_DOCUMENTS]
    require(
        sum(row["elementCount"] for row in wave_documents)
        == EXPECTED_SOURCE_WAVE_ELEMENT_COUNT,
        "source wave authored denominator drifted",
    )
    require(
        all(row["sourceProfileEnabledCount"] == 0 for row in wave_documents),
        "source wave material admission changed before family transaction",
    )
    aggregate = authored_document_summary(AGGREGATE_DOCUMENT)
    impact_rows = impact_occurrences(source_inventory)
    wave_cue_rows = source_wave_cues(cues)
    code_witnesses = verify_code_witnesses()

    input_paths = (
        SOURCE_OCCURRENCE_INVENTORY,
        SOURCE_RESOURCE_CATALOG,
        SOURCE_MATERIAL_EVIDENCE,
        VALTAN_CUES,
        MATERIAL_TEMPLATE,
        DOCUMENT_RENDERER,
        FAMILY_SHADER,
        *(repository_path(path) for path in SOURCE_WAVE_DOCUMENTS),
        repository_path(AGGREGATE_DOCUMENT),
    )
    family_counts = Counter(row["familyCandidate"]["family"] for row in carriers)
    external_module_package_counts = Counter(
        dependency["packageName"]
        for row in carriers
        for dependency in row["externalModuleClosure"]["dependencies"]
    )
    external_module_reference_count = sum(
        row["externalModuleClosure"]["referenceCount"] for row in carriers
    )
    require(
        external_module_reference_count == EXPECTED_EXTERNAL_MODULE_REFERENCE_COUNT,
        "Atk_01_02 external module reference denominator drifted: "
        f"{external_module_reference_count}",
    )
    require(
        len(external_module_package_counts) == EXPECTED_EXTERNAL_MODULE_PACKAGE_COUNT,
        "Atk_01_02 external module package denominator drifted: "
        f"{len(external_module_package_counts)}",
    )
    return {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "patternId": PATTERN_ID,
        "semanticStageId": STAGE_ID,
        "gameplayActionId": ACTION_ID,
        "clipOccurrenceId": CLIP_OCCURRENCE_ID,
        "policy": {
            "variantKey": (
                "child+parent+effectiveStaticSet+carrierVF+pass+pixelEquation+namedABI"
            ),
            "blendModeAloneIsFamily": False,
            "rawDxbcProductAdmission": "FORBIDDEN_UNTIL_FULL_ABI_CLOSURE",
            "unknownFamilyDisposition": "FAIL_CLOSE",
            "serverGameplayMutation": "FORBIDDEN",
            "aggregateRetirement": "ATOMIC_WITH_EIGHT_CUE_PRODUCT_REPLACEMENT",
        },
        "sourceSystem": {
            "sourceSystemId": SOURCE_SYSTEM_ID,
            "rootEmitterDenominator": int(system["rootEmitterDenominator"]),
            "carrierCount": len(carriers),
            "carrierShapeCounts": EXPECTED_SHAPE_COUNTS,
            "currentDispositionCounts": dict(
                sorted(
                    Counter(
                        row["currentSourceAdapterDisposition"] for row in carriers
                    ).items()
                )
            ),
            "uniqueChildMaterialCount": len(
                {row["childMaterialPath"] for row in carriers}
            ),
            "familyCandidateCounts": dict(sorted(family_counts.items())),
            "externalModuleClosure": {
                "status": "BLOCKED_EXTERNAL_GRAPH_PACKAGES_NOT_LOADED",
                "referenceCount": external_module_reference_count,
                "packageCount": len(external_module_package_counts),
                "packageCounts": dict(sorted(external_module_package_counts.items())),
                "syntheticDefaultPolicy": "FORBIDDEN",
            },
            "carriers": carriers,
        },
        "timing": {
            "existingSourceWaveCues": wave_cue_rows,
            "unprojectedImpactOccurrences": impact_rows,
            "futureOrderedCueCount": 8,
            "fourthBeatGameplayDisposition": "AUXILIARY_PRESENTATION_ONLY",
        },
        "currentProduct": {
            "aggregate": aggregate,
            "sourceWaveDocuments": wave_documents,
            "sourceWaveElementCount": sum(
                row["elementCount"] for row in wave_documents
            ),
            "sourceWaveSourceProfileEnabledCount": sum(
                row["sourceProfileEnabledCount"] for row in wave_documents
            ),
            "materialFidelityDisposition": (
                "DRAW_SUBMISSION_PROVEN_MATERIAL_EQUATION_NOT_ADMITTED"
            ),
        },
        "finiteEvaluatorWitnesses": code_witnesses,
        "inputs": [
            {
                "path": path.relative_to(ROOT).as_posix(),
                "sha256": sha256_file(path),
            }
            for path in input_paths
        ],
        "summary": {
            "carrierCount": len(carriers),
            "spriteCarrierCount": EXPECTED_SHAPE_COUNTS["sprite"],
            "meshCarrierCount": EXPECTED_SHAPE_COUNTS["mesh"],
            "localDecalCarrierCount": EXPECTED_SHAPE_COUNTS["decal"],
            "sourceWaveElementCount": EXPECTED_SOURCE_WAVE_ELEMENT_COUNT,
            "currentAdmittedFamilyCarrierCount": sum(
                row["productAdmission"] == "ADMITTED" for row in carriers
            ),
            "blockedFamilyCarrierCount": sum(
                row["productAdmission"] != "ADMITTED" for row in carriers
            ),
            "externalModuleReferenceCount": external_module_reference_count,
            "externalModulePackageCount": len(external_module_package_counts),
            "carrierWithExternalModuleDependencyCount": sum(
                row["externalModuleClosure"]["referenceCount"] > 0
                for row in carriers
            ),
            "existingSourceWaveCueCount": len(wave_cue_rows),
            "unprojectedImpactCueCount": len(impact_rows),
        },
    }


def atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        json.loads(temporary.read_text(encoding="utf-8"))
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def check_exact(path: Path, payload: bytes) -> None:
    require(path.is_file(), f"family inventory is missing: {path}")
    actual = path.read_bytes()
    require(
        actual == payload,
        "family inventory is stale: "
        f"expected={sha256_bytes(payload)} actual={sha256_bytes(actual)}",
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--output", type=Path, default=OUTPUT_PATH)
    args = parser.parse_args(argv)
    try:
        inventory = build_inventory()
        payload = pretty_bytes(inventory)
        if args.write:
            atomic_write(args.output, payload)
            label = "written"
        elif args.check:
            check_exact(args.output, payload)
            label = "checked"
        else:
            label = "dry-run"
    except InventoryError as error:
        print(f"[FAIL] {error}")
        return 1
    summary = inventory["summary"]
    print(
        "[PASS] Valtan FRONT_BACK_FRONT family inventory "
        f"{label}: carriers={summary['carrierCount']}, "
        f"blocked={summary['blockedFamilyCarrierCount']}, "
        f"sourceWaves={summary['sourceWaveElementCount']}, "
        f"futureCues={summary['existingSourceWaveCueCount'] + summary['unprojectedImpactCueCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
