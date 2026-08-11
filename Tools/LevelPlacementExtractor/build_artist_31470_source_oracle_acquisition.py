#!/usr/bin/env python3
"""Freeze the Artist F Source actual-output provider acquisition result.

This receipt is deliberately an acquisition result, not a semantic oracle.  It
joins the frozen 29-row Source feasibility matrix to an exhaustive read-only
search record and records the smallest external artifact contract that could
change execution readiness.  Accessible local, Git, backup and current-runtime
paths produced zero source-era native output providers.  VSS remains explicitly
permission-unchecked.  Consequently every Source blocker remains blocked and
Product admission remains false.
"""

from __future__ import annotations

import argparse
import copy
import json
import sys
from pathlib import Path
from typing import Any

from build_artist_31470_custom_handler_oracle import (
    validate_receipt as validate_custom_handler_receipt,
)
from build_artist_31470_source_execution_semantics import (
    canonical_sha256,
    canonical_text_sha256,
    json_bytes,
    require,
    sha256_file,
    validate_receipt as validate_source_execution_receipt,
)
from effect_source_contract_io import load_strict_json_object


SCHEMA = "lostark.effect-source-oracle-acquisition"
FORMAT_VERSION = 1
CANONICAL_PLAN_COMMIT = "7ffb8a3bf123703ea451cbe53a178f449f102fbe"
SOURCE_EXECUTION_SELF_SHA256 = (
    "7e1113dd05bcc9b51056cacc27da1805f7a6d26f65dda5b72c99d26c3141a71c"
)
SOURCE_EXECUTION_RAW_SHA256 = (
    "806ea1519330cf7725f99ddd98cbb905199052e64117cce35356196a11b3c8c4"
)
CUSTOM_HANDLER_SELF_SHA256 = (
    "0da627b3ed5b100014f2a2ac1fa3591d861c6a241befee65ca856b406dedaadc"
)
CUSTOM_HANDLER_RAW_SHA256 = (
    "73020e409f448a7716ef100d85338a7a6713912dd957fe44fa50d9a6f9714bac"
)
SOURCE_RECEIPT_RAW_SHA256 = (
    "54d90dc43e4c4c049705c030280aaf4f6147034962bd74e8b974f5fa4e79c3db"
)

SOURCE_EXECUTION_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)
CUSTOM_HANDLER_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.custom-handler-oracle.receipt.json"
)
SOURCE_RECEIPT_PATH = "Data/Effects/Imported/Artist/skill.31470.source-receipt.json"
OUTPUT_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-oracle-acquisition.receipt.json"
)

EXPECTED_CLASS_COUNTS = {
    "efparticlemodulelocationonground": 2,
    "efparticlemodulelocationprimitivecylinderspin": 2,
    "efparticlemodulelocationprimitivecylinderspin_seeded": 3,
    "efparticlemoduletypedatadecal": 3,
    "efparticlemoduletypedatalight": 1,
    "efparticlemodulevelocityoverlifetime": 4,
    "particlemodulecolor_seeded": 2,
    "particlemodulecolorscaleoverlife": 1,
    "particlemodulelifetime_seeded": 1,
    "particlemodulelocation_seeded": 2,
    "particlemodulelocationprimitivecylinder_seeded": 3,
    "particlemodulemeshrotation": 2,
    "particlemodulemeshrotation_seeded": 1,
    "particlemodulesize_seeded": 1,
    "particlemodulevelocity_seeded": 1,
}

STANDARD_SEEDED_CLASSES = (
    "particlemodulecolor_seeded",
    "particlemodulelifetime_seeded",
    "particlemodulelocation_seeded",
    "particlemodulelocationprimitivecylinder_seeded",
    "particlemodulemeshrotation_seeded",
    "particlemodulesize_seeded",
    "particlemodulevelocity_seeded",
)

