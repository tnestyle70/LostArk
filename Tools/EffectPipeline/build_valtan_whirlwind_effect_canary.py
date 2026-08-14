#!/usr/bin/env python3
"""Build and verify the non-product v13 Valtan Whirlwind Effect canary.

The builder traces every selected stage-002 source occurrence to its first-LOD
Cascade emitter.  A carrier becomes visible only when its portable SourceRecipe,
source Material profile, runtime resources, and (for Mesh) cooked geometry are
all evidence-closed.  Unsupported Trail/Light and incomplete Dust carriers stay
explicitly fail-closed.  This slice never writes Catalog or AnimEvent mappings.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import struct
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
if str(SCRIPT_PATH.parent) not in sys.path:
    sys.path.insert(0, str(SCRIPT_PATH.parent))
LEVEL_PIPELINE_ROOT = REPOSITORY_ROOT / "Tools/LevelPlacementExtractor"
if str(LEVEL_PIPELINE_ROOT) not in sys.path:
    sys.path.insert(0, str(LEVEL_PIPELINE_ROOT))

from validate_boss_pattern_effects import (  # noqa: E402
    ContractError,
    load_json,
    validate_mapping,
)
import build_imported_effect_documents as imported_effects  # noqa: E402
import build_skill_effect_source_receipt as source_receipts  # noqa: E402
from build_effect_source_material_contract import (  # noqa: E402
    dynamic_parameter_semantics,
    merged_source_parameters,
    normalize_scalar_parameter,
    normalize_switch_parameter,
    normalize_texture_parameter,
    normalize_vector_parameter,
    runtime_profile_dynamic_parameter_semantics,
    stable_profile_id,
)
from extract_umodel_material_dependencies import parse_material_dump  # noqa: E402
from materialize_artist_31470_portable_particle_carriers import (  # noqa: E402
    MaterializeError,
    portable_recipe,
)


SCHEMA_RELATIVE_PATH = Path(
    "Tools/EffectPipeline/Schemas/lostark.boss-pattern-effects.schema.json"
)
MAPPING_RELATIVE_PATH = Path(
    "Data/Animation/Authored/Valtan/Valtan.patterneffects.json"
)
RUNTIME_RESOURCE_ROOT = Path("Client/Bin/Resources")

EXPECTED_EFFECT_ASSET_ID = "effect.valtan.pattern.420633.active"
EXPECTED_PATTERN_ID = "VALTAN_WHIRLWIND"
EXPECTED_STAGE_ID = "SPIN"
EXPECTED_ACTION_ID = "valtan.attack.whirlwind.active"
EXPECTED_SOURCE_ACTION_ID = 420633
EXPECTED_SOURCE_STAGE_INDEX = 2
EXPECTED_SOURCE_STAGE_PATH = [1, 2, 3]
EXPECTED_SOURCE_CLIPS = {
    1: "Att_Battle_20_02",
    2: "Att_Battle_20_03",
    3: "Att_Battle_20_04",
}
EXPECTED_RUNTIME_CLIP = "mesh_att_battle_20_03"
EXPECTED_SOURCE_OCCURRENCE_IDS = (
    "action-420633/stage-002/notify-004",
    "action-420633/stage-002/notify-005",
    "action-420633/stage-002/notify-006",
    "action-420633/stage-002/notify-009",
)
EXPECTED_FAIL_CLOSED = {
    "action-420633/stage-002/notify-001": (
        "Effect",
        "UNRESOLVED_SOURCE_PAYLOAD",
        "NOTIFY_HAS_NO_EXPLICIT_OBJECT_REFERENCE",
    ),
    "action-420633/stage-002/notify-002": (
        "Effect",
        "UNRESOLVED_SOURCE_PAYLOAD",
        "NOTIFY_HAS_NO_EXPLICIT_OBJECT_REFERENCE",
    ),
    "action-420633/stage-002/notify-003": (
        "Effect",
        "UNRESOLVED_SOURCE_PAYLOAD",
        "NOTIFY_HAS_NO_EXPLICIT_OBJECT_REFERENCE",
    ),
    "action-420633/stage-002/notify-007": (
        "PawnMaterialParam",
        "SOURCE_PARAMETERS_SERIALIZED",
        "PAWN_MATERIAL_PARAM_RUNTIME_CHANNEL_NOT_IMPLEMENTED",
    ),
    "action-420633/stage-002/notify-008": (
        "ViewShake",
        "SOURCE_PARAMETERS_SERIALIZED",
        "VIEW_SHAKE_RUNTIME_CHANNEL_NOT_IMPLEMENTED",
    ),
}
EXPECTED_GRAPH_ROOTS = {
    "fx_bs_01.trail.par_n_mrhg_trail_01": "FX_BS_01:export:5555",
    "fx_mn_rpbf_00_n.par_n_rpbf_dust_01_01": (
        "FX_MN_RPBF_00_N:export:7380"
    ),
    "fx_mn_rpbf_00_n.par_n_rpbf_wwind_01": (
        "FX_MN_RPBF_00_N:export:7422"
    ),
    "fx_cm_02.light.par_mp_light_05_l": "FX_CM_02:export:7020",
}
EXPECTED_BRANCH_CONDITION_ID = "action-420633/stage-001/notify-004"
EXPECTED_CARRIER_DENOMINATOR = 9
EXPECTED_VISIBLE_CARRIER_COUNT = 3
EXPECTED_FAIL_CLOSED_CARRIER_COUNT = 6
EXPECTED_CARRIER_BLOCKERS = {
    "valtan.420633.notify004.emitter5259": (
        "ANIMATION_TRAIL_RUNTIME_BINDING_NOT_MATERIALIZED",
        "PORTABLE_V13_UNSUPPORTED_MODULE_PARTICLEMODULETYPEDATAANIMTRAIL",
    ),
    "valtan.420633.notify004.emitter5260": (
        "ANIMATION_TRAIL_RUNTIME_BINDING_NOT_MATERIALIZED",
        "PORTABLE_V13_UNSUPPORTED_MODULE_PARTICLEMODULETYPEDATAANIMTRAIL",
    ),
    "valtan.420633.notify004.emitter5258": (
        "ANIMATION_TRAIL_RUNTIME_BINDING_NOT_MATERIALIZED",
        "PORTABLE_V13_UNSUPPORTED_MODULE_PARTICLEMODULETYPEDATAANIMTRAIL",
    ),
    "valtan.420633.notify005.emitter7034": (
        "SOURCE_DYNAMIC_PARAMETER_ABI_NOT_EVIDENCE_CLOSED",
        "SOURCE_MATERIAL_FINITE_PROFILE_NOT_EVIDENCE_CLOSED",
    ),
    "valtan.420633.notify005.emitter7035": (
        "SOURCE_MATERIAL_REQUIRED_RUNTIME_RESOURCES_NOT_COOKED_FOR_VALTAN",
    ),
    "valtan.420633.notify009.emitter6823": (
        "LIGHT_SOURCE_GRAPH_EXTERNAL_MODULES_UNRESOLVED",
        "POINT_LIGHT_COMPONENT_NOT_DECODED",
        "PORTABLE_V13_UNSUPPORTED_MODULE_EFPARTICLEMODULETYPEDATALIGHT",
        "SOURCE_ANCHOR_NOT_EXPLICIT",
    ),
}


@dataclass(frozen=True)
class BoneRow:
    index: int
    name: str
    parent_index: int
    name_hash: int


def pretty_json_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            indent=2,
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            while chunk := stream.read(1024 * 1024):
                digest.update(chunk)
    except OSError as error:
        raise ContractError(f"could not hash {path}: {error}") from error
    return digest.hexdigest()


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _relative_artifact_path(root: Path, value: str, label: str) -> Path:
    relative = PurePosixPath(value)
    if relative.is_absolute() or ".." in relative.parts or "." in relative.parts:
        raise ContractError(f"{label} escapes or ambiguously addresses its root")
    return root.joinpath(*relative.parts)


def _discover_source_artifact_root(
    catalog: dict[str, Any],
    expected_name: str,
    override: Path | None,
) -> Path:
    if override is not None:
        candidate = override.resolve()
        if candidate.name.casefold() != expected_name.casefold():
            raise ContractError(
                f"source artifact root must be named {expected_name}: {candidate}"
            )
        return candidate
    roots: set[Path] = set()
    for row in catalog.get("sourcePackageGraphs", []):
        graph_file = row.get("graphFile")
        if not isinstance(graph_file, str) or not graph_file:
            continue
        path = Path(graph_file).resolve()
        for candidate in (path.parent, *path.parents):
            if candidate.name.casefold() == expected_name.casefold():
                roots.add(candidate)
                break
    if len(roots) != 1:
        raise ContractError(
            f"could not discover one {expected_name} source artifact root"
        )
    return next(iter(roots))


def _fixed_ascii(value: bytes, label: str) -> str:
    value = value.split(b"\0", 1)[0]
    try:
        return value.decode("ascii")
    except UnicodeDecodeError as error:
        raise ContractError(f"WModel {label} is not ASCII") from error


def read_wmodel_bones(path: Path) -> list[BoneRow]:
    """Read the actual embedded WSKL table; do not accept a byte substring."""

    try:
        payload = path.read_bytes()
    except OSError as error:
        raise ContractError(f"could not read WModel {path}: {error}") from error
    if len(payload) < 48:
        raise ContractError("WModel is truncated before its section table")
    magic, major, _minor, flags, content_size = struct.unpack_from(
        "<4sHHII", payload, 0
    )
    if (
        magic != b"WINT"
        or major != 1
        or flags != 0
        or content_size != len(payload) - 16
    ):
        raise ContractError("WModel outer WINT header is invalid")
    (
        model_magic,
        section_count,
        _animation_count,
        _model_flags,
        *_reserved,
    ) = struct.unpack_from("<4sIII4I", payload, 16)
    if model_magic != b"WMOD" or not 2 <= section_count <= 4096:
        raise ContractError("WModel WMOD metadata is invalid")
    table_end = 48 + section_count * 64
    if table_end > len(payload):
        raise ContractError("WModel section table is truncated")

    skeleton_sections: list[tuple[int, int]] = []
    for index in range(section_count):
        section_type, _section_index, offset, size, _name = struct.unpack_from(
            "<IIQQ40s", payload, 48 + index * 64
        )
        if offset > content_size or size > content_size - offset:
            raise ContractError("WModel section points outside the package")
        if section_type == 3:
            skeleton_sections.append((int(offset), int(size)))
    if len(skeleton_sections) != 1:
        raise ContractError(
            "WModel must contain exactly one physical skeleton section"
        )

    section_offset, section_size = skeleton_sections[0]
    section_start = 16 + section_offset
    if section_size < 48 or section_start + section_size > len(payload):
        raise ContractError("WModel skeleton section is truncated")
    embedded_magic, embedded_major, _minor, embedded_flags, embedded_size = (
        struct.unpack_from("<4sHHII", payload, section_start)
    )
    if (
        embedded_magic != b"WINT"
        or embedded_major != 1
        or embedded_flags != 0
        or embedded_size + 16 != section_size
    ):
        raise ContractError("WModel embedded skeleton WINT header is invalid")
    skeleton_start = section_start + 16
    skeleton_magic, bone_count, socket_count, *_ = struct.unpack_from(
        "<4sII5I", payload, skeleton_start
    )
    if skeleton_magic != b"WSKL" or not 1 <= bone_count <= 512:
        raise ContractError("WModel WSKL metadata is invalid")
    if socket_count > 256:
        raise ContractError("WModel WSKL socket count is invalid")
    expected_content_size = 32 + bone_count * 256 + 128 + socket_count * 128
    if embedded_size != expected_content_size:
        raise ContractError(
            "WModel WSKL payload size does not match its bone/socket counts"
        )

    rows: list[BoneRow] = []
    names: set[str] = set()
    hashes: set[int] = set()
    bone_start = skeleton_start + 32
    for index in range(bone_count):
        name_hash, encoded_name, parent_index = struct.unpack_from(
            "<Q64si", payload, bone_start + index * 256
        )
        name = _fixed_ascii(encoded_name, f"bone[{index}] name")
        if not name or name in names or name_hash == 0 or name_hash in hashes:
            raise ContractError("WModel WSKL has an empty or duplicate bone")
        if parent_index >= index or parent_index < -1:
            raise ContractError("WModel WSKL hierarchy is not parent-before-child")
        names.add(name)
        hashes.add(name_hash)
        rows.append(BoneRow(index, name, parent_index, name_hash))
    return rows


def derive_bone_evidence(
    bones: list[BoneRow],
    requested_name: str,
    runtime_model_asset_id: str,
    model_sha256: str,
) -> dict[str, Any]:
    exact = [row for row in bones if row.name == requested_name]
    if len(exact) > 1:
        raise ContractError(f"WModel has duplicate exact bone names: {requested_name}")
    if exact:
        row = exact[0]
        policy = "EXACT_CASE_SENSITIVE"
    else:
        folded = [
            row for row in bones if row.name.casefold() == requested_name.casefold()
        ]
        if len(folded) != 1:
            raise ContractError(
                f"runtime bone {requested_name!r} has {len(folded)} unique "
                "case-fold matches; admission must remain fail-closed and root "
                "fallback is forbidden"
            )
        row = folded[0]
        policy = "UNIQUE_ASCII_CASEFOLD"
    return {
        "sourceAnchorSlotId": requested_name,
        "runtimeModelAssetId": runtime_model_asset_id,
        "runtimeBoneName": row.name,
        "boneIndex": row.index,
        "boneNameHash": f"{row.name_hash:016x}",
        "matchPolicy": policy,
        "modelSha256": model_sha256,
        "admission": "ADMITTED_EXPLICIT_RUNTIME_BONE",
    }


def _one(rows: list[dict[str, Any]], predicate: Any, label: str) -> dict[str, Any]:
    matches = [row for row in rows if predicate(row)]
    if len(matches) != 1:
        raise ContractError(f"expected exactly one {label}, found {len(matches)}")
    return matches[0]


def _validate_fixed_identity(mapping: dict[str, Any]) -> dict[str, Any]:
    if mapping["bossArchetypeId"] != "BOSS_VALTAN":
        raise ContractError("Whirlwind canary bossArchetypeId changed")
    if len(mapping["bindings"]) != 1:
        raise ContractError("Whirlwind canary requires exactly one binding")
    binding = mapping["bindings"][0]
    expected = (
        EXPECTED_PATTERN_ID,
        EXPECTED_STAGE_ID,
        EXPECTED_ACTION_ID,
        EXPECTED_EFFECT_ASSET_ID,
    )
    actual = (
        binding["patternId"],
        binding["semanticStageId"],
        binding["actionId"],
        binding["effectAssetId"],
    )
    if actual != expected:
        raise ContractError(f"Whirlwind action-qualified identity changed: {actual!r}")
    branch = binding["sourceBranch"]
    if (
        branch["sourceActionId"] != EXPECTED_SOURCE_ACTION_ID
        or branch["stagePath"] != EXPECTED_SOURCE_STAGE_PATH
        or branch["sourceStageIndex"] != EXPECTED_SOURCE_STAGE_INDEX
        or branch["sourceClipName"] != EXPECTED_SOURCE_CLIPS[2]
        or branch["runtimeClipName"] != EXPECTED_RUNTIME_CLIP
    ):
        raise ContractError("Whirlwind source branch identity changed")
    source_ids = tuple(row["notifyId"] for row in binding["sourceOccurrences"])
    if source_ids != EXPECTED_SOURCE_OCCURRENCE_IDS:
        raise ContractError(f"Whirlwind exact source occurrence set changed: {source_ids!r}")
    fail_ids = {row["notifyId"] for row in binding["failClosedOccurrences"]}
    if fail_ids != set(EXPECTED_FAIL_CLOSED):
        raise ContractError(f"Whirlwind fail-closed occurrence set changed: {fail_ids!r}")
    return binding


def _validate_encounter(
    repository_root: Path,
    mapping: dict[str, Any],
    binding: dict[str, Any],
) -> None:
    path = repository_root / mapping["gameplayAuthority"]["encounterDocument"]
    encounter = load_json(path)
    if encounter.get("bossArchetypeId") != mapping["bossArchetypeId"]:
        raise ContractError("Encounter boss archetype disagrees with effect mapping")
    pattern = _one(
        encounter.get("patterns", []),
        lambda row: row.get("patternId") == binding["patternId"],
        "Encounter Whirlwind pattern",
    )
    if EXPECTED_SOURCE_ACTION_ID not in pattern.get("sourceActionIds", []):
        raise ContractError("Encounter Whirlwind lost source action 420633")
    stage = _one(
        pattern.get("stages", []),
        lambda row: row.get("stageId") == binding["semanticStageId"],
        "Encounter Whirlwind semantic stage",
    )
    if stage.get("actionId") != binding["actionId"]:
        raise ContractError("Encounter Whirlwind stage actionId changed")
    if mapping["gameplayAuthority"]["duplicatedGameplayFields"]:
        raise ContractError("gameplay timing/damage must not be copied into mapping")


def _validate_animation_binding(
    repository_root: Path,
    binding: dict[str, Any],
) -> None:
    document = load_json(repository_root / binding["animationBindingDocument"])
    row = _one(
        document.get("bindings", []),
        lambda item: item.get("actionId") == binding["actionId"],
        "action-qualified Valtan animation binding",
    )
    if row.get("clip") != binding["sourceBranch"]["runtimeClipName"]:
        raise ContractError("Valtan animation binding clip differs from source mapping")


def _source_action_path(
    catalog: dict[str, Any],
    profile_id: str,
    override: Path | None,
) -> Path:
    if override is not None:
        return override.resolve()
    row = _one(
        catalog.get("sourceActionDocuments", []),
        lambda item: item.get("profileId") == profile_id,
        f"source action document for {profile_id}",
    )
    value = row.get("path")
    if not isinstance(value, str) or not value:
        raise ContractError("source action document has no path")
    return Path(value)


def _validate_source_action(
    source_path: Path,
    binding: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    branch = binding["sourceBranch"]
    actual_sha = raw_sha256(source_path)
    if actual_sha != branch["sourceDocumentSha256"]:
        raise ContractError(
            "source action document SHA-256 differs from the pinned mapping"
        )
    source = load_json(source_path)
    if source.get("profileId") != branch["profileId"]:
        raise ContractError("source action profileId differs from mapping")
    action = _one(
        source.get("actions", []),
        lambda row: row.get("actionId") == branch["sourceActionId"],
        "source action 420633",
    )
    stages = {row.get("stageIndex"): row for row in action.get("stages", [])}
    for stage_index, expected_clip in EXPECTED_SOURCE_CLIPS.items():
        stage = stages.get(stage_index)
        if stage is None:
            raise ContractError(f"source branch lost stage {stage_index}")
        clips = stage.get("animationClips", [])
        if len(clips) != 1 or clips[0].get("clipName") != expected_clip:
            raise ContractError(
                f"source branch stage {stage_index} clip changed from {expected_clip}"
            )

    preceding = stages[1]
    condition = _one(
        preceding.get("notifies", []),
        lambda row: row.get("notifyId") == EXPECTED_BRANCH_CONDITION_ID,
        "preceding source branch condition",
    )
    mapped_condition = branch["condition"]
    if (
        condition.get("sourceType") != mapped_condition["sourceType"]
        or condition.get("resolutionStatus")
        != mapped_condition["sourceResolutionStatus"]
    ):
        raise ContractError("source branch condition evidence changed")

    active = stages[EXPECTED_SOURCE_STAGE_INDEX]
    notify_by_id = {
        row.get("notifyId"): row for row in active.get("notifies", [])
    }
    for mapped in binding["sourceOccurrences"]:
        notify = notify_by_id.get(mapped["notifyId"])
        if notify is None:
            raise ContractError(f"missing exact source notify {mapped['notifyId']}")
        expected_fields = {
            "sourceType": notify.get("sourceType"),
            "sourceResolutionStatus": notify.get("resolutionStatus"),
            "sourceNotifyTimeSeconds": notify.get("localTimeSeconds"),
            "sourceNotifyDurationSeconds": notify.get("durationSeconds"),
            "assetReferences": notify.get("assetReferences", []),
        }
        for field, expected_value in expected_fields.items():
            if mapped.get(field) != expected_value:
                raise ContractError(
                    f"{mapped['notifyId']} {field} differs from source action"
                )
        labels = notify.get("serializedLabels", [])
        source_anchor = mapped["attachment"]["sourceAnchorSlotId"]
        if source_anchor and source_anchor not in labels:
            raise ContractError(
                f"{mapped['notifyId']} source anchor has no serialized source evidence"
            )
        if not source_anchor and "B_EffectRoot" in labels:
            raise ContractError(
                f"{mapped['notifyId']} dropped serialized B_EffectRoot evidence"
            )

    fail_by_id = {
        row["notifyId"]: row for row in binding["failClosedOccurrences"]
    }
    unsupported_by_id = {
        row.get("notifyId"): row
        for row in active.get("unsupportedUnresolved", [])
    }
    for notify_id, (source_type, status, reason) in EXPECTED_FAIL_CLOSED.items():
        notify = notify_by_id.get(notify_id)
        mapped = fail_by_id[notify_id]
        if notify is None or notify.get("sourceType") != source_type:
            raise ContractError(f"fail-closed source notify changed: {notify_id}")
        if notify.get("resolutionStatus") != status:
            raise ContractError(f"fail-closed source status changed: {notify_id}")
        if mapped["sourceType"] != source_type or mapped["sourceResolutionStatus"] != status:
            raise ContractError(f"fail-closed mapping status changed: {notify_id}")
        if mapped["serializedLabels"] != notify.get("serializedLabels", []):
            raise ContractError(f"fail-closed serialized labels changed: {notify_id}")
        if mapped["reason"] != reason or mapped["disposition"] != "FAIL_CLOSED":
            raise ContractError(f"fail-closed admission changed: {notify_id}")
        if source_type == "Effect":
            if unsupported_by_id.get(notify_id, {}).get("reason") != reason:
                raise ContractError(f"unresolved Effect reason changed: {notify_id}")

    selected_types = {"Effect", "PawnMaterialParam", "ViewShake"}
    source_fail_ids = {
        row.get("notifyId")
        for row in active.get("notifies", [])
        if row.get("sourceType") in selected_types
    }
    if source_fail_ids != set(EXPECTED_FAIL_CLOSED):
        raise ContractError("source stage unresolved presentation denominator changed")
    return source, active


def _particle_reference(mapped: dict[str, Any]) -> str:
    rows = [
        row["objectPath"]
        for row in mapped["assetReferences"]
        if row["className"] == "ParticleSystem"
    ]
    if len(rows) != 1:
        raise ContractError(
            f"{mapped['notifyId']} must preserve one ParticleSystem reference"
        )
    return rows[0]


def _validate_source_catalog(
    catalog: dict[str, Any],
    binding: dict[str, Any],
) -> None:
    systems = catalog.get("sourceSystems", [])
    for mapped in binding["sourceOccurrences"]:
        source_system = mapped["sourceSystem"]
        catalog_id = source_system["catalogSourceAsset"]
        row = _one(
            systems,
            lambda item: str(item.get("sourceAsset", "")).casefold()
            == catalog_id.casefold(),
            f"source system {catalog_id}",
        )
        if str(_particle_reference(mapped)).casefold() != catalog_id.casefold():
            raise ContractError(
                f"{mapped['notifyId']} ParticleSystem/canonical catalog ID differ"
            )
        if row.get("resolutionStatus") != source_system["resolutionStatus"]:
            raise ContractError(f"source system resolution changed: {catalog_id}")
        root = row.get("graph", {}).get("rootNodeId")
        expected_root = EXPECTED_GRAPH_ROOTS.get(catalog_id.casefold())
        if expected_root is None:
            raise ContractError(f"unexpected Whirlwind source system: {catalog_id}")
        if root != expected_root or root != source_system["graphRootNodeId"]:
            raise ContractError(f"source graph root changed: {catalog_id}")
        if EXPECTED_SOURCE_ACTION_ID not in row.get("actionIds", []):
            raise ContractError(f"source system lost action 420633: {catalog_id}")
        if EXPECTED_SOURCE_CLIPS[2] not in row.get("clipNames", []):
            raise ContractError(f"source system lost active clip evidence: {catalog_id}")


def _load_source_artifacts(
    repository_root: Path,
    binding: dict[str, Any],
    source_artifact_root_override: Path | None,
) -> tuple[dict[str, Any], Path, dict[str, Any], dict[str, Any]]:
    evidence = binding["sourceEvidence"]
    action_catalog_path = (
        repository_root / binding["sourceParticleResourceCatalogDocument"]
    )
    if raw_sha256(action_catalog_path) != evidence[
        "sourceParticleResourceCatalogSha256"
    ]:
        raise ContractError("action particle/resource catalog SHA-256 changed")
    action_catalog = load_json(action_catalog_path)
    if (
        action_catalog.get("schema")
        != "lostark.unbound-class-particle-resource-catalog"
        or action_catalog.get("formatVersion") != 1
        or action_catalog.get("characterClass") != "VALTAN"
        or action_catalog.get("bindingStatus") != "ACTION_NOTIFY_BOUND"
    ):
        raise ContractError("action particle/resource catalog identity is invalid")
    _validate_source_catalog(action_catalog, binding)

    source_root = _discover_source_artifact_root(
        action_catalog,
        evidence["artifactRootName"],
        source_artifact_root_override,
    )
    graph_specs: list[tuple[str, Path]] = []
    catalog_graphs = action_catalog.get("sourcePackageGraphs", [])
    for graph_document in evidence["sourceGraphDocuments"]:
        logical_package = graph_document["logicalPackage"]
        graph_path = _relative_artifact_path(
            source_root,
            graph_document["sourcePath"],
            f"source graph {logical_package}",
        )
        if raw_sha256(graph_path) != graph_document["sha256"]:
            raise ContractError(f"source graph SHA-256 changed: {logical_package}")
        catalog_row = _one(
            catalog_graphs,
            lambda row: str(row.get("logicalPackage", "")).casefold()
            == logical_package.casefold(),
            f"action catalog graph {logical_package}",
        )
        if Path(str(catalog_row.get("graphFile") or "")).resolve() != graph_path.resolve():
            raise ContractError(
                f"action catalog/source graph path differs: {logical_package}"
            )
        graph_specs.append((logical_package, graph_path))
    graph_index = source_receipts.load_graphs(graph_specs)

    cook_path = _relative_artifact_path(
        source_root,
        evidence["runtimeCookReceiptSourcePath"],
        "runtime cook receipt",
    )
    if raw_sha256(cook_path) != evidence["runtimeCookReceiptSha256"]:
        raise ContractError("Valtan runtime cook receipt SHA-256 changed")
    cook = load_json(cook_path)
    if cook.get("characterClass") != "VALTAN" or cook.get("failures") != []:
        raise ContractError("Valtan runtime cook receipt is not a clean Valtan cook")
    return action_catalog, source_root, cook, graph_index


def _runtime_binding_rows(cook: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    seen: set[str] = set()
    for asset in cook.get("assets", []):
        source_path = str(asset.get("sourceAssetPath") or "")
        runtime_asset_id = str(asset.get("runtimeAssetId") or "")
        status = str(asset.get("status") or "")
        if not source_path or not runtime_asset_id or status not in {"COPIED", "COOKED"}:
            continue
        key = source_path.casefold()
        if key in seen:
            raise ContractError(
                f"runtime cook receipt has duplicate source asset: {source_path}"
            )
        seen.add(key)
        rows.append(
            {
                "role": str(asset.get("role") or ""),
                "sourceObjectPath": source_path,
                "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                "assetId": runtime_asset_id,
                "candidateCount": 1,
            }
        )
    return rows


def _material_rows_for_systems(
    action_catalog: dict[str, Any],
    systems: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    required = source_receipts.required_material_sources(
        [
            {
                "sourceSystemId": row["sourceSystemId"],
                "graph": {"resourceBindings": row["resourceBindings"]},
            }
            for row in systems
        ]
    )
    rows = action_catalog.get("materialParameterBindings", [])
    return [
        _one(
            rows,
            lambda row, key=key: str(
                row.get("sourceMaterialPath", "")
            ).casefold()
            == key,
            f"action material {required[key]['sourceMaterialPath']}",
        )
        for key in sorted(required)
    ]


def _cook_asset(
    cook: dict[str, Any], source_object_path: str
) -> dict[str, Any]:
    return _one(
        cook.get("assets", []),
        lambda row: str(row.get("sourceAssetPath", "")).casefold()
        == source_object_path.casefold(),
        f"runtime cook asset {source_object_path}",
    )


def _validate_action_catalog_asset(
    action_catalog: dict[str, Any],
    source_object_path: str,
    source_system_id: str,
    role: str,
) -> None:
    row = _one(
        action_catalog.get("assets", []),
        lambda item: str(item.get("sourceAssetPath", "")).casefold()
        == source_object_path.casefold(),
        f"action catalog asset {source_object_path}",
    )
    if (
        EXPECTED_SOURCE_ACTION_ID not in row.get("actionIds", [])
        or role.casefold()
        not in {str(value).casefold() for value in row.get("roles", [])}
        or source_system_id.casefold()
        not in {
            str(value).casefold() for value in row.get("sourceSystems", [])
        }
        or row.get("resolutionStatus") != "RESOLVED_SOURCE_PACKAGE"
    ):
        raise ContractError(
            f"action catalog ownership is incomplete: {source_object_path}"
        )


def _validate_visible_resources(
    repository_root: Path,
    source_root: Path,
    cook: dict[str, Any],
    action_catalog: dict[str, Any],
    source_system_id: str,
    carrier: dict[str, Any],
    imported_resources: list[dict[str, Any]],
) -> None:
    mapped_pairs = {
        (row["slotId"], row["assetId"]) for row in carrier["resources"]
    }
    imported_pairs = {
        (row["slotId"], row["assetId"]) for row in imported_resources
    }
    if carrier["rendererShape"] == "mesh":
        allowed_extra = {
            (row["slotId"], row["assetId"])
            for row in carrier["resources"]
            if row["slotId"] == "base"
            and any(
                mask["sourceObjectPath"].casefold()
                == row["sourceObjectPath"].casefold()
                and mask["assetId"] == row["assetId"]
                for mask in carrier["resources"]
                if mask["slotId"] == "mask"
            )
        }
        if not imported_pairs.issubset(mapped_pairs) or (
            mapped_pairs - imported_pairs
        ) != allowed_extra:
            raise ContractError(
                f"{carrier['carrierId']} Mesh resource roles differ from exact source cook"
            )
    elif mapped_pairs != imported_pairs:
        raise ContractError(
            f"{carrier['carrierId']} resources differ from exact source cook"
        )

    for resource in carrier["resources"]:
        _validate_action_catalog_asset(
            action_catalog,
            resource["sourceObjectPath"],
            source_system_id,
            "mesh" if resource["slotId"] == "meshModel" else "texture",
        )
        cooked = _cook_asset(cook, resource["sourceObjectPath"])
        if (
            cooked.get("runtimeAssetId") != resource["assetId"]
            or cooked.get("sha256") != resource["sha256"]
            or cooked.get("status") not in {"COPIED", "COOKED"}
        ):
            raise ContractError(
                f"{carrier['carrierId']} runtime resource evidence changed: "
                f"{resource['sourceObjectPath']}"
            )
        runtime_path = (
            repository_root
            / RUNTIME_RESOURCE_ROOT
            / PurePosixPath(resource["assetId"])
        )
        if raw_sha256(runtime_path) != resource["sha256"]:
            raise ContractError(
                f"{carrier['carrierId']} runtime resource file changed: "
                f"{resource['assetId']}"
            )

    geometry = carrier.get("geometryEvidence")
    if geometry is None:
        return
    if (
        raw_sha256(
            _relative_artifact_path(
                source_root, geometry["sourceGltfPath"], "source Mesh glTF"
            )
        )
        != geometry["sourceGltfSha256"]
        or raw_sha256(
            _relative_artifact_path(
                source_root, geometry["sourceBinPath"], "source Mesh buffer"
            )
        )
        != geometry["sourceBinSha256"]
    ):
        raise ContractError(
            f"{carrier['carrierId']} source Mesh geometry evidence changed"
        )
    cooked = _cook_asset(cook, geometry["sourceObjectPath"])
    if (
        cooked.get("status") != geometry["cookStatus"]
        or cooked.get("runtimeAssetId") != geometry["runtimeAssetId"]
        or cooked.get("sha256") != geometry["runtimeSha256"]
    ):
        raise ContractError(
            f"{carrier['carrierId']} cooked Mesh geometry evidence changed"
        )


def _source_profile_from_repository_contract(
    repository_root: Path,
    carrier: dict[str, Any],
) -> tuple[dict[str, Any], str]:
    admission = carrier["materialAdmission"]
    evidence_path = repository_root / admission["evidencePath"]
    if raw_sha256(evidence_path) != admission["evidenceSha256"]:
        raise ContractError(
            f"{carrier['carrierId']} source Material contract SHA-256 changed"
        )
    contract = load_json(evidence_path)
    identity = _one(
        contract.get("materialIdentities", []),
        lambda row: str(row.get("sourceMaterialPath", "")).casefold()
        == carrier["sourceMaterialPath"].casefold(),
        f"source Material contract identity {carrier['sourceMaterialPath']}",
    )
    expected = (
        admission["profileId"],
        admission["runtimeShaderProfileId"],
        admission["parentMaterialPath"],
        admission["semanticStatus"],
    )
    actual = (
        identity.get("profileId"),
        identity.get("runtimeShaderProfileId"),
        identity.get("parentMaterialPath"),
        identity.get("semanticStatus"),
    )
    if actual != expected or identity.get("fallbackBlockedReason") is not None:
        raise ContractError(
            f"{carrier['carrierId']} source Material contract admission changed"
        )
    resource_by_source = {
        row["sourceObjectPath"].casefold(): row["assetId"]
        for row in carrier["resources"]
    }
    textures = []
    for row in identity.get("sourceParameters", {}).get("textures", []):
        normalized = normalize_texture_parameter(row)
        source_path = str(normalized.get("sourceObjectPath") or "")
        if source_path.casefold() in resource_by_source:
            normalized["assetId"] = resource_by_source[source_path.casefold()]
        textures.append(normalized)
    profile = {
        "enabled": True,
        "profileId": admission["profileId"],
        "runtimeShaderProfileId": admission["runtimeShaderProfileId"],
        "parentMaterialPath": admission["parentMaterialPath"],
        "semanticStatus": "reconstructed_profile",
        "textures": textures,
        "scalars": [
            normalize_scalar_parameter(row)
            for row in identity.get("sourceParameters", {}).get("scalars", [])
        ],
        "vectors": [
            normalize_vector_parameter(row)
            for row in identity.get("sourceParameters", {}).get("vectors", [])
        ],
        "staticSwitches": [
            normalize_switch_parameter(row)
            for row in identity.get("sourceParameters", {}).get(
                "staticSwitches", []
            )
        ],
        "dynamicParameterSemantics": ["unbound"] * 4,
        "subUVMode": "none",
    }
    render_state = identity.get("renderState", {})
    if (
        render_state.get("blendMode") != "BLEND_Additive"
        or render_state.get("twoSided") is not False
        or render_state.get("disableDepthTest") is not False
    ):
        raise ContractError(
            f"{carrier['carrierId']} source Material render state changed"
        )
    return profile, "additive_one_sided_depth_read"


def _source_profile_from_parent_props(
    source_root: Path,
    carrier: dict[str, Any],
    material_row: dict[str, Any],
    portable: dict[str, Any],
) -> tuple[dict[str, Any], str]:
    admission = carrier["materialAdmission"]
    evidence_path = _relative_artifact_path(
        source_root, admission["evidencePath"], "parent Material props"
    )
    if raw_sha256(evidence_path) != admission["evidenceSha256"]:
        raise ContractError(
            f"{carrier['carrierId']} parent Material props SHA-256 changed"
        )
    evidence = parse_material_dump(evidence_path.read_text(encoding="utf-8-sig"))
    if (
        material_row.get("parent") != admission["parentMaterialPath"]
        or stable_profile_id(admission["parentMaterialPath"])
        != admission["profileId"]
        or admission["runtimeShaderProfileId"]
        != "effect.ue3.missiletrail-01.v1"
    ):
        raise ContractError(
            f"{carrier['carrierId']} finite missile-trail profile identity changed"
        )
    render_state = evidence.get("renderState", {})
    if (
        render_state.get("blendMode") != "BLEND_Translucent"
        or render_state.get("twoSided") is not False
        or render_state.get("disableDepthTest") is not False
        or evidence.get("expressionCoverage", {}).get("topologyStatus")
        != "NON_NULL_ENTRIES_PRESENT"
    ):
        raise ContractError(
            f"{carrier['carrierId']} parent missile-trail evidence is incomplete"
        )
    selected = copy.deepcopy(material_row)
    selected["materialEvidence"] = evidence
    parameters = merged_source_parameters(selected, {})
    profile = {
        "enabled": True,
        "profileId": admission["profileId"],
        "runtimeShaderProfileId": admission["runtimeShaderProfileId"],
        "parentMaterialPath": admission["parentMaterialPath"],
        "semanticStatus": "reconstructed_profile",
        # Runtime slots below are exact and pinned.  Sampler metadata for the
        # two textures not covered by a UE3 sampling receipt is intentionally
        # not guessed into named-texture metadata.
        "textures": [],
        "scalars": [
            normalize_scalar_parameter(row)
            for row in parameters.get("scalars", [])
        ],
        "vectors": [
            normalize_vector_parameter(row)
            for row in parameters.get("vectors", [])
        ],
        "staticSwitches": [
            normalize_switch_parameter(row)
            for row in parameters.get("staticSwitches", [])
        ],
        "dynamicParameterSemantics": runtime_profile_dynamic_parameter_semantics(
            admission["runtimeShaderProfileId"], {"sourceRecipe": portable}
        ),
        "subUVMode": "none",
    }
    if profile["dynamicParameterSemantics"] != [
        "missile_alpha_pan",
        "missile_noise_strength",
        "missile_noise_pan",
        "missile_dissolve",
    ]:
        raise ContractError(
            f"{carrier['carrierId']} missile-trail DynamicParameter ABI changed"
        )
    return profile, "alpha_one_sided_depth_read"


def _validate_fail_closed_source_evidence(
    carrier: dict[str, Any],
    system: dict[str, Any],
    raw_recipe: dict[str, Any],
    imported_resources: list[dict[str, Any]],
    cook: dict[str, Any],
) -> None:
    carrier_id = carrier["carrierId"]
    expected_blockers = EXPECTED_CARRIER_BLOCKERS.get(carrier_id)
    if expected_blockers is None or tuple(carrier["blockers"]) != expected_blockers:
        raise ContractError(f"{carrier_id} fail-closed blocker identity changed")
    classes = {
        str(module.get("className") or "").casefold()
        for module in raw_recipe["modules"]
    }
    if ".notify004." in carrier_id:
        if "particlemoduletypedataanimtrail" not in classes:
            raise ContractError(f"{carrier_id} lost Animation Trail type evidence")
        return
    if carrier_id.endswith("emitter7034"):
        dynamic = _one(
            raw_recipe["modules"],
            lambda row: str(row.get("className", "")).casefold()
            == "particlemoduleparameterdynamic",
            "Dust DynamicParameter module",
        )
        names = [
            literal["value"]
            for literal in dynamic["literals"]
            if str(literal.get("propertyPath", "")).endswith(".paramname")
        ]
        if names != [
            "dissolve_density(0~1)",
            "alpha_power(1~)",
            "emissive_tiling(0.5~2)",
            "lamp_time(0~1)",
        ] or {row["slotId"] for row in imported_resources} != {
            "base",
            "dissolve",
        }:
            raise ContractError(
                f"{carrier_id} Dust material/parameter evidence changed"
            )
        return
    if carrier_id.endswith("emitter7035"):
        cooked_paths = {
            str(row.get("sourceAssetPath") or "").casefold()
            for row in cook.get("assets", [])
        }
        required_aura = {
            "fx_tex_00.fx_a_glow_009",
            "fx_tex_high_00.fx_a_cloud_026",
        }
        if imported_resources or cooked_paths.intersection(required_aura):
            raise ContractError(
                f"{carrier_id} Aura resource admission changed; re-audit mapping"
            )
        return
    if carrier_id.endswith("emitter6823"):
        unresolved = system["unresolvedExternalReferences"]
        external_modules = [
            row
            for row in unresolved
            if row.get("reason") == "external_graph_package_not_loaded"
            and row.get("property") == "modules"
        ]
        point_lights = [
            row for row in unresolved if row.get("property") == "pointlightcomponent"
        ]
        if (
            "efparticlemoduletypedatalight" not in classes
            or len(external_modules) != 6
            or len(point_lights) != 1
        ):
            raise ContractError(f"{carrier_id} typed Light closure changed")
        return
    raise ContractError(f"unexpected fail-closed source carrier: {carrier_id}")


def _build_source_carriers(
    repository_root: Path,
    binding: dict[str, Any],
    action_catalog: dict[str, Any],
    source_root: Path,
    cook: dict[str, Any],
    graph_index: dict[str, Any],
) -> list[dict[str, Any]]:
    systems: list[dict[str, Any]] = []
    detailed_nodes: dict[str, dict[str, Any]] = {}
    detailed_edges: list[dict[str, Any]] = []
    occurrence_by_system: dict[str, dict[str, Any]] = {}
    for occurrence in binding["sourceOccurrences"]:
        source_asset = occurrence["sourceSystem"]["catalogSourceAsset"]
        parts = source_receipts.source_asset_parts(source_asset)
        if parts is None:
            raise ContractError(f"invalid source system identity: {source_asset}")
        root = source_receipts.find_particle_system(graph_index, *parts)
        if root is None:
            raise ContractError(f"source graph has no exact system: {source_asset}")
        package_key, root_row = root
        graph = source_receipts.collect_system_graph(
            graph_index, package_key, root_row
        )
        if graph["rootNodeId"] != occurrence["sourceSystem"]["graphRootNodeId"]:
            raise ContractError(f"source graph root changed: {source_asset}")
        system = {
            "sourceSystemId": source_asset.casefold(),
            "sourceAsset": source_asset,
            "logicalPackage": graph_index["packages"][package_key]["package"],
            "objectName": root_row["objectName"],
            "objectPath": root_row.get("objectPath"),
            "rootNodeId": graph["rootNodeId"],
            "nodeIds": graph["nodeIds"],
            "resourceBindings": graph["resourceBindings"],
            "unresolvedExternalReferences": graph[
                "unresolvedExternalReferences"
            ],
            "summary": graph["summary"],
        }
        systems.append(system)
        detailed_nodes.update(graph["nodes"])
        detailed_edges.extend(
            {**edge, "sourceSystemId": system["sourceSystemId"]}
            for edge in graph["edges"]
        )
        occurrence_by_system[system["sourceSystemId"]] = occurrence

    normalized_graph = {
        "schema": "lostark.normalized-effect-source-graph",
        "schemaVersion": 1,
        "characterClass": "VALTAN",
        "skillId": EXPECTED_SOURCE_ACTION_ID,
        "sourceSystems": systems,
        "nodes": [detailed_nodes[key] for key in sorted(detailed_nodes)],
        "edges": detailed_edges,
        "materialParameterBindings": _material_rows_for_systems(
            action_catalog, systems
        ),
        "runtimeResourceBindings": _runtime_binding_rows(cook),
    }
    source_index = imported_effects.SourceIndex(normalized_graph, {"packages": []})
    partitions = imported_effects.selected_lod_partitions(
        normalized_graph, source_index, {"packages": []}
    )
    derived: list[dict[str, Any]] = []
    system_offsets: dict[str, int] = {}
    for partition_index, (system, emitter, lod, modules) in enumerate(
        partitions, start=1
    ):
        system_id = str(system["sourceSystemId"])
        occurrence = occurrence_by_system[system_id]
        source_order = system_offsets.get(system_id, 0)
        system_offsets[system_id] = source_order + 1
        mapped_carriers = occurrence["admission"]["carriers"]
        if source_order >= len(mapped_carriers):
            raise ContractError(
                f"{occurrence['notifyId']} source emitter denominator grew"
            )
        carrier = mapped_carriers[source_order]
        renderer_shape = imported_effects.classify(system, modules)[2]
        if (
            carrier["sourceOrder"] != source_order
            or carrier["sourceEmitterNodeId"] != emitter.source_id
            or carrier["sourceEmitterPath"] != emitter.object_path
            or carrier["sourceLodNodeId"] != lod.source_id
            or carrier["rendererShape"] != renderer_shape
        ):
            raise ContractError(
                f"{occurrence['notifyId']} exact emitter/LOD identity changed"
            )
        detail, detail_mappings, bursts = imported_effects.emitter_detail(
            source_index,
            lod,
            modules,
            occurrence["sourceNotifyTimeSeconds"],
            occurrence["sourceNotifyDurationSeconds"],
            partition_index,
        )
        raw_recipe = imported_effects.build_source_recipe(
            source_index, modules, renderer_shape, bursts
        )
        recipe_admission = carrier["sourceRecipe"]
        distribution_count = sum(
            len(module["distributions"])
            for module in raw_recipe["modules"]
        )
        if (
            len(raw_recipe["modules"]) != recipe_admission["moduleCount"]
            or distribution_count != recipe_admission["distributionCount"]
            or canonical_sha256(raw_recipe)
            != recipe_admission["sourceRecipeSha256"]
        ):
            raise ContractError(
                f"{carrier['carrierId']} exact SourceRecipe evidence changed"
            )
        portable: dict[str, Any] | None = None
        try:
            portable = portable_recipe(raw_recipe)
        except MaterializeError as error:
            if recipe_admission["portableStatus"] not in {
                "FAIL_CLOSED_UNSUPPORTED_MODULE_FAMILY",
                "FAIL_CLOSED_INCOMPLETE_SOURCE_GRAPH",
            }:
                raise ContractError(
                    f"{carrier['carrierId']} unexpectedly lost portable SourceRecipe: {error}"
                ) from error
        else:
            if recipe_admission["portableStatus"] not in {
                "PORTABLE_AUTHORED_V13",
                "PORTABLE_RECIPE_MATERIAL_BLOCKED",
            } or canonical_sha256(portable) != recipe_admission.get(
                "portableRecipeSha256"
            ):
                raise ContractError(
                    f"{carrier['carrierId']} portable SourceRecipe admission changed"
                )

        module_ids = {
            lod.source_id,
            emitter.source_id,
            *(module.source_id.split("@ref:", 1)[0] for module in modules),
        }
        imported_resources, _resource_receipt, material_rows = (
            imported_effects.choose_resources(
                system, module_ids, normalized_graph
            )
        )
        if len(material_rows) != 1 or str(
            material_rows[0].get("sourceMaterialPath") or ""
        ).casefold() != carrier["sourceMaterialPath"].casefold():
            raise ContractError(
                f"{carrier['carrierId']} exact source Material binding changed"
            )
        material_row = material_rows[0]
        if carrier["disposition"] == "VISIBLE_EXECUTABLE":
            if portable is None or material_row.get("resolutionStatus") != (
                "RESOLVED_EXACT_SOURCE_PACKAGE"
            ):
                raise ContractError(
                    f"{carrier['carrierId']} visible source evidence is incomplete"
                )
            _validate_action_catalog_asset(
                action_catalog,
                carrier["sourceMaterialPath"],
                system_id,
                "material",
            )
            _validate_visible_resources(
                repository_root,
                source_root,
                cook,
                action_catalog,
                system_id,
                carrier,
                imported_resources,
            )
            if carrier["materialAdmission"]["evidenceKind"] == (
                "REPOSITORY_SOURCE_MATERIAL_CONTRACT"
            ):
                source_profile, render_profile = (
                    _source_profile_from_repository_contract(
                        repository_root, carrier
                    )
                )
                source_profile["dynamicParameterSemantics"] = (
                    dynamic_parameter_semantics({"sourceRecipe": portable})
                )
            else:
                source_profile, render_profile = _source_profile_from_parent_props(
                    source_root, carrier, material_row, portable
                )
            if renderer_shape == "mesh":
                detail["mesh"]["useModelMaterial"] = (
                    imported_effects.mesh_uses_model_material(
                        modules, detail_mappings
                    )
                )
                detail["particle"]["billboard"] = False
            material = {
                "templateId": "effect.standard",
                "sourceMaterialPath": carrier["sourceMaterialPath"],
                "renderProfile": render_profile,
                "sourceProfile": source_profile,
            }
            resources = [
                {"slotId": row["slotId"], "assetId": row["assetId"]}
                for row in carrier["resources"]
            ]
        else:
            _validate_fail_closed_source_evidence(
                carrier,
                system,
                raw_recipe,
                imported_resources,
                cook,
            )
            material = {
                "templateId": "effect.standard",
                "sourceMaterialPath": carrier["sourceMaterialPath"],
                "renderProfile": "alpha_two_sided_depth_read",
                "sourceProfile": {"enabled": False},
            }
            resources = []
        derived.append(
            {
                "occurrence": occurrence,
                "carrier": carrier,
                "detail": detail,
                "portableRecipe": portable,
                "material": material,
                "resources": resources,
            }
        )

    if any(
        system_offsets.get(
            occurrence["sourceSystem"]["catalogSourceAsset"].casefold(), 0
        )
        != occurrence["admission"]["carrierDenominator"]
        for occurrence in binding["sourceOccurrences"]
    ):
        raise ContractError("first-LOD source carrier denominator changed")
    if len(derived) != EXPECTED_CARRIER_DENOMINATOR:
        raise ContractError("Whirlwind total source carrier denominator changed")
    return derived


def validate_source_contract(
    repository_root: Path,
    mapping: dict[str, Any],
    schema: dict[str, Any],
    source_action_override: Path | None = None,
    source_artifact_root_override: Path | None = None,
) -> tuple[dict[str, Any], dict[str, Any], list[dict[str, Any]]]:
    validate_mapping(mapping, schema)
    binding = _validate_fixed_identity(mapping)
    _validate_encounter(repository_root, mapping, binding)
    _validate_animation_binding(repository_root, binding)
    catalog = load_json(repository_root / binding["sourceCatalogDocument"])
    source_path = _source_action_path(
        catalog,
        binding["sourceBranch"]["profileId"],
        source_action_override,
    )
    catalog_source_row = _one(
        catalog.get("sourceActionDocuments", []),
        lambda row: row.get("profileId") == binding["sourceBranch"]["profileId"],
        "catalog source action identity",
    )
    if catalog_source_row.get("sha256") != binding["sourceBranch"]["sourceDocumentSha256"]:
        raise ContractError("source catalog/action mapping SHA-256 pin differs")
    _validate_source_action(source_path, binding)
    _validate_source_catalog(catalog, binding)

    bone = binding["modelBoneEvidence"]
    model_path = (
        repository_root / RUNTIME_RESOURCE_ROOT / bone["runtimeModelAssetId"]
    )
    actual_sha = raw_sha256(model_path)
    derived_bone = derive_bone_evidence(
        read_wmodel_bones(model_path),
        bone["sourceAnchorSlotId"],
        bone["runtimeModelAssetId"],
        actual_sha,
    )
    if derived_bone != bone:
        raise ContractError(
            "B_EffectRoot physical bone evidence differs from mapping; "
            "silent root fallback is forbidden"
        )
    for row in binding["sourceOccurrences"]:
        attachment = row["attachment"]
        if attachment["sourceAnchorSlotId"] == bone["sourceAnchorSlotId"]:
            if (
                attachment["runtimeBoneName"] != bone["runtimeBoneName"]
                or attachment["admission"]
                != "ADMITTED_EXPLICIT_RUNTIME_BONE"
            ):
                raise ContractError(
                    f"{row['notifyId']} did not use the proven physical bone"
                )
        elif attachment["runtimeBoneName"]:
            raise ContractError(
                f"{row['notifyId']} guessed a runtime bone without source evidence"
            )
    action_catalog, source_root, cook, graph_index = _load_source_artifacts(
        repository_root,
        binding,
        source_artifact_root_override,
    )
    carriers = _build_source_carriers(
        repository_root,
        binding,
        action_catalog,
        source_root,
        cook,
        graph_index,
    )
    return binding, catalog, carriers


def _default_detail(start: float, duration: float) -> dict[str, Any]:
    # A zero-duration source notify is valid provenance, while the v13 codec
    # requires every editable carrier to own a positive finite lifetime.  This
    # 1 ms value is carrier metadata only; the exact source duration remains in
    # Valtan.patterneffects.json and is never treated as gameplay timing.
    carrier_lifetime = max(duration, 0.001)
    return {
        "transform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
            "velocityPerSecond": [0.0, 0.0, 0.0],
        },
        "color": {
            "offset": [0.0, 0.0, 0.0, 0.0],
            "multiply": [1.0, 1.0, 1.0, 1.0],
            "clip": 0.0,
            "emissiveIntensity": 1.0,
            "distortionIntensity": 0.0,
            "distortionOnBaseMaterial": False,
            "radialTime": 0.0,
            "radialIntensity": 0.0,
        },
        "uv": {
            "start": [0.0, 0.0],
            "speed": [0.0, 0.0],
            "wave": False,
            "waveAmplitude": [0.0, 0.0],
            "waveFrequency": 1.0,
            "sequence": False,
            "loop": True,
            "sequenceTerm": 0.1,
            "tileColumns": 1,
            "tileRows": 1,
            "tileIndex": 0,
        },
        "timing": {
            "startDelaySeconds": start,
            "lifeTimeSeconds": carrier_lifetime,
            "afterImageSeconds": 0.0,
            "dissolveStartNormalized": 1.0,
        },
        "mesh": {"useModelMaterial": True},
        "sprite": {"billboard": True, "billboardRollDegrees": 0.0},
        "decal": {"size": [1.0, 1.0], "depth": 0.25},
        "linearLerp": {
            "position": False,
            "endPosition": [0.0, 0.0, 0.0],
            "rotation": False,
            "endRotationDegrees": [0.0, 0.0, 0.0],
            "revolution": False,
            "endRevolutionDegreesPerSecond": [0.0, 0.0, 0.0],
            "scale": False,
            "endScale": [1.0, 1.0, 1.0],
            "velocity": False,
            "endVelocityPerSecond": [0.0, 0.0, 0.0],
            "colorOffset": False,
            "endColorOffset": [0.0, 0.0, 0.0, 0.0],
            "colorMultiply": False,
            "endColorMultiply": [1.0, 1.0, 1.0, 1.0],
            "emissiveIntensity": False,
            "endEmissiveIntensity": 1.0,
        },
        "particle": {
            "maxParticles": 1,
            "spawnRatePerSecond": 0.0,
            "burstCount": 0,
            "randomSeed": 420633,
            "lifeTimeSeconds": [carrier_lifetime, carrier_lifetime],
            "initialPositionMin": [0.0, 0.0, 0.0],
            "initialPositionMax": [0.0, 0.0, 0.0],
            "initialVelocityMin": [0.0, 0.0, 0.0],
            "initialVelocityMax": [0.0, 0.0, 0.0],
            "acceleration": [0.0, 0.0, 0.0],
            "startSize": [1.0, 1.0],
            "endSize": [1.0, 1.0],
            "localSpace": True,
            "billboard": True,
        },
        "trail": {
            "maxPoints": 64,
            "pointLifeTimeSeconds": 0.35,
            "sampleIntervalSeconds": 0.0166667,
            "minimumDistance": 0.01,
            "startWidth": 0.2,
            "endWidth": 0.0,
            "faceCamera": True,
        },
        "afterImage": {
            "sampleIntervalSeconds": 0.05,
            "maxCopies": 16,
            "alphaExponent": 1.0,
        },
        "light": {"enabled": False},
        "screenPost": {"enabled": False},
    }


def _element_kind(
    occurrence: dict[str, Any], carrier: dict[str, Any]
) -> str:
    if occurrence["sourceType"] == "Trails":
        return "trail"
    if carrier["rendererShape"] == "light":
        return "light"
    return "particle"


def _build_element(
    binding: dict[str, Any],
    derived: dict[str, Any],
    occurrence_index: int,
) -> dict[str, Any]:
    occurrence = derived["occurrence"]
    carrier = derived["carrier"]
    attachment = occurrence["attachment"]
    admitted_bone = attachment["admission"] == "ADMITTED_EXPLICIT_RUNTIME_BONE"
    particle_path = _particle_reference(occurrence)
    visible = carrier["disposition"] == "VISIBLE_EXECUTABLE"
    emitter_leaf = carrier["sourceEmitterPath"].rsplit(".", 1)[-1]
    notify_leaf = occurrence["notifyId"].rsplit("/", 1)[-1]
    return {
        "id": carrier["carrierId"],
        "displayName": f"{notify_leaf} | {emitter_leaf} | "
        + ("portable v13" if visible else "fail-closed"),
        "groupId": "valtan.whirlwind.420633.active",
        "sourceNode": (
            f"{occurrence['notifyId']}|{carrier['sourceEmitterNodeId']}|"
            f"{carrier['sourceLodNodeId']}"
        ),
        "visible": visible,
        "kind": _element_kind(occurrence, carrier),
        "resources": copy.deepcopy(derived["resources"]),
        "material": copy.deepcopy(derived["material"]),
        "actionCueAttachment": {
            "enabled": admitted_bone,
            "follow": admitted_bone,
            "sourceAnchorSlotId": attachment["sourceAnchorSlotId"],
            "runtimeAnchorSlotId": attachment["sourceAnchorSlotId"],
            "runtimeBoneName": attachment["runtimeBoneName"],
            "snapshotRootSourceBasisYawDegrees": 0.0,
            "socketLocalTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        },
        "transformInheritance": {
            "enabled": False,
            "masterElementId": "",
        },
        "detail": copy.deepcopy(derived["detail"]),
        "sourceRecipe": (
            copy.deepcopy(derived["portableRecipe"])
            if visible
            else {
                "enabled": False,
                "rendererShape": "",
                "emitterDelaySeconds": 0.0,
                "emitterDurationSeconds": 0.0,
                "emitterLoopCount": 0,
                "bursts": [],
                "modules": [],
            }
        ),
        "sourcePresentation": {
            "enabled": True,
            "schema": "lostark.effect-source-presentation",
            "version": 1,
            "profileId": (
                "valtan.portable-authored-v13.v1"
                if visible
                else "valtan.source-evidence-only.v1"
            ),
            "status": "reconstructed" if visible else "unresolved",
            "sourceObjectPath": particle_path,
            "sourceActionCueId": binding["actionId"],
            "sourceEventId": occurrence["notifyId"],
            "sourceOccurrenceIndex": occurrence_index,
            "sourceTimeSeconds": occurrence["sourceNotifyTimeSeconds"],
            "parameters": [],
        },
    }


def build_canary(
    binding: dict[str, Any], source_carriers: list[dict[str, Any]]
) -> dict[str, Any]:
    occurrence_indices = {
        row["notifyId"]: index
        for index, row in enumerate(binding["sourceOccurrences"])
    }
    return {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": binding["effectAssetId"],
        "displayName": "Valtan Whirlwind 420633 Active | Portable Canary",
        "particleSystem": {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        },
        "modelCues": [],
        "elements": [
            _build_element(
                binding,
                row,
                occurrence_indices[row["occurrence"]["notifyId"]],
            )
            for row in source_carriers
        ],
    }


def validate_canary(
    document: dict[str, Any],
    binding: dict[str, Any],
    source_carriers: list[dict[str, Any]],
) -> None:
    expected_root_fields = {
        "schema",
        "version",
        "effectAssetId",
        "displayName",
        "particleSystem",
        "modelCues",
        "elements",
    }
    if set(document) != expected_root_fields:
        raise ContractError("v13 canary root field set changed")
    if (
        document["schema"] != "lostark.effect-authoring"
        or document["version"] != 13
        or document["effectAssetId"] != binding["effectAssetId"]
        or document["modelCues"] != []
    ):
        raise ContractError("v13 canary identity changed")
    if not 1 <= len(document["displayName"].encode("utf-8")) <= 64:
        raise ContractError("v13 canary displayName exceeds the codec limit")
    rows = document.get("elements")
    if not isinstance(rows, list) or len(rows) != EXPECTED_CARRIER_DENOMINATOR:
        raise ContractError("v13 canary source carrier denominator changed")
    expected_by_id = {
        row["carrier"]["carrierId"]: row for row in source_carriers
    }
    seen: set[str] = set()
    expected_element_fields = {
        "id",
        "displayName",
        "groupId",
        "sourceNode",
        "visible",
        "kind",
        "resources",
        "material",
        "actionCueAttachment",
        "transformInheritance",
        "detail",
        "sourceRecipe",
        "sourcePresentation",
    }
    for index, element in enumerate(rows):
        if set(element) != expected_element_fields:
            raise ContractError(f"v13 canary element[{index}] field set changed")
        if not 1 <= len(element["displayName"].encode("utf-8")) <= 64:
            raise ContractError(f"v13 canary element[{index}] displayName exceeds 64 bytes")
        if len(element["sourceNode"].encode("utf-8")) > 256:
            raise ContractError(f"v13 canary element[{index}] sourceNode exceeds 256 bytes")
        carrier_id = element.get("id")
        derived = expected_by_id.get(carrier_id)
        if derived is None or carrier_id in seen:
            raise ContractError("v13 canary carrier identity is missing or duplicate")
        seen.add(carrier_id)
        occurrence_index = next(
            position
            for position, occurrence in enumerate(binding["sourceOccurrences"])
            if occurrence["notifyId"]
            == derived["occurrence"]["notifyId"]
        )
        expected = _build_element(binding, derived, occurrence_index)
        if element != expected:
            raise ContractError(
                f"v13 canary carrier differs from source evidence: {carrier_id}"
            )
        detail = element["detail"]
        if (
            detail["timing"]["lifeTimeSeconds"] <= 0.0
            or detail["particle"]["lifeTimeSeconds"][0] <= 0.0
        ):
            raise ContractError(
                f"v13 canary carrier violates positive lifetime: {carrier_id}"
            )
        visible = derived["carrier"]["disposition"] == "VISIBLE_EXECUTABLE"
        if (
            element["visible"] is not visible
            or bool(element["resources"]) is not visible
            or element["sourceRecipe"]["enabled"] is not visible
            or element["material"]["sourceProfile"]["enabled"] is not visible
        ):
            raise ContractError(
                f"v13 canary execution admission differs: {carrier_id}"
            )
        presentation = element["sourcePresentation"]
        if (
            presentation.get("enabled") is not True
            or presentation.get("status")
            != ("reconstructed" if visible else "unresolved")
        ):
            raise ContractError(
                f"v13 canary source presentation admission differs: {carrier_id}"
            )
    if seen != set(expected_by_id):
        raise ContractError("v13 canary exact carrier set changed")
    visible_count = sum(row["visible"] is True for row in rows)
    if (
        visible_count != EXPECTED_VISIBLE_CARRIER_COUNT
        or len(rows) - visible_count != EXPECTED_FAIL_CLOSED_CARRIER_COUNT
    ):
        raise ContractError("v13 canary visible/fail-closed denominator changed")
    admission = binding["productAdmission"]
    if (
        admission["status"] != "FAIL_CLOSED_NON_PRODUCT_CANARY"
        or admission["productCatalogMapped"] is not False
        or admission["animationEventMapped"] is not False
    ):
        raise ContractError("v13 canary was accidentally promoted to product mapping")


def write_transactionally(document: dict[str, Any], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    payload = pretty_json_bytes(document)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            prefix=output_path.name + ".",
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
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def build_and_write(
    repository_root: Path = REPOSITORY_ROOT,
    mapping_path: Path | None = None,
    output_path: Path | None = None,
    source_action_override: Path | None = None,
    source_artifact_root_override: Path | None = None,
    check: bool = False,
) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    mapping_path = (
        mapping_path.resolve()
        if mapping_path is not None
        else repository_root / MAPPING_RELATIVE_PATH
    )
    mapping = load_json(mapping_path)
    schema = load_json(repository_root / SCHEMA_RELATIVE_PATH)
    binding, _catalog, source_carriers = validate_source_contract(
        repository_root,
        mapping,
        schema,
        source_action_override,
        source_artifact_root_override,
    )
    document = build_canary(binding, source_carriers)
    validate_canary(document, binding, source_carriers)
    output_path = (
        output_path.resolve()
        if output_path is not None
        else repository_root / binding["effectDocument"]
    )
    expected = pretty_json_bytes(document)
    if check:
        try:
            actual = output_path.read_bytes()
        except OSError as error:
            raise ContractError(f"could not read canary output {output_path}: {error}") from error
        if actual != expected:
            raise ContractError(f"v13 canary output is stale: {output_path}")
    else:
        write_transactionally(document, output_path)
    return document


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument("--mapping", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--source-action", type=Path)
    parser.add_argument("--source-artifact-root", type=Path)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args()
    try:
        document = build_and_write(
            arguments.repository_root,
            arguments.mapping,
            arguments.output,
            arguments.source_action,
            arguments.source_artifact_root,
            arguments.check,
        )
    except ContractError as error:
        print(f"[valtan-whirlwind-canary] FAIL: {error}")
        return 1
    verb = "verified" if arguments.check else "generated"
    print(
        f"[valtan-whirlwind-canary] PASS: {verb} "
        f"{document['effectAssetId']} v{document['version']} "
        f"with {sum(row['visible'] for row in document['elements'])}/"
        f"{len(document['elements'])} visible source carriers; product mapping blocked"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
