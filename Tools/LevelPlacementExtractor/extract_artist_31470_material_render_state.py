#!/usr/bin/env python3
"""Extract explicit Artist F Material render-state evidence from raw UE3 UPKs.

The checked receipt is intentionally derived from package bytes.  Omitted tagged
properties remain omitted evidence; this tool never substitutes engine defaults.
"""

from __future__ import annotations

import argparse
import copy
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
MATERIAL_CLOSURE_PARSER_PATH = SCRIPT_PATH.with_name(
    "extract_ue3_effect_material_closure.py"
)
MATERIAL_GRAPH_PARSER_PATH = SCRIPT_PATH.with_name("extract_ue3_material_graph.py")
CONTRACT_ROOT = "ARTIST/31470/F"
EXPECTED_SOURCE_PACK_MANIFEST_BYTE_COUNT = 270014
EXPECTED_SOURCE_PACK_MANIFEST_SHA256 = (
    "8ddce11f3cdd36efc4098b127da860b3e77e0f6916263412f1089cce3967d62d"
)

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
BASE_MATERIAL_FIELDS = (*RENDER_STATE_FIELDS, "expressions")
EXPRESSION_PARAMETER_FIELDS = (
    "parametername",
    "group",
    "defaultvalue",
    "texture",
)
TEXTURE_SAMPLER_FIELDS = ("addressx", "addressy", "srgb")
BLEND_MODE_DOMAIN = (
    "blend_opaque",
    "blend_masked",
    "blend_translucent",
    "blend_additive",
    "blend_modulate",
    "blend_softmasked",
    "blend_alphacomposite",
    "blend_ditheredtranslucent",
    "blend_max",
)
LIGHTING_MODEL_DOMAIN = (
    "mlm_phong",
    "mlm_nondirectional",
    "mlm_unlit",
    "mlm_shprtdiffuse",
    "mlm_custom",
    "mlm_anisotropic",
    "mlm_max",
)
TEXTURE_ADDRESS_DOMAIN = ("ta_wrap", "ta_clamp", "ta_mirror")


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


def exact_json_int(value: Any, expected: int, label: str) -> int:
    require(type(value) is int and value == expected, f"invalid exact integer: {label}")
    return value


def source_manifest_packages(
    manifest_path: Path,
) -> tuple[dict[tuple[str, str], str], dict[str, Any]]:
    require(manifest_path.is_file(), f"source-pack manifest is missing: {manifest_path}")
    manifest_bytes = manifest_path.read_bytes()
    require(
        len(manifest_bytes) == EXPECTED_SOURCE_PACK_MANIFEST_BYTE_COUNT
        and hashlib.sha256(manifest_bytes).hexdigest()
        == EXPECTED_SOURCE_PACK_MANIFEST_SHA256,
        "source-pack manifest raw bytes changed",
    )
    manifest = load_json(manifest_path)
    exact_json_int(manifest.get("schemaVersion"), 1, "source-pack manifest schemaVersion")
    rows = manifest.get("packages")
    require(isinstance(rows, list), "source-pack manifest packages must be a list")
    result: dict[tuple[str, str], str] = {}
    logical_owners: dict[str, tuple[str, str]] = {}
    for row in rows:
        require(isinstance(row, dict), "source-pack manifest package must be an object")
        if row.get("resolved") is not True:
            continue
        physical = str(row.get("physicalPackage") or "")
        package_sha256 = str(row.get("sha256") or "")
        logical = str(row.get("logicalPackage") or "")
        require(
            bool(physical) and bool(logical) and len(package_sha256) == 64,
            "resolved source-pack manifest identity is incomplete",
        )
        key = (physical.casefold(), package_sha256)
        require(key not in result, f"duplicate manifest physical/SHA identity: {physical}")
        logical_key = logical.casefold()
        require(
            logical_key not in logical_owners,
            f"duplicate manifest logical package: {logical}",
        )
        result[key] = logical
        logical_owners[logical_key] = key
    require(bool(result), "source-pack manifest has no resolved packages")
    evidence = {
        "pathHint": manifest_path.name,
        "hashDomain": "RAW_ARTIFACT_BYTES",
        "byteCount": len(manifest_bytes),
        "sha256": EXPECTED_SOURCE_PACK_MANIFEST_SHA256,
        "schemaVersion": 1,
        "identityStatus": "AUTHENTICATED_IMMUTABLE_FIXTURE",
    }
    return result, evidence


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
                "encodedValueHex": encoded_value.hex(),
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


