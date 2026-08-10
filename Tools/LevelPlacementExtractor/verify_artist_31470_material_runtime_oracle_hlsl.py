#!/usr/bin/env python3
"""Execute the Artist F Material numeric oracle through D3D11 WARP HLSL."""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import struct
from pathlib import Path
from typing import Any, Iterable


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-runtime-oracle.receipt.json"
)
DEFAULT_HLSL = REPO_ROOT / (
    "Tools/MaterialEvaluatorHarness/Shader_Artist31470MaterialOracle.hlsl"
)
DEFAULT_D3DCOMPILER = Path(
    r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll"
)
EXPECTED_D3DCOMPILER_SHA256 = (
    "ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8"
)

D3D_DRIVER_TYPE_WARP = 5
D3D11_SDK_VERSION = 7
D3D11_USAGE_DEFAULT = 0
D3D11_USAGE_STAGING = 3
D3D11_BIND_SHADER_RESOURCE = 0x8
D3D11_BIND_UNORDERED_ACCESS = 0x80
D3D11_CPU_ACCESS_READ = 0x20000
D3D11_RESOURCE_MISC_BUFFER_STRUCTURED = 0x40
D3D11_SRV_DIMENSION_BUFFER = 1
D3D11_UAV_DIMENSION_BUFFER = 1
D3D11_MAP_READ = 1


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def tracked_text_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    return data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")


def read_receipt(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), "Material runtime oracle receipt is not an object")
    return value


class BufferDesc(ctypes.Structure):
    _fields_ = [
        ("ByteWidth", ctypes.c_uint32),
        ("Usage", ctypes.c_uint32),
        ("BindFlags", ctypes.c_uint32),
        ("CPUAccessFlags", ctypes.c_uint32),
        ("MiscFlags", ctypes.c_uint32),
        ("StructureByteStride", ctypes.c_uint32),
    ]


class SubresourceData(ctypes.Structure):
    _fields_ = [
        ("pSysMem", ctypes.c_void_p),
        ("SysMemPitch", ctypes.c_uint32),
        ("SysMemSlicePitch", ctypes.c_uint32),
    ]


class ShaderResourceViewDesc(ctypes.Structure):
    _fields_ = [
        ("Format", ctypes.c_uint32),
        ("ViewDimension", ctypes.c_uint32),
        ("Data", ctypes.c_uint32 * 4),
    ]


class UnorderedAccessViewDesc(ctypes.Structure):
    _fields_ = [
        ("Format", ctypes.c_uint32),
        ("ViewDimension", ctypes.c_uint32),
        ("Data", ctypes.c_uint32 * 4),
    ]


class MappedSubresource(ctypes.Structure):
    _fields_ = [
        ("pData", ctypes.c_void_p),
        ("RowPitch", ctypes.c_uint32),
        ("DepthPitch", ctypes.c_uint32),
    ]


def com_method(
    pointer: ctypes.c_void_p,
    index: int,
    result_type: Any,
    *argument_types: Any,
) -> Any:
    table = ctypes.cast(
        pointer, ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))
    ).contents
    return ctypes.WINFUNCTYPE(
        result_type, ctypes.c_void_p, *argument_types
    )(table[index])


def release(pointer: ctypes.c_void_p) -> None:
    if pointer and pointer.value:
        com_method(pointer, 2, ctypes.c_ulong)(pointer)
        pointer.value = None


def blob_bytes(pointer: ctypes.c_void_p) -> bytes:
    get_pointer = com_method(pointer, 3, ctypes.c_void_p)
    get_size = com_method(pointer, 4, ctypes.c_size_t)
    return ctypes.string_at(get_pointer(pointer), get_size(pointer))


