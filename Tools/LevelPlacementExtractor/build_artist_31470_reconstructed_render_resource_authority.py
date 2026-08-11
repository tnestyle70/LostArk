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
DEFAULT_RUNTIME_CATALOG = ROOT / "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"

BASE_INTEGRATION_COMMIT = "846394cb39306c3d5f781c3eae83adb59324ace3"
BASE_INTEGRATION_TREE = "68be53aa25a3c7ed697e5c13485dba68ea79ebe6"
CANDIDATE_BUILDER_COMMIT = "a85b8b41afb2f2a51bceafa55d06bf0937b1a245"
CANDIDATE_BUILDER_TREE = "384ed35ca808ab9a71a4edb703ca4d9121b48c18"
PARSER_INTEGRATION_COMMIT = "eacb58bda2315e858c562677bbf38c17d5d3e785"
PARSER_INTEGRATION_TREE = "8a2828fe2b3deb9c1270143b78a6edd6211d4801"
APPROVAL_AUTHORITY_COMMIT = "d053522f6c993730d1ee7a8eb156861f63a02b6d"
APPROVAL_AUTHORITY_TREE = "68be53aa25a3c7ed697e5c13485dba68ea79ebe6"
PUBLISHER_INTEGRATION_COMMIT = "932d648f95bc7f2d9c7209fad55aee8a857c94d7"
PUBLISHER_ORIGINAL_COMMIT = "74c692755791e592d6f808f3c50b3321c60af181"
PUBLISHER_TREE = "f4da1e28a13875ae162eec4aa273c75647caa3a9"

PROGRAM_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
PROGRAM_BLOB_ID = "345ab15bbb76648a650eaa854f18c4cd63cb1556"
PROGRAM_BYTE_COUNT = 15_072_141
PROGRAM_RAW_SHA256 = "72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849"
PROGRAM_SHA256 = "618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b"
PROGRAM_INPUT_ARTIFACT_COUNT = 13
PROGRAM_INPUT_ARTIFACTS_ORDERED_SHA256 = (
    "938dbd9573ca3a5784675ba9d412b9dc3c12a7431a06c70e37d8c9bf2e614eaa"
)

PARSER_FILES = (
    (
        "Client/Public/Effect_RuntimeAuthority.h",
        "ad35f6921d290492a87ac7303dc599c9a341e4ee",
    ),
    (
        "Client/Private/Effect_RuntimeAuthority.cpp",
        "5aea53f02da79e2a48eefd9650b6c941f0c349fa",
    ),
)

APPROVAL_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-render-resource-binding-approved-v1.receipt.json"
)
APPROVAL_BLOB_ID = "7e17f42d15f77a24e9149e150e277cbdff2dc900"
APPROVAL_BYTE_COUNT = 376_183
APPROVAL_RAW_SHA256 = "68ae71bd70260270404d4a7b6c296e41f74d0031d27899b56a4376c1b11f4931"
APPROVAL_SELF_SHA256 = "d643c9bf1bc2f10a887c805534b28e4322646cea426656de61b894e5b6284644"
APPROVAL_DECISION_SHA256 = (
    "4731ed9c2882c948373ec54f56087803145447851f3fc793fb8e9fa9d96cc957"
)

MATERIAL_TEXTURE_ARTIFACT_ID = "materialTextureBinding"
MATERIAL_TEXTURE_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-texture-runtime-binding.receipt.json"
)
MATERIAL_TEXTURE_AUTHORITY_COMMIT = "1a0b1a6834d562dac02db4f57dda54644d75695b"
MATERIAL_TEXTURE_AUTHORITY_TREE = "84cc7ef8cde7a7cf5194b0ed2ccf56a45a927b57"
MATERIAL_TEXTURE_BLOB_ID = "1a917e44d5605e322e2c554db21573d856b05874"
MATERIAL_TEXTURE_TRACKED_SHA256 = (
    "87a28be564308117ac666206382c94ce5ee2bf37a47111cbef717994a0266077"
)
MATERIAL_TEXTURE_SELF_SHA256 = (
    "3e722cf02085497c63083fbf51161ff5fd6670be91607737863b9c4019e55b48"
)

