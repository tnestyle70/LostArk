#!/usr/bin/env python3
"""Build the V6 generic Effect visual-program execution projection corpus.

The corpus does not mutate the product catalog.  It pins the executable legacy
document rows that already back the three LMB BA programs, keeps unsupported
source rows fail-closed, and expresses Artist F LocalDecal #20/#21 through the
same selector and family-adapter contract.  A later product consumer may stage
the admitted projections into the existing EFFECT_DOCUMENT_DESC playback path.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import tempfile
from collections import Counter
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent

SCHEMA_RELATIVE_PATH = (
    "Tools/EffectPipeline/Schemas/"
    "lostark.effect-visual-program-corpus.schema.json"
)
ADMISSION_RELATIVE_PATH = (
    "Data/Effects/Imported/CombatBA/"
    "lmb-combo-3class.v1.runtime-admission.json"
)
NATIVE_SOURCE_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json"
)
RUNTIME_PROGRAM_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
LOCAL_DECAL_RECEIPT_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.local-decal-acquisition.receipt.json"
)
ACTIVE_INVENTORY_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/"
    "skill.31470.source-active-effect-inventory.receipt.json"
)
LANCE_34010_SOURCE_RECEIPT_RELATIVE_PATH = (
    "Data/Effects/Imported/LanceMaster/skill.34010.source-receipt.json"
)
LANCE_34010_RESTORATION_CANDIDATE_PATTERN = (
    "Data/Effects/Authored/"
    "effect.lancemaster.skill.34010.ba{stage}.restoration-candidate.effect.json"
)
WARLORD_CANARY_RELATIVE_PATH = (
    "Data/Effects/Imported/Warlord/Warlord.combat-source-stage-manifest.json"
)
VALTAN_CANARY_RELATIVE_PATH = (
    "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
DEFAULT_OUTPUT_RELATIVE_PATH = (
    "Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json"
)

CORPUS_SCHEMA = "lostark.effect-visual-program-corpus"
CORPUS_VERSION = 1
CORPUS_ID = "effect.visual-program-corpus.v1"
CONTRACT_ROLE = "PHASE2_EXECUTION_PROJECTION_INPUT_NOT_RUNTIME_CATALOG"

EXPECTED_STAGE_COUNT = 12
EXPECTED_SCHEDULE_COUNT = 35
EXPECTED_BA_ROW_COUNT = 133
EXPECTED_LOCAL_DECAL_ROW_COUNT = 2
EXPECTED_VISUAL_ROW_COUNT = 135
EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT = 5
EXPECTED_CASCADE_RIBBON_VISUAL_ROW_COUNT = 4
EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT = 4
EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT = 1
EXPECTED_LEGACY_COUNT = 66
EXPECTED_FAIL_CLOSED_COUNT = 63
EXPECTED_ADMITTED_COUNT = 72

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.-]+$")

FAMILY_BY_SOURCE = {
    "MESH_PARTICLE": "MESH_PARTICLE",
    "SPRITE_PARTICLE": "SPRITE_PARTICLE",
    "DECAL_PARTICLE": "DECAL_PARTICLE",
    "CASCADE_RIBBON": "CASCADE_RIBBON",
    "LIGHT_PARTICLE": "LIGHT_PARTICLE",
    "SCREEN_POST": "SCREEN_POST",
}
ADAPTER_BY_FAMILY = {
    "MESH_PARTICLE": "mesh-particle-document-v12",
    "SPRITE_PARTICLE": "sprite-particle-document-v12",
    "DECAL_PARTICLE": "decal-particle-document-v12",
    "CASCADE_RIBBON": "cascade-ribbon-document-v12",
    "ANIMATION_TRAIL": "animation-trail-document-v12",
    "LIGHT_PARTICLE": "light-particle-document-v12",
    "SCREEN_POST": "screen-post-document-v12",
}
RESOURCE_ROLE_BY_SLOT = {
    "base": "BASE_COLOR",
    "diffuse": "DIFFUSE",
    "dissolve": "DISSOLVE",
    "emissive": "EMISSIVE",
    "height": "HEIGHT",
    "mask": "MASK",
    "meshModel": "MESH_MODEL",
    "noise": "NOISE",
    "normal": "NORMAL",
    "specular": "SPECULAR",
}
ALL_RESOURCE_ROLES = tuple(sorted(set(RESOURCE_ROLE_BY_SLOT.values())))


class ContractError(ValueError):
    """Raised when any parse, identity, join, or admission invariant fails."""


def _reject_non_finite(value: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {value}")


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8-sig"),
            parse_constant=_reject_non_finite,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ContractError(f"cannot parse JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_json_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as error:
        raise ContractError(f"cannot hash input {path}: {error}") from error
    return digest.hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2)
        + "\n"
    ).encode("utf-8")


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def _require_dict(value: Any, label: str) -> dict[str, Any]:
    _require(isinstance(value, dict), f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    _require(isinstance(value, list), f"{label} must be an array")
    return value


def _require_string(value: Any, label: str) -> str:
    _require(isinstance(value, str) and bool(value), f"{label} must be a string")
    return value


def _require_sha(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(bool(SHA256_RE.fullmatch(text)), f"{label} must be lowercase SHA-256")
    return text


def _require_stable_id(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(bool(STABLE_ID_RE.fullmatch(text)), f"{label} is not a stable ID")
    return text


def _expect_keys(value: dict[str, Any], expected: Iterable[str], label: str) -> None:
    expected_set = set(expected)
    actual = set(value)
    _require(
        actual == expected_set,
        f"{label} keys mismatch: missing={sorted(expected_set - actual)} "
        f"extra={sorted(actual - expected_set)}",
    )


def _relative_path(path_text: Any, label: str) -> str:
    text = _require_string(path_text, label).replace("\\", "/")
    path = PurePosixPath(text)
    _require(not path.is_absolute(), f"{label} must be repository-relative")
    _require(".." not in path.parts, f"{label} cannot escape the repository")
    _require(not re.match(r"^[A-Za-z]:", text), f"{label} cannot be drive-qualified")
    return text


def _asset_id(asset_id: Any, label: str) -> str:
    text = _relative_path(asset_id, label)
    _require(text.startswith("Effect/"), f"{label} must be Effect-relative")
    return text


def _seal_row(row: dict[str, Any], field: str) -> dict[str, Any]:
    _require(field not in row, f"row is already sealed with {field}")
    row[field] = canonical_json_sha256(row)
    return row


def _verify_seal(row: dict[str, Any], field: str, label: str) -> None:
    expected = _require_sha(row.get(field), f"{label}.{field}")
    unsigned = copy.deepcopy(row)
    del unsigned[field]
    _require(canonical_json_sha256(unsigned) == expected, f"{label}.{field} is stale")


def _input_artifact(
    repository_root: Path,
    relative_path: str,
    roles: Iterable[str],
) -> dict[str, Any]:
    relative_path = _relative_path(relative_path, "input artifact path")
    path = repository_root / relative_path
    _require(path.is_file(), f"missing input artifact: {relative_path}")
    role_list = sorted({_require_stable_id(role, "input role") for role in roles})
    _require(bool(role_list), f"input artifact has no role: {relative_path}")
    return {
        "path": relative_path,
        "rawSha256": raw_sha256(path),
        "roles": role_list,
    }


def _merge_input_artifact(
    registry: dict[str, dict[str, Any]],
    artifact: dict[str, Any],
) -> None:
    path = artifact["path"]
    existing = registry.get(path)
    if existing is None:
        registry[path] = copy.deepcopy(artifact)
        return
    _require(
        existing["rawSha256"] == artifact["rawSha256"],
        f"conflicting input hash: {path}",
    )
    existing["roles"] = sorted(set(existing["roles"]) | set(artifact["roles"]))


def _validate_sealed_document(document: dict[str, Any], field: str, label: str) -> None:
    if field not in document:
        return
    _verify_seal(document, field, label)


def _load_admission(repository_root: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    artifact = _input_artifact(
        repository_root,
        ADMISSION_RELATIVE_PATH,
        ["COMBAT_BA_RUNTIME_ADMISSION_INDEX"],
    )
    admission = load_json(repository_root / ADMISSION_RELATIVE_PATH)
    _require(
        admission.get("schema") == "lostark.effect-source-runtime-admission-index"
        and admission.get("formatVersion") == 1,
        "combat BA admission header mismatch",
    )
    _validate_sealed_document(admission, "artifactSha256", "combat BA admission")
    _require(admission.get("contractRole") == "EVIDENCE_JOIN_NOT_RUNTIME_AUTHORITY", "combat BA admission role changed")
    denominators = _require_dict(admission.get("denominators"), "admission.denominators")
    expected = {
        "stageCount": EXPECTED_STAGE_COUNT,
        "particleOccurrenceCount": EXPECTED_SCHEDULE_COUNT,
        "sourceRendererRowCount": EXPECTED_BA_ROW_COUNT,
        "currentLegacyEmitterCount": EXPECTED_LEGACY_COUNT,
    }
    for key, value in expected.items():
        _require(denominators.get(key) == value, f"admission denominator changed: {key}")
    global_admission = _require_dict(admission.get("admission"), "admission.admission")
    _require(global_admission.get("currentProductMutation") is False, "source admission mutated current product")
    return admission, artifact


def _adapter_contracts() -> list[dict[str, Any]]:
    contracts = []
    for family, adapter_id in sorted(ADAPTER_BY_FAMILY.items(), key=lambda item: item[1]):
        packet_layouts = ["EFFECT_DOCUMENT_ELEMENT_V12", "NONE"]
        if adapter_id == "local-decal-rt0-bounded-v1":
            packet_layouts = ["LOCAL_DECAL_RT0_SIX_SRV_V1", "NONE"]
        elif adapter_id == "cascade-ribbon-document-v12":
            packet_layouts = [
                "EFFECT_DOCUMENT_ELEMENT_V12",
                "CASCADE_RIBBON_TYPED_PACKET_V1",
                "NONE",
            ]
        elif adapter_id == "animation-trail-document-v12":
            packet_layouts = ["ANIMATION_TRAIL_ELEMENT_V1", "NONE"]
        contracts.append(
            {
                "adapterId": adapter_id,
                "family": family,
                "packetLayouts": packet_layouts,
                "allowedResourceRoles": list(ALL_RESOURCE_ROLES),
            }
        )
    contracts.append(
        {
            "adapterId": "local-decal-rt0-bounded-v1",
            "family": "DECAL_PARTICLE",
            "packetLayouts": ["LOCAL_DECAL_RT0_SIX_SRV_V1", "NONE"],
            "allowedResourceRoles": list(ALL_RESOURCE_ROLES),
        }
    )
    return sorted(contracts, key=lambda row: row["adapterId"])


def _resource_role(
    repository_root: Path,
    slot_id: str,
    asset_id: str,
    resolution_status: str,
    *,
    role: str | None = None,
    shader_register: str | None = None,
    source_channel: str | None = None,
    require_payload: bool = False,
) -> dict[str, Any]:
    asset_id = _asset_id(asset_id, "resource assetId")
    role = role or RESOURCE_ROLE_BY_SLOT.get(slot_id)
    _require(role in ALL_RESOURCE_ROLES, f"unknown resource slot/role: {slot_id}/{role}")
    path = repository_root / "Client/Bin/Resources" / PurePosixPath(asset_id)
    exists = path.is_file()
    _require(not require_payload or exists, f"missing admitted resource payload: {asset_id}")
    return {
        "role": role,
        "slotId": slot_id,
        "assetId": asset_id,
        "resolutionStatus": resolution_status,
        "rawSha256": raw_sha256(path) if exists else None,
        "byteCount": path.stat().st_size if exists else None,
        "shaderRegister": shader_register,
        "sourceChannel": source_channel,
    }


def _find_record(document: dict[str, Any], relative_path: str, record_id: str) -> dict[str, Any]:
    if "emitters" in document and relative_path.endswith("runtime-program.candidate.json"):
        matches = [row for row in document["emitters"] if row.get("evidenceId") == record_id]
    elif relative_path.endswith("source-receipt.json") and isinstance(document.get("timeline"), dict):
        matches = [
            row for row in document["timeline"].get("events", [])
            if isinstance(row, dict) and row.get("eventId") == record_id
        ]
    elif "document" in document and isinstance(document.get("document"), dict):
        matches = [row for row in document["document"].get("elements", []) if row.get("id") == record_id]
    else:
        matches = [row for row in document.get("elements", []) if row.get("id") == record_id]
    _require(len(matches) == 1, f"payload record join is not unique: {relative_path}/{record_id}")
    return _require_dict(matches[0], f"payload record {record_id}")


def _record_sha(record: dict[str, Any]) -> str:
    if "rowSha256" in record:
        _verify_seal(record, "rowSha256", "sealed payload record")
        return record["rowSha256"]
    return canonical_json_sha256(record)


def _payload_ref(
    repository_root: Path,
    relative_path: str,
    record_id: str,
    expected_record_sha: str | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    relative_path = _relative_path(relative_path, "payload path")
    path = repository_root / relative_path
    document = load_json(path)
    record = _find_record(document, relative_path, record_id)
    record_sha = _record_sha(record)
    if expected_record_sha is not None:
        _require(record_sha == expected_record_sha, f"payload record SHA changed: {relative_path}/{record_id}")
    return (
        {
            "path": relative_path,
            "rawSha256": raw_sha256(path),
            "recordId": record_id,
            "recordSha256": record_sha,
        },
        record,
    )


def _build_schedules(admission: dict[str, Any]) -> list[dict[str, Any]]:
    schedules = []
    for raw in _require_list(admission.get("particleOccurrences"), "particleOccurrences"):
        occurrence = _require_dict(raw, "particleOccurrence")
        schedule = {
            "presentationOccurrenceId": _require_stable_id(occurrence.get("occurrenceId"), "occurrenceId"),
            "stageId": _require_stable_id(occurrence.get("stageId"), "occurrence.stageId"),
            "effectAssetId": _require_stable_id(occurrence.get("productEffectAssetId"), "occurrence.effectAssetId"),
            "sourceEventId": occurrence.get("sourceEventId"),
            "stageLocalTimeSeconds": occurrence.get("stageLocalTimeSeconds"),
            "sourceRowIds": list(occurrence.get("sourceRowIds", [])),
        }
        _seal_row(schedule, "scheduleSha256")
        schedules.append(schedule)
    schedules.sort(key=lambda row: row["presentationOccurrenceId"])
    _require(len(schedules) == EXPECTED_SCHEDULE_COUNT, "presentation schedule count changed")
    return schedules


def _ba_occurrence_id(effect_asset_id: str, source_element_id: str) -> str:
    identity = {
        "effectAssetId": effect_asset_id,
        "sourceElementId": source_element_id,
    }
    return f"visual-occurrence.{canonical_json_sha256(identity)}"


def _source_import_path_by_product(
    repository_root: Path,
    admission: dict[str, Any],
) -> dict[str, str]:
    result: dict[str, str] = {}
    for artifact in _require_list(admission.get("inputArtifacts"), "admission.inputArtifacts"):
        item = _require_dict(artifact, "admission.inputArtifact")
        if "IMPORTED_SOURCE_DOCUMENT" not in item.get("roles", []):
            continue
        path = _relative_path(item.get("path"), "imported source path")
        document = load_json(repository_root / path)
        imported_id = _require_string(document.get("effectAssetId"), "imported effectAssetId")
        _require(imported_id.endswith(".imported"), f"unexpected imported effect ID: {imported_id}")
        product_prefix = imported_id[: -len(".imported")]
        _require(product_prefix not in result, f"duplicate imported source prefix: {product_prefix}")
        result[product_prefix] = path
    return result


def _typed_family_resolution(
    source_element: dict[str, Any],
    declared_renderer_family: str,
) -> tuple[str, dict[str, Any], dict[str, Any]]:
    """Resolve a family from TypeData before legacy renderer metadata.

    UE3 ParticleSpriteEmitter is also the container for mesh and ribbon
    TypeData.  Therefore rendererFamily/rendererShape are fallback evidence,
    never authority when exactly one supported TypeData module is present.
    """
    recipe = _require_dict(source_element.get("sourceRecipe"), "source element SourceRecipe")
    modules = _require_list(recipe.get("modules"), "source element SourceRecipe.modules")
    typed = []
    for module_value in modules:
        module = _require_dict(module_value, "source TypeData module")
        class_name = str(module.get("className", "")).lower()
        if class_name in {"particlemoduletypedatamesh", "particlemoduletypedataribbon"}:
            typed.append((class_name, module))
    _require(len(typed) <= 1, "source emitter has conflicting supported TypeData modules")
    if typed:
        class_name, module = typed[0]
        family = (
            "MESH_PARTICLE"
            if class_name == "particlemoduletypedatamesh"
            else "CASCADE_RIBBON"
        )
        resolution_kind = "TYPE_DATA_MODULE_V1"
        module_stable_id = _require_string(module.get("stableId"), "TypeData stableId")
        module_object_path = _require_string(module.get("objectPath"), "TypeData objectPath")
        module_sha = canonical_json_sha256(module)
    else:
        family = FAMILY_BY_SOURCE.get(declared_renderer_family)
        _require(family is not None, f"unknown source renderer family: {declared_renderer_family}")
        resolution_kind = "DECLARED_RENDERER_FAMILY_FALLBACK_V1"
        class_name = ""
        module_stable_id = ""
        module_object_path = ""
        module_sha = None
    resolved_shape = {
        "MESH_PARTICLE": "mesh",
        "SPRITE_PARTICLE": "sprite",
        "DECAL_PARTICLE": "decal",
        "CASCADE_RIBBON": "ribbon",
        "LIGHT_PARTICLE": "light",
        "SCREEN_POST": "screenPost",
    }[family]
    projected_recipe = copy.deepcopy(recipe)
    projected_recipe["rendererShape"] = resolved_shape
    resolution = {
        "resolutionKind": resolution_kind,
        "declaredRendererFamily": declared_renderer_family,
        "declaredRendererShape": str(recipe.get("rendererShape", "")),
        "resolvedFamily": family,
        "resolvedRendererShape": resolved_shape,
        "typeDataStableId": module_stable_id,
        "typeDataClassName": class_name,
        "typeDataObjectPath": module_object_path,
        "typeDataModuleSha256": module_sha,
    }
    _seal_row(resolution, "resolutionSha256")
    return family, resolution, projected_recipe


def _lance_restoration_target(
    repository_root: Path,
    stage_index: int,
    suffix: str,
) -> tuple[str, dict[str, Any], dict[str, Any]]:
    stage_number = stage_index + 1
    path = LANCE_34010_RESTORATION_CANDIDATE_PATTERN.format(stage=stage_number)
    record_id = f"manual.trackb.ba{stage_number}.{suffix}"
    payload, record = _payload_ref(repository_root, path, record_id)
    _require(record.get("kind") == "trail", f"restoration target is not a trail: {record_id}")
    return path, payload, record


def _product_prefix(effect_asset_id: str) -> str:
    match = re.fullmatch(r"(.+)\.ba[1-9][0-9]*", effect_asset_id)
    _require(match is not None, f"invalid BA product effect asset ID: {effect_asset_id}")
    return match.group(1)


def _target_component_records(
    repository_root: Path,
    stage: dict[str, Any],
) -> dict[str, tuple[str, dict[str, Any], dict[str, Any]]]:
    result: dict[str, tuple[str, dict[str, Any], dict[str, Any]]] = {}
    current = _require_dict(stage.get("currentProduct"), "stage.currentProduct")
    for cue_value in _require_list(current.get("componentCues"), "componentCues"):
        cue = _require_dict(cue_value, "componentCue")
        path = _relative_path(cue.get("componentPath"), "componentPath")
        _require(raw_sha256(repository_root / path) == cue.get("componentRawSha256"), f"component hash changed: {path}")
        component = load_json(repository_root / path)
        document = _require_dict(component.get("document"), f"component.document {path}")
        for element_value in _require_list(document.get("elements"), f"component.elements {path}"):
            element = _require_dict(element_value, "component element")
            element_id = _require_string(element.get("id"), "component element ID")
            _require(element_id not in result, f"duplicate executable element in stage: {element_id}")
            result[element_id] = (path, cue, element)
    return result


def _build_ba_rows(
    repository_root: Path,
    admission: dict[str, Any],
    input_registry: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    stages = {
        _require_stable_id(stage.get("stageId"), "stageId"): stage
        for stage in _require_list(admission.get("stages"), "admission.stages")
    }
    _require(len(stages) == EXPECTED_STAGE_COUNT, "BA stage count changed")
    source_paths = _source_import_path_by_product(repository_root, admission)
    target_records = {
        stage_id: _target_component_records(repository_root, stage)
        for stage_id, stage in stages.items()
    }
    rows = []
    for source_value in _require_list(admission.get("sourceRows"), "admission.sourceRows"):
        source = _require_dict(source_value, "sourceRow")
        effect_asset_id = _require_stable_id(source.get("productEffectAssetId"), "source productEffectAssetId")
        source_element_id = _require_string(source.get("sourceElementId"), "sourceElementId")
        source_path = source_paths.get(_product_prefix(effect_asset_id))
        _require(source_path is not None, f"no imported source document for {effect_asset_id}")
        source_payload, source_element = _payload_ref(
            repository_root,
            source_path,
            source_element_id,
            _require_sha(source.get("sourceElementSha256"), "sourceElementSha256"),
        )
        _merge_input_artifact(
            input_registry,
            _input_artifact(repository_root, source_path, ["COMBAT_BA_SOURCE_PAYLOAD"]),
        )

        source_family = _require_string(source.get("rendererFamily"), "rendererFamily")
        family, typed_family_resolution, projected_source_recipe = (
            _typed_family_resolution(source_element, source_family)
        )
        adapter_id = ADAPTER_BY_FAMILY[family]
        source_recipe_raw = _require_dict(source.get("sourceRecipe"), "sourceRecipe")
        source_recipe_evidence_sha = canonical_json_sha256(
            _require_dict(source_element.get("sourceRecipe"), "source payload SourceRecipe")
        )
        _require(
            source_recipe_evidence_sha ==
            _require_sha(source_recipe_raw.get("sourceRecipeSha256"), "sourceRecipeSha256"),
            "source admission recipe SHA diverged from imported payload",
        )
        source_recipe = {
            "enabled": source_recipe_raw.get("enabled") is True,
            "rendererShape": str(source_recipe_raw.get("rendererShape", "")),
            "resolvedRendererShape": typed_family_resolution["resolvedRendererShape"],
            "sourceRowSha256": canonical_json_sha256(source),
            "sourceRecipeEvidenceSha256": source_recipe_evidence_sha,
            "recipeSha256": canonical_json_sha256(projected_source_recipe),
            "moduleClosureSha256": _require_sha(source_recipe_raw.get("moduleClosureSha256"), "moduleClosureSha256"),
            "moduleCount": source_recipe_raw.get("moduleCount"),
            "typedFamilyResolution": typed_family_resolution,
        }
        attachment = _require_dict(source.get("attachment"), "source attachment")
        source_import_basis = {
            "basisId": _require_stable_id(source.get("importBasisId"), "importBasisId"),
            "status": "UNRESOLVED_FAIL_CLOSED",
            "attachmentSha256": _require_sha(attachment.get("attachmentSha256"), "attachmentSha256"),
        }
        legacy = _require_dict(source.get("legacyProjection"), "legacyProjection")
        selected = legacy.get("selectionDecision") == "selected"
        typed_ribbon_projection = family == "CASCADE_RIBBON"
        admission_row = _require_dict(source.get("admission"), "source admission")
        original_blockers = sorted(
            {_require_stable_id(item, "source blocker") for item in admission_row.get("blockers", [])}
        )
        target_payload = None
        resource_roles: list[dict[str, Any]] = []
        if selected:
            _require(admission_row.get("disposition") == "LEGACY_APPROXIMATION", "selected row lost legacy disposition")
            stage_id = _require_stable_id(source.get("stageId"), "source stageId")
            target_id = _require_string(legacy.get("targetElementId"), "legacy targetElementId")
            target = target_records[stage_id].get(target_id)
            _require(target is not None, f"legacy target is not executable in stage: {stage_id}/{target_id}")
            target_path, cue, target_element = target
            target_payload, _ = _payload_ref(repository_root, target_path, target_id)
            _merge_input_artifact(
                input_registry,
                _input_artifact(repository_root, target_path, ["COMBAT_BA_EXECUTABLE_TARGET_PAYLOAD"]),
            )
            target_kind = target_element.get("kind")
            expected_kind = "mesh" if family == "MESH_PARTICLE" else "sprite"
            _require(target_kind == expected_kind, f"legacy target family mismatch: {stage_id}/{target_id}")
            for resource in target_element.get("resources", []):
                item = _require_dict(resource, "target resource")
                resource_roles.append(
                    _resource_role(
                        repository_root,
                        _require_string(item.get("slotId"), "target resource slot"),
                        _require_string(item.get("assetId"), "target resource asset"),
                        "TARGET_DOCUMENT_REFERENCE",
                        require_payload=True,
                    )
                )
            _require(bool(resource_roles), f"admitted legacy target has no resources: {target_id}")
            execution_basis = {
                "status": "TARGET_DOCUMENT_BOUNDED",
                "basisSha256": canonical_json_sha256(
                    {
                        "componentPath": target_path,
                        "componentRawSha256": target_payload["rawSha256"],
                        "targetElementId": target_id,
                        "targetExecutableElementSha256": target_payload["recordSha256"],
                        "selectionEvidenceElementSha256": _require_sha(legacy.get("targetElementSha256"), "legacy targetElementSha256"),
                        "localTransformSha256": _require_sha(cue.get("localTransformSha256"), "component localTransformSha256"),
                    }
                ),
            }
            projection = {
                "family": family,
                "adapterId": adapter_id,
                "packetLayout": "EFFECT_DOCUMENT_ELEMENT_V12",
                "tuningEligibleTransform": True,
                "fidelity": "LEGACY_APPROXIMATION",
                "disposition": "ADMITTED_BOUNDED",
                "nativeExecution": False,
                "sourceExact": False,
                "executionBasis": execution_basis,
                "resourceRoles": sorted(resource_roles, key=lambda item: (item["slotId"], item["assetId"])),
                "admissionBlockers": [],
                "preservedLimitations": sorted(set(original_blockers) | {"LEGACY_APPROXIMATION_NOT_SOURCE_EXACT"}),
                "productMutation": False,
            }
        elif typed_ribbon_projection:
            _require(
                source.get("characterClass") == "LANCE_MASTER" and
                source.get("skillId") == 34010,
                "bounded CascadeRibbon target is outside the pinned Lance BA corpus",
            )
            stage_index = source.get("stageIndex")
            _require(
                type(stage_index) is int and 0 <= stage_index < 4,
                "Lance CascadeRibbon stage index is invalid",
            )
            target_path, target_payload, target_element = _lance_restoration_target(
                repository_root, stage_index, "ribbon-companion-blocked"
            )
            _merge_input_artifact(
                input_registry,
                _input_artifact(
                    repository_root,
                    target_path,
                    ["COMBAT_BA_CASCADE_RIBBON_TARGET_PAYLOAD"],
                ),
            )
            for resource in _require_list(target_element.get("resources"), "CascadeRibbon resources"):
                item = _require_dict(resource, "CascadeRibbon resource")
                resource_roles.append(
                    _resource_role(
                        repository_root,
                        _require_string(item.get("slotId"), "CascadeRibbon resource slot"),
                        _require_string(item.get("assetId"), "CascadeRibbon resource asset"),
                        "TARGET_DOCUMENT_REFERENCE",
                        require_payload=True,
                    )
                )
            execution_basis = {
                "status": "TARGET_DOCUMENT_BOUNDED",
                "basisSha256": canonical_json_sha256(
                    {
                        "targetPayload": target_payload,
                        "typedFamilyResolutionSha256": typed_family_resolution["resolutionSha256"],
                        "projectedSourceRecipeSha256": source_recipe["recipeSha256"],
                    }
                ),
            }
            projection = {
                "family": family,
                "adapterId": adapter_id,
                "packetLayout": "EFFECT_DOCUMENT_ELEMENT_V12",
                "tuningEligibleTransform": True,
                "fidelity": "BOUNDED_RECONSTRUCTION",
                "disposition": "ADMITTED_BOUNDED",
                "nativeExecution": False,
                "sourceExact": False,
                "executionBasis": execution_basis,
                "resourceRoles": sorted(resource_roles, key=lambda item: (item["slotId"], item["assetId"])),
                "admissionBlockers": [],
                "preservedLimitations": [
                    "BOUNDED_CASCADE_RIBBON_NOT_NATIVE_SOURCE_EXACT",
                    "MANUAL_TARGET_CARRIER_VALUES_REQUIRE_USER_REVIEW",
                ],
                "productMutation": False,
            }
        else:
            for resource in source.get("resourceBindings", []):
                item = _require_dict(resource, "source evidence resource")
                resource_roles.append(
                    _resource_role(
                        repository_root,
                        _require_string(item.get("slotId"), "source resource slot"),
                        _require_string(item.get("assetId"), "source resource asset"),
                        "SOURCE_EVIDENCE_ONLY",
                    )
                )
            _require(bool(original_blockers), f"fail-closed row has no blocker: {source.get('sourceRowId')}")
            projection = {
                "family": family,
                "adapterId": adapter_id,
                "packetLayout": "NONE",
                "tuningEligibleTransform": False,
                "fidelity": "EVIDENCE_ONLY",
                "disposition": "FAIL_CLOSED",
                "nativeExecution": False,
                "sourceExact": False,
                "executionBasis": {
                    "status": "UNRESOLVED_FAIL_CLOSED",
                    "basisSha256": canonical_json_sha256(
                        {
                            "importBasisId": source_import_basis["basisId"],
                            "attachmentSha256": source_import_basis["attachmentSha256"],
                            "blockers": original_blockers,
                        }
                    ),
                },
                "resourceRoles": sorted(resource_roles, key=lambda item: (item["slotId"], item["assetId"])),
                "admissionBlockers": original_blockers,
                "preservedLimitations": [],
                "productMutation": False,
            }

        selector = {
            "effectAssetId": effect_asset_id,
            "occurrenceId": _ba_occurrence_id(effect_asset_id, source_element_id),
        }
        row = {
            "selector": selector,
            "selectorSha256": canonical_json_sha256(selector),
            "provenance": {
                "scope": "COMBAT_BA",
                "characterClass": _require_string(source.get("characterClass"), "characterClass"),
                "skillId": source.get("skillId"),
                "stageIndex": source.get("stageIndex"),
                "sourceOrder": source.get("sourceOrder"),
                "sourceElementId": source_element_id,
                "sourceRowId": _require_stable_id(source.get("sourceRowId"), "sourceRowId"),
            },
            "schedule": {
                "stageId": source.get("stageId"),
                "presentationOccurrenceId": source.get("particleOccurrenceId"),
                "joinKind": _require_string(source.get("stageJoin", {}).get("kind"), "stageJoin.kind"),
                "sourceEventId": source.get("stageJoin", {}).get("sourceEventId"),
                "sourceTimelineSeconds": source.get("stageJoin", {}).get("sourceTimelineSeconds"),
                "externalScheduleGroupId": None,
            },
            "sourcePayload": source_payload,
            "targetPayload": target_payload,
            "sourceRecipe": source_recipe,
            "sourceImportBasis": source_import_basis,
            "executionProjection": projection,
        }
        _seal_row(row, "rowSha256")
        rows.append(row)
    _require(len(rows) == EXPECTED_BA_ROW_COUNT, "combat BA visual row count changed")
    return rows


def _local_decal_resource_roles(
    repository_root: Path,
    receipt: dict[str, Any],
    inventory: dict[str, Any],
) -> list[dict[str, Any]]:
    identities = {
        item["assetId"]: item
        for item in _require_list(inventory.get("runtimeResourceIdentities"), "runtimeResourceIdentities")
        if isinstance(item, dict) and isinstance(item.get("assetId"), str)
    }
    active = [
        item for item in inventory.get("activeElements", [])
        if isinstance(item, dict) and item.get("activeElementId") == "source-active-020"
    ]
    _require(len(active) == 1, "Artist F LocalDecal active inventory row #20 is not unique")
    mappings = {
        item.get("parameterName"): item
        for item in active[0].get("resourceMappings", [])
        if isinstance(item, dict)
    }
    role_specs = [
        ("DIFFUSE", "diffuse", "01.diffmap_a", "t1", "RGBA"),
        ("NORMAL", "normal", "06.normalmap", "t3", "RG"),
        ("SPECULAR", "specular", "01.specmap", "t4", "RGB"),
        ("EMISSIVE", "emissive", "01.emismap", "t5", "R"),
    ]
    result = []
    for role, slot_id, parameter, register, channel in role_specs:
        mapping = _require_dict(mappings.get(parameter), f"LocalDecal mapping {parameter}")
        asset_id = _require_string(mapping.get("assetId"), f"LocalDecal asset {parameter}")
        identity = _require_dict(identities.get(asset_id), f"runtime identity {asset_id}")
        resource = _resource_role(
            repository_root,
            slot_id,
            asset_id,
            "EXACT_RUNTIME_PAYLOAD_SHA256",
            role=role,
            shader_register=register,
            source_channel=channel,
            require_payload=True,
        )
        _require(resource["rawSha256"] == identity.get("sha256"), f"runtime identity SHA changed: {asset_id}")
        _require(resource["byteCount"] == identity.get("bytes"), f"runtime identity size changed: {asset_id}")
        result.append(resource)
    by_semantic = {
        item.get("sourceSemantic"): item
        for item in _require_list(receipt.get("assets"), "local decal receipt assets")
        if isinstance(item, dict)
    }
    for role, slot_id, semantic, register in (
        ("HEIGHT", "height", "height_parallax", "t0"),
        ("DISSOLVE", "dissolve", "dissolve_mask", "t2"),
    ):
        item = _require_dict(by_semantic.get(semantic), f"LocalDecal source semantic {semantic}")
        dds = _require_dict(item.get("dds"), f"LocalDecal DDS {semantic}")
        resource = _resource_role(
            repository_root,
            slot_id,
            _require_string(item.get("runtimeAssetId"), f"LocalDecal runtimeAssetId {semantic}"),
            "EXACT_RUNTIME_PAYLOAD_SHA256",
            role=role,
            shader_register=register,
            source_channel=_require_string(item.get("sourceShaderChannel"), f"LocalDecal channel {semantic}"),
            require_payload=True,
        )
        _require(resource["rawSha256"] == dds.get("rawSha256"), f"LocalDecal exact DDS SHA changed: {semantic}")
        _require(resource["byteCount"] == dds.get("byteCount"), f"LocalDecal exact DDS size changed: {semantic}")
        result.append(resource)
    result.sort(key=lambda item: int(item["shaderRegister"][1:]))
    _require([item["shaderRegister"] for item in result] == [f"t{i}" for i in range(6)], "LocalDecal packet is not a six-SRV t0..t5 layout")
    return result


def _supplemental_occurrence_id(
    effect_asset_id: str,
    stable_source_id: str,
    family: str,
) -> str:
    identity = {
        "effectAssetId": effect_asset_id,
        "stableSourceId": stable_source_id,
        "family": family,
    }
    return f"visual-occurrence.{canonical_json_sha256(identity)}"


def _trail_target_packet(record: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any]]:
    detail = _require_dict(record.get("detail"), "trail target detail")
    timing = copy.deepcopy(_require_dict(detail.get("timing"), "trail target timing"))
    trail = copy.deepcopy(_require_dict(detail.get("trail"), "trail target geometry"))
    attachment = copy.deepcopy(
        _require_dict(record.get("actionCueAttachment"), "trail target attachment")
    )
    return timing, {"attachment": attachment, "trail": trail}


def _build_animation_trail_supplemental_elements(
    repository_root: Path,
    input_registry: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    source_path = LANCE_34010_SOURCE_RECEIPT_RELATIVE_PATH
    source_document = load_json(repository_root / source_path)
    _merge_input_artifact(
        input_registry,
        _input_artifact(repository_root, source_path, ["ANIMATION_TRAIL_SOURCE_NOTIFY"]),
    )
    events = [
        item for item in _require_list(source_document.get("timeline", {}).get("events"), "Lance timeline events")
        if isinstance(item, dict) and item.get("sourceType") == "Trails"
    ]
    _require(len(events) == EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT, "Lance AnimationTrail notify denominator changed")
    result: list[dict[str, Any]] = []
    for event in events:
        stage_index = event.get("clipSequenceIndex")
        _require(type(stage_index) is int and 0 <= stage_index < 4, "AnimationTrail stage index is invalid")
        stage_number = stage_index + 1
        effect_asset_id = f"effect.lancemaster.skill.34010.ba{stage_number}"
        event_id = _require_stable_id(event.get("eventId"), "AnimationTrail eventId")
        source_payload, source_record = _payload_ref(
            repository_root, source_path, event_id
        )
        target_path, target_payload, target_record = _lance_restoration_target(
            repository_root, stage_index, "animtrail-companion"
        )
        _merge_input_artifact(
            input_registry,
            _input_artifact(
                repository_root,
                target_path,
                ["ANIMATION_TRAIL_TARGET_ELEMENT"],
            ),
        )
        target_timing, target_contract = _trail_target_packet(target_record)
        _require(
            abs(float(target_timing.get("startDelaySeconds")) - float(event.get("localTimeSeconds"))) <= 5e-5 and
            abs(float(target_timing.get("lifeTimeSeconds")) - float(event.get("durationSeconds"))) <= 5e-5,
            f"AnimationTrail target timing diverged from source notify: {event_id}",
        )
        resources = []
        for resource in _require_list(target_record.get("resources"), "AnimationTrail resources"):
            item = _require_dict(resource, "AnimationTrail resource")
            resources.append(
                _resource_role(
                    repository_root,
                    _require_string(item.get("slotId"), "AnimationTrail resource slot"),
                    _require_string(item.get("assetId"), "AnimationTrail resource asset"),
                    "TARGET_DOCUMENT_REFERENCE",
                    require_payload=True,
                )
            )
        _require(bool(resources), f"AnimationTrail target has no resources: {event_id}")
        packet = {
            "packetVersion": 1,
            "adapterId": "animation-trail-document-v12",
            "boundedSemanticReplay": True,
            "nativeExecution": False,
            "runtimeCarrier": "EFFECT_TYPED_ANIMATION_TRAIL_V1",
            "sourceNotifyType": "Trails",
            "sourceEventId": event_id,
            "sourceEventRecordSha256": canonical_json_sha256(source_record),
            "sourceAsset": _require_string(event.get("sourceAsset"), "AnimationTrail sourceAsset"),
            "clip": _require_string(event.get("clip"), "AnimationTrail clip"),
            "localTimeSeconds": event.get("localTimeSeconds"),
            "globalTimeSeconds": event.get("globalTimeSeconds"),
            "durationSeconds": event.get("durationSeconds"),
            "targetElementId": _require_string(target_record.get("id"), "AnimationTrail targetElementId"),
            "targetTiming": target_timing,
            "attachment": target_contract["attachment"],
            "trail": target_contract["trail"],
            "preservedLimitations": [
                "ANIMATION_TRAIL_BOUNDED_RECONSTRUCTION_NOT_NATIVE_SOURCE_EXACT"
            ],
        }
        _seal_row(packet, "packetSha256")
        selector = {
            "effectAssetId": effect_asset_id,
            "occurrenceId": _supplemental_occurrence_id(
                effect_asset_id, event_id, "ANIMATION_TRAIL"
            ),
        }
        row = {
            "selector": selector,
            "selectorSha256": canonical_json_sha256(selector),
            "provenance": {
                "scope": "COMBAT_BA_ANIMATION_TRAIL",
                "characterClass": "LANCE_MASTER",
                "skillId": 34010,
                "stageIndex": stage_index,
                "sourceStableId": event_id,
            },
            "schedule": {
                "stageId": f"skill.34010.stage.{stage_index}",
                "sourceEventId": event_id,
                "sourceTimelineSeconds": event.get("globalTimeSeconds"),
                "localTimeSeconds": event.get("localTimeSeconds"),
                "durationSeconds": event.get("durationSeconds"),
            },
            "sourcePayload": source_payload,
            "targetPayload": target_payload,
            "family": "ANIMATION_TRAIL",
            "adapterId": "animation-trail-document-v12",
            "packetLayout": "ANIMATION_TRAIL_ELEMENT_V1",
            "fidelity": "BOUNDED_RECONSTRUCTION",
            "disposition": "ADMITTED_BOUNDED",
            "tuningEligibleTransform": True,
            "resourcePacket": sorted(resources, key=lambda item: (item["slotId"], item["assetId"])),
            "cascadeRibbonPacket": None,
            "animationTrailPacket": packet,
            "admissionBlockers": [],
        }
        _seal_row(row, "rowSha256")
        result.append(row)
    return result


def _find_typed_module(
    source_recipe: dict[str, Any], class_name: str
) -> dict[str, Any]:
    matches = [
        item for item in _require_list(source_recipe.get("modules"), "SourceRecipe.modules")
        if isinstance(item, dict) and str(item.get("className", "")).lower() == class_name
    ]
    _require(len(matches) == 1, f"typed module join is not unique: {class_name}")
    return matches[0]


def _literal_number(
    module: dict[str, Any],
    property_path: str,
    default: float | None = None,
) -> float:
    matches = [
        item.get("value") for item in _require_list(module.get("literals"), "typed module literals")
        if isinstance(item, dict) and item.get("propertyPath") == property_path and item.get("kind") == "number"
    ]
    if not matches and default is not None:
        return default
    _require(len(matches) == 1 and isinstance(matches[0], (int, float)), f"missing typed literal: {property_path}")
    return float(matches[0])


def _build_artist_cascade_ribbon_supplemental_element(
    repository_root: Path,
    input_registry: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    source_path = RUNTIME_PROGRAM_RELATIVE_PATH
    source_payload, emitter = _payload_ref(
        repository_root, source_path, "source-active-003"
    )
    target_path = NATIVE_SOURCE_RELATIVE_PATH
    target_id = _require_string(emitter.get("sourceElementId"), "Artist ribbon sourceElementId")
    target_payload, target = _payload_ref(repository_root, target_path, target_id)
    for path, role in (
        (source_path, "ARTIST_CASCADE_RIBBON_RUNTIME_SOURCE"),
        (target_path, "ARTIST_CASCADE_RIBBON_TARGET_ELEMENT"),
    ):
        _merge_input_artifact(input_registry, _input_artifact(repository_root, path, [role]))
    _require(target.get("kind") == "trail", "Artist source-active-003 target is not a trail")
    source_recipe = _require_dict(target.get("sourceRecipe"), "Artist ribbon SourceRecipe")
    _require(source_recipe.get("enabled") is True and source_recipe.get("rendererShape") == "ribbon", "Artist ribbon SourceRecipe is not enabled ribbon")
    typed = _find_typed_module(source_recipe, "particlemoduletypedataribbon")
    target_timing, target_contract = _trail_target_packet(target)
    adapter = _require_dict(emitter.get("ribbonAdapter"), "Artist ribbon adapter")
    packet = {
        "packetVersion": 1,
        "adapterId": "cascade-ribbon-document-v12",
        "boundedSemanticReplay": True,
        "nativeExecution": False,
        "runtimeCarrier": "EFFECT_TYPED_CASCADE_RIBBON_V1",
        "typeDataStableId": _require_string(typed.get("stableId"), "Artist ribbon TypeData stableId"),
        "typeDataClassName": _require_string(typed.get("className"), "Artist ribbon TypeData className").lower(),
        "typeDataObjectPath": _require_string(typed.get("objectPath"), "Artist ribbon TypeData objectPath"),
        "typeDataModuleSha256": canonical_json_sha256(typed),
        "resolvedRendererShape": "ribbon",
        "tilingDistance": _literal_number(typed, "tilingdistance") * 0.01,
        "distanceTessellationStepSize": _literal_number(typed, "distancetessellationstepsize") * 0.01,
        "tangentTessellationScalar": _literal_number(typed, "tangenttessellationscalar", 0.0),
        "lodValidity": _literal_number(typed, "lodvalidity"),
        "operationalMaxPoints": adapter.get("operationalMaxPoints"),
        "targetTiming": target_timing,
        "attachment": target_contract["attachment"],
        "trail": target_contract["trail"],
        "sourceRecipeSha256": canonical_json_sha256(source_recipe),
        "moduleClosureSha256": canonical_json_sha256(source_recipe["modules"]),
        "moduleCount": len(source_recipe["modules"]),
        "preservedLimitations": [
            "CASCADE_RIBBON_BOUNDED_RECONSTRUCTION_NOT_NATIVE_SOURCE_EXACT"
        ],
    }
    _seal_row(packet, "packetSha256")
    selector = {
        "effectAssetId": "effect.artist.skill.31470",
        "occurrenceId": "source-active-003",
    }
    row = {
        "selector": selector,
        "selectorSha256": canonical_json_sha256(selector),
        "provenance": {
            "scope": "ARTIST_F_CASCADE_RIBBON",
            "characterClass": "ARTIST",
            "skillId": 31470,
            "stageIndex": 0,
            "sourceStableId": "source-active-003",
        },
        "schedule": {
            "stageId": "artist.31470.active",
            "sourceEventId": emitter.get("sourceOccurrenceId"),
            "sourceTimelineSeconds": 0.0,
            "localTimeSeconds": 0.0,
            "durationSeconds": emitter.get("timing", {}).get("emitterDurationSeconds"),
        },
        "sourcePayload": source_payload,
        "targetPayload": target_payload,
        "family": "CASCADE_RIBBON",
        "adapterId": "cascade-ribbon-document-v12",
        "packetLayout": "CASCADE_RIBBON_TYPED_PACKET_V1",
        "fidelity": "BOUNDED_RECONSTRUCTION",
        "disposition": "ADMITTED_BOUNDED",
        "tuningEligibleTransform": True,
        "resourcePacket": [],
        "cascadeRibbonPacket": packet,
        "animationTrailPacket": None,
        "admissionBlockers": [],
    }
    _seal_row(row, "rowSha256")
    return row


def _build_supplemental_elements(
    repository_root: Path,
    input_registry: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    result = _build_animation_trail_supplemental_elements(
        repository_root, input_registry
    )
    result.append(
        _build_artist_cascade_ribbon_supplemental_element(
            repository_root, input_registry
        )
    )
    result.sort(key=lambda row: (row["selector"]["effectAssetId"], row["selector"]["occurrenceId"]))
    _require(len(result) == EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT, "supplemental element denominator changed")
    return result


def _build_local_decal_rows(
    repository_root: Path,
    input_registry: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    local_inputs = [
        (NATIVE_SOURCE_RELATIVE_PATH, "ARTIST_F_NATIVE_SOURCE_CANDIDATE"),
        (RUNTIME_PROGRAM_RELATIVE_PATH, "ARTIST_F_TYPED_RUNTIME_PROGRAM_CANDIDATE"),
        (LOCAL_DECAL_RECEIPT_RELATIVE_PATH, "ARTIST_F_LOCAL_DECAL_ACQUISITION_RECEIPT"),
        (ACTIVE_INVENTORY_RELATIVE_PATH, "ARTIST_F_ACTIVE_EFFECT_INVENTORY"),
    ]
    for path, role in local_inputs:
        _merge_input_artifact(input_registry, _input_artifact(repository_root, path, [role]))
    native = load_json(repository_root / NATIVE_SOURCE_RELATIVE_PATH)
    program = load_json(repository_root / RUNTIME_PROGRAM_RELATIVE_PATH)
    receipt = load_json(repository_root / LOCAL_DECAL_RECEIPT_RELATIVE_PATH)
    inventory = load_json(repository_root / ACTIVE_INVENTORY_RELATIVE_PATH)
    _validate_sealed_document(program, "programSha256", "Artist F runtime program")
    resource_roles = _local_decal_resource_roles(repository_root, receipt, inventory)
    native_by_id = {
        item.get("id"): item
        for item in _require_list(native.get("elements"), "native source elements")
        if isinstance(item, dict)
    }
    emitters = {
        item.get("evidenceId"): item
        for item in _require_list(program.get("emitters"), "runtime program emitters")
        if isinstance(item, dict)
    }
    schedules = {
        item.get("scheduleId"): item
        for item in _require_list(program.get("actionSchedules"), "action schedules")
        if isinstance(item, dict)
    }
    rows = []
    for source_order, occurrence_id in ((20, "source-active-020"), (21, "source-active-021")):
        emitter = _require_dict(emitters.get(occurrence_id), f"Artist F LocalDecal emitter {occurrence_id}")
        _verify_seal(emitter, "rowSha256", f"Artist F emitter {occurrence_id}")
        _require(emitter.get("order") == source_order, f"Artist F LocalDecal order changed: {occurrence_id}")
        _require(emitter.get("rendererType") == "DecalParticle", f"Artist F LocalDecal renderer changed: {occurrence_id}")
        source_element_id = _require_string(emitter.get("sourceElementId"), "LocalDecal sourceElementId")
        source_element = _require_dict(native_by_id.get(source_element_id), f"native source element {source_element_id}")
        _require(source_element.get("kind") == "decal", f"native LocalDecal source kind changed: {source_element_id}")
        source_payload, _ = _payload_ref(repository_root, NATIVE_SOURCE_RELATIVE_PATH, source_element_id)
        target_payload, _ = _payload_ref(repository_root, RUNTIME_PROGRAM_RELATIVE_PATH, occurrence_id, emitter["rowSha256"])
        source_recipe_raw = _require_dict(source_element.get("sourceRecipe"), "LocalDecal sourceRecipe")
        local_resolution = {
            "resolutionKind": "DECLARED_RENDERER_FAMILY_FALLBACK_V1",
            "declaredRendererFamily": "DECAL_PARTICLE",
            "declaredRendererShape": _require_string(source_recipe_raw.get("rendererShape"), "LocalDecal rendererShape"),
            "resolvedFamily": "DECAL_PARTICLE",
            "resolvedRendererShape": "decal",
            "typeDataStableId": "",
            "typeDataClassName": "",
            "typeDataObjectPath": "",
            "typeDataModuleSha256": None,
        }
        _seal_row(local_resolution, "resolutionSha256")
        source_recipe = {
            "enabled": source_recipe_raw.get("enabled") is True,
            "rendererShape": _require_string(source_recipe_raw.get("rendererShape"), "LocalDecal rendererShape"),
            "resolvedRendererShape": "decal",
            "sourceRowSha256": emitter["rowSha256"],
            "sourceRecipeEvidenceSha256": canonical_json_sha256(source_recipe_raw),
            "recipeSha256": canonical_json_sha256(source_recipe_raw),
            "moduleClosureSha256": _require_sha(source_recipe_raw.get("sourceClosureSha256"), "LocalDecal sourceClosureSha256"),
            "moduleCount": len(_require_list(source_recipe_raw.get("modules"), "LocalDecal modules")),
            "typedFamilyResolution": local_resolution,
        }
        attachment = _require_dict(emitter.get("actionCueAttachment"), "LocalDecal actionCueAttachment")
        attachment_sha = canonical_json_sha256(attachment)
        schedule = _require_dict(schedules.get(emitter.get("scheduleId")), "LocalDecal action schedule")
        _verify_seal(schedule, "rowSha256", "LocalDecal action schedule")
        decal_adapter = _require_dict(emitter.get("decalAdapter"), "LocalDecal typed descriptor")
        _require(decal_adapter.get("executionAdmission") is True, "LocalDecal typed descriptor is not admitted")
        selector = {
            "effectAssetId": "effect.artist.skill.31470",
            "occurrenceId": occurrence_id,
        }
        row = {
            "selector": selector,
            "selectorSha256": canonical_json_sha256(selector),
            "provenance": {
                "scope": "ARTIST_F_LOCAL_DECAL",
                "characterClass": "ARTIST",
                "skillId": 31470,
                "stageIndex": 0,
                "sourceOrder": source_order,
                "sourceElementId": source_element_id,
                "sourceRowId": occurrence_id,
            },
            "schedule": {
                "stageId": None,
                "presentationOccurrenceId": None,
                "joinKind": "ARTIST_F_ACTION_SCHEDULE",
                "sourceEventId": None,
                "sourceTimelineSeconds": schedule.get("globalTimeSeconds"),
                "externalScheduleGroupId": emitter.get("scheduleId"),
            },
            "sourcePayload": source_payload,
            "targetPayload": target_payload,
            "sourceRecipe": source_recipe,
            "sourceImportBasis": {
                "basisId": "effect.import-basis.artist.local-decal-rt0-bounded-v1",
                "status": "SOURCE_TYPED_BOUNDED",
                "attachmentSha256": attachment_sha,
            },
            "executionProjection": {
                "family": "DECAL_PARTICLE",
                "adapterId": "local-decal-rt0-bounded-v1",
                "packetLayout": "LOCAL_DECAL_RT0_SIX_SRV_V1",
                "tuningEligibleTransform": True,
                "fidelity": "BOUNDED_RECONSTRUCTION",
                "disposition": "ADMITTED_BOUNDED",
                "nativeExecution": False,
                "sourceExact": False,
                "executionBasis": {
                    "status": "SOURCE_TYPED_BOUNDED",
                    "basisSha256": canonical_json_sha256(
                        {
                            "typedDescriptorSha256": _require_sha(decal_adapter.get("adapterSha256"), "LocalDecal adapterSha256"),
                            "attachmentSha256": attachment_sha,
                            "rendererProjectionSha256": _require_sha(emitter.get("rendererRuntimeConfig", {}).get("sourceProjectionSha256"), "LocalDecal renderer projection SHA"),
                            "resourcePacket": [
                                {
                                    "role": item["role"],
                                    "register": item["shaderRegister"],
                                    "assetId": item["assetId"],
                                    "rawSha256": item["rawSha256"],
                                }
                                for item in resource_roles
                            ],
                        }
                    ),
                },
                "resourceRoles": copy.deepcopy(resource_roles),
                "admissionBlockers": [],
                "preservedLimitations": [
                    "NATIVE_MRT_NOT_ADMITTED",
                    "NATIVE_VF_PASS_NOT_ADMITTED",
                    "RT0_SIX_SRV_BOUNDED_REPLAY_NOT_SOURCE_EXACT",
                ],
                "productMutation": False,
            },
        }
        _seal_row(row, "rowSha256")
        rows.append(row)
    return rows


def validate_selector_for_adapter(
    selector: dict[str, Any],
    family: str,
    adapter_id: str,
    packet_layout: str,
    adapter_contracts: list[dict[str, Any]],
) -> None:
    _expect_keys(selector, ["effectAssetId", "occurrenceId"], "selector")
    _require_stable_id(selector.get("effectAssetId"), "selector.effectAssetId")
    _require_stable_id(selector.get("occurrenceId"), "selector.occurrenceId")
    contracts = {item.get("adapterId"): item for item in adapter_contracts}
    contract = contracts.get(adapter_id)
    _require(contract is not None, f"unknown visual family adapter: {adapter_id}")
    _require(contract.get("family") == family, f"adapter/family mismatch: {adapter_id}/{family}")
    _require(packet_layout in contract.get("packetLayouts", []), f"adapter packet layout mismatch: {adapter_id}/{packet_layout}")


def _build_extension_canaries(
    repository_root: Path,
    input_registry: dict[str, dict[str, Any]],
    adapter_contracts: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    specs = (
        (
            "extension-canary.warlord.generic-selector.v1",
            "WARLORD",
            "effect.warlord.skill.17000.ba1",
            "extension-canary-warlord-001",
            WARLORD_CANARY_RELATIVE_PATH,
            "MESH_PARTICLE",
            "mesh-particle-document-v12",
        ),
        (
            "extension-canary.valtan.generic-selector.v1",
            "VALTAN",
            "effect.valtan.schema-canary",
            "extension-canary-valtan-001",
            VALTAN_CANARY_RELATIVE_PATH,
            "SPRITE_PARTICLE",
            "sprite-particle-document-v12",
        ),
    )
    result = []
    for canary_id, domain, effect_asset_id, occurrence_id, path, family, adapter_id in specs:
        source_artifact = _input_artifact(repository_root, path, ["EXTENSION_CANARY_SOURCE"])
        _merge_input_artifact(input_registry, source_artifact)
        selector = {"effectAssetId": effect_asset_id, "occurrenceId": occurrence_id}
        validate_selector_for_adapter(selector, family, adapter_id, "NONE", adapter_contracts)
        canary = {
            "canaryId": canary_id,
            "domain": domain,
            "selector": selector,
            "selectorSha256": canonical_json_sha256(selector),
            "sourceArtifact": source_artifact,
            "family": family,
            "adapterId": adapter_id,
            "packetLayout": "NONE",
            "fidelity": "EVIDENCE_ONLY",
            "disposition": "FAIL_CLOSED",
            "productCountContribution": False,
            "admissionBlockers": ["EXECUTABLE_VISUAL_ROWS_NOT_ADMITTED_IN_PHASE1_SCOPE"],
        }
        _seal_row(canary, "canarySha256")
        result.append(canary)
    return result


def build_corpus(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    input_registry: dict[str, dict[str, Any]] = {}
    schema_artifact = _input_artifact(repository_root, SCHEMA_RELATIVE_PATH, ["VISUAL_PROGRAM_CORPUS_SCHEMA"])
    _merge_input_artifact(input_registry, schema_artifact)
    admission, admission_artifact = _load_admission(repository_root)
    _merge_input_artifact(input_registry, admission_artifact)
    adapter_contracts = _adapter_contracts()
    schedules = _build_schedules(admission)
    rows = _build_ba_rows(repository_root, admission, input_registry)
    rows.extend(_build_local_decal_rows(repository_root, input_registry))
    rows.sort(key=lambda row: (row["selector"]["effectAssetId"], row["selector"]["occurrenceId"]))
    supplemental_elements = _build_supplemental_elements(
        repository_root, input_registry
    )
    canaries = _build_extension_canaries(repository_root, input_registry, adapter_contracts)
    corpus = {
        "schema": CORPUS_SCHEMA,
        "formatVersion": CORPUS_VERSION,
        "corpusId": CORPUS_ID,
        "contractRole": CONTRACT_ROLE,
        "selectorContract": {
            "exactFields": ["effectAssetId", "occurrenceId"],
            "prohibitedFields": ["characterClass", "skillId", "sourceOrder"],
        },
        "adapterContracts": adapter_contracts,
        "inputArtifacts": sorted(input_registry.values(), key=lambda item: item["path"]),
        "extensionCanaries": sorted(canaries, key=lambda item: item["canaryId"]),
        "presentationSchedules": schedules,
        "visualRows": rows,
        "supplementalElements": supplemental_elements,
        "denominators": {
            "stageCount": EXPECTED_STAGE_COUNT,
            "presentationScheduleCount": EXPECTED_SCHEDULE_COUNT,
            "visualRowCount": EXPECTED_VISUAL_ROW_COUNT,
            "combatBaVisualRowCount": EXPECTED_BA_ROW_COUNT,
            "artistFLocalDecalVisualRowCount": EXPECTED_LOCAL_DECAL_ROW_COUNT,
            "cascadeRibbonVisualRowCount": EXPECTED_CASCADE_RIBBON_VISUAL_ROW_COUNT,
            "supplementalElementCount": EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT,
            "artistFCascadeRibbonElementCount": EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT,
            "animationTrailElementCount": EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT,
            "admittedBoundedCount": EXPECTED_ADMITTED_COUNT,
            "failClosedCount": EXPECTED_FAIL_CLOSED_COUNT,
            "legacyApproximationCount": EXPECTED_LEGACY_COUNT,
            "boundedReconstructionCount": (
                EXPECTED_LOCAL_DECAL_ROW_COUNT +
                EXPECTED_CASCADE_RIBBON_VISUAL_ROW_COUNT
            ),
            "productMutationCount": 0,
        },
        "transactionPolicy": {
            "loadOrder": ["parse", "validate", "stage", "commit"],
            "commitMode": "ATOMIC_REPLACE_AFTER_FULL_CORPUS_VALIDATION",
            "failureAction": "PRESERVE_PREVIOUS_ARTIFACT_AND_PRODUCT_RUNTIME",
            "productMutation": False,
            "runtimeCatalogMutation": False,
        },
    }
    _seal_row(corpus, "artifactSha256")
    validate_corpus(corpus, repository_root)
    return corpus


def _validate_payload_ref(payload: dict[str, Any], repository_root: Path, label: str) -> dict[str, Any]:
    _expect_keys(payload, ["path", "rawSha256", "recordId", "recordSha256"], label)
    path = _relative_path(payload.get("path"), f"{label}.path")
    _require(raw_sha256(repository_root / path) == _require_sha(payload.get("rawSha256"), f"{label}.rawSha256"), f"{label} raw hash is stale")
    document = load_json(repository_root / path)
    record = _find_record(document, path, _require_string(payload.get("recordId"), f"{label}.recordId"))
    _require(_record_sha(record) == _require_sha(payload.get("recordSha256"), f"{label}.recordSha256"), f"{label} record hash is stale")
    return record


def _validate_resource(resource: dict[str, Any], repository_root: Path, admitted: bool, label: str) -> None:
    _expect_keys(
        resource,
        ["role", "slotId", "assetId", "resolutionStatus", "rawSha256", "byteCount", "shaderRegister", "sourceChannel"],
        label,
    )
    _require(resource.get("role") in ALL_RESOURCE_ROLES, f"{label} has unknown role")
    asset_id = _asset_id(resource.get("assetId"), f"{label}.assetId")
    status = resource.get("resolutionStatus")
    _require(status in {"TARGET_DOCUMENT_REFERENCE", "SOURCE_EVIDENCE_ONLY", "EXACT_RUNTIME_PAYLOAD_SHA256"}, f"{label} has unknown resolution status")
    if admitted:
        _require(status != "SOURCE_EVIDENCE_ONLY", f"{label} is unresolved for an admitted row")
        _require(resource.get("rawSha256") is not None and resource.get("byteCount") is not None, f"{label} admitted payload identity is incomplete")
    if resource.get("rawSha256") is not None:
        expected_sha = _require_sha(resource.get("rawSha256"), f"{label}.rawSha256")
        path = repository_root / "Client/Bin/Resources" / PurePosixPath(asset_id)
        _require(path.is_file(), f"{label} resource payload is missing")
        _require(raw_sha256(path) == expected_sha, f"{label} resource hash is stale")
        _require(path.stat().st_size == resource.get("byteCount"), f"{label} resource size is stale")


def validate_corpus(corpus: dict[str, Any], repository_root: Path = REPOSITORY_ROOT) -> None:
    repository_root = repository_root.resolve()
    _expect_keys(
        corpus,
        [
            "schema", "formatVersion", "corpusId", "contractRole", "selectorContract",
            "adapterContracts", "inputArtifacts", "extensionCanaries", "presentationSchedules",
            "visualRows", "supplementalElements", "denominators",
            "transactionPolicy", "artifactSha256",
        ],
        "corpus",
    )
    _require(corpus.get("schema") == CORPUS_SCHEMA and corpus.get("formatVersion") == CORPUS_VERSION, "corpus header mismatch")
    _require(corpus.get("corpusId") == CORPUS_ID and corpus.get("contractRole") == CONTRACT_ROLE, "corpus identity/role mismatch")
    _verify_seal(corpus, "artifactSha256", "corpus")
    selector_contract = _require_dict(corpus.get("selectorContract"), "selectorContract")
    _expect_keys(selector_contract, ["exactFields", "prohibitedFields"], "selectorContract")
    _require(selector_contract.get("exactFields") == ["effectAssetId", "occurrenceId"], "selector field contract changed")
    _require(selector_contract.get("prohibitedFields") == ["characterClass", "skillId", "sourceOrder"], "selector prohibition contract changed")

    adapters = _require_list(corpus.get("adapterContracts"), "adapterContracts")
    adapter_ids = [item.get("adapterId") for item in adapters if isinstance(item, dict)]
    _require(len(adapters) == 8 and len(set(adapter_ids)) == 8, "adapter allowlist is incomplete or duplicated")
    _require(adapters == sorted(adapters, key=lambda item: item["adapterId"]), "adapter allowlist is not deterministic")
    for index, value in enumerate(adapters):
        adapter = _require_dict(value, f"adapterContracts[{index}]")
        _expect_keys(adapter, ["adapterId", "family", "packetLayouts", "allowedResourceRoles"], f"adapterContracts[{index}]")
        _require_stable_id(adapter.get("adapterId"), "adapterId")
        _require(adapter.get("family") in ADAPTER_BY_FAMILY, "adapter has unknown family")
        _require("NONE" in adapter.get("packetLayouts", []), "adapter cannot express fail-closed rows")
        _require(set(adapter.get("allowedResourceRoles", [])) == set(ALL_RESOURCE_ROLES), "adapter resource role allowlist changed")

    inputs = _require_list(corpus.get("inputArtifacts"), "inputArtifacts")
    _require(inputs == sorted(inputs, key=lambda item: item["path"]), "input artifacts are not deterministic")
    input_paths: set[str] = set()
    for index, value in enumerate(inputs):
        item = _require_dict(value, f"inputArtifacts[{index}]")
        _expect_keys(item, ["path", "rawSha256", "roles"], f"inputArtifacts[{index}]")
        path = _relative_path(item.get("path"), f"inputArtifacts[{index}].path")
        _require(path not in input_paths, f"duplicate input artifact: {path}")
        input_paths.add(path)
        _require(raw_sha256(repository_root / path) == _require_sha(item.get("rawSha256"), f"inputArtifacts[{index}].rawSha256"), f"input artifact hash is stale: {path}")
        _require(bool(item.get("roles")) and len(item["roles"]) == len(set(item["roles"])), f"input artifact roles invalid: {path}")

    pinned_admission = load_json(repository_root / ADMISSION_RELATIVE_PATH)
    pinned_ba_source_row_hashes = {
        item.get("sourceRowId"): canonical_json_sha256(item)
        for item in _require_list(pinned_admission.get("sourceRows"), "pinned admission sourceRows")
        if isinstance(item, dict)
    }
    pinned_program = load_json(repository_root / RUNTIME_PROGRAM_RELATIVE_PATH)
    pinned_local_source_row_hashes = {
        item.get("evidenceId"): item.get("rowSha256")
        for item in _require_list(pinned_program.get("emitters"), "pinned Artist F emitters")
        if isinstance(item, dict)
    }

    canaries = _require_list(corpus.get("extensionCanaries"), "extensionCanaries")
    _require(len(canaries) == 2 and {item.get("domain") for item in canaries} == {"WARLORD", "VALTAN"}, "Warlord/Valtan extension canaries are incomplete")
    for index, value in enumerate(canaries):
        canary = _require_dict(value, f"extensionCanaries[{index}]")
        _verify_seal(canary, "canarySha256", f"extensionCanaries[{index}]")
        _require(canonical_json_sha256(canary.get("selector")) == canary.get("selectorSha256"), "extension canary selector hash is stale")
        validate_selector_for_adapter(canary.get("selector"), canary.get("family"), canary.get("adapterId"), canary.get("packetLayout"), adapters)
        _require(canary.get("fidelity") == "EVIDENCE_ONLY" and canary.get("disposition") == "FAIL_CLOSED", "extension canary was falsely admitted")
        _require(canary.get("productCountContribution") is False and bool(canary.get("admissionBlockers")), "extension canary product boundary changed")
        source = _require_dict(canary.get("sourceArtifact"), "extension canary source")
        _require(source.get("path") in input_paths, "extension canary source is not pinned as an input")
        _require(raw_sha256(repository_root / source["path"]) == source.get("rawSha256"), "extension canary source hash is stale")

    schedules = _require_list(corpus.get("presentationSchedules"), "presentationSchedules")
    _require(len(schedules) == EXPECTED_SCHEDULE_COUNT, "presentation schedule denominator changed")
    _require(schedules == sorted(schedules, key=lambda item: item["presentationOccurrenceId"]), "presentation schedules are not deterministic")
    schedule_ids: set[str] = set()
    schedule_source_rows: set[str] = set()
    for index, value in enumerate(schedules):
        schedule = _require_dict(value, f"presentationSchedules[{index}]")
        _verify_seal(schedule, "scheduleSha256", f"presentationSchedules[{index}]")
        schedule_id = _require_stable_id(schedule.get("presentationOccurrenceId"), "presentationOccurrenceId")
        _require(schedule_id not in schedule_ids, f"duplicate presentation schedule: {schedule_id}")
        schedule_ids.add(schedule_id)
        row_ids = schedule.get("sourceRowIds", [])
        _require(bool(row_ids) and not schedule_source_rows.intersection(row_ids), f"presentation schedule source-row overlap: {schedule_id}")
        schedule_source_rows.update(row_ids)

    supplemental = _require_list(
        corpus.get("supplementalElements"), "supplementalElements"
    )
    _require(
        len(supplemental) == EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT,
        "supplemental element denominator changed",
    )
    _require(
        supplemental == sorted(
            supplemental,
            key=lambda item: (
                item["selector"]["effectAssetId"],
                item["selector"]["occurrenceId"],
            ),
        ),
        "supplemental elements are not deterministic",
    )
    supplemental_selectors: set[tuple[str, str]] = set()
    supplemental_counts = Counter()
    for index, value in enumerate(supplemental):
        item = _require_dict(value, f"supplementalElements[{index}]")
        _expect_keys(
            item,
            [
                "selector", "selectorSha256", "provenance", "schedule",
                "sourcePayload", "targetPayload", "family", "adapterId",
                "packetLayout", "fidelity", "disposition",
                "tuningEligibleTransform", "resourcePacket",
                "cascadeRibbonPacket", "animationTrailPacket",
                "admissionBlockers", "rowSha256",
            ],
            f"supplementalElements[{index}]",
        )
        _verify_seal(item, "rowSha256", f"supplementalElements[{index}]")
        selector = _require_dict(item.get("selector"), "supplemental selector")
        validate_selector_for_adapter(
            selector,
            item.get("family"),
            item.get("adapterId"),
            item.get("packetLayout"),
            adapters,
        )
        _require(
            canonical_json_sha256(selector) == item.get("selectorSha256"),
            "supplemental selector SHA is stale",
        )
        selector_key = (selector["effectAssetId"], selector["occurrenceId"])
        _require(
            selector_key not in supplemental_selectors,
            f"duplicate supplemental selector: {selector_key}",
        )
        supplemental_selectors.add(selector_key)
        source_record = _validate_payload_ref(
            _require_dict(item.get("sourcePayload"), "supplemental sourcePayload"),
            repository_root,
            "supplemental sourcePayload",
        )
        target_record = _validate_payload_ref(
            _require_dict(item.get("targetPayload"), "supplemental targetPayload"),
            repository_root,
            "supplemental targetPayload",
        )
        _require(
            target_record.get("kind") == "trail" and
            item.get("disposition") == "ADMITTED_BOUNDED" and
            item.get("fidelity") == "BOUNDED_RECONSTRUCTION" and
            item.get("tuningEligibleTransform") is True and
            item.get("admissionBlockers") == [],
            "supplemental element admission/target is invalid",
        )
        family = item.get("family")
        if family == "ANIMATION_TRAIL":
            _require(item.get("cascadeRibbonPacket") is None, "AnimationTrail contains a Cascade packet")
            packet = _require_dict(item.get("animationTrailPacket"), "AnimationTrail packet")
            _verify_seal(packet, "packetSha256", "AnimationTrail packet")
            _require(
                source_record.get("eventId") == packet.get("sourceEventId") and
                source_record.get("sourceType") == "Trails" and
                source_record.get("sourceAsset") == packet.get("sourceAsset") and
                canonical_json_sha256(source_record) == packet.get("sourceEventRecordSha256") and
                target_record.get("id") == packet.get("targetElementId"),
                "AnimationTrail source notify/target packet is stale",
            )
        else:
            _require(family == "CASCADE_RIBBON", "unknown supplemental family")
            _require(item.get("animationTrailPacket") is None, "CascadeRibbon contains AnimationTrail packet")
            packet = _require_dict(item.get("cascadeRibbonPacket"), "CascadeRibbon packet")
            _verify_seal(packet, "packetSha256", "CascadeRibbon packet")
            target_recipe = _require_dict(target_record.get("sourceRecipe"), "CascadeRibbon target SourceRecipe")
            _require(
                target_recipe.get("enabled") is True and
                target_recipe.get("rendererShape") == "ribbon" and
                canonical_json_sha256(target_recipe) == packet.get("sourceRecipeSha256") and
                canonical_json_sha256(target_recipe.get("modules", [])) == packet.get("moduleClosureSha256"),
                "CascadeRibbon target recipe packet is stale",
            )
        for resource_index, resource_value in enumerate(item.get("resourcePacket", [])):
            _validate_resource(
                _require_dict(resource_value, "supplemental resource"),
                repository_root,
                True,
                f"supplementalElements[{index}].resourcePacket[{resource_index}]",
            )
        supplemental_counts[family] += 1
    _require(
        supplemental_counts["ANIMATION_TRAIL"] == EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT and
        supplemental_counts["CASCADE_RIBBON"] == EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT,
        "supplemental family denominator changed",
    )

    rows = _require_list(corpus.get("visualRows"), "visualRows")
    _require(len(rows) == EXPECTED_VISUAL_ROW_COUNT, "visual row denominator changed")
    _require(rows == sorted(rows, key=lambda item: (item["selector"]["effectAssetId"], item["selector"]["occurrenceId"])), "visual rows are not deterministic")
    selectors: set[tuple[str, str]] = set()
    counts = Counter()
    ba_stage_ids: set[str] = set()
    ba_source_row_ids: set[str] = set()
    ba_particle_source_row_ids: set[str] = set()
    for index, value in enumerate(rows):
        row = _require_dict(value, f"visualRows[{index}]")
        _expect_keys(
            row,
            [
                "selector", "selectorSha256", "provenance", "schedule",
                "sourcePayload", "targetPayload", "sourceRecipe",
                "sourceImportBasis", "executionProjection", "rowSha256",
            ],
            f"visualRows[{index}]",
        )
        _verify_seal(row, "rowSha256", f"visualRows[{index}]")
        selector = _require_dict(row.get("selector"), f"visualRows[{index}].selector")
        projection = _require_dict(row.get("executionProjection"), f"visualRows[{index}].executionProjection")
        validate_selector_for_adapter(selector, projection.get("family"), projection.get("adapterId"), projection.get("packetLayout"), adapters)
        _require(canonical_json_sha256(selector) == row.get("selectorSha256"), f"visualRows[{index}] selector hash is stale")
        selector_key = (selector["effectAssetId"], selector["occurrenceId"])
        _require(selector_key not in selectors, f"duplicate visual selector: {selector_key}")
        selectors.add(selector_key)
        provenance = _require_dict(row.get("provenance"), "row provenance")
        _expect_keys(
            provenance,
            [
                "scope", "characterClass", "skillId", "stageIndex",
                "sourceOrder", "sourceElementId", "sourceRowId",
            ],
            "row provenance",
        )
        row_schedule = _require_dict(row.get("schedule"), "row schedule")
        _expect_keys(
            row_schedule,
            [
                "stageId", "presentationOccurrenceId", "joinKind",
                "sourceEventId", "sourceTimelineSeconds",
                "externalScheduleGroupId",
            ],
            "row schedule",
        )
        source_recipe = _require_dict(row.get("sourceRecipe"), "row sourceRecipe")
        _expect_keys(
            source_recipe,
            [
                "enabled", "rendererShape", "resolvedRendererShape",
                "sourceRowSha256", "sourceRecipeEvidenceSha256",
                "recipeSha256", "moduleClosureSha256", "moduleCount",
                "typedFamilyResolution",
            ],
            "row sourceRecipe",
        )
        _require_sha(source_recipe.get("sourceRowSha256"), "sourceRecipe.sourceRowSha256")
        _require_sha(source_recipe.get("recipeSha256"), "sourceRecipe.recipeSha256")
        _require_sha(source_recipe.get("sourceRecipeEvidenceSha256"), "sourceRecipe.sourceRecipeEvidenceSha256")
        _require_sha(source_recipe.get("moduleClosureSha256"), "sourceRecipe.moduleClosureSha256")
        resolution = _require_dict(
            source_recipe.get("typedFamilyResolution"), "typedFamilyResolution"
        )
        _expect_keys(
            resolution,
            [
                "resolutionKind", "declaredRendererFamily",
                "declaredRendererShape", "resolvedFamily",
                "resolvedRendererShape", "typeDataStableId",
                "typeDataClassName", "typeDataObjectPath",
                "typeDataModuleSha256", "resolutionSha256",
            ],
            "typedFamilyResolution",
        )
        _verify_seal(resolution, "resolutionSha256", "typedFamilyResolution")
        _require(
            resolution.get("resolvedFamily") == projection.get("family") and
            resolution.get("resolvedRendererShape") == source_recipe.get("resolvedRendererShape"),
            "typed family resolution diverged from execution projection",
        )
        source_basis = _require_dict(row.get("sourceImportBasis"), "row sourceImportBasis")
        _expect_keys(
            source_basis,
            ["basisId", "status", "attachmentSha256"],
            "row sourceImportBasis",
        )
        _require_stable_id(source_basis.get("basisId"), "sourceImportBasis.basisId")
        _require_sha(source_basis.get("attachmentSha256"), "sourceImportBasis.attachmentSha256")
        _expect_keys(
            projection,
            [
                "family", "adapterId", "packetLayout",
                "tuningEligibleTransform", "fidelity", "disposition",
                "nativeExecution", "sourceExact", "executionBasis",
                "resourceRoles", "admissionBlockers",
                "preservedLimitations", "productMutation",
            ],
            "row executionProjection",
        )
        execution_basis = _require_dict(projection.get("executionBasis"), "executionBasis")
        _expect_keys(execution_basis, ["status", "basisSha256"], "executionBasis")
        _require_sha(execution_basis.get("basisSha256"), "executionBasis.basisSha256")
        if provenance.get("scope") == "COMBAT_BA":
            _require(selector["occurrenceId"] == _ba_occurrence_id(selector["effectAssetId"], provenance.get("sourceElementId")), "BA occurrence identity leaked class/skill/order or became unstable")
            ba_stage_ids.add(row.get("schedule", {}).get("stageId"))
            source_row_id = provenance.get("sourceRowId")
            _require(source_row_id not in ba_source_row_ids, "duplicate BA source row reference")
            ba_source_row_ids.add(source_row_id)
            _require(
                source_recipe.get("sourceRowSha256") ==
                    pinned_ba_source_row_hashes.get(source_row_id),
                "BA sourceRecipe source-row reference is stale",
            )
            if row_schedule.get("presentationOccurrenceId") is not None:
                _require(
                    row_schedule.get("presentationOccurrenceId") in schedule_ids,
                    "BA row references an unknown presentation schedule",
                )
                ba_particle_source_row_ids.add(provenance.get("sourceRowId"))
        else:
            _require(provenance.get("scope") == "ARTIST_F_LOCAL_DECAL", "unknown visual row scope")
            _require(selector["effectAssetId"] == "effect.artist.skill.31470" and selector["occurrenceId"] in {"source-active-020", "source-active-021"}, "Artist F LocalDecal selector changed")
            _require(
                source_recipe.get("sourceRowSha256") ==
                    pinned_local_source_row_hashes.get(selector["occurrenceId"]),
                "Artist F LocalDecal source-row reference is stale",
            )
        source_record = _validate_payload_ref(
            _require_dict(row.get("sourcePayload"), "sourcePayload"),
            repository_root,
            "sourcePayload",
        )
        source_record_recipe = _require_dict(
            source_record.get("sourceRecipe"), "source payload SourceRecipe"
        )
        _require(
            canonical_json_sha256(source_record_recipe) ==
                source_recipe.get("sourceRecipeEvidenceSha256"),
            "source payload recipe evidence is stale",
        )
        if provenance.get("scope") == "COMBAT_BA":
            _require(
                canonical_json_sha256(source_record_recipe.get("modules", [])) ==
                    source_recipe.get("moduleClosureSha256"),
                "source payload module evidence is stale",
            )
        projected_recipe = copy.deepcopy(source_record_recipe)
        projected_recipe["rendererShape"] = source_recipe.get("resolvedRendererShape")
        _require(
            canonical_json_sha256(projected_recipe) == source_recipe.get("recipeSha256"),
            "resolved SourceRecipe SHA is stale",
        )
        disposition = projection.get("disposition")
        admitted = disposition == "ADMITTED_BOUNDED"
        if admitted:
            _require(row.get("targetPayload") is not None, "admitted row has no executable target payload")
            _validate_payload_ref(_require_dict(row.get("targetPayload"), "targetPayload"), repository_root, "targetPayload")
            _require(projection.get("tuningEligibleTransform") is True, "admitted row is not transform-tuning eligible")
            _require(projection.get("packetLayout") != "NONE", "admitted row has no executable packet")
            _require(projection.get("executionBasis", {}).get("status") != "UNRESOLVED_FAIL_CLOSED", "admitted row has unresolved execution basis")
            _require(projection.get("admissionBlockers") == [], "admitted row still has admission blockers")
            _require(
                bool(projection.get("resourceRoles")) or
                projection.get("family") == "CASCADE_RIBBON",
                "admitted row has no resolved resources",
            )
        else:
            _require(disposition == "FAIL_CLOSED", "unknown visual row disposition")
            _require(row.get("targetPayload") is None, "fail-closed row points at an executable target")
            _require(projection.get("packetLayout") == "NONE" and projection.get("tuningEligibleTransform") is False, "fail-closed execution packet changed")
            _require(projection.get("executionBasis", {}).get("status") == "UNRESOLVED_FAIL_CLOSED", "fail-closed row claims a resolved basis")
            _require(bool(projection.get("admissionBlockers")), "fail-closed row has no blocker")
        _require(projection.get("nativeExecution") is False and projection.get("sourceExact") is False and projection.get("productMutation") is False, "row overclaims native/source-exact/product mutation")
        for resource_index, resource_value in enumerate(projection.get("resourceRoles", [])):
            _validate_resource(_require_dict(resource_value, "resourceRole"), repository_root, admitted, f"visualRows[{index}].resourceRoles[{resource_index}]")
        counts[("disposition", disposition)] += 1
        counts[("fidelity", projection.get("fidelity"))] += 1
        counts[("scope", provenance.get("scope"))] += 1
        counts[("family", projection.get("family"))] += 1
    _require(len(ba_stage_ids) == EXPECTED_STAGE_COUNT, "BA stage closure changed")
    _require(
        ba_particle_source_row_ids == schedule_source_rows,
        "BA presentation schedule/source-row closure changed",
    )
    _require(counts[("scope", "COMBAT_BA")] == EXPECTED_BA_ROW_COUNT, "BA visual row count changed")
    _require(counts[("scope", "ARTIST_F_LOCAL_DECAL")] == EXPECTED_LOCAL_DECAL_ROW_COUNT, "LocalDecal visual row count changed")
    _require(counts[("disposition", "ADMITTED_BOUNDED")] == EXPECTED_ADMITTED_COUNT, "admitted bounded count changed")
    _require(counts[("disposition", "FAIL_CLOSED")] == EXPECTED_FAIL_CLOSED_COUNT, "fail-closed count changed")
    _require(counts[("fidelity", "LEGACY_APPROXIMATION")] == EXPECTED_LEGACY_COUNT, "legacy approximation count changed")
    _require(
        counts[("fidelity", "BOUNDED_RECONSTRUCTION")] ==
            EXPECTED_LOCAL_DECAL_ROW_COUNT + EXPECTED_CASCADE_RIBBON_VISUAL_ROW_COUNT,
        "bounded reconstruction count changed",
    )
    expected_families = {
        "MESH_PARTICLE": 41,
        "SPRITE_PARTICLE": 69,
        "CASCADE_RIBBON": 4,
        "DECAL_PARTICLE": 4,
        "LIGHT_PARTICLE": 15,
        "SCREEN_POST": 2,
    }
    _require(
        {family: counts[("family", family)] for family in expected_families} == expected_families,
        "visual row family denominator changed",
    )
    denominators = _require_dict(corpus.get("denominators"), "denominators")
    expected_denominators = {
        "stageCount": EXPECTED_STAGE_COUNT,
        "presentationScheduleCount": EXPECTED_SCHEDULE_COUNT,
        "visualRowCount": EXPECTED_VISUAL_ROW_COUNT,
        "combatBaVisualRowCount": EXPECTED_BA_ROW_COUNT,
        "artistFLocalDecalVisualRowCount": EXPECTED_LOCAL_DECAL_ROW_COUNT,
        "cascadeRibbonVisualRowCount": EXPECTED_CASCADE_RIBBON_VISUAL_ROW_COUNT,
        "supplementalElementCount": EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT,
        "artistFCascadeRibbonElementCount": EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT,
        "animationTrailElementCount": EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT,
        "admittedBoundedCount": EXPECTED_ADMITTED_COUNT,
        "failClosedCount": EXPECTED_FAIL_CLOSED_COUNT,
        "legacyApproximationCount": EXPECTED_LEGACY_COUNT,
        "boundedReconstructionCount": (
            EXPECTED_LOCAL_DECAL_ROW_COUNT +
            EXPECTED_CASCADE_RIBBON_VISUAL_ROW_COUNT
        ),
        "productMutationCount": 0,
    }
    _require(denominators == expected_denominators, "corpus denominators are stale")
    transaction = _require_dict(corpus.get("transactionPolicy"), "transactionPolicy")
    _require(transaction == {
        "loadOrder": ["parse", "validate", "stage", "commit"],
        "commitMode": "ATOMIC_REPLACE_AFTER_FULL_CORPUS_VALIDATION",
        "failureAction": "PRESERVE_PREVIOUS_ARTIFACT_AND_PRODUCT_RUNTIME",
        "productMutation": False,
        "runtimeCatalogMutation": False,
    }, "transaction/product-mutation boundary changed")


def write_corpus_transactionally(
    corpus: dict[str, Any],
    output_path: Path,
    repository_root: Path = REPOSITORY_ROOT,
) -> None:
    validate_corpus(corpus, repository_root)
    output_path = output_path.resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    payload = pretty_json_bytes(corpus)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            prefix=f".{output_path.name}.",
            suffix=".tmp",
            dir=output_path.parent,
            delete=False,
        ) as stream:
            temporary_path = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_path, output_path)
        temporary_path = None
    finally:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()


def build_and_write(
    repository_root: Path,
    output_path: Path,
    *,
    check: bool = False,
) -> dict[str, Any]:
    corpus = build_corpus(repository_root)
    expected = pretty_json_bytes(corpus)
    if check:
        _require(output_path.is_file(), f"missing generated visual-program corpus: {output_path}")
        _require(output_path.read_bytes() == expected, f"generated visual-program corpus is stale: {output_path}")
    else:
        write_corpus_transactionally(corpus, output_path, repository_root)
    return corpus


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument("--output", type=Path, default=Path(DEFAULT_OUTPUT_RELATIVE_PATH))
    parser.add_argument("--check", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    repository_root = args.repository_root.resolve()
    output_path = args.output
    if not output_path.is_absolute():
        output_path = repository_root / output_path
    try:
        corpus = build_and_write(repository_root, output_path, check=args.check)
    except ContractError as error:
        print(f"FAIL: {error}")
        return 1
    mode = "CHECK" if args.check else "WRITE"
    print(
        f"PASS: {mode} {output_path} rows={len(corpus['visualRows'])} "
        f"schedules={len(corpus['presentationSchedules'])} "
        f"sha256={corpus['artifactSha256']} productMutation=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
