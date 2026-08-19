#!/usr/bin/env python3
"""Validate the fail-closed boss pattern -> authored Effect mapping contract.

The validator intentionally implements only the JSON Schema keywords used by
lostark.boss-pattern-effects.schema.json.  Keeping this dependency-free lets
the EffectPipeline checks run on the repository's stock Python installation.
Cross-document/source-evidence checks for the first Valtan canary live in
build_valtan_whirlwind_effect_canary.py.
"""

from __future__ import annotations

import argparse
import json
import math
import re
from pathlib import Path, PurePosixPath
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
DEFAULT_SCHEMA = (
    SCRIPT_PATH.parent
    / "Schemas"
    / "lostark.boss-pattern-effects.schema.json"
)
DEFAULT_MAPPING = (
    REPOSITORY_ROOT
    / "Data"
    / "Animation"
    / "Authored"
    / "Valtan"
    / "Valtan.patterneffects.json"
)

FORBIDDEN_GAMEPLAY_FIELDS = {
    "durationMs",
    "hitShape",
    "hitOuterRadius",
    "hitInnerRadius",
    "hitAngleDegrees",
    "hitLength",
    "hitHalfWidth",
    "hitCount",
    "hitIntervalMs",
    "serverDamageProfileId",
    "pushRangeM",
    "pushMs",
    "knockdown",
    "downMs",
    "damage",
    "damageProfileId",
}


class ContractError(ValueError):
    """Raised when a mapping or its schema violates the repository contract."""


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise ContractError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def canonical_json(value: Any) -> str:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    )


def _resolve_local_ref(root_schema: dict[str, Any], reference: str) -> dict[str, Any]:
    if not reference.startswith("#/"):
        raise ContractError(f"only local JSON Schema references are supported: {reference}")
    value: Any = root_schema
    for raw_token in reference[2:].split("/"):
        token = raw_token.replace("~1", "/").replace("~0", "~")
        if not isinstance(value, dict) or token not in value:
            raise ContractError(f"JSON Schema reference does not resolve: {reference}")
        value = value[token]
    if not isinstance(value, dict):
        raise ContractError(f"JSON Schema reference is not an object: {reference}")
    return value


def _type_matches(instance: Any, expected: str) -> bool:
    if expected == "object":
        return isinstance(instance, dict)
    if expected == "array":
        return isinstance(instance, list)
    if expected == "string":
        return isinstance(instance, str)
    if expected == "boolean":
        return isinstance(instance, bool)
    if expected == "integer":
        return isinstance(instance, int) and not isinstance(instance, bool)
    if expected == "number":
        return (
            isinstance(instance, (int, float))
            and not isinstance(instance, bool)
            and math.isfinite(float(instance))
        )
    if expected == "null":
        return instance is None
    raise ContractError(f"unsupported JSON Schema type keyword: {expected}")


