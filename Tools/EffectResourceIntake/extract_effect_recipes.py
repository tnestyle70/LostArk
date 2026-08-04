"""Extract Cascade recipes from cooked LostArk UPK packages.

Reads a package, walks every ParticleSystem down to its module parameters and
writes one JSON document per package.

The UE3 container work (AES, LZ4, name/import/export tables, tagged properties)
is reused from Tools/LevelPlacementExtractor/extract_ue3_placements.py.  That
file belongs to the map tool and is imported unchanged.

Usage:
    python extract_effect_recipes.py <package.upk> [...] --out <dir>
    python extract_effect_recipes.py --glob "SourceRaw/Glaivier/*.upk" --out <dir>
"""
from __future__ import annotations

import argparse
import collections
import datetime
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import Any

_UE3_TOOL_DIR = Path(__file__).resolve().parents[1] / "LevelPlacementExtractor"
sys.path.insert(0, str(_UE3_TOOL_DIR))

import extract_ue3_placements as ue  # noqa: E402

SCHEMA_VERSION = 2

# Property values whose struct type is not understood by the map tool decoder
# come back as a 32-byte hex preview, which loses data.  Keep the whole payload
# instead so nested structures can be re-parsed.  Confined to this process.
_decode_original = ue.decode_property_value


def _decode_keep_payload(property_type, struct_type, payload, names, bool_value):
    value = _decode_original(property_type, struct_type, payload, names, bool_value)
    if isinstance(value, dict) and "hex" in value:
        return {"__raw__": payload}
    if isinstance(value, dict) and set(value) == {"count", "size"}:
        return {"__raw__": payload, "__array__": True}
    return value


ue.decode_property_value = _decode_keep_payload


def _f32(bits: int) -> float:
    return struct.unpack("<f", struct.pack("<i", bits))[0]


def _is_raw(value: Any) -> bool:
    return isinstance(value, dict) and "__raw__" in value


class PackageReader:
    """One decompressed package plus the lookups needed to resolve references."""

    def __init__(self, path: Path):
        self.path = path
        physical = path.read_bytes()
        self.sha256 = hashlib.sha256(physical).hexdigest()
        self.summary = ue.parse_summary(physical)
        self.logical = ue.decompress_package(physical, self.summary, ue.LOSTARK_KR_AES_KEY)
        self.names = ue.parse_name_table(self.logical, self.summary)
        self.imports = ue.parse_import_table(self.logical, self.summary, self.names)
        self.exports = ue.parse_export_table(self.logical, self.summary, self.names)
        self.errors: list[dict[str, Any]] = []

    def class_of(self, entry) -> str:
        return ue.package_ref_name(entry.class_index, self.imports, self.exports).casefold()

    def ref_path(self, ref: int) -> str | None:
        if not isinstance(ref, int) or ref == 0:
            return None
        try:
            return ue.package_ref_path(ref, self.imports, self.exports)
        except Exception:
            return ue.package_ref_name(ref, self.imports, self.exports)

    def export_of(self, ref: int):
        """Resolve a positive package reference to an export entry."""
        if not isinstance(ref, int) or ref <= 0 or ref > len(self.exports):
            return None
        return self.exports[ref - 1]

    def properties_of(self, entry) -> dict[str, Any]:
        if entry is None or entry.serial_size <= 0:
            return {}
        serial = self.logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
        try:
            properties, _ = ue.parse_tagged_properties(serial, self.names, self.summary.version)
            return properties
        except Exception as error:
            self.errors.append(
                {"export": entry.index, "object": entry.object_name, "error": str(error)}
            )
            return {}

    def nested(self, payload: bytes) -> dict[str, Any] | None:
        try:
            properties, _ = ue.parse_tagged_properties_at(payload, self.names, 0)
            return properties
        except Exception:
            return None


def decode_object_array(payload: bytes) -> list[int] | None:
    if len(payload) < 4:
        return None
    count = struct.unpack_from("<i", payload, 0)[0]
    if count < 0 or len(payload) != 4 + count * 4:
        return None
    return list(struct.unpack_from(f"<{count}i", payload, 4))


