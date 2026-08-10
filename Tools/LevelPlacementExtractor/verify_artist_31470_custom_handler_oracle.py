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
from extract_ue3_placements import LOSTARK_KR_AES_KEY, package_ref_path
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
PRODUCT_BLOCKER = "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED"
STREAM_ALGORITHM = "UE3_LCG_196314165_907633515_MANTISSA23_V1"
ORACLE_OCCURRENCE_SEED = 0x13579BDF
SAMPLE_TIMES = (0.0, 0.25, 1.0)

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


def verify_numeric_samples(module: dict[str, Any], occurrence: dict[str, Any]) -> None:
    seeds = module["seed"]["randomSeeds"]
    expected_seed = seeds[0] if seeds else ORACLE_OCCURRENCE_SEED
    expected_seed_source = (
        "SOURCE_DECODED_FIRST_RANDOM_SEED"
        if seeds else "FIXED_ORACLE_OCCURRENCE_STREAM_SEED_NOT_SOURCE_VALUE"
    )
    samples = occurrence.get("numericOracleSamples") or []
    require(len(samples) == 3, "numeric sample denominator changed")
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
            == "FIXED_SEED_STREAM_NATIVE_WRAPPER_OWNS_FRANDOMSTREAM",
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
        require(sample.get("numericInputParity") is True
                and sample.get("baseHandlerInputSha256") == digest
                and sample.get("exactSeededHandlerInputSha256") == digest,
                "base/seeded numeric parity changed")


def verify_shallow(root: Path, source: dict[str, Any], receipt: dict[str, Any]) -> dict[str, int]:
    require(receipt.get("schema") == SCHEMA, "oracle schema changed")
    require(type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == 1,
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
                and row.get("handlerCapabilityId") == capability(exact)
                and row.get("baseHandlerCapabilityId") == capability(base)
                and row.get("decision") == "READY_FOR_EXACT_NATIVE_ALIAS"
                and row.get("normalizedStringAliasAllowed") is False
                and row.get("blockers") == [],
                "standard exact handler grant changed")
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
            verify_numeric_samples(module, occurrence)
            standard_ids.add(module["moduleOccurrenceId"])
    require(len(standard_ids) == 11, "standard resolved blocker coverage changed")

    expected_grants = [{
        "handlerCapabilityId": capability(exact),
        "exactSourceClass": exact,
        "baseHandlerCapabilityId": capability(base),
        "grant": "EXACT_CLASS_HANDLER_ALIAS",
        "requiredEvidenceDecision": "NATIVE_EXACT_ALIAS_VERIFIED",
    } for exact, base, _, _ in STANDARD]
    require(receipt.get("capabilityGrants") == expected_grants, "capability grants changed")

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
    resolved = 0
    remaining = 0
    for owner in module_owners:
        module = source_by_id[owner["moduleOccurrenceId"]]
        require(owner.get("exactSourceClass") == module["exactSourceClass"]
                and owner.get("sourceRecordSha256") == module["sourceRecordSha256"]
                and owner.get("sourceBlockers") == module["blockers"]
                and owner.get("ownerIds"), "module blocker owner join changed")
        if owner["moduleOccurrenceId"] in standard_ids:
            require(owner.get("postJoinDecision") == "READY_FOR_HANDLER"
                    and owner.get("ownerKind") == "RESOLVED_EXACT_HANDLER_CAPABILITY"
                    and owner.get("ownerIds") == [capability(module["exactSourceClass"])]
                    and owner.get("remainingBlockers") == [],
                    "resolved seeded owner changed")
            resolved += 1
        elif owner["moduleOccurrenceId"] in custom_ids:
            require(owner.get("postJoinDecision") == "BLOCKED"
                    and owner.get("ownerKind") == "BLOCKED_CUSTOM_MODULE_HANDLER"
                    and owner.get("ownerIds") == [capability(module["exactSourceClass"])]
                    and owner.get("remainingBlockers") == [HANDLER_BLOCKER],
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
                    and owner.get("remainingBlockers") == [DISTRIBUTION_BLOCKER],
                    "custom distribution module owner changed")
            remaining += 1
    require(resolved == 11 and remaining == 18, "post-join module decisions changed")

    distribution_owners = receipt.get("distributionBlockerOwnership") or []
    require(len(distribution_owners) == 3
            and {row.get("distributionId") for row in distribution_owners}
            == set(expected_distribution_ids),
            "distribution blocker owner coverage changed")
    for owner in distribution_owners:
        require(owner.get("postJoinDecision") == "BLOCKED"
                and owner.get("ownerKind") == "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR"
                and owner.get("ownerId") == evaluator_id
                and owner.get("remainingBlockers") == [DISTRIBUTION_BLOCKER],
                "distribution blocker owner changed")

    summary = receipt.get("summary") or {}
    require(summary.get("sourceBlockedModuleCount") == 29
            and summary.get("standardSeededOccurrenceCount") == 11
            and summary.get("blockedCustomModuleOccurrenceCount") == 15
            and summary.get("blockedCustomDistributionOccurrenceCount") == 3
            and summary.get("moduleBlockerOwnerCount") == 29
            and summary.get("resolvedModuleBlockerCount") == 11
            and summary.get("remainingBlockedModuleCount") == 18
            and summary.get("distributionBlockerOwnerCount") == 3
            and summary.get("ownerlessBlockerCount") == 0
            and summary.get("projectedModuleDecisionCountsAfterJoin")
            == {"READY_FOR_HANDLER": 381, "BLOCKED": 18}
            and summary.get("productAdmissionCount") == 0
            and summary.get("silentFallbackCount") == 0,
            "oracle summary changed")
    require(receipt.get("blockerUnion")
            == [DISTRIBUTION_BLOCKER, HANDLER_BLOCKER, PRODUCT_BLOCKER]
            and receipt.get("productAdmission") == {
                "allowed": False,
                "blockers": [DISTRIBUTION_BLOCKER, HANDLER_BLOCKER, PRODUCT_BLOCKER],
            }, "Product blocker boundary changed")
    return {"ready": 381, "blocked": 18, "distributionBlocked": 3}


def get_proc(module: Any, name: str) -> int:
    kernel32 = ctypes.windll.kernel32
    kernel32.GetProcAddress.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    kernel32.GetProcAddress.restype = ctypes.c_void_p
    address = kernel32.GetProcAddress(module._handle, name.encode("ascii"))
    require(bool(address), f"native export missing: {name}")
    return int(address)


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
        require(entry.index == evidence["exportIndex"]
                and entry.serial_size == evidence["serialSize"]
                and sha256_file_bytes(serial) == evidence["serialSha256"]
                and (package_ref_path(entry.super_index, package.imports, package.exports) or "").casefold()
                == evidence["superClass"],
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
        f"distributionBlocked={result['distributionBlocked']} ownerless=0 product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F custom handler oracle verification failed: {error}", file=sys.stderr)
        raise SystemExit(1)