RUNTIME_CATALOG_PATH = "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
RUNTIME_CATALOG_BLOB_ID = "ca360e952dd110f0246a5e0f1374baf77b7ebc0c"
RUNTIME_CATALOG_BYTE_COUNT = 26_255_931
RUNTIME_CATALOG_RAW_SHA256 = (
    "bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29"
)
RUNTIME_CATALOG_COMPONENT_COUNT = 555
RUNTIME_CATALOG_EFFECT_COUNT = 102
RUNTIME_ENTRY_EFFECT_INDEX = 0
RUNTIME_ENTRY_CANONICAL_SHA256 = (
    "e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2"
)
RUNTIME_LINK_SHA256 = "74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2"
RUNTIME_RECEIPT_SELF_SHA256 = (
    "5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3"
)
RUNTIME_OUTER_RECEIPT_SHA256 = (
    "92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94"
)
PUBLISHER_TOOL_BLOBS = (
    (
        "Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py",
        "130260991146c92988bd916f41d183463883e056",
    ),
    (
        "Tools/EffectPipeline/build_effect_derived_artifact.py",
        "35231dd7621ba8809e1745c9532e6e63dbd09696",
    ),
    (
        "Tools/EffectPipeline/Publish-Effects.ps1",
        "db261bdf6dfd88988da79ad7d2324ab6cc980f09",
    ),
)

EXPECTED_TEXTURE_BINDING_COUNT = 72
EXPECTED_UNIQUE_RESOURCE_COUNT = 48
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
_PUBLISHER_CATALOG_CACHE: dict[str, Any] | None = None
_PUBLISHER_EXPECTED_ENTRY_CACHE: dict[str, Any] | None = None


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
    if _APPROVED_PROGRAM_CACHE is None:
        program = program_module.load_json_bytes(raw, str(path))
        program_module.validate_program(program)
        _APPROVED_PROGRAM_CACHE = program
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
    program: dict[str, Any], path: Path = DEFAULT_APPROVAL
) -> tuple[bytes, dict[str, Any]]:
    global _APPROVED_MATERIAL_CACHE
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), "Material approval must be UTF-8 without BOM")
    require(b"\r" not in raw, "Material approval must remain canonical LF")
    require(len(raw) == APPROVAL_BYTE_COUNT, "Material approval byte-count mismatch")
    require(hashlib.sha256(raw).hexdigest() == APPROVAL_RAW_SHA256,
            "Material approval raw SHA-256 mismatch")
    if _APPROVED_MATERIAL_CACHE is None:
        approval = _parse_json_object_bytes(raw, "Material approval")
        approval_builder.validate_receipt(approval, program)
        _APPROVED_MATERIAL_CACHE = approval
    approval = _APPROVED_MATERIAL_CACHE
    require(approval["receiptSha256"] == APPROVAL_SELF_SHA256,
            "Material approval self identity mismatch")
    require(material_approval.decision_projection_sha256(approval) == APPROVAL_DECISION_SHA256,
            "Material approval decision identity mismatch")
    return raw, approval


def _extract_reconstructed_runtime_entry(
    catalog: dict[str, Any], label: str
) -> tuple[int, dict[str, Any]]:
    require(
        tuple(catalog.keys()) == ("schema", "formatVersion", "components", "effects"),
        f"{label}: runtime catalog root key/order changed",
    )
    publisher_module.validate_runtime_catalog(catalog)
    matches = [
        (index, row)
        for index, row in enumerate(catalog["effects"])
        if row.get("effectAssetId") == publisher_module.RECONSTRUCTED_EFFECT_ID
    ]
    require(len(matches) == 1, f"{label}: Artist 31470 publisher entry denominator changed")
    index, entry = matches[0]
    publisher_module.validate_reconstructed_runtime_entry(entry)
    return index, entry


