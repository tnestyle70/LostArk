#!/usr/bin/env python3
"""Close source-exact UE3 material texture bindings for cooked shader variants.

The input is a class-neutral G03 material-map receipt.  For every exact target
this tool joins, in order:

* the parent Material's cooked ``ReferencedTextures`` list;
* the selected MIC resource's effective ``ReferencedTextures`` list;
* raw MIC texture overrides keyed by the full ``FName`` (base + number);
* uniform-expression index -> native t/s binding wire;
* the exact source Texture2D export and authenticated extracted DDS bytes; and
* serialized Texture2D address/filter/sRGB fields; and
* the official v975 Engine Texture CDO/UEnum source-revision defaults.

Texture properties use one precedence rule: serialized Texture2D property,
source-revision CDO serialized property, then unresolved native constructor.
The protected TextureLODSettings hardware-filter projection remains blocked.
Authored generic resource slots are never consulted.
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
DEFAULT_OFFICIAL_V975_ROOT = Path(
    "C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/ARTIST/"
    "31470_TrackA_20260812/OfficialRefShaderCacheV974"
)
DEFAULT_OFFICIAL_V975_MANIFEST = DEFAULT_OFFICIAL_V975_ROOT / "45_975.json"
DEFAULT_OFFICIAL_V975_ENGINE_PACKAGE = (
    DEFAULT_OFFICIAL_V975_ROOT / "NE1FENCQ4UNE9ZPRENOQS.v975.u"
)

OFFICIAL_V975_MANIFEST_SHA256 = (
    "331bfb3ef14cafc5a31f9006bc7590540589d61718527f2668eeb58ee7ec96e9"
)
OFFICIAL_V975_ENGINE_LOGICAL_PATH = "/EFGame/ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u"
OFFICIAL_V975_ENGINE_PHYSICAL_SHA256 = (
    "3b6de4c2cf785174d3cbeb8c9b31bdec846bf51737bcee0887160dcf64d58c3f"
)
OFFICIAL_V975_ENGINE_LOGICAL_SHA256 = (
    "b64c0d72c5d7479c9a98cd3ea40b085341894a30c8694a0e9b6bcee91d6964fd"
)
OFFICIAL_V975_ENGINE_RAW_MD5 = "d24d1cb9e034558aec501a4adbc7553d"
OFFICIAL_V975_ENGINE_RAW_BYTE_SIZE = 1_397_047
OFFICIAL_V975_ENGINE_PACKED_MD5 = "cb27c7febb8553208465913a9f5415f5"
OFFICIAL_V975_ENGINE_PACKED_BYTE_SIZE = 1_142_697

SOURCE_REVISION_CDO_EXPECTATIONS = {
    "Default__Texture": {
        "className": "texture",
        "exportIndexZeroBased": 9747,
        "archetypePath": "Default__Surface",
        "serialOffset": 3_754_293,
        "serialByteSize": 351,
        "serialSha256": (
            "630d85dd8451cd9be1cbafbb75422f08b2e854e04b62ced8fd7ec03a6a2e6516"
        ),
        "propertyStreamSha256": (
            "c727a72743714d4d891a03ac8185556839359086c447e92434a89c7acc9ee9d8"
        ),
    },
    "Default__Texture2D": {
        "className": "texture2d",
        "exportIndexZeroBased": 9831,
        "archetypePath": "Default__Texture",
        "serialOffset": 3_758_613,
        "serialByteSize": 101,
        "serialSha256": (
            "6fa2b3c999339b71b228a3fb1c8ec5d5c4dba920fedc1a93004bcfd568e1c086"
        ),
        "propertyStreamSha256": (
            "5c387eee6f93eccc70116ca9e5f4434dbe9179d5647d6efc01862aeaa00ed64e"
        ),
    },
}

SOURCE_REVISION_ENUM_EXPECTATIONS = {
    "TextureFilter": {
        "exportIndexZeroBased": 9661,
        "serialOffset": 3_749_475,
        "serialByteSize": 44,
        "serialSha256": (
            "84074bed4c695561388b0d3f1488b6adff2b212989fec65a999ab8d7461a8a93"
        ),
        "values": ["TF_Nearest", "TF_Linear", "TF_MAX"],
    },
    "TextureAddress": {
        "exportIndexZeroBased": 9662,
        "serialOffset": 3_749_519,
        "serialByteSize": 52,
        "serialSha256": (
            "5c0eb6470a0f5d244ccf84d265c38ec3ed6456c02cb50c6ea502d02dbca03aff"
        ),
        "values": ["TA_Wrap", "TA_Clamp", "TA_Mirror", "TA_MAX"],
    },
    "TextureGroup": {
        "exportIndexZeroBased": 9663,
        "serialOffset": 3_749_571,
        "serialByteSize": 284,
        "serialSha256": (
            "69b375fdbf273f59fde2efae75d047d25089a198dec8797c52e6b386138211e3"
        ),
        "valueCount": 33,
        "requiredOrdinals": {
            "TEXTUREGROUP_Effects": 13,
            "TEXTUREGROUP_EffectsNotFiltered": 14,
            "TEXTUREGROUP_EffectsNormalMap": 31,
            "TEXTUREGROUP_MAX": 32,
        },
    },
}


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


def md5_file(path: Path) -> str:
    digest = hashlib.md5()
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
        newline="\n",
    )
    temporary.replace(path)


def source_descriptor(path: Path, role: str) -> dict[str, Any]:
    resolved = path.resolve()
    try:
        display_path = resolved.relative_to(REPOSITORY_ROOT.resolve()).as_posix()
    except ValueError:
        display_path = resolved.as_posix()
    return {
        "path": display_path,
        "byteSize": resolved.stat().st_size,
        "sha256": sha256_file(resolved),
        "role": role,
    }


def official_manifest_file_evidence(
    manifest: dict[str, Any], logical_path: str
) -> dict[str, Any]:
    matches = []
    for raw in manifest.get("files", []):
        require(isinstance(raw, str), "official manifest file row is not text")
        parts = [part.strip() for part in raw.split("|")]
        if len(parts) >= 12 and folded(parts[2]) == folded(logical_path):
            matches.append(parts)
    require(
        len(matches) == 1,
        f"official manifest path is not unique: {logical_path}",
    )
    parts = matches[0]
    require(parts[1] in {"F", "X"}, "official manifest row is not a file")
    require(parts[5] in {"gz", "xd"}, "official manifest codec changed")
    return {
        "manifestOrdinal": int(parts[0]),
        "entryType": parts[1],
        "logicalPath": parts[2],
        "version": int(parts[3]),
        "sequence": int(parts[4]),
        "codec": parts[5],
        "rawByteSize": int(parts[6]),
        "packedByteSize": int(parts[7]),
        "rawMd5": parts[8],
        "packedMd5": parts[9],
        "cdnRelativePath": f"v{int(parts[3])}/{int(parts[4])}.gz",
    }


def unique_export_by_name(package: Any, object_name: str, class_name: str) -> Any:
    matches = [
        entry
        for entry in package.exports
        if folded(entry.object_name) == folded(object_name)
        and folded(
            package_ref_name(entry.class_index, package.imports, package.exports)
        )
        == folded(class_name)
    ]
    require(
        len(matches) == 1,
        f"expected one {class_name} export named {object_name} (found {len(matches)})",
    )
    return matches[0]


def enum_export_evidence(
    package: Any, object_name: str, expectation: dict[str, Any]
) -> dict[str, Any]:
    entry = unique_export_by_name(package, object_name, "Enum")
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    require(entry.index == expectation["exportIndexZeroBased"], f"{object_name} export index changed")
    require(entry.serial_offset == expectation["serialOffset"], f"{object_name} serial offset changed")
    require(entry.serial_size == expectation["serialByteSize"], f"{object_name} serial size changed")
    require(sha256_bytes(serial) == expectation["serialSha256"], f"{object_name} serial changed")
    require(len(serial) >= 20, f"{object_name} enum serial is truncated")
    count = struct.unpack_from("<i", serial, 16)[0]
    require(count >= 0 and 20 + count * 8 == len(serial), f"{object_name} enum shape changed")
    values = []
    for ordinal in range(count):
        name_index, name_number = struct.unpack_from("<ii", serial, 20 + ordinal * 8)
        require(0 <= name_index < len(package.names), f"{object_name} enum FName is invalid")
        require(name_number == 0, f"{object_name} enum FName number changed")
        values.append(package.names[name_index])
    if "values" in expectation:
        require(values == expectation["values"], f"{object_name} values changed")
    else:
        require(count == expectation["valueCount"], f"{object_name} value count changed")
        for value, ordinal in expectation["requiredOrdinals"].items():
            require(values[ordinal] == value, f"{object_name} ordinal changed: {value}")
    return {
        "objectPath": package_ref_path(entry.index + 1, package.imports, package.exports),
        "exportIndexZeroBased": entry.index,
        "serialOffset": entry.serial_offset,
        "serialByteSize": entry.serial_size,
        "serialSha256": sha256_bytes(serial),
        "enumPrefixSha256": sha256_bytes(serial[:16]),
        "valueCount": count,
        "values": [
            {"ordinal": ordinal, "name": value}
            for ordinal, value in enumerate(values)
        ],
        "fidelity": "SOURCE_EXACT_V975_UENUM_SERIAL",
    }


def cdo_export_evidence(
    package: Any, object_name: str, expectation: dict[str, Any]
) -> dict[str, Any]:
    entry = unique_export_by_name(package, object_name, expectation["className"])
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    require(entry.index == expectation["exportIndexZeroBased"], f"{object_name} export index changed")
    require(entry.serial_offset == expectation["serialOffset"], f"{object_name} serial offset changed")
    require(entry.serial_size == expectation["serialByteSize"], f"{object_name} serial size changed")
    require(sha256_bytes(serial) == expectation["serialSha256"], f"{object_name} serial changed")
    archetype_path = package_ref_path(
        entry.archetype_index, package.imports, package.exports
    )
    require(archetype_path == expectation["archetypePath"], f"{object_name} archetype changed")
    records, property_start, property_end = parse_property_records(
        serial, package.names, package.summary.version
    )
    require(
        sha256_bytes(serial[property_start:property_end])
        == expectation["propertyStreamSha256"],
        f"{object_name} property stream changed",
    )
    fields = {
        name: property_evidence(records, name)
        for name in ("addressx", "addressy", "srgb", "filter", "lodgroup")
    }
    return {
        "objectPath": package_ref_path(entry.index + 1, package.imports, package.exports),
        "className": expectation["className"],
        "exportIndexZeroBased": entry.index,
        "archetypePackageReference": entry.archetype_index,
        "archetypePath": archetype_path,
        "serialOffset": entry.serial_offset,
        "serialByteSize": entry.serial_size,
        "serialSha256": sha256_bytes(serial),
        "propertyStreamStart": property_start,
        "propertyStreamEnd": property_end,
        "propertyStreamSha256": sha256_bytes(serial[property_start:property_end]),
        "fields": fields,
        "fidelity": "SOURCE_EXACT_V975_TEXTURE_CDO_SERIAL",
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
        "tagOffset": row["tagOffset"],
        "valueOffset": row["valueOffset"],
        "recordEndOffset": row["recordEndOffset"],
        "recordSha256": row["recordSha256"],
        "encodedValueSha256": row["encodedValueSha256"],
    }


def source_revision_texture_abi_evidence(
    official_manifest_path: Path, engine_package_path: Path
) -> dict[str, Any]:
    require(official_manifest_path.is_file(), f"official manifest missing: {official_manifest_path}")
    require(engine_package_path.is_file(), f"source Engine package missing: {engine_package_path}")
    require(
        sha256_file(official_manifest_path) == OFFICIAL_V975_MANIFEST_SHA256,
        "official v975 manifest identity changed",
    )
    require(
        sha256_file(engine_package_path) == OFFICIAL_V975_ENGINE_PHYSICAL_SHA256,
        "official v975 Engine package identity changed",
    )
    manifest = read_json(official_manifest_path)
    require(manifest.get("version_no") == 975, "official manifest revision changed")
    manifest_row = official_manifest_file_evidence(
        manifest, OFFICIAL_V975_ENGINE_LOGICAL_PATH
    )
    require(manifest_row["version"] == 975, "Engine manifest version changed")
    require(manifest_row["sequence"] == 14, "Engine manifest sequence changed")
    require(
        manifest_row["rawByteSize"] == OFFICIAL_V975_ENGINE_RAW_BYTE_SIZE
        and manifest_row["rawMd5"] == OFFICIAL_V975_ENGINE_RAW_MD5,
        "Engine manifest raw identity changed",
    )
    require(
        manifest_row["packedByteSize"] == OFFICIAL_V975_ENGINE_PACKED_BYTE_SIZE
        and manifest_row["packedMd5"] == OFFICIAL_V975_ENGINE_PACKED_MD5,
        "Engine manifest packed identity changed",
    )
    require(
        engine_package_path.stat().st_size == manifest_row["rawByteSize"]
        and md5_file(engine_package_path) == manifest_row["rawMd5"],
        "Engine package disagrees with official manifest",
    )

    package = load_package(engine_package_path, LOSTARK_KR_AES_KEY)
    summary = package.summary
    require(sha256_bytes(package.logical) == OFFICIAL_V975_ENGINE_LOGICAL_SHA256, "Engine logical package changed")
    require(
        summary.version == 868
        and summary.licensee_version == 16
        and summary.engine_version == 12097
        and summary.cooker_version == 136
        and summary.package_guid_hex == "8fb2f40c3e328a478b44f5e14511c09d"
        and summary.name_count == 21134
        and summary.import_count == 185
        and summary.export_count == 32875,
        "Engine package summary changed",
    )

    default_texture = cdo_export_evidence(
        package,
        "Default__Texture",
        SOURCE_REVISION_CDO_EXPECTATIONS["Default__Texture"],
    )
    default_texture2d = cdo_export_evidence(
        package,
        "Default__Texture2D",
        SOURCE_REVISION_CDO_EXPECTATIONS["Default__Texture2D"],
    )
    require(
        all(
            default_texture2d["fields"][name]["status"] == "OMITTED_FROM_EXPORT"
            for name in ("addressx", "addressy", "srgb", "filter", "lodgroup")
        ),
        "Default__Texture2D relevant override set changed",
    )
    require(
        default_texture["fields"]["addressx"]["status"] == "OMITTED_FROM_EXPORT"
        and default_texture["fields"]["addressy"]["status"] == "OMITTED_FROM_EXPORT"
        and default_texture["fields"]["lodgroup"]["status"] == "OMITTED_FROM_EXPORT",
        "Default__Texture native-default boundary changed",
    )
    srgb_default = default_texture["fields"]["srgb"]
    require(
        srgb_default["status"] == "SERIALIZED_EXPLICIT"
        and srgb_default["value"] is True
        and srgb_default["tagOffset"] == 4
        and srgb_default["valueOffset"] == 28
        and srgb_default["recordEndOffset"] == 29
        and srgb_default["recordSha256"]
        == "a3f8d954fd1e7446518a66b0e796d019755c2ee7f0dc15cfd347e67410e64e61",
        "Default__Texture SRGB record changed",
    )
    filter_default = default_texture["fields"]["filter"]
    require(
        filter_default["status"] == "SERIALIZED_EXPLICIT"
        and folded(filter_default["value"]) == "tf_linear"
        and filter_default["tagOffset"] == 191
        and filter_default["valueOffset"] == 223
        and filter_default["recordEndOffset"] == 231
        and filter_default["recordSha256"]
        == "4c140c1453a336286281e8b0a4e80b17c0ffdde73bef0465494e6e046db7ef5c",
        "Default__Texture Filter record changed",
    )

    enums = {
        name: enum_export_evidence(package, name, expectation)
        for name, expectation in SOURCE_REVISION_ENUM_EXPECTATIONS.items()
    }
    require(
        "tf_default"
        not in {folded(row["name"]) for row in enums["TextureFilter"]["values"]},
        "v975 unexpectedly contains TF_Default",
    )

    effective_defaults: dict[str, dict[str, Any]] = {}
    for name in ("addressx", "addressy", "srgb", "filter", "lodgroup"):
        source = None
        for cdo in (default_texture2d, default_texture):
            if cdo["fields"][name]["status"] == "SERIALIZED_EXPLICIT":
                source = cdo
                break
        if source is None:
            effective_defaults[name] = {
                "status": "NATIVE_CONSTRUCTOR_UNRESOLVED",
                "fidelity": "SOURCE_REVISION_NATIVE_CONSTRUCTOR_NOT_SERIALIZED",
            }
            continue
        field = source["fields"][name]
        effective_defaults[name] = {
            "status": "SOURCE_REVISION_CDO_SERIALIZED",
            "declaringCdo": source["objectPath"],
            "propertyName": field["propertyName"],
            "propertyType": field["propertyType"],
            "value": field["value"],
            "tagOffset": field["tagOffset"],
            "valueOffset": field["valueOffset"],
            "recordEndOffset": field["recordEndOffset"],
            "recordSha256": field["recordSha256"],
            "encodedValueSha256": field["encodedValueSha256"],
            "fidelity": "SOURCE_EXACT_V975_CDO_SERIALIZED_PROPERTY",
        }

    evidence = {
        "sourceRevision": 975,
        "officialManifestRow": manifest_row,
        "enginePackage": {
            "logicalPath": OFFICIAL_V975_ENGINE_LOGICAL_PATH,
            "physicalByteSize": engine_package_path.stat().st_size,
            "physicalMd5": md5_file(engine_package_path),
            "physicalSha256": sha256_file(engine_package_path),
            "logicalByteSize": len(package.logical),
            "logicalSha256": sha256_bytes(package.logical),
            "packageVersion": summary.version,
            "licenseeVersion": summary.licensee_version,
            "engineVersion": summary.engine_version,
            "cookerVersion": summary.cooker_version,
            "packageGuidHex": summary.package_guid_hex,
            "nameCount": summary.name_count,
            "importCount": summary.import_count,
            "exportCount": summary.export_count,
        },
        "cdoChain": {
            "precedence": ["Default__Texture2D", "Default__Texture", "native-constructor"],
            "defaultTexture": default_texture,
            "defaultTexture2D": default_texture2d,
        },
        "effectiveSerializedDefaults": effective_defaults,
        "enums": enums,
        "textureLodSettings": {
            "finalHardwareFilterResolved": False,
            "status": "PROTECTED_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED",
            "fidelity": "BLOCKER_NOT_GENERIC_UE3_ASSUMPTION",
        },
        "sourceRevisionCdoClosed": True,
        "sourceRevisionEnumsClosed": True,
        "nativeConstructorDefaultsClosed": False,
        "textureLodSettingsHardwareFilterClosed": False,
    }
    validate_source_revision_texture_abi(evidence)
    return evidence


def validate_source_revision_texture_abi(evidence: dict[str, Any]) -> None:
    require(evidence.get("sourceRevision") == 975, "source revision evidence changed")
    manifest_row = evidence.get("officialManifestRow", {})
    require(
        manifest_row.get("logicalPath") == OFFICIAL_V975_ENGINE_LOGICAL_PATH
        and manifest_row.get("version") == 975
        and manifest_row.get("sequence") == 14
        and manifest_row.get("rawByteSize") == OFFICIAL_V975_ENGINE_RAW_BYTE_SIZE
        and manifest_row.get("rawMd5") == OFFICIAL_V975_ENGINE_RAW_MD5
        and manifest_row.get("packedByteSize") == OFFICIAL_V975_ENGINE_PACKED_BYTE_SIZE
        and manifest_row.get("packedMd5") == OFFICIAL_V975_ENGINE_PACKED_MD5,
        "official Engine manifest evidence changed",
    )
    package = evidence.get("enginePackage", {})
    require(
        package.get("physicalByteSize") == OFFICIAL_V975_ENGINE_RAW_BYTE_SIZE
        and package.get("physicalMd5") == OFFICIAL_V975_ENGINE_RAW_MD5
        and package.get("physicalSha256") == OFFICIAL_V975_ENGINE_PHYSICAL_SHA256
        and package.get("logicalByteSize") == 6_093_451
        and package.get("logicalSha256") == OFFICIAL_V975_ENGINE_LOGICAL_SHA256,
        "source Engine package evidence changed",
    )
    cdo_chain = evidence.get("cdoChain", {})
    require(
        cdo_chain.get("precedence")
        == ["Default__Texture2D", "Default__Texture", "native-constructor"],
        "source CDO precedence changed",
    )
    for key, object_name in (
        ("defaultTexture", "Default__Texture"),
        ("defaultTexture2D", "Default__Texture2D"),
    ):
        cdo = cdo_chain.get(key, {})
        expected = SOURCE_REVISION_CDO_EXPECTATIONS[object_name]
        require(
            cdo.get("exportIndexZeroBased") == expected["exportIndexZeroBased"]
            and cdo.get("archetypePath") == expected["archetypePath"]
            and cdo.get("serialOffset") == expected["serialOffset"]
            and cdo.get("serialByteSize") == expected["serialByteSize"]
            and cdo.get("serialSha256") == expected["serialSha256"]
            and cdo.get("propertyStreamSha256")
            == expected["propertyStreamSha256"],
            f"{object_name} receipt identity changed",
        )
    defaults = evidence.get("effectiveSerializedDefaults", {})
    require(
        defaults.get("srgb", {}).get("status") == "SOURCE_REVISION_CDO_SERIALIZED"
        and defaults.get("srgb", {}).get("value") is True,
        "source CDO SRGB default changed",
    )
    require(
        defaults.get("filter", {}).get("status") == "SOURCE_REVISION_CDO_SERIALIZED"
        and folded(defaults.get("filter", {}).get("value")) == "tf_linear",
        "source CDO Filter default changed",
    )
    require(
        all(
            defaults.get(name, {}).get("status") == "NATIVE_CONSTRUCTOR_UNRESOLVED"
            for name in ("addressx", "addressy", "lodgroup")
        ),
        "source native-default boundary changed",
    )
    enums = evidence.get("enums", {})
    for name, expected in SOURCE_REVISION_ENUM_EXPECTATIONS.items():
        enum = enums.get(name, {})
        require(
            enum.get("exportIndexZeroBased") == expected["exportIndexZeroBased"]
            and enum.get("serialOffset") == expected["serialOffset"]
            and enum.get("serialByteSize") == expected["serialByteSize"]
            and enum.get("serialSha256") == expected["serialSha256"],
            f"{name} receipt identity changed",
        )
    filter_values = [
        row.get("name") for row in enums.get("TextureFilter", {}).get("values", [])
    ]
    require(
        filter_values == SOURCE_REVISION_ENUM_EXPECTATIONS["TextureFilter"]["values"],
        "TextureFilter enum receipt changed",
    )
    address_values = [
        row.get("name") for row in enums.get("TextureAddress", {}).get("values", [])
    ]
    require(
        address_values == SOURCE_REVISION_ENUM_EXPECTATIONS["TextureAddress"]["values"],
        "TextureAddress enum receipt changed",
    )
    group_rows = enums.get("TextureGroup", {}).get("values", [])
    require(len(group_rows) == 33, "TextureGroup enum receipt changed")
    for value, ordinal in SOURCE_REVISION_ENUM_EXPECTATIONS["TextureGroup"][
        "requiredOrdinals"
    ].items():
        require(group_rows[ordinal].get("name") == value, f"TextureGroup receipt ordinal changed: {value}")
    require(
        evidence.get("sourceRevisionCdoClosed") is True
        and evidence.get("sourceRevisionEnumsClosed") is True
        and evidence.get("nativeConstructorDefaultsClosed") is False
        and evidence.get("textureLodSettingsHardwareFilterClosed") is False
        and evidence.get("textureLodSettings", {}).get("finalHardwareFilterResolved") is False,
        "source revision admission boundary changed",
    )


def sampler_projection(
    fields: dict[str, dict[str, Any]], source_revision_abi: dict[str, Any]
) -> dict[str, Any]:
    defaults = source_revision_abi["effectiveSerializedDefaults"]
    enum_values = {
        name: {folded(row["name"]) for row in enum_row["values"]}
        for name, enum_row in source_revision_abi["enums"].items()
    }
    blockers: list[str] = []

    def effective(field_name: str) -> dict[str, Any]:
        field = fields[field_name]
        if field["status"] == "SERIALIZED_EXPLICIT":
            return {
                "status": "SERIALIZED_EXPLICIT",
                "value": field.get("value"),
                "propertySource": "TEXTURE2D_EXPORT",
                "fidelity": "SOURCE_EXACT_SERIALIZED_PROPERTY",
            }
        default = defaults[field_name]
        if default["status"] == "SOURCE_REVISION_CDO_SERIALIZED":
            return {
                "status": "SOURCE_REVISION_CDO_INHERITED",
                "value": default.get("value"),
                "propertySource": default["declaringCdo"],
                "recordSha256": default["recordSha256"],
                "fidelity": "SOURCE_EXACT_V975_CDO_SERIALIZED_PROPERTY",
            }
        return {
            "status": "NATIVE_CONSTRUCTOR_UNRESOLVED",
            "value": None,
            "propertySource": "NATIVE_CONSTRUCTOR",
            "fidelity": "SOURCE_REVISION_NATIVE_CONSTRUCTOR_NOT_SERIALIZED",
        }

    def address(field_name: str, blocker: str) -> dict[str, Any]:
        result = effective(field_name)
        if result["status"] == "NATIVE_CONSTRUCTOR_UNRESOLVED":
            blockers.append(blocker)
            result["sourceExact"] = False
            return result
        token = folded(result["value"])
        require(
            token in enum_values["TextureAddress"] and token != "ta_max",
            f"unsupported {field_name}: {token}",
        )
        result["value"] = token
        result["sourceExact"] = True
        return result

    address_u = address("addressx", "ADDRESS_U_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED")
    address_v = address("addressy", "ADDRESS_V_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED")

    srgb = effective("srgb")
    if srgb["status"] == "NATIVE_CONSTRUCTOR_UNRESOLVED":
        blockers.append("COLOR_SPACE_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED")
        color_space = {**srgb, "sourceExact": False}
    else:
        require(type(srgb.get("value")) is bool, "Texture2D sRGB is not bool")
        color_space = {
            **srgb,
            "value": "srgb" if srgb["value"] else "linear",
            "sourceExact": True,
        }

    filter_field = effective("filter")
    if filter_field["status"] == "NATIVE_CONSTRUCTOR_UNRESOLVED":
        blockers.append("FILTER_SELECTOR_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED")
        selector = None
        filter_selector = {**filter_field, "sourceExact": False}
    else:
        selector = folded(filter_field.get("value"))
        require(
            selector in enum_values["TextureFilter"] and selector != "tf_max",
            f"unsupported Texture2D filter for v975: {selector}",
        )
        filter_selector = {
            **filter_field,
            "value": selector,
            "sourceExact": True,
        }

    lod_field = effective("lodgroup")
    if lod_field["status"] == "NATIVE_CONSTRUCTOR_UNRESOLVED":
        blockers.append("LOD_GROUP_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED")
        lod_projection = {**lod_field, "sourceExact": False}
    else:
        lod_value = folded(lod_field.get("value"))
        require(
            lod_value in enum_values["TextureGroup"]
            and lod_value != "texturegroup_max",
            f"unsupported Texture2D LODGroup for v975: {lod_value}",
        )
        lod_projection = {**lod_field, "value": lod_value, "sourceExact": True}

    hardware_filter_exact = False
    if selector is not None:
        blockers.append(
            "FILTER_"
            + selector.upper()
            + "_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED"
        )
    hardware_filter = {
        "value": None,
        "selector": selector,
        "sourceExact": hardware_filter_exact,
        "fidelity": "PROTECTED_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED",
    }

    source_exact_sampler = (
        address_u["sourceExact"]
        and address_v["sourceExact"]
        and color_space["sourceExact"]
        and filter_selector["sourceExact"]
        and lod_projection["sourceExact"]
        and hardware_filter_exact
    )
    return {
        "precedence": "TEXTURE2D_EXPLICIT_THEN_SOURCE_CDO_THEN_NATIVE_UNRESOLVED",
        "addressU": address_u,
        "addressV": address_v,
        "filterSelector": filter_selector,
        "lodGroup": lod_projection,
        "hardwareFilter": hardware_filter,
        "resolvedFilter": None,
        "colorSpace": color_space,
        "sourceExactColorSpace": color_space["sourceExact"],
        "sourceExactFilterSelector": filter_selector["sourceExact"],
        "sourceExactAddressAxisCount": int(address_u["sourceExact"])
        + int(address_v["sourceExact"]),
        "sourceExactLodGroup": lod_projection["sourceExact"],
        "sourceExactHardwareFilter": hardware_filter_exact,
        "sourceExactSamplerAndColorSpace": source_exact_sampler,
        "blockers": sorted(set(blockers)),
    }


def texture_export_evidence(
    source_path: str,
    asset: dict[str, Any],
    source_root: Path,
    package_cache: dict[str, Any],
    source_revision_abi: dict[str, Any],
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
        "samplerAndColorSpace": sampler_projection(fields, source_revision_abi),
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
    official_manifest_path: Path,
    engine_package_path: Path,
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
    source_revision_abi = source_revision_texture_abi_evidence(
        official_manifest_path, engine_package_path
    )
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
                    source_path,
                    texture_asset,
                    source_root,
                    package_cache,
                    source_revision_abi,
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
        source_exact_color_space_count = sum(
            binding["sourceTexture2D"]["samplerAndColorSpace"][
                "sourceExactColorSpace"
            ]
            for binding in bindings
        )
        source_exact_filter_selector_count = sum(
            binding["sourceTexture2D"]["samplerAndColorSpace"][
                "sourceExactFilterSelector"
            ]
            for binding in bindings
        )
        source_exact_address_axis_count = sum(
            binding["sourceTexture2D"]["samplerAndColorSpace"][
                "sourceExactAddressAxisCount"
            ]
            for binding in bindings
        )
        source_exact_lod_group_count = sum(
            binding["sourceTexture2D"]["samplerAndColorSpace"][
                "sourceExactLodGroup"
            ]
            for binding in bindings
        )
        source_exact_hardware_filter_count = sum(
            binding["sourceTexture2D"]["samplerAndColorSpace"][
                "sourceExactHardwareFilter"
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
                "sourceExactColorSpaceBindingCount": source_exact_color_space_count,
                "sourceExactFilterSelectorBindingCount": source_exact_filter_selector_count,
                "sourceExactAddressAxisCount": source_exact_address_axis_count,
                "sourceExactAddressAxisDenominator": len(bindings) * 2,
                "sourceExactLodGroupBindingCount": source_exact_lod_group_count,
                "sourceExactHardwareFilterBindingCount": source_exact_hardware_filter_count,
                "sourceValueTextureSamplerAdmission": (
                    runtime_dds_admitted and sampler_admitted
                ),
                "blockers": sorted(target_blockers),
            }
        )

    exact_targets = [
        row for row in targets if row["status"] != "BLOCKED_UPSTREAM_NO_EXACT_MATERIAL_MAP"
    ]
    glasshole = next(
        row
        for row in exact_targets
        if row["targetId"] == "dimensionmaster-w-glasshole-02"
    )
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": exact.get("identity"),
        "scope": {
            "stage": "G03_6_SOURCE_REVISION_TEXTURE_CDO_SAMPLER_EVIDENCE",
            "classNeutralExtractor": True,
            "authoredGenericResourceSlotsRead": False,
            "sourceRevisionTextureCdoClosed": True,
            "sourceRevisionTextureEnumsClosed": True,
            "sourceRevisionNativeConstructorDefaultsClosed": False,
            "sourceRevisionTextureLodSettingsHardwareFilterClosed": False,
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
            "officialV975Manifest": source_descriptor(
                official_manifest_path, "PINNED_OFFICIAL_SOURCE_REVISION_MANIFEST"
            ),
            "officialV975EnginePackage": source_descriptor(
                engine_package_path, "PINNED_OFFICIAL_SOURCE_REVISION_ENGINE_PACKAGE"
            ),
            "extractor": source_descriptor(
                Path(__file__).resolve(), "TRACKED_SOURCE"
            ),
        },
        "sourceRevisionTextureAbi": source_revision_abi,
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
            "glasshole02SamplerEvidence": {
                "bindingCount": glasshole["uniformTextureBindingCount"],
                "sourceExactColorSpaceBindingCount": glasshole[
                    "sourceExactColorSpaceBindingCount"
                ],
                "sourceExactFilterSelectorBindingCount": glasshole[
                    "sourceExactFilterSelectorBindingCount"
                ],
                "sourceExactAddressAxisCount": glasshole[
                    "sourceExactAddressAxisCount"
                ],
                "sourceExactAddressAxisDenominator": glasshole[
                    "sourceExactAddressAxisDenominator"
                ],
                "sourceExactLodGroupBindingCount": glasshole[
                    "sourceExactLodGroupBindingCount"
                ],
                "sourceExactHardwareFilterBindingCount": glasshole[
                    "sourceExactHardwareFilterBindingCount"
                ],
                "sourceExactFullSamplerTargetCount": int(
                    glasshole["sourceExactSamplerAdmission"]
                ),
            },
            "uniqueEffectiveTextureCount": len(
                {
                    binding["effectiveSourceObjectPath"]
                    for row in exact_targets
                    for binding in row["uniformTextureBindings"]
                }
            ),
            "result": (
                "PASS_G03_6_SOURCE_REVISION_TEXTURE_CDO_ENUM_"
                "PARTIAL_SAMPLER_CLOSURE_FULL_SAMPLER_BLOCKED"
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
    validate_source_revision_texture_abi(receipt.get("sourceRevisionTextureAbi", {}))
    summary = receipt.get("summary", {})
    require(summary.get("exactTargetCount") == 5, "W exact target denominator changed")
    require(summary.get("upstreamBlockedTargetCount") == 1, "W blocked denominator changed")
    require(summary.get("uniformTextureBindingCount") == 24, "W texture wire denominator changed")
    require(summary.get("sourceExactTextureBindingCount") == 5, "exact texture closure failed")
    require(summary.get("uniqueEffectiveTextureCount") == 23, "effective texture denominator changed")
    require(summary.get("sourceExactSamplerTargetCount") == 0, "sampler blocker unexpectedly changed")
    require(summary.get("runtimeDdsParityTargetCount") == 5, "runtime DDS parity closure regressed")
    glass = summary.get("glasshole02SamplerEvidence", {})
    require(
        glass.get("bindingCount") == 7
        and glass.get("sourceExactColorSpaceBindingCount") == 7
        and glass.get("sourceExactFilterSelectorBindingCount") == 7
        and glass.get("sourceExactAddressAxisCount") == 1
        and glass.get("sourceExactAddressAxisDenominator") == 14
        and glass.get("sourceExactLodGroupBindingCount") == 6
        and glass.get("sourceExactHardwareFilterBindingCount") == 0
        and glass.get("sourceExactFullSamplerTargetCount") == 0,
        "Glasshole02 partial sampler evidence changed",
    )
    glasshole = next(
        (
            row
            for row in receipt.get("targets", [])
            if row.get("targetId") == "dimensionmaster-w-glasshole-02"
        ),
        None,
    )
    require(glasshole is not None, "Glasshole02 target missing")
    glass_blockers = set(glasshole.get("blockers", []))
    require(
        "COLOR_SPACE_SOURCE_REVISION_CDO_UNRESOLVED" not in glass_blockers
        and "FILTER_SELECTOR_SOURCE_REVISION_CDO_UNRESOLVED" not in glass_blockers
        and "FILTER_TF_DEFAULT_TEXTURELODSETTINGS_UNRESOLVED" not in glass_blockers
        and "FILTER_TF_LINEAR_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED"
        in glass_blockers
        and "ADDRESS_U_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED" in glass_blockers
        and "ADDRESS_V_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED" in glass_blockers
        and "LOD_GROUP_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED" in glass_blockers,
        "Glasshole02 blocker boundary changed",
    )
    require(
        receipt.get("scope", {}).get("sourceRevisionTextureCdoClosed") is True
        and receipt.get("scope", {}).get("sourceRevisionTextureEnumsClosed") is True
        and receipt.get("scope", {}).get(
            "sourceRevisionNativeConstructorDefaultsClosed"
        )
        is False
        and receipt.get("scope", {}).get(
            "sourceRevisionTextureLodSettingsHardwareFilterClosed"
        )
        is False
        and receipt.get("scope", {}).get("runtimeAdmission") is False
        and receipt.get("scope", {}).get("visualAdmission") is False,
        "source ABI, runtime, or visual admission boundary changed",
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
    parser.add_argument(
        "--official-v975-manifest",
        type=Path,
        default=DEFAULT_OFFICIAL_V975_MANIFEST,
    )
    parser.add_argument(
        "--official-v975-engine-package",
        type=Path,
        default=DEFAULT_OFFICIAL_V975_ENGINE_PACKAGE,
    )
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
        args.official_v975_manifest,
        args.official_v975_engine_package,
    )
    validate_receipt(receipt)
    if args.check:
        require(args.output.is_file(), f"receipt missing: {args.output}")
        require(read_json(args.output) == receipt, f"receipt stale: {args.output}")
    else:
        write_json_atomic(args.output, receipt)
    summary = receipt["summary"]
    print(
        "PASS G03-6 source-revision texture/sampler closure "
        f"targets={summary['exactTargetCount']} "
        f"bindings={summary['uniformTextureBindingCount']} "
        f"textures={summary['uniqueEffectiveTextureCount']} "
        f"glassColor={summary['glasshole02SamplerEvidence']['sourceExactColorSpaceBindingCount']}/7 "
        f"glassFilter={summary['glasshole02SamplerEvidence']['sourceExactFilterSelectorBindingCount']}/7 "
        f"samplerExact={summary['sourceExactSamplerTargetCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
