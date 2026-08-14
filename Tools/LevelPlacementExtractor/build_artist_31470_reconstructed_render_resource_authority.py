#!/usr/bin/env python3
"""Build the immutable Artist 31470 reconstructed render-resource authority.

This offline sidecar joins the frozen 13-input reconstructed program and its
independently approved Material policy to the byte-exact DDS files in the
canonical main Client/Bin/Resources tree.  It records actual compressed DDS
and SRV descriptors separately from the earlier 1x1 RGBA policy fixtures.

The artifact never performs action-time I/O and does not admit a C++ consumer,
runtime execution, or Product use.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import struct
import subprocess
import sys
from collections import Counter
from pathlib import Path, PurePosixPath
from typing import Any, Mapping


def discover_repository_root() -> Path:
    current = Path.cwd().resolve()
    for candidate in (current, *current.parents):
        if (
            (candidate / ".git").exists()
            and (candidate / "Tools/EffectPipeline").is_dir()
            and (candidate / "Data/Effects/Imported/Artist/Materials").is_dir()
        ):
            return candidate
    raise RuntimeError("cannot locate canonical LostArk repository root")


ROOT = discover_repository_root()
SCRIPT_DIR = ROOT / "Tools/LevelPlacementExtractor"
EFFECT_PIPELINE_DIR = ROOT / "Tools/EffectPipeline"
for search_path in (SCRIPT_DIR, EFFECT_PIPELINE_DIR):
    if str(search_path) not in sys.path:
        sys.path.insert(0, str(search_path))

import artist_31470_material_render_resource_binding_approval as material_approval
import artist_31470_reconstructed_render_resource_authority as authority_module
import build_artist_31470_material_render_resource_binding_approval as approval_builder
import build_artist_31470_reconstructed_runtime_program as program_module
import build_effect_derived_artifact as publisher_module
import effect_source_contract_io as strict_io


SCHEMA = "lostark.artist-31470-reconstructed-render-resource-authority-receipt"
FORMAT_VERSION = 1
AUTHORITY_ID = "ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1"

DEFAULT_PROGRAM = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
DEFAULT_APPROVAL = ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-render-resource-binding-approved-v1.receipt.json"
)
DEFAULT_OUTPUT = ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.reconstructed-render-resource-authority.receipt.json"
)
DEFAULT_TOOL = Path(__file__).resolve()

BASE_INTEGRATION_COMMIT = "ddef21a5314eb8c3db891d36f702cfeda3149f20"
BASE_INTEGRATION_TREE = "36a36b889dae7be092e0d2f6f3c3aee2c28bc462"
CANDIDATE_BUILDER_COMMIT = "ddef21a5314eb8c3db891d36f702cfeda3149f20"
CANDIDATE_BUILDER_TREE = "36a36b889dae7be092e0d2f6f3c3aee2c28bc462"
PARSER_INTEGRATION_COMMIT = "a57f5d27bb1ac29f890e6cb59121c886991f28d5"
PARSER_INTEGRATION_TREE = "bea6e94e0535038bdaabfb53f3f3442ef2fe296c"
APPROVAL_AUTHORITY_COMMIT = "ddef21a5314eb8c3db891d36f702cfeda3149f20"
APPROVAL_AUTHORITY_TREE = "36a36b889dae7be092e0d2f6f3c3aee2c28bc462"

PROGRAM_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
PROGRAM_BLOB_ID = "4b090268b95ea590587276c8048c3b235ee33571"
PROGRAM_BYTE_COUNT = 15_121_873
PROGRAM_RAW_SHA256 = "430ed1aa42a34e23d1f216a69c6f51e81a8cbcdbb03318930894e0dfe16cd6c6"
PROGRAM_SHA256 = "0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802"
PROGRAM_INPUT_ARTIFACT_COUNT = 13
PROGRAM_INPUT_ARTIFACTS_ORDERED_SHA256 = (
    "bcf87806b3635019442f6787c2ca6aed15d7012f2dd4c04d33b448f80814415f"
)

PARSER_FILES = (
    (
        "Client/Public/Effect_RuntimeAuthority.h",
        "f86fcaa1f7554e5b1c274f3eaec3a6a0f8f22d9a",
    ),
    (
        "Client/Private/Effect_RuntimeAuthority.cpp",
        "cf33b11374379e52c0eab9c57d34063b4fe78b93",
    ),
)

APPROVAL_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-render-resource-binding-approved-v1.receipt.json"
)
APPROVAL_BLOB_ID = "8650d36707869ae219217f7f45dd2c9b23910485"
APPROVAL_BYTE_COUNT = 378_236
APPROVAL_RAW_SHA256 = "a73a4e36e5860dc37961a236270c4ca3245025711f05e64a56503cd839b6cd74"
APPROVAL_SELF_SHA256 = "9ca692c688c3987746ab811e4f2504d7186b2905efa7fa8b3446e8c4bf053ac6"
APPROVAL_DECISION_SHA256 = (
    "1b0e8e224b7b1b98b1606f123423ff1bca287271d6f24deda3a16c880c89994d"
)

MATERIAL_TEXTURE_ARTIFACT_ID = "materialTextureBinding"
MATERIAL_TEXTURE_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-texture-runtime-binding.receipt.json"
)
MATERIAL_TEXTURE_AUTHORITY_COMMIT = "bc770990e58386fd58958c759f0e095bc3d237b1"
MATERIAL_TEXTURE_AUTHORITY_TREE = "f5253605b4e06fc5a761685e5cb8ed6eec0cb2e3"
MATERIAL_TEXTURE_BLOB_ID = "f7c75bb92a1a972cca564e4fb021718dc8d320fb"
MATERIAL_TEXTURE_TRACKED_SHA256 = (
    "51d8ba83bde613117dd169bff09b2b12e9593b9258f408c2f72ac88db0279cb6"
)
MATERIAL_TEXTURE_SELF_SHA256 = (
    "07d04d8342ab6d31669d80514cfc7d168e51662819a5903978e2fc3503956aeb"
)

EXPECTED_TEXTURE_BINDING_COUNT = 77
EXPECTED_UNIQUE_RESOURCE_COUNT = 52
EXPECTED_RECIPE_COUNT = 27
EXPECTED_RENDERER_SLOT_COUNT = 57
EXPECTED_AMBIGUOUS_RENDERER_COUNT = 3
EXPECTED_RENDER_STATE_DESCRIPTOR_COUNT = 46

BLOCKERS = [
    "AUTOMATED_WARP_DESCRIPTOR_AND_BINDING_PROBE_REQUIRED",
    "CXX_RENDER_RESOURCE_TRANSACTIONAL_CONSUMER_NOT_IMPLEMENTED",
    "MANUAL_ARTIST_F_EYE_VALIDATION_REQUIRED",
    "PRODUCT_RUNTIME_RENDER_RESOURCE_CONSUMER_NOT_ADMITTED",
    "RECONSTRUCTED_MATERIAL_POLICY_IS_NOT_SOURCE_EVIDENCE",
]

ROOT_KEYS = (
    "schema", "formatVersion", "authorityId", "characterClass", "skillId",
    "inputSlot", "authorityContract", "sourceEvidence", "textureResources",
    "textureBindings", "neutralProviders", "recipeTextureBindings",
    "rendererSlotBindings", "renderStateDescriptors", "blockerProjection",
    "admission", "summary", "decisionProjectionSha256", "receiptSha256",
)
_APPROVED_PROGRAM_CACHE: dict[str, Any] | None = None
_APPROVED_MATERIAL_CACHE: dict[str, Any] | None = None


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


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


def seal_row(row: dict[str, Any]) -> dict[str, Any]:
    require("rowSha256" not in row, "row was already sealed")
    row["rowSha256"] = canonical_sha256(row)
    return row


def strict_ordered_equal(actual: Any, expected: Any, label: str = "root") -> None:
    require(type(actual) is type(expected), f"{label}: strict type changed")
    if isinstance(expected, dict):
        require(tuple(actual.keys()) == tuple(expected.keys()), f"{label}: key/order changed")
        for key in expected:
            strict_ordered_equal(actual[key], expected[key], f"{label}.{key}")
    elif isinstance(expected, list):
        require(len(actual) == len(expected), f"{label}: list length changed")
        for index, (left, right) in enumerate(zip(actual, expected, strict=True)):
            strict_ordered_equal(left, right, f"{label}[{index}]")
    else:
        require(actual == expected, f"{label}: value changed")


def validate_recursive_types(value: Any, label: str = "root") -> None:
    if value is None or isinstance(value, (str, bool)):
        return
    if type(value) is int:
        return
    if type(value) is float:
        require(math.isfinite(value), f"{label}: non-finite float")
        return
    if isinstance(value, list):
        for index, item in enumerate(value):
            validate_recursive_types(item, f"{label}[{index}]")
        return
    if isinstance(value, dict):
        for key, item in value.items():
            require(type(key) is str, f"{label}: non-string object key")
            validate_recursive_types(item, f"{label}.{key}")
        return
    raise ValueError(f"{label}: unsupported recursive type {type(value).__name__}")


def _parse_json_object_bytes(raw: bytes, label: str) -> dict[str, Any]:
    require(not raw.startswith(b"\xef\xbb\xbf"), f"{label}: UTF-8 BOM is forbidden")

    def no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            require(key not in result, f"{label}: duplicate JSON key {key}")
            result[key] = value
        return result

    def no_non_finite(token: str) -> Any:
        raise ValueError(f"{label}: non-finite JSON token {token}")

    try:
        value = json.loads(
            raw.decode("utf-8"),
            object_pairs_hook=no_duplicates,
            parse_constant=no_non_finite,
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"{label}: invalid UTF-8 JSON: {exc}") from exc
    require(type(value) is dict, f"{label}: root must be object")
    return value


def _git_text(*args: str) -> str:
    completed = subprocess.run(
        ["git", *args],
        cwd=ROOT,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    require(completed.returncode == 0,
            f"git {' '.join(args)} failed: {completed.stderr.decode(errors='replace').strip()}")
    return completed.stdout.decode("utf-8").strip()


def _validate_git_identity(commit: str, tree: str) -> None:
    require(_git_text("show", "-s", "--format=%T", commit) == tree,
            f"frozen Git tree changed for {commit}")


def _validate_git_blob(commit: str, path: str, blob_id: str) -> None:
    require(_git_text("rev-parse", f"{commit}:{path}") == blob_id,
            f"frozen Git blob changed: {commit}:{path}")


def discover_canonical_resources_root() -> Path:
    common_dir_text = _git_text("rev-parse", "--git-common-dir")
    common_dir = Path(common_dir_text)
    if not common_dir.is_absolute():
        common_dir = ROOT / common_dir
    common_dir = common_dir.resolve()
    require(common_dir.name == ".git", "Git common directory is not canonical .git")
    resource_root = common_dir.parent / "Client/Bin/Resources"
    require(resource_root.is_dir(), "canonical main Client/Bin/Resources is missing")
    return resource_root.resolve()


def _require_canonical_resources_root(resources_root: Path | None) -> Path:
    canonical = discover_canonical_resources_root()
    selected = canonical if resources_root is None else resources_root.resolve()
    require(selected == canonical,
            "resource authority must read the canonical main Client/Bin/Resources tree")
    return canonical


def _read_program(path: Path = DEFAULT_PROGRAM) -> tuple[bytes, dict[str, Any]]:
    global _APPROVED_PROGRAM_CACHE
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), "program must be UTF-8 without BOM")
    require(b"\r" not in raw, "program must remain canonical LF")
    require(len(raw) == PROGRAM_BYTE_COUNT, "frozen program byte-count mismatch")
    require(hashlib.sha256(raw).hexdigest() == PROGRAM_RAW_SHA256,
            "frozen program raw SHA-256 mismatch")
    use_cache = path.resolve() == DEFAULT_PROGRAM.resolve()
    if not use_cache or _APPROVED_PROGRAM_CACHE is None:
        program = program_module.load_json_bytes(raw, str(path))
        program_module.validate_program(program)
        if use_cache:
            _APPROVED_PROGRAM_CACHE = program
    else:
        program = _APPROVED_PROGRAM_CACHE
    require(program["programSha256"] == PROGRAM_SHA256, "frozen program SHA mismatch")
    require(program["summary"]["inputArtifactCount"] == PROGRAM_INPUT_ARTIFACT_COUNT,
            "frozen program input-artifact count mismatch")
    section = next(
        row for row in program["sectionDigests"] if row["sectionName"] == "inputArtifacts"
    )
    require(section["rowCount"] == PROGRAM_INPUT_ARTIFACT_COUNT,
            "input-artifact section denominator mismatch")
    require(section["orderedSha256"] == PROGRAM_INPUT_ARTIFACTS_ORDERED_SHA256,
            "input-artifact ordered projection mismatch")
    approval_builder.validate_program_authorities(program)
    return raw, program


def _read_approval(
    program: dict[str, Any],
    path: Path = DEFAULT_APPROVAL,
    *,
    require_independent_approval: bool = True,
) -> tuple[bytes, dict[str, Any]]:
    global _APPROVED_MATERIAL_CACHE
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), "Material approval must be UTF-8 without BOM")
    require(b"\r" not in raw, "Material approval must remain canonical LF")
    require(len(raw) == APPROVAL_BYTE_COUNT, "Material approval byte-count mismatch")
    require(hashlib.sha256(raw).hexdigest() == APPROVAL_RAW_SHA256,
            "Material approval raw SHA-256 mismatch")
    use_cache = path.resolve() == DEFAULT_APPROVAL.resolve()
    if not use_cache or _APPROVED_MATERIAL_CACHE is None:
        approval = _parse_json_object_bytes(raw, "Material approval")
        approval_builder.validate_receipt(
            approval,
            program,
            _program_already_validated=True,
            require_approval=require_independent_approval,
        )
        if use_cache:
            _APPROVED_MATERIAL_CACHE = approval
    else:
        approval = _APPROVED_MATERIAL_CACHE
    if require_independent_approval:
        material_approval.require_approved_receipt(approval)
    require(approval["receiptSha256"] == APPROVAL_SELF_SHA256,
            "Material approval self identity mismatch")
    require(material_approval.decision_projection_sha256(approval) == APPROVAL_DECISION_SHA256,
            "Material approval decision identity mismatch")
    return raw, approval


def _build_publisher_base_authority(
    program: dict[str, Any],
) -> dict[str, Any]:
    payload = program_module.output_bytes(program)
    require(len(payload) == PROGRAM_BYTE_COUNT,
            "publisher projection Program byte-count mismatch")
    require(hashlib.sha256(payload).hexdigest() == PROGRAM_RAW_SHA256,
            "publisher projection Program raw SHA mismatch")
    projection = publisher_module.make_reconstructed_base_authority_projection(
        payload
    )
    publisher_module.validate_reconstructed_base_authority_projection(projection)
    require(projection["programSha256"] == PROGRAM_SHA256,
            "publisher projection Program identity mismatch")
    require(projection["sourceExact"] is False and
            projection["runtimeExecutionAdmission"] is False and
            projection["productAdmission"] is False,
            "publisher projection admission changed")
    return projection


def _material_texture_artifact(program: dict[str, Any]) -> dict[str, Any]:
    rows = [
        row for row in program["inputArtifacts"]
        if row["artifactId"] == MATERIAL_TEXTURE_ARTIFACT_ID
    ]
    require(len(rows) == 1, "program Material texture authority denominator changed")
    row = rows[0]
    require(row["path"] == MATERIAL_TEXTURE_PATH, "Material texture authority path changed")
    require(row["authorityCommitId"] == MATERIAL_TEXTURE_AUTHORITY_COMMIT,
            "Material texture authority commit changed")
    require(row["authorityTreeId"] == MATERIAL_TEXTURE_AUTHORITY_TREE,
            "Material texture authority tree changed")
    require(row["blobId"] == MATERIAL_TEXTURE_BLOB_ID,
            "Material texture authority blob changed")
    require(row["trackedTextSha256"] == MATERIAL_TEXTURE_TRACKED_SHA256,
            "Material texture authority tracked identity changed")
    require(row["selfSha256"] == MATERIAL_TEXTURE_SELF_SHA256,
            "Material texture authority self identity changed")
    return row


def _validate_frozen_git_inputs() -> None:
    for commit, tree in (
        (BASE_INTEGRATION_COMMIT, BASE_INTEGRATION_TREE),
        (CANDIDATE_BUILDER_COMMIT, CANDIDATE_BUILDER_TREE),
        (PARSER_INTEGRATION_COMMIT, PARSER_INTEGRATION_TREE),
        (APPROVAL_AUTHORITY_COMMIT, APPROVAL_AUTHORITY_TREE),
        (MATERIAL_TEXTURE_AUTHORITY_COMMIT, MATERIAL_TEXTURE_AUTHORITY_TREE),
    ):
        _validate_git_identity(commit, tree)
    _validate_git_blob(BASE_INTEGRATION_COMMIT, PROGRAM_PATH, PROGRAM_BLOB_ID)
    _validate_git_blob(APPROVAL_AUTHORITY_COMMIT, APPROVAL_PATH, APPROVAL_BLOB_ID)
    _validate_git_blob(
        MATERIAL_TEXTURE_AUTHORITY_COMMIT,
        MATERIAL_TEXTURE_PATH,
        MATERIAL_TEXTURE_BLOB_ID,
    )
    for path, blob_id in PARSER_FILES:
        _validate_git_blob(PARSER_INTEGRATION_COMMIT, path, blob_id)


def _validate_runtime_asset_id(asset_id: str) -> PurePosixPath:
    require(type(asset_id) is str and asset_id != "", "runtimeAssetId must be non-empty string")
    require("\\" not in asset_id, f"runtimeAssetId uses backslash: {asset_id}")
    require(":" not in asset_id, f"runtimeAssetId is drive-qualified: {asset_id}")
    pure = PurePosixPath(asset_id)
    require(not pure.is_absolute(), f"runtimeAssetId is absolute: {asset_id}")
    require(pure.as_posix() == asset_id, f"runtimeAssetId is not normalized: {asset_id}")
    require(all(part not in ("", ".", "..") for part in pure.parts),
            f"runtimeAssetId escapes Resources: {asset_id}")
    require(pure.parts[0] in {"Fonts", "Character", "Deploy", "Effect", "Map", "UI"},
            f"runtimeAssetId top-level directory is not admitted: {asset_id}")
    require(asset_id.endswith(".dds"), f"render texture is not DDS: {asset_id}")
    return pure


def _read_exact_resource(resources_root: Path, asset_id: str) -> bytes:
    pure = _validate_runtime_asset_id(asset_id)
    current = resources_root
    for part in pure.parts:
        require(current.is_dir(), f"resource parent is not directory: {asset_id}")
        names = os.listdir(current)
        require(part in names, f"resource path case or component mismatch: {asset_id}")
        current = current / part
        require(not current.is_symlink(), f"resource path contains symlink: {asset_id}")
    require(current.is_file(), f"resource is missing or not a regular file: {asset_id}")
    resolved = current.resolve()
    try:
        resolved.relative_to(resources_root)
    except ValueError as exc:
        raise ValueError(f"resource escapes canonical root: {asset_id}") from exc
    return current.read_bytes()


def _dds_payload_size(width: int, height: int, mip_levels: int, block_bytes: int) -> int:
    total = 0
    for level in range(mip_levels):
        mip_width = max(1, width >> level)
        mip_height = max(1, height >> level)
        total += max(1, (mip_width + 3) // 4) * max(1, (mip_height + 3) // 4) * block_bytes
    return total


def parse_dds(raw: bytes, asset_id: str) -> dict[str, Any]:
    require(len(raw) >= 128, f"DDS is truncated: {asset_id}")
    require(raw[:4] == b"DDS ", f"DDS magic mismatch: {asset_id}")
    values = struct.unpack_from("<31I", raw, 4)
    header_size, flags, height, width, linear_size, depth, raw_mips = values[:7]
    reserved1 = list(values[7:18])
    pf_size, pf_flags, fourcc_u32, rgb_bits = values[18:22]
    masks = list(values[22:26])
    caps, caps2, caps3, caps4, reserved2 = values[26:31]
    fourcc_bytes = struct.pack("<I", fourcc_u32)
    try:
        fourcc = fourcc_bytes.decode("ascii")
    except UnicodeDecodeError as exc:
        raise ValueError(f"DDS FourCC is not ASCII: {asset_id}") from exc

    require(header_size == 124, f"DDS header size mismatch: {asset_id}")
    require(pf_size == 32, f"DDS pixel-format size mismatch: {asset_id}")
    require((pf_flags & 0x4) != 0, f"DDS pixel format is not FourCC: {asset_id}")
    require(fourcc != "DX10", f"DDS DX10 extension is outside frozen contract: {asset_id}")
    require(fourcc in {"DXT1", "DXT5", "ATI2"},
            f"DDS compression is unsupported: {asset_id}:{fourcc}")
    require(width > 0 and height > 0, f"DDS dimensions must be positive: {asset_id}")
    require(depth in (0, 1), f"DDS is not a 2D texture: {asset_id}")
    require(caps2 == 0 and caps3 == 0 and caps4 == 0,
            f"DDS cube/volume/extended caps are outside frozen contract: {asset_id}")

    effective_mips = raw_mips if raw_mips > 0 else 1
    compression = {
        "DXT1": ("BC1", 8, 71, "DXGI_FORMAT_BC1_UNORM", 72, "DXGI_FORMAT_BC1_UNORM_SRGB"),
        "DXT5": ("BC3", 16, 77, "DXGI_FORMAT_BC3_UNORM", 78, "DXGI_FORMAT_BC3_UNORM_SRGB"),
        "ATI2": ("BC5", 16, 83, "DXGI_FORMAT_BC5_UNORM", None, ""),
    }[fourcc]
    family, block_bytes, linear_dxgi, linear_name, srgb_dxgi, srgb_name = compression
    expected_payload = _dds_payload_size(width, height, effective_mips, block_bytes)
    actual_payload = len(raw) - 128
    require(actual_payload == expected_payload,
            f"DDS compressed payload size mismatch: {asset_id}")
    require(linear_size == _dds_payload_size(width, height, 1, block_bytes),
            f"DDS top-level linear size mismatch: {asset_id}")

    return {
        "magic": "DDS ",
        "headerSize": header_size,
        "flags": flags,
        "height": height,
        "width": width,
        "pitchOrLinearSize": linear_size,
        "depth": depth,
        "rawMipMapCount": raw_mips,
        "effectiveMipLevelCount": effective_mips,
        "reserved1": reserved1,
        "pixelFormat": {
            "size": pf_size,
            "flags": pf_flags,
            "fourCC": fourcc,
            "rgbBitCount": rgb_bits,
            "rBitMask": masks[0],
            "gBitMask": masks[1],
            "bBitMask": masks[2],
            "aBitMask": masks[3],
        },
        "caps": caps,
        "caps2": caps2,
        "caps3": caps3,
        "caps4": caps4,
        "reserved2": reserved2,
        "dataOffset": 128,
        "payloadByteCount": actual_payload,
        "expectedCompressedPayloadByteCount": expected_payload,
        "payloadByteCountExact": True,
        "compression": {
            "family": family,
            "bytesPerFourByFourBlock": block_bytes,
            "linearDxgiFormat": linear_dxgi,
            "linearDxgiFormatName": linear_name,
            "srgbDxgiFormat": srgb_dxgi,
            "srgbDxgiFormatName": srgb_name,
        },
    }


def _color_policy(policy_row: dict[str, Any]) -> str:
    sampler = policy_row["samplerDescriptor"]
    require(type(sampler) is dict, "texture policy lacks sampler descriptor")
    color = sampler["srvColorSpace"]
    require(color in {"SRGB", "LINEAR"}, "sampler color-space domain changed")
    require(type(sampler["sRgb"]) is bool, "sampler sRgb must be strict bool")
    require(sampler["sRgb"] == (color == "SRGB"), "sampler sRgb/color-space mismatch")
    return color


def _actual_srv(dds: dict[str, Any], color: str, asset_id: str) -> dict[str, Any]:
    compression = dds["compression"]
    if color == "SRGB":
        require(compression["srgbDxgiFormat"] is not None,
                f"DDS format has no sRGB SRV variant: {asset_id}")
        fmt = compression["srgbDxgiFormat"]
        name = compression["srgbDxgiFormatName"]
    else:
        fmt = compression["linearDxgiFormat"]
        name = compression["linearDxgiFormatName"]
    return {
        "Format": fmt,
        "FormatName": name,
        "ViewDimension": 4,
        "ViewDimensionName": "D3D11_SRV_DIMENSION_TEXTURE2D",
        "MostDetailedMip": 0,
        "MipLevels": dds["effectiveMipLevelCount"],
        "srvColorSpace": color,
    }


def _validate_prior_fixture(policy: dict[str, Any], color: str) -> dict[str, Any]:
    oracle = policy["d3dSrvOracle"]
    require(type(oracle) is dict, "sampler policy lacks prior SRV oracle")
    expected = oracle["expectedSrv"]
    actual = oracle["actualSrv"]
    strict_ordered_equal(actual, expected, "priorPolicyFixture.actualSrv")
    require(expected["Format"] == (29 if color == "SRGB" else 28),
            "prior policy fixture is not the RGBA8 color-space oracle")
    require(expected["ViewDimension"] == 4 and expected["MostDetailedMip"] == 0,
            "prior policy fixture view changed")
    require(expected["MipLevels"] == 1 and expected["srvColorSpace"] == color,
            "prior policy fixture mip/color changed")
    require(oracle["decision"] == "PASS" and oracle["numericTolerance"] == 0.0,
            "prior policy fixture did not pass exactly")
    return {
        "fixtureKind": "MATERIAL_POLICY_1X1_RGBA8_SRV_ORACLE_NOT_ACTUAL_DDS_DESCRIPTOR",
        "policyRowId": oracle["policyRowId"],
        "expectedSrv": copy.deepcopy(expected),
        "actualSrv": copy.deepcopy(actual),
        "numericTolerance": oracle["numericTolerance"],
        "decision": oracle["decision"],
    }


def build_texture_authority(
    program: dict[str, Any], resources_root: Path
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], dict[str, bytes]]:
    bindings = program["materialTextureBindings"]
    require(len(bindings) == EXPECTED_TEXTURE_BINDING_COUNT,
            "Material texture binding denominator changed")
    policy_rows = {row["policyRowId"]: row for row in program["materialPolicyRows"]}
    recipe_rows = {row["recipeId"]: row for row in program["materialRecipes"]}

    grouped: dict[str, list[dict[str, Any]]] = {}
    colors: dict[str, set[str]] = {}
    for expected_order, binding in enumerate(bindings):
        require(binding["order"] == expected_order, "Material texture binding order changed")
        require(binding["resolutionStatus"] == "RESOLVED_EXACT_RUNTIME_ASSET",
                "Material texture binding is unresolved")
        require(binding["recipeId"] in recipe_rows, "texture binding recipe owner is missing")
        policy = policy_rows.get(binding["samplerPolicyRowId"])
        require(policy is not None, "texture binding sampler owner is missing")
        require(policy["recipeId"] == binding["recipeId"],
                "texture binding sampler recipe owner changed")
        color = _color_policy(policy)
        asset_id = binding["runtimeAssetId"]
        _validate_runtime_asset_id(asset_id)
        grouped.setdefault(asset_id, []).append(binding)
        colors.setdefault(asset_id, set()).add(color)

    require(len(grouped) == EXPECTED_UNIQUE_RESOURCE_COUNT,
            "unique actual DDS denominator changed")
    require(all(len(value) == 1 for value in colors.values()),
            "one DDS is requested with incompatible color policies")

    resource_rows: list[dict[str, Any]] = []
    resource_payloads: dict[str, bytes] = {}
    for resource_order, (asset_id, owners) in enumerate(grouped.items()):
        raw = _read_exact_resource(resources_root, asset_id)
        resource_payloads[asset_id] = raw
        dds = parse_dds(raw, asset_id)
        color = next(iter(colors[asset_id]))
        actual_srv = _actual_srv(dds, color, asset_id)
        classification = f"{dds['compression']['family']}_{color}"
        resource_rows.append(seal_row({
            "resourceAuthorityId": f"render-resource-{canonical_sha256(asset_id)[:20]}",
            "order": resource_order,
            "runtimeAssetId": asset_id,
            "candidateBindingIds": [row["bindingId"] for row in owners],
            "candidateBindingRowSha256": [row["rowSha256"] for row in owners],
            "candidateBindingCount": len(owners),
            "byteCount": len(raw),
            "rawSha256": hashlib.sha256(raw).hexdigest(),
            "ddsHeader": dds,
            "actualCompressedFormatClassification": classification,
            "colorSpacePolicy": color,
            "actualExpectedSrvDescriptor": actual_srv,
            "resourceIdentityBasis": "CANONICAL_MAIN_RESOURCES_BYTE_EXACT_DDS",
            "absolutePathRecorded": False,
            "actionTimeIoAllowed": False,
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "product": False,
        }))

    resources_by_asset = {row["runtimeAssetId"]: row for row in resource_rows}
    binding_rows: list[dict[str, Any]] = []
    for binding in bindings:
        policy = policy_rows[binding["samplerPolicyRowId"]]
        color = _color_policy(policy)
        resource = resources_by_asset[binding["runtimeAssetId"]]
        actual_srv = _actual_srv(resource["ddsHeader"], color, binding["runtimeAssetId"])
        strict_ordered_equal(
            actual_srv,
            resource["actualExpectedSrvDescriptor"],
            "binding.actualExpectedSrvDescriptor",
        )
        binding_rows.append(seal_row({
            "bindingAuthorityId": f"render-binding-{binding['order']:02d}",
            "order": binding["order"],
            "candidateBindingId": binding["bindingId"],
            "candidateBindingRowSha256": binding["rowSha256"],
            "recipeId": binding["recipeId"],
            "materialInputFieldId": binding["materialInputFieldId"],
            "samplerPolicyRowId": binding["samplerPolicyRowId"],
            "samplerPolicyRowSha256": policy["rowSha256"],
            "materialOccurrenceIds": copy.deepcopy(binding["materialOccurrenceIds"]),
            "sourceBindingId": binding["sourceBindingId"],
            "sourceBindingRowSha256": binding["sourceBindingRowSha256"],
            "sourceTextureResourceId": binding["sourceTextureResourceId"],
            "sourceTextureResourceRowSha256": binding["sourceTextureResourceRowSha256"],
            "sourceReceiptStatus": binding["sourceReceiptStatus"],
            "runtimeAssetId": binding["runtimeAssetId"],
            "resourceAuthorityId": resource["resourceAuthorityId"],
            "resourceAuthorityRowSha256": resource["rowSha256"],
            "samplerDescriptor": copy.deepcopy(policy["samplerDescriptor"]),
            "colorSpacePolicy": color,
            "priorPolicySrvFixture": _validate_prior_fixture(policy, color),
            "actualDdsSrvDescriptor": actual_srv,
            "actualDdsByteCount": resource["byteCount"],
            "actualDdsRawSha256": resource["rawSha256"],
            "bindingIdentityBasis": (
                "FROZEN_MATERIAL_BINDING_ROW_AND_CANONICAL_MAIN_RESOURCES_BYTES"
            ),
            "actionTimeIoAllowed": False,
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "product": False,
        }))
    return resource_rows, binding_rows, resource_payloads


def _source_evidence(
    program: dict[str, Any],
    approval: dict[str, Any],
    publisher_projection: dict[str, Any],
) -> dict[str, Any]:
    material_texture = _material_texture_artifact(program)
    parser_files = []
    for path, blob_id in PARSER_FILES:
        parser_files.append({
            "path": path,
            "parserIntegrationBlobId": blob_id,
            "currentTrackedTextSha256": strict_io.tracked_text_sha256(ROOT / path),
        })
    publisher_module.validate_reconstructed_base_authority_projection(
        publisher_projection
    )
    publisher_tool_path = (
        "Tools/EffectPipeline/build_effect_derived_artifact.py"
    )
    return {
        "programAndParserTuple": {
            "path": PROGRAM_PATH,
            "candidateBuilderCommitId": CANDIDATE_BUILDER_COMMIT,
            "candidateBuilderTreeId": CANDIDATE_BUILDER_TREE,
            "candidateBlobIdAtIntegration": PROGRAM_BLOB_ID,
            "rawByteCount": PROGRAM_BYTE_COUNT,
            "rawSha256": PROGRAM_RAW_SHA256,
            "programId": program["programId"],
            "programVersion": program["programVersion"],
            "programSha256": PROGRAM_SHA256,
            "inputArtifactCount": PROGRAM_INPUT_ARTIFACT_COUNT,
            "inputArtifactsOrderedSha256": PROGRAM_INPUT_ARTIFACTS_ORDERED_SHA256,
            "parserIntegrationCommitId": PARSER_INTEGRATION_COMMIT,
            "parserIntegrationTreeId": PARSER_INTEGRATION_TREE,
            "parserFiles": parser_files,
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "materialTextureBindingAuthority": {
            "programInputArtifact": copy.deepcopy(material_texture),
            "path": MATERIAL_TEXTURE_PATH,
            "authorityCommitId": MATERIAL_TEXTURE_AUTHORITY_COMMIT,
            "authorityTreeId": MATERIAL_TEXTURE_AUTHORITY_TREE,
            "blobId": MATERIAL_TEXTURE_BLOB_ID,
            "trackedTextSha256": MATERIAL_TEXTURE_TRACKED_SHA256,
            "receiptSha256": MATERIAL_TEXTURE_SELF_SHA256,
            "orderedBindingCount": len(program["materialTextureBindings"]),
            "uniqueRuntimeAssetCount": len({
                row["runtimeAssetId"] for row in program["materialTextureBindings"]
            }),
            "sourceExact": False,
        },
        "materialRenderResourceApproval": {
            "path": APPROVAL_PATH,
            "authorityCommitId": APPROVAL_AUTHORITY_COMMIT,
            "authorityTreeId": APPROVAL_AUTHORITY_TREE,
            "integrationCommitId": BASE_INTEGRATION_COMMIT,
            "integrationTreeId": BASE_INTEGRATION_TREE,
            "blobId": APPROVAL_BLOB_ID,
            "rawByteCount": APPROVAL_BYTE_COUNT,
            "rawSha256": APPROVAL_RAW_SHA256,
            "receiptSha256": APPROVAL_SELF_SHA256,
            "decisionProjectionSha256": APPROVAL_DECISION_SHA256,
            "approvalId": approval["approvalId"],
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "publisherRuntimeCatalogAuthority": {
            "authorityScope": (
                "BASE_RUNTIME_ENTRY_PROJECTION_BEFORE_RENDER_RESOURCE_SIDECAR"
            ),
            "runtimeCatalogBytesRead": False,
            "completedRuntimeEntryRead": False,
            "renderResourceSidecarRead": False,
            "selfReferenceExcluded": True,
            "projectionKeyCount": len(publisher_projection),
            "projectionKeyOrder": list(publisher_projection.keys()),
            "projectionCanonicalSha256": canonical_sha256(
                publisher_projection
            ),
            "baseProjection": copy.deepcopy(publisher_projection),
            "publicValidator": {
                "path": publisher_tool_path,
                "builderFunction": (
                    "make_reconstructed_base_authority_projection"
                ),
                "validatorFunction": (
                    "validate_reconstructed_base_authority_projection"
                ),
            },
        },
        "generatorAndValidator": {
            "path": (
                "Tools/LevelPlacementExtractor/"
                "build_artist_31470_reconstructed_render_resource_authority.py"
            ),
            "trackedTextSha256": strict_io.tracked_text_sha256(DEFAULT_TOOL),
            "role": "OFFLINE_DETERMINISTIC_BUILDER_AND_STRICT_CURRENT_BYTES_VALIDATOR",
        },
        "canonicalResourceRootContract": {
            "rootId": "GIT_COMMON_WORKTREE/Client/Bin/Resources",
            "absolutePathRecorded": False,
            "readAtOfflineBuildAndValidation": True,
            "actionTimeIoAllowed": False,
        },
        "sourceExact": False,
    }


def _summary(
    resources: list[dict[str, Any]],
    bindings: list[dict[str, Any]],
    approval: dict[str, Any],
) -> dict[str, Any]:
    resource_formats = Counter(
        row["actualCompressedFormatClassification"] for row in resources
    )
    binding_formats = Counter(
        (row["actualDdsSrvDescriptor"]["Format"], row["actualDdsSrvDescriptor"]["FormatName"])
        for row in bindings
    )
    binding_colors = Counter(row["colorSpacePolicy"] for row in bindings)
    return {
        "textureResourceCount": len(resources),
        "textureBindingCount": len(bindings),
        "resourceFormatCounts": [
            {"classification": name, "count": count}
            for name, count in sorted(resource_formats.items())
        ],
        "bindingSrvDxgiFormatCounts": [
            {"dxgiFormat": fmt, "dxgiFormatName": name, "count": count}
            for (fmt, name), count in sorted(binding_formats.items())
        ],
        "bindingColorSpaceCounts": [
            {"colorSpacePolicy": name, "count": count}
            for name, count in sorted(binding_colors.items())
        ],
        "neutralProviderCount": len(approval["neutralProviders"]),
        "recipeTextureBindingCount": len(approval["recipeTextureBindings"]),
        "rendererSlotBindingCount": len(approval["rendererSlotBindings"]),
        "ambiguousRendererDecisionCount": sum(
            row["candidateCount"] == 2 for row in approval["rendererSlotBindings"]
        ),
        "renderStateDescriptorCount": len(approval["renderStateDescriptors"]),
        "blendDescriptorCount": sum(
            row["descriptorKind"] == "D3D11_BLEND_DESC"
            for row in approval["renderStateDescriptors"]
        ),
        "twoSidedRasterDescriptorCount": sum(
            row["descriptorKind"] == "D3D11_RASTERIZER_DESC"
            for row in approval["renderStateDescriptors"]
        ),
        "disableDepthDescriptorCount": sum(
            row["descriptorKind"] == "D3D11_DEPTH_STENCIL_DESC"
            for row in approval["renderStateDescriptors"]
        ),
        "actionTimeIoAllowed": False,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "product": False,
    }


def build_receipt(
    program: dict[str, Any],
    approval: dict[str, Any],
    *,
    resources_root: Path | None = None,
    require_independent_approval: bool = True,
    _inputs_already_validated: bool = False,
) -> tuple[dict[str, Any], dict[str, bytes]]:
    _validate_frozen_git_inputs()
    approved_root = _require_canonical_resources_root(resources_root)
    if not _inputs_already_validated:
        program_module.validate_program(program)
        approval_builder.validate_receipt(
            approval,
            program,
            require_approval=require_independent_approval,
        )
    require(program["programSha256"] == PROGRAM_SHA256, "program identity changed")
    require(approval["receiptSha256"] == APPROVAL_SELF_SHA256, "approval identity changed")
    _material_texture_artifact(program)
    publisher_projection = _build_publisher_base_authority(program)

    resources, bindings, payloads = build_texture_authority(program, approved_root)
    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "authorityId": AUTHORITY_ID,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "authorityContract": {
            "authorityKind": "IMMUTABLE_RECONSTRUCTED_RENDER_RESOURCE_SIDECAR",
            "resourceIdentityDomain": "CANONICAL_MAIN_RESOURCES_RELATIVE_DDS_BYTES",
            "materialDecisionDomain": "INDEPENDENT_RECONSTRUCTED_POLICY_APPROVAL",
            "priorPolicySrvFixtureIsActualDdsDescriptor": False,
            "runtimeNameOrRoleHeuristicsAllowed": False,
            "absoluteResourcePathsAllowedInReceipt": False,
            "actionTimeIoAllowed": False,
            "transactionPolicy": "PARSE_VALIDATE_STAGE_COMMIT_OR_ROLLBACK",
            "partialCommitAllowed": False,
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "sourceEvidence": _source_evidence(
            program,
            approval,
            publisher_projection,
        ),
        "textureResources": resources,
        "textureBindings": bindings,
        "neutralProviders": copy.deepcopy(approval["neutralProviders"]),
        "recipeTextureBindings": copy.deepcopy(approval["recipeTextureBindings"]),
        "rendererSlotBindings": copy.deepcopy(approval["rendererSlotBindings"]),
        "renderStateDescriptors": copy.deepcopy(approval["renderStateDescriptors"]),
        "blockerProjection": {
            "blockers": BLOCKERS,
            "canonicalResourceBytesVerifiedAtOfflineBuildAndValidation": True,
            "actionTimeIoAllowed": False,
            "bindingFailureBehavior": "ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET",
            "partialCommitAllowed": False,
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "admission": {
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "summary": _summary(resources, bindings, approval),
    }
    receipt["decisionProjectionSha256"] = authority_module.decision_projection_sha256(receipt)
    receipt["receiptSha256"] = canonical_sha256(receipt)
    validate_recursive_types(receipt)
    return receipt, payloads


def _validate_sealed_rows(receipt: dict[str, Any]) -> None:
    for section in (
        "textureResources", "textureBindings", "neutralProviders",
        "recipeTextureBindings", "rendererSlotBindings", "renderStateDescriptors",
    ):
        for index, row in enumerate(receipt[section]):
            require(tuple(row.keys())[-1] == "rowSha256",
                    f"{section}[{index}]: rowSha256 must be last")
            unsigned = copy.deepcopy(row)
            digest = unsigned.pop("rowSha256")
            require(type(digest) is str and canonical_sha256(unsigned) == digest,
                    f"{section}[{index}]: row seal mismatch")


def _validate_supplied_resource_payloads(
    supplied: Mapping[str, bytes], actual: Mapping[str, bytes]
) -> None:
    require(tuple(supplied.keys()) == tuple(actual.keys()),
            "supplied external DDS object key/order changed")
    for asset_id in actual:
        require(type(supplied[asset_id]) is bytes,
                f"supplied external DDS is not bytes: {asset_id}")
        require(supplied[asset_id] == actual[asset_id],
                f"supplied external DDS bytes differ from canonical Resources: {asset_id}")


def validate_receipt(
    receipt: dict[str, Any],
    program: dict[str, Any] | None = None,
    approval: dict[str, Any] | None = None,
    *,
    resources_root: Path | None = None,
    program_path: Path = DEFAULT_PROGRAM,
    approval_path: Path = DEFAULT_APPROVAL,
    supplied_resource_payloads: Mapping[str, bytes] | None = None,
    supplied_approval_bytes: bytes | None = None,
    supplied_tool_bytes: bytes | None = None,
    require_independent_approval: bool = True,
) -> None:
    validate_recursive_types(receipt)
    require(tuple(receipt.keys()) == ROOT_KEYS, "receipt root key/order changed")
    require(receipt["schema"] == SCHEMA and receipt["formatVersion"] == FORMAT_VERSION,
            "receipt schema/version changed")
    require(tuple(receipt.keys())[-1] == "receiptSha256", "receiptSha256 must be last")
    unsigned = copy.deepcopy(receipt)
    receipt_sha = unsigned.pop("receiptSha256")
    require(type(receipt_sha) is str and canonical_sha256(unsigned) == receipt_sha,
            "receipt self seal mismatch")
    require(receipt["decisionProjectionSha256"] ==
            authority_module.decision_projection_sha256(receipt),
            "decision projection self field mismatch")
    _validate_sealed_rows(receipt)

    _, approved_program = _read_program(program_path)
    _, approved_material = _read_approval(
        approved_program,
        approval_path,
        require_independent_approval=require_independent_approval,
    )
    if program is not None:
        program_module.validate_program(program)
        strict_ordered_equal(program, approved_program, "suppliedProgram")
    if approval is not None:
        approval_builder.validate_receipt(
            approval,
            approved_program,
            _program_already_validated=True,
            require_approval=require_independent_approval,
        )
        strict_ordered_equal(approval, approved_material, "suppliedApproval")
    if supplied_approval_bytes is not None:
        supplied_approval = _parse_json_object_bytes(
            supplied_approval_bytes, "supplied Material approval"
        )
        approval_builder.validate_receipt(
            supplied_approval,
            approved_program,
            _program_already_validated=True,
            require_approval=require_independent_approval,
        )
        strict_ordered_equal(supplied_approval, approved_material, "suppliedApprovalBytes")
    if supplied_tool_bytes is not None:
        require(
            hashlib.sha256(strict_io.normalize_utf8_eol(supplied_tool_bytes)).hexdigest()
            == strict_io.tracked_text_sha256(DEFAULT_TOOL),
            "supplied current-tool object differs from actual validator",
        )

    expected, actual_payloads = build_receipt(
        approved_program,
        approved_material,
        resources_root=resources_root,
        require_independent_approval=require_independent_approval,
        _inputs_already_validated=True,
    )
    if supplied_resource_payloads is not None:
        _validate_supplied_resource_payloads(supplied_resource_payloads, actual_payloads)
    strict_ordered_equal(receipt, expected)
    if require_independent_approval:
        authority_module.require_approved_receipt(receipt)


def serialized_receipt(receipt: dict[str, Any]) -> bytes:
    return (
        json.dumps(receipt, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--program", type=Path, default=DEFAULT_PROGRAM)
    parser.add_argument("--approval", type=Path, default=DEFAULT_APPROVAL)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument(
        "--allow-unapproved",
        action="store_true",
        help="bootstrap only: build/check before independent pins are frozen",
    )
    args = parser.parse_args()

    raw, program = _read_program(args.program)
    require(hashlib.sha256(raw).hexdigest() == PROGRAM_RAW_SHA256,
            "command-line program is not the frozen candidate")
    approval_raw, approval = _read_approval(
        program,
        args.approval,
        require_independent_approval=not args.allow_unapproved,
    )
    require(hashlib.sha256(approval_raw).hexdigest() == APPROVAL_RAW_SHA256,
            "command-line approval is not frozen")
    receipt, _ = build_receipt(
        program,
        approval,
        require_independent_approval=not args.allow_unapproved,
        _inputs_already_validated=True,
    )
    validate_receipt(
        receipt,
        program,
        approval,
        program_path=args.program,
        approval_path=args.approval,
        require_independent_approval=not args.allow_unapproved,
    )
    expected = serialized_receipt(receipt)

    if args.check:
        require(args.output.is_file(), f"generated receipt is missing: {args.output}")
        require(args.output.read_bytes() == expected,
                "generated receipt is stale or not canonical LF UTF-8")
        summary = receipt["summary"]
        print(
            "PASS Artist 31470 reconstructed render-resource authority "
            f"resources={summary['textureResourceCount']} "
            f"bindings={summary['textureBindingCount']} "
            f"recipes={summary['recipeTextureBindingCount']} "
            f"renderer={summary['rendererSlotBindingCount']} "
            f"receipt={receipt['receiptSha256']} "
            "runtime=false product=false"
        )
        return 0

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(expected)
    print(f"WROTE {args.output}")
    print(f"byteCount={len(expected)}")
    print(f"rawSha256={hashlib.sha256(expected).hexdigest()}")
    print(f"receiptSha256={receipt['receiptSha256']}")
    print(f"decisionProjectionSha256={receipt['decisionProjectionSha256']}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, TypeError, StopIteration) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
