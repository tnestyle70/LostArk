#!/usr/bin/env python3
"""Build the Artist F exact seeded-handler/native dispatch oracle.

The source-execution receipt deliberately blocks every class whose old runtime
executor used an ``ef`` prefix or ``_seeded`` suffix alias.  This follow-up
artifact proves only the seven stock Engine seeded aliases.  It combines:

* the frozen source-execution payload identity;
* the installed, hash-pinned Engine.u class/super/property shape;
* the installed, hash-pinned EFEngine.dll export ABI; and
* live wrapper dispatch inspection showing that each seeded Spawn dispatches
  to its exact base SpawnEx overload with an FRandomStream argument.

Lost Ark EF classes and EFDistributionVectorMultiplyParticleParameter remain
blocked.  No class is admitted from its name or inheritance alone, and this
artifact never grants Product admission.
"""

from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import json
import math
import os
import struct
import sys
from pathlib import Path
from typing import Any

from build_artist_31470_source_execution_semantics import (
    canonical_sha256,
    canonical_text_sha256,
    evaluate_descriptor,
    json_bytes,
    validate_receipt as validate_source_execution_receipt,
)
from effect_source_contract_io import load_strict_json_object
from extract_ue3_effect_material_closure import find_export, load_package
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
)


SCHEMA = "lostark.effect-custom-handler-oracle"
FORMAT_VERSION = 1
SOURCE_EXECUTION_RECEIPT_SHA256 = (
    "7e1113dd05bcc9b51056cacc27da1805f7a6d26f65dda5b72c99d26c3141a71c"
)
CUSTOM_HANDLER_BLOCKER = "EXACT_SOURCE_CLASS_HANDLER_NUMERIC_ORACLE_REQUIRED"
CUSTOM_DISTRIBUTION_BLOCKER = "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN"
PRODUCT_BLOCKER = "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED"

EXPECTED_NATIVE = {
    "EFEngine.dll": {
        "bytes": 19606896,
        "sha256": "d086134c7c9e5be10e55c2efdad5a6f35d0c8dc9a02e217def5593aa738f3f4d",
    },
    "LOSTARK.exe": {
        "bytes": 50684768,
        "sha256": "4864342f255a20f9f555f5182a565aefefab64db4c4292d287f241c6cbffd43f",
    },
}

STANDARD_SEEDED = (
    {
        "exact": "particlemodulecolor_seeded",
        "native": "Color",
        "base": "particlemodulecolor",
        "baseNative": "Color",
        "baseQualifier": "QEAAX",
        "occurrences": 2,
        "documentation": "https://dev.epicgames.com/documentation/en-us/unreal-engine/color-modules?application_version=4.27",
    },
    {
        "exact": "particlemodulelifetime_seeded",
        "native": "Lifetime",
        "base": "particlemodulelifetime",
        "baseNative": "Lifetime",
        "baseQualifier": "QEAAX",
        "occurrences": 1,
        "documentation": "https://dev.epicgames.com/documentation/en-us/unreal-engine/lifetime-modules?application_version=4.27",
    },
    {
        "exact": "particlemodulelocation_seeded",
        "native": "Location",
        "base": "particlemodulelocation",
        "baseNative": "Location",
        "baseQualifier": "MEAAX",
        "occurrences": 2,
        "documentation": "https://dev.epicgames.com/documentation/en-us/unreal-engine/location-modules?application_version=4.27",
    },
    {
        "exact": "particlemodulelocationprimitivecylinder_seeded",
        "native": "LocationPrimitiveCylinder",
        "base": "particlemodulelocationprimitivecylinder",
        "baseNative": "LocationPrimitiveCylinder",
        "baseQualifier": "QEAAX",
        "occurrences": 3,
        "documentation": "https://dev.epicgames.com/documentation/en-us/unreal-engine/location-modules?application_version=4.27",
    },
    {
        "exact": "particlemodulemeshrotation_seeded",
        "native": "MeshRotation",
        "base": "particlemodulemeshrotation",
        "baseNative": "MeshRotation",
        "baseQualifier": "QEAAX",
        "occurrences": 1,
        "documentation": "https://dev.epicgames.com/documentation/en-us/unreal-engine/rotation-modules?application_version=4.27",
    },
    {
        "exact": "particlemodulesize_seeded",
        "native": "Size",
        "base": "particlemodulesize",
        "baseNative": "Size",
        "baseQualifier": "QEAAX",
        "occurrences": 1,
        "documentation": "https://dev.epicgames.com/documentation/unreal-engine/size-modules?application_version=4.27",
    },
    {
        "exact": "particlemodulevelocity_seeded",
        "native": "Velocity",
        "base": "particlemodulevelocity",
        "baseNative": "Velocity",
        "baseQualifier": "QEAAX",
        "occurrences": 1,
        "documentation": "https://dev.epicgames.com/documentation/en-us/unreal-engine/velocity-modules?application_version=4.27",
    },
)

CUSTOM_MODULES = (
    {
        "exact": "efparticlemodulelocationonground",
        "super": "engine.particlemodulelocationbase",
        "children": (
            "skiplocation", "adjustlocation", "fskipheight", "btickupdate",
            "bcontinouscheck", "benableskipheight", "foffsetheight", "fcheckbounds",
        ),
        "occurrences": 2,
    },
    {
        "exact": "efparticlemodulelocationprimitivecylinderspin",
        "super": "engine.particlemodulelocationprimitivebase",
        "children": (
            "spinangle", "spinaxis", "startcylinderrot", "startheight",
            "startradius", "badjustforworldspace", "radialvelocity",
            "cylinderspinaxis",
        ),
        "occurrences": 2,
    },
    {
        "exact": "efparticlemodulelocationprimitivecylinderspin_seeded",
        "super": "efparticlemodulelocationprimitivecylinderspin",
        "children": ("randomseedinfo",),
        "occurrences": 3,
    },
    {
        "exact": "efparticlemoduletypedatadecal",
        "super": "engine.particlemoduletypedatabase",
        "children": (
            "bonlycalcrotationyaw", "balwaysdecalupdate",
            "buseplayercharacterrotation", "blendrange", "rotation",
            "farplane", "nearplane", "defaultsize",
        ),
        "occurrences": 3,
    },
    {
        "exact": "efparticlemoduletypedatalight",
        "super": "engine.particlemoduletypedatabase",
        "children": ("pointlightcomponent",),
        "occurrences": 1,
    },
    {
        "exact": "efparticlemodulevelocityoverlifetime",
        "super": "engine.particlemodulevelocitybase",
        "children": ("absolute", "veloverlife"),
        "occurrences": 4,
    },
)