def decode_struct_array(reader: PackageReader, payload: bytes) -> list[dict] | None:
    """Array of non-atomic structs: count, then one tagged-property block each."""
    if len(payload) < 4:
        return None
    count = struct.unpack_from("<i", payload, 0)[0]
    if not 0 < count <= 4096:
        return None
    items: list[dict] = []
    offset = 4
    for _ in range(count):
        try:
            properties, offset = ue.parse_tagged_properties_at(payload, reader.names, offset)
        except Exception:
            return None
        items.append(simplify(reader, properties))
    return items if offset == len(payload) else None


def simplify(reader: PackageReader, properties: dict[str, Any]) -> dict[str, Any]:
    """Flatten a tagged-property dict into plain JSON-friendly values."""
    out: dict[str, Any] = {}
    for key, item in properties.items():
        out[key] = simplify_value(reader, item)
    return out


def simplify_value(reader: PackageReader, item: dict[str, Any]) -> Any:
    kind = (item.get("type") or "").casefold()
    struct_type = (item.get("structType") or "").casefold()
    value = item.get("value")

    if struct_type.startswith("rawdistribution"):
        return decode_distribution(reader, struct_type, value)

    if kind == "objectproperty" and isinstance(value, int):
        if value == 0:
            return None
        return {"ref": value, "path": reader.ref_path(value)}

    if kind == "byteproperty" and _is_raw(value):
        # A plain byte stays numeric; an enum is stored as an FName referring to
        # the value name, e.g. screenalignment -> "psa_square".
        payload = value["__raw__"]
        if len(payload) == 1:
            return payload[0]
        if len(payload) >= 8:
            index, number = struct.unpack_from("<2i", payload, 0)
            if 0 <= index < len(reader.names):
                name = reader.names[index]
                return f"{name}_{number}" if number else name
        return {"unparsed_bytes": len(payload), "hex": payload.hex()}

    if _is_raw(value):
        payload = value["__raw__"]
        if value.get("__array__"):
            objects = decode_object_array(payload)
            if objects is not None:
                return [
                    {"ref": r, "path": reader.ref_path(r)} if r else None for r in objects
                ]
            structs = decode_struct_array(reader, payload)
            if structs is not None:
                return structs
            floats = decode_float_array(payload)
            if floats is not None:
                return floats
        nested = reader.nested(payload)
        if nested is not None:
            return simplify(reader, nested)
        return {"unparsed_bytes": len(payload), "hex": payload.hex()}

    if isinstance(value, list) and value and all(isinstance(v, int) for v in value):
        return value
    return value


def decode_float_array(payload: bytes) -> list[float] | None:
    if len(payload) < 4:
        return None
    count = struct.unpack_from("<i", payload, 0)[0]
    if count < 0 or len(payload) != 4 + count * 4:
        return None
    return [round(v, 6) for v in struct.unpack_from(f"<{count}f", payload, 4)]


def decode_distribution(reader: PackageReader, struct_type: str, value: Any) -> dict[str, Any]:
    """A RawDistribution is either an authored object reference or a baked table.

    Measured on cooked LostArk packages: when `distribution` is non-zero the
    `lookuptable` is empty, and when it is zero the table carries the values.
    """
    result: dict[str, Any] = {"distribution_type": struct_type}
    if not _is_raw(value):
        result["kind"] = "unavailable"
        return result

    inner = reader.nested(value["__raw__"])
    if inner is None:
        result["kind"] = "unparsed"
        result["hex"] = value["__raw__"].hex()
        return result

    ref = inner.get("distribution", {}).get("value", 0)
    table_item = inner.get("lookuptable", {}).get("value")
    table: list[int] = table_item if isinstance(table_item, list) else []

    if isinstance(ref, int) and ref != 0:
        entry = reader.export_of(ref)
        result["kind"] = "authored"
        result["object"] = reader.ref_path(ref)
        if entry is not None:
            result["class"] = reader.class_of(entry)
            result["values"] = simplify(reader, reader.properties_of(entry))
        return result

    if table:
        # Bit patterns, not integers.  The cooked table header layout is not
        # verified, so the values are reported as-is rather than as a curve.
        result["kind"] = "baked"
        result["lookup_table"] = [round(_f32(v), 6) for v in table]
        return result

    result["kind"] = "empty"
    return result


def read_module(reader: PackageReader, ref: int) -> dict[str, Any] | None:
    entry = reader.export_of(ref)
    if entry is None:
        return None
    return {
        "export_index": entry.index,
        "class": reader.class_of(entry),
        "object_name": entry.object_name,
        "params": simplify(reader, reader.properties_of(entry)),
    }