NATIVE_FAMILY_DEFINITIONS = (
    {
        "clusterId": "source.native-family.ue3-standard-seeded-spawnex.v1",
        "nativeFamily": "UE3_STANDARD_SEEDED_SPAWN_TO_BASE_SPAWN_EX",
        "exactSourceClasses": STANDARD_SEEDED_CLASSES,
        "currentCallableDecision": "ADDRESSABLE_BUT_LIVE_ENGINE_OBJECT_GRAPH_REQUIRED",
        "reconstructionRoute": "CONTROLLED_CURRENT_HOST_OR_PUBLIC_UE3_RECONSTRUCTION",
    },
    {
        "clusterId": "source.native-family.ef-location-on-ground.v1",
        "nativeFamily": "EF_LOCATION_ON_GROUND",
        "exactSourceClasses": ("efparticlemodulelocationonground",),
        "currentCallableDecision": "NO_EXACT_EXPORTED_ENTRY",
        "reconstructionRoute": "SOURCE_ERA_CAPTURE_OR_NATIVE_REVERSE_ENGINEERING",
    },
    {
        "clusterId": "source.native-family.ef-cylinder-spin.v1",
        "nativeFamily": "EF_CYLINDER_SPIN_SEEDED_AND_UNSEEDED",
        "exactSourceClasses": (
            "efparticlemodulelocationprimitivecylinderspin",
            "efparticlemodulelocationprimitivecylinderspin_seeded",
        ),
        "currentCallableDecision": "NO_EXACT_EXPORTED_ENTRY",
        "reconstructionRoute": "SOURCE_ERA_CAPTURE_OR_NATIVE_REVERSE_ENGINEERING",
    },
    {
        "clusterId": "source.native-family.ef-decal-typedata.v1",
        "nativeFamily": "EF_DECAL_TYPEDATA",
        "exactSourceClasses": ("efparticlemoduletypedatadecal",),
        "currentCallableDecision": "NO_EXACT_EXPORTED_ENTRY",
        "reconstructionRoute": "SOURCE_ERA_CAPTURE_OR_NATIVE_REVERSE_ENGINEERING",
    },
    {
        "clusterId": "source.native-family.ef-light-typedata.v1",
        "nativeFamily": "EF_POINT_LIGHT_TYPEDATA",
        "exactSourceClasses": ("efparticlemoduletypedatalight",),
        "currentCallableDecision": "NO_EXACT_EXPORTED_ENTRY",
        "reconstructionRoute": "SOURCE_ERA_CAPTURE_OR_NATIVE_REVERSE_ENGINEERING",
    },
    {
        "clusterId": "source.native-family.ef-velocity-over-life.v1",
        "nativeFamily": "EF_VELOCITY_OVER_LIFETIME",
        "exactSourceClasses": ("efparticlemodulevelocityoverlifetime",),
        "currentCallableDecision": "NO_EXACT_EXPORTED_ENTRY",
        "reconstructionRoute": "SOURCE_ERA_CAPTURE_OR_NATIVE_REVERSE_ENGINEERING",
    },
    {
        "clusterId": "source.native-family.ef-vector-multiply-parameter.v1",
        "nativeFamily": "EF_DISTRIBUTION_VECTOR_MULTIPLY_PARTICLE_PARAMETER",
        "exactSourceClasses": (
            "particlemodulecolorscaleoverlife",
            "particlemodulemeshrotation",
        ),
        "currentCallableDecision": "NO_EXACT_EXPORTED_GET_VALUE_ENTRY",
        "reconstructionRoute": "SOURCE_ERA_CAPTURE_OR_NATIVE_REVERSE_ENGINEERING",
    },
)

CURRENT_PACKAGE_IDENTITIES = {
    "FX_CM_01": {
        "bytes": 923880,
        "sha256": "b11ab557cc8f3c87d1525c0ff9d237aed62f3d49e3394a810d9204066a7bca5b",
        "sourceArchiveExactCopyCount": 3,
    },
    "FX_CM_02": {
        "bytes": 255488,
        "sha256": "79de79f59ac835e96b54c7e2217b55579266239834cc9fd8eede51a7e5e1fc32",
        "sourceArchiveExactCopyCount": 3,
    },
    "FX_PC_SDM_00": {
        "bytes": 158616,
        "sha256": "735db8b2347566c43835f636d2175e9a7e7e24407096a4d6614c17cd85adadaa",
        "sourceArchiveExactCopyCount": 3,
    },
    "FX_PC_SDM_05": {
        "bytes": 96082,
        "sha256": "b2947f7af5409a386cb1bf56ce53358161da47aa9ad3d07d2fc3817be834102d",
        "sourceArchiveExactCopyCount": 3,
    },
    "FX_PC_SDM_07": {
        "bytes": 66557,
        "sha256": "5a05218926f81e9ec4f9d8bc63bb7474bdb92e482b562b0af779c9fbb22e12a5",
        "sourceArchiveExactCopyCount": 0,
    },
}


