#!/usr/bin/env python3
"""Close source-exact UE3 material texture bindings for cooked shader variants.

The input is a class-neutral G03 material-map receipt.  For every exact target
this tool joins, in order:

* the parent Material's cooked ``ReferencedTextures`` list;
* the selected MIC resource's effective ``ReferencedTextures`` list;
* raw MIC texture overrides keyed by the full ``FName`` (base + number);
* uniform-expression index -> native t/s binding wire;
* the exact source Texture2D export and authenticated extracted DDS bytes; and
* serialized Texture2D address/filter/sRGB fields.

Omitted Texture2D properties are deliberately not promoted to source-exact
sampler state.  The receipt records the UE3 native-constructor candidate and a
blocker until the matching source-revision CDO/TextureLODSettings configuration
is available.  Authored generic resource slots are never consulted.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

from extract_artist_31470_material_native_resource import (  # noqa: E402
    parse_material_resource_tail,
)
from extract_artist_31470_material_render_state import (  # noqa: E402
    parse_property_records,
)
from extract_artist_31470_main_ref_shader_cache import require  # noqa: E402
from extract_ue3_effect_material_closure import (  # noqa: E402
    find_export,
    load_package,
)
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


SCHEMA = "lostark.effect-ue3-material-texture-sampler-closure-receipt"
FORMAT_VERSION = 1
EXACT_STATUS = "EXACT_MATERIAL_SHADER_MAP"

DEFAULT_EXACT_RECEIPT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-material-maps.receipt.json"
)
DEFAULT_RESOURCE_MANIFEST = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/"
    "DimensionMaster.resource-source-manifest.json"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-texture-sampler-closure.receipt.json"
)
DEFAULT_SOURCE_ROOT = Path(
    "C:/Users/user/Desktop/Resource_LostArk/00_SourcePackages/"
    "Effect_DIMENSIONMASTER_20260803_v3/Dependencies"
)
DEFAULT_MATERIAL_DDS_RECEIPT = Path(
    "C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/"
    "FourClassMaterials/FourClass.material-extract.receipt.json"
)
DEFAULT_MATERIAL_DDS_ROOT = DEFAULT_MATERIAL_DDS_RECEIPT.parent / "export"
DEFAULT_RAW_DDS_RECEIPT = Path(
    "C:/Users/user/Desktop/Resource_LostArk/05_Reports/RawResourceInventory/"
    "R8-42b437d59bf56c71-20260811T1630KST-v1/"
    "raw-resource-extraction-run-v1.json"
)
DEFAULT_RAW_DDS_ROOT = DEFAULT_RAW_DDS_RECEIPT.parent
DEFAULT_RUNTIME_TEXTURE_ROOT = (
    REPOSITORY_ROOT / "Client/Bin/Resources/Effect/DimensionMaster/Textures"
)


def folded(value: Any) -> str:
    return str(value or "").strip().casefold()


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while payload := stream.read(1024 * 1024):
            digest.update(payload)
    return digest.hexdigest()


def canonical_json_sha256(value: Any) -> str:
    return sha256_bytes(
        json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
        ).encode("utf-8")
    )


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def source_descriptor(path: Path, role: str) -> dict[str, Any]:
    return {
        "path": path.as_posix(),
        "byteSize": path.stat().st_size,
        "sha256": sha256_file(path),
        "role": role,
    }


def strict_export(package: Any, relative_path: str, classes: set[str]) -> Any:
    matches = []
    for entry in package.exports:
        object_path = package_ref_path(
            entry.index + 1, package.imports, package.exports
        )
        class_name = package_ref_name(
            entry.class_index, package.imports, package.exports
        )
        if (
            folded(object_path) == folded(relative_path)
            and folded(class_name) in classes
        ):
            matches.append(entry)
    require(
        len(matches) == 1,
        f"expected one {sorted(classes)} export in {package.path.name}: "
        f"{relative_path} (found {len(matches)})",
    )
    return matches[0]


def manifest_asset_index(resource_manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in resource_manifest.get("assets", []):
        require(isinstance(row, dict), "resource manifest asset is not an object")
        key = folded(row.get("sourceAssetPath"))
        if not key:
            continue
        previous = result.get(key)
        require(previous is None or previous == row, f"ambiguous resource asset: {key}")
        result[key] = row
    return result


def resolve_texture_manifest_asset(
    source_path: str, assets: dict[str, dict[str, Any]]
) -> dict[str, Any]:
    """Resolve a Texture2D export without inventing a leaf-to-package mapping.

    The DimensionMaster resource manifest is an occurrence manifest, so it does
    not necessarily enumerate dependency textures that are reached only through
    a parent Material's native resource table.  Its logical-package mapping is
    still authoritative: every enumerated asset in one logical package must map
    to the same physical package.  We use that package-level closure and then
    require the requested Texture2D export to exist exactly once in the package.
    """

    exact = assets.get(folded(source_path))
    if exact is not None:
        return exact
    logical_package, separator, relative_path = source_path.partition(".")
    require(separator and relative_path, f"invalid Texture2D object path: {source_path}")
    package_rows = [
        row
        for row in assets.values()
        if folded(row.get("logicalPackage")) == folded(logical_package)
        and row.get("physicalPackage")
    ]
    physical_packages = sorted(
        {str(row["physicalPackage"]) for row in package_rows}, key=str.casefold
    )
    require(
        len(physical_packages) == 1,
        f"Texture2D logical package mapping is not unique: {logical_package}",
    )
    return {
        "sourceAssetPath": source_path,
        "logicalPackage": logical_package,
        "physicalPackage": physical_packages[0],
        "roles": ["texture"],
        "resolutionStatus": "RESOLVED_BY_EXACT_LOGICAL_PACKAGE_MAPPING",
    }


def resolve_parent_manifest_asset(
    target_path: str, assets: dict[str, dict[str, Any]]
) -> dict[str, Any]:
    exact = assets.get(folded(target_path))
    if exact is not None:
        return exact
    suffix_matches = [
        row
        for key, row in assets.items()
        if folded(target_path).endswith("." + key)
        and "material_parent" in {folded(role) for role in row.get("roles", [])}
    ]
    require(
        len(suffix_matches) == 1,
        f"parent Material manifest mapping is ambiguous: {target_path}",
    )
    return suffix_matches[0]


def manifest_relative_object(row: dict[str, Any]) -> str:
    source_path = str(row.get("sourceAssetPath") or "")
    logical = str(row.get("logicalPackage") or "")
    prefix = logical + "."
    require(
        folded(source_path).startswith(folded(prefix)),
        f"manifest asset does not start with logical package: {source_path}",
    )
    return source_path[len(prefix) :]


def package_relative_object_path(full_path: str) -> str:
    _outer_package, separator, relative_path = full_path.partition(".")
    require(separator and relative_path, f"object path has no outer package: {full_path}")
    return relative_path


def resolve_texture_reference(package: Any, reference: int) -> dict[str, Any]:
    require(reference != 0, "effective ReferencedTextures contains null")
    object_path = package_ref_path(reference, package.imports, package.exports)
    if reference < 0:
        class_name = package.imports[-reference - 1].class_name
    else:
        require(reference <= len(package.exports), "texture export reference is invalid")
        entry = package.exports[reference - 1]
        class_name = package_ref_name(
            entry.class_index, package.imports, package.exports
        )
    require(folded(class_name) == "texture2d", f"not Texture2D: {object_path}")
    return {
        "packageReference": reference,
        "objectPath": object_path,
        "className": "texture2d",
    }


def parse_mic_effective_referenced_textures(
    tail: bytes, package: Any, static_set_offset: int
) -> dict[str, Any]:
    """Decode the FMaterialInstanceResource texture union before the static set.

    Lost Ark v868 serializes a 36-byte resource header followed by the ordered
    Texture2D references.  The exact shader-map receipt supplies the independently
    decoded static-set offset, which bounds this parse and prevents a window scan.
    """

    require(len(tail) >= 40, "MIC native resource is truncated")
    require(0 < static_set_offset <= len(tail), "invalid MIC static-set offset")
    header = tail[:36]
    texture_count = struct.unpack_from("<I", tail, 36)[0]
    require(texture_count <= 256, "MIC effective texture count is unbounded")
    end = 40 + texture_count * 4
    require(end <= static_set_offset, "MIC texture list overlaps static set")
    references = []
    for index in range(texture_count):
        reference = struct.unpack_from("<i", tail, 40 + index * 4)[0]
        references.append(
            {
                "index": index,
                "offsetInNativeTail": 40 + index * 4,
                **resolve_texture_reference(package, reference),
            }
        )
    return {
        "headerByteCount": 36,
        "headerRawSha256": sha256_bytes(header),
        "referencedTextureCountOffsetInNativeTail": 36,
        "referencedTextures": references,
        "referencedTexturesEndOffsetInNativeTail": end,
        "opaqueBytesBeforeStaticSet": static_set_offset - end,
        "staticParameterSetOffsetInNativeTail": static_set_offset,
        "nativeTailByteCount": len(tail),
        "nativeTailSha256": sha256_bytes(tail),
        "fidelity": "SOURCE_EXACT_MIC_MATERIAL_RESOURCE_ORDERED_REFERENCED_TEXTURES",
    }


def _read_fname(payload: bytes, offset: int, names: list[str]) -> tuple[str, int, int]:
    require(offset + 8 <= len(payload), "FName is truncated")
    index, number = struct.unpack_from("<ii", payload, offset)
    require(0 <= index < len(names) and number >= 0, "FName is invalid")
    return names[index], number, offset + 8


def parse_tagged_struct_array_raw(
    payload: bytes, names: list[str]
) -> list[dict[str, dict[str, Any]]]:
    require(len(payload) >= 4, "tagged struct array is truncated")
    count = struct.unpack_from("<i", payload, 0)[0]
    require(0 <= count <= 4096, "tagged struct array count is invalid")
    offset = 4
    rows: list[dict[str, dict[str, Any]]] = []
    for _ in range(count):
        fields: dict[str, dict[str, Any]] = {}
        while True:
            tag_start = offset
            property_name, property_number, offset = _read_fname(payload, offset, names)
            if folded(property_name) == "none":
                break
            property_type, property_type_number, offset = _read_fname(
                payload, offset, names
            )
            require(offset + 8 <= len(payload), "property tag is truncated")
            data_size, array_index = struct.unpack_from("<ii", payload, offset)
            offset += 8
            require(data_size >= 0 and array_index >= 0, "property tag is invalid")
            struct_type = None
            bool_value = None
            type_key = folded(property_type)
            if type_key == "structproperty":
                struct_type, _, offset = _read_fname(payload, offset, names)
            elif type_key == "boolproperty":
                require(offset < len(payload), "BoolProperty is truncated")
                bool_value = bool(payload[offset])
                offset += 1
            elif type_key == "byteproperty":
                _, _, offset = _read_fname(payload, offset, names)
            serialized_size = data_size + 8 if type_key == "intproperty" else data_size
            require(offset + serialized_size <= len(payload), "property value is truncated")
            value_offset = offset
            raw_value = payload[offset : offset + serialized_size]
            offset += serialized_size
            field: dict[str, Any] = {
                "propertyName": property_name,
                "propertyNameNumber": property_number,
                "propertyType": property_type,
                "propertyTypeNumber": property_type_number,
                "arrayIndex": array_index,
                "structType": struct_type,
                "tagOffset": tag_start,
                "valueOffset": value_offset,
                "recordEndOffset": offset,
                "recordSha256": sha256_bytes(payload[tag_start:offset]),
            }
            if type_key == "nameproperty":
                value, number, end = _read_fname(raw_value, 0, names)
                require(end == len(raw_value), "NameProperty payload has trailing bytes")
                field["value"] = value
                field["valueNameNumber"] = number
            elif type_key in {"objectproperty", "componentproperty"}:
                require(len(raw_value) == 4, "object reference shape changed")
                field["value"] = struct.unpack_from("<i", raw_value, 0)[0]
            elif type_key == "boolproperty":
                field["value"] = bool_value
            elif type_key == "structproperty" and folded(struct_type) == "guid":
                require(len(raw_value) == 16, "GUID property shape changed")
                field["valueGuidHex"] = raw_value.hex()
            fields[folded(property_name)] = field
        rows.append(fields)
    require(offset == len(payload), "tagged struct array has trailing bytes")
    return rows


def decode_mic_texture_overrides(
    serial: bytes, names: list[str], package: Any, package_version: int
) -> dict[str, Any]:
    records, stream_start, stream_end = parse_property_records(
        serial, names, package_version
    )
    matches = [
        row
        for row in records
        if folded(row.get("propertyName")) == "textureparametervalues"
        and int(row.get("arrayIndex", -1)) == 0
    ]
    require(len(matches) <= 1, "duplicate TextureParameterValues property")
    if not matches:
        return {
            "propertyStatus": "OMITTED",
            "propertyStreamStart": stream_start,
            "propertyStreamEnd": stream_end,
            "overrides": [],
        }
    record = matches[0]
    payload = bytes.fromhex(str(record["encodedValueHex"]))
    parsed = parse_tagged_struct_array_raw(payload, names)
    overrides = []
    seen: set[tuple[str, int]] = set()
    for index, fields in enumerate(parsed):
        name_field = fields.get("parametername")
        value_field = fields.get("parametervalue")
        require(name_field is not None and value_field is not None, "texture override fields missing")
        require(folded(name_field.get("propertyType")) == "nameproperty", "override name type changed")
        require(folded(value_field.get("propertyType")) == "objectproperty", "override value type changed")
        name = str(name_field["value"])
        name_number = int(name_field["valueNameNumber"])
        key = (folded(name), name_number)
        require(key not in seen, f"duplicate MIC texture override FName: {key}")
        seen.add(key)
        reference = int(value_field["value"])
        overrides.append(
            {
                "index": index,
                "parameterName": name,
                "parameterNameNumber": name_number,
                "parameterFNameKey": f"{folded(name)}#{name_number}",
                "packageReference": reference,
                "sourceObjectPath": (
                    package_ref_path(reference, package.imports, package.exports)
                    if reference
                    else None
                ),
                "parameterNameRecordSha256": name_field["recordSha256"],
                "parameterValueRecordSha256": value_field["recordSha256"],
                "expressionGuidHex": (fields.get("expressionguid") or {}).get(
                    "valueGuidHex"
                ),
            }
        )
    return {
        "propertyStatus": "SERIALIZED_EXPLICIT",
        "propertyStreamStart": stream_start,
        "propertyStreamEnd": stream_end,
        "propertyRecordSha256": record["recordSha256"],
        "propertyPayloadSha256": record["encodedValueSha256"],
        "overrides": overrides,
    }


def resolve_uniform_texture_bindings(
    expressions: list[dict[str, Any]],
    wires: list[dict[str, Any]],
    mic_references: list[dict[str, Any]],
    overrides: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    wire_by_expression = {
        int(row["expressionIndexOrGroup"]): row for row in wires
    }
    require(
        set(wire_by_expression) == set(range(len(expressions))),
        "native texture wire does not cover every uniform expression",
    )
    override_by_key = {
        (folded(row["parameterName"]), int(row["parameterNameNumber"])): row
        for row in overrides
    }
    result = []
    for expression_index, expression in enumerate(expressions):
        fallback_index = int(expression["referencedTextureIndex"])
        require(
            0 <= fallback_index < len(mic_references),
            "uniform fallback index is outside MIC ReferencedTextures",
        )
        fallback = mic_references[fallback_index]
        name = expression.get("parameterName")
        name_number = int(expression.get("parameterNameNumber", 0))
        override = (
            override_by_key.get((folded(name), name_number)) if name else None
        )
        override_applied = bool(override and override.get("sourceObjectPath"))
        effective_path = (
            str(override["sourceObjectPath"])
            if override_applied
            else str(fallback["objectPath"])
        )
        wire = wire_by_expression[expression_index]
        result.append(
            {
                "uniformExpressionIndex": expression_index,
                "expressionType": expression["typeName"],
                "parameterName": name,
                "parameterNameNumber": name_number if name else None,
                "parameterFNameKey": (
                    f"{folded(name)}#{name_number}" if name else None
                ),
                "fallbackReferencedTextureIndex": fallback_index,
                "fallbackSourceObjectPath": fallback["objectPath"],
                "micOverrideApplied": override_applied,
                "micOverrideIndex": int(override["index"]) if override_applied else None,
                "effectiveSourceObjectPath": effective_path,
                "textureRegister": f"t{int(wire['baseIndex'])}",
                "samplerRegister": f"s{int(wire['bufferIndexOrSamplerIndex'])}",
                "bindingFidelity": (
                    "SOURCE_EXACT_UNIFORM_EXPRESSION_PLUS_NATIVE_WIRE_PLUS_"
                    "MIC_EFFECTIVE_RESOURCE_AND_FULL_FNAME_OVERRIDE"
                ),
            }
        )
    return result


def property_evidence(records: list[dict[str, Any]], name: str) -> dict[str, Any]:
    matches = [
        row
        for row in records
        if folded(row.get("propertyName")) == folded(name)
        and int(row.get("arrayIndex", -1)) == 0
    ]
    require(len(matches) <= 1, f"duplicate Texture2D field: {name}")
    if not matches:
        return {"status": "OMITTED_FROM_EXPORT"}
    row = matches[0]
    return {
        "status": "SERIALIZED_EXPLICIT",
        "propertyName": row["propertyName"],
        "propertyType": row["propertyType"],
        "value": row.get("value"),
        "recordSha256": row["recordSha256"],
        "encodedValueSha256": row["encodedValueSha256"],
    }


def sampler_projection(fields: dict[str, dict[str, Any]]) -> dict[str, Any]:
    blockers: list[str] = []

    def address(field_name: str, blocker: str) -> dict[str, Any]:
        field = fields[field_name]
        if field["status"] == "SERIALIZED_EXPLICIT":
            token = folded(field.get("value"))
            require(token in {"ta_wrap", "ta_clamp"}, f"unsupported {field_name}: {token}")
            return {"value": token, "fidelity": "SOURCE_EXACT_SERIALIZED_PROPERTY"}
        blockers.append(blocker)
        return {
            "valueCandidate": "ta_wrap",
            "fidelity": "UE3_NATIVE_CONSTRUCTOR_CANDIDATE_NOT_SOURCE_REVISION_CDO",
        }

    address_u = address("addressx", "ADDRESS_U_SOURCE_REVISION_CDO_UNRESOLVED")
    address_v = address("addressy", "ADDRESS_V_SOURCE_REVISION_CDO_UNRESOLVED")

    srgb = fields["srgb"]
    if srgb["status"] == "SERIALIZED_EXPLICIT":
        require(type(srgb.get("value")) is bool, "Texture2D sRGB is not bool")
        color_space = {
            "value": "srgb" if srgb["value"] else "linear",
            "fidelity": "SOURCE_EXACT_SERIALIZED_PROPERTY",
        }
    else:
        blockers.append("COLOR_SPACE_SOURCE_REVISION_CDO_UNRESOLVED")
        color_space = {
            "valueCandidate": "srgb",
            "fidelity": "UE3_NATIVE_CONSTRUCTOR_CANDIDATE_NOT_SOURCE_REVISION_CDO",
        }

    filter_field = fields["filter"]
    if filter_field["status"] == "SERIALIZED_EXPLICIT":
        selector = folded(filter_field.get("value"))
        require(
            selector in {"tf_nearest", "tf_linear", "tf_default"},
            f"unsupported Texture2D filter: {selector}",
        )
        selector_fidelity = "SOURCE_EXACT_SERIALIZED_PROPERTY"
    else:
        selector = "tf_default"
        selector_fidelity = (
            "UE3_NATIVE_CONSTRUCTOR_CANDIDATE_NOT_SOURCE_REVISION_CDO"
        )
        blockers.append("FILTER_SELECTOR_SOURCE_REVISION_CDO_UNRESOLVED")

    lod_group = fields["lodgroup"]
    lod_value = (
        folded(lod_group.get("value"))
        if lod_group["status"] == "SERIALIZED_EXPLICIT"
        else None
    )
    if selector == "tf_default":
        blockers.append("FILTER_TF_DEFAULT_TEXTURELODSETTINGS_UNRESOLVED")
        resolved_filter = None
    else:
        resolved_filter = "point" if selector == "tf_nearest" else "linear"

    return {
        "addressU": address_u,
        "addressV": address_v,
        "filterSelector": {
            "value": selector,
            "fidelity": selector_fidelity,
        },
        "lodGroup": {
            "value": lod_value,
            "fidelity": (
                "SOURCE_EXACT_SERIALIZED_PROPERTY"
                if lod_value is not None
                else "SOURCE_REVISION_CDO_UNRESOLVED"
            ),
        },
        "resolvedFilter": resolved_filter,
        "colorSpace": color_space,
        "sourceExactSamplerAndColorSpace": not blockers,
        "blockers": sorted(set(blockers)),
    }


def texture_export_evidence(
    source_path: str,
    asset: dict[str, Any],
    source_root: Path,
    package_cache: dict[str, Any],
) -> dict[str, Any]:
    physical_name = str(asset.get("physicalPackage") or "")
    package_path = source_root / physical_name
    require(package_path.is_file(), f"Texture2D source package missing: {physical_name}")
    key = physical_name.casefold()
    if key not in package_cache:
        package_cache[key] = load_package(package_path, LOSTARK_KR_AES_KEY)
    package = package_cache[key]
    entry = strict_export(
        package, manifest_relative_object(asset), {"texture2d", "texture"}
    )
    class_name = package_ref_name(
        entry.class_index, package.imports, package.exports
    )
    require(folded(class_name) == "texture2d", f"effective texture is not Texture2D: {source_path}")
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    records, property_start, property_end = parse_property_records(
        serial, package.names, package.summary.version
    )
    fields = {
        name: property_evidence(records, name)
        for name in ("addressx", "addressy", "srgb", "filter", "lodgroup")
    }
    return {
        "sourceObjectPath": source_path,
        "manifestLogicalPackage": asset.get("logicalPackage"),
        "packageResolutionStatus": asset.get("resolutionStatus"),
        "physicalPackage": physical_name,
        "physicalPackageByteSize": package_path.stat().st_size,
        "physicalPackageSha256": sha256_file(package_path),
        "objectPath": package_ref_path(
            entry.index + 1, package.imports, package.exports
        ),
        "className": folded(class_name),
        "exportIndexZeroBased": entry.index,
        "serialOffset": entry.serial_offset,
        "serialByteSize": entry.serial_size,
        "serialSha256": sha256_bytes(serial),
        "propertyStreamStart": property_start,
        "propertyStreamEnd": property_end,
        "fields": fields,
        "samplerAndColorSpace": sampler_projection(fields),
    }


def material_dds_candidates(
    receipt: dict[str, Any], source_material_path: str
) -> tuple[dict[str, dict[str, Any]], str | None]:
    matches = [
        row
        for row in receipt.get("candidates", [])
        if folded(row.get("material_path")) == folded(source_material_path)
    ]
    require(len(matches) == 1, f"material DDS candidate not unique: {source_material_path}")
    candidate = matches[0]
    require(
        candidate.get("resolutionStatus") == "RESOLVED_UMODEL_EXPORT",
        f"material DDS export unresolved: {source_material_path}",
    )
    result = {}
    for row in candidate.get("exportedTextures", []):
        key = folded(row.get("texturePath"))
        require(key and key not in result, f"duplicate material DDS export: {key}")
        result[key] = row
    return result, candidate.get("exportRoot")


def raw_dds_index(receipt: dict[str, Any]) -> dict[str, dict[str, Any]]:
    corpus = receipt.get("corpora", {}).get("FourClass", {})
    result = {}
    for row in corpus.get("requests", []):
        if folded(row.get("role")) != "texture":
            continue
        key = folded(row.get("sourceAssetPath"))
        payloads = [
            value
            for value in row.get("payloads", [])
            if folded(value.get("kind")) == "dds"
        ]
        if not key or not payloads:
            continue
        require(len(payloads) == 1 and key not in result, f"ambiguous raw DDS: {key}")
        result[key] = payloads[0]
    return result


def close_dds_identity(
    source_path: str,
    material_rows: dict[str, dict[str, Any]],
    material_export_root: str | None,
    raw_rows: dict[str, dict[str, Any]],
    material_dds_root: Path,
    raw_dds_root: Path,
    runtime_root: Path,
) -> dict[str, Any]:
    key = folded(source_path)
    evidence = []
    material = material_rows.get(key)
    if material is not None:
        require(material_export_root is not None, "material DDS export root missing")
        path = material_dds_root / str(material_export_root) / str(material["relativeFile"])
        require(path.is_file(), f"material DDS payload missing: {path}")
        evidence.append(
            {
                "provider": "MATCHED_SOURCE_MATERIAL_UMODEL_EXPORT",
                "relativePath": f"{material_export_root}/{material['relativeFile']}",
                "byteSize": int(material["byteSize"]),
                "sha256": str(material["sha256"]),
                "fileIdentityVerified": (
                    path.stat().st_size == int(material["byteSize"])
                    and sha256_file(path) == str(material["sha256"])
                ),
            }
        )
    raw = raw_rows.get(key)
    if raw is not None:
        path = raw_dds_root / str(raw["relativePath"])
        require(path.is_file(), f"raw inventory DDS payload missing: {path}")
        evidence.append(
            {
                "provider": "AUTHENTICATED_FOURCLASS_RAW_RESOURCE_INVENTORY",
                "relativePath": str(raw["relativePath"]),
                "byteSize": int(raw["byteSize"]),
                "sha256": str(raw["sha256"]),
                "fileIdentityVerified": (
                    path.stat().st_size == int(raw["byteSize"])
                    and sha256_file(path) == str(raw["sha256"])
                ),
            }
        )
    require(evidence, f"no authenticated DDS evidence: {source_path}")
    require(all(row["fileIdentityVerified"] for row in evidence), f"DDS file identity mismatch: {source_path}")
    identities = {(row["byteSize"], row["sha256"]) for row in evidence}
    require(len(identities) == 1, f"DDS providers disagree: {source_path}")
    byte_size, digest = next(iter(identities))

    logical_package = source_path.split(".", 1)[0]
    leaf = source_path.rsplit(".", 1)[-1] + ".dds"
    runtime_path = runtime_root / logical_package.upper() / leaf
    runtime = {
        "relativePath": (
            runtime_path.relative_to(REPOSITORY_ROOT / "Client/Bin/Resources").as_posix()
        ),
        "status": "MISSING",
        "byteSize": None,
        "sha256": None,
        "sourceExactParity": False,
    }
    if runtime_path.is_file():
        runtime_digest = sha256_file(runtime_path)
        runtime.update(
            {
                "status": "PRESENT",
                "byteSize": runtime_path.stat().st_size,
                "sha256": runtime_digest,
                "sourceExactParity": (
                    runtime_path.stat().st_size == byte_size
                    and runtime_digest == digest
                ),
            }
        )
    return {
        "sourceObjectPath": source_path,
        "sourceExactDds": {"byteSize": byte_size, "sha256": digest},
        "evidence": evidence,
        "runtimeDimensionMaster": runtime,
    }


def build_receipt(
    exact_receipt_path: Path,
    resource_manifest_path: Path,
    source_root: Path,
    material_dds_receipt_path: Path,
    material_dds_root: Path,
    raw_dds_receipt_path: Path,
    raw_dds_root: Path,
    runtime_texture_root: Path,
) -> dict[str, Any]:
    exact = read_json(exact_receipt_path)
    require(
        exact.get("schema") == "lostark.effect-ue3-material-shader-map-receipt"
        and exact.get("scope", {}).get("nativeShaderObjectBindingsDecoded") is True,
        "unsupported exact material-map receipt",
    )
    resource_manifest = read_json(resource_manifest_path)
    assets = manifest_asset_index(resource_manifest)
    material_dds_receipt = read_json(material_dds_receipt_path)
    raw_dds_receipt = read_json(raw_dds_receipt_path)
    raw_rows = raw_dds_index(raw_dds_receipt)
    package_cache: dict[str, Any] = {}
    texture_cache: dict[str, dict[str, Any]] = {}
    dds_cache: dict[tuple[str, str], dict[str, Any]] = {}
    targets = []

    for exact_target in exact.get("targets", []):
        if exact_target.get("status") != EXACT_STATUS:
            targets.append(
                {
                    "targetId": exact_target.get("targetId"),
                    "familyId": exact_target.get("familyId"),
                    "status": "BLOCKED_UPSTREAM_NO_EXACT_MATERIAL_MAP",
                    "upstreamStatus": exact_target.get("status"),
                    "sourceExactTextureBindingAdmission": False,
                    "runtimeDdsParityAdmission": False,
                    "sourceExactSamplerAdmission": False,
                }
            )
            continue

        mic = exact_target["mic"]
        source_package_path = source_root / str(mic["sourcePackageFileName"])
        require(source_package_path.is_file(), f"MIC package missing: {source_package_path}")
        source_key = source_package_path.name.casefold()
        if source_key not in package_cache:
            package_cache[source_key] = load_package(
                source_package_path, LOSTARK_KR_AES_KEY
            )
        source_package = package_cache[source_key]
        mic_entry = strict_export(
            source_package,
            str(mic["micObjectPath"]),
            {"materialinstanceconstant"},
        )
        mic_serial = source_package.logical[
            mic_entry.serial_offset : mic_entry.serial_offset + mic_entry.serial_size
        ]
        require(
            sha256_bytes(mic_serial) == mic["serialSha256"],
            f"MIC serial identity changed: {exact_target['targetId']}",
        )
        _properties, property_end = parse_tagged_properties(
            mic_serial, source_package.names, source_package.summary.version
        )
        require(property_end == mic["propertyStreamEnd"], "MIC property end changed")
        mic_tail = mic_serial[property_end:]
        mic_resource = parse_mic_effective_referenced_textures(
            mic_tail,
            source_package,
            int(mic["staticParameterSetOffsetInNativeTail"]),
        )
        mic_overrides = decode_mic_texture_overrides(
            mic_serial,
            source_package.names,
            source_package,
            source_package.summary.version,
        )

        parent_asset = resolve_parent_manifest_asset(
            str(exact_target["parentMaterialPath"]), assets
        )
        parent_package_path = source_root / str(parent_asset["physicalPackage"])
        require(parent_package_path.is_file(), f"parent package missing: {parent_package_path}")
        parent_key = parent_package_path.name.casefold()
        if parent_key not in package_cache:
            package_cache[parent_key] = load_package(
                parent_package_path, LOSTARK_KR_AES_KEY
            )
        parent_package = package_cache[parent_key]
        # The target identity keeps the cooked outer package followed by the
        # exact group/object path.  Resource occurrence manifests may use the
        # first group as their logical-package key, so stripping via the
        # manifest row would incorrectly turn ``fx_m.Parent`` into ``Parent``.
        parent_relative = package_relative_object_path(
            str(exact_target["parentMaterialPath"])
        )
        parent_entry = strict_export(
            parent_package, parent_relative, {"material", "decalmaterial"}
        )
        parent_serial = parent_package.logical[
            parent_entry.serial_offset : parent_entry.serial_offset
            + parent_entry.serial_size
        ]
        _parent_properties, parent_end = parse_tagged_properties(
            parent_serial, parent_package.names, parent_package.summary.version
        )
        parent_resource = parse_material_resource_tail(
            parent_serial[parent_end:], parent_package
        )
        require(
            parent_resource["materialStateGuidHex"]
            == exact_target["baseMaterialIdHex"],
            f"parent Material state GUID changed: {exact_target['targetId']}",
        )

        material_rows, material_export_root = material_dds_candidates(
            material_dds_receipt, str(exact_target["sourceMaterialPath"])
        )
        bindings = resolve_uniform_texture_bindings(
            exact_target["materialMap"]["uniformExpressionSet"][
                "pixelTexture2DExpressions"
            ],
            exact_target["nativeShaderObjectBinding"]["textures"],
            mic_resource["referencedTextures"],
            mic_overrides["overrides"],
        )
        target_blockers: set[str] = set()
        for binding in bindings:
            source_path = str(binding["effectiveSourceObjectPath"])
            texture_asset = resolve_texture_manifest_asset(source_path, assets)
            if folded(source_path) not in texture_cache:
                texture_cache[folded(source_path)] = texture_export_evidence(
                    source_path, texture_asset, source_root, package_cache
                )
            dds_key = (folded(exact_target["sourceMaterialPath"]), folded(source_path))
            if dds_key not in dds_cache:
                dds_cache[dds_key] = close_dds_identity(
                    source_path,
                    material_rows,
                    material_export_root,
                    raw_rows,
                    material_dds_root,
                    raw_dds_root,
                    runtime_texture_root,
                )
            texture = texture_cache[folded(source_path)]
            dds = dds_cache[dds_key]
            binding["sourceTexture2D"] = texture
            binding["ddsIdentity"] = dds
            if not dds["runtimeDimensionMaster"]["sourceExactParity"]:
                target_blockers.add("RUNTIME_DIMENSIONMASTER_DDS_MISSING_OR_MISMATCH")
            target_blockers.update(texture["samplerAndColorSpace"]["blockers"])

        unique_effective = sorted(
            {binding["effectiveSourceObjectPath"] for binding in bindings},
            key=str.casefold,
        )
        runtime_dds_admitted = all(
            binding["ddsIdentity"]["runtimeDimensionMaster"]["sourceExactParity"]
            for binding in bindings
        )
        sampler_admitted = all(
            binding["sourceTexture2D"]["samplerAndColorSpace"][
                "sourceExactSamplerAndColorSpace"
            ]
            for binding in bindings
        )
        targets.append(
            {
                "targetId": exact_target["targetId"],
                "familyId": exact_target["familyId"],
                "rendererType": exact_target["rendererType"],
                "sourceMaterialPath": exact_target["sourceMaterialPath"],
                "parentMaterialPath": exact_target["parentMaterialPath"],
                "status": "EXACT_TEXTURE_BINDING_SAMPLER_BLOCKED",
                "parentMaterialResource": {
                    "physicalPackage": parent_package_path.name,
                    "physicalPackageSha256": sha256_file(parent_package_path),
                    "objectPath": parent_relative,
                    "exportIndexZeroBased": parent_entry.index,
                    "serialSha256": sha256_bytes(parent_serial),
                    "materialStateGuidHex": parent_resource[
                        "materialStateGuidHex"
                    ],
                    "referencedTextures": parent_resource["referencedTextures"],
                    "nativeTailSha256": parent_resource["nativeTailSha256"],
                    "fidelity": "SOURCE_EXACT_PARENT_MATERIAL_RESOURCE",
                },
                "micMaterialResource": mic_resource,
                "micTextureOverrides": mic_overrides,
                "uniformTextureBindings": bindings,
                "uniformTextureBindingCount": len(bindings),
                "uniqueEffectiveTextureCount": len(unique_effective),
                "uniqueEffectiveTexturePaths": unique_effective,
                "sourceExactTextureBindingAdmission": True,
                "runtimeDdsParityAdmission": runtime_dds_admitted,
                "sourceExactSamplerAdmission": sampler_admitted,
                "sourceValueTextureSamplerAdmission": (
                    runtime_dds_admitted and sampler_admitted
                ),
                "blockers": sorted(target_blockers),
            }
        )

    exact_targets = [
        row for row in targets if row["status"] != "BLOCKED_UPSTREAM_NO_EXACT_MATERIAL_MAP"
    ]
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": exact.get("identity"),
        "scope": {
            "stage": "G03_5_EXACT_TEXTURE_BINDING_SAMPLER_EVIDENCE",
            "classNeutralExtractor": True,
            "authoredGenericResourceSlotsRead": False,
            "sourceExactTextureBindingAdmission": all(
                row["sourceExactTextureBindingAdmission"] for row in exact_targets
            ),
            "sourceExactSamplerAdmission": all(
                row["sourceExactSamplerAdmission"] for row in exact_targets
            ),
            "runtimeAdmission": False,
            "visualAdmission": False,
        },
        "inputs": {
            "exactMaterialMapReceipt": source_descriptor(
                exact_receipt_path, "CANONICAL_G03_3_INPUT"
            ),
            "resourceSourceManifest": source_descriptor(
                resource_manifest_path, "SOURCE_ASSET_TO_PHYSICAL_PACKAGE_INDEX"
            ),
            "materialDdsReceipt": source_descriptor(
                material_dds_receipt_path, "MATCHED_MATERIAL_UMODEL_DDS_EVIDENCE"
            ),
            "rawDdsReceipt": source_descriptor(
                raw_dds_receipt_path, "AUTHENTICATED_RAW_DDS_EVIDENCE"
            ),
            "extractor": source_descriptor(
                Path(__file__).resolve(), "TRACKED_SOURCE"
            ),
        },
        "targets": targets,
        "summary": {
            "targetCount": len(targets),
            "exactTargetCount": len(exact_targets),
            "upstreamBlockedTargetCount": len(targets) - len(exact_targets),
            "uniformTextureBindingCount": sum(
                row["uniformTextureBindingCount"] for row in exact_targets
            ),
            "sourceExactTextureBindingCount": sum(
                row["sourceExactTextureBindingAdmission"] for row in exact_targets
            ),
            "runtimeDdsParityTargetCount": sum(
                row["runtimeDdsParityAdmission"] for row in exact_targets
            ),
            "sourceExactSamplerTargetCount": sum(
                row["sourceExactSamplerAdmission"] for row in exact_targets
            ),
            "sourceValueTextureSamplerTargetCount": sum(
                row["sourceValueTextureSamplerAdmission"] for row in exact_targets
            ),
            "uniqueEffectiveTextureCount": len(
                {
                    binding["effectiveSourceObjectPath"]
                    for row in exact_targets
                    for binding in row["uniformTextureBindings"]
                }
            ),
            "result": (
                "PASS_G03_5_EXACT_TEXTURE_BINDINGS_"
                "SAMPLER_AND_RUNTIME_DDS_BLOCKERS_EXPLICIT"
            ),
        },
    }
    sealed = dict(receipt)
    receipt["receiptSha256"] = canonical_json_sha256(sealed)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "receipt schema mismatch")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "receipt version mismatch")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(sealed), "receipt digest mismatch")
    summary = receipt.get("summary", {})
    require(summary.get("exactTargetCount") == 5, "W exact target denominator changed")
    require(summary.get("upstreamBlockedTargetCount") == 1, "W blocked denominator changed")
    require(summary.get("uniformTextureBindingCount") == 24, "W texture wire denominator changed")
    require(summary.get("sourceExactTextureBindingCount") == 5, "exact texture closure failed")
    require(summary.get("uniqueEffectiveTextureCount") == 23, "effective texture denominator changed")
    require(summary.get("sourceExactSamplerTargetCount") == 0, "sampler blocker unexpectedly changed")
    require(summary.get("runtimeDdsParityTargetCount") == 4, "runtime DDS parity denominator changed")
    require(
        receipt.get("scope", {}).get("runtimeAdmission") is False
        and receipt.get("scope", {}).get("visualAdmission") is False,
        "runtime or visual admission must remain false",
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exact-receipt", type=Path, default=DEFAULT_EXACT_RECEIPT)
    parser.add_argument("--resource-manifest", type=Path, default=DEFAULT_RESOURCE_MANIFEST)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--material-dds-receipt", type=Path, default=DEFAULT_MATERIAL_DDS_RECEIPT)
    parser.add_argument("--material-dds-root", type=Path, default=DEFAULT_MATERIAL_DDS_ROOT)
    parser.add_argument("--raw-dds-receipt", type=Path, default=DEFAULT_RAW_DDS_RECEIPT)
    parser.add_argument("--raw-dds-root", type=Path, default=DEFAULT_RAW_DDS_ROOT)
    parser.add_argument("--runtime-texture-root", type=Path, default=DEFAULT_RUNTIME_TEXTURE_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)

    receipt = build_receipt(
        args.exact_receipt,
        args.resource_manifest,
        args.source_root,
        args.material_dds_receipt,
        args.material_dds_root,
        args.raw_dds_receipt,
        args.raw_dds_root,
        args.runtime_texture_root,
    )
    validate_receipt(receipt)
    if args.check:
        require(args.output.is_file(), f"receipt missing: {args.output}")
        require(read_json(args.output) == receipt, f"receipt stale: {args.output}")
    else:
        write_json_atomic(args.output, receipt)
    summary = receipt["summary"]
    print(
        "PASS G03-5 texture/sampler closure "
        f"targets={summary['exactTargetCount']} "
        f"bindings={summary['uniformTextureBindingCount']} "
        f"textures={summary['uniqueEffectiveTextureCount']} "
        f"samplerExact={summary['sourceExactSamplerTargetCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
