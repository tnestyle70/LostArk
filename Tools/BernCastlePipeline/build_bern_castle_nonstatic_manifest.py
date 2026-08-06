#!/usr/bin/env python3
"""Freeze Bern Castle non-static UE3 data into a validated source manifest.

This is a source-evidence builder, not a runtime converter.  It reads only the
core ``LV_BER_BERNCASTLE_T_*`` level packages named by the placement extractor,
keeps exact package/export offsets and hashes, and records every supported
component before a renderer-specific implementation is attempted.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import sys
import tempfile
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Sequence


HERE = Path(__file__).resolve().parent
LEVEL_TOOL_DIR = HERE.parent / "LevelPlacementExtractor"
if str(LEVEL_TOOL_DIR) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOL_DIR))

import extract_ue3_placements as ue3  # noqa: E402


AREA_ID = "LV_BER_BERNCASTLE"
LEVEL_PREFIX = "LV_BER_BERNCASTLE_T_"
SCHEMA_VERSION = 1
WATER_PATTERN = (
    "water",
    "fountain",
    "river",
    "pond",
    "pool",
    "canal",
    "ocean",
)

COMPONENT_TYPES: dict[str, str] = {
    "decalcomponent": "decal",
    "instancedstaticmeshcomponent": "foliage",
    "particlesystemcomponent": "particle",
    "pointlightcomponent": "light",
    "spotlightcomponent": "light",
    "dominantspotlightcomponent": "light",
    "dominantpointlightcomponent": "light",
    "dominantdirectionallightcomponent": "light",
    "directionallightcomponent": "light",
    "skylightcomponent": "light",
    "exponentialheightfogcomponent": "fog",
    "winddirectionalsourcecomponent": "wind",
}

DEFAULT_EXPECTED_COUNTS = {
    "sourcePackages": 22,
    "items": 3592,
    "decal": 86,
    "foliage": 1697,
    "water": 108,
    "particle": 1373,
    "light": 326,
    "fog": 1,
    "wind": 1,
    "decalActors": 86,
    "foliageActors": 14,
    "emitterActors": 1373,
    "uniqueDecalMaterials": 11,
    "uniqueFoliageMeshes": 15,
    "uniqueParticleSystems": 109,
    "uniqueWaterAssets": 15,
}

OBJECT_PROPERTY_TYPES = {
    "objectproperty",
    "componentproperty",
    "interfaceproperty",
}


class ManifestError(RuntimeError):
    pass


@dataclass(frozen=True)
class LevelSource:
    logical_name: str
    physical_path: Path
    placement_path: Path
    placement_document: dict[str, Any]


@dataclass
class PackageContext:
    source: LevelSource
    physical_sha256: str
    physical_size: int
    summary: Any
    logical: bytes
    names: list[str]
    imports: list[Any]
    exports: list[Any]
    property_cache: dict[
        int, tuple[dict[str, Any] | None, int, int, str | None]
    ]


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while True:
            block = stream.read(1024 * 1024)
            if not block:
                break
            digest.update(block)
    return digest.hexdigest().upper()


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ManifestError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ManifestError(f"JSON root must be an object: {path}")
    return value


def stable_id(kind: str, level: str, source_key: str) -> str:
    digest = hashlib.sha1(
        f"{kind.casefold()}|{level.casefold()}|{source_key.casefold()}".encode("utf-8")
    ).hexdigest()[:16].upper()
    return f"BERN_NS_{kind.upper()}_{digest}"


def parse_expected(specifications: Sequence[str] | None) -> dict[str, int]:
    if not specifications:
        return dict(DEFAULT_EXPECTED_COUNTS)
    result: dict[str, int] = {}
    for specification in specifications:
        key, separator, raw_value = specification.partition("=")
        if not separator or not key or key in result:
            raise ManifestError(f"invalid/duplicate expected count: {specification!r}")
        try:
            value = int(raw_value)
        except ValueError as error:
            raise ManifestError(f"expected count is not an integer: {specification!r}") from error
        if value < 0:
            raise ManifestError(f"expected count cannot be negative: {specification!r}")
        result[key] = value
    return result


def discover_level_sources(placement_directories: Sequence[Path]) -> list[LevelSource]:
    by_level: dict[str, LevelSource] = {}
    seen_paths: set[Path] = set()
    for directory in placement_directories:
        if not directory.is_dir():
            raise ManifestError(f"placement directory is missing: {directory}")
        for path in sorted(directory.glob("*.placements.json"), key=lambda p: p.name.casefold()):
            resolved = path.resolve()
            if resolved in seen_paths:
                continue
            seen_paths.add(resolved)
            document = load_json(path)
            source = document.get("source")
            if not isinstance(source, dict):
                raise ManifestError(f"placement source is missing: {path}")
            logical_name = str(source.get("logicalPackage", ""))
            if not logical_name.startswith(LEVEL_PREFIX):
                continue
            physical_text = str(source.get("physicalPackage", ""))
            physical_path = Path(physical_text)
            if not physical_path.is_file():
                raise ManifestError(
                    f"physical package for {logical_name} is missing: {physical_path}"
                )
            if logical_name in by_level:
                raise ManifestError(f"duplicate level source: {logical_name}")
            if document.get("schemaVersion") != 1:
                raise ManifestError(f"unsupported placement schema: {path}")
            by_level[logical_name] = LevelSource(
                logical_name=logical_name,
                physical_path=physical_path,
                placement_path=path,
                placement_document=document,
            )
    if not by_level:
        raise ManifestError("no core Bern Castle level sources were found")
    return [by_level[key] for key in sorted(by_level, key=str.casefold)]


def load_package(source: LevelSource, aes_key: str) -> PackageContext:
    physical = source.physical_path.read_bytes()
    summary = ue3.parse_summary(physical)
    logical = ue3.decompress_package(physical, summary, aes_key)
    names = ue3.parse_name_table(logical, summary)
    imports = ue3.parse_import_table(logical, summary, names)
    exports = ue3.parse_export_table(logical, summary, names)
    declared_exports = source.placement_document["source"].get("exportCount")
    if declared_exports is not None and int(declared_exports) != len(exports):
        raise ManifestError(
            f"export count drift for {source.logical_name}: "
            f"placement={declared_exports}, package={len(exports)}"
        )
    return PackageContext(
        source=source,
        physical_sha256=sha256_bytes(physical),
        physical_size=len(physical),
        summary=summary,
        logical=logical,
        names=names,
        imports=imports,
        exports=exports,
        property_cache={},
    )


def _decode_property(
    property_type: str,
    struct_type: str | None,
    payload: bytes,
    names: list[str],
    bool_value: bool | None,
) -> Any:
    kind = property_type.casefold()
    structure = (struct_type or "").casefold()
    if kind == "structproperty" and structure == "matrix" and len(payload) >= 64:
        values = list(struct.unpack_from("<16f", payload))
        return {
            "rows": [values[offset : offset + 4] for offset in range(0, 16, 4)],
            # Lost Ark's UE3 Matrix payload stores the homogeneous 1.0 first
            # in the final four-float group, followed by X/Y/Z translation.
            "translation": {"x": values[13], "y": values[14], "z": values[15]},
        }
    return ue3.decode_property_value(
        property_type, struct_type, payload, names, bool_value
    )


def _parse_properties_at(
    serial_data: bytes, names: list[str], start_offset: int
) -> tuple[dict[str, Any], int]:
    reader = ue3.Reader(serial_data, start_offset)
    properties: dict[str, Any] = {}
    while reader.offset < len(serial_data):
        property_name, _ = ue3.parse_fname(reader, names)
        if property_name.casefold() == "none":
            return properties, reader.offset
        property_type, _ = ue3.parse_fname(reader, names)
        data_size = reader.i32()
        array_index = reader.i32()
        if data_size < 0 or reader.offset + data_size > len(serial_data) + 32:
            raise ue3.ExtractionError(
                f"invalid property {property_name} size {data_size} at 0x{reader.offset:X}"
            )
        struct_type: str | None = None
        bool_value: bool | None = None
        enum_name: str | None = None
        type_key = property_type.casefold()
        if type_key == "structproperty":
            struct_type, _ = ue3.parse_fname(reader, names)
        elif type_key == "boolproperty":
            bool_value = bool(reader.read(1)[0])
        elif type_key == "byteproperty":
            enum_name, _ = ue3.parse_fname(reader, names)
        serialized_size = data_size + 8 if type_key == "intproperty" else data_size
        payload = reader.read(serialized_size)
        key = property_name if array_index == 0 else f"{property_name}[{array_index}]"
        item: dict[str, Any] = {
            "type": property_type,
            "structType": struct_type,
            "value": _decode_property(
                property_type, struct_type, payload, names, bool_value
            ),
            "rawHex": payload.hex(),
        }
        if enum_name is not None:
            item["enumName"] = enum_name
        if bool_value is not None:
            item["boolValue"] = bool_value
        properties[key] = item
    raise ue3.ExtractionError("tagged property stream has no None terminator")


def parse_properties_lossless(
    serial_data: bytes, names: list[str], package_version: int
) -> tuple[dict[str, Any], int, int]:
    minimum_offset = 4 if package_version >= 322 else 0
    # Dominant-light components put a large native shadow payload before their
    # tagged-property stream (one Bern directional light has a ~3 MiB prefix),
    # so the generic extractor's 256-byte probe window is not sufficient here.
    search_end = len(serial_data) - 20
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
            properties, end_offset = _parse_properties_at(
                serial_data, names, start_offset
            )
            return properties, start_offset, end_offset
        except (ue3.ExtractionError, UnicodeError, struct.error) as error:
            failures.append(f"0x{start_offset:X}: {error}")
    detail = failures[0] if failures else "no plausible first property tag"
    raise ue3.ExtractionError(f"could not locate tagged properties ({detail})")


def properties_for(
    context: PackageContext, entry: Any
) -> tuple[dict[str, Any] | None, int, int, str | None]:
    cached = context.property_cache.get(entry.index)
    if cached is not None:
        return cached
    if entry.serial_size <= 0:
        result = (None, 0, 0, "export has no serial payload")
    else:
        serial = context.logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        try:
            properties, start_offset, end_offset = parse_properties_lossless(
                serial, context.names, context.summary.version
            )
            result = (properties, start_offset, end_offset, None)
        except Exception as error:
            result = (None, 0, 0, f"{type(error).__name__}: {error}")
    context.property_cache[entry.index] = result
    return result


def package_ref_class(context: PackageContext, reference: int) -> str | None:
    if reference < 0:
        index = -reference - 1
        if 0 <= index < len(context.imports):
            return context.imports[index].class_name
    elif reference > 0:
        index = reference - 1
        if 0 <= index < len(context.exports):
            entry = context.exports[index]
            return ue3.package_ref_name(
                entry.class_index, context.imports, context.exports
            )
    return None


def normalize_properties(
    context: PackageContext, properties: dict[str, Any] | None
) -> dict[str, Any] | None:
    if properties is None:
        return None
    normalized: dict[str, Any] = {}
    for name, source_item in properties.items():
        item = dict(source_item)
        kind = str(item.get("type", "")).casefold()
        value = item.get("value")
        if kind in OBJECT_PROPERTY_TYPES and isinstance(value, int):
            item["reference"] = {
                "index": value,
                "objectName": ue3.package_ref_name(
                    value, context.imports, context.exports
                ) if value else None,
                "objectPath": ue3.package_ref_path(
                    value, context.imports, context.exports
                ) if value else None,
                "class": package_ref_class(context, value) if value else None,
            }
        normalized[name] = item
    return normalized


def property_value(properties: dict[str, Any] | None, name: str) -> Any:
    if properties is None:
        return None
    wanted = name.casefold()
    for key, item in properties.items():
        if key.casefold() == wanted:
            return item.get("value")
    return None


def owner_of(context: PackageContext, entry: Any) -> Any | None:
    cursor = entry.package_index
    seen: set[int] = set()
    while cursor > 0 and cursor not in seen:
        seen.add(cursor)
        index = cursor - 1
        if not 0 <= index < len(context.exports):
            return None
        candidate = context.exports[index]
        class_name = ue3.package_ref_name(
            candidate.class_index, context.imports, context.exports
        ).casefold()
        if (
            class_name.endswith("actor")
            or class_name in {
                "emitter",
                "decalactor",
                "instancedfoliageactor",
                "pointlightmovable",
                "dominantspotlight",
                "dominantpointlight",
                "dominantdirectionallight",
                "exponentialheightfog",
                "winddirectionalsource",
            }
        ):
            return candidate
        cursor = candidate.package_index
    return None


def _transform_part(properties: dict[str, Any] | None, component: bool) -> dict[str, Any]:
    if component:
        return {
            "translation": property_value(properties, "Translation"),
            "rotation": property_value(properties, "Rotation"),
            "scale": property_value(properties, "Scale"),
            "scale3D": property_value(properties, "Scale3D"),
            "cachedParentToWorld": property_value(properties, "CachedParentToWorld"),
            "oldPosition": property_value(properties, "OldPosition"),
            "hitLocation": property_value(properties, "HitLocation"),
        }
    return {
        "location": property_value(properties, "Location"),
        "rotation": property_value(properties, "Rotation"),
        "drawScale": property_value(properties, "DrawScale"),
        "drawScale3D": property_value(properties, "DrawScale3D"),
    }


def make_transform(
    owner_properties: dict[str, Any] | None,
    component_properties: dict[str, Any] | None,
    kind: str,
) -> dict[str, Any]:
    owner = _transform_part(owner_properties, component=False)
    component = _transform_part(component_properties, component=True)
    if component["cachedParentToWorld"] is not None:
        status = "resolved-from-cached-parent-to-world"
        resolved = component["cachedParentToWorld"].get("translation")
    elif owner["location"] is not None:
        status = "resolved-from-owner"
        resolved = owner["location"]
    elif component["translation"] is not None:
        status = "resolved-from-component"
        resolved = component["translation"]
    else:
        status = "not-serialized"
        resolved = None
    limitations: list[str] = []
    if kind == "foliage":
        limitations.append(
            "Per-instance transforms remain in the native InstancedStaticMeshComponent tail."
        )
        status = "native-instance-tail-not-decoded"
    return {
        "coordinateSystem": "UE3-native",
        "status": status,
        "resolvedPosition": resolved,
        "owner": owner,
        "component": component,
        "limitations": limitations,
    }


def collect_references(
    properties: dict[str, Any] | None,
    runtime_static_paths: set[str],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    references: list[dict[str, Any]] = []
    missing: list[dict[str, Any]] = []
    if properties is None:
        return references, missing
    for property_name, item in properties.items():
        reference = item.get("reference")
        if not isinstance(reference, dict) or not reference.get("index"):
            continue
        folded = property_name.casefold()
        if "material" in folded:
            role = "material"
        elif "texture" in folded:
            role = "texture"
        elif folded == "staticmesh":
            role = "staticMesh"
        elif folded == "template":
            role = "particleSystem"
        else:
            role = "object"
        row = {"property": property_name, "role": role, **reference}
        path = reference.get("objectPath")
        row["runtimeAvailability"] = (
            "available"
            if role == "staticMesh" and isinstance(path, str) and path.casefold() in runtime_static_paths
            else "missing"
            if role == "staticMesh"
            else "notAssessed"
        )
        references.append(row)
        if not path or str(path).startswith("<bad-"):
            missing.append(
                {
                    "kind": "unresolvedPackageReference",
                    "property": property_name,
                    "index": reference.get("index"),
                }
            )
        elif row["runtimeAvailability"] == "missing":
            missing.append(
                {
                    "kind": "missingRuntimeStaticMesh",
                    "property": property_name,
                    "objectPath": path,
                }
            )
    references.sort(key=lambda row: (row["role"], row["property"].casefold()))
    return references, missing


def export_record(context: PackageContext, entry: Any) -> dict[str, Any]:
    class_name = ue3.package_ref_name(
        entry.class_index, context.imports, context.exports
    )
    return {
        "exportIndex": entry.index,
        "class": class_name,
        "objectName": entry.object_name,
        "objectPath": ue3.package_ref_path(
            entry.index + 1, context.imports, context.exports
        ),
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
    }


def component_item(
    context: PackageContext,
    entry: Any,
    kind: str,
    runtime_static_paths: set[str],
) -> dict[str, Any]:
    (
        component_props_raw,
        property_start,
        property_end,
        property_error,
    ) = properties_for(context, entry)
    component_props = normalize_properties(context, component_props_raw)
    owner = owner_of(context, entry)
    owner_props_raw: dict[str, Any] | None = None
    owner_error: str | None = None
    if owner is not None:
        owner_props_raw, _owner_start, _owner_end, owner_error = properties_for(
            context, owner
        )
    owner_props = normalize_properties(context, owner_props_raw)
    references, missing = collect_references(component_props, runtime_static_paths)
    serial = context.logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
    native_prefix = serial[:property_start] if property_error is None else serial
    native_suffix = serial[property_end:] if property_error is None else b""
    component_path = ue3.package_ref_path(
        entry.index + 1, context.imports, context.exports
    ) or entry.object_name
    item = {
        "id": stable_id(kind, context.source.logical_name, component_path),
        "type": kind,
        "source": {
            "level": context.source.logical_name,
            "packageSha256": context.physical_sha256,
            "component": export_record(context, entry),
            "actor": export_record(context, owner) if owner is not None else None,
        },
        "transform": make_transform(owner_props_raw, component_props_raw, kind),
        "references": references,
        "missingReferences": missing,
        "payload": {
            "schema": "ue3-tagged-properties-v1",
            "parseStatus": "parsed" if property_error is None else "opaque",
            "parseError": property_error,
            "properties": component_props,
            "serial": {
                "offset": entry.serial_offset,
                "size": entry.serial_size,
                "sha256": sha256_bytes(serial),
            },
            "propertyStream": {
                "offsetWithinSerial": property_start,
                "endWithinSerial": property_end,
            },
            "nativePrefix": {
                "offsetWithinSerial": 0,
                "size": len(native_prefix),
                "sha256": sha256_bytes(native_prefix),
                "prefixHex": native_prefix[:64].hex(),
            },
            "nativeSuffix": {
                "offsetWithinSerial": property_end if property_error is None else entry.serial_size,
                "size": len(native_suffix),
                "sha256": sha256_bytes(native_suffix),
                "prefixHex": native_suffix[:64].hex(),
            },
            "ownerPropertyParseError": owner_error,
        },
    }
    if property_error is not None:
        item["missingReferences"].append(
            {"kind": "componentPropertiesOpaque", "error": property_error}
        )
    return item


def load_static_asset_index(path: Path) -> tuple[dict[str, dict[str, Any]], set[str]]:
    document = load_json(path)
    rows = document.get("assets")
    if not isinstance(rows, list):
        raise ManifestError("static asset manifest has no assets array")
    by_path: dict[str, dict[str, Any]] = {}
    ids: set[str] = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ManifestError("invalid static asset row")
        full_path = str(row.get("fullPath", ""))
        asset_id = str(row.get("assetId", ""))
        if not full_path or not asset_id:
            raise ManifestError("static asset row has an empty ID/path")
        folded = full_path.casefold()
        if folded in by_path or asset_id in ids:
            raise ManifestError(f"duplicate static asset ID/path: {asset_id} {full_path}")
        by_path[folded] = row
        ids.add(asset_id)
    return by_path, set(by_path)


def load_source_receipt(source_root: Path | None, asset_id: str) -> dict[str, Any] | None:
    if source_root is None:
        return None
    path = source_root / asset_id / "source.receipt.json"
    if not path.is_file():
        return None
    return load_json(path)


def water_items(
    sources: Sequence[LevelSource],
    assets_by_path: dict[str, dict[str, Any]],
    source_root: Path | None,
) -> list[dict[str, Any]]:
    candidates = {
        path: row
        for path, row in assets_by_path.items()
        if any(token in path for token in WATER_PATTERN)
    }
    items: list[dict[str, Any]] = []
    for source in sources:
        placements = source.placement_document.get("placements")
        if not isinstance(placements, list):
            raise ManifestError(f"placements is not an array: {source.placement_path}")
        for placement in placements:
            if not isinstance(placement, dict):
                raise ManifestError(f"invalid placement row: {source.placement_path}")
            asset_path = str(placement.get("asset", {}).get("objectPath", ""))
            asset = candidates.get(asset_path.casefold())
            if asset is None:
                continue
            placement_id = str(placement.get("placementId", ""))
            if not placement_id:
                raise ManifestError(f"water placement has no placementId: {source.placement_path}")
            receipt = load_source_receipt(source_root, str(asset["assetId"]))
            materials = receipt.get("materials", []) if receipt is not None else []
            texture_refs: list[dict[str, Any]] = []
            if isinstance(materials, list):
                for material in materials:
                    if not isinstance(material, dict):
                        continue
                    roles = material.get("roles", {})
                    if not isinstance(roles, dict):
                        continue
                    for role, texture in roles.items():
                        if isinstance(texture, dict):
                            texture_refs.append(
                                {
                                    "material": material.get("name"),
                                    "role": role,
                                    "parameter": texture.get("parameter"),
                                    "path": texture.get("texture"),
                                    "sha256": texture.get("sha256"),
                                }
                            )
            missing: list[dict[str, Any]] = []
            if receipt is None:
                missing.append(
                    {
                        "kind": "missingSourceReceipt",
                        "assetId": asset["assetId"],
                    }
                )
            items.append(
                {
                    "id": stable_id("water", source.logical_name, placement_id),
                    "type": "water",
                    "source": {
                        "level": source.logical_name,
                        "placementId": placement_id,
                        "placementFile": source.placement_path.name,
                        "actor": placement.get("actor"),
                        "component": placement.get("component"),
                    },
                    "transform": placement.get("transform"),
                    "references": {
                        "asset": {
                            "assetId": asset["assetId"],
                            "objectPath": asset_path,
                        },
                        "materials": materials,
                        "textures": sorted(
                            texture_refs,
                            key=lambda row: (
                                str(row.get("material", "")).casefold(),
                                str(row.get("role", "")).casefold(),
                            ),
                        ),
                    },
                    "missingReferences": missing,
                    "payload": {
                        "schema": "static-placement-v1",
                        "placement": placement,
                        "note": "Candidate remains part of the static scene; this row freezes water-specific authoring evidence.",
                    },
                }
            )
    return items


def _unique_reference_count(items: Sequence[dict[str, Any]], kind: str, role: str) -> int:
    values: set[str] = set()
    for item in items:
        if item.get("type") != kind:
            continue
        refs = item.get("references", [])
        if not isinstance(refs, list):
            continue
        for row in refs:
            if row.get("role") == role and row.get("objectPath"):
                values.add(str(row["objectPath"]).casefold())
    return len(values)


def summarize(
    sources: Sequence[LevelSource],
    items: Sequence[dict[str, Any]],
    actor_counts: Counter[str],
) -> dict[str, Any]:
    type_counts = Counter(str(item.get("type", "")) for item in items)
    unresolved = sum(len(item.get("missingReferences", [])) for item in items)
    water_assets = {
        str(item.get("references", {}).get("asset", {}).get("objectPath", "")).casefold()
        for item in items
        if item.get("type") == "water"
    }
    return {
        "sourcePackages": len(sources),
        "items": len(items),
        **{key: type_counts.get(key, 0) for key in COMPONENT_TYPES.values()},
        "water": type_counts.get("water", 0),
        "decalActors": actor_counts.get("decalactor", 0),
        "foliageActors": actor_counts.get("instancedfoliageactor", 0),
        "emitterActors": actor_counts.get("emitter", 0),
        "uniqueDecalMaterials": _unique_reference_count(items, "decal", "material"),
        "uniqueFoliageMeshes": _unique_reference_count(items, "foliage", "staticMesh"),
        "uniqueParticleSystems": _unique_reference_count(items, "particle", "particleSystem"),
        "uniqueWaterAssets": len(water_assets - {""}),
        "missingReferenceRecords": unresolved,
        "opaquePayloads": sum(
            1
            for item in items
            if item.get("payload", {}).get("parseStatus") == "opaque"
        ),
    }


def validate_manifest(
    manifest: dict[str, Any], expected_counts: dict[str, int]
) -> None:
    if manifest.get("schemaVersion") != SCHEMA_VERSION:
        raise ManifestError("manifest schemaVersion mismatch")
    if manifest.get("areaId") != AREA_ID:
        raise ManifestError("manifest areaId mismatch")
    items = manifest.get("items")
    if not isinstance(items, list):
        raise ManifestError("manifest items must be an array")
    ids: set[str] = set()
    source_keys: set[tuple[str, str]] = set()
    for item in items:
        if not isinstance(item, dict):
            raise ManifestError("manifest item must be an object")
        item_id = str(item.get("id", ""))
        if not item_id or item_id in ids:
            raise ManifestError(f"empty/duplicate manifest item ID: {item_id!r}")
        ids.add(item_id)
        source = item.get("source")
        if not isinstance(source, dict) or not source.get("level"):
            raise ManifestError(f"item has no source level: {item_id}")
        if not str(source["level"]).startswith(LEVEL_PREFIX):
            raise ManifestError(f"non-core source level in item {item_id}: {source['level']}")
        source_locator = (
            str(source.get("component", {}).get("exportIndex"))
            if isinstance(source.get("component"), dict)
            else str(source.get("placementId", ""))
        )
        key = (str(source["level"]).casefold(), source_locator.casefold())
        if key in source_keys:
            raise ManifestError(f"duplicate source locator: {key}")
        source_keys.add(key)
    summary = manifest.get("summary")
    if not isinstance(summary, dict):
        raise ManifestError("manifest summary is missing")
    for key, expected in expected_counts.items():
        actual = summary.get(key)
        if actual != expected:
            raise ManifestError(
                f"expected count gate failed for {key}: expected={expected}, actual={actual}"
            )


def build_manifest(
    placement_directories: Sequence[Path],
    static_asset_manifest: Path,
    source_root: Path | None,
    expected_counts: dict[str, int],
    aes_key: str = ue3.LOSTARK_KR_AES_KEY,
) -> dict[str, Any]:
    # Parse: discover immutable package/placement inputs.
    sources = discover_level_sources(placement_directories)
    assets_by_path, runtime_static_paths = load_static_asset_index(static_asset_manifest)
    items: list[dict[str, Any]] = []
    package_rows: list[dict[str, Any]] = []
    actor_counts: Counter[str] = Counter()

    # Stage: decode every source into a new in-memory document.  No output is
    # touched until all packages and water candidates have passed validation.
    for source in sources:
        context = load_package(source, aes_key)
        class_counts = Counter(
            ue3.package_ref_name(
                entry.class_index, context.imports, context.exports
            ).casefold()
            for entry in context.exports
        )
        actor_counts.update(class_counts)
        package_rows.append(
            {
                "logicalName": source.logical_name,
                "physicalPath": str(source.physical_path),
                "physicalSize": context.physical_size,
                "physicalSha256": context.physical_sha256,
                "placementFile": str(source.placement_path),
                "placementSha256": sha256_file(source.placement_path),
                "packageVersion": context.summary.version,
                "licenseeVersion": context.summary.licensee_version,
                "engineVersion": context.summary.engine_version,
                "exportCount": len(context.exports),
            }
        )
        for entry in context.exports:
            class_name = ue3.package_ref_name(
                entry.class_index, context.imports, context.exports
            ).casefold()
            kind = COMPONENT_TYPES.get(class_name)
            if kind is None:
                continue
            items.append(component_item(context, entry, kind, runtime_static_paths))

    items.extend(water_items(sources, assets_by_path, source_root))
    items.sort(key=lambda item: (item["type"], item["source"]["level"], item["id"]))
    manifest = {
        "schemaVersion": SCHEMA_VERSION,
        "areaId": AREA_ID,
        "purpose": "authoritative non-static source evidence; not a runtime-ready asset",
        "sources": package_rows,
        "summary": summarize(sources, items, actor_counts),
        "items": items,
    }

    # Validate: count gates and ID/source uniqueness are checked before commit.
    validate_manifest(manifest, expected_counts)
    return manifest


def atomic_commit_json(
    output: Path, manifest: dict[str, Any], expected_counts: dict[str, int]
) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    serialized = json.dumps(manifest, ensure_ascii=False, indent=2) + "\n"
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{output.name}.", suffix=".staging", dir=str(output.parent)
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(serialized)
            stream.flush()
            os.fsync(stream.fileno())
        staged = load_json(temporary)
        validate_manifest(staged, expected_counts)
        os.replace(temporary, output)
    finally:
        if temporary.exists():
            temporary.unlink()


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--placements",
        action="append",
        required=True,
        type=Path,
        help="Placement directory; repeat for split extraction roots.",
    )
    parser.add_argument("--static-assets", required=True, type=Path)
    parser.add_argument(
        "--source-root",
        type=Path,
        help="Optional exact static source-pack root used for water material/texture receipts.",
    )
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument(
        "--expected",
        action="append",
        help="Override all default gates with repeatable key=count pairs.",
    )
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--summary", action="store_true")
    parser.add_argument("--aes-key", default=ue3.LOSTARK_KR_AES_KEY)
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        expected = parse_expected(args.expected)
        manifest = build_manifest(
            placement_directories=args.placements,
            static_asset_manifest=args.static_assets,
            source_root=args.source_root,
            expected_counts=expected,
            aes_key=args.aes_key,
        )
        if not args.dry_run:
            atomic_commit_json(args.output, manifest, expected)
        if args.summary:
            print(json.dumps(manifest["summary"], ensure_ascii=False, indent=2))
        else:
            action = "validated" if args.dry_run else "committed"
            print(
                f"{action}: {args.output} | items={manifest['summary']['items']} "
                f"packages={manifest['summary']['sourcePackages']}"
            )
        return 0
    except (ManifestError, ue3.ExtractionError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