def refs_of(value: Any) -> list[int]:
    """Normalise a property value that holds one or more export references."""
    if isinstance(value, int):
        return [value] if value else []
    if isinstance(value, dict):
        if "ref" in value:
            return [value["ref"]] if value["ref"] else []
        if _is_raw(value):
            objects = decode_object_array(value["__raw__"])
            return [r for r in objects if r] if objects else []
    if isinstance(value, list):
        out: list[int] = []
        for item in value:
            out.extend(refs_of(item))
        return out
    return []


def read_lod_level(reader: PackageReader, ref: int, index: int) -> dict[str, Any] | None:
    entry = reader.export_of(ref)
    if entry is None:
        return None
    raw = reader.properties_of(entry)
    level: dict[str, Any] = {
        "index": index,
        "export_index": entry.index,
        "peak_active_particles": ue.property_value(raw, "PeakActiveParticles", None),
    }
    if isinstance(level["peak_active_particles"], dict):
        level["peak_active_particles"] = level["peak_active_particles"].get("value")

    modules: list[dict[str, Any]] = []
    unresolved_module_refs: list[dict[str, Any]] = []
    seen: set[int] = set()
    for key in ("RequiredModule", "SpawnModule", "TypeDataModule", "Modules"):
        item = ue.property_value(raw, key, None)
        source = item.get("value") if isinstance(item, dict) and "value" in item else item
        for ref_value in refs_of(source):
            if ref_value in seen:
                continue
            seen.add(ref_value)
            module = read_module(reader, ref_value)
            if module is not None:
                module["slot"] = key
                modules.append(module)
            else:
                unresolved_module_refs.append({
                    "slot": key,
                    "ref": ref_value,
                    "path": reader.ref_path(ref_value),
                    "reason": (
                        "external_import_not_materialized"
                        if ref_value < 0 else "missing_local_export"
                    ),
                })
    level["modules"] = modules
    level["unresolved_module_refs"] = unresolved_module_refs
    return level


def read_emitter(reader: PackageReader, ref: int, index: int) -> dict[str, Any] | None:
    entry = reader.export_of(ref)
    if entry is None:
        return None
    raw = reader.properties_of(entry)
    item = ue.property_value(raw, "LODLevels", None)
    source = item.get("value") if isinstance(item, dict) and "value" in item else item
    levels = []
    unresolved_lod_refs = []
    for i, lod_ref in enumerate(refs_of(source)):
        level = read_lod_level(reader, lod_ref, i)
        if level is not None:
            levels.append(level)
        else:
            unresolved_lod_refs.append({
                "index": i,
                "ref": lod_ref,
                "path": reader.ref_path(lod_ref),
                "reason": (
                    "external_import_not_materialized"
                    if lod_ref < 0 else "missing_local_export"
                ),
            })
    name_item = ue.property_value(raw, "EmitterName", None)
    if isinstance(name_item, dict):
        name_item = name_item.get("value")
    return {
        "index": index,
        "export_index": entry.index,
        "class": reader.class_of(entry),
        "object_name": entry.object_name,
        "emitter_name": name_item,
        "lod_levels": levels,
        "unresolved_lod_refs": unresolved_lod_refs,
    }


def read_particle_system(reader: PackageReader, entry) -> dict[str, Any]:
    raw = reader.properties_of(entry)
    item = ue.property_value(raw, "Emitters", None)
    source = item.get("value") if isinstance(item, dict) and "value" in item else item
    emitters = [
        emitter
        for i, ref in enumerate(refs_of(source))
        if (emitter := read_emitter(reader, ref, i)) is not None
    ]
    system = {k: simplify_value(reader, v) for k, v in raw.items() if k != "emitters"}
    return {
        "object_name": entry.object_name,
        "export_index": entry.index,
        "system": system,
        "emitters": emitters,
    }


def logical_name(path: Path) -> str:
    """SourceRaw files are named LOGICAL__HASH.upk; raw archive files are hashes."""
    stem = path.stem
    return stem.split("__")[0].lower() if "__" in stem else stem.lower()


