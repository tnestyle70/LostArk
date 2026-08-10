#!/usr/bin/env python3
"""Compile the reviewed Artist F source-semantics receipt into a typed program.

This is an offline, fail-closed compiler input.  It deliberately preserves blocked
handlers and never grants runtime or Product admission.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

from effect_source_contract_io import load_strict_json_object  # noqa: E402
import build_artist_31470_source_execution_semantics as source_semantics  # noqa: E402


SCHEMA = "lostark.effect-source-runtime-program"
FORMAT_VERSION = 1
EFFECT_ASSET_ID = "effect.artist.skill.31470.f"
FINAL_OWNER_BLOCKER = "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED"

OPCODE_BY_EXACT_CLASS = {
    "particlemoduleacceleration": "ACCELERATION",
    "particlemodulecameraoffset": "CAMERA_OFFSET",
    "particlemodulecolor": "COLOR",
    "particlemodulecoloroverlife": "COLOR_OVER_LIFE",
    "particlemodulecolorscaleoverlife": "COLOR_SCALE_OVER_LIFE",
    "particlemodulelifetime": "LIFETIME",
    "particlemodulelocation": "LOCATION",
    "particlemodulelocationdirect": "LOCATION_DIRECT",
    "particlemodulelocationprimitivecylinder": "LOCATION_PRIMITIVE_CYLINDER",
    "particlemodulelocationprimitivesphere": "LOCATION_PRIMITIVE_SPHERE",
    "particlemodulemeshrotation": "MESH_ROTATION",
    "particlemodulemeshrotationrate": "MESH_ROTATION_RATE",
    "particlemodulemeshrotationratemultiplylife": "MESH_ROTATION_RATE_MULTIPLY_LIFE",
    "particlemodulemeshrotationrateoverlife": "MESH_ROTATION_RATE_OVER_LIFE",
    "particlemoduleorientationaxislock": "ORIENTATION_AXIS_LOCK",
    "particlemoduleparameterdynamic": "PARAMETER_DYNAMIC",
    "particlemodulerequired": "REQUIRED",
    "particlemodulerotation": "ROTATION",
    "particlemodulerotationrate": "ROTATION_RATE",
    "particlemodulesize": "SIZE",
    "particlemodulesizemultiplylife": "SIZE_MULTIPLY_LIFE",
    "particlemodulespawn": "SPAWN",
    "particlemodulespawnperunit": "SPAWN_PER_UNIT",
    "particlemodulesubuv": "SUBUV",
    "particlemoduletypedatamesh": "TYPE_DATA_MESH",
    "particlemoduletypedataribbon": "TYPE_DATA_RIBBON",
    "particlemodulevelocity": "VELOCITY",
}


class ProgramError(ValueError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ProgramError(message)


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


def pretty_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def exact_keys(value: Any, expected: set[str], label: str) -> None:
    require(isinstance(value, dict), f"{label} must be an object")
    require(set(value) == expected, f"{label} fields changed")


def execution_contract(blockers: list[str]) -> dict[str, Any]:
    ordered = sorted(set(blockers))
    return {
        "artifactBindingBlockerSet": [],
        "artifactBindingBlockerCount": 0,
        "executionBlockerSet": ordered,
        "executionBlockerCount": len(ordered),
        "executionAdmission": not ordered,
    }


def payload_unsigned(payload: dict[str, Any]) -> dict[str, Any]:
    return {
        "stableId": payload["stableId"],
        "className": payload["className"],
        "objectPath": payload["objectPath"],
        "literals": payload["literals"],
        "distributions": payload["distributions"],
    }


def runtime_distribution_adapter(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "distributionId": row["distributionId"],
        "payloadDistributionId": row["payloadDistributionId"],
        "evaluatorCapabilityId": row["evaluatorCapabilityId"],
        "decision": row["decision"],
        "blockers": row["blockers"],
    }


def source_index(receipt: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any]]:
    emitters: dict[str, Any] = {}
    modules: dict[str, Any] = {}
    for occurrence in receipt["occurrences"]:
        emitter_id = occurrence["occurrenceCompositeId"]
        require(emitter_id not in emitters, f"duplicate source emitter: {emitter_id}")
        emitters[emitter_id] = occurrence
        for module in occurrence["modules"]:
            module_id = module["moduleOccurrenceId"]
            require(module_id not in modules, f"duplicate source module: {module_id}")
            modules[module_id] = module
    return emitters, modules


def expected_handler_receipts(receipt: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for capability in receipt["handlerCapabilities"]:
        rows.append(
            {
                "handlerId": capability["handlerCapabilityId"],
                "handlerSha256": canonical_sha256(capability),
                "executionContract": execution_contract(capability["blockers"]),
            }
        )
    rows.append(
        {
            "handlerId": "source.program.final-integration-gate.v1",
            "handlerSha256": canonical_sha256(
                {"blocker": FINAL_OWNER_BLOCKER, "effectAssetId": EFFECT_ASSET_ID}
            ),
            "executionContract": execution_contract([FINAL_OWNER_BLOCKER]),
        }
    )
    rows.sort(key=lambda row: row["handlerId"])
    return rows


def build_program(receipt: dict[str, Any], receipt_path: Path) -> dict[str, Any]:
    source_semantics.validate_receipt(receipt)
    capability_by_class = {
        row["exactSourceClass"]: row for row in receipt["handlerCapabilities"]
    }
    require(
        len(capability_by_class) == len(receipt["handlerCapabilities"]),
        "duplicate source handler capability",
    )

    emitters: list[dict[str, Any]] = []
    opcodes: list[dict[str, Any]] = []
    handler_receipts: list[dict[str, Any]] = []
    all_blockers: set[str] = set()
    distribution_count = 0
    ready_opcode_count = 0

    for capability in receipt["handlerCapabilities"]:
        decision = capability["decision"]
        blockers = list(capability["blockers"])
        require(
            decision in {"READY_FOR_HANDLER", "BLOCKED"},
            "unsupported handler capability decision",
        )
        require(
            (decision == "READY_FOR_HANDLER") == (not blockers),
            "handler capability blocker/decision mismatch",
        )
        all_blockers.update(blockers)

    for occurrence in receipt["occurrences"]:
        emitter_id = occurrence["occurrenceCompositeId"]
        emitter_opcodes: list[str] = []
        for expected_order, module in enumerate(occurrence["modules"]):
            require(module["order"] == expected_order, "source module order is not contiguous")
            module_id = module["moduleOccurrenceId"]
            exact_class = module["exactSourceClass"]
            capability = capability_by_class.get(exact_class)
            require(capability is not None, "source module capability is missing")
            require(
                module["handlerCapabilityId"] == capability["handlerCapabilityId"],
                "source module capability identity mismatch",
            )
            payload = copy.deepcopy(module["typedPayload"])
            require(
                canonical_sha256(payload_unsigned(payload)) == payload["payloadSha256"],
                "source module typed payload digest mismatch",
            )
            module_blockers = sorted(set(module["blockers"]))
            require(
                (module["decision"] == "READY_FOR_HANDLER") == (not module_blockers),
                "source module blocker/decision mismatch",
            )
            if module["decision"] == "READY_FOR_HANDLER":
                opcode = OPCODE_BY_EXACT_CLASS.get(exact_class)
                require(opcode is not None, f"ready exact class has no opcode: {exact_class}")
                ready_opcode_count += 1
            else:
                opcode = "EXACT_CLASS_BLOCKED"
            all_blockers.update(module_blockers)
            distribution_count += len(module["distributionAdapters"])
            opcode_row = {
                "opcodeId": module_id,
                "emitterId": emitter_id,
                "order": module["order"],
                "opcode": opcode,
                "handlerId": module["handlerCapabilityId"],
                "exactSourceClass": exact_class,
                "sourceObjectId": module["sourceObjectId"],
                "sourceRecordSha256": module["sourceRecordSha256"],
                "payload": payload,
                "distributionAdapters": [
                    runtime_distribution_adapter(row)
                    for row in module["distributionAdapters"]
                ],
                "seed": copy.deepcopy(module["seed"]),
                "implicitDefaults": copy.deepcopy(module["implicitDefaults"]),
                "decision": module["decision"],
                "blockers": module_blockers,
            }
            opcodes.append(opcode_row)
            emitter_opcodes.append(module_id)
        emitters.append(
            {
                "emitterId": emitter_id,
                "evidenceId": occurrence["evidenceId"],
                "sourceOccurrenceId": occurrence["sourceOccurrenceId"],
                "sourceSystemId": occurrence["sourceSystemId"],
                "sourceEmitterPath": occurrence["sourceEmitterPath"],
                "rendererType": occurrence["rendererType"],
                "selectedLod": copy.deepcopy(occurrence["selectedLod"]),
                "actionCueParameterInputs": copy.deepcopy(
                    occurrence["actionCueParameterInputs"]
                ),
                "orderedOpcodeIds": emitter_opcodes,
            }
        )

    all_blockers.update(receipt["blockerUnion"])
    all_blockers.add(FINAL_OWNER_BLOCKER)
    handler_receipts = expected_handler_receipts(receipt)

    summary = {
        "emitterCount": len(emitters),
        "opcodeCount": len(opcodes),
        "readyOpcodeCount": ready_opcode_count,
        "blockedOpcodeCount": len(opcodes) - ready_opcode_count,
        "distributionCount": distribution_count,
        "handlerReceiptCount": len(handler_receipts),
        "silentFallbackCount": 0,
    }
    program: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "effectAssetId": EFFECT_ASSET_ID,
        "sourceExecutionReceiptSha256": receipt["receiptSha256"],
        "sourceExecutionReceiptCanonicalSha256": canonical_sha256(receipt),
        "sourceExecutionReceiptObservedRawSha256": hashlib.sha256(
            receipt_path.read_bytes()
        ).hexdigest(),
        "runtimeSemanticAuthority": "TYPED_SOURCE_PROGRAM_CANDIDATE",
        "emitters": emitters,
        "opcodes": opcodes,
        "handlerReceipts": handler_receipts,
        "executionContract": execution_contract(sorted(all_blockers)),
        "summary": summary,
        "runtimeExecutionAdmission": False,
        "productAdmission": False,
    }
    program["programSha256"] = canonical_sha256(program)
    validate_program(program, receipt)
    return program


def validate_program(program: dict[str, Any], receipt: dict[str, Any]) -> None:
    source_semantics.validate_receipt(receipt)
    exact_keys(
        program,
        {
            "schema",
            "formatVersion",
            "effectAssetId",
            "sourceExecutionReceiptSha256",
            "sourceExecutionReceiptCanonicalSha256",
            "sourceExecutionReceiptObservedRawSha256",
            "runtimeSemanticAuthority",
            "emitters",
            "opcodes",
            "handlerReceipts",
            "executionContract",
            "summary",
            "runtimeExecutionAdmission",
            "productAdmission",
            "programSha256",
        },
        "source runtime program",
    )
    require(program["schema"] == SCHEMA, "source runtime program schema mismatch")
    require(
        type(program["formatVersion"]) is int
        and program["formatVersion"] == FORMAT_VERSION,
        "source runtime program version mismatch",
    )
    unsigned = copy.deepcopy(program)
    stored_hash = unsigned.pop("programSha256")
    require(canonical_sha256(unsigned) == stored_hash, "program self hash mismatch")
    require(program["effectAssetId"] == EFFECT_ASSET_ID, "program asset mismatch")
    require(
        program["sourceExecutionReceiptSha256"] == receipt["receiptSha256"]
        and program["sourceExecutionReceiptCanonicalSha256"]
        == canonical_sha256(receipt),
        "program source receipt identity mismatch",
    )
    require(
        program["runtimeSemanticAuthority"] == "TYPED_SOURCE_PROGRAM_CANDIDATE",
        "program semantic authority mismatch",
    )
    source_emitters, source_modules = source_index(receipt)
    emitters = {row["emitterId"]: row for row in program["emitters"]}
    opcodes = {row["opcodeId"]: row for row in program["opcodes"]}
    require(len(emitters) == len(program["emitters"]), "duplicate program emitter")
    require(len(opcodes) == len(program["opcodes"]), "duplicate program opcode")
    require(set(emitters) == set(source_emitters), "program emitter coverage mismatch")
    require(set(opcodes) == set(source_modules), "program opcode coverage mismatch")
    require(
        program["handlerReceipts"] == expected_handler_receipts(receipt),
        "program handler receipts drift",
    )

    ready = 0
    distributions = 0
    all_blockers: set[str] = {FINAL_OWNER_BLOCKER}
    for emitter_id, emitter in emitters.items():
        source_emitter = source_emitters[emitter_id]
        expected_ids = [row["moduleOccurrenceId"] for row in source_emitter["modules"]]
        require(emitter["orderedOpcodeIds"] == expected_ids, "program opcode order mismatch")
        for field in (
            "evidenceId",
            "sourceOccurrenceId",
            "sourceSystemId",
            "sourceEmitterPath",
            "rendererType",
            "selectedLod",
            "actionCueParameterInputs",
        ):
            require(emitter[field] == source_emitter[field], f"program emitter {field} drift")
    for opcode_id, opcode in opcodes.items():
        source_module = source_modules[opcode_id]
        source_emitter_id = opcode_id.rsplit("::module:", 1)[0]
        require(opcode["emitterId"] == source_emitter_id, "opcode emitter reassigned")
        for field in (
            "order",
            "handlerCapabilityId",
            "exactSourceClass",
            "sourceObjectId",
            "sourceRecordSha256",
            "typedPayload",
            "seed",
            "implicitDefaults",
            "decision",
            "blockers",
        ):
            program_field = "handlerId" if field == "handlerCapabilityId" else (
                "payload" if field == "typedPayload" else field
            )
            require(opcode[program_field] == source_module[field], f"opcode {field} drift")
        require(
            opcode["distributionAdapters"]
            == [
                runtime_distribution_adapter(row)
                for row in source_module["distributionAdapters"]
            ],
            "opcode distribution adapter drift",
        )
        expected_opcode = (
            OPCODE_BY_EXACT_CLASS.get(source_module["exactSourceClass"])
            if source_module["decision"] == "READY_FOR_HANDLER"
            else "EXACT_CLASS_BLOCKED"
        )
        require(opcode["opcode"] == expected_opcode, "opcode semantic mapping drift")
        require(
            canonical_sha256(payload_unsigned(opcode["payload"]))
            == opcode["payload"]["payloadSha256"],
            "opcode payload digest mismatch",
        )
        distributions += len(opcode["distributionAdapters"])
        all_blockers.update(opcode["blockers"])
        ready += opcode["decision"] == "READY_FOR_HANDLER"

    all_blockers.update(receipt["blockerUnion"])
    expected_contract = execution_contract(sorted(all_blockers))
    require(program["executionContract"] == expected_contract, "program blocker union drift")
    require(
        program["runtimeExecutionAdmission"] is False
        and program["productAdmission"] is False,
        "candidate program admitted runtime or Product",
    )
    expected_summary = {
        "emitterCount": len(source_emitters),
        "opcodeCount": len(source_modules),
        "readyOpcodeCount": ready,
        "blockedOpcodeCount": len(source_modules) - ready,
        "distributionCount": distributions,
        "handlerReceiptCount": len(program["handlerReceipts"]),
        "silentFallbackCount": 0,
    }
    require(program["summary"] == expected_summary, "program summary mismatch")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--source-execution-receipt",
        type=Path,
        default=ROOT
        / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-execution-semantics.receipt.json",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT
        / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-runtime-program.candidate.json",
    )
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    receipt = load_strict_json_object(args.source_execution_receipt)
    program = build_program(receipt, args.source_execution_receipt)
    if args.check:
        current = load_strict_json_object(args.output)
        validate_program(current, receipt)
        require(canonical_bytes(current) == canonical_bytes(program), "program is stale")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_suffix(args.output.suffix + ".tmp")
        temporary.write_bytes(pretty_bytes(program))
        temporary.replace(args.output)
    print(
        "Artist F source runtime program: "
        f"emitters={program['summary']['emitterCount']} "
        f"opcodes={program['summary']['opcodeCount']} "
        f"ready={program['summary']['readyOpcodeCount']} "
        f"blocked={program['summary']['blockedOpcodeCount']} "
        f"distributions={program['summary']['distributionCount']} "
        "runtime=false product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F source runtime program failed: {error}", file=sys.stderr)
        raise SystemExit(1)
