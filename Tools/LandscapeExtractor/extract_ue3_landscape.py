#!/usr/bin/env python3
"""Extract Lost Ark UE3 Landscape data and build CModel-compatible tiles.

The source of truth produced by this tool is the decoded UE3 Landscape data:
height/weight mip chains, layer allocations, material parameters, holes, and
component transforms.  The generated glTF/WModel files and baked textures are
display derivatives for the current single-material CModel runtime.
"""

from __future__ import annotations

import argparse
import binascii
import hashlib
import importlib.util
import json
import math
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import uuid
import zlib
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable, Sequence


SCRIPT_DIR = Path(__file__).resolve().parent
REPOSITORY_ROOT = SCRIPT_DIR.parents[1]
PLACEMENT_EXTRACTOR = (
    SCRIPT_DIR.parent / "LevelPlacementExtractor" / "extract_ue3_placements.py"
)
LANDSCAPE_ZSCALE = 1.0 / 128.0
IMPORTED_ID_BIT = 1 << 63
COMPONENT_GRID_SIZE = 63
COMPONENT_QUAD_SIZE = 62
MATERIAL_NAME = "LANDSCAPE_BAKED"
CLIFF_MATERIAL_NAME = "LANDSCAPE_CLIFF"
UE3_LANDSCAPE_HOLE_THRESHOLD = 170
CLIFF_BLEND_STEEP_UP = 0.35
CLIFF_BLEND_FLAT_UP = 0.75


class LandscapeError(RuntimeError):
    pass


def is_ue3_landscape_hole(weight: float) -> bool:
    """Match UE3 Landscape __DataLayer__'s strict collision/render cutoff."""
    return weight > UE3_LANDSCAPE_HOLE_THRESHOLD


