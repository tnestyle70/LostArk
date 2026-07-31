"""Resolve LostArk effect materials down to the texture files they sample.

A recipe produced by extract_effect_recipes.py names a material instance, e.g.
`fx_m_mi_00.fx_mi.fx_d_pa_atta_05_10_tr`, but the Effect Tool needs a DDS path.
This reads MaterialInstanceConstant objects out of the cooked packages, pulls
their texture / scalar / vector parameters, and - when a texture index built by
Build-EffectTextureIndex is supplied - maps each texture to a file on disk.

Usage:
    python extract_effect_materials.py --glob "<...>/*.upk" --out <dir> \
        [--texture-index <effect_texture_index.csv>] [--map-out <lookup.json>]
"""
from __future__ import annotations

import argparse
import collections
import csv
import datetime
import json
import struct
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import extract_effect_recipes as recipes  # noqa: E402  (also installs the payload patch)

ue = recipes.ue

# A few cooked packages carry one malformed UTF-16 name. The shared reader lets
# the UnicodeDecodeError escape, which throws away the entire package - and
# fx_m_mi_00 alone is referenced 2,575 times by the Glaivier recipes. Wrap the
# reader here instead of editing the map tool's parser, the same way
# extract_effect_recipes.py wraps decode_property_value. The bytes are consumed
# before the decode fails, so returning an empty name keeps the stream aligned
# and costs one name out of several thousand.
_original_fstring = ue.Reader.fstring


def _tolerant_fstring(self) -> str:
    try:
        return _original_fstring(self)
    except UnicodeDecodeError:
        return ""


ue.Reader.fstring = _tolerant_fstring

SCHEMA_VERSION = 2
MATERIAL_CLASSES = ("materialinstanceconstant", "materialinstancetimevarying", "material")


def linear_color(value: Any) -> Any:
    """Vector parameters arrive as a 16-byte FLinearColor payload."""
    if isinstance(value, dict) and value.get("unparsed_bytes") == 16:
        r, g, b, a = struct.unpack("<4f", bytes.fromhex(value["hex"]))
        return {"r": round(r, 6), "g": round(g, 6), "b": round(b, 6), "a": round(a, 6)}
    return value


def parameter_list(entries: Any, decode=lambda v: v) -> list[dict[str, Any]]:
    """Flatten a *ParameterValues array, dropping the expression GUID noise."""
    if not isinstance(entries, list):
        return []
    out = []
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        name = entry.get("parametername")
        if name is None:
            continue
        out.append({"name": name, "value": decode(entry.get("parametervalue"))})
    return out


def texture_key(path: str | None) -> tuple[str, str] | None:
    """`fx_tex_03.fx_e_fluid_006` -> ('FX_TEX_03', 'fx_e_fluid_006')."""
    if not path:
        return None
    parts = path.split(".")
    if len(parts) < 2:
        return None
    return parts[0].upper(), parts[-1].lower()


