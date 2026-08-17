#!/usr/bin/env python3
"""Reconnaissance scan for cooked UE3 Material shader-map presence.

RECONNAISSANCE ONLY. This answers "does a shader map for this material exist in
the cache, and roughly what parameter names live near it". It is NOT a shader
map decoder and its output must not be treated as a family contract.

Three limits make that distinction load-bearing:

  1. It walks a fixed byte window around the FIRST GUID hit. Shader maps for one
     material are adjacent, so the window mixes several maps together. The
     expression list it prints is therefore a union across maps, not the array
     of any single map.
  2. Uniform-expression serialization order is NOT the shader constant register
     order. UE3 packs scalars into float4 registers, drops expressions the
     compiler proved unused, and binds vectors and textures through separate
     tables. The native shader-object binding arrays are required for that, and
     `extract_artist_31470_main_ref_shader_cache.py` interprets them separately.
  3. Finding a parent material's state GUID proves the parent has maps. It does
     not select which permutation a given MaterialInstanceConstant resolves to;
     that needs the MIC's FStaticParameterSet joined against one exact map.

Use this to decide whether the exact path is worth opening for a family. Use the
class-neutral generalization of the Artist exact extractor to actually open it.

Original intent, kept for context:
Recover the ordered uniform-expression table of a cooked UE3 Material.

The grouped runtime profile guesses which authored scalar feeds which shader
constant by matching substrings in the parameter name. That guess is why
`in_opa_str` never reaches the shader (the matcher looks for "opacity") and why
one `_u` scalar overwrites both UV axes. The cooked RefShaderCache does not need
guessing: next to each material's shader map it serializes the uniform
expression array in the exact order the compiled shader reads its constants,
and every entry carries its parameter name.

This tool locates a material's shader map by its base Material state GUID and
decodes that array. The output is the authoritative parameter -> constant-slot
ordering for the family.

Read-only. It never writes into the game installation.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(
    0, str(REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"))

from extract_ue3_effect_material_closure import load_package  # noqa: E402
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY, parse_tagged_properties,
)

DEFAULT_INSTALL = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC")
DEFAULT_CACHE_NAME = "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk"

# UE3 serializes each uniform expression as an FName type tag followed by the
# expression's own payload. Only the shapes this corpus actually uses are
# decoded; anything else stops the walk rather than being guessed at.
EXPRESSION_TYPES = {
    "fmaterialuniformexpressionscalarparameter": "scalar",
    "fmaterialuniformexpressionvectorparameter": "vector",
    "fmaterialuniformexpressiontexture": "texture",
    "fmaterialuniformexpressiontextureparameter": "textureParameter",
    "fmaterialuniformexpressionstaticparameter": "staticParameter",
}


def read_fname(data: bytes, offset: int, names: list[str]) -> tuple[str, int]:
    """FName is (int32 name-table index, int32 instance number)."""
    if offset + 8 > len(data):
        return "", offset
    index, number = struct.unpack_from("<iI", data, offset)
    if not (0 <= index < len(names)) or number > 100000:
        return "", offset
    return names[index], offset + 8


def base_material_guid(package: Any, object_leaf: str) -> dict[str, Any] | None:
    """The state GUID lives at tail[16:32] past the tagged property stream."""
    from extract_ue3_placements import package_ref_name, package_ref_path
    for entry in package.exports:
        full = package_ref_name(
            entry.index + 1, package.imports, package.exports)
        if full.casefold().rsplit(".", 1)[-1] != object_leaf.casefold():
            continue
        serial = package.logical[
            entry.serial_offset: entry.serial_offset + entry.serial_size]
        try:
            _props, end = parse_tagged_properties(
                serial, package.names, package.summary.version)
        except Exception:                                      # noqa: BLE001
            continue
        tail = serial[end:]
        if len(tail) < 32:
            continue
        return {
            "objectPath": package_ref_path(
                entry.index + 1, package.imports, package.exports),
            "exportIndex": entry.index,
            "serialSize": entry.serial_size,
            "propertyStreamEnd": end,
            "baseMaterialIdHex": tail[16:32].hex(),
        }
    return None


def walk_expressions(
    data: bytes, names: list[str], start: int, window: int
) -> list[dict[str, Any]]:
    """Collect uniform expressions in serialized order within a window.

    The walk steps by 4 bytes rather than trusting a fixed record size: the
    payload after each type tag varies per expression kind and an unrecognized
    kind must not silently shift every following entry.
    """
    out: list[dict[str, Any]] = []
    limit = min(len(data) - 8, start + window)
    offset = max(0, start)
    while offset < limit:
        name, after = read_fname(data, offset, names)
        kind = EXPRESSION_TYPES.get(name.casefold())
        if kind is None:
            offset += 4
            continue
        row: dict[str, Any] = {
            "order": len(out),
            "offset": offset,
            "expressionType": name,
            "kind": kind,
        }
        parameter, after_param = read_fname(data, after, names)
        if kind in ("scalar", "vector", "textureParameter", "staticParameter"):
            row["parameterName"] = parameter
            cursor = after_param
            if kind == "scalar" and cursor + 4 <= len(data):
                row["defaultValue"] = round(
                    struct.unpack_from("<f", data, cursor)[0], 6)
            elif kind == "vector" and cursor + 16 <= len(data):
                row["defaultValue"] = [
                    round(v, 6)
                    for v in struct.unpack_from("<4f", data, cursor)]
        elif kind == "texture" and after + 4 <= len(data):
            row["textureIndex"] = struct.unpack_from("<i", data, after)[0]
        out.append(row)
        offset = after
    return out


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--install-root", type=Path, default=DEFAULT_INSTALL)
    parser.add_argument("--cache-name", default=DEFAULT_CACHE_NAME)
    parser.add_argument(
        "--material", action="append", required=True, metavar="PKG:LEAF",
        help="Material package file name and object leaf, e.g. "
             "ZHJ4TC4PCK4PR4J22HIXEYUXBU.upk:fx_j_pa_glasshole_02_tr")
    parser.add_argument("--window", type=int, default=8192,
                        help="Bytes to walk after each shader-map hit.")
    parser.add_argument("--back", type=int, default=4096,
                        help="Bytes to walk before each hit.")
    parser.add_argument("--output", type=Path, default=None)
    arguments = parser.parse_args(argv)

    targets = []
    for spec in arguments.material:
        package_name, _, leaf = spec.partition(":")
        if not package_name or not leaf:
            parser.error(f"--material expects PKG:LEAF, got {spec!r}")
        targets.append((package_name, leaf))

    identities: dict[str, dict[str, Any]] = {}
    by_package: dict[str, list[str]] = {}
    for package_name, leaf in targets:
        by_package.setdefault(package_name, []).append(leaf)

    for package_name, leaves in by_package.items():
        path = arguments.install_root / "Packages" / package_name
        if not path.is_file():
            path = arguments.install_root / package_name
        if not path.is_file():
            print(f"FAIL: material package not found: {package_name}")
            return 1
        package = load_package(path, LOSTARK_KR_AES_KEY)
        for leaf in leaves:
            identity = base_material_guid(package, leaf)
            if identity is None:
                print(f"FAIL: no state GUID for {leaf} in {package_name}")
                return 1
            identity["package"] = package_name
            identity["packageSha256"] = package.sha256
            identities[leaf] = identity
            print(f"{leaf:<32} GUID {identity['baseMaterialIdHex']}")
        del package

    cache_path = arguments.install_root / arguments.cache_name
    if not cache_path.is_file():
        print(f"FAIL: shader cache not found: {cache_path}")
        return 1
    print(f"\nloading {cache_path.name} "
          f"({cache_path.stat().st_size / 1e6:.0f} MB physical)")
    cache = load_package(cache_path, LOSTARK_KR_AES_KEY)
    data = cache.logical
    names = cache.names
    print(f"logical {len(data) / 1e6:.0f} MB, name table {len(names)}\n")

    report: dict[str, Any] = {
        "schema": "lostark.ue3-material-uniform-expression-table",
        "formatVersion": 1,
        "shaderCache": {
            "fileName": cache_path.name,
            "physicalByteSize": cache_path.stat().st_size,
            "logicalByteSize": len(data),
            "sha256": cache.sha256,
        },
        "materials": {},
    }

    for leaf, identity in identities.items():
        raw = bytes.fromhex(identity["baseMaterialIdHex"])
        hits: list[int] = []
        cursor = 0
        while True:
            found = data.find(raw, cursor)
            if found < 0:
                break
            hits.append(found)
            cursor = found + 1
        entry: dict[str, Any] = dict(identity)
        entry["shaderMapHitCount"] = len(hits)
        entry["shaderMapOffsets"] = hits[:32]
        if not hits:
            entry["status"] = "NO_SHADER_MAP_IN_CACHE"
            report["materials"][leaf] = entry
            print(f"{leaf:<32} NO MATCH")
            continue

        expressions = walk_expressions(
            data, names, hits[0] - arguments.back,
            arguments.back + arguments.window)
        entry["status"] = "SHADER_MAP_JOINED"
        entry["uniformExpressions"] = expressions
        counts: dict[str, int] = {}
        for row in expressions:
            counts[row["kind"]] = counts.get(row["kind"], 0) + 1
        entry["uniformExpressionCounts"] = counts
        report["materials"][leaf] = entry
        print(f"{leaf:<32} JOINED  hits={len(hits):<3} "
              f"expressions={len(expressions)} {counts}")

    if arguments.output is not None:
        arguments.output.parent.mkdir(parents=True, exist_ok=True)
        arguments.output.write_text(
            json.dumps(report, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8")
        print(f"\n-> {arguments.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
