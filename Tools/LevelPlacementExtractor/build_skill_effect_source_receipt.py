#!/usr/bin/env python3
"""Build an exact animation-timed effect source receipt from UE3 particle graphs.

The receipt deliberately separates an Animation/CModel-owned combined mesh
animation from effect overlays.  It never guesses an Effect Element kind from
an object name.  Particle graph class names, property edges, and explicit
Material/Mesh/Texture properties are the only automatic classification inputs.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from collections import Counter, deque
from pathlib import Path
from typing import Any


BLOCK_RE = re.compile(
    r'^"(?P<clip>[^"]+)"\s+skill=(?P<skill>\d+)\s+'
    r'len=(?P<length>-?\d+(?:\.\d+)?)\s+name="(?P<name>.*)"$'
)
NOTIFY_RE = re.compile(
    r'^\s+n\s+t=(?P<time>-?\d+(?:\.\d+)?)\s+'
    r'd=(?P<duration>-?\d+(?:\.\d+)?)\s+'
    r'kind=(?P<kind>\S+)\s+src=(?P<source>\S+)\s+'
    r'asset="(?P<asset>[^"]*)"\s+label="(?P<label>[^"]*)"\s+'
    r'win=(?P<window>\S+)\s*$'
)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def parse_animnotify(path: Path) -> dict[str, dict[str, Any]]:
    clips: dict[str, dict[str, Any]] = {}
    current: dict[str, Any] | None = None
    for line_number, line in enumerate(
        path.read_text(encoding="utf-8-sig").splitlines(), start=1
    ):
        block_match = BLOCK_RE.match(line)
        if block_match:
            current = {
                "clip": block_match.group("clip"),
                "skillId": int(block_match.group("skill")),
                "lengthSeconds": float(block_match.group("length")),
                "displayName": block_match.group("name"),
                "sourceLine": line_number,
                "notifies": [],
            }
            clips[current["clip"]] = current
            continue

        notify_match = NOTIFY_RE.match(line)
        if notify_match and current is not None:
            current["notifies"].append(
                {
                    "localTimeSeconds": float(notify_match.group("time")),
                    "durationSeconds": float(notify_match.group("duration")),
                    "kind": notify_match.group("kind"),
                    "sourceType": notify_match.group("source"),
                    "sourceAsset": notify_match.group("asset"),
                    "label": notify_match.group("label"),
                    "window": notify_match.group("window"),
                    "sourceLine": line_number,
                }
            )
    return clips


def flatten_bound_clips(values: Any, skill_id: int) -> list[str]:
    """Flatten ACTIVE and COMBO binding shapes without losing source order."""
    if not isinstance(values, list) or not values:
        raise ValueError(f"skill {skill_id} has an invalid clips array")
    result: list[str] = []

    def append(value: Any) -> None:
        if isinstance(value, str):
            clip_name = value
        elif isinstance(value, dict):
            clip_name = str(value.get("clip") or "")
        elif isinstance(value, list):
            if not value:
                raise ValueError(
                    f"skill {skill_id} has an invalid combo stage"
                )
            for nested in value:
                append(nested)
            return
        else:
            clip_name = ""
        if not clip_name:
            raise ValueError(f"skill {skill_id} has an invalid clip entry")
        result.append(clip_name)

    for item in values:
        append(item)
    return result


def load_bound_clips(path: Path, skill_id: int) -> tuple[dict[str, Any], list[str]]:
    document = json.loads(path.read_text(encoding="utf-8-sig"))
    matches = [
        row for row in document.get("bindings", []) if int(row.get("skillId", -1)) == skill_id
    ]
    if len(matches) != 1:
        raise ValueError(f"skill {skill_id} must have exactly one binding, got {len(matches)}")
    clip_names = flatten_bound_clips(matches[0].get("clips"), skill_id)
    # Return the untouched document so playMs/playRate and future per-clip
    # metadata remain available to downstream receipts.
    return document, clip_names


def build_timeline(
    clip_names: list[str], clip_catalog: dict[str, dict[str, Any]], skill_id: int
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], float]:
    clips = []
    events = []
    offset = 0.0
    event_index = 0
    for sequence_index, clip_name in enumerate(clip_names):
        clip = clip_catalog.get(clip_name)
        if clip is None:
            raise ValueError(f"bound clip is absent from animnotify: {clip_name}")
        if int(clip["skillId"]) != skill_id:
            raise ValueError(
                f"bound clip {clip_name} belongs to skill {clip['skillId']}, expected {skill_id}"
            )
        clips.append(
            {
                "sequenceIndex": sequence_index,
                "clip": clip_name,
                "offsetSeconds": round(offset, 7),
                "lengthSeconds": clip["lengthSeconds"],
                "sourceLine": clip["sourceLine"],
            }
        )
        for notify in clip["notifies"]:
            if notify["kind"] not in {"EFFECT", "SHAKE"}:
                continue
            event_index += 1
            events.append(
                {
                    "eventId": f"source-event-{event_index:03d}",
                    "clip": clip_name,
                    "clipSequenceIndex": sequence_index,
                    "localTimeSeconds": notify["localTimeSeconds"],
                    "globalTimeSeconds": round(offset + notify["localTimeSeconds"], 7),
                    "durationSeconds": notify["durationSeconds"],
                    "kind": notify["kind"],
                    "sourceType": notify["sourceType"],
                    "sourceAsset": notify["sourceAsset"],
                    "sourceLine": notify["sourceLine"],
                }
            )
        offset += float(clip["lengthSeconds"])
    return clips, events, round(offset, 7)


def parse_graph_spec(value: str) -> tuple[str, Path]:
    package, separator, raw_path = value.partition("=")
    if not separator or not package.strip() or not raw_path.strip():
        raise argparse.ArgumentTypeError("graph must be PACKAGE=PATH")
    return package.strip(), Path(raw_path.strip())


def source_asset_parts(source_asset: str) -> tuple[str, str, str] | None:
    parts = [part for part in source_asset.split(".") if part]
    if len(parts) < 2:
        return None
    return parts[0], ".".join(parts[1:]), parts[-1]


def full_node_path(package: str, object_path: str | None) -> str | None:
    if not object_path:
        return None
    folded_package = package.casefold()
    folded_path = object_path.casefold()
    if folded_path == folded_package or folded_path.startswith(folded_package + "."):
        return folded_path
    return folded_package + "." + folded_path


def resource_role(property_name: str) -> str | None:
    folded = property_name.casefold()
    if "material" in folded:
        return "material"
    if folded == "mesh" or folded.endswith("mesh"):
        return "mesh"
    if "texture" in folded:
        return "texture"
    return None


def load_graphs(graph_specs: list[tuple[str, Path]]) -> dict[str, Any]:
    packages: dict[str, dict[str, Any]] = {}
    nodes_by_full_path: dict[str, tuple[str, dict[str, Any]]] = {}
    for requested_package, path in graph_specs:
        graph = json.loads(path.read_text(encoding="utf-8-sig"))
        package = str(graph.get("package") or requested_package)
        if package.casefold() != requested_package.casefold():
            raise ValueError(
                f"graph package mismatch: requested {requested_package}, file contains {package}"
            )
        key = package.casefold()
        if key in packages:
            raise ValueError(f"duplicate graph package: {package}")
        by_export = {int(row["exportIndex"]): row for row in graph.get("objects", [])}
        packages[key] = {
            "package": package,
            "path": path,
            "graph": graph,
            "byExport": by_export,
        }
        for row in graph.get("objects", []):
            path_key = full_node_path(package, row.get("objectPath"))
            if path_key:
                nodes_by_full_path[path_key] = (key, row)
    return {"packages": packages, "nodesByFullPath": nodes_by_full_path}


def find_particle_system(
    graph_index: dict[str, Any], package: str, object_path: str, object_name: str
) -> tuple[str, dict[str, Any]] | None:
    package_key = package.casefold()
    package_graph = graph_index["packages"].get(package_key)
    if package_graph is None:
        return None

    exact_path = full_node_path(package, object_path)
    exact = graph_index["nodesByFullPath"].get(exact_path or "")
    if exact and exact[1]["className"].casefold() == "particlesystem":
        return exact

    matches = [
        row
        for row in package_graph["graph"].get("objects", [])
        if row["className"].casefold() == "particlesystem"
        and row["objectName"].casefold() == object_name.casefold()
    ]
    if len(matches) == 1:
        return package_key, matches[0]
    return None


def resolve_reference(
    graph_index: dict[str, Any], source_package_key: str, reference: dict[str, Any]
) -> tuple[str, dict[str, Any]] | None:
    package_index = int(reference.get("packageIndex", 0))
    if package_index > 0:
        package_graph = graph_index["packages"][source_package_key]
        row = package_graph["byExport"].get(package_index - 1)
        return (source_package_key, row) if row is not None else None

    object_path = reference.get("objectPath")
    if object_path:
        return graph_index["nodesByFullPath"].get(object_path.casefold())
    return None


def collect_system_graph(
    graph_index: dict[str, Any], root_package_key: str, root: dict[str, Any]
) -> dict[str, Any]:
    queue: deque[tuple[str, dict[str, Any]]] = deque([(root_package_key, root)])
    visited: set[str] = set()
    nodes: dict[str, dict[str, Any]] = {}
    edges: list[dict[str, Any]] = []
    resources: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []

    while queue:
        package_key, row = queue.popleft()
        package = graph_index["packages"][package_key]["package"]
        node_id = f"{package}:export:{row['exportIndex']}"
        if node_id in visited:
            continue
        visited.add(node_id)
        nodes[node_id] = {
            "nodeId": node_id,
            "package": package,
            "exportIndex": row["exportIndex"],
            "className": row["className"],
            "objectName": row["objectName"],
            "objectPath": row.get("objectPath"),
            "properties": row.get("properties", {}),
        }

        for reference_index, reference in enumerate(row.get("references", [])):
            target = resolve_reference(graph_index, package_key, reference)
            base = {
                "sourceNodeId": node_id,
                "referenceIndex": reference_index,
                "property": reference.get("property"),
                "packageIndex": reference.get("packageIndex"),
                "objectPath": reference.get("objectPath"),
            }
            if target is not None:
                target_package_key, target_row = target
                target_package = graph_index["packages"][target_package_key]["package"]
                target_id = f"{target_package}:export:{target_row['exportIndex']}"
                edges.append({**base, "targetNodeId": target_id})
                if target_id not in visited:
                    queue.append(target)
                continue

            role = resource_role(str(reference.get("property", "")))
            if role is not None:
                resources.append({**base, "role": role})
            else:
                reason = (
                    "external_graph_package_not_loaded"
                    if int(reference.get("packageIndex", 0)) < 0
                    else "local_reference_is_not_a_decoded_particle_graph_node"
                )
                unresolved.append({**base, "reason": reason})

    class_counts = Counter(node["className"] for node in nodes.values())
    feature_classes = sorted(
        name
        for name in class_counts
        if any(token in name.casefold() for token in ("typedata", "ribbon", "subuv", "decal"))
    )
    return {
        "rootNodeId": f"{graph_index['packages'][root_package_key]['package']}:export:{root['exportIndex']}",
        "nodeIds": sorted(nodes),
        "nodes": nodes,
        "edges": edges,
        "resourceBindings": resources,
        "unresolvedExternalReferences": unresolved,
        "summary": {
            "nodeCount": len(nodes),
            "edgeCount": len(edges),
            "emitterCount": sum(
                count
                for name, count in class_counts.items()
                if name.casefold().endswith("emitter")
            ),
            "lodLevelCount": sum(
                count
                for name, count in class_counts.items()
                if name.casefold() == "particlelodlevel"
            ),
            "moduleCount": sum(
                count
                for name, count in class_counts.items()
                if name.casefold().startswith(("particlemodule", "efparticlemodule"))
            ),
            "distributionCount": sum(
                count
                for name, count in class_counts.items()
                if name.casefold().startswith("distribution")
            ),
            "resourceBindingCount": len(resources),
            "unresolvedExternalReferenceCount": len(unresolved),
            "featureClasses": feature_classes,
        },
    }


def compact_unresolved(systems: list[dict[str, Any]], events: list[dict[str, Any]]) -> list[dict[str, Any]]:
    grouped: dict[tuple[str, str, str], dict[str, Any]] = {}
    for system in systems:
        for row in system["graph"]["unresolvedExternalReferences"]:
            key = (str(row.get("objectPath")), str(row.get("property")), str(row.get("reason")))
            item = grouped.setdefault(
                key,
                {
                    "sourceObjectPath": row.get("objectPath"),
                    "property": row.get("property"),
                    "reason": row.get("reason"),
                    "occurrences": 0,
                    "sourceSystems": set(),
                },
            )
            item["occurrences"] += 1
            item["sourceSystems"].add(system["sourceAsset"])

    result = []
    for item in grouped.values():
        result.append(
            {
                **{key: value for key, value in item.items() if key != "sourceSystems"},
                "sourceSystems": sorted(item["sourceSystems"], key=str.casefold),
            }
        )
    for event in events:
        if event.get("resolutionStatus") in {"UNSUPPORTED_SOURCE_NOTIFY", "UNRESOLVED_SOURCE_SYSTEM"}:
            result.append(
                {
                    "sourceEventId": event["eventId"],
                    "sourceObjectPath": event.get("sourceAsset") or None,
                    "reason": event["resolutionStatus"].casefold(),
                    "sourceSystems": [],
                    "occurrences": 1,
                }
            )
    return sorted(
        result,
        key=lambda row: (
            str(row.get("sourceObjectPath", "")).casefold(),
            str(row.get("property", "")).casefold(),
        ),
    )


def source_package_receipts(graph_index: dict[str, Any]) -> list[dict[str, Any]]:
    rows = []
    for package_graph in graph_index["packages"].values():
        graph = package_graph["graph"]
        graph_path: Path = package_graph["path"]
        physical_path = Path(str(graph.get("physicalPackage", "")))
        row = {
            "logicalPackage": package_graph["package"],
            "physicalPackage": physical_path.name if physical_path.name else None,
            "graphSha256": sha256_file(graph_path),
            "graphSummary": graph.get("summary", {}),
        }
        if physical_path.is_file():
            row.update(
                {
                    "sourcePackageBytes": physical_path.stat().st_size,
                    "sourcePackageSha256": sha256_file(physical_path),
                }
            )
        rows.append(row)
    return sorted(rows, key=lambda row: row["logicalPackage"].casefold())


def load_source_package_manifest(path: Path | None) -> dict[str, str]:
    if path is None:
        return {}
    document = json.loads(path.read_text(encoding="utf-8-sig"))
    return {
        str(row["logicalPackage"]).casefold(): str(row["physicalPackage"])
        for row in document.get("packages", [])
        if row.get("logicalPackage") and row.get("physicalPackage")
    }


def material_candidate_signature(candidate: dict[str, Any]) -> str:
    payload = {
        key: candidate.get(key)
        for key in ("material_path", "object_name", "class", "parent", "textures", "scalars", "vectors")
    }
    return json.dumps(payload, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def resolve_material_parameters(
    material_path: str,
    material_catalog: dict[str, list[dict[str, Any]]],
    package_manifest: dict[str, str],
) -> dict[str, Any]:
    parts = material_path.split(".")
    source_root = parts[0].casefold() if parts else ""
    candidate_keys = [material_path.casefold()]
    if len(parts) > 1:
        candidate_keys.append(".".join(parts[1:]).casefold())

    matches: list[tuple[str, dict[str, Any]]] = []
    seen: set[tuple[str, str]] = set()
    for key in candidate_keys:
        for candidate in material_catalog.get(key, []):
            identity = (key, str(candidate.get("source_file", "")).casefold())
            if identity in seen:
                continue
            seen.add(identity)
            matches.append((key, candidate))

    expected_physical = package_manifest.get(source_root)
    exact_source = [
        item
        for item in matches
        if expected_physical
        and str(item[1].get("source_file", "")).casefold() == expected_physical.casefold()
    ]
    selected_pool = exact_source or matches
    signatures = {material_candidate_signature(item[1]) for item in selected_pool}
    if len(selected_pool) == 1:
        status = "RESOLVED_EXACT_SOURCE_PACKAGE" if exact_source else "RESOLVED_UNIQUE_PATH"
        selected = selected_pool[0]
    elif selected_pool and len(signatures) == 1:
        status = "RESOLVED_IDENTICAL_COPIES"
        selected = selected_pool[0]
    elif selected_pool:
        status = "AMBIGUOUS_MATERIAL_PATH"
        selected = None
    else:
        status = "UNRESOLVED_MATERIAL_PATH"
        selected = None

    result: dict[str, Any] = {
        "sourceMaterialPath": material_path,
        "sourceLogicalPackage": parts[0] if parts else None,
        "expectedPhysicalPackage": expected_physical,
        "resolutionStatus": status,
        "candidateCount": len(matches),
    }
    if selected is not None:
        key, candidate = selected
        result.update(
            {
                "catalogKey": key,
                "sourcePhysicalPackage": candidate.get("source_file"),
                "materialPath": candidate.get("material_path"),
                "objectName": candidate.get("object_name"),
                "className": candidate.get("class"),
                "parent": candidate.get("parent"),
                "parentSourcePhysicalPackage": candidate.get(
                    "parent_source_file"
                ),
                "textures": candidate.get("textures", []),
                "scalars": candidate.get("scalars", []),
                "vectors": candidate.get("vectors", []),
            }
        )
    else:
        result["candidates"] = [
            {
                "catalogKey": key,
                "sourcePhysicalPackage": candidate.get("source_file"),
                "className": candidate.get("class"),
                "parent": candidate.get("parent"),
            }
            for key, candidate in matches
        ]
    return result


def build_material_parameter_bindings(
    systems: list[dict[str, Any]],
    material_map_path: Path | None,
    source_package_manifest_path: Path | None,
) -> list[dict[str, Any]]:
    if material_map_path is None:
        return []
    document = json.loads(material_map_path.read_text(encoding="utf-8-sig"))
    raw_catalog = document.get("materials", {})
    material_catalog = {
        str(key).casefold(): list(value) for key, value in raw_catalog.items()
    }
    package_manifest = load_source_package_manifest(source_package_manifest_path)
    paths = sorted(
        {
            str(binding["objectPath"])
            for system in systems
            for binding in system["graph"]["resourceBindings"]
            if binding.get("role") == "material" and binding.get("objectPath")
        },
        key=str.casefold,
    )
    return [
        resolve_material_parameters(path, material_catalog, package_manifest) for path in paths
    ]


def required_material_sources(
    systems: list[dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    required: dict[str, dict[str, Any]] = {}
    for system in systems:
        source_system_id = str(system.get("sourceSystemId") or "")
        if not source_system_id:
            raise ValueError("source system is missing sourceSystemId")
        for binding in system["graph"]["resourceBindings"]:
            if binding.get("role") != "material" or not binding.get("objectPath"):
                continue
            source_path = str(binding["objectPath"])
            key = source_path.casefold()
            row = required.setdefault(
                key,
                {
                    "sourceMaterialPath": source_path,
                    "sourceSystemIds": set(),
                },
            )
            if row["sourceMaterialPath"] != source_path:
                raise ValueError(
                    "material path has inconsistent case-sensitive identities: "
                    f"{row['sourceMaterialPath']} / {source_path}"
                )
            row["sourceSystemIds"].add(source_system_id.casefold())
    return required


def load_resolved_material_catalog(
    systems: list[dict[str, Any]],
    catalog_path: Path,
    character_class: str,
    skill_id: int,
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    """Select only exact, action-owned rows from an audited material catalog."""
    document = json.loads(catalog_path.read_text(encoding="utf-8-sig"))
    if document.get("schema") != "lostark.unbound-class-particle-resource-catalog":
        raise ValueError("resolved material catalog schema is invalid")
    if document.get("formatVersion") != 1:
        raise ValueError("resolved material catalog formatVersion is invalid")
    if str(document.get("characterClass") or "").casefold() != character_class.casefold():
        raise ValueError("resolved material catalog characterClass does not match")
    if document.get("bindingStatus") != "ACTION_NOTIFY_BOUND":
        raise ValueError("resolved material catalog is not action-notify bound")

    material_rows = document.get("materialParameterBindings")
    asset_rows = document.get("assets")
    if not isinstance(material_rows, list) or not isinstance(asset_rows, list):
        raise ValueError("resolved material catalog has invalid binding arrays")

    def unique_index(
        rows: list[dict[str, Any]], key_name: str, label: str
    ) -> dict[str, dict[str, Any]]:
        result: dict[str, dict[str, Any]] = {}
        for index, row in enumerate(rows):
            if not isinstance(row, dict):
                raise ValueError(f"{label}[{index}] is not an object")
            raw_key = row.get(key_name)
            if not isinstance(raw_key, str) or not raw_key:
                raise ValueError(f"{label}[{index}] has no {key_name}")
            key = raw_key.casefold()
            if key in result:
                raise ValueError(f"resolved material catalog has duplicate {label}: {raw_key}")
            result[key] = row
        return result

    material_index = unique_index(
        material_rows, "sourceMaterialPath", "materialParameterBindings"
    )
    asset_index = unique_index(asset_rows, "sourceAssetPath", "assets")
    required = required_material_sources(systems)
    selected: list[dict[str, Any]] = []
    exact_count = 0
    explicit_fallback_count = 0
    for key in sorted(required):
        requirement = required[key]
        source_path = requirement["sourceMaterialPath"]
        material = material_index.get(key)
        asset = asset_index.get(key)
        if material is None or asset is None:
            raise ValueError(
                f"resolved material catalog is missing required material evidence: {source_path}"
            )

        expected_logical = source_path.split(".", 1)[0]
        if str(material.get("sourceLogicalPackage") or "").casefold() != expected_logical.casefold():
            raise ValueError(
                f"material logical package identity does not match: {source_path}"
            )
        if str(asset.get("logicalPackage") or "").casefold() != expected_logical.casefold():
            raise ValueError(
                f"material asset logical package identity does not match: {source_path}"
            )
        roles = asset.get("roles")
        if not isinstance(roles, list) or "material" not in {
            str(role).casefold() for role in roles
        }:
            raise ValueError(f"catalog asset is not material evidence: {source_path}")
        action_ids = asset.get("actionIds")
        if not isinstance(action_ids, list) or skill_id not in action_ids:
            raise ValueError(
                f"catalog material is not owned by action {skill_id}: {source_path}"
            )
        catalog_systems = asset.get("sourceSystems")
        if not isinstance(catalog_systems, list):
            raise ValueError(f"catalog material has invalid source systems: {source_path}")
        catalog_system_ids = {str(value).casefold() for value in catalog_systems}
        if not requirement["sourceSystemIds"].issubset(catalog_system_ids):
            raise ValueError(
                f"catalog material is not owned by every selected source system: {source_path}"
            )

        status = str(material.get("resolutionStatus") or "")
        asset_status = str(asset.get("resolutionStatus") or "")
        if status == "RESOLVED_EXACT_SOURCE_PACKAGE":
            expected_physical = material.get("expectedPhysicalPackage")
            source_physical = material.get("sourcePhysicalPackage")
            asset_physical = asset.get("physicalPackage")
            if not all(
                isinstance(value, str) and value
                for value in (expected_physical, source_physical, asset_physical)
            ):
                raise ValueError(
                    f"resolved material has incomplete physical package identity: {source_path}"
                )
            physical_identities = {
                str(expected_physical).casefold(),
                str(source_physical).casefold(),
                str(asset_physical).casefold(),
            }
            if len(physical_identities) != 1 or asset_status != "RESOLVED_SOURCE_PACKAGE":
                raise ValueError(
                    f"resolved material physical package identity does not match: {source_path}"
                )
            exact_count += 1
        elif (
            status == "UNRESOLVED_MATERIAL_PATH"
            and asset_status == "ENGINE_DEFAULT_PARTICLE_FALLBACK"
            and asset.get("physicalPackage") is None
        ):
            explicit_fallback_count += 1
        else:
            raise ValueError(
                f"material catalog row is unresolved or ambiguous: {source_path} ({status})"
            )
        selected.append(material)

    action_documents = document.get("sourceActionDocuments", [])
    if not isinstance(action_documents, list):
        raise ValueError("resolved material catalog has invalid sourceActionDocuments")
    action_document_hashes = []
    for index, row in enumerate(action_documents):
        if not isinstance(row, dict):
            raise ValueError(f"sourceActionDocuments[{index}] is not an object")
        digest = row.get("sha256")
        if not isinstance(digest, str) or re.fullmatch(r"[0-9a-f]{64}", digest) is None:
            raise ValueError(f"sourceActionDocuments[{index}] has an invalid sha256")
        action_document_hashes.append(digest)

    provenance = {
        "schema": document["schema"],
        "formatVersion": document["formatVersion"],
        "characterClass": document["characterClass"],
        "bindingStatus": document["bindingStatus"],
        "catalogSha256": sha256_file(catalog_path),
        "sourceActionDocumentSha256": sorted(action_document_hashes),
        "requiredMaterialPathCount": len(selected),
        "resolvedExactSourcePackageCount": exact_count,
        "explicitEngineFallbackCount": explicit_fallback_count,
    }
    return selected, provenance


def resolve_runtime_resource_bindings(
    systems: list[dict[str, Any]],
    material_bindings: list[dict[str, Any]],
    resources_root: Path | None,
    effect_relative_root: str | None,
    runtime_cook_receipt_path: Path | None = None,
) -> list[dict[str, Any]]:
    if resources_root is None or effect_relative_root is None:
        return []
    effect_parts = [part for part in re.split(r"[\\/]", effect_relative_root) if part]
    effect_root = resources_root.joinpath(*effect_parts)
    rows = []
    cook_index: dict[str, list[str]] | None = None
    if runtime_cook_receipt_path is not None:
        cook_document = json.loads(
            runtime_cook_receipt_path.read_text(encoding="utf-8-sig")
        )
        cook_index = {}
        for asset in cook_document.get("assets", []):
            source_path = asset.get("sourceAssetPath")
            runtime_asset_id = asset.get("runtimeAssetId")
            if not source_path or not runtime_asset_id:
                continue
            cook_index.setdefault(str(source_path).casefold(), []).append(
                str(runtime_asset_id)
            )

    def resolve_from_cook_receipt(source_path: str) -> tuple[str, str | None, int]:
        if cook_index is None:
            raise ValueError("runtime cook receipt index is not available")
        candidate_ids = sorted(set(cook_index.get(source_path.casefold(), [])))
        existing_ids = [
            asset_id
            for asset_id in candidate_ids
            if resources_root.joinpath(*asset_id.split("/")).is_file()
        ]
        if len(existing_ids) == 1 and len(candidate_ids) == 1:
            return "RESOLVED_RUNTIME_ASSET", existing_ids[0], 1
        if len(candidate_ids) > 1:
            return "AMBIGUOUS_RUNTIME_ASSET", None, len(candidate_ids)
        return "MISSING_RUNTIME_ASSET", None, len(candidate_ids)

    meshes = sorted(
        {
            str(binding["objectPath"])
            for system in systems
            for binding in system["graph"]["resourceBindings"]
            if binding.get("role") == "mesh" and binding.get("objectPath")
        },
        key=str.casefold,
    )
    mesh_files = list((effect_root / "Meshes").glob("*.wmodel"))
    for source_path in meshes:
        if cook_index is not None:
            status, asset_id, candidate_count = resolve_from_cook_receipt(source_path)
        else:
            object_name = source_path.split(".")[-1]
            matches = [
                path
                for path in mesh_files
                if path.stem.casefold() == object_name.casefold()
            ]
            status = (
                "RESOLVED_RUNTIME_ASSET"
                if len(matches) == 1
                else "AMBIGUOUS_RUNTIME_ASSET"
                if matches
                else "MISSING_RUNTIME_ASSET"
            )
            asset_id = (
                matches[0].relative_to(resources_root).as_posix()
                if len(matches) == 1
                else None
            )
            candidate_count = len(matches)
        rows.append(
            {
                "role": "mesh",
                "sourceObjectPath": source_path,
                "resolutionStatus": status,
                "assetId": asset_id,
                "candidateCount": candidate_count,
            }
        )

    textures = sorted(
        {
            str(texture["texture"])
            for material in material_bindings
            if str(material.get("resolutionStatus", "")).startswith("RESOLVED_")
            for texture in material.get("textures", [])
            if texture.get("texture")
        },
        key=str.casefold,
    )
    for source_path in textures:
        if cook_index is not None:
            status, asset_id, candidate_count = resolve_from_cook_receipt(source_path)
        else:
            parts = source_path.split(".")
            source_package = parts[0]
            object_name = parts[-1]
            expected = (
                effect_root
                / "Textures"
                / source_package.upper()
                / f"{object_name}.dds"
            )
            status = (
                "RESOLVED_RUNTIME_ASSET"
                if expected.is_file()
                else "MISSING_RUNTIME_ASSET"
            )
            asset_id = (
                expected.relative_to(resources_root).as_posix()
                if expected.is_file()
                else None
            )
            candidate_count = 1 if expected.is_file() else 0
        rows.append(
            {
                "role": "texture",
                "sourceObjectPath": source_path,
                "resolutionStatus": status,
                "assetId": asset_id,
                "candidateCount": candidate_count,
            }
        )
    return rows


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--skill-id", type=int, required=True)
    parser.add_argument("--character-class", required=True)
    parser.add_argument("--input-slot", required=True)
    parser.add_argument("--skill-bindings", type=Path, required=True)
    parser.add_argument("--animnotify", type=Path, required=True)
    parser.add_argument("--graph", action="append", type=parse_graph_spec, required=True)
    parser.add_argument("--receipt-output", type=Path, required=True)
    parser.add_argument("--graph-output", type=Path, required=True)
    parser.add_argument("--material-map", type=Path)
    parser.add_argument("--source-package-manifest", type=Path)
    parser.add_argument("--resolved-material-catalog", type=Path)
    parser.add_argument("--runtime-resources-root", type=Path)
    parser.add_argument("--runtime-effect-relative-root")
    parser.add_argument("--runtime-cook-receipt", type=Path)
    parser.add_argument(
        "--combined-mesh-animation-status",
        default="ORIGINAL_MATCH_CONFIRMED",
        choices=("ORIGINAL_MATCH_CONFIRMED", "UNVERIFIED", "MISSING"),
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.resolved_material_catalog is not None and (
        args.material_map is not None or args.source_package_manifest is not None
    ):
        raise ValueError(
            "--resolved-material-catalog cannot be combined with "
            "--material-map or --source-package-manifest"
        )
    binding_document, bound_clips = load_bound_clips(args.skill_bindings, args.skill_id)
    clip_catalog = parse_animnotify(args.animnotify)
    clips, events, duration = build_timeline(bound_clips, clip_catalog, args.skill_id)
    graph_index = load_graphs(args.graph)

    systems_by_asset: dict[str, dict[str, Any]] = {}
    for event in events:
        if event["kind"] == "SHAKE" or event["sourceType"] == "Effect":
            event["resolutionStatus"] = "OUT_OF_EFFECT_DOCUMENT"
            continue
        if event["sourceType"] != "PlayParticleEffect" or not event["sourceAsset"]:
            event["resolutionStatus"] = "UNSUPPORTED_SOURCE_NOTIFY"
            continue

        parts = source_asset_parts(event["sourceAsset"])
        if parts is None:
            event["resolutionStatus"] = "UNRESOLVED_SOURCE_SYSTEM"
            continue
        package, object_path, object_name = parts
        root = find_particle_system(graph_index, package, object_path, object_name)
        if root is None:
            event["resolutionStatus"] = "UNRESOLVED_SOURCE_SYSTEM"
            continue

        event["resolutionStatus"] = "RESOLVED_PARTICLE_GRAPH"
        event["sourceSystemId"] = event["sourceAsset"].casefold()
        if event["sourceSystemId"] not in systems_by_asset:
            package_key, root_row = root
            systems_by_asset[event["sourceSystemId"]] = {
                "sourceSystemId": event["sourceSystemId"],
                "sourceAsset": event["sourceAsset"],
                "logicalPackage": graph_index["packages"][package_key]["package"],
                "objectName": root_row["objectName"],
                "objectPath": root_row.get("objectPath"),
                "graph": collect_system_graph(graph_index, package_key, root_row),
            }

    systems = sorted(systems_by_asset.values(), key=lambda row: row["sourceAsset"].casefold())
    detailed_nodes: dict[str, dict[str, Any]] = {}
    detailed_edges: list[dict[str, Any]] = []
    detailed_systems = []
    for system in systems:
        graph = system["graph"]
        detailed_nodes.update(graph["nodes"])
        detailed_edges.extend(
            {**edge, "sourceSystemId": system["sourceSystemId"]} for edge in graph["edges"]
        )
        detailed_systems.append(
            {
                **{key: value for key, value in system.items() if key != "graph"},
                "rootNodeId": graph["rootNodeId"],
                "nodeIds": graph["nodeIds"],
                "resourceBindings": graph["resourceBindings"],
                "unresolvedExternalReferences": graph["unresolvedExternalReferences"],
                "summary": graph["summary"],
            }
        )

    packages = source_package_receipts(graph_index)
    material_catalog_evidence = None
    if args.resolved_material_catalog is not None:
        material_bindings, material_catalog_evidence = load_resolved_material_catalog(
            systems,
            args.resolved_material_catalog,
            args.character_class,
            args.skill_id,
        )
    else:
        material_bindings = build_material_parameter_bindings(
            systems, args.material_map, args.source_package_manifest
        )
    resolved_material_count = sum(
        str(row["resolutionStatus"]).startswith("RESOLVED_") for row in material_bindings
    )
    runtime_resources = resolve_runtime_resource_bindings(
        systems,
        material_bindings,
        args.runtime_resources_root,
        args.runtime_effect_relative_root,
        args.runtime_cook_receipt,
    )
    resolved_runtime_resource_count = sum(
        row["resolutionStatus"] == "RESOLVED_RUNTIME_ASSET" for row in runtime_resources
    )
    graph_artifact = {
        "schema": "lostark.normalized-effect-source-graph",
        "schemaVersion": 1,
        "characterClass": args.character_class,
        "skillId": args.skill_id,
        "sourceSystems": detailed_systems,
        "nodes": [detailed_nodes[key] for key in sorted(detailed_nodes)],
        "edges": detailed_edges,
        "materialParameterBindings": material_bindings,
        "runtimeResourceBindings": runtime_resources,
        "summary": {
            "sourceSystemCount": len(systems),
            "uniqueNodeCount": len(detailed_nodes),
            "edgeOccurrenceCount": len(detailed_edges),
            "resourceBindingOccurrenceCount": sum(
                len(system["resourceBindings"]) for system in detailed_systems
            ),
            "unresolvedExternalReferenceOccurrenceCount": sum(
                len(system["unresolvedExternalReferences"]) for system in detailed_systems
            ),
            "uniqueMaterialBindingCount": len(material_bindings),
            "resolvedMaterialBindingCount": resolved_material_count,
            "runtimeResourceBindingCount": len(runtime_resources),
            "resolvedRuntimeResourceBindingCount": resolved_runtime_resource_count,
        },
    }
    if material_catalog_evidence is not None:
        graph_artifact["materialBindingCatalogEvidence"] = material_catalog_evidence

    compact_systems = [
        {
            **{key: value for key, value in system.items() if key != "graph"},
            "rootNodeId": system["graph"]["rootNodeId"],
            "summary": system["graph"]["summary"],
            "resourceBindings": system["graph"]["resourceBindings"],
        }
        for system in systems
    ]
    unsupported = compact_unresolved(systems, events)
    receipt = {
        "schema": "lostark.effect-source-receipt",
        "schemaVersion": 1,
        "characterClass": args.character_class,
        "inputSlot": args.input_slot,
        "skillId": args.skill_id,
        "animationAssetId": binding_document.get("animationAssetId"),
        "animationVisualContract": {
            "combinedMeshAnimationStatus": args.combined_mesh_animation_status,
            "combinedMeshAnimationOwner": "Animation/CModel",
            "reconstructCombinedMeshAsEffectElement": False,
            "effectExtractionScope": [
                "particle",
                "texture_or_sprite",
                "trail",
                "distortion",
                "post_effect",
                "particle_owned_mesh_emitter",
            ],
        },
        "timeline": {
            "durationSeconds": duration,
            "bindingClips": next(
                row["clips"]
                for row in binding_document.get("bindings", [])
                if int(row.get("skillId", -1)) == args.skill_id
            ),
            "clips": clips,
            "events": events,
        },
        "sourcePackages": packages,
        "sourceSystems": compact_systems,
        "materialParameterBindings": material_bindings,
        "runtimeResourceBindings": runtime_resources,
        "unsupportedUnresolved": unsupported,
        "conversionState": {
            "normalizedSourceGraph": "EXTRACTED",
            "materialInstanceParameterBindings": (
                "EXTRACTED"
                if material_bindings and resolved_material_count == len(material_bindings)
                else "PARTIAL"
                if material_bindings
                else "PENDING"
            ),
            "runtimeElementMapping": "PENDING",
            "importedEffectDocument": "PENDING",
        },
        "summary": {
            "sourceSystemCount": len(systems),
            "effectEventCount": sum(event["kind"] == "EFFECT" for event in events),
            "resolvedParticleEventCount": sum(
                event.get("resolutionStatus") == "RESOLVED_PARTICLE_GRAPH" for event in events
            ),
            "unsupportedNotifyCount": sum(
                event.get("resolutionStatus") == "UNSUPPORTED_SOURCE_NOTIFY" for event in events
            ),
            "unresolvedSourceSystemCount": sum(
                event.get("resolutionStatus") == "UNRESOLVED_SOURCE_SYSTEM" for event in events
            ),
            "relatedViewShakeCount": sum(event["kind"] == "SHAKE" for event in events),
            "unsupportedUnresolvedUniqueCount": len(unsupported),
            "uniqueMaterialBindingCount": len(material_bindings),
            "resolvedMaterialBindingCount": resolved_material_count,
            "runtimeResourceBindingCount": len(runtime_resources),
            "resolvedRuntimeResourceBindingCount": resolved_runtime_resource_count,
        },
    }
    if material_catalog_evidence is not None:
        receipt["materialBindingCatalogEvidence"] = material_catalog_evidence

    args.graph_output.parent.mkdir(parents=True, exist_ok=True)
    args.graph_output.write_text(
        json.dumps(graph_artifact, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    args.receipt_output.parent.mkdir(parents=True, exist_ok=True)
    args.receipt_output.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(
        json.dumps(
            {
                "receipt": str(args.receipt_output),
                "graph": str(args.graph_output),
                **receipt["summary"],
                **graph_artifact["summary"],
            },
            ensure_ascii=False,
        )
    )
    return 0 if not receipt["summary"]["unresolvedSourceSystemCount"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