def load_placement_module() -> Any:
    specification = importlib.util.spec_from_file_location(
        "lostark_ue3_placement", PLACEMENT_EXTRACTOR
    )
    if specification is None or specification.loader is None:
        raise LandscapeError(f"could not load {PLACEMENT_EXTRACTOR}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[specification.name] = module
    specification.loader.exec_module(module)
    return module


UE3 = load_placement_module()


@dataclass(frozen=True)
class TaggedRecord:
    name: str
    property_type: str
    struct_type: str | None
    array_index: int
    payload: bytes
    bool_value: bool | None
    enum_name: str | None


@dataclass(frozen=True)
class TextureMip:
    level: int
    width: int
    height: int
    bulk_flags: int
    element_count: int
    size_on_disk: int
    logical_offset: int
    bgra: bytes


@dataclass(frozen=True)
class DecodedTexture:
    export_index: int
    object_name: str
    pixel_format: str
    source_art_flags: int
    source_art_element_count: int
    source_art_size_on_disk: int
    source_art_offset: int
    mips: tuple[TextureMip, ...]


@dataclass(frozen=True)
class LayerAllocation:
    layer_name: str
    texture_index: int
    channel: int


@dataclass
class PackageData:
    logical_name: str
    physical_path: Path
    summary: Any
    logical: bytes
    names: list[str]
    imports: list[Any]
    exports: list[Any]


@dataclass
class LandscapeComponent:
    logical_package: str
    export_index: int
    object_name: str
    section_base_x: int
    section_base_y: int
    component_size_quads: int
    subsection_size_quads: int
    num_subsections: int
    heightmap_ref: int
    heightmap_name: str
    heightmap_path: str
    heightmap_scale_bias: tuple[float, float, float, float]
    weightmap_refs: list[int]
    weightmap_names: list[str]
    weightmap_paths: list[str]
    weightmap_scale_bias: tuple[float, float, float, float]
    weightmap_subsection_offset: float
    allocations: list[LayerAllocation]
    material_instance_ref: int
    material_instance_name: str
    cached_local_box: dict[str, list[float]]
    height_texture: DecodedTexture | None = None
    weight_textures: list[DecodedTexture] | None = None


@dataclass(frozen=True)
class LandscapeCollisionComponent:
    logical_package: str
    export_index: int
    object_name: str
    section_base_x: int
    section_base_y: int
    collision_size_quads: int
    collision_scale: float
    include_holes: bool
    component_layer_names: tuple[str, ...]
    heights: tuple[int, ...]
    dominant_layers: tuple[int, ...]


@dataclass
class LandscapeProxy:
    logical_package: str
    export_index: int
    object_name: str
    landscape_guid: str
    location: tuple[float, float, float]
    draw_scale: float
    draw_scale3d: tuple[float, float, float]
    component_size_quads: int
    subsection_size_quads: int
    num_subsections: int
    landscape_material_ref: int
    landscape_material_path: str
    landscape_material_name: str
    component_refs: list[int]
    collision_component_refs: list[int]


@dataclass(frozen=True)
class ImageRgba:
    width: int
    height: int
    pixels: tuple[tuple[int, int, int, int], ...]


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write_bytes(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "wb") as output:
            output.write(data)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def atomic_write_text(path: Path, text: str) -> None:
    atomic_write_bytes(path, text.encode("utf-8"))


def json_text(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2) + "\n"


def json_value(value: Any) -> Any:
    """Convert decoded UE values to a lossless JSON-safe representation."""
    if value is None or isinstance(value, (bool, int, float, str)):
        return value
    if isinstance(value, bytes):
        return {"rawHex": value.hex(), "size": len(value)}
    if isinstance(value, (list, tuple)):
        return [json_value(item) for item in value]
    if isinstance(value, dict):
        return {str(key): json_value(item) for key, item in value.items()}
    return {"pythonType": type(value).__name__, "text": str(value)}


def parse_records_at(
    serial_data: bytes,
    names: list[str],
    start_offset: int,
) -> tuple[list[TaggedRecord], int]:
    reader = UE3.Reader(serial_data, start_offset)
    records: list[TaggedRecord] = []
    while reader.offset < len(serial_data):
        property_name, _ = UE3.parse_fname(reader, names)
        if property_name.casefold() == "none":
            return records, reader.offset

        property_type, _ = UE3.parse_fname(reader, names)
        data_size = reader.i32()
        array_index = reader.i32()
        if data_size < 0:
            raise LandscapeError(f"negative property size for {property_name}")

        struct_type: str | None = None
        bool_value: bool | None = None
        enum_name: str | None = None
        type_key = property_type.casefold()
        if type_key == "structproperty":
            struct_type, _ = UE3.parse_fname(reader, names)
        elif type_key == "boolproperty":
            bool_value = bool(reader.read(1)[0])
        elif type_key == "byteproperty":
            enum_name, _ = UE3.parse_fname(reader, names)

        serialized_size = data_size + 8 if type_key == "intproperty" else data_size
        payload = reader.read(serialized_size)
        records.append(
            TaggedRecord(
                name=property_name,
                property_type=property_type,
                struct_type=struct_type,
                array_index=array_index,
                payload=payload,
                bool_value=bool_value,
                enum_name=enum_name,
            )
        )
    raise LandscapeError("tagged property stream has no None terminator")


def find_records(
    serial_data: bytes,
    names: list[str],
    package_version: int,
) -> tuple[list[TaggedRecord], int, int]:
    minimum_offset = 4 if package_version >= 322 else 0
    search_end = min(len(serial_data) - 20, 256)
    failures: list[str] = []
    for start_offset in range(minimum_offset, max(minimum_offset, search_end) + 1):
        try:
            name_index, name_number, type_index, type_number = struct.unpack_from(
                "<4i", serial_data, start_offset
            )
        except struct.error:
            break
        if (
            not 0 <= name_index < len(names)
            or name_number < 0
            or not 0 <= type_index < len(names)
            or type_number < 0
            or not names[type_index].casefold().endswith("property")
        ):
            continue
        try:
            records, end_offset = parse_records_at(serial_data, names, start_offset)
            return records, start_offset, end_offset
        except (LandscapeError, UE3.ExtractionError) as error:
            failures.append(f"0x{start_offset:X}: {error}")
    detail = failures[0] if failures else "no plausible first property tag"
    raise LandscapeError(f"could not locate tagged properties ({detail})")


def record_map(records: Sequence[TaggedRecord]) -> dict[str, TaggedRecord]:
    return {record.name.casefold(): record for record in records}


def decode_record(record: TaggedRecord, names: list[str]) -> Any:
    kind = record.property_type.casefold()
    structure = (record.struct_type or "").casefold()
    payload = record.payload
    if kind == "byteproperty":
        if len(payload) == 1:
            return payload[0]
        if len(payload) >= 8:
            return UE3.parse_fname(UE3.Reader(payload), names)[0]
    if kind == "structproperty" and structure in ("vector4", "linearcolor"):
        if len(payload) >= 16:
            return tuple(float(value) for value in struct.unpack_from("<4f", payload))
    if kind == "structproperty" and structure == "guid" and len(payload) >= 16:
        return payload[:16].hex()
    if kind == "structproperty" and structure == "box" and len(payload) >= 25:
        values = struct.unpack_from("<6fB", payload)
        return {
            "min": [float(value) for value in values[:3]],
            "max": [float(value) for value in values[3:6]],
            "isValid": bool(values[6]),
        }
    return UE3.decode_property_value(
        record.property_type,
        record.struct_type,
        record.payload,
        names,
        record.bool_value,
    )


def decoded_record_json(record: TaggedRecord, names: list[str]) -> Any:
    try:
        return json_value(decode_record(record, names))
    except (LandscapeError, UE3.ExtractionError, ValueError, struct.error) as error:
        return {
            "decodeError": str(error),
            "rawHex": record.payload.hex(),
            "size": len(record.payload),
        }


def value(
    records_by_name: dict[str, TaggedRecord],
    name: str,
    names: list[str],
    default: Any,
) -> Any:
    record = records_by_name.get(name.casefold())
    return default if record is None else decode_record(record, names)


def vector3(value: Any, default: Sequence[float]) -> tuple[float, float, float]:
    if isinstance(value, dict) and all(axis in value for axis in ("x", "y", "z")):
        return tuple(float(value[axis]) for axis in ("x", "y", "z"))
    return tuple(float(component) for component in default)  # type: ignore[return-value]


def vector4(value: Any, default: Sequence[float]) -> tuple[float, float, float, float]:
    if isinstance(value, tuple) and len(value) == 4:
        return tuple(float(component) for component in value)  # type: ignore[return-value]
    return tuple(float(component) for component in default)  # type: ignore[return-value]


def int_array(record: TaggedRecord | None) -> list[int]:
    if record is None or len(record.payload) < 4:
        return []
    count = struct.unpack_from("<i", record.payload, 0)[0]
    if count < 0 or len(record.payload) != 4 + count * 4:
        raise LandscapeError(f"invalid int array property {record.name}")
    return list(struct.unpack_from(f"<{count}i", record.payload, 4))


def fname_array(record: TaggedRecord | None, names: Sequence[str]) -> list[str]:
    if record is None or len(record.payload) < 4:
        return []
    count = struct.unpack_from("<i", record.payload, 0)[0]
    if count < 0 or len(record.payload) != 4 + count * 8:
        raise LandscapeError(f"invalid FName array property {record.name}")
    reader = UE3.Reader(record.payload, 4)
    result = [UE3.parse_fname(reader, list(names))[0] for _ in range(count)]
    if reader.offset != len(record.payload):
        raise LandscapeError(f"FName array property {record.name} has trailing bytes")
    return result


def tagged_struct_array(
    record: TaggedRecord,
    names: list[str],
) -> list[dict[str, Any]]:
    if len(record.payload) < 4:
        raise LandscapeError(f"short struct array {record.name}")
    count = struct.unpack_from("<i", record.payload, 0)[0]
    if count < 0 or count > 100000:
        raise LandscapeError(f"invalid struct array count {count} for {record.name}")
    offset = 4
    result: list[dict[str, Any]] = []
    for _ in range(count):
        records, offset = parse_records_at(record.payload, names, offset)
        result.append(
            {
                item.name.casefold(): decode_record(item, names)
                for item in records
            }
        )
    if offset != len(record.payload):
        raise LandscapeError(
            f"struct array {record.name} ended at {offset}, expected {len(record.payload)}"
        )
    return result


def object_path(package: PackageData, package_index: int) -> str | None:
    path = UE3.package_ref_path(package_index, package.imports, package.exports)
    if path is None:
        return None
    if package_index > 0 and "." in path:
        first = path.split(".", 1)[0].casefold()
        if first in ("tex", "landscape", "persistentlevel", "theworld"):
            return f"{package.logical_name.casefold()}.{path}"
    return path


def object_name(package: PackageData, package_index: int) -> str:
    return UE3.package_ref_name(package_index, package.imports, package.exports)


def class_name(package: PackageData, export: Any) -> str:
    return object_name(package, export.class_index)


def load_package(
    physical_path: Path,
    logical_name: str,
    aes_key: str,
) -> PackageData:
    physical = physical_path.read_bytes()
    summary = UE3.parse_summary(physical)
    logical = UE3.decompress_package(physical, summary, aes_key)
    names = UE3.parse_name_table(logical, summary)
    imports = UE3.parse_import_table(logical, summary, names)
    exports = UE3.parse_export_table(logical, summary, names)
    return PackageData(
        logical_name=logical_name,
        physical_path=physical_path,
        summary=summary,
        logical=logical,
        names=names,
        imports=imports,
        exports=exports,
    )


def export_serial(package: PackageData, export: Any) -> bytes:
    start = export.serial_offset
    end = start + export.serial_size
    if start < 0 or end > len(package.logical):
        raise LandscapeError(f"invalid serial range for export {export.index}")
    return package.logical[start:end]


def decompress_texture_bulk(payload: bytes, expected_size: int) -> bytes:
    if len(payload) == expected_size:
        return payload
    reader = UE3.Reader(payload)
    tag = reader.u32()
    block_size = reader.i32()
    sum_compressed = reader.i32()
    sum_uncompressed = reader.i32()
    if tag != UE3.PACKAGE_FILE_TAG:
        raise LandscapeError(f"wrong Texture2D bulk tag 0x{tag:08X}")
    if block_size <= 0 or sum_uncompressed != expected_size:
        raise LandscapeError("Texture2D bulk summary mismatch")
    block_count = math.ceil(sum_uncompressed / block_size)
    sizes = [reader.unpack("<II") for _ in range(block_count)]
    if sum(item[0] for item in sizes) != sum_compressed:
        raise LandscapeError("Texture2D block table compressed size mismatch")
    output = bytearray()
    for compressed_size, uncompressed_size in sizes:
        output.extend(
            UE3.decompress_lz4_block(reader.read(compressed_size), uncompressed_size)
        )
    if reader.offset != len(payload) or len(output) != expected_size:
        raise LandscapeError("Texture2D bulk payload length mismatch")
    return bytes(output)


def decode_texture(package: PackageData, texture_ref: int) -> DecodedTexture:
    if texture_ref <= 0:
        raise LandscapeError(f"Texture2D ref is not a local export: {texture_ref}")
    export_index = texture_ref - 1
    if not 0 <= export_index < len(package.exports):
        raise LandscapeError(f"Texture2D ref is outside ExportTable: {texture_ref}")
    export = package.exports[export_index]
    if class_name(package, export).casefold() != "texture2d":
        raise LandscapeError(f"export {export_index} is not Texture2D")

    serial = export_serial(package, export)
    records, _property_start, property_end = find_records(
        serial, package.names, package.summary.version
    )
    texture_properties = record_map(records)
    pixel_format = str(
        value(texture_properties, "format", package.names, "")
    )
    if pixel_format.casefold() != "pf_a8r8g8b8":
        raise LandscapeError(
            f"Texture2D {export.object_name} format is {pixel_format!r}, "
            "expected PF_A8R8G8B8"
        )
    reader = UE3.Reader(serial, property_end)
    source_art_flags = reader.i32()
    source_art_element_count = reader.i32()
    source_art_size_on_disk = reader.i32()
    source_art_offset = reader.i32()
    if source_art_size_on_disk:
        source_payload = reader.read(source_art_size_on_disk)
        if source_art_element_count and len(source_payload) != source_art_element_count:
            raise LandscapeError("unexpected inline Texture2D SourceArt payload")

    mip_count = reader.i32()
    if mip_count <= 0 or mip_count > 32:
        raise LandscapeError(f"invalid Texture2D mip count {mip_count}")
    mips: list[TextureMip] = []
    for level in range(mip_count):
        bulk_flags = reader.i32()
        element_count = reader.i32()
        size_on_disk = reader.i32()
        logical_offset = reader.i32()
        payload_offset = export.serial_offset + reader.offset
        payload = reader.read(size_on_disk)
        width = reader.i32()
        height = reader.i32()
        if logical_offset != payload_offset:
            raise LandscapeError(
                f"Texture2D mip {level} offset {logical_offset:#x} != {payload_offset:#x}"
            )
        decoded = decompress_texture_bulk(payload, element_count)
        if element_count != width * height * 4:
            raise LandscapeError(
                f"Texture2D mip {level} is not PF_A8R8G8B8: "
                f"{element_count} != {width}*{height}*4"
            )
        mips.append(
            TextureMip(
                level=level,
                width=width,
                height=height,
                bulk_flags=bulk_flags,
                element_count=element_count,
                size_on_disk=size_on_disk,
                logical_offset=logical_offset,
                bgra=decoded,
            )
        )

    if len(serial) - reader.offset != 48:
        raise LandscapeError(
            f"Texture2D {export.object_name} tail is {len(serial) - reader.offset}, expected 48"
        )
    return DecodedTexture(
        export_index=export.index,
        object_name=export.object_name,
        pixel_format=pixel_format,
        source_art_flags=source_art_flags,
        source_art_element_count=source_art_element_count,
        source_art_size_on_disk=source_art_size_on_disk,
        source_art_offset=source_art_offset,
        mips=tuple(mips),
    )


def decode_layer_allocations(
    record: TaggedRecord,
    names: list[str],
) -> list[LayerAllocation]:
    result: list[LayerAllocation] = []
    for item in tagged_struct_array(record, names):
        layer_name = str(item.get("layername", ""))
        texture_index = int(item.get("weightmaptextureindex", -1))
        channel = int(item.get("weightmaptexturechannel", -1))
        if not layer_name or texture_index < 0 or not 0 <= channel <= 3:
            raise LandscapeError(f"invalid WeightmapLayerAllocation {item}")
        result.append(LayerAllocation(layer_name, texture_index, channel))
    return result


def parse_material_instance(
    package: PackageData,
    material_ref: int,
) -> dict[str, Any]:
    if material_ref <= 0:
        return {
            "ref": material_ref,
            "path": object_path(package, material_ref),
            "name": object_name(package, material_ref),
        }
    export = package.exports[material_ref - 1]
    serial = export_serial(package, export)
    records, start, end = find_records(serial, package.names, package.summary.version)
    by_name = record_map(records)
    result: dict[str, Any] = {
        "ref": material_ref,
        "exportIndex": export.index,
        "name": export.object_name,
        "class": class_name(package, export),
        "path": object_path(package, material_ref),
        "propertyStart": start,
        "propertyEnd": end,
        "serialSize": export.serial_size,
        "serialPrefixSize": start,
        "serialTailSize": len(serial) - end,
        "parentRef": int(value(by_name, "parent", package.names, 0)),
        "taggedProperties": [
            {
                "name": record.name,
                "type": record.property_type,
                "structType": record.struct_type,
                "arrayIndex": record.array_index,
                "boolValue": record.bool_value,
                "enumName": record.enum_name,
                "payloadSize": len(record.payload),
                "payloadSha256": sha256_bytes(record.payload),
                "decoded": decoded_record_json(record, package.names),
            }
            for record in records
        ],
    }
    result["parentPath"] = object_path(package, result["parentRef"])
    for property_name, output_name in (
        ("textureparametervalues", "textureParameters"),
        ("scalarparametervalues", "scalarParameters"),
        ("vectorparametervalues", "vectorParameters"),
    ):
        record = by_name.get(property_name)
        if record is None:
            result[output_name] = []
            continue
        parameters = tagged_struct_array(record, package.names)
        converted: list[dict[str, Any]] = []
        for parameter in parameters:
            row = dict(parameter)
            parameter_value = row.get("parametervalue")
            if property_name == "textureparametervalues" and isinstance(
                parameter_value, int
            ):
                row["parameterPath"] = object_path(package, parameter_value)
                row["parameterObject"] = object_name(package, parameter_value)
            converted.append(row)
        result[output_name] = converted
    return result


def parse_landscape_package(package: PackageData) -> tuple[LandscapeProxy, list[LandscapeComponent], list[dict[str, Any]]]:
    proxy_exports = [
        export
        for export in package.exports
        if class_name(package, export).casefold() == "landscapeproxy"
    ]
    if len(proxy_exports) != 1:
        raise LandscapeError(
            f"{package.logical_name} has {len(proxy_exports)} LandscapeProxy exports"
        )
    proxy_export = proxy_exports[0]
    proxy_records, _start, _end = find_records(
        export_serial(package, proxy_export), package.names, package.summary.version
    )
    proxy_map = record_map(proxy_records)
    component_refs = int_array(proxy_map.get("landscapecomponents"))
    collision_refs = int_array(proxy_map.get("collisioncomponents"))
    material_ref = int(value(proxy_map, "landscapematerial", package.names, 0))
    proxy = LandscapeProxy(
        logical_package=package.logical_name,
        export_index=proxy_export.index,
        object_name=proxy_export.object_name,
        landscape_guid=str(value(proxy_map, "landscapeguid", package.names, "")),
        location=vector3(value(proxy_map, "location", package.names, None), (0, 0, 0)),
        draw_scale=float(value(proxy_map, "drawscale", package.names, 1.0)),
        draw_scale3d=vector3(
            value(proxy_map, "drawscale3d", package.names, None), (1, 1, 1)
        ),
        component_size_quads=int(
            value(proxy_map, "componentsizequads", package.names, 0)
        ),
        subsection_size_quads=int(
            value(proxy_map, "subsectionsizequads", package.names, 0)
        ),
        num_subsections=int(value(proxy_map, "numsubsections", package.names, 0)),
        landscape_material_ref=material_ref,
        landscape_material_path=object_path(package, material_ref) or "",
        landscape_material_name=object_name(package, material_ref),
        component_refs=component_refs,
        collision_component_refs=collision_refs,
    )

    components: list[LandscapeComponent] = []
    local_material_refs: set[int] = set()
    for component_ref in component_refs:
        if component_ref <= 0 or component_ref > len(package.exports):
            raise LandscapeError(f"invalid LandscapeComponent ref {component_ref}")
        export = package.exports[component_ref - 1]
        if class_name(package, export).casefold() != "landscapecomponent":
            raise LandscapeError(f"ref {component_ref} is not LandscapeComponent")
        records, _component_start, _component_end = find_records(
            export_serial(package, export), package.names, package.summary.version
        )
        properties = record_map(records)
        height_ref = int(value(properties, "heightmaptexture", package.names, 0))
        weight_refs = int_array(properties.get("weightmaptextures"))
        material_instance_ref = int(
            value(properties, "materialinstance", package.names, 0)
        )
        allocation_record = properties.get("weightmaplayerallocations")
        if allocation_record is None:
            raise LandscapeError(f"component {export.index} has no layer allocations")
        cached_box = value(properties, "cachedlocalbox", package.names, None)
        if not isinstance(cached_box, dict):
            raise LandscapeError(f"component {export.index} has no CachedLocalBox")
        component = LandscapeComponent(
            logical_package=package.logical_name,
            export_index=export.index,
            object_name=export.object_name,
            section_base_x=int(value(properties, "sectionbasex", package.names, 0)),
            section_base_y=int(value(properties, "sectionbasey", package.names, 0)),
            component_size_quads=int(
                value(properties, "componentsizequads", package.names, 0)
            ),
            subsection_size_quads=int(
                value(properties, "subsectionsizequads", package.names, 0)
            ),
            num_subsections=int(
                value(properties, "numsubsections", package.names, 0)
            ),
            heightmap_ref=height_ref,
            heightmap_name=object_name(package, height_ref),
            heightmap_path=object_path(package, height_ref) or "",
            heightmap_scale_bias=vector4(
                value(properties, "heightmapscalebias", package.names, None),
                (0, 0, 0, 0),
            ),
            weightmap_refs=weight_refs,
            weightmap_names=[object_name(package, ref) for ref in weight_refs],
            weightmap_paths=[object_path(package, ref) or "" for ref in weight_refs],
            weightmap_scale_bias=vector4(
                value(properties, "weightmapscalebias", package.names, None),
                (0, 0, 0, 0),
            ),
            weightmap_subsection_offset=float(
                value(properties, "weightmapsubsectionoffset", package.names, 0.0)
            ),
            allocations=decode_layer_allocations(allocation_record, package.names),
            material_instance_ref=material_instance_ref,
            material_instance_name=object_name(package, material_instance_ref),
            cached_local_box=cached_box,
        )
        if (
            component.component_size_quads != proxy.component_size_quads
            or component.subsection_size_quads != proxy.subsection_size_quads
            or component.num_subsections != proxy.num_subsections
        ):
            raise LandscapeError(f"component {export.index} size differs from proxy")
        local_material_refs.add(material_instance_ref)
        components.append(component)

    # Preserve every local Landscape material instance, including variants
    # which are present in the package but not selected by the 42 current
    # components.  Component references alone yield only the reachable subset
    # (66 here), while LAND01/LAND02 contain 78 source instances in total.
    local_material_refs.update(
        export.index + 1
        for export in package.exports
        if class_name(package, export).casefold()
        == "landscapematerialinstanceconstant"
    )
    local_materials: list[dict[str, Any]] = []
    pending_material_refs = list(sorted(local_material_refs))
    parsed_material_refs: set[int] = set()
    while pending_material_refs:
        material_ref = pending_material_refs.pop(0)
        if material_ref <= 0 or material_ref in parsed_material_refs:
            continue
        parsed_material_refs.add(material_ref)
        material = parse_material_instance(package, material_ref)
        local_materials.append(material)
        parent_ref = int(material.get("parentRef", 0))
        if parent_ref > 0:
            parent_export = package.exports[parent_ref - 1]
            if class_name(package, parent_export).casefold() == "landscapematerialinstanceconstant":
                pending_material_refs.append(parent_ref)
    local_materials.sort(key=lambda item: int(item["exportIndex"]))
    return proxy, components, local_materials


def decode_inline_bulk_array(
    serial: bytes,
    offset: int,
    absolute_serial_offset: int,
    element_format: str,
    label: str,
) -> tuple[tuple[int, ...], int]:
    """Decode the uncompressed inline FUntypedBulkData used by Landscape collision."""
    if offset + 16 > len(serial):
        raise LandscapeError(f"{label} has no complete BulkData header")
    bulk_flags, element_count, size_on_disk, logical_offset = struct.unpack_from(
        "<4i", serial, offset
    )
    if bulk_flags != 0:
        raise LandscapeError(f"{label} uses unsupported BulkData flags {bulk_flags:#x}")
    if element_count < 0:
        raise LandscapeError(f"{label} has negative element count {element_count}")
    element_size = struct.calcsize(element_format)
    expected_size = element_count * element_size
    if size_on_disk != expected_size:
        raise LandscapeError(
            f"{label} size {size_on_disk} != {element_count}*{element_size}"
        )
    payload_offset = offset + 16
    payload_end = payload_offset + size_on_disk
    if payload_end > len(serial):
        raise LandscapeError(f"{label} payload exceeds export serial")
    expected_logical_offset = absolute_serial_offset + payload_offset
    if logical_offset != expected_logical_offset:
        raise LandscapeError(
            f"{label} logical offset {logical_offset:#x} != "
            f"{expected_logical_offset:#x}"
        )
    values = struct.unpack_from(f"<{element_count}{element_format}", serial, payload_offset)
    return tuple(int(item) for item in values), payload_end


def parse_landscape_collision_components(
    package: PackageData,
    proxy: LandscapeProxy,
) -> list[LandscapeCollisionComponent]:
    collisions: list[LandscapeCollisionComponent] = []
    for collision_ref in proxy.collision_component_refs:
        if collision_ref <= 0 or collision_ref > len(package.exports):
            raise LandscapeError(f"invalid Landscape collision ref {collision_ref}")
        export = package.exports[collision_ref - 1]
        if class_name(package, export).casefold() != "landscapeheightfieldcollisioncomponent":
            raise LandscapeError(
                f"ref {collision_ref} is not LandscapeHeightfieldCollisionComponent"
            )
        serial = export_serial(package, export)
        records, _property_start, property_end = find_records(
            serial, package.names, package.summary.version
        )
        properties = record_map(records)
        heights, offset = decode_inline_bulk_array(
            serial,
            property_end,
            export.serial_offset,
            "H",
            f"collision export {export.index} heights",
        )
        dominant_layers, offset = decode_inline_bulk_array(
            serial,
            offset,
            export.serial_offset,
            "B",
            f"collision export {export.index} dominant layers",
        )
        if offset != len(serial):
            raise LandscapeError(
                f"collision export {export.index} has {len(serial) - offset} trailing bytes"
            )
        layer_names = tuple(fname_array(properties.get("componentlayers"), package.names))
        collisions.append(
            LandscapeCollisionComponent(
                logical_package=package.logical_name,
                export_index=export.index,
                object_name=export.object_name,
                section_base_x=int(value(properties, "sectionbasex", package.names, 0)),
                section_base_y=int(value(properties, "sectionbasey", package.names, 0)),
                collision_size_quads=int(
                    value(properties, "collisionsizequads", package.names, 0)
                ),
                collision_scale=float(
                    value(properties, "collisionscale", package.names, 1.0)
                ),
                include_holes=bool(
                    value(properties, "bincludeholes", package.names, False)
                ),
                component_layer_names=layer_names,
                heights=heights,
                dominant_layers=dominant_layers,
            )
        )
    by_section = {
        (collision.section_base_x, collision.section_base_y): collision
        for collision in collisions
    }
    if len(by_section) != len(collisions):
        raise LandscapeError("duplicate Landscape collision SectionBase coordinate")
    return collisions


def validate_collision_height_contract(
    component: LandscapeComponent,
    collision: LandscapeCollisionComponent,
) -> dict[str, Any]:
    if (
        component.logical_package != collision.logical_package
        or component.section_base_x != collision.section_base_x
        or component.section_base_y != collision.section_base_y
    ):
        raise LandscapeError("render/collision Landscape component identity mismatch")
    if collision.collision_size_quads != component.component_size_quads:
        raise LandscapeError(
            f"collision {collision.export_index} quad size "
            f"{collision.collision_size_quads} != {component.component_size_quads}"
        )
    if abs(collision.collision_scale - 1.0) > 1.0e-8:
        raise LandscapeError(
            f"collision {collision.export_index} scale is {collision.collision_scale}"
        )
    expected_count = (component.component_size_quads + 1) ** 2
    if len(collision.heights) != expected_count:
        raise LandscapeError(
            f"collision {collision.export_index} height count "
            f"{len(collision.heights)} != {expected_count}"
        )
    if len(collision.dominant_layers) != expected_count:
        raise LandscapeError(
            f"collision {collision.export_index} dominant layer count "
            f"{len(collision.dominant_layers)} != {expected_count}"
        )
    mismatches = 0
    maximum_difference = 0
    grid_size = component.component_size_quads + 1
    for y in range(grid_size):
        for x in range(grid_size):
            render_height = height16_at(component, x, y)
            collision_height = collision.heights[y * grid_size + x]
            difference = abs(render_height - collision_height)
            mismatches += int(difference != 0)
            maximum_difference = max(maximum_difference, difference)
    if mismatches:
        raise LandscapeError(
            f"component {component.export_index} has {mismatches} render/collision "
            f"height mismatches (max {maximum_difference})"
        )
    return {
        "collisionExportIndex": collision.export_index,
        "sampleCount": expected_count,
        "mismatchCount": mismatches,
        "maximumDifference": maximum_difference,
        "heightRange": [min(collision.heights), max(collision.heights)],
        "includeHoles": collision.include_holes,
        "componentLayerNames": list(collision.component_layer_names),
    }


def logical_texel(quad: int, subsection_size: int) -> int:
    if quad < 0 or quad > subsection_size * 2:
        raise LandscapeError(f"logical Landscape coordinate is outside component: {quad}")
    return quad if quad <= subsection_size else quad + 1


def pixel_rgba(mip: TextureMip, x: int, y: int) -> tuple[int, int, int, int]:
    if not 0 <= x < mip.width or not 0 <= y < mip.height:
        raise LandscapeError(f"Texture2D pixel is outside mip: {x},{y}")
    offset = (y * mip.width + x) * 4
    blue, green, red, alpha = mip.bgra[offset : offset + 4]
    return red, green, blue, alpha


def height16_at(component: LandscapeComponent, x: int, y: int) -> int:
    if component.height_texture is None:
        raise LandscapeError("height texture has not been decoded")
    mip = component.height_texture.mips[0]
    texture_x = logical_texel(x, component.subsection_size_quads)
    texture_y = logical_texel(y, component.subsection_size_quads)
    red, green, _blue, _alpha = pixel_rgba(mip, texture_x, texture_y)
    return red * 256 + green


def height_packed_normal_at(
    component: LandscapeComponent, x: int, y: int
) -> tuple[int, int]:
    if component.height_texture is None:
        raise LandscapeError("height texture has not been decoded")
    mip = component.height_texture.mips[0]
    texture_x = logical_texel(x, component.subsection_size_quads)
    texture_y = logical_texel(y, component.subsection_size_quads)
    _red, _green, blue, alpha = pixel_rgba(mip, texture_x, texture_y)
    return blue, alpha


def source_normal_client_at(
    component: LandscapeComponent, x: int, y: int
) -> tuple[float, float, float]:
    blue, alpha = height_packed_normal_at(component, x, y)
    normal_x = blue / 127.5 - 1.0
    normal_y = alpha / 127.5 - 1.0
    normal_z = math.sqrt(max(0.0, 1.0 - normal_x * normal_x - normal_y * normal_y))
    # UE (X,Y,Z) -> Client (X,Z,-Y).
    return normalize3((normal_x, normal_z, -normal_y))


def local_height_at(component: LandscapeComponent, x: int, y: int) -> float:
    return (height16_at(component, x, y) - 32768) * LANDSCAPE_ZSCALE


def layer_weight_at(
    component: LandscapeComponent,
    allocation: LayerAllocation,
    x: int,
    y: int,
) -> int:
    if component.weight_textures is None:
        raise LandscapeError("weight textures have not been decoded")
    if allocation.texture_index >= len(component.weight_textures):
        raise LandscapeError(
            f"weight texture index {allocation.texture_index} is outside component"
        )
    mip = component.weight_textures[allocation.texture_index].mips[0]
    texture_x = logical_texel(x, component.subsection_size_quads)
    texture_y = logical_texel(y, component.subsection_size_quads)
    rgba = pixel_rgba(mip, texture_x, texture_y)
    return rgba[allocation.channel]


def hole_layer_allocation(
    component: LandscapeComponent,
) -> LayerAllocation | None:
    return next(
        (
            allocation
            for allocation in component.allocations
            if allocation.layer_name.casefold() == "__datalayer__"
        ),
        None,
    )


def quad_is_hole(component: LandscapeComponent, x: int, y: int) -> bool:
    allocation = hole_layer_allocation(component)
    return allocation is not None and is_ue3_landscape_hole(
        layer_weight_at(component, allocation, x, y)
    )


def validate_component_texture_contract(
    component: LandscapeComponent,
    hole_mask_tolerance: int = 0,
) -> dict[str, Any]:
    if component.height_texture is None or component.weight_textures is None:
        raise LandscapeError("component textures have not been decoded")
    expected_texture_size = (
        component.subsection_size_quads + 1
    ) * component.num_subsections
    referenced = [component.height_texture, *component.weight_textures]
    for texture in referenced:
        mip = texture.mips[0]
        if mip.width != expected_texture_size or mip.height != expected_texture_size:
            raise LandscapeError(
                f"{component.logical_package}:{component.export_index} texture "
                f"{texture.object_name} is {mip.width}x{mip.height}, expected "
                f"{expected_texture_size}x{expected_texture_size}"
            )
    expected_height_bias = (
        1.0 / expected_texture_size,
        1.0 / expected_texture_size,
        0.0,
        0.0,
    )
    expected_weight_bias = (
        1.0 / expected_texture_size,
        1.0 / expected_texture_size,
        0.5 / expected_texture_size,
        0.5 / expected_texture_size,
    )
    if any(
        abs(actual - expected) > 1.0e-8
        for actual, expected in zip(
            component.heightmap_scale_bias, expected_height_bias
        )
    ):
        raise LandscapeError(
            f"component {component.export_index} unexpected HeightmapScaleBias "
            f"{component.heightmap_scale_bias}"
        )
    if component.weight_textures and (
        any(
            abs(actual - expected) > 1.0e-8
            for actual, expected in zip(
                component.weightmap_scale_bias, expected_weight_bias
            )
        )
        or abs(component.weightmap_subsection_offset - 0.5) > 1.0e-8
    ):
        raise LandscapeError(
            f"component {component.export_index} unexpected Weightmap mapping "
            f"{component.weightmap_scale_bias} / "
            f"{component.weightmap_subsection_offset}"
        )

    # Heightmaps duplicate the shared subsection border.  Weightmaps use the
    # same 32x32-per-subsection layout, but channels which are not referenced
    # by WeightmapLayerAllocations are undefined storage.  Some Lost Ark
    # packages contain unrelated bytes in those unused channels, so comparing
    # all four RGBA bytes would reject otherwise valid Landscape data.
    height_seam_mismatch = 0
    weight_seam_mismatch = 0
    seam_sample_count = 0
    seam_left = component.subsection_size_quads
    seam_right = seam_left + 1
    height_mip = component.height_texture.mips[0]
    for coordinate in range(expected_texture_size):
        seam_sample_count += 2
        left = pixel_rgba(height_mip, seam_left, coordinate)
        right = pixel_rgba(height_mip, seam_right, coordinate)
        bottom = pixel_rgba(height_mip, coordinate, seam_left)
        top = pixel_rgba(height_mip, coordinate, seam_right)
        height_seam_mismatch += int(left != right)
        height_seam_mismatch += int(bottom != top)

    channels_by_texture: dict[int, set[int]] = {}
    for allocation in component.allocations:
        channels_by_texture.setdefault(allocation.texture_index, set()).add(
            allocation.channel
        )
    for texture_index, channels in sorted(channels_by_texture.items()):
        if texture_index >= len(component.weight_textures):
            raise LandscapeError(
                f"component {component.export_index} allocation references missing "
                f"weight texture {texture_index}"
            )
        mip = component.weight_textures[texture_index].mips[0]
        for coordinate in range(expected_texture_size):
            seam_sample_count += 2 * len(channels)
            left = pixel_rgba(mip, seam_left, coordinate)
            right = pixel_rgba(mip, seam_right, coordinate)
            bottom = pixel_rgba(mip, coordinate, seam_left)
            top = pixel_rgba(mip, coordinate, seam_right)
            for channel in channels:
                weight_seam_mismatch += int(left[channel] != right[channel])
                weight_seam_mismatch += int(bottom[channel] != top[channel])
    seam_mismatch = height_seam_mismatch + weight_seam_mismatch
    if seam_mismatch:
        raise LandscapeError(
            f"component {component.export_index} has {height_seam_mismatch} height and "
            f"{weight_seam_mismatch} allocated-weight subsection seam mismatches"
        )

    local_heights = [
        local_height_at(component, x, y)
        for y in range(component.component_size_quads + 1)
        for x in range(component.component_size_quads + 1)
    ]
    cached_min = float(component.cached_local_box["min"][2])
    cached_max = float(component.cached_local_box["max"][2])
    height_min = min(local_heights)
    height_max = max(local_heights)
    if abs(height_min - cached_min) > 1.0e-6 or abs(height_max - cached_max) > 1.0e-6:
        raise LandscapeError(
            f"component {component.export_index} height range "
            f"{height_min}..{height_max} != CachedLocalBox {cached_min}..{cached_max}"
        )

    hole_allocation = hole_layer_allocation(component)
    hole_values: set[int] = set()
    hole_count = 0
    hole_intermediate_count = 0
    if hole_allocation is not None:
        for y in range(component.component_size_quads + 1):
            for x in range(component.component_size_quads + 1):
                weight = layer_weight_at(component, hole_allocation, x, y)
                hole_values.add(weight)
                hole_count += int(is_ue3_landscape_hole(weight))
                hole_intermediate_count += int(weight not in (0, 255))
        if hole_intermediate_count > hole_mask_tolerance:
            raise LandscapeError(
                f"component {component.export_index} __DataLayer__ has "
                f"{hole_intermediate_count} non-binary samples (tolerance "
                f"{hole_mask_tolerance}): {sorted(hole_values)}"
            )

    return {
        "heightMin": height_min,
        "heightMax": height_max,
        "subsectionSeamSampleCount": seam_sample_count,
        "subsectionSeamMismatchCount": seam_mismatch,
        "heightSubsectionSeamMismatchCount": height_seam_mismatch,
        "heightNormalSource": "Heightmap B/A packed UE normal",
        "heightNormalDecode": (
            "UE=(B/127.5-1,A/127.5-1,sqrt(1-x*x-y*y)); Client=(x,z,-y)"
        ),
        "allocatedWeightSubsectionSeamMismatchCount": weight_seam_mismatch,
        "holeSampleCount": hole_count,
        "holeValues": sorted(hole_values),
        "holeIntermediateSampleCount": hole_intermediate_count,
    }


def validate_component_edges(
    components: Sequence[LandscapeComponent],
) -> dict[str, Any]:
    by_section = {
        (component.section_base_x, component.section_base_y): component
        for component in components
    }
    if len(by_section) != len(components):
        raise LandscapeError("duplicate Landscape SectionBase coordinate")
    edge_count = 0
    sample_count = 0
    mismatch_count = 0
    packed_normal_mismatch_count = 0
    max_difference = 0
    cross_package_edge_count = 0
    for (base_x, base_y), component in by_section.items():
        right = by_section.get((base_x + component.component_size_quads, base_y))
        top = by_section.get((base_x, base_y + component.component_size_quads))
        if right is not None:
            edge_count += 1
            cross_package_edge_count += int(
                right.logical_package != component.logical_package
            )
            for coordinate in range(component.component_size_quads + 1):
                left_height = height16_at(
                    component, component.component_size_quads, coordinate
                )
                right_height = height16_at(right, 0, coordinate)
                difference = abs(left_height - right_height)
                sample_count += 1
                mismatch_count += int(difference != 0)
                packed_normal_mismatch_count += int(
                    height_packed_normal_at(
                        component, component.component_size_quads, coordinate
                    )
                    != height_packed_normal_at(right, 0, coordinate)
                )
                max_difference = max(max_difference, difference)
        if top is not None:
            edge_count += 1
            cross_package_edge_count += int(top.logical_package != component.logical_package)
            for coordinate in range(component.component_size_quads + 1):
                bottom_height = height16_at(
                    component, coordinate, component.component_size_quads
                )
                top_height = height16_at(top, coordinate, 0)
                difference = abs(bottom_height - top_height)
                sample_count += 1
                mismatch_count += int(difference != 0)
                packed_normal_mismatch_count += int(
                    height_packed_normal_at(
                        component, coordinate, component.component_size_quads
                    )
                    != height_packed_normal_at(top, coordinate, 0)
                )
                max_difference = max(max_difference, difference)
    if mismatch_count or packed_normal_mismatch_count:
        raise LandscapeError(
            "Landscape component borders have "
            f"{mismatch_count} height and {packed_normal_mismatch_count} "
            "packed-normal mismatches"
        )
    return {
        "adjacentEdgeCount": edge_count,
        "crossPackageEdgeCount": cross_package_edge_count,
        "sampleCount": sample_count,
        "mismatchCount": mismatch_count,
        "maxHeight16Difference": max_difference,
        "packedNormalMismatchCount": packed_normal_mismatch_count,
    }


def png_chunk(name: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + name
        + payload
        + struct.pack(">I", binascii.crc32(name + payload) & 0xFFFFFFFF)
    )


def encode_png_rgba(image: ImageRgba) -> bytes:
    if len(image.pixels) != image.width * image.height:
        raise LandscapeError("RGBA image pixel count mismatch")
    raw = bytearray()
    for y in range(image.height):
        raw.append(0)
        for red, green, blue, alpha in image.pixels[
            y * image.width : (y + 1) * image.width
        ]:
            raw.extend((red, green, blue, alpha))
    header = struct.pack(">IIBBBBB", image.width, image.height, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", header)
        + png_chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + png_chunk(b"IEND", b"")
    )


def mip_image(mip: TextureMip) -> ImageRgba:
    pixels = tuple(
        pixel_rgba(mip, x, y)
        for y in range(mip.height)
        for x in range(mip.width)
    )
    return ImageRgba(mip.width, mip.height, pixels)


def write_texture_source(
    texture: DecodedTexture,
    destination: Path,
) -> dict[str, Any]:
    destination.mkdir(parents=True, exist_ok=True)
    mip_rows: list[dict[str, Any]] = []
    for mip in texture.mips:
        raw_path = destination / f"mip{mip.level}.bgra8"
        atomic_write_bytes(raw_path, mip.bgra)
        mip_rows.append(
            {
                "level": mip.level,
                "width": mip.width,
                "height": mip.height,
                "bulkFlags": f"0x{mip.bulk_flags:08X}",
                "elementCount": mip.element_count,
                "sizeOnDisk": mip.size_on_disk,
                "logicalOffset": f"0x{mip.logical_offset:X}",
                "rawFile": raw_path.name,
                "sha256": sha256_file(raw_path),
            }
        )
    preview_path = destination / "mip0.png"
    atomic_write_bytes(preview_path, encode_png_rgba(mip_image(texture.mips[0])))
    metadata = {
        "schemaVersion": 1,
        "objectName": texture.object_name,
        "exportIndex": texture.export_index,
        "pixelFormat": texture.pixel_format,
        "rawChannelOrder": "BGRA",
        "sourceArt": {
            "flags": f"0x{texture.source_art_flags:08X}",
            "elementCount": texture.source_art_element_count,
            "sizeOnDisk": texture.source_art_size_on_disk,
            "logicalOffset": f"0x{texture.source_art_offset:X}",
        },
        "mips": mip_rows,
        "preview": preview_path.name,
    }
    atomic_write_text(destination / "texture.json", json_text(metadata))
    return metadata


def write_export_serial_sources(
    package: PackageData,
    groups: dict[str, Sequence[int]],
    destination: Path,
    output_root: Path,
) -> dict[str, list[dict[str, Any]]]:
    """Preserve complete decompressed export serials, including opaque tails."""
    destination.mkdir(parents=True, exist_ok=True)
    result: dict[str, list[dict[str, Any]]] = {}
    seen: set[tuple[str, int]] = set()
    for group_name, references in groups.items():
        rows: list[dict[str, Any]] = []
        for reference in sorted(set(int(item) for item in references)):
            if reference <= 0 or reference > len(package.exports):
                raise LandscapeError(
                    f"{group_name} contains invalid local export ref {reference}"
                )
            if (group_name, reference) in seen:
                raise LandscapeError(f"duplicate serial source {group_name}:{reference}")
            seen.add((group_name, reference))
            export = package.exports[reference - 1]
            serial = export_serial(package, export)
            file_path = destination / (
                f"{group_name}_{export.index:05d}_{export.object_name}.serial.bin"
            )
            atomic_write_bytes(file_path, serial)
            rows.append(
                {
                    "ref": reference,
                    "exportIndex": export.index,
                    "objectName": export.object_name,
                    "class": class_name(package, export),
                    "serialSize": len(serial),
                    "serialSha256": sha256_file(file_path),
                    "source": relative_posix(file_path, output_root),
                }
            )
        result[group_name] = rows
    atomic_write_text(
        destination / "serial_index.json",
        json_text(
            {
                "schemaVersion": 1,
                "logicalPackage": package.logical_name,
                "groups": result,
            }
        ),
    )
    return result


def run_process(
    command: Sequence[str],
    cwd: Path,
    operation: str,
) -> str:
    completed = subprocess.run(
        [str(item) for item in command],
        cwd=cwd,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        check=False,
        creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0,
    )
    output = completed.stdout + ("\n" if completed.stdout else "") + completed.stderr
    if completed.returncode != 0:
        raise LandscapeError(
            f"{operation} failed with exit {completed.returncode}\n{output[-4000:]}"
        )
    return output


def portable_process_log(text: str, output_root: Path) -> str:
    resolved = output_root.resolve()
    stage_name = re.escape(resolved.name)
    result = re.sub(
        rf"(?i)[A-Za-z]:[\\/][^\r\n]*?[\\/]_work[\\/]{stage_name}",
        "<OUTPUT_ROOT>",
        text,
    )
    markers = {
        str(resolved),
        str(resolved).replace("\\", "/"),
    }
    try:
        relative = resolved.relative_to(REPOSITORY_ROOT)
        markers.add(str(relative))
        markers.add(str(relative).replace("\\", "/"))
    except ValueError:
        pass
    for marker in sorted(markers, key=len, reverse=True):
        if marker:
            result = result.replace(marker, "<OUTPUT_ROOT>")
    return result


def export_master_material(
    umodel: Path,
    package_root: Path,
    region: str,
    logical_package: str,
    material_name: str,
    output: Path,
    pack_root: Path,
) -> dict[str, Any]:
    output.mkdir(parents=True, exist_ok=True)
    common = [
        str(umodel),
        "-export",
        "-game=lostark",
        f"-{region}",
        "-nameresolve",
        f"-path={package_root}",
        "-uncook",
        f"-obj={material_name}",
        logical_package,
    ]
    dds_output = output / "dds"
    tga_output = output / "tga"
    dds_output.mkdir(parents=True, exist_ok=True)
    tga_output.mkdir(parents=True, exist_ok=True)
    dds_log = run_process(
        [*common[:-1], f"-out={dds_output}", "-dds", common[-1]],
        umodel.parent,
        "UModel master material DDS export",
    )
    tga_log = run_process(
        [*common[:-1], f"-out={tga_output}", common[-1]],
        umodel.parent,
        "UModel master material TGA export",
    )
    atomic_write_text(
        output / "umodel_dds.log", portable_process_log(dds_log, pack_root)
    )
    atomic_write_text(
        output / "umodel_tga.log", portable_process_log(tga_log, pack_root)
    )
    material_files = list(dds_output.rglob(f"{material_name}.mat"))
    property_files = list(dds_output.rglob(f"{material_name}.props.txt"))
    if len(material_files) != 1 or len(property_files) != 1:
        raise LandscapeError("UModel did not export the master Landscape material")
    dds_files = sorted(dds_output.rglob("*.dds"))
    tga_files = sorted(tga_output.rglob("*.tga"))
    if not dds_files:
        raise LandscapeError("UModel did not export deterministic DDS layer textures")
    return {
        "ddsRoot": str(dds_output),
        "tgaRoot": str(tga_output),
        "materialFile": str(material_files[0].relative_to(output)).replace(
            "\\", "/"
        ),
        "propertiesFile": str(property_files[0].relative_to(output)).replace(
            "\\", "/"
        ),
        "ddsFileCount": len(dds_files),
        "tgaFileCount": len(tga_files),
        "ddsFiles": [
            {
                "path": str(path.relative_to(dds_output)).replace("\\", "/"),
                "size": path.stat().st_size,
                "sha256": sha256_file(path),
            }
            for path in dds_files
        ],
    }


def texture_serial_mip_headers(
    package: PackageData, export: Any
) -> tuple[list[dict[str, Any]], int]:
    serial = export_serial(package, export)
    records, _start, property_end = find_records(
        serial, package.names, package.summary.version
    )
    reader = UE3.Reader(serial, property_end)
    _source_art_flags = reader.i32()
    source_art_element_count = reader.i32()
    source_art_size_on_disk = reader.i32()
    _source_art_offset = reader.i32()
    if source_art_size_on_disk:
        reader.read(source_art_size_on_disk)
    mip_count = reader.i32()
    if mip_count <= 0 or mip_count > 32:
        raise LandscapeError(
            f"invalid dependency Texture2D mip count {mip_count}: "
            f"{export.object_name}"
        )
    rows: list[dict[str, Any]] = []
    for level in range(mip_count):
        bulk_flags = reader.i32()
        element_count = reader.i32()
        size_on_disk = reader.i32()
        logical_offset = reader.i32()
        reader.read(size_on_disk)
        width = reader.i32()
        height = reader.i32()
        rows.append(
            {
                "level": level,
                "width": width,
                "height": height,
                "bulkFlags": f"0x{bulk_flags:08X}",
                "elementCount": element_count,
                "sizeOnDisk": size_on_disk,
                "logicalOffset": f"0x{logical_offset:X}",
            }
        )
    return rows, source_art_element_count


def preserve_master_texture_dependencies(
    definition: dict[str, Any],
    umodel: Path,
    package_root: Path,
    region: str,
    aes_key: str,
    destination: Path,
    output_root: Path,
    package_cache: dict[str, PackageData],
    additional_dependency_paths: Sequence[str] = (),
) -> dict[str, Any]:
    dependency_by_key: dict[str, str] = {}
    for parameter in definition.get("textureParameters", []):
        if parameter.get("parameterPath"):
            path = str(parameter["parameterPath"])
            dependency_by_key.setdefault(path.casefold(), path)
    for item in additional_dependency_paths:
        path = str(item)
        dependency_by_key.setdefault(path.casefold(), path)
    dependency_paths = [
        dependency_by_key[key] for key in sorted(dependency_by_key)
    ]
    rows: list[dict[str, Any]] = []
    package_rows: dict[str, dict[str, Any]] = {}
    for dependency_path in dependency_paths:
        pieces = dependency_path.split(".")
        if len(pieces) < 2:
            raise LandscapeError(
                f"invalid master texture dependency {dependency_path!r}"
            )
        logical_package = (
            str(definition["logicalPackage"])
            if pieces[0].casefold() == "tex"
            else pieces[0].upper()
        )
        object_name_value = pieces[-1]
        package = package_cache.get(logical_package)
        if package is None:
            physical = UE3.resolve_physical_package(
                umodel, package_root, logical_package, region
            )
            package = load_package(physical, logical_package, aes_key)
            package_cache[logical_package] = package
        package_rows.setdefault(
            logical_package,
            {
                "logicalPackage": logical_package,
                "physicalPackageFile": package.physical_path.name,
                "physicalPackageSha256": sha256_file(package.physical_path),
            },
        )
        matches = [
            export
            for export in package.exports
            if export.object_name.casefold() == object_name_value.casefold()
            and class_name(package, export).casefold() == "texture2d"
        ]
        if len(matches) != 1:
            raise LandscapeError(
                f"{dependency_path} resolves to {len(matches)} Texture2D exports"
            )
        export = matches[0]
        serial = export_serial(package, export)
        serial_path = (
            destination
            / logical_package
            / f"{export.index:05d}_{export.object_name}.texture.serial.bin"
        )
        atomic_write_bytes(serial_path, serial)
        records, property_start, property_end = find_records(
            serial, package.names, package.summary.version
        )
        properties = record_map(records)
        mip_headers, source_art_element_count = texture_serial_mip_headers(
            package, export
        )
        rows.append(
            {
                "parameterPath": dependency_path,
                "logicalPackage": logical_package,
                "objectName": export.object_name,
                "ref": export.index + 1,
                "exportIndex": export.index,
                "pixelFormat": str(
                    value(properties, "format", package.names, "")
                ),
                "sizeX": int(value(properties, "sizex", package.names, 0)),
                "sizeY": int(value(properties, "sizey", package.names, 0)),
                "propertyStart": property_start,
                "propertyEnd": property_end,
                "serialSize": len(serial),
                "serialTailSize": len(serial) - property_end,
                "sourceArtElementCount": source_art_element_count,
                "mips": mip_headers,
                "serialSource": relative_posix(serial_path, output_root),
                "serialSha256": sha256_file(serial_path),
                "taggedProperties": [
                    {
                        "name": record.name,
                        "type": record.property_type,
                        "structType": record.struct_type,
                        "arrayIndex": record.array_index,
                        "payloadSize": len(record.payload),
                        "payloadSha256": sha256_bytes(record.payload),
                        "decoded": decoded_record_json(record, package.names),
                    }
                    for record in records
                ],
            }
        )
    index_path = destination / "dependency_serial_index.json"
    document = {
        "schemaVersion": 1,
        "source": "master material TextureParameterValues",
        "dependencyCount": len(rows),
        "packageCount": len(package_rows),
        "packages": [package_rows[key] for key in sorted(package_rows)],
        "dependencies": rows,
    }
    atomic_write_text(index_path, json_text(document))
    return {
        "source": relative_posix(index_path, output_root),
        "dependencyCount": len(rows),
        "packageCount": len(package_rows),
    }


def decode_tga(path: Path) -> ImageRgba:
    data = path.read_bytes()
    if len(data) < 18:
        raise LandscapeError(f"short TGA file: {path}")
    (
        id_length,
        color_map_type,
        image_type,
        _color_map_first,
        _color_map_length,
        _color_map_depth,
        _origin_x,
        _origin_y,
        width,
        height,
        pixel_depth,
        descriptor,
    ) = struct.unpack_from("<BBBHHBHHHHBB", data, 0)
    if color_map_type != 0 or image_type not in (2, 10) or pixel_depth not in (24, 32):
        raise LandscapeError(f"unsupported TGA format in {path}")
    bytes_per_pixel = pixel_depth // 8
    offset = 18 + id_length
    file_pixels: list[tuple[int, int, int, int]] = []

    def read_pixel() -> tuple[int, int, int, int]:
        nonlocal offset
        if offset + bytes_per_pixel > len(data):
            raise LandscapeError(f"truncated TGA pixel data: {path}")
        blue, green, red = data[offset : offset + 3]
        alpha = data[offset + 3] if bytes_per_pixel == 4 else 255
        offset += bytes_per_pixel
        return red, green, blue, alpha

    pixel_count = width * height
    if image_type == 2:
        file_pixels = [read_pixel() for _ in range(pixel_count)]
    else:
        while len(file_pixels) < pixel_count:
            if offset >= len(data):
                raise LandscapeError(f"truncated TGA RLE stream: {path}")
            packet = data[offset]
            offset += 1
            count = (packet & 0x7F) + 1
            if packet & 0x80:
                pixel = read_pixel()
                file_pixels.extend([pixel] * count)
            else:
                file_pixels.extend(read_pixel() for _ in range(count))
        if len(file_pixels) != pixel_count:
            raise LandscapeError(f"TGA RLE pixel count mismatch: {path}")

    top_origin = bool(descriptor & 0x20)
    right_origin = bool(descriptor & 0x10)
    pixels: list[tuple[int, int, int, int] | None] = [None] * pixel_count
    for index, pixel in enumerate(file_pixels):
        source_x = index % width
        source_y = index // width
        x = width - 1 - source_x if right_origin else source_x
        y = source_y if top_origin else height - 1 - source_y
        pixels[y * width + x] = pixel
    return ImageRgba(width, height, tuple(pixel for pixel in pixels if pixel is not None))


def rgb565_triplet(value: int) -> tuple[int, int, int]:
    red = (value >> 11) & 0x1F
    green = (value >> 5) & 0x3F
    blue = value & 0x1F
    return (
        (red * 255 + 15) // 31,
        (green * 255 + 31) // 63,
        (blue * 255 + 15) // 31,
    )


def block_compression_colors(
    block: bytes,
    force_four_color: bool,
) -> list[tuple[int, int, int, int]]:
    """Decode the 8-byte BC1-style colour block shared by DXT1 and DXT5."""
    endpoint0, endpoint1 = struct.unpack_from("<HH", block, 0)
    indices = struct.unpack_from("<I", block, 4)[0]
    color0 = rgb565_triplet(endpoint0)
    color1 = rgb565_triplet(endpoint1)
    if force_four_color or endpoint0 > endpoint1:
        color2 = tuple((2 * color0[i] + color1[i]) // 3 for i in range(3))
        color3 = tuple((color0[i] + 2 * color1[i]) // 3 for i in range(3))
        alpha3 = 255
    else:
        color2 = tuple((color0[i] + color1[i]) // 2 for i in range(3))
        color3 = (0, 0, 0)
        alpha3 = 0
    palette = (
        (*color0, 255),
        (*color1, 255),
        (*color2, 255),
        (*color3, alpha3),
    )
    return [palette[(indices >> (2 * texel)) & 0x3] for texel in range(16)]


def block_compression_channel(block: bytes) -> list[int]:
    """Decode the 8-byte BC4-style single channel block (DXT5 alpha, ATI2 halves)."""
    first, second = block[0], block[1]
    if first > second:
        # Eight interpolated values: weights must total 7.
        table = [first, second] + [
            ((6 - step) * first + (1 + step) * second) // 7 for step in range(6)
        ]
    else:
        # Six interpolated values plus the two explicit endpoints; weights total 5.
        table = (
            [first, second]
            + [((4 - step) * first + (1 + step) * second) // 5 for step in range(4)]
            + [0, 255]
        )
    indices = int.from_bytes(block[2:8], "little")
    return [table[(indices >> (3 * texel)) & 0x7] for texel in range(16)]


def decode_dds(path: Path) -> ImageRgba:
    """Decode a DXT1 / DXT5 / ATI2 DDS file into straight RGBA.

    UModel writes uncompressed Lost Ark textures as TGA but keeps block
    compressed ones in their native DDS form, so Landscape layer textures are
    only available in this format for levels whose layers are DXT compressed.
    """
    data = path.read_bytes()
    if len(data) < 128 or data[:4] != b"DDS ":
        raise LandscapeError(f"not a DDS file: {path}")
    height, width = struct.unpack_from("<II", data, 12)
    four_cc = data[84:88]
    block_size = {b"DXT1": 8, b"DXT5": 16, b"ATI2": 16}.get(four_cc)
    if block_size is None:
        raise LandscapeError(f"unsupported DDS FourCC {four_cc!r} in {path}")
    if width <= 0 or height <= 0 or width % 4 or height % 4:
        raise LandscapeError(f"unsupported DDS dimensions {width}x{height} in {path}")
    blocks_x = width // 4
    blocks_y = height // 4
    required = blocks_x * blocks_y * block_size
    payload = data[128 : 128 + required]
    if len(payload) != required:
        raise LandscapeError(f"truncated DDS payload in {path}")

    pixels: list[tuple[int, int, int, int]] = [(0, 0, 0, 255)] * (width * height)
    for block_y in range(blocks_y):
        for block_x in range(blocks_x):
            start = (block_y * blocks_x + block_x) * block_size
            block = payload[start : start + block_size]
            if four_cc == b"DXT1":
                texels = block_compression_colors(block, False)
            elif four_cc == b"DXT5":
                alphas = block_compression_channel(block[0:8])
                colors = block_compression_colors(block[8:16], True)
                texels = [
                    (colors[texel][0], colors[texel][1], colors[texel][2], alphas[texel])
                    for texel in range(16)
                ]
            else:
                # ATI2 stores only X and Y; Z is reconstructed as a unit normal
                # because the layer blend reads the blue channel directly.
                reds = block_compression_channel(block[0:8])
                greens = block_compression_channel(block[8:16])
                texels = []
                for texel in range(16):
                    normal_x = reds[texel] / 127.5 - 1.0
                    normal_y = greens[texel] / 127.5 - 1.0
                    normal_z = math.sqrt(
                        max(0.0, 1.0 - normal_x * normal_x - normal_y * normal_y)
                    )
                    texels.append(
                        (
                            reds[texel],
                            greens[texel],
                            min(255, max(0, round((normal_z + 1.0) * 127.5))),
                            255,
                        )
                    )
            for texel, pixel in enumerate(texels):
                x = block_x * 4 + (texel % 4)
                y = block_y * 4 + (texel // 4)
                pixels[y * width + x] = pixel
    return ImageRgba(width, height, tuple(pixels))


def decode_layer_texture(path: Path) -> ImageRgba:
    suffix = path.suffix.casefold()
    if suffix == ".tga":
        return decode_tga(path)
    if suffix == ".dds":
        return decode_dds(path)
    raise LandscapeError(f"unsupported layer texture format: {path}")


def material_texture_file(
    dds_root: Path,
    material_package: str,
    texture_path: str,
) -> Path:
    pieces = texture_path.split(".")
    if len(pieces) < 2:
        raise LandscapeError(f"invalid material texture path {texture_path!r}")
    if pieces[0].casefold() == "tex":
        package_name = material_package
    else:
        package_name = pieces[0]
    object_name_value = pieces[-1]
    expected_root = dds_root / package_name.upper()
    candidates = list(expected_root.rglob(f"{object_name_value}.dds"))
    if len(candidates) != 1:
        raise LandscapeError(
            f"could not uniquely locate {texture_path}: {[str(path) for path in candidates]}"
        )
    return candidates[0]


def parse_master_material_definition(
    package: PackageData,
    material_name: str,
) -> dict[str, Any]:
    matches = [
        export
        for export in package.exports
        if export.object_name.casefold() == material_name.casefold()
        and class_name(package, export).casefold() == "materialinstanceconstant"
    ]
    if len(matches) != 1:
        raise LandscapeError(
            f"{package.logical_name} has {len(matches)} matching master materials"
        )
    definition = parse_material_instance(package, matches[0].index + 1)
    definition["logicalPackage"] = package.logical_name
    definition["physicalPackage"] = str(package.physical_path)
    definition["physicalPackageSha256"] = sha256_file(package.physical_path)

    layer_info: list[dict[str, Any]] = []
    for export in package.exports:
        if class_name(package, export).casefold() != "landscapelayerinfoobject":
            continue
        records, start, end = find_records(
            export_serial(package, export), package.names, package.summary.version
        )
        properties: dict[str, Any] = {}
        for record in records:
            decoded = decode_record(record, package.names)
            if record.property_type.casefold() in (
                "objectproperty",
                "componentproperty",
            ) and isinstance(decoded, int):
                properties[record.name] = {
                    "ref": decoded,
                    "path": object_path(package, decoded),
                    "name": object_name(package, decoded),
                }
            else:
                properties[record.name] = json_value(decoded)
        layer_info.append(
            {
                "exportIndex": export.index,
                "objectName": export.object_name,
                "propertyStart": start,
                "propertyEnd": end,
                "serialSize": export.serial_size,
                "serialTailSize": export.serial_size - end,
                "properties": properties,
                "taggedProperties": [
                    {
                        "name": record.name,
                        "type": record.property_type,
                        "structType": record.struct_type,
                        "arrayIndex": record.array_index,
                        "payloadSize": len(record.payload),
                        "payloadSha256": sha256_bytes(record.payload),
                        "decoded": decoded_record_json(record, package.names),
                    }
                    for record in records
                ],
            }
        )
    definition["layerInfoObjects"] = layer_info
    return definition


def parameter_map(
    definition: dict[str, Any],
    collection_name: str,
) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for parameter in definition.get(collection_name, []):
        name = str(parameter.get("parametername", "")).casefold()
        if name:
            result[name] = parameter
    return result


def clamp_byte(value: float) -> int:
    return max(0, min(255, int(round(value))))


def sample_image_repeat(image: ImageRgba, u: float, v: float) -> tuple[float, float, float, float]:
    u -= math.floor(u)
    v -= math.floor(v)
    x = int(u * image.width) % image.width
    y = int(v * image.height) % image.height
    red, green, blue, alpha = image.pixels[y * image.width + x]
    return red / 255.0, green / 255.0, blue / 255.0, alpha / 255.0


def texture_coordinate(value: float, subsection_size: int) -> float:
    return value if value <= subsection_size else value + 1.0


def sample_component_weight(
    component: LandscapeComponent,
    allocation: LayerAllocation,
    x: float,
    y: float,
) -> float:
    if component.weight_textures is None:
        raise LandscapeError("component weight textures are missing")
    texture = component.weight_textures[allocation.texture_index].mips[0]
    texture_x = texture_coordinate(x, component.subsection_size_quads)
    texture_y = texture_coordinate(y, component.subsection_size_quads)
    x0 = max(0, min(texture.width - 1, int(math.floor(texture_x))))
    y0 = max(0, min(texture.height - 1, int(math.floor(texture_y))))
    x1 = min(texture.width - 1, x0 + 1)
    y1 = min(texture.height - 1, y0 + 1)
    fx = texture_x - x0
    fy = texture_y - y0
    values = (
        pixel_rgba(texture, x0, y0)[allocation.channel],
        pixel_rgba(texture, x1, y0)[allocation.channel],
        pixel_rgba(texture, x0, y1)[allocation.channel],
        pixel_rgba(texture, x1, y1)[allocation.channel],
    )
    top = values[0] * (1.0 - fx) + values[1] * fx
    bottom = values[2] * (1.0 - fx) + values[3] * fx
    return (top * (1.0 - fy) + bottom * fy) / 255.0


def build_layer_sources(
    definition: dict[str, Any],
    material_dds_root: Path,
) -> dict[str, dict[str, Any]]:
    scalar_parameters = parameter_map(definition, "scalarParameters")
    vector_parameters = parameter_map(definition, "vectorParameters")
    texture_parameters = parameter_map(definition, "textureParameters")
    material_package = str(definition["logicalPackage"])
    layers: dict[str, dict[str, Any]] = {}
    layer_names = [f"layer{layer_index:02d}" for layer_index in range(1, 8)]
    layer_names.append("layercliff")
    for layer in layer_names:
        diffuse_parameter = texture_parameters.get(f"{layer}_diffuse")
        if diffuse_parameter is None or not diffuse_parameter.get("parameterPath"):
            continue
        diffuse_path = material_texture_file(
            material_dds_root,
            material_package,
            str(diffuse_parameter["parameterPath"]),
        )
        normal_parameter = texture_parameters.get(f"{layer}_normal")
        normal_path = None
        if normal_parameter is not None and normal_parameter.get("parameterPath"):
            normal_path = material_texture_file(
                material_dds_root,
                material_package,
                str(normal_parameter["parameterPath"]),
            )
        color_parameter = vector_parameters.get(f"{layer}_color", {})
        color_value = color_parameter.get("parametervalue", (1.0, 1.0, 1.0, 1.0))
        layers[layer] = {
            "diffusePath": str(diffuse_path),
            "normalPath": str(normal_path) if normal_path is not None else None,
            "diffuse": decode_layer_texture(diffuse_path),
            "normal": (
                decode_layer_texture(normal_path) if normal_path is not None else None
            ),
            "tiling": float(
                scalar_parameters.get(f"{layer}_tiling", {}).get(
                    "parametervalue", 1.0
                )
            ),
            "rotation": float(
                scalar_parameters.get(f"{layer}_rotation", {}).get(
                    "parametervalue", 0.0
                )
            ),
            "brightness": float(
                scalar_parameters.get(f"{layer}_brightness", {}).get(
                    "parametervalue", 1.0
                )
            ),
            "desaturation": float(
                scalar_parameters.get(f"{layer}_desaturation", {}).get(
                    "parametervalue", 0.0
                )
            ),
            "normalIntensity": float(
                scalar_parameters.get(f"{layer}_normal_intensity", {}).get(
                    "parametervalue", 1.0
                )
            ),
            "color": tuple(float(component) for component in color_value),
            "sourceTexturePath": diffuse_parameter["parameterPath"],
            "sourceNormalPath": (
                normal_parameter.get("parameterPath")
                if normal_parameter is not None
                else None
            ),
        }
    return layers


def rotate_uv(u: float, v: float, degrees: float) -> tuple[float, float]:
    radians = math.radians(degrees)
    sine = math.sin(radians)
    cosine = math.cos(radians)
    return u * cosine - v * sine, u * sine + v * cosine


def bilinear(values: Sequence[float], fraction_x: float, fraction_y: float) -> float:
    top = values[0] * (1.0 - fraction_x) + values[1] * fraction_x
    bottom = values[2] * (1.0 - fraction_x) + values[3] * fraction_x
    return top * (1.0 - fraction_y) + bottom * fraction_y


def sample_component_height(
    component: LandscapeComponent,
    x: float,
    y: float,
) -> float:
    x0 = max(0, min(component.component_size_quads, int(math.floor(x))))
    y0 = max(0, min(component.component_size_quads, int(math.floor(y))))
    x1 = min(component.component_size_quads, x0 + 1)
    y1 = min(component.component_size_quads, y0 + 1)
    return bilinear(
        (
            local_height_at(component, x0, y0),
            local_height_at(component, x1, y0),
            local_height_at(component, x0, y1),
            local_height_at(component, x1, y1),
        ),
        x - x0,
        y - y0,
    )


def sample_component_source_normal(
    component: LandscapeComponent,
    x: float,
    y: float,
) -> tuple[float, float, float]:
    x0 = max(0, min(component.component_size_quads, int(math.floor(x))))
    y0 = max(0, min(component.component_size_quads, int(math.floor(y))))
    x1 = min(component.component_size_quads, x0 + 1)
    y1 = min(component.component_size_quads, y0 + 1)
    normals = (
        source_normal_client_at(component, x0, y0),
        source_normal_client_at(component, x1, y0),
        source_normal_client_at(component, x0, y1),
        source_normal_client_at(component, x1, y1),
    )
    fraction_x = x - x0
    fraction_y = y - y0
    return normalize3(
        tuple(
            bilinear(
                tuple(normal[channel] for normal in normals),
                fraction_x,
                fraction_y,
            )
            for channel in range(3)
        )
    )


def cliff_blend_weight(up_component: float) -> float:
    if up_component <= CLIFF_BLEND_STEEP_UP:
        return 1.0
    if up_component >= CLIFF_BLEND_FLAT_UP:
        return 0.0
    normalized = (CLIFF_BLEND_FLAT_UP - up_component) / (
        CLIFF_BLEND_FLAT_UP - CLIFF_BLEND_STEEP_UP
    )
    return normalized * normalized * (3.0 - 2.0 * normalized)


def component_world_point(
    component: LandscapeComponent,
    proxy: LandscapeProxy,
    local_x: float,
    local_y: float,
) -> tuple[float, float, float]:
    scale_x = proxy.draw_scale * proxy.draw_scale3d[0]
    scale_y = proxy.draw_scale * proxy.draw_scale3d[1]
    scale_z = proxy.draw_scale * proxy.draw_scale3d[2]
    return (
        (proxy.location[0] + (component.section_base_x + local_x) * scale_x)
        * 0.01,
        (proxy.location[2] + sample_component_height(component, local_x, local_y) * scale_z)
        * 0.01,
        -(proxy.location[1] + (component.section_base_y + local_y) * scale_y)
        * 0.01,
    )


def sample_cliff_projection(
    image: ImageRgba,
    layer: dict[str, Any],
    world_point: Sequence[float],
    source_normal: Sequence[float],
    component_span: Sequence[float],
) -> tuple[float, float, float, float]:
    span_x = max(abs(component_span[0]), 1.0e-8)
    span_z = max(abs(component_span[1]), 1.0e-8)
    vertical_span = max(span_x, span_z)
    uv_x = rotate_uv(
        world_point[2] / span_z * layer["tiling"],
        world_point[1] / vertical_span * layer["tiling"],
        layer["rotation"],
    )
    uv_z = rotate_uv(
        world_point[0] / span_x * layer["tiling"],
        world_point[1] / vertical_span * layer["tiling"],
        layer["rotation"],
    )
    sample_x = sample_image_repeat(image, *uv_x)
    sample_z = sample_image_repeat(image, *uv_z)
    weight_x = abs(source_normal[0])
    weight_z = abs(source_normal[2])
    weight_sum = weight_x + weight_z
    if weight_sum <= 1.0e-8:
        return sample_z
    return tuple(
        (sample_x[channel] * weight_x + sample_z[channel] * weight_z)
        / weight_sum
        for channel in range(4)
    )  # type: ignore[return-value]


def triangle_is_cliff(normal: Sequence[float]) -> bool:
    """Route faces outside the top-material slope range to side UVs."""
    return normal[1] < CLIFF_BLEND_FLAT_UP


def cliff_face_uv(
    world_point: Sequence[float],
    face_normal: Sequence[float],
    component_span: Sequence[float],
    layer: dict[str, Any],
) -> tuple[float, float]:
    """Project one cliff face in world height instead of top-down UV0.

    UE3's Landscape material performs a side projection for steep faces.  A
    single top-down baked texture cannot represent that projection because a
    near-vertical triangle has almost no X/Z UV span.  The current CMaterial
    path can still preserve the projection by putting steep triangles in a
    second glTF primitive whose UV includes world height.
    """
    span_x = max(abs(component_span[0]), 1.0e-8)
    span_z = max(abs(component_span[1]), 1.0e-8)
    vertical_span = max(span_x, span_z)
    if abs(face_normal[0]) >= abs(face_normal[2]):
        horizontal = world_point[2] / span_z
    else:
        horizontal = world_point[0] / span_x
    return rotate_uv(
        horizontal * layer["tiling"],
        world_point[1] / vertical_span * layer["tiling"],
        layer["rotation"],
    )


def adjusted_layer_diffuse(
    sample: Sequence[float],
    layer: dict[str, Any],
) -> tuple[float, float, float]:
    red, green, blue = sample[:3]
    luminance = red * 0.299 + green * 0.587 + blue * 0.114
    desaturation = layer["desaturation"]
    color = layer["color"]
    return (
        (red * (1.0 - desaturation) + luminance * desaturation)
        * layer["brightness"]
        * color[0],
        (green * (1.0 - desaturation) + luminance * desaturation)
        * layer["brightness"]
        * color[1],
        (blue * (1.0 - desaturation) + luminance * desaturation)
        * layer["brightness"]
        * color[2],
    )


def decoded_layer_normal(
    sample: Sequence[float],
    layer: dict[str, Any],
) -> tuple[float, float, float]:
    return (
        (sample[0] * 2.0 - 1.0) * layer["normalIntensity"],
        (sample[1] * 2.0 - 1.0) * layer["normalIntensity"],
        sample[2] * 2.0 - 1.0,
    )


def bake_component_textures(
    component: LandscapeComponent,
    proxy: LandscapeProxy,
    layers: dict[str, dict[str, Any]],
    resolution: int,
) -> tuple[ImageRgba, ImageRgba, ImageRgba, dict[str, Any]]:
    if resolution < 64 or resolution > 2048:
        raise LandscapeError("bake resolution must be between 64 and 2048")
    visual_allocations = [
        allocation
        for allocation in component.allocations
        if allocation.layer_name.casefold() != "__datalayer__"
    ]
    if not visual_allocations:
        raise LandscapeError(f"component {component.export_index} has no visual layers")
    missing_layers = sorted(
        {
            allocation.layer_name.casefold()
            for allocation in visual_allocations
            if allocation.layer_name.casefold() not in layers
        }
    )
    if missing_layers:
        raise LandscapeError(
            f"component {component.export_index} has unmapped material layers {missing_layers}"
        )
    cliff_layer = layers.get("layercliff")
    if cliff_layer is None:
        raise LandscapeError("master Landscape material has no layercliff source")
    hole_allocation = hole_layer_allocation(component)
    diffuse_pixels: list[tuple[int, int, int, int]] = []
    normal_pixels: list[tuple[int, int, int, int]] = []
    hole_pixels: list[tuple[int, int, int, int]] = []
    visual_layer_set = sorted(
        {allocation.layer_name.casefold() for allocation in visual_allocations}
    )
    component_span = (
        component.component_size_quads
        * proxy.draw_scale
        * proxy.draw_scale3d[0]
        * 0.01,
        component.component_size_quads
        * proxy.draw_scale
        * proxy.draw_scale3d[1]
        * 0.01,
    )
    for pixel_y in range(resolution):
        local_y = pixel_y * component.component_size_quads / (resolution - 1)
        for pixel_x in range(resolution):
            local_x = pixel_x * component.component_size_quads / (resolution - 1)
            weights = [
                sample_component_weight(component, allocation, local_x, local_y)
                for allocation in visual_allocations
            ]
            weight_sum = sum(weights)
            if weight_sum <= 1.0e-8:
                weights = [1.0] + [0.0] * (len(weights) - 1)
                weight_sum = 1.0
            weights = [weight / weight_sum for weight in weights]

            diffuse_accumulator = [0.0, 0.0, 0.0]
            normal_accumulator = [0.0, 0.0, 0.0]
            for allocation, weight in zip(visual_allocations, weights):
                layer_name = allocation.layer_name.casefold()
                layer = layers[layer_name]
                world_u = (
                    component.section_base_x + local_x
                ) / component.component_size_quads
                world_v = (
                    component.section_base_y + local_y
                ) / component.component_size_quads
                uv_u, uv_v = rotate_uv(
                    world_u * layer["tiling"],
                    world_v * layer["tiling"],
                    layer["rotation"],
                )
                adjusted = adjusted_layer_diffuse(
                    sample_image_repeat(layer["diffuse"], uv_u, uv_v),
                    layer,
                )
                for channel in range(3):
                    diffuse_accumulator[channel] += adjusted[channel] * weight

                if layer["normal"] is None:
                    sampled_normal = (0.0, 0.0, 1.0)
                else:
                    sampled_normal = decoded_layer_normal(
                        sample_image_repeat(layer["normal"], uv_u, uv_v),
                        layer,
                    )
                for channel in range(3):
                    normal_accumulator[channel] += sampled_normal[channel] * weight

            source_normal = sample_component_source_normal(
                component, local_x, local_y
            )
            cliff_weight = cliff_blend_weight(source_normal[1])
            if cliff_weight > 0.0:
                world_point = component_world_point(
                    component, proxy, local_x, local_y
                )
                cliff_diffuse = adjusted_layer_diffuse(
                    sample_cliff_projection(
                        cliff_layer["diffuse"],
                        cliff_layer,
                        world_point,
                        source_normal,
                        component_span,
                    ),
                    cliff_layer,
                )
                if cliff_layer["normal"] is None:
                    cliff_normal = (0.0, 0.0, 1.0)
                else:
                    cliff_normal = decoded_layer_normal(
                        sample_cliff_projection(
                            cliff_layer["normal"],
                            cliff_layer,
                            world_point,
                            source_normal,
                            component_span,
                        ),
                        cliff_layer,
                    )
                for channel in range(3):
                    diffuse_accumulator[channel] = (
                        diffuse_accumulator[channel] * (1.0 - cliff_weight)
                        + cliff_diffuse[channel] * cliff_weight
                    )
                    normal_accumulator[channel] = (
                        normal_accumulator[channel] * (1.0 - cliff_weight)
                        + cliff_normal[channel] * cliff_weight
                    )

            normal_length = math.sqrt(
                sum(component_value * component_value for component_value in normal_accumulator)
            )
            if normal_length <= 1.0e-8:
                normal_accumulator = [0.0, 0.0, 1.0]
                normal_length = 1.0
            normal_accumulator = [
                component_value / normal_length
                for component_value in normal_accumulator
            ]
            hole_weight = (
                sample_component_weight(
                    component, hole_allocation, local_x, local_y
                )
                if hole_allocation is not None
                else 0.0
            )
            alpha = 0 if is_ue3_landscape_hole(hole_weight * 255.0) else 255
            diffuse_pixels.append(
                (
                    clamp_byte(diffuse_accumulator[0] * 255.0),
                    clamp_byte(diffuse_accumulator[1] * 255.0),
                    clamp_byte(diffuse_accumulator[2] * 255.0),
                    alpha,
                )
            )
            normal_pixels.append(
                (
                    clamp_byte((normal_accumulator[0] * 0.5 + 0.5) * 255.0),
                    clamp_byte((normal_accumulator[1] * 0.5 + 0.5) * 255.0),
                    clamp_byte((normal_accumulator[2] * 0.5 + 0.5) * 255.0),
                    255,
                )
            )
            hole_value = clamp_byte(hole_weight * 255.0)
            hole_pixels.append((hole_value, hole_value, hole_value, 255))
    return (
        ImageRgba(resolution, resolution, tuple(diffuse_pixels)),
        ImageRgba(resolution, resolution, tuple(normal_pixels)),
        ImageRgba(resolution, resolution, tuple(hole_pixels)),
        {
            "mode": "offline-display-derivative",
            "authoritative": False,
            "visualLayers": visual_layer_set,
            "cliffLayer": "layercliff",
            "cliffProjection": "height-aware-side-projection",
            "cliffBlendSteepUp": CLIFF_BLEND_STEEP_UP,
            "cliffBlendFlatUp": CLIFF_BLEND_FLAT_UP,
            "holeLayer": hole_allocation.layer_name if hole_allocation else None,
            "holeThreshold": UE3_LANDSCAPE_HOLE_THRESHOLD,
            "holeThresholdComparison": ">",
            "renderHoleMode": "top-left-owned-quad-topology",
            "resolution": resolution,
            "limitations": [
                "UE3 Landscape material graph is not executed",
                "cliff blend thresholds and side projection are a deterministic approximation",
                "raw Weightmaps and original material parameters remain authoritative",
            ],
        },
    )


def normalize3(value: Sequence[float]) -> tuple[float, float, float]:
    length = math.sqrt(sum(component * component for component in value))
    if length <= 1.0e-12:
        return 0.0, 1.0, 0.0
    return tuple(component / length for component in value)  # type: ignore[return-value]


def subtract3(left: Sequence[float], right: Sequence[float]) -> tuple[float, float, float]:
    return tuple(left[index] - right[index] for index in range(3))  # type: ignore[return-value]


def cross3(left: Sequence[float], right: Sequence[float]) -> tuple[float, float, float]:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def component_positions(
    component: LandscapeComponent,
    proxy: LandscapeProxy,
) -> list[tuple[float, float, float]]:
    draw_scale = proxy.draw_scale
    scale_x = draw_scale * proxy.draw_scale3d[0] * 0.01
    scale_y = draw_scale * proxy.draw_scale3d[1] * 0.01
    scale_z = draw_scale * proxy.draw_scale3d[2] * 0.01
    return [
        (
            x * scale_x,
            local_height_at(component, x, y) * scale_z,
            -y * scale_y,
        )
        for y in range(component.component_size_quads + 1)
        for x in range(component.component_size_quads + 1)
    ]


def component_normals_and_tangents(
    component: LandscapeComponent,
    positions: Sequence[tuple[float, float, float]],
    grid_size: int,
) -> tuple[list[tuple[float, float, float]], list[tuple[float, float, float, float]]]:
    normals: list[tuple[float, float, float]] = []
    tangents: list[tuple[float, float, float, float]] = []
    for y in range(grid_size):
        for x in range(grid_size):
            left = positions[y * grid_size + max(0, x - 1)]
            right = positions[y * grid_size + min(grid_size - 1, x + 1)]
            normal = source_normal_client_at(component, x, y)
            source_tangent = normalize3(subtract3(right, left))
            tangent_projection = sum(
                source_tangent[index] * normal[index] for index in range(3)
            )
            tangent_x = normalize3(
                tuple(
                    source_tangent[index] - normal[index] * tangent_projection
                    for index in range(3)
                )
            )
            normals.append(normal)
            tangents.append((*tangent_x, 1.0))
    return normals, tangents


def align4(buffer: bytearray) -> None:
    while len(buffer) % 4:
        buffer.append(0)


def write_component_gltf(
    component: LandscapeComponent,
    proxy: LandscapeProxy,
    destination: Path,
    asset_id: str,
    cliff_layer: dict[str, Any],
) -> dict[str, Any]:
    grid_size = component.component_size_quads + 1
    positions = component_positions(component, proxy)
    normals, tangents = component_normals_and_tangents(
        component, positions, grid_size
    )
    texture_coordinates = [
        (
            x / component.component_size_quads,
            y / component.component_size_quads,
        )
        for y in range(grid_size)
        for x in range(grid_size)
    ]
    top_indices: list[int] = []
    cliff_positions: list[tuple[float, float, float]] = []
    cliff_normals: list[tuple[float, float, float]] = []
    cliff_texture_coordinates: list[tuple[float, float]] = []
    cliff_tangents: list[tuple[float, float, float, float]] = []
    cliff_indices: list[int] = []
    hole_quad_count = 0
    anchor = (
        (
            proxy.location[0]
            + component.section_base_x
            * proxy.draw_scale
            * proxy.draw_scale3d[0]
        )
        * 0.01,
        proxy.location[2] * 0.01,
        -(
            proxy.location[1]
            + component.section_base_y
            * proxy.draw_scale
            * proxy.draw_scale3d[1]
        )
        * 0.01,
    )
    component_span = (
        component.component_size_quads
        * proxy.draw_scale
        * proxy.draw_scale3d[0]
        * 0.01,
        component.component_size_quads
        * proxy.draw_scale
        * proxy.draw_scale3d[1]
        * 0.01,
    )

    def append_cliff_triangle(source_indices: Sequence[int]) -> None:
        triangle_positions = [positions[index] for index in source_indices]
        face_normal = normalize3(
            cross3(
                subtract3(triangle_positions[1], triangle_positions[0]),
                subtract3(triangle_positions[2], triangle_positions[0]),
            )
        )
        world_points = [
            tuple(anchor[axis] + point[axis] for axis in range(3))
            for point in triangle_positions
        ]
        face_uvs = [
            cliff_face_uv(point, face_normal, component_span, cliff_layer)
            for point in world_points
        ]
        edge1 = subtract3(triangle_positions[1], triangle_positions[0])
        edge2 = subtract3(triangle_positions[2], triangle_positions[0])
        delta_u1 = face_uvs[1][0] - face_uvs[0][0]
        delta_v1 = face_uvs[1][1] - face_uvs[0][1]
        delta_u2 = face_uvs[2][0] - face_uvs[0][0]
        delta_v2 = face_uvs[2][1] - face_uvs[0][1]
        determinant = delta_u1 * delta_v2 - delta_u2 * delta_v1
        if abs(determinant) <= 1.0e-8:
            face_tangent = (0.0, 0.0, 1.0) if abs(face_normal[0]) >= abs(
                face_normal[2]
            ) else (1.0, 0.0, 0.0)
        else:
            inverse = 1.0 / determinant
            face_tangent = normalize3(
                tuple(
                    (edge1[axis] * delta_v2 - edge2[axis] * delta_v1)
                    * inverse
                    for axis in range(3)
                )
            )
        first_index = len(cliff_positions)
        for source_index, point, uv in zip(
            source_indices, triangle_positions, face_uvs
        ):
            normal = normals[source_index]
            projection = sum(
                face_tangent[axis] * normal[axis] for axis in range(3)
            )
            tangent = normalize3(
                tuple(
                    face_tangent[axis] - normal[axis] * projection
                    for axis in range(3)
                )
            )
            cliff_positions.append(point)
            cliff_normals.append(normal)
            cliff_texture_coordinates.append(uv)
            cliff_tangents.append((*tangent, 1.0))
        cliff_indices.extend((first_index, first_index + 1, first_index + 2))

    for y in range(component.component_size_quads):
        for x in range(component.component_size_quads):
            if quad_is_hole(component, x, y):
                hole_quad_count += 1
                continue
            top_left = y * grid_size + x
            top_right = top_left + 1
            bottom_left = top_left + grid_size
            bottom_right = bottom_left + 1
            for triangle in (
                (top_left, top_right, bottom_left),
                (top_right, bottom_right, bottom_left),
            ):
                triangle_positions = [positions[index] for index in triangle]
                face_normal = normalize3(
                    cross3(
                        subtract3(triangle_positions[1], triangle_positions[0]),
                        subtract3(triangle_positions[2], triangle_positions[0]),
                    )
                )
                if triangle_is_cliff(face_normal):
                    append_cliff_triangle(triangle)
                else:
                    top_indices.extend(triangle)
    if top_indices and max(top_indices) > 65535:
        raise LandscapeError("Landscape component requires 32-bit indices")
    if cliff_indices and max(cliff_indices) > 65535:
        raise LandscapeError("Landscape cliff primitive requires 32-bit indices")

    binary = bytearray()
    buffer_views: list[dict[str, Any]] = []
    accessors: list[dict[str, Any]] = []

    def append_view(data: bytes, target: int | None) -> int:
        align4(binary)
        offset = len(binary)
        binary.extend(data)
        view: dict[str, Any] = {
            "buffer": 0,
            "byteOffset": offset,
            "byteLength": len(data),
        }
        if target is not None:
            view["target"] = target
        buffer_views.append(view)
        return len(buffer_views) - 1

    def append_accessor(
        data: bytes,
        component_type: int,
        count: int,
        value_type: str,
        target: int,
        minimum: Sequence[float] | None = None,
        maximum: Sequence[float] | None = None,
    ) -> int:
        accessor: dict[str, Any] = {
            "bufferView": append_view(data, target),
            "componentType": component_type,
            "count": count,
            "type": value_type,
        }
        if minimum is not None:
            accessor["min"] = list(minimum)
        if maximum is not None:
            accessor["max"] = list(maximum)
        accessors.append(accessor)
        return len(accessors) - 1

    minimum = [min(item[axis] for item in positions) for axis in range(3)]
    maximum = [max(item[axis] for item in positions) for axis in range(3)]
    top_position_accessor = append_accessor(
        b"".join(struct.pack("<3f", *item) for item in positions),
        5126,
        len(positions),
        "VEC3",
        34962,
        minimum,
        maximum,
    )
    top_normal_accessor = append_accessor(
        b"".join(struct.pack("<3f", *item) for item in normals),
        5126,
        len(normals),
        "VEC3",
        34962,
    )
    top_uv_accessor = append_accessor(
        b"".join(struct.pack("<2f", *item) for item in texture_coordinates),
        5126,
        len(texture_coordinates),
        "VEC2",
        34962,
    )
    top_tangent_accessor = append_accessor(
        b"".join(struct.pack("<4f", *item) for item in tangents),
        5126,
        len(tangents),
        "VEC4",
        34962,
    )
    primitives: list[dict[str, Any]] = []
    if top_indices:
        top_index_accessor = append_accessor(
            b"".join(struct.pack("<H", item) for item in top_indices),
            5123,
            len(top_indices),
            "SCALAR",
            34963,
            [min(top_indices)],
            [max(top_indices)],
        )
        primitives.append(
            {
                "attributes": {
                    "POSITION": top_position_accessor,
                    "NORMAL": top_normal_accessor,
                    "TEXCOORD_0": top_uv_accessor,
                    "TANGENT": top_tangent_accessor,
                },
                "indices": top_index_accessor,
                "material": 0,
                "mode": 4,
            }
        )
    if cliff_indices:
        cliff_minimum = [
            min(item[axis] for item in cliff_positions) for axis in range(3)
        ]
        cliff_maximum = [
            max(item[axis] for item in cliff_positions) for axis in range(3)
        ]
        cliff_position_accessor = append_accessor(
            b"".join(struct.pack("<3f", *item) for item in cliff_positions),
            5126,
            len(cliff_positions),
            "VEC3",
            34962,
            cliff_minimum,
            cliff_maximum,
        )
        cliff_normal_accessor = append_accessor(
            b"".join(struct.pack("<3f", *item) for item in cliff_normals),
            5126,
            len(cliff_normals),
            "VEC3",
            34962,
        )
        cliff_uv_accessor = append_accessor(
            b"".join(
                struct.pack("<2f", *item)
                for item in cliff_texture_coordinates
            ),
            5126,
            len(cliff_texture_coordinates),
            "VEC2",
            34962,
        )
        cliff_tangent_accessor = append_accessor(
            b"".join(struct.pack("<4f", *item) for item in cliff_tangents),
            5126,
            len(cliff_tangents),
            "VEC4",
            34962,
        )
        cliff_index_accessor = append_accessor(
            b"".join(struct.pack("<H", item) for item in cliff_indices),
            5123,
            len(cliff_indices),
            "SCALAR",
            34963,
            [min(cliff_indices)],
            [max(cliff_indices)],
        )
        primitives.append(
            {
                "attributes": {
                    "POSITION": cliff_position_accessor,
                    "NORMAL": cliff_normal_accessor,
                    "TEXCOORD_0": cliff_uv_accessor,
                    "TANGENT": cliff_tangent_accessor,
                },
                "indices": cliff_index_accessor,
                "material": 1,
                "mode": 4,
            }
        )
    binary_name = destination.with_suffix(".bin").name
    document = {
        "asset": {"version": "2.0", "generator": "LostArk LandscapeExtractor"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"name": asset_id, "mesh": 0}],
        "meshes": [
            {
                "name": asset_id,
                "primitives": primitives,
            }
        ],
        "materials": [
            {
                "name": MATERIAL_NAME,
                "pbrMetallicRoughness": {
                    "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0,
                },
                "doubleSided": False,
            },
            {
                "name": CLIFF_MATERIAL_NAME,
                "pbrMetallicRoughness": {
                    "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0,
                },
                "doubleSided": False,
            },
        ],
        "buffers": [{"uri": binary_name, "byteLength": len(binary)}],
        "bufferViews": buffer_views,
        "accessors": accessors,
        "extras": {
            "source": f"{component.logical_package}:export:{component.export_index}",
            "coordinateSystem": "Client (UE X,Z,-Y), meters",
            "proxyScaleBakedIntoVertices": True,
            "holesAppliedToTopology": True,
            "holeOwnership": "top-left-sample",
            "holeThreshold": UE3_LANDSCAPE_HOLE_THRESHOLD,
            "cliffMaterial": CLIFF_MATERIAL_NAME,
            "cliffProjection": "dominant-axis-world-height",
            "cliffFaceUpThreshold": CLIFF_BLEND_FLAT_UP,
        },
    }
    atomic_write_bytes(destination.with_suffix(".bin"), bytes(binary))
    atomic_write_text(destination, json_text(document))
    return {
        "sourceVertexCount": len(positions),
        "vertexCount": len(positions) + len(cliff_positions),
        "indexCount": len(top_indices) + len(cliff_indices),
        "triangleCount": (len(top_indices) + len(cliff_indices)) // 3,
        "topTriangleCount": len(top_indices) // 3,
        "cliffTriangleCount": len(cliff_indices) // 3,
        "holeQuadCount": hole_quad_count,
        "normalSource": "UE3 Heightmap B/A packed normal",
        "boundsMin": minimum,
        "boundsMax": maximum,
        "gltf": destination.name,
        "binary": binary_name,
    }


def stable_asset_id(component: LandscapeComponent) -> tuple[str, str]:
    canonical = (
        f"{component.logical_package.casefold()}:"
        f"landscapecomponent:export:{component.export_index}"
    )
    digest = hashlib.sha1(canonical.encode("utf-8")).hexdigest()[:12].upper()
    package_suffix = component.logical_package.rsplit("_", 1)[-1].upper()
    return canonical, f"MAP_{digest}_{package_suffix}_LC_{component.export_index:05d}"


def stable_placement_id(source_placement_id: str) -> int:
    digest = hashlib.sha256(source_placement_id.encode("utf-8")).digest()
    return int.from_bytes(digest[:8], "big") | IMPORTED_ID_BIT


def repository_argument(path: Path) -> str:
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(REPOSITORY_ROOT))
    except ValueError:
        return str(resolved)


def cook_wmodel(
    converter: Path,
    gltf_path: Path,
    diffuse_path: Path,
    normal_path: Path,
    cliff_diffuse_path: Path,
    cliff_normal_path: Path | None,
    output_path: Path,
) -> dict[str, Any]:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    command = [
        str(converter),
        repository_argument(gltf_path),
        "-o",
        repository_argument(output_path),
        "--pretransform",
        "--no-auto-textures",
        "--scale",
        "100",
        "--material-remap",
        f"{MATERIAL_NAME}={repository_argument(diffuse_path)}",
        "--normal-remap",
        f"{MATERIAL_NAME}={repository_argument(normal_path)}",
        "--material-remap",
        f"{CLIFF_MATERIAL_NAME}={repository_argument(cliff_diffuse_path)}",
    ]
    if cliff_normal_path is not None:
        command.extend(
            [
                "--normal-remap",
                f"{CLIFF_MATERIAL_NAME}={repository_argument(cliff_normal_path)}",
            ]
        )
    cook_log = run_process(command, REPOSITORY_ROOT, "ModelAssetConverter cook")
    if not output_path.is_file():
        raise LandscapeError(f"converter did not create {output_path}")
    header = output_path.read_bytes()[:20]
    if len(header) < 20 or header[:4] != b"WINT" or header[16:20] != b"WMOD":
        raise LandscapeError(f"invalid WModel container header: {output_path}")
    info_log = run_process(
        [str(converter), "info", repository_argument(output_path)],
        REPOSITORY_ROOT,
        "ModelAssetConverter info",
    )
    model_bytes = output_path.read_bytes()
    # A raw b":\\" test is not valid for a binary container: index and
    # floating-point bytes can coincidentally form those two characters.  Only
    # reject a printable drive-qualified path with at least one directory.
    if re.search(rb"[A-Za-z]:[\\/][A-Za-z0-9_. -]+[\\/]", model_bytes):
        raise LandscapeError(f"WModel contains an absolute path: {output_path}")
    textures = sorted((output_path.parent / "textures").glob("*"))
    if len(textures) < 2:
        raise LandscapeError(f"WModel pack is missing baked textures: {output_path}")
    return {
        "model": output_path.name,
        "size": output_path.stat().st_size,
        "sha256": sha256_file(output_path),
        "textureFiles": [
            {
                "name": texture.name,
                "size": texture.stat().st_size,
                "sha256": sha256_file(texture),
            }
            for texture in textures
            if texture.is_file()
        ],
        "cookLog": cook_log,
        "infoLog": info_log,
    }


def quote(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def component_world_anchor(
    component: LandscapeComponent,
    proxy: LandscapeProxy,
) -> tuple[float, float, float]:
    scale_x = proxy.draw_scale * proxy.draw_scale3d[0]
    scale_y = proxy.draw_scale * proxy.draw_scale3d[1]
    ue_x = proxy.location[0] + component.section_base_x * scale_x
    ue_y = proxy.location[1] + component.section_base_y * scale_y
    ue_z = proxy.location[2]
    return ue_x * 0.01, ue_z * 0.01, -ue_y * 0.01


def write_map_documents(
    output_root: Path,
    area_id: str,
    runtime_rows: Sequence[dict[str, Any]],
    group_label: str = "Bern Castle Landscape",
) -> dict[str, Any]:
    data_root = output_root / "DataFiles" / "Map"
    data_root.mkdir(parents=True, exist_ok=True)
    catalog_rows: list[str] = []
    placement_rows: list[str] = []
    default_render_profile = "Opaque Back 1 1 0 0 1 1 1 50 1 1 1 1 1"
    for row in sorted(runtime_rows, key=lambda item: str(item["assetId"])):
        asset_id = str(row["assetId"])
        prototype_tag = "Prototype_Component_Model_" + asset_id
        catalog_rows.append(
            " ".join(
                (
                    quote(asset_id),
                    quote(str(row["label"])),
                    quote(str(row["modelPath"])),
                    quote(prototype_tag),
                    "1 1 1 Origin",
                    quote("landscape"),
                    quote(group_label),
                    quote(str(row["evidence"])),
                    default_render_profile,
                )
            )
        )
        position = row["position"]
        placement_rows.append(
            " ".join(
                (
                    str(row["placementId"]),
                    quote(str(row["sourcePlacementId"])),
                    quote(str(row["sourceLevel"])),
                    quote("component"),
                    quote(asset_id),
                    *(format(float(value), ".9g") for value in position),
                    "0 0 0 1",
                    "1 1 1",
                    "1",
                )
            )
        )
    catalog_path = data_root / f"{area_id}.mapassets"
    placement_path = data_root / f"{area_id}.mapplacements"
    atomic_write_text(
        catalog_path,
        f"LOSTARK_MAP_ASSET_CATALOG 4 {quote(area_id)} {len(catalog_rows)}\n"
        + "\n".join(catalog_rows)
        + "\n",
    )
    atomic_write_text(
        placement_path,
        f"LOSTARK_MAP_PLACEMENTS 2 {quote(area_id)} {len(placement_rows)}\n"
        + "\n".join(placement_rows)
        + "\n",
    )
    return {
        "areaId": area_id,
        "assetCount": len(catalog_rows),
        "placementCount": len(placement_rows),
        "catalog": str(catalog_path.relative_to(output_root)).replace("\\", "/"),
        "catalogSha256": sha256_file(catalog_path),
        "placements": str(placement_path.relative_to(output_root)).replace("\\", "/"),
        "placementsSha256": sha256_file(placement_path),
    }


def component_source_document(
    component: LandscapeComponent,
    proxy: LandscapeProxy,
    texture_paths: dict[int, str],
    validation: dict[str, Any],
    local_material: dict[str, Any] | None,
) -> dict[str, Any]:
    canonical, asset_id = stable_asset_id(component)
    return {
        "schemaVersion": 1,
        "canonicalId": canonical,
        "assetId": asset_id,
        "logicalPackage": component.logical_package,
        "component": {
            "exportIndex": component.export_index,
            "objectName": component.object_name,
            "sectionBase": [component.section_base_x, component.section_base_y],
            "componentSizeQuads": component.component_size_quads,
            "subsectionSizeQuads": component.subsection_size_quads,
            "numSubsections": component.num_subsections,
            "cachedLocalBox": component.cached_local_box,
        },
        "proxy": {
            "exportIndex": proxy.export_index,
            "objectName": proxy.object_name,
            "landscapeGuid": proxy.landscape_guid,
            "location": list(proxy.location),
            "drawScale": proxy.draw_scale,
            "drawScale3D": list(proxy.draw_scale3d),
            "landscapeMaterialPath": proxy.landscape_material_path,
            "collisionComponentRefs": proxy.collision_component_refs,
        },
        "heightmap": {
            "ref": component.heightmap_ref,
            "name": component.heightmap_name,
            "path": component.heightmap_path,
            "scaleBias": list(component.heightmap_scale_bias),
            "source": texture_paths[component.heightmap_ref],
            "decode": {
                "height16": "R*256+G",
                "localZ": "(height16-32768)/128",
                "packedNormal": (
                    "UE=(B/127.5-1,A/127.5-1,sqrt(1-x*x-y*y)); "
                    "Client=(x,z,-y)"
                ),
            },
        },
        "weightmaps": [
            {
                "ref": reference,
                "name": name,
                "path": path,
                "source": texture_paths[reference],
            }
            for reference, name, path in zip(
                component.weightmap_refs,
                component.weightmap_names,
                component.weightmap_paths,
            )
        ],
        "weightmapScaleBias": list(component.weightmap_scale_bias),
        "weightmapSubsectionOffset": component.weightmap_subsection_offset,
        "layerAllocations": [asdict(allocation) for allocation in component.allocations],
        "holeContract": {
            "layerName": "__datalayer__",
            "threshold": UE3_LANDSCAPE_HOLE_THRESHOLD,
            "thresholdComparison": ">",
            "renderMode": "top-left-owned-quad-topology",
            "collisionGenerated": False,
            "topologyRemoved": True,
            "ownership": "the top-left __DataLayer__ sample owns both triangles of a quad",
        },
        "materialInstance": local_material,
        "validation": validation,
    }


def relative_posix(path: Path, root: Path) -> str:
    return str(path.relative_to(root)).replace("\\", "/")


def build_landscape_pack(args: argparse.Namespace, stage: Path) -> dict[str, Any]:
    source_root = stage / "SourceRaw"
    derived_root = stage / "SourceDerived"
    runtime_asset_root = stage / "Resources"
    report_root = stage / "Reports"
    for path in (source_root, derived_root, runtime_asset_root, report_root):
        path.mkdir(parents=True, exist_ok=True)

    packages: list[PackageData] = []
    proxies: dict[str, LandscapeProxy] = {}
    components: list[LandscapeComponent] = []
    collisions_by_package: dict[str, list[LandscapeCollisionComponent]] = {}
    local_materials_by_package: dict[str, list[dict[str, Any]]] = {}
    package_rows: list[dict[str, Any]] = []
    for logical_name in args.packages:
        physical_path = UE3.resolve_physical_package(
            args.umodel, args.package_root, logical_name, args.region
        )
        package = load_package(physical_path, logical_name, args.aes_key)
        proxy, package_components, local_materials = parse_landscape_package(package)
        package_collisions = parse_landscape_collision_components(package, proxy)
        packages.append(package)
        proxies[logical_name] = proxy
        collisions_by_package[logical_name] = package_collisions
        local_materials_by_package[logical_name] = local_materials
        components.extend(package_components)
        package_rows.append(
            {
                "logicalPackage": logical_name,
                "physicalPackage": str(physical_path),
                "physicalPackageSha256": sha256_file(physical_path),
                "packageVersion": package.summary.version,
                "licenseeVersion": package.summary.licensee_version,
                "engineVersion": package.summary.engine_version,
                "exportCount": package.summary.export_count,
                "landscapeComponentCount": len(package_components),
                "landscapeCollisionComponentCount": len(package_collisions),
                "localTexture2DCount": sum(
                    class_name(package, export).casefold() == "texture2d"
                    for export in package.exports
                ),
                "localLandscapeMaterialInstanceCount": len(local_materials),
            }
        )
    if len(components) != args.expect_components:
        raise LandscapeError(
            f"Landscape component count {len(components)} != expected {args.expect_components}"
        )
    landscape_guids = {proxy.landscape_guid for proxy in proxies.values()}
    material_paths = {proxy.landscape_material_path for proxy in proxies.values()}
    proxy_transforms = {
        (proxy.location, proxy.draw_scale, proxy.draw_scale3d)
        for proxy in proxies.values()
    }
    if len(landscape_guids) != 1 or len(material_paths) != 1 or len(proxy_transforms) != 1:
        raise LandscapeError("LAND packages do not describe one compatible Landscape")

    texture_metadata_by_package: dict[str, dict[int, dict[str, Any]]] = {}
    texture_output_by_package: dict[str, dict[int, str]] = {}
    component_validations: dict[tuple[str, int], dict[str, Any]] = {}
    collision_validations: dict[tuple[str, int], dict[str, Any]] = {}
    for package in packages:
        package_components = [
            component
            for component in components
            if component.logical_package == package.logical_name
        ]
        referenced_texture_refs = sorted(
            {
                reference
                for component in package_components
                for reference in [component.heightmap_ref, *component.weightmap_refs]
            }
        )
        all_texture_refs = sorted(
            export.index + 1
            for export in package.exports
            if class_name(package, export).casefold() == "texture2d"
        )
        decoded = {
            reference: decode_texture(package, reference)
            for reference in referenced_texture_refs
        }
        texture_metadata_by_package[package.logical_name] = {}
        texture_output_by_package[package.logical_name] = {}
        for reference, texture in decoded.items():
            destination = (
                source_root
                / package.logical_name
                / "Textures"
                / f"{texture.export_index:05d}_{texture.object_name}"
            )
            metadata = write_texture_source(texture, destination)
            texture_metadata_by_package[package.logical_name][reference] = metadata
            texture_output_by_package[package.logical_name][reference] = relative_posix(
                destination, stage
            )
        local_material_by_ref = {
            int(material["ref"]): material
            for material in local_materials_by_package[package.logical_name]
        }
        component_source_dir = source_root / package.logical_name / "Components"
        component_source_dir.mkdir(parents=True, exist_ok=True)
        collision_by_section = {
            (collision.section_base_x, collision.section_base_y): collision
            for collision in collisions_by_package[package.logical_name]
        }
        component_sections = {
            (component.section_base_x, component.section_base_y)
            for component in package_components
        }
        if set(collision_by_section) != component_sections:
            raise LandscapeError(
                f"{package.logical_name} render/collision SectionBase sets differ"
            )
        for component in package_components:
            component.height_texture = decoded[component.heightmap_ref]
            component.weight_textures = [
                decoded[reference] for reference in component.weightmap_refs
            ]
            validation = validate_component_texture_contract(
                component, args.hole_mask_tolerance
            )
            component_validations[(package.logical_name, component.export_index)] = validation
            collision_validation = validate_collision_height_contract(
                component,
                collision_by_section[
                    (component.section_base_x, component.section_base_y)
                ],
            )
            collision_validations[
                (package.logical_name, component.export_index)
            ] = collision_validation
            document = component_source_document(
                component,
                proxies[package.logical_name],
                texture_output_by_package[package.logical_name],
                validation,
                local_material_by_ref.get(component.material_instance_ref),
            )
            document["collisionHeightValidation"] = collision_validation
            atomic_write_text(
                component_source_dir / f"component_{component.export_index:05d}.json",
                json_text(document),
            )
        serial_groups = write_export_serial_sources(
            package,
            {
                "proxy": [proxies[package.logical_name].export_index + 1],
                "component": [item.export_index + 1 for item in package_components],
                "collision": proxies[
                    package.logical_name
                ].collision_component_refs,
                "material": [
                    int(material["ref"])
                    for material in local_materials_by_package[package.logical_name]
                ],
                "texture": all_texture_refs,
            },
            source_root / package.logical_name / "Serials",
            stage,
        )
        material_serial_by_ref = {
            int(row["ref"]): row for row in serial_groups["material"]
        }
        for material in local_materials_by_package[package.logical_name]:
            source_row = material_serial_by_ref[int(material["ref"])]
            material["serialSource"] = source_row["source"]
            material["serialSha256"] = source_row["serialSha256"]
        atomic_write_text(
            source_root / package.logical_name / "local_material_instances.json",
            json_text(
                {
                    "schemaVersion": 2,
                    "count": len(local_materials_by_package[package.logical_name]),
                    "materials": local_materials_by_package[package.logical_name],
                }
            ),
        )

    edge_validation = validate_component_edges(components)

    master_material_path = next(iter(material_paths))
    material_pieces = master_material_path.split(".")
    if len(material_pieces) < 2:
        raise LandscapeError(f"invalid master material path {master_material_path}")
    material_logical_package = material_pieces[0].upper()
    material_name = material_pieces[-1]
    material_physical = UE3.resolve_physical_package(
        args.umodel, args.package_root, material_logical_package, args.region
    )
    material_package = load_package(
        material_physical, material_logical_package, args.aes_key
    )
    material_definition = parse_master_material_definition(
        material_package, material_name
    )
    material_source_root = source_root / "MasterMaterial"
    material_export = export_master_material(
        args.umodel,
        args.package_root,
        args.region,
        material_logical_package,
        material_name,
        material_source_root,
        stage,
    )
    master_serial_groups = write_export_serial_sources(
        material_package,
        {
            "master_material": [int(material_definition["ref"])],
            "layer_info": [
                int(item["exportIndex"]) + 1
                for item in material_definition["layerInfoObjects"]
            ],
        },
        material_source_root / "Serials",
        stage,
    )
    master_serial = master_serial_groups["master_material"][0]
    material_definition["serialSource"] = master_serial["source"]
    material_definition["serialSha256"] = master_serial["serialSha256"]
    layer_serial_by_export = {
        int(row["exportIndex"]): row
        for row in master_serial_groups["layer_info"]
    }
    for item in material_definition["layerInfoObjects"]:
        serial_row = layer_serial_by_export[int(item["exportIndex"])]
        item["serialSource"] = serial_row["source"]
        item["serialSha256"] = serial_row["serialSha256"]
    material_definition["textureDependencySerials"] = (
        preserve_master_texture_dependencies(
            material_definition,
            args.umodel,
            args.package_root,
            args.region,
            args.aes_key,
            material_source_root / "DependencySerials",
            stage,
            {material_logical_package: material_package},
            [
                (
                    f"{str(row['path']).split('/')[0]}.tex."
                    f"{Path(str(row['path'])).stem}"
                )
                for row in material_export["ddsFiles"]
            ],
        )
    )
    material_definition["umodelExport"] = {
        key: value
        for key, value in material_export.items()
        if key not in ("ddsRoot", "tgaRoot")
    }
    atomic_write_text(
        material_source_root / "master_material.json",
        json_text(material_definition),
    )
    layer_sources = build_layer_sources(
        material_definition, Path(material_export["ddsRoot"])
    )
    layer_manifest = {
        layer_name: {
            key: (
                relative_posix(Path(value), stage)
                if key in ("diffusePath", "normalPath") and value is not None
                else value
            )
            for key, value in layer.items()
            if key not in ("diffuse", "normal")
        }
        for layer_name, layer in layer_sources.items()
    }
    atomic_write_text(
        material_source_root / "layer_sources.json", json_text(layer_manifest)
    )

    runtime_rows: list[dict[str, Any]] = []
    total_vertices = 0
    total_triangles = 0
    for component in sorted(
        components,
        key=lambda item: (
            item.section_base_x,
            item.section_base_y,
            item.logical_package,
        ),
    ):
        proxy = proxies[component.logical_package]
        canonical, asset_id = stable_asset_id(component)
        component_derived = derived_root / asset_id
        component_derived.mkdir(parents=True, exist_ok=True)
        diffuse, normal, hole, bake_metadata = bake_component_textures(
            component, proxy, layer_sources, args.bake_resolution
        )
        diffuse_path = component_derived / "baked_diffuse.png"
        normal_path = component_derived / "baked_normal.png"
        hole_path = component_derived / "hole_mask.png"
        atomic_write_bytes(diffuse_path, encode_png_rgba(diffuse))
        atomic_write_bytes(normal_path, encode_png_rgba(normal))
        atomic_write_bytes(hole_path, encode_png_rgba(hole))
        gltf_path = component_derived / f"{asset_id}.gltf"
        geometry = write_component_gltf(
            component,
            proxy,
            gltf_path,
            asset_id,
            layer_sources["layercliff"],
        )
        total_vertices += int(geometry["vertexCount"])
        total_triangles += int(geometry["triangleCount"])

        runtime_directory = (
            runtime_asset_root
            / "Map"
            / args.pack_name
            / "Landscape"
            / asset_id
        )
        model_path = runtime_directory / f"{asset_id}.wmodel"
        cook_result = None
        if not args.no_cook:
            cook_result = cook_wmodel(
                args.converter,
                gltf_path,
                diffuse_path,
                normal_path,
                Path(layer_sources["layercliff"]["diffusePath"]),
                (
                    Path(layer_sources["layercliff"]["normalPath"])
                    if layer_sources["layercliff"]["normalPath"] is not None
                    else None
                ),
                model_path,
            )
            atomic_write_text(
                runtime_directory / "converter.log",
                portable_process_log(cook_result["cookLog"], stage),
            )
            atomic_write_text(
                runtime_directory / "info.log",
                portable_process_log(cook_result["infoLog"], stage),
            )
        placement_source_id = (
            f"{component.logical_package}:landscape:export:{component.export_index}"
        )
        runtime_rows.append(
            {
                "canonicalId": canonical,
                "assetId": asset_id,
                "label": (
                    f"{args.label_prefix} "
                    f"{component.logical_package.rsplit('_', 1)[-1]} "
                    f"Landscape {component.export_index}"
                ),
                "modelPath": (
                    f"Map/{args.pack_name}/Landscape/{asset_id}/{asset_id}.wmodel"
                ),
                "sourcePlacementId": placement_source_id,
                "sourceLevel": component.logical_package,
                "placementId": stable_placement_id(placement_source_id),
                "position": component_world_anchor(component, proxy),
                "evidence": (
                    "UE3 LandscapeComponent exact: "
                    f"{component.logical_package}:export:{component.export_index}"
                ),
                "sectionBase": [component.section_base_x, component.section_base_y],
                "geometry": geometry,
                "bake": {
                    **bake_metadata,
                    "diffuse": relative_posix(diffuse_path, stage),
                    "normal": relative_posix(normal_path, stage),
                    "holeMask": relative_posix(hole_path, stage),
                },
                "runtime": (
                    {
                        key: value
                        for key, value in cook_result.items()
                        if key not in ("cookLog", "infoLog")
                    }
                    if cook_result is not None
                    else None
                ),
            }
        )

    if any(
        int(row["geometry"]["sourceVertexCount"]) != 3969
        for row in runtime_rows
    ):
        raise LandscapeError("unexpected source Landscape vertex count")
    total_hole_quads = sum(
        int(row["geometry"]["holeQuadCount"]) for row in runtime_rows
    )
    expected_triangles = len(components) * 7688 - total_hole_quads * 2
    if total_triangles != expected_triangles:
        raise LandscapeError(
            f"unexpected total Landscape triangle count {total_triangles} "
            f"!= {expected_triangles}"
        )

    map_documents = None
    if not args.no_cook:
        map_documents = write_map_documents(
            stage, args.area_id, runtime_rows, args.group_label
        )

    section_x = sorted({component.section_base_x for component in components})
    section_y = sorted({component.section_base_y for component in components})
    occupancy = {
        (component.section_base_x, component.section_base_y) for component in components
    }
    missing_cells = [
        [x, y] for y in section_y for x in section_x if (x, y) not in occupancy
    ]
    world_bounds = {
        "ue3Centimeters": {
            "min": [
                min(
                    proxies[item.logical_package].location[0]
                    + item.section_base_x
                    * proxies[item.logical_package].draw_scale
                    * proxies[item.logical_package].draw_scale3d[0]
                    for item in components
                ),
                min(
                    proxies[item.logical_package].location[1]
                    + item.section_base_y
                    * proxies[item.logical_package].draw_scale
                    * proxies[item.logical_package].draw_scale3d[1]
                    for item in components
                ),
                min(
                    local_height_at(item, x, y)
                    * proxies[item.logical_package].draw_scale
                    * proxies[item.logical_package].draw_scale3d[2]
                    + proxies[item.logical_package].location[2]
                    for item in components
                    for y in range(item.component_size_quads + 1)
                    for x in range(item.component_size_quads + 1)
                ),
            ],
            "max": [
                max(
                    proxies[item.logical_package].location[0]
                    + (item.section_base_x + item.component_size_quads)
                    * proxies[item.logical_package].draw_scale
                    * proxies[item.logical_package].draw_scale3d[0]
                    for item in components
                ),
                max(
                    proxies[item.logical_package].location[1]
                    + (item.section_base_y + item.component_size_quads)
                    * proxies[item.logical_package].draw_scale
                    * proxies[item.logical_package].draw_scale3d[1]
                    for item in components
                ),
                max(
                    local_height_at(item, x, y)
                    * proxies[item.logical_package].draw_scale
                    * proxies[item.logical_package].draw_scale3d[2]
                    + proxies[item.logical_package].location[2]
                    for item in components
                    for y in range(item.component_size_quads + 1)
                    for x in range(item.component_size_quads + 1)
                ),
            ],
        }
    }

    manifest = {
        "schemaVersion": 1,
        "assetKind": "UE3 Landscape full-data extraction",
        "authoritativeSource": "SourceRaw",
        "displayDerivative": "SourceDerived + Resources",
        "packages": package_rows,
        "landscape": {
            "guid": next(iter(landscape_guids)),
            "componentCount": len(components),
            "collisionComponentCount": sum(
                len(proxy.collision_component_refs) for proxy in proxies.values()
            ),
            "localTexture2DCount": sum(
                int(row["localTexture2DCount"]) for row in package_rows
            ),
            "heightmapCount": len(
                {
                    (component.logical_package, component.heightmap_ref)
                    for component in components
                }
            ),
            "weightmapCount": len(
                {
                    (component.logical_package, reference)
                    for component in components
                    for reference in component.weightmap_refs
                }
            ),
            "localMaterialInstanceCount": sum(
                len(materials) for materials in local_materials_by_package.values()
            ),
            "localMaterialSerialCount": sum(
                len(materials) for materials in local_materials_by_package.values()
            ),
            "layerAllocationCount": sum(
                len(component.allocations) for component in components
            ),
            "grid": {
                "sectionBaseX": section_x,
                "sectionBaseY": section_y,
                "occupiedCellCount": len(occupancy),
                "missingCells": missing_cells,
            },
            "vertexCount": total_vertices,
            "triangleCount": total_triangles,
            "holeQuadCount": total_hole_quads,
            "worldBounds": world_bounds,
        },
        "masterMaterial": {
            "path": master_material_path,
            "logicalPackage": material_logical_package,
            "name": material_name,
            "source": "SourceRaw/MasterMaterial/master_material.json",
            "textureDependencySerials": material_definition[
                "textureDependencySerials"
            ],
        },
        "validation": {
            "componentCountExpected": args.expect_components,
            "componentTextureContractsPassed": len(component_validations),
            "collisionHeightContractsPassed": len(collision_validations),
            "collisionHeightSampleCount": sum(
                int(validation["sampleCount"])
                for validation in collision_validations.values()
            ),
            "collisionHeightMismatchCount": sum(
                int(validation["mismatchCount"])
                for validation in collision_validations.values()
            ),
            "edge": edge_validation,
            "wmodelCount": (
                sum(row["runtime"] is not None for row in runtime_rows)
                if not args.no_cook
                else 0
            ),
        },
        "mapDocuments": map_documents,
        "runtimeAssets": runtime_rows,
        "limitations": [
            "baked diffuse/normal textures are display derivatives, not the UE3 material graph",
            "__DataLayer__ uses the UE3 >170 rule and removes both triangles owned by each hole sample",
            (
                f"the {len(components)}-asset validation catalog remains a "
                "separate source document until an area-specific merge step"
            ),
        ],
    }
    manifest_path = stage / args.manifest_name
    atomic_write_text(manifest_path, json_text(manifest))
    report = {
        "status": "PASS",
        "componentCount": len(components),
        "heightmapCount": manifest["landscape"]["heightmapCount"],
        "weightmapCount": manifest["landscape"]["weightmapCount"],
        "collisionComponentCount": manifest["landscape"][
            "collisionComponentCount"
        ],
        "collisionHeightSampleCount": manifest["validation"][
            "collisionHeightSampleCount"
        ],
        "collisionHeightMismatchCount": manifest["validation"][
            "collisionHeightMismatchCount"
        ],
        "holeQuadCount": manifest["landscape"]["holeQuadCount"],
        "localTexture2DCount": manifest["landscape"]["localTexture2DCount"],
        "localMaterialInstanceCount": manifest["landscape"][
            "localMaterialInstanceCount"
        ],
        "masterTextureDependencySerialCount": material_definition[
            "textureDependencySerials"
        ]["dependencyCount"],
        "layerAllocationCount": manifest["landscape"]["layerAllocationCount"],
        "subsectionSeamMismatchCount": sum(
            validation["subsectionSeamMismatchCount"]
            for validation in component_validations.values()
        ),
        "componentEdgeValidation": edge_validation,
        "missingGridCells": missing_cells,
        "wmodelCount": manifest["validation"]["wmodelCount"],
        "catalog": map_documents,
        "manifestSha256": sha256_file(manifest_path),
    }
    atomic_write_text(report_root / "extraction_report.json", json_text(report))
    return report


def install_pack(
    pack_root: Path,
    client_root: Path,
    area_id: str,
    pack_name: str = "LV_BER_BERNCASTLE_T",
) -> dict[str, Any]:
    resource_source = pack_root / "Resources" / "Map" / pack_name
    resource_destination = (
        client_root / "Bin" / "Resources" / "Map" / pack_name
    )
    catalog_source = pack_root / "DataFiles" / "Map" / f"{area_id}.mapassets"
    placements_source = pack_root / "DataFiles" / "Map" / f"{area_id}.mapplacements"
    data_destination = client_root / "Bin" / "DataFiles" / "Map"
    if not resource_source.is_dir() or not catalog_source.is_file() or not placements_source.is_file():
        raise LandscapeError("pack is incomplete and cannot be installed")
    landscape_destination = resource_destination / "Landscape"
    catalog_destination = data_destination / catalog_source.name
    placements_destination = data_destination / placements_source.name
    for destination in (
        landscape_destination,
        catalog_destination,
        placements_destination,
    ):
        if destination.exists():
            raise LandscapeError(f"install target already exists: {destination}")
    resource_destination.mkdir(parents=True, exist_ok=True)
    data_destination.mkdir(parents=True, exist_ok=True)
    shutil.copytree(resource_source / "Landscape", landscape_destination)
    shutil.copy2(catalog_source, catalog_destination)
    shutil.copy2(placements_source, placements_destination)
    return {
        "resource": str(landscape_destination),
        "catalog": str(catalog_destination),
        "placements": str(placements_destination),
    }


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--converter", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--aes-key", default=UE3.LOSTARK_KR_AES_KEY)
    parser.add_argument("--area-id", default="LV_BER_BERNCASTLE_LANDSCAPE")
    parser.add_argument("--bake-resolution", type=int, default=256)
    parser.add_argument("--hole-mask-tolerance", type=int, default=0)
    parser.add_argument("--pack-name", default="LV_BER_BERNCASTLE_T")
    parser.add_argument("--group-label", default="Bern Castle Landscape")
    parser.add_argument("--label-prefix", default="Bern")
    parser.add_argument("--manifest-name", default="bern_castle_landscape_manifest.json")
    parser.add_argument("--expect-components", type=int, default=42)
    parser.add_argument("--no-cook", action="store_true")
    parser.add_argument("--install-client-root", type=Path)
    parser.add_argument("packages", nargs="+")
    args = parser.parse_args(argv)
    for path, label in (
        (args.umodel, "UModel"),
        (args.converter, "ModelAssetConverter"),
    ):
        if not path.is_file():
            parser.error(f"{label} does not exist: {path}")
    if not args.package_root.is_dir():
        parser.error(f"package root does not exist: {args.package_root}")
    return args


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    output = args.output.resolve()
    if output.exists():
        raise LandscapeError(f"output already exists: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    stage = output.parent / f".landscape-stage-{uuid.uuid4().hex[:8]}"
    stage.mkdir()
    try:
        report = build_landscape_pack(args, stage)
        os.replace(stage, output)
        installed = None
        if args.install_client_root is not None:
            installed = install_pack(
                output, args.install_client_root, args.area_id, args.pack_name
            )
        print(
            json.dumps(
                {
                    "status": "PASS",
                    "output": str(output),
                    "installed": installed,
                    **report,
                },
                ensure_ascii=False,
                indent=2,
            )
        )
        return 0
    except BaseException:
        if (
            stage.is_dir()
            and stage.parent == output.parent
            and stage.name.startswith(".landscape-stage-")
        ):
            shutil.rmtree(stage)
        raise


if __name__ == "__main__":
    raise SystemExit(main())