SAMPLE_TIMES = (0.0, 0.25, 1.0)
FIXED_ORACLE_OCCURRENCE_SEED = 0x13579BDF
FRANDOM_STREAM_ALGORITHM = "UE3_LCG_196314165_907633515_MANTISSA23_V1"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def random_stream_units(initial_seed: int, draw_offset: int, count: int) -> tuple[float, ...]:
    """Return UE3 FRandomStream fractions from one explicit 32-bit seed.

    The alias proof does not infer a seed from a class name.  The source seed
    row selects ``initial_seed``; only the one empty source array uses the
    documented fixed oracle occurrence seed.  Both the exact seeded wrapper
    and its exact base SpawnEx receive the same resulting stream state.
    """

    require(type(initial_seed) is int, "random stream seed must be an integer")
    require(type(draw_offset) is int and draw_offset >= 0, "random draw offset invalid")
    require(type(count) is int and count > 0, "random draw count invalid")
    state = initial_seed & 0xFFFFFFFF
    values: list[float] = []
    for index in range(draw_offset + count):
        state = (state * 196314165 + 907633515) & 0xFFFFFFFF
        if index < draw_offset:
            continue
        bits = 0x3F800000 | (state & 0x007FFFFF)
        values.append(struct.unpack("<f", struct.pack("<I", bits))[0] - 1.0)
    return tuple(values)


def verify_file_identity(path: Path, expected: dict[str, Any]) -> dict[str, Any]:
    require(path.is_file(), f"native binary missing: {path}")
    require(path.stat().st_size == expected["bytes"], f"native binary size changed: {path.name}")
    digest = sha256_file(path)
    require(digest == expected["sha256"], f"native binary SHA changed: {path.name}")
    return {
        "fileName": path.name,
        "bytes": path.stat().st_size,
        "sha256": digest,
        "provenance": "LATE_PINNED_CURRENT_INSTALLED_NATIVE_BINARY",
        "sourceEraIdentityPinned": False,
    }


def rva_reader(data: bytes) -> tuple[int, list[tuple[int, int, int]]]:
    require(data[:2] == b"MZ", "native binary DOS header changed")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    require(data[pe:pe + 4] == b"PE\0\0", "native binary PE header changed")
    section_count = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    optional = pe + 24
    magic = struct.unpack_from("<H", data, optional)[0]
    require(magic in (0x10B, 0x20B), "native binary optional header changed")
    data_directory = optional + (112 if magic == 0x20B else 96)
    export_rva = struct.unpack_from("<I", data, data_directory)[0]
    section_offset = optional + optional_size
    sections = []
    for index in range(section_count):
        offset = section_offset + index * 40
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from(
            "<IIII", data, offset + 8
        )
        sections.append((virtual_address, max(virtual_size, raw_size), raw_offset))
    return export_rva, sections


def file_offset_for_rva(rva: int, sections: list[tuple[int, int, int]]) -> int:
    for virtual_address, size, raw_offset in sections:
        if virtual_address <= rva < virtual_address + size:
            return raw_offset + rva - virtual_address
    raise ValueError(f"PE RVA is outside raw sections: 0x{rva:x}")


def parse_pe_exports(path: Path) -> dict[str, int]:
    data = path.read_bytes()
    export_rva, sections = rva_reader(data)
    require(export_rva != 0, f"native binary export directory missing: {path.name}")
    export_offset = file_offset_for_rva(export_rva, sections)
    fields = struct.unpack_from("<IIHHIIIIIII", data, export_offset)
    _, _, _, _, _, base, function_count, name_count, functions_rva, names_rva, ordinals_rva = fields
    require(function_count > 0 and name_count > 0, f"native exports empty: {path.name}")
    functions_offset = file_offset_for_rva(functions_rva, sections)
    names_offset = file_offset_for_rva(names_rva, sections)
    ordinals_offset = file_offset_for_rva(ordinals_rva, sections)
    result: dict[str, int] = {}
    for index in range(name_count):
        name_rva = struct.unpack_from("<I", data, names_offset + index * 4)[0]
        name_offset = file_offset_for_rva(name_rva, sections)
        end = data.index(b"\0", name_offset)
        name = data[name_offset:end].decode("ascii")
        ordinal_index = struct.unpack_from("<H", data, ordinals_offset + index * 2)[0]
        require(ordinal_index < function_count, f"native export ordinal changed: {name}")
        function_rva = struct.unpack_from("<I", data, functions_offset + ordinal_index * 4)[0]
        result[name] = function_rva
    require(base > 0, f"native export ordinal base changed: {path.name}")
    return result


def class_evidence(package: Any, exact: str, expected_super: str | None = None,
                   expected_children: tuple[str, ...] | None = None) -> dict[str, Any]:
    entry = find_export(package, exact)
    actual_path = package_ref_path(entry.index + 1, package.imports, package.exports)
    super_path = package_ref_path(entry.super_index, package.imports, package.exports) or ""
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
    serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
    if expected_super is not None:
        require(super_path.casefold() == expected_super.casefold(), f"class super changed: {exact}")
    if expected_children is not None:
        require(
            tuple(row["name"] for row in children) == expected_children,
            f"class direct children changed: {exact}",
        )
    return {
        "className": exact.casefold(),
        "objectPath": actual_path,
        "exportIndex": entry.index,
        "superClass": super_path.casefold(),
        "serialSize": entry.serial_size,
        "serialSha256": hashlib.sha256(serial).hexdigest(),
        "directChildren": children,
        "uFunctionChildCount": sum(row["className"] == "function" for row in children),
        "provenance": "CURRENT_SCRIPT_PACKAGE_CLASS_METADATA",
        "sourceEraIdentityPinned": False,
    }