def compile_hlsl(hlsl_path: Path, compiler_path: Path) -> tuple[bytes, dict[str, Any]]:
    require(hlsl_path.is_file(), f"HLSL oracle is missing: {hlsl_path}")
    require(compiler_path.is_file(), f"D3D compiler is missing: {compiler_path}")
    compiler_bytes = compiler_path.read_bytes()
    require(sha256(compiler_bytes) == EXPECTED_D3DCOMPILER_SHA256, "D3D compiler identity changed")
    source = tracked_text_bytes(hlsl_path)
    dll = ctypes.WinDLL(str(compiler_path))
    compile_function = dll.D3DCompile
    compile_function.argtypes = [
        ctypes.c_void_p,
        ctypes.c_size_t,
        ctypes.c_char_p,
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_char_p,
        ctypes.c_char_p,
        ctypes.c_uint32,
        ctypes.c_uint32,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_void_p),
    ]
    compile_function.restype = ctypes.c_long
    source_buffer = ctypes.create_string_buffer(source)
    code = ctypes.c_void_p()
    errors = ctypes.c_void_p()
    result = compile_function(
        source_buffer,
        len(source),
        hlsl_path.name.encode("ascii"),
        None,
        None,
        b"main",
        b"cs_5_0",
        0,
        0,
        ctypes.byref(code),
        ctypes.byref(errors),
    )
    try:
        error_text = blob_bytes(errors).decode("utf-8", "replace") if errors.value else ""
        require(result >= 0 and code.value, f"D3DCompile failed 0x{result & 0xFFFFFFFF:08x}: {error_text}")
        bytecode = blob_bytes(code)
    finally:
        release(errors)
        release(code)
    return bytecode, {
        "fileName": compiler_path.name,
        "byteSize": len(compiler_bytes),
        "rawSha256": sha256(compiler_bytes),
        "hashRole": "EXTERNAL_RAW_BYTES",
    }


def oracle_input_bytes(receipt: dict[str, Any]) -> tuple[bytes, list[list[float]]]:
    contract = receipt.get("evaluatorContract") or {}
    samples = contract.get("inputSamples") or []
    families = receipt.get("familyEvaluators") or []
    recipes = receipt.get("materialRecipeBindings") or []
    require(len(families) == 23 and len(recipes) == 27 and len(samples) == 4, "Material HLSL oracle denominator changed")
    payload = bytearray()
    expected: list[list[float]] = []

    def append_sample(
        feature_mask: int,
        identity_index: int,
        sample: dict[str, Any],
        expected_float4: list[float],
    ) -> None:
        uv_scale = list(sample["uvScale"]) + [0.0, 0.0]
        values = (
            list(sample["panRotationAux"])
            + list(sample["texture0"])
            + list(sample["texture1"])
            + list(sample["color"])
            + list(sample["params0"])
            + list(sample["params1"])
        )
        payload.extend(
            struct.pack(
                "<IIff" + "f" * 28,
                int(feature_mask),
                identity_index,
                float(sample["time"]),
                0.0,
                *[float(value) for value in uv_scale + values],
            )
        )
        expected.append([float(value) for value in expected_float4])

    for family_index, family in enumerate(families):
        require(len(family.get("sampleRows") or []) == len(samples), "family sample rows changed")
        for sample_index, sample in enumerate(samples):
            append_sample(
                int(family["featureMask"]),
                family_index,
                sample,
                family["sampleRows"][sample_index]["expectedFloat4"],
            )
    for recipe_index, recipe in enumerate(recipes):
        recipe_samples = recipe.get("numericBindingSamples") or []
        require(len(recipe_samples) == len(samples), "recipe sample rows changed")
        for row in recipe_samples:
            append_sample(
                int(recipe["recipeFeatureMask"]),
                len(families) + recipe_index,
                row["input"],
                row["expectedFloat4"],
            )
    require(len(payload) == len(expected) * 128, "Material HLSL oracle input ABI changed")
    return bytes(payload), expected


