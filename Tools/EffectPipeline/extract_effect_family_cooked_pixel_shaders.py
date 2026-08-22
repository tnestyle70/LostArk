#!/usr/bin/env python3
"""Extract one cooked pixel-shader blob per Effect material family.

`build_effect_family_shader_map_index.py` proves where each family's cooked
static permutations live.  `extract_ue3_material_shader_maps.py` extracts one
permutation once every offset and hash for it has already been pinned by hand.
Between the two sits the step nothing automated: choosing the permutation a
family's own child material actually selects, and pulling its base-pass pixel
program out of the cache.

This driver does that for every family at once.  For each family it takes the
child MaterialInstanceConstant the authored corpus names most often, decodes
that child's native FStaticParameterSet, and uses the engine-equality identity
to pick exactly one material map.  It then selects the base-pass pixel shader
for the carrier the corpus uses - particle sprite or local mesh - and writes
the packed DXBC slice out.

Every gate the pinned extractor enforces still applies afterwards.  A blob
written here is the cooked program for that permutation and nothing more: no
vertex factory, sampler state, render state or runtime binding is admitted by
its presence.  Families that resolve ambiguously are recorded as blocked
rather than resolved by preference.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import sys
import time
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
for extra in (str(Path(__file__).resolve().parent), str(LEVEL_TOOLS)):
    if extra not in sys.path:
        sys.path.insert(0, extra)

from derive_artist_31470_main_shader_map_identity import (  # noqa: E402
    engine_equivalent_static_parameter_set,
)
from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    package_tables,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    canonical_json_sha256,
    parse_static_parameter_set,
    validate_dxbc_container,
)
from extract_ue3_effect_material_closure import load_package  # noqa: E402
from extract_ue3_material_shader_maps import (  # noqa: E402
    extract_selected_packed_dxbc,
    parse_material_map,
    parse_shader_code_layout,
    select_structural_vf_pass_candidate,
)
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    parse_tagged_properties,
)

AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
SHADER_MAP_INDEX = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-shader-map-index.v1.json")
DEFAULT_BLOB_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/CookedShaders"
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json")
DEFAULT_SOURCE_PACK_MANIFEST = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
    r"\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json"
)
DEFAULT_CACHE = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST"
    r"\31470_TrackA_20260812\OfficialRefShaderCacheV974"
    r"\EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)

SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
FORMAT_VERSION = 1
SHADER_MAP_SCHEMA = "lostark.effect-family-shader-map-index"
SHADER_MAP_FORMAT_VERSION = 1

CARRIER_POLICIES = {
    "sprite": {
        "rendererType": "SpriteParticle",
        "family": "PARTICLE_SPRITE",
        "namePrefix": "fparticle",
        "excludeNameContains": ["beamtrail"],
        "passPixelShaderType":
            "tbasepasspixelshaderfnolightmappolicyskylight",
        "selectionFidelity":
            "STRUCTURAL_PARTICLE_VF_FAMILY_WITHOUT_NATIVE_EMITTER_ABI",
        "actualVfPassAdmission": False,
    },
    "mesh": {
        "rendererType": "MeshParticle",
        "family": "LOCAL_MESH",
        "vertexFactoryType": "flocalvertexfactory",
        "passPixelShaderType":
            "tbasepasspixelshaderfnolightmappolicyskylight",
        "selectionFidelity":
            "SOURCE_MESH_RENDERER_PLUS_STRUCTURAL_LOCALVF"
            "_WITHOUT_NATIVE_EMITTER_ABI",
        "actualVfPassAdmission": False,
    },
}

MESH_SLOT = "meshModel"


class ExtractError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ExtractError(message)


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(json.dumps(
        value, sort_keys=True, separators=(",", ":"),
        ensure_ascii=False).encode("utf-8")).hexdigest()


def raw_file_identity(path: Path, description: str) -> dict[str, Any]:
    require(path.is_file(), f"{description} is missing: {path}")
    digest = hashlib.sha256()
    byte_size = 0
    with path.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
            byte_size += len(chunk)
    return {
        "rawSha256": digest.hexdigest(),
        "byteSize": byte_size,
    }


def require_shader_map_identity(
    index: dict[str, Any],
    source_pack_identity: dict[str, Any],
    cache_identity: dict[str, Any],
) -> None:
    require(index.get("schema") == SHADER_MAP_SCHEMA,
            "shader-map index schema is not supported")
    require(index.get("formatVersion") == SHADER_MAP_FORMAT_VERSION,
            "shader-map index formatVersion is not supported")
    artifact_sha = index.get("artifactSha256")
    require(isinstance(artifact_sha, str) and len(artifact_sha) == 64,
            "shader-map index artifactSha256 is missing")
    payload = dict(index)
    payload.pop("artifactSha256", None)
    require(canonical_sha256(payload) == artifact_sha,
            "shader-map index artifactSha256 drifted")
    inputs = index.get("inputs") or {}
    require(
        inputs.get("sourcePackManifestRawSha256")
        == source_pack_identity["rawSha256"]
        and inputs.get("sourcePackManifestByteSize")
        == source_pack_identity["byteSize"],
        "source pack manifest differs from the shader-map index pin",
    )
    require(
        inputs.get("refShaderCacheRawSha256")
        == cache_identity["rawSha256"]
        and inputs.get("refShaderCacheByteSize")
        == cache_identity["byteSize"],
        "RefShaderCache differs from the shader-map index pin",
    )


def build_receipt(
    rows: list[dict[str, Any]],
    index: dict[str, Any],
    shader_map_identity: dict[str, Any],
    cache_path: Path,
    cache_identity: dict[str, Any],
) -> dict[str, Any]:
    extracted = sum(row.get("status") == "EXTRACTED" for row in rows)
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "admits": "COOKED_PROGRAM_ONLY",
            "note": (
                "Selecting a permutation and extracting its base-pass pixel "
                "program does not admit a vertex factory, sampler state, "
                "render state, runtime binding or visual result."
            ),
        },
        "inputs": {
            "shaderMapIndex":
                "Data/Effects/Contracts/"
                "effect-family-shader-map-index.v1.json",
            "shaderMapArtifactSha256": index["artifactSha256"],
            "shaderMapRawSha256": shader_map_identity["rawSha256"],
            "refShaderCacheFileName": cache_path.name,
            "refShaderCacheRawSha256": cache_identity["rawSha256"],
            "refShaderCacheByteSize": cache_identity["byteSize"],
            "blobDirectory": "Data/Effects/CookedShaders",
        },
        "summary": {
            "familyCount": len(rows),
            "extractedCount": extracted,
            "blockedCount": len(rows) - extracted,
            "extractedOccurrenceCount": sum(
                row["occurrenceCount"] for row in rows
                if row["status"] == "EXTRACTED"),
            "blockerCounts": dict(sorted(collections.Counter(
                row.get("blocker", "").split(":")[0].split("(")[0].strip()
                for row in rows if row["status"] == "BLOCKED").items())),
        },
        "families": rows,
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    return receipt


def write_receipt(path: Path, receipt: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(
            receipt, indent=2, ensure_ascii=False) + "\n")
    temporary.replace(path)


def collect_children() -> dict[str, dict[str, Any]]:
    """Parent -> the child MICs that select it, ranked by occurrence."""
    parents: dict[str, dict[str, Any]] = {}
    for path in sorted(AUTHORED_DIRECTORY.glob("*.effect.json")):
        document = read_json(path)
        for element in document.get("elements", []):
            material = element.get("material") or {}
            profile = material.get("sourceProfile") or {}
            parent = profile.get("parentMaterialPath")
            if not parent:
                continue
            row = parents.setdefault(parent, {
                "children": collections.Counter(),
                "carriers": collections.Counter(),
            })
            child = material.get("sourceMaterialPath")
            if child:
                row["children"][child] += 1
            slots = {
                resource.get("slotId")
                for resource in element.get("resources", [])
            }
            row["carriers"]["mesh" if MESH_SLOT in slots else "sprite"] += 1
    return parents


def load_package_index(manifest_path: Path) -> dict[str, Path]:
    manifest = read_json(manifest_path)
    root = manifest_path.parent
    index: dict[str, Path] = {}
    for package in manifest.get("packages", []):
        if not package.get("resolved"):
            continue
        name = str(package.get("logicalPackage") or "").lower()
        relative = package.get("relativePath")
        if name and relative:
            index.setdefault(name, root / relative)
    return index


def find_mic(package: Any, leaf: str) -> dict[str, Any] | None:
    for entry in package.exports:
        if entry.object_name.lower() != leaf:
            continue
        class_name = package_ref_name(
            entry.class_index, package.imports, package.exports).lower()
        if class_name != "materialinstanceconstant":
            continue
        return {
            "exportIndexZeroBased": entry.index,
            "serialOffset": entry.serial_offset,
            "serialByteSize": entry.serial_size,
        }
    return None


def child_static_identity(package: Any, row: dict[str, Any],
                          base_material_id: str) -> dict[str, Any]:
    serial = package.logical[
        row["serialOffset"]: row["serialOffset"] + row["serialByteSize"]]
    _, property_end = parse_tagged_properties(
        serial, package.names, package.summary.version)
    tail = serial[property_end:]
    require(bool(tail), "child MIC has no native static resource")
    needle = bytes.fromhex(base_material_id)
    offsets = []
    cursor = 0
    while True:
        found = tail.find(needle, cursor)
        if found < 0:
            break
        offsets.append(found)
        cursor = found + 1
    require(len(offsets) == 1,
            "child MIC base Material ID is absent or ambiguous")
    static_set = parse_static_parameter_set(tail, offsets[0], package.names)
    equality = engine_equivalent_static_parameter_set(static_set)
    return {
        "staticParameterSetRawSha256": static_set["rawSha256"],
        "engineEqualityStaticParameterSetSha256":
            canonical_json_sha256(equality),
        "staticSwitchCount": len(
            static_set.get("staticSwitchParameters") or []),
    }


def extract_family(
    family: dict[str, Any],
    children: dict[str, Any],
    package_index: dict[str, Path],
    package_cache: dict[str, Any],
    cache: dict[str, Any],
    layout: dict[str, Any],
    blob_directory: Path,
) -> dict[str, Any]:
    parent = family["parentMaterialPath"]
    resolution = family["resolution"]
    base_id = resolution.get("baseMaterialIdHex")
    require(bool(base_id), "family base Material ID is unresolved")
    contexts = (family["cacheScan"] or {}).get("materialMapContexts") or []
    require(bool(contexts), "family has no cooked material map")

    carrier_counts = children.get("carriers") or collections.Counter()
    carrier = (carrier_counts.most_common(1)[0][0]
               if carrier_counts else "sprite")
    policy = CARRIER_POLICIES[carrier]

    identity = None
    chosen_child = None
    errors: list[str] = []
    for child, _ in children["children"].most_common():
        segments = child.split(".")
        leaf = segments[-1].lower()
        package_names = []
        if len(segments) >= 3 and segments[0].lower() in package_index:
            package_names.append(segments[0].lower())
        package_names.append(resolution.get("packageName"))
        for package_name in [name for name in package_names if name]:
            package = package_cache.get(package_name)
            if package is None:
                if package_name not in package_index:
                    continue
                package = load_package(
                    package_index[package_name], LOSTARK_KR_AES_KEY)
                package_cache[package_name] = package
            row = find_mic(package, leaf)
            if row is None:
                continue
            try:
                identity = child_static_identity(package, row, base_id)
            except Exception as error:  # noqa: BLE001 - recorded, not raised
                errors.append(f"{child}: {error}")
                continue
            chosen_child = child
            break
        if identity is not None:
            break

    if identity is None:
        require(
            len(contexts) == 1,
            "child static set unresolved and the family has several "
            f"permutations ({len(contexts)}): "
            + ("; ".join(errors[:2]) if errors else "no child MIC located"))
        context = contexts[0]
        selection = "SINGLE_PERMUTATION_FAMILY"
    else:
        matches = [
            row for row in contexts
            if row["engineEqualityStaticParameterSetSha256"]
            == identity["engineEqualityStaticParameterSetSha256"]
        ]
        require(len(matches) == 1,
                f"engine-equality permutation is absent or ambiguous "
                f"({len(matches)} of {len(contexts)})")
        context = matches[0]
        selection = "CHILD_MIC_ENGINE_EQUALITY"

    material_map = parse_material_map(
        cache, layout, context,
        context["engineEqualityStaticParameterSetSha256"])
    candidate = select_structural_vf_pass_candidate(
        material_map, policy["rendererType"], policy)
    reference = candidate["selectedPixelPassReference"]
    require(reference is not None,
            "structural pixel pass reference is ambiguous")

    return {
        "parentMaterialPath": parent,
        "status": "RESOLVED",
        "carrier": carrier,
        "rendererType": policy["rendererType"],
        "childMaterialPath": chosen_child,
        "permutationSelection": selection,
        "permutationCount": len(contexts),
        "materialMapLogicalOffset": context["logicalOffset"],
        "engineEqualityStaticParameterSetSha256":
            context["engineEqualityStaticParameterSetSha256"],
        "vertexFactoryCandidateCount": candidate["vertexFactoryCandidateCount"],
        "vertexFactoryTypes": sorted({
            row["vertexFactoryType"]
            for row in candidate["vertexFactoryCandidates"]
        }),
        "shaderType": reference["shaderType"],
        "shaderIdHex": reference["shaderIdHex"],
        "uniformExpressionCounts": material_map["uniformExpressionCounts"],
        "admits": "COOKED_PROGRAM_ONLY",
    }


def extract_resolved_blobs(
    cache: dict[str, Any],
    layout: dict[str, Any],
    rows: list[dict[str, Any]],
    blob_directory: Path,
) -> int:
    """Pull every resolved family's program in one descriptor-table pass.

    The packed descriptor scan walks the whole cooked code section, so doing it
    per family costs the same walk once per family.  Collecting the references
    first turns 194 walks into one.
    """
    references = {
        row["shaderIdHex"]: {
            "shaderType": row["shaderType"],
            "shaderIdHex": row["shaderIdHex"],
        }
        for row in rows if row["status"] == "RESOLVED"
    }
    if not references:
        return 0
    blobs = extract_selected_packed_dxbc(
        cache, layout, list(references.values()))
    blob_directory.mkdir(parents=True, exist_ok=True)
    extracted = 0
    for row in rows:
        if row["status"] != "RESOLVED":
            continue
        payload = blobs.get(row["shaderIdHex"]) or {}
        bytecode = payload.get("_bytecode")
        if not isinstance(bytecode, (bytes, bytearray)):
            row["status"] = "BLOCKED"
            row["blocker"] = "packed DXBC payload is absent"
            continue
        bytecode = bytes(bytecode)
        validate_dxbc_container(bytecode)
        digest = hashlib.sha256(bytecode).hexdigest()
        (blob_directory / f"{digest}.dxbc").write_bytes(bytecode)
        row["status"] = "EXTRACTED"
        row["dxbcSha256"] = digest
        row["dxbcByteSize"] = len(bytecode)
        extracted += 1
    return extracted


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-pack-manifest", type=Path,
                        default=DEFAULT_SOURCE_PACK_MANIFEST)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--blob-directory", type=Path,
                        default=DEFAULT_BLOB_DIRECTORY)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--limit", type=int, default=0,
                        help="Extract only the N largest families.")
    parser.add_argument("--only", nargs="*", default=None,
                        help="Extract only these parent material paths.")
    arguments = parser.parse_args(argv)

    source_pack_path = arguments.source_pack_manifest.resolve()
    cache_path = arguments.cache.resolve()
    shader_map_identity = raw_file_identity(
        SHADER_MAP_INDEX, "shader-map index")
    source_pack_identity = raw_file_identity(
        source_pack_path, "source pack manifest")
    cache_identity = raw_file_identity(cache_path, "pinned RefShaderCache")
    index = read_json(SHADER_MAP_INDEX)
    require_shader_map_identity(
        index, source_pack_identity, cache_identity)
    children = collect_children()
    package_index = load_package_index(source_pack_path)
    families = [
        row for row in index["families"]
        if row["cookedEvidence"] == "COOKED_MATERIAL_MAPS_PRESENT"
    ]
    if arguments.only:
        wanted = set(arguments.only)
        families = [row for row in families
                    if row["parentMaterialPath"] in wanted]
    if arguments.limit:
        families = families[: arguments.limit]

    print(f"opening cache: {arguments.cache.name}", flush=True)
    started = time.time()
    cache = package_tables(cache_path)
    layout = parse_shader_code_layout(cache)
    print(f"cache tables in {time.time() - started:.1f}s", flush=True)

    package_cache: dict[str, Any] = {}
    rows = []
    started = time.time()
    for family in families:
        parent = family["parentMaterialPath"]
        try:
            row = extract_family(
                family, children.get(parent, {
                    "children": collections.Counter(),
                    "carriers": collections.Counter()}),
                package_index, package_cache, cache, layout,
                arguments.blob_directory)
            print(f"RESOLVED {parent} -> {row['shaderIdHex'][:12]} "
                  f"({row['carrier']}, {row['permutationSelection']})",
                  flush=True)
        except Exception as error:  # noqa: BLE001 - blocked, not fatal
            row = {
                "parentMaterialPath": parent,
                "status": "BLOCKED",
                "blocker": str(error)[:400],
            }
            print(f"SKIP     {parent}: {str(error)[:120]}", flush=True)
        row["occurrenceCount"] = family["occurrenceCount"]
        rows.append(row)
    print(f"resolved {len(rows)} families in {time.time() - started:.1f}s",
          flush=True)

    started = time.time()
    extracted = extract_resolved_blobs(
        cache, layout, rows, arguments.blob_directory)
    print(f"extracted {extracted} programs in {time.time() - started:.1f}s",
          flush=True)

    rows.sort(key=lambda item: (-item["occurrenceCount"],
                                item["parentMaterialPath"]))
    receipt = build_receipt(
        rows, index, shader_map_identity, cache_path, cache_identity)
    write_receipt(arguments.output, receipt)
    print(f"WROTE: {arguments.output}")
    print(f"RESULT: extracted={extracted} blocked={len(rows) - extracted}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
