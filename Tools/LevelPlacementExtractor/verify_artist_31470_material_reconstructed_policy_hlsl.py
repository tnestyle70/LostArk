#!/usr/bin/env python3
"""Replay all reconstructed Material policy rows through D3D11 WARP."""

from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import json
import math
import struct
from pathlib import Path
from typing import Any, Iterable

import effect_source_contract_io as effect_source_contract_io_module
import verify_artist_31470_material_runtime_oracle_hlsl as runtime_warp_support_module

from verify_artist_31470_material_runtime_oracle_hlsl import (
    D3D11_COMPARISON_LESS,
    D3D11_COMPARISON_NEVER,
    D3D11_CULL_BACK,
    D3D11_CULL_NONE,
    D3D11_DEPTH_WRITE_MASK_ALL,
    D3D11_FILL_SOLID,
    DepthStencilDesc,
    RasterizerDesc,
    SamplerDesc,
    ShaderResourceViewDesc,
    compile_hlsl,
    com_method,
    create_warp_device,
    execute_compute,
    release,
    round_trip_state_desc,
    sha256,
    tracked_text_bytes,
)


def discover_repository_root() -> Path:
    current = Path.cwd().resolve()
    for candidate in (current, *current.parents):
        if (
            (candidate / ".git").exists()
            and (candidate / "Tools/LevelPlacementExtractor").is_dir()
            and (candidate / "Data/Effects/Imported/Artist/Materials").is_dir()
        ):
            return candidate
    raise RuntimeError("cannot locate canonical LostArk repository root from current working directory")