def _read_publisher_runtime_authority(
    path: Path = DEFAULT_RUNTIME_CATALOG,
) -> tuple[bytes, dict[str, Any], dict[str, Any], dict[str, Any]]:
    global _PUBLISHER_CATALOG_CACHE, _PUBLISHER_EXPECTED_ENTRY_CACHE
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), "runtime catalog must be UTF-8 without BOM")
    require(len(raw) == RUNTIME_CATALOG_BYTE_COUNT, "runtime catalog byte-count mismatch")
    require(hashlib.sha256(raw).hexdigest() == RUNTIME_CATALOG_RAW_SHA256,
            "runtime catalog raw SHA-256 mismatch")

    # The exact raw identity is re-read on every validation.  The cold path must
    # parse these same bytes rather than reopening the path after the hash check;
    # otherwise a split read could validate a different catalog object.  Cache
    # only that exact parsed object, then invoke the public entry validator on
    # every call so all three current tool dependencies are always re-read.
    use_cache = path.resolve() == DEFAULT_RUNTIME_CATALOG.resolve()
    if not use_cache or _PUBLISHER_CATALOG_CACHE is None:
        catalog = _parse_json_object_bytes(raw, "current publisher runtime catalog")
        validate_recursive_types(catalog, "currentPublisherCatalog")
        index, entry = _extract_reconstructed_runtime_entry(
            catalog,
            "currentPublisherCatalog",
        )
        expected_entry = publisher_module.prepare_reconstructed_runtime_entry(
            DEFAULT_PROGRAM
        )
        publisher_module.validate_reconstructed_runtime_entry(expected_entry)
        strict_ordered_equal(
            entry,
            expected_entry,
            "publisherRuntimeEntryVsFrozenExpected",
        )
        if use_cache:
            _PUBLISHER_CATALOG_CACHE = catalog
            _PUBLISHER_EXPECTED_ENTRY_CACHE = expected_entry
    else:
        catalog = _PUBLISHER_CATALOG_CACHE
        index = RUNTIME_ENTRY_EFFECT_INDEX
        entry = catalog["effects"][index]
        publisher_module.validate_reconstructed_runtime_entry(entry)
        require(_PUBLISHER_EXPECTED_ENTRY_CACHE is not None,
                "publisher expected-entry cache is missing")
        expected_entry = _PUBLISHER_EXPECTED_ENTRY_CACHE
        strict_ordered_equal(
            entry,
            expected_entry,
            "publisherRuntimeEntryVsFrozenExpected",
        )
    require(index == RUNTIME_ENTRY_EFFECT_INDEX, "Artist 31470 runtime effect order changed")
    require(len(catalog["components"]) == RUNTIME_CATALOG_COMPONENT_COUNT,
            "runtime component denominator changed")
    require(len(catalog["effects"]) == RUNTIME_CATALOG_EFFECT_COUNT,
            "runtime effect denominator changed")

    link = entry["reconstructedRuntimeProgram"]
    receipt = entry["publishReceipt"]
    require(tuple(entry.keys()) == tuple(publisher_module.RECONSTRUCTED_ENTRY_KEYS),
            "publisher outer-10 key/order changed")
    require(tuple(link.keys()) == tuple(publisher_module.RECONSTRUCTED_LINK_KEYS),
            "publisher link-16 key/order changed")
    require(tuple(receipt.keys()) == tuple(publisher_module.RECONSTRUCTED_RECEIPT_KEYS),
            "publisher receipt-25 key/order changed")
    require(len(entry) == 10 and len(link) == 16 and len(receipt) == 25,
            "publisher exact tuple denominator changed")
    require(len(receipt["toolDependencies"]) == 3,
            "publisher tool dependency denominator changed")
    require(canonical_sha256(entry) == RUNTIME_ENTRY_CANONICAL_SHA256,
            "publisher runtime entry canonical identity changed")
    require(canonical_sha256(link) == RUNTIME_LINK_SHA256,
            "publisher runtime link identity changed")
    require(receipt["receiptSha256"] == RUNTIME_RECEIPT_SELF_SHA256,
            "publisher receipt self identity changed")
    require(entry["publishReceiptSha256"] == RUNTIME_OUTER_RECEIPT_SHA256,
            "publisher outer receipt identity changed")
    require(canonical_sha256(receipt) == RUNTIME_OUTER_RECEIPT_SHA256,
            "publisher complete receipt identity changed")
    require(entry["sourceExact"] is False,
            "publisher sourceExact admission changed")
    require(entry["runtimeExecutionAdmission"] is False,
            "publisher runtime execution admission changed")
    require(entry["productAdmission"] is False,
            "publisher Product admission changed")
    return raw, catalog, entry, expected_entry


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
        (PUBLISHER_INTEGRATION_COMMIT, PUBLISHER_TREE),
        (PUBLISHER_ORIGINAL_COMMIT, PUBLISHER_TREE),
    ):
        _validate_git_identity(commit, tree)
    _validate_git_blob(BASE_INTEGRATION_COMMIT, PROGRAM_PATH, PROGRAM_BLOB_ID)
    _validate_git_blob(APPROVAL_AUTHORITY_COMMIT, APPROVAL_PATH, APPROVAL_BLOB_ID)
    _validate_git_blob(
        MATERIAL_TEXTURE_AUTHORITY_COMMIT,
        MATERIAL_TEXTURE_PATH,
        MATERIAL_TEXTURE_BLOB_ID,
    )
    _validate_git_blob(
        PUBLISHER_INTEGRATION_COMMIT,
        RUNTIME_CATALOG_PATH,
        RUNTIME_CATALOG_BLOB_ID,
    )
    _validate_git_blob(
        PUBLISHER_ORIGINAL_COMMIT,
        RUNTIME_CATALOG_PATH,
        RUNTIME_CATALOG_BLOB_ID,
    )
    for path, blob_id in PUBLISHER_TOOL_BLOBS:
        _validate_git_blob(PUBLISHER_INTEGRATION_COMMIT, path, blob_id)
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
    publisher_catalog: dict[str, Any],
    publisher_entry: dict[str, Any],
) -> dict[str, Any]:
    material_texture = _material_texture_artifact(program)
    parser_files = []
    for path, blob_id in PARSER_FILES:
        parser_files.append({
            "path": path,
            "parserIntegrationBlobId": blob_id,
            "currentTrackedTextSha256": strict_io.tracked_text_sha256(ROOT / path),
        })
    publisher_link = publisher_entry["reconstructedRuntimeProgram"]
    publisher_receipt = publisher_entry["publishReceipt"]
    publisher_tools = []
    for tool, (path, blob_id) in zip(
        publisher_receipt["toolDependencies"],
        PUBLISHER_TOOL_BLOBS,
        strict=True,
    ):
        require(tool["path"] == path, "publisher tool/blob order changed")
        publisher_tools.append({
            "role": tool["role"],
            "path": tool["path"],
            "hashDomain": tool["hashDomain"],
            "sha256": tool["sha256"],
            "publisherIntegrationBlobId": blob_id,
        })
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
            "path": RUNTIME_CATALOG_PATH,
            "publisherIntegrationCommitId": PUBLISHER_INTEGRATION_COMMIT,
            "publisherOriginalCommitId": PUBLISHER_ORIGINAL_COMMIT,
            "publisherTreeId": PUBLISHER_TREE,
            "trackedBlobId": RUNTIME_CATALOG_BLOB_ID,
            "currentCheckoutByteCount": RUNTIME_CATALOG_BYTE_COUNT,
            "currentCheckoutRawSha256": RUNTIME_CATALOG_RAW_SHA256,
            "currentCheckoutCarriageReturnCount": 1,
            "schema": publisher_catalog["schema"],
            "formatVersion": publisher_catalog["formatVersion"],
            "componentCount": len(publisher_catalog["components"]),
            "effectCount": len(publisher_catalog["effects"]),
            "artist31470EffectIndex": RUNTIME_ENTRY_EFFECT_INDEX,
            "outerKeyCount": len(publisher_entry),
            "outerKeyOrder": list(publisher_entry.keys()),
            "outerCanonicalSha256": RUNTIME_ENTRY_CANONICAL_SHA256,
            "linkKeyCount": len(publisher_link),
            "linkKeyOrder": list(publisher_link.keys()),
            "linkCanonicalSha256": RUNTIME_LINK_SHA256,
            "receiptKeyCount": len(publisher_receipt),
            "receiptKeyOrder": list(publisher_receipt.keys()),
            "receiptSelfSha256": RUNTIME_RECEIPT_SELF_SHA256,
            "outerPublishReceiptSha256": RUNTIME_OUTER_RECEIPT_SHA256,
            "toolDependencyCount": len(publisher_tools),
            "toolDependencies": publisher_tools,
            "publicValidator": {
                "path": "Tools/EffectPipeline/build_effect_derived_artifact.py",
                "function": "validate_reconstructed_runtime_entry",
                "currentToolReadsRequired": True,
                "extractedEntryStrictEqualFrozenExpected": True,
            },
            "payloadKind": publisher_entry["payloadKind"],
            "effectAssetId": publisher_entry["effectAssetId"],
            "artifactRevision": publisher_entry["artifactRevision"],
            "compilerRevision": publisher_entry["compilerRevision"],
            "sourceExact": publisher_entry["sourceExact"],
            "runtimeExecutionAdmission": publisher_entry["runtimeExecutionAdmission"],
            "productAdmission": publisher_entry["productAdmission"],
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
    _inputs_already_validated: bool = False,
    _publisher_authority: tuple[
        bytes, dict[str, Any], dict[str, Any], dict[str, Any]
    ] | None = None,
) -> tuple[dict[str, Any], dict[str, bytes]]:
    _validate_frozen_git_inputs()
    approved_root = _require_canonical_resources_root(resources_root)
    if not _inputs_already_validated:
        program_module.validate_program(program)
        approval_builder.validate_receipt(approval, program)
    require(program["programSha256"] == PROGRAM_SHA256, "program identity changed")
    require(approval["receiptSha256"] == APPROVAL_SELF_SHA256, "approval identity changed")
    _material_texture_artifact(program)
    if _publisher_authority is None:
        _publisher_authority = _read_publisher_runtime_authority()
    _, publisher_catalog, publisher_entry, publisher_expected = _publisher_authority
    strict_ordered_equal(
        publisher_entry,
        publisher_expected,
        "publisherRuntimeEntryVsFrozenExpected",
    )

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
            publisher_catalog,
            publisher_entry,
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
    supplied_resource_payloads: Mapping[str, bytes] | None = None,
    supplied_approval_bytes: bytes | None = None,
    supplied_tool_bytes: bytes | None = None,
    supplied_publisher_entry: dict[str, Any] | None = None,
    supplied_runtime_catalog_bytes: bytes | None = None,
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

    _, approved_program = _read_program()
    _, approved_material = _read_approval(approved_program)
    if program is not None:
        program_module.validate_program(program)
        strict_ordered_equal(program, approved_program, "suppliedProgram")
    if approval is not None:
        approval_builder.validate_receipt(approval, approved_program)
        strict_ordered_equal(approval, approved_material, "suppliedApproval")
    if supplied_approval_bytes is not None:
        supplied_approval = _parse_json_object_bytes(
            supplied_approval_bytes, "supplied Material approval"
        )
        approval_builder.validate_receipt(supplied_approval, approved_program)
        strict_ordered_equal(supplied_approval, approved_material, "suppliedApprovalBytes")
    if supplied_tool_bytes is not None:
        require(
            hashlib.sha256(strict_io.normalize_utf8_eol(supplied_tool_bytes)).hexdigest()
            == strict_io.tracked_text_sha256(DEFAULT_TOOL),
            "supplied current-tool object differs from actual validator",
        )

    publisher_authority = _read_publisher_runtime_authority()
    _, actual_catalog, actual_publisher_entry, _ = publisher_authority
    if supplied_publisher_entry is not None:
        publisher_module.validate_reconstructed_runtime_entry(
            supplied_publisher_entry
        )
        strict_ordered_equal(
            supplied_publisher_entry,
            actual_publisher_entry,
            "suppliedPublisherEntry",
        )
    if supplied_runtime_catalog_bytes is not None:
        supplied_catalog = _parse_json_object_bytes(
            supplied_runtime_catalog_bytes,
            "supplied publisher runtime catalog",
        )
        _, supplied_entry = _extract_reconstructed_runtime_entry(
            supplied_catalog,
            "suppliedPublisherCatalog",
        )
        strict_ordered_equal(
            supplied_entry,
            actual_publisher_entry,
            "suppliedPublisherCatalog.artist31470",
        )
        strict_ordered_equal(
            supplied_catalog,
            actual_catalog,
            "suppliedPublisherCatalog",
        )

    expected, actual_payloads = build_receipt(
        approved_program,
        approved_material,
        resources_root=resources_root,
        _inputs_already_validated=True,
        _publisher_authority=publisher_authority,
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
    approval_raw, approval = _read_approval(program, args.approval)
    require(hashlib.sha256(approval_raw).hexdigest() == APPROVAL_RAW_SHA256,
            "command-line approval is not frozen")
    receipt, _ = build_receipt(program, approval)
    validate_receipt(
        receipt,
        program,
        approval,
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
