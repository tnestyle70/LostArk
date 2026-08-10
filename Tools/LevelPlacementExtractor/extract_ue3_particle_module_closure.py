#!/usr/bin/env python3
"""Extract the external UE3 particle modules referenced by one normalized skill graph."""

from __future__ import annotations

import argparse
import json
from collections import defaultdict, deque
from pathlib import Path

from extract_ue3_particle_graph import (
    is_particle_graph_class,
    property_references,
)
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    decompress_package,
    package_ref_name,
    package_ref_path,
    parse_export_table,
    parse_import_table,
    parse_name_table,
    parse_summary,
    parse_tagged_properties,
)


SUBSTITUTIONS = {
    "Q": ("QP", "QD", "QW", "Q4"),
    "-": ("QL", "QB", "QO", "Q5"),
    "_": ("QC", "QN", "QT", "Q9"),
    "X": ("XU", "XN", "XH", "X3"),
    "!": ("XW", "XS", "XZ", "X0"),
}


def escape_package_name(value: str) -> str:
    result = ""
    for character in value.upper():
        choices = SUBSTITUTIONS.get(character)
        result += choices[len(result) % 4] if choices else character
    return result


def obfuscate_package_name(logical_name: str) -> str:
    """Return the physical Lost Ark package stem for a logical package name."""

    unpadded_length = len(logical_name)
    escaped = escape_package_name(logical_name)
    if unpadded_length < 20:
        escaped = escape_package_name(
            logical_name + "!" + "." * (20 - unpadded_length - 1)
        )
    characters = list(escaped)
    output = []
    length = len(characters)
    for index, character in enumerate(characters):
        value = ord(character)
        if "0" <= character <= "9":
            value += 43
        encoded = (length + 7 * (value - 65)) % 36 + 65
        if encoded >= 91:
            encoded -= 43
        encoded_character = chr(encoded)
        output.append(encoded_character)
        padding_index = index + unpadded_length
        if padding_index < length and characters[padding_index] == ".":
            characters[padding_index] = encoded_character
    return "".join(output)


def base_property_name(value: str) -> str:
    return value.split("[", 1)[0].casefold()


def outgoing_edges(graph: dict) -> dict[str, list[dict]]:
    result: dict[str, list[dict]] = defaultdict(list)
    for edge in graph.get("edges", []):
        result[str(edge["sourceNodeId"])].append(edge)
    for rows in result.values():
        rows.sort(key=lambda row: int(row.get("referenceIndex", 0)))
    return result


def select_lod0_node_ids(graph: dict) -> set[str]:
    outgoing = outgoing_edges(graph)
    selected: set[str] = set()
    for system in graph.get("sourceSystems", []):
        root_node_id = str(system["rootNodeId"])
        emitter_edges = [
            row
            for row in outgoing.get(root_node_id, [])
            if base_property_name(str(row.get("property", ""))) == "emitters"
        ]
        for emitter_edge in emitter_edges:
            emitter_node_id = emitter_edge.get("targetNodeId")
            if not emitter_node_id:
                continue
            lod_edges = [
                row
                for row in outgoing.get(str(emitter_node_id), [])
                if base_property_name(str(row.get("property", ""))) == "lodlevels"
                and row.get("targetNodeId")
            ]
            if lod_edges:
                selected.add(str(lod_edges[0]["targetNodeId"]))
    return selected


def collect_external_lod0_requests(graph: dict) -> dict[str, list[dict]]:
    selected_lods = select_lod0_node_ids(graph)
    grouped: dict[str, list[dict]] = defaultdict(list)
    seen: set[tuple[str, str, str]] = set()
    for system in graph.get("sourceSystems", []):
        for row in system.get("unresolvedExternalReferences", []):
            if (
                str(row.get("sourceNodeId")) not in selected_lods
                or row.get("reason") != "external_graph_package_not_loaded"
            ):
                continue
            object_path = str(row.get("objectPath") or "")
            logical_package, separator, relative_path = object_path.partition(".")
            if not separator:
                continue
            key = (
                str(row["sourceNodeId"]),
                str(row.get("property", "")),
                object_path.casefold(),
            )
            if key in seen:
                continue
            seen.add(key)
            grouped[logical_package.casefold()].append(
                {
                    "sourceNodeId": str(row["sourceNodeId"]),
                    "referenceIndex": int(row.get("referenceIndex", 0)),
                    "property": str(row.get("property", "")),
                    "sourceSystemId": str(system["sourceSystemId"]),
                    "objectPath": object_path,
                    "relativeObjectPath": relative_path,
                }
            )
    return grouped


