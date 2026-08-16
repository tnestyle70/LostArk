#!/usr/bin/env python3
"""Build the shared four-class source Material identity contract.

The per-skill Imported documents and their normalized source graphs already
retain the exact UE3 Material instance path, physical package, parameters, and
runtime texture bindings.  This compiler joins those inputs once per unique
``(sourceMaterialPath, sourcePhysicalPackage)`` tuple.  The three existing
finite contracts remain the higher-authority seeds; this output contains only
the remaining identities and never guesses from a character or display name.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys
import tempfile
from collections import Counter
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
EXTERNAL_SOURCE_ROOT = (ROOT.parent / "Resource_LostArk").resolve()
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))

from build_effect_source_material_contract import (  # noqa: E402
    build_contract,
    finite_profile_runtime_resource_contract_satisfied,
    runtime_shader_profile_id,
)


OUTPUT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.source-material-contract.json"
)
RECEIPT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.source-material-contract.receipt.json"
)
RUNTIME_ROOT = ROOT / "Client/Bin/Resources"
PHYSICAL_MATERIAL_ROOT = EXTERNAL_SOURCE_ROOT / (
    "05_Reports/EffectExtraction/DIMENSIONMASTER/materials"
)
# Parent Material3 declaration evidence (group tags, render state, collected
# parameters).  The physical ``.materials.json`` maps carry only the child
# MaterialInstance overrides, so without this document every identity stays
# ``NOT_CAPTURED`` and the grouped-translucent selector has no primary lane
# to route.  Missing/blank is not fatal: the compiler then behaves exactly as
# it did before the capture existed.
MATERIAL_EVIDENCE_PATH = ROOT / (
    "Data/Effects/Imported/FourClassCombat/"
    "FourClassCombat.source-material-evidence.json"
)

CLASS_MANIFESTS = {
    "ARTIST": ROOT
    / "Data/Effects/Imported/Artist/Artist.combat-source-stage-manifest.json",
    "DIMENSIONMASTER": ROOT
    / (
        "Data/Effects/Imported/DimensionMaster/"
        "DimensionMaster.combat-source-stage-manifest.json"
    ),
    "LANCE_MASTER": ROOT
    / (
        "Data/Effects/Imported/LanceMaster/"
        "LanceMaster.combat-source-stage-manifest.json"
    ),
    "WARLORD": ROOT
    / "Data/Effects/Imported/Warlord/Warlord.combat-source-stage-manifest.json",
}
CLASS_RESOURCE_MANIFESTS = {
    "ARTIST": ROOT
    / "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json",
    "DIMENSIONMASTER": ROOT
    / (
        "Data/Effects/Imported/DimensionMaster/"
        "DimensionMaster.resource-source-manifest.json"
    ),
    "LANCE_MASTER": ROOT
    / (
        "Data/Effects/Imported/LanceMaster/"
        "LanceMaster.resource-source-manifest.json"
    ),
}
SEED_CONTRACT_PATHS = (
    ROOT
    / "Data/Effects/Imported/Artist/Converted/skill.31000.source-material-contract.json",
    ROOT
    / (
        "Data/Effects/Imported/DimensionMaster/ActionSource/"
        "DimensionMaster.A.source-material-contract.json"
    ),
    ROOT
    / "Data/Effects/Imported/LanceMaster/Converted/skill.34010.source-material-contract.json",
)


class CorpusContractError(RuntimeError):
    """Raised when a Material identity cannot be joined deterministically."""


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise CorpusContractError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise CorpusContractError(f"JSON root must be an object: {path}")
    return value


def repository_path(value: str, label: str) -> Path:
    relative = Path(value)
    if not value or ".." in relative.parts:
        raise CorpusContractError(f"unsafe {label} path: {value}")
    resolved = relative.resolve() if relative.is_absolute() else (ROOT / relative).resolve()
    if not any(
        resolved == allowed or allowed in resolved.parents
        for allowed in (ROOT.resolve(), EXTERNAL_SOURCE_ROOT)
    ):
        raise CorpusContractError(
            f"{label} escapes the admitted source roots: {value}"
        )
    if not resolved.is_file():
        raise CorpusContractError(f"missing {label}: {value}")
    return resolved


def source_path_label(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def exact_parameter_rows(
    graph_rows: list[dict[str, Any]],
    map_rows: list[dict[str, Any]],
    *,
    family: str,
    value_field: str,
    identity: str,
) -> list[dict[str, Any]]:
    merged: dict[str, dict[str, Any]] = {}
    for source, label in ((map_rows, "map"), (graph_rows, "graph")):
        for row in source:
            if not isinstance(row, dict):
                raise CorpusContractError(
                    f"{identity} {family} {label} row is invalid"
                )
            name = str(row.get("name") or "")
            key = name.casefold()
            if not key:
                continue
            existing = merged.get(key)
            if existing is not None and canonical_sha256(
                existing.get(value_field)
            ) != canonical_sha256(row.get(value_field)):
                raise CorpusContractError(
                    f"{identity} {family} parameter conflicts: {name}"
                )
            if existing is None or label == "graph":
                merged[key] = copy.deepcopy(row)
    return [merged[key] for key in sorted(merged)]


def merge_exact_physical_material_evidence(
    source_graph: dict[str, Any],
    cache: dict[str, dict[str, Any] | None],
) -> dict[str, Any]:
    result = copy.deepcopy(source_graph)
    for row in result.get("materialParameterBindings", []):
        if not isinstance(row, dict):
            raise CorpusContractError("normalized Material row must be an object")
        if row.get("resolutionStatus") != "RESOLVED_EXACT_SOURCE_PACKAGE":
            continue
        wrapper_candidate_count = int(row.get("candidateCount") or 0)
        # Some extractor wrappers count the same resolved package through two
        # or three discovery lanes.  That wrapper cardinality is not Material
        # object ambiguity: the physical package map is authoritative here.
        # Keep the generic compiler strict and normalize only the audited
        # wrapper forms after proving one full object path in the exact map.
        if wrapper_candidate_count not in {1, 2, 3}:
            continue
        physical = str(row.get("sourcePhysicalPackage") or "")
        material_path = str(row.get("materialPath") or "")
        source_path = str(row.get("sourceMaterialPath") or "")
        if not physical or not material_path or not source_path:
            row["resolutionStatus"] = "INCOMPLETE_EXACT_MATERIAL_IDENTITY"
            row["candidateCount"] = 0
            continue
        physical_key = physical.casefold()
        if physical_key not in cache:
            map_path = PHYSICAL_MATERIAL_ROOT / (
                physical.removesuffix(".upk").casefold() + ".materials.json"
            )
            cache[physical_key] = load_json(map_path) if map_path.is_file() else None
        material_map = cache[physical_key]
        if material_map is None:
            row["resolutionStatus"] = "MISSING_PHYSICAL_MATERIAL_MAP"
            row["candidateCount"] = 0
            continue
        map_source = material_map.get("source")
        if (
            not isinstance(map_source, dict)
            or str(map_source.get("file") or "").casefold() != physical_key
        ):
            raise CorpusContractError(
                f"physical Material map package drifted: {physical}"
            )
        candidates = [
            value
            for value in material_map.get("materials", [])
            if isinstance(value, dict)
            and str(value.get("material_path") or "").casefold()
            == material_path.casefold()
        ]
        if len(candidates) != 1:
            row["resolutionStatus"] = (
                "MISSING_PHYSICAL_MATERIAL_ROW"
                if not candidates
                else "AMBIGUOUS_PHYSICAL_MATERIAL_ROW"
            )
            row["candidateCount"] = len(candidates)
            continue
        physical_row = candidates[0]
        row["candidateCount"] = 1
        graph_parent = str(row.get("parent") or "")
        map_parent = str(physical_row.get("parent") or "")
        if (
            graph_parent
            and map_parent
            and graph_parent.casefold() != map_parent.casefold()
        ):
            raise CorpusContractError(
                f"physical Material parent conflicts: {source_path}"
            )
        row["parent"] = graph_parent or map_parent or material_path
        for family, value_field in (
            ("textures", "texture"),
            ("scalars", "value"),
            ("vectors", "value"),
            ("static_switches", "value"),
        ):
            graph_rows = row.get(family) or []
            map_rows = physical_row.get(family) or []
            if graph_rows or map_rows:
                row[family] = exact_parameter_rows(
                    graph_rows,
                    map_rows,
                    family=family,
                    value_field=value_field,
                    identity=source_path,
                )
        row["physicalMaterialEvidence"] = {
            "sourceFile": physical,
            "materialPath": material_path,
            "mapPath": source_path_label(
                PHYSICAL_MATERIAL_ROOT
                / (physical.removesuffix(".upk").casefold() + ".materials.json")
            ),
            "sourceWrapperCandidateCount": wrapper_candidate_count,
        }
    return result


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def material_key(identity: dict[str, Any]) -> tuple[str, str]:
    return (
        str(identity.get("sourceMaterialPath") or "").casefold(),
        str(identity.get("sourcePhysicalPackage") or "").casefold(),
    )


def load_seed_keys() -> set[tuple[str, str]]:
    keys: set[tuple[str, str]] = set()
    for path in SEED_CONTRACT_PATHS:
        contract = load_json(path)
        identities = contract.get("materialIdentities")
        if (
            contract.get("schema") != "lostark.effect-source-material-contract"
            or contract.get("formatVersion") != 1
            or not isinstance(identities, list)
        ):
            raise CorpusContractError(f"seed Material contract drifted: {path}")
        for identity in identities:
            if not isinstance(identity, dict):
                raise CorpusContractError(f"invalid seed Material row: {path}")
            key = material_key(identity)
            if not key[0] or not key[1] or key in keys:
                raise CorpusContractError(
                    f"missing/duplicate seed Material identity: {path}/{key}"
                )
            keys.add(key)
    return keys


def source_artifacts() -> list[tuple[str, dict[str, Any]]]:
    rows: dict[Path, tuple[str, dict[str, Any]]] = {}
    for character_class, manifest_path in CLASS_MANIFESTS.items():
        manifest = load_json(manifest_path)
        if (
            manifest.get("schema") != "lostark.combat-effect-source-stage-manifest"
            or manifest.get("characterClass") != character_class
        ):
            raise CorpusContractError(f"source manifest drifted: {manifest_path}")
        for skill in manifest.get("skills", []):
            for stage in skill.get("stages", []):
                for artifact in stage.get("sourceArtifacts", []):
                    if not isinstance(artifact, dict):
                        raise CorpusContractError(
                            f"source artifact must be an object: {manifest_path}"
                        )
                    descriptor = artifact.get("importedDocument")
                    if not isinstance(descriptor, dict):
                        raise CorpusContractError(
                            f"source artifact has no Imported document: {manifest_path}"
                        )
                    imported_path = repository_path(
                        str(descriptor.get("path") or ""), "Imported Effect"
                    )
                    previous = rows.get(imported_path)
                    if previous is not None:
                        if previous[0] != character_class or previous[1] != artifact:
                            raise CorpusContractError(
                                "Imported Effect maps to two source artifact identities: "
                                f"{imported_path}"
                            )
                        continue
                    rows[imported_path] = (character_class, copy.deepcopy(artifact))
    return [rows[path] for path in sorted(rows, key=lambda value: value.as_posix())]


def synthetic_resource_manifest(character_class: str) -> dict[str, Any]:
    path = CLASS_RESOURCE_MANIFESTS.get(character_class)
    if path is not None:
        return load_json(path)
    # Warlord's exact runtime bindings are already preserved in each normalized
    # graph.  An empty manifest prevents parent-default suffix guessing while
    # still allowing those graph-owned assets to resolve.
    return {
        "schema": "lostark.class-effect-resource-source-manifest",
        "formatVersion": 1,
        "characterClass": character_class,
        "assets": [],
    }


def contract_invariant(identity: dict[str, Any]) -> dict[str, Any]:
    return {
        key: copy.deepcopy(identity.get(key))
        for key in (
            "sourceMaterialPath",
            "sourcePhysicalPackage",
            "parentMaterialPath",
            "profileId",
            "runtimeShaderProfileId",
            "intendedRuntimeShaderProfileId",
            "fallbackBlockedReason",
            "sourceEvidenceResolved",
            "productAdmissionStatus",
            "requiredRuntimeBindings",
            "roleResolvedRuntimeBindings",
            "sourceTextureRoleDiagnostics",
            "blockedRuntimeAssetIds",
            "blockedRuntimeBindings",
            "semanticStatus",
            "sourceParameters",
            "renderState",
        )
    }


def parameter_semantics(identity: dict[str, Any]) -> dict[tuple[str, str], Any]:
    parameters = identity.get("sourceParameters")
    if not isinstance(parameters, dict):
        raise CorpusContractError(
            f"compiled Material parameters are invalid: {material_key(identity)}"
        )
    result: dict[tuple[str, str], Any] = {}
    value_fields = {
        "textures": "texture",
        "scalars": "value",
        "vectors": "value",
        "staticSwitches": "value",
    }
    for family, value_field in value_fields.items():
        for row in parameters.get(family, []):
            if not isinstance(row, dict):
                raise CorpusContractError(
                    f"compiled Material parameter row is invalid: {material_key(identity)}"
                )
            name = str(row.get("name") or "").casefold()
            if not name:
                continue
            key = (family, name)
            value = copy.deepcopy(row.get(value_field))
            previous = result.get(key)
            if previous is not None and canonical_sha256(previous) != canonical_sha256(value):
                raise CorpusContractError(
                    f"duplicate Material parameter disagrees: {material_key(identity)}/{key}"
                )
            result[key] = value
    return result


def identity_evidence_score(identity: dict[str, Any]) -> tuple[int, ...]:
    profile = str(identity.get("runtimeShaderProfileId") or "")
    profile_rank = (
        0
        if profile == "effect.ue3.fallback-blocked.v1"
        else 1
        if profile == "effect.ue3.grouped-translucent.v1"
        else 2
    )
    parameters = identity.get("sourceParameters") or {}
    parameter_count = sum(
        len(parameters.get(name, []))
        for name in ("textures", "scalars", "vectors", "staticSwitches")
    )
    return (
        int(identity.get("sourceEvidenceResolved") is True),
        profile_rank,
        int(identity.get("renderState") is not None),
        parameter_count,
        len(identity.get("roleResolvedRuntimeBindings") or []),
    )


def merge_compiled_identity(
    existing: dict[str, Any], incoming: dict[str, Any]
) -> dict[str, Any]:
    key = material_key(existing)
    if key != material_key(incoming):
        raise CorpusContractError("attempted to merge two Material identities")
    for field in (
        "parentMaterialPath",
        "profileId",
        "intendedRuntimeShaderProfileId",
    ):
        left = str(existing.get(field) or "").casefold()
        right = str(incoming.get(field) or "").casefold()
        if left != right:
            raise CorpusContractError(
                f"one Material/package identity has two {field} values: {key}"
            )
    left_parameters = parameter_semantics(existing)
    right_parameters = parameter_semantics(incoming)
    for parameter_key in left_parameters.keys() & right_parameters.keys():
        if canonical_sha256(left_parameters[parameter_key]) != canonical_sha256(
            right_parameters[parameter_key]
        ):
            raise CorpusContractError(
                "one Material/package identity has conflicting parameter values: "
                f"{key}/{parameter_key}"
            )
    left_render = existing.get("renderState")
    right_render = incoming.get("renderState")
    if (
        left_render is not None
        and right_render is not None
        and canonical_sha256(left_render) != canonical_sha256(right_render)
    ):
        raise CorpusContractError(
            f"one Material/package identity has conflicting render state: {key}"
        )
    selected = copy.deepcopy(
        incoming
        if identity_evidence_score(incoming) > identity_evidence_score(existing)
        else existing
    )
    other = existing if selected is not existing else incoming
    selected_parent_package = str(
        selected.get("parentSourcePhysicalPackage") or ""
    )
    other_parent_package = str(other.get("parentSourcePhysicalPackage") or "")
    if (
        selected_parent_package
        and other_parent_package
        and selected_parent_package.casefold() != other_parent_package.casefold()
    ):
        raise CorpusContractError(
            "one Material identity has two parent packages: "
            f"{key}/{selected_parent_package}/{other_parent_package}"
        )
    if not selected_parent_package and other_parent_package:
        selected["parentSourcePhysicalPackage"] = other_parent_package
    selected["materialEvidenceSource"] = "CORPUS_EXACT_EVIDENCE_UNION"
    return selected


def enforce_corpus_admission_boundary(
    character_class: str, identity: dict[str, Any]
) -> dict[str, Any]:
    result = copy.deepcopy(identity)
    source_path = str(result.get("sourceMaterialPath") or "").casefold()
    intended_profile = runtime_shader_profile_id(
        str(result.get("parentMaterialPath") or ""), source_path
    )
    if intended_profile:
        result["intendedRuntimeShaderProfileId"] = intended_profile
    if source_path.startswith("enginematerials."):
        result["sourceEvidenceResolved"] = False
        result["runtimeShaderProfileId"] = "effect.ue3.fallback-blocked.v1"
        result["fallbackBlockedReason"] = "ENGINE_MATERIAL_POLICY_NOT_AUTHORED"
        result["productAdmissionStatus"] = "BLOCKED_SOURCE_EVIDENCE"
        result["requiredRuntimeBindings"] = []
        return result

    required = result.get("requiredRuntimeBindings") or []
    if character_class != "DIMENSIONMASTER" and required:
        if any(
            str(row.get("assetId") or "").startswith("Effect/DimensionMaster/")
            for row in required
            if isinstance(row, dict)
        ):
            profile_id = str(result.get("runtimeShaderProfileId") or "")
            if not finite_profile_runtime_resource_contract_satisfied(
                profile_id,
                result.get("currentResourceBindings") or [],
                result.get("sourceParameters") or {},
            ):
                result["runtimeShaderProfileId"] = (
                    "effect.ue3.fallback-blocked.v1"
                )
                result["fallbackBlockedReason"] = (
                    "MISSING_CLASS_LOCAL_FINITE_PROFILE_RESOURCE"
                )
                result["productAdmissionStatus"] = "BLOCKED_FALLBACK_PROFILE"
            # The aggregate contract never serializes a class-local donor.
            # Each occurrence is rebound from its own source receipt later.
            result["requiredRuntimeBindings"] = []
    return result


def build_projection() -> tuple[dict[str, Any], dict[str, Any]]:
    seed_keys = load_seed_keys()
    identities_by_key: dict[tuple[str, str], dict[str, Any]] = {}
    occurrence_ids_by_key: dict[tuple[str, str], set[str]] = {}
    blocked_without_package: dict[str, set[str]] = {}
    input_rows: list[dict[str, str]] = []
    artifact_count_by_class: Counter[str] = Counter()
    material_map_cache: dict[str, dict[str, Any] | None] = {}
    normalized_wrapper_row_count = 0
    normalized_wrapper_keys: set[tuple[str, str]] = set()
    material_evidence: dict[str, Any] | None = None
    if MATERIAL_EVIDENCE_PATH.is_file():
        material_evidence = load_json(MATERIAL_EVIDENCE_PATH)
        if (
            material_evidence.get("schema")
            != "lostark.effect-source-material-evidence"
            or material_evidence.get("formatVersion") != 1
        ):
            raise CorpusContractError(
                f"parent Material evidence drifted: {MATERIAL_EVIDENCE_PATH}"
            )
        input_rows.append(
            {
                "path": source_path_label(MATERIAL_EVIDENCE_PATH),
                "sha256": file_sha256(MATERIAL_EVIDENCE_PATH),
            }
        )

    for character_class, artifact in source_artifacts():
        descriptors = {
            label: artifact.get(field)
            for label, field in (
                ("Imported Effect", "importedDocument"),
                ("normalized graph", "normalizedGraph"),
                ("conversion receipt", "conversionReceipt"),
            )
        }
        if not all(isinstance(value, dict) for value in descriptors.values()):
            raise CorpusContractError(
                f"source artifact descriptors are incomplete: {artifact}"
            )
        paths = {
            label: repository_path(str(value.get("path") or ""), label)
            for label, value in descriptors.items()
        }
        normalized_graph = merge_exact_physical_material_evidence(
            load_json(paths["normalized graph"]), material_map_cache
        )
        for row in normalized_graph.get("materialParameterBindings", []):
            if not isinstance(row, dict):
                continue
            evidence = row.get("physicalMaterialEvidence")
            if (
                not isinstance(evidence, dict)
                or int(evidence.get("sourceWrapperCandidateCount") or 0) <= 1
            ):
                continue
            key = (
                str(row.get("sourceMaterialPath") or "").casefold(),
                str(row.get("sourcePhysicalPackage") or "").casefold(),
            )
            if not key[0] or not key[1]:
                raise CorpusContractError(
                    "normalized wrapper Material identity is incomplete"
                )
            normalized_wrapper_row_count += 1
            normalized_wrapper_keys.add(key)

        contract, _ = build_contract(
            load_json(paths["Imported Effect"]),
            normalized_graph,
            load_json(paths["conversion receipt"]),
            synthetic_resource_manifest(character_class),
            material_map=material_evidence,
            runtime_resource_root=RUNTIME_ROOT,
        )
        artifact_count_by_class[character_class] += 1
        for path in paths.values():
            input_rows.append(
                {
                    "path": source_path_label(path),
                    "sha256": file_sha256(path),
                }
            )
        for raw_identity in contract.get("materialIdentities", []):
            identity = enforce_corpus_admission_boundary(
                character_class, raw_identity
            )
            key = material_key(identity)
            if not key[0]:
                raise CorpusContractError(
                    f"compiled Material identity is incomplete: {key}"
                )
            if not key[1]:
                blocked_without_package.setdefault(key[0], set()).update(
                    str(value)
                    for value in identity.get("runtimeOccurrenceElementIds", [])
                    if str(value)
                )
                continue
            if key in seed_keys:
                continue
            occurrences = {
                str(value)
                for value in identity.get("runtimeOccurrenceElementIds", [])
                if str(value)
            }
            existing = identities_by_key.get(key)
            if existing is None:
                identities_by_key[key] = identity
                occurrence_ids_by_key[key] = occurrences
                continue
            identities_by_key[key] = merge_compiled_identity(existing, identity)
            occurrence_ids_by_key[key].update(occurrences)

    identities: list[dict[str, Any]] = []
    for key in sorted(identities_by_key):
        identity = identities_by_key[key]
        occurrence_ids = sorted(occurrence_ids_by_key[key], key=str.casefold)
        identity["runtimeOccurrenceElementIds"] = occurrence_ids
        identity["runtimeOccurrenceCount"] = len(occurrence_ids)
        if key in normalized_wrapper_keys:
            identity["exactPhysicalWrapperEvidence"] = {
                "resolutionStatus": "RESOLVED_EXACT_SOURCE_PACKAGE",
                "sourceCandidateCount": "2_OR_3",
                "normalizedCandidateCount": 1,
                "physicalPackageSourceIdentity": "EXACT",
                "fullMaterialPathMatchCount": 1,
            }
        identities.append(identity)

    normalized_wrapper_identities = [
        row
        for row in identities
        if isinstance(row.get("exactPhysicalWrapperEvidence"), dict)
    ]

    for physical_key, material_map in sorted(material_map_cache.items()):
        if material_map is None:
            continue
        map_path = PHYSICAL_MATERIAL_ROOT / (
            physical_key.removesuffix(".upk") + ".materials.json"
        )
        input_rows.append(
            {"path": source_path_label(map_path), "sha256": file_sha256(map_path)}
        )

    profile_members: dict[str, list[dict[str, Any]]] = {}
    for identity in identities:
        profile_members.setdefault(str(identity["profileId"]), []).append(identity)
    profiles = []
    for profile_id, members in sorted(profile_members.items()):
        profiles.append(
            {
                "profileId": profile_id,
                "parentMaterialPath": members[0]["parentMaterialPath"],
                "semanticStatus": "RECONSTRUCTED_PROFILE",
                "runtimeShaderProfileIds": sorted(
                    {str(row["runtimeShaderProfileId"]) for row in members}
                ),
                "materialIdentityCount": len(members),
                "materialIdentities": sorted(
                    (str(row["sourceMaterialPath"]) for row in members),
                    key=str.casefold,
                ),
                "runtimeOccurrenceCount": sum(
                    int(row["runtimeOccurrenceCount"]) for row in members
                ),
            }
        )

    counts = {
        "seedMaterialIdentityCount": len(seed_keys),
        "compiledMaterialIdentityCount": len(identities),
        "blockedMissingPhysicalPackageIdentityCount": len(
            blocked_without_package
        ),
        "blockedMissingPhysicalPackageOccurrenceCount": sum(
            len(rows) for rows in blocked_without_package.values()
        ),
        "totalMaterialIdentityCount": (
            len(seed_keys) + len(identities) + len(blocked_without_package)
        ),
        "compiledOccurrenceCount": sum(
            int(row["runtimeOccurrenceCount"]) for row in identities
        ),
        "sourceEvidenceResolvedIdentityCount": sum(
            row.get("sourceEvidenceResolved") is True for row in identities
        ),
        "admittedIdentityCount": sum(
            row.get("productAdmissionStatus")
            == "ADMITTED_RECONSTRUCTED_PROFILE"
            for row in identities
        ),
        "fallbackBlockedIdentityCount": sum(
            row.get("productAdmissionStatus") == "BLOCKED_FALLBACK_PROFILE"
            for row in identities
        ),
        "sourceEvidenceBlockedIdentityCount": sum(
            row.get("productAdmissionStatus") == "BLOCKED_SOURCE_EVIDENCE"
            for row in identities
        ),
        "sourceArtifactCount": sum(artifact_count_by_class.values()),
        "exactPhysicalWrapperNormalizedRowCount": normalized_wrapper_row_count,
        "exactPhysicalWrapperMaterialIdentityCount": len(
            normalized_wrapper_keys
        ),
        "exactPhysicalWrapperCompiledIdentityCount": len(
            normalized_wrapper_identities
        ),
        "exactPhysicalWrapperSeedIdentityCount": len(
            normalized_wrapper_keys
        )
        - len(normalized_wrapper_identities),
        "exactPhysicalWrapperAdmittedIdentityCount": sum(
            row.get("productAdmissionStatus")
            == "ADMITTED_RECONSTRUCTED_PROFILE"
            for row in normalized_wrapper_identities
        ),
        "exactPhysicalWrapperAdmittedOccurrenceCount": sum(
            int(row.get("runtimeOccurrenceCount") or 0)
            for row in normalized_wrapper_identities
            if row.get("productAdmissionStatus")
            == "ADMITTED_RECONSTRUCTED_PROFILE"
        ),
        "exactPhysicalWrapperFallbackBlockedIdentityCount": sum(
            row.get("productAdmissionStatus") == "BLOCKED_FALLBACK_PROFILE"
            for row in normalized_wrapper_identities
        ),
        "exactPhysicalWrapperFallbackBlockedOccurrenceCount": sum(
            int(row.get("runtimeOccurrenceCount") or 0)
            for row in normalized_wrapper_identities
            if row.get("productAdmissionStatus") == "BLOCKED_FALLBACK_PROFILE"
        ),
    }
    contract = {
        "schema": "lostark.effect-source-material-contract",
        "formatVersion": 1,
        "effectAssetId": "effect.four-class.combat.source-material-contract",
        "summary": counts,
        "materialIdentities": identities,
        "profiles": profiles,
        "failures": [
            {
                "sourceMaterialPath": row["sourceMaterialPath"],
                "sourcePhysicalPackage": row["sourcePhysicalPackage"],
                "status": row["productAdmissionStatus"],
            }
            for row in identities
            if row.get("productAdmissionStatus") != "ADMITTED_RECONSTRUCTED_PROFILE"
        ]
        + [
            {
                "sourceMaterialPath": path,
                "sourcePhysicalPackage": None,
                "status": "BLOCKED_MISSING_SOURCE_PACKAGE",
                "runtimeOccurrenceCount": len(occurrences),
            }
            for path, occurrences in sorted(blocked_without_package.items())
        ],
    }
    receipt = {
        "schema": "lostark.four-class-source-material-contract-receipt",
        "formatVersion": 1,
        "contractEffectAssetId": contract["effectAssetId"],
        "counts": counts,
        "artifactCountByClass": dict(sorted(artifact_count_by_class.items())),
        "sources": sorted(
            {row["path"]: row for row in input_rows}.values(),
            key=lambda row: row["path"],
        ),
        "contractCanonicalSha256": canonical_sha256(contract),
    }
    return contract, receipt


def serialized(value: dict[str, Any]) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2) + "\n"


def commit(outputs: dict[Path, str]) -> None:
    staged: dict[Path, Path] = {}
    try:
        for path, payload in outputs.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            handle, temporary = tempfile.mkstemp(
                prefix=path.name + ".", suffix=".tmp", dir=path.parent
            )
            os.close(handle)
            temporary_path = Path(temporary)
            temporary_path.write_text(payload, encoding="utf-8")
            staged[path] = temporary_path
        for path, temporary in staged.items():
            os.replace(temporary, path)
    finally:
        for temporary in staged.values():
            try:
                temporary.unlink()
            except OSError:
                pass


def check(outputs: dict[Path, str]) -> None:
    stale = [
        path.relative_to(ROOT).as_posix()
        for path, payload in outputs.items()
        if not path.is_file()
        or load_json(path) != json.loads(payload)
    ]
    if stale:
        raise CorpusContractError(
            "four-class source Material contract is stale: " + ", ".join(stale)
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    args = parser.parse_args()

    contract, receipt = build_projection()
    outputs = {
        OUTPUT_PATH: serialized(contract),
        RECEIPT_PATH: serialized(receipt),
    }
    if args.write:
        commit(outputs)
        check(outputs)
        print(f"Four-class source Material contract written: {receipt['counts']}")
    elif args.check:
        check(outputs)
        print(f"Four-class source Material contract check PASS: {receipt['counts']}")
    else:
        print(f"Four-class source Material contract dry-run PASS: {receipt['counts']}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (CorpusContractError, ValueError) as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
