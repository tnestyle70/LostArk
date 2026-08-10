#!/usr/bin/env python3
"""Independent verifier for the Artist F seeded/custom handler oracle."""

from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import Any

from effect_source_contract_io import load_strict_json_object
from extract_ue3_effect_material_closure import find_export, load_package
from extract_ue3_placements import LOSTARK_KR_AES_KEY, package_ref_name, package_ref_path
from verify_artist_31470_source_execution_semantics import (
    canonical_sha256,
    canonical_text_sha256,
    evaluate,
    verify_receipt as verify_source_execution_receipt,
)


SCHEMA = "lostark.effect-custom-handler-oracle"
SOURCE_SHA = "7e1113dd05bcc9b51056cacc27da1805f7a6d26f65dda5b72c99d26c3141a71c"
HANDLER_BLOCKER = "EXACT_SOURCE_CLASS_HANDLER_NUMERIC_ORACLE_REQUIRED"
DISTRIBUTION_BLOCKER = "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN"
SOURCE_ERA_HANDLER_BLOCKER = "SOURCE_ERA_NATIVE_HANDLER_IDENTITY_UNPINNED"
ACTUAL_OUTPUT_BLOCKER = "EXACT_NATIVE_PARTICLE_OUTPUT_ORACLE_REQUIRED"
SOURCE_ERA_EVALUATOR_BLOCKER = (
    "SOURCE_ERA_DISTRIBUTION_EVALUATOR_IDENTITY_UNPINNED"
)
ACTUAL_DISTRIBUTION_OUTPUT_BLOCKER = (
    "EXACT_NATIVE_DISTRIBUTION_OUTPUT_ORACLE_REQUIRED"
)
PRODUCT_BLOCKER = "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED"
STREAM_ALGORITHM = "UE3_LCG_196314165_907633515_MANTISSA23_V1"
ORACLE_OCCURRENCE_SEED = 0x13579BDF
SAMPLE_TIMES = (0.0, 0.25, 1.0)

FEASIBILITY_MODULE_ROW_FIELDS = {
    "moduleOccurrenceId",
    "exactSourceClass",
    "family",
    "requiredRuntimeOutputs",
    "sourceEraPackageOrBinaryIdentity",
    "currentRevisionEvidenceIdentity",
    "nativeEntryOrDispatchIdentity",
    "numericOracleInputDomain",
    "numericOracleExpectedOutput",
    "independentOracleImplementation",
    "oracleProvider",
    "pilotFixtureIds",
    "pilotExpectedMutatedOutputs",
    "numericTolerance",
    "pilotDecision",
    "fidelityDecision",
    "executionDecision",
    "owner",
    "remainingBlockers",
}

STANDARD = (
    ("particlemodulecolor_seeded", "particlemodulecolor", "Color", 2),
    ("particlemodulelifetime_seeded", "particlemodulelifetime", "Lifetime", 1),
    ("particlemodulelocation_seeded", "particlemodulelocation", "Location", 2),
    (
        "particlemodulelocationprimitivecylinder_seeded",
        "particlemodulelocationprimitivecylinder",
        "LocationPrimitiveCylinder",
        3,
    ),
    ("particlemodulemeshrotation_seeded", "particlemodulemeshrotation", "MeshRotation", 1),
    ("particlemodulesize_seeded", "particlemodulesize", "Size", 1),
    ("particlemodulevelocity_seeded", "particlemodulevelocity", "Velocity", 1),
)

