#!/usr/bin/env python3
"""Name every constant-buffer lane and texture slot a family's program reads.

The translated HLSL is exact but anonymous: it reads `cb0[11].z` because the
cooked program does.  This tool resolves the author's parameter names onto
the native-bound CB0 and texture lanes and records whether each folded
expression reads Time.  It does not resolve an occurrence's source value.

Three pieces of already-proven evidence close that:

* the material map's uniform-expression set, which serializes the author's
  parameter names next to the expression tree that computes them;
* the native shader object's binding wire, which says exactly which CB0 byte
  offset each scalar group and vector expression lands on, and which texture
  and sampler register each texture expression uses;
* the DXBC declarations, which the wire has to close over.

The output is the join of the three for native-bound expressions.  It also
marks every lane whose
expression tree contains `FMaterialUniformExpressionTime`, because a renderer
that uploads those once at spawn produces a still image where the source
scrolls - the panning and rotation lanes are exactly the time-dependent ones.

This admits nothing.  Naming a lane does not choose a vertex factory, a
sampler state, a render state, or a runtime binding.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import math
import re
import struct
import sys
import time
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
for extra in (str(Path(__file__).resolve().parent), str(LEVEL_TOOLS)):
    if extra not in sys.path:
        sys.path.insert(0, extra)

from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    DEFAULT_D3DCOMPILER,
    D3DDisassembler,
    package_tables,
)
from extract_ue3_material_shader_maps import (  # noqa: E402
    extract_selected_packed_dxbc,
    extract_selected_shader_objects,
    _parse_wire_array,
    parse_dxbc_declaration_closure,
    parse_material_map,
    parse_shader_code_layout,
    select_unique_native_binding_arrays,
)

SHADER_MAP_INDEX = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-shader-map-index.v1.json")
COOKED_RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json")
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-named-abi.v1.json")
COOKED_SHADER_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/CookedShaders"
DEFAULT_CACHE = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST"
    r"\31470_TrackA_20260812\OfficialRefShaderCacheV974"
    r"\EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)

SCHEMA = "lostark.effect-family-named-abi"
FORMAT_VERSION = 1
SHADER_MAP_SCHEMA = "lostark.effect-family-shader-map-index"
COOKED_RECEIPT_SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
EXPECTED_EXTRACTED_FAMILY_COUNT = 180
EXPECTED_UNIQUE_EXTRACTED_PROGRAM_COUNT = 169
EXPECTED_STRICT_NON_EMPTY_FAMILY_COUNT = 160
EXPECTED_LENIENT_EMPTY_RESOURCE_FAMILY_COUNT = 20
EXPECTED_STRICT_RESOLVED_COUNT = 160
EXPECTED_LENIENT_RESOLVED_COUNT = 2
EXPECTED_LENIENT_BLOCKED_COUNT = 18

COMPONENTS = "xyzw"
TIME_EXPRESSION = "fmaterialuniformexpressiontime"
CONSTANT_BUFFER_ROW_BYTES = 16
SHA256_PATTERN = re.compile(r"[0-9a-f]{64}")
STRICT_BINDING_SELECTION = "STRICT_NON_EMPTY_G03_3"
LENIENT_BINDING_SELECTION = "LENIENT_LEGAL_EMPTY_RESOURCE_ABI"
AMBIGUOUS_CANDIDATE_REASON = "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS"
EXPECTED_BLOCKED_PARENTS = (
    "bfx_m.bfx_i_pa_glow_01_ad",
    "bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad",
    "bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_tr",
    "bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad",
    "fx_m.fx_d_pa_capturergbsplit_01_tr",
    "fx_m.fx_d_pa_dark_05_tr",
    "fx_m.fx_d_pa_glow_02_ad",
    "fx_m.fx_d_pa_ring_11_tr",
    "fx_m.fx_m_me_splitline_99_tr",
    "fx_m.fx_o_pa_circledisort_01_ad",
    "fx_m_mi_02.fx_m.fx_j_pa_circledisort_01_ad",
    "fx_m_mi_02.fx_m.fx_j_pa_dot_ad_01",
    "fx_m_mi_l_00.fx_m.fx_l_me_icesurfacee_01_tr",
    "fx_m_mi_m_00.fx_m.fx_m_me_splitline_99_tr",
    "fx_m_mi_o_00.fx_m.fx_o_pa_circledisort_01_ad",
    "fx_mastermaterial.fx_mm.fx_mm_distortion_01_ad",
    "fx_mastermaterial.fx_mm.fx_mm_maskcontrol_01_tr",
    "fx_mm.fx_mm_distortion_01_ad",
)
EXPECTED_BLOCKED_PARENT_SET_SHA256 = (
    "7428ae97975da9015edac0e0176e6e6731c5314b590f01d2dba8f923a25c2d56"
)
EXPECTED_LENIENT_RESOLVED_PARENTS = (
    "fx_m_mi_02.fx_m.fx_j_me_splitline_01_1_ad",
    "fx_m_mi_d_00.fx_m.fx_d_pa_flare_03_ad",
)
EXPECTED_LENIENT_RESOLVED_PARENT_SET_SHA256 = (
    "887eea47be00b072cf7ced9543990eac5b0efb2b419702c4721d30a19b09c47c"
)


class NamedAbiError(RuntimeError):
    pass


class BindingCandidateError(NamedAbiError):
    def __init__(self, candidate_count: int) -> None:
        super().__init__(
            "native binding-array candidate is ambiguous: "
            f"{candidate_count}")
        self.reason_code = AMBIGUOUS_CANDIDATE_REASON
        self.candidate_count = candidate_count


def require(condition: bool, message: str) -> None:
    if not condition:
        raise NamedAbiError(message)


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
    return {"rawSha256": digest.hexdigest(), "byteSize": byte_size}


def read_json(path: Path, description: str = "JSON document") -> Any:
    payload = raw_file_identity(path, description)
    del payload
    try:
        return json.loads(path.read_bytes().decode("utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise NamedAbiError(
            f"{description} is not valid UTF-8 JSON: {error}") from error


def read_artifact(
    path: Path,
    schema: str,
    description: str,
) -> tuple[dict[str, Any], dict[str, Any]]:
    identity = raw_file_identity(path, description)
    document = read_json(path, description)
    require(isinstance(document, dict), f"{description} root must be an object")
    require(document.get("schema") == schema,
            f"{description} schema is not supported")
    require(document.get("formatVersion") == FORMAT_VERSION,
            f"{description} formatVersion is not supported")
    artifact_sha = document.get("artifactSha256")
    require(
        isinstance(artifact_sha, str)
        and SHA256_PATTERN.fullmatch(artifact_sha) is not None,
        f"{description} artifactSha256 is missing or malformed",
    )
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    require(canonical_sha256(unsigned) == artifact_sha,
            f"{description} artifactSha256 drifted")
    return document, identity


def cooked_shader_set_identity(
    cooked_rows: list[dict[str, Any]],
) -> dict[str, Any]:
    identities: dict[str, dict[str, Any]] = {}
    for row in cooked_rows:
        digest = row.get("dxbcSha256")
        require(
            isinstance(digest, str)
            and SHA256_PATTERN.fullmatch(digest) is not None,
            "EXTRACTED cooked family has malformed dxbcSha256",
        )
        path = COOKED_SHADER_DIRECTORY / f"{digest}.dxbc"
        identity = raw_file_identity(path, f"cooked DXBC {digest}")
        require(identity["rawSha256"] == digest,
                f"cooked DXBC raw SHA-256 drifted: {path.name}")
        require(identity["byteSize"] == row.get("dxbcByteSize"),
                f"cooked DXBC byte size drifted: {path.name}")
        previous = identities.setdefault(digest, identity)
        require(previous == identity,
                f"shared cooked DXBC identity is inconsistent: {digest}")
    require(
        len(identities) == EXPECTED_UNIQUE_EXTRACTED_PROGRAM_COUNT,
        "unique EXTRACTED cooked program denominator must be "
        f"{EXPECTED_UNIQUE_EXTRACTED_PROGRAM_COUNT}; got {len(identities)}",
    )
    return {
        "programCount": len(identities),
        "setSha256": canonical_sha256(identities),
    }


def parse_named_abi_dxbc_closure(
    disassembly: dict[str, Any],
) -> dict[str, Any]:
    """Project declarations while accepting a legal texture-free program."""
    profile = str(disassembly.get("profile", ""))
    require(profile.startswith("ps_"),
            "selected BasePass shader is not a pixel shader")
    declarations = list(disassembly.get("declarations", []))
    cb0_sizes = [
        int(match.group(1))
        for line in declarations
        for match in [re.search(r"\bcb0\[(\d+)\]", line, re.IGNORECASE)]
        if match
    ]
    require(len(cb0_sizes) == 1,
            "DXBC CB0 declaration is absent or ambiguous")
    declared_textures = sorted({
        int(match.group(1))
        for line in declarations
        if line.startswith("dcl_resource_")
        for match in [re.search(r"\bt(\d+)\b", line, re.IGNORECASE)]
        if match
    })
    declared_samplers = sorted({
        int(match.group(1))
        for line in declarations
        if line.startswith("dcl_sampler ")
        for match in [re.search(r"\bs(\d+)\b", line, re.IGNORECASE)]
        if match
    })
    sample_pair_counts: dict[str, int] = {}
    for line in disassembly.get("instructions", []):
        if not re.match(r"^sample(?:_|\s)", line, re.IGNORECASE):
            continue
        registers = re.findall(r"\b([ts]\d+)\b", line, re.IGNORECASE)
        require(len(registers) == 2,
                "DXBC sample register pair is ambiguous")
        pair = f"{registers[0].casefold()}/{registers[1].casefold()}"
        sample_pair_counts[pair] = sample_pair_counts.get(pair, 0) + 1
    sampled_textures = sorted({
        int(pair.split("/")[0][1:]) for pair in sample_pair_counts
    })
    sampled_samplers = sorted({
        int(pair.split("/")[1][1:]) for pair in sample_pair_counts
    })
    require(declared_textures == sampled_textures,
            "DXBC declared texture registers differ from sampled registers")
    require(declared_samplers == sampled_samplers,
            "DXBC declared sampler registers differ from sampled registers")
    return {
        "profile": profile,
        "normalizedDisassemblySha256":
            disassembly["normalizedDisassemblySha256"],
        "declarationSha256": disassembly["declarationSha256"],
        "instructionSha256": disassembly["instructionSha256"],
        "instructionCount": disassembly["instructionCount"],
        "declaredConstantBuffer0Float4Count": cb0_sizes[0],
        "declaredTextureRegisters": declared_textures,
        "declaredSamplerRegisters": declared_samplers,
        "observedSamplePairCounts": dict(sorted(sample_pair_counts.items())),
    }


def scan_named_abi_binding_candidates(
    object_bytes: bytes,
    object_logical_offset: int,
    uniform_counts: dict[str, int],
    dxbc_closure: dict[str, Any],
) -> list[dict[str, Any]]:
    """Find native ABI triples without inventing non-empty denominators."""
    scalar_count = int(uniform_counts["pixelScalarExpressions"])
    vector_count = int(uniform_counts["pixelVectorExpressions"])
    texture_count = int(uniform_counts["pixelTexture2DExpressions"])
    scalar_group_count = math.ceil(scalar_count / 4)
    cb0_size = int(dxbc_closure["declaredConstantBuffer0Float4Count"])
    declared_textures = set(dxbc_closure["declaredTextureRegisters"])
    declared_samplers = set(dxbc_closure["declaredSamplerRegisters"])
    observed_pairs = set(dxbc_closure["observedSamplePairCounts"])
    candidates = []
    for start in range(48, max(48, len(object_bytes) - 11)):
        try:
            offset = start
            scalar_rows, offset = _parse_wire_array(
                object_bytes, offset, "scalarGroups", None,
                scalar_group_count, object_logical_offset,
                allow_empty=True)
            vector_rows, offset = _parse_wire_array(
                object_bytes, offset, "vectors", None,
                vector_count, object_logical_offset,
                allow_empty=True)
            texture_rows, offset = _parse_wire_array(
                object_bytes, offset, "textures", None,
                texture_count, object_logical_offset,
                allow_empty=True)

            scalar_keys = [
                row["expressionIndexOrGroup"] for row in scalar_rows]
            vector_keys = [
                row["expressionIndexOrGroup"] for row in vector_rows]
            texture_keys = [
                row["expressionIndexOrGroup"] for row in texture_rows]
            require(
                len(set(scalar_keys)) == len(scalar_keys)
                and set(scalar_keys).issubset(range(scalar_group_count)),
                "scalar group keys do not close over uniform expressions",
            )
            require(
                len(set(vector_keys)) == len(vector_keys)
                and set(vector_keys).issubset(range(vector_count)),
                "vector keys do not close over uniform expressions",
            )
            require(
                len(set(texture_keys)) == len(texture_keys)
                and set(texture_keys).issubset(range(texture_count)),
                "texture keys do not close over uniform expressions",
            )
            constant_rows = scalar_rows + vector_rows
            require(
                all(
                    row["numBytesOrResources"] == CONSTANT_BUFFER_ROW_BYTES
                    and row["bufferIndexOrSamplerIndex"] == 0
                    and row["baseIndex"] % CONSTANT_BUFFER_ROW_BYTES == 0
                    for row in constant_rows
                ),
                "constant-buffer wire shape changed",
            )
            cb_slots = [
                row["baseIndex"] // CONSTANT_BUFFER_ROW_BYTES
                for row in constant_rows
            ]
            require(len(cb_slots) == len(set(cb_slots)),
                    "native constant-buffer slots overlap")
            sorted_cb_slots = sorted(cb_slots)
            require(
                all(0 <= slot < cb0_size for slot in sorted_cb_slots),
                "native wires fall outside the DXBC CB0 declaration",
            )
            require(
                not sorted_cb_slots
                or sorted_cb_slots
                == list(range(sorted_cb_slots[0], sorted_cb_slots[-1] + 1)),
                "native material constant-buffer slots are not contiguous",
            )
            unowned_cb_slots = sorted(
                set(range(cb0_size)) - set(sorted_cb_slots))
            leading_unowned = (
                list(range(0, sorted_cb_slots[0]))
                if sorted_cb_slots else list(range(cb0_size))
            )
            trailing_unowned = (
                list(range(sorted_cb_slots[-1] + 1, cb0_size))
                if sorted_cb_slots else []
            )
            require(
                all(row["numBytesOrResources"] == 1
                    for row in texture_rows),
                "texture binding wire shape changed",
            )
            material_pairs = {
                f"t{row['baseIndex']}/s{row['bufferIndexOrSamplerIndex']}"
                for row in texture_rows
            }
            require(len(material_pairs) == len(texture_rows),
                    "native texture binding pairs overlap")
            require(material_pairs.issubset(observed_pairs),
                    "native texture wires are not sampled by the DXBC")
            material_textures = {row["baseIndex"] for row in texture_rows}
            material_samplers = {
                row["bufferIndexOrSamplerIndex"] for row in texture_rows}
            require(
                material_textures.issubset(declared_textures)
                and material_samplers.issubset(declared_samplers),
                "native texture wires are outside DXBC declarations",
            )
            extra_pairs = observed_pairs - material_pairs
            for pair in extra_pairs:
                texture_text, sampler_text = pair.split("/")
                require(
                    int(texture_text[1:]) not in material_textures
                    and int(sampler_text[1:]) not in material_samplers,
                    "non-material engine sample pair conflicts with material ABI",
                )

            arrays_bytes = object_bytes[start:offset]
            semantic_rows = {
                name: [
                    {key: value for key, value in row.items()
                     if key != "logicalOffset"}
                    for row in rows
                ]
                for name, rows in (
                    ("scalarGroups", scalar_rows),
                    ("vectors", vector_rows),
                    ("textures", texture_rows),
                )
            }
            candidate = {
                "bindingArraysOffsetInShaderObject": start,
                "bindingArraysByteSize": offset - start,
                "bindingArraysRawSha256":
                    hashlib.sha256(arrays_bytes).hexdigest(),
                **semantic_rows,
                "constantBufferClosure": {
                    "declaredConstantBuffer0Float4Count": cb0_size,
                    "minimumNativeBoundConstantBuffer0Slot": (
                        sorted_cb_slots[0] if sorted_cb_slots else None),
                    "maximumNativeBoundConstantBuffer0Slot": (
                        sorted_cb_slots[-1] if sorted_cb_slots else None),
                    "boundConstantBuffer0Slots": sorted_cb_slots,
                    "unownedConstantBuffer0Slots": unowned_cb_slots,
                    "leadingUnownedConstantBuffer0Slots": leading_unowned,
                    "trailingUnownedConstantBuffer0Slots": trailing_unowned,
                    "unownedConstantBuffer0SlotPolicy":
                        "PRESERVE_ENGINE_OR_PASS_OWNED_PREFIX_AND_SUFFIX_ROWS",
                    "scalarUniformExpressionGroupDenominator":
                        scalar_group_count,
                    "nativeScalarWireCount": len(scalar_rows),
                    "scalarExpressionGroupsWithoutNativeWire": sorted(
                        set(range(scalar_group_count)) - set(scalar_keys)),
                },
                "textureSampleClosure": {
                    "materialSamplePairs": sorted(material_pairs),
                    "unownedEngineSamplePairs": sorted(extra_pairs),
                    "allObservedSamplePairCounts":
                        dxbc_closure["observedSamplePairCounts"],
                },
            }
            candidate["bindingSemanticSha256"] = canonical_sha256({
                key: candidate[key]
                for key in (
                    "scalarGroups", "vectors", "textures",
                    "constantBufferClosure", "textureSampleClosure")
            })
            candidates.append(candidate)
        except (NamedAbiError, ValueError, struct.error, KeyError, IndexError):
            continue
    return candidates


def select_named_abi_binding_arrays(
    object_bytes: bytes,
    object_logical_offset: int,
    uniform_counts: dict[str, int],
    dxbc_closure: dict[str, Any],
) -> dict[str, Any]:
    candidates = scan_named_abi_binding_candidates(
        object_bytes, object_logical_offset, uniform_counts, dxbc_closure)
    require(bool(candidates),
            "native binding-array candidate is absent")
    if len(candidates) > 1:
        raise BindingCandidateError(len(candidates))
    return {**candidates[0], "candidateCount": 1}


def binding_selection_mode(
    disassembly: dict[str, Any],
    uniform_counts: dict[str, int],
) -> str:
    has_sample = any(
        re.match(r"^sample(?:_|\s)", line, re.IGNORECASE)
        for line in disassembly.get("instructions", [])
    )
    if (int(uniform_counts["pixelVectorExpressions"]) > 0
            and int(uniform_counts["pixelTexture2DExpressions"]) > 0
            and has_sample):
        return STRICT_BINDING_SELECTION
    return LENIENT_BINDING_SELECTION


def resolve_named_abi_binding(
    disassembly: dict[str, Any],
    shader_object: dict[str, Any],
    uniform_counts: dict[str, int],
) -> tuple[dict[str, Any], dict[str, Any], str]:
    mode = binding_selection_mode(disassembly, uniform_counts)
    if mode == STRICT_BINDING_SELECTION:
        closure = parse_dxbc_declaration_closure(disassembly)
        wire = select_unique_native_binding_arrays(
            shader_object["_bytes"], shader_object["logicalOffset"],
            uniform_counts, closure)
    else:
        closure = parse_named_abi_dxbc_closure(disassembly)
        wire = select_named_abi_binding_arrays(
            shader_object["_bytes"], shader_object["logicalOffset"],
            uniform_counts, closure)
    return closure, wire, mode


def expression_facts(expression: Any) -> tuple[list[str], bool]:
    """Every parameter name in a tree, and whether the tree reads Time."""
    names: list[str] = []
    uses_time = False
    stack = [expression]
    while stack:
        node = stack.pop()
        if isinstance(node, dict):
            type_name = str(node.get("typeName", ""))
            if type_name == TIME_EXPRESSION:
                uses_time = True
            name = node.get("parameterName")
            if name and name not in names:
                names.append(str(name))
            stack.extend(node.values())
        elif isinstance(node, list):
            stack.extend(node)
    return names, uses_time


def describe_expression(expression: Any) -> str:
    """A one-line shape for a folded tree, so the receipt stays readable."""
    if not isinstance(expression, dict):
        return "constant"
    type_name = str(expression.get("typeName", "")).replace(
        "fmaterialuniformexpression", "")
    if type_name == "scalarparameter" or type_name == "vectorparameter":
        return f"{type_name}:{expression.get('parameterName')}"
    inner = [
        describe_expression(expression[key])
        for key in ("a", "b", "input", "x", "y")
        if key in expression
    ]
    return f"{type_name}({', '.join(inner)})" if inner else type_name


def named_lanes(
    material_map: dict[str, Any],
    wire: dict[str, Any],
) -> dict[str, Any]:
    """Join uniform expressions to the CB0 lanes and slots the wire assigns."""
    uniform = material_map["uniformExpressionSet"]
    scalars = uniform["pixelScalarExpressions"]
    vectors = uniform["pixelVectorExpressions"]
    textures = uniform["pixelTexture2DExpressions"]

    scalar_lanes = []
    for row in wire["scalarGroups"]:
        group = row["expressionIndexOrGroup"]
        base = row["baseIndex"]
        require(base % CONSTANT_BUFFER_ROW_BYTES == 0,
                "scalar group is not row aligned")
        constant_row = base // CONSTANT_BUFFER_ROW_BYTES
        for lane in range(4):
            index = group * 4 + lane
            if index >= len(scalars):
                break
            names, uses_time = expression_facts(scalars[index])
            scalar_lanes.append({
                "expressionIndex": index,
                "scalarGroup": group,
                "constantRegister": f"cb0[{constant_row}].{COMPONENTS[lane]}",
                "constantRow": constant_row,
                "component": COMPONENTS[lane],
                "parameterNames": names,
                "expression": describe_expression(scalars[index]),
                "timeDependent": uses_time,
            })

    vector_lanes = []
    for row in wire["vectors"]:
        index = row["expressionIndexOrGroup"]
        base = row["baseIndex"]
        require(base % CONSTANT_BUFFER_ROW_BYTES == 0,
                "vector expression is not row aligned")
        constant_row = base // CONSTANT_BUFFER_ROW_BYTES
        names, uses_time = expression_facts(vectors[index])
        vector_lanes.append({
            "expressionIndex": index,
            "constantRegister": f"cb0[{constant_row}]",
            "constantRow": constant_row,
            "parameterNames": names,
            "expression": describe_expression(vectors[index]),
            "timeDependent": uses_time,
        })

    texture_slots = []
    for row in wire["textures"]:
        index = row["expressionIndexOrGroup"]
        expression = textures[index]
        texture_slots.append({
            "expressionIndex": index,
            "textureRegister": f"t{row['baseIndex']}",
            "samplerRegister": f"s{row['bufferIndexOrSamplerIndex']}",
            "parameterName": expression.get("parameterName"),
            "referencedTextureIndex": expression.get("referencedTextureIndex"),
            "isParameter": bool(expression.get("parameterName")),
        })

    scalar_lanes.sort(key=lambda item: item["expressionIndex"])
    vector_lanes.sort(key=lambda item: item["expressionIndex"])
    texture_slots.sort(key=lambda item: item["expressionIndex"])
    return {
        "scalarLanes": scalar_lanes,
        "vectorLanes": vector_lanes,
        "textureSlots": texture_slots,
    }


def resolve_material_map(
    index_row: dict[str, Any],
    cooked_row: dict[str, Any],
    cache: dict[str, Any],
    layout: dict[str, Any],
) -> dict[str, Any]:
    contexts = index_row["cacheScan"]["materialMapContexts"]
    matches = [
        row for row in contexts
        if row["logicalOffset"] == cooked_row["materialMapLogicalOffset"]
    ]
    require(len(matches) == 1, "material-map context is absent or ambiguous")
    context = matches[0]
    return parse_material_map(
        cache, layout, context,
        context["engineEqualityStaticParameterSetSha256"])


def build_family(
    parent: str,
    cooked_row: dict[str, Any],
    material_map: dict[str, Any],
    bytecode: bytes,
    shader_object: dict[str, Any],
    disassembler: D3DDisassembler,
) -> dict[str, Any]:
    require(
        hashlib.sha256(bytecode).hexdigest() == cooked_row["dxbcSha256"],
        f"cache DXBC differs from cooked receipt: {parent}",
    )
    disassembly = disassembler.disassemble(bytecode)
    closure, wire, selection_mode = resolve_named_abi_binding(
        disassembly, shader_object, material_map["uniformExpressionCounts"])
    lanes = named_lanes(material_map, wire)

    time_scalars = [row for row in lanes["scalarLanes"] if row["timeDependent"]]
    time_vectors = [row for row in lanes["vectorLanes"] if row["timeDependent"]]
    named_scalars = [row for row in lanes["scalarLanes"] if row["parameterNames"]]
    named_vectors = [row for row in lanes["vectorLanes"] if row["parameterNames"]]

    return {
        "parentMaterialPath": parent,
        "status": "RESOLVED_NAMED_MAPPING",
        "friendlyName": material_map["friendlyName"],
        "carrier": cooked_row["carrier"],
        "childMaterialPath": cooked_row["childMaterialPath"],
        "dxbcSha256": cooked_row["dxbcSha256"],
        "instructionCount": disassembly["instructionCount"],
        "declaredConstantBuffer0Float4Count":
            closure["declaredConstantBuffer0Float4Count"],
        "uniformExpressionCounts": material_map["uniformExpressionCounts"],
        "nativeBindingWire": {
            "selectionMode": selection_mode,
            "offsetInShaderObject": wire["bindingArraysOffsetInShaderObject"],
            "byteSize": wire["bindingArraysByteSize"],
            "rawSha256": wire["bindingArraysRawSha256"],
            "bindingSemanticSha256": wire["bindingSemanticSha256"],
            "scalarGroups": wire["scalarGroups"],
            "vectors": wire["vectors"],
            "textures": wire["textures"],
            "constantBufferClosure": wire["constantBufferClosure"],
            "textureSampleClosure": wire["textureSampleClosure"],
        },
        **lanes,
        "summary": {
            "scalarLaneCount": len(lanes["scalarLanes"]),
            "vectorLaneCount": len(lanes["vectorLanes"]),
            "textureSlotCount": len(lanes["textureSlots"]),
            "namedScalarLaneCount": len(named_scalars),
            "namedVectorLaneCount": len(named_vectors),
            "timeDependentScalarLaneCount": len(time_scalars),
            "timeDependentVectorLaneCount": len(time_vectors),
            "timeDependentRegisters": sorted({
                row["constantRegister"] for row in time_scalars + time_vectors
            }),
        },
        "admits": "NAMED_LANE_IDENTITY_ONLY",
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--only", nargs="*", default=None,
                        help="Restrict to these parent material paths.")
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--d3dcompiler", type=Path,
                        default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    arguments = parser.parse_args(argv)
    require(arguments.limit >= 0, "--limit must be non-negative")
    partial_selection = arguments.only is not None or arguments.limit != 0
    require(
        not (partial_selection
             and arguments.output.resolve() == DEFAULT_OUTPUT.resolve()),
        "partial --only/--limit generation cannot overwrite the canonical "
        "named ABI receipt; provide a non-canonical --output",
    )

    index_document, index_identity = read_artifact(
        SHADER_MAP_INDEX, SHADER_MAP_SCHEMA, "shader-map index")
    cooked_document, cooked_identity = read_artifact(
        COOKED_RECEIPT, COOKED_RECEIPT_SCHEMA, "cooked shader receipt")
    cooked_inputs = cooked_document.get("inputs")
    require(isinstance(cooked_inputs, dict),
            "cooked shader receipt inputs must be an object")
    require(
        cooked_inputs.get("shaderMapArtifactSha256")
        == index_document["artifactSha256"],
        "cooked shader receipt pins a different shader-map artifact",
    )
    require(
        cooked_inputs.get("shaderMapRawSha256")
        == index_identity["rawSha256"],
        "cooked shader receipt pins different shader-map bytes",
    )

    index_rows = index_document.get("families")
    require(isinstance(index_rows, list),
            "shader-map index families must be an array")
    index: dict[str, dict[str, Any]] = {}
    for row in index_rows:
        require(isinstance(row, dict),
                "shader-map family must be an object")
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                "shader-map family parentMaterialPath is missing")
        require(parent not in index,
                f"shader-map index duplicates family: {parent}")
        index[parent] = row

    cooked_families = cooked_document.get("families")
    require(isinstance(cooked_families, list),
            "cooked shader receipt families must be an array")
    full_cooked = [row for row in cooked_families
                   if isinstance(row, dict)
                   and row.get("status") == "EXTRACTED"]
    require(
        len(full_cooked) == EXPECTED_EXTRACTED_FAMILY_COUNT,
        "EXTRACTED family denominator must be "
        f"{EXPECTED_EXTRACTED_FAMILY_COUNT}; got {len(full_cooked)}",
    )
    cooked_parents = [row.get("parentMaterialPath") for row in full_cooked]
    require(
        all(isinstance(parent, str) and bool(parent)
            for parent in cooked_parents),
        "EXTRACTED cooked family parentMaterialPath is missing",
    )
    require(len(cooked_parents) == len(set(cooked_parents)),
            "cooked shader receipt duplicates an EXTRACTED family")
    require(set(cooked_parents).issubset(index),
            "EXTRACTED cooked family is absent from shader-map index")
    cooked_shader_identity = cooked_shader_set_identity(full_cooked)

    cache_path = arguments.cache.resolve()
    compiler_path = arguments.d3dcompiler.resolve()
    cache_identity = raw_file_identity(cache_path, "RefShaderCache")
    compiler_identity = raw_file_identity(compiler_path, "D3DCompiler")
    require(
        cooked_inputs.get("refShaderCacheFileName") == cache_path.name
        and cooked_inputs.get("refShaderCacheRawSha256")
        == cache_identity["rawSha256"]
        and cooked_inputs.get("refShaderCacheByteSize")
        == cache_identity["byteSize"],
        "named ABI input cache differs from the cooked shader receipt pin",
    )

    cooked = list(full_cooked)
    if arguments.only is not None:
        wanted = set(arguments.only)
        cooked = [row for row in cooked
                  if row["parentMaterialPath"] in wanted]
    if arguments.limit:
        cooked = cooked[: arguments.limit]
    require(bool(cooked), "no extracted family matched the selection")

    print(f"opening cache: {cache_path.name}", flush=True)
    started = time.time()
    cache = package_tables(cache_path)
    layout = parse_shader_code_layout(cache)
    disassembler = D3DDisassembler(compiler_path)
    print(f"cache tables in {time.time() - started:.1f}s", flush=True)

    # The packed-descriptor scan and the shader-object walk each traverse the
    # whole cooked section, so both are done once for every family together.
    maps: dict[str, Any] = {}
    references: dict[str, dict[str, str]] = {}
    rows = []
    for cooked_row in cooked:
        parent = cooked_row["parentMaterialPath"]
        maps[parent] = resolve_material_map(
            index[parent], cooked_row, cache, layout)
        references[cooked_row["shaderIdHex"]] = {
            "shaderType": cooked_row["shaderType"],
            "shaderIdHex": cooked_row["shaderIdHex"],
        }
    started = time.time()
    blobs = extract_selected_packed_dxbc(
        cache, layout, list(references.values()))
    objects = extract_selected_shader_objects(
        cache, layout, list(references.values()))["byShaderId"]
    for cooked_row in cooked:
        parent = cooked_row["parentMaterialPath"]
        if parent not in maps:
            continue
        shader_id = cooked_row["shaderIdHex"]
        require(shader_id in blobs,
                f"batched DXBC extraction omitted shader: {parent}")
        require(shader_id in objects,
                f"batched shader-object extraction omitted shader: {parent}")
        require(
            hashlib.sha256(bytes(blobs[shader_id]["_bytecode"])).hexdigest()
            == cooked_row["dxbcSha256"],
            f"cache DXBC differs from cooked receipt: {parent}",
        )
    print(f"batched DXBC and shader objects in {time.time() - started:.1f}s",
          flush=True)

    resolved = 0
    for cooked_row in cooked:
        parent = cooked_row["parentMaterialPath"]
        bytecode = bytes(
            blobs[cooked_row["shaderIdHex"]]["_bytecode"])
        selection_mode = binding_selection_mode(
            disassembler.disassemble(bytecode),
            maps[parent]["uniformExpressionCounts"],
        )
        try:
            row = build_family(
                parent, cooked_row, maps[parent],
                bytecode,
                objects[cooked_row["shaderIdHex"]],
                disassembler)
            resolved += 1
            summary = row["summary"]
            print(f"NAMED {parent} "
                  f"scalars={summary['namedScalarLaneCount']}"
                  f"/{summary['scalarLaneCount']} "
                  f"vectors={summary['namedVectorLaneCount']}"
                  f"/{summary['vectorLaneCount']} "
                  f"textures={summary['textureSlotCount']} "
                  f"time={summary['timeDependentRegisters']}", flush=True)
        except BindingCandidateError as error:
            require(selection_mode == LENIENT_BINDING_SELECTION,
                    "strict ABI family cannot be admitted as blocked")
            require(error.candidate_count > 1,
                    "ambiguous candidate blocker requires candidateCount > 1")
            row = {
                "parentMaterialPath": parent,
                "status": "BLOCKED",
                "blocker": {
                    "reasonCode": error.reason_code,
                    "candidateCount": error.candidate_count,
                },
                "bindingSelectionMode": selection_mode,
            }
            print(f"SKIP  {parent}: {str(error)[:140]}", flush=True)
        rows.append(row)

    selection_counts = collections.Counter()
    outcome_counts = collections.Counter()
    for row in rows:
        mode = (
            row.get("nativeBindingWire", {}).get("selectionMode")
            if row.get("status") == "RESOLVED_NAMED_MAPPING"
            else row.get("bindingSelectionMode")
        )
        if mode:
            selection_counts[mode] += 1
            outcome_counts[f"{mode}:{row['status']}"] += 1
    require(
        selection_counts[STRICT_BINDING_SELECTION]
        == EXPECTED_STRICT_NON_EMPTY_FAMILY_COUNT,
        "strict non-empty ABI family denominator must be "
        f"{EXPECTED_STRICT_NON_EMPTY_FAMILY_COUNT}; got "
        f"{selection_counts[STRICT_BINDING_SELECTION]}",
    )
    require(
        selection_counts[LENIENT_BINDING_SELECTION]
        == EXPECTED_LENIENT_EMPTY_RESOURCE_FAMILY_COUNT,
        "lenient empty-resource ABI family denominator must be "
        f"{EXPECTED_LENIENT_EMPTY_RESOURCE_FAMILY_COUNT}; got "
        f"{selection_counts[LENIENT_BINDING_SELECTION]}",
    )
    expected_outcomes = {
        f"{STRICT_BINDING_SELECTION}:RESOLVED_NAMED_MAPPING":
            EXPECTED_STRICT_RESOLVED_COUNT,
        f"{LENIENT_BINDING_SELECTION}:RESOLVED_NAMED_MAPPING":
            EXPECTED_LENIENT_RESOLVED_COUNT,
        f"{LENIENT_BINDING_SELECTION}:BLOCKED":
            EXPECTED_LENIENT_BLOCKED_COUNT,
    }
    require(dict(sorted(outcome_counts.items())) == expected_outcomes,
            "named ABI binding outcome cohort regressed")
    require(resolved == (EXPECTED_STRICT_RESOLVED_COUNT
                         + EXPECTED_LENIENT_RESOLVED_COUNT),
            f"resolved named mapping count must be 162; got {resolved}")
    blocked_parents = sorted(
        row["parentMaterialPath"] for row in rows
        if row["status"] == "BLOCKED")
    require(tuple(blocked_parents) == EXPECTED_BLOCKED_PARENTS,
            "blocked parent cohort differs from the validated 18-family set")
    blocked_parent_sha = canonical_sha256(blocked_parents)
    require(blocked_parent_sha == EXPECTED_BLOCKED_PARENT_SET_SHA256,
            "blocked parent cohort SHA-256 changed")
    lenient_resolved_parents = sorted(
        row["parentMaterialPath"] for row in rows
        if row["status"] == "RESOLVED_NAMED_MAPPING"
        and row["nativeBindingWire"]["selectionMode"]
        == LENIENT_BINDING_SELECTION)
    require(tuple(lenient_resolved_parents)
            == EXPECTED_LENIENT_RESOLVED_PARENTS,
            "lenient resolved cohort differs from the validated two families")
    lenient_resolved_sha = canonical_sha256(lenient_resolved_parents)
    require(
        lenient_resolved_sha
        == EXPECTED_LENIENT_RESOLVED_PARENT_SET_SHA256,
        "lenient resolved cohort SHA-256 changed",
    )

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "admits": "NAMED_LANE_IDENTITY_ONLY",
            "note": (
                "A resolved mapping joins a native-bound uniform expression "
                "to the constant or texture register the cooked program "
                "reads. It is not runtime ABI closure and does not admit a "
                "source value, vertex factory, sampler state, render state "
                "or runtime binding."
            ),
        },
        "inputs": {
            "shaderMapIndex":
                "Data/Effects/Contracts/effect-family-shader-map-index.v1.json",
            "shaderMapArtifactSha256": index_document["artifactSha256"],
            "shaderMapRawSha256": index_identity["rawSha256"],
            "shaderMapByteSize": index_identity["byteSize"],
            "cookedPixelShaders":
                "Data/Effects/Contracts/"
                "effect-family-cooked-pixel-shaders.v1.json",
            "cookedPixelShadersArtifactSha256":
                cooked_document["artifactSha256"],
            "cookedPixelShadersRawSha256": cooked_identity["rawSha256"],
            "cookedPixelShadersByteSize": cooked_identity["byteSize"],
            "cookedShaderDirectory": "Data/Effects/CookedShaders",
            "cookedShaderProgramCount":
                cooked_shader_identity["programCount"],
            "cookedShaderSetSha256": cooked_shader_identity["setSha256"],
            "refShaderCacheFileName": cache_path.name,
            "refShaderCacheRawSha256": cache_identity["rawSha256"],
            "refShaderCacheByteSize": cache_identity["byteSize"],
            "d3dCompilerFileName": compiler_path.name,
            "d3dCompilerRawSha256": compiler_identity["rawSha256"],
            "d3dCompilerByteSize": compiler_identity["byteSize"],
        },
        "summary": {
            "familyCount": len(rows),
            "resolvedNamedMappingCount": resolved,
            "blockedCount": len(rows) - resolved,
            "bindingSelectionCounts": dict(sorted(selection_counts.items())),
            "bindingOutcomeCounts": expected_outcomes,
            "blockerCounts": {AMBIGUOUS_CANDIDATE_REASON:
                              EXPECTED_LENIENT_BLOCKED_COUNT},
            "blockedParents": blocked_parents,
            "blockedParentSetSha256": blocked_parent_sha,
            "lenientResolvedParents": lenient_resolved_parents,
            "lenientResolvedParentSetSha256": lenient_resolved_sha,
        },
        "families": rows,
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    arguments.output.parent.mkdir(parents=True, exist_ok=True)
    temporary = arguments.output.with_suffix(arguments.output.suffix + ".tmp")
    temporary.write_text(
        json.dumps(receipt, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n")
    temporary.replace(arguments.output)
    print(f"WROTE: {arguments.output}")
    print(f"RESULT: resolved={resolved} blocked={len(rows) - resolved}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
