#!/usr/bin/env python3
"""Extract exact seeded-module and local-vector-field runtime semantics."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from extract_ue3_particle_graph import property_references
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


@dataclass
class ParsedPackage:
    logical_name: str
    path: Path
    logical: bytes
    summary: Any
    names: list[str]
    imports: list[Any]
    exports: list[Any]


def folded(value: Any) -> str:
    return str(value or "").casefold()


def property_value(properties: dict[str, Any], name: str) -> Any:
    wanted = name.casefold()
    for key, item in properties.items():
        if key.casefold() != wanted:
            continue
        if isinstance(item, dict) and "value" in item:
            return item["value"]
        return item
    return None


def nested_properties(value: Any) -> dict[str, Any]:
    if not isinstance(value, dict):
        return {}
    properties = value.get("properties")
    return properties if isinstance(properties, dict) else {}


def signed_word_to_float(value: int) -> float:
    return struct.unpack("<f", struct.pack("<i", int(value)))[0]


def encode_vector_field(
    size_x: int,
    size_y: int,
    size_z: int,
    words: list[int],
) -> bytes:
    sample_count = size_x * size_y * size_z
    component_count = sample_count * 3
    if min(size_x, size_y, size_z) <= 0:
        raise ValueError("vector field dimensions must be positive")
    if len(words) < component_count:
        raise ValueError(
            f"vector field payload is short: {len(words)} < {component_count}"
        )
    trailing = words[component_count:]
    if any(int(value) != 0 for value in trailing):
        raise ValueError("vector field has non-zero trailing source data")
    values = [signed_word_to_float(value) for value in words[:component_count]]
    if not all(math.isfinite(value) for value in values):
        raise ValueError("vector field contains a non-finite component")
    return (
        struct.pack(
            "<4sIIIII",
            b"WVF1",
            1,
            size_x,
            size_y,
            size_z,
            sample_count,
        )
        + struct.pack(f"<{component_count}f", *values)
    )


class SourcePack:
    def __init__(self, root: Path, aes_key: str) -> None:
        self.root = root
        self.aes_key = aes_key
        manifest = json.loads(
            (root / "source_pack_manifest.json").read_text(encoding="utf-8-sig")
        )
        self.rows = {
            folded(row.get("logicalPackage")): row
            for row in manifest.get("packages", [])
            if row.get("resolved") and row.get("relativePath")
        }
        self.cache: dict[str, ParsedPackage] = {}

    def load(self, logical_name: str) -> ParsedPackage:
        key = folded(logical_name)
        cached = self.cache.get(key)
        if cached is not None:
            return cached
        row = self.rows.get(key)
        if row is None:
            raise FileNotFoundError(f"source package is not staged: {logical_name}")
        path = self.root / str(row["relativePath"])
        physical = path.read_bytes()
        summary = parse_summary(physical)
        logical = decompress_package(physical, summary, self.aes_key)
        names = parse_name_table(logical, summary)
        imports = parse_import_table(logical, summary, names)
        exports = parse_export_table(logical, summary, names)
        package = ParsedPackage(
            logical_name=str(row["logicalPackage"]),
            path=path,
            logical=logical,
            summary=summary,
            names=names,
            imports=imports,
            exports=exports,
        )
        self.cache[key] = package
        return package

    def export_properties(
        self, logical_name: str, export_index: int
    ) -> tuple[ParsedPackage, Any, dict[str, Any]]:
        package = self.load(logical_name)
        if export_index < 0 or export_index >= len(package.exports):
            raise ValueError(
                f"export index is out of range: {logical_name}:{export_index}"
            )
        entry = package.exports[export_index]
        serial = package.logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        properties, end = parse_tagged_properties(
            serial, package.names, package.summary.version
        )
        if end != len(serial):
            raise ValueError(
                f"property stream has trailing bytes: {logical_name}:{export_index}"
            )
        return package, entry, properties

    def find_export(
        self, logical_name: str, relative_object_path: str
    ) -> tuple[ParsedPackage, Any, dict[str, Any]]:
        package = self.load(logical_name)
        wanted = folded(relative_object_path)
        for entry in package.exports:
            path = package_ref_path(
                entry.index + 1, package.imports, package.exports
            )
            if folded(path) == wanted:
                return self.export_properties(logical_name, entry.index)
        raise ValueError(
            f"source object is missing: {logical_name}.{relative_object_path}"
        )


def source_records(
    graph_paths: list[Path], closure_root: Path
) -> list[dict[str, Any]]:
    records: dict[str, dict[str, Any]] = {}
    for graph_path in graph_paths:
        graph = json.loads(graph_path.read_text(encoding="utf-8-sig"))
        for node in graph.get("nodes", []):
            class_name = folded(node.get("className"))
            if not (
                class_name.endswith("_seeded")
                or class_name == "particlemodulelocalvectorfield"
            ):
                continue
            logical_name = str(node.get("package") or "")
            object_path = str(node.get("objectPath") or "")
            full_path = f"{logical_name}.{object_path}"
            records[folded(full_path)] = {
                "logicalPackage": logical_name,
                "objectPath": object_path,
                "fullObjectPath": full_path,
                "className": class_name,
                "exportIndex": int(node["exportIndex"]),
            }
    for closure_path in sorted(closure_root.glob("*.external-module-closure.json")):
        closure = json.loads(closure_path.read_text(encoding="utf-8-sig"))
        for package in closure.get("packages", []):
            logical_name = str(package.get("logicalPackage") or "")
            for row in package.get("objects", []):
                class_name = folded(row.get("className"))
                if not (
                    class_name.endswith("_seeded")
                    or class_name == "particlemodulelocalvectorfield"
                ):
                    continue
                object_path = str(row.get("objectPath") or "")
                full_path = f"{logical_name}.{object_path}"
                records[folded(full_path)] = {
                    "logicalPackage": logical_name,
                    "objectPath": object_path,
                    "fullObjectPath": full_path,
                    "className": class_name,
                    "exportIndex": int(row["exportIndex"]),
                }
    return [records[key] for key in sorted(records)]


def extract(
    graph_paths: list[Path],
    closure_root: Path,
    source_pack_root: Path,
    resource_root: Path,
    resource_prefix: str,
    aes_key: str = LOSTARK_KR_AES_KEY,
) -> dict[str, Any]:
    source_pack = SourcePack(source_pack_root, aes_key)
    modules: dict[str, dict[str, Any]] = {}
    vector_fields: dict[str, dict[str, Any]] = {}
    resource_root.mkdir(parents=True, exist_ok=True)

    for record in source_records(graph_paths, closure_root):
        package, entry, properties = source_pack.find_export(
            record["logicalPackage"], record["objectPath"]
        )
        actual_path = package_ref_path(
            entry.index + 1, package.imports, package.exports
        )
        if folded(actual_path) != folded(record["objectPath"]):
            raise ValueError(
                "source export identity mismatch: "
                f"{record['fullObjectPath']} != {package.logical_name}.{actual_path}"
            )
        overlay: dict[str, Any] = {}
        if record["className"].endswith("_seeded"):
            seed_info = nested_properties(property_value(properties, "randomseedinfo"))
            seeds = property_value(seed_info, "randomseeds")
            overlay["randomSeeds"] = (
                [int(seed) for seed in seeds]
                if isinstance(seeds, list) else []
            )
            overlay["resetSeedOnEmitterLooping"] = bool(
                property_value(seed_info, "bresetseedonemitterlooping")
                if property_value(seed_info, "bresetseedonemitterlooping") is not None
                else True
            )
            overlay["randomlySelectSeedArray"] = bool(
                property_value(seed_info, "brandomlyselectseedarray") or False
            )
            overlay["getSeedFromInstance"] = bool(
                property_value(seed_info, "bgetseedfrominstance") or False
            )
            overlay["instanceSeedIsIndex"] = bool(
                property_value(seed_info, "binstanceseedisindex") or False
            )

        if record["className"] == "particlemodulelocalvectorfield":
            references = property_references(
                properties, package.imports, package.exports
            )
            vector_reference = next(
                (
                    row for row in references
                    if folded(row.get("property")) == "vectorfield"
                ),
                None,
            )
            if vector_reference is None:
                raise ValueError(
                    f"local vector field has no VectorField reference: {record['fullObjectPath']}"
                )
            vector_object_path = str(vector_reference["objectPath"])
            logical_name, separator, relative_path = vector_object_path.partition(".")
            if not separator:
                logical_name = package.logical_name
                relative_path = vector_object_path
                vector_object_path = f"{logical_name}.{relative_path}"
            vector_key = folded(vector_object_path)
            vector_row = vector_fields.get(vector_key)
            if vector_row is None:
                vector_package, vector_entry, vector_properties = source_pack.find_export(
                    logical_name, relative_path
                )
                vector_class = package_ref_name(
                    vector_entry.class_index,
                    vector_package.imports,
                    vector_package.exports,
                )
                if folded(vector_class) != "vectorfield":
                    raise ValueError(
                        f"referenced object is not VectorField: {vector_object_path}"
                    )
                size_x = int(property_value(vector_properties, "sizex"))
                size_y = int(property_value(vector_properties, "sizey"))
                size_z = int(property_value(vector_properties, "sizez"))
                words = property_value(vector_properties, "vectordata")
                if not isinstance(words, list):
                    raise ValueError(
                        f"VectorData is not decoded: {vector_object_path}"
                    )
                binary = encode_vector_field(size_x, size_y, size_z, words)
                digest = hashlib.sha256(binary).hexdigest()
                safe_package = "".join(
                    character if character.isalnum() else "_"
                    for character in logical_name.casefold()
                )
                file_name = f"{safe_package}.{digest[:16]}.wvectorfield"
                output_path = resource_root / file_name
                temporary = output_path.with_suffix(output_path.suffix + ".tmp")
                temporary.write_bytes(binary)
                temporary.replace(output_path)
                asset_id = f"{resource_prefix.rstrip('/')}/{file_name}"
                vector_row = {
                    "sourceObjectPath": vector_object_path,
                    "assetId": asset_id,
                    "size": [size_x, size_y, size_z],
                    "sampleCount": size_x * size_y * size_z,
                    "sha256": digest,
                    "sourcePackage": str(vector_package.path),
                    "sourceExportIndex": vector_entry.index,
                }
                vector_fields[vector_key] = vector_row
            overlay["vectorFieldObjectPath"] = vector_object_path
            overlay["vectorFieldAssetId"] = vector_row["assetId"]

        if overlay:
            modules[folded(record["fullObjectPath"])] = overlay

    return {
        "schema": "lostark.ue3-effect-semantic-overlay",
        "formatVersion": 1,
        "modules": modules,
        "vectorFields": [vector_fields[key] for key in sorted(vector_fields)],
        "summary": {
            "seededModuleCount": sum("randomSeeds" in row for row in modules.values()),
            "localVectorFieldModuleCount": sum(
                "vectorFieldAssetId" in row for row in modules.values()
            ),
            "vectorFieldAssetCount": len(vector_fields),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--closure-root", type=Path, required=True)
    parser.add_argument("--source-pack-root", type=Path, required=True)
    parser.add_argument("--resource-root", type=Path, required=True)
    parser.add_argument(
        "--resource-prefix",
        default="Effect/DimensionMaster/VectorFields",
    )
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8-sig"))
    graph_paths = [Path(value) for value in manifest.get("sourceGraphs", [])]
    result = extract(
        graph_paths,
        args.closure_root,
        args.source_pack_root,
        args.resource_root,
        args.resource_prefix,
        args.aes_key,
    )
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