ROOT = discover_repository_root()
DEFAULT_RECEIPT = ROOT / "Data/Effects/Imported/Artist/Materials/skill.31470.material-reconstructed-approved-v1.receipt.json"
DEFAULT_HLSL = ROOT / "Tools/MaterialEvaluatorHarness/Shader_Artist31470MaterialReconstructedPolicy.hlsl"
DEFAULT_D3DCOMPILER = Path(r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll")

POLICY_KIND_CODES = {
    "RENDER_STATE": 1,
    "STATIC_PERMUTATION": 2,
    "SAMPLER_DESCRIPTOR": 3,
}
STATIC_DECISION_CODES = {
    "SOURCE_ARCHIVE_MIC_EXACT_OVERRIDE_RETAINED_AS_RECONSTRUCTION_POLICY": 1,
    "SOURCE_ARCHIVE_MIC_NONOVERRIDE_PARENT_DEFAULT_POLICY": 2,
    "SOURCE_ARCHIVE_PARENT_STATIC_DEFAULT_POLICY": 3,
}

DXGI_FORMAT_R8G8B8A8_TYPELESS = 27
DXGI_FORMAT_R8G8B8A8_UNORM = 28
DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29
D3D11_USAGE_DEFAULT = 0
D3D11_BIND_SHADER_RESOURCE = 0x8
D3D11_SRV_DIMENSION_TEXTURE2D = 4


class SampleDesc(ctypes.Structure):
    _fields_ = [("Count", ctypes.c_uint32), ("Quality", ctypes.c_uint32)]


class Texture2DDesc(ctypes.Structure):
    _fields_ = [
        ("Width", ctypes.c_uint32),
        ("Height", ctypes.c_uint32),
        ("MipLevels", ctypes.c_uint32),
        ("ArraySize", ctypes.c_uint32),
        ("Format", ctypes.c_uint32),
        ("SampleDesc", SampleDesc),
        ("Usage", ctypes.c_uint32),
        ("BindFlags", ctypes.c_uint32),
        ("CPUAccessFlags", ctypes.c_uint32),
        ("MiscFlags", ctypes.c_uint32),
    ]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def validate_loaded_module_identities() -> None:
    identities = (
        (
            "RECONSTRUCTED_POLICY_WARP_VERIFIER",
            Path(__file__).resolve(),
            (ROOT / "Tools/LevelPlacementExtractor/verify_artist_31470_material_reconstructed_policy_hlsl.py").resolve(),
        ),
        (
            "STRICT_JSON_OBJECT_LOADER",
            Path(effect_source_contract_io_module.__file__).resolve(),
            (ROOT / "Tools/LevelPlacementExtractor/effect_source_contract_io.py").resolve(),
        ),
        (
            "RUNTIME_WARP_SUPPORT",
            Path(runtime_warp_support_module.__file__).resolve(),
            (ROOT / "Tools/LevelPlacementExtractor/verify_artist_31470_material_runtime_oracle_hlsl.py").resolve(),
        ),
    )
    for dependency_id, actual, expected in identities:
        require(
            actual == expected,
            f"loaded module path mismatch: {dependency_id} expected={expected} actual={actual}",
        )


def read_json(path: Path) -> dict[str, Any]:
    validate_loaded_module_identities()
    return effect_source_contract_io_module.load_strict_json_object(path)


def policy_rows(receipt: dict[str, Any]) -> list[dict[str, Any]]:
    rows = (
        list(receipt.get("renderStatePolicies") or [])
        + list(receipt.get("staticPermutationPolicies") or [])
        + list(receipt.get("samplerPolicies") or [])
    )
    require(len(rows) == 255, "policy row denominator changed")
    require([row.get("policyOrder") for row in rows] == list(range(255)), "policy row order changed")
    return rows


def selected_scalar(row: dict[str, Any]) -> float:
    selected = row["selectedValue"]
    selected_type = selected["type"]
    if selected_type == "BOOL":
        require(type(selected["value"]) is bool, "policy bool value has invalid type")
        return float(selected["value"])
    if selected_type == "FLOAT32":
        require(type(selected["value"]) is float and math.isfinite(selected["value"]), "policy float is invalid")
        return selected["value"]
    require(selected_type == "ENUM", "policy selected scalar type is invalid")
    ordinal = selected.get("ordinal")
    require(type(ordinal) is int and not isinstance(ordinal, bool), "policy enum ordinal is invalid")
    return float(ordinal)


def oracle_input_bytes(receipt: dict[str, Any]) -> tuple[bytes, list[list[float]], list[str]]:
    payload = bytearray()
    expected: list[list[float]] = []
    row_ids: list[str] = []
    for row in policy_rows(receipt):
        kind = POLICY_KIND_CODES[row["policyKind"]]
        field_code = 0
        decision_code = 0
        value0 = [0.0, 0.0, 0.0, 0.0]
        if kind == 1:
            from build_artist_31470_material_reconstructed_policy import RENDER_FIELD_CODES

            field_code = RENDER_FIELD_CODES[row["fieldName"]]
            value0[0] = selected_scalar(row)
        elif kind == 2:
            decision_code = STATIC_DECISION_CODES[row["providerBasis"]["basisId"]]
            value0[0] = selected_scalar(row)
        else:
            descriptor = row["selectedDescriptor"]
            field_code = int(descriptor["filter"]["d3d11"])
            decision_code = int(descriptor["addressU"]["d3d11"])
            value0[0] = float(descriptor["sRgb"])
        payload.extend(
            struct.pack(
                "<4I28f",
                kind,
                field_code,
                decision_code,
                int(row["selectedDescriptor"]["addressV"]["d3d11"]) if kind == 3 else 0,
                *value0,
                *([0.0] * 24),
            )
        )
        expected_row = [float(value) for value in row["numericOracle"]["expectedFloat4"]]
        require(all(math.isfinite(value) for value in expected_row), "policy expected output is non-finite")
        expected.append(expected_row)
        row_ids.append(row["policyRowId"])
    require(len(payload) == len(expected) * 128, "reconstructed policy HLSL input ABI changed")
    return bytes(payload), expected, row_ids


def run_hlsl_oracle(receipt: dict[str, Any], hlsl_path: Path, compiler_path: Path) -> dict[str, Any]:
    validate_loaded_module_identities()
    bytecode, compiler_identity = compile_hlsl(hlsl_path, compiler_path)
    input_bytes, expected, row_ids = oracle_input_bytes(receipt)
    raw_output = execute_compute(bytecode, input_bytes, len(expected))
    actual = [list(struct.unpack_from("<4f", raw_output, index * 16)) for index in range(len(expected))]
    row_results: list[dict[str, Any]] = []
    for row_id, expected_row, actual_row in zip(row_ids, expected, actual, strict=True):
        require(all(math.isfinite(value) for value in actual_row), f"non-finite WARP output: {row_id}")
        require(actual_row == expected_row, f"zero-tolerance WARP mismatch: {row_id}")
        row_results.append(
            {
                "policyRowId": row_id,
                "expectedFloat4": expected_row,
                "actualFloat4": actual_row,
                "numericTolerance": 0.0,
                "decision": "PASS",
            }
        )
    projection = json.dumps(row_results, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
    return {
        "verified": True,
        "backend": "D3D11_WARP_COMPUTE",
        "entryPoint": "main",
        "targetProfile": "cs_5_0",
        "compiler": compiler_identity,
        "hlslTrackedTextSha256": sha256(tracked_text_bytes(hlsl_path)),
        "compiledDxbcSha256": sha256(bytecode),
        "sampleCount": len(row_results),
        "inputBytesSha256": sha256(input_bytes),
        "outputFloat32BytesSha256": sha256(raw_output),
        "numericTolerance": 0.0,
        "maxAbsoluteError": 0.0,
        "rowResults": row_results,
        "rowResultsSha256": hashlib.sha256(projection).hexdigest(),
    }


def descriptor_result(
    row: dict[str, Any], descriptor_kind: str, expected: dict[str, Any], actual: dict[str, Any]
) -> dict[str, Any]:
    require(expected == actual, f"zero-tolerance WARP descriptor mismatch: {row['policyRowId']}")

    def require_finite(value: Any) -> None:
        if isinstance(value, dict):
            for child in value.values():
                require_finite(child)
        elif isinstance(value, list):
            for child in value:
                require_finite(child)
        elif type(value) in (int, float):
            require(math.isfinite(float(value)), f"non-finite WARP descriptor value: {row['policyRowId']}")

    require_finite(actual)
    return {
        "policyRowId": row["policyRowId"],
        "descriptorKind": descriptor_kind,
        "expectedDescriptor": expected,
        "actualDescriptor": actual,
        "numericTolerance": 0.0,
        "decision": "PASS",
    }


def depth_descriptor_projection(value: DepthStencilDesc) -> dict[str, Any]:
    def stencil_face(face: Any) -> dict[str, int]:
        return {
            "StencilFailOp": int(face.StencilFailOp),
            "StencilDepthFailOp": int(face.StencilDepthFailOp),
            "StencilPassOp": int(face.StencilPassOp),
            "StencilFunc": int(face.StencilFunc),
        }

    return {
        "DepthEnable": bool(value.DepthEnable),
        "DepthWriteMask": int(value.DepthWriteMask),
        "DepthFunc": int(value.DepthFunc),
        "StencilEnable": bool(value.StencilEnable),
        "StencilReadMask": int(value.StencilReadMask),
        "StencilWriteMask": int(value.StencilWriteMask),
        "FrontFace": stencil_face(value.FrontFace),
        "BackFace": stencil_face(value.BackFace),
    }


def rasterizer_descriptor_projection(value: RasterizerDesc) -> dict[str, Any]:
    return {
        "FillMode": int(value.FillMode),
        "CullMode": int(value.CullMode),
        "FrontCounterClockwise": bool(value.FrontCounterClockwise),
        "DepthBias": int(value.DepthBias),
        "DepthBiasClamp": float(value.DepthBiasClamp),
        "SlopeScaledDepthBias": float(value.SlopeScaledDepthBias),
        "DepthClipEnable": bool(value.DepthClipEnable),
        "ScissorEnable": bool(value.ScissorEnable),
        "MultisampleEnable": bool(value.MultisampleEnable),
        "AntialiasedLineEnable": bool(value.AntialiasedLineEnable),
    }


def sampler_descriptor_projection(value: SamplerDesc) -> dict[str, Any]:
    return {
        "Filter": int(value.Filter),
        "AddressU": int(value.AddressU),
        "AddressV": int(value.AddressV),
        "AddressW": int(value.AddressW),
        "MipLODBias": float(value.MipLODBias),
        "MaxAnisotropy": int(value.MaxAnisotropy),
        "ComparisonFunc": int(value.ComparisonFunc),
        "BorderColor": [float(value.BorderColor[index]) for index in range(4)],
        "MinLOD": float(value.MinLOD),
        "MaxLOD": float(value.MaxLOD),
    }


def run_warp_descriptor_oracle(receipt: dict[str, Any]) -> dict[str, Any]:
    validate_loaded_module_identities()
    device, context, feature_level = create_warp_device()
    texture = ctypes.c_void_p()
    try:
        results: list[dict[str, Any]] = []
        srv_results: list[dict[str, Any]] = []
        create_texture2d = com_method(
            device,
            5,
            ctypes.c_long,
            ctypes.POINTER(Texture2DDesc),
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_void_p),
        )
        texture_desc = Texture2DDesc(
            1,
            1,
            1,
            1,
            DXGI_FORMAT_R8G8B8A8_TYPELESS,
            SampleDesc(1, 0),
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0,
        )
        create_result = create_texture2d(device, ctypes.byref(texture_desc), None, ctypes.byref(texture))
        require(create_result >= 0 and texture.value, "WARP typeless texture creation failed")
        create_srv = com_method(
            device,
            7,
            ctypes.c_long,
            ctypes.c_void_p,
            ctypes.POINTER(ShaderResourceViewDesc),
            ctypes.POINTER(ctypes.c_void_p),
        )
        for row in policy_rows(receipt):
            oracle_id = row.get("d3dStateOracleId")
            if oracle_id is None:
                continue
            if oracle_id == "D3D11_DEPTH_STENCIL_DESC":
                disable_depth = row["selectedValue"]["value"]
                require(type(disable_depth) is bool, "depth policy value is invalid")
                descriptor = DepthStencilDesc()
                descriptor.DepthEnable = 0 if disable_depth else 1
                descriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL
                descriptor.DepthFunc = D3D11_COMPARISON_LESS
                descriptor.StencilEnable = 0
                descriptor.StencilReadMask = 0xFF
                descriptor.StencilWriteMask = 0xFF
                for face in (descriptor.FrontFace, descriptor.BackFace):
                    face.StencilFailOp = 1  # D3D11_STENCIL_OP_KEEP
                    face.StencilDepthFailOp = 1
                    face.StencilPassOp = 1
                    face.StencilFunc = 8  # D3D11_COMPARISON_ALWAYS
                expected = depth_descriptor_projection(descriptor)
                actual = depth_descriptor_projection(round_trip_state_desc(device, 21, descriptor))
                results.append(descriptor_result(row, oracle_id, expected, actual))
            elif oracle_id == "D3D11_RASTERIZER_DESC":
                two_sided = row["selectedValue"]["value"]
                require(type(two_sided) is bool, "two-sided policy value is invalid")
                descriptor = RasterizerDesc()
                descriptor.FillMode = D3D11_FILL_SOLID
                descriptor.CullMode = D3D11_CULL_NONE if two_sided else D3D11_CULL_BACK
                descriptor.DepthClipEnable = 1
                expected = rasterizer_descriptor_projection(descriptor)
                actual = rasterizer_descriptor_projection(round_trip_state_desc(device, 22, descriptor))
                results.append(descriptor_result(row, oracle_id, expected, actual))
            else:
                require(oracle_id == "D3D11_SAMPLER_DESC", "unknown WARP descriptor oracle")
                selected = row["selectedDescriptor"]
                descriptor = SamplerDesc()
                descriptor.Filter = selected["filter"]["d3d11"]
                descriptor.AddressU = selected["addressU"]["d3d11"]
                descriptor.AddressV = selected["addressV"]["d3d11"]
                descriptor.AddressW = selected["addressW"]["d3d11"]
                descriptor.MipLODBias = selected["mipLODBias"]
                descriptor.MaxAnisotropy = selected["maxAnisotropy"]
                descriptor.ComparisonFunc = selected["comparisonFunc"]["d3d11"]
                for index, value in enumerate(selected["borderColor"]):
                    descriptor.BorderColor[index] = value
                descriptor.MinLOD = selected["minLOD"]
                descriptor.MaxLOD = selected["maxLOD"]
                expected = sampler_descriptor_projection(descriptor)
                actual = sampler_descriptor_projection(round_trip_state_desc(device, 23, descriptor))
                results.append(descriptor_result(row, oracle_id, expected, actual))
                view = ctypes.c_void_p()
                try:
                    view_desc = ShaderResourceViewDesc()
                    view_desc.Format = (
                        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
                        if selected["sRgb"]
                        else DXGI_FORMAT_R8G8B8A8_UNORM
                    )
                    view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D
                    view_desc.Data[0] = 0
                    view_desc.Data[1] = 1
                    create_result = create_srv(
                        device,
                        texture,
                        ctypes.byref(view_desc),
                        ctypes.byref(view),
                    )
                    require(create_result >= 0 and view.value, f"WARP SRV creation failed: {row['policyRowId']}")
                    actual_desc = ShaderResourceViewDesc()
                    get_desc = com_method(view, 8, None, ctypes.POINTER(ShaderResourceViewDesc))
                    get_desc(view, ctypes.byref(actual_desc))
                    expected_srv = {
                        "Format": int(view_desc.Format),
                        "ViewDimension": int(view_desc.ViewDimension),
                        "MostDetailedMip": int(view_desc.Data[0]),
                        "MipLevels": int(view_desc.Data[1]),
                        "srvColorSpace": "SRGB" if selected["sRgb"] else "LINEAR",
                    }
                    actual_srv = {
                        "Format": int(actual_desc.Format),
                        "ViewDimension": int(actual_desc.ViewDimension),
                        "MostDetailedMip": int(actual_desc.Data[0]),
                        "MipLevels": int(actual_desc.Data[1]),
                        "srvColorSpace": (
                            "SRGB"
                            if actual_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
                            else "LINEAR"
                            if actual_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM
                            else "UNKNOWN"
                        ),
                    }
                    require(expected_srv == actual_srv, f"zero-tolerance WARP SRV mismatch: {row['policyRowId']}")
                    srv_results.append(
                        {
                            "policyRowId": row["policyRowId"],
                            "expectedSrv": expected_srv,
                            "actualSrv": actual_srv,
                            "numericTolerance": 0.0,
                            "decision": "PASS",
                        }
                    )
                finally:
                    release(view)
        require(len(results) == 107, "WARP descriptor row denominator changed")
        require(len(srv_results) == 72, "WARP SRV color-space row denominator changed")
        projection = json.dumps(results, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
        srv_projection = json.dumps(srv_results, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
        return {
            "verified": True,
            "backend": "D3D11_WARP_STATE_OBJECTS",
            "featureLevel": feature_level,
            "descriptorRowCount": len(results),
            "numericTolerance": 0.0,
            "rowResults": results,
            "rowResultsSha256": hashlib.sha256(projection).hexdigest(),
            "srvColorSpaceRowCount": len(srv_results),
            "srvRowResults": srv_results,
            "srvRowResultsSha256": hashlib.sha256(srv_projection).hexdigest(),
        }
    finally:
        release(texture)
        release(context)
        release(device)


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--receipt", type=Path, default=DEFAULT_RECEIPT)
    parser.add_argument("--hlsl", type=Path, default=DEFAULT_HLSL)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    validate_loaded_module_identities()
    args = parse_args(argv)
    receipt = read_json(args.receipt)
    import build_artist_31470_material_reconstructed_policy as policy_builder

    expected_builder = (
        ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_reconstructed_policy.py"
    ).resolve()
    actual_builder = Path(policy_builder.__file__).resolve()
    require(
        actual_builder == expected_builder,
        f"loaded module path mismatch: RECONSTRUCTED_POLICY_BUILDER expected={expected_builder} actual={actual_builder}",
    )

    policy_builder.validate_policy_receipt(
        receipt,
        read_json(policy_builder.DEFAULT_RUNTIME_RECEIPT),
        read_json(policy_builder.DEFAULT_ACQUISITION_RECEIPT),
        read_json(policy_builder.DEFAULT_MATERIAL_CONTRACT),
        hlsl_path=args.hlsl,
        verifier_path=policy_builder.DEFAULT_VERIFIER,
    )
    expected_hlsl = run_hlsl_oracle(receipt, args.hlsl, args.d3dcompiler)
    expected_warp = run_warp_descriptor_oracle(receipt)
    require(receipt["hlslVerification"] == expected_hlsl, "stored HLSL policy verification is stale")
    require(receipt["warpDescriptorVerification"] == expected_warp, "stored WARP descriptor verification is stale")
    print("PASS: Artist F reconstructed Material policy WARP rows=255 descriptors=107 tolerance=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
