#!/usr/bin/env python3
"""Verify the checked named-ABI and FlowTrail offline parity evidence.

This is a cheap checked-artifact gate.  It does not parse the 245 MB shader
cache or execute WARP.  It verifies that both receipts are self-hashed, pin the
current upstream bytes and external tools, close over the exact cooked-family
denominator and still describe only the evidence their generators produced.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import math
import re
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SHADER_MAP = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-shader-map-index.v1.json"
)
DEFAULT_COOKED_RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json"
)
DEFAULT_NAMED_ABI = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-named-abi.v1.json"
)
DEFAULT_PARITY = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-time-varying-parity.v1.json"
)
DEFAULT_COOKED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/CookedShaders"
DEFAULT_AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
DEFAULT_CACHE = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST"
    r"\31470_TrackA_20260812\OfficialRefShaderCacheV974"
    r"\EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)
DEFAULT_D3DCOMPILER = Path(
    r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0"
    r"\x64\d3dcompiler_47.dll"
)

SHADER_MAP_SCHEMA = "lostark.effect-family-shader-map-index"
COOKED_SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
NAMED_ABI_SCHEMA = "lostark.effect-family-named-abi"
PARITY_SCHEMA = "lostark.effect-family-time-varying-parity"
FORMAT_VERSION = 1
EXPECTED_EXTRACTED_FAMILY_COUNT = 180
EXPECTED_UNIQUE_PROGRAM_COUNT = 169
EXPECTED_STRICT_NON_EMPTY_FAMILY_COUNT = 160
EXPECTED_LENIENT_EMPTY_RESOURCE_FAMILY_COUNT = 20
EXPECTED_STRICT_RESOLVED_COUNT = 160
EXPECTED_LENIENT_RESOLVED_COUNT = 2
EXPECTED_LENIENT_BLOCKED_COUNT = 18
RESOLVED = "RESOLVED_NAMED_MAPPING"
BLOCKED = "BLOCKED"
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
NAMED_ADMISSION = "NAMED_LANE_IDENTITY_ONLY"
PARITY_ADMISSION = "OFFLINE_RECONSTRUCTED_CB0_EQUATION_RESPONSE_ONLY"
PARITY_PASS = "AUTHORED_RECONSTRUCTED_CB0_VALUE_PARITY"
MOTION_PASS = "OFFLINE_EVALUATOR_CB0_CHANGES_WITH_TIME"
CANONICAL_PARENT = "fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr"
CANONICAL_EFFECT_ASSET_ID = "effect.lancemaster.skill.34150.unified"
CANONICAL_ELEMENT_ID = "authored.source-particle.1dda1a259e98ed79e8fbb978"
CANONICAL_TIMES = [0.0, 0.25, 0.5, 1.0, 2.0]
PARITY_TOLERANCE = 1e-4
MOTION_THRESHOLD = 1e-4
SHA256_PATTERN = re.compile(r"[0-9a-f]{64}")
CB_REGISTER_PATTERN = re.compile(r"cb0\[(\d+)\](?:\.([xyzw]))?")
TEXTURE_REGISTER_PATTERN = re.compile(r"t(\d+)")
SAMPLER_REGISTER_PATTERN = re.compile(r"s(\d+)")
REQUIRED_NON_PROOFS = {
    "CHECKED_HLSLI_ARTIFACT_IDENTITY",
    "ACTUAL_TEXTURE_OR_SAMPLER_SEMANTICS",
    "ACTUAL_CARRIER_OR_VERTEX_FACTORY_INPUTS",
    "ENGINE_OWNED_CONSTANT_BUFFER_VALUES",
    "RUNTIME_CONSTANT_BUFFER_UPLOAD",
    "RENDER_STATE_OR_PASS_OR_MRT",
    "UV_OR_OUTPUT_MOTION",
    "VISUAL_FIDELITY_OR_PRODUCT_ADMISSION",
}
REQUIRED_PROOFS = {
    "DXBC_AND_REGENERATED_TRANSLATION_AGREE_AT_RECONSTRUCTED_CB0",
    "OFFLINE_EVALUATOR_PRODUCES_TIME_VARYING_CB0_ROWS",
}


class VerificationError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def raw_identity(path: Path, description: str) -> dict[str, Any]:
    require(path.is_file(), f"{description} is missing: {path}")
    digest = hashlib.sha256()
    byte_size = 0
    try:
        with path.open("rb") as stream:
            while chunk := stream.read(1024 * 1024):
                digest.update(chunk)
                byte_size += len(chunk)
    except OSError as error:
        raise VerificationError(
            f"{description} could not be read: {path}: {error}"
        ) from error
    return {"rawSha256": digest.hexdigest(), "byteSize": byte_size}


def read_artifact(
    path: Path,
    schema: str,
    description: str,
) -> tuple[dict[str, Any], dict[str, Any]]:
    identity = raw_identity(path, description)
    payload = path.read_bytes()
    require(b"\r" not in payload, f"{description} is not LF-only")
    try:
        document = json.loads(payload.decode("utf-8"))
    except (UnicodeError, json.JSONDecodeError) as error:
        raise VerificationError(
            f"{description} is not valid UTF-8 JSON: {error}"
        ) from error
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


def require_pin(
    inputs: dict[str, Any],
    prefix: str,
    identity: dict[str, Any],
    description: str,
) -> None:
    require(inputs.get(f"{prefix}RawSha256") == identity["rawSha256"],
            f"{description} raw SHA-256 pin drifted")
    require(inputs.get(f"{prefix}ByteSize") == identity["byteSize"],
            f"{description} byte-size pin drifted")


def require_sha256(value: Any, description: str) -> str:
    require(
        isinstance(value, str) and SHA256_PATTERN.fullmatch(value) is not None,
        f"{description} is missing or malformed",
    )
    return value


def require_count(value: Any, description: str) -> int:
    require(isinstance(value, int) and not isinstance(value, bool) and value >= 0,
            f"{description} must be a non-negative integer")
    return value


def require_finite_number(value: Any, description: str) -> float:
    require(isinstance(value, (int, float)) and not isinstance(value, bool),
            f"{description} must be numeric")
    result = float(value)
    require(math.isfinite(result), f"{description} must be finite")
    return result


def cooked_denominator(
    cooked: dict[str, Any],
    cooked_directory: Path,
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    families = cooked.get("families")
    require(isinstance(families, list),
            "cooked shader receipt families must be an array")
    by_parent: dict[str, dict[str, Any]] = {}
    programs: dict[str, dict[str, Any]] = {}
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"cooked family {offset} must be an object")
        if row.get("status") != "EXTRACTED":
            continue
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"cooked family {offset} parentMaterialPath is missing")
        require(parent not in by_parent,
                f"cooked receipt duplicates EXTRACTED family: {parent}")
        by_parent[parent] = row
        digest = require_sha256(
            row.get("dxbcSha256"), f"cooked family {parent} dxbcSha256")
        byte_size = require_count(
            row.get("dxbcByteSize"), f"cooked family {parent} dxbcByteSize")
        require(byte_size > 0, f"cooked family {parent} DXBC is empty")
        path = cooked_directory / f"{digest}.dxbc"
        identity = raw_identity(path, f"cooked DXBC {digest}")
        require(identity["rawSha256"] == digest,
                f"cooked DXBC raw SHA-256 drifted: {path.name}")
        require(identity["byteSize"] == byte_size,
                f"cooked DXBC byte size drifted: {path.name}")
        previous = programs.setdefault(digest, identity)
        require(previous == identity,
                f"shared cooked DXBC identity is inconsistent: {digest}")
    require(len(by_parent) == EXPECTED_EXTRACTED_FAMILY_COUNT,
            "EXTRACTED family denominator must be "
            f"{EXPECTED_EXTRACTED_FAMILY_COUNT}; got {len(by_parent)}")
    require(len(programs) == EXPECTED_UNIQUE_PROGRAM_COUNT,
            "unique cooked program denominator must be "
            f"{EXPECTED_UNIQUE_PROGRAM_COUNT}; got {len(programs)}")
    return by_parent, programs


def validate_scalar_lane(
    lane: Any,
    scalar_count: int,
    declared_rows: int,
    description: str,
) -> None:
    require(isinstance(lane, dict), f"{description} must be an object")
    index = require_count(lane.get("expressionIndex"),
                          f"{description}.expressionIndex")
    require(index < scalar_count, f"{description} expressionIndex is outside ABI")
    group = require_count(lane.get("scalarGroup"),
                          f"{description}.scalarGroup")
    require(group == index // 4, f"{description} scalarGroup is inconsistent")
    row = require_count(lane.get("constantRow"),
                        f"{description}.constantRow")
    require(row < declared_rows, f"{description} constantRow is outside CB0")
    component = lane.get("component")
    require(component in "xyzw", f"{description} component is malformed")
    require(lane.get("constantRegister") == f"cb0[{row}].{component}",
            f"{description} constantRegister is inconsistent")
    require(isinstance(lane.get("parameterNames"), list),
            f"{description} parameterNames must be an array")
    require(isinstance(lane.get("timeDependent"), bool),
            f"{description} timeDependent must be boolean")


def validate_vector_lane(
    lane: Any,
    vector_count: int,
    declared_rows: int,
    description: str,
) -> None:
    require(isinstance(lane, dict), f"{description} must be an object")
    index = require_count(lane.get("expressionIndex"),
                          f"{description}.expressionIndex")
    require(index < vector_count, f"{description} expressionIndex is outside ABI")
    row = require_count(lane.get("constantRow"),
                        f"{description}.constantRow")
    require(row < declared_rows, f"{description} constantRow is outside CB0")
    require(lane.get("constantRegister") == f"cb0[{row}]",
            f"{description} constantRegister is inconsistent")
    require(isinstance(lane.get("parameterNames"), list),
            f"{description} parameterNames must be an array")
    require(isinstance(lane.get("timeDependent"), bool),
            f"{description} timeDependent must be boolean")


def validate_texture_slot(
    slot: Any,
    texture_count: int,
    description: str,
) -> None:
    require(isinstance(slot, dict), f"{description} must be an object")
    index = require_count(slot.get("expressionIndex"),
                          f"{description}.expressionIndex")
    require(index < texture_count,
            f"{description} expressionIndex is outside ABI")
    require(isinstance(slot.get("textureRegister"), str)
            and TEXTURE_REGISTER_PATTERN.fullmatch(slot["textureRegister"]),
            f"{description} textureRegister is malformed")
    require(isinstance(slot.get("samplerRegister"), str)
            and SAMPLER_REGISTER_PATTERN.fullmatch(slot["samplerRegister"]),
            f"{description} samplerRegister is malformed")
    parameter = slot.get("parameterName")
    require(parameter is None or isinstance(parameter, str),
            f"{description} parameterName is malformed")
    require(slot.get("isParameter") is bool(parameter),
            f"{description} isParameter is inconsistent")


def validate_mapping_row(
    row: dict[str, Any],
    cooked_row: dict[str, Any],
) -> None:
    parent = row["parentMaterialPath"]
    require(row.get("admits") == NAMED_ADMISSION,
            f"named mapping admission changed: {parent}")
    require(row.get("dxbcSha256") == cooked_row.get("dxbcSha256"),
            f"named mapping DXBC differs from cooked receipt: {parent}")
    require(row.get("carrier") == cooked_row.get("carrier"),
            f"named mapping carrier differs from cooked receipt: {parent}")
    require(row.get("childMaterialPath") == cooked_row.get("childMaterialPath"),
            f"named mapping child differs from cooked receipt: {parent}")
    counts = row.get("uniformExpressionCounts")
    require(counts == cooked_row.get("uniformExpressionCounts"),
            f"named mapping uniform denominator drifted: {parent}")
    require(isinstance(counts, dict),
            f"named mapping uniform counts are absent: {parent}")
    scalar_count = require_count(counts.get("pixelScalarExpressions"),
                                 f"{parent} scalar denominator")
    vector_count = require_count(counts.get("pixelVectorExpressions"),
                                 f"{parent} vector denominator")
    texture_count = require_count(counts.get("pixelTexture2DExpressions"),
                                  f"{parent} texture denominator")
    declared = require_count(
        row.get("declaredConstantBuffer0Float4Count"),
        f"{parent} declared CB0 rows")
    require(declared > 0, f"{parent} declared CB0 is empty")
    wire = row.get("nativeBindingWire")
    require(isinstance(wire, dict), f"{parent} nativeBindingWire is absent")
    require(
        wire.get("selectionMode")
        in {STRICT_BINDING_SELECTION, LENIENT_BINDING_SELECTION},
        f"{parent} native binding selectionMode is missing or invalid",
    )
    require_sha256(wire.get("rawSha256"), f"{parent} native wire rawSha256")
    require_sha256(wire.get("bindingSemanticSha256"),
                   f"{parent} native wire bindingSemanticSha256")
    closure = wire.get("constantBufferClosure")
    require(isinstance(closure, dict),
            f"{parent} constantBufferClosure is absent")
    require(closure.get("declaredConstantBuffer0Float4Count") == declared,
            f"{parent} native wire CB0 denominator is inconsistent")

    scalars = row.get("scalarLanes")
    vectors = row.get("vectorLanes")
    textures = row.get("textureSlots")
    require(isinstance(scalars, list) and isinstance(vectors, list)
            and isinstance(textures, list),
            f"{parent} ABI lane arrays are malformed")
    for offset, lane in enumerate(scalars):
        validate_scalar_lane(lane, scalar_count, declared,
                             f"{parent} scalar lane {offset}")
    for offset, lane in enumerate(vectors):
        validate_vector_lane(lane, vector_count, declared,
                             f"{parent} vector lane {offset}")
    for offset, slot in enumerate(textures):
        validate_texture_slot(slot, texture_count,
                              f"{parent} texture slot {offset}")
    require(len({row["expressionIndex"] for row in scalars}) == len(scalars),
            f"{parent} duplicates scalar expression mapping")
    require(len({row["expressionIndex"] for row in vectors}) == len(vectors),
            f"{parent} duplicates vector expression mapping")
    require(len({row["expressionIndex"] for row in textures}) == len(textures),
            f"{parent} duplicates texture expression mapping")

    native_scalars = wire.get("scalarGroups")
    native_vectors = wire.get("vectors")
    native_textures = wire.get("textures")
    texture_closure = wire.get("textureSampleClosure")
    require(
        isinstance(native_scalars, list)
        and isinstance(native_vectors, list)
        and isinstance(native_textures, list),
        f"{parent} raw native binding arrays are absent",
    )
    require(isinstance(texture_closure, dict),
            f"{parent} textureSampleClosure is absent")
    semantic_payload = {
        "scalarGroups": native_scalars,
        "vectors": native_vectors,
        "textures": native_textures,
        "constantBufferClosure": closure,
        "textureSampleClosure": texture_closure,
    }
    require(
        canonical_sha256(semantic_payload)
        == wire["bindingSemanticSha256"],
        f"{parent} bindingSemanticSha256 drifted",
    )

    def native_constant_row(native: Any, description: str) -> tuple[int, int]:
        require(isinstance(native, dict), f"{description} must be an object")
        expression = require_count(
            native.get("expressionIndexOrGroup"),
            f"{description}.expressionIndexOrGroup")
        base = require_count(native.get("baseIndex"),
                             f"{description}.baseIndex")
        require(base % 16 == 0, f"{description} baseIndex is not row aligned")
        require(native.get("numBytesOrResources") == 16
                and native.get("bufferIndexOrSamplerIndex") == 0,
                f"{description} constant wire shape changed")
        return expression, base // 16

    scalar_wire_rows = dict(
        native_constant_row(native, f"{parent} native scalar {offset}")
        for offset, native in enumerate(native_scalars)
    )
    vector_wire_rows = dict(
        native_constant_row(native, f"{parent} native vector {offset}")
        for offset, native in enumerate(native_vectors)
    )
    require(len(scalar_wire_rows) == len(native_scalars),
            f"{parent} duplicates native scalar group")
    require(len(vector_wire_rows) == len(native_vectors),
            f"{parent} duplicates native vector expression")

    scalar_group_denominator = math.ceil(scalar_count / 4)
    require(closure.get("scalarUniformExpressionGroupDenominator")
            == scalar_group_denominator,
            f"{parent} scalar group denominator drifted")
    without_native = closure.get("scalarExpressionGroupsWithoutNativeWire")
    require(isinstance(without_native, list)
            and without_native == sorted(set(without_native)),
            f"{parent} scalar groups without native wire are malformed")
    expected_scalar_groups = (
        set(range(scalar_group_denominator)) - set(without_native))
    require(set(scalar_wire_rows) == expected_scalar_groups,
            f"{parent} native scalar group closure is inconsistent")
    require(closure.get("nativeScalarWireCount") == len(native_scalars),
            f"{parent} native scalar wire count drifted")
    expected_scalar_indexes = {
        group * 4 + lane
        for group in expected_scalar_groups
        for lane in range(4)
        if group * 4 + lane < scalar_count
    }
    require({lane["expressionIndex"] for lane in scalars}
            == expected_scalar_indexes,
            f"{parent} scalar lanes do not close over native groups")
    for lane in scalars:
        require(lane["constantRow"]
                == scalar_wire_rows[lane["scalarGroup"]],
                f"{parent} scalar lane moved from its native CB0 row")

    require(set(vector_wire_rows)
            == {lane["expressionIndex"] for lane in vectors},
            f"{parent} vector lanes do not close over native expressions")
    for lane in vectors:
        require(lane["constantRow"]
                == vector_wire_rows[lane["expressionIndex"]],
                f"{parent} vector lane moved from its native CB0 row")

    native_bound_rows = sorted(
        set(scalar_wire_rows.values()) | set(vector_wire_rows.values()))
    require(closure.get("boundConstantBuffer0Slots") == native_bound_rows,
            f"{parent} native bound CB0 slots drifted")
    require({lane["constantRow"] for lane in scalars + vectors}
            == set(native_bound_rows),
            f"{parent} lane CB0 rows differ from native closure")
    unowned_rows = sorted(set(range(declared)) - set(native_bound_rows))
    require(closure.get("unownedConstantBuffer0Slots") == unowned_rows,
            f"{parent} unowned CB0 slots drifted")
    expected_minimum = native_bound_rows[0] if native_bound_rows else None
    expected_maximum = native_bound_rows[-1] if native_bound_rows else None
    require(closure.get("minimumNativeBoundConstantBuffer0Slot")
            == expected_minimum,
            f"{parent} minimum native CB0 slot drifted")
    require(closure.get("maximumNativeBoundConstantBuffer0Slot")
            == expected_maximum,
            f"{parent} maximum native CB0 slot drifted")
    expected_leading = (
        list(range(expected_minimum))
        if expected_minimum is not None else list(range(declared)))
    expected_trailing = (
        list(range(expected_maximum + 1, declared))
        if expected_maximum is not None else [])
    require(closure.get("leadingUnownedConstantBuffer0Slots")
            == expected_leading,
            f"{parent} leading engine-owned CB0 slots drifted")
    require(closure.get("trailingUnownedConstantBuffer0Slots")
            == expected_trailing,
            f"{parent} trailing engine-owned CB0 slots drifted")

    native_texture_pairs: dict[int, str] = {}
    for offset, native in enumerate(native_textures):
        require(isinstance(native, dict),
                f"{parent} native texture {offset} must be an object")
        expression = require_count(
            native.get("expressionIndexOrGroup"),
            f"{parent} native texture {offset} expression")
        texture_register = require_count(
            native.get("baseIndex"),
            f"{parent} native texture {offset} register")
        sampler_register = require_count(
            native.get("bufferIndexOrSamplerIndex"),
            f"{parent} native texture {offset} sampler")
        require(native.get("numBytesOrResources") == 1,
                f"{parent} native texture wire shape changed")
        require(expression not in native_texture_pairs,
                f"{parent} duplicates native texture expression")
        native_texture_pairs[expression] = (
            f"t{texture_register}/s{sampler_register}")
    lane_texture_pairs = {
        slot["expressionIndex"]:
            f"{slot['textureRegister']}/{slot['samplerRegister']}"
        for slot in textures
    }
    require(lane_texture_pairs == native_texture_pairs,
            f"{parent} texture lanes differ from native t/s wiring")
    material_pairs = sorted(native_texture_pairs.values())
    require(texture_closure.get("materialSamplePairs") == material_pairs,
            f"{parent} material sample-pair closure drifted")
    engine_pairs = texture_closure.get("unownedEngineSamplePairs")
    observed = texture_closure.get("allObservedSamplePairCounts")
    require(isinstance(engine_pairs, list)
            and engine_pairs == sorted(set(engine_pairs)),
            f"{parent} engine sample pairs are malformed")
    require(isinstance(observed, dict)
            and all(isinstance(value, int) and value > 0
                    for value in observed.values()),
            f"{parent} observed sample-pair counts are malformed")
    require(not (set(material_pairs) & set(engine_pairs)),
            f"{parent} material and engine sample pairs overlap")
    require(set(observed) == set(material_pairs) | set(engine_pairs),
            f"{parent} observed sample-pair closure is incomplete")

    summary = row.get("summary")
    require(isinstance(summary, dict), f"{parent} summary is absent")
    expected = {
        "scalarLaneCount": len(scalars),
        "vectorLaneCount": len(vectors),
        "textureSlotCount": len(textures),
        "namedScalarLaneCount": sum(bool(lane["parameterNames"])
                                    for lane in scalars),
        "namedVectorLaneCount": sum(bool(lane["parameterNames"])
                                    for lane in vectors),
        "timeDependentScalarLaneCount": sum(lane["timeDependent"]
                                             for lane in scalars),
        "timeDependentVectorLaneCount": sum(lane["timeDependent"]
                                             for lane in vectors),
        "timeDependentRegisters": sorted({
            lane["constantRegister"]
            for lane in scalars + vectors if lane["timeDependent"]
        }),
    }
    for key, value in expected.items():
        require(summary.get(key) == value,
                f"{parent} summary.{key} is inconsistent")


def verify_named_abi(
    named: dict[str, Any],
    named_inputs: dict[str, tuple[dict[str, Any], dict[str, Any]]],
    cooked_by_parent: dict[str, dict[str, Any]],
    programs: dict[str, dict[str, Any]],
    cache_path: Path,
    compiler_path: Path,
) -> dict[str, dict[str, Any]]:
    index, index_identity = named_inputs["shaderMap"]
    cooked, cooked_identity = named_inputs["cooked"]
    del index, cooked
    inputs = named.get("inputs")
    require(isinstance(inputs, dict), "named ABI inputs must be an object")
    require(inputs.get("shaderMapArtifactSha256")
            == named_inputs["shaderMap"][0]["artifactSha256"],
            "named ABI shader-map artifact pin drifted")
    require(inputs.get("cookedPixelShadersArtifactSha256")
            == named_inputs["cooked"][0]["artifactSha256"],
            "named ABI cooked artifact pin drifted")
    require_pin(inputs, "shaderMap", index_identity, "named ABI shader-map")
    require_pin(inputs, "cookedPixelShaders", cooked_identity,
                "named ABI cooked receipt")
    cache_identity = raw_identity(cache_path, "RefShaderCache")
    compiler_identity = raw_identity(compiler_path, "D3DCompiler")
    require(inputs.get("refShaderCacheFileName") == cache_path.name,
            "named ABI RefShaderCache filename drifted")
    require(inputs.get("d3dCompilerFileName") == compiler_path.name,
            "named ABI D3DCompiler filename drifted")
    require_pin(inputs, "refShaderCache", cache_identity,
                "named ABI RefShaderCache")
    require_pin(inputs, "d3dCompiler", compiler_identity,
                "named ABI D3DCompiler")
    require(inputs.get("cookedShaderProgramCount") == len(programs),
            "named ABI cooked program count pin drifted")
    require(inputs.get("cookedShaderSetSha256") == canonical_sha256(programs),
            "named ABI cooked shader set pin drifted")
    identity = named.get("identity")
    require(isinstance(identity, dict)
            and identity.get("admits") == NAMED_ADMISSION,
            "named ABI admission boundary changed")

    families = named.get("families")
    require(isinstance(families, list), "named ABI families must be an array")
    by_parent: dict[str, dict[str, Any]] = {}
    blockers: collections.Counter[str] = collections.Counter()
    selection_counts: collections.Counter[str] = collections.Counter()
    outcome_counts: collections.Counter[str] = collections.Counter()
    blocked_parents = []
    lenient_resolved_parents = []
    resolved = 0
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"named ABI family {offset} must be an object")
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"named ABI family {offset} parentMaterialPath is missing")
        require(parent not in by_parent,
                f"named ABI duplicates family: {parent}")
        require(parent in cooked_by_parent,
                f"named ABI family is outside cooked denominator: {parent}")
        by_parent[parent] = row
        status = row.get("status")
        require(status in {RESOLVED, BLOCKED},
                f"named ABI family has no explicit valid status: {parent}")
        if status == RESOLVED:
            resolved += 1
            validate_mapping_row(row, cooked_by_parent[parent])
            mode = row["nativeBindingWire"]["selectionMode"]
            selection_counts[mode] += 1
            outcome_counts[f"{mode}:{RESOLVED}"] += 1
            if mode == LENIENT_BINDING_SELECTION:
                lenient_resolved_parents.append(parent)
        else:
            blocker = row.get("blocker")
            require(isinstance(blocker, dict)
                    and set(blocker) == {"reasonCode", "candidateCount"},
                    f"BLOCKED named ABI family has no structured blocker: "
                    f"{parent}")
            require(blocker.get("reasonCode") == AMBIGUOUS_CANDIDATE_REASON,
                    f"BLOCKED named ABI family has unknown reason: {parent}")
            candidate_count = blocker.get("candidateCount")
            require(isinstance(candidate_count, int)
                    and not isinstance(candidate_count, bool)
                    and candidate_count > 1,
                    f"BLOCKED named ABI family candidateCount must exceed 1: "
                    f"{parent}")
            mode = row.get("bindingSelectionMode")
            require(mode == LENIENT_BINDING_SELECTION,
                    f"only lenient empty-resource families may be BLOCKED: "
                    f"{parent}")
            selection_counts[mode] += 1
            outcome_counts[f"{mode}:{BLOCKED}"] += 1
            blockers[blocker["reasonCode"]] += 1
            blocked_parents.append(parent)
    require(set(by_parent) == set(cooked_by_parent),
            "named ABI family denominator differs from cooked receipt")
    summary = named.get("summary")
    require(isinstance(summary, dict), "named ABI summary must be an object")
    blocked_parents.sort()
    lenient_resolved_parents.sort()
    blocked_parent_sha = canonical_sha256(blocked_parents)
    lenient_resolved_sha = canonical_sha256(lenient_resolved_parents)
    expected_outcomes = {key: value for key, value in {
        f"{STRICT_BINDING_SELECTION}:{RESOLVED}":
            EXPECTED_STRICT_RESOLVED_COUNT,
        f"{LENIENT_BINDING_SELECTION}:{RESOLVED}":
            EXPECTED_LENIENT_RESOLVED_COUNT,
        f"{LENIENT_BINDING_SELECTION}:{BLOCKED}":
            EXPECTED_LENIENT_BLOCKED_COUNT,
    }.items() if value}
    expected_summary = {
        "familyCount": len(by_parent),
        "resolvedNamedMappingCount": resolved,
        "blockedCount": len(by_parent) - resolved,
        "bindingSelectionCounts": dict(sorted(selection_counts.items())),
        "bindingOutcomeCounts": expected_outcomes,
        "blockerCounts": dict(sorted(blockers.items())),
        "blockedParents": blocked_parents,
        "blockedParentSetSha256": blocked_parent_sha,
        "lenientResolvedParents": lenient_resolved_parents,
        "lenientResolvedParentSetSha256": lenient_resolved_sha,
    }
    for key, value in expected_summary.items():
        require(summary.get(key) == value,
                f"named ABI summary.{key} is inconsistent")
    require("closedCount" not in summary,
            "named ABI summary must not call mapping resolution closure")
    require(dict(sorted(outcome_counts.items())) == expected_outcomes,
            "named ABI binding outcome cohort regressed")
    require(resolved == (EXPECTED_STRICT_RESOLVED_COUNT
                         + EXPECTED_LENIENT_RESOLVED_COUNT),
            f"resolved named mapping count must be 162; got {resolved}")
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
    require(tuple(blocked_parents) == EXPECTED_BLOCKED_PARENTS,
            "blocked parent cohort differs from the validated 18-family set")
    require(blocked_parent_sha == EXPECTED_BLOCKED_PARENT_SET_SHA256,
            "blocked parent cohort SHA-256 changed")
    require(tuple(lenient_resolved_parents)
            == EXPECTED_LENIENT_RESOLVED_PARENTS,
            "lenient resolved cohort differs from the validated two families")
    require(
        lenient_resolved_sha
        == EXPECTED_LENIENT_RESOLVED_PARENT_SET_SHA256,
        "lenient resolved cohort SHA-256 changed",
    )
    return by_parent


def verify_parity(
    parity: dict[str, Any],
    parity_input_documents: dict[str, tuple[dict[str, Any], dict[str, Any]]],
    named_by_parent: dict[str, dict[str, Any]],
    authored_directory: Path,
    cooked_directory: Path,
    cache_path: Path,
    compiler_path: Path,
) -> None:
    identity = parity.get("identity")
    require(isinstance(identity, dict)
            and identity.get("admits") == PARITY_ADMISSION,
            "parity admission boundary changed")
    non_proofs = identity.get("doesNotProve")
    require(isinstance(non_proofs, list)
            and set(non_proofs) == REQUIRED_NON_PROOFS,
            "parity non-proof boundary is incomplete")
    proofs = identity.get("proves")
    require(isinstance(proofs, list) and set(proofs) == REQUIRED_PROOFS,
            "parity proof boundary changed")
    inputs = parity.get("inputs")
    require(isinstance(inputs, dict), "parity inputs must be an object")
    prefix_map = {
        "shaderMap": "shaderMap",
        "cooked": "cookedPixelShaders",
        "named": "namedAbi",
    }
    for key, prefix in prefix_map.items():
        document, raw = parity_input_documents[key]
        require(inputs.get(f"{prefix}ArtifactSha256")
                == document["artifactSha256"],
                f"parity {prefix} artifact pin drifted")
        require_pin(inputs, prefix, raw, f"parity {prefix}")

    authored_path = authored_directory / f"{CANONICAL_EFFECT_ASSET_ID}.effect.json"
    authored_identity = raw_identity(authored_path, "FlowTrail authored document")
    require_pin(inputs, "authoredDocument", authored_identity,
                "parity authored document")
    require(inputs.get("authoredDocument")
            == "Data/Effects/Authored/"
            f"{CANONICAL_EFFECT_ASSET_ID}.effect.json",
            "parity authored document path drifted")
    cache_identity = raw_identity(cache_path, "RefShaderCache")
    compiler_identity = raw_identity(compiler_path, "D3DCompiler")
    require_pin(inputs, "refShaderCache", cache_identity,
                "parity RefShaderCache")
    require_pin(inputs, "d3dCompiler", compiler_identity,
                "parity D3DCompiler")
    require(inputs.get("refShaderCacheFileName") == cache_path.name,
            "parity RefShaderCache filename drifted")
    require(inputs.get("d3dCompilerFileName") == compiler_path.name,
            "parity D3DCompiler filename drifted")

    occurrence = parity.get("occurrence")
    require(isinstance(occurrence, dict), "parity occurrence must be an object")
    require(occurrence.get("parentMaterialPath") == CANONICAL_PARENT,
            "parity parent is not the canonical FlowTrail family")
    require(occurrence.get("effectAssetId") == CANONICAL_EFFECT_ASSET_ID,
            "parity effectAssetId is not canonical")
    require(occurrence.get("elementId") == CANONICAL_ELEMENT_ID,
            "parity elementId is not canonical")
    require(occurrence.get("sourceProfileSemanticStatus")
            == "reconstructed_profile",
            "parity must disclose reconstructed_profile source semantics")
    require(occurrence.get("admits") == PARITY_ADMISSION,
            "parity occurrence admission boundary changed")
    named_row = named_by_parent.get(CANONICAL_PARENT)
    require(named_row is not None and named_row.get("status") == RESOLVED,
            "FlowTrail named mapping is not resolved")
    require(occurrence.get("dxbcSha256") == named_row.get("dxbcSha256"),
            "parity DXBC differs from named ABI")
    digest = occurrence["dxbcSha256"]
    dxbc_identity = raw_identity(
        cooked_directory / f"{digest}.dxbc", "FlowTrail cooked DXBC")
    require_pin(inputs, "cookedDxbc", dxbc_identity, "parity cooked DXBC")
    require(inputs.get("cookedDxbcFileName") == f"{digest}.dxbc",
            "parity cooked DXBC filename drifted")

    authored = json.loads(authored_path.read_text(encoding="utf-8"))
    matches = [row for row in authored.get("elements", [])
               if row.get("id") == CANONICAL_ELEMENT_ID]
    require(len(matches) == 1, "FlowTrail authored occurrence is absent")
    material = matches[0].get("material", {})
    profile = material.get("sourceProfile", {})
    require(profile.get("semanticStatus") == "reconstructed_profile",
            "FlowTrail authored sourceProfile semantics changed")
    require(profile.get("parentMaterialPath") == CANONICAL_PARENT,
            "FlowTrail authored parent material changed")
    require(material.get("sourceMaterialPath")
            == occurrence.get("sourceMaterialPath"),
            "FlowTrail authored source material changed")
    require(material.get("renderProfile") == occurrence.get("renderProfile"),
            "FlowTrail authored render profile changed")

    times = occurrence.get("gameTimesSampled")
    require(times == CANONICAL_TIMES, "parity sampled times changed")
    parity_delta = require_finite_number(
        occurrence.get("worstParityDelta"), "parity worst delta")
    require(parity_delta <= PARITY_TOLERANCE,
            "parity worst delta exceeds tolerance")
    motion = require_finite_number(
        occurrence.get("constantMotionAcrossTime"), "parity CB0 motion")
    require(motion > MOTION_THRESHOLD,
            "offline evaluator CB0 does not change across time")
    require_finite_number(
        occurrence.get("outputMotionAcrossTime"), "parity output motion")
    require(occurrence.get("parityResult") == PARITY_PASS,
            "parity success status is not bounded reconstructed-CB0 parity")
    require(occurrence.get("motionResult") == MOTION_PASS,
            "parity motion status is not offline evaluator motion")
    fixture = occurrence.get("fixtureBoundary")
    require(isinstance(fixture, dict)
            and fixture.get("runtimeRendererExecuted") is False
            and fixture.get("carrierInputs")
            == "SYNTHETIC_CONSTANT_FLOAT4_ROWS"
            and fixture.get("engineConstantBuffers")
            == "SYNTHETIC_CONSTANT_FLOAT4_ROWS"
            and fixture.get("textures") == "SYNTHETIC_CONSTANT_1X1_RGBA"
            and fixture.get("translationSource")
            == "REGENERATED_FROM_CURRENT_TRANSLATOR_NOT_CHECKED_HLSLI",
            "parity synthetic fixture boundary is incomplete")
    require(occurrence.get("outputMotionCaveat")
            == "REPLAY_BINDS_1X1_TEXTURES_SO_UV_PANNING_CANNOT_SHOW",
            "parity 1x1 texture/output-motion caveat changed")

    expected_time_rows = sorted({
        match.group(0)
        for register in named_row["summary"]["timeDependentRegisters"]
        for match in [re.match(r"cb0\[\d+\]", register)]
        if match
    })
    require(occurrence.get("expectedTimeDependentRows") == expected_time_rows,
            "parity expected time rows differ from named ABI")
    moving_rows = occurrence.get("movingConstantRows")
    require(isinstance(moving_rows, list) and bool(moving_rows),
            "parity movingConstantRows must be non-empty")
    require(set(moving_rows).issubset(expected_time_rows),
            "parity reports motion outside named time-dependent rows")
    samples = occurrence.get("samples")
    require(isinstance(samples, list) and len(samples) == len(CANONICAL_TIMES),
            "parity samples denominator changed")
    for expected_time, sample in zip(CANONICAL_TIMES, samples):
        require(isinstance(sample, dict), "parity sample must be an object")
        require(sample.get("gameTimeSeconds") == expected_time,
                "parity sample time/order changed")
        delta = require_finite_number(
            sample.get("worstRelativeDelta"), "parity sample delta")
        require(delta <= PARITY_TOLERANCE,
                "parity sample delta exceeds tolerance")
        rt0 = sample.get("renderTarget0")
        require(isinstance(rt0, list) and bool(rt0),
                "parity sample RT0 is malformed")
        for component, value in enumerate(rt0):
            require_finite_number(value,
                                  f"parity sample RT0 component {component}")


def verify(
    shader_map_path: Path,
    cooked_receipt_path: Path,
    named_abi_path: Path,
    parity_path: Path,
    cooked_directory: Path,
    authored_directory: Path,
    cache_path: Path,
    compiler_path: Path,
) -> dict[str, int]:
    shader_map, shader_map_identity = read_artifact(
        shader_map_path, SHADER_MAP_SCHEMA, "shader-map index")
    cooked, cooked_identity = read_artifact(
        cooked_receipt_path, COOKED_SCHEMA, "cooked shader receipt")
    named, named_identity = read_artifact(
        named_abi_path, NAMED_ABI_SCHEMA, "named ABI receipt")
    parity, _ = read_artifact(
        parity_path, PARITY_SCHEMA, "time-varying parity receipt")
    cooked_by_parent, programs = cooked_denominator(
        cooked, cooked_directory)
    named_by_parent = verify_named_abi(
        named,
        {
            "shaderMap": (shader_map, shader_map_identity),
            "cooked": (cooked, cooked_identity),
        },
        cooked_by_parent,
        programs,
        cache_path,
        compiler_path,
    )
    verify_parity(
        parity,
        {
            "shaderMap": (shader_map, shader_map_identity),
            "cooked": (cooked, cooked_identity),
            "named": (named, named_identity),
        },
        named_by_parent,
        authored_directory,
        cooked_directory,
        cache_path,
        compiler_path,
    )
    return {
        "familyCount": len(named_by_parent),
        "resolvedNamedMappingCount": named["summary"][
            "resolvedNamedMappingCount"],
        "blockedCount": named["summary"]["blockedCount"],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--shader-map", type=Path, default=DEFAULT_SHADER_MAP)
    parser.add_argument("--cooked-receipt", type=Path,
                        default=DEFAULT_COOKED_RECEIPT)
    parser.add_argument("--named-abi", type=Path, default=DEFAULT_NAMED_ABI)
    parser.add_argument("--parity", type=Path, default=DEFAULT_PARITY)
    parser.add_argument("--cooked-directory", type=Path,
                        default=DEFAULT_COOKED_DIRECTORY)
    parser.add_argument("--authored-directory", type=Path,
                        default=DEFAULT_AUTHORED_DIRECTORY)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--d3dcompiler", type=Path,
                        default=DEFAULT_D3DCOMPILER)
    arguments = parser.parse_args(argv)
    try:
        result = verify(
            arguments.shader_map.resolve(),
            arguments.cooked_receipt.resolve(),
            arguments.named_abi.resolve(),
            arguments.parity.resolve(),
            arguments.cooked_directory.resolve(),
            arguments.authored_directory.resolve(),
            arguments.cache.resolve(),
            arguments.d3dcompiler.resolve(),
        )
    except VerificationError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: Effect-family named ABI evidence "
        f"families={result['familyCount']} "
        f"resolved={result['resolvedNamedMappingCount']} "
        f"blocked={result['blockedCount']} "
        "FlowTrail=AUTHORED_RECONSTRUCTED_CB0_VALUE_PARITY"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
