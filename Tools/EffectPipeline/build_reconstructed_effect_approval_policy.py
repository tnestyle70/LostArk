#!/usr/bin/env python3
"""Build an immutable, fail-closed reconstructed Effect approval policy.

The policy does not reconstruct or execute an Effect.  It joins reviewed Git
objects from independent evidence lanes to a versioned set of permitted
reconstruction families.  Source fidelity is never upgraded by this join.
Version 1 is deliberately non-executable and non-Product; later runtime work
must satisfy the recorded numeric, mutation, resource, and manual gates before
another admission artifact may be created.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path
from typing import Any, Iterable


SOURCE_SCHEMA = "lostark.effect-reconstruction-approval-policy-source"
RECEIPT_SCHEMA = "lostark.effect-reconstruction-approval-policy"
FORMAT_VERSION = 1
EXPECTED_LANES = {"SOURCE", "MATERIAL", "GEOMETRY", "RUNTIME_FOUNDATION"}
POLICY_SOURCE_PATH = (
    "Data/Effects/Policies/Artist/"
    "artist.31470.f.reconstructed-approved-v1.policy-source.json"
)
POLICY_SCHEMA_PATH = (
    "Tools/EffectPipeline/Schemas/"
    "lostark.effect-reconstruction-approval-policy.schema.json"
)
OUTPUT_PATH = (
    "Data/Effects/Policies/Artist/"
    "artist.31470.f.reconstructed-approved-v1.policy.receipt.json"
)

SOURCE_EXECUTION_BLOCKERS = (
    "RECONSTRUCTED_HANDLER_NOT_IMPLEMENTED",
    "INDEPENDENT_MUTATED_OUTPUT_ORACLE_NOT_PASSED",
    "RUNTIME_HANDLER_CONSUMPTION_NOT_PROVEN",
)
MATERIAL_EXECUTION_BLOCKERS = (
    "RECONSTRUCTED_VALUE_OR_STATE_NOT_MATERIALIZED",
    "INDEPENDENT_NUMERIC_OR_STATE_ORACLE_NOT_PASSED",
    "RUNTIME_MATERIAL_CONSUMPTION_NOT_PROVEN",
)
GLOBAL_EXECUTION_BLOCKERS = (
    "R2_TYPED_MATERIALIZATION_NOT_COMPLETE",
    "R3_TYPED_EXECUTOR_NOT_COMPLETE",
    "R4_GEOMETRY_AND_MATERIAL_RUNTIME_BINDING_NOT_COMPLETE",
    "R5_SIX_RENDERER_FAMILIES_NOT_COMPLETE",
    "R6_35_OCCURRENCE_RUNTIME_AND_MANUAL_VALIDATION_NOT_COMPLETE",
    "R7_FREEZE_BUILD_AND_TRANSACTION_REGRESSION_NOT_COMPLETE",
)
GLOBAL_PRODUCT_BLOCKERS = (
    "EXECUTION_ADMISSION_FALSE",
    "PRODUCT_ORACLE_35_OF_35_NOT_PASSED",
    "MANUAL_HUMAN_EYE_VALIDATION_0_OF_35",
    "CATALOG_PUBLISH_TRANSACTION_NOT_RUN",
)

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
GIT_ID_RE = re.compile(r"^[0-9a-f]{40}$")
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.:-]+$")
SAFE_PATH_RE = re.compile(r"^[A-Za-z0-9_.\-/]+$")

V1_POLICY_ID = "artist.31470.f.reconstructed-approved-v1"
V1_TARGET = {
    "characterClass": "ARTIST",
    "skillId": 31470,
    "inputSlot": "F",
    "effectAssetId": "effect.artist.skill.31470.f",
}
V1_DENOMINATORS = {
    "sourceExecutionRowCount": 29,
    "materialRenderStateRowCount": 89,
    "materialStaticPermutationRowCount": 94,
    "materialSamplerRowCount": 77,
    "materialExecutionRowCount": 260,
    "materialArithmeticFamilyCount": 23,
    "geometryCarrierCount": 7,
    "effectOccurrenceCount": 35,
}
V1_FROZEN_LANE_IDENTITIES = {
    "SOURCE": {
        "commitId": "7da937aeaa34c088c694e8eb4f53ff1f7f848ef3",
        "treeId": "e5687551ac558abf63966c84f8cb5b33cf873188",
        "receiptArtifacts": (
            (
                "Data/Effects/Imported/Artist/Candidates/"
                "skill.31470.source-oracle-acquisition.receipt.json",
                "7df6558e8ce13276c3c2abc964a9f86e2ca0008f",
            ),
        ),
        "requiredSourceArtifacts": (),
    },
    "MATERIAL": {
        "commitId": "3ba493de5fde8d058ddee7e0fa0e6c3e466faa43",
        "treeId": "45726d803640038ad6254d5286089bcf5f8c9247",
        "receiptArtifacts": (
            (
                "Data/Effects/Imported/Artist/Materials/"
                "skill.31470.material-source-value-acquisition.receipt.json",
                "c7082098982632b14daa3dea0f2e2549cc752c7f",
            ),
            (
                "Data/Effects/Imported/Artist/Materials/"
                "skill.31470.material-runtime-oracle.receipt.json",
                "ec36b84df38be92832361b72253cf9be39e4da88",
            ),
        ),
        "requiredSourceArtifacts": (),
    },
    "GEOMETRY": {
        "commitId": "0aca792819fdda3f541bb7cec7451c5ed93c6467",
        "treeId": "ef01fb07c1381d2852f5b2c1f58a86b693a55786",
        "receiptArtifacts": (
            (
                "Data/Effects/Imported/Artist/Geometry/"
                "skill.31470.geometry-resource-binding.receipt.json",
                "30d1de331ac0815cb3ac7d129638b16ce373d403",
            ),
        ),
        "requiredSourceArtifacts": (),
    },
    "RUNTIME_FOUNDATION": {
        "commitId": "38ebe7cf7dceb5054bde93812907173cc0f98c67",
        "treeId": "1baecdfc51000380c525cb8041b7b5c3fc505a62",
        "receiptArtifacts": (),
        "requiredSourceArtifacts": (
            ("Client/Public/Effect_RuntimeAuthority.h",
             "65646f785a291a6f6dfdb0112858965b501935d0"),
            ("Client/Private/Effect_RuntimeAuthority.cpp",
             "5a69e4b433e8c28bedd998735f6b6b7985c5b861"),
            ("Client/Public/Effect_Catalog.h",
             "4c9e9ce42e064fab445ac05e76e697fa50e4b8fe"),
            ("Client/Private/Effect_Catalog.cpp",
             "fc76075c8c7d80e5e18719d68c789d5430c390d8"),
            ("Tools/ProjectAudit/Test-EffectRuntimeAuthority.ps1",
             "b8e06f79b95f1e05d42e23bdb5dcfc87979813cc"),
        ),
    },
}
V1_SOURCE_FAMILY_BINDINGS = {
    "UE3_STANDARD_SEEDED_SPAWN_TO_BASE_SPAWN_EX": (
        "RECONSTRUCTED_APPROVED_CURRENT_NATIVE_SEEDED_WRAPPER_V1",
        (
            "SAME_SEED_REPEAT_BIT_STABLE",
            "DIFFERENT_SEED_CHANGES_OUTPUT",
            "BASE_AND_CURRENT_PARTICLE_STATE_MATCH_INDEPENDENT_ORACLE",
        ),
    ),
    "EF_CYLINDER_SPIN_SEEDED_AND_UNSEEDED": (
        "RECONSTRUCTED_APPROVED_EF_CYLINDER_SPIN_V1",
        (
            "SEEDED_AND_UNSEEDED_FIXED_BASIS_OUTPUT",
            "SURFACE_HEIGHT_RADIUS_VELOCITY_ROTATION_MUTATION",
            "SEED_STREAM_ORDER_MUTATION",
        ),
    ),
    "EF_LOCATION_ON_GROUND": (
        "RECONSTRUCTED_APPROVED_GROUND_QUERY_V1",
        (
            "FIXED_GROUND_SCENE_SKIP_ZERO_ONE_AND_NO_HIT",
            "LOCAL_WORLD_TRANSFORM_MUTATION",
            "GROUND_QUERY_FAILURE_ROLLBACK",
        ),
    ),
    "EF_DECAL_TYPEDATA": (
        "RECONSTRUCTED_APPROVED_DECAL_TYPEDATA_V1",
        (
            "DECAL_PROJECTION_ROTATION_SIZE_BLEND_FIXED_TIME",
            "SIGNED_SOURCE_SPACE_MUTATION",
            "INVALID_PROJECTION_ROLLBACK",
        ),
    ),
    "EF_POINT_LIGHT_TYPEDATA": (
        "RECONSTRUCTED_APPROVED_LIGHT_TYPEDATA_V1",
        (
            "LIGHT_BRIGHTNESS_RADIUS_COLOR_FALLOFF_LIFETIME_FIXED_TIME",
            "LIGHT_CDO_FIELD_MUTATION",
            "DEFERRED_ATTENUATION_NUMERIC_ORACLE",
        ),
    ),
    "EF_VELOCITY_OVER_LIFETIME": (
        "RECONSTRUCTED_APPROVED_VELOCITY_OVER_LIFE_V1",
        (
            "LOCAL_AND_WORLD_VELOCITY_FIXED_NORMALIZED_LIFETIME",
            "OWNER_SCALE_AND_ROTATION_MUTATION",
            "FIXED_TIMESTEP_REPEAT_STABLE",
        ),
    ),
    "EF_DISTRIBUTION_VECTOR_MULTIPLY_PARTICLE_PARAMETER": (
        "RECONSTRUCTED_APPROVED_EF_VECTOR_MULTIPLY_V1",
        (
            "PARAMETER_PRESENT_AND_ABSENT_VECTOR_OUTPUT",
            "DIRECT_NORMAL_ABS_MODE_MUTATION",
            "FOUR_RANGE_AND_CONSTANT_FALLBACK_MUTATION",
        ),
    ),
}
V1_MATERIAL_RULE_DOMAIN_SHA256 = {
    "RENDER_STATE": "64b2265e5fa25e210d7aaf6628dc788adda8d4c382748f5f8320c0a94412ebc4",
    "STATIC_PERMUTATION": "86a8e78a09c404fbe3b705581d8cb5e31306567e5a91ba54af485bca5e06e705",
    "SAMPLER": "2e72ef1eca062a6f0d4e1741f27c6b73bb61f9b6a0a978a3b188d6bfb8a44f0f",
}
V1_FORBIDDEN_CLAIMS = (
    "SOURCE_EXACT",
    "SOURCE_EXACT_SAMPLER",
    "SOURCE_EXACT_GRAPH",
    "CURRENT_REVISION_AS_SOURCE_ERA",
    "RECONSTRUCTION_APPROVAL_REMOVES_EVIDENCE_BLOCKER",
    "RAW_SOURCE_RECIPE_IS_RUNTIME_AUTHORITY",
    "PARTIAL_PRODUCT_ADMISSION",
    "AUTOMATED_SCREENSHOT_OR_IMAGE_ORACLE",
)
V1_ALLOWED_PARTIAL_SOURCE_LABELS = {
    "SOURCE_EXACT_OVERRIDE_VALUE_SELECTION_SEMANTICS_UNPROVEN",
    "SOURCE_EXACT_NONOVERRIDE_VALUE_INHERITANCE_SEMANTICS_UNPROVEN",
    "SOURCE_EXACT_PARENT_DEFAULT_VALUE_NATIVE_SELECTION_UNPROVEN",
    "SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS",
    "SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN",
}
V1_ALLOWED_PARTIAL_SOURCE_LABEL_COUNTS = {
    "SOURCE_EXACT_OVERRIDE_VALUE_SELECTION_SEMANTICS_UNPROVEN": 23,
    "SOURCE_EXACT_NONOVERRIDE_VALUE_INHERITANCE_SEMANTICS_UNPROVEN": 43,
    "SOURCE_EXACT_PARENT_DEFAULT_VALUE_NATIVE_SELECTION_UNPROVEN": 28,
    "SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS": 1,
    "SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN": 3,
}
V1_REQUIRED_GLOBAL_GATES = (
    "ALL_29_SOURCE_ROWS_HAVE_VERSIONED_HANDLER_AND_INDEPENDENT_NUMERIC_ORACLE",
    "ALL_260_MATERIAL_ROWS_HAVE_VALUE_SELECTION_STATE_OUTPUT_AND_MUTATION_ORACLE",
    "ALL_23_ARITHMETIC_FAMILIES_PASS_CPU_AND_WARP_INDEPENDENT_SAMPLES",
    "ALL_7_GEOMETRY_BINDINGS_MATCH_EXPECTED_TUPLE_AND_CONSUME_PRESCALE_ONCE",
    "ALL_35_OCCURRENCES_PASS_FIXED_SEED_TIME_RUNTIME_PACKET_ORACLE",
    "ALL_35_OCCURRENCES_PASS_MANUAL_HUMAN_EYE_RUNTIME_VALIDATION",
    "CATALOG_PREPARE_ATTACH_LOAD_PREWARM_HAS_ZERO_RUNTIME_IO_AND_TRANSACTIONAL_ROLLBACK",
    "DEBUG_RELEASE_BUILD_HARNESS_AND_FOCUSED_PROJECT_AUDIT_PASS",
)
V1_ROLLBACK_CONDITIONS = (
    "FROZEN_COMMIT_TREE_BLOB_OR_RECEIPT_IDENTITY_MISMATCH",
    "UPSTREAM_ROW_ID_OR_DENOMINATOR_MISMATCH",
    "EVIDENCE_BLOCKER_OR_FIDELITY_LABEL_LOSS",
    "UNKNOWN_OR_UNVERSIONED_RECONSTRUCTION_FAMILY",
    "NUMERIC_STATE_OR_MUTATION_ORACLE_FAILURE",
    "GEOMETRY_MATERIAL_RESOURCE_OR_PREPARED_IDENTITY_MISMATCH",
    "RUNTIME_ATTACH_REVISION_POINTER_OR_HASH_MISMATCH",
    "MANUAL_OCCURRENCE_VALIDATION_FAILURE",
    "PARTIAL_OR_STALE_PRODUCT_STAGE",
)
V1_POLICY_SECTION_SHA256 = {
    "target": "b947c92093cf8a3a64de30acbad10c3ecdbedd3e39ce59b90441741bf7ec1df7",
    "approvalDecision": "2812912e0d76ecddf365e17ccc619363d32f18302b4a357f6174704baf95075f",
    "frozenInputs": "f268565266e68104201ff61970fd97d56153bae64ab8789bff1311cf6e8c8c7d",
    "denominators": "d3e1377aec856e05ec2428385465934f9a6b5925f2750e65c21974157d1486c8",
    "sourceFamilyPolicies": "a04768f32b2ba9007d74920cd392215ac742914c976c4ff9b43eb240f4c0509e",
    "materialExecutionFamilies": "79cb194c346658f19da6c0826def09c6e41fa97df664a2cc3a4c2769a29e9c1f",
    "materialRowRules": "7a9fdca74acc375519185f28bb7190da6ebf784ddc40beeb508eb5ec88fba824",
    "forbiddenClaims": "cd52f2855294579178687feb5bacede9f8427d3dae79bf40035bc5a0222c4687",
    "requiredGlobalGates": "6e519e675e67edf03f537a25def95abfb2d724088df40fefd24598c98bedf35e",
    "rollbackConditions": "c4bad89887b56bcfea1a8c6f33961ba650f1bac68f61fe0365047d766fb377f9",
}
V1_POLICY_SOURCE_CANONICAL_SHA256 = (
    "944c20dbb287bf98677ca2bece56c616b9b5333d952e3f065a1cc2e305deca67"
)
V1_POLICY_SCHEMA_CANONICAL_SHA256 = (
    "d441711c2a88e58121d7ed3f335bdfa0dc38ed72c522232614ffbda1e8bcce81"
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def require_exact_keys(value: dict[str, Any], expected: Iterable[str], label: str) -> None:
    expected_set = set(expected)
    actual_set = set(value)
    require(
        actual_set == expected_set,
        f"{label} keys changed: missing={sorted(expected_set - actual_set)} "
        f"extra={sorted(actual_set - expected_set)}",
    )


def require_exact_int(value: Any, expected: int, label: str) -> None:
    require(type(value) is int and value == expected, f"{label} must be integer {expected}")


def require_bool(value: Any, expected: bool, label: str) -> None:
    require(type(value) is bool and value is expected, f"{label} must be {expected}")


def require_sha256(value: Any, label: str) -> str:
    require(isinstance(value, str) and SHA256_RE.fullmatch(value) is not None,
            f"{label} must be lowercase SHA-256")
    return value


def require_git_id(value: Any, label: str) -> str:
    require(isinstance(value, str) and GIT_ID_RE.fullmatch(value) is not None,
            f"{label} must be a full lowercase Git object id")
    return value


def require_stable_id(value: Any, label: str) -> str:
    require(isinstance(value, str) and STABLE_ID_RE.fullmatch(value) is not None,
            f"{label} must be a stable identifier")
    return value


def require_safe_path(value: Any, label: str) -> str:
    require(isinstance(value, str) and SAFE_PATH_RE.fullmatch(value) is not None,
            f"{label} must be a repository-relative path")
    parts = value.split("/")
    require(value and not value.startswith("/") and "\\" not in value and ":" not in value,
            f"{label} must be a repository-relative POSIX path")
    require(all(part not in {"", ".", ".."} for part in parts),
            f"{label} contains an unsafe segment")
    return value


def object_without_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def parse_strict_json_bytes(payload: bytes, label: str) -> dict[str, Any]:
    require(not payload.startswith(b"\xef\xbb\xbf"), f"JSON must not contain BOM: {label}")
    try:
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=object_without_duplicate_keys,
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot parse JSON {label}: {exc}") from exc
    require(isinstance(value, dict), f"JSON root must be an object: {label}")
    return value


def load_strict_json_object(path: Path) -> dict[str, Any]:
    require(path.is_file(), f"JSON file is missing: {path}")
    return parse_strict_json_bytes(path.read_bytes(), str(path))


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def pretty_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def canonical_sha256(value: Any) -> str:
    return sha256_bytes(canonical_json_bytes(value))


def tracked_text_sha256(payload: bytes) -> str:
    text = payload.decode("utf-8")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    return sha256_bytes(normalized)


def reseal_receipt(value: dict[str, Any]) -> dict[str, Any]:
    result = copy.deepcopy(value)
    result.pop("receiptSha256", None)
    result["receiptSha256"] = canonical_sha256(result)
    return result


def verify_self_hash(value: dict[str, Any], expected: str, label: str) -> None:
    require_sha256(expected, f"{label}.receiptSha256")
    unsigned = copy.deepcopy(value)
    actual_field = unsigned.pop("receiptSha256", None)
    require(actual_field == expected, f"{label} receipt SHA field differs")
    require(canonical_sha256(unsigned) == expected, f"{label} receipt self-hash differs")


def run_git(root: Path, *arguments: str, binary: bool = False) -> bytes | str:
    completed = subprocess.run(
        ["git", "-C", str(root), *arguments],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if completed.returncode != 0:
        detail = completed.stderr.decode("utf-8", errors="replace").strip()
        raise ValueError(f"git {' '.join(arguments)} failed: {detail}")
    if binary:
        return completed.stdout
    return completed.stdout.decode("utf-8").strip()


def validate_policy_schema_document(schema: dict[str, Any]) -> None:
    require(schema.get("$schema") == "https://json-schema.org/draft/2020-12/schema",
            "policy JSON Schema draft changed")
    require(
        schema.get("$id") ==
        "https://lostark.local/schemas/effect-reconstruction-approval-policy-v1.json",
        "policy JSON Schema id changed",
    )
    require(schema.get("type") == "object", "policy JSON Schema root must be object")
    require(schema.get("additionalProperties") is False,
            "policy JSON Schema root must reject additional properties")
    properties = schema.get("properties")
    require(isinstance(properties, dict), "policy JSON Schema properties missing")
    require(properties.get("schema", {}).get("const") == RECEIPT_SCHEMA,
            "policy JSON Schema receipt schema const changed")
    require(properties.get("formatVersion", {}).get("const") == FORMAT_VERSION,
            "policy JSON Schema format version const changed")
    required = schema.get("required")
    require(isinstance(required, list) and len(required) == len(set(required)),
            "policy JSON Schema required fields invalid")
    require(set(required) == set(properties),
            "policy JSON Schema root must require every declared property")
    validate_closed_schema_node(schema, schema, "$", is_root=True)
    require(canonical_sha256(schema) == V1_POLICY_SCHEMA_CANONICAL_SHA256,
            "V1 policy schema semantic identity changed; create a new policy version")


def validate_closed_schema_node(
    schema: dict[str, Any],
    root_schema: dict[str, Any],
    path: str,
    *,
    is_root: bool = False,
) -> None:
    allowed_keywords = {
        "$schema", "$id", "title", "$defs", "$ref", "type", "const", "enum",
        "pattern", "minLength", "maxLength", "minimum", "minItems", "maxItems",
        "uniqueItems", "items", "additionalProperties", "required", "properties",
    }
    require(set(schema).issubset(allowed_keywords),
            f"unsupported or weakening JSON Schema keyword at {path}")
    if not is_root:
        require("$schema" not in schema and "$id" not in schema and "title" not in schema,
                f"nested JSON Schema metadata is forbidden at {path}")
    if "$ref" in schema:
        require(set(schema) == {"$ref"}, f"JSON Schema reference has siblings at {path}")
        reference = schema["$ref"]
        require(isinstance(reference, str) and reference.startswith("#/$defs/"),
                f"unsupported JSON Schema reference at {path}")
        definition = reference.removeprefix("#/$defs/")
        require(definition in root_schema.get("$defs", {}),
                f"unknown JSON Schema reference at {path}")
        return

    declared_type = schema.get("type")
    type_names = [declared_type] if isinstance(declared_type, str) else declared_type
    if type_names is not None:
        require(isinstance(type_names, list) and type_names and
                all(name in {"object", "array", "string", "integer", "number",
                             "boolean", "null"} for name in type_names),
                f"unsupported JSON Schema type at {path}")
    is_object = declared_type == "object" or "properties" in schema
    if is_object:
        require(schema.get("additionalProperties") is False,
                f"JSON Schema object must reject additional properties at {path}")
        properties = schema.get("properties")
        required = schema.get("required")
        require(isinstance(properties, dict) and isinstance(required, list),
                f"JSON Schema object closure is incomplete at {path}")
        require(len(required) == len(set(required)) and set(required) == set(properties),
                f"JSON Schema required/properties differ at {path}")
        for name, child in properties.items():
            require(isinstance(child, dict), f"JSON Schema property is not object at {path}")
            validate_closed_schema_node(child, root_schema, f"{path}.properties.{name}")
    else:
        require("additionalProperties" not in schema and "required" not in schema and
                "properties" not in schema,
                f"non-object JSON Schema has object keywords at {path}")

    if declared_type == "array":
        require(isinstance(schema.get("items"), dict),
                f"JSON Schema array items missing at {path}")
        validate_closed_schema_node(schema["items"], root_schema, f"{path}.items")
    else:
        require("items" not in schema, f"non-array JSON Schema has items at {path}")

    if "$defs" in schema:
        require(is_root and isinstance(schema["$defs"], dict),
                f"nested JSON Schema definitions are forbidden at {path}")
        for name, definition in schema["$defs"].items():
            require(isinstance(definition, dict), f"JSON Schema definition invalid: {name}")
            validate_closed_schema_node(
                definition, root_schema, f"$.$defs.{name}"
            )


def validate_json_schema_instance(
    value: Any,
    schema: dict[str, Any],
    root_schema: dict[str, Any] | None = None,
    path: str = "$",
) -> None:
    """Validate the closed policy schema subset without an external dependency."""
    if root_schema is None:
        root_schema = schema
    if "$ref" in schema:
        reference = schema["$ref"]
        require(isinstance(reference, str) and reference.startswith("#/$defs/"),
                f"unsupported JSON Schema reference at {path}")
        definition = reference.removeprefix("#/$defs/")
        definitions = root_schema.get("$defs", {})
        require(definition in definitions, f"unknown JSON Schema definition at {path}")
        validate_json_schema_instance(value, definitions[definition], root_schema, path)
        return

    if "const" in schema:
        require(value == schema["const"] and type(value) is type(schema["const"]),
                f"JSON Schema const differs at {path}")
    if "enum" in schema:
        require(any(value == item and type(value) is type(item) for item in schema["enum"]),
                f"JSON Schema enum differs at {path}")

    expected_types = schema.get("type")
    if isinstance(expected_types, str):
        expected_types = [expected_types]
    if isinstance(expected_types, list):
        type_matches = {
            "object": lambda item: isinstance(item, dict),
            "array": lambda item: isinstance(item, list),
            "string": lambda item: isinstance(item, str),
            "integer": lambda item: type(item) is int,
            "number": lambda item: type(item) in {int, float},
            "boolean": lambda item: type(item) is bool,
            "null": lambda item: item is None,
        }
        require(all(name in type_matches for name in expected_types),
                f"unsupported JSON Schema type at {path}")
        require(any(type_matches[name](value) for name in expected_types),
                f"JSON Schema type differs at {path}")

    if isinstance(value, str):
        if "minLength" in schema:
            require(len(value) >= schema["minLength"],
                    f"JSON Schema string too short at {path}")
        if "maxLength" in schema:
            require(len(value) <= schema["maxLength"],
                    f"JSON Schema string too long at {path}")
        if "pattern" in schema:
            require(re.search(schema["pattern"], value) is not None,
                    f"JSON Schema pattern differs at {path}")

    if type(value) in {int, float} and "minimum" in schema:
        require(value >= schema["minimum"], f"JSON Schema minimum differs at {path}")

    if isinstance(value, list):
        if "minItems" in schema:
            require(len(value) >= schema["minItems"],
                    f"JSON Schema array too short at {path}")
        if "maxItems" in schema:
            require(len(value) <= schema["maxItems"],
                    f"JSON Schema array too long at {path}")
        if schema.get("uniqueItems") is True:
            projected = [canonical_json_bytes(item) for item in value]
            require(len(projected) == len(set(projected)),
                    f"JSON Schema array duplicates at {path}")
        item_schema = schema.get("items")
        if isinstance(item_schema, dict):
            for index, item in enumerate(value):
                validate_json_schema_instance(
                    item, item_schema, root_schema, f"{path}[{index}]"
                )

    if isinstance(value, dict):
        required = schema.get("required", [])
        require(all(key in value for key in required),
                f"JSON Schema required property missing at {path}")
        properties = schema.get("properties", {})
        if schema.get("additionalProperties") is False:
            require(set(value).issubset(properties),
                    f"JSON Schema additional property at {path}")
        for key, child_schema in properties.items():
            if key in value:
                validate_json_schema_instance(
                    value[key], child_schema, root_schema, f"{path}.{key}"
                )


def validate_policy_source(source: dict[str, Any]) -> None:
    require_exact_keys(source, (
        "schema", "formatVersion", "policyId", "policyVersion", "target",
        "approvalDecision", "frozenInputs", "denominators",
        "sourceFamilyPolicies", "materialExecutionFamilies", "materialRowRules",
        "forbiddenClaims", "requiredGlobalGates", "rollbackConditions",
    ), "policy source")
    require(source["schema"] == SOURCE_SCHEMA, "policy source schema changed")
    require_exact_int(source["formatVersion"], FORMAT_VERSION, "policy source formatVersion")
    require_exact_int(source["policyVersion"], 1, "policy source policyVersion")
    require_stable_id(source["policyId"], "policyId")

    target = source["target"]
    require(isinstance(target, dict), "policy target must be an object")
    require_exact_keys(target, ("characterClass", "skillId", "inputSlot", "effectAssetId"),
                       "policy target")
    require(isinstance(target["characterClass"], str) and target["characterClass"],
            "target characterClass invalid")
    require(type(target["skillId"]) is int and target["skillId"] > 0,
            "target skillId invalid")
    require(isinstance(target["inputSlot"], str) and target["inputSlot"],
            "target inputSlot invalid")
    require_stable_id(target["effectAssetId"], "target effectAssetId")

    approval = source["approvalDecision"]
    require(isinstance(approval, dict), "approvalDecision must be an object")
    require_exact_keys(approval, (
        "classification", "scope", "sourceFidelityPromotionAllowed",
        "runtimeExecutionGranted", "productAdmissionGranted",
    ), "approvalDecision")
    require(approval["classification"] == "RECONSTRUCTED_APPROVED_V1",
            "approval classification changed")
    require(approval["scope"] == "IMPLEMENTATION_AND_VALIDATION_ROUTE_ONLY",
            "approval scope changed")
    require_bool(approval["sourceFidelityPromotionAllowed"], False,
                 "source fidelity promotion")
    require_bool(approval["runtimeExecutionGranted"], False, "runtime execution grant")
    require_bool(approval["productAdmissionGranted"], False, "Product grant")

    lanes = source["frozenInputs"]
    require(isinstance(lanes, list) and len(lanes) == 4, "four frozen lanes are required")
    lane_ids: list[str] = []
    for lane in lanes:
        require(isinstance(lane, dict), "frozen lane must be an object")
        require_exact_keys(lane, (
            "laneId", "commitId", "treeId", "receiptArtifacts",
            "requiredSourceArtifacts",
        ), "frozen lane")
        lane_id = require_stable_id(lane["laneId"], "laneId")
        lane_ids.append(lane_id)
        require_git_id(lane["commitId"], f"{lane_id}.commitId")
        require_git_id(lane["treeId"], f"{lane_id}.treeId")
        for artifact in lane["receiptArtifacts"]:
            require(isinstance(artifact, dict), "receipt artifact must be an object")
            require_exact_keys(artifact, (
                "path", "blobId", "schema", "formatVersion", "receiptSha256",
            ), "receipt artifact")
            require_safe_path(artifact["path"], "receipt artifact path")
            require_git_id(artifact["blobId"], "receipt artifact blobId")
            require(isinstance(artifact["schema"], str) and artifact["schema"],
                    "receipt artifact schema invalid")
            require(type(artifact["formatVersion"]) is int,
                    "receipt artifact formatVersion must be integer")
            require_sha256(artifact["receiptSha256"], "receipt artifact receiptSha256")
        for artifact in lane["requiredSourceArtifacts"]:
            require(isinstance(artifact, dict), "source artifact must be an object")
            require_exact_keys(artifact, ("path", "blobId"), "source artifact")
            require_safe_path(artifact["path"], "source artifact path")
            require_git_id(artifact["blobId"], "source artifact blobId")
    require(len(lane_ids) == len(set(lane_ids)) and set(lane_ids) == EXPECTED_LANES,
            "frozen lane set changed")

    denominators = source["denominators"]
    require(isinstance(denominators, dict), "denominators must be an object")
    require_exact_keys(denominators, (
        "sourceExecutionRowCount", "materialRenderStateRowCount",
        "materialStaticPermutationRowCount", "materialSamplerRowCount",
        "materialExecutionRowCount", "materialArithmeticFamilyCount",
        "geometryCarrierCount", "effectOccurrenceCount",
    ), "denominators")
    for name, value in denominators.items():
        require(type(value) is int and value > 0, f"denominator {name} must be positive integer")
    require(
        denominators["materialExecutionRowCount"] ==
        denominators["materialRenderStateRowCount"] +
        denominators["materialStaticPermutationRowCount"] +
        denominators["materialSamplerRowCount"],
        "Material denominator sum changed",
    )

    source_families = source["sourceFamilyPolicies"]
    require(isinstance(source_families, list) and source_families,
            "source family policies are required")
    for family in source_families:
        require_exact_keys(family, (
            "upstreamNativeFamily", "policyFamilyId", "closureBasis",
            "requiredOracleIds",
        ), "source family")
        require_stable_id(family["upstreamNativeFamily"], "upstream native family")
        require_stable_id(family["policyFamilyId"], "source policy family")
        require_stable_id(family["closureBasis"], "source closure basis")
        validate_unique_stable_ids(family["requiredOracleIds"], "source oracle ids")
    require_unique(source_families, "upstreamNativeFamily", "source upstream families")
    require_unique(source_families, "policyFamilyId", "source policy families")

    material_families = source["materialExecutionFamilies"]
    require(isinstance(material_families, list) and material_families,
            "material execution families are required")
    for family in material_families:
        require_exact_keys(family, ("policyFamilyId", "closureBasis", "requiredOracleIds"),
                           "material family")
        require_stable_id(family["policyFamilyId"], "material policy family")
        require_stable_id(family["closureBasis"], "material closure basis")
        validate_unique_stable_ids(family["requiredOracleIds"], "material oracle ids")
    require_unique(material_families, "policyFamilyId", "material policy families")
    material_family_ids = {row["policyFamilyId"] for row in material_families}
    require("RECONSTRUCTED_APPROVED_ARITHMETIC_V1" in material_family_ids,
            "arithmetic reconstruction family is missing")

    rules = source["materialRowRules"]
    require(isinstance(rules, list) and rules, "material row rules are required")
    for rule in rules:
        require_exact_keys(rule, (
            "domain", "conditions", "policyFamilyId", "evidenceFidelity",
        ), "material row rule")
        require(rule["domain"] in {"RENDER_STATE", "STATIC_PERMUTATION", "SAMPLER"},
                "material row rule domain invalid")
        require(rule["policyFamilyId"] in material_family_ids,
                "material row rule references unknown family")
        require_stable_id(rule["evidenceFidelity"], "material evidence fidelity")
        require(isinstance(rule["conditions"], list) and rule["conditions"],
                "material row rule conditions missing")
        for condition in rule["conditions"]:
            require_exact_keys(condition, ("path", "equals"), "material rule condition")
            require(isinstance(condition["path"], str) and condition["path"],
                    "material rule condition path invalid")

    validate_unique_stable_ids(source["forbiddenClaims"], "forbidden claims")
    validate_unique_stable_ids(source["requiredGlobalGates"], "required global gates")
    validate_unique_stable_ids(source["rollbackConditions"], "rollback conditions")
    require("SOURCE_EXACT" in source["forbiddenClaims"], "SOURCE_EXACT must be forbidden")
    require("SOURCE_EXACT_SAMPLER" in source["forbiddenClaims"],
            "SOURCE_EXACT_SAMPLER must be forbidden")
    require("PARTIAL_PRODUCT_ADMISSION" in source["forbiddenClaims"],
            "partial Product admission must be forbidden")
    validate_v1_production_invariants(source)


def validate_v1_production_invariants(source: dict[str, Any]) -> None:
    require(source["policyId"] == V1_POLICY_ID, "V1 policy id changed")
    require(source["policyVersion"] == 1, "V1 policy version changed")
    require(source["target"] == V1_TARGET, "V1 target identity changed")
    require(source["denominators"] == V1_DENOMINATORS, "V1 denominators changed")
    require(tuple(source["forbiddenClaims"]) == V1_FORBIDDEN_CLAIMS,
            "V1 forbidden fidelity contract changed")
    require(tuple(source["requiredGlobalGates"]) == V1_REQUIRED_GLOBAL_GATES,
            "V1 global admission gates changed")
    require(tuple(source["rollbackConditions"]) == V1_ROLLBACK_CONDITIONS,
            "V1 rollback contract changed")

    actual_lanes: dict[str, dict[str, Any]] = {}
    for lane in source["frozenInputs"]:
        actual_lanes[lane["laneId"]] = {
            "commitId": lane["commitId"],
            "treeId": lane["treeId"],
            "receiptArtifacts": tuple(
                (row["path"], row["blobId"]) for row in lane["receiptArtifacts"]
            ),
            "requiredSourceArtifacts": tuple(
                (row["path"], row["blobId"])
                for row in lane["requiredSourceArtifacts"]
            ),
        }
    require(actual_lanes == V1_FROZEN_LANE_IDENTITIES,
            "V1 frozen lane commit/tree/path/blob table changed")

    actual_source_bindings = {
        row["upstreamNativeFamily"]: (
            row["policyFamilyId"], tuple(row["requiredOracleIds"])
        )
        for row in source["sourceFamilyPolicies"]
    }
    require(actual_source_bindings == V1_SOURCE_FAMILY_BINDINGS,
            "V1 Source family/oracle mapping changed")

    actual_material_domains = {
        domain: [
            row for row in source["materialRowRules"] if row["domain"] == domain
        ]
        for domain in V1_MATERIAL_RULE_DOMAIN_SHA256
    }
    require(all(actual_material_domains.values()) and
            sum(map(len, actual_material_domains.values())) ==
            len(source["materialRowRules"]),
            "V1 Material domain rule coverage changed")
    for domain, expected_sha in V1_MATERIAL_RULE_DOMAIN_SHA256.items():
        require(canonical_sha256(actual_material_domains[domain]) == expected_sha,
                f"V1 Material rule/fidelity mapping changed: {domain}")

    for section, expected_sha in V1_POLICY_SECTION_SHA256.items():
        require(canonical_sha256(source[section]) == expected_sha,
                f"V1 policy semantic section changed: {section}")
    require(canonical_sha256(source) == V1_POLICY_SOURCE_CANONICAL_SHA256,
            "V1 policy source semantic identity changed; create a new policy version")


def validate_unique_stable_ids(values: Any, label: str) -> None:
    require(isinstance(values, list) and values, f"{label} must be a nonempty array")
    for value in values:
        require_stable_id(value, label)
    require(len(values) == len(set(values)), f"{label} contains duplicates")


def require_unique(rows: list[dict[str, Any]], field: str, label: str) -> None:
    values = [row[field] for row in rows]
    require(len(values) == len(set(values)), f"{label} contains duplicates")


def verify_frozen_inputs(
    root: Path,
    lanes: list[dict[str, Any]],
) -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]]]:
    resolved: list[dict[str, Any]] = []
    receipts_by_schema: dict[str, dict[str, Any]] = {}
    for lane in lanes:
        lane_id = lane["laneId"]
        commit_id = lane["commitId"]
        actual_commit = run_git(root, "rev-parse", f"{commit_id}^{{commit}}")
        require(actual_commit == commit_id, f"{lane_id} frozen commit differs")
        actual_tree = run_git(root, "show", "-s", "--format=%T", commit_id)
        require(actual_tree == lane["treeId"], f"{lane_id} frozen tree differs")

        receipt_artifacts: list[dict[str, Any]] = []
        for expected in lane["receiptArtifacts"]:
            path = expected["path"]
            blob_id = run_git(root, "rev-parse", f"{commit_id}:{path}")
            require(blob_id == expected["blobId"], f"{lane_id} receipt blob differs: {path}")
            payload = run_git(root, "cat-file", "blob", blob_id, binary=True)
            assert isinstance(payload, bytes)
            receipt = parse_strict_json_bytes(payload, f"{commit_id}:{path}")
            require(receipt.get("schema") == expected["schema"],
                    f"{lane_id} receipt schema differs: {path}")
            require_exact_int(receipt.get("formatVersion"), expected["formatVersion"],
                              f"{lane_id} receipt formatVersion")
            verify_self_hash(receipt, expected["receiptSha256"], f"{lane_id}:{path}")
            require(expected["schema"] not in receipts_by_schema,
                    f"duplicate frozen receipt schema: {expected['schema']}")
            receipts_by_schema[expected["schema"]] = receipt
            receipt_artifacts.append({
                "path": path,
                "gitBlobId": blob_id,
                "rawByteCount": len(payload),
                "rawSha256": sha256_bytes(payload),
                "canonicalJsonSha256": canonical_sha256(receipt),
                "schema": receipt["schema"],
                "formatVersion": receipt["formatVersion"],
                "receiptSha256": receipt["receiptSha256"],
            })

        source_artifacts: list[dict[str, Any]] = []
        for expected in lane["requiredSourceArtifacts"]:
            path = expected["path"]
            blob_id = run_git(root, "rev-parse", f"{commit_id}:{path}")
            require(blob_id == expected["blobId"], f"{lane_id} source blob differs: {path}")
            payload = run_git(root, "cat-file", "blob", blob_id, binary=True)
            assert isinstance(payload, bytes)
            source_artifacts.append({
                "path": path,
                "gitBlobId": blob_id,
                "rawByteCount": len(payload),
                "rawSha256": sha256_bytes(payload),
                "trackedTextSha256": tracked_text_sha256(payload),
            })

        resolved.append({
            "laneId": lane_id,
            "commitId": commit_id,
            "treeId": lane["treeId"],
            "identityRole": "REVIEWED_FROZEN_GIT_TREE",
            "receiptArtifacts": receipt_artifacts,
            "requiredSourceArtifacts": source_artifacts,
        })
    return sorted(resolved, key=lambda row: row["laneId"]), receipts_by_schema


def assert_target(receipt: dict[str, Any], target: dict[str, Any], label: str) -> None:
    character = str(receipt.get("characterClass", "")).upper()
    require(character == target["characterClass"].upper(), f"{label} character class differs")
    require(type(receipt.get("skillId")) is int and receipt["skillId"] == target["skillId"],
            f"{label} skill id differs")
    if "inputSlot" in receipt:
        require(receipt["inputSlot"] == target["inputSlot"], f"{label} input slot differs")


def build_source_rows(
    source: dict[str, Any],
    policies: list[dict[str, Any]],
    expected_count: int,
    policy_semantic_sha256: str,
    policy_schema_sha256: str,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    policy_by_native = {row["upstreamNativeFamily"]: row for row in policies}
    clusters = source.get("nativeFamilyClusters")
    class_rows = source.get("sourceBlockerClassRows")
    require(isinstance(clusters, list) and isinstance(class_rows, list),
            "Source feasibility rows missing")
    require({row["nativeFamily"] for row in clusters} == set(policy_by_native),
            "Source reconstruction family coverage changed")

    class_to_cluster: dict[str, dict[str, Any]] = {}
    family_rows: list[dict[str, Any]] = []
    for cluster in clusters:
        policy = policy_by_native[cluster["nativeFamily"]]
        require(cluster.get("sourceExactDecision") == "BLOCKED",
                "Source exact family blocker was removed")
        require(cluster.get("sourceEraProviderDecision") == "NOT_ACQUIRED",
                "Source-era provider unexpectedly changed")
        for exact_class in cluster["exactSourceClasses"]:
            require(exact_class not in class_to_cluster, "Source class belongs to two families")
            class_to_cluster[exact_class] = cluster
        family_rows.append({
            "upstreamClusterId": cluster["clusterId"],
            "upstreamNativeFamily": cluster["nativeFamily"],
            "policyFamilyId": policy["policyFamilyId"],
            "closureBasis": policy["closureBasis"],
            "moduleOccurrenceCount": cluster["moduleOccurrenceCount"],
            "requiredMutatedOutputs": sorted(cluster["requiredMutatedOutputs"]),
            "requiredOracleIds": list(policy["requiredOracleIds"]),
            "sourceExact": False,
            "executionAdmission": False,
        })

    rows: list[dict[str, Any]] = []
    seen_occurrences: set[str] = set()
    for class_row in class_rows:
        exact_class = class_row["exactSourceClass"]
        cluster = class_to_cluster.get(exact_class)
        require(cluster is not None, f"Source class has no reconstruction family: {exact_class}")
        policy = policy_by_native[cluster["nativeFamily"]]
        occurrence_ids = class_row["moduleOccurrenceIds"]
        require(class_row["moduleOccurrenceCount"] == len(occurrence_ids),
                "Source class occurrence count differs")
        require(class_row.get("decision") == "BLOCKED_NO_SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER",
                "Source class provider blocker was removed")
        require(class_row.get("actualOutputOracleCount") == 0,
                "Source upstream actual-output oracle unexpectedly changed")
        for occurrence_id in occurrence_ids:
            require(occurrence_id not in seen_occurrences, "duplicate Source occurrence id")
            seen_occurrences.add(occurrence_id)
            identity = {
                "exactSourceClass": exact_class,
                "moduleOccurrenceId": occurrence_id,
                "upstreamClusterId": cluster["clusterId"],
                "requiredMutatedOutput": class_row["requiredMutatedOutput"],
                "upstreamDecision": class_row["decision"],
            }
            identity_sha = canonical_sha256(identity)
            policy_binding = {
                "policySemanticSha256": policy_semantic_sha256,
                "policySchemaSha256": policy_schema_sha256,
                "policyVersion": 1,
                "upstreamIdentitySha256": identity_sha,
                "policyFamilyId": policy["policyFamilyId"],
                "closureBasis": policy["closureBasis"],
                "evidenceFidelity": "SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED",
                "requiredOracleIds": list(policy["requiredOracleIds"]),
            }
            policy_binding_sha = canonical_sha256(policy_binding)
            rows.append({
                "policyRowId": f"reconstructed-source-{policy_binding_sha[:20]}",
                "upstreamIdentitySha256": identity_sha,
                "policyBindingSha256": policy_binding_sha,
                **identity,
                "policyFamilyId": policy["policyFamilyId"],
                "evidenceFidelity": "SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED",
                "executionFidelity": "RECONSTRUCTED_APPROVED_V1_NOT_IMPLEMENTED",
                "sourceExact": False,
                "preservedEvidenceBlockers": [class_row["decision"]],
                "policyFidelityGuards": [
                    "SOURCE_EXACT_NOT_GRANTED_BY_RECONSTRUCTION_APPROVAL",
                    "CURRENT_REVISION_MUST_NOT_BE_LABELED_SOURCE_ERA",
                ],
                "requiredOracleIds": list(policy["requiredOracleIds"]),
                "executionBlockers": list(SOURCE_EXECUTION_BLOCKERS),
                "executionAdmission": False,
                "productAdmission": False,
            })
    rows.sort(key=lambda row: row["moduleOccurrenceId"])
    family_rows.sort(key=lambda row: row["policyFamilyId"])
    require(len(rows) == expected_count, "Source execution denominator changed")
    require(sum(row["moduleOccurrenceCount"] for row in family_rows) == expected_count,
            "Source family denominator sum changed")
    return rows, family_rows


def dotted_value(row: dict[str, Any], path: str) -> Any:
    current: Any = row
    for part in path.split("."):
        require(isinstance(current, dict) and part in current,
                f"material rule path is missing: {path}")
        current = current[part]
    return current


def rule_matches(row: dict[str, Any], rule: dict[str, Any]) -> bool:
    return all(dotted_value(row, condition["path"]) == condition["equals"]
               for condition in rule["conditions"])


def build_material_rows(
    acquisition: dict[str, Any],
    family_policies: list[dict[str, Any]],
    rules: list[dict[str, Any]],
    denominators: dict[str, int],
    policy_semantic_sha256: str,
    policy_schema_sha256: str,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    family_by_id = {row["policyFamilyId"]: row for row in family_policies}
    domain_inputs = (
        ("RENDER_STATE", "renderStateRows", denominators["materialRenderStateRowCount"]),
        ("STATIC_PERMUTATION", "staticPermutationRows",
         denominators["materialStaticPermutationRowCount"]),
        ("SAMPLER", "strictSamplerRows", denominators["materialSamplerRowCount"]),
    )
    matrices = acquisition.get("matrices")
    require(isinstance(matrices, dict), "Material feasibility matrices missing")
    rows: list[dict[str, Any]] = []
    seen_ids: set[str] = set()
    for domain, array_name, expected_count in domain_inputs:
        upstream_rows = matrices.get(array_name)
        require(isinstance(upstream_rows, list) and len(upstream_rows) == expected_count,
                f"Material {domain} denominator changed")
        domain_rules = [rule for rule in rules if rule["domain"] == domain]
        require(domain_rules, f"Material {domain} rules missing")
        for upstream in upstream_rows:
            matching = [rule for rule in domain_rules if rule_matches(upstream, rule)]
            require(len(matching) == 1,
                    f"Material row must match exactly one reconstruction rule: "
                    f"{upstream.get('matrixRowId')}")
            rule = matching[0]
            family = family_by_id[rule["policyFamilyId"]]
            row_id = upstream.get("matrixRowId")
            require(isinstance(row_id, str) and row_id not in seen_ids,
                    "Material matrix row id missing or duplicated")
            seen_ids.add(row_id)
            occurrences = upstream.get("materialOccurrenceIds")
            require(isinstance(occurrences, list) and occurrences,
                    f"Material row occurrences missing: {row_id}")
            require(len(occurrences) == len(set(occurrences)),
                    f"Material row occurrences duplicated: {row_id}")
            blockers = upstream.get("remainingBlockers")
            require(isinstance(blockers, list) and blockers,
                    f"Material row upstream blockers missing: {row_id}")
            require(upstream.get("executionReady") is False,
                    f"Material upstream row unexpectedly executable: {row_id}")
            partial_exact = upstream.get("partialSourceExactFields", [])
            require(isinstance(partial_exact, list), "partial exact field list invalid")
            identity = {
                "domain": domain,
                "upstreamMatrixRowId": row_id,
                "materialRecipeId": upstream["materialRecipeId"],
                "materialOccurrenceIds": sorted(occurrences),
                "fieldId": upstream["fieldId"],
                "fieldKind": upstream["fieldKind"],
                "bindingOrigin": upstream["bindingOriginAndOwner"]["bindingOrigin"],
                "upstreamDecision": upstream["sourceValueDecision"],
            }
            identity_sha = canonical_sha256(identity)
            policy_binding = {
                "policySemanticSha256": policy_semantic_sha256,
                "policySchemaSha256": policy_schema_sha256,
                "policyVersion": 1,
                "upstreamIdentitySha256": identity_sha,
                "policyFamilyId": family["policyFamilyId"],
                "closureBasis": family["closureBasis"],
                "evidenceFidelity": rule["evidenceFidelity"],
                "requiredOracleIds": list(family["requiredOracleIds"]),
            }
            policy_binding_sha = canonical_sha256(policy_binding)
            rows.append({
                "policyRowId": f"reconstructed-material-{policy_binding_sha[:20]}",
                "upstreamIdentitySha256": identity_sha,
                "policyBindingSha256": policy_binding_sha,
                **identity,
                "policyFamilyId": family["policyFamilyId"],
                "evidenceFidelity": rule["evidenceFidelity"],
                "executionFidelity": "RECONSTRUCTED_APPROVED_V1_NOT_MATERIALIZED",
                "sourceValueAcquired": bool(upstream.get("sourceValueAcquired", False)),
                "partialSourceExactFields": list(partial_exact),
                "previousSamplerAdmission": upstream.get("previousAdmission"),
                "fullDescriptorSourceExact": False,
                "sourceExact": False,
                "preservedEvidenceBlockers": sorted(blockers),
                "policyFidelityGuards": [
                    "SOURCE_EXACT_DESCRIPTOR_NOT_GRANTED_BY_RECONSTRUCTION_APPROVAL",
                    "CURRENT_CDO_OR_ROLE_POLICY_MUST_REMAIN_LABELED_RECONSTRUCTED",
                ],
                "requiredOracleIds": list(family["requiredOracleIds"]),
                "executionBlockers": sorted(set(blockers) | set(MATERIAL_EXECUTION_BLOCKERS)),
                "executionAdmission": False,
                "productAdmission": False,
            })
    rows.sort(key=lambda row: (row["domain"], row["upstreamMatrixRowId"]))
    require(len(rows) == denominators["materialExecutionRowCount"],
            "Material total denominator changed")
    return rows, sorted(family_policies, key=lambda row: row["policyFamilyId"])


def build_arithmetic_rows(
    runtime_receipt: dict[str, Any],
    arithmetic_policy: dict[str, Any],
    expected_count: int,
) -> list[dict[str, Any]]:
    upstream = runtime_receipt.get("familyEvaluators")
    require(isinstance(upstream, list) and len(upstream) == expected_count,
            "Material arithmetic family denominator changed")
    rows: list[dict[str, Any]] = []
    seen: set[str] = set()
    for family in upstream:
        family_id = family["familyId"]
        require(family_id not in seen, "duplicate Material arithmetic family")
        seen.add(family_id)
        require(family.get("graphProvenance") == "RECONSTRUCTED_GRAPH",
                "Material graph provenance was promoted")
        require(family.get("sourceExact") is False, "Material graph became Source exact")
        require(family.get("cpuNumericOracleVerified") is True and
                family.get("hlslNumericOracleVerified") is True,
                "Material arithmetic independent oracle regressed")
        evidence_blockers = family.get("evidenceBlockers")
        runtime_blockers = family.get("runtimeBlockers")
        require(isinstance(evidence_blockers, list) and evidence_blockers,
                "Material arithmetic evidence blockers missing")
        require(isinstance(runtime_blockers, list) and runtime_blockers,
                "Material arithmetic runtime blockers missing")
        identity = {
            "upstreamFamilyId": family_id,
            "familyIdentitySha256": family["familyIdentitySha256"],
            "evaluatorId": family["evaluatorId"],
            "evaluatorVersion": family["evaluatorVersion"],
            "evaluatorSha256": family["evaluatorSha256"],
        }
        rows.append({
            "policyRowId": f"reconstructed-arithmetic-{canonical_sha256(identity)[:20]}",
            **identity,
            "policyFamilyId": arithmetic_policy["policyFamilyId"],
            "evidenceFidelity": "RECONSTRUCTED_NUMERICALLY_VERIFIED_GRAPH_NOT_SOURCE_EXACT",
            "sourceExact": False,
            "cpuNumericOracleVerified": True,
            "hlslNumericOracleVerified": True,
            "preservedEvidenceBlockers": sorted(evidence_blockers),
            "requiredOracleIds": list(arithmetic_policy["requiredOracleIds"]),
            "executionBlockers": sorted(set(runtime_blockers) |
                                        {"FINAL_RUNTIME_MATERIAL_BINDING_NOT_VERIFIED"}),
            "executionAdmission": False,
            "productAdmission": False,
        })
    return sorted(rows, key=lambda row: row["upstreamFamilyId"])


def build_geometry_rows(receipt: dict[str, Any], expected_count: int) -> list[dict[str, Any]]:
    assets = receipt.get("assets")
    require(isinstance(assets, list) and len(assets) == expected_count,
            "Geometry carrier denominator changed")
    rows: list[dict[str, Any]] = []
    seen: set[str] = set()
    for asset in assets:
        asset_id = asset["assetId"]
        require(asset_id not in seen, "duplicate Geometry asset id")
        seen.add(asset_id)
        expected = asset["expectedTuple"]
        require(expected.get("geometryPreScale") == 0.01,
                "Geometry pre-scale contract changed")
        require(asset.get("artifactBindingIntegrity") ==
                "EXPECTED_G02_TUPLE_MATCHES_STAGED_BYTES",
                "Geometry artifact binding integrity changed")
        require(asset.get("runtimeGeometryPreScaleConsumed") is False,
                "Geometry runtime pre-scale unexpectedly admitted")
        require(asset.get("productAdmission") is False,
                "Geometry Product admission unexpectedly opened")
        rows.append({
            "assetId": asset_id,
            "sourceObject": asset["sourceObject"],
            "candidateResourceSha256": asset["candidateResource"]["sha256"],
            "payloadSha256": expected["payloadSha256"],
            "metadataIdentitySha256": expected["metadataIdentitySha256"],
            "geometryPreScale": expected["geometryPreScale"],
            "artifactBindingIntegrity": asset["artifactBindingIntegrity"],
            "sourceFidelity": "GEOMETRY_ARTIFACT_BINDING_NOT_SOURCE_EXACT",
            "sourceExact": False,
            "executionBlockers": [
                "RUNTIME_GEOMETRY_PRESCALE_NOT_CONSUMED",
                "FINAL_BOUNDS_CACHE_AND_SHADER_CONSUMPTION_NOT_PROVEN",
            ],
            "executionAdmission": False,
            "productAdmission": False,
        })
    return sorted(rows, key=lambda row: row["assetId"])


def validate_v1_fidelity_projection(
    source_rows: list[dict[str, Any]],
    material_rows: list[dict[str, Any]],
    arithmetic_rows: list[dict[str, Any]],
    geometry_rows: list[dict[str, Any]],
) -> None:
    all_rows = [*source_rows, *material_rows, *arithmetic_rows, *geometry_rows]
    for row in all_rows:
        require(row.get("sourceExact") is False,
                f"V1 row was promoted to Source exact: {row.get('policyRowId', row.get('assetId'))}")
        require(row.get("executionAdmission") is False,
                "V1 row execution admission must remain false")
        require(row.get("productAdmission") is False,
                "V1 row Product admission must remain false")
        for field in ("evidenceFidelity", "executionFidelity", "sourceFidelity"):
            label = row.get(field)
            if label is None:
                continue
            require(label not in {"SOURCE_EXACT", "SOURCE_EXACT_SAMPLER", "SOURCE_EXACT_GRAPH"},
                    f"forbidden full Source fidelity label used: {label}")
            if label.startswith("SOURCE_EXACT_"):
                require(label in V1_ALLOWED_PARTIAL_SOURCE_LABELS,
                        f"unapproved partial Source fidelity label used: {label}")

    partial_counts = Counter(
        row["evidenceFidelity"] for row in material_rows
        if row["evidenceFidelity"].startswith("SOURCE_EXACT_")
    )
    require(dict(partial_counts) == V1_ALLOWED_PARTIAL_SOURCE_LABEL_COUNTS,
            "V1 partial Source fidelity label coverage changed")

    sampler_rows = [row for row in material_rows if row["domain"] == "SAMPLER"]
    require(len(sampler_rows) == 77, "V1 strict sampler denominator changed")
    require(all(row["fullDescriptorSourceExact"] is False for row in sampler_rows),
            "V1 sampler full descriptor was promoted to Source exact")
    require(all(row["evidenceFidelity"] != "SOURCE_EXACT_SAMPLER"
                for row in sampler_rows),
            "V1 sampler forbidden full-exact label used")
    former_exact = [
        row for row in sampler_rows
        if row["previousSamplerAdmission"] == "SOURCE_EXACT_SAMPLER"
    ]
    former_counts = Counter(row["evidenceFidelity"] for row in former_exact)
    require(former_counts == Counter({
        "SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS": 1,
        "SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN": 3,
    }), "V1 former exact sampler 1+3 classification changed")


def build_receipt(
    root: Path,
    policy_source: dict[str, Any],
    policy_schema: dict[str, Any],
    policy_source_path: str = POLICY_SOURCE_PATH,
    policy_schema_path: str = POLICY_SCHEMA_PATH,
) -> dict[str, Any]:
    validate_policy_source(policy_source)
    validate_policy_schema_document(policy_schema)
    frozen_inputs, receipts = verify_frozen_inputs(root, policy_source["frozenInputs"])
    target = policy_source["target"]

    source_receipt = receipts["lostark.effect-source-oracle-acquisition"]
    material_acquisition = receipts[
        "lostark.artist-31470-material-source-value-acquisition-receipt"
    ]
    material_runtime = receipts["lostark.artist-31470-material-runtime-oracle-receipt"]
    geometry_receipt = receipts["lostark.artist-31470-geometry-resource-binding-receipt"]
    for label, upstream in (
        ("Source", source_receipt),
        ("Material acquisition", material_acquisition),
        ("Material runtime", material_runtime),
        ("Geometry", geometry_receipt),
    ):
        assert_target(upstream, target, label)

    denominators = policy_source["denominators"]
    source_rows, source_families = build_source_rows(
        source_receipt,
        policy_source["sourceFamilyPolicies"],
        denominators["sourceExecutionRowCount"],
        V1_POLICY_SOURCE_CANONICAL_SHA256,
        V1_POLICY_SCHEMA_CANONICAL_SHA256,
    )
    material_rows, material_families = build_material_rows(
        material_acquisition,
        policy_source["materialExecutionFamilies"],
        policy_source["materialRowRules"],
        denominators,
        V1_POLICY_SOURCE_CANONICAL_SHA256,
        V1_POLICY_SCHEMA_CANONICAL_SHA256,
    )
    arithmetic_policy = next(
        row for row in policy_source["materialExecutionFamilies"]
        if row["policyFamilyId"] == "RECONSTRUCTED_APPROVED_ARITHMETIC_V1"
    )
    arithmetic_rows = build_arithmetic_rows(
        material_runtime,
        arithmetic_policy,
        denominators["materialArithmeticFamilyCount"],
    )
    geometry_rows = build_geometry_rows(
        geometry_receipt,
        denominators["geometryCarrierCount"],
    )
    validate_v1_fidelity_projection(
        source_rows, material_rows, arithmetic_rows, geometry_rows
    )

    former_exact_sampler = [
        row for row in material_rows if row["previousSamplerAdmission"] == "SOURCE_EXACT_SAMPLER"
    ]
    partial_tag_count = sum(
        row["evidenceFidelity"] == "SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS"
        for row in former_exact_sampler
    )
    default_unproven_count = sum(
        row["evidenceFidelity"] ==
        "SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN"
        for row in former_exact_sampler
    )
    require(len(former_exact_sampler) == 4 and partial_tag_count == 1 and
            default_unproven_count == 3,
            "former exact sampler 1+3 reclassification changed")

    unsigned = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "policyId": policy_source["policyId"],
        "policyVersion": policy_source["policyVersion"],
        "target": copy.deepcopy(target),
        "policyRole": "IMMUTABLE_RECONSTRUCTION_ROUTE_NOT_RUNTIME_AUTHORITY",
        "approvalDecision": copy.deepcopy(policy_source["approvalDecision"]),
        "policySourceIdentity": {
            "path": policy_source_path,
            "canonicalJsonSha256": canonical_sha256(policy_source),
        },
        "policySchemaIdentity": {
            "path": policy_schema_path,
            "canonicalJsonSha256": canonical_sha256(policy_schema),
        },
        "frozenInputs": frozen_inputs,
        "denominators": copy.deepcopy(denominators),
        "sourceExecutionFamilies": source_families,
        "sourceRows": source_rows,
        "materialExecutionFamilies": material_families,
        "materialRows": material_rows,
        "materialArithmeticRows": arithmetic_rows,
        "geometryRows": geometry_rows,
        "fidelityPolicy": {
            "sourceExactAdmission": False,
            "sourceExactUpgradeAllowed": False,
            "reconstructionApprovalRemovesEvidenceBlockers": False,
            "executionFidelityLabel": "RECONSTRUCTED_APPROVED_V1",
            "forbiddenClaims": list(policy_source["forbiddenClaims"]),
        },
        "admissionPolicy": {
            "policyRouteApproved": True,
            "evidenceJoinIntegrity": True,
            "sourceExactAdmission": False,
            "executionAdmission": False,
            "productAdmission": False,
            "requiredGlobalGates": list(policy_source["requiredGlobalGates"]),
            "executionBlockers": list(GLOBAL_EXECUTION_BLOCKERS),
            "productBlockers": list(GLOBAL_PRODUCT_BLOCKERS),
        },
        "manualValidation": {
            "mode": "HUMAN_EYE_RUNTIME_OCCURRENCE_CHECKLIST",
            "requiredOccurrenceCount": denominators["effectOccurrenceCount"],
            "completedOccurrenceCount": 0,
            "automatedScreenshotOrImageOracleAllowed": False,
            "captureArtifactRequired": False,
            "status": "NOT_STARTED",
        },
        "rollbackConditions": list(policy_source["rollbackConditions"]),
        "summary": {
            "sourceExecutionRowCount": len(source_rows),
            "sourceExecutionFamilyCount": len(source_families),
            "materialRenderStateRowCount": sum(
                row["domain"] == "RENDER_STATE" for row in material_rows
            ),
            "materialStaticPermutationRowCount": sum(
                row["domain"] == "STATIC_PERMUTATION" for row in material_rows
            ),
            "materialSamplerRowCount": sum(
                row["domain"] == "SAMPLER" for row in material_rows
            ),
            "materialExecutionRowCount": len(material_rows),
            "materialArithmeticFamilyCount": len(arithmetic_rows),
            "formerExactSamplerReauditCount": len(former_exact_sampler),
            "formerExactSamplerPartialTagsCount": partial_tag_count,
            "formerExactSamplerDefaultUnprovenCount": default_unproven_count,
            "samplerFullSourceExactCount": sum(
                row["fullDescriptorSourceExact"] for row in material_rows
                if row["domain"] == "SAMPLER"
            ),
            "forbiddenFullFidelityLabelCount": sum(
                row.get(field) in {"SOURCE_EXACT", "SOURCE_EXACT_SAMPLER", "SOURCE_EXACT_GRAPH"}
                for row in [*source_rows, *material_rows, *arithmetic_rows, *geometry_rows]
                for field in ("evidenceFidelity", "executionFidelity", "sourceFidelity")
            ),
            "geometryCarrierCount": len(geometry_rows),
            "manualValidationCount": 0,
            "sourceExactCount": 0,
            "executionAdmissionCount": 0,
            "productAdmissionCount": 0,
        },
    }
    receipt = reseal_receipt(unsigned)
    validate_json_schema_instance(receipt, policy_schema)
    return receipt


def validate_receipt(
    receipt: dict[str, Any],
    root: Path,
    policy_source: dict[str, Any],
    policy_schema: dict[str, Any],
    policy_source_path: str = POLICY_SOURCE_PATH,
    policy_schema_path: str = POLICY_SCHEMA_PATH,
) -> None:
    expected = build_receipt(
        root,
        policy_source,
        policy_schema,
        policy_source_path,
        policy_schema_path,
    )
    validate_receipt_against_expected(receipt, expected)


def validate_receipt_against_expected(
    receipt: dict[str, Any],
    expected: dict[str, Any],
) -> None:
    """Validate one stored receipt against a separately rebuilt immutable snapshot."""
    require(receipt.get("schema") == RECEIPT_SCHEMA, "policy receipt schema changed")
    require_exact_int(receipt.get("formatVersion"), FORMAT_VERSION,
                      "policy receipt formatVersion")
    receipt_sha = receipt.get("receiptSha256")
    require_sha256(receipt_sha, "policy receipt receiptSha256")
    verify_self_hash(receipt, receipt_sha, "policy receipt")
    require(receipt == expected,
            "policy receipt differs from frozen inputs and approval source")


def generated_matches(path: Path, expected: bytes) -> bool:
    if not path.is_file():
        return False
    actual = path.read_bytes().replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    normalized = expected.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return actual == normalized


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--policy-source", default=POLICY_SOURCE_PATH)
    parser.add_argument("--schema", default=POLICY_SCHEMA_PATH)
    parser.add_argument("--output", default=OUTPUT_PATH)
    parser.add_argument("--check", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = args.root.resolve()
    policy_source_path = require_safe_path(args.policy_source, "policy source CLI path")
    schema_path = require_safe_path(args.schema, "policy schema CLI path")
    output_path = require_safe_path(args.output, "policy output CLI path")
    source = load_strict_json_object(root / policy_source_path)
    schema = load_strict_json_object(root / schema_path)
    receipt = build_receipt(root, source, schema, policy_source_path, schema_path)
    validate_receipt(receipt, root, source, schema, policy_source_path, schema_path)
    expected = pretty_json_bytes(receipt)
    destination = root / output_path
    if args.check:
        if not generated_matches(destination, expected):
            print(f"stale reconstructed approval policy: {output_path}", file=sys.stderr)
            return 1
    else:
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(expected)
    print(
        "PASS: reconstructed approval policy "
        f"source={len(receipt['sourceRows'])} "
        f"material={len(receipt['materialRows'])} "
        f"sampler={receipt['summary']['materialSamplerRowCount']} "
        f"geometry={len(receipt['geometryRows'])} "
        "sourceExact=false execution=false product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