def artifact_identity(root: Path, relative_path: str, value: dict[str, Any]) -> dict[str, Any]:
    path = root / relative_path
    require(path.is_file(), f"acquisition input is missing: {relative_path}")
    return {
        "path": relative_path,
        "rawSha256": sha256_file(path),
        "canonicalJsonSha256": canonical_sha256(value),
        "receiptSha256": value.get("receiptSha256"),
    }


def build_class_rows(custom: dict[str, Any]) -> list[dict[str, Any]]:
    grouped: dict[str, list[dict[str, Any]]] = {}
    for row in custom["feasibilityMatrix"]["moduleRows"]:
        grouped.setdefault(row["exactSourceClass"], []).append(row)
    require(
        {name: len(rows) for name, rows in grouped.items()} == EXPECTED_CLASS_COUNTS,
        "Source acquisition class/occurrence denominator changed",
    )
    result = []
    for exact_class in sorted(grouped):
        rows = sorted(grouped[exact_class], key=lambda row: row["moduleOccurrenceId"])
        required_outputs = sorted({row["requiredRuntimeOutputs"] for row in rows})
        require(len(required_outputs) == 1, f"required output split changed: {exact_class}")
        result.append({
            "exactSourceClass": exact_class,
            "sourceFamily": rows[0]["family"],
            "moduleOccurrenceIds": [row["moduleOccurrenceId"] for row in rows],
            "moduleOccurrenceCount": len(rows),
            "requiredMutatedOutput": required_outputs[0],
            "sourceEraProviderId": None,
            "actualOutputPilotIds": [],
            "actualOutputOracleCount": 0,
            "numericTolerance": None,
            "owner": copy.deepcopy(rows[0]["owner"]),
            "decision": "BLOCKED_NO_SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER",
        })
    return result