def load_texture_index(path: Path) -> dict[tuple[str, str], dict[str, Any]]:
    index: dict[tuple[str, str], dict[str, Any]] = {}
    with path.open(encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            package = (row.get("package") or "").upper()
            name = (row.get("name") or "").lower()
            if not package or not name:
                continue
            index[(package, name)] = row
    return index


def read_material(reader: recipes.PackageReader, entry) -> dict[str, Any]:
    simple = recipes.simplify(reader, reader.properties_of(entry))
    textures = parameter_list(simple.get("textureparametervalues"))
    for item in textures:
        value = item["value"]
        item["texture"] = value.get("path") if isinstance(value, dict) else None
        item.pop("value", None)
    parent = simple.get("parent")
    return {
        "material_path": reader.ref_path(entry.index + 1),
        "object_name": entry.object_name,
        "class": reader.class_of(entry),
        "export_index": entry.index,
        "parent": parent.get("path") if isinstance(parent, dict) else None,
        "textures": textures,
        "scalars": parameter_list(simple.get("scalarparametervalues")),
        "vectors": parameter_list(simple.get("vectorparametervalues"), linear_color),
    }


def resolve_textures(
    materials: list[dict[str, Any]],
    index: dict[tuple[str, str], dict[str, Any]],
    package_name: str,
) -> collections.Counter:
    stats: collections.Counter = collections.Counter()
    for material in materials:
        for item in material["textures"]:
            path = item.get("texture")
            # A texture defined in this same package has no package prefix.
            if path and "." not in path:
                path = f"{package_name}.{path}"
            key = texture_key(path)
            row = index.get(key) if key else None
            if row is None:
                stats["texture_unresolved" if path else "texture_missing"] += 1
                continue
            stats["texture_resolved"] += 1
            item["dds_path"] = row.get("tool_path")
            item["width"] = row.get("width")
            item["height"] = row.get("height")
            if row.get("frames") not in (None, "", "0"):
                item["subuv"] = {
                    "columns": row.get("atlas_cols"),
                    "rows": row.get("atlas_rows"),
                    "frames": row.get("frames"),
                }
    return stats


def extract(path: Path, index: dict[tuple[str, str], dict[str, Any]]) -> dict[str, Any]:
    reader = recipes.PackageReader(path)
    package = recipes.logical_name(path)
    materials = [
        read_material(reader, entry)
        for entry in reader.exports
        if reader.class_of(entry) in MATERIAL_CLASSES and entry.serial_size > 0
    ]
    stats = collections.Counter(
        materials=len(materials),
        textures=sum(len(m["textures"]) for m in materials),
    )
    if index:
        stats.update(resolve_textures(materials, index, package))
    return {
        "schema_version": SCHEMA_VERSION,
        "generated_at": datetime.datetime.now().astimezone().isoformat(),
        "provenance": "reconstructed",
        "source": {
            "file": path.name,
            "logical_package": package,
            "sha256": reader.sha256,
        },
        "stats": dict(stats),
        "parse_errors": reader.errors,
        "materials": materials,
    }


def parse_args(argv=None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packages", nargs="*", type=Path)
    parser.add_argument("--glob", default=None, help="glob relative to the working directory")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--texture-index", type=Path, default=None)
    parser.add_argument(
        "--map-out",
        type=Path,
        default=None,
        help=(
            "also write a flat material_path lookup preserving parent, "
            "textures, scalar parameters, and vector parameters"
        ),
    )
    parser.add_argument("--skip-empty", action="store_true")
    return parser.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)
    paths = list(args.packages)
    if args.glob:
        paths.extend(sorted(Path().glob(args.glob)))
    if not paths:
        print("no packages given", file=sys.stderr)
        return 2

    index = load_texture_index(args.texture_index) if args.texture_index else {}
    if args.texture_index:
        print(f"texture index: {len(index)} entries")

    args.out.mkdir(parents=True, exist_ok=True)
    lookup: dict[str, Any] = {}
    totals: collections.Counter = collections.Counter()
    failures = 0

    for path in paths:
        try:
            document = extract(path, index)
        except Exception as error:
            failures += 1
            print(f"FAIL {path.name}: {type(error).__name__}: {error}", file=sys.stderr)
            continue
        stats = document["stats"]
        totals.update(stats)
        if stats.get("materials") or not args.skip_empty:
            target = args.out / f"{document['source']['logical_package']}.materials.json"
            target.write_text(
                json.dumps(document, indent=1, ensure_ascii=False) + "\n", encoding="utf-8"
            )
        if args.map_out:
            # Raw archive files have obfuscated names, so the owning package name
            # is unknown.  Key on the in-package path (group.object) instead: a
            # recipe reference like `fx_m_mi_00.fx_mi.name` joins after dropping
            # its first segment.  Several packages can share a path, so keep a
            # list rather than letting one silently win.
            for material in document["materials"]:
                key = (material["material_path"] or material["object_name"]).lower()
                lookup.setdefault(key, []).append(
                    {
                        "source_file": document["source"]["file"],
                        "material_path": material["material_path"],
                        "object_name": material["object_name"],
                        "class": material["class"],
                        "parent": material["parent"],
                        "textures": material["textures"],
                        "scalars": material["scalars"],
                        "vectors": material["vectors"],
                    }
                )
        print(
            f"{document['source']['logical_package']:24s} "
            f"materials={stats.get('materials', 0):5d} "
            f"textures={stats.get('textures', 0):5d} "
            f"resolved={stats.get('texture_resolved', 0):5d} "
            f"unresolved={stats.get('texture_unresolved', 0):5d} "
            f"errors={len(document['parse_errors'])}"
        )

    if args.map_out:
        args.map_out.parent.mkdir(parents=True, exist_ok=True)
        args.map_out.write_text(
            json.dumps(
                {
                    "schema_version": SCHEMA_VERSION,
                    "generated_at": datetime.datetime.now().astimezone().isoformat(),
                    "count": len(lookup),
                    "materials": lookup,
                },
                indent=1,
                ensure_ascii=False,
            )
            + "\n",
            encoding="utf-8",
        )
        print(f"lookup: {len(lookup)} materials -> {args.map_out}")

    print("totals:", dict(totals))
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
