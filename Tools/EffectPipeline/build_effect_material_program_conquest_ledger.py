"""Build the canonical cross-domain Effect Material Program conquest ledger.

The tuple cohort inventory remains the evidence census.  This projection makes
Program work schedulable without promoting representative DXBC, named ABI, a
descriptor, or a renderer candidate into runtime admission.  The JSON artifact
is authoritative; the CSV is a deterministic, derived review surface.

This v1 contract intentionally uses the fail-closed Python validator in this
module instead of a second JSON Schema copy.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import io
import json
import os
import re
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

import build_effect_tuple_cohort_inventory as cohort_inventory
import build_effect_material_program_registry as material_registry


SCHEMA = "lostark.effect-material-program-conquest-ledger"
FORMAT_VERSION = 1
REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
JSON_OUTPUT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/effect-material-program-conquest-ledger.v1.json"
)
CSV_OUTPUT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/effect-material-program-conquest-ledger.v1.csv"
)
SOURCE_BUILDER_RELATIVE_PATH = Path(
    "Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py"
)
SOURCE_ARTIFACT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json"
)
TRANSLATIONS_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/effect-family-hlsl-translations.v1.json"
)
TRANSLATED_SHADER_DIRECTORY = Path("Data/Effects/TranslatedShaders")
PUBLIC_REGISTRY_BUILDER_RELATIVE_PATH = Path(
    "Tools/EffectPipeline/build_effect_material_program_registry.py"
)
PUBLIC_REGISTRY_BASE_RELATIVE_PATH = Path(
    "Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json"
)
PUBLIC_REGISTRY_FRAGMENT_ROOT_RELATIVE_PATH = Path(
    "Data/Effects/MaterialPrograms/Fragments"
)
PUBLIC_EFFECT_CATALOG_RELATIVE_PATH = Path("Data/Effects/EffectCatalog.json")
PUBLIC_DATA_ROOT_RELATIVE_PATH = Path("Data")

DENOMINATORS = (
    "AUTHORED",
    "PRODUCT",
    "FOUR_CHARACTER_PRODUCT",
    "VALTAN_PRODUCT",
)
EXPECTED_DENOMINATOR_COUNTS = {
    "AUTHORED": 7566,
    "PRODUCT": 2554,
    "FOUR_CHARACTER_PRODUCT": 1885,
    "VALTAN_PRODUCT": 669,
}
EXPECTED_LITERAL_TRANSLATION_COUNT = 169
EXPECTED_PRODUCT_DISTINCT_TYPED_PROGRAM_COUNT = 17
EXPECTED_PRODUCT_DISTINCT_EXACT_LITERAL_PROGRAM_COUNT = 111
EXPECTED_PRODUCT_DISTINCT_EXACT_UNTRANSLATED_PROGRAM_COUNT = 1
EXPECTED_PRODUCT_EXACT_UNTRANSLATED_OCCURRENCE_COUNT = 4
EXPECTED_PUBLIC_PROGRAM_ALLOCATION_COUNT = 3
EXPECTED_PUBLIC_BACKEND_OPCODES = {
    ("runtimeMaterialV2", 6),
    ("runtimeMaterialV2", 3),
    ("localDecal", 14),
}
CHARACTER_DOMAINS = {"Artist", "DimensionMaster", "LanceMaster", "Warlord"}

SOURCE_KINDS = (
    "TYPED_RUNTIME_PROGRAM",
    "LITERAL_DXBC_PROGRAM",
    "EXACT_DXBC_UNTRANSLATED",
    "BLOCKED_PROGRAM_EVIDENCE",
    "STATUS_ONLY",
)

# Rank zero is the first implementation/admission tranche.  A rank expresses
# evidence readiness, not Product or visual admission.
STATUS_POLICY: dict[str, dict[str, Any]] = {
    "TYPED_RUNTIME_PROGRAM_DECLARED": {
        "conquestStatus": "RUNTIME_PROGRAM_DECLARED_VERIFY_ADMISSION",
        "priorityRank": 0,
        "priorityCode": "P0_VERIFY_TYPED_RUNTIME_ADMISSION",
        "equationReadiness": "TYPED_RUNTIME_EQUATION_DECLARED",
        "equationReady": True,
    },
    "DXBC_OCCURRENCE_EXACT": {
        "conquestStatus": "EXACT_LITERAL_EQUATION_READY_FOR_LOWERING",
        "priorityRank": 1,
        "priorityCode": "P1_LOWER_EXACT_LITERAL_EQUATION",
        "equationReadiness": "EXACT_DXBC_AND_HLSLI_READY",
        "equationReady": True,
    },
    "DXBC_OCCURRENCE_EXACT_UNTRANSLATED": {
        "conquestStatus": "EXACT_DXBC_TRANSLATION_REQUIRED",
        "priorityRank": 2,
        "priorityCode": "P2_TRANSLATE_EXACT_DXBC",
        "equationReadiness": "EXACT_DXBC_ONLY_TRANSLATION_REQUIRED",
        "equationReady": False,
    },
    "DXBC_FAMILY_REPRESENTATIVE_ONLY": {
        "conquestStatus": "OCCURRENCE_STATIC_PERMUTATION_REQUIRED",
        "priorityRank": 3,
        "priorityCode": "P3_EXTRACT_OCCURRENCE_STATIC_PERMUTATION",
        "equationReadiness": "REPRESENTATIVE_DXBC_NOT_OCCURRENCE_EQUATION",
        "equationReady": False,
    },
    "BOUNDED_SOURCE_PROFILE_ONLY": {
        "conquestStatus": "SOURCE_PROFILE_RECONSTRUCTION_REQUIRED",
        "priorityRank": 4,
        "priorityCode": "P4_RECONSTRUCT_BOUNDED_SOURCE_PROFILE",
        "equationReadiness": "BOUNDED_SOURCE_PROFILE_NO_EQUATION",
        "equationReady": False,
    },
    "NO_PROGRAM_EVIDENCE": {
        "conquestStatus": "PROGRAM_EQUATION_EVIDENCE_REQUIRED",
        "priorityRank": 5,
        "priorityCode": "P5_RECOVER_PROGRAM_EQUATION_EVIDENCE",
        "equationReadiness": "NO_EQUATION_EVIDENCE",
        "equationReady": False,
    },
    "NOT_APPLICABLE_PRESENTATION": {
        "conquestStatus": "NOT_APPLICABLE_PRESENTATION",
        "priorityRank": 9,
        "priorityCode": "NA_PRESENTATION",
        "equationReadiness": "NOT_APPLICABLE_PRESENTATION",
        "equationReady": None,
    },
}

BLOCKER_AXIS_BY_CODE = {
    "CHILD_PARENT_BLOCKED_EXACT_PROMOTION_FORBIDDEN": "PROGRAM",
    "CHILD_PARENT_RESOLUTION_BLOCKED": "PROGRAM",
    "COMPILED_DRAW_DISPATCH_UNPROVEN": "CARRIER",
    "COOKED_CARRIER_MISMATCH": "CARRIER",
    "CURRENT_PACKET_CAPACITY_EXCEEDED": "LAYOUT",
    "DXBC_EXTRACTION_BLOCKED": "PROGRAM",
    "EXACT_VARIANT_ACTUAL_VF_PASS_UNPROVEN": "CARRIER",
    "EXACT_VARIANT_PACKET_TRANSLATION_REQUIRED": "LAYOUT",
    "EXACT_VARIANT_RENDERER_KIND_MISMATCH": "CARRIER",
    "EXACT_VARIANT_RUNTIME_ADMISSION_UNPROVEN": "PRODUCT",
    "EXACT_VARIANT_VISUAL_ADMISSION_UNPROVEN": "PRODUCT",
    "LITERAL_TRANSLATION_MISSING": "PROGRAM",
    "MESH_TYPEDATA_MESH_MODEL_MISSING": "CARRIER",
    "NAMED_ABI_BLOCKED": "LAYOUT",
    "OCCURRENCE_STATIC_PERMUTATION_NOT_EXTRACTED": "PROGRAM",
    "OUTPUT_TOPOLOGY_MRT_UNPROVEN": "STATE",
    "PARENT_UNRESOLVED": "PROGRAM",
    "PRODUCT_CONSUMER_ABSENT": "PRODUCT",
    "PROGRAM_EQUATION_EVIDENCE_ABSENT": "PROGRAM",
    "PROGRAM_EVIDENCE_ABSENT": "PROGRAM",
    "RIBBON_TYPEDATA_COARSE_BUCKET_SPRITE": "CARRIER",
    "RUNTIME_PACKET_NOT_MATERIALIZED": "LAYOUT",
    "SAMPLER_STATE_UNPROVEN": "STATE",
    "SCALAR_VECTOR_PACKING_UNRESOLVED": "LAYOUT",
    "SCENE_INPUTS_UNPROVEN": "STATE",
    "SHADERMAP_ABSENT": "PROGRAM",
    "SOURCE_CARRIER_REPLACEMENT_REQUIRED": "CARRIER",
    "SOURCE_REGISTER_BINDING_UNRESOLVED": "LAYOUT",
    "SOURCE_VALUE_REPLAY_UNPROVEN": "DESCRIPTOR",
    "STAGE_INPUT_SEMANTICS_UNPROVEN": "LAYOUT",
    "TEXTURE_PARAMETER_TO_SLOT_UNRESOLVED": "DESCRIPTOR",
    "TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED": "LAYOUT",
    "VERTEX_FACTORY_UNPROVEN": "CARRIER",
    "WPO_VERTEX_PROGRAM_UNPROVEN": "CARRIER",
}
BLOCKER_AXES = tuple(sorted(set(BLOCKER_AXIS_BY_CODE.values())))

PUBLIC_ID_FORBIDDEN_TOKENS = {
    "artist",
    "dimensionmaster",
    "lancemaster",
    "warlord",
    "valtan",
    "class",
    "skill",
    "boss",
    "occurrence",
    "file",
    "filename",
}

CSV_COLUMNS = (
    "programLedgerId",
    "sourceKind",
    "candidateIdentity",
    "sourceProgramCandidateIds",
    "programEvidenceIds",
    "programStatusCounts",
    "publicAllocationStatus",
    "publicProgramIds",
    "publicLayoutIds",
    "conquestStatus",
    "priorityRank",
    "priorityCode",
    "equationReadiness",
    "equationReady",
    "backend",
    "opcode",
    "opcodeAllocation",
    "dxbcSha256s",
    "hlsliSha256s",
    "functionNames",
    "authoredOccurrenceCount",
    "productOccurrenceCount",
    "fourCharacterProductOccurrenceCount",
    "valtanProductOccurrenceCount",
    "domainOccurrenceCounts",
    "carrierCounts",
    "fineRendererKindCounts",
    "renderProfileCounts",
    "sourceProfileEnabledCounts",
    "layoutStatusCounts",
    "adapterStatusCounts",
    "descriptorStatusCounts",
    "blockerAxisCounts",
    "blockerCounts",
    "occurrenceProjectionSha256",
)

SHA256_RE = re.compile(r"[0-9a-f]{64}")
LEDGER_ID_RE = re.compile(r"material-program\.[0-9a-f]{64}")


class LedgerError(ValueError):
    """A conquest-ledger contract violation detected before commit."""


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise LedgerError(message)


def _reject_non_finite(token: str) -> None:
    raise LedgerError("JSON contains a non-finite number: " + token)


def _object_without_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        _require(key not in result, "JSON object contains duplicate key: " + key)
        result[key] = value
    return result


def _decode_json(payload: bytes, label: str) -> Any:
    try:
        text = payload.decode("utf-8")
    except UnicodeError as error:
        raise LedgerError(label + " is not UTF-8") from error
    try:
        return json.loads(
            text,
            object_pairs_hook=_object_without_duplicate_keys,
            parse_constant=_reject_non_finite,
        )
    except LedgerError:
        raise
    except (TypeError, ValueError) as error:
        raise LedgerError(label + " is not valid JSON") from error


def _seal_current_conventional_inventory(
    repository_root: Path, live_inventory: dict[str, Any]
) -> dict[str, Any]:
    path = repository_root / SOURCE_ARTIFACT_RELATIVE_PATH
    _require(path.is_file(), "conventional tuple inventory artifact is missing")
    payload = path.read_bytes()
    parsed = _decode_json(payload, SOURCE_ARTIFACT_RELATIVE_PATH.as_posix())
    _require(isinstance(parsed, dict), "conventional tuple inventory root must be an object")
    cohort_inventory.validate_inventory(parsed)
    expected = cohort_inventory.pretty_json_bytes(live_inventory)
    _require(
        payload == expected,
        "conventional tuple inventory artifact is stale relative to live build_inventory()",
    )
    _require(
        parsed.get("artifactSha256") == live_inventory.get("artifactSha256"),
        "conventional tuple inventory artifact hash disagrees with live inventory",
    )
    return {
        "conventionalArtifactPath": SOURCE_ARTIFACT_RELATIVE_PATH.as_posix(),
        "conventionalArtifactByteSize": len(payload),
        "conventionalArtifactRawSha256": hashlib.sha256(payload).hexdigest(),
        "conventionalArtifactDeclaredSha256": parsed["artifactSha256"],
        "conventionalArtifactByteCurrent": True,
    }


def _require_public_id_is_class_neutral(value: Any, label: str) -> str:
    _require(isinstance(value, str) and bool(value), label + " must be a non-empty string")
    tokens = {token for token in re.split(r"[._-]+", value.lower()) if token}
    forbidden = sorted(tokens & PUBLIC_ID_FORBIDDEN_TOKENS)
    _require(
        not forbidden,
        label + " contains forbidden class/skill/boss/occurrence/file token(s): "
        + ", ".join(forbidden),
    )
    return value


def _input_file_record(
    repository_root: Path, absolute_path: Path, roles: Iterable[str]
) -> dict[str, Any]:
    resolved_root = repository_root.resolve()
    resolved = absolute_path.resolve(strict=True)
    try:
        relative = resolved.relative_to(resolved_root).as_posix()
    except ValueError as error:
        raise LedgerError("public registry input escapes repository root") from error
    _require(resolved.is_file() and not resolved.is_symlink(), "public registry input is not a regular file: " + relative)
    payload = resolved.read_bytes()
    return {
        "path": relative,
        "roles": sorted(set(roles)),
        "byteSize": len(payload),
        "rawSha256": hashlib.sha256(payload).hexdigest(),
    }


def _build_public_registry_projection(
    repository_root: Path,
) -> tuple[dict[str, Any], dict[tuple[str, int], dict[str, Any]]]:
    base_path = repository_root / PUBLIC_REGISTRY_BASE_RELATIVE_PATH
    fragment_root = repository_root / PUBLIC_REGISTRY_FRAGMENT_ROOT_RELATIVE_PATH
    effect_catalog_path = repository_root / PUBLIC_EFFECT_CATALOG_RELATIVE_PATH
    data_root = repository_root / PUBLIC_DATA_ROOT_RELATIVE_PATH
    registry = material_registry.build_registry(
        base_path,
        effect_catalog_path,
        data_root,
        fragment_root,
    )
    _require(isinstance(registry, dict), "public registry builder returned a non-object")

    programs_by_pair: dict[tuple[str, int], dict[str, Any]] = {}
    programs_by_id: dict[str, dict[str, Any]] = {}
    for program in registry["programs"]:
        program_id = _require_public_id_is_class_neutral(
            program["programId"], "public programId"
        )
        pair = (program["backend"], program["opcode"])
        _require(pair not in programs_by_pair, "public registry aliases one backend/opcode through multiple programIds")
        _require(program_id not in programs_by_id, "public registry contains a duplicate programId")
        programs_by_pair[pair] = program
        programs_by_id[program_id] = program
    _require(
        set(programs_by_pair) == EXPECTED_PUBLIC_BACKEND_OPCODES,
        "public Program allocation is not exactly S6/M3/D14",
    )

    layout_ids = set()
    for layout in registry["layouts"]:
        layout_id = _require_public_id_is_class_neutral(
            layout["layoutId"], "public layoutId"
        )
        _require(layout_id not in layout_ids, "public registry contains a duplicate layoutId")
        layout_ids.add(layout_id)

    layouts_by_program: dict[str, set[str]] = defaultdict(set)
    for binding in registry["bindings"]:
        program_id = binding["programId"]
        layout_id = binding["layoutId"]
        _require(program_id in programs_by_id, "public binding references an unknown programId")
        _require(layout_id in layout_ids, "public binding references an unknown layoutId")
        layouts_by_program[program_id].add(layout_id)
    _require(set(layouts_by_program) == set(programs_by_id), "public Program has no binding/Layout witness")
    _require(
        set().union(*layouts_by_program.values()) == layout_ids,
        "public registry contains an unreferenced Layout alias",
    )

    public_programs: list[dict[str, Any]] = []
    allocations: dict[tuple[str, int], dict[str, Any]] = {}
    for pair in sorted(programs_by_pair):
        program = programs_by_pair[pair]
        program_id = program["programId"]
        allocation = {
            "programIds": [program_id],
            "layoutIds": sorted(layouts_by_program[program_id]),
        }
        allocations[pair] = allocation
        public_programs.append(
            {
                "backend": pair[0],
                "opcode": pair[1],
                **allocation,
            }
        )

    input_roles_by_path: dict[Path, set[str]] = defaultdict(set)
    input_roles_by_path[repository_root / PUBLIC_REGISTRY_BUILDER_RELATIVE_PATH].add(
        "PUBLIC_REGISTRY_BUILDER"
    )
    input_roles_by_path[base_path].add("PUBLIC_REGISTRY_BASE")
    fragment_paths = sorted(fragment_root.glob("*.json"), key=lambda path: path.name)
    _require(bool(fragment_paths), "public registry fragment set is empty")
    for path in fragment_paths:
        input_roles_by_path[path].add("PUBLIC_REGISTRY_FRAGMENT")
    input_roles_by_path[effect_catalog_path].add("PUBLIC_EFFECT_CATALOG")

    effect_catalog = material_registry.load_json(
        effect_catalog_path, "public Effect source catalog", require_lf=False
    )
    bound_effect_ids = {binding["effectAssetId"] for binding in registry["bindings"]}
    authored_paths: dict[str, Path] = {}
    for entry in effect_catalog["effects"]:
        if entry.get("effectAssetId") not in bound_effect_ids:
            continue
        effect_id = entry["effectAssetId"]
        _require(effect_id not in authored_paths, "duplicate bound Effect catalog identity")
        relative = PurePosixPath(entry["authoringPath"])
        authored_paths[effect_id] = data_root.joinpath(*relative.parts)
    _require(set(authored_paths) == bound_effect_ids, "public binding authored input set is incomplete")
    for path in authored_paths.values():
        input_roles_by_path[path].add("PUBLIC_BINDING_AUTHORED_DOCUMENT")

    inputs = [
        _input_file_record(repository_root, path, roles)
        for path, roles in sorted(
            input_roles_by_path.items(), key=lambda item: item[0].as_posix()
        )
    ]
    projection = {
        "builderPath": PUBLIC_REGISTRY_BUILDER_RELATIVE_PATH.as_posix(),
        "basePath": PUBLIC_REGISTRY_BASE_RELATIVE_PATH.as_posix(),
        "fragmentRoot": PUBLIC_REGISTRY_FRAGMENT_ROOT_RELATIVE_PATH.as_posix(),
        "effectCatalogPath": PUBLIC_EFFECT_CATALOG_RELATIVE_PATH.as_posix(),
        "dataRoot": PUBLIC_DATA_ROOT_RELATIVE_PATH.as_posix(),
        "inputs": inputs,
        "registrySha256": canonical_sha256(registry),
        "programCount": len(registry["programs"]),
        "layoutCount": len(registry["layouts"]),
        "bindingCount": len(registry["bindings"]),
        "publicPrograms": public_programs,
    }
    return projection, allocations


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=1, allow_nan=False) + "\n").encode(
        "utf-8"
    )


def _require_sha256(value: Any, label: str) -> str:
    _require(isinstance(value, str) and SHA256_RE.fullmatch(value) is not None, label + " is not SHA-256")
    return value


def _sorted_counter(values: Iterable[str]) -> dict[str, int]:
    return dict(sorted(Counter(values).items()))


def _counter_by_denominator(
    projections: list[dict[str, Any]], field: str
) -> dict[str, dict[str, int]]:
    return {
        denominator: _sorted_counter(
            row[field] for row in projections if denominator in row["denominators"]
        )
        for denominator in DENOMINATORS
    }


def _nested_counter_by_denominator(
    projections: list[dict[str, Any]], values_for_row: Any
) -> dict[str, dict[str, int]]:
    result: dict[str, dict[str, int]] = {}
    for denominator in DENOMINATORS:
        counter: Counter[str] = Counter()
        for row in projections:
            if denominator in row["denominators"]:
                counter.update(values_for_row(row))
        result[denominator] = dict(sorted(counter.items()))
    return result


def _denominators_for_occurrence(occurrence: dict[str, Any]) -> list[str]:
    result = ["AUTHORED"]
    if occurrence["scopeBits"]["productConsumed"]:
        _require(
            occurrence["productStatus"] == "PRODUCT_JOIN_CLOSED",
            "Product denominator contains an open Product join",
        )
        result.append("PRODUCT")
        if occurrence["domain"] == "Valtan":
            result.append("VALTAN_PRODUCT")
        else:
            _require(
                occurrence["domain"] in CHARACTER_DOMAINS,
                "Product occurrence is outside the four-character/Valtan denominator",
            )
            result.append("FOUR_CHARACTER_PRODUCT")
    return result


def _load_literal_translations(repository_root: Path) -> dict[str, dict[str, Any]]:
    path = repository_root / TRANSLATIONS_RELATIVE_PATH
    payload = path.read_bytes()
    parsed = _decode_json(payload, TRANSLATIONS_RELATIVE_PATH.as_posix())
    _require(isinstance(parsed, list), "literal translation catalog root must be an array")
    _require(
        len(parsed) == EXPECTED_LITERAL_TRANSLATION_COUNT,
        "literal translation denominator drifted",
    )
    result: dict[str, dict[str, Any]] = {}
    for row in parsed:
        _require(isinstance(row, dict), "literal translation row must be an object")
        _require(row.get("status") == "TRANSLATED", "literal translation row is not TRANSLATED")
        dxbc_sha = _require_sha256(row.get("dxbcSha256"), "literal DXBC hash")
        hlsli_sha = _require_sha256(row.get("hlslSha256"), "literal HLSLI hash")
        function_name = row.get("functionName")
        _require(isinstance(function_name, str) and bool(function_name), "literal functionName is missing")
        _require(row.get("dxbc") == dxbc_sha + ".dxbc", "literal DXBC filename/hash identity drifted")
        _require(dxbc_sha not in result, "duplicate literal DXBC identity")
        hlsli_path = repository_root / TRANSLATED_SHADER_DIRECTORY / (function_name + ".hlsli")
        _require(hlsli_path.is_file(), "literal HLSLI file is missing: " + hlsli_path.as_posix())
        _require(hashlib.sha256(hlsli_path.read_bytes()).hexdigest() == hlsli_sha, "literal HLSLI file hash disagrees")
        result[dxbc_sha] = {
            "dxbcSha256": dxbc_sha,
            "hlsliSha256": hlsli_sha,
            "functionName": function_name,
        }
    return result


def _program_source_identity(
    occurrence: dict[str, Any],
    candidates: dict[str, dict[str, Any]],
    evidence: dict[str, dict[str, Any]],
    translations: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    program = occurrence["program"]
    if program["programCandidateId"] is not None:
        candidate_id = program["programCandidateId"]
        _require(candidate_id in candidates, "occurrence references an unknown Program candidate")
        candidate = candidates[candidate_id]
        if candidate["kind"] == "TYPED_RUNTIME_PROGRAM":
            return {
                "sourceKind": "TYPED_RUNTIME_PROGRAM",
                "backend": candidate["backend"],
                "opcode": candidate["opcode"],
            }
        _require(candidate["kind"] == "DXBC_LITERAL_TRANSLATION", "unknown Program candidate kind")
        dxbc_sha = candidate["dxbcSha256"]
        _require(dxbc_sha in translations, "literal candidate is absent from translation catalog")
        return {
            "sourceKind": "LITERAL_DXBC_PROGRAM",
            "dxbcSha256": dxbc_sha,
        }
    if program["programEvidenceId"] is not None:
        evidence_id = program["programEvidenceId"]
        _require(evidence_id in evidence, "occurrence references unknown Program evidence")
        evidence_row = evidence[evidence_id]
        if program["status"] == "DXBC_OCCURRENCE_EXACT_UNTRANSLATED":
            dxbc_sha = evidence_row.get("dxbcSha256")
            _require_sha256(dxbc_sha, "exact untranslated DXBC hash")
            if dxbc_sha in translations:
                return {
                    "sourceKind": "LITERAL_DXBC_PROGRAM",
                    "dxbcSha256": dxbc_sha,
                }
            return {
                "sourceKind": "EXACT_DXBC_UNTRANSLATED",
                "dxbcSha256": dxbc_sha,
            }
        return {
            "sourceKind": "BLOCKED_PROGRAM_EVIDENCE",
            "programEvidenceId": evidence_id,
            "programStatus": program["status"],
        }
    return {
        "sourceKind": "STATUS_ONLY",
        "programStatus": program["status"],
    }


def _program_ledger_id(identity: dict[str, Any]) -> str:
    return "material-program." + canonical_sha256(identity)


def _render_profile_for_occurrence(
    occurrence: dict[str, Any], adapters: dict[str, dict[str, Any]]
) -> str:
    adapter_id = occurrence["adapter"]["adapterCandidateId"]
    if adapter_id is None:
        return "UNRESOLVED"
    _require(adapter_id in adapters, "occurrence references an unknown adapter candidate")
    render_profile = adapters[adapter_id].get("renderProfile")
    _require(isinstance(render_profile, str) and bool(render_profile), "adapter renderProfile is missing")
    return render_profile


def _build_occurrence_projection(
    occurrence: dict[str, Any],
    adapters: dict[str, dict[str, Any]],
    candidates: dict[str, dict[str, Any]],
    evidence: dict[str, dict[str, Any]],
    translations: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    identity = _program_source_identity(occurrence, candidates, evidence, translations)
    policy = _policy_for_identity(identity)
    blockers = occurrence["blockers"]
    _require(isinstance(blockers, list) and blockers == sorted(set(blockers)), "source blockers are not sorted and unique")
    unknown_blockers = sorted(set(blockers) - set(BLOCKER_AXIS_BY_CODE))
    _require(not unknown_blockers, "unclassified blocker code(s): " + ", ".join(unknown_blockers))
    return {
        "occurrenceId": occurrence["occurrenceId"],
        "programLedgerId": _program_ledger_id(identity),
        "effectAssetId": occurrence["effectAssetId"],
        "elementId": occurrence["elementId"],
        "domain": occurrence["domain"],
        "denominators": _denominators_for_occurrence(occurrence),
        "carrier": occurrence["carrier"],
        "fineRendererKind": occurrence["fineRendererKind"],
        "renderProfile": _render_profile_for_occurrence(occurrence, adapters),
        "sourceProfileEnabled": occurrence["sourceProfileEnabled"],
        "programStatus": occurrence["program"]["status"],
        "conquestStatus": policy["conquestStatus"],
        "priorityCode": policy["priorityCode"],
        "equationReadiness": policy["equationReadiness"],
        "equationReady": (
            "NOT_APPLICABLE"
            if policy["equationReady"] is None
            else ("READY" if policy["equationReady"] else "BLOCKED")
        ),
        "layoutStatus": occurrence["layout"]["status"],
        "adapterStatus": occurrence["adapter"]["status"],
        "descriptorStatus": occurrence["descriptor"]["status"],
        "blockerCodes": blockers,
    }


def _policy_for_identity(identity: dict[str, Any]) -> dict[str, Any]:
    source_kind = identity["sourceKind"]
    if source_kind == "TYPED_RUNTIME_PROGRAM":
        return STATUS_POLICY["TYPED_RUNTIME_PROGRAM_DECLARED"]
    if source_kind == "LITERAL_DXBC_PROGRAM":
        return STATUS_POLICY["DXBC_OCCURRENCE_EXACT"]
    if source_kind == "EXACT_DXBC_UNTRANSLATED":
        return STATUS_POLICY["DXBC_OCCURRENCE_EXACT_UNTRANSLATED"]
    _require(source_kind in {"BLOCKED_PROGRAM_EVIDENCE", "STATUS_ONLY"}, "unknown Program source identity")
    return STATUS_POLICY[identity["programStatus"]]


def _equation_projection(
    identity: dict[str, Any],
    evidence_rows: list[dict[str, Any]],
    translations: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    policy = _policy_for_identity(identity)
    dxbc_hashes: set[str] = set()
    hlsli_hashes: set[str] = set()
    function_names: set[str] = set()
    backend: str | None = None
    opcode: int | None = None
    opcode_allocation: str

    source_kind = identity["sourceKind"]
    if source_kind == "TYPED_RUNTIME_PROGRAM":
        backend = identity["backend"]
        opcode = identity["opcode"]
        opcode_allocation = "ALLOCATED"
    elif source_kind == "LITERAL_DXBC_PROGRAM":
        dxbc_sha = identity["dxbcSha256"]
        _require(dxbc_sha in translations, "literal Program identity is absent from translation catalog")
        translation = translations[dxbc_sha]
        dxbc_hashes.add(dxbc_sha)
        hlsli_hashes.add(translation["hlsliSha256"])
        function_names.add(translation["functionName"])
        opcode_allocation = "OPCODE_UNALLOCATED"
    elif source_kind == "EXACT_DXBC_UNTRANSLATED":
        dxbc_hashes.add(identity["dxbcSha256"])
        opcode_allocation = "OPCODE_UNALLOCATED"
    elif source_kind == "BLOCKED_PROGRAM_EVIDENCE":
        opcode_allocation = "NOT_ELIGIBLE_EVIDENCE_OPEN"
    else:
        _require(source_kind == "STATUS_ONLY", "unknown Program equation source kind")
        opcode_allocation = "NOT_APPLICABLE"

    for evidence in evidence_rows:
        for field in ("dxbcSha256", "cookedDxbcSha256"):
            value = evidence.get(field)
            if value is not None:
                dxbc_hashes.add(value)

    if source_kind == "TYPED_RUNTIME_PROGRAM":
        _require(not dxbc_hashes and not hlsli_hashes, "typed Program acquired literal shader hashes")
    elif source_kind == "LITERAL_DXBC_PROGRAM":
        _require(len(dxbc_hashes) == 1 and len(hlsli_hashes) == 1 and len(function_names) == 1, "exact equation identity is not singular")
    elif source_kind == "EXACT_DXBC_UNTRANSLATED":
        _require(len(dxbc_hashes) == 1 and not hlsli_hashes, "exact untranslated Program evidence is malformed")
    elif source_kind == "STATUS_ONLY":
        _require(not evidence_rows and not dxbc_hashes, "presentation row contains Program evidence")

    for value in sorted(dxbc_hashes | hlsli_hashes):
        _require_sha256(value, "equation hash")
    return {
        "readiness": policy["equationReadiness"],
        "ready": policy["equationReady"],
        "backend": backend,
        "opcode": opcode,
        "opcodeAllocation": opcode_allocation,
        "dxbcSha256s": sorted(dxbc_hashes),
        "hlsliSha256s": sorted(hlsli_hashes),
        "functionNames": sorted(function_names),
    }


def _public_allocation_projection(
    identity: dict[str, Any],
    public_allocations: dict[tuple[str, int], dict[str, Any]],
) -> dict[str, Any]:
    source_kind = identity["sourceKind"]
    if source_kind == "TYPED_RUNTIME_PROGRAM":
        allocation = public_allocations.get((identity["backend"], identity["opcode"]))
        if allocation is None:
            return {
                "status": "TYPED_OPCODE_ONLY_NOT_PUBLIC_PROGRAM",
                "programIds": [],
                "layoutIds": [],
            }
        return {
            "status": "PUBLIC_PROGRAM_ALLOCATED",
            "programIds": allocation["programIds"],
            "layoutIds": allocation["layoutIds"],
        }
    if source_kind in {"LITERAL_DXBC_PROGRAM", "EXACT_DXBC_UNTRANSLATED"}:
        return {
            "status": "OPCODE_UNALLOCATED",
            "programIds": [],
            "layoutIds": [],
        }
    if source_kind == "BLOCKED_PROGRAM_EVIDENCE":
        return {
            "status": "NOT_ELIGIBLE_EVIDENCE_OPEN",
            "programIds": [],
            "layoutIds": [],
        }
    _require(source_kind == "STATUS_ONLY", "unknown public allocation source kind")
    return {
        "status": "NOT_APPLICABLE",
        "programIds": [],
        "layoutIds": [],
    }


def _projection_identity(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "occurrenceId": row["occurrenceId"],
        "programLedgerId": row["programLedgerId"],
        "denominators": row["denominators"],
    }


def _count_denominators(rows: list[dict[str, Any]]) -> dict[str, int]:
    return {
        denominator: sum(denominator in row["denominators"] for row in rows)
        for denominator in DENOMINATORS
    }


def _aggregate_program_row(
    program_ledger_id: str,
    identity: dict[str, Any],
    occurrence_rows: list[dict[str, Any]],
    source_occurrences: list[dict[str, Any]],
    candidates: dict[str, dict[str, Any]],
    evidence: dict[str, dict[str, Any]],
    translations: dict[str, dict[str, Any]],
    public_allocations: dict[tuple[str, int], dict[str, Any]],
) -> dict[str, Any]:
    _require(len(occurrence_rows) == len(source_occurrences), "mismatched Program group")
    identities = [
        _program_source_identity(row, candidates, evidence, translations)
        for row in source_occurrences
    ]
    _require(all(row == identity for row in identities), "Program ledger group merged different source identities")
    _require(_program_ledger_id(identity) == program_ledger_id, "Program ledger ID is not content-addressed")
    source_kind = identity["sourceKind"]
    source_candidate_ids = sorted(
        {
            row["program"]["programCandidateId"]
            for row in source_occurrences
            if row["program"]["programCandidateId"] is not None
        }
    )
    evidence_ids = sorted(
        {
            row["program"]["programEvidenceId"]
            for row in source_occurrences
            if row["program"]["programEvidenceId"] is not None
        }
    )
    _require(all(value in candidates for value in source_candidate_ids), "Program candidate lookup failed")
    _require(all(value in evidence for value in evidence_ids), "Program evidence lookup failed")
    evidence_rows = [evidence[value] for value in evidence_ids]
    if source_kind == "TYPED_RUNTIME_PROGRAM":
        _require(bool(source_candidate_ids), "typed Program has no source candidate")
        for candidate_id in source_candidate_ids:
            candidate = candidates[candidate_id]
            _require(
                candidate["kind"] == "TYPED_RUNTIME_PROGRAM"
                and candidate["backend"] == identity["backend"]
                and candidate["opcode"] == identity["opcode"],
                "typed source candidate disagrees with class-neutral identity",
            )
    elif source_kind == "LITERAL_DXBC_PROGRAM":
        for candidate_id in source_candidate_ids:
            candidate = candidates[candidate_id]
            _require(candidate["kind"] == "DXBC_LITERAL_TRANSLATION" and candidate["dxbcSha256"] == identity["dxbcSha256"], "literal source candidate disagrees with DXBC identity")
    elif source_kind == "BLOCKED_PROGRAM_EVIDENCE":
        _require(not source_candidate_ids, "blocked evidence row acquired a Program candidate")
        _require(evidence_ids == [identity["programEvidenceId"]], "evidence Program row is not singular")
    elif source_kind == "STATUS_ONLY":
        _require(not evidence_ids and not source_candidate_ids, "status-only Program row contains evidence")
    elif source_kind == "EXACT_DXBC_UNTRANSLATED":
        _require(not source_candidate_ids and bool(evidence_ids), "untranslated DXBC row evidence is malformed")

    policy = _policy_for_identity(identity)
    blocker_counts = Counter(
        blocker for row in occurrence_rows for blocker in row["blockerCodes"]
    )
    blocker_axis_counts: Counter[str] = Counter()
    for blocker, count in blocker_counts.items():
        blocker_axis_counts[BLOCKER_AXIS_BY_CODE[blocker]] += count

    source_profile_counts = Counter(
        "ENABLED" if row["sourceProfileEnabled"] else "DISABLED"
        for row in occurrence_rows
    )
    projection_identity = [_projection_identity(row) for row in occurrence_rows]
    public_allocation = _public_allocation_projection(identity, public_allocations)
    return {
        "programLedgerId": program_ledger_id,
        "sourceKind": source_kind,
        "candidateIdentity": identity,
        "sourceProgramCandidateIds": source_candidate_ids,
        "programEvidenceIds": evidence_ids,
        "programStatusCounts": _sorted_counter(row["programStatus"] for row in occurrence_rows),
        "publicAllocation": public_allocation,
        "conquestStatus": policy["conquestStatus"],
        "priority": {
            "rank": policy["priorityRank"],
            "code": policy["priorityCode"],
        },
        "equation": _equation_projection(identity, evidence_rows, translations),
        "occurrenceCounts": _count_denominators(occurrence_rows),
        "domainOccurrenceCounts": _sorted_counter(row["domain"] for row in occurrence_rows),
        "carrierCounts": _sorted_counter(row["carrier"] for row in occurrence_rows),
        "fineRendererKindCounts": _sorted_counter(row["fineRendererKind"] for row in occurrence_rows),
        "renderProfileCounts": _sorted_counter(row["renderProfile"] for row in occurrence_rows),
        "sourceProfileEnabledCounts": dict(sorted(source_profile_counts.items())),
        "layoutStatusCounts": _sorted_counter(row["layoutStatus"] for row in occurrence_rows),
        "adapterStatusCounts": _sorted_counter(row["adapterStatus"] for row in occurrence_rows),
        "descriptorStatusCounts": _sorted_counter(row["descriptorStatus"] for row in occurrence_rows),
        "blockerAxisCounts": dict(sorted(blocker_axis_counts.items())),
        "blockerCounts": dict(sorted(blocker_counts.items())),
        "occurrenceProjectionSha256": canonical_sha256(projection_identity),
    }


def _priority_code_for_projection(row: dict[str, Any]) -> str:
    return row["priorityCode"]


def _conquest_status_for_projection(row: dict[str, Any]) -> str:
    return row["conquestStatus"]


def _equation_readiness_for_projection(row: dict[str, Any]) -> str:
    return row["equationReadiness"]


def _equation_ready_for_projection(row: dict[str, Any]) -> str:
    return row["equationReady"]


def build_ledger(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    inventory = cohort_inventory.build_inventory(repository_root)
    cohort_inventory.validate_inventory(inventory)
    conventional_inventory_seal = _seal_current_conventional_inventory(
        repository_root, inventory
    )
    translations = _load_literal_translations(repository_root)
    public_registry, public_allocations = _build_public_registry_projection(
        repository_root
    )

    candidates = {row["programCandidateId"]: row for row in inventory["programCandidates"]}
    evidence = {row["programEvidenceId"]: row for row in inventory["programEvidence"]}
    adapters = {row["adapterCandidateId"]: row for row in inventory["adapterCandidates"]}
    _require(len(candidates) == len(inventory["programCandidates"]), "duplicate Program candidate ID in source inventory")
    _require(len(evidence) == len(inventory["programEvidence"]), "duplicate Program evidence ID in source inventory")
    _require(len(adapters) == len(inventory["adapterCandidates"]), "duplicate adapter candidate ID in source inventory")

    source_occurrences = sorted(inventory["occurrences"], key=lambda row: row["occurrenceId"])
    occurrence_projections = [
        _build_occurrence_projection(row, adapters, candidates, evidence, translations)
        for row in source_occurrences
    ]
    _require(
        [row["occurrenceId"] for row in occurrence_projections]
        == sorted(row["occurrenceId"] for row in occurrence_projections),
        "occurrence projections are not sorted",
    )
    denominator_counts = _count_denominators(occurrence_projections)
    _require(
        denominator_counts == EXPECTED_DENOMINATOR_COUNTS,
        "conquest denominator drifted: " + repr(denominator_counts),
    )
    _require(
        denominator_counts["PRODUCT"]
        == denominator_counts["FOUR_CHARACTER_PRODUCT"] + denominator_counts["VALTAN_PRODUCT"],
        "Product denominator is not partitioned by four-character and Valtan scopes",
    )

    projections_by_program: dict[str, list[dict[str, Any]]] = defaultdict(list)
    occurrences_by_program: dict[str, list[dict[str, Any]]] = defaultdict(list)
    identities_by_program: dict[str, dict[str, Any]] = {}

    def register_identity(identity: dict[str, Any]) -> str:
        ledger_id = _program_ledger_id(identity)
        previous = identities_by_program.setdefault(ledger_id, identity)
        _require(previous == identity, "Program identity SHA-256 collision")
        return ledger_id

    for dxbc_sha in sorted(translations):
        register_identity({"sourceKind": "LITERAL_DXBC_PROGRAM", "dxbcSha256": dxbc_sha})
    for occurrence, projection in zip(source_occurrences, occurrence_projections, strict=True):
        identity = _program_source_identity(occurrence, candidates, evidence, translations)
        _require(register_identity(identity) == projection["programLedgerId"], "occurrence Program identity drifted")
        projections_by_program[projection["programLedgerId"]].append(projection)
        occurrences_by_program[projection["programLedgerId"]].append(occurrence)

    program_rows = [
        _aggregate_program_row(
            program_ledger_id,
            identities_by_program[program_ledger_id],
            projections_by_program[program_ledger_id],
            occurrences_by_program[program_ledger_id],
            candidates,
            evidence,
            translations,
            public_allocations,
        )
        for program_ledger_id in sorted(identities_by_program)
    ]
    program_rows.sort(
        key=lambda row: (row["priority"]["rank"], row["programLedgerId"])
    )
    used_candidate_ids = sorted(
        {
            candidate_id
            for row in program_rows
            for candidate_id in row["sourceProgramCandidateIds"]
        }
    )
    _require(len(used_candidate_ids) == len(candidates), "source Program candidate coverage drifted")
    literal_rows = [row for row in program_rows if row["sourceKind"] == "LITERAL_DXBC_PROGRAM"]
    _require(len(literal_rows) == EXPECTED_LITERAL_TRANSLATION_COUNT, "literal Program dedupe denominator drifted")
    _require(all(row["equation"]["opcodeAllocation"] == "OPCODE_UNALLOCATED" for row in literal_rows), "literal Program acquired an invented opcode")
    product_distinct_typed_count = sum(
        row["sourceKind"] == "TYPED_RUNTIME_PROGRAM"
        and row["occurrenceCounts"]["PRODUCT"] > 0
        for row in program_rows
    )
    product_distinct_exact_literal_count = sum(
        row["sourceKind"] == "LITERAL_DXBC_PROGRAM"
        and row["occurrenceCounts"]["PRODUCT"] > 0
        for row in program_rows
    )
    product_exact_untranslated_rows = [
        row
        for row in program_rows
        if row["sourceKind"] == "EXACT_DXBC_UNTRANSLATED"
        and row["occurrenceCounts"]["PRODUCT"] > 0
    ]
    product_exact_untranslated_occurrence_count = sum(
        row["occurrenceCounts"]["PRODUCT"]
        for row in product_exact_untranslated_rows
    )
    public_program_allocation_count = sum(
        row["publicAllocation"]["status"] == "PUBLIC_PROGRAM_ALLOCATED"
        for row in program_rows
    )
    _require(product_distinct_typed_count == EXPECTED_PRODUCT_DISTINCT_TYPED_PROGRAM_COUNT, "Product distinct typed Program denominator drifted")
    _require(product_distinct_exact_literal_count == EXPECTED_PRODUCT_DISTINCT_EXACT_LITERAL_PROGRAM_COUNT, "Product distinct exact literal Program denominator drifted")
    _require(len(product_exact_untranslated_rows) == EXPECTED_PRODUCT_DISTINCT_EXACT_UNTRANSLATED_PROGRAM_COUNT, "Product distinct exact untranslated Program denominator drifted")
    _require(product_exact_untranslated_occurrence_count == EXPECTED_PRODUCT_EXACT_UNTRANSLATED_OCCURRENCE_COUNT, "Product exact untranslated occurrence denominator drifted")
    _require(public_program_allocation_count == EXPECTED_PUBLIC_PROGRAM_ALLOCATION_COUNT, "public Program allocation denominator drifted")

    projection_identity = [_projection_identity(row) for row in occurrence_projections]
    projection_hashes = {
        denominator: canonical_sha256(
            [
                _projection_identity(row)
                for row in occurrence_projections
                if denominator in row["denominators"]
            ]
        )
        for denominator in DENOMINATORS
    }

    artifact: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": "PROGRAM_CANDIDATE_DEDUPED_EVIDENCE_LEDGER_NO_RUNTIME_OR_VISUAL_ADMISSION",
        "inputInventory": {
            "builderPath": SOURCE_BUILDER_RELATIVE_PATH.as_posix(),
            "buildMode": "IN_PROCESS_BUILD_INVENTORY_NOT_ARTIFACT_FILE_READ",
            "schema": inventory["schema"],
            "formatVersion": inventory["formatVersion"],
            "liveArtifactSha256": inventory["artifactSha256"],
            **conventional_inventory_seal,
            "literalTranslationCatalogPath": TRANSLATIONS_RELATIVE_PATH.as_posix(),
            "literalTranslationCatalogSha256": hashlib.sha256(
                (repository_root / TRANSLATIONS_RELATIVE_PATH).read_bytes()
            ).hexdigest(),
        },
        "publicRegistry": public_registry,
        "policies": {
            "authoritativeArtifact": JSON_OUTPUT_RELATIVE_PATH.as_posix(),
            "derivedArtifact": CSV_OUTPUT_RELATIVE_PATH.as_posix(),
            "contractValidation": "PYTHON_FAIL_CLOSED_VALIDATOR_NO_JSON_SCHEMA",
            "programDedupe": "TYPED_BACKEND_OPCODE_OR_LITERAL_DXBC_SHA256_ELSE_BLOCKED_EVIDENCE_ID",
            "programIdentityClassNeutral": "CLASS_SKILL_FILENAME_OCCURRENCE_FORBIDDEN",
            "literalOpcodeAllocation": "ALL_LITERAL_DXBC_PROGRAMS_OPCODE_UNALLOCATED",
            "denominators": {
                "AUTHORED": "ALL_TARGET_AUTHORED_OCCURRENCES",
                "PRODUCT": "SCOPE_BITS_PRODUCT_CONSUMED_AND_PRODUCT_JOIN_CLOSED",
                "FOUR_CHARACTER_PRODUCT": "PRODUCT_IN_ARTIST_DIMENSIONMASTER_LANCEMASTER_WARLORD",
                "VALTAN_PRODUCT": "PRODUCT_IN_VALTAN",
            },
            "priority": "LOWER_RANK_FIRST_EVIDENCE_READINESS_ONLY",
            "programOrdering": "PRIORITY_RANK_THEN_PROGRAM_LEDGER_ID",
            "representativeProgramPromotion": "FORBIDDEN",
            "descriptorPromotion": "FORBIDDEN",
            "adapterPromotion": "FORBIDDEN",
            "runtimeAdmission": "NOT_PROVIDED_BY_THIS_ARTIFACT",
            "visualAdmission": "USER_ONLY",
            "stableProjectionHash": "CANONICAL_SHA256_OF_SORTED_OCCURRENCE_PROGRAM_AND_DENOMINATOR_IDENTITIES",
        },
        "summary": {
            "denominatorCounts": denominator_counts,
            "programRowCount": len(program_rows),
            "sourceProgramCandidateCount": len(used_candidate_ids),
            "typedRuntimeProgramCount": sum(row["sourceKind"] == "TYPED_RUNTIME_PROGRAM" for row in program_rows),
            "literalDxbcProgramCount": len(literal_rows),
            "opcodeUnallocatedLiteralProgramCount": sum(
                row["sourceKind"] == "LITERAL_DXBC_PROGRAM"
                and row["equation"]["opcodeAllocation"] == "OPCODE_UNALLOCATED"
                for row in program_rows
            ),
            "productDistinctTypedProgramCount": product_distinct_typed_count,
            "productDistinctExactLiteralProgramCount": product_distinct_exact_literal_count,
            "productDistinctExactUntranslatedProgramCount": len(
                product_exact_untranslated_rows
            ),
            "productExactUntranslatedOccurrenceCount": (
                product_exact_untranslated_occurrence_count
            ),
            "publicProgramAllocationCount": public_program_allocation_count,
            "publicAllocationStatusCounts": _sorted_counter(
                row["publicAllocation"]["status"] for row in program_rows
            ),
            "programSourceKindCounts": _sorted_counter(row["sourceKind"] for row in program_rows),
            "programStatusCountsByDenominator": _counter_by_denominator(occurrence_projections, "programStatus"),
            "conquestStatusCountsByDenominator": _nested_counter_by_denominator(
                occurrence_projections, lambda row: [_conquest_status_for_projection(row)]
            ),
            "priorityCountsByDenominator": _nested_counter_by_denominator(
                occurrence_projections, lambda row: [_priority_code_for_projection(row)]
            ),
            "equationReadinessCountsByDenominator": _nested_counter_by_denominator(
                occurrence_projections, lambda row: [_equation_readiness_for_projection(row)]
            ),
            "equationReadyCountsByDenominator": _nested_counter_by_denominator(
                occurrence_projections, lambda row: [_equation_ready_for_projection(row)]
            ),
            "carrierCountsByDenominator": _counter_by_denominator(occurrence_projections, "carrier"),
            "renderProfileCountsByDenominator": _counter_by_denominator(occurrence_projections, "renderProfile"),
            "layoutStatusCountsByDenominator": _counter_by_denominator(occurrence_projections, "layoutStatus"),
            "descriptorStatusCountsByDenominator": _counter_by_denominator(occurrence_projections, "descriptorStatus"),
            "blockerAxisCountsByDenominator": _nested_counter_by_denominator(
                occurrence_projections,
                lambda row: [BLOCKER_AXIS_BY_CODE[value] for value in row["blockerCodes"]],
            ),
            "blockerCountsByDenominator": _nested_counter_by_denominator(
                occurrence_projections, lambda row: row["blockerCodes"]
            ),
            "stableProjectionSha256": canonical_sha256(projection_identity),
            "stableProjectionHashesByDenominator": projection_hashes,
            "programRowsSha256": canonical_sha256(program_rows),
        },
        "programs": program_rows,
        "occurrenceProjections": occurrence_projections,
        "transaction": {
            "model": "PARSE_VALIDATE_STAGE_ATOMIC_REPLACE_WITH_PAIR_ROLLBACK",
            "partialCommitAllowed": False,
            "jsonAuthority": True,
            "csvDerived": True,
        },
    }
    artifact["artifactSha256"] = canonical_sha256(artifact)
    validate_ledger(artifact)
    return artifact


def _validate_string_count_map(value: Any, label: str) -> None:
    _require(isinstance(value, dict), label + " must be an object")
    _require(list(value) == sorted(value), label + " keys are not sorted")
    for key, count in value.items():
        _require(isinstance(key, str) and bool(key), label + " has an invalid key")
        _require(isinstance(count, int) and not isinstance(count, bool) and count > 0, label + " has an invalid count")


def _validate_occurrence_projection(row: Any) -> None:
    expected = {
        "occurrenceId", "programLedgerId", "effectAssetId", "elementId", "domain",
        "denominators", "carrier", "fineRendererKind", "renderProfile",
        "sourceProfileEnabled", "programStatus", "conquestStatus", "priorityCode",
        "equationReadiness", "equationReady", "layoutStatus", "adapterStatus",
        "descriptorStatus", "blockerCodes",
    }
    _require(isinstance(row, dict) and set(row) == expected, "occurrence projection fields drifted")
    _require(isinstance(row["occurrenceId"], str) and row["occurrenceId"].startswith("occurrence."), "invalid occurrence ID")
    _require(isinstance(row["programLedgerId"], str) and LEDGER_ID_RE.fullmatch(row["programLedgerId"]) is not None, "invalid Program ledger ID")
    for field in ("effectAssetId", "elementId", "domain", "carrier", "fineRendererKind", "renderProfile", "conquestStatus", "priorityCode", "equationReadiness", "layoutStatus", "adapterStatus", "descriptorStatus"):
        _require(isinstance(row[field], str) and bool(row[field]), "invalid occurrence projection " + field)
    _require(row["programStatus"] in STATUS_POLICY, "unknown Program status in occurrence projection")
    _require(row["equationReady"] in {"READY", "BLOCKED", "NOT_APPLICABLE"}, "invalid occurrence equation-ready state")
    _require(isinstance(row["sourceProfileEnabled"], bool), "sourceProfileEnabled must be boolean")
    _require(isinstance(row["denominators"], list) and row["denominators"], "denominators must be a non-empty list")
    _require(row["denominators"] == [value for value in DENOMINATORS if value in row["denominators"]], "denominators are not canonical")
    _require(row["denominators"][0] == "AUTHORED", "AUTHORED denominator is missing")
    _require(("PRODUCT" in row["denominators"]) == (len(row["denominators"]) == 3), "Product denominator partition is malformed")
    _require(not ({"FOUR_CHARACTER_PRODUCT", "VALTAN_PRODUCT"} <= set(row["denominators"])), "occurrence belongs to both Product slices")
    _require(isinstance(row["blockerCodes"], list) and row["blockerCodes"] == sorted(set(row["blockerCodes"])), "blocker codes are not sorted/unique")
    _require(set(row["blockerCodes"]) <= set(BLOCKER_AXIS_BY_CODE), "occurrence contains an unclassified blocker")


def _validate_program_row(row: Any) -> None:
    expected = {
        "programLedgerId", "sourceKind", "candidateIdentity", "sourceProgramCandidateIds",
        "programEvidenceIds", "programStatusCounts", "publicAllocation", "conquestStatus", "priority", "equation", "occurrenceCounts",
        "domainOccurrenceCounts", "carrierCounts", "fineRendererKindCounts",
        "renderProfileCounts", "sourceProfileEnabledCounts", "layoutStatusCounts",
        "adapterStatusCounts", "descriptorStatusCounts", "blockerAxisCounts",
        "blockerCounts", "occurrenceProjectionSha256",
    }
    _require(isinstance(row, dict) and set(row) == expected, "Program row fields drifted")
    _require(isinstance(row["programLedgerId"], str) and LEDGER_ID_RE.fullmatch(row["programLedgerId"]) is not None, "invalid Program ledger row ID")
    _require(row["sourceKind"] in SOURCE_KINDS, "invalid Program source kind")
    identity = row["candidateIdentity"]
    _require(isinstance(identity, dict) and identity.get("sourceKind") == row["sourceKind"], "candidate identity/source kind disagrees")
    if row["sourceKind"] == "TYPED_RUNTIME_PROGRAM":
        _require(set(identity) == {"sourceKind", "backend", "opcode"}, "typed identity contains non-class-neutral fields")
        _require(isinstance(identity["backend"], str) and bool(identity["backend"]), "typed identity backend is invalid")
        _require(isinstance(identity["opcode"], int) and not isinstance(identity["opcode"], bool) and identity["opcode"] >= 0, "typed identity opcode is invalid")
    elif row["sourceKind"] in {"LITERAL_DXBC_PROGRAM", "EXACT_DXBC_UNTRANSLATED"}:
        _require(set(identity) == {"sourceKind", "dxbcSha256"}, "DXBC identity contains non-class-neutral fields")
        _require_sha256(identity["dxbcSha256"], "candidate identity DXBC hash")
    elif row["sourceKind"] == "BLOCKED_PROGRAM_EVIDENCE":
        _require(set(identity) == {"sourceKind", "programEvidenceId", "programStatus"}, "blocked evidence identity fields drifted")
        _require(isinstance(identity["programEvidenceId"], str) and identity["programEvidenceId"].startswith("program-evidence."), "blocked identity evidence ID is invalid")
        _require(identity["programStatus"] in STATUS_POLICY, "blocked identity Program status is invalid")
    else:
        _require(set(identity) == {"sourceKind", "programStatus"}, "status-only identity fields drifted")
        _require(identity["programStatus"] == "NOT_APPLICABLE_PRESENTATION", "unexpected status-only Program identity")
    _require(_program_ledger_id(identity) == row["programLedgerId"], "Program ledger ID is not its class-neutral candidate identity")
    policy = _policy_for_identity(identity)
    _require(row["conquestStatus"] == policy["conquestStatus"], "Program conquest status disagrees with policy")
    _require(row["priority"] == {"rank": policy["priorityRank"], "code": policy["priorityCode"]}, "Program priority disagrees with policy")
    _require(isinstance(row["sourceProgramCandidateIds"], list) and row["sourceProgramCandidateIds"] == sorted(set(row["sourceProgramCandidateIds"])), "source Program candidate IDs are not sorted/unique")
    _require(all(isinstance(value, str) and value.startswith("program.") for value in row["sourceProgramCandidateIds"]), "invalid source Program candidate ID")
    _require(isinstance(row["programEvidenceIds"], list) and row["programEvidenceIds"] == sorted(set(row["programEvidenceIds"])), "Program evidence IDs are not sorted/unique")
    _require(all(isinstance(value, str) and value.startswith("program-evidence.") for value in row["programEvidenceIds"]), "invalid Program evidence ID")
    if row["sourceKind"] == "TYPED_RUNTIME_PROGRAM":
        _require(bool(row["sourceProgramCandidateIds"]), "typed row has no source candidate")
    elif row["sourceKind"] == "BLOCKED_PROGRAM_EVIDENCE":
        _require(not row["sourceProgramCandidateIds"] and row["programEvidenceIds"] == [identity["programEvidenceId"]], "blocked evidence row identity is malformed")
    elif row["sourceKind"] == "STATUS_ONLY":
        _require(not row["sourceProgramCandidateIds"] and not row["programEvidenceIds"], "status-only row contains evidence")
    _validate_string_count_map(row["programStatusCounts"], "Program row programStatusCounts")
    _require(set(row["programStatusCounts"]) <= set(STATUS_POLICY), "Program row has unknown occurrence status")
    public_allocation = row["publicAllocation"]
    _require(
        isinstance(public_allocation, dict)
        and set(public_allocation) == {"status", "programIds", "layoutIds"},
        "public allocation fields drifted",
    )
    _require(
        isinstance(public_allocation["programIds"], list)
        and public_allocation["programIds"] == sorted(set(public_allocation["programIds"])),
        "public programId list is not sorted/unique",
    )
    _require(
        isinstance(public_allocation["layoutIds"], list)
        and public_allocation["layoutIds"] == sorted(set(public_allocation["layoutIds"])),
        "public layoutId list is not sorted/unique",
    )
    for value in public_allocation["programIds"]:
        _require_public_id_is_class_neutral(value, "Program row public programId")
    for value in public_allocation["layoutIds"]:
        _require_public_id_is_class_neutral(value, "Program row public layoutId")
    if row["sourceKind"] == "TYPED_RUNTIME_PROGRAM":
        _require(
            public_allocation["status"]
            in {"PUBLIC_PROGRAM_ALLOCATED", "TYPED_OPCODE_ONLY_NOT_PUBLIC_PROGRAM"},
            "typed Program public allocation status is invalid",
        )
        _require(
            bool(public_allocation["programIds"])
            == (public_allocation["status"] == "PUBLIC_PROGRAM_ALLOCATED"),
            "typed Program public programId witness disagrees with status",
        )
        _require(
            bool(public_allocation["layoutIds"])
            == (public_allocation["status"] == "PUBLIC_PROGRAM_ALLOCATED"),
            "typed Program public layoutId witness disagrees with status",
        )
    else:
        expected_public_status = {
            "LITERAL_DXBC_PROGRAM": "OPCODE_UNALLOCATED",
            "EXACT_DXBC_UNTRANSLATED": "OPCODE_UNALLOCATED",
            "BLOCKED_PROGRAM_EVIDENCE": "NOT_ELIGIBLE_EVIDENCE_OPEN",
            "STATUS_ONLY": "NOT_APPLICABLE",
        }[row["sourceKind"]]
        _require(public_allocation == {"status": expected_public_status, "programIds": [], "layoutIds": []}, "non-typed Program public allocation drifted")
    equation = row["equation"]
    _require(isinstance(equation, dict) and set(equation) == {"readiness", "ready", "backend", "opcode", "opcodeAllocation", "dxbcSha256s", "hlsliSha256s", "functionNames"}, "equation projection fields drifted")
    _require(equation["readiness"] == policy["equationReadiness"] and equation["ready"] is policy["equationReady"], "equation readiness disagrees with policy")
    for field in ("dxbcSha256s", "hlsliSha256s"):
        _require(isinstance(equation[field], list) and equation[field] == sorted(set(equation[field])), field + " is not sorted/unique")
        for value in equation[field]:
            _require_sha256(value, field)
    _require(isinstance(equation["functionNames"], list) and equation["functionNames"] == sorted(set(equation["functionNames"])), "function names are not sorted/unique")
    if row["sourceKind"] == "LITERAL_DXBC_PROGRAM":
        _require(len(equation["dxbcSha256s"]) == len(equation["hlsliSha256s"]) == len(equation["functionNames"]) == 1, "exact equation hashes are not singular")
        _require(equation["opcodeAllocation"] == "OPCODE_UNALLOCATED", "literal Program opcode was invented")
    elif row["sourceKind"] == "TYPED_RUNTIME_PROGRAM":
        _require(isinstance(equation["backend"], str) and isinstance(equation["opcode"], int), "typed equation backend/opcode is missing")
        _require(equation["opcodeAllocation"] == "ALLOCATED", "typed Program allocation marker drifted")
    else:
        _require(equation["backend"] is None and equation["opcode"] is None, "untyped equation acquired backend/opcode")
        expected_allocation = {
            "EXACT_DXBC_UNTRANSLATED": "OPCODE_UNALLOCATED",
            "BLOCKED_PROGRAM_EVIDENCE": "NOT_ELIGIBLE_EVIDENCE_OPEN",
            "STATUS_ONLY": "NOT_APPLICABLE",
        }[row["sourceKind"]]
        _require(equation["opcodeAllocation"] == expected_allocation, "non-literal opcode allocation marker drifted")
    _require(isinstance(row["occurrenceCounts"], dict) and list(row["occurrenceCounts"]) == list(DENOMINATORS), "Program denominator counts drifted")
    for count in row["occurrenceCounts"].values():
        _require(isinstance(count, int) and not isinstance(count, bool) and count >= 0, "invalid Program denominator count")
    if row["occurrenceCounts"]["AUTHORED"] == 0:
        _require(row["sourceKind"] == "LITERAL_DXBC_PROGRAM", "non-catalog Program row has no authored occurrence")
    for field in (
        "domainOccurrenceCounts", "carrierCounts", "fineRendererKindCounts", "renderProfileCounts",
        "sourceProfileEnabledCounts", "layoutStatusCounts", "adapterStatusCounts",
        "descriptorStatusCounts", "blockerAxisCounts", "blockerCounts",
    ):
        _validate_string_count_map(row[field], "Program row " + field)
    _require(set(row["blockerAxisCounts"]) <= set(BLOCKER_AXES), "unknown blocker axis")
    _require(set(row["blockerCounts"]) <= set(BLOCKER_AXIS_BY_CODE), "unknown blocker count key")
    _require_sha256(row["occurrenceProjectionSha256"], "Program occurrence projection hash")


def _aggregate_program_from_projections(
    row: dict[str, Any], projections: list[dict[str, Any]]
) -> dict[str, Any]:
    blocker_counts = Counter(value for projection in projections for value in projection["blockerCodes"])
    axis_counts: Counter[str] = Counter()
    for blocker, count in blocker_counts.items():
        axis_counts[BLOCKER_AXIS_BY_CODE[blocker]] += count
    return {
        "programStatusCounts": _sorted_counter(value["programStatus"] for value in projections),
        "occurrenceCounts": _count_denominators(projections),
        "domainOccurrenceCounts": _sorted_counter(value["domain"] for value in projections),
        "carrierCounts": _sorted_counter(value["carrier"] for value in projections),
        "fineRendererKindCounts": _sorted_counter(value["fineRendererKind"] for value in projections),
        "renderProfileCounts": _sorted_counter(value["renderProfile"] for value in projections),
        "sourceProfileEnabledCounts": _sorted_counter("ENABLED" if value["sourceProfileEnabled"] else "DISABLED" for value in projections),
        "layoutStatusCounts": _sorted_counter(value["layoutStatus"] for value in projections),
        "adapterStatusCounts": _sorted_counter(value["adapterStatus"] for value in projections),
        "descriptorStatusCounts": _sorted_counter(value["descriptorStatus"] for value in projections),
        "blockerAxisCounts": dict(sorted(axis_counts.items())),
        "blockerCounts": dict(sorted(blocker_counts.items())),
        "occurrenceProjectionSha256": canonical_sha256([_projection_identity(value) for value in projections]),
    }


def _validate_public_registry_projection(
    value: Any,
) -> dict[tuple[str, int], dict[str, Any]]:
    expected_fields = {
        "builderPath",
        "basePath",
        "fragmentRoot",
        "effectCatalogPath",
        "dataRoot",
        "inputs",
        "registrySha256",
        "programCount",
        "layoutCount",
        "bindingCount",
        "publicPrograms",
    }
    _require(isinstance(value, dict) and set(value) == expected_fields, "public registry projection fields drifted")
    _require(value["builderPath"] == PUBLIC_REGISTRY_BUILDER_RELATIVE_PATH.as_posix(), "public registry builder path drifted")
    _require(value["basePath"] == PUBLIC_REGISTRY_BASE_RELATIVE_PATH.as_posix(), "public registry base path drifted")
    _require(value["fragmentRoot"] == PUBLIC_REGISTRY_FRAGMENT_ROOT_RELATIVE_PATH.as_posix(), "public registry fragment root drifted")
    _require(value["effectCatalogPath"] == PUBLIC_EFFECT_CATALOG_RELATIVE_PATH.as_posix(), "public Effect catalog path drifted")
    _require(value["dataRoot"] == PUBLIC_DATA_ROOT_RELATIVE_PATH.as_posix(), "public registry Data root drifted")
    _require_sha256(value["registrySha256"], "public registry projection hash")

    inputs = value["inputs"]
    _require(isinstance(inputs, list) and bool(inputs), "public registry input seal is empty")
    input_paths: list[str] = []
    for row in inputs:
        _require(isinstance(row, dict) and set(row) == {"path", "roles", "byteSize", "rawSha256"}, "public registry input fields drifted")
        _require(isinstance(row["path"], str) and bool(row["path"]), "public registry input path is invalid")
        _require(isinstance(row["roles"], list) and row["roles"] == sorted(set(row["roles"])) and bool(row["roles"]), "public registry input roles are invalid")
        _require(isinstance(row["byteSize"], int) and not isinstance(row["byteSize"], bool) and row["byteSize"] > 0, "public registry input byteSize is invalid")
        _require_sha256(row["rawSha256"], "public registry input raw hash")
        input_paths.append(row["path"])
    _require(input_paths == sorted(set(input_paths)), "public registry inputs are not path-sorted/unique")
    _require(PUBLIC_REGISTRY_BUILDER_RELATIVE_PATH.as_posix() in input_paths, "public registry builder input is not sealed")
    _require(PUBLIC_REGISTRY_BASE_RELATIVE_PATH.as_posix() in input_paths, "public registry base input is not sealed")

    public_programs = value["publicPrograms"]
    _require(isinstance(public_programs, list), "publicPrograms must be an array")
    allocations: dict[tuple[str, int], dict[str, Any]] = {}
    ordered_pairs: list[tuple[str, int]] = []
    all_layout_ids: set[str] = set()
    all_program_ids: set[str] = set()
    for row in public_programs:
        _require(isinstance(row, dict) and set(row) == {"backend", "opcode", "programIds", "layoutIds"}, "public Program projection fields drifted")
        pair = (row["backend"], row["opcode"])
        _require(isinstance(pair[0], str) and bool(pair[0]), "public Program backend is invalid")
        _require(isinstance(pair[1], int) and not isinstance(pair[1], bool) and pair[1] >= 0, "public Program opcode is invalid")
        _require(pair not in allocations, "public backend/opcode alias detected")
        ordered_pairs.append(pair)
        _require(isinstance(row["programIds"], list) and len(row["programIds"]) == 1, "public backend/opcode must have exactly one programId")
        _require(isinstance(row["layoutIds"], list) and row["layoutIds"] == sorted(set(row["layoutIds"])) and bool(row["layoutIds"]), "public Program layout IDs are invalid")
        for program_id in row["programIds"]:
            _require_public_id_is_class_neutral(program_id, "sealed public programId")
            _require(program_id not in all_program_ids, "public programId aliases multiple backend/opcodes")
            all_program_ids.add(program_id)
        for layout_id in row["layoutIds"]:
            _require_public_id_is_class_neutral(layout_id, "sealed public layoutId")
            all_layout_ids.add(layout_id)
        allocations[pair] = {
            "programIds": row["programIds"],
            "layoutIds": row["layoutIds"],
        }
    _require(ordered_pairs == sorted(ordered_pairs), "public Programs are not backend/opcode sorted")
    _require(set(allocations) == EXPECTED_PUBLIC_BACKEND_OPCODES, "sealed public Program set is not S6/M3/D14")
    _require(value["programCount"] == EXPECTED_PUBLIC_PROGRAM_ALLOCATION_COUNT == len(public_programs), "public registry Program count drifted")
    _require(value["layoutCount"] == len(all_layout_ids), "public registry Layout count disagrees")
    _require(isinstance(value["bindingCount"], int) and value["bindingCount"] >= EXPECTED_PUBLIC_PROGRAM_ALLOCATION_COUNT, "public registry binding count is invalid")
    return allocations


def validate_ledger(document: Any) -> None:
    root_fields = {
        "schema", "formatVersion", "identity", "inputInventory", "publicRegistry", "policies", "summary",
        "programs", "occurrenceProjections", "transaction", "artifactSha256",
    }
    _require(isinstance(document, dict) and set(document) == root_fields, "ledger root fields drifted")
    _require(document["schema"] == SCHEMA and document["formatVersion"] == FORMAT_VERSION, "ledger schema/version mismatch")
    _require(document["identity"] == "PROGRAM_CANDIDATE_DEDUPED_EVIDENCE_LEDGER_NO_RUNTIME_OR_VISUAL_ADMISSION", "ledger identity drifted")
    _require(isinstance(document["inputInventory"], dict), "inputInventory must be an object")
    _require(document["inputInventory"].get("builderPath") == SOURCE_BUILDER_RELATIVE_PATH.as_posix(), "input builder path drifted")
    _require(document["inputInventory"].get("buildMode") == "IN_PROCESS_BUILD_INVENTORY_NOT_ARTIFACT_FILE_READ", "input inventory build mode drifted")
    _require(document["inputInventory"].get("conventionalArtifactPath") == SOURCE_ARTIFACT_RELATIVE_PATH.as_posix(), "input artifact path drifted")
    _require_sha256(document["inputInventory"].get("liveArtifactSha256"), "live input inventory artifact hash")
    _require(document["inputInventory"].get("conventionalArtifactByteCurrent") is True, "conventional tuple artifact is not byte-current")
    _require(isinstance(document["inputInventory"].get("conventionalArtifactByteSize"), int) and document["inputInventory"]["conventionalArtifactByteSize"] > 0, "conventional tuple artifact byte size is invalid")
    _require_sha256(document["inputInventory"].get("conventionalArtifactRawSha256"), "conventional tuple artifact raw hash")
    _require_sha256(document["inputInventory"].get("conventionalArtifactDeclaredSha256"), "conventional tuple artifact declared hash")
    _require(document["inputInventory"]["conventionalArtifactDeclaredSha256"] == document["inputInventory"]["liveArtifactSha256"], "conventional/live tuple artifact hashes disagree")
    _require(document["inputInventory"].get("literalTranslationCatalogPath") == TRANSLATIONS_RELATIVE_PATH.as_posix(), "literal translation catalog path drifted")
    _require_sha256(document["inputInventory"].get("literalTranslationCatalogSha256"), "literal translation catalog hash")
    _require(document["policies"].get("contractValidation") == "PYTHON_FAIL_CLOSED_VALIDATOR_NO_JSON_SCHEMA", "validator authority drifted")
    _require(document["policies"].get("programIdentityClassNeutral") == "CLASS_SKILL_FILENAME_OCCURRENCE_FORBIDDEN", "class-neutral Program identity policy drifted")
    _require(document["policies"].get("literalOpcodeAllocation") == "ALL_LITERAL_DXBC_PROGRAMS_OPCODE_UNALLOCATED", "literal opcode allocation policy drifted")
    _require(document["policies"].get("programOrdering") == "PRIORITY_RANK_THEN_PROGRAM_LEDGER_ID", "Program ordering policy drifted")
    _require(document["policies"].get("representativeProgramPromotion") == "FORBIDDEN", "representative Program promotion policy drifted")
    _require(document["policies"].get("runtimeAdmission") == "NOT_PROVIDED_BY_THIS_ARTIFACT", "runtime admission boundary drifted")
    _require(document["policies"].get("visualAdmission") == "USER_ONLY", "visual admission boundary drifted")
    _require(document["transaction"] == {"model": "PARSE_VALIDATE_STAGE_ATOMIC_REPLACE_WITH_PAIR_ROLLBACK", "partialCommitAllowed": False, "jsonAuthority": True, "csvDerived": True}, "transaction contract drifted")
    public_allocations = _validate_public_registry_projection(document["publicRegistry"])

    programs = document["programs"]
    projections = document["occurrenceProjections"]
    _require(isinstance(programs, list) and isinstance(projections, list), "ledger collections must be arrays")
    for row in programs:
        _validate_program_row(row)
    for row in projections:
        _validate_occurrence_projection(row)
    program_ids = [row["programLedgerId"] for row in programs]
    occurrence_ids = [row["occurrenceId"] for row in projections]
    _require(len(program_ids) == len(set(program_ids)), "Program rows are not unique")
    _require(
        program_ids
        == [
            row["programLedgerId"]
            for row in sorted(
                programs,
                key=lambda value: (
                    value["priority"]["rank"],
                    value["programLedgerId"],
                ),
            )
        ],
        "Program rows are not in priority/identity order",
    )
    _require(occurrence_ids == sorted(set(occurrence_ids)), "occurrence projections are not sorted/unique")
    projected_program_ids = {row["programLedgerId"] for row in projections}
    _require(projected_program_ids <= set(program_ids), "occurrence projection references an unknown Program")
    _require(
        all(
            row["programLedgerId"] in projected_program_ids
            or row["sourceKind"] == "LITERAL_DXBC_PROGRAM"
            for row in programs
        ),
        "only catalog literal Programs may have zero occurrences",
    )
    candidate_ids = [
        candidate_id
        for row in programs
        for candidate_id in row["sourceProgramCandidateIds"]
    ]
    _require(len(candidate_ids) == len(set(candidate_ids)), "source Program candidate projects to multiple class-neutral identities")

    by_program: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for projection in projections:
        by_program[projection["programLedgerId"]].append(projection)
    for row in programs:
        aggregate = _aggregate_program_from_projections(row, by_program[row["programLedgerId"]])
        for field, expected in aggregate.items():
            _require(row[field] == expected, "Program aggregate drifted: " + row["programLedgerId"] + " " + field)
        expected_ready = (
            "NOT_APPLICABLE"
            if row["equation"]["ready"] is None
            else ("READY" if row["equation"]["ready"] else "BLOCKED")
        )
        for projection in by_program[row["programLedgerId"]]:
            _require(projection["conquestStatus"] == row["conquestStatus"], "occurrence conquest status disagrees with Program")
            _require(projection["priorityCode"] == row["priority"]["code"], "occurrence priority disagrees with Program")
            _require(projection["equationReadiness"] == row["equation"]["readiness"], "occurrence equation readiness disagrees with Program")
            _require(projection["equationReady"] == expected_ready, "occurrence equation-ready state disagrees with Program")
        _require(
            row["publicAllocation"]
            == _public_allocation_projection(row["candidateIdentity"], public_allocations),
            "Program public allocation disagrees with sealed public registry",
        )

    summary = document["summary"]
    _require(isinstance(summary, dict), "summary must be an object")
    denominator_counts = _count_denominators(projections)
    _require(denominator_counts == EXPECTED_DENOMINATOR_COUNTS, "ledger denominator counts drifted")
    _require(summary.get("denominatorCounts") == denominator_counts, "summary denominator counts disagree")
    _require(summary.get("programRowCount") == len(programs), "summary Program row count disagrees")
    _require(summary.get("sourceProgramCandidateCount") == len(candidate_ids), "summary source candidate count disagrees")
    typed_program_count = sum(row["sourceKind"] == "TYPED_RUNTIME_PROGRAM" for row in programs)
    literal_programs = [row for row in programs if row["sourceKind"] == "LITERAL_DXBC_PROGRAM"]
    _require(summary.get("typedRuntimeProgramCount") == typed_program_count, "summary typed Program count disagrees")
    _require(summary.get("literalDxbcProgramCount") == EXPECTED_LITERAL_TRANSLATION_COUNT == len(literal_programs), "summary literal Program count disagrees")
    _require(summary.get("opcodeUnallocatedLiteralProgramCount") == EXPECTED_LITERAL_TRANSLATION_COUNT, "summary unallocated literal opcode count disagrees")
    _require(all(row["equation"]["opcodeAllocation"] == "OPCODE_UNALLOCATED" for row in literal_programs), "literal opcode allocation is not fail-closed")
    product_distinct_typed_count = sum(
        row["sourceKind"] == "TYPED_RUNTIME_PROGRAM"
        and row["occurrenceCounts"]["PRODUCT"] > 0
        for row in programs
    )
    product_distinct_exact_literal_count = sum(
        row["sourceKind"] == "LITERAL_DXBC_PROGRAM"
        and row["occurrenceCounts"]["PRODUCT"] > 0
        for row in programs
    )
    product_exact_untranslated_rows = [
        row
        for row in programs
        if row["sourceKind"] == "EXACT_DXBC_UNTRANSLATED"
        and row["occurrenceCounts"]["PRODUCT"] > 0
    ]
    product_exact_untranslated_occurrence_count = sum(
        row["occurrenceCounts"]["PRODUCT"]
        for row in product_exact_untranslated_rows
    )
    public_program_allocation_count = sum(
        row["publicAllocation"]["status"] == "PUBLIC_PROGRAM_ALLOCATED"
        for row in programs
    )
    _require(summary.get("productDistinctTypedProgramCount") == EXPECTED_PRODUCT_DISTINCT_TYPED_PROGRAM_COUNT == product_distinct_typed_count, "summary Product distinct typed Program count disagrees")
    _require(summary.get("productDistinctExactLiteralProgramCount") == EXPECTED_PRODUCT_DISTINCT_EXACT_LITERAL_PROGRAM_COUNT == product_distinct_exact_literal_count, "summary Product distinct exact literal Program count disagrees")
    _require(summary.get("productDistinctExactUntranslatedProgramCount") == EXPECTED_PRODUCT_DISTINCT_EXACT_UNTRANSLATED_PROGRAM_COUNT == len(product_exact_untranslated_rows), "summary Product distinct exact untranslated Program count disagrees")
    _require(summary.get("productExactUntranslatedOccurrenceCount") == EXPECTED_PRODUCT_EXACT_UNTRANSLATED_OCCURRENCE_COUNT == product_exact_untranslated_occurrence_count, "summary Product exact untranslated occurrence count disagrees")
    _require(summary.get("publicProgramAllocationCount") == EXPECTED_PUBLIC_PROGRAM_ALLOCATION_COUNT == public_program_allocation_count, "summary public Program allocation count disagrees")
    _require(
        summary.get("publicAllocationStatusCounts")
        == _sorted_counter(row["publicAllocation"]["status"] for row in programs),
        "summary public allocation status counts disagree",
    )
    _require(summary.get("programSourceKindCounts") == _sorted_counter(row["sourceKind"] for row in programs), "summary source-kind counts disagree")
    expected_summary = {
        "programStatusCountsByDenominator": _counter_by_denominator(projections, "programStatus"),
        "conquestStatusCountsByDenominator": _nested_counter_by_denominator(projections, lambda row: [_conquest_status_for_projection(row)]),
        "priorityCountsByDenominator": _nested_counter_by_denominator(projections, lambda row: [_priority_code_for_projection(row)]),
        "equationReadinessCountsByDenominator": _nested_counter_by_denominator(projections, lambda row: [_equation_readiness_for_projection(row)]),
        "equationReadyCountsByDenominator": _nested_counter_by_denominator(projections, lambda row: [_equation_ready_for_projection(row)]),
        "carrierCountsByDenominator": _counter_by_denominator(projections, "carrier"),
        "renderProfileCountsByDenominator": _counter_by_denominator(projections, "renderProfile"),
        "layoutStatusCountsByDenominator": _counter_by_denominator(projections, "layoutStatus"),
        "descriptorStatusCountsByDenominator": _counter_by_denominator(projections, "descriptorStatus"),
        "blockerAxisCountsByDenominator": _nested_counter_by_denominator(projections, lambda row: [BLOCKER_AXIS_BY_CODE[value] for value in row["blockerCodes"]]),
        "blockerCountsByDenominator": _nested_counter_by_denominator(projections, lambda row: row["blockerCodes"]),
    }
    for field, expected in expected_summary.items():
        _require(summary.get(field) == expected, "summary aggregate disagrees: " + field)
    identity = [_projection_identity(row) for row in projections]
    _require(summary.get("stableProjectionSha256") == canonical_sha256(identity), "stable projection hash disagrees")
    expected_projection_hashes = {
        denominator: canonical_sha256([_projection_identity(row) for row in projections if denominator in row["denominators"]])
        for denominator in DENOMINATORS
    }
    _require(summary.get("stableProjectionHashesByDenominator") == expected_projection_hashes, "denominator projection hashes disagree")
    _require(summary.get("programRowsSha256") == canonical_sha256(programs), "Program rows hash disagrees")

    artifact_hash = document["artifactSha256"]
    _require_sha256(artifact_hash, "ledger artifact hash")
    payload = dict(document)
    payload.pop("artifactSha256")
    _require(artifact_hash == canonical_sha256(payload), "ledger artifact hash disagrees")


def _json_cell(value: Any) -> str:
    return canonical_json_bytes(value).decode("utf-8")


def _csv_row(program: dict[str, Any]) -> dict[str, str]:
    equation = program["equation"]
    counts = program["occurrenceCounts"]
    ready = equation["ready"]
    return {
        "programLedgerId": program["programLedgerId"],
        "sourceKind": program["sourceKind"],
        "candidateIdentity": _json_cell(program["candidateIdentity"]),
        "sourceProgramCandidateIds": _json_cell(program["sourceProgramCandidateIds"]),
        "programEvidenceIds": _json_cell(program["programEvidenceIds"]),
        "programStatusCounts": _json_cell(program["programStatusCounts"]),
        "publicAllocationStatus": program["publicAllocation"]["status"],
        "publicProgramIds": _json_cell(program["publicAllocation"]["programIds"]),
        "publicLayoutIds": _json_cell(program["publicAllocation"]["layoutIds"]),
        "conquestStatus": program["conquestStatus"],
        "priorityRank": str(program["priority"]["rank"]),
        "priorityCode": program["priority"]["code"],
        "equationReadiness": equation["readiness"],
        "equationReady": "NOT_APPLICABLE" if ready is None else ("TRUE" if ready else "FALSE"),
        "backend": equation["backend"] or "",
        "opcode": "" if equation["opcode"] is None else str(equation["opcode"]),
        "opcodeAllocation": equation["opcodeAllocation"],
        "dxbcSha256s": _json_cell(equation["dxbcSha256s"]),
        "hlsliSha256s": _json_cell(equation["hlsliSha256s"]),
        "functionNames": _json_cell(equation["functionNames"]),
        "authoredOccurrenceCount": str(counts["AUTHORED"]),
        "productOccurrenceCount": str(counts["PRODUCT"]),
        "fourCharacterProductOccurrenceCount": str(counts["FOUR_CHARACTER_PRODUCT"]),
        "valtanProductOccurrenceCount": str(counts["VALTAN_PRODUCT"]),
        "domainOccurrenceCounts": _json_cell(program["domainOccurrenceCounts"]),
        "carrierCounts": _json_cell(program["carrierCounts"]),
        "fineRendererKindCounts": _json_cell(program["fineRendererKindCounts"]),
        "renderProfileCounts": _json_cell(program["renderProfileCounts"]),
        "sourceProfileEnabledCounts": _json_cell(program["sourceProfileEnabledCounts"]),
        "layoutStatusCounts": _json_cell(program["layoutStatusCounts"]),
        "adapterStatusCounts": _json_cell(program["adapterStatusCounts"]),
        "descriptorStatusCounts": _json_cell(program["descriptorStatusCounts"]),
        "blockerAxisCounts": _json_cell(program["blockerAxisCounts"]),
        "blockerCounts": _json_cell(program["blockerCounts"]),
        "occurrenceProjectionSha256": program["occurrenceProjectionSha256"],
    }


def csv_bytes(document: dict[str, Any]) -> bytes:
    validate_ledger(document)
    stream = io.StringIO(newline="")
    writer = csv.DictWriter(stream, fieldnames=CSV_COLUMNS, lineterminator="\n")
    writer.writeheader()
    for program in document["programs"]:
        writer.writerow(_csv_row(program))
    return stream.getvalue().encode("utf-8")


def validate_csv_bytes(document: dict[str, Any], payload: bytes, label: str = "derived CSV") -> None:
    try:
        text = payload.decode("utf-8")
    except UnicodeError as error:
        raise LedgerError(label + " is not UTF-8") from error
    try:
        reader = csv.DictReader(io.StringIO(text, newline=""))
        _require(tuple(reader.fieldnames or ()) == CSV_COLUMNS, label + " header drifted")
        rows = list(reader)
    except csv.Error as error:
        raise LedgerError(label + " is malformed CSV") from error
    expected = [_csv_row(row) for row in document["programs"]]
    _require(rows == expected, label + " is not the exact JSON-derived projection")


def _stage_bytes(output_path: Path, payload: bytes) -> Path:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    file_descriptor, temporary_name = tempfile.mkstemp(
        prefix=output_path.name + ".",
        suffix=".tmp",
        dir=output_path.parent,
    )
    temporary_path = Path(temporary_name)
    try:
        with os.fdopen(file_descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
    except Exception:
        temporary_path.unlink(missing_ok=True)
        raise
    return temporary_path


def _replace_staged_output(staged_path: Path, output_path: Path) -> None:
    os.replace(staged_path, output_path)


def _restore_output(output_path: Path, original: bytes | None) -> None:
    if original is None:
        output_path.unlink(missing_ok=True)
        return
    staged = _stage_bytes(output_path, original)
    try:
        os.replace(staged, output_path)
    finally:
        staged.unlink(missing_ok=True)


def write_outputs(
    document: dict[str, Any], json_path: Path, csv_path: Path
) -> None:
    validate_ledger(document)
    json_payload = pretty_json_bytes(document)
    csv_payload = csv_bytes(document)
    staged_json: Path | None = None
    staged_csv: Path | None = None
    originals = {
        json_path: json_path.read_bytes() if json_path.is_file() else None,
        csv_path: csv_path.read_bytes() if csv_path.is_file() else None,
    }
    replaced: list[Path] = []
    try:
        staged_json = _stage_bytes(json_path, json_payload)
        staged_csv = _stage_bytes(csv_path, csv_payload)
        parsed = _decode_json(staged_json.read_bytes(), staged_json.name)
        validate_ledger(parsed)
        validate_csv_bytes(parsed, staged_csv.read_bytes(), staged_csv.name)
        _replace_staged_output(staged_json, json_path)
        staged_json = None
        replaced.append(json_path)
        _replace_staged_output(staged_csv, csv_path)
        staged_csv = None
        replaced.append(csv_path)
    except Exception as error:
        restore_errors: list[str] = []
        for output_path in reversed(replaced):
            try:
                _restore_output(output_path, originals[output_path])
            except OSError as restore_error:
                restore_errors.append(str(restore_error))
        if restore_errors:
            raise LedgerError(
                "output replacement failed and rollback failed: "
                + str(error)
                + "; "
                + "; ".join(restore_errors)
            ) from error
        raise
    finally:
        if staged_json is not None:
            staged_json.unlink(missing_ok=True)
        if staged_csv is not None:
            staged_csv.unlink(missing_ok=True)


def run(repository_root: Path, check: bool) -> int:
    repository_root = repository_root.resolve()
    document = build_ledger(repository_root)
    json_path = repository_root / JSON_OUTPUT_RELATIVE_PATH
    csv_path = repository_root / CSV_OUTPUT_RELATIVE_PATH
    expected_json = pretty_json_bytes(document)
    expected_csv = csv_bytes(document)
    if check:
        stale: list[str] = []
        if not json_path.is_file() or json_path.read_bytes() != expected_json:
            stale.append(JSON_OUTPUT_RELATIVE_PATH.as_posix())
        else:
            parsed = _decode_json(json_path.read_bytes(), JSON_OUTPUT_RELATIVE_PATH.as_posix())
            validate_ledger(parsed)
        if not csv_path.is_file() or csv_path.read_bytes() != expected_csv:
            stale.append(CSV_OUTPUT_RELATIVE_PATH.as_posix())
        else:
            validate_csv_bytes(document, csv_path.read_bytes(), CSV_OUTPUT_RELATIVE_PATH.as_posix())
        if stale:
            print("STALE: " + ", ".join(stale), file=sys.stderr)
            return 1
        print(
            "PASS: conquest ledger JSON/CSV are current ("
            + str(document["summary"]["programRowCount"])
            + " Program rows)"
        )
        return 0
    write_outputs(document, json_path, csv_path)
    print(
        "WROTE: conquest ledger JSON/CSV ("
        + str(document["summary"]["programRowCount"])
        + " Program rows; "
        + str(document["summary"]["denominatorCounts"]["AUTHORED"])
        + " authored occurrences)"
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail unless both generated artifacts are byte-current")
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    arguments = parser.parse_args(argv)
    try:
        return run(arguments.repository_root, arguments.check)
    except (LedgerError, cohort_inventory.InventoryError, OSError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