def decorated_symbols(row: dict[str, Any]) -> dict[str, str]:
    native = row["native"]
    base = row["baseNative"]
    return {
        "spawn": f"?Spawn@UParticleModule{native}_Seeded@@UEAAXPEAUFParticleEmitterInstance@@HM@Z",
        "spawnEx": (
            f"?SpawnEx@UParticleModule{base}@@{row['baseQualifier']}"
            "PEAUFParticleEmitterInstance@@HMPEAVFRandomStream@@@Z"
        ),
        "getRandomSeedInfo": (
            f"?GetRandomSeedInfo@UParticleModule{native}_Seeded@@"
            "UEAAPEAUFParticleRandomSeedInfo@@XZ"
        ),
        "emitterLoopingNotify": (
            f"?EmitterLoopingNotify@UParticleModule{native}_Seeded@@"
            "UEAAXPEAUFParticleEmitterInstance@@@Z"
        ),
        "prepPerInstanceBlock": (
            f"?PrepPerInstanceBlock@UParticleModule{native}_Seeded@@"
            "UEAAIPEAUFParticleEmitterInstance@@PEAX@Z"
        ),
        "requiredBytesPerInstance": (
            f"?RequiredBytesPerInstance@UParticleModule{native}_Seeded@@"
            "UEAAIPEAUFParticleEmitterInstance@@@Z"
        ),
        "vtable": f"??_7UParticleModule{native}_Seeded@@6B@",
    }


def get_proc_address(module: Any, name: str) -> int:
    kernel32 = ctypes.windll.kernel32
    kernel32.GetProcAddress.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    kernel32.GetProcAddress.restype = ctypes.c_void_p
    address = kernel32.GetProcAddress(module._handle, name.encode("ascii"))
    require(bool(address), f"loaded native export missing: {name}")
    return int(address)


def native_dispatch_evidence(
    row: dict[str, Any], module: Any, exports: dict[str, int]
) -> dict[str, Any]:
    symbols = decorated_symbols(row)
    for role, name in symbols.items():
        require(name in exports, f"native {role} export missing: {row['exact']}")
    spawn = get_proc_address(module, symbols["spawn"])
    spawn_ex = get_proc_address(module, symbols["spawnEx"])
    wrapper = ctypes.string_at(spawn, 96)
    require(b"\xff\x90\xa0\x00\x00\x00" in wrapper,
            f"seed stream owner dispatch changed: {row['exact']}")
    store_pattern = b"\x48\x89\x44\x24\x20"
    store_offset = wrapper.find(store_pattern)
    require(store_offset >= 0, f"FRandomStream fifth argument store changed: {row['exact']}")

    if row["exact"] == "particlemodulelocation_seeded":
        pattern = b"\x41\xff\x91"
        dispatch_offset = wrapper.find(pattern)
        require(dispatch_offset >= 0, "seeded Location virtual dispatch changed")
        vtable_slot = struct.unpack_from("<I", wrapper, dispatch_offset + 3)[0]
        vtable = get_proc_address(module, symbols["vtable"])
        dispatch_target = ctypes.c_uint64.from_address(vtable + vtable_slot).value
        require(dispatch_target == spawn_ex, "seeded Location vtable no longer targets base SpawnEx")
        dispatch_kind = "SEEDED_VTABLE_SLOT_TO_EXACT_BASE_SPAWN_EX"
        dispatch_detail = {"vtableSlotBytes": vtable_slot}
    else:
        direct_targets = []
        for index in range(len(wrapper) - 4):
            if wrapper[index] != 0xE8:
                continue
            target = spawn + index + 5 + struct.unpack_from("<i", wrapper, index + 1)[0]
            if target == spawn_ex:
                direct_targets.append(index)
        require(len(direct_targets) == 1,
                f"seeded wrapper does not uniquely call base SpawnEx: {row['exact']}")
        dispatch_offset = direct_targets[0]
        dispatch_kind = "DIRECT_REL32_TO_EXACT_BASE_SPAWN_EX"
        dispatch_detail = {}

    base_address = int(module._handle)
    return {
        "decision": "NATIVE_EXACT_ALIAS_VERIFIED",
        "spawnExport": symbols["spawn"],
        "spawnRva": spawn - base_address,
        "baseSpawnExExport": symbols["spawnEx"],
        "baseSpawnExRva": spawn_ex - base_address,
        "randomSeedHelperExports": [
            {"role": role, "symbol": symbols[role], "rva": exports[symbols[role]]}
            for role in (
                "getRandomSeedInfo", "emitterLoopingNotify",
                "prepPerInstanceBlock", "requiredBytesPerInstance",
            )
        ],
        "randomStreamReturnStoredAsFifthArgument": True,
        "randomStreamStoreOffset": store_offset,
        "dispatchKind": dispatch_kind,
        "dispatchOffset": dispatch_offset,
        "wrapperFirst96BytesSha256": hashlib.sha256(wrapper).hexdigest(),
        **dispatch_detail,
    }


def module_rows(source: dict[str, Any]) -> list[dict[str, Any]]:
    return [module for occurrence in source["occurrences"] for module in occurrence["modules"]]


