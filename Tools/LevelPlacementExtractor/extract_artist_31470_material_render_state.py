#!/usr/bin/env python3
"""Extract explicit Artist F Material render-state evidence from raw UE3 UPKs.

The checked receipt is intentionally derived from package bytes.  Omitted tagged
properties remain omitted evidence; this tool never substitutes engine defaults.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import Any

from extract_ue3_effect_material_closure import (
    decode_material_instance,
    find_export,
    load_package,
)
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    ExtractionError,
    Reader,
    decode_property_value,
    package_ref_name,
    package_ref_path,
    parse_fname,
)


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
PARSER_PATH = SCRIPT_PATH.with_name("extract_ue3_placements.py")

SOURCE_INSTANCE_FIELDS = (
    "parent",
    "scalarparametervalues",
    "vectorparametervalues",
    "textureparametervalues",
    "overridedtwosided",
    "bhasstaticpermutationresource",
)
RENDER_STATE_FIELDS = (
    "blendmode",
    "lightingmodel",
    "twosided",
    "bdisabledepthtest",
    "opacitymaskclipvalue",
    "buseonelayerdistortion",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def folded(value: Any) -> str:
    return str(value or "").casefold()


def reject_duplicate_object_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON object key: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream, object_pairs_hook=reject_duplicate_object_keys)
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def canonical_payload(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_payload(value)).hexdigest()


def raw_file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def normalize_tracked_text_bytes(content: bytes) -> bytes:
    require(not content.startswith(b"\xef\xbb\xbf"), "tracked JSON must not have a BOM")
    try:
        text = content.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("tracked JSON is not UTF-8") from error
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    try:
        json.loads(normalized, object_pairs_hook=reject_duplicate_object_keys)
    except json.JSONDecodeError as error:
        raise ValueError("tracked JSON is invalid") from error
    return normalized.encode("utf-8")


def tracked_source_text_sha256(path: Path) -> str:
    content = path.read_bytes()
    require(not content.startswith(b"\xef\xbb\xbf"), "tracked source must not have a BOM")
    try:
        text = content.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("tracked source is not UTF-8") from error
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    return hashlib.sha256(normalized.encode("utf-8")).hexdigest()


def tracked_json_text_sha256(path: Path) -> str:
    return hashlib.sha256(
        normalize_tracked_text_bytes(path.read_bytes())
    ).hexdigest()


def output_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            indent=2,
            sort_keys=False,
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def check_or_write_json(path: Path, value: dict[str, Any], check: bool) -> None:
    expected = output_bytes(value)
    if check:
        require(path.is_file(), f"generated output is missing: {path}")
        current = normalize_tracked_text_bytes(path.read_bytes())
        require(current == expected, f"generated output is stale: {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(expected)


def repository_path(path: Path) -> str:
    return path.resolve().relative_to(REPO_ROOT).as_posix()


def parse_property_records_at(
    serial: bytes, names: list[str], start_offset: int
) -> tuple[list[dict[str, Any]], int]:
    reader = Reader(serial, start_offset)
    records: list[dict[str, Any]] = []
    seen: set[tuple[str, int]] = set()
    while reader.offset < len(serial):
        tag_offset = reader.offset
        property_name, _ = parse_fname(reader, names)
        if folded(property_name) == "none":
            return records, reader.offset

        property_type, _ = parse_fname(reader, names)
        data_size = reader.i32()
        array_index = reader.i32()
        require(data_size >= 0, f"negative property size: {property_name}")
        identity = (folded(property_name), array_index)
        require(identity not in seen, f"duplicate tagged property: {identity}")
        seen.add(identity)

        struct_type: str | None = None
        bool_value: bool | None = None
        bool_value_offset: int | None = None
        type_key = folded(property_type)
        if type_key == "structproperty":
            struct_type, _ = parse_fname(reader, names)
        elif type_key == "boolproperty":
            bool_value_offset = reader.offset
            bool_value = bool(reader.read(1)[0])
        elif type_key == "byteproperty":
            parse_fname(reader, names)

        serialized_size = data_size + 8 if type_key == "intproperty" else data_size
        payload_offset = reader.offset
        payload = reader.read(serialized_size)
        record_end_offset = reader.offset
        value = decode_property_value(
            property_type,
            struct_type,
            payload,
            names,
            bool_value,
            property_name,
        )
        encoded_value = (
            serial[bool_value_offset : bool_value_offset + 1]
            if bool_value_offset is not None
            else payload
        )
        records.append(
            {
                "propertyName": property_name,
                "arrayIndex": array_index,
                "propertyType": property_type,
                "structType": struct_type,
                "declaredDataSize": data_size,
                "serializedPayloadSize": serialized_size,
                "tagOffset": tag_offset,
                "valueOffset": (
                    bool_value_offset
                    if bool_value_offset is not None
                    else payload_offset
                ),
                "recordEndOffset": record_end_offset,
                "value": value,
                "encodedValueSha256": hashlib.sha256(encoded_value).hexdigest(),
                "recordSha256": hashlib.sha256(
                    serial[tag_offset:record_end_offset]
                ).hexdigest(),
            }
        )
    raise ExtractionError("tagged property stream has no None terminator")


def parse_property_records(
    serial: bytes, names: list[str], package_version: int
) -> tuple[list[dict[str, Any]], int, int]:
    minimum_offset = 4 if package_version >= 322 else 0
    search_end = min(len(serial) - 20, 256)
    failures: list[str] = []
    for start_offset in range(minimum_offset, max(minimum_offset, search_end) + 1):
        try:
            name_index, name_number, type_index, type_number = struct.unpack_from(
                "<4i", serial, start_offset
            )
        except struct.error:
            break
        if (
            not 0 <= name_index < len(names)
            or name_number < 0
            or not 0 <= type_index < len(names)
            or type_number < 0
            or not folded(names[type_index]).endswith("property")
        ):
            continue
        try:
            records, end_offset = parse_property_records_at(
                serial, names, start_offset
            )
            return records, start_offset, end_offset
        except (ExtractionError, ValueError, struct.error) as error:
            failures.append(f"0x{start_offset:X}: {error}")
    detail = failures[0] if failures else "no plausible first property tag"
    raise ExtractionError(f"could not locate tagged properties ({detail})")


def package_file_index(root: Path) -> dict[str, Path]:
    require(root.is_dir(), f"source package root is missing: {root}")
    result: dict[str, Path] = {}
    for path in root.rglob("*.upk"):
        key = path.name.casefold()
        require(key not in result, f"duplicate physical UPK filename: {path.name}")
        result[key] = path
    require(bool(result), f"source package root contains no UPKs: {root}")
    return result


def field_evidence(
    records: list[dict[str, Any]],
    field_name: str,
    serial_offset: int,
    imports: list[Any],
    exports: list[Any],
) -> dict[str, Any]:
    matches = [
        row for row in records
        if folded(row.get("propertyName")) == field_name.casefold()
        and int(row.get("arrayIndex", -1)) == 0
    ]
    require(len(matches) <= 1, f"duplicate render-state field: {field_name}")
    if not matches:
        return {
            "propertyName": field_name,
            "status": "OMITTED_FROM_EXPORT",
            "fidelity": "UNRESOLVED_DEFAULT_PROVENANCE",
        }
    result = dict(matches[0])
    result["status"] = "SERIALIZED_EXPLICIT"
    result["fidelity"] = "SOURCE_EXACT_TAGGED_PROPERTY"
    for key in ("tagOffset", "valueOffset", "recordEndOffset"):
        result[f"absoluteLogical{key[0].upper()}{key[1:]}"] = (
            serial_offset + int(result[key])
        )
    value = result.get("value")
    if folded(result.get("propertyType")) in {
        "objectproperty",
        "componentproperty",
        "interfaceproperty",
    } and isinstance(value, int):
        result["resolvedObjectPath"] = (
            package_ref_path(value, imports, exports) if value else None
        )
    return result


def export_evidence(
    package: Any,
    object_path: str,
    expected_package_sha256: str,
    expected_export_index: int,
    expected_class_names: set[str],
    selected_fields: tuple[str, ...],
) -> dict[str, Any]:
    require(
        package.sha256 == expected_package_sha256,
        f"raw package SHA mismatch: {package.path.name}",
    )
    entry = find_export(package, object_path)
    class_name = package_ref_name(
        entry.class_index, package.imports, package.exports
    ) or ""
    resolved_path = package_ref_path(
        entry.index + 1, package.imports, package.exports
    )
    require(
        folded(class_name) in expected_class_names,
        f"unexpected export class for {object_path}: {class_name}",
    )
    require(
        entry.index == expected_export_index,
        f"export index changed for {object_path}: {entry.index}",
    )
    require(
        folded(resolved_path) == folded(object_path),
        f"export path changed: {resolved_path} != {object_path}",
    )
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    require(
        len(serial) == entry.serial_size,
        f"truncated export serial: {object_path}",
    )
    records, property_start, property_end = parse_property_records(
        serial, package.names, package.summary.version
    )
    evidence_id = "render-export-" + hashlib.sha256(
        f"{package.sha256}::{folded(resolved_path)}::{folded(class_name)}".encode()
    ).hexdigest()[:16]
    return {
        "evidenceId": evidence_id,
        "physicalPackage": package.path.name,
        "physicalPackageByteCount": package.path.stat().st_size,
        "physicalPackageSha256": package.sha256,
        "packageVersion": package.summary.version,
        "exportIndex": entry.index,
        "packageReference": entry.index + 1,
        "objectPath": resolved_path,
        "className": class_name,
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": hashlib.sha256(serial).hexdigest(),
        "propertyStreamStart": property_start,
        "propertyStreamEnd": property_end,
        "trailingByteCount": len(serial) - property_end,
        "fields": {
            name: field_evidence(
                records,
                name,
                entry.serial_offset,
                package.imports,
                package.exports,
            )
            for name in selected_fields
        },
    }


def require_material_rows(closure: dict[str, Any]) -> list[dict[str, Any]]:
    require(
        closure.get("schema") == "lostark.ue3-effect-material-closure"
        and closure.get("formatVersion") == 1,
        "unsupported active material closure",
    )
    raw_rows = closure.get("materials")
    require(isinstance(raw_rows, list), "material closure rows must be a list")
    rows = [row for row in raw_rows if isinstance(row, dict) and row.get("material") is not None]
    require(len(rows) == 27, f"rendered material denominator changed: {len(rows)}")
    keys = [folded(row.get("sourceMaterialPath")) for row in rows]
    require(all(keys), "rendered material path is blank")
    require(len(set(keys)) == len(keys), "duplicate rendered material path")
    return sorted(rows, key=lambda row: folded(row["sourceMaterialPath"]))


def build_receipt(
    closure_path: Path,
    source_package_root: Path,
    aes_key: str = LOSTARK_KR_AES_KEY,
) -> dict[str, Any]:
    closure = load_json(closure_path)
    rows = require_material_rows(closure)
    package_paths = package_file_index(source_package_root)
    package_cache: dict[str, Any] = {}
    exports_by_id: dict[str, dict[str, Any]] = {}
    bindings: list[dict[str, Any]] = []

    def package(physical_name: str) -> Any:
        key = physical_name.casefold()
        require(key in package_paths, f"physical UPK is missing: {physical_name}")
        if key not in package_cache:
            package_cache[key] = load_package(package_paths[key], aes_key)
        return package_cache[key]

    def add_export(value: dict[str, Any]) -> str:
        evidence_id = str(value["evidenceId"])
        previous = exports_by_id.get(evidence_id)
        require(
            previous is None or previous == value,
            f"render export evidence ID collision: {evidence_id}",
        )
        exports_by_id[evidence_id] = value
        return evidence_id

    for row in rows:
        source_path = str(row["sourceMaterialPath"])
        material = row.get("material")
        require(isinstance(material, dict), f"material payload missing: {source_path}")
        physical_name = str(row.get("sourcePhysicalPackage") or "")
        physical_sha = str(row.get("sourcePhysicalPackageSha256") or "")
        material_class = folded(material.get("className"))
        require(
            material_class in {"materialinstanceconstant", "material"},
            f"unsupported source material class: {source_path}",
        )
        source_fields = (
            SOURCE_INSTANCE_FIELDS
            if material_class == "materialinstanceconstant"
            else RENDER_STATE_FIELDS
        )
        source_export_index = material.get("exportIndex")
        if source_export_index is None and material_class == "material":
            material_graph = row.get("materialGraph")
            require(
                isinstance(material_graph, dict)
                and isinstance(material_graph.get("graph"), dict),
                f"source Material graph identity missing: {source_path}",
            )
            source_export_index = material_graph["graph"].get(
                "materialExportIndex"
            )
        source_export = export_evidence(
            package(physical_name),
            str(material.get("objectPath") or ""),
            physical_sha,
            int(source_export_index if source_export_index is not None else -1),
            {material_class},
            source_fields,
        )
        source_evidence_id = add_export(source_export)
        require(
            folded(source_export["className"]) == material_class,
            f"source material class mismatch: {source_path}",
        )
        if material_class == "materialinstanceconstant":
            decoded_instance = decode_material_instance(
                package(physical_name), str(material.get("objectPath") or "")
            )
            for closure_name in (
                "parent",
                "scalarParameters",
                "vectorParameters",
                "textureParameters",
            ):
                require(
                    decoded_instance.get(closure_name) == material.get(closure_name),
                    f"closure input disagrees with raw UPK: "
                    f"{source_path}.{closure_name}",
                )
            for field_name, closure_name in (
                ("overridedtwosided", "overrideTwoSided"),
                ("bhasstaticpermutationresource", "hasStaticPermutationResource"),
            ):
                raw_field = source_export["fields"][field_name]
                raw_value = (
                    raw_field.get("value")
                    if raw_field["status"] == "SERIALIZED_EXPLICIT"
                    else None
                )
                require(
                    raw_value == material.get(closure_name),
                    f"closure field disagrees with raw UPK: {source_path}.{closure_name}",
                )

        graph_holder = row.get("materialGraph") or row.get("parentGraph")
        require(isinstance(graph_holder, dict), f"base Material graph missing: {source_path}")
        graph = graph_holder.get("graph")
        require(isinstance(graph, dict), f"base Material graph payload missing: {source_path}")
        base_export = export_evidence(
            package(str(graph_holder.get("physicalPackage") or "")),
            str(graph.get("materialPath") or ""),
            str(graph_holder.get("physicalPackageSha256") or ""),
            int(graph.get("materialExportIndex", -1)),
            {"material", "decalmaterial"},
            RENDER_STATE_FIELDS,
        )
        base_evidence_id = add_export(base_export)
        bindings.append(
            {
                "sourceMaterialPath": source_path,
                "sourceExportEvidenceId": source_evidence_id,
                "renderStateExportEvidenceId": base_evidence_id,
                "renderStateOrigin": (
                    "SELF_MATERIAL"
                    if source_evidence_id == base_evidence_id
                    else "PARENT_MATERIAL"
                ),
            }
        )

    receipt: dict[str, Any] = {
        "schema": "lostark.artist-31470-material-render-state-evidence-receipt",
        "formatVersion": 1,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "source": {
            "activeMaterialClosure": {
                "path": repository_path(closure_path),
                "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_json_text_sha256(closure_path),
            },
            "sourcePackages": {
                "sourceRootId": "Resource_LostArk",
                "hashDomain": "RAW_ARTIFACT_BYTES",
            },
            "generator": {
                "path": repository_path(SCRIPT_PATH),
                "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_source_text_sha256(SCRIPT_PATH),
            },
            "rawPackageParser": {
                "path": repository_path(PARSER_PATH),
                "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_source_text_sha256(PARSER_PATH),
            },
            "reproductionCommand": [
                "python",
                repository_path(SCRIPT_PATH),
                "--material-closure",
                repository_path(closure_path),
                "--source-package-root",
                "{SOURCE_PACKAGE_ROOT}",
                "--output",
                "Data/Effects/Imported/Artist/Materials/skill.31470.material-render-state-evidence.receipt.json",
                "--check",
            ],
        },
        "bindings": bindings,
        "exports": sorted(
            exports_by_id.values(),
            key=lambda row: (
                folded(row["physicalPackage"]),
                folded(row["objectPath"]),
            ),
        ),
        "summary": {
            "materialRecipeCount": len(bindings),
            "uniqueRawExportCount": len(exports_by_id),
            "rawPackageCount": len(package_cache),
            "sourceMaterialInstanceCount": sum(
                folded(exports_by_id[row["sourceExportEvidenceId"]]["className"])
                == "materialinstanceconstant"
                for row in bindings
            ),
            "sourceRawMaterialCount": sum(
                folded(exports_by_id[row["sourceExportEvidenceId"]]["className"])
                == "material"
                for row in bindings
            ),
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--material-closure", required=True, type=Path)
    parser.add_argument("--source-package-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    receipt = build_receipt(
        args.material_closure, args.source_package_root, args.aes_key
    )
    check_or_write_json(args.output, receipt, args.check)
    print(
        "Artist F material render-state evidence "
        f"{'check' if args.check else 'write'}: "
        f"recipes={receipt['summary']['materialRecipeCount']} "
        f"exports={receipt['summary']['uniqueRawExportCount']} "
        f"packages={receipt['summary']['rawPackageCount']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ExtractionError, OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