def build_native_family_clusters(class_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    by_class = {row["exactSourceClass"]: row for row in class_rows}
    result = []
    seen: set[str] = set()
    for definition in NATIVE_FAMILY_DEFINITIONS:
        classes = list(definition["exactSourceClasses"])
        require(all(name in by_class for name in classes), "native family class is missing")
        seen.update(classes)
        outputs = sorted({by_class[name]["requiredMutatedOutput"] for name in classes})
        result.append({
            "clusterId": definition["clusterId"],
            "nativeFamily": definition["nativeFamily"],
            "exactSourceClasses": classes,
            "moduleOccurrenceCount": sum(
                by_class[name]["moduleOccurrenceCount"] for name in classes
            ),
            "requiredMutatedOutputs": outputs,
            "sourceEraProviderId": None,
            "sourceEraProviderDecision": "NOT_ACQUIRED",
            "currentCallableDecision": definition["currentCallableDecision"],
            "standaloneActualOutputPilotCount": 0,
            "sourceExactDecision": "BLOCKED",
            "reconstructionCandidate": {
                "route": definition["reconstructionRoute"],
                "status": "NOT_STARTED",
                "explicitUserApprovalRequired": True,
                "mayGrantSourceExact": False,
                "mayGrantReconstructedNumericallyVerifiedOnlyAfterIndependentOutput": True,
            },
            "owner": "SOURCE_SPECIALIST",
        })
    require(seen == set(EXPECTED_CLASS_COUNTS), "native family coverage changed")
    require(sum(row["moduleOccurrenceCount"] for row in result) == 29,
            "native family occurrence denominator changed")
    return result


def build_package_comparisons(source_receipt: dict[str, Any]) -> list[dict[str, Any]]:
    packages = {
        row["logicalPackage"]: row for row in source_receipt["sourcePackages"]
        if row["logicalPackage"] in CURRENT_PACKAGE_IDENTITIES
    }
    require(set(packages) == set(CURRENT_PACKAGE_IDENTITIES),
            "source receipt package comparison denominator changed")
    result = []
    for logical in sorted(packages):
        source = packages[logical]
        current = CURRENT_PACKAGE_IDENTITIES[logical]
        same_revision = (
            source["sourcePackageBytes"] == current["bytes"]
            and source["sourcePackageSha256"] == current["sha256"]
        )
        source_copy_count = current["sourceArchiveExactCopyCount"]
        result.append({
            "logicalPackage": logical,
            "physicalPackage": source["physicalPackage"],
            "sourceEra": {
                "bytes": source["sourcePackageBytes"],
                "sha256": source["sourcePackageSha256"],
            },
            "currentInstalled": {
                "bytes": current["bytes"],
                "sha256": current["sha256"],
            },
            "sameRevision": same_revision,
            "sourceArchiveExactCopyCount": source_copy_count,
            "decision": (
                "SOURCE_PACKAGE_BYTES_AVAILABLE_BUT_NATIVE_PROVIDER_ABSENT"
                if source_copy_count > 0
                else "SOURCE_PACKAGE_BYTES_MISSING_AND_NATIVE_PROVIDER_ABSENT"
            ),
        })
    require(sum(row["sameRevision"] for row in result) == 2,
            "source/current package identity split changed")
    require(sum(row["sourceArchiveExactCopyCount"] == 0 for row in result) == 1,
            "missing source package denominator changed")
    return result


def audited_search_roots() -> list[dict[str, Any]]:
    """Return frozen observations from the 2026-08-10 read-only acquisition audit."""
    return [
        {
            "auditId": "local.users.exact-target-recursive.v1",
            "scope": "C:/Users/user",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "Exact native/script target names absent outside current install; Desktop backups included.",
        },
        {
            "auditId": "local.program-files.exact-target-recursive.v1",
            "scope": "C:/Program Files; C:/Program Files (x86)",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "No source-era EFEngine/LOSTARK/Engine/EFGame target.",
        },
        {
            "auditId": "local.program-data-current-install.v1",
            "scope": "C:/ProgramData/Smilegate/Games/LOSTARK",
            "status": "CURRENT_REVISION_ONLY",
            "providerCount": 0,
            "detail": "cacheii/STOVE data and LPK set expose current identities only; no historical delta, old or backup target.",
        },
        {
            "auditId": "local.desktop-archives.v1",
            "scope": "Desktop backup/resource roots",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "36 ZIP central directories and 1,813 archived UPKs inspected; no target native/script provider; 7z/rar count 0.",
        },
        {
            "auditId": "local.recycle-bin.v1",
            "scope": "Recycle Bin exact-target scan",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "Exact target count 0.",
        },
        {
            "auditId": "local.file-history.v1",
            "scope": "Windows FileHistory",
            "status": "ABSENT",
            "providerCount": 0,
            "detail": "FileHistory root absent.",
        },
        {
            "auditId": "local.windows-old.v1",
            "scope": "C:/Windows.old",
            "status": "ABSENT",
            "providerCount": 0,
            "detail": "Windows.old root absent.",
        },
        {
            "auditId": "local.onedrive.v1",
            "scope": "OneDrive exact-target scan",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "Documents/attachments only; exact target count 0.",
        },
        {
            "auditId": "local.vss.v1",
            "scope": "Win32_ShadowCopy and vssadmin inventory",
            "status": "PERMISSION_UNCHECKED",
            "providerCount": None,
            "detail": "Non-admin token cannot enumerate VSS. No elevation, mount, recovery or write attempted.",
        },
        {
            "auditId": "local.usn-visible-journal.v1",
            "scope": "Accessible C: USN journal",
            "status": "EXHAUSTED_VISIBLE_SCOPE",
            "providerCount": 0,
            "detail": "Recent exact target identity hits 0; this is not a VSS substitute.",
        },
        {
            "auditId": "git.reachable-and-bundle.v1",
            "scope": "all local refs/history plus pre-purge bundle at 545ca00a",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "No source-era native/script/cache target path or blob.",
        },
        {
            "auditId": "git.remote-recoverables.v1",
            "scope": "origin heads/tags, Actions artifacts/cache, Releases, user container packages",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "79 heads/tags; Actions artifacts 0, Actions cache 0, Releases 0, user container packages none.",
        },
        {
            "auditId": "git.local-lfs-content-store.v1",
            "scope": "1,546 LFS objects / 728,803,042 bytes",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "PE 21, UPK 0, game-native 0; 19,818,496-byte b367 candidate is Assimp v6.0.4.",
        },
        {
            "auditId": "git.unreachable-objects.v1",
            "scope": "all unreachable objects including non-JSON objects over 1 MiB",
            "status": "EXHAUSTED",
            "providerCount": 0,
            "detail": "All 6,235 refs-outside blobs closed: the only UE3 UPK is a cooked map without ShaderCache/Material/MIC/ParticleSystem exports; the 5,980 sub-1MiB blobs contain no PE/UPK/ZIP; SQLite/mapplacement/opaque buffers provide no native executor.",
        },
        {
            "auditId": "runtime.current-callable-surface.v1",
            "scope": "installed EFEngine.dll and LOSTARK.exe exports plus safe standalone fixture paths",
            "status": "EXHAUSTED_SAFE_CONTROLLED_SCOPE",
            "providerCount": 0,
            "detail": "Seeded wrappers are addressable but require live UObject/emitter/world/distribution graph; EF custom and evaluator exact exports are absent; live injection/hooking is neither independent nor source-era safe evidence.",
        },
    ]


def build_receipt(
    root: Path,
    source_execution: dict[str, Any],
    custom_handler: dict[str, Any],
    source_receipt: dict[str, Any],
) -> dict[str, Any]:
    validate_source_execution_receipt(source_execution)
    validate_custom_handler_receipt(custom_handler, source_execution)
    require(source_execution.get("receiptSha256") == SOURCE_EXECUTION_SELF_SHA256,
            "frozen Source execution identity changed")
    require(custom_handler.get("receiptSha256") == CUSTOM_HANDLER_SELF_SHA256,
            "frozen custom-handler identity changed")
    require(sha256_file(root / SOURCE_EXECUTION_PATH) == SOURCE_EXECUTION_RAW_SHA256,
            "Source execution raw identity changed")
    require(sha256_file(root / CUSTOM_HANDLER_PATH) == CUSTOM_HANDLER_RAW_SHA256,
            "custom-handler raw identity changed")
    require(sha256_file(root / SOURCE_RECEIPT_PATH) == SOURCE_RECEIPT_RAW_SHA256,
            "Source receipt raw identity changed")
    require(source_receipt.get("skillId") == 31470,
            "Source receipt skill identity changed")

    class_rows = build_class_rows(custom_handler)
    native_clusters = build_native_family_clusters(class_rows)
    search_roots = audited_search_roots()
    vss_rows = [row for row in search_roots if row["status"] == "PERMISSION_UNCHECKED"]
    require(len(vss_rows) == 1 and vss_rows[0]["auditId"] == "local.vss.v1",
            "VSS permission boundary changed")

    generator_path = Path(__file__).resolve()
    result = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "canonicalPlanCommit": CANONICAL_PLAN_COMMIT,
        "scope": "SOURCE_ACTUAL_OUTPUT_PROVIDER_ACQUISITION_ONLY",
        "repoArtifactInputs": [
            artifact_identity(root, SOURCE_EXECUTION_PATH, source_execution),
            artifact_identity(root, CUSTOM_HANDLER_PATH, custom_handler),
            artifact_identity(root, SOURCE_RECEIPT_PATH, source_receipt),
        ],
        "toolIdentity": {
            "path": str(generator_path.relative_to(root)).replace("\\", "/"),
            "canonicalTextSha256": canonical_text_sha256(generator_path),
        },
        "searchContract": {
            "mode": "READ_ONLY",
            "targetIdentityNames": [
                "EFEngine.dll", "LOSTARK.exe", "Engine.u", "EFGame.u",
                "ShaderCache", "source-era target UPKs",
            ],
            "sourceEraCut": "2026-08-03 Artist F extraction revision or an identity-proven matching revision",
            "valueInferenceAllowed": False,
            "imageOrScreenshotOracleAllowed": False,
            "currentDigestOrWrapperDataflowMayPromoteReady": False,
        },
        "auditedSearchRoots": search_roots,
        "sourcePackageRevisionComparisons": build_package_comparisons(source_receipt),
        "rejectedUnreachableCandidate": {
            "gitBlob": "7d19e7036aee09d6502f6f1527425aa3d52853ff",
            "bytes": 5303178,
            "rawSha256": "f130d3d0b4a048832885cc0d13eb8a88b7d6eb0b1a46dba6181c4d1cd3c4239f",
            "ue3Identity": {
                "magic": "c1832a9e",
                "version": 868,
                "licenseeVersion": 16,
                "engineVersion": 12097,
                "nameCount": 707,
                "exportCount": 1871,
                "importCount": 628,
                "game": "80004E",
            },
            "classification": "COOKED_LEVEL_MAP_INSTANCE_DATA",
            "shaderMaterialParticleSystemExportCount": 0,
            "decision": "REJECTED_NOT_A_NATIVE_OR_SCRIPT_OUTPUT_PROVIDER",
        },
        "unreachableObjectClosure": {
            "refsOutsideBlobCount": 6235,
            "subOneMiB": {
                "blobCount": 5980,
                "totalBytes": 379505715,
                "magicCounts": {
                    "LFS_POINTER": 1232,
                    "OTHER": 4748,
                    "PE": 0,
                    "UE3_UPK": 0,
                    "ZIP": 0,
                },
                "artifactNameHitCount": 10,
                "artifactNameHitDecision": "EXISTING_SCRIPTS_RECEIPTS_AND_INVENTORY_ONLY",
            },
            "pointerTreeMapping": {
                "mappedEntryCount": 798,
                "extensionCounts": {
                    "DDS": 704,
                    "PNG": 58,
                    "mapassets": 17,
                    "mapplacements": 15,
                    "TGA": 2,
                    "mapset": 1,
                    "EXE": 1,
                },
                "soleExecutableName": "ModelAssetConverter.exe",
                "soleExecutableDecision": "PROJECT_TOOL_NOT_GAME_NATIVE_PROVIDER",
            },
            "unmappedPointers": {
                "pointerCount": 757,
                "targetBytes": 137947876,
                "allTargetsPresentInLocalLfsStore": True,
                "magicCounts": {"OTHER": 757, "PE": 0, "UE3_UPK": 0, "ZIP": 0},
                "maximumTargetBytes": 4945376,
            },
            "decision": "EXHAUSTED_NO_SOURCE_NATIVE_SCRIPT_OR_CACHE_PROVIDER",
        },
        "currentCallableSurface": {
            "currentNativeBinaries": copy.deepcopy(custom_handler["currentInstalledNativeBinaries"]),
            "standardSeeded": {
                "exactWrapperFamilyCount": 7,
                "moduleOccurrenceCount": 11,
                "requiresLiveEngineObjectGraph": True,
                "safeStandaloneFixtureCount": 0,
                "actualMutatedOutputPilotCount": 0,
                "fidelity": "CURRENT_REVISION_ONLY",
            },
            "efCustom": {
                "exactExportedEntryCount": 0,
                "moduleOccurrenceCount": 15,
                "actualMutatedOutputPilotCount": 0,
            },
            "customDistribution": {
                "exactExportedGetValueEntryCount": 0,
                "moduleOccurrenceCount": 3,
                "actualNumericOutputPilotCount": 0,
            },
            "liveGameInjection": {
                "attempted": False,
                "acceptedAsIndependentOracle": False,
                "decision": "REJECTED_UNSAFE_NONINDEPENDENT_AND_CURRENT_REVISION_ONLY",
            },
        },
        "sourceBlockerClassRows": class_rows,
        "nativeFamilyClusters": native_clusters,
        "fidelityBranches": {
            "sourceExact": {
                "decision": "BLOCKED_PROVIDER_NOT_ACQUIRED",
                "readyModuleOccurrenceCount": 0,
                "blockedModuleOccurrenceCount": 29,
                "explicitApprovalRequiredToRelax": True,
            },
            "reconstructedNumericallyVerifiedMaximum": {
                "decision": "NOT_STARTED_APPROVAL_REQUIRED",
                "approved": False,
                "sourceExactClaimAllowed": False,
                "productAdmissionAllowed": False,
                "minimumEvidence": "INDEPENDENT_SAME_INPUT_ACTUAL_MUTATED_OUTPUT_FOR_EACH_NATIVE_FAMILY",
            },
        },
        "externalArtifactIntakeContract": {
            "acceptableAlternatives": [
                {
                    "alternativeId": "PAIRED_SOURCE_ERA_RUNTIME",
                    "requiredArtifacts": [
                        "EFEngine.dll", "LOSTARK.exe", "logical Engine.u",
                        "logical EFGame.u", "all exact target UPKs for one revision",
                    ],
                    "sameRevisionProofRequired": True,
                },
                {
                    "alternativeId": "AUTHENTICATED_SOURCE_ERA_NUMERIC_CAPTURE",
                    "requiredArtifacts": [
                        "binary and package identity manifest",
                        "fixed fixture inputs",
                        "pre/post full numeric particle or component state",
                        "native family/occurrence mapping",
                    ],
                    "sameRevisionProofRequired": True,
                },
            ],
            "identityFieldsRequired": [
                "raw bytes", "byte length", "SHA-256", "file/product version",
                "timestamp", "Authenticode signer/status when applicable",
                "logical-to-physical package map", "single-revision manifest",
            ],
            "fixtureFieldsRequired": [
                "29 occurrence mapping to 15 exact classes and 7 native families",
                "fixed seed/random stream", "fixed emitter/relative time",
                "fixed world query and parameter state", "pre-state",
                "expected mutated numeric post-state", "absolute/relative tolerance",
            ],
            "rejectedEvidence": [
                "current-only binary or package identity",
                "input digest parity",
                "wrapper dataflow without mutated output",
                "class name, suffix or inheritance",
                "image or screenshot comparison",
                "unattributed numeric values",
            ],
        },
        "blockerDelta": {
            "beforeBlockedModuleOccurrenceCount": 29,
            "afterBlockedModuleOccurrenceCount": 29,
            "resolvedModuleOccurrenceCount": 0,
            "beforeActualOutputOracleCount": 0,
            "afterActualOutputOracleCount": 0,
            "ownerlessBlockerCount": 0,
        },
        "summary": {
            "sourceBlockerModuleOccurrenceCount": 29,
            "sourceBlockerExactClassCount": 15,
            "nativeFamilyClusterCount": 7,
            "sourceEraActualOutputProviderCount": 0,
            "actualMutatedOutputPilotCount": 0,
            "sourceExactReadyModuleOccurrenceCount": 0,
            "remainingBlockedModuleOccurrenceCount": 29,
            "accessibleSearchRootAuditCount": len(search_roots) - 1,
            "permissionUncheckedRootCount": 1,
            "ownerlessBlockerCount": 0,
            "evidenceAcquisitionDecision": "PASS_ACCESSIBLE_SCOPE_EXHAUSTED",
            "executionReadinessDecision": "BLOCKED",
            "nextStageDecision": "NO_GO",
            "productAdmissionCount": 0,
        },
        "nextStageAdmission": {
            "allowed": False,
            "decision": "NO_GO_SOURCE_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED",
            "allowedWork": [
                "EXTERNAL_ARTIFACT_INTAKE",
                "VSS_READ_ONLY_INVENTORY_IF_ADMINISTRATIVELY_AUTHORIZED",
                "RECONSTRUCTION_BRANCH_ONLY_AFTER_EXPLICIT_USER_APPROVAL",
            ],
            "forbiddenWork": [
                "FINAL_MATERIALIZER",
                "PLAYBACK",
                "RENDERER",
                "PRODUCT_ADMISSION",
            ],
        },
        "productAdmission": {
            "allowed": False,
            "blockers": [
                "SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED",
                "SOURCE_29_EXECUTION_READINESS_BLOCKED",
                "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED",
            ],
        },
    }
    result["receiptSha256"] = canonical_sha256(result)
    validate_receipt(result, source_execution, custom_handler, source_receipt, root,
                     compare_rebuilt=False)
    return result