def validate_schema_instance(
    instance: Any,
    schema: dict[str, Any],
    root_schema: dict[str, Any] | None = None,
    path: str = "$",
) -> None:
    """Validate one instance with the small draft-2020 subset used here."""

    root_schema = root_schema or schema
    if "$ref" in schema:
        resolved = _resolve_local_ref(root_schema, str(schema["$ref"]))
        validate_schema_instance(instance, resolved, root_schema, path)
        return

    if "const" in schema and instance != schema["const"]:
        raise ContractError(f"{path} must equal {schema['const']!r}")
    if "enum" in schema and instance not in schema["enum"]:
        raise ContractError(f"{path} is not one of {schema['enum']!r}")

    expected_type = schema.get("type")
    if expected_type is not None:
        accepted = (
            expected_type
            if isinstance(expected_type, list)
            else [expected_type]
        )
        if not any(_type_matches(instance, str(item)) for item in accepted):
            raise ContractError(f"{path} has the wrong JSON type; expected {accepted!r}")

    if isinstance(instance, str):
        if len(instance) < int(schema.get("minLength", 0)):
            raise ContractError(f"{path} is shorter than minLength")
        pattern = schema.get("pattern")
        if pattern is not None and re.search(str(pattern), instance) is None:
            raise ContractError(f"{path} does not match {pattern!r}")

    if (
        isinstance(instance, (int, float))
        and not isinstance(instance, bool)
        and "minimum" in schema
        and float(instance) < float(schema["minimum"])
    ):
        raise ContractError(f"{path} is below minimum {schema['minimum']}")

    if isinstance(instance, list):
        if len(instance) < int(schema.get("minItems", 0)):
            raise ContractError(f"{path} has too few items")
        if "maxItems" in schema and len(instance) > int(schema["maxItems"]):
            raise ContractError(f"{path} has too many items")
        if schema.get("uniqueItems"):
            identities = [canonical_json(item) for item in instance]
            if len(identities) != len(set(identities)):
                raise ContractError(f"{path} contains duplicate items")
        item_schema = schema.get("items")
        if item_schema is not None:
            if not isinstance(item_schema, dict):
                raise ContractError(f"{path} schema items must be an object")
            for index, value in enumerate(instance):
                validate_schema_instance(
                    value,
                    item_schema,
                    root_schema,
                    f"{path}[{index}]",
                )

    if isinstance(instance, dict):
        required = schema.get("required", [])
        if not isinstance(required, list):
            raise ContractError(f"{path} schema required must be an array")
        missing = [key for key in required if key not in instance]
        if missing:
            raise ContractError(f"{path} is missing required fields: {missing!r}")
        properties = schema.get("properties", {})
        if not isinstance(properties, dict):
            raise ContractError(f"{path} schema properties must be an object")
        if schema.get("additionalProperties") is False:
            extras = sorted(set(instance) - set(properties))
            if extras:
                raise ContractError(f"{path} contains unknown fields: {extras!r}")
        for key, value in instance.items():
            child_schema = properties.get(key)
            if child_schema is not None:
                if not isinstance(child_schema, dict):
                    raise ContractError(f"{path}.{key} schema must be an object")
                validate_schema_instance(
                    value,
                    child_schema,
                    root_schema,
                    f"{path}.{key}",
                )


def _walk_field_names(value: Any, path: str = "$") -> list[tuple[str, str]]:
    result: list[tuple[str, str]] = []
    if isinstance(value, dict):
        for key, child in value.items():
            child_path = f"{path}.{key}"
            result.append((key, child_path))
            result.extend(_walk_field_names(child, child_path))
    elif isinstance(value, list):
        for index, child in enumerate(value):
            result.extend(_walk_field_names(child, f"{path}[{index}]"))
    return result


def validate_repository_path(value: str, label: str) -> None:
    if "\\" in value or re.match(r"^[A-Za-z]:", value):
        raise ContractError(f"{label} must be a repository-relative POSIX path")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or "." in path.parts:
        raise ContractError(f"{label} escapes or ambiguously addresses the repository")