def numeric_samples(module: dict[str, Any]) -> list[dict[str, Any]]:
    seed_values = module["seed"]["randomSeeds"]
    if seed_values:
        initial_seed = seed_values[0]
        seed_source = "SOURCE_DECODED_FIRST_RANDOM_SEED"
    else:
        initial_seed = FIXED_ORACLE_OCCURRENCE_SEED
        seed_source = "FIXED_ORACLE_OCCURRENCE_STREAM_SEED_NOT_SOURCE_VALUE"
    samples = []
    for sample_index, emitter_time in enumerate(SAMPLE_TIMES):
        draw_offset = sample_index * 4
        random_units = random_stream_units(initial_seed, draw_offset, 4)
        values = []
        for distribution in module["typedPayload"]["distributions"]:
            descriptor = distribution["descriptor"]
            require(
                descriptor.get("payloadStatus") == "INLINE_SOURCE_PAYLOAD",
                f"stock seeded handler contains non-inline distribution: {module['moduleOccurrenceId']}",
            )
            value = evaluate_descriptor(descriptor, emitter_time, random_units)
            require(all(math.isfinite(float(component)) for component in value),
                    "seeded numeric oracle produced non-finite value")
            values.append({
                "payloadDistributionId": distribution["payloadDistributionId"],
                "propertyPath": descriptor["propertyPath"],
                "value": value,
            })
        handler_input = {
            "payloadSha256": module["typedPayload"]["payloadSha256"],
            "emitterTime": emitter_time,
            "fixedSeed": initial_seed,
            "fixedSeedSource": seed_source,
            "randomStreamAlgorithm": FRANDOM_STREAM_ALGORITHM,
            "randomStreamDrawOffset": draw_offset,
            "randomUnits": list(random_units),
            "evaluatedDistributions": values,
        }
        digest = canonical_sha256(handler_input)
        samples.append({
            "sampleId": f"{module['moduleOccurrenceId']}::sample:{sample_index:03d}",
            **handler_input,
            "baseHandlerInputSha256": digest,
            "exactSeededHandlerInputSha256": digest,
            "numericInputParity": True,
            "randomStreamContract": "FIXED_SEED_STREAM_NATIVE_WRAPPER_OWNS_FRANDOMSTREAM",
        })
    return samples


def exact_handler_capability(exact_class: str) -> str:
    component = exact_class.replace("_", ".")
    return "source.module.exact." + component + ".v1"


