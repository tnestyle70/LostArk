#!/usr/bin/env python3
"""Build the fail-closed Artist F renderer restoration coverage matrix.

The matrix joins all 27 material recipes and all 35 active occurrences to the
official v974 and current installed RefShaderCache packages.  It proves only
serialized FStaticParameterSet equality and candidate shader references.  A
descriptor tail is not native shader-selection evidence, and a decompressed
record may contain more than one DXBC container, so neither a candidate code
index nor a structural map join is promoted to selected/admitted execution.
"""

from __future__ import annotations

import argparse
import copy
import gc
import hashlib
import json
import math
import struct
from collections import Counter
from pathlib import Path
from typing import Any

from derive_artist_31470_main_shader_map_identity import (
    engine_equivalent_static_parameter_set,
    normalized_static_parameter_set,
)
from extract_artist_31470_main_ref_shader_cache import (
    EXPECTED_INSTALLED_CACHE,
    EXPECTED_OFFICIAL_CACHE,
    DEFAULT_INSTALLED_CACHE,
    DEFAULT_OFFICIAL_CACHE,
    package_tables,
    parse_cache_code_index,
    read_fname_at,
    read_fstring_at,
)
from extract_artist_31470_shader_cache_oracle import (
    canonical_json_sha256,
    parse_static_parameter_set,
    validate_dxbc_container,
)
from extract_ue3_placements import decompress_lz4_block


SCHEMA = "lostark.artist-31470-renderer-restoration-matrix-receipt"
FORMAT_VERSION = 1
EXPECTED_SOURCE_STATIC_IDENTITY_PROJECTION_SHA256 = (
    "c2a9a801ab6162bfff40e5a3e15cdf9d8c9f2158ab6dd2a8b4f7dba0dea0b732"
)
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_SOURCE_INVENTORY = REPO_ROOT / (
    "Data/Effects/Imported/Artist/"
    "skill.31470.source-active-effect-inventory.receipt.json"
)
DEFAULT_MATERIAL_CONTRACT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.typed-material-evidence-contract.json"
)
DEFAULT_SHADER_ORACLE = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.shader-cache-oracle.receipt.json"
)
DEFAULT_MAIN_CACHE_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-ref-shader-cache.receipt.json"
)
DEFAULT_EXECUTION_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.renderer-restoration-matrix.receipt.json"
)

UNIFORM_ARRAYS = (
    "pixelVectorExpressions",
    "pixelScalarExpressions",
    "pixelTexture2DExpressions",
    "textureCubeExpressions",
    "vertexVectorExpressions",
    "vertexScalarExpressions",
    "vertexTexture2DExpressions",
    "hullVectorExpressions",
    "hullScalarExpressions",
    "hullTexture2DExpressions",
    "domainVectorExpressions",
    "domainScalarExpressions",
    "domainTexture2DExpressions",
)
RENDERER_FAMILY_COUNTS = {
    "MeshParticle": 13,
    "SpriteParticle": 16,
    "DecalParticle": 3,
    "CascadeRibbon": 1,
    "LightParticle": 1,
    "ScreenPost": 1,
}
EXPECTED_DIRECT_DEFAULT_OCCURRENCES = {
    "source-active-002",
    "source-active-003",
    "source-active-004",
    "source-active-019",
    "source-active-031",
}
DEFERRED_OCCURRENCES = {"source-active-032", "source-active-034"}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def _reject_duplicate_json_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _reject_nonfinite_json_constant(value: str) -> None:
    raise ValueError(f"non-finite JSON constant: {value}")


def read_json_strict(path: Path) -> dict[str, Any]:
    value = json.loads(
        path.read_text(encoding="utf-8-sig"),
        object_pairs_hook=_reject_duplicate_json_keys,
        parse_constant=_reject_nonfinite_json_constant,
    )
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def validate_self_digest(value: dict[str, Any], field: str, label: str) -> None:
    claimed = value.get(field)
    require(isinstance(claimed, str) and len(claimed) == 64, f"{label} seal missing")
    unsigned = copy.deepcopy(value)
    unsigned.pop(field)
    require(canonical_sha256(unsigned) == claimed, f"{label} seal mismatch")


def digest_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def tracked_text_sha256(path: Path) -> str:
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), f"BOM is forbidden: {path}")
    normalized = raw.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(normalized).hexdigest()


def seal_row(row: dict[str, Any]) -> None:
    require("rowSha256" not in row, "row is already sealed")
    row["rowSha256"] = canonical_sha256(row)


def seal_receipt(receipt: dict[str, Any]) -> None:
    require("receiptSha256" not in receipt, "receipt is already sealed")
    receipt["receiptSha256"] = canonical_sha256(receipt)


def input_identity(path: Path, value: dict[str, Any]) -> dict[str, Any]:
    identity = {
        "path": path.relative_to(REPO_ROOT).as_posix(),
        "rawSha256": digest_file(path),
        "canonicalJsonSha256": canonical_sha256(value),
    }
    for field in ("receiptSha256", "contractSha256"):
        if isinstance(value.get(field), str):
            identity[field] = value[field]
    return identity