def validate_mapping_semantics(mapping: dict[str, Any]) -> None:
    forbidden = [
        path
        for key, path in _walk_field_names(mapping)
        if key in FORBIDDEN_GAMEPLAY_FIELDS
    ]
    if forbidden:
        raise ContractError(
            "boss effect mapping duplicated gameplay-owned fields: "
            + ", ".join(forbidden)
        )

    gameplay = mapping["gameplayAuthority"]
    validate_repository_path(
        gameplay["encounterDocument"],
        "gameplayAuthority.encounterDocument",
    )
    if gameplay["duplicatedGameplayFields"]:
        raise ContractError("duplicatedGameplayFields must remain empty")

    bindings = mapping["bindings"]
    if bindings != sorted(bindings, key=lambda row: row["bindingId"]):
        raise ContractError("bindings must be sorted by bindingId")
    binding_ids: set[str] = set()
    qualified_keys: set[tuple[Any, ...]] = set()
    for index, binding in enumerate(bindings):
        label = f"bindings[{index}]"
        binding_id = binding["bindingId"]
        if binding_id in binding_ids:
            raise ContractError(f"duplicate bindingId: {binding_id}")
        binding_ids.add(binding_id)
        for field in (
            "effectDocument",
            "animationBindingDocument",
            "sourceCatalogDocument",
            "sourceParticleResourceCatalogDocument",
        ):
            validate_repository_path(binding[field], f"{label}.{field}")

        source_evidence = binding["sourceEvidence"]
        validate_repository_path(
            source_evidence["runtimeCookReceiptSourcePath"],
            f"{label}.sourceEvidence.runtimeCookReceiptSourcePath",
        )
        graph_documents = source_evidence["sourceGraphDocuments"]
        if graph_documents != sorted(
            graph_documents, key=lambda row: row["logicalPackage"].casefold()
        ):
            raise ContractError(
                f"{label}.sourceEvidence.sourceGraphDocuments must be sorted"
            )
        graph_packages = [row["logicalPackage"].casefold() for row in graph_documents]
        if len(graph_packages) != len(set(graph_packages)):
            raise ContractError(f"{label} has duplicate source graph packages")
        for graph_index, graph_document in enumerate(graph_documents):
            validate_repository_path(
                graph_document["sourcePath"],
                f"{label}.sourceEvidence.sourceGraphDocuments[{graph_index}].sourcePath",
            )

        branch = binding["sourceBranch"]
        if branch["sourceStageIndex"] not in branch["stagePath"]:
            raise ContractError(f"{label} sourceStageIndex is absent from stagePath")
        qualified = (
            binding["patternId"],
            binding["semanticStageId"],
            binding["actionId"],
            branch["sourceActionId"],
            branch["branchId"],
            branch["sourceStageIndex"],
            branch["runtimeClipName"],
        )
        if qualified in qualified_keys:
            raise ContractError(f"duplicate action-qualified binding: {qualified!r}")
        qualified_keys.add(qualified)

        bone = binding["modelBoneEvidence"]
        validate_repository_path(
            bone["runtimeModelAssetId"],
            f"{label}.modelBoneEvidence.runtimeModelAssetId",
        )

        source_rows = binding["sourceOccurrences"]
        if source_rows != sorted(source_rows, key=lambda row: row["notifyId"]):
            raise ContractError(f"{label}.sourceOccurrences must be sorted by notifyId")
        fail_rows = binding["failClosedOccurrences"]
        if fail_rows != sorted(fail_rows, key=lambda row: row["notifyId"]):
            raise ContractError(
                f"{label}.failClosedOccurrences must be sorted by notifyId"
            )
        source_ids = [row["notifyId"] for row in source_rows]
        fail_ids = [row["notifyId"] for row in fail_rows]
        if len(source_ids) != len(set(source_ids)):
            raise ContractError(f"{label} has duplicate source occurrence IDs")
        if len(fail_ids) != len(set(fail_ids)):
            raise ContractError(f"{label} has duplicate fail-closed occurrence IDs")
        overlap = sorted(set(source_ids).intersection(fail_ids))
        if overlap:
            raise ContractError(f"{label} occurrence admission overlaps: {overlap!r}")

        expected_stage = f"stage-{branch['sourceStageIndex']:03d}"
        if any(f"/{expected_stage}/" not in notify_id for notify_id in source_ids + fail_ids):
            raise ContractError(f"{label} occurrence escaped the selected source stage")
        carrier_ids: set[str] = set()
        for row in source_rows:
            admission = row["admission"]
            carriers = admission["carriers"]
            denominator = admission["carrierDenominator"]
            visible_count = sum(
                carrier["disposition"] == "VISIBLE_EXECUTABLE"
                for carrier in carriers
            )
            blocked_count = len(carriers) - visible_count
            if (
                len(carriers) != denominator
                or admission["visibleExecutableCarrierCount"] != visible_count
                or admission["failClosedCarrierCount"] != blocked_count
            ):
                raise ContractError(
                    f"{label} {row['notifyId']} carrier denominator/counts disagree"
                )
            if [carrier["sourceOrder"] for carrier in carriers] != list(
                range(denominator)
            ):
                raise ContractError(
                    f"{label} {row['notifyId']} carrier sourceOrder is not contiguous"
                )
            expected_projection = (
                "FAIL_CLOSED"
                if visible_count == 0
                else "PORTABLE_AUTHORED_V13"
                if visible_count == denominator
                else "PARTIAL_PORTABLE_AUTHORED_V13"
            )
            if admission["executableProjection"] != expected_projection:
                raise ContractError(
                    f"{label} {row['notifyId']} executableProjection/counts disagree"
                )
            blockers = admission["blockers"]
            if blockers != sorted(blockers):
                raise ContractError(
                    f"{label} {row['notifyId']} blockers must be sorted"
                )
            carrier_blocker_union = sorted(
                {
                    blocker
                    for carrier in carriers
                    for blocker in carrier["blockers"]
                }
            )
            if blockers != carrier_blocker_union:
                raise ContractError(
                    f"{label} {row['notifyId']} blockers must equal carrier blockers"
                )
            for carrier_index, carrier in enumerate(carriers):
                carrier_label = (
                    f"{label} {row['notifyId']} carrier[{carrier_index}]"
                )
                carrier_id = carrier["carrierId"]
                if carrier_id in carrier_ids:
                    raise ContractError(f"duplicate carrierId: {carrier_id}")
                carrier_ids.add(carrier_id)
                carrier_blockers = carrier["blockers"]
                if carrier_blockers != sorted(carrier_blockers):
                    raise ContractError(
                        f"{carrier_label} blockers must be sorted"
                    )
                recipe = carrier["sourceRecipe"]
                material = carrier["materialAdmission"]
                resources = carrier["resources"]
                resource_slots = [resource["slotId"] for resource in resources]
                if len(resource_slots) != len(set(resource_slots)):
                    raise ContractError(
                        f"{carrier_label} has duplicate runtime resource slots"
                    )
                for resource_index, resource in enumerate(resources):
                    validate_repository_path(
                        resource["assetId"],
                        f"{carrier_label}.resources[{resource_index}].assetId",
                    )
                if carrier["disposition"] == "VISIBLE_EXECUTABLE":
                    if (
                        carrier_blockers
                        or recipe["portableStatus"] != "PORTABLE_AUTHORED_V13"
                        or "portableRecipeSha256" not in recipe
                        or material["status"]
                        != "ADMITTED_RECONSTRUCTED_PROFILE"
                        or not resources
                    ):
                        raise ContractError(
                            f"{carrier_label} visible admission evidence is incomplete"
                        )
                    required_material_fields = {
                        "profileId",
                        "runtimeShaderProfileId",
                        "parentMaterialPath",
                        "semanticStatus",
                        "evidenceKind",
                        "evidencePath",
                        "evidenceSha256",
                    }
                    if not required_material_fields.issubset(material):
                        raise ContractError(
                            f"{carrier_label} visible material evidence is incomplete"
                        )
                    validate_repository_path(
                        material["evidencePath"],
                        f"{carrier_label}.materialAdmission.evidencePath",
                    )
                    if carrier["rendererShape"] == "mesh":
                        geometry = carrier.get("geometryEvidence")
                        if (
                            geometry is None
                            or resource_slots.count("meshModel") != 1
                        ):
                            raise ContractError(
                                f"{carrier_label} mesh geometry evidence is incomplete"
                            )
                        for field in ("sourceGltfPath", "sourceBinPath"):
                            validate_repository_path(
                                geometry[field],
                                f"{carrier_label}.geometryEvidence.{field}",
                            )
                        validate_repository_path(
                            geometry["runtimeAssetId"],
                            f"{carrier_label}.geometryEvidence.runtimeAssetId",
                        )
                    elif "geometryEvidence" in carrier or "meshModel" in resource_slots:
                        raise ContractError(
                            f"{carrier_label} non-mesh carrier has mesh geometry"
                        )
                else:
                    if (
                        not carrier_blockers
                        or material != {"status": "FAIL_CLOSED"}
                        or resources
                        or "geometryEvidence" in carrier
                    ):
                        raise ContractError(
                            f"{carrier_label} fail-closed carrier leaked executable data"
                        )
        blockers = binding["productAdmission"]["blockers"]
        if blockers != sorted(blockers):
            raise ContractError(f"{label} product blockers must be sorted")

    if bindings:
        first = bindings[0]
        identity = (
            first["patternId"],
            first["semanticStageId"],
            first["actionId"],
        )
        authority_identity = (
            gameplay["patternId"],
            gameplay["stageId"],
            gameplay["actionId"],
        )
        if identity != authority_identity:
            raise ContractError(
                "first action-qualified binding disagrees with gameplay authority reference"
            )


def validate_mapping(
    mapping: dict[str, Any],
    schema: dict[str, Any],
) -> None:
    if schema.get("$id") != "lostark.boss-pattern-effects.schema.json":
        raise ContractError("unexpected boss pattern effect schema identity")
    if schema.get("additionalProperties") is not False:
        raise ContractError("boss pattern effect root schema must be strict")
    validate_schema_instance(mapping, schema)
    validate_mapping_semantics(mapping)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mapping", type=Path, default=DEFAULT_MAPPING)
    parser.add_argument("--schema", type=Path, default=DEFAULT_SCHEMA)
    arguments = parser.parse_args()
    try:
        mapping = load_json(arguments.mapping.resolve())
        schema = load_json(arguments.schema.resolve())
        validate_mapping(mapping, schema)
    except ContractError as error:
        print(f"[boss-pattern-effects] FAIL: {error}")
        return 1
    print(
        "[boss-pattern-effects] PASS: "
        f"{arguments.mapping} ({len(mapping['bindings'])} binding(s))"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