def extract(path: Path, package_override: str | None = None) -> dict[str, Any]:
    reader = PackageReader(path)
    package = (package_override or logical_name(path)).lower()
    assets = []
    for entry in reader.exports:
        if reader.class_of(entry) != "particlesystem" or entry.serial_size <= 0:
            continue
        asset = read_particle_system(reader, entry)
        asset["asset_id"] = f"{package}.{asset['object_name']}".lower()
        assets.append(asset)

    stats: collections.Counter = collections.Counter()
    module_classes: collections.Counter = collections.Counter()

    def walk(node: Any) -> None:
        if isinstance(node, dict):
            kind = node.get("kind")
            if kind and "distribution_type" in node:
                stats[f"distribution_{kind}"] += 1
            if "unparsed_bytes" in node:
                stats["unparsed_values"] += 1
            for value in node.values():
                walk(value)
        elif isinstance(node, list):
            for value in node:
                walk(value)

    for asset in assets:
        stats["particle_systems"] += 1
        for emitter in asset["emitters"]:
            stats["emitters"] += 1
            for unresolved in emitter.get("unresolved_lod_refs", []):
                stats["unresolved_lod_refs"] += 1
                if unresolved.get("ref", 0) < 0:
                    stats["external_import_lod_refs"] += 1
                else:
                    stats["missing_local_lod_refs"] += 1
            for level in emitter["lod_levels"]:
                stats["lod_levels"] += 1
                for unresolved in level.get("unresolved_module_refs", []):
                    stats["unresolved_module_refs"] += 1
                    if unresolved.get("ref", 0) < 0:
                        stats["external_import_module_refs"] += 1
                    else:
                        stats["missing_local_module_refs"] += 1
                for module in level["modules"]:
                    stats["modules"] += 1
                    module_classes[module["class"]] += 1
        walk(asset)

    return {
        "schema_version": SCHEMA_VERSION,
        "generated_at": datetime.datetime.now().astimezone().isoformat(),
        "provenance": "reconstructed_local_exports",
        "source": {
            "file": path.name,
            "logical_package": package,
            "bytes": path.stat().st_size,
            "sha256": reader.sha256,
            "package_version": reader.summary.version,
        },
        "stats": dict(stats),
        "module_classes": dict(module_classes.most_common()),
        "parse_errors": reader.errors,
        "assets": assets,
    }


def parse_args(argv=None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packages", nargs="*", type=Path)
    parser.add_argument("--glob", default=None, help="glob relative to the repository root")
    parser.add_argument("--out", type=Path, required=True, help="output directory")
    parser.add_argument(
        "--source-pack-manifest", type=Path, default=None,
        help="source_pack_manifest.json used to restore logical package names",
    )
    parser.add_argument("--summary-only", action="store_true", help="print stats, write nothing")
    return parser.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)
    paths = list(args.packages)
    if args.glob:
        paths.extend(sorted(Path().glob(args.glob)))
    if not paths:
        print("no packages given", file=sys.stderr)
        return 2

    args.out.mkdir(parents=True, exist_ok=True)
    logical_by_physical: dict[str, str] = {}
    if args.source_pack_manifest is not None:
        source_pack = json.loads(
            args.source_pack_manifest.read_text(encoding="utf-8")
        )
        logical_by_physical = {
            str(row.get("physicalPackage") or "").casefold():
                str(row.get("logicalPackage") or "")
            for row in source_pack.get("packages", [])
            if row.get("resolved")
        }
    failures = 0
    for path in paths:
        try:
            document = extract(
                path, logical_by_physical.get(path.name.casefold()) or None
            )
        except Exception as error:
            failures += 1
            print(f"FAIL {path.name}: {type(error).__name__}: {error}", file=sys.stderr)
            continue
        stats = document["stats"]
        print(
            f"{document['source']['logical_package']:20s} "
            f"systems={stats.get('particle_systems', 0):4d} "
            f"emitters={stats.get('emitters', 0):5d} "
            f"modules={stats.get('modules', 0):6d} "
            f"dist(authored/baked/empty)="
            f"{stats.get('distribution_authored', 0)}/"
            f"{stats.get('distribution_baked', 0)}/"
            f"{stats.get('distribution_empty', 0)} "
            f"unparsed={stats.get('unparsed_values', 0)} "
            f"external_refs={stats.get('external_import_module_refs', 0)} "
            f"errors={len(document['parse_errors'])}"
        )
        if args.summary_only:
            continue
        target = args.out / f"{document['source']['logical_package']}.recipe.json"
        target.write_text(
            json.dumps(document, indent=1, ensure_ascii=False) + "\n", encoding="utf-8"
        )
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
