#!/usr/bin/env python3
"""Replay a family at authored reconstructed CB0 values over sampled times.

This harness narrows an equation-parity check to one authored occurrence whose
sourceProfile is explicitly `reconstructed_profile`.  It evaluates the cooked
uniform-expression trees at several game times and, for each reconstructed
CB0, runs both the original DXBC and a freshly compiled translation on the
same WARP device.  It proves only two offline facts:

* both programs agree for these reconstructed authored CB0 values; and
* the offline evaluator produces changing CB0 rows from time expressions.

It does not execute the C++ renderer or its upload path.  Carrier inputs,
engine-owned constant buffers and 1x1 textures are synthetic, so it proves
neither UV/output motion nor texture, sampler, vertex-factory, render-state,
runtime-upload or visual fidelity.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import sys
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
    parse_material_map,
    parse_shader_code_layout,
)
from evaluate_ue3_material_uniform_expressions import (  # noqa: E402
    evaluate_expression,
)
from replay_ue3_material_pixel_shaders import (  # noqa: E402
    CarrierCompiler,
    GenericWarpReplay,
    build_carrier_source,
    dxbc_chunks,
    parse_signature,
)
from translate_ue3_dxbc_to_hlsl import parse_declarations  # noqa: E402
from verify_ue3_dxbc_hlsl_translation import (  # noqa: E402
    PRINT_PRECISION_TOLERANCE,
    build_standalone_source,
    compile_pixel_shader,
    replay_declarations,
)
from build_effect_family_named_abi import (  # noqa: E402
    COOKED_RECEIPT_SCHEMA,
    DEFAULT_CACHE,
    NamedAbiError,
    COOKED_RECEIPT,
    SCHEMA as NAMED_ABI_SCHEMA,
    SHADER_MAP_SCHEMA,
    SHADER_MAP_INDEX,
    canonical_sha256,
    raw_file_identity,
    read_json,
    read_artifact,
    resolve_named_abi_binding,
    resolve_material_map,
)

AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-time-varying-parity.v1.json")
NAMED_ABI_RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-named-abi.v1.json")

SCHEMA = "lostark.effect-family-time-varying-parity"
FORMAT_VERSION = 1
CANONICAL_PARENT = "fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr"
CANONICAL_EFFECT_ASSET_ID = "effect.lancemaster.skill.34150.unified"
CANONICAL_ELEMENT_ID = "authored.source-particle.1dda1a259e98ed79e8fbb978"
RECONSTRUCTED_SEMANTIC_STATUS = "reconstructed_profile"
PARITY_PASS = "AUTHORED_RECONSTRUCTED_CB0_VALUE_PARITY"
PARITY_FAIL = "AUTHORED_RECONSTRUCTED_CB0_VALUE_MISMATCH"
MOTION_PASS = "OFFLINE_EVALUATOR_CB0_CHANGES_WITH_TIME"
MOTION_FAIL = "OFFLINE_EVALUATOR_CB0_FROZEN_ACROSS_TIME"
ADMITS = "OFFLINE_RECONSTRUCTED_CB0_EQUATION_RESPONSE_ONLY"

DEFAULT_TIMES = (0.0, 0.25, 0.5, 1.0, 2.0)
# Two samples count as "moving" when some component differs by more than this.
# Well below anything visible, but far above float noise.
MOTION_THRESHOLD = 1e-4


class ParityError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ParityError(message)


def occurrence_parameters(
    effect_asset_id: str, element_id: str
) -> dict[str, Any]:
    """The authored reconstructed scalar/vector profile for one occurrence."""
    path = AUTHORED_DIRECTORY / f"{effect_asset_id}.effect.json"
    require(path.is_file(), f"authored document is missing: {path}")
    document = read_json(path, "authored effect document")
    matches = [row for row in document["elements"]
               if row.get("id") == element_id]
    require(len(matches) == 1, f"occurrence is absent: {element_id}")
    profile = matches[0]["material"]["sourceProfile"]
    require(
        profile.get("semanticStatus") == RECONSTRUCTED_SEMANTIC_STATUS,
        "time-varying parity requires an explicitly reconstructed_profile "
        "sourceProfile",
    )
    return {
        "elementId": element_id,
        "sourceProfileSemanticStatus": profile["semanticStatus"],
        "parentMaterialPath": profile["parentMaterialPath"],
        "sourceMaterialPath": matches[0]["material"]["sourceMaterialPath"],
        "renderProfile": matches[0]["material"]["renderProfile"],
        "scalars": [
            {"parameterName": row["name"], "parameterNameNumber": 0,
             "value": float(row["value"])}
            for row in profile.get("scalars", [])
        ],
        "vectors": [
            {"parameterName": row["name"], "parameterNameNumber": 0,
             "value": [float(value) for value in row["value"]]}
            for row in profile.get("vectors", [])
        ],
        "textures": profile.get("textures", []),
        "resources": matches[0].get("resources", []),
    }


def constant_rows_at_time(
    material_map: dict[str, Any],
    wire: dict[str, Any],
    parameters: dict[str, Any],
    game_time_seconds: float,
) -> list[list[float]]:
    """Evaluate the cooked expression trees into CB0 for one game time."""
    uniform = material_map["uniformExpressionSet"]
    scalar_map = {
        (row["parameterName"].casefold(), row["parameterNameNumber"]):
            row["value"]
        for row in parameters["scalars"]
    }
    vector_map = {
        (row["parameterName"].casefold(), row["parameterNameNumber"]):
            row["value"]
        for row in parameters["vectors"]
    }

    def evaluate(expression: Any) -> list[float]:
        return evaluate_expression(
            expression, scalar_map, vector_map,
            game_time_seconds=game_time_seconds,
            real_time_seconds=game_time_seconds)

    scalars = [evaluate(row)[0] for row in uniform["pixelScalarExpressions"]]
    vectors = [evaluate(row) for row in uniform["pixelVectorExpressions"]]

    declared = int(wire["constantBufferClosure"][
        "declaredConstantBuffer0Float4Count"])
    rows: list[list[float]] = [[0.0, 0.0, 0.0, 0.0] for _ in range(declared)]
    for group_row in wire["scalarGroups"]:
        group = int(group_row["expressionIndexOrGroup"])
        slot = int(group_row["baseIndex"]) // 16
        value = [0.0, 0.0, 0.0, 0.0]
        for lane in range(4):
            index = group * 4 + lane
            if index < len(scalars):
                value[lane] = scalars[index]
        rows[slot] = value
    for vector_row in wire["vectors"]:
        index = int(vector_row["expressionIndexOrGroup"])
        rows[int(vector_row["baseIndex"]) // 16] = vectors[index]
    return rows


def compare(left: list[list[float]], right: list[list[float]]) -> float:
    require(len(left) == len(right),
            "replay output render-target counts differ")
    worst = 0.0
    for target, (a_row, b_row) in enumerate(zip(left, right)):
        require(len(a_row) == len(b_row),
                f"replay output shape differs at render target {target}")
        for component, (a, b) in enumerate(zip(a_row, b_row)):
            require(
                math.isfinite(a) and math.isfinite(b),
                "replay output contains NaN/Inf at render target "
                f"{target} component {component}",
            )
            worst = max(worst, abs(a - b) / max(1.0, abs(a), abs(b)))
    return worst


def require_finite_rows(
    rows: list[list[float]],
    description: str,
) -> None:
    for row_index, row in enumerate(rows):
        require(len(row) == 4,
                f"{description} row {row_index} is not float4")
        for component, value in enumerate(row):
            require(
                math.isfinite(value),
                f"{description} contains NaN/Inf at row {row_index} "
                f"component {component}",
            )


def validate_sample_times(times: list[float]) -> list[float]:
    require(len(times) >= 2, "at least two game-time samples are required")
    normalized = [float(value) for value in times]
    require(all(math.isfinite(value) for value in normalized),
            "game-time samples must be finite")
    require(len(set(normalized)) == len(normalized),
            "game-time samples must be unique")
    return normalized


def require_input_pin(
    inputs: dict[str, Any],
    prefix: str,
    identity: dict[str, Any],
    description: str,
) -> None:
    require(inputs.get(f"{prefix}RawSha256") == identity["rawSha256"],
            f"named ABI pins different {description} bytes")
    require(inputs.get(f"{prefix}ByteSize") == identity["byteSize"],
            f"named ABI pins a different {description} byte size")


def load_provenance(
    parent: str,
    effect_asset_id: str,
    cache_path: Path,
    d3dcompiler: Path,
) -> tuple[
    dict[str, Any],
    dict[str, Any],
    dict[str, Any],
    dict[str, Any],
]:
    index_document, index_identity = read_artifact(
        SHADER_MAP_INDEX, SHADER_MAP_SCHEMA, "shader-map index")
    cooked_document, cooked_identity = read_artifact(
        COOKED_RECEIPT, COOKED_RECEIPT_SCHEMA, "cooked shader receipt")
    named_document, named_identity = read_artifact(
        NAMED_ABI_RECEIPT, NAMED_ABI_SCHEMA, "named ABI receipt")
    named_inputs = named_document.get("inputs")
    require(isinstance(named_inputs, dict),
            "named ABI receipt inputs must be an object")
    require(
        named_inputs.get("shaderMapArtifactSha256")
        == index_document["artifactSha256"],
        "named ABI pins a different shader-map artifact",
    )
    require(
        named_inputs.get("cookedPixelShadersArtifactSha256")
        == cooked_document["artifactSha256"],
        "named ABI pins a different cooked shader artifact",
    )
    require_input_pin(named_inputs, "shaderMap", index_identity,
                      "shader-map")
    require_input_pin(named_inputs, "cookedPixelShaders", cooked_identity,
                      "cooked shader receipt")

    cache_identity = raw_file_identity(cache_path, "RefShaderCache")
    compiler_identity = raw_file_identity(d3dcompiler, "D3DCompiler")
    require_input_pin(named_inputs, "refShaderCache", cache_identity,
                      "RefShaderCache")
    require_input_pin(named_inputs, "d3dCompiler", compiler_identity,
                      "D3DCompiler")
    require(named_inputs.get("refShaderCacheFileName") == cache_path.name,
            "named ABI pins a different RefShaderCache filename")
    require(named_inputs.get("d3dCompilerFileName") == d3dcompiler.name,
            "named ABI pins a different D3DCompiler filename")

    index_rows = index_document.get("families")
    require(isinstance(index_rows, list),
            "shader-map index families must be an array")
    index_matches = [row for row in index_rows
                     if isinstance(row, dict)
                     and row.get("parentMaterialPath") == parent]
    require(len(index_matches) == 1,
            f"family is absent or duplicated in shader-map index: {parent}")
    cooked_rows = cooked_document.get("families")
    require(isinstance(cooked_rows, list),
            "cooked shader receipt families must be an array")
    cooked_matches = [row for row in cooked_rows
                      if isinstance(row, dict)
                      and row.get("parentMaterialPath") == parent
                      and row.get("status") == "EXTRACTED"]
    require(len(cooked_matches) == 1,
            f"family has no unique cooked program: {parent}")
    cooked_row = cooked_matches[0]
    named_rows = named_document.get("families")
    require(isinstance(named_rows, list),
            "named ABI receipt families must be an array")
    named_matches = [row for row in named_rows
                     if isinstance(row, dict)
                     and row.get("parentMaterialPath") == parent]
    require(len(named_matches) == 1,
            f"family is absent or duplicated in named ABI: {parent}")
    named_row = named_matches[0]
    require(named_row.get("status") == "RESOLVED_NAMED_MAPPING",
            f"family has no resolved named mapping: {parent}")
    require(named_row.get("dxbcSha256") == cooked_row.get("dxbcSha256"),
            "named ABI and cooked receipt disagree on DXBC identity")

    dxbc_digest = cooked_row["dxbcSha256"]
    dxbc_path = REPOSITORY_ROOT / "Data/Effects/CookedShaders" / (
        f"{dxbc_digest}.dxbc")
    dxbc_identity = raw_file_identity(dxbc_path, "FlowTrail cooked DXBC")
    require(dxbc_identity["rawSha256"] == dxbc_digest,
            "FlowTrail cooked DXBC raw SHA-256 drifted")
    require(dxbc_identity["byteSize"] == cooked_row.get("dxbcByteSize"),
            "FlowTrail cooked DXBC byte size drifted")

    authored_path = AUTHORED_DIRECTORY / f"{effect_asset_id}.effect.json"
    authored_identity = raw_file_identity(
        authored_path, "authored effect document")
    provenance = {
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
        "namedAbi": "Data/Effects/Contracts/effect-family-named-abi.v1.json",
        "namedAbiArtifactSha256": named_document["artifactSha256"],
        "namedAbiRawSha256": named_identity["rawSha256"],
        "namedAbiByteSize": named_identity["byteSize"],
        "authoredDocument":
            f"Data/Effects/Authored/{effect_asset_id}.effect.json",
        "authoredDocumentRawSha256": authored_identity["rawSha256"],
        "authoredDocumentByteSize": authored_identity["byteSize"],
        "cookedDxbcFileName": dxbc_path.name,
        "cookedDxbcRawSha256": dxbc_identity["rawSha256"],
        "cookedDxbcByteSize": dxbc_identity["byteSize"],
        "refShaderCacheFileName": cache_path.name,
        "refShaderCacheRawSha256": cache_identity["rawSha256"],
        "refShaderCacheByteSize": cache_identity["byteSize"],
        "d3dCompilerFileName": d3dcompiler.name,
        "d3dCompilerRawSha256": compiler_identity["rawSha256"],
        "d3dCompilerByteSize": compiler_identity["byteSize"],
    }
    return index_matches[0], cooked_row, named_row, provenance


def verify(
    parent: str,
    effect_asset_id: str,
    element_id: str,
    times: list[float],
    cache_path: Path,
    d3dcompiler: Path,
) -> tuple[dict[str, Any], dict[str, Any]]:
    times = validate_sample_times(times)
    parameters = occurrence_parameters(effect_asset_id, element_id)
    require(parameters["parentMaterialPath"] == parent,
            "occurrence does not belong to the requested family")
    index_row, cooked_row, named_row, provenance = load_provenance(
        parent, effect_asset_id, cache_path, d3dcompiler)

    cache = package_tables(cache_path)
    layout = parse_shader_code_layout(cache)
    disassembler = D3DDisassembler(d3dcompiler)
    material_map = resolve_material_map(index_row, cooked_row, cache,
                                        layout)
    reference = {"shaderType": cooked_row["shaderType"],
                 "shaderIdHex": cooked_row["shaderIdHex"]}
    bytecode = bytes(
        extract_selected_packed_dxbc(cache, layout, [reference])[
            reference["shaderIdHex"]]["_bytecode"])
    require(hashlib.sha256(bytecode).hexdigest() == cooked_row["dxbcSha256"],
            "cache DXBC differs from the cooked receipt")
    disassembly = disassembler.disassemble(bytecode)
    payload = extract_selected_shader_objects(
        cache, layout, [reference])["byShaderId"][reference["shaderIdHex"]]
    _, wire, _ = resolve_named_abi_binding(
        disassembly, payload, material_map["uniformExpressionCounts"])

    chunks = dxbc_chunks(bytecode)
    input_signature = parse_signature(chunks["ISGN"])
    output_signature = parse_signature(chunks["OSGN"])
    source = build_standalone_source(
        disassembly, input_signature, output_signature)
    compiler = CarrierCompiler(d3dcompiler)
    translated = compile_pixel_shader(
        compiler, source.encode("utf-8"), parent.rsplit(".", 1)[-1])
    carrier_source, carrier_rows = build_carrier_source(input_signature)
    carrier_dxbc, _ = compiler.compile(carrier_source, "carrier")

    declarations = parse_declarations(disassembly["declarations"])
    replay_decl = replay_declarations(declarations)
    render_targets = max(
        (row["register"] for row in output_signature
         if row["register"] < 8), default=0) + 1
    engine_rows = {
        register: [[0.5, 0.5, 0.5, 1.0] for _ in range(count)]
        for register, count in replay_decl[
            "constantBufferFloat4Counts"].items()
        if register != "0"
    }
    carrier = [[0.37, 0.61, 0.5, 1.0] for _ in range(carrier_rows)]
    textures = {str(register): [0.6, 0.7, 0.8, 1.0]
                for register in replay_decl["textureRegisters"]}

    replay = GenericWarpReplay()
    samples = []
    for game_time in times:
        cb0 = constant_rows_at_time(material_map, wire, parameters, game_time)
        require_finite_rows(cb0, "offline reconstructed CB0")
        fixture = {
            "constantBuffers": {"0": cb0, **engine_rows},
            "carrierRows": carrier,
            "textures": textures,
        }
        original = replay.run(bytecode, carrier_dxbc, fixture, replay_decl,
                              render_targets)
        produced = replay.run(translated, carrier_dxbc, fixture, replay_decl,
                              render_targets)
        samples.append({
            "gameTimeSeconds": game_time,
            "worstRelativeDelta": compare(original, produced),
            "constantRows": cb0,
            "renderTarget0": original[0],
        })

    parity = max(row["worstRelativeDelta"] for row in samples)

    # This is an offline evaluator gate only.  It proves the reconstructed CB0
    # can change with time; it never exercises the renderer's upload path.
    moving_rows: dict[int, float] = {}
    baseline = samples[0]["constantRows"]
    for later in samples[1:]:
        require(len(baseline) == len(later["constantRows"]),
                "offline reconstructed CB0 row counts differ across time")
        for slot, (a_row, b_row) in enumerate(zip(baseline,
                                                  later["constantRows"])):
            delta = max(abs(a - b) for a, b in zip(a_row, b_row))
            if delta > moving_rows.get(slot, 0.0):
                moving_rows[slot] = delta
    moving_rows = {slot: value for slot, value in moving_rows.items()
                   if value > MOTION_THRESHOLD}
    constant_motion = max(moving_rows.values(), default=0.0)

    output_motion = 0.0
    for later in samples[1:]:
        for a, b in zip(samples[0]["renderTarget0"], later["renderTarget0"]):
            output_motion = max(output_motion, abs(a - b))

    bound_rows = sorted({
        f"cb0[{int(row['baseIndex']) // 16}]"
        for row in wire["scalarGroups"] + wire["vectors"]
    })
    expected_time_rows = sorted({
        match.group(0)
        for register in named_row["summary"]["timeDependentRegisters"]
        for match in [re.match(r"cb0\[\d+\]", register)]
        if match
    })
    moving_row_names = sorted(f"cb0[{slot}]" for slot in moving_rows)
    require(set(moving_row_names).issubset(expected_time_rows),
            "offline CB0 changed outside named time-dependent rows")
    row = {
        "parentMaterialPath": parent,
        "effectAssetId": effect_asset_id,
        "elementId": element_id,
        "sourceMaterialPath": parameters["sourceMaterialPath"],
        "sourceProfileSemanticStatus":
            parameters["sourceProfileSemanticStatus"],
        "renderProfile": parameters["renderProfile"],
        "dxbcSha256": cooked_row["dxbcSha256"],
        "instructionCount": disassembly["instructionCount"],
        "gameTimesSampled": times,
        "worstParityDelta": parity,
        "constantMotionAcrossTime": constant_motion,
        "movingConstantRows": moving_row_names,
        "expectedTimeDependentRows": expected_time_rows,
        "outputMotionAcrossTime": output_motion,
        "outputMotionCaveat":
            "REPLAY_BINDS_1X1_TEXTURES_SO_UV_PANNING_CANNOT_SHOW",
        "boundConstantRows": bound_rows,
        "fixtureBoundary": {
            "translationSource":
                "REGENERATED_FROM_CURRENT_TRANSLATOR_NOT_CHECKED_HLSLI",
            "carrierInputs": "SYNTHETIC_CONSTANT_FLOAT4_ROWS",
            "engineConstantBuffers": "SYNTHETIC_CONSTANT_FLOAT4_ROWS",
            "textures": "SYNTHETIC_CONSTANT_1X1_RGBA",
            "runtimeRendererExecuted": False,
        },
        "samples": [
            {key: value for key, value in row.items()
             if key != "constantRows"}
            for row in samples
        ],
        "parityResult": (
            PARITY_PASS if parity <= PRINT_PRECISION_TOLERANCE
            else PARITY_FAIL),
        "motionResult": (
            MOTION_PASS if constant_motion > MOTION_THRESHOLD
            else MOTION_FAIL),
        "admits": ADMITS,
    }
    return row, provenance


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--parent", required=True)
    parser.add_argument("--effect-asset-id", required=True)
    parser.add_argument("--element-id", required=True)
    parser.add_argument("--times", type=float, nargs="+",
                        default=list(DEFAULT_TIMES))
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--d3dcompiler", type=Path,
                        default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    arguments = parser.parse_args(argv)

    try:
        normalized_times = validate_sample_times(arguments.times)
        canonical_request = (
            arguments.parent == CANONICAL_PARENT
            and arguments.effect_asset_id == CANONICAL_EFFECT_ASSET_ID
            and arguments.element_id == CANONICAL_ELEMENT_ID
            and normalized_times == list(DEFAULT_TIMES)
        )
        require(
            arguments.output.resolve() != DEFAULT_OUTPUT.resolve()
            or canonical_request,
            "non-canonical occurrence/times cannot overwrite the canonical "
            "time-varying parity receipt",
        )
        row, provenance = verify(
            arguments.parent, arguments.effect_asset_id, arguments.element_id,
            normalized_times, arguments.cache.resolve(),
            arguments.d3dcompiler.resolve())
    except (ParityError, NamedAbiError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    passed = (row["parityResult"] == PARITY_PASS
              and row["motionResult"] == MOTION_PASS)
    if not passed:
        print(
            f"FAIL: {row['parityResult']} {row['motionResult']}; "
            "existing receipt was preserved",
            file=sys.stderr,
        )
        return 1

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "admits": ADMITS,
            "proves": [
                "DXBC_AND_REGENERATED_TRANSLATION_AGREE_AT_RECONSTRUCTED_CB0",
                "OFFLINE_EVALUATOR_PRODUCES_TIME_VARYING_CB0_ROWS",
            ],
            "doesNotProve": [
                "CHECKED_HLSLI_ARTIFACT_IDENTITY",
                "ACTUAL_TEXTURE_OR_SAMPLER_SEMANTICS",
                "ACTUAL_CARRIER_OR_VERTEX_FACTORY_INPUTS",
                "ENGINE_OWNED_CONSTANT_BUFFER_VALUES",
                "RUNTIME_CONSTANT_BUFFER_UPLOAD",
                "RENDER_STATE_OR_PASS_OR_MRT",
                "UV_OR_OUTPUT_MOTION",
                "VISUAL_FIDELITY_OR_PRODUCT_ADMISSION",
            ],
        },
        "inputs": provenance,
        "occurrence": row,
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
    print(f"RESULT: {row['parityResult']} delta={row['worstParityDelta']:.3e} "
          f"{row['motionResult']} rows={row['movingConstantRows']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