CUSTOM = (
    ("efparticlemodulelocationonground", 2),
    ("efparticlemodulelocationprimitivecylinderspin", 2),
    ("efparticlemodulelocationprimitivecylinderspin_seeded", 3),
    ("efparticlemoduletypedatadecal", 3),
    ("efparticlemoduletypedatalight", 1),
    ("efparticlemodulevelocityoverlifetime", 4),
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def capability(exact_class: str) -> str:
    return "source.module.exact." + exact_class.replace("_", ".") + ".v1"


def current_evidence_id(exact_class: str) -> str:
    return "source.module.current-alias-evidence." + exact_class.replace("_", ".") + ".v1"


def module_rows(source: dict[str, Any]) -> list[dict[str, Any]]:
    return [module for occurrence in source["occurrences"] for module in occurrence["modules"]]


def random_units(seed: int, offset: int) -> list[float]:
    require(type(seed) is int and type(offset) is int and offset >= 0, "stream input invalid")
    state = seed & 0xFFFFFFFF
    values: list[float] = []
    for index in range(offset + 4):
        state = (state * 196314165 + 907633515) & 0xFFFFFFFF
        if index >= offset:
            bits = 0x3F800000 | (state & 0x007FFFFF)
            values.append(struct.unpack("<f", struct.pack("<I", bits))[0] - 1.0)
    return values


def verify_diagnostic_inputs(module: dict[str, Any], occurrence: dict[str, Any]) -> None:
    seeds = module["seed"]["randomSeeds"]
    expected_seed = seeds[0] if seeds else ORACLE_OCCURRENCE_SEED
    expected_seed_source = (
        "SOURCE_DECODED_FIRST_RANDOM_SEED"
        if seeds else "FIXED_ORACLE_OCCURRENCE_STREAM_SEED_NOT_SOURCE_VALUE"
    )
    samples = occurrence.get("diagnosticFixedSeedInputs") or []
    require(len(samples) == 3, "diagnostic input denominator changed")
    for sample_index, sample in enumerate(samples):
        offset = sample_index * 4
        units = random_units(expected_seed, offset)
        require(
            sample.get("sampleId")
            == f"{module['moduleOccurrenceId']}::sample:{sample_index:03d}"
            and sample.get("payloadSha256") == module["typedPayload"]["payloadSha256"]
            and sample.get("emitterTime") == SAMPLE_TIMES[sample_index]
            and sample.get("fixedSeed") == expected_seed
            and sample.get("fixedSeedSource") == expected_seed_source
            and sample.get("randomStreamAlgorithm") == STREAM_ALGORITHM
            and sample.get("randomStreamDrawOffset") == offset
            and sample.get("randomUnits") == units
            and sample.get("randomStreamContract")
            == "CURRENT_WRAPPER_FIXED_SEED_INPUT_DIAGNOSTIC_ONLY"
            and sample.get("role")
            == "DIAGNOSTIC_TYPED_INPUT_NOT_NATIVE_PARTICLE_OUTPUT_ORACLE",
            "fixed-seed numeric input changed",
        )
        expected_values = []
        for distribution in module["typedPayload"]["distributions"]:
            descriptor = distribution["descriptor"]
            require(descriptor.get("payloadStatus") == "INLINE_SOURCE_PAYLOAD",
                    "stock seeded distribution is not inline")
            expected_values.append({
                "payloadDistributionId": distribution["payloadDistributionId"],
                "propertyPath": descriptor["propertyPath"],
                "value": evaluate(descriptor, SAMPLE_TIMES[sample_index], units),
            })
        require(sample.get("evaluatedDistributions") == expected_values,
                "fixed-seed distribution evaluation changed")
        handler_input = {
            name: copy.deepcopy(sample[name]) for name in (
                "payloadSha256", "emitterTime", "fixedSeed", "fixedSeedSource",
                "randomStreamAlgorithm", "randomStreamDrawOffset", "randomUnits",
                "evaluatedDistributions",
            )
        }
        digest = canonical_sha256(handler_input)
        require(sample.get("typedInputSha256") == digest,
                "diagnostic typed input digest changed")


def verify_shallow(root: Path, source: dict[str, Any], receipt: dict[str, Any]) -> dict[str, int]:
    require(receipt.get("schema") == SCHEMA, "oracle schema changed")
    require(type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == 2,
            "oracle version changed")
    unsigned = copy.deepcopy(receipt)
    stored_hash = str(unsigned.pop("receiptSha256", ""))
    require(stored_hash and canonical_sha256(unsigned) == stored_hash, "oracle self hash changed")
    require(receipt.get("characterClass") == "ARTIST"
            and receipt.get("skillId") == 31470
            and receipt.get("inputSlot") == "F",
            "oracle root identity changed")
    require(source.get("receiptSha256") == SOURCE_SHA
            and receipt.get("sourceExecutionReceipt", {}).get("receiptSha256") == SOURCE_SHA
            and receipt["sourceExecutionReceipt"].get("canonicalJsonSha256")
            == canonical_sha256(source),
            "source execution join changed")
    tool = receipt.get("toolIdentity") or {}
    tool_path = root / str(tool.get("path", ""))
    require(tool_path.is_file()
            and tool.get("canonicalTextSha256") == canonical_text_sha256(tool_path),
            "oracle tool identity changed")

    modules = module_rows(source)
    require(len(modules) == 399 and len({row["moduleOccurrenceId"] for row in modules}) == 399,
            "source module denominator changed")
    source_by_id = {row["moduleOccurrenceId"]: row for row in modules}
    blocked_modules = [row for row in modules if row["decision"] == "BLOCKED"]
    blocked_distributions = [
        distribution for module in modules for distribution in module["distributionAdapters"]
        if distribution["decision"] == "BLOCKED"
    ]
    require(len(blocked_modules) == 29 and len(blocked_distributions) == 3,
            "source blocker denominator changed")

    standard = receipt.get("standardSeededHandlers") or []
    require([row.get("exactSourceClass") for row in standard]
            == [row[0] for row in STANDARD], "standard class order changed")
    standard_ids: set[str] = set()
    for (exact, base, _, count), row in zip(STANDARD, standard):
        expected_modules = [module for module in modules if module["exactSourceClass"] == exact]
        require(len(expected_modules) == count
                and row.get("baseSourceClass") == base
                and row.get("handlerEvidenceId") == current_evidence_id(exact)
                and row.get("candidateHandlerCapabilityId") == capability(exact)
                and row.get("baseHandlerCapabilityId") == capability(base)
                and row.get("decision") == "BLOCKED_CURRENT_REVISION_ALIAS_EVIDENCE_ONLY"
                and row.get("normalizedStringAliasAllowed") is False
                and row.get("blockers") == [
                    HANDLER_BLOCKER, ACTUAL_OUTPUT_BLOCKER, SOURCE_ERA_HANDLER_BLOCKER,
                ]
                and row.get("actualNativeParticleOutputOracle", {}).get("decision")
                == "UNAVAILABLE"
                and row.get("actualNativeParticleOutputOracle", {}).get("sampleCount") == 0
                and row.get("actualNativeParticleOutputOracle", {}).get(
                    "inputDigestParityAccepted"
                ) is False,
                "standard current evidence was incorrectly admitted")
        dispatch = row.get("currentNativeDispatchEvidence") or {}
        require(dispatch.get("decision")
                == "CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE"
                and dispatch.get("randomStreamFifthArgumentDataflow", {}).get("decision")
                == "CURRENT_NATIVE_RAX_TO_FIFTH_ARGUMENT_DATAFLOW_VERIFIED"
                and dispatch["randomStreamFifthArgumentDataflow"].get(
                    "storeImmediatelyPrecedesSpawnExDispatch"
                ) is True
                and dispatch["randomStreamFifthArgumentDataflow"].get(
                    "spawnExDispatchOffset"
                ) == dispatch.get("dispatchOffset"),
                "standard current wrapper evidence changed")
        script = row.get("currentScriptClassEvidence") or {}
        require(script.get("sourceEraIdentityPinned") is False
                and script.get("uFunctionChildCount") == 0
                and len(script.get("directChildren") or []) == 1
                and script["directChildren"][0].get("name") == "randomseedinfo"
                and script["directChildren"][0].get("className") == "structproperty",
                "standard current script child chain changed")
        occurrences = row.get("occurrences") or []
        require(len(occurrences) == count, "standard occurrence count changed")
        by_id = {occurrence["moduleOccurrenceId"]: occurrence for occurrence in occurrences}
        require(len(by_id) == count, "standard occurrence identity duplicated")
        for module in expected_modules:
            occurrence = by_id.get(module["moduleOccurrenceId"])
            require(occurrence is not None
                    and occurrence.get("sourceObjectId") == module["sourceObjectId"]
                    and occurrence.get("sourceRecordSha256") == module["sourceRecordSha256"]
                    and occurrence.get("payloadSha256") == module["typedPayload"]["payloadSha256"]
                    and occurrence.get("seedEvidence") == module["seed"],
                    "standard occurrence source join changed")
            verify_diagnostic_inputs(module, occurrence)
            standard_ids.add(module["moduleOccurrenceId"])
    require(len(standard_ids) == 11, "standard blocker coverage changed")
    require(receipt.get("capabilityGrants") == [],
            "current-revision evidence created a capability grant")

    custom = receipt.get("blockedCustomModuleHandlers") or []
    require([row.get("exactSourceClass") for row in custom]
            == [row[0] for row in CUSTOM], "custom class order changed")
    custom_ids: set[str] = set()
    for (exact, count), row in zip(CUSTOM, custom):
        expected_ids = [
            module["moduleOccurrenceId"] for module in modules
            if module["exactSourceClass"] == exact
        ]
        require(len(expected_ids) == count
                and row.get("occurrenceIds") == expected_ids
                and row.get("handlerCapabilityId") == capability(exact)
                and row.get("decision") == "BLOCKED"
                and row.get("blockers") == [HANDLER_BLOCKER]
                and row.get("installedNativeExportMatches") == [],
                "custom handler blocker changed")
        custom_ids.update(expected_ids)
    require(len(custom_ids) == 15, "custom handler blocker coverage changed")

    distribution = receipt.get("blockedCustomDistributionEvaluator") or {}
    evaluator_id = "source.distribution.exact.efdistributionvectormultiplyparticleparameter.v1"
    require(distribution.get("exactSourceClass")
            == "efdistributionvectormultiplyparticleparameter"
            and distribution.get("evaluatorCapabilityId") == evaluator_id
            and distribution.get("decision") == "BLOCKED"
            and distribution.get("blockers") == [DISTRIBUTION_BLOCKER]
            and distribution.get("installedNativeExportMatches") == [],
            "custom distribution blocker changed")
    expected_distribution_ids = [row["distributionId"] for row in blocked_distributions]
    require([row.get("distributionId") for row in distribution.get("occurrences") or []]
            == expected_distribution_ids, "custom distribution occurrence join changed")

    module_owners = receipt.get("moduleBlockerOwnership") or []
    require(len(module_owners) == 29
            and {row.get("moduleOccurrenceId") for row in module_owners}
            == {row["moduleOccurrenceId"] for row in blocked_modules},
            "module blocker owner coverage changed")
    remaining = 0
    for owner in module_owners:
        module = source_by_id[owner["moduleOccurrenceId"]]
        require(owner.get("exactSourceClass") == module["exactSourceClass"]
                and owner.get("sourceRecordSha256") == module["sourceRecordSha256"]
                and owner.get("sourceBlockers") == module["blockers"]
                and owner.get("ownerIds"), "module blocker owner join changed")
        if owner["moduleOccurrenceId"] in standard_ids:
            require(owner.get("postJoinDecision") == "BLOCKED"
                    and owner.get("ownerKind")
                    == "BLOCKED_CURRENT_REVISION_ALIAS_EVIDENCE_ONLY"
                    and owner.get("ownerIds")
                    == [current_evidence_id(module["exactSourceClass"])]
                    and owner.get("remainingBlockers") == [
                        HANDLER_BLOCKER, ACTUAL_OUTPUT_BLOCKER, SOURCE_ERA_HANDLER_BLOCKER,
                    ],
                    "current seeded evidence incorrectly resolved a module")
            remaining += 1
        elif owner["moduleOccurrenceId"] in custom_ids:
            require(owner.get("postJoinDecision") == "BLOCKED"
                    and owner.get("ownerKind") == "BLOCKED_CUSTOM_MODULE_HANDLER"
                    and owner.get("ownerIds") == [capability(module["exactSourceClass"])]
                    and owner.get("remainingBlockers") == [
                        HANDLER_BLOCKER, ACTUAL_OUTPUT_BLOCKER, SOURCE_ERA_HANDLER_BLOCKER,
                    ],
                    "custom handler owner changed")
            remaining += 1
        else:
            expected_ids = [
                row["distributionId"] for row in module["distributionAdapters"]
                if row["decision"] == "BLOCKED"
            ]
            require(owner.get("postJoinDecision") == "BLOCKED"
                    and owner.get("ownerKind") == "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR"
                    and owner.get("ownerIds") == expected_ids
                    and owner.get("remainingBlockers") == [
                        DISTRIBUTION_BLOCKER,
                        ACTUAL_OUTPUT_BLOCKER,
                        SOURCE_ERA_EVALUATOR_BLOCKER,
                    ],
                    "custom distribution module owner changed")
            remaining += 1
    require(remaining == 29, "post-join module decisions changed")

    distribution_owners = receipt.get("distributionBlockerOwnership") or []
    require(len(distribution_owners) == 3
            and {row.get("distributionId") for row in distribution_owners}
            == set(expected_distribution_ids),
            "distribution blocker owner coverage changed")
    for owner in distribution_owners:
        require(owner.get("postJoinDecision") == "BLOCKED"
                and owner.get("ownerKind") == "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR"
                and owner.get("ownerId") == evaluator_id
                and owner.get("remainingBlockers") == [
                    DISTRIBUTION_BLOCKER,
                    ACTUAL_DISTRIBUTION_OUTPUT_BLOCKER,
                    SOURCE_ERA_EVALUATOR_BLOCKER,
                ],
                "distribution blocker owner changed")

    summary = receipt.get("summary") or {}
    require(summary.get("sourceBlockedModuleCount") == 29
            and summary.get("standardSeededOccurrenceCount") == 11
            and summary.get("blockedCustomModuleOccurrenceCount") == 15
            and summary.get("blockedCustomDistributionOccurrenceCount") == 3
            and summary.get("moduleBlockerOwnerCount") == 29
            and summary.get("currentCrossRevisionAliasEvidenceCount") == 7
            and summary.get("actualNativeParticleOutputOracleCount") == 0
            and summary.get("nativeExactAliasAdmissionCount") == 0
            and summary.get("capabilityGrantCount") == 0
            and summary.get("resolvedModuleBlockerCount") == 0
            and summary.get("remainingBlockedModuleCount") == 29
            and summary.get("distributionBlockerOwnerCount") == 3
            and summary.get("ownerlessBlockerCount") == 0
            and summary.get("projectedModuleDecisionCountsAfterJoin")
            == {"READY_FOR_HANDLER": 370, "BLOCKED": 29}
            and summary.get("projectedDistributionDecisionCountsAfterJoin")
            == {"READY_FOR_HANDLER": 626, "BLOCKED": 3}
            and summary.get("productAdmissionCount") == 0
            and summary.get("silentFallbackCount") == 0,
            "oracle summary changed")
    require(receipt.get("blockerUnion")
            == [
                ACTUAL_DISTRIBUTION_OUTPUT_BLOCKER, ACTUAL_OUTPUT_BLOCKER,
                DISTRIBUTION_BLOCKER, HANDLER_BLOCKER, SOURCE_ERA_EVALUATOR_BLOCKER,
                SOURCE_ERA_HANDLER_BLOCKER, PRODUCT_BLOCKER,
            ]
            and receipt.get("productAdmission") == {
                "allowed": False,
                "blockers": [
                    ACTUAL_DISTRIBUTION_OUTPUT_BLOCKER, ACTUAL_OUTPUT_BLOCKER,
                    DISTRIBUTION_BLOCKER, HANDLER_BLOCKER, SOURCE_ERA_EVALUATOR_BLOCKER,
                    SOURCE_ERA_HANDLER_BLOCKER, PRODUCT_BLOCKER,
                ],
            }, "Product blocker boundary changed")
    matrix = receipt.get("feasibilityMatrix") or {}
    matrix_summary = matrix.get("summary") or {}
    matrix_rows = matrix.get("moduleRows") or []
    matrix_distribution_rows = matrix.get("distributionRows") or []
    require(len(matrix_rows) == 29
            and len(matrix.get("moduleFamilies") or []) == 15
            and len(matrix_distribution_rows) == 3
            and matrix_summary == {
                "moduleFamilyCount": 15,
                "moduleMatrixRowCount": 29,
                "requiredFieldCompleteRowCount": 29,
                "distributionMatrixRowCount": 3,
                "actualNativeParticleOutputOracleCount": 0,
                "actualNativeDistributionOutputOracleCount": 0,
                "pilotFixtureCount": 0,
                "feasibleModuleRowCount": 0,
                "verifiedIrrelevantModuleRowCount": 0,
                "blockedModuleRowCount": 29,
                "ownerlessBlockerCount": 0,
                "inputDigestParityAcceptedAsOutputOracleCount": 0,
                "unresolvedExecutionRowCount": 29,
                "evidenceIntegrityDecision": "FROZEN_REVIEW_REQUIRED",
                "executionReadinessDecision": "BLOCKED",
            }, "feasibility matrix summary changed")
    require({row.get("moduleOccurrenceId") for row in matrix_rows}
            == {row["moduleOccurrenceId"] for row in blocked_modules}
            and all(set(row) == FEASIBILITY_MODULE_ROW_FIELDS for row in matrix_rows),
            "feasibility matrix coverage or admission changed")
    matrix_by_id = {row["moduleOccurrenceId"]: row for row in matrix_rows}
    for module in blocked_modules:
        row = matrix_by_id[module["moduleOccurrenceId"]]
        require(
            row.get("exactSourceClass") == module["exactSourceClass"]
            and row.get("sourceEraPackageOrBinaryIdentity", {}).get("moduleSourceObjectId")
            == module["sourceObjectId"]
            and row.get("sourceEraPackageOrBinaryIdentity", {}).get(
                "moduleSourceRecordSha256"
            ) == module["sourceRecordSha256"]
            and row.get("sourceEraPackageOrBinaryIdentity", {}).get(
                "handlerOrEvaluatorBinaryIdentity"
            ) is None
            and row.get("numericOracleInputDomain", {}).get("sourceRecordSha256")
            == module["sourceRecordSha256"]
            and row.get("numericOracleInputDomain", {}).get("typedPayloadSha256")
            == module["typedPayload"]["payloadSha256"]
            and row.get("numericOracleExpectedOutput", {}).get("requiredRuntimeOutputs")
            == row.get("requiredRuntimeOutputs")
            and row.get("numericOracleExpectedOutput", {}).get("numericValues") == []
            and row.get("pilotExpectedMutatedOutputs", {}).get("requiredRuntimeOutputs")
            == row.get("requiredRuntimeOutputs")
            and row.get("pilotExpectedMutatedOutputs", {}).get("numericValues") == []
            and row.get("independentOracleImplementation", {}).get("implementationId") is None
            and row.get("oracleProvider", {}).get("providerId") is None
            and row.get("pilotFixtureIds") == []
            and row.get("numericTolerance", {}).get("absolute") is None
            and row.get("numericTolerance", {}).get("relative") is None
            and row.get("pilotDecision") == "BLOCKED"
            and row.get("executionDecision") == "BLOCKED"
            and row.get("owner", {}).get("role") == "SOURCE_SPECIALIST"
            and bool(row.get("owner", {}).get("finalCapabilityId"))
            and ACTUAL_OUTPUT_BLOCKER in row.get("remainingBlockers", []),
            "feasibility matrix row admitted unavailable actual output",
        )
    require(sum(
        row.get("fidelityDecision") == "CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE"
        for row in matrix_rows
    ) == 11, "current seeded feasibility row denominator changed")
    require({row.get("distributionId") for row in matrix_distribution_rows}
            == set(expected_distribution_ids)
            and all(row.get("oracleProvider") is None
                    and row.get("pilotFixtureIds") == []
                    and row.get("numericTolerance") is None
                    and row.get("executionDecision") == "BLOCKED"
                    and bool(row.get("owner", {}).get("finalCapabilityId"))
                    for row in matrix_distribution_rows),
            "distribution feasibility matrix admitted unavailable actual output")
    return {
        "ready": 370,
        "blocked": 29,
        "distributionReady": 626,
        "distributionBlocked": 3,
    }


def get_proc(module: Any, name: str) -> int:
    kernel32 = ctypes.windll.kernel32
    kernel32.GetProcAddress.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    kernel32.GetProcAddress.restype = ctypes.c_void_p
    address = kernel32.GetProcAddress(module._handle, name.encode("ascii"))
    require(bool(address), f"native export missing: {name}")
    return int(address)


def derive_current_dataflow(wrapper: bytes, spawn_dispatch_offset: int) -> dict[str, Any]:
    stream_call = bytes.fromhex("ff 90 a0 00 00 00")
    arg_store = bytes.fromhex("48 89 44 24 20")
    require(wrapper.count(stream_call) == 1, "deep random-stream call changed")
    require(wrapper.count(arg_store) == 1, "deep RAX argument store changed")
    call_offset = wrapper.index(stream_call)
    store_offset = wrapper.index(arg_store)
    between = wrapper[call_offset + len(stream_call):store_offset]
    require(between in {
        bytes.fromhex("44 8b c3 0f 28 de 48 8b d7 48 8b ce"),
        bytes.fromhex("4c 8b 0e 0f 28 de 44 8b c3 48 8b d7 48 8b ce"),
    }, "deep RAX-to-fifth-argument dataflow changed")
    require(store_offset + len(arg_store) == spawn_dispatch_offset,
            "deep fifth-argument store no longer precedes SpawnEx dispatch")
    return {
        "decision": "CURRENT_NATIVE_RAX_TO_FIFTH_ARGUMENT_DATAFLOW_VERIFIED",
        "streamOwnerCallOffset": call_offset,
        "streamOwnerVirtualSlotBytes": 160,
        "streamReturnRegister": "RAX",
        "instructionsBeforeStoreHex": between.hex(),
        "fifthArgumentStoreOffset": store_offset,
        "fifthArgumentStoreInstructionHex": arg_store.hex(),
        "windowsX64FifthArgumentStackOffsetBytes": 32,
        "spawnExDispatchOffset": spawn_dispatch_offset,
        "storeImmediatelyPrecedesSpawnExDispatch": True,
        "proofRole": "CURRENT_REVISION_DATAFLOW_NOT_SOURCE_ERA_OR_OUTPUT_EQUIVALENCE",
    }


def verify_deep(
    source: dict[str, Any], receipt: dict[str, Any], release_root: Path, binary_root: Path
) -> None:
    identities = receipt.get("currentInstalledNativeBinaries") or []
    require({row.get("fileName") for row in identities} == {"EFEngine.dll", "LOSTARK.exe"},
            "native identity denominator changed")
    for row in identities:
        path = binary_root / row["fileName"]
        require(path.is_file() and path.stat().st_size == row["bytes"]
                and sha256_file(path) == row["sha256"]
                and row.get("sourceEraIdentityPinned") is False,
                f"native binary identity changed: {row['fileName']}")
    for script in source["currentRevisionDefaultEvidence"]["scriptPackages"]:
        path = release_root / script["physicalPackage"]
        require(path.is_file() and path.stat().st_size == script["bytes"]
                and sha256_file(path) == script["sha256"],
                f"script package identity changed: {path.name}")
    script_rows = {
        row["logicalPackage"].casefold(): row
        for row in source["currentRevisionDefaultEvidence"]["scriptPackages"]
    }
    engine = load_package(release_root / script_rows["engine"]["physicalPackage"], LOSTARK_KR_AES_KEY)
    efgame = load_package(release_root / script_rows["efgame"]["physicalPackage"], LOSTARK_KR_AES_KEY)
    evidence_rows = [row["currentScriptClassEvidence"]
                     for row in receipt["standardSeededHandlers"]]
    evidence_rows += [row["currentScriptClassEvidence"]
                      for row in receipt["blockedCustomModuleHandlers"]]
    evidence_rows.append(receipt["blockedCustomDistributionEvaluator"]["currentScriptClassEvidence"])
    for evidence in evidence_rows:
        package = engine if evidence["className"].startswith("particlemodule") else efgame
        entry = find_export(package, evidence["className"])
        serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        children = []
        for child in package.exports:
            if child.package_index != entry.index + 1:
                continue
            children.append({
                "name": child.object_name.casefold(),
                "className": (
                    package_ref_name(child.class_index, package.imports, package.exports) or ""
                ).casefold(),
                "exportIndex": child.index,
                "serialSize": child.serial_size,
            })
        require(entry.index == evidence["exportIndex"]
                and entry.serial_size == evidence["serialSize"]
                and sha256_file_bytes(serial) == evidence["serialSha256"]
                and (package_ref_path(entry.super_index, package.imports, package.exports) or "").casefold()
                == evidence["superClass"]
                and children == evidence["directChildren"]
                and sum(row["className"] == "function" for row in children)
                == evidence["uFunctionChildCount"],
                f"script class evidence changed: {evidence['className']}")

    engine_path = binary_root / "EFEngine.dll"
    with __import__("os").add_dll_directory(str(binary_root)):
        native = ctypes.WinDLL(str(engine_path))
    base_address = int(native._handle)
    native_name_by_exact = {exact: native_name for exact, _, native_name, _ in STANDARD}
    for row in receipt["standardSeededHandlers"]:
        dispatch = row["currentNativeDispatchEvidence"]
        spawn = get_proc(native, dispatch["spawnExport"])
        spawn_ex = get_proc(native, dispatch["baseSpawnExExport"])
        wrapper = ctypes.string_at(spawn, 96)
        require(spawn - base_address == dispatch["spawnRva"]
                and spawn_ex - base_address == dispatch["baseSpawnExRva"]
                and sha256_file_bytes(wrapper) == dispatch["wrapperFirst96BytesSha256"],
                f"native wrapper bytes changed: {row['exactSourceClass']}")
        require(derive_current_dataflow(wrapper, dispatch["dispatchOffset"])
                == dispatch["randomStreamFifthArgumentDataflow"],
                "native random-stream fifth-argument dataflow evidence changed")
        offset = dispatch["dispatchOffset"]
        if dispatch["dispatchKind"] == "DIRECT_REL32_TO_EXACT_BASE_SPAWN_EX":
            require(wrapper[offset] == 0xE8
                    and spawn + offset + 5 + struct.unpack_from("<i", wrapper, offset + 1)[0]
                    == spawn_ex, "native direct dispatch target changed")
        else:
            native_name = native_name_by_exact[row["exactSourceClass"]]
            vtable = get_proc(native, f"??_7UParticleModule{native_name}_Seeded@@6B@")
            slot = dispatch["vtableSlotBytes"]
            require(ctypes.c_uint64.from_address(vtable + slot).value == spawn_ex,
                    "native vtable dispatch target changed")


def sha256_file_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--source-execution", type=Path,
        default=root / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-execution-semantics.receipt.json",
    )
    parser.add_argument(
        "--receipt", type=Path,
        default=root / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.custom-handler-oracle.receipt.json",
    )
    parser.add_argument(
        "--release-root", type=Path,
        default=Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"),
    )
    parser.add_argument(
        "--binary-root", type=Path,
        default=Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\Binaries\Win64"),
    )
    parser.add_argument("--shallow", action="store_true")
    args = parser.parse_args()
    source = load_strict_json_object(args.source_execution)
    inputs = {
        name: load_strict_json_object(root / row["path"])
        for name, row in source.get("inputs", {}).items()
    }
    verify_source_execution_receipt(
        source, root=root, inputs=inputs,
        release_root=None if args.shallow else args.release_root,
    )
    receipt = load_strict_json_object(args.receipt)
    result = verify_shallow(root, source, receipt)
    if not args.shallow:
        verify_deep(source, receipt, args.release_root, args.binary_root)
    print(
        "Artist F custom handler independent oracle: "
        f"ready={result['ready']} blocked={result['blocked']} "
        f"distributionReady={result['distributionReady']} "
        f"distributionBlocked={result['distributionBlocked']} ownerless=0 product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F custom handler oracle verification failed: {error}", file=sys.stderr)
        raise SystemExit(1)