def execute_compute(bytecode: bytes, input_bytes: bytes, output_count: int) -> bytes:
    d3d11 = ctypes.WinDLL("d3d11")
    create_device = d3d11.D3D11CreateDevice
    create_device.argtypes = [
        ctypes.c_void_p,
        ctypes.c_uint32,
        ctypes.c_void_p,
        ctypes.c_uint32,
        ctypes.c_void_p,
        ctypes.c_uint32,
        ctypes.c_uint32,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_uint32),
        ctypes.POINTER(ctypes.c_void_p),
    ]
    create_device.restype = ctypes.c_long
    device = ctypes.c_void_p()
    context = ctypes.c_void_p()
    feature_level = ctypes.c_uint32()
    result = create_device(
        None,
        D3D_DRIVER_TYPE_WARP,
        None,
        0,
        None,
        0,
        D3D11_SDK_VERSION,
        ctypes.byref(device),
        ctypes.byref(feature_level),
        ctypes.byref(context),
    )
    require(result >= 0 and device.value and context.value, f"D3D11 WARP creation failed: 0x{result & 0xFFFFFFFF:08x}")

    input_buffer = ctypes.c_void_p()
    input_view = ctypes.c_void_p()
    output_buffer = ctypes.c_void_p()
    output_view = ctypes.c_void_p()
    staging_buffer = ctypes.c_void_p()
    shader = ctypes.c_void_p()
    try:
        create_buffer = com_method(
            device,
            3,
            ctypes.c_long,
            ctypes.POINTER(BufferDesc),
            ctypes.POINTER(SubresourceData),
            ctypes.POINTER(ctypes.c_void_p),
        )
        create_srv = com_method(
            device,
            7,
            ctypes.c_long,
            ctypes.c_void_p,
            ctypes.POINTER(ShaderResourceViewDesc),
            ctypes.POINTER(ctypes.c_void_p),
        )
        create_uav = com_method(
            device,
            8,
            ctypes.c_long,
            ctypes.c_void_p,
            ctypes.POINTER(UnorderedAccessViewDesc),
            ctypes.POINTER(ctypes.c_void_p),
        )
        create_compute_shader = com_method(
            device,
            18,
            ctypes.c_long,
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_void_p),
        )

        input_desc = BufferDesc(
            len(input_bytes),
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
            128,
        )
        input_data_buffer = ctypes.create_string_buffer(input_bytes)
        input_data = SubresourceData(
            ctypes.cast(input_data_buffer, ctypes.c_void_p), 0, 0
        )
        result = create_buffer(device, ctypes.byref(input_desc), ctypes.byref(input_data), ctypes.byref(input_buffer))
        require(result >= 0 and input_buffer.value, f"D3D11 input buffer creation failed: 0x{result & 0xFFFFFFFF:08x}")

        srv_desc = ShaderResourceViewDesc()
        srv_desc.Format = 0
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER
        srv_desc.Data[0] = 0
        srv_desc.Data[1] = output_count
        result = create_srv(device, input_buffer, ctypes.byref(srv_desc), ctypes.byref(input_view))
        require(result >= 0 and input_view.value, f"D3D11 input SRV creation failed: 0x{result & 0xFFFFFFFF:08x}")

        output_bytes = output_count * 16
        output_desc = BufferDesc(
            output_bytes,
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_UNORDERED_ACCESS,
            0,
            D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
            16,
        )
        result = create_buffer(device, ctypes.byref(output_desc), None, ctypes.byref(output_buffer))
        require(result >= 0 and output_buffer.value, f"D3D11 output buffer creation failed: 0x{result & 0xFFFFFFFF:08x}")

        uav_desc = UnorderedAccessViewDesc()
        uav_desc.Format = 0
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER
        uav_desc.Data[0] = 0
        uav_desc.Data[1] = output_count
        uav_desc.Data[2] = 0
        result = create_uav(device, output_buffer, ctypes.byref(uav_desc), ctypes.byref(output_view))
        require(result >= 0 and output_view.value, f"D3D11 output UAV creation failed: 0x{result & 0xFFFFFFFF:08x}")

        staging_desc = BufferDesc(
            output_bytes,
            D3D11_USAGE_STAGING,
            0,
            D3D11_CPU_ACCESS_READ,
            D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
            16,
        )
        result = create_buffer(device, ctypes.byref(staging_desc), None, ctypes.byref(staging_buffer))
        require(result >= 0 and staging_buffer.value, f"D3D11 staging buffer creation failed: 0x{result & 0xFFFFFFFF:08x}")

        bytecode_buffer = ctypes.create_string_buffer(bytecode)
        result = create_compute_shader(
            device,
            bytecode_buffer,
            len(bytecode),
            None,
            ctypes.byref(shader),
        )
        require(result >= 0 and shader.value, f"D3D11 compute shader creation failed: 0x{result & 0xFFFFFFFF:08x}")

        cs_set_srvs = com_method(context, 67, None, ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_void_p))
        cs_set_uavs = com_method(context, 68, None, ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_void_p), ctypes.c_void_p)
        cs_set_shader = com_method(context, 69, None, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32)
        dispatch = com_method(context, 41, None, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32)
        copy_resource = com_method(context, 47, None, ctypes.c_void_p, ctypes.c_void_p)
        map_resource = com_method(context, 14, ctypes.c_long, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(MappedSubresource))
        unmap_resource = com_method(context, 15, None, ctypes.c_void_p, ctypes.c_uint32)

        srv_array = (ctypes.c_void_p * 1)(input_view.value)
        uav_array = (ctypes.c_void_p * 1)(output_view.value)
        cs_set_srvs(context, 0, 1, srv_array)
        cs_set_uavs(context, 0, 1, uav_array, None)
        cs_set_shader(context, shader, None, 0)
        dispatch(context, (output_count + 63) // 64, 1, 1)
        copy_resource(context, staging_buffer, output_buffer)

        mapped = MappedSubresource()
        result = map_resource(context, staging_buffer, 0, D3D11_MAP_READ, 0, ctypes.byref(mapped))
        require(result >= 0 and mapped.pData, f"D3D11 staging map failed: 0x{result & 0xFFFFFFFF:08x}")
        try:
            return ctypes.string_at(mapped.pData, output_bytes)
        finally:
            unmap_resource(context, staging_buffer, 0)
    finally:
        release(shader)
        release(staging_buffer)
        release(output_view)
        release(output_buffer)
        release(input_view)
        release(input_buffer)
        release(context)
        release(device)


def run_hlsl_oracle(
    receipt: dict[str, Any], hlsl_path: Path, compiler_path: Path
) -> dict[str, Any]:
    bytecode, compiler_identity = compile_hlsl(hlsl_path, compiler_path)
    input_bytes, expected = oracle_input_bytes(receipt)
    raw_output = execute_compute(bytecode, input_bytes, len(expected))
    actual = [
        list(struct.unpack_from("<4f", raw_output, index * 16))
        for index in range(len(expected))
    ]
    tolerance = float((receipt.get("evaluatorContract") or {}).get("numericTolerance") or 0.0)
    require(tolerance > 0.0, "Material HLSL tolerance is invalid")
    max_error = 0.0
    for expected_row, actual_row in zip(expected, actual, strict=True):
        for expected_value, actual_value in zip(expected_row, actual_row, strict=True):
            max_error = max(max_error, abs(expected_value - actual_value))
    require(max_error <= tolerance, f"Material HLSL numeric oracle mismatch: {max_error} > {tolerance}")
    return {
        "verified": True,
        "backend": "D3D11_WARP_COMPUTE",
        "entryPoint": "main",
        "targetProfile": "cs_5_0",
        "compiler": compiler_identity,
        "hlslTrackedTextSha256": sha256(tracked_text_bytes(hlsl_path)),
        "compiledDxbcSha256": sha256(bytecode),
        "sampleCount": len(expected),
        "inputBytesSha256": sha256(input_bytes),
        "outputFloat32BytesSha256": sha256(raw_output),
        "numericTolerance": tolerance,
        "maxAbsoluteError": max_error,
    }


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--receipt", type=Path, default=DEFAULT_RECEIPT)
    parser.add_argument("--hlsl", type=Path, default=DEFAULT_HLSL)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    receipt = read_receipt(args.receipt)
    from build_artist_31470_material_runtime_oracle import (
        DEFAULT_MATERIAL_CONTRACT,
        read_json,
        validate_runtime_receipt,
        validate_runtime_receipt_source_bindings,
    )

    validate_runtime_receipt(receipt)
    validate_runtime_receipt_source_bindings(
        receipt, read_json(DEFAULT_MATERIAL_CONTRACT)
    )
    result = run_hlsl_oracle(receipt, args.hlsl, args.d3dcompiler)
    expected = receipt.get("hlslVerification") or {}
    require(expected == result, "stored Material HLSL verification is stale")
    print(
        "PASS: Artist F Material HLSL oracle "
        f"samples={result['sampleCount']} maxError={result['maxAbsoluteError']:.9g}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