def build_blocker_ownership(
    source: dict[str, Any],
    standard_rows: list[dict[str, Any]],
    custom_rows: list[dict[str, Any]],
    custom_distribution: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    standard_by_class = {row["exactSourceClass"]: row for row in standard_rows}
    custom_by_class = {row["exactSourceClass"]: row for row in custom_rows}
    distribution_occurrences = {
        row["distributionId"]: row
        for row in custom_distribution["occurrences"]
    }
    module_ownership: list[dict[str, Any]] = []
    distribution_ownership: list[dict[str, Any]] = []
    for module in module_rows(source):
        if module["decision"] != "BLOCKED":
            continue
        exact_class = module["exactSourceClass"]
        if exact_class in standard_by_class:
            handler = standard_by_class[exact_class]
            owner_kind = "RESOLVED_EXACT_HANDLER_CAPABILITY"
            owner_ids = [handler["handlerCapabilityId"]]
            post_join_decision = "READY_FOR_HANDLER"
            remaining_blockers: list[str] = []
        elif exact_class in custom_by_class:
            handler = custom_by_class[exact_class]
            owner_kind = "BLOCKED_CUSTOM_MODULE_HANDLER"
            owner_ids = [handler["handlerCapabilityId"]]
            post_join_decision = "BLOCKED"
            remaining_blockers = [CUSTOM_HANDLER_BLOCKER]
        else:
            blocked_distributions = [
                row for row in module["distributionAdapters"]
                if row["decision"] == "BLOCKED"
            ]
            require(blocked_distributions, "blocked module has no blocker owner")
            require(all(
                row["distributionId"] in distribution_occurrences
                and row["blockers"] == [CUSTOM_DISTRIBUTION_BLOCKER]
                for row in blocked_distributions
            ), "blocked module distribution owner changed")
            owner_kind = "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR"
            owner_ids = [row["distributionId"] for row in blocked_distributions]
            post_join_decision = "BLOCKED"
            remaining_blockers = [CUSTOM_DISTRIBUTION_BLOCKER]
        module_ownership.append({
            "moduleOccurrenceId": module["moduleOccurrenceId"],
            "exactSourceClass": exact_class,
            "sourceRecordSha256": module["sourceRecordSha256"],
            "sourceBlockers": copy.deepcopy(module["blockers"]),
            "postJoinDecision": post_join_decision,
            "ownerKind": owner_kind,
            "ownerIds": owner_ids,
            "remainingBlockers": remaining_blockers,
        })
    for module in module_rows(source):
        for distribution in module["distributionAdapters"]:
            if distribution["decision"] != "BLOCKED":
                continue
            require(
                distribution["distributionId"] in distribution_occurrences,
                "blocked distribution has no evaluator owner",
            )
            distribution_ownership.append({
                "distributionId": distribution["distributionId"],
                "moduleOccurrenceId": module["moduleOccurrenceId"],
                "exactSourceClass": distribution["exactSourceClass"],
                "sourceBlockers": copy.deepcopy(distribution["blockers"]),
                "postJoinDecision": "BLOCKED",
                "ownerKind": "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR",
                "ownerId": custom_distribution["evaluatorCapabilityId"],
                "remainingBlockers": [CUSTOM_DISTRIBUTION_BLOCKER],
            })
    require(len(module_ownership) == 29, "blocked module owner denominator changed")
    require(len(distribution_ownership) == 3, "blocked distribution owner denominator changed")
    return module_ownership, distribution_ownership


def build_receipt(
    root: Path,
    source: dict[str, Any],
    release_root: Path,
    binary_root: Path,
) -> dict[str, Any]:
    validate_source_execution_receipt(source)
    require(
        source["receiptSha256"] == SOURCE_EXECUTION_RECEIPT_SHA256,
        "frozen source execution receipt identity changed",
    )
    require(
        source.get("characterClass") == "ARTIST"
        and source.get("skillId") == 31470
        and source.get("inputSlot") == "F",
        "source execution root identity changed",
    )

    engine_script_row = next(
        row for row in source["currentRevisionDefaultEvidence"]["scriptPackages"]
        if row["logicalPackage"].casefold() == "engine"
    )
    efgame_script_row = next(
        row for row in source["currentRevisionDefaultEvidence"]["scriptPackages"]
        if row["logicalPackage"].casefold() == "efgame"
    )
    for script_row in (engine_script_row, efgame_script_row):
        script_path = release_root / script_row["physicalPackage"]
        require(script_path.is_file(), f"script package missing: {script_path}")
        require(script_path.stat().st_size == script_row["bytes"],
                f"script package size changed: {script_path.name}")
        require(sha256_file(script_path) == script_row["sha256"],
                f"script package SHA changed: {script_path.name}")
    engine = load_package(release_root / engine_script_row["physicalPackage"], LOSTARK_KR_AES_KEY)
    efgame = load_package(release_root / efgame_script_row["physicalPackage"], LOSTARK_KR_AES_KEY)

    binary_identities = []
    for name, expected in EXPECTED_NATIVE.items():
        binary_identities.append(verify_file_identity(binary_root / name, expected))
    engine_dll = binary_root / "EFEngine.dll"
    engine_exports = parse_pe_exports(engine_dll)
    lostark_exports = parse_pe_exports(binary_root / "LOSTARK.exe")
    with os.add_dll_directory(str(binary_root)):
        loaded_engine = ctypes.WinDLL(str(engine_dll))

    modules = module_rows(source)
    standard_rows = []
    grants = []
    for definition in STANDARD_SEEDED:
        exact = definition["exact"]
        exact_native_class = "ParticleModule" + definition["native"] + "_Seeded"
        base_native_class = "ParticleModule" + definition["baseNative"]
        evidence = class_evidence(
            engine, exact_native_class,
            expected_super=base_native_class,
            expected_children=("randomseedinfo",),
        )
        require(
            len(evidence["directChildren"]) == 1
            and evidence["directChildren"][0]["className"] == "structproperty"
            and evidence["uFunctionChildCount"] == 0,
            f"stock seeded class shape changed: {exact}",
        )
        occurrences = [row for row in modules if row["exactSourceClass"] == exact]
        require(len(occurrences) == definition["occurrences"],
                f"stock seeded occurrence denominator changed: {exact}")
        dispatch = native_dispatch_evidence(definition, loaded_engine, engine_exports)
        occurrence_rows = []
        for module in occurrences:
            require(module["seed"] is not None, f"stock seeded source seed missing: {exact}")
            occurrence_rows.append({
                "moduleOccurrenceId": module["moduleOccurrenceId"],
                "sourceObjectId": module["sourceObjectId"],
                "sourceRecordSha256": module["sourceRecordSha256"],
                "payloadSha256": module["typedPayload"]["payloadSha256"],
                "seedEvidence": copy.deepcopy(module["seed"]),
                "numericOracleSamples": numeric_samples(module),
            })
        capability = exact_handler_capability(exact)
        base_capability = exact_handler_capability(definition["base"])
        standard_rows.append({
            "exactSourceClass": exact,
            "baseSourceClass": definition["base"],
            "handlerCapabilityId": capability,
            "baseHandlerCapabilityId": base_capability,
            "decision": "READY_FOR_EXACT_NATIVE_ALIAS",
            "normalizedStringAliasAllowed": False,
            "aliasMechanism": "NATIVE_SEEDED_SPAWN_TO_EXACT_BASE_SPAWN_EX",
            "officialCascadeDocumentation": {
                "url": definition["documentation"],
                "claim": "SEEDED_MODULE_IS_BASE_MODULE_WITH_RANDOM_SEED_INFORMATION",
                "role": "PUBLIC_SEMANTIC_CORROBORATION_NOT_BINARY_IDENTITY",
            },
            "currentScriptClassEvidence": evidence,
            "currentNativeDispatchEvidence": dispatch,
            "occurrences": occurrence_rows,
            "blockers": [],
        })
        grants.append({
            "handlerCapabilityId": capability,
            "exactSourceClass": exact,
            "baseHandlerCapabilityId": base_capability,
            "grant": "EXACT_CLASS_HANDLER_ALIAS",
            "requiredEvidenceDecision": "NATIVE_EXACT_ALIAS_VERIFIED",
        })

    custom_rows = []
    for definition in CUSTOM_MODULES:
        exact = definition["exact"]
        evidence = class_evidence(
            efgame, exact,
            expected_super=definition["super"],
            expected_children=definition["children"],
        )
        occurrences = [row for row in modules if row["exactSourceClass"] == exact]
        require(len(occurrences) == definition["occurrences"],
                f"custom module occurrence denominator changed: {exact}")
        exported_matches = sorted(
            name for name in (*engine_exports.keys(), *lostark_exports.keys())
            if exact.casefold() in name.casefold()
        )
        require(not exported_matches, f"custom native symbols became inspectable: {exact}")
        custom_rows.append({
            "exactSourceClass": exact,
            "handlerCapabilityId": exact_handler_capability(exact),
            "decision": "BLOCKED",
            "currentScriptClassEvidence": evidence,
            "installedNativeExportMatches": exported_matches,
            "occurrenceIds": [row["moduleOccurrenceId"] for row in occurrences],
            "reason": "NO_EXACT_NATIVE_DISPATCH_OR_CONTROLLED_NUMERIC_EVALUATOR_ORACLE",
            "blockers": [CUSTOM_HANDLER_BLOCKER],
        })

    distribution_occurrences = []
    for module in modules:
        for distribution in module["distributionAdapters"]:
            if distribution.get("exactSourceClass") != "efdistributionvectormultiplyparticleparameter":
                continue
            distribution_occurrences.append({
                "distributionId": distribution["distributionId"],
                "moduleOccurrenceId": module["moduleOccurrenceId"],
                "sourceFidelity": distribution["sourceFidelity"],
                "currentRevisionFields": copy.deepcopy(distribution["currentRevisionFields"]),
                "blockers": copy.deepcopy(distribution["blockers"]),
            })
    require(len(distribution_occurrences) == 3,
            "custom multiply distribution occurrence denominator changed")
    distribution_class = class_evidence(
        efgame,
        "EFDistributionVectorMultiplyParticleParameter",
        expected_super="Engine.DistributionVectorParameterBase",
        expected_children=(),
    )
    distribution_export_matches = sorted(
        name for name in (*engine_exports.keys(), *lostark_exports.keys())
        if "efdistributionvectormultiplyparticleparameter" in name.casefold()
    )
    require(not distribution_export_matches,
            "custom multiply native symbol became inspectable")
    custom_distribution = {
        "exactSourceClass": "efdistributionvectormultiplyparticleparameter",
        "evaluatorCapabilityId": (
            "source.distribution.exact.efdistributionvectormultiplyparticleparameter.v1"
        ),
        "decision": "BLOCKED",
        "currentScriptClassEvidence": distribution_class,
        "installedNativeExportMatches": distribution_export_matches,
        "occurrences": distribution_occurrences,
        "reason": "NO_EXACT_NATIVE_GET_VALUE_OR_CONTROLLED_PARAMETER_INPUT_NUMERIC_ORACLE",
        "blockers": [CUSTOM_DISTRIBUTION_BLOCKER],
    }

    module_ownership, distribution_ownership = build_blocker_ownership(
        source, standard_rows, custom_rows, custom_distribution
    )

    generator_path = Path(__file__).resolve()
    projected_ready = source["summary"]["moduleDecisionCounts"]["READY_FOR_HANDLER"] + sum(
        len(row["occurrences"]) for row in standard_rows
    )
    projected_blocked = source["summary"]["denominators"]["moduleCount"] - projected_ready
    result = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "EXACT_SEEDED_HANDLER_ALIAS_AND_CUSTOM_BLOCKER_ORACLE",
        "sourceExecutionReceipt": {
            "path": (
                "Data/Effects/Imported/Artist/Candidates/"
                "skill.31470.source-execution-semantics.receipt.json"
            ),
            "receiptSha256": source["receiptSha256"],
            "canonicalJsonSha256": canonical_sha256(source),
        },
        "toolIdentity": {
            "path": str(generator_path.relative_to(root)).replace("\\", "/"),
            "canonicalTextSha256": canonical_text_sha256(generator_path),
        },
        "currentInstalledNativeBinaries": binary_identities,
        "standardSeededHandlers": standard_rows,
        "capabilityGrants": grants,
        "blockedCustomModuleHandlers": custom_rows,
        "blockedCustomDistributionEvaluator": custom_distribution,
        "moduleBlockerOwnership": module_ownership,
        "distributionBlockerOwnership": distribution_ownership,
        "summary": {
            "sourceBlockedModuleCount": len(module_ownership),
            "standardSeededFamilyCount": len(standard_rows),
            "standardSeededOccurrenceCount": sum(len(row["occurrences"]) for row in standard_rows),
            "nativeExactAliasCount": sum(
                row["currentNativeDispatchEvidence"]["decision"] == "NATIVE_EXACT_ALIAS_VERIFIED"
                for row in standard_rows
            ),
            "capabilityGrantCount": len(grants),
            "blockedCustomModuleFamilyCount": len(custom_rows),
            "blockedCustomModuleOccurrenceCount": sum(len(row["occurrenceIds"]) for row in custom_rows),
            "blockedCustomDistributionFamilyCount": 1,
            "blockedCustomDistributionOccurrenceCount": len(distribution_occurrences),
            "moduleBlockerOwnerCount": len(module_ownership),
            "resolvedModuleBlockerCount": sum(
                row["postJoinDecision"] == "READY_FOR_HANDLER"
                for row in module_ownership
            ),
            "remainingBlockedModuleCount": sum(
                row["postJoinDecision"] == "BLOCKED"
                for row in module_ownership
            ),
            "distributionBlockerOwnerCount": len(distribution_ownership),
            "ownerlessBlockerCount": 0,
            "projectedModuleDecisionCountsAfterJoin": {
                "READY_FOR_HANDLER": projected_ready,
                "BLOCKED": projected_blocked,
            },
            "productAdmissionCount": 0,
            "silentFallbackCount": 0,
        },
        "blockerUnion": [
            CUSTOM_DISTRIBUTION_BLOCKER,
            CUSTOM_HANDLER_BLOCKER,
            PRODUCT_BLOCKER,
        ],
        "productAdmission": {
            "allowed": False,
            "blockers": [
                CUSTOM_DISTRIBUTION_BLOCKER,
                CUSTOM_HANDLER_BLOCKER,
                PRODUCT_BLOCKER,
            ],
        },
    }
    result["receiptSha256"] = canonical_sha256(result)
    validate_receipt(result, source)
    return result