def canonical_path_targets_object(
    canonical_path: Any, object_path: Any, logical_package: Any
) -> bool:
    canonical = folded(canonical_path)
    target = folded(object_path)
    logical = folded(logical_package)
    return bool(canonical and target and logical) and canonical in {
        target,
        f"{logical}.{target}",
    }


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
    enum_domains = {
        "blendmode": BLEND_MODE_DOMAIN,
        "lightingmodel": LIGHTING_MODEL_DOMAIN,
        "addressx": TEXTURE_ADDRESS_DOMAIN,
        "addressy": TEXTURE_ADDRESS_DOMAIN,
    }
    domain = enum_domains.get(field_name.casefold())
    if domain is not None:
        normalized_value = folded(value)
        require(
            normalized_value in domain,
            f"enum value is outside pinned domain: {field_name}={value}",
        )
        result["enumDomain"] = list(domain)
        result["enumOrdinal"] = domain.index(normalized_value)
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


def nested_property_value(value: Any, name: str) -> Any:
    if not isinstance(value, dict):
        return None
    properties = value.get("properties")
    if not isinstance(properties, dict):
        return None
    wanted = name.casefold()
    for key, item in properties.items():
        if key.casefold() == wanted and isinstance(item, dict):
            return item.get("value")
    return None


def raw_expression_inputs(
    records: list[dict[str, Any]], imports: list[Any], exports: list[Any]
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for record in records:
        reference = nested_property_value(record.get("value"), "expression")
        if not isinstance(reference, int):
            continue
        row: dict[str, Any] = {
            "input": record["propertyName"],
            "packageIndex": reference,
        }
        for name in ("outputindex", "mask", "maskr", "maskg", "maskb", "maska"):
            value = nested_property_value(record.get("value"), name)
            if isinstance(value, (bool, int, float)):
                row[name] = value
        row["objectPath"] = (
            package_ref_path(reference, imports, exports) if reference else None
        )
        row["propertyRecordSha256"] = record["recordSha256"]
        result.append(row)
    return result


def expression_evidence(
    package: Any,
    base_material_evidence_id: str,
    source_order: int,
    package_reference: int,
    expected: dict[str, Any],
) -> dict[str, Any]:
    require(package_reference > 0, "expression evidence cannot target a null reference")
    require(
        int(expected.get("sourceOrder", -1)) == source_order,
        "expression source order changed",
    )
    require(
        int(expected.get("exportIndex", -1)) + 1 == package_reference,
        f"expression package reference changed at source order {source_order}",
    )
    entry = package.exports[package_reference - 1]
    class_name = package_ref_name(
        entry.class_index, package.imports, package.exports
    ) or ""
    object_path = package_ref_path(
        package_reference, package.imports, package.exports
    )
    require(
        folded(class_name).startswith("materialexpression"),
        f"graph expression class is invalid: {class_name}",
    )
    require(
        folded(class_name) == folded(expected.get("className"))
        and folded(object_path) == folded(expected.get("objectPath")),
        f"graph expression identity changed: {object_path}",
    )
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    records, property_start, property_end = parse_property_records(
        serial, package.names, package.summary.version
    )
    fields = {
        name: field_evidence(
            records,
            name,
            entry.serial_offset,
            package.imports,
            package.exports,
        )
        for name in EXPRESSION_PARAMETER_FIELDS
    }
    inputs = raw_expression_inputs(records, package.imports, package.exports)
    texture_field = fields["texture"]
    texture_reference = (
        int(texture_field["value"])
        if texture_field["status"] == "SERIALIZED_EXPLICIT"
        else 0
    )
    projection = {
        "sourceOrder": source_order,
        "exportIndex": entry.index,
        "className": class_name,
        "objectPath": object_path,
        "parameterName": (
            fields["parametername"].get("value")
            if fields["parametername"]["status"] == "SERIALIZED_EXPLICIT"
            else None
        ),
        "group": (
            fields["group"].get("value")
            if fields["group"]["status"] == "SERIALIZED_EXPLICIT"
            else None
        ),
        "defaultValue": (
            fields["defaultvalue"].get("value")
            if fields["defaultvalue"]["status"] == "SERIALIZED_EXPLICIT"
            else None
        ),
        "texturePackageIndex": texture_reference,
        "textureObjectPath": (
            texture_field.get("resolvedObjectPath") if texture_reference else None
        ),
        "inputs": inputs,
        "serialSize": entry.serial_size,
        "propertyStreamEnd": property_end,
    }
    comparable_expected = {
        "sourceOrder": expected.get("sourceOrder"),
        "exportIndex": expected.get("exportIndex"),
        "className": expected.get("className"),
        "objectPath": expected.get("objectPath"),
        "parameterName": expected.get("parameterName"),
        "group": expected.get("group"),
        "defaultValue": expected.get("defaultValue"),
        "textureObjectPath": expected.get("textureObjectPath"),
        "inputs": expected.get("inputs"),
        "serialSize": expected.get("serialSize"),
        "propertyStreamEnd": expected.get("propertyStreamEnd"),
    }
    comparable_projection = {
        key: value for key, value in projection.items()
        if key != "texturePackageIndex"
    }
    comparable_projection["inputs"] = [
        {
            key: value for key, value in row.items()
            if key != "propertyRecordSha256"
        }
        for row in inputs
    ]
    require(
        comparable_projection == comparable_expected,
        f"closure expression projection disagrees with raw UPK: {object_path}",
    )
    return {
        "evidenceId": stable_expression_evidence_id(
            package.sha256, object_path, source_order
        ),
        "baseMaterialEvidenceId": base_material_evidence_id,
        "physicalPackage": package.path.name,
        "physicalPackageSha256": package.sha256,
        "packageVersion": package.summary.version,
        "sourceOrder": source_order,
        "rawReferenceFromBaseExpressions": package_reference,
        "exportIndex": entry.index,
        "packageReference": package_reference,
        "objectPath": object_path,
        "className": class_name,
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": hashlib.sha256(serial).hexdigest(),
        "propertyStreamStart": property_start,
        "propertyStreamEnd": property_end,
        "fields": fields,
        "projection": projection,
    }


def stable_expression_evidence_id(
    package_sha256: str, object_path: str, source_order: int
) -> str:
    payload = f"{package_sha256}::{folded(object_path)}::{source_order}".encode()
    return "graph-expression-" + hashlib.sha256(payload).hexdigest()[:16]


def texture_sampler_evidence(
    asset: dict[str, Any], package: Any
) -> dict[str, Any]:
    logical_path = str(asset.get("logicalObjectPath") or "")
    texture = asset.get("sourceTexture2D")
    dds = asset.get("dds")
    require(
        isinstance(texture, dict) and isinstance(dds, dict),
        f"DDS texture evidence is incomplete: {logical_path}",
    )
    relative_path = logical_path.split(".", 1)[1]
    export = export_evidence(
        package,
        relative_path,
        str(texture.get("physicalPackageSha256") or ""),
        int(texture.get("exportIndex", -1)),
        {"texture2d"},
        TEXTURE_SAMPLER_FIELDS,
    )
    require(
        folded(texture.get("className")) == "texture2d"
        and int(texture.get("packageReference", -1))
        == int(export["packageReference"])
        and int(texture.get("serialOffset", -1)) == int(export["serialOffset"])
        and int(texture.get("serialSize", -1)) == int(export["serialSize"])
        and texture.get("serialSha256") == export["serialSha256"],
        f"DDS Texture2D export identity disagrees with raw UPK: {logical_path}",
    )
    sampling = texture.get("sampling")
    require(isinstance(sampling, dict), f"DDS sampler projection missing: {logical_path}")
    normalized: dict[str, Any] = {}
    for axis, field_name, result_name, evidence_name in (
        ("U", "addressx", "addressU", "addressUEvidence"),
        ("V", "addressy", "addressV", "addressVEvidence"),
    ):
        field = export["fields"][field_name]
        if field["status"] == "SERIALIZED_EXPLICIT":
            address = folded(field["value"]).removeprefix("ta_")
            evidence = "SERIALIZED_PROPERTY_EXACT"
        else:
            address = "wrap"
            evidence = "UE3_TEXTURE_CLASS_DEFAULT"
        require(
            sampling.get(result_name) == address
            and sampling.get(evidence_name) == evidence,
            f"DDS sampler {axis} projection disagrees with raw UPK: {logical_path}",
        )
        normalized[result_name] = address
        normalized[evidence_name] = evidence
    srgb_field = export["fields"]["srgb"]
    if srgb_field["status"] == "SERIALIZED_EXPLICIT":
        color_space = "srgb" if srgb_field["value"] else "linear"
        color_evidence = "SERIALIZED_PROPERTY_EXACT"
    else:
        color_space = "srgb"
        color_evidence = "UE3_TEXTURE_CLASS_DEFAULT"
    require(
        sampling.get("colorSpace") == color_space
        and sampling.get("colorSpaceEvidence") == color_evidence,
        f"DDS color-space projection disagrees with raw UPK: {logical_path}",
    )
    normalized["colorSpace"] = color_space
    normalized["colorSpaceEvidence"] = color_evidence
    return {
        "logicalObjectPath": logical_path,
        "export": export,
        "sampling": normalized,
        "dds": {
            "relativePath": asset.get("sourceExtractedDdsRelativePath"),
            "byteCount": dds.get("byteCount"),
            "sha256": dds.get("sha256"),
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
    exact_dds_receipt_path: Path,
    source_package_root: Path,
    source_pack_manifest_path: Path,
    aes_key: str = LOSTARK_KR_AES_KEY,
) -> dict[str, Any]:
    closure = load_json(closure_path)
    exact_dds_receipt = load_json(exact_dds_receipt_path)
    require(
        exact_dds_receipt.get("schema")
        == "lostark.artist-effect-exact-dds-recovery-receipt"
        and exact_dds_receipt.get("formatVersion") == 1,
        "unsupported exact DDS receipt",
    )
    rows = require_material_rows(closure)
    manifest_packages, manifest_evidence = source_manifest_packages(
        source_pack_manifest_path
    )
    package_paths = package_file_index(source_package_root)
    package_cache: dict[str, Any] = {}
    exports_by_id: dict[str, dict[str, Any]] = {}
    graph_expressions_by_id: dict[str, dict[str, Any]] = {}
    processed_graphs: dict[str, dict[str, Any]] = {}
    bindings: list[dict[str, Any]] = []

    def package(physical_name: str) -> Any:
        key = physical_name.casefold()
        require(key in package_paths, f"physical UPK is missing: {physical_name}")
        if key not in package_cache:
            package_cache[key] = load_package(package_paths[key], aes_key)
            loaded = package_cache[key]
            require(
                (key, loaded.sha256) in manifest_packages,
                f"raw package is absent from authenticated source manifest: {physical_name}",
            )
        return package_cache[key]

    def logical_package(physical_name: str, package_sha256: str) -> str:
        key = (physical_name.casefold(), package_sha256)
        require(
            key in manifest_packages,
            f"package identity is absent from authenticated source manifest: {physical_name}",
        )
        return manifest_packages[key]

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
            else BASE_MATERIAL_FIELDS
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
        source_export["logicalPackage"] = logical_package(
            source_export["physicalPackage"],
            source_export["physicalPackageSha256"],
        )
        require(
            canonical_path_targets_object(
                source_path,
                source_export["objectPath"],
                source_export["logicalPackage"],
            ),
            f"canonical source Material path does not target the raw export: {source_path}",
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
            source_export["instanceParameters"] = {
                "scalar": copy.deepcopy(decoded_instance["scalarParameters"]),
                "vector": copy.deepcopy(decoded_instance["vectorParameters"]),
                "texture": copy.deepcopy(decoded_instance["textureParameters"]),
            }
        source_evidence_id = add_export(source_export)
        require(
            folded(source_export["className"]) == material_class,
            f"source material class mismatch: {source_path}",
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
            BASE_MATERIAL_FIELDS,
        )
        base_export["logicalPackage"] = logical_package(
            base_export["physicalPackage"],
            base_export["physicalPackageSha256"],
        )
        base_evidence_id = add_export(base_export)
        if material_class == "materialinstanceconstant":
            parent_field = source_export["fields"]["parent"]
            require(
                parent_field.get("status") == "SERIALIZED_EXPLICIT"
                and canonical_path_targets_object(
                    parent_field.get("resolvedObjectPath"),
                    base_export["objectPath"],
                    base_export["logicalPackage"],
                )
                and canonical_path_targets_object(
                    material.get("parent"),
                    base_export["objectPath"],
                    base_export["logicalPackage"],
                ),
                f"raw MIC Parent does not select the parentGraph export: {source_path}",
            )
        else:
            require(
                source_evidence_id == base_evidence_id,
                f"raw Material graph does not target the source export: {source_path}",
            )
        raw_expression_field = base_export["fields"]["expressions"]
        require(
            raw_expression_field["status"] == "SERIALIZED_EXPLICIT"
            and isinstance(raw_expression_field.get("value"), list),
            f"base Material raw Expressions array missing: {source_path}",
        )
        raw_expression_refs = raw_expression_field["value"]
        expected_expressions = graph.get("expressions")
        require(
            isinstance(expected_expressions, list),
            f"closure graph expressions missing: {source_path}",
        )
        expected_by_order = {
            int(expression.get("sourceOrder", -1)): expression
            for expression in expected_expressions
            if isinstance(expression, dict)
        }
        require(
            len(expected_by_order) == len(expected_expressions),
            f"duplicate closure expression source order: {source_path}",
        )
        require(
            len(raw_expression_refs)
            == int(graph.get("summary", {}).get("expressionEntryCount", -1)),
            f"closure graph expression denominator disagrees with raw UPK: {source_path}",
        )
        require(
            sum(isinstance(reference, int) and reference != 0 for reference in raw_expression_refs)
            == len(expected_expressions),
            f"closure non-null expression denominator disagrees with raw UPK: {source_path}",
        )
        graph_identity = (
            f"{base_export['physicalPackageSha256']}::"
            f"{folded(base_export['objectPath'])}"
        )
        graph_projection = {
            "baseMaterialEvidenceId": base_evidence_id,
            "rawExpressionReferences": copy.deepcopy(raw_expression_refs),
            "closureExpressionCount": len(expected_expressions),
        }
        previous_graph = processed_graphs.get(graph_identity)
        require(
            previous_graph is None or previous_graph == graph_projection,
            f"repeated graph projection changed: {graph_identity}",
        )
        if previous_graph is None:
            processed_graphs[graph_identity] = graph_projection
            base_package = package(str(graph_holder.get("physicalPackage") or ""))
            for source_order, reference in enumerate(raw_expression_refs):
                require(
                    isinstance(reference, int),
                    f"raw expression reference is not an integer: {graph_identity}",
                )
                expected_expression = expected_by_order.get(source_order)
                if reference == 0:
                    require(
                        expected_expression is None,
                        f"closure expression occupies a raw null slot: {graph_identity}@{source_order}",
                    )
                    continue
                require(
                    expected_expression is not None,
                    f"raw expression is absent from closure: {graph_identity}@{source_order}",
                )
                evidence = expression_evidence(
                    base_package,
                    base_evidence_id,
                    source_order,
                    reference,
                    expected_expression,
                )
                evidence["logicalPackage"] = base_export["logicalPackage"]
                evidence_id = evidence["evidenceId"]
                require(
                    evidence_id not in graph_expressions_by_id,
                    f"duplicate graph expression evidence ID: {evidence_id}",
                )
                graph_expressions_by_id[evidence_id] = evidence
        bindings.append(
            {
                "sourceMaterialPath": source_path,
                "sourceExportEvidenceId": source_evidence_id,
                "renderStateExportEvidenceId": base_evidence_id,
                "sourceMaterialIdentity": {
                    "canonicalSourceMaterialPath": source_path,
                    "logicalPackage": source_export["logicalPackage"],
                    "physicalPackage": source_export["physicalPackage"],
                    "physicalPackageSha256": source_export[
                        "physicalPackageSha256"
                    ],
                    "exportIndex": source_export["exportIndex"],
                    "objectPath": source_export["objectPath"],
                    "rawExportEvidenceId": source_evidence_id,
                },
                "selectedGraphIdentity": {
                    "logicalPackage": base_export["logicalPackage"],
                    "physicalPackage": base_export["physicalPackage"],
                    "physicalPackageSha256": base_export[
                        "physicalPackageSha256"
                    ],
                    "exportIndex": base_export["exportIndex"],
                    "objectPath": base_export["objectPath"],
                    "rawExportEvidenceId": base_evidence_id,
                    "rawParentReferencePath": (
                        source_export["fields"]["parent"].get(
                            "resolvedObjectPath"
                        )
                        if material_class == "materialinstanceconstant"
                        else None
                    ),
                },
                "renderStateOrigin": (
                    "SELF_MATERIAL"
                    if source_evidence_id == base_evidence_id
                    else "PARENT_MATERIAL"
                ),
            }
        )

    texture_sampler_exports: list[dict[str, Any]] = []
    dds_assets = exact_dds_receipt.get("assets")
    require(isinstance(dds_assets, list) and len(dds_assets) == 4, "exact DDS asset denominator changed")
    seen_texture_paths: set[str] = set()
    for asset in dds_assets:
        require(isinstance(asset, dict), "exact DDS asset must be an object")
        logical_path = folded(asset.get("logicalObjectPath"))
        require(bool(logical_path) and logical_path not in seen_texture_paths, "duplicate exact DDS texture path")
        seen_texture_paths.add(logical_path)
        texture = asset.get("sourceTexture2D")
        require(isinstance(texture, dict), f"Texture2D evidence missing: {logical_path}")
        physical_name = str(texture.get("physicalPackage") or "")
        texture_evidence = texture_sampler_evidence(
            asset, package(physical_name)
        )
        texture_export = texture_evidence["export"]
        texture_export["logicalPackage"] = logical_package(
            texture_export["physicalPackage"],
            texture_export["physicalPackageSha256"],
        )
        texture_sampler_exports.append(texture_evidence)

    receipt: dict[str, Any] = {
        "schema": "lostark.artist-31470-material-render-state-evidence-receipt",
        "formatVersion": 3,
        "root": CONTRACT_ROOT,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "source": {
            "activeMaterialClosure": {
                "path": repository_path(closure_path),
                "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_json_text_sha256(closure_path),
            },
            "exactDdsReceipt": {
                "path": repository_path(exact_dds_receipt_path),
                "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_json_text_sha256(
                    exact_dds_receipt_path
                ),
            },
            "sourcePackages": {
                "sourceRootId": "Resource_LostArk",
                "hashDomain": "RAW_ARTIFACT_BYTES",
            },
            "sourcePackManifest": manifest_evidence,
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
            "materialClosureParser": {
                "path": repository_path(MATERIAL_CLOSURE_PARSER_PATH),
                "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_source_text_sha256(
                    MATERIAL_CLOSURE_PARSER_PATH
                ),
            },
            "materialGraphParser": {
                "path": repository_path(MATERIAL_GRAPH_PARSER_PATH),
                "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
                "canonicalTextSha256": tracked_source_text_sha256(
                    MATERIAL_GRAPH_PARSER_PATH
                ),
            },
            "reproductionCommand": [
                "python",
                repository_path(SCRIPT_PATH),
                "--material-closure",
                repository_path(closure_path),
                "--exact-dds-receipt",
                repository_path(exact_dds_receipt_path),
                "--source-package-root",
                "{SOURCE_PACKAGE_ROOT}",
                "--source-pack-manifest",
                "{SOURCE_PACK_MANIFEST}",
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
        "graphExpressions": sorted(
            graph_expressions_by_id.values(),
            key=lambda row: (
                row["baseMaterialEvidenceId"],
                row["sourceOrder"],
            ),
        ),
        "textureSamplerExports": sorted(
            texture_sampler_exports,
            key=lambda row: folded(row["logicalObjectPath"]),
        ),
        "summary": {
            "materialRecipeCount": len(bindings),
            "uniqueRawExportCount": len(exports_by_id),
            "rawPackageCount": len(package_cache),
            "uniqueBaseMaterialGraphCount": len(processed_graphs),
            "graphExpressionEvidenceCount": len(graph_expressions_by_id),
            "textureSamplerExportCount": len(texture_sampler_exports),
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
    parser.add_argument("--exact-dds-receipt", required=True, type=Path)
    parser.add_argument("--source-package-root", required=True, type=Path)
    parser.add_argument("--source-pack-manifest", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    receipt = build_receipt(
        args.material_closure,
        args.exact_dds_receipt,
        args.source_package_root,
        args.source_pack_manifest,
        args.aes_key,
    )
    check_or_write_json(args.output, receipt, args.check)
    print(
        "Artist F material render-state evidence "
        f"{'check' if args.check else 'write'}: "
        f"recipes={receipt['summary']['materialRecipeCount']} "
        f"exports={receipt['summary']['uniqueRawExportCount']} "
        f"expressions={receipt['summary']['graphExpressionEvidenceCount']} "
        f"textures={receipt['summary']['textureSamplerExportCount']} "
        f"packages={receipt['summary']['rawPackageCount']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ExtractionError, OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