def extract_requested_package(
    package_root: Path,
    logical_package: str,
    requests: list[dict],
    aes_key: str,
    seed_packages: dict[str, dict] | None = None,
) -> dict:
    physical_path = package_root / f"{obfuscate_package_name(logical_package)}.upk"
    if not physical_path.is_file():
        seed = (seed_packages or {}).get(logical_package.casefold())
        seed_objects = list(seed.get("objects", [])) if seed else []
        available = {
            str(row.get("objectPath", "")).casefold()
            for row in seed_objects
        }
        unresolved = [
            row for row in requests
            if str(row["relativeObjectPath"]).casefold() not in available
        ]
        return {
            "logicalPackage": logical_package.upper(),
            "physicalPackage": (
                str(seed.get("physicalPackage")) if seed
                else physical_path.name
            ),
            "resolutionSource": "SEED_CLOSURE" if seed else "MISSING_PACKAGE",
            "requestedReferences": requests,
            "unresolvedRequestedReferences": unresolved,
            "objects": seed_objects,
            "propertyErrors": [],
            "summary": {
                "requestCount": len(requests),
                "resolvedRequestCount": len(requests) - len(unresolved),
                "unresolvedRequestCount": len(unresolved),
                "closureObjectCount": len(seed_objects),
                "propertyErrorCount": 0,
            },
        }

    physical = physical_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)

    path_to_export: dict[str, object] = {}
    graph_export_indexes: set[int] = set()
    for entry in exports:
        class_name = package_ref_name(entry.class_index, imports, exports)
        if not is_particle_graph_class(class_name):
            continue
        object_path = package_ref_path(entry.index + 1, imports, exports)
        if object_path:
            path_to_export[object_path.casefold()] = entry
            graph_export_indexes.add(entry.index)

    requested_entries = []
    unresolved = []
    for request in requests:
        relative_path = str(request["relativeObjectPath"]).casefold()
        entry = path_to_export.get(relative_path)
        if entry is None:
            unresolved.append(request)
        else:
            requested_entries.append(entry)

    queue = deque(entry.index for entry in requested_entries)
    visited: set[int] = set()
    objects = []
    errors = []
    while queue:
        export_index = queue.popleft()
        if export_index in visited:
            continue
        visited.add(export_index)
        entry = exports[export_index]
        class_name = package_ref_name(entry.class_index, imports, exports)
        serial_data = logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        try:
            properties, property_end = parse_tagged_properties(
                serial_data, names, summary.version
            )
            references = property_references(properties, imports, exports)
        except Exception as error:
            errors.append(
                {
                    "exportIndex": entry.index,
                    "className": class_name,
                    "objectName": entry.object_name,
                    "error": str(error),
                }
            )
            continue

        objects.append(
            {
                "objectId": f"{logical_package.upper()}:export:{entry.index}",
                "exportIndex": entry.index,
                "className": class_name,
                "classPath": package_ref_path(entry.class_index, imports, exports),
                "objectName": entry.object_name,
                "objectPath": package_ref_path(entry.index + 1, imports, exports),
                "archetypeIndex": entry.archetype_index,
                "archetypePath": package_ref_path(
                    entry.archetype_index, imports, exports
                ),
                "properties": properties,
                "references": references,
                "requestedDirectly": entry in requested_entries,
                "propertyStreamEnd": property_end,
            }
        )
        for reference in references:
            package_index = int(reference["packageIndex"])
            if package_index <= 0:
                continue
            referenced_export_index = package_index - 1
            if referenced_export_index in graph_export_indexes:
                queue.append(referenced_export_index)

    objects.sort(key=lambda row: int(row["exportIndex"]))
    return {
        "logicalPackage": logical_package.upper(),
        "physicalPackage": physical_path.name,
        "resolutionSource": "SOURCE_PACKAGE",
        "requestedReferences": requests,
        "unresolvedRequestedReferences": unresolved,
        "objects": objects,
        "propertyErrors": errors,
        "summary": {
            "requestCount": len(requests),
            "resolvedRequestCount": len(requests) - len(unresolved),
            "unresolvedRequestCount": len(unresolved),
            "closureObjectCount": len(objects),
            "propertyErrorCount": len(errors),
        },
    }


def build_closure(
    graph: dict,
    package_root: Path,
    aes_key: str = LOSTARK_KR_AES_KEY,
    seed_closures: list[dict] | None = None,
) -> dict:
    grouped = collect_external_lod0_requests(graph)
    seed_packages: dict[str, dict] = {}
    for closure in seed_closures or []:
        for package in closure.get("packages", []):
            logical_package = str(package.get("logicalPackage", "")).casefold()
            if logical_package and logical_package not in seed_packages:
                seed_packages[logical_package] = package
    packages = []
    for logical_package, requests in sorted(grouped.items()):
        packages.append(
            extract_requested_package(
                package_root, logical_package, requests, aes_key, seed_packages
            )
        )
    return {
        "schema": "lostark.ue3-particle-external-module-closure",
        "schemaVersion": 1,
        "characterClass": graph.get("characterClass"),
        "skillId": graph.get("skillId"),
        "lodPolicy": "FIRST_LOD_ONLY",
        "packages": packages,
        "summary": {
            "packageCount": len(packages),
            "requestCount": sum(
                row["summary"]["requestCount"] for row in packages
            ),
            "resolvedRequestCount": sum(
                row["summary"]["resolvedRequestCount"] for row in packages
            ),
            "unresolvedRequestCount": sum(
                row["summary"]["unresolvedRequestCount"] for row in packages
            ),
            "closureObjectCount": sum(
                row["summary"]["closureObjectCount"] for row in packages
            ),
            "propertyErrorCount": sum(
                row["summary"]["propertyErrorCount"] for row in packages
            ),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--package-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument(
        "--seed-closure",
        action="append",
        default=[],
        type=Path,
        help=(
            "Previously audited closure used only when its physical UPK is no "
            "longer present; every requested object path must still resolve."
        ),
    )
    args = parser.parse_args()

    graph = json.loads(args.normalized_graph.read_text(encoding="utf-8-sig"))
    seed_closures = [
        json.loads(path.read_text(encoding="utf-8-sig"))
        for path in args.seed_closure
    ]
    result = build_closure(
        graph, args.package_root, args.aes_key, seed_closures
    )
    if result["summary"]["unresolvedRequestCount"]:
        raise ValueError(
            "external particle module closure has unresolved requested references"
        )
    if result["summary"]["propertyErrorCount"]:
        raise ValueError("external particle module closure has property errors")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    temporary = args.output.with_suffix(args.output.suffix + ".tmp")
    temporary.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(args.output)
    print(json.dumps({"output": str(args.output), **result["summary"]}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