def source_static_identity_projection(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    projection = []
    for row in rows:
        static_set = row["staticParameterSet"]
        projection.append(
            {
                "recipeId": row["recipeId"],
                "sourceMaterialPath": row["sourceMaterialPath"],
                "materialClass": row["materialIdentity"]["materialClass"],
                "staticLookupStatus": row["staticLookupStatus"],
                "baseMaterialIdHex": (
                    None if static_set is None else static_set["baseMaterialIdHex"]
                ),
                "engineEqualitySha256": (
                    None if static_set is None else static_set["engineEqualitySha256"]
                ),
            }
        )
    return projection


def build_recipe_inputs(
    material_contract: dict[str, Any], shader_oracle: dict[str, Any]
) -> list[dict[str, Any]]:
    native_by_id = {
        row["recipeId"]: row for row in shader_oracle["recipeNativeKeys"]
    }
    occurrences_by_recipe: dict[str, list[str]] = {}
    for occurrence in material_contract["occurrences"]:
        occurrences_by_recipe.setdefault(occurrence["materialRecipeId"], []).append(
            occurrence["occurrenceId"]
        )
    rows = []
    for recipe in material_contract["materialRecipes"]:
        recipe_id = recipe["recipeId"]
        native = native_by_id.get(recipe_id)
        require(native is not None, f"native recipe key is missing: {recipe_id}")
        require(
            native["sourceMaterialPath"] == recipe["sourceMaterialPath"],
            f"native recipe material differs: {recipe_id}",
        )
        static_set = native.get("staticParameterSet")
        material_class = recipe["identity"]["materialClass"].casefold()
        has_static = static_set is not None
        require(
            has_static == bool(native["hasStaticPermutationResource"]),
            f"static resource flag differs: {recipe_id}",
        )
        row = {
            "recipeId": recipe_id,
            "sourceMaterialPath": recipe["sourceMaterialPath"],
            "rendererShapes": list(recipe["rendererShapes"]),
            "arithmeticFamilyId": native["arithmeticFamilyId"],
            "materialIdentity": {
                "materialClass": material_class,
                "logicalPackage": recipe["identity"]["logicalPackage"],
                "physicalPackage": recipe["identity"]["physicalPackage"],
                "physicalPackageSha256": recipe["identity"]["physicalPackageSha256"],
                "materialObjectPath": recipe["identity"]["materialObjectPath"],
                "materialSerialSha256": recipe["identity"]["materialSerialSha256"],
            },
            "occurrenceIds": sorted(occurrences_by_recipe.get(recipe_id, [])),
            "sourceInputCounts": {
                key: len(recipe["inputs"][key])
                for key in (
                    "scalarOverrides",
                    "vectorOverrides",
                    "textureOverrides",
                    "parentDefaults",
                )
            },
            "renderStateEvidence": {
                "partialCullExact": recipe["renderState"]["partialCullExact"],
                "fullCullModeExact": recipe["renderState"]["fullCullModeExact"],
                "fullRenderStateExact": recipe["renderState"]["fullRenderStateExact"],
            },
            "staticParameterSet": None,
            "staticLookupStatus": (
                "STATIC_PARAMETER_SET_AVAILABLE"
                if has_static
                else "DIRECT_MATERIAL_IDENTITY_UNAUTHENTICATED"
            ),
        }
        if has_static:
            normalized = normalized_static_parameter_set(static_set)
            engine = engine_equivalent_static_parameter_set(static_set)
            row["staticParameterSet"] = {
                "baseMaterialIdHex": static_set["baseMaterialIdHex"],
                "rawSha256": static_set["rawSha256"],
                "serializedSemanticSha256": canonical_json_sha256(normalized),
                "engineEqualitySha256": canonical_json_sha256(engine),
                "normalized": normalized,
                "engineEquivalent": engine,
                "switchCount": len(static_set["staticSwitchParameters"]),
                "componentMaskCount": len(
                    static_set["staticComponentMaskParameters"]
                ),
                "terrainLayerWeightCount": len(
                    static_set["terrainLayerWeightParameters"]
                ),
            }
        else:
            row["staticLookupStatus"] = (
                "DIRECT_MATERIAL_HAS_NO_STATIC_PARAMETER_SET"
                if material_class == "material"
                else "MIC_NATIVE_STATIC_KEY_ABSENT"
            )
        rows.append(row)
    rows.sort(key=lambda row: row["recipeId"])
    require(len(rows) == 27, "material recipe denominator changed")
    require(
        sum(row["staticParameterSet"] is not None for row in rows) == 24,
        "static recipe denominator changed",
    )
    require(
        sum(row["staticParameterSet"] is None for row in rows) == 3,
        "direct Material recipe denominator changed",
    )
    return rows


def _public_renderer_expression(value: dict[str, Any]) -> dict[str, Any]:
    return {
        key: (
            _public_renderer_expression(item)
            if isinstance(item, dict) and "typeName" in item
            else item
        )
        for key, item in value.items()
        if key != "endOffset"
    }


def _semantic_renderer_expression(value: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {"typeName": value["typeName"]}
    for key, item in value.items():
        if key in ("typeName", "offset", "byteSize", "endOffset"):
            continue
        if isinstance(item, dict) and "typeName" in item:
            result[key] = _semantic_renderer_expression(item)
        else:
            result[key] = item
    return result


def parse_renderer_uniform_expression(
    data: bytes,
    offset: int,
    names: list[str],
    *,
    depth: int = 0,
    node_budget: list[int] | None = None,
) -> dict[str, Any]:
    """Parse UE3 uniform forms observed across all Artist F material maps.

    The pre-existing main-cache parser intentionally remains pinned to its two
    proven maps.  G01 needs the wider corpus, so this parser is local to the
    restoration matrix and cannot stale the sealed main replay receipt chain.
    """

    require(depth <= 64, "uniform expression recursion is excessive")
    if node_budget is None:
        node_budget = [0]
    node_budget[0] += 1
    require(node_budget[0] <= 8192, "uniform expression node budget is excessive")
    start = offset
    type_name, number, offset = read_fname_at(data, offset, names)
    require(number == 0, "numbered uniform-expression type is unsupported")
    folded = type_name.casefold()
    require(
        folded.startswith("fmaterialuniformexpression"),
        "uniform-expression type name is invalid",
    )
    row: dict[str, Any] = {"typeName": folded, "offset": start}

    def child(child_offset: int) -> dict[str, Any]:
        return parse_renderer_uniform_expression(
            data,
            child_offset,
            names,
            depth=depth + 1,
            node_budget=node_budget,
        )

    if folded == "fmaterialuniformexpressionfoldedmath":
        row["a"] = child(offset)
        row["b"] = child(row["a"]["endOffset"])
        offset = row["b"]["endOffset"]
        require(offset < len(data), "FoldedMath operation is truncated")
        operation_names = ("ADD", "SUB", "MUL", "DIV", "DOT")
        row["operationOrdinal"] = data[offset]
        require(
            row["operationOrdinal"] < len(operation_names),
            "FoldedMath operation is invalid",
        )
        row["operationNameIfObserved"] = operation_names[row["operationOrdinal"]]
        offset += 1
    elif folded == "fmaterialuniformexpressionappendvector":
        row["a"] = child(offset)
        row["b"] = child(row["a"]["endOffset"])
        offset = row["b"]["endOffset"]
        require(offset + 4 <= len(data), "AppendVector component count is truncated")
        row["componentsFromA"] = struct.unpack_from("<I", data, offset)[0]
        require(
            1 <= row["componentsFromA"] <= 3,
            "AppendVector component count is invalid",
        )
        offset += 4
    elif folded == "fmaterialuniformexpressionsine":
        row["input"] = child(offset)
        offset = row["input"]["endOffset"]
        require(offset + 4 <= len(data), "Sine UBOOL is truncated")
        value = struct.unpack_from("<I", data, offset)[0]
        require(value in (0, 1), "Sine UBOOL is invalid")
        row["isCosine"] = bool(value)
        offset += 4
    elif folded == "fmaterialuniformexpressionconstant":
        require(offset + 17 <= len(data), "constant uniform expression is truncated")
        row["value"] = list(struct.unpack_from("<4f", data, offset))
        require(all(math.isfinite(value) for value in row["value"]), "constant value is non-finite")
        row["valueTypeOrdinal"] = data[offset + 16]
        require(
            row["valueTypeOrdinal"] in (1, 2, 4, 8, 15),
            "constant numeric value type is invalid",
        )
        offset += 17
    elif folded == "fmaterialuniformexpressionscalarparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(name_number == 0, "numbered scalar parameter is unsupported")
        require(offset + 4 <= len(data), "scalar default is truncated")
        row["parameterName"] = name
        row["defaultValue"] = struct.unpack_from("<f", data, offset)[0]
        require(math.isfinite(row["defaultValue"]), "scalar default is non-finite")
        offset += 4
    elif folded == "fmaterialuniformexpressionvectorparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(name_number == 0, "numbered vector parameter is unsupported")
        require(offset + 16 <= len(data), "vector default is truncated")
        row["parameterName"] = name
        row["defaultValue"] = list(struct.unpack_from("<4f", data, offset))
        require(all(math.isfinite(value) for value in row["defaultValue"]), "vector default is non-finite")
        offset += 16
    elif folded in (
        "fmaterialuniformexpressiontime",
        "fmaterialuniformexpressionrealtime",
    ):
        pass
    elif folded in (
        "fmaterialuniformexpressionperiodic",
        "fmaterialuniformexpressionsquareroot",
        "fmaterialuniformexpressionlength",
        "fmaterialuniformexpressionfloor",
        "fmaterialuniformexpressionceil",
        "fmaterialuniformexpressionfrac",
        "fmaterialuniformexpressionabs",
    ):
        row["input"] = child(offset)
        offset = row["input"]["endOffset"]
    elif folded in (
        "fmaterialuniformexpressionmin",
        "fmaterialuniformexpressionmax",
        "fmaterialuniformexpressionfmod",
    ):
        row["a"] = child(offset)
        row["b"] = child(row["a"]["endOffset"])
        offset = row["b"]["endOffset"]
    elif folded == "fmaterialuniformexpressionclamp":
        row["input"] = child(offset)
        row["min"] = child(row["input"]["endOffset"])
        row["max"] = child(row["min"]["endOffset"])
        offset = row["max"]["endOffset"]
    elif folded == "fmaterialuniformexpressiontexture":
        require(offset + 4 <= len(data), "fixed texture index is truncated")
        row["referencedTextureIndex"] = struct.unpack_from("<i", data, offset)[0]
        require(row["referencedTextureIndex"] >= 0, "fixed texture index is negative")
        offset += 4
    elif folded in (
        "fmaterialuniformexpressiontextureparameter",
        "fmaterialuniformexpressionflipbooktextureparameter",
    ):
        name, name_number, offset = read_fname_at(data, offset, names)
        require(name_number == 0, "numbered texture parameter is unsupported")
        require(offset + 4 <= len(data), "texture fallback index is truncated")
        row["parameterName"] = name
        row["referencedTextureIndex"] = struct.unpack_from("<i", data, offset)[0]
        require(row["referencedTextureIndex"] >= 0, "texture fallback index is negative")
        offset += 4
    else:
        raise ValueError(f"unsupported uniform-expression type: {type_name}")

    row["byteSize"] = offset - start
    row["endOffset"] = offset
    return row


def parse_renderer_uniform_expression_set(
    data: bytes, offset: int, names: list[str]
) -> dict[str, Any]:
    start = offset
    arrays: dict[str, list[dict[str, Any]]] = {}
    node_budget = [0]
    total_top_level = 0
    for array_name in UNIFORM_ARRAYS:
        require(offset + 4 <= len(data), f"{array_name} count is truncated")
        count = struct.unpack_from("<i", data, offset)[0]
        offset += 4
        require(0 <= count <= 4096, f"{array_name} count is invalid")
        total_top_level += count
        require(total_top_level <= 4096, "uniform-expression top-level denominator is excessive")
        rows = []
        for _ in range(count):
            expression = parse_renderer_uniform_expression(
                data, offset, names, node_budget=node_budget
            )
            rows.append(_public_renderer_expression(expression))
            offset = expression["endOffset"]
        arrays[array_name] = rows
    raw = data[start:offset]
    semantic = {
        key: [_semantic_renderer_expression(row) for row in rows]
        for key, rows in arrays.items()
    }
    return {
        **arrays,
        "offset": start,
        "byteSize": offset - start,
        "rawSha256": hashlib.sha256(raw).hexdigest(),
        "semanticSha256": canonical_json_sha256(semantic),
        "endOffset": offset,
    }


def _scan_static_parameter_sets(
    package: dict[str, Any],
    code_index: dict[str, Any],
    recipe_rows: list[dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    reader = package["reader"]
    names = package["names"]
    static_rows = [row for row in recipe_rows if row["staticParameterSet"]]
    patterns: dict[str, bytes] = {}
    for row in static_rows:
        base_id = row["staticParameterSet"]["baseMaterialIdHex"]
        patterns.setdefault(base_id, bytes.fromhex(base_id))
    hits: dict[str, list[int]] = {key: [] for key in patterns}
    start = code_index["shaderCodeSectionEndLogicalOffset"] + 8
    cursor = start
    overlap = b""
    while cursor < reader.logical_size:
        payload = reader.read_logical_range(
            cursor, min(8 * 1024 * 1024, reader.logical_size - cursor)
        )
        combined = overlap + payload
        origin = cursor - len(overlap)
        for base_id, pattern in patterns.items():
            local = 0
            while True:
                found = combined.find(pattern, local)
                if found < 0:
                    break
                absolute = origin + found
                if absolute >= start:
                    hits[base_id].append(absolute)
                local = found + 1
        overlap = combined[-15:]
        cursor += len(payload)

    result: dict[str, dict[str, Any]] = {}
    for recipe in static_rows:
        target = recipe["staticParameterSet"]
        exact_rows = []
        for absolute in sorted(set(hits[target["baseMaterialIdHex"]])):
            candidate = reader.read_logical_range(
                absolute, min(64 * 1024, reader.logical_size - absolute)
            )
            try:
                static_set = parse_static_parameter_set(candidate, 0, names)
                engine_sha = canonical_json_sha256(
                    engine_equivalent_static_parameter_set(static_set)
                )
            except (ValueError, struct.error):
                continue
            if engine_sha != target["engineEqualitySha256"]:
                continue
            suffix = None
            map_context = False
            if static_set["endOffset"] + 20 <= len(candidate):
                suffix = list(
                    struct.unpack_from("<IIIII", candidate, static_set["endOffset"])
                )
                map_context = bool(
                    suffix[0] == 868
                    and suffix[1] == 16
                    and absolute < suffix[2] <= reader.logical_size
                    and suffix[3] == 0
                    and 0 < suffix[4] <= 128
                )
            exact_rows.append(
                {
                    "logicalOffset": absolute,
                    "rawSha256": static_set["rawSha256"],
                    "engineEqualitySha256": engine_sha,
                    "suffixU32": suffix,
                    "mapContext": map_context,
                }
            )
        map_rows = [row for row in exact_rows if row["mapContext"]]
        if len(map_rows) == 1:
            status = "EXACT_ONE_MAP_CONTEXT"
            map_row = map_rows[0]
        elif not map_rows:
            status = "MAP_CONTEXT_MISSING"
            map_row = None
        else:
            status = "MAP_CONTEXT_AMBIGUOUS"
            map_row = None
        result[recipe["recipeId"]] = {
            "baseMaterialIdRawHitCount": len(
                set(hits[target["baseMaterialIdHex"]])
            ),
            "engineEquivalentStaticSetOccurrenceCount": len(exact_rows),
            "exactMapContextCount": len(map_rows),
            "mapContextStatus": status,
            "mapRow": map_row,
            "mapContextLogicalOffsets": [row["logicalOffset"] for row in map_rows],
        }
    return result


def classify_shader_pass(shader_type: str) -> str:
    value = shader_type.casefold()
    for marker, label in (
        ("basepass", "BASE_PASS"),
        ("distortion", "DISTORTION"),
        ("depthonly", "DEPTH_ONLY"),
        ("shadowdepth", "SHADOW_DEPTH"),
        ("velocity", "VELOCITY"),
        ("hitproxy", "HIT_PROXY"),
        ("light", "LIGHTING"),
    ):
        if marker in value:
            return label
    return "UNCLASSIFIED_SHADER_TYPE"


def enumerate_dxbc_containers(payload: bytes) -> list[dict[str, Any]]:
    containers = []
    cursor = 0
    occupied_end = 0
    while True:
        offset = payload.find(b"DXBC", cursor)
        if offset < 0:
            break
        cursor = offset + 1
        if offset < occupied_end or offset + 28 > len(payload):
            continue
        declared_size = struct.unpack_from("<I", payload, offset + 24)[0]
        if declared_size < 36 or offset + declared_size > len(payload):
            continue
        bytecode = payload[offset : offset + declared_size]
        try:
            container = validate_dxbc_container(bytecode)
        except ValueError:
            continue
        occupied_end = offset + declared_size
        containers.append(
            {
                "offsetInDecompressedRecord": offset,
                "byteSize": declared_size,
                "dxbcSha256": container["sha256"],
                "chunkFourCc": [row["fourCc"] for row in container["chunks"]],
            }
        )
    return containers


def resolve_code_candidate(
    package: dict[str, Any], descriptor: dict[str, Any]
) -> dict[str, Any]:
    position = descriptor["codePosition"]
    base = {
        "codeIndexCandidate": descriptor["codeIndexCandidate"],
        "opaqueDescriptorTailU32": descriptor["opaqueDescriptorTailU32"],
    }
    if position is None:
        return {
            **base,
            "status": "UNRESOLVED_CODE_MAPPING",
            "candidatePosition": None,
            "dxbcContainers": [],
        }
    reader = package["reader"]
    compressed = reader.read_logical_range(
        position["compressedLogicalOffset"], position["compressedByteSize"]
    )
    payload = decompress_lz4_block(compressed, position["uncompressedByteSize"])
    containers = enumerate_dxbc_containers(payload)
    if len(containers) == 1:
        status = "CANDIDATE_SINGLE_DXBC"
    elif len(containers) > 1:
        status = "CANDIDATE_DXBC_BUNDLE"
    else:
        status = "CANDIDATE_PAYLOAD_WITHOUT_DXBC_CONTAINER"
    return {
        **base,
        "status": status,
        "candidatePosition": {
            key: position[key]
            for key in (
                "codeIndex",
                "compressedLogicalOffset",
                "compressedByteSize",
                "uncompressedByteSize",
            )
        },
        "compressedSha256": hashlib.sha256(compressed).hexdigest(),
        "decompressedSha256": hashlib.sha256(payload).hexdigest(),
        "dxbcContainers": containers,
    }


def parse_material_map_candidate(
    package: dict[str, Any],
    code_index: dict[str, Any],
    recipe: dict[str, Any],
    search: dict[str, Any],
) -> dict[str, Any]:
    reader = package["reader"]
    names = package["names"]
    start = search["mapRow"]["logicalOffset"]
    end = search["mapRow"]["suffixU32"][2]
    data = reader.read_logical_range(start, end - start)
    static_set = parse_static_parameter_set(data, 0, names)
    require(
        canonical_json_sha256(engine_equivalent_static_parameter_set(static_set))
        == recipe["staticParameterSet"]["engineEqualitySha256"],
        f"map static equality differs: {recipe['recipeId']}",
    )
    offset = static_set["endOffset"]
    suffix = list(struct.unpack_from("<IIIII", data, offset))
    offset += 20
    require(
        suffix[:4] == [868, 16, end, 0] and 0 < suffix[4] <= 128,
        f"map suffix differs: {recipe['recipeId']}",
    )
    vertex_factories = []
    referenced_ids = []
    for vf_index in range(suffix[4]):
        require(offset + 4 <= len(data), "VF shader count is truncated")
        count = struct.unpack_from("<I", data, offset)[0]
        offset += 4
        require(0 < count <= 256, "VF shader count is invalid")
        references = []
        for reference_index in range(count):
            shader_type, number, _ = read_fname_at(data, offset, names)
            require(number == 0, "numbered shader type is unsupported")
            shader_id = data[offset + 8 : offset + 24].hex()
            repeated_type, repeated_number, _ = read_fname_at(
                data, offset + 24, names
            )
            require(
                repeated_number == 0 and repeated_type == shader_type,
                "shader type repeat differs",
            )
            descriptor = code_index["descriptorById"].get(shader_id)
            require(descriptor is not None, "map references unknown shader ID")
            require(
                descriptor["shaderType"] == shader_type,
                "descriptor shader type differs",
            )
            references.append(
                {
                    "referenceIndex": reference_index,
                    "shaderType": shader_type,
                    "passFamilyCandidate": classify_shader_pass(shader_type),
                    "shaderIdHex": shader_id,
                    "descriptorGroupIndex": descriptor["groupIndex"],
                    "descriptorIndex": descriptor["descriptorIndex"],
                }
            )
            referenced_ids.append(shader_id)
            offset += 32
        vertex_factory, number, offset = read_fname_at(data, offset, names)
        require(number == 0, "numbered vertex factory is unsupported")
        vertex_factories.append(
            {
                "vertexFactoryIndex": vf_index,
                "vertexFactoryType": vertex_factory,
                "selectionAdmission": False,
                "shaderReferences": references,
            }
        )
    require(offset + 16 <= len(data), "map opaque identity is truncated")
    opaque_identity = data[offset : offset + 16]
    offset += 16
    friendly_name, offset = read_fstring_at(data, offset)
    repeated_set = parse_static_parameter_set(data, offset, names)
    require(
        repeated_set["rawSha256"] == static_set["rawSha256"],
        "repeated static set differs",
    )
    offset = repeated_set["endOffset"]
    uniform_set = parse_renderer_uniform_expression_set(data, offset, names)
    offset = uniform_set["endOffset"]
    require(offset + 4 == len(data), "map uniform trailer size differs")
    require(
        struct.unpack_from("<I", data, offset)[0] == code_index["platform"],
        "map trailer platform differs",
    )
    code_candidates = {}
    for shader_id in sorted(set(referenced_ids)):
        code_candidates[shader_id] = resolve_code_candidate(
            package, code_index["descriptorById"][shader_id]
        )
    code_status_counts = Counter(
        row["status"] for row in code_candidates.values()
    )
    for vertex_factory in vertex_factories:
        for reference in vertex_factory["shaderReferences"]:
            candidate = code_candidates[reference["shaderIdHex"]]
            reference["codeMappingStatus"] = candidate["status"]
            reference["descriptorTailCodeIndexAdmission"] = False
            reference["dxbcContainerCount"] = len(candidate["dxbcContainers"])
    uniform_projection = {name: uniform_set[name] for name in UNIFORM_ARRAYS}
    map_projection = {
        "staticParameterSet": normalized_static_parameter_set(static_set),
        "engineEqualityStaticParameterSet": engine_equivalent_static_parameter_set(
            static_set
        ),
        "suffixVersion": suffix[:2],
        "vertexFactories": [
            {
                "vertexFactoryType": row["vertexFactoryType"],
                "shaderReferences": [
                    {
                        "shaderType": reference["shaderType"],
                        "shaderIdHex": reference["shaderIdHex"],
                    }
                    for reference in row["shaderReferences"]
                ],
            }
            for row in vertex_factories
        ],
        "opaqueIdentityHex": opaque_identity.hex(),
        "friendlyName": friendly_name,
        "uniformExpressionSemanticSha256": uniform_set["semanticSha256"],
        "trailerPlatform": code_index["platform"],
    }
    return {
        "logicalOffset": start,
        "logicalEndOffset": end,
        "byteSize": len(data),
        "rawSha256": hashlib.sha256(data).hexdigest(),
        "friendlyName": friendly_name,
        "opaqueIdentityHex": opaque_identity.hex(),
        "vertexFactoryCount": len(vertex_factories),
        "shaderReferenceCount": len(referenced_ids),
        "uniqueShaderObjectCount": len(code_candidates),
        "vertexFactories": vertex_factories,
        "uniformExpressionCounts": {
            name: len(uniform_set[name]) for name in UNIFORM_ARRAYS
        },
        "uniformExpressionProjectionSha256": canonical_json_sha256(
            uniform_projection
        ),
        "codeMappingStatusCounts": dict(sorted(code_status_counts.items())),
        "codeCandidates": code_candidates,
        "constantTextureSamplerRegisterClosure": {
            "status": "UNRESOLVED_PENDING_NATIVE_SHADER_OBJECT_BINDING",
            "selectionAdmission": False,
        },
        "semanticMapSha256": canonical_json_sha256(map_projection),
    }


def validate_cache_identity(
    identity: dict[str, Any], expected: dict[str, Any]
) -> None:
    checks = {
        "fileName": expected["fileName"],
        "physicalByteSize": expected["byteSize"],
        "rawSha256": expected["sha256"],
        "logicalByteSize": expected["logicalByteSize"],
        "nameCount": expected["nameCount"],
        "importCount": expected["importCount"],
        "exportCount": expected["exportCount"],
        "serialOffset": expected["serialOffset"],
        "serialSize": expected["serialSize"],
        "packageGuidHex": expected["packageGuidHex"],
    }
    require(
        all(identity.get(key) == value for key, value in checks.items()),
        f"cache identity differs: {identity.get('fileName')}",
    )


def extract_cache_cohort(
    cohort_id: str,
    path: Path,
    expected: dict[str, Any],
    recipe_rows: list[dict[str, Any]],
) -> dict[str, Any]:
    require(path.is_file(), f"RefShaderCache is missing: {path}")
    package = package_tables(path)
    validate_cache_identity(package["identity"], expected)
    code_index = parse_cache_code_index(package)
    require(
        code_index["descriptorCount"] == expected["descriptorCount"]
        and code_index["embeddedCodeCount"] == expected["embeddedCodeCount"]
        and code_index["shaderCodeSectionEndLogicalOffset"]
        == expected["shaderCodeSectionEndLogicalOffset"],
        f"cache code denominator differs: {cohort_id}",
    )
    searches = _scan_static_parameter_sets(package, code_index, recipe_rows)
    maps = []
    for recipe in recipe_rows:
        if recipe["staticParameterSet"] is None:
            continue
        search = searches[recipe["recipeId"]]
        map_candidate = None
        parse_status = search["mapContextStatus"]
        parse_error = None
        if parse_status == "EXACT_ONE_MAP_CONTEXT":
            try:
                map_candidate = parse_material_map_candidate(
                    package, code_index, recipe, search
                )
                parse_status = "MAP_CONTEXT_PARSED"
            except (ValueError, struct.error) as error:
                parse_status = "MAP_CONTEXT_PARSE_BLOCK"
                parse_error = f"{type(error).__name__}: {error}"
        maps.append(
            {
                "recipeId": recipe["recipeId"],
                "search": search,
                "parseStatus": parse_status,
                "parseError": parse_error,
                "materialMapCandidate": map_candidate,
            }
        )
    public_groups = [
        {
            key: value
            for key, value in row.items()
            if key
            in {
                "groupIndex",
                "shaderType",
                "descriptorCount",
                "embeddedCodeCount",
                "mappedDescriptorCount",
                "descriptorTableSha256",
                "codeHeaderProjectionSha256",
            }
        }
        for row in code_index["groups"]
    ]
    return {
        "cohortId": cohort_id,
        "cacheIdentity": package["identity"],
        "codeIndexSummary": {
            "platform": code_index["platform"],
            "groupCount": code_index["groupCount"],
            "descriptorCount": code_index["descriptorCount"],
            "embeddedCodeCount": code_index["embeddedCodeCount"],
            "shaderObjectCount": code_index["shaderObjectCount"],
            "shaderCodeSectionEndLogicalOffset": code_index[
                "shaderCodeSectionEndLogicalOffset"
            ],
            "groupsProjectionSha256": canonical_json_sha256(public_groups),
        },
        "searchedRecipeCount": len(maps),
        "joinedRecipeCount": sum(
            row["materialMapCandidate"] is not None for row in maps
        ),
        "maps": maps,
    }


def type_data_evidence(execution_row: dict[str, Any]) -> list[dict[str, Any]]:
    rows = []
    for module in execution_row["modules"]:
        class_name = str(module["exactSourceClass"])
        if "typedata" not in class_name.casefold():
            continue
        rows.append(
            {
                "moduleOccurrenceId": module["moduleOccurrenceId"],
                "exactSourceClass": class_name,
                "sourceObjectId": module["sourceObjectId"],
                "sourceRecordSha256": module["sourceRecordSha256"],
                "sourceDecision": module["decision"],
                "sourceBlockers": list(module["blockers"]),
            }
        )
    return rows


def build_occurrences(
    source_inventory: dict[str, Any],
    material_contract: dict[str, Any],
    execution_receipt: dict[str, Any],
    recipe_coverage: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    material_by_occurrence = {
        row["occurrenceId"]: row for row in material_contract["occurrences"]
    }
    execution_by_occurrence = {
        row["evidenceId"]: row for row in execution_receipt["occurrences"]
    }
    rows = []
    for element in source_inventory["activeElements"]:
        occurrence_id = element["activeElementId"]
        material = material_by_occurrence.get(occurrence_id)
        execution = execution_by_occurrence.get(occurrence_id)
        require(execution is not None, f"execution occurrence is missing: {occurrence_id}")
        require(
            execution["rendererType"] == element["rendererType"],
            f"execution renderer differs: {occurrence_id}",
        )
        type_data = type_data_evidence(execution)
        blockers = ["NATIVE_RENDERER_SELECTION_NOT_ADMITTED"]
        recipe_id = None
        if material is None:
            require(
                element["rendererType"] == "LightParticle",
                f"material occurrence unexpectedly absent: {occurrence_id}",
            )
            disposition = "RECONSTRUCTED_ONLY"
            blockers.extend(
                [
                    "ENGINE_BUILTIN_PARTICLE_MATERIAL",
                    "POINT_LIGHT_CPU_CONSUMER_DEFERRED",
                ]
            )
            binding = None
        else:
            recipe_id = material["materialRecipeId"]
            coverage = recipe_coverage[recipe_id]
            disposition = coverage["disposition"]
            blockers.extend(coverage["blockers"])
            binding = {
                "sourceInputCounts": coverage["sourceInputCounts"],
                "registerClosureStatus": coverage[
                    "registerClosureStatus"
                ],
            }
        if occurrence_id in DEFERRED_OCCURRENCES:
            blockers.append("USER_PRIORITY_DEFERRED_LIGHT_OR_SCREEN_POST")
        row = {
            "occurrenceId": occurrence_id,
            "cueId": element["cueId"],
            "rendererFamily": element["rendererType"],
            "sourceSystemId": element["sourceSystemId"],
            "sourceEmitter": element["sourceEmitter"],
            "sourceEmitterNodeId": element["sourceEmitterNode"]["nodeId"],
            "sourceEmitterRecordSha256": element["sourceEmitterNode"][
                "recordSha256"
            ],
            "sourceMaterialPath": (
                material["sourceMaterialPath"] if material else None
            ),
            "recipeId": recipe_id,
            "typeDataEvidence": type_data,
            "nativeSelectionEvidenceStatus": (
                "SOURCE_TYPEDATA_PRESENT_SELECTION_NOT_ADMITTED"
                if type_data
                else "SOURCE_DEFAULT_RENDERER_SELECTION_NOT_ADMITTED"
            ),
            "bindingClosure": binding,
            "deferredByUserPriority": occurrence_id in DEFERRED_OCCURRENCES,
            "disposition": disposition,
            "selectedVfPassAdmission": False,
            "productAdmission": False,
            "blockers": sorted(set(blockers)),
        }
        seal_row(row)
        rows.append(row)
    rows.sort(key=lambda row: row["occurrenceId"])
    return rows


def project_code_candidate(shader_id: str, candidate: dict[str, Any]) -> dict[str, Any]:
    return {
        "shaderIdHex": shader_id,
        "codeIndexCandidate": candidate["codeIndexCandidate"],
        "opaqueDescriptorTailU32": candidate["opaqueDescriptorTailU32"],
        "descriptorTailCodeIndexAdmission": False,
        "status": candidate["status"],
        "candidatePosition": candidate["candidatePosition"],
        "compressedSha256": candidate.get("compressedSha256"),
        "decompressedSha256": candidate.get("decompressedSha256"),
        "dxbcContainers": copy.deepcopy(candidate["dxbcContainers"]),
    }


def combine_recipe_coverage(
    recipe_rows: list[dict[str, Any]],
    official: dict[str, Any],
    installed: dict[str, Any],
) -> list[dict[str, Any]]:
    official_by_id = {row["recipeId"]: row for row in official["maps"]}
    installed_by_id = {row["recipeId"]: row for row in installed["maps"]}
    coverage = []
    for recipe in recipe_rows:
        recipe_id = recipe["recipeId"]
        blockers = ["NATIVE_RENDERER_SELECTION_NOT_ADMITTED"]
        cache_joins = []
        if recipe["staticParameterSet"] is None:
            disposition = "RECONSTRUCTED_ONLY"
            blockers.append("STATIC_PARAMETER_SET_IDENTITY_UNAUTHENTICATED")
            blockers.append(recipe["staticLookupStatus"])
            register_status = "UNAVAILABLE_WITHOUT_AUTHENTICATED_SHADER_MAP"
        else:
            official_source = official_by_id.get(recipe_id)
            installed_source = installed_by_id.get(recipe_id)
            require(official_source is not None, f"official search row is missing: {recipe_id}")
            require(installed_source is not None, f"installed search row is missing: {recipe_id}")
            official_map = official_source["materialMapCandidate"]
            if official_map is None:
                disposition = "CACHE_MISSING_BLOCK"
                register_status = "UNAVAILABLE_WITHOUT_OFFICIAL_SHADER_MAP"
                blockers.append(
                    "OFFICIAL_" + official_source["parseStatus"]
                )
            else:
                has_official_dxbc = any(
                    candidate["dxbcContainers"]
                    for candidate in official_map["codeCandidates"].values()
                )
                if has_official_dxbc:
                    disposition = "JOINED_CANDIDATE"
                    register_status = "UNRESOLVED_PENDING_NATIVE_SHADER_OBJECT_BINDING"
                else:
                    disposition = "CODE_MISSING_BLOCK"
                    register_status = "UNAVAILABLE_WITHOUT_OFFICIAL_DXBC_CANDIDATE"
                    blockers.append("OFFICIAL_DXBC_CONTAINER_CANDIDATE_ABSENT")
            for cohort_id, source, authority in (
                (
                    official["cohortId"],
                    official_source,
                    "SAME_DISTRIBUTION_SOURCE_CANDIDATE",
                ),
                (
                    installed["cohortId"],
                    installed_source,
                    "CROSS_REVISION_CORROBORATION",
                ),
            ):
                material_map = source["materialMapCandidate"]
                if material_map is None:
                    blockers.append(
                        f"{cohort_id}_{source['parseStatus']}"
                    )
                    continue
                statuses = material_map["codeMappingStatusCounts"]
                if statuses.get("UNRESOLVED_CODE_MAPPING", 0):
                    blockers.append("UNRESOLVED_CODE_MAPPING")
                if statuses.get("CANDIDATE_DXBC_BUNDLE", 0):
                    blockers.append("MULTI_DXBC_BUNDLE_REQUIRES_CODE_TABLE_DECODE")
                if statuses.get("CANDIDATE_PAYLOAD_WITHOUT_DXBC_CONTAINER", 0):
                    blockers.append("CANDIDATE_PAYLOAD_WITHOUT_DXBC_CONTAINER")
                cache_joins.append(
                    {
                        "cohortId": cohort_id,
                        "authority": authority,
                        "exactMapContextCount": source["search"][
                            "exactMapContextCount"
                        ],
                        "mapContextStatus": source["search"]["mapContextStatus"],
                        "mapParseStatus": source["parseStatus"],
                        "materialMapLogicalOffset": material_map["logicalOffset"],
                        "materialMapLogicalEndOffset": material_map[
                            "logicalEndOffset"
                        ],
                        "materialMapByteSize": material_map["byteSize"],
                        "materialMapRawSha256": material_map["rawSha256"],
                        "semanticMapSha256": material_map["semanticMapSha256"],
                        "vertexFactoryCount": material_map["vertexFactoryCount"],
                        "shaderReferenceCount": material_map[
                            "shaderReferenceCount"
                        ],
                        "uniformExpressionCounts": material_map[
                            "uniformExpressionCounts"
                        ],
                        "codeMappingStatusCounts": statuses,
                        "shaderObjectCandidates": [
                            project_code_candidate(shader_id, candidate)
                            for shader_id, candidate in sorted(
                                material_map["codeCandidates"].items()
                            )
                        ],
                        "candidateVertexFactories": [
                            {
                                "vertexFactoryType": row["vertexFactoryType"],
                                "shaderReferences": row["shaderReferences"],
                                "selectionAdmission": False,
                            }
                            for row in material_map["vertexFactories"]
                        ],
                    }
                )
        row = {
            **copy.deepcopy(recipe),
            "cacheJoins": cache_joins,
            "registerClosureStatus": register_status,
            "disposition": disposition,
            "selectedVfPassAdmission": False,
            "productAdmission": False,
            "blockers": sorted(set(blockers)),
        }
        seal_row(row)
        coverage.append(row)
    return coverage


def _vertex_factory_projection(material_map: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        {
            "vertexFactoryType": row["vertexFactoryType"],
            "shaderReferences": [
                {
                    "shaderType": reference["shaderType"],
                    "shaderIdHex": reference["shaderIdHex"],
                }
                for reference in row["shaderReferences"]
            ],
        }
        for row in material_map["vertexFactories"]
    ]


def validate_main_golden(
    main_cache_receipt: dict[str, Any],
    official: dict[str, Any],
    installed: dict[str, Any],
) -> dict[str, Any]:
    rows = []
    expected_recipes = {
        "material-recipe-03cc03b86c1a4c8f",
        "material-recipe-daf220acad2b656e",
    }
    expected_occurrences = {
        "material-recipe-03cc03b86c1a4c8f": [
            "source-active-009",
            "source-active-010",
        ],
        "material-recipe-daf220acad2b656e": ["source-active-011"],
    }
    decision = main_cache_receipt["decision"]
    require(
        decision["exactSameDistributionCohortShaderMapJoinCount"] == 2
        and decision["targetOccurrenceCount"] == 3
        and decision["exactLocalVfBasePassPixelDxbcCount"] == 2
        and decision["exactLocalVfBasePassVertexPermutationCount"] == 4
        and decision["textureRegisterBindingCount"] == 7
        and decision["meshParticleToLocalVfNativeCallChainAdmission"] is False
        and decision["occurrenceSelectedLocalVfRuntimeAdmission"] is False,
        "main golden decision boundary changed",
    )
    for receipt_key, cohort in (
        ("officialRefShaderCacheV974", official),
        ("currentInstalledRefShaderCache", installed),
    ):
        matrix_by_recipe = {row["recipeId"]: row for row in cohort["maps"]}
        golden_targets = main_cache_receipt[receipt_key]["mainTargets"]
        require(
            {row["recipeId"] for row in golden_targets} == expected_recipes,
            f"main golden target identity changed: {receipt_key}",
        )
        for golden in golden_targets:
            recipe_id = golden["recipeId"]
            require(
                golden["occurrenceIds"] == expected_occurrences[recipe_id],
                f"main golden occurrence identity changed: {receipt_key}/{recipe_id}",
            )
            matrix_row = matrix_by_recipe.get(recipe_id)
            require(matrix_row is not None, f"main golden search row is absent: {recipe_id}")
            material_map = matrix_row["materialMapCandidate"]
            require(material_map is not None, f"main golden map is absent: {recipe_id}")
            golden_map = golden["materialMap"]
            for field in (
                "logicalOffset",
                "logicalEndOffset",
                "byteSize",
                "rawSha256",
                "semanticMapSha256",
            ):
                require(
                    material_map[field] == golden_map[field],
                    f"main golden map {field} differs: {receipt_key}/{recipe_id}",
                )
            require(
                _vertex_factory_projection(material_map)
                == _vertex_factory_projection(golden_map),
                f"main golden VF/shader projection differs: {receipt_key}/{recipe_id}",
            )
            expected_uniform_counts = {
                name: len(golden_map["uniformExpressionSet"][name])
                for name in UNIFORM_ARRAYS
            }
            require(
                material_map["uniformExpressionCounts"] == expected_uniform_counts,
                f"main golden uniform counts differ: {receipt_key}/{recipe_id}",
            )
            selected_rows = []
            for selected in golden_map["selectedOriginalDxbc"]:
                candidate = material_map["codeCandidates"].get(
                    selected["shaderIdHex"]
                )
                require(
                    candidate is not None and candidate["candidatePosition"] is not None,
                    f"main golden code candidate is absent: {receipt_key}/{recipe_id}",
                )
                position = candidate["candidatePosition"]
                require(
                    position["compressedLogicalOffset"]
                    == selected["compressedLogicalOffset"]
                    and position["compressedByteSize"]
                    == selected["compressedByteSize"]
                    and position["uncompressedByteSize"]
                    == selected["uncompressedByteSize"]
                    and candidate["compressedSha256"]
                    == selected["compressedSha256"],
                    f"main golden compressed code differs: {receipt_key}/{recipe_id}",
                )
                matching_dxbc = [
                    row
                    for row in candidate["dxbcContainers"]
                    if row["dxbcSha256"] == selected["dxbcSha256"]
                ]
                require(
                    len(matching_dxbc) == 1,
                    f"main golden DXBC differs: {receipt_key}/{recipe_id}",
                )
                selected_rows.append(
                    {
                        "shaderType": selected["shaderType"],
                        "shaderIdHex": selected["shaderIdHex"],
                        "dxbcSha256": selected["dxbcSha256"],
                    }
                )
            rows.append(
                {
                    "cohortId": cohort["cohortId"],
                    "recipeId": recipe_id,
                    "occurrenceIds": list(golden["occurrenceIds"]),
                    "materialMapRawSha256": material_map["rawSha256"],
                    "semanticMapSha256": material_map["semanticMapSha256"],
                    "selectedOriginalDxbc": selected_rows,
                    "textureRegisterBindingCount": len(
                        golden["textureRegisterBindings"]
                    ),
                    "textureRegisterBindingsSha256": canonical_sha256(
                        golden["textureRegisterBindings"]
                    ),
                    "constantBufferBindingsSha256": canonical_sha256(
                        golden["constantBufferBindings"]
                    ),
                    "constantBufferBindingStatus": "PRESERVED_FROM_MAIN_GOLDEN_RECEIPT",
                }
            )
    return {
        "status": "MAIN_2_RECIPE_3_OCCURRENCE_GOLDEN_MATCH",
        "rowCount": len(rows),
        "rows": rows,
    }


def build_receipt(
    source_inventory_path: Path,
    material_contract_path: Path,
    shader_oracle_path: Path,
    main_cache_receipt_path: Path,
    execution_receipt_path: Path,
    official_cache_path: Path,
    installed_cache_path: Path,
) -> dict[str, Any]:
    source_inventory = read_json_strict(source_inventory_path)
    material_contract = read_json_strict(material_contract_path)
    shader_oracle = read_json_strict(shader_oracle_path)
    main_cache_receipt = read_json_strict(main_cache_receipt_path)
    execution_receipt = read_json_strict(execution_receipt_path)
    require(
        source_inventory["schema"]
        == "lostark.source-active-effect-inventory-receipt",
        "source inventory schema changed",
    )
    require(
        material_contract["schema"]
        == "lostark.artist-31470-typed-material-evidence-contract",
        "material contract schema changed",
    )
    require(
        shader_oracle["schema"]
        == "lostark.artist-31470-shader-cache-oracle-receipt",
        "shader oracle schema changed",
    )
    require(
        main_cache_receipt["schema"]
        == "lostark.artist-31470-main-ref-shader-cache-receipt",
        "main cache receipt schema changed",
    )
    require(
        execution_receipt["schema"]
        == "lostark.effect-source-execution-semantics",
        "execution receipt schema changed",
    )
    validate_self_digest(material_contract, "contractSha256", "material contract")
    validate_self_digest(shader_oracle, "receiptSha256", "shader oracle")
    validate_self_digest(main_cache_receipt, "receiptSha256", "main cache receipt")
    validate_self_digest(execution_receipt, "receiptSha256", "execution receipt")
    source_ids = [row["activeElementId"] for row in source_inventory["activeElements"]]
    material_ids = [row["occurrenceId"] for row in material_contract["occurrences"]]
    execution_ids = [row["evidenceId"] for row in execution_receipt["occurrences"]]
    require(
        source_ids == [f"source-active-{index:03d}" for index in range(35)],
        "source occurrence identity changed",
    )
    require(
        sorted(material_ids)
        == [f"source-active-{index:03d}" for index in range(34)]
        and len(set(material_ids)) == 34,
        "material occurrence identity changed",
    )
    require(
        sorted(execution_ids)
        == [f"source-active-{index:03d}" for index in range(35)]
        and len(set(execution_ids)) == 35,
        "execution occurrence identity changed",
    )
    require(
        len(material_contract["materialRecipes"]) == 27
        and len({row["recipeId"] for row in material_contract["materialRecipes"]})
        == 27,
        "material recipe identity changed",
    )
    recipes = build_recipe_inputs(material_contract, shader_oracle)
    source_identity_sha = canonical_sha256(source_static_identity_projection(recipes))
    require(
        source_identity_sha == EXPECTED_SOURCE_STATIC_IDENTITY_PROJECTION_SHA256,
        "source static identity projection changed",
    )
    official = extract_cache_cohort(
        "OFFICIAL_SAME_DISTRIBUTION_V974",
        official_cache_path,
        EXPECTED_OFFICIAL_CACHE,
        recipes,
    )
    gc.collect()
    installed = extract_cache_cohort(
        "CURRENT_INSTALLED_CACHE",
        installed_cache_path,
        EXPECTED_INSTALLED_CACHE,
        recipes,
    )
    main_golden = validate_main_golden(
        main_cache_receipt, official, installed
    )
    coverage = combine_recipe_coverage(recipes, official, installed)
    coverage_by_id = {row["recipeId"]: row for row in coverage}
    occurrences = build_occurrences(
        source_inventory, material_contract, execution_receipt, coverage_by_id
    )
    dispositions = Counter(row["disposition"] for row in occurrences)
    recipe_dispositions = Counter(row["disposition"] for row in coverage)
    code_statuses = Counter()
    for cohort in (official, installed):
        for map_row in cohort["maps"]:
            material_map = map_row["materialMapCandidate"]
            if material_map is not None:
                code_statuses.update(material_map["codeMappingStatusCounts"])
    cache_joins = [
        join for row in coverage for join in row["cacheJoins"]
    ]
    shader_candidates = [
        candidate
        for join in cache_joins
        for candidate in join["shaderObjectCandidates"]
    ]
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": {
            "activeOccurrenceCount": 35,
            "renderedMaterialOccurrenceCount": 34,
            "materialRecipeCount": 27,
            "coreRendererMilestone": "33/35",
            "deferredOccurrenceIds": sorted(DEFERRED_OCCURRENCES),
        },
        "toolIdentity": {
            "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
            "trackedTextSha256": tracked_text_sha256(SCRIPT_PATH),
        },
        "inputs": [
            input_identity(source_inventory_path, source_inventory),
            input_identity(material_contract_path, material_contract),
            input_identity(shader_oracle_path, shader_oracle),
            input_identity(main_cache_receipt_path, main_cache_receipt),
            input_identity(execution_receipt_path, execution_receipt),
        ],
        "cacheCohorts": [
            {
                key: value
                for key, value in cohort.items()
                if key != "maps"
            }
            for cohort in (official, installed)
        ],
        "sourceStaticIdentityProjectionSha256": source_identity_sha,
        "mainGoldenCrossCheck": main_golden,
        "recipeCoverage": coverage,
        "occurrences": occurrences,
        "summary": {
            "activeOccurrenceCount": len(occurrences),
            "materialOccurrenceCount": sum(
                row["recipeId"] is not None for row in occurrences
            ),
            "engineBuiltinOccurrenceCount": sum(
                row["recipeId"] is None for row in occurrences
            ),
            "materialRecipeCount": len(coverage),
            "staticRecipeCount": sum(
                row["staticParameterSet"] is not None for row in coverage
            ),
            "directDefaultRecipeCount": sum(
                row["staticParameterSet"] is None for row in coverage
            ),
            "directMaterialRecipeCount": sum(
                row["materialIdentity"]["materialClass"] == "material"
                for row in coverage
            ),
            "micWithoutNativeStaticKeyRecipeCount": sum(
                row["staticLookupStatus"] == "MIC_NATIVE_STATIC_KEY_ABSENT"
                for row in coverage
            ),
            "staticRecipeOccurrenceCount": sum(
                row["recipeId"] is not None
                and coverage_by_id[row["recipeId"]]["staticParameterSet"]
                is not None
                for row in occurrences
            ),
            "directDefaultOccurrenceCount": sum(
                row["recipeId"] is not None
                and coverage_by_id[row["recipeId"]]["staticParameterSet"]
                is None
                for row in occurrences
            ),
            "directMaterialOccurrenceCount": sum(
                row["recipeId"] is not None
                and coverage_by_id[row["recipeId"]]["materialIdentity"][
                    "materialClass"
                ]
                == "material"
                for row in occurrences
            ),
            "micWithoutNativeStaticKeyOccurrenceCount": sum(
                row["recipeId"] is not None
                and coverage_by_id[row["recipeId"]]["staticLookupStatus"]
                == "MIC_NATIVE_STATIC_KEY_ABSENT"
                for row in occurrences
            ),
            "rendererFamilyCounts": dict(
                sorted(Counter(row["rendererFamily"] for row in occurrences).items())
            ),
            "recipeDispositionCounts": dict(sorted(recipe_dispositions.items())),
            "occurrenceDispositionCounts": dict(sorted(dispositions.items())),
            "cacheJoinCount": len(cache_joins),
            "officialSameDistributionJoinCount": sum(
                join["authority"] == "SAME_DISTRIBUTION_SOURCE_CANDIDATE"
                for join in cache_joins
            ),
            "installedCrossRevisionJoinCount": sum(
                join["authority"] == "CROSS_REVISION_CORROBORATION"
                for join in cache_joins
            ),
            "shaderReferenceCountAcrossCohorts": sum(
                sum(
                    len(vertex_factory["shaderReferences"])
                    for vertex_factory in join["candidateVertexFactories"]
                )
                for join in cache_joins
            ),
            "shaderObjectCandidateCountAcrossCohorts": len(shader_candidates),
            "dxbcContainerCandidateCountAcrossCohorts": sum(
                len(candidate["dxbcContainers"])
                for candidate in shader_candidates
            ),
            "descriptorTailCodeIndexAdmissionCount": sum(
                candidate["descriptorTailCodeIndexAdmission"]
                for candidate in shader_candidates
            ),
            "selectedVfPassAdmissionCount": sum(
                row["selectedVfPassAdmission"] for row in occurrences
            ),
            "codeMappingStatusCountsAcrossCohorts": dict(
                sorted(code_statuses.items())
            ),
            "cacheMissingBlockCount": sum(
                row["disposition"] == "CACHE_MISSING_BLOCK" for row in coverage
            ),
            "codeMissingBlockCount": sum(
                row["disposition"] == "CODE_MISSING_BLOCK" for row in coverage
            ),
            "productAdmissionCount": 0,
        },
        "admission": {
            "structuralStaticSetJoin": sum(
                join["authority"] == "SAME_DISTRIBUTION_SOURCE_CANDIDATE"
                for join in cache_joins
            )
            == 24,
            "descriptorTailIsNativeCodeIndex": False,
            "candidateVfPassIsSelected": False,
            "nativeRendererSelection": False,
            "runtimeExecution": False,
            "visualApproval": False,
            "product": False,
        },
    }
    seal_receipt(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "matrix schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "matrix version changed")
    claimed = receipt.get("receiptSha256")
    require(isinstance(claimed, str) and len(claimed) == 64, "receipt seal missing")
    unsigned = copy.deepcopy(receipt)
    unsigned.pop("receiptSha256")
    require(canonical_sha256(unsigned) == claimed, "receipt seal mismatch")
    require(
        receipt.get("scope")
        == {
            "activeOccurrenceCount": 35,
            "renderedMaterialOccurrenceCount": 34,
            "materialRecipeCount": 27,
            "coreRendererMilestone": "33/35",
            "deferredOccurrenceIds": sorted(DEFERRED_OCCURRENCES),
        },
        "matrix scope changed",
    )
    require(
        receipt.get("toolIdentity")
        == {
            "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
            "trackedTextSha256": tracked_text_sha256(SCRIPT_PATH),
        },
        "matrix tool identity changed",
    )
    expected_input_paths = (
        DEFAULT_SOURCE_INVENTORY,
        DEFAULT_MATERIAL_CONTRACT,
        DEFAULT_SHADER_ORACLE,
        DEFAULT_MAIN_CACHE_RECEIPT,
        DEFAULT_EXECUTION_RECEIPT,
    )
    expected_inputs = [
        input_identity(path, read_json_strict(path)) for path in expected_input_paths
    ]
    require(receipt.get("inputs") == expected_inputs, "matrix input identity changed")
    cohort_by_id = {
        row["cohortId"]: row for row in receipt.get("cacheCohorts", [])
    }
    require(
        len(cohort_by_id) == 2
        and set(cohort_by_id)
        == {"OFFICIAL_SAME_DISTRIBUTION_V974", "CURRENT_INSTALLED_CACHE"},
        "matrix cache cohort identity changed",
    )
    for cohort_id, expected_cache in (
        ("OFFICIAL_SAME_DISTRIBUTION_V974", EXPECTED_OFFICIAL_CACHE),
        ("CURRENT_INSTALLED_CACHE", EXPECTED_INSTALLED_CACHE),
    ):
        cohort = cohort_by_id[cohort_id]
        validate_cache_identity(cohort["cacheIdentity"], expected_cache)
        require(
            cohort["searchedRecipeCount"] == 24
            and cohort["codeIndexSummary"]["descriptorCount"]
            == expected_cache["descriptorCount"]
            and cohort["codeIndexSummary"]["embeddedCodeCount"]
            == expected_cache["embeddedCodeCount"]
            and cohort["codeIndexSummary"]["shaderCodeSectionEndLogicalOffset"]
            == expected_cache["shaderCodeSectionEndLogicalOffset"],
            f"matrix cache cohort denominator changed: {cohort_id}",
        )
    recipes = receipt.get("recipeCoverage") or []
    occurrences = receipt.get("occurrences") or []
    require(len(recipes) == 27, "recipe denominator changed")
    require(len(occurrences) == 35, "occurrence denominator changed")
    require(
        len({row["recipeId"] for row in recipes}) == 27,
        "recipe IDs are duplicated",
    )
    require(
        len({row["occurrenceId"] for row in occurrences}) == 35,
        "occurrence IDs are duplicated",
    )
    for row in [*recipes, *occurrences]:
        claimed_row = row.get("rowSha256")
        unsigned_row = copy.deepcopy(row)
        unsigned_row.pop("rowSha256", None)
        require(
            isinstance(claimed_row, str)
            and canonical_sha256(unsigned_row) == claimed_row,
            "matrix row seal mismatch",
        )
    recipe_by_id = {row["recipeId"]: row for row in recipes}
    source_identity_sha = canonical_sha256(source_static_identity_projection(recipes))
    require(
        receipt.get("sourceStaticIdentityProjectionSha256") == source_identity_sha
        and source_identity_sha == EXPECTED_SOURCE_STATIC_IDENTITY_PROJECTION_SHA256,
        "source static identity projection changed",
    )
    allowed_dispositions = {
        "JOINED_CANDIDATE",
        "SELECTED_ADMITTED",
        "CACHE_MISSING_BLOCK",
        "CODE_MISSING_BLOCK",
        "RECONSTRUCTED_ONLY",
    }
    cache_joins: list[dict[str, Any]] = []
    shader_candidates: list[dict[str, Any]] = []
    code_statuses: Counter[str] = Counter()
    for row in recipes:
        require(row["disposition"] in allowed_dispositions, "unknown recipe disposition")
        require(not row["selectedVfPassAdmission"], "selected recipe VF/pass opened")
        require(not row["productAdmission"], "recipe Product admission opened")
        static_set = row["staticParameterSet"]
        joins = row["cacheJoins"]
        cohort_ids = [join["cohortId"] for join in joins]
        require(len(cohort_ids) == len(set(cohort_ids)), "duplicate recipe cache cohort")
        if static_set is None:
            require(not joins, "direct/default recipe gained a ShaderMap join")
            require(
                row["disposition"] == "RECONSTRUCTED_ONLY",
                "direct/default recipe disposition changed",
            )
            require(
                row["staticLookupStatus"]
                in {
                    "DIRECT_MATERIAL_HAS_NO_STATIC_PARAMETER_SET",
                    "MIC_NATIVE_STATIC_KEY_ABSENT",
                },
                "direct/default lookup status changed",
            )
        else:
            normalized = static_set["normalized"]
            engine = static_set["engineEquivalent"]
            require(
                static_set["serializedSemanticSha256"]
                == canonical_json_sha256(normalized),
                "normalized static-set digest changed",
            )
            require(
                static_set["engineEqualitySha256"]
                == canonical_json_sha256(engine),
                "engine-equality static-set digest changed",
            )
            require(
                static_set["baseMaterialIdHex"] == engine["baseMaterialIdHex"],
                "static-set BaseMaterialId changed",
            )
            require(
                static_set["switchCount"]
                == len(normalized["staticSwitchParameters"])
                and static_set["componentMaskCount"]
                == len(normalized["staticComponentMaskParameters"])
                and static_set["terrainLayerWeightCount"]
                == len(normalized["terrainLayerWeightParameters"]),
                "static-set parameter denominator changed",
            )
            official_join = [
                join
                for join in joins
                if join["authority"] == "SAME_DISTRIBUTION_SOURCE_CANDIDATE"
            ]
            if official_join:
                require(len(official_join) == 1, "official cache join is ambiguous")
                has_official_dxbc = any(
                    candidate["dxbcContainers"]
                    for candidate in official_join[0]["shaderObjectCandidates"]
                )
                require(
                    (
                        row["disposition"] == "JOINED_CANDIDATE"
                        and has_official_dxbc
                    )
                    or (
                        row["disposition"] == "CODE_MISSING_BLOCK"
                        and not has_official_dxbc
                    ),
                    "joined recipe code disposition changed",
                )
            else:
                require(
                    row["disposition"] == "CACHE_MISSING_BLOCK",
                    "missing official cache disposition changed",
                )
        for join in joins:
            require(join["exactMapContextCount"] == 1, "cache map context count changed")
            require(
                join["mapContextStatus"] == "EXACT_ONE_MAP_CONTEXT"
                and join["mapParseStatus"] == "MAP_CONTEXT_PARSED",
                "joined map is not exactly parsed",
            )
            require(
                join["authority"]
                in {
                    "SAME_DISTRIBUTION_SOURCE_CANDIDATE",
                    "CROSS_REVISION_CORROBORATION",
                },
                "cache authority changed",
            )
            candidates = join["shaderObjectCandidates"]
            candidate_by_id = {
                candidate["shaderIdHex"]: candidate for candidate in candidates
            }
            require(
                len(candidate_by_id) == len(candidates),
                "shader-object candidate ID is duplicated",
            )
            status_counts = Counter(candidate["status"] for candidate in candidates)
            require(
                dict(sorted(status_counts.items())) == join["codeMappingStatusCounts"],
                "code mapping status counts changed",
            )
            for candidate in candidates:
                require(
                    candidate["descriptorTailCodeIndexAdmission"] is False,
                    "descriptor-tail code index was admitted",
                )
                status = candidate["status"]
                require(
                    status
                    in {
                        "UNRESOLVED_CODE_MAPPING",
                        "CANDIDATE_SINGLE_DXBC",
                        "CANDIDATE_DXBC_BUNDLE",
                        "CANDIDATE_PAYLOAD_WITHOUT_DXBC_CONTAINER",
                    },
                    "unknown code mapping status",
                )
                containers = candidate["dxbcContainers"]
                if status == "UNRESOLVED_CODE_MAPPING":
                    require(
                        candidate["candidatePosition"] is None and not containers,
                        "unresolved code mapping gained a payload",
                    )
                else:
                    require(
                        candidate["candidatePosition"] is not None,
                        "candidate code payload position is absent",
                    )
                if status == "CANDIDATE_SINGLE_DXBC":
                    require(len(containers) == 1, "single-DXBC status changed")
                elif status == "CANDIDATE_DXBC_BUNDLE":
                    require(len(containers) > 1, "DXBC bundle status changed")
                elif status == "CANDIDATE_PAYLOAD_WITHOUT_DXBC_CONTAINER":
                    require(not containers, "container-free status changed")
                for container in containers:
                    require(
                        isinstance(container["dxbcSha256"], str)
                        and len(container["dxbcSha256"]) == 64
                        and container["byteSize"] >= 36,
                        "DXBC container projection changed",
                    )
            reference_count = 0
            for vertex_factory in join["candidateVertexFactories"]:
                require(
                    vertex_factory["selectionAdmission"] is False,
                    "candidate vertex factory was selected",
                )
                for reference in vertex_factory["shaderReferences"]:
                    reference_count += 1
                    candidate = candidate_by_id.get(reference["shaderIdHex"])
                    require(candidate is not None, "shader reference candidate is absent")
                    require(
                        reference["passFamilyCandidate"]
                        == classify_shader_pass(reference["shaderType"]),
                        "shader pass classification changed",
                    )
                    require(
                        reference["codeMappingStatus"] == candidate["status"]
                        and reference["descriptorTailCodeIndexAdmission"] is False
                        and reference["dxbcContainerCount"]
                        == len(candidate["dxbcContainers"]),
                        "shader reference code projection changed",
                    )
            require(
                reference_count == join["shaderReferenceCount"],
                "shader reference denominator changed",
            )
            cache_joins.append(join)
            shader_candidates.extend(candidates)
            code_statuses.update(status_counts)

    require(
        [row["occurrenceId"] for row in occurrences]
        == [f"source-active-{index:03d}" for index in range(35)],
        "occurrence ordering or identity changed",
    )
    occurrence_ids_by_recipe: dict[str, list[str]] = {}
    renderer_shape_by_family = {
        "MeshParticle": "mesh",
        "SpriteParticle": "sprite",
        "CascadeRibbon": "sprite",
        "DecalParticle": "decal",
        "ScreenPost": "screenpost",
    }
    for row in occurrences:
        require(row["disposition"] in allowed_dispositions, "unknown occurrence disposition")
        require(not row["selectedVfPassAdmission"], "occurrence VF/pass admission opened")
        require(not row["productAdmission"], "occurrence Product admission opened")
        recipe_id = row["recipeId"]
        if recipe_id is None:
            require(
                row["occurrenceId"] == "source-active-034"
                and row["rendererFamily"] == "LightParticle"
                and row["disposition"] == "RECONSTRUCTED_ONLY",
                "engine-builtin light boundary changed",
            )
        else:
            recipe = recipe_by_id.get(recipe_id)
            require(recipe is not None, "occurrence recipe join is absent")
            occurrence_ids_by_recipe.setdefault(recipe_id, []).append(
                row["occurrenceId"]
            )
            require(
                row["disposition"] == recipe["disposition"],
                "occurrence disposition differs from recipe",
            )
            require(
                row["sourceMaterialPath"] == recipe["sourceMaterialPath"],
                "occurrence material differs from recipe",
            )
            require(
                renderer_shape_by_family.get(row["rendererFamily"])
                in recipe["rendererShapes"],
                "occurrence renderer shape differs from recipe",
            )

    for recipe in recipes:
        require(
            recipe["occurrenceIds"]
            == sorted(occurrence_ids_by_recipe.get(recipe["recipeId"], [])),
            "recipe occurrence membership changed",
        )

    direct_default_occurrences = {
        row["occurrenceId"]
        for row in occurrences
        if row["recipeId"] is not None
        and recipe_by_id[row["recipeId"]]["staticParameterSet"] is None
    }
    require(
        direct_default_occurrences == EXPECTED_DIRECT_DEFAULT_OCCURRENCES,
        "direct/default occurrence boundary changed",
    )
    summary = receipt["summary"]
    expected_recipe_dispositions = dict(
        sorted(Counter(row["disposition"] for row in recipes).items())
    )
    expected_occurrence_dispositions = dict(
        sorted(Counter(row["disposition"] for row in occurrences).items())
    )
    expected_family_counts = dict(
        sorted(Counter(row["rendererFamily"] for row in occurrences).items())
    )
    require(summary["activeOccurrenceCount"] == 35, "active denominator changed")
    require(summary["materialOccurrenceCount"] == 34, "material denominator changed")
    require(summary["engineBuiltinOccurrenceCount"] == 1, "builtin denominator changed")
    require(summary["materialRecipeCount"] == 27, "recipe summary changed")
    require(summary["staticRecipeCount"] == 24, "static recipe summary changed")
    require(summary["directDefaultRecipeCount"] == 3, "direct/default recipe summary changed")
    require(summary["directMaterialRecipeCount"] == 2, "direct Material recipe summary changed")
    require(summary["micWithoutNativeStaticKeyRecipeCount"] == 1, "MIC no-key recipe summary changed")
    require(summary["staticRecipeOccurrenceCount"] == 29, "static occurrence summary changed")
    require(summary["directDefaultOccurrenceCount"] == 5, "direct/default occurrence summary changed")
    require(summary["directMaterialOccurrenceCount"] == 4, "direct Material occurrence summary changed")
    require(summary["micWithoutNativeStaticKeyOccurrenceCount"] == 1, "MIC no-key occurrence summary changed")
    require(
        expected_family_counts == dict(sorted(RENDERER_FAMILY_COUNTS.items()))
        and summary["rendererFamilyCounts"] == expected_family_counts,
        "renderer family denominator changed",
    )
    require(summary["cacheJoinCount"] == len(cache_joins), "cache join summary changed")
    require(
        summary["officialSameDistributionJoinCount"]
        == sum(join["authority"] == "SAME_DISTRIBUTION_SOURCE_CANDIDATE" for join in cache_joins)
        and summary["installedCrossRevisionJoinCount"]
        == sum(join["authority"] == "CROSS_REVISION_CORROBORATION" for join in cache_joins),
        "cache authority join summary changed",
    )
    require(
        summary["recipeDispositionCounts"] == expected_recipe_dispositions
        and summary["occurrenceDispositionCounts"] == expected_occurrence_dispositions,
        "disposition summary changed",
    )
    require(
        summary["shaderObjectCandidateCountAcrossCohorts"] == len(shader_candidates)
        and summary["dxbcContainerCandidateCountAcrossCohorts"]
        == sum(len(candidate["dxbcContainers"]) for candidate in shader_candidates)
        and summary["descriptorTailCodeIndexAdmissionCount"] == 0
        and summary["codeMappingStatusCountsAcrossCohorts"]
        == dict(sorted(code_statuses.items())),
        "shader code candidate summary changed",
    )
    require(
        summary["shaderReferenceCountAcrossCohorts"]
        == sum(join["shaderReferenceCount"] for join in cache_joins),
        "shader reference summary changed",
    )
    require(summary["selectedVfPassAdmissionCount"] == 0, "VF/pass admission opened")
    require(
        summary["cacheMissingBlockCount"]
        == sum(row["disposition"] == "CACHE_MISSING_BLOCK" for row in recipes)
        and summary["codeMissingBlockCount"]
        == sum(row["disposition"] == "CODE_MISSING_BLOCK" for row in recipes),
        "hard blocker summary changed",
    )
    require(summary["productAdmissionCount"] == 0, "Product admission opened")
    require(
        {row["occurrenceId"] for row in occurrences if row["deferredByUserPriority"]}
        == DEFERRED_OCCURRENCES,
        "Light/Post deferred boundary changed",
    )
    require(
        {row["occurrenceId"] for row in occurrences if row["recipeId"] is not None}
        == {f"source-active-{index:03d}" for index in range(34)},
        "material occurrence identity changed",
    )
    require(
        all(not row["selectedVfPassAdmission"] for row in recipes + occurrences),
        "selected VF/pass admission opened",
    )
    golden = receipt["mainGoldenCrossCheck"]
    require(
        golden["status"] == "MAIN_2_RECIPE_3_OCCURRENCE_GOLDEN_MATCH"
        and golden["rowCount"] == 4
        and {
            (row["cohortId"], row["recipeId"])
            for row in golden["rows"]
        }
        == {
            (cohort_id, recipe_id)
            for cohort_id in (
                "OFFICIAL_SAME_DISTRIBUTION_V974",
                "CURRENT_INSTALLED_CACHE",
            )
            for recipe_id in (
                "material-recipe-03cc03b86c1a4c8f",
                "material-recipe-daf220acad2b656e",
            )
        },
        "main golden cross-check changed",
    )
    cohort_recipe_joins = {
        (join["cohortId"], recipe["recipeId"]): join
        for recipe in recipes
        for join in recipe["cacheJoins"]
    }
    main_cache_receipt = read_json_strict(DEFAULT_MAIN_CACHE_RECEIPT)
    validate_self_digest(main_cache_receipt, "receiptSha256", "main cache receipt")
    main_target_by_key = {}
    for receipt_key, cohort_id in (
        ("officialRefShaderCacheV974", "OFFICIAL_SAME_DISTRIBUTION_V974"),
        ("currentInstalledRefShaderCache", "CURRENT_INSTALLED_CACHE"),
    ):
        for target in main_cache_receipt[receipt_key]["mainTargets"]:
            main_target_by_key[(cohort_id, target["recipeId"])] = target
    for golden_row in golden["rows"]:
        main_target = main_target_by_key.get(
            (golden_row["cohortId"], golden_row["recipeId"])
        )
        require(main_target is not None, "main golden source target is absent")
        expected_selected_dxbc = [
            {
                "shaderType": selected["shaderType"],
                "shaderIdHex": selected["shaderIdHex"],
                "dxbcSha256": selected["dxbcSha256"],
            }
            for selected in main_target["materialMap"]["selectedOriginalDxbc"]
        ]
        require(
            golden_row["occurrenceIds"] == main_target["occurrenceIds"]
            and golden_row["textureRegisterBindingCount"]
            == len(main_target["textureRegisterBindings"])
            and golden_row["textureRegisterBindingsSha256"]
            == canonical_sha256(main_target["textureRegisterBindings"])
            and golden_row["constantBufferBindingsSha256"]
            == canonical_sha256(main_target["constantBufferBindings"])
            and golden_row["selectedOriginalDxbc"] == expected_selected_dxbc,
            "main golden source projection differs",
        )
        join = cohort_recipe_joins.get(
            (golden_row["cohortId"], golden_row["recipeId"])
        )
        require(join is not None, "main golden cache join is absent")
        require(
            join["materialMapRawSha256"]
            == golden_row["materialMapRawSha256"]
            and join["semanticMapSha256"] == golden_row["semanticMapSha256"],
            "main golden map projection differs",
        )
        candidate_by_id = {
            candidate["shaderIdHex"]: candidate
            for candidate in join["shaderObjectCandidates"]
        }
        for selected in golden_row["selectedOriginalDxbc"]:
            candidate = candidate_by_id.get(selected["shaderIdHex"])
            require(
                candidate is not None
                and sum(
                    container["dxbcSha256"] == selected["dxbcSha256"]
                    for container in candidate["dxbcContainers"]
                )
                == 1,
                "main golden DXBC projection differs",
            )
    admission = receipt["admission"]
    require(
        admission["structuralStaticSetJoin"]
        == (summary["officialSameDistributionJoinCount"] == 24),
        "structural static join admission changed",
    )
    require(
        all(
            admission[field] is False
            for field in (
                "descriptorTailIsNativeCodeIndex",
                "candidateVfPassIsSelected",
                "nativeRendererSelection",
                "runtimeExecution",
                "visualApproval",
                "product",
            )
        ),
        "matrix fail-closed admission changed",
    )


def write_or_check(path: Path, receipt: dict[str, Any], check: bool) -> None:
    encoded = (
        json.dumps(receipt, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")
    if check:
        require(path.is_file(), f"matrix receipt is missing: {path}")
        require(path.read_bytes() == encoded, "matrix receipt is stale")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(encoded)
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-inventory", type=Path, default=DEFAULT_SOURCE_INVENTORY)
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--shader-oracle", type=Path, default=DEFAULT_SHADER_ORACLE)
    parser.add_argument("--main-cache-receipt", type=Path, default=DEFAULT_MAIN_CACHE_RECEIPT)
    parser.add_argument("--execution-receipt", type=Path, default=DEFAULT_EXECUTION_RECEIPT)
    parser.add_argument("--official-cache", type=Path, default=DEFAULT_OFFICIAL_CACHE)
    parser.add_argument("--installed-cache", type=Path, default=DEFAULT_INSTALLED_CACHE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        require(args.output.is_file(), f"matrix receipt is missing: {args.output}")
        receipt = read_json_strict(args.output)
        validate_receipt(receipt)
        summary = receipt["summary"]
        print(
            "PASS: Artist F renderer restoration matrix shallow "
            f"occurrences={summary['activeOccurrenceCount']} "
            f"recipes={summary['materialRecipeCount']} "
            f"static={summary['staticRecipeCount']}/24 "
            f"joins={summary['cacheJoinCount']} selected=0 product=false"
        )
        return 0
    receipt = build_receipt(
        args.source_inventory,
        args.material_contract,
        args.shader_oracle,
        args.main_cache_receipt,
        args.execution_receipt,
        args.official_cache,
        args.installed_cache,
    )
    validate_receipt(receipt)
    write_or_check(args.output, receipt, args.check)
    summary = receipt["summary"]
    mode = "deep-check" if args.check else "deep-write"
    print(
        f"PASS: Artist F renderer restoration matrix {mode} "
        f"occurrences={summary['activeOccurrenceCount']} "
        f"recipes={summary['materialRecipeCount']} static={summary['staticRecipeCount']}/24 "
        f"joins={summary['cacheJoinCount']} selected=0 product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