def validate_receipt(receipt: dict[str, Any], source: dict[str, Any] | None = None) -> None:
    require(receipt.get("schema") == SCHEMA, "custom handler oracle schema changed")
    require(type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == 1,
            "custom handler oracle version changed")
    unsigned = copy.deepcopy(receipt)
    expected_hash = str(unsigned.pop("receiptSha256", ""))
    require(expected_hash and canonical_sha256(unsigned) == expected_hash,
            "custom handler oracle self hash changed")
    require(receipt.get("characterClass") == "ARTIST"
            and receipt.get("skillId") == 31470
            and receipt.get("inputSlot") == "F",
            "custom handler oracle root identity changed")
    require(
        receipt.get("sourceExecutionReceipt", {}).get("receiptSha256")
        == SOURCE_EXECUTION_RECEIPT_SHA256,
        "custom handler oracle source receipt changed",
    )
    summary = receipt.get("summary") or {}
    require(summary == {
        "sourceBlockedModuleCount": 29,
        "standardSeededFamilyCount": 7,
        "standardSeededOccurrenceCount": 11,
        "nativeExactAliasCount": 7,
        "capabilityGrantCount": 7,
        "blockedCustomModuleFamilyCount": 6,
        "blockedCustomModuleOccurrenceCount": 15,
        "blockedCustomDistributionFamilyCount": 1,
        "blockedCustomDistributionOccurrenceCount": 3,
        "moduleBlockerOwnerCount": 29,
        "resolvedModuleBlockerCount": 11,
        "remainingBlockedModuleCount": 18,
        "distributionBlockerOwnerCount": 3,
        "ownerlessBlockerCount": 0,
        "projectedModuleDecisionCountsAfterJoin": {
            "READY_FOR_HANDLER": 381,
            "BLOCKED": 18,
        },
        "productAdmissionCount": 0,
        "silentFallbackCount": 0,
    }, "custom handler oracle summary changed")
    standard = receipt.get("standardSeededHandlers") or []
    require(len(standard) == 7 and len(receipt.get("capabilityGrants") or []) == 7,
            "custom handler oracle grant denominator changed")
    require(
        [row.get("exactSourceClass") for row in standard]
        == [row["exact"] for row in STANDARD_SEEDED],
        "stock seeded handler class order changed",
    )
    all_standard_occurrences: list[str] = []
    for definition, row in zip(STANDARD_SEEDED, standard):
        exact = definition["exact"]
        require(row.get("decision") == "READY_FOR_EXACT_NATIVE_ALIAS"
                and row.get("normalizedStringAliasAllowed") is False
                and row.get("aliasMechanism") == "NATIVE_SEEDED_SPAWN_TO_EXACT_BASE_SPAWN_EX"
                and row.get("baseSourceClass") == definition["base"]
                and row.get("handlerCapabilityId") == exact_handler_capability(exact)
                and row.get("baseHandlerCapabilityId")
                == exact_handler_capability(definition["base"])
                and not row.get("blockers"),
                "stock seeded handler is not exact-alias ready")
        require(row.get("currentNativeDispatchEvidence", {}).get("decision")
                == "NATIVE_EXACT_ALIAS_VERIFIED"
                and row["currentNativeDispatchEvidence"].get(
                    "randomStreamReturnStoredAsFifthArgument"
                ) is True
                and row["currentNativeDispatchEvidence"].get("dispatchKind") in {
                    "DIRECT_REL32_TO_EXACT_BASE_SPAWN_EX",
                    "SEEDED_VTABLE_SLOT_TO_EXACT_BASE_SPAWN_EX",
                },
                "stock seeded native dispatch proof changed")
        class_evidence_row = row.get("currentScriptClassEvidence") or {}
        require(
            class_evidence_row.get("className") == exact
            and class_evidence_row.get("superClass") == definition["base"]
            and class_evidence_row.get("uFunctionChildCount") == 0
            and class_evidence_row.get("sourceEraIdentityPinned") is False,
            "stock seeded current script evidence changed",
        )
        require(len(row.get("occurrences") or []) == definition["occurrences"],
                "stock seeded occurrence count changed")
        for occurrence in row.get("occurrences") or []:
            all_standard_occurrences.append(occurrence.get("moduleOccurrenceId", ""))
            require(len(occurrence.get("numericOracleSamples") or []) == 3,
                    "stock seeded numeric sample denominator changed")
            for sample_index, sample in enumerate(occurrence["numericOracleSamples"]):
                handler_input = {
                    name: copy.deepcopy(sample[name])
                    for name in (
                        "payloadSha256", "emitterTime", "fixedSeed",
                        "fixedSeedSource", "randomStreamAlgorithm",
                        "randomStreamDrawOffset", "randomUnits",
                        "evaluatedDistributions",
                    )
                }
                digest = canonical_sha256(handler_input)
                require(sample.get("numericInputParity") is True
                        and sample.get("randomStreamAlgorithm") == FRANDOM_STREAM_ALGORITHM
                        and sample.get("emitterTime") == SAMPLE_TIMES[sample_index]
                        and sample.get("randomStreamDrawOffset") == sample_index * 4
                        and sample.get("randomUnits") == list(random_stream_units(
                            sample["fixedSeed"], sample_index * 4, 4
                        ))
                        and sample.get("baseHandlerInputSha256") == digest
                        and sample.get("exactSeededHandlerInputSha256") == digest,
                        "stock seeded numeric input parity changed")
    require(len(all_standard_occurrences) == len(set(all_standard_occurrences)) == 11,
            "stock seeded occurrence identity changed")
    expected_grants = [{
        "handlerCapabilityId": exact_handler_capability(row["exact"]),
        "exactSourceClass": row["exact"],
        "baseHandlerCapabilityId": exact_handler_capability(row["base"]),
        "grant": "EXACT_CLASS_HANDLER_ALIAS",
        "requiredEvidenceDecision": "NATIVE_EXACT_ALIAS_VERIFIED",
    } for row in STANDARD_SEEDED]
    require(receipt.get("capabilityGrants") == expected_grants,
            "stock seeded capability grant join changed")
    custom = receipt.get("blockedCustomModuleHandlers") or []
    require(
        [row.get("exactSourceClass") for row in custom]
        == [row["exact"] for row in CUSTOM_MODULES],
        "custom handler class order changed",
    )
    all_custom_occurrences: list[str] = []
    for definition, row in zip(CUSTOM_MODULES, custom):
        all_custom_occurrences.extend(row.get("occurrenceIds") or [])
        require(
            row.get("decision") == "BLOCKED"
            and row.get("handlerCapabilityId") == exact_handler_capability(definition["exact"])
            and row.get("blockers") == [CUSTOM_HANDLER_BLOCKER]
            and row.get("installedNativeExportMatches") == []
            and len(row.get("occurrenceIds") or []) == definition["occurrences"]
            and row.get("currentScriptClassEvidence", {}).get("sourceEraIdentityPinned") is False,
            "custom handler blocker laundering detected",
        )
    require(len(all_custom_occurrences) == len(set(all_custom_occurrences)) == 15,
            "custom handler occurrence identity changed")
    distribution = receipt.get("blockedCustomDistributionEvaluator") or {}
    require(distribution.get("decision") == "BLOCKED"
            and distribution.get("blockers") == [CUSTOM_DISTRIBUTION_BLOCKER]
            and len(distribution.get("occurrences") or []) == 3,
            "custom distribution blocker laundering detected")
    require(
        distribution.get("exactSourceClass")
        == "efdistributionvectormultiplyparticleparameter"
        and distribution.get("evaluatorCapabilityId")
        == "source.distribution.exact.efdistributionvectormultiplyparticleparameter.v1"
        and distribution.get("installedNativeExportMatches") == []
        and distribution.get("currentScriptClassEvidence", {}).get(
            "sourceEraIdentityPinned"
        ) is False,
        "custom distribution evaluator identity changed",
    )
    module_owners = receipt.get("moduleBlockerOwnership") or []
    distribution_owners = receipt.get("distributionBlockerOwnership") or []
    require(len(module_owners) == 29
            and len({row.get("moduleOccurrenceId") for row in module_owners}) == 29,
            "module blocker ownership coverage changed")
    require(len(distribution_owners) == 3
            and len({row.get("distributionId") for row in distribution_owners}) == 3,
            "distribution blocker ownership coverage changed")
    grant_ids = {row["handlerCapabilityId"] for row in expected_grants}
    custom_ids = {row["handlerCapabilityId"] for row in custom}
    distribution_id = distribution["evaluatorCapabilityId"]
    for row in module_owners:
        require(row.get("ownerIds"), "module blocker owner is empty")
        if row.get("postJoinDecision") == "READY_FOR_HANDLER":
            require(row.get("ownerKind") == "RESOLVED_EXACT_HANDLER_CAPABILITY"
                    and set(row["ownerIds"]).issubset(grant_ids)
                    and row.get("remainingBlockers") == [],
                    "resolved module blocker ownership changed")
        else:
            require(row.get("postJoinDecision") == "BLOCKED"
                    and row.get("remainingBlockers") in (
                        [CUSTOM_HANDLER_BLOCKER], [CUSTOM_DISTRIBUTION_BLOCKER]
                    ), "remaining module blocker ownership changed")
            if row.get("ownerKind") == "BLOCKED_CUSTOM_MODULE_HANDLER":
                require(set(row["ownerIds"]).issubset(custom_ids),
                        "custom handler owner ID changed")
            else:
                require(row.get("ownerKind") == "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR",
                        "custom distribution module owner kind changed")
    for row in distribution_owners:
        require(row.get("postJoinDecision") == "BLOCKED"
                and row.get("ownerKind") == "BLOCKED_CUSTOM_DISTRIBUTION_EVALUATOR"
                and row.get("ownerId") == distribution_id
                and row.get("remainingBlockers") == [CUSTOM_DISTRIBUTION_BLOCKER],
                "distribution blocker owner changed")
    if source is not None:
        validate_source_execution_receipt(source)
        require(source.get("receiptSha256") == SOURCE_EXECUTION_RECEIPT_SHA256
                and receipt["sourceExecutionReceipt"].get("canonicalJsonSha256")
                == canonical_sha256(source),
                "source execution receipt join changed")
        source_modules = module_rows(source)
        source_by_id = {row["moduleOccurrenceId"]: row for row in source_modules}
        require(len(source_by_id) == 399, "source module identity denominator changed")
        for row in standard:
            expected_modules = [
                module for module in source_modules
                if module["exactSourceClass"] == row["exactSourceClass"]
            ]
            expected_occurrences = []
            for module in expected_modules:
                expected_occurrences.append({
                    "moduleOccurrenceId": module["moduleOccurrenceId"],
                    "sourceObjectId": module["sourceObjectId"],
                    "sourceRecordSha256": module["sourceRecordSha256"],
                    "payloadSha256": module["typedPayload"]["payloadSha256"],
                    "seedEvidence": copy.deepcopy(module["seed"]),
                    "numericOracleSamples": numeric_samples(module),
                })
            require(row["occurrences"] == expected_occurrences,
                    "stock seeded source occurrence join changed")
        for row in custom:
            expected_ids = [
                module["moduleOccurrenceId"] for module in source_modules
                if module["exactSourceClass"] == row["exactSourceClass"]
            ]
            require(row["occurrenceIds"] == expected_ids,
                    "custom handler source occurrence join changed")
        expected_module_owners, expected_distribution_owners = build_blocker_ownership(
            source, standard, custom, distribution
        )
        require(module_owners == expected_module_owners,
                "module blocker ownership source join changed")
        require(distribution_owners == expected_distribution_owners,
                "distribution blocker ownership source join changed")
    require(receipt.get("blockerUnion") == [
        CUSTOM_DISTRIBUTION_BLOCKER, CUSTOM_HANDLER_BLOCKER, PRODUCT_BLOCKER,
    ], "custom handler blocker union changed")
    require(receipt.get("productAdmission", {}).get("allowed") is False,
            "custom handler oracle granted Product admission")


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--source-execution", type=Path,
        default=root / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-execution-semantics.receipt.json",
    )
    parser.add_argument(
        "--release-root", type=Path,
        default=Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"),
    )
    parser.add_argument(
        "--binary-root", type=Path,
        default=Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\Binaries\Win64"),
    )
    parser.add_argument(
        "--output", type=Path,
        default=root / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.custom-handler-oracle.receipt.json",
    )
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    source = load_strict_json_object(args.source_execution)
    result = build_receipt(root, source, args.release_root, args.binary_root)
    if args.check:
        current = load_strict_json_object(args.output)
        validate_receipt(current, source)
        require(json_bytes(current) == json_bytes(result), "custom handler oracle is stale")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_suffix(args.output.suffix + ".tmp")
        temporary.write_bytes(json_bytes(result))
        temporary.replace(args.output)
    print(
        "Artist F custom handler oracle: "
        f"seeded={result['summary']['standardSeededOccurrenceCount']} "
        f"nativeAliases={result['summary']['nativeExactAliasCount']} "
        f"customBlocked={result['summary']['blockedCustomModuleOccurrenceCount']} "
        f"distributionBlocked={result['summary']['blockedCustomDistributionOccurrenceCount']} "
        "product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F custom handler oracle failed: {error}", file=sys.stderr)
        raise SystemExit(1)
