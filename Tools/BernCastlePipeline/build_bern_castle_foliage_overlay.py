#!/usr/bin/env python3
"""Decode Bern Castle UE3 foliage instances into a MapTool overlay manifest.

The source of truth is the original level UPK referenced by the non-static
manifest.  Component transforms are not substituted for instance transforms:
the script reopens every InstancedStaticMeshComponent serial, verifies its
recorded hashes, decodes the native FInstancedStaticMeshInstanceData array, and
converts each matrix to the existing Client coordinate contract.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import struct
import sys
import tempfile
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, Iterable, Sequence


SCRIPT_DIR = Path(__file__).resolve().parent
LEVEL_TOOL_DIR = SCRIPT_DIR.parent / "LevelPlacementExtractor"
if str(LEVEL_TOOL_DIR) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOL_DIR))

import build_maptool_scene as scene  # noqa: E402
import extract_ue3_placements as ue3  # noqa: E402


AREA_ID = "LV_BER_BERNCASTLE"
EXPECTED_COMPONENTS = 1697
EXPECTED_INSTANCES = 17651
EXPECTED_UNIQUE_MESHES = 15
EXPECTED_BASE_MESHES = 4
EXPECTED_SUPPLEMENT_MESHES = 11
EXPECTED_BASE_COMPONENTS = 290
EXPECTED_BASE_INSTANCES = 938
EXPECTED_SUPPLEMENT_COMPONENTS = 1407
EXPECTED_SUPPLEMENT_INSTANCES = 16713
EXPECTED_ANY_NEGATIVE = 0
EXPECTED_REFLECTED = 0
EDITOR_ID_MASK = (1 << 63) - 1
MATRIX_EPSILON = 1.0e-5
ORTHOGONAL_EPSILON = 1.0e-4
INSTANCE_ELEMENT_SIZE = 80
NATIVE_FIXED_TAIL_SIZE = 69

DEFAULT_NONSTATIC = Path(
    r"C:\LostArkExtract\bern_full\manifests\bern_castle_nonstatic.json"
)
DEFAULT_BASE_STATIC = Path(
    r"C:\LostArkExtract\bern_full\manifests\bern_castle_assets.json"
)
DEFAULT_BASE_RUNTIME = Path(
    r"C:\LostArkExtract\bern_full\manifests\bern_castle_runtime_assets.json"
)
DEFAULT_SUPPLEMENT_RUNTIME = Path(
    r"C:\LostArkExtract\bern_full\foliage\manifests"
    r"\bern_castle_foliage_supplement_runtime_assets.json"
)
DEFAULT_OUTPUT = Path(
    r"C:\LostArkExtract\bern_full\manifests\bern_castle_foliage_overlay.json"
)


class FoliageOverlayError(RuntimeError):
    pass


@dataclass(frozen=True)
class ExportView:
    index: int
    class_name: str
    object_name: str
    object_path: str
    serial_offset: int
    serial_size: int


@dataclass(frozen=True)
class PackageView:
    logical_name: str
    physical_path: str
    physical_sha256: str
    package_version: int
    export_count: int
    logical: bytes
    exports: tuple[ExportView, ...]


@dataclass(frozen=True)
class DecodedInstance:
    matrix_rows: tuple[tuple[float, float, float, float], ...]
    lightmap_uv_bias: tuple[float, float]
    shadowmap_uv_bias: tuple[float, float]
    position: tuple[float, float, float]
    quaternion: tuple[float, float, float, float]
    signed_scale: tuple[float, float, float]


@dataclass(frozen=True)
class DecodedNativeSuffix:
    version: int
    instance_count: int
    shadow_map_references: tuple[int, ...]
    light_guid_count: int
    light_guid_sha256: str
    fixed_tail_sha256: str
    element_size: int
    instances: tuple[DecodedInstance, ...]


@dataclass(frozen=True)
class Expectations:
    components: int = EXPECTED_COMPONENTS
    instances: int = EXPECTED_INSTANCES
    unique_meshes: int = EXPECTED_UNIQUE_MESHES
    base_meshes: int = EXPECTED_BASE_MESHES
    supplement_meshes: int = EXPECTED_SUPPLEMENT_MESHES
    base_components: int = EXPECTED_BASE_COMPONENTS
    base_instances: int = EXPECTED_BASE_INSTANCES
    supplement_components: int = EXPECTED_SUPPLEMENT_COMPONENTS
    supplement_instances: int = EXPECTED_SUPPLEMENT_INSTANCES
    any_negative: int = EXPECTED_ANY_NEGATIVE
    reflected: int = EXPECTED_REFLECTED


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise FoliageOverlayError(f"JSON root is not an object: {path}")
    return value


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def normalized_object_path(value: Any, label: str) -> str:
    result = str(value).strip().casefold()
    if not result or "." not in result:
        raise FoliageOverlayError(f"invalid {label}: {value!r}")
    return result


def safe_relative_model(value: Any, label: str) -> Path:
    path = Path(str(value).replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts or path.suffix.casefold() != ".wmodel":
        raise FoliageOverlayError(f"invalid {label}: {value!r}")
    return path


def stable_placement_id(source_id: str) -> int:
    value = int.from_bytes(hashlib.sha256(source_id.encode("utf-8")).digest()[:8], "big")
    value &= EDITOR_ID_MASK
    if value == 0:
        raise FoliageOverlayError(f"stable placement ID resolved to zero: {source_id}")
    return value


def load_original_package(source: dict[str, Any]) -> PackageView:
    logical_name = str(source.get("logicalName", ""))
    if not logical_name.startswith("LV_BER_BERNCASTLE_T_"):
        raise FoliageOverlayError(f"invalid Bern source logical package: {logical_name!r}")
    path = Path(str(source.get("physicalPath", "")))
    if not path.is_file():
        raise FoliageOverlayError(f"source UPK is missing: {path}")
    physical = path.read_bytes()
    expected_size = int(source.get("physicalSize", len(physical)))
    if len(physical) != expected_size:
        raise FoliageOverlayError(
            f"source UPK size mismatch: {logical_name} {len(physical)} != {expected_size}"
        )
    digest = sha256_bytes(physical)
    if digest != str(source.get("physicalSha256", "")).upper():
        raise FoliageOverlayError(f"source UPK hash mismatch: {logical_name}")

    summary = ue3.parse_summary(physical)
    if summary.version != int(source.get("packageVersion", summary.version)):
        raise FoliageOverlayError(f"package version mismatch: {logical_name}")
    if summary.export_count != int(source.get("exportCount", summary.export_count)):
        raise FoliageOverlayError(f"package export count mismatch: {logical_name}")
    logical = ue3.decompress_package(physical, summary, ue3.LOSTARK_KR_AES_KEY)
    names = ue3.parse_name_table(logical, summary)
    imports = ue3.parse_import_table(logical, summary, names)
    exports = ue3.parse_export_table(logical, summary, names)
    views = tuple(
        ExportView(
            index=entry.index,
            class_name=ue3.package_ref_name(
                entry.class_index, imports, exports
            ).casefold(),
            object_name=entry.object_name,
            object_path=(
                ue3.package_ref_path(entry.index + 1, imports, exports)
                or entry.object_name
            ).casefold(),
            serial_offset=entry.serial_offset,
            serial_size=entry.serial_size,
        )
        for entry in exports
    )
    return PackageView(
        logical_name=logical_name,
        physical_path=str(path.resolve()),
        physical_sha256=digest,
        package_version=summary.version,
        export_count=len(views),
        logical=logical,
        exports=views,
    )


def _determinant3(rows: Sequence[Sequence[float]]) -> float:
    return (
        rows[0][0] * (rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1])
        - rows[0][1] * (rows[1][0] * rows[2][2] - rows[1][2] * rows[2][0])
        + rows[0][2] * (rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0])
    )


def decode_instance_matrix(values: Sequence[float], label: str) -> DecodedInstance:
    if len(values) != 20 or not all(math.isfinite(value) for value in values):
        raise FoliageOverlayError(f"non-finite or malformed instance data: {label}")
    rows = tuple(
        tuple(float(value) for value in values[offset : offset + 4])
        for offset in range(0, 16, 4)
    )
    if (
        max(abs(rows[row][3]) for row in range(3)) > MATRIX_EPSILON
        or abs(rows[3][3] - 1.0) > MATRIX_EPSILON
    ):
        raise FoliageOverlayError(f"invalid homogeneous instance matrix: {label}")

    unsigned_scale = [
        math.sqrt(sum(rows[row][column] ** 2 for column in range(3)))
        for row in range(3)
    ]
    if any(value < MATRIX_EPSILON for value in unsigned_scale):
        raise FoliageOverlayError(f"zero instance scale axis: {label}")
    rotation = [
        [rows[row][column] / unsigned_scale[row] for column in range(3)]
        for row in range(3)
    ]
    determinant = _determinant3(rotation)
    signed_scale = list(unsigned_scale)
    if determinant < 0.0:
        # A reflected matrix has no proper rotation quaternion.  Use a stable
        # X-axis sign convention and keep the reflection in signedScale.
        signed_scale[0] = -signed_scale[0]
        rotation[0] = [-value for value in rotation[0]]
        determinant = -determinant
    if abs(determinant - 1.0) > ORTHOGONAL_EPSILON:
        raise FoliageOverlayError(
            f"instance rotation determinant is not one ({determinant}): {label}"
        )
    orthogonal_error = max(
        abs(
            sum(rotation[left][axis] * rotation[right][axis] for axis in range(3))
            - (1.0 if left == right else 0.0)
        )
        for left in range(3)
        for right in range(3)
    )
    if orthogonal_error > ORTHOGONAL_EPSILON:
        raise FoliageOverlayError(
            f"instance rotation is not orthogonal ({orthogonal_error}): {label}"
        )

    client_rotation = scene.mat_mul(
        scene.mat_mul(scene.mat_transpose(scene.BASIS), rotation), scene.BASIS
    )
    quaternion = scene.standard_column_quaternion(
        scene.mat_transpose(client_rotation)
    )
    round_trip = scene.directx_row_matrix_from_quaternion(quaternion)
    round_trip_error = max(
        abs(round_trip[row][column] - client_rotation[row][column])
        for row in range(3)
        for column in range(3)
    )
    if round_trip_error > ORTHOGONAL_EPSILON:
        raise FoliageOverlayError(
            f"instance quaternion round trip failed ({round_trip_error}): {label}"
        )

    position = (rows[3][0] * 0.01, rows[3][2] * 0.01, -rows[3][1] * 0.01)
    client_scale = (signed_scale[0], signed_scale[2], signed_scale[1])
    return DecodedInstance(
        matrix_rows=rows,
        lightmap_uv_bias=(float(values[16]), float(values[17])),
        shadowmap_uv_bias=(float(values[18]), float(values[19])),
        position=position,
        quaternion=tuple(float(value) for value in quaternion),
        signed_scale=tuple(float(value) for value in client_scale),
    )


def decode_native_suffix(
    suffix: bytes,
    resolve_reference: Callable[[int], ExportView],
    label: str,
) -> DecodedNativeSuffix:
    cursor = 0

    def read(fmt: str) -> tuple[Any, ...]:
        nonlocal cursor
        size = struct.calcsize(fmt)
        if cursor + size > len(suffix):
            raise FoliageOverlayError(
                f"native suffix truncated at {cursor}/{len(suffix)}: {label}"
            )
        result = struct.unpack_from(fmt, suffix, cursor)
        cursor += size
        return result

    version, instance_count = read("<II")
    if version != 1:
        raise FoliageOverlayError(f"unsupported native suffix version {version}: {label}")
    if not 1 <= instance_count <= 1_000_000:
        raise FoliageOverlayError(f"invalid native instance count {instance_count}: {label}")

    shadow_references = tuple(read("<i")[0] for _ in range(instance_count))
    for reference in shadow_references:
        if reference <= 0:
            raise FoliageOverlayError(
                f"native shadow map reference is not an export: {reference} {label}"
            )
        resolved = resolve_reference(reference)
        if resolved.class_name != "shadowmap2d":
            raise FoliageOverlayError(
                f"native reference {reference} is {resolved.class_name}, not ShadowMap2D: {label}"
            )

    zero_field, layout_version, light_guid_count = read("<iiI")
    if zero_field != 0 or layout_version != 2:
        raise FoliageOverlayError(
            f"native suffix fixed header mismatch ({zero_field}, {layout_version}): {label}"
        )
    if not 1 <= light_guid_count <= 5:
        raise FoliageOverlayError(
            f"native light GUID count outside Bern contract: {light_guid_count} {label}"
        )
    guid_size = light_guid_count * 16
    if cursor + guid_size + NATIVE_FIXED_TAIL_SIZE + 8 > len(suffix):
        raise FoliageOverlayError(f"native optional GUID blocks are truncated: {label}")
    guid_bytes = suffix[cursor : cursor + guid_size]
    cursor += guid_size
    fixed_tail = suffix[cursor : cursor + NATIVE_FIXED_TAIL_SIZE]
    cursor += NATIVE_FIXED_TAIL_SIZE

    # These fields are opaque renderer/light-cache data, but their float lanes
    # are known and must remain finite before the cursor advances to BulkData.
    finite_offsets = (4, 8, 12, 20, 24, 28, 36, 40, 44, 48, 52)
    for offset in finite_offsets:
        value = struct.unpack_from("<f", fixed_tail, offset)[0]
        if not math.isfinite(value):
            raise FoliageOverlayError(f"non-finite native fixed-tail float: {label}")

    element_size, repeated_count = read("<II")
    if element_size != INSTANCE_ELEMENT_SIZE:
        raise FoliageOverlayError(
            f"native instance element size {element_size} != {INSTANCE_ELEMENT_SIZE}: {label}"
        )
    if repeated_count != instance_count:
        raise FoliageOverlayError(
            f"native instance count mismatch {repeated_count} != {instance_count}: {label}"
        )
    expected_end = cursor + instance_count * INSTANCE_ELEMENT_SIZE
    if expected_end != len(suffix):
        raise FoliageOverlayError(
            f"native suffix end mismatch {expected_end} != {len(suffix)}: {label}"
        )

    instances = []
    for index in range(instance_count):
        values = struct.unpack_from("<20f", suffix, cursor)
        cursor += INSTANCE_ELEMENT_SIZE
        instances.append(decode_instance_matrix(values, f"{label}:instance:{index}"))
    if cursor != len(suffix):
        raise FoliageOverlayError(f"native suffix was not consumed exactly: {label}")
    return DecodedNativeSuffix(
        version=version,
        instance_count=instance_count,
        shadow_map_references=shadow_references,
        light_guid_count=light_guid_count,
        light_guid_sha256=sha256_bytes(guid_bytes),
        fixed_tail_sha256=sha256_bytes(fixed_tail),
        element_size=element_size,
        instances=tuple(instances),
    )


def _manifest_assets(document: dict[str, Any], label: str) -> list[dict[str, Any]]:
    if document.get("schemaVersion") != 1 or document.get("areaId") != AREA_ID:
        raise FoliageOverlayError(f"{label} schema/areaId mismatch")
    assets = document.get("assets")
    if not isinstance(assets, list) or document.get("assetCount") != len(assets):
        raise FoliageOverlayError(f"{label} asset count mismatch")
    ids = [str(asset.get("assetId", "")) for asset in assets]
    paths = [normalized_object_path(asset.get("fullPath", ""), label) for asset in assets]
    if "" in ids or len(set(ids)) != len(ids) or len(set(paths)) != len(paths):
        raise FoliageOverlayError(f"{label} has empty/duplicate asset identity")
    return assets


def _static_mesh_reference(item: dict[str, Any]) -> dict[str, Any]:
    references = item.get("references")
    if not isinstance(references, list):
        raise FoliageOverlayError(f"foliage references are not an array: {item.get('id')}")
    rows = [
        row
        for row in references
        if isinstance(row, dict) and row.get("role") == "staticMesh"
    ]
    if len(rows) != 1 or str(rows[0].get("class", "")).casefold() != "staticmesh":
        raise FoliageOverlayError(
            f"foliage component does not have one exact StaticMesh: {item.get('id')}"
        )
    normalized_object_path(rows[0].get("objectPath"), "foliage mesh path")
    return rows[0]


def _hash_source(path: Path | None) -> dict[str, Any] | None:
    if path is None:
        return None
    return {"path": str(path.resolve()), "sha256": sha256_file(path)}


def build_overlay(
    nonstatic: dict[str, Any],
    base_static: dict[str, Any],
    base_runtime: dict[str, Any],
    supplement_runtime: dict[str, Any],
    *,
    expectations: Expectations = Expectations(),
    package_loader: Callable[[dict[str, Any]], PackageView] = load_original_package,
    supplement_model_prefix: str = "Map/LV_BER_BERNCASTLE",
    source_paths: dict[str, Path | None] | None = None,
) -> dict[str, Any]:
    if nonstatic.get("schemaVersion") != 1 or nonstatic.get("areaId") != AREA_ID:
        raise FoliageOverlayError("non-static manifest schema/areaId mismatch")
    summary = nonstatic.get("summary")
    if not isinstance(summary, dict) or int(summary.get("foliage", -1)) != expectations.components:
        raise FoliageOverlayError("non-static foliage summary count mismatch")
    sources = nonstatic.get("sources")
    if not isinstance(sources, list):
        raise FoliageOverlayError("non-static manifest has no source packages")
    source_by_level = {str(source.get("logicalName", "")): source for source in sources}
    if "" in source_by_level or len(source_by_level) != len(sources):
        raise FoliageOverlayError("duplicate/empty non-static source package")

    base_assets = _manifest_assets(base_static, "base static manifest")
    base_runtime_assets = _manifest_assets(base_runtime, "base runtime manifest")
    base_ids = {str(asset["assetId"]) for asset in base_assets}
    if base_ids != {str(asset["assetId"]) for asset in base_runtime_assets}:
        raise FoliageOverlayError("base static/runtime asset sets differ")
    base_by_path = {
        normalized_object_path(asset["fullPath"], "base static path"): asset
        for asset in base_assets
    }

    supplement_assets = _manifest_assets(
        supplement_runtime, "foliage supplement runtime manifest"
    )
    if supplement_runtime.get("kind") != "foliage-staticmesh-supplement-runtime":
        raise FoliageOverlayError("supplement runtime kind mismatch")
    supplement_by_path = {
        normalized_object_path(asset["fullPath"], "supplement path"): asset
        for asset in supplement_assets
    }
    supplement_ids = {str(asset["assetId"]) for asset in supplement_assets}
    if set(base_by_path) & set(supplement_by_path) or base_ids & supplement_ids:
        raise FoliageOverlayError("base and supplement asset identities overlap")
    if len(supplement_assets) != expectations.supplement_meshes:
        raise FoliageOverlayError("supplement unique mesh count gate failed")
    for asset in supplement_assets:
        safe_relative_model(asset.get("model"), "supplement runtime model")
        if str(asset.get("wmodelMagic", "")) not in ("WINT", "WMOD"):
            raise FoliageOverlayError(
                f"invalid supplement WModel magic: {asset.get('assetId')}"
            )

    foliage_items = [item for item in nonstatic.get("items", []) if item.get("type") == "foliage"]
    if len(foliage_items) != expectations.components:
        raise FoliageOverlayError("foliage component count gate failed")
    item_ids = [str(item.get("id", "")) for item in foliage_items]
    if "" in item_ids or len(set(item_ids)) != len(item_ids):
        raise FoliageOverlayError("duplicate/empty foliage item ID")

    package_cache: dict[str, PackageView] = {}
    seen_component_locators: set[tuple[str, int]] = set()
    seen_source_ids: set[str] = set()
    seen_placement_ids: dict[int, str] = {}
    components: list[dict[str, Any]] = []
    placements: list[dict[str, Any]] = []
    component_counts: Counter[str] = Counter()
    instance_counts: Counter[str] = Counter()
    level_counts: Counter[str] = Counter()
    guid_counts: Counter[int] = Counter()
    base_component_count = base_instance_count = 0
    supplement_component_count = supplement_instance_count = 0
    any_negative_count = reflected_count = 0

    for item in sorted(
        foliage_items,
        key=lambda row: (
            str(row.get("source", {}).get("level", "")).casefold(),
            int(row.get("source", {}).get("component", {}).get("exportIndex", -1)),
            str(row.get("id", "")),
        ),
    ):
        item_id = str(item["id"])
        source_record = item.get("source")
        if not isinstance(source_record, dict):
            raise FoliageOverlayError(f"foliage source is missing: {item_id}")
        level = str(source_record.get("level", ""))
        source = source_by_level.get(level)
        if source is None:
            raise FoliageOverlayError(f"foliage source package join failed: {item_id}")
        package = package_cache.get(level)
        if package is None:
            package = package_loader(source)
            if package.logical_name != level:
                raise FoliageOverlayError(f"package loader identity mismatch: {level}")
            package_cache[level] = package
        if str(source_record.get("packageSha256", "")).upper() != package.physical_sha256:
            raise FoliageOverlayError(f"component package hash mismatch: {item_id}")

        component_record = source_record.get("component")
        if not isinstance(component_record, dict):
            raise FoliageOverlayError(f"component source record is missing: {item_id}")
        export_index = int(component_record.get("exportIndex", -1))
        locator = (level, export_index)
        if locator in seen_component_locators:
            raise FoliageOverlayError(f"duplicate foliage component locator: {locator}")
        seen_component_locators.add(locator)
        if not 0 <= export_index < len(package.exports):
            raise FoliageOverlayError(f"component export is outside table: {item_id}")
        component = package.exports[export_index]
        if component.class_name != "instancedstaticmeshcomponent":
            raise FoliageOverlayError(
                f"component export class is {component.class_name}: {item_id}"
            )
        if (
            component.object_name.casefold()
            != str(component_record.get("objectName", "")).casefold()
            or component.object_path
            != str(component_record.get("objectPath", "")).casefold()
            or component.serial_offset != int(component_record.get("serialOffset", -1))
            or component.serial_size != int(component_record.get("serialSize", -1))
        ):
            raise FoliageOverlayError(f"component export identity drift: {item_id}")
        serial_end = component.serial_offset + component.serial_size
        if component.serial_offset < 0 or serial_end > len(package.logical):
            raise FoliageOverlayError(f"component serial range is invalid: {item_id}")
        serial = package.logical[component.serial_offset:serial_end]
        payload = item.get("payload")
        if not isinstance(payload, dict) or payload.get("parseStatus") != "parsed":
            raise FoliageOverlayError(f"component payload is not parsed: {item_id}")
        serial_meta = payload.get("serial", {})
        if (
            int(serial_meta.get("offset", -1)) != component.serial_offset
            or int(serial_meta.get("size", -1)) != component.serial_size
            or str(serial_meta.get("sha256", "")).upper() != sha256_bytes(serial)
        ):
            raise FoliageOverlayError(f"component serial evidence mismatch: {item_id}")
        property_end = int(payload.get("propertyStream", {}).get("endWithinSerial", -1))
        if not 0 <= property_end <= len(serial):
            raise FoliageOverlayError(f"component property end is invalid: {item_id}")
        suffix = serial[property_end:]
        suffix_meta = payload.get("nativeSuffix", {})
        if (
            int(suffix_meta.get("offsetWithinSerial", -1)) != property_end
            or int(suffix_meta.get("size", -1)) != len(suffix)
            or str(suffix_meta.get("sha256", "")).upper() != sha256_bytes(suffix)
            or not suffix.startswith(bytes.fromhex(str(suffix_meta.get("prefixHex", ""))))
        ):
            raise FoliageOverlayError(f"component native suffix evidence mismatch: {item_id}")

        reference = _static_mesh_reference(item)
        mesh_path = normalized_object_path(reference["objectPath"], "foliage mesh")
        base_asset = base_by_path.get(mesh_path)
        supplement_asset = supplement_by_path.get(mesh_path)
        if (base_asset is None) == (supplement_asset is None):
            raise FoliageOverlayError(f"foliage mesh join is not exact: {item_id} {mesh_path}")
        asset = base_asset if base_asset is not None else supplement_asset
        assert asset is not None
        asset_id = str(asset["assetId"])

        def resolve_shadow_map(reference_index: int) -> ExportView:
            export = reference_index - 1
            if not 0 <= export < len(package.exports):
                raise FoliageOverlayError(
                    f"shadow map reference outside export table: {reference_index} {item_id}"
                )
            return package.exports[export]

        decoded = decode_native_suffix(suffix, resolve_shadow_map, item_id)
        guid_counts[decoded.light_guid_count] += 1
        component_counts[asset_id] += 1
        instance_counts[asset_id] += decoded.instance_count
        level_counts[level] += decoded.instance_count
        if base_asset is not None:
            base_component_count += 1
            base_instance_count += decoded.instance_count
        else:
            supplement_component_count += 1
            supplement_instance_count += decoded.instance_count

        shadow_evidence = []
        for reference_index in decoded.shadow_map_references:
            shadow = resolve_shadow_map(reference_index)
            shadow_evidence.append(
                {
                    "packageIndex": reference_index,
                    "exportIndex": shadow.index,
                    "objectName": shadow.object_name,
                    "objectPath": shadow.object_path,
                    "class": shadow.class_name,
                }
            )
        components.append(
            {
                "itemId": item_id,
                "sourceLevel": level,
                "sourcePackageSha256": package.physical_sha256,
                "component": {
                    "exportIndex": component.index,
                    "objectName": component.object_name,
                    "objectPath": component.object_path,
                    "serialOffset": component.serial_offset,
                    "serialSize": component.serial_size,
                    "serialSha256": sha256_bytes(serial),
                },
                "mesh": {
                    "assetId": asset_id,
                    "objectPath": mesh_path,
                    "catalogSource": "base" if base_asset is not None else "supplement",
                },
                "nativeSuffix": {
                    "offsetWithinSerial": property_end,
                    "size": len(suffix),
                    "sha256": sha256_bytes(suffix),
                    "version": decoded.version,
                    "instanceCount": decoded.instance_count,
                    "shadowMapReferences": shadow_evidence,
                    "lightGuidCount": decoded.light_guid_count,
                    "lightGuidSha256": decoded.light_guid_sha256,
                    "fixedTailSha256": decoded.fixed_tail_sha256,
                    "elementSize": decoded.element_size,
                },
            }
        )

        for instance_index, instance in enumerate(decoded.instances):
            source_id = (
                f"{level}:foliage:export:{component.index}:instance:{instance_index}"
            )
            if source_id in seen_source_ids:
                raise FoliageOverlayError(f"duplicate foliage source placement ID: {source_id}")
            seen_source_ids.add(source_id)
            placement_id = stable_placement_id(source_id)
            previous = seen_placement_ids.get(placement_id)
            if previous is not None:
                raise FoliageOverlayError(
                    f"foliage placement ID collision: {previous!r} / {source_id!r}"
                )
            seen_placement_ids[placement_id] = source_id
            any_negative = any(value < 0.0 for value in instance.signed_scale)
            reflected = math.prod(instance.signed_scale) < 0.0
            any_negative_count += int(any_negative)
            reflected_count += int(reflected)
            placements.append(
                {
                    "placementId": placement_id,
                    "sourcePlacementId": source_id,
                    "sourceLevel": level,
                    "transformSource": "overlay",
                    "assetId": asset_id,
                    "position": list(instance.position),
                    "quaternion": list(instance.quaternion),
                    "scale": list(instance.signed_scale),
                    "visible": True,
                    "evidence": {
                        "kind": "UE3-FInstancedStaticMeshInstanceData",
                        "sourceComponentId": item_id,
                        "nativeInstanceIndex": instance_index,
                        "shadowMapReference": shadow_evidence[instance_index],
                        "ue3MatrixRows": [list(row) for row in instance.matrix_rows],
                        "lightmapUVBias": list(instance.lightmap_uv_bias),
                        "shadowmapUVBias": list(instance.shadowmap_uv_bias),
                    },
                }
            )

    total_instances = len(placements)
    used_base_ids = {asset_id for asset_id in component_counts if asset_id in base_ids}
    used_supplement_ids = {
        asset_id for asset_id in component_counts if asset_id in supplement_ids
    }
    gates = {
        "components": (len(components), expectations.components),
        "instances": (total_instances, expectations.instances),
        "uniqueMeshes": (len(component_counts), expectations.unique_meshes),
        "baseMeshes": (len(used_base_ids), expectations.base_meshes),
        "supplementMeshes": (
            len(used_supplement_ids), expectations.supplement_meshes
        ),
        "baseComponents": (base_component_count, expectations.base_components),
        "baseInstances": (base_instance_count, expectations.base_instances),
        "supplementComponents": (
            supplement_component_count, expectations.supplement_components
        ),
        "supplementInstances": (
            supplement_instance_count, expectations.supplement_instances
        ),
        "anyNegative": (any_negative_count, expectations.any_negative),
        "reflected": (reflected_count, expectations.reflected),
    }
    failures = [f"{name}={actual} expected={expected}" for name, (actual, expected) in gates.items() if actual != expected]
    if failures:
        raise FoliageOverlayError("foliage count gate failed: " + ", ".join(failures))
    if int(supplement_runtime.get("usageCount", -1)) != supplement_component_count:
        raise FoliageOverlayError("supplement runtime usageCount/component count mismatch")
    supplement_by_id = {str(asset["assetId"]): asset for asset in supplement_assets}
    for asset_id in used_supplement_ids:
        if int(supplement_by_id[asset_id].get("usageCount", -1)) != component_counts[asset_id]:
            raise FoliageOverlayError(f"supplement per-asset usageCount mismatch: {asset_id}")

    prefix = Path(supplement_model_prefix.replace("\\", "/"))
    if prefix.is_absolute() or ".." in prefix.parts:
        raise FoliageOverlayError("supplement model prefix is unsafe")
    overlay_assets = []
    for asset in sorted(supplement_assets, key=lambda row: str(row["assetId"])):
        asset_id = str(asset["assetId"])
        relative_model = safe_relative_model(asset["model"], "supplement model")
        overlay_assets.append(
            {
                "assetId": asset_id,
                "label": str(asset["fullPath"]).split(".")[-1],
                "modelPath": (prefix / relative_model).as_posix(),
                "prototypeTag": "Prototype_Component_Model_" + asset_id,
                "defaultScale": [1.0, 1.0, 1.0],
                "anchor": "Origin",
                "groupId": "bern-foliage-native",
                "groupLabel": "Bern Castle Native Foliage",
                "evidence": (
                    "UE3 InstancedStaticMeshComponent exact StaticMesh: "
                    + str(asset["fullPath"])
                    + "; WModel SHA-256 "
                    + str(asset.get("wmodelSha256", ""))
                ),
                "source": {
                    "fullPath": str(asset["fullPath"]),
                    "wmodelSha256": str(asset.get("wmodelSha256", "")),
                    "runtimeReceiptSha256": str(asset.get("runtimeReceiptSha256", "")),
                },
            }
        )

    components.sort(key=lambda row: (row["sourceLevel"].casefold(), row["component"]["exportIndex"]))
    placements.sort(key=lambda row: row["sourcePlacementId"].casefold())
    source_paths = source_paths or {}
    result = {
        "schemaVersion": 1,
        "areaId": AREA_ID,
        "kind": "foliage-native-instance-overlay",
        "status": "ue3-native-suffix-decoded-and-source-verified",
        "coordinateContract": {
            "source": "UE3 row-major FMatrix in centimeters",
            "position": "Client=(UE.X, UE.Z, -UE.Y)*0.01",
            "rotation": "Client=B^T*UE*B, B=((1,0,0),(0,0,-1),(0,1,0))",
            "scale": "Client=(UE.X, UE.Z, UE.Y); reflection stored on signed X",
        },
        "sourceManifests": {
            "nonstatic": _hash_source(source_paths.get("nonstatic")),
            "baseStatic": _hash_source(source_paths.get("baseStatic")),
            "baseRuntime": _hash_source(source_paths.get("baseRuntime")),
            "supplementRuntime": _hash_source(source_paths.get("supplementRuntime")),
        },
        "summary": {
            "componentCount": len(components),
            "instanceCount": total_instances,
            "uniqueMeshCount": len(component_counts),
            "baseMeshCount": len(used_base_ids),
            "supplementMeshCount": len(used_supplement_ids),
            "baseComponentCount": base_component_count,
            "baseInstanceCount": base_instance_count,
            "supplementComponentCount": supplement_component_count,
            "supplementInstanceCount": supplement_instance_count,
            "overlayAssetDefinitionCount": len(overlay_assets),
            "anyNegativeScaleCount": any_negative_count,
            "reflectedCount": reflected_count,
            "lightGuidCountDistribution": {
                str(key): value for key, value in sorted(guid_counts.items())
            },
            "sourceLevelInstanceCounts": dict(sorted(level_counts.items())),
            "assetComponentCounts": dict(sorted(component_counts.items())),
            "assetInstanceCounts": dict(sorted(instance_counts.items())),
        },
        "assets": overlay_assets,
        "components": components,
        "placements": placements,
    }
    validate_overlay(result, expectations)
    return result


def validate_overlay(document: dict[str, Any], expectations: Expectations) -> None:
    if (
        document.get("schemaVersion") != 1
        or document.get("areaId") != AREA_ID
        or document.get("kind") != "foliage-native-instance-overlay"
    ):
        raise FoliageOverlayError("overlay document identity mismatch")
    assets = document.get("assets")
    components = document.get("components")
    placements = document.get("placements")
    if not isinstance(assets, list) or not isinstance(components, list) or not isinstance(placements, list):
        raise FoliageOverlayError("overlay arrays are malformed")
    if len(assets) != expectations.supplement_meshes:
        raise FoliageOverlayError("overlay asset definition count mismatch")
    if len(components) != expectations.components or len(placements) != expectations.instances:
        raise FoliageOverlayError("overlay component/placement count mismatch")
    asset_ids = [str(asset.get("assetId", "")) for asset in assets]
    source_ids = [str(row.get("sourcePlacementId", "")) for row in placements]
    placement_ids = [int(row.get("placementId", 0)) for row in placements]
    component_ids = [str(row.get("itemId", "")) for row in components]
    if len(set(asset_ids)) != len(asset_ids) or "" in asset_ids:
        raise FoliageOverlayError("overlay asset IDs are duplicate/empty")
    if len(set(source_ids)) != len(source_ids) or "" in source_ids:
        raise FoliageOverlayError("overlay source placement IDs are duplicate/empty")
    if len(set(placement_ids)) != len(placement_ids):
        raise FoliageOverlayError("overlay placement IDs collide")
    if any(value <= 0 or value > EDITOR_ID_MASK for value in placement_ids):
        raise FoliageOverlayError("overlay placement ID is outside low domain")
    if len(set(component_ids)) != len(component_ids) or "" in component_ids:
        raise FoliageOverlayError("overlay component IDs are duplicate/empty")


def atomic_write_overlay(
    output: Path, document: dict[str, Any], expectations: Expectations
) -> None:
    validate_overlay(document, expectations)
    output.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=output.name + ".", suffix=".staging", dir=output.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as stream:
            json.dump(document, stream, ensure_ascii=False, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        staged = load_json(temporary)
        validate_overlay(staged, expectations)
        os.replace(temporary, output)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--nonstatic", type=Path, default=DEFAULT_NONSTATIC)
    parser.add_argument("--base-static", type=Path, default=DEFAULT_BASE_STATIC)
    parser.add_argument("--base-runtime", type=Path, default=DEFAULT_BASE_RUNTIME)
    parser.add_argument("--supplement-runtime", type=Path, default=DEFAULT_SUPPLEMENT_RUNTIME)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--supplement-model-prefix", default="Map/LV_BER_BERNCASTLE")
    parser.add_argument("--expect-components", type=int, default=EXPECTED_COMPONENTS)
    parser.add_argument("--expect-instances", type=int, default=EXPECTED_INSTANCES)
    parser.add_argument("--expect-unique-meshes", type=int, default=EXPECTED_UNIQUE_MESHES)
    parser.add_argument("--expect-base-meshes", type=int, default=EXPECTED_BASE_MESHES)
    parser.add_argument("--expect-supplement-meshes", type=int, default=EXPECTED_SUPPLEMENT_MESHES)
    parser.add_argument("--expect-base-components", type=int, default=EXPECTED_BASE_COMPONENTS)
    parser.add_argument("--expect-base-instances", type=int, default=EXPECTED_BASE_INSTANCES)
    parser.add_argument("--expect-supplement-components", type=int, default=EXPECTED_SUPPLEMENT_COMPONENTS)
    parser.add_argument("--expect-supplement-instances", type=int, default=EXPECTED_SUPPLEMENT_INSTANCES)
    parser.add_argument("--expect-any-negative", type=int, default=EXPECTED_ANY_NEGATIVE)
    parser.add_argument("--expect-reflected", type=int, default=EXPECTED_REFLECTED)
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    paths = {
        "nonstatic": args.nonstatic.resolve(),
        "baseStatic": args.base_static.resolve(),
        "baseRuntime": args.base_runtime.resolve(),
        "supplementRuntime": args.supplement_runtime.resolve(),
    }
    for label, path in paths.items():
        if not path.is_file():
            raise FoliageOverlayError(f"{label} manifest is missing: {path}")
    expectations = Expectations(
        components=args.expect_components,
        instances=args.expect_instances,
        unique_meshes=args.expect_unique_meshes,
        base_meshes=args.expect_base_meshes,
        supplement_meshes=args.expect_supplement_meshes,
        base_components=args.expect_base_components,
        base_instances=args.expect_base_instances,
        supplement_components=args.expect_supplement_components,
        supplement_instances=args.expect_supplement_instances,
        any_negative=args.expect_any_negative,
        reflected=args.expect_reflected,
    )
    overlay = build_overlay(
        load_json(paths["nonstatic"]),
        load_json(paths["baseStatic"]),
        load_json(paths["baseRuntime"]),
        load_json(paths["supplementRuntime"]),
        expectations=expectations,
        supplement_model_prefix=args.supplement_model_prefix,
        source_paths=paths,
    )
    if not args.dry_run:
        atomic_write_overlay(args.output.resolve(), overlay, expectations)
    print(
        json.dumps(
            {
                "status": "validated" if args.dry_run else "written",
                "output": None if args.dry_run else str(args.output.resolve()),
                **overlay["summary"],
            },
            ensure_ascii=False,
        )
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FoliageOverlayError, OSError, ValueError, struct.error) as error:
        print(json.dumps({"status": "failed", "error": str(error)}), file=sys.stderr)
        raise SystemExit(1)