def validate_receipt(
    receipt: dict[str, Any],
    source_execution: dict[str, Any],
    custom_handler: dict[str, Any],
    source_receipt: dict[str, Any],
    root: Path,
    *,
    compare_rebuilt: bool = True,
) -> None:
    require(receipt.get("schema") == SCHEMA, "acquisition schema changed")
    require(type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == 1,
            "acquisition version changed")
    unsigned = copy.deepcopy(receipt)
    expected_self_hash = str(unsigned.pop("receiptSha256", ""))
    require(expected_self_hash and canonical_sha256(unsigned) == expected_self_hash,
            "acquisition self hash changed")
    require(receipt.get("characterClass") == "ARTIST"
            and receipt.get("skillId") == 31470
            and receipt.get("inputSlot") == "F",
            "acquisition root identity changed")
    summary = receipt.get("summary") or {}
    require(summary == {
        "sourceBlockerModuleOccurrenceCount": 29,
        "sourceBlockerExactClassCount": 15,
        "nativeFamilyClusterCount": 7,
        "sourceEraActualOutputProviderCount": 0,
        "actualMutatedOutputPilotCount": 0,
        "sourceExactReadyModuleOccurrenceCount": 0,
        "remainingBlockedModuleOccurrenceCount": 29,
        "accessibleSearchRootAuditCount": 14,
        "permissionUncheckedRootCount": 1,
        "ownerlessBlockerCount": 0,
        "evidenceAcquisitionDecision": "PASS_ACCESSIBLE_SCOPE_EXHAUSTED",
        "executionReadinessDecision": "BLOCKED",
        "nextStageDecision": "NO_GO",
        "productAdmissionCount": 0,
    }, "acquisition summary changed")
    require(receipt.get("blockerDelta") == {
        "beforeBlockedModuleOccurrenceCount": 29,
        "afterBlockedModuleOccurrenceCount": 29,
        "resolvedModuleOccurrenceCount": 0,
        "beforeActualOutputOracleCount": 0,
        "afterActualOutputOracleCount": 0,
        "ownerlessBlockerCount": 0,
    }, "acquisition blocker delta changed")
    require(receipt.get("nextStageAdmission", {}).get("allowed") is False
            and receipt["nextStageAdmission"].get("decision")
            == "NO_GO_SOURCE_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED",
            "acquisition next-stage gate changed")
    require(receipt.get("productAdmission", {}).get("allowed") is False,
            "acquisition granted Product admission")
    vss = [row for row in receipt.get("auditedSearchRoots", [])
           if row.get("auditId") == "local.vss.v1"]
    require(len(vss) == 1 and vss[0].get("status") == "PERMISSION_UNCHECKED"
            and vss[0].get("providerCount") is None,
            "VSS was laundered into an exhausted root")
    require(len(receipt.get("sourceBlockerClassRows", [])) == 15
            and sum(row.get("moduleOccurrenceCount", 0)
                    for row in receipt["sourceBlockerClassRows"]) == 29
            and all(row.get("sourceEraProviderId") is None
                    and row.get("actualOutputOracleCount") == 0
                    and row.get("decision")
                    == "BLOCKED_NO_SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER"
                    for row in receipt["sourceBlockerClassRows"]),
            "Source blocker class rows were promoted or changed")
    require(len(receipt.get("nativeFamilyClusters", [])) == 7
            and sum(row.get("moduleOccurrenceCount", 0)
                    for row in receipt["nativeFamilyClusters"]) == 29
            and all(row.get("sourceEraProviderId") is None
                    and row.get("standaloneActualOutputPilotCount") == 0
                    and row.get("sourceExactDecision") == "BLOCKED"
                    and row.get("reconstructionCandidate", {}).get("explicitUserApprovalRequired")
                    is True
                    for row in receipt["nativeFamilyClusters"]),
            "native family acquisition gate changed")
    branches = receipt.get("fidelityBranches") or {}
    require(branches.get("sourceExact", {}).get("decision")
            == "BLOCKED_PROVIDER_NOT_ACQUIRED"
            and branches.get("reconstructedNumericallyVerifiedMaximum", {}).get("approved")
            is False
            and branches["reconstructedNumericallyVerifiedMaximum"].get(
                "productAdmissionAllowed") is False,
            "fidelity branch approval boundary changed")
    intake = receipt.get("externalArtifactIntakeContract") or {}
    require([row.get("alternativeId") for row in intake.get("acceptableAlternatives", [])]
            == ["PAIRED_SOURCE_ERA_RUNTIME", "AUTHENTICATED_SOURCE_ERA_NUMERIC_CAPTURE"]
            and "current-only binary or package identity" in intake.get("rejectedEvidence", [])
            and "expected mutated numeric post-state" in intake.get("fixtureFieldsRequired", []),
            "external artifact intake contract changed")
    if compare_rebuilt:
        expected = build_receipt(root, source_execution, custom_handler, source_receipt)
        require(json_bytes(receipt) == json_bytes(expected),
                "acquisition receipt differs from deterministic reconstruction")


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-execution", type=Path,
                        default=root / SOURCE_EXECUTION_PATH)
    parser.add_argument("--custom-handler", type=Path,
                        default=root / CUSTOM_HANDLER_PATH)
    parser.add_argument("--source-receipt", type=Path,
                        default=root / SOURCE_RECEIPT_PATH)
    parser.add_argument("--output", type=Path, default=root / OUTPUT_PATH)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    source_execution = load_strict_json_object(args.source_execution)
    custom_handler = load_strict_json_object(args.custom_handler)
    source_receipt = load_strict_json_object(args.source_receipt)
    result = build_receipt(root, source_execution, custom_handler, source_receipt)
    if args.check:
        current = load_strict_json_object(args.output)
        validate_receipt(current, source_execution, custom_handler, source_receipt, root)
        require(json_bytes(current) == json_bytes(result), "acquisition receipt is stale")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_suffix(args.output.suffix + ".tmp")
        temporary.write_bytes(json_bytes(result))
        temporary.replace(args.output)
    print(
        "Artist F Source oracle acquisition: "
        f"classes={result['summary']['sourceBlockerExactClassCount']} "
        f"families={result['summary']['nativeFamilyClusterCount']} "
        f"blocked={result['summary']['remainingBlockedModuleOccurrenceCount']} "
        f"providers={result['summary']['sourceEraActualOutputProviderCount']} "
        "nextStage=NO_GO product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F Source oracle acquisition failed: {error}", file=sys.stderr)
        raise SystemExit(1)
