#!/usr/bin/env python3
"""Freeze the fail-closed Artist F Material numeric-oracle acquisition matrix.

This receipt records what survived in the 23 cooked Material families and the
exact current installation roots searched for an independent arithmetic
oracle.  It never treats a cooked native tail or a current script package as a
historical shader graph, and it never opens execution or Product admission.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any

import build_artist_31470_material_evidence_contract as material_contract
from extract_artist_31470_material_render_state import parse_property_records
from extract_ue3_effect_material_closure import load_package
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
)


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
CONTRACT_ROOT = "ARTIST/31470/F"
EXPECTED_FAMILY_COUNT = 23
EXPECTED_RECIPE_COUNT = 27
EXPECTED_EXPRESSION_COUNT = 925
EXPECTED_NULL_EXPRESSION_COUNT = 1803
EXPECTED_RESOLVED_EDGE_COUNT = 125
EXPECTED_UNRESOLVED_EDGE_COUNT = 502
EXPECTED_SCRIPT_PACKAGES = (
    {
        "packageRole": "CURRENT_ENGINE_SCRIPT_PACKAGE",
        "relativePath": "EFGame/ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u",
        "byteCount": 1396327,
        "sha256": "cee4257abe9a60730d48bab16e742f12123c71dd7f13faf7807c14647e989434",
    },
    {
        "packageRole": "CURRENT_EFGAME_SCRIPT_PACKAGE",
        "relativePath": "EFGame/ReleasePC/NU1V7NCQ4YAE9ZPJVNOQS.u",
        "byteCount": 893410,
        "sha256": "620a21b9ca6800b6179e2fda07dc28491747d8a44eafdd666e133bed2a5c81cc",
    },
)
EXPECTED_UMODEL = {
    "byteCount": 1766400,
    "sha256": "b9573cdcbb7e9d26dbf60a0e3af47fb5af8543140873da8483c26d58cf40b249",
}
EXPECTED_SHADER_CACHE_PACKAGE = {
    "relativePath": "EFGame/ReleasePC/9XUFAXIP8BXBAP1NIEG66EF.upk",
    "byteCount": 270965156,
    "sha256": "be77e8af4443c4cca5614bec0545c0c735ab04a8b68a3781fb9dfb5a5f2123ad",
    "packageVersion": 868,
    "logicalByteCount": 943207579,
    "exportCount": 1596,
    "shaderCacheExportCount": 1596,
}
EXPECTED_SHADER_CACHE_NAMES = (
    "sc_lv_customizingtool_classselect_sl01",
    "sc_lv_customizingtool_classselect_sl02",
    "sc_lv_customizingtool_classselect_sl03",
    "sc_lv_customizingtool_classselect_sl06",
    "sc_lv_customizingtool_classselect_sl08",
    "sc_lv_eflobby_sl_class",
    "sc_lv_lobby_classselect_sl01",
    "sc_lv_lobby_classselect_sl02",
    "sc_lv_lobby_classselect_sl03",
    "sc_lv_lobby_classselect_sl06",
    "sc_lv_lobby_classselect_sl08",
)
EXPECTED_SHADER_CACHE_CANDIDATE_SHA256 = (
    "c9804e8bad1d49248d9124c45c5104c67540247ecd318b608a6c9a21844d64c0"
)
EXPECTED_MATERIAL_LEAF_PACKAGE = {
    "relativePath": "EFGame/ReleasePC/DKV6KRSCXY3T6D9CJIK3G.upk",
    "byteCount": 141154941,
    "sha256": "c0c3e35b48d8589d2e5014c99c64c0c32e05eace7ae02cfc8e6566f4eaf40150",
    "packageVersion": 868,
    "logicalByteCount": 602422069,
    "exportCount": 1323421,
}
EXPECTED_MATERIAL_LEAF_CANDIDATE_SHA256 = (
    "1932be90d5db54cc22d2f6bb81c0caaba07db95f0b1b951118b00165c3978254"
)
EXPECTED_MATERIAL_NATIVE_STATE_KEYS = (
    "005e33a28768264aa2658973cc2dbfc9",
    "02ad72bf79d0d84690fa3f7c56d84227",
    "0549f19fc653a14dad4cd9c28542db74",
    "06e9a0ae14b09646b949a245fc42aa3c",
    "08ba4c6219d0ae4cab5ae5a9400021cd",
    "3502e06c8444c34f80e2b17bf17019eb",
    "3c8f7117bed1184d81e6dbff7d3cd502",
    "444a3ee447dbcc4d94bd3dafda21bd97",
    "456ccdbe1a2744408b372ff8c96319d4",
    "472b223e00495c46b52f6519a11b0cd0",
    "8ab65484fb34864a94f9bfeb4bd5e4a4",
    "9f1fac11036bed4c8f759d8e0c56cd47",
    "a5bf44053a7aa044a4e0ea92965c03aa",
    "a86b19c58d96174fb4ec89192461506c",
    "ab93547563f5e941ba43974fdaa3038f",
    "ac4afcef4373b74bac49ddf5f9a125f1",
    "b1f4ebf9dc948c41bd2830d999ba16cc",
    "b52d4d07d830af46a01d2ef44ba0f79a",
    "b80be8893528fd448d8958fde338673f",
    "bcac11669fe7d4459770491e85956309",
    "d82768956cbba845af8cd43c040400fa",
    "e589e4b09c1d4a4398c989580bab37ad",
    "e6395f1a35750a44b15b920e2180146b",
)
EXPECTED_MATERIAL_NATIVE_STATE_KEY_SHA256 = (
    "6c08dafe7c8ba8232507e1312dbb7358dc8fe8b8c1c2610266025c844f2d4d0c"
)
EXPECTED_ENVIRONMENT_EVIDENCE_SHA256 = (
    "e28ea5e95ed2de28f5b0f84ce63fca0aeb5193d998f2fce9c782772a1b97e65b"
)
MINIMUM_NUMERIC_ORACLE = {
    "owner": "G05-M",
    "oracleKind": "INDEPENDENT_NUMERIC_MATERIAL_EVALUATOR_OR_CAPTURE",
    "requiredInputDomain": [
        "ordered recipe scalar/vector/texture inputs",
        "resolved static permutation selections",
        "fixed UV/sub-UV, particle parameters, and deterministic sample times",
        "source-grounded sampler and render-state inputs",
    ],
    "requiredOutputDomain": [
        "numeric material output channels used by the recovered family",
        "opacity/opacity-mask/distortion and depth-cull decisions when applicable",
    ],
    "minimumSampleContract": {
        "fixedSeed": 31470,
        "fixedTimeStepSeconds": 0.016666666666666666,
        "minimumDistinctInputVectors": 4,
        "minimumDistinctTimeSamples": 4,
        "absoluteTolerance": 1e-6,
        "relativeTolerance": 1e-6,
    },
    "independenceRequirement": (
        "native/source-era evaluator capture or independently authenticated "
        "historical shader bytecode; the reconstructed evaluator cannot certify itself"
    ),
    "imageValidationAllowed": False,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    return material_contract.load_json(path)


def repository_path(path: Path) -> str:
    return path.resolve().relative_to(REPO_ROOT.resolve()).as_posix()


def source_evidence(path: Path, role: str) -> dict[str, Any]:
    return {
        "role": role,
        "path": repository_path(path),
        "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
        "canonicalTextSha256": material_contract.tracked_json_text_sha256(path),
    }


def tool_evidence(path: Path, role: str) -> dict[str, Any]:
    return {
        "role": role,
        "path": repository_path(path),
        "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        "canonicalTextSha256": material_contract.tracked_source_text_sha256(path),
    }


def normalize_root(path: Path) -> str:
    return path.resolve().as_posix()


def package_identity(path: Path, expected: dict[str, Any]) -> None:
    require(path.is_file(), f"candidate package is missing: {path}")
    require(
        path.stat().st_size == expected["byteCount"]
        and raw_sha256(path) == expected["sha256"],
        f"candidate package raw identity changed: {path}",
    )


def serial_row(
    package: Any,
    entry: Any,
    class_name: str,
    include_material_tail: bool = False,
) -> dict[str, Any]:
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    result = {
        "exportIndex": entry.index,
        "packageReference": entry.index + 1,
        "objectPath": package_ref_path(
            entry.index + 1, package.imports, package.exports
        ),
        "className": class_name,
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": hashlib.sha256(serial).hexdigest(),
    }
    if include_material_tail:
        _, property_start, property_end = parse_property_records(
            serial, package.names, package.summary.version
        )
        trailing = serial[property_end:]
        require(
            len(trailing) >= 32,
            f"installed Material native tail is too short: {entry.object_name}",
        )
        result.update(
            {
                "propertyStreamStart": property_start,
                "propertyStreamEnd": property_end,
                "nativeTailByteCount": len(trailing),
                "nativeTailSha256": hashlib.sha256(trailing).hexdigest(),
                "nativeStateKey": {
                    "offsetInNativeTail": 16,
                    "byteCount": 16,
                    "hex": trailing[16:32].hex(),
                    "status": "OBSERVED_CURRENT_REVISION_UNINTERPRETED",
                },
            }
        )
    return result


def capture_environment(
    root: Path,
    umodel_exe: Path,
    family_targets: list[dict[str, str]],
) -> dict[str, Any]:
    require(root.is_dir(), f"installed release root is missing: {root}")
    require(umodel_exe.is_file(), f"UModel v7 is missing: {umodel_exe}")
    require(
        umodel_exe.stat().st_size == EXPECTED_UMODEL["byteCount"]
        and raw_sha256(umodel_exe) == EXPECTED_UMODEL["sha256"],
        "UModel v7 raw identity changed",
    )
    script_packages: list[dict[str, Any]] = []
    for expected in EXPECTED_SCRIPT_PACKAGES:
        path = root / Path(expected["relativePath"])
        require(path.is_file(), f"current script package is missing: {path}")
        actual = {
            **expected,
            "hashDomain": "RAW_ARTIFACT_BYTES",
            "fidelity": "CURRENT_REVISION_LATE_PINNED_METADATA_ONLY",
        }
        require(
            path.stat().st_size == expected["byteCount"]
            and raw_sha256(path) == expected["sha256"],
            f"current script package identity changed: {path}",
        )
        script_packages.append(actual)
    shader_path = root / Path(EXPECTED_SHADER_CACHE_PACKAGE["relativePath"])
    package_identity(shader_path, EXPECTED_SHADER_CACHE_PACKAGE)
    shader_package = load_package(shader_path, LOSTARK_KR_AES_KEY)
    require(
        shader_package.summary.version
        == EXPECTED_SHADER_CACHE_PACKAGE["packageVersion"]
        and len(shader_package.logical)
        == EXPECTED_SHADER_CACHE_PACKAGE["logicalByteCount"]
        and len(shader_package.exports)
        == EXPECTED_SHADER_CACHE_PACKAGE["exportCount"],
        "ShaderCache package table identity changed",
    )
    shader_rows: list[dict[str, Any]] = []
    shader_export_count = 0
    state_key_pattern = re.compile(
        b"|".join(
            re.escape(bytes.fromhex(value))
            for value in EXPECTED_MATERIAL_NATIVE_STATE_KEYS
        )
    )
    state_key_hits: list[dict[str, Any]] = []
    wanted_names = {name.casefold() for name in EXPECTED_SHADER_CACHE_NAMES}
    for entry in shader_package.exports:
        class_name = (
            package_ref_name(
                entry.class_index, shader_package.imports, shader_package.exports
            )
            or ""
        ).casefold()
        if class_name == "shadercache":
            shader_export_count += 1
            serial = shader_package.logical[
                entry.serial_offset : entry.serial_offset + entry.serial_size
            ]
            for match in state_key_pattern.finditer(serial):
                state_key_hits.append(
                    {
                        "stateKeyHex": match.group(0).hex(),
                        "shaderCacheExportIndex": entry.index,
                        "shaderCacheObjectPath": package_ref_path(
                            entry.index + 1,
                            shader_package.imports,
                            shader_package.exports,
                        ),
                        "offsetInShaderCacheSerial": match.start(),
                    }
                )
        if entry.object_name.casefold() in wanted_names:
            require(class_name == "shadercache", "candidate export is not ShaderCache")
            shader_rows.append(serial_row(shader_package, entry, class_name))
    shader_rows.sort(key=lambda row: row["exportIndex"])
    require(
        shader_export_count
        == EXPECTED_SHADER_CACHE_PACKAGE["shaderCacheExportCount"]
        and {row["objectPath"].casefold() for row in shader_rows} == wanted_names
        and len(shader_rows) == len(EXPECTED_SHADER_CACHE_NAMES),
        "ShaderCache candidate set changed",
    )
    require(
        not state_key_hits,
        "a direct Material native-state-key/ShaderCache serial match requires review",
    )
    del shader_package

    material_path = root / Path(EXPECTED_MATERIAL_LEAF_PACKAGE["relativePath"])
    package_identity(material_path, EXPECTED_MATERIAL_LEAF_PACKAGE)
    material_package = load_package(material_path, LOSTARK_KR_AES_KEY)
    require(
        material_package.summary.version
        == EXPECTED_MATERIAL_LEAF_PACKAGE["packageVersion"]
        and len(material_package.logical)
        == EXPECTED_MATERIAL_LEAF_PACKAGE["logicalByteCount"]
        and len(material_package.exports)
        == EXPECTED_MATERIAL_LEAF_PACKAGE["exportCount"],
        "installed Material-leaf package table identity changed",
    )
    material_rows: list[dict[str, Any]] = []
    target_leaves = {
        row["sourceMaterialObjectPath"].casefold().split(".")[-1]
        for row in family_targets
    }
    candidates_by_leaf: dict[str, list[tuple[Any, str, str]]] = {}
    for entry in material_package.exports:
        leaf = entry.object_name.casefold()
        if leaf not in target_leaves:
            continue
        class_name = (
            package_ref_name(
                entry.class_index,
                material_package.imports,
                material_package.exports,
            )
            or ""
        ).casefold()
        if class_name not in {"material", "decalmaterial"}:
            continue
        current_path = package_ref_path(
            entry.index + 1,
            material_package.imports,
            material_package.exports,
        )
        if isinstance(current_path, str):
            candidates_by_leaf.setdefault(leaf, []).append(
                (entry, class_name, current_path)
            )
    for target in sorted(family_targets, key=lambda row: row["familyId"]):
        expected_path = target["sourceMaterialObjectPath"].casefold()
        candidates: list[tuple[Any, str]] = []
        for entry, class_name, current_path in candidates_by_leaf.get(
            expected_path.split(".")[-1], []
        ):
            if current_path.casefold().endswith(expected_path):
                candidates.append((entry, class_name))
        require(
            len(candidates) == 1,
            f"installed Material leaf is not unique: {target}",
        )
        entry, class_name = candidates[0]
        material_rows.append(
            {
                "familyId": target["familyId"],
                "sourceMaterialObjectPath": target["sourceMaterialObjectPath"],
                **serial_row(
                    material_package,
                    entry,
                    class_name,
                    include_material_tail=True,
                ),
            }
        )
    extracted_state_keys = sorted(
        row["nativeStateKey"]["hex"] for row in material_rows
    )
    require(
        extracted_state_keys == list(EXPECTED_MATERIAL_NATIVE_STATE_KEYS)
        and canonical_sha256(extracted_state_keys)
        == EXPECTED_MATERIAL_NATIVE_STATE_KEY_SHA256,
        "installed Material native-state-key fixture changed",
    )
    del material_package
    return {
        "searchId": "artist-31470-current-installation-material-oracle-search",
        "rootKind": "CURRENT_INSTALLED_RELEASE_TREE",
        "searchedRoot": normalize_root(root),
        "hashDomain": "RAW_PACKAGE_CLASS_AND_EXPORT_TABLE_SNAPSHOT",
        "filenameOnlySearchConclusion": "RETIRED_AS_INSUFFICIENT",
        "umodel": {
            "path": normalize_root(umodel_exe),
            "byteCount": EXPECTED_UMODEL["byteCount"],
            "sha256": EXPECTED_UMODEL["sha256"],
            "hashDomain": "RAW_ARTIFACT_BYTES",
            "commandTemplate": (
                "umodel_lostark_v7.exe -list -game=lostark -kr "
                "-path=<ReleasePC> <full physical upk path>"
            ),
            "nameResolveUsed": False,
        },
        "shaderCache": {
            "package": copy.deepcopy(EXPECTED_SHADER_CACHE_PACKAGE),
            "candidateSelectionRationale": (
                "class-select/customizing/effect-lobby cache names are the closest "
                "installed context candidates for Artist F; material-to-cache entry "
                "membership remains unproven"
            ),
            "candidates": shader_rows,
            "candidateSha256": canonical_sha256(shader_rows),
            "nativeStateKeyJoinSearch": {
                "searchDomain": "ALL_1596_SHADERCACHE_EXPORT_SERIAL_BYTES",
                "comparison": "EXACT_16_BYTE_SUBSEQUENCE",
                "materialStateKeyCount": len(EXPECTED_MATERIAL_NATIVE_STATE_KEYS),
                "materialStateKeySha256": EXPECTED_MATERIAL_NATIVE_STATE_KEY_SHA256,
                "matchCount": len(state_key_hits),
                "matches": state_key_hits,
                "outcome": "NO_DIRECT_STATE_KEY_BINDING",
            },
            "decoderStatus": "BINARY_SCHEMA_UNRESOLVED",
            "numericOracleAvailable": False,
        },
        "installedMaterialLeaves": {
            "package": copy.deepcopy(EXPECTED_MATERIAL_LEAF_PACKAGE),
            "selectionRule": "unique current object path suffix match per source family",
            "families": material_rows,
            "classCounts": dict(
                sorted(Counter(row["className"] for row in material_rows).items())
            ),
            "familyEvidenceSha256": canonical_sha256(material_rows),
            "coverageStatus": "CURRENT_REVISION_23_OF_23_COOKED_PARTIAL",
            "historicalSourceEquivalence": False,
        },
        "scriptPackages": script_packages,
        "historicalSourceEquivalence": False,
        "graphOracleAvailable": False,
    }


def validate_environment(
    snapshot: Any,
    family_targets: list[dict[str, str]],
    installed_root: Path | None = None,
    umodel_exe: Path | None = None,
) -> None:
    require(isinstance(snapshot, dict), "environment search snapshot is missing")
    require(
        snapshot.get("searchId")
        == "artist-31470-current-installation-material-oracle-search"
        and snapshot.get("rootKind") == "CURRENT_INSTALLED_RELEASE_TREE"
        and snapshot.get("hashDomain")
        == "RAW_PACKAGE_CLASS_AND_EXPORT_TABLE_SNAPSHOT"
        and snapshot.get("filenameOnlySearchConclusion")
        == "RETIRED_AS_INSUFFICIENT"
        and snapshot.get("historicalSourceEquivalence") is False
        and snapshot.get("graphOracleAvailable") is False,
        "environment search evidence was weakened",
    )
    require(
        snapshot.get("scriptPackages")
        == [
            {
                **row,
                "hashDomain": "RAW_ARTIFACT_BYTES",
                "fidelity": "CURRENT_REVISION_LATE_PINNED_METADATA_ONLY",
            }
            for row in EXPECTED_SCRIPT_PACKAGES
        ],
        "current script package evidence changed",
    )
    umodel = snapshot.get("umodel")
    require(
        isinstance(umodel, dict)
        and umodel.get("byteCount") == EXPECTED_UMODEL["byteCount"]
        and umodel.get("sha256") == EXPECTED_UMODEL["sha256"]
        and umodel.get("hashDomain") == "RAW_ARTIFACT_BYTES"
        and umodel.get("commandTemplate")
        == (
            "umodel_lostark_v7.exe -list -game=lostark -kr "
            "-path=<ReleasePC> <full physical upk path>"
        )
        and umodel.get("nameResolveUsed") is False,
        "UModel acquisition evidence changed",
    )
    shader = snapshot.get("shaderCache")
    state_key_search = shader.get("nativeStateKeyJoinSearch") if isinstance(shader, dict) else None
    require(
        isinstance(shader, dict)
        and shader.get("package") == EXPECTED_SHADER_CACHE_PACKAGE
        and shader.get("decoderStatus") == "BINARY_SCHEMA_UNRESOLVED"
        and shader.get("numericOracleAvailable") is False
        and type(shader.get("candidates")) is list
        and len(shader["candidates"]) == len(EXPECTED_SHADER_CACHE_NAMES)
        and {row.get("objectPath", "").casefold() for row in shader["candidates"]}
        == {name.casefold() for name in EXPECTED_SHADER_CACHE_NAMES}
        and shader.get("candidateSha256")
        == canonical_sha256(shader["candidates"]),
        "ShaderCache acquisition evidence changed",
    )
    require(
        isinstance(state_key_search, dict)
        and state_key_search.get("searchDomain")
        == "ALL_1596_SHADERCACHE_EXPORT_SERIAL_BYTES"
        and state_key_search.get("comparison") == "EXACT_16_BYTE_SUBSEQUENCE"
        and state_key_search.get("materialStateKeyCount") == 23
        and state_key_search.get("materialStateKeySha256")
        == EXPECTED_MATERIAL_NATIVE_STATE_KEY_SHA256
        and state_key_search.get("matchCount") == 0
        and state_key_search.get("matches") == []
        and state_key_search.get("outcome") == "NO_DIRECT_STATE_KEY_BINDING",
        "direct Material-state-key/ShaderCache join boundary changed",
    )
    if EXPECTED_SHADER_CACHE_CANDIDATE_SHA256:
        require(
            shader["candidateSha256"]
            == EXPECTED_SHADER_CACHE_CANDIDATE_SHA256,
            "ShaderCache candidate identity fixture changed",
        )
    leaves = snapshot.get("installedMaterialLeaves")
    require(
        isinstance(leaves, dict)
        and leaves.get("package") == EXPECTED_MATERIAL_LEAF_PACKAGE
        and leaves.get("coverageStatus")
        == "CURRENT_REVISION_23_OF_23_COOKED_PARTIAL"
        and leaves.get("historicalSourceEquivalence") is False
        and type(leaves.get("families")) is list
        and len(leaves["families"]) == EXPECTED_FAMILY_COUNT
        and leaves.get("classCounts") == {"decalmaterial": 1, "material": 22}
        and {row.get("familyId") for row in leaves["families"]}
        == {row["familyId"] for row in family_targets}
        and leaves.get("familyEvidenceSha256")
        == canonical_sha256(leaves["families"]),
        "installed Material-leaf evidence changed",
    )
    if EXPECTED_MATERIAL_LEAF_CANDIDATE_SHA256:
        require(
            leaves["familyEvidenceSha256"]
            == EXPECTED_MATERIAL_LEAF_CANDIDATE_SHA256,
            "installed Material-leaf identity fixture changed",
        )
    require(
        isinstance(snapshot.get("searchedRoot"), str)
        and snapshot.get("searchedRoot"),
        "searched root is missing",
    )
    if EXPECTED_ENVIRONMENT_EVIDENCE_SHA256:
        require(
            canonical_sha256(snapshot) == EXPECTED_ENVIRONMENT_EVIDENCE_SHA256,
            "installed oracle-acquisition environment fixture changed",
        )
    if installed_root is not None:
        require(umodel_exe is not None, "deep check requires UModel v7")
        require(
            capture_environment(installed_root, umodel_exe, family_targets)
            == snapshot,
            "current installation search snapshot changed",
        )


def explicit_field_count(expressions: list[dict[str, Any]], field_name: str) -> int:
    return sum(
        isinstance(row.get("fields", {}).get(field_name), dict)
        and row["fields"][field_name].get("status") == "SERIALIZED_EXPLICIT"
        for row in expressions
    )


def family_targets(contract: dict[str, Any]) -> list[dict[str, str]]:
    return sorted(
        [
            {
                "familyId": str(row["familyId"]),
                "sourceMaterialObjectPath": str(
                    row["exactIdentity"]["materialObjectPath"]
                ),
            }
            for row in contract["graphFamilies"]
        ],
        key=lambda row: row["familyId"],
    )


def family_matrix(
    contract: dict[str, Any],
    raw_receipt: dict[str, Any],
    environment: dict[str, Any],
) -> list[dict[str, Any]]:
    raw_exports = {row["evidenceId"]: row for row in raw_receipt["exports"]}
    expressions_by_base: dict[str, list[dict[str, Any]]] = {}
    for row in raw_receipt["graphExpressions"]:
        expressions_by_base.setdefault(row["baseMaterialEvidenceId"], []).append(row)
    bindings = {
        row["sourceMaterialPath"].casefold(): row for row in raw_receipt["bindings"]
    }
    recipes_by_family: dict[str, list[dict[str, Any]]] = {}
    for recipe in contract["materialRecipes"]:
        recipes_by_family.setdefault(recipe["arithmeticFamilyId"], []).append(recipe)
    installed_leaf_by_family = {
        row["familyId"]: row
        for row in environment["installedMaterialLeaves"]["families"]
    }

    result: list[dict[str, Any]] = []
    for family in sorted(contract["graphFamilies"], key=lambda row: row["familyId"]):
        base_id = family["rawEvidence"]["baseMaterialEvidenceId"]
        base_export = raw_exports[base_id]
        expressions = sorted(
            expressions_by_base.get(base_id, []), key=lambda row: row["sourceOrder"]
        )
        recipes = sorted(
            recipes_by_family.get(family["familyId"], []),
            key=lambda row: row["recipeId"],
        )
        installed_leaf = installed_leaf_by_family[family["familyId"]]
        node_types = dict(sorted(Counter(row["className"] for row in expressions).items()))
        resolved_edges = sum(
            type(edge.get("packageIndex")) is int
            and edge.get("packageIndex") != 0
            and isinstance(edge.get("objectPath"), str)
            and bool(edge.get("objectPath"))
            for row in expressions
            for edge in row["projection"]["inputs"]
        )
        unresolved_edges = sum(
            not (
                type(edge.get("packageIndex")) is int
                and edge.get("packageIndex") != 0
                and isinstance(edge.get("objectPath"), str)
                and bool(edge.get("objectPath"))
            )
            for row in expressions
            for edge in row["projection"]["inputs"]
        )
        recipe_acquisition: list[dict[str, Any]] = []
        for recipe in recipes:
            binding = bindings[recipe["sourceMaterialPath"].casefold()]
            source_export = raw_exports[binding["sourceExportEvidenceId"]]
            recipe_acquisition.append(
                {
                    "recipeId": recipe["recipeId"],
                    "sourceMaterialPath": recipe["sourceMaterialPath"],
                    "sourceExportEvidenceId": source_export["evidenceId"],
                    "sourceClassName": source_export["className"],
                    "sourcePhysicalPackage": source_export["physicalPackage"],
                    "sourcePhysicalPackageSha256": source_export[
                        "physicalPackageSha256"
                    ],
                    "nativeTailByteCount": source_export["trailingByteCount"],
                    "nativeTailSha256": source_export["trailingBytesSha256"],
                    "nativeTailStatus": "OBSERVED_OPAQUE_NOT_A_GRAPH_ORACLE",
                    "oracleStatus": "SHADERCACHE_PRESENT_DECODER_PENDING",
                }
            )
        blockers = [set(recipe["blockers"]) for recipe in recipes]
        result.append(
            {
                "familyId": family["familyId"],
                "familyIdentitySha256": family["identitySha256"],
                "graphProvenance": "RECONSTRUCTED_GRAPH",
                "sourceExactGraph": False,
                "exactBaseMaterialIdentity": copy.deepcopy(family["exactIdentity"]),
                "baseMaterialRawEvidence": {
                    "evidenceId": base_id,
                    "className": base_export["className"],
                    "physicalPackage": base_export["physicalPackage"],
                    "physicalPackageSha256": base_export[
                        "physicalPackageSha256"
                    ],
                    "exportIndex": base_export["exportIndex"],
                    "packageReference": base_export["packageReference"],
                    "objectPath": base_export["objectPath"],
                    "serialOffset": base_export["serialOffset"],
                    "serialSize": base_export["serialSize"],
                    "serialSha256": base_export["serialSha256"],
                    "propertyStreamEnd": base_export["propertyStreamEnd"],
                    "nativeTailByteCount": base_export["trailingByteCount"],
                    "nativeTailSha256": base_export["trailingBytesSha256"],
                    "nativeTailStatus": "OBSERVED_OPAQUE_NOT_A_GRAPH_ORACLE",
                },
                "survivingEvidence": {
                    "expressionEntryCount": family["cookedEvidence"][
                        "expressionEntryCount"
                    ],
                    "nonNullExpressionCount": len(expressions),
                    "nodeTypeCounts": node_types,
                    "resolvedInputEdgeCount": resolved_edges,
                    "serializedParameterNameCount": explicit_field_count(
                        expressions, "parametername"
                    ),
                    "serializedDefaultValueCount": explicit_field_count(
                        expressions, "defaultvalue"
                    ),
                    "serializedTextureDefaultCount": explicit_field_count(
                        expressions, "texture"
                    ),
                    "expressionEvidenceSha256": family["rawEvidence"][
                        "expressionEvidenceSha256"
                    ],
                },
                "missingExactInputs": {
                    "cookedNullExpressionSlotCount": family["cookedEvidence"][
                        "nullExpressionCount"
                    ],
                    "unresolvedInputEdgeCount": unresolved_edges,
                    "staticSelectionUnresolvedRecipeCount": sum(
                        "STATIC_PERMUTATION_SELECTIONS_UNRESOLVED" in row
                        for row in blockers
                    ),
                    "fullRenderStateUnresolvedRecipeCount": sum(
                        "FULL_RENDER_STATE_UNRESOLVED" in row for row in blockers
                    ),
                    "fullCullModeUnresolvedRecipeCount": sum(
                        "FULL_CULL_MODE_UNRESOLVED" in row for row in blockers
                    ),
                    "samplerIncompleteRecipeCount": sum(
                        "SAMPLER_BINDINGS_INCOMPLETE" in row for row in blockers
                    ),
                    "materialShaderMapKeyUnresolved": True,
                    "independentNumericOracleMissing": True,
                },
                "candidateArtifactsChecked": [
                    {
                        "artifactClass": "COOKED_BASE_MATERIAL_NATIVE_TAIL",
                        "artifactName": base_export["objectPath"],
                        "packageRoot": base_export["logicalPackage"],
                        "outcome": "OPAQUE_NATIVE_TAIL_OBSERVED_NOT_GRAPH_ORACLE",
                    },
                    {
                        "artifactClass": "CURRENT_INSTALLED_MATERIAL_LEAF",
                        "artifactName": installed_leaf["objectPath"],
                        "packageRoot": environment["searchedRoot"],
                        "rawIdentity": copy.deepcopy(installed_leaf),
                        "outcome": "CURRENT_REVISION_COOKED_PARTIAL_NOT_GRAPH_ORACLE",
                    },
                    {
                        "artifactClass": "INSTALLED_SHADERCACHE_EXPORT_SET",
                        "artifactName": environment["shaderCache"]["package"][
                            "relativePath"
                        ],
                        "packageRoot": environment["searchedRoot"],
                        "candidateExportCount": len(
                            environment["shaderCache"]["candidates"]
                        ),
                        "candidateSha256": environment["shaderCache"][
                            "candidateSha256"
                        ],
                        "outcome": "PRESENT_BINARY_DECODER_PENDING",
                    },
                    {
                        "artifactClass": "CURRENT_SCRIPT_PACKAGE_METADATA",
                        "artifactName": "Engine.u + EFGame.u",
                        "packageRoot": environment["searchedRoot"],
                        "outcome": "CURRENT_REVISION_ONLY_NOT_HISTORICAL_GRAPH_ORACLE",
                    },
                ],
                "recipeAcquisition": recipe_acquisition,
                "minimumIndependentNumericOracle": copy.deepcopy(
                    MINIMUM_NUMERIC_ORACLE
                ),
                "oracleStatus": "SHADERCACHE_PRESENT_DECODER_PENDING",
                "evaluatorImplemented": False,
                "executable": False,
                "product": False,
                "nextOwner": "G05-M",
            }
        )
    return result


def build_receipt(
    contract: dict[str, Any],
    raw_receipt: dict[str, Any],
    closure: dict[str, Any],
    environment: dict[str, Any],
    contract_path: Path,
    raw_receipt_path: Path,
    closure_path: Path,
) -> dict[str, Any]:
    material_contract.validate_contract(contract)
    rows = material_contract.closure_rows(closure)
    material_contract.validate_render_receipt(raw_receipt, rows)
    targets = family_targets(contract)
    validate_environment(environment, targets)
    require(
        contract.get("sourceEvidence", {})
        .get("renderStateReceipt", {})
        .get("canonicalTextSha256")
        == material_contract.tracked_json_text_sha256(raw_receipt_path),
        "typed contract is not bound to the selected raw render receipt",
    )
    families = family_matrix(contract, raw_receipt, environment)
    summary = {
        "materialFamilyCount": len(families),
        "materialRecipeCount": sum(len(row["recipeAcquisition"]) for row in families),
        "survivingExpressionCount": sum(
            row["survivingEvidence"]["nonNullExpressionCount"] for row in families
        ),
        "cookedNullExpressionCount": sum(
            row["missingExactInputs"]["cookedNullExpressionSlotCount"]
            for row in families
        ),
        "resolvedInputEdgeCount": sum(
            row["survivingEvidence"]["resolvedInputEdgeCount"] for row in families
        ),
        "unresolvedInputEdgeCount": sum(
            row["missingExactInputs"]["unresolvedInputEdgeCount"]
            for row in families
        ),
        "oracleAvailableFamilyCount": 0,
        "implementedEvaluatorCount": 0,
        "executableFamilyCount": 0,
        "productFamilyCount": 0,
        "installedMaterialLeafFamilyCount": len(
            environment["installedMaterialLeaves"]["families"]
        ),
        "installedMaterialLeafClassCounts": copy.deepcopy(
            environment["installedMaterialLeaves"]["classCounts"]
        ),
        "shaderCacheExportCount": environment["shaderCache"]["package"][
            "shaderCacheExportCount"
        ],
        "selectedShaderCacheCandidateCount": len(
            environment["shaderCache"]["candidates"]
        ),
        "familyAcquisitionSha256": canonical_sha256(families),
    }
    receipt = {
        "schema": "lostark.artist-31470-material-oracle-acquisition-receipt",
        "formatVersion": 1,
        "root": CONTRACT_ROOT,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "EVIDENCE_ACQUISITION_ONLY",
        "source": {
            "typedMaterialContract": source_evidence(
                contract_path, "TYPED_MATERIAL_CONTRACT"
            ),
            "rawRenderReceipt": source_evidence(
                raw_receipt_path, "RAW_RENDER_STATE_RECEIPT"
            ),
            "activeMaterialClosure": source_evidence(
                closure_path, "ACTIVE_MATERIAL_CLOSURE"
            ),
            "generator": tool_evidence(SCRIPT_PATH, "ORACLE_ACQUISITION_GENERATOR"),
            "materialContractBuilder": tool_evidence(
                REPO_ROOT
                / "Tools/LevelPlacementExtractor/"
                "build_artist_31470_material_evidence_contract.py",
                "MATERIAL_CONTRACT_VALIDATOR",
            ),
            "rawPackageParser": tool_evidence(
                REPO_ROOT
                / "Tools/LevelPlacementExtractor/extract_ue3_placements.py",
                "RAW_PACKAGE_TABLE_PARSER",
            ),
            "packageLoader": tool_evidence(
                REPO_ROOT
                / "Tools/LevelPlacementExtractor/"
                "extract_ue3_effect_material_closure.py",
                "RAW_PACKAGE_DECOMPRESSOR",
            ),
            "materialPropertyParser": tool_evidence(
                REPO_ROOT
                / "Tools/LevelPlacementExtractor/"
                "extract_artist_31470_material_render_state.py",
                "MATERIAL_PROPERTY_STREAM_PARSER",
            ),
        },
        "environmentSearch": copy.deepcopy(environment),
        "families": families,
        "summary": summary,
        "admission": {
            "evidenceIntegrity": True,
            "executionReady": False,
            "product": False,
            "blockers": [
                "COOKED_STRIPPED_ARITHMETIC_GRAPH",
                "SHADERCACHE_BINARY_DECODER_UNIMPLEMENTED",
                "MATERIAL_SHADER_MAP_KEY_UNRESOLVED",
                "SHADERCACHE_MATERIAL_MEMBERSHIP_UNRESOLVED",
                "RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED",
            ],
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def validate_receipt(
    receipt: dict[str, Any],
    contract: dict[str, Any],
    raw_receipt: dict[str, Any],
    closure: dict[str, Any],
    contract_path: Path,
    raw_receipt_path: Path,
    closure_path: Path,
    installed_root: Path | None = None,
    umodel_exe: Path | None = None,
) -> None:
    require(
        receipt.get("schema")
        == "lostark.artist-31470-material-oracle-acquisition-receipt"
        and type(receipt.get("formatVersion")) is int
        and receipt.get("formatVersion") == 1
        and receipt.get("root") == CONTRACT_ROOT
        and receipt.get("characterClass") == "ARTIST"
        and type(receipt.get("skillId")) is int
        and receipt.get("skillId") == 31470
        and receipt.get("inputSlot") == "F"
        and receipt.get("scope") == "EVIDENCE_ACQUISITION_ONLY",
        "unsupported Material oracle acquisition receipt",
    )
    expected_digest = receipt.get("receiptSha256")
    require(isinstance(expected_digest, str), "oracle receipt digest is missing")
    payload = copy.deepcopy(receipt)
    payload.pop("receiptSha256", None)
    require(
        expected_digest == canonical_sha256(payload),
        "oracle acquisition receipt digest mismatch",
    )
    targets = family_targets(contract)
    validate_environment(
        receipt.get("environmentSearch"),
        targets,
        installed_root,
        umodel_exe,
    )
    expected = build_receipt(
        contract,
        raw_receipt,
        closure,
        receipt["environmentSearch"],
        contract_path,
        raw_receipt_path,
        closure_path,
    )
    require(receipt == expected, "oracle acquisition receipt is not source-derived")
    summary = receipt["summary"]
    require(
        summary["materialFamilyCount"] == EXPECTED_FAMILY_COUNT
        and summary["materialRecipeCount"] == EXPECTED_RECIPE_COUNT
        and summary["survivingExpressionCount"] == EXPECTED_EXPRESSION_COUNT
        and summary["cookedNullExpressionCount"]
        == EXPECTED_NULL_EXPRESSION_COUNT
        and summary["resolvedInputEdgeCount"] == EXPECTED_RESOLVED_EDGE_COUNT
        and summary["unresolvedInputEdgeCount"] == EXPECTED_UNRESOLVED_EDGE_COUNT
        and summary["oracleAvailableFamilyCount"] == 0
        and summary["implementedEvaluatorCount"] == 0
        and summary["executableFamilyCount"] == 0
        and summary["productFamilyCount"] == 0
        and summary["installedMaterialLeafFamilyCount"] == EXPECTED_FAMILY_COUNT
        and summary["installedMaterialLeafClassCounts"]
        == {"decalmaterial": 1, "material": 22}
        and summary["shaderCacheExportCount"] == 1596
        and summary["selectedShaderCacheCandidateCount"]
        == len(EXPECTED_SHADER_CACHE_NAMES),
        "oracle acquisition denominator or admission changed",
    )
    require(
        len(receipt["families"]) == EXPECTED_FAMILY_COUNT
        and all(
            row["oracleStatus"] == "SHADERCACHE_PRESENT_DECODER_PENDING"
            and row["graphProvenance"] == "RECONSTRUCTED_GRAPH"
            and row["sourceExactGraph"] is False
            and row["evaluatorImplemented"] is False
            and row["executable"] is False
            and row["product"] is False
            and row["missingExactInputs"]["materialShaderMapKeyUnresolved"]
            is True
            and row["minimumIndependentNumericOracle"]["imageValidationAllowed"]
            is False
            for row in receipt["families"]
        ),
        "oracle or Product fidelity was laundered",
    )


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--typed-contract", type=Path, required=True)
    parser.add_argument("--raw-render-receipt", type=Path, required=True)
    parser.add_argument("--material-closure", type=Path, required=True)
    parser.add_argument("--installed-release-root", type=Path)
    parser.add_argument("--umodel-exe", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--check", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    contract = load_json(args.typed_contract)
    raw_receipt = load_json(args.raw_render_receipt)
    closure = load_json(args.material_closure)
    if args.check:
        checked = load_json(args.output)
        validate_receipt(
            checked,
            contract,
            raw_receipt,
            closure,
            args.typed_contract,
            args.raw_render_receipt,
            args.material_closure,
            args.installed_release_root,
            args.umodel_exe,
        )
        print(
            "Artist F Material oracle acquisition check: "
            "families=23 expressions=925 edges=125/502 oracle=0 product=false"
        )
        return 0
    require(
        args.installed_release_root is not None,
        "generation requires --installed-release-root",
    )
    require(args.umodel_exe is not None, "generation requires --umodel-exe")
    environment = capture_environment(
        args.installed_release_root,
        args.umodel_exe,
        family_targets(contract),
    )
    receipt = build_receipt(
        contract,
        raw_receipt,
        closure,
        environment,
        args.typed_contract,
        args.raw_render_receipt,
        args.material_closure,
    )
    validate_receipt(
        receipt,
        contract,
        raw_receipt,
        closure,
        args.typed_contract,
        args.raw_render_receipt,
        args.material_closure,
        None,
        None,
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(
        "Artist F Material oracle acquisition write: "
        "families=23 expressions=925 edges=125/502 oracle=0 product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
