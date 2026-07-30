#!/usr/bin/env python3
"""Recover gameplay-owned Prop placement from Lost Ark DeployData.loa.

This extractor intentionally keeps DeployData separate from the UE3 PS/SL
StaticMesh placement stream.  A visual gameplay Prop is joined from three
authoritative sources:

* DeployData.loa: stable deploy actor id and UE3 Transform
* EFTable_Prop.db: gameplay definition, model LookInfo id, hit/state contract
* EFDLProp_*.loa: fractured/skeletal mesh, particle and audio references

The fixed offsets below are validated against every CEFDeployActor_Prop record
before output.  Unknown trailing fields are not guessed or silently promoted
to runtime state.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import sqlite3
import struct
import tempfile
from collections import Counter
from pathlib import Path
from typing import Any, Iterable

from build_maptool_scene import convert_position, convert_rotation, imported_id


CLASS_NAME = b"CEFDeployActor_Prop\0"
CLASS_FIELD = struct.pack("<I", len(CLASS_NAME)) + CLASS_NAME
POSITION_OFFSET = 0x14
ROTATION_OFFSET = 0x20
DEPLOY_ACTOR_ID_OFFSET = 0x44
DEPLOY_SCALE_OFFSET = 0x4C
ACTIVE_OFFSET = 0x74
PROP_DEFINITION_ID_OFFSET = 0x78
MINIMUM_RECORD_BYTES = 0x7C
OBJECT_REFERENCE = re.compile(rb"([A-Za-z][A-Za-z0-9_]*)'([^']+)'")
TRIGGER_NODE_CLASS = re.compile(rb"CEFSeqTN_[A-Za-z0-9_]+")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            json.dump(value, output, ensure_ascii=False, indent=2)
            output.write("\n")
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def marker_offsets(data: bytes) -> list[int]:
    offsets: list[int] = []
    cursor = 0
    while True:
        offset = data.find(CLASS_NAME, cursor)
        if offset < 0:
            break
        if offset < 4 or data[offset - 4 : offset + len(CLASS_NAME)] != CLASS_FIELD:
            raise ValueError(f"invalid CEFDeployActor_Prop class field at 0x{offset:X}")
        if offset + MINIMUM_RECORD_BYTES > len(data):
            raise ValueError(f"truncated CEFDeployActor_Prop record at 0x{offset:X}")
        offsets.append(offset)
        cursor = offset + len(CLASS_NAME)
    if not offsets:
        raise ValueError("DeployData contains no CEFDeployActor_Prop records")
    return offsets


def record_ends(data: bytes, offsets: list[int]) -> list[int]:
    """Return exclusive record boundaries without accepting a missing sentinel."""
    ends = [offset - 4 for offset in offsets[1:]]
    final_marker = data.find(b"CEFDeployActor_", offsets[-1] + len(CLASS_NAME))
    if final_marker < 4:
        raise ValueError("final CEFDeployActor_Prop record has no following actor marker")
    ends.append(final_marker - 4)
    return ends


def signed_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<i", data, offset)[0]


def load_prop_rows(database: Path, definition_ids: Iterable[int]) -> dict[int, dict[str, Any]]:
    columns = (
        "PrimaryKey, SecondaryKey, Name, Comment, Type, TouchType, ZoneObject, "
        "ZoneObjectType, FallDownType, FallDownByWalk, FallDownTime, "
        "FallDownTriggerSignal, ActorClass, ActorClass2, Model, Scale, Debris, "
        "BlockProjectile, SightCheck, HitType, HitAmount, HitStep1, HitStep2, "
        "HitStep3, TouchAmount, RestoreDelay, JointHP, Blocking, DefaultAction, "
        "StateOnActionId, StateOn2ActionId, StateOn3ActionId, StateOffActionId, "
        "SpawnActionId, DespawnActionTime, HitMeshType, VolumeShapeType, SourceRow"
    )
    rows: dict[int, dict[str, Any]] = {}
    with sqlite3.connect(database) as connection:
        connection.row_factory = sqlite3.Row
        for definition_id in sorted(set(definition_ids)):
            row = connection.execute(
                f"SELECT {columns} FROM Prop WHERE PrimaryKey = ?", (definition_id,)
            ).fetchone()
            if row is None:
                raise ValueError(f"Prop definition is missing from EFTable_Prop.db: {definition_id}")
            rows[definition_id] = dict(row)
    return rows


def load_state_off_actions(
    game_action_database: Path | None,
    skill_effect_database: Path | None,
    action_ids: Iterable[int],
) -> dict[int, dict[str, Any]]:
    action_ids = sorted({action_id for action_id in action_ids if action_id})
    if not action_ids:
        return {}
    if game_action_database is None or skill_effect_database is None:
        return {}

    placeholders = ",".join("?" for _ in action_ids)
    actions: dict[int, list[dict[str, Any]]] = {action_id: [] for action_id in action_ids}
    with sqlite3.connect(game_action_database) as connection:
        connection.row_factory = sqlite3.Row
        rows = connection.execute(
            "SELECT PrimaryKey, SecondaryKey, Type, EffectId, ClassifyType, "
            "ClassifyIndex, SourceRow FROM GameAction "
            f"WHERE PrimaryKey IN ({placeholders}) ORDER BY PrimaryKey, SecondaryKey",
            action_ids,
        ).fetchall()
    effect_ids = sorted({int(row["EffectId"]) for row in rows if int(row["EffectId"])})
    effects: dict[int, list[dict[str, Any]]] = {effect_id: [] for effect_id in effect_ids}
    if effect_ids:
        effect_placeholders = ",".join("?" for _ in effect_ids)
        with sqlite3.connect(skill_effect_database) as connection:
            connection.row_factory = sqlite3.Row
            effect_rows = connection.execute(
                "SELECT PrimaryKey, SecondaryKey, Key, ValueA, ValueB, HitType, "
                "Target, AreaType, AreaRange, AreaHeight, HittedSetUseType, "
                "HittedSetSkillKey, HitStrengthX, HitStrengthZ, FallDown, SourceRow "
                "FROM SkillEffect "
                f"WHERE PrimaryKey IN ({effect_placeholders}) "
                "ORDER BY PrimaryKey, SecondaryKey",
                effect_ids,
            ).fetchall()
        for row in effect_rows:
            effects[int(row["PrimaryKey"])].append(dict(row))

    for row in rows:
        action = dict(row)
        effect_id = int(action["EffectId"])
        action["skillEffects"] = effects.get(effect_id, [])
        actions[int(action["PrimaryKey"])].append(action)
    unresolved = [action_id for action_id, rows_for_id in actions.items() if not rows_for_id]
    if unresolved:
        raise ValueError(f"StateOffActionId missing from GameAction DB: {unresolved}")
    return {
        action_id: {"stateOffActionId": action_id, "gameActions": action_rows}
        for action_id, action_rows in actions.items()
    }


def resolve_static_mesh_gltf(
    static_raw_root: Path | None, reference: dict[str, str]
) -> str | None:
    if static_raw_root is None or reference["kind"] not in (
        "FracturedStaticMesh",
        "StaticMesh",
    ):
        return None
    object_path = reference["objectPath"]
    package = object_path.split(".", 1)[0]
    object_name = object_path.rsplit(".", 1)[-1].casefold() + ".gltf"
    matches = [
        path
        for path in static_raw_root.glob(
            f"{package}__*/{package}/StaticMesh3/*.gltf"
        )
        if path.name.casefold() == object_name
    ]
    if len(matches) > 1:
        raise ValueError(f"ambiguous extracted mesh for {object_path}: {matches}")
    return str(matches[0]) if matches else None


def resolve_intact_sibling_gltf(extracted_path: str | None) -> str | None:
    """Resolve a same-package naming sibling without promoting it to a direct ref."""
    if extracted_path is None:
        return None
    fractured = Path(extracted_path)
    suffix = "_fractured.gltf"
    if not fractured.name.casefold().endswith(suffix):
        return None
    intact = fractured.with_name(fractured.name[: -len(suffix)] + ".gltf")
    return str(intact) if intact.is_file() else None


def parse_lookinfo(
    directory: Path | None, static_raw_root: Path | None
) -> dict[str, dict[str, Any]]:
    if directory is None:
        return {}
    if not directory.is_dir():
        raise ValueError(f"LookInfo directory does not exist: {directory}")

    result: dict[str, dict[str, Any]] = {}
    for path in sorted(directory.glob("EFDLProp_*.loa")):
        references: list[dict[str, str]] = []
        seen: set[tuple[str, str]] = set()
        for match in OBJECT_REFERENCE.finditer(path.read_bytes()):
            kind = match.group(1).decode("ascii")
            object_path = match.group(2).decode("ascii")
            key = (kind, object_path)
            if key in seen:
                continue
            seen.add(key)
            references.append({"kind": kind, "objectPath": object_path})
        model_key = path.stem
        mesh_references = [
            reference
            for reference in references
            if reference["kind"]
            in ("FracturedStaticMesh", "StaticMesh", "SkeletalMesh")
        ]
        for reference in mesh_references:
            reference["extractedStaticMeshGltf"] = resolve_static_mesh_gltf(
                static_raw_root, reference
            )
            reference["extractedIntactSiblingGltf"] = resolve_intact_sibling_gltf(
                reference["extractedStaticMeshGltf"]
            )
            if reference["extractedIntactSiblingGltf"] is not None:
                reference["intactSiblingEvidence"] = (
                    "same-package-name-sibling-not-LookInfo-direct-reference"
                )
        result[model_key] = {
            "source": str(path),
            "sha256": sha256(path),
            "meshReferences": mesh_references,
            "particleReferences": [
                reference
                for reference in references
                if reference["kind"] == "ParticleSystem"
            ],
            "audioReferences": [
                reference for reference in references if reference["kind"] == "AkEvent"
            ],
            "allObjectReferences": references,
        }
    return result


def classify(prop: dict[str, Any]) -> str:
    if int(prop["Type"]) == 2 or any(
        int(prop[field]) != 0
        for field in (
            "Debris",
            "BlockProjectile",
            "StateOffActionId",
            "DespawnActionTime",
            "HitMeshType",
        )
    ) or int(prop["HitAmount"]) > 1:
        return "destructible-or-stateful"
    if str(prop["Model"]):
        return "visual-prop"
    if int(prop["Blocking"]):
        return "gameplay-volume-or-blocker"
    return "nonvisual-gameplay-prop"


def read_record(
    data: bytes,
    offset: int,
    record_end: int,
    record_index: int,
    prop: dict[str, Any],
    lookinfo: dict[str, dict[str, Any]],
    state_off_actions: dict[int, dict[str, Any]],
    trigger_data: bytes | None,
    arena_center: tuple[float, float, float],
    arena_radius_cm: float,
) -> dict[str, Any]:
    x, y, z = struct.unpack_from("<3f", data, offset + POSITION_OFFSET)
    pitch, yaw, roll = struct.unpack_from("<3i", data, offset + ROTATION_OFFSET)
    deploy_actor_id = struct.unpack_from("<I", data, offset + DEPLOY_ACTOR_ID_OFFSET)[0]
    deploy_scale_percent = struct.unpack_from("<I", data, offset + DEPLOY_SCALE_OFFSET)[0]
    raw_field_0x74 = struct.unpack_from("<I", data, offset + ACTIVE_OFFSET)[0]
    definition_id = struct.unpack_from("<I", data, offset + PROP_DEFINITION_ID_OFFSET)[0]

    if definition_id != int(prop["PrimaryKey"]):
        raise ValueError(f"Prop DB join changed while reading record {record_index}")
    if not all(math.isfinite(value) for value in (x, y, z)):
        raise ValueError(f"non-finite position in record {record_index}")
    if not (0x10000000 <= deploy_actor_id <= 0x1FFFFFFF):
        raise ValueError(
            f"unexpected deploy actor id 0x{deploy_actor_id:08X} in record {record_index}"
        )
    if deploy_scale_percent == 0 or deploy_scale_percent > 10000:
        raise ValueError(f"invalid deploy scale in record {record_index}")
    if record_end <= offset or record_end > len(data):
        raise ValueError(f"invalid record boundary in record {record_index}")

    source_id = f"DeployData:37051:Prop:0x{deploy_actor_id:08X}"
    rotation = {"pitch": pitch, "yaw": yaw, "roll": roll}
    database_scale = int(prop["Scale"])
    combined_scale = deploy_scale_percent * database_scale / 10000.0
    distance_xy = math.hypot(x - arena_center[0], y - arena_center[1])
    model = str(prop["Model"])
    model_info = lookinfo.get(model)
    state_off_action_id = int(prop["StateOffActionId"])
    trigger_binary_occurrence_count = (
        trigger_data.count(struct.pack("<I", deploy_actor_id))
        if trigger_data is not None
        else None
    )
    return {
        "recordIndex": record_index,
        "recordOffset": offset,
        "sourcePlacementId": source_id,
        "runtimePlacementId": imported_id(source_id),
        "deployActorId": deploy_actor_id,
        "deployActorIdHex": f"0x{deploy_actor_id:08X}",
        "propDefinitionId": definition_id,
        "classification": classify(prop),
        "rawRecordSha256": hashlib.sha256(data[offset - 4 : record_end]).hexdigest().upper(),
        "rawValidatedFields": {
            "field0x64": struct.unpack_from("<I", data, offset + 0x64)[0],
            "field0x68": struct.unpack_from("<I", data, offset + 0x68)[0],
            "field0x74": raw_field_0x74,
            "field0x90": struct.unpack_from("<I", data, offset + 0x90)[0],
            "field0x94": struct.unpack_from("<I", data, offset + 0x94)[0],
            "field0x9C": struct.unpack_from("<I", data, offset + 0x9C)[0],
            "field0xA4": struct.unpack_from("<I", data, offset + 0xA4)[0],
            "field0xCC": struct.unpack_from("<I", data, offset + 0xCC)[0],
            "field0xD0": struct.unpack_from("<I", data, offset + 0xD0)[0],
            "field0xD4": struct.unpack_from("<I", data, offset + 0xD4)[0],
            "field0xE4": struct.unpack_from("<I", data, offset + 0xE4)[0],
            "field0x108": struct.unpack_from("<I", data, offset + 0x108)[0],
        },
        "sourceTransform": {
            "positionCm": {"x": x, "y": y, "z": z},
            "rotationUnits": rotation,
            "deployScalePercent": deploy_scale_percent,
            "databaseScalePercent": database_scale,
        },
        "clientTransform": {
            "positionMeters": list(convert_position({"x": x, "y": y, "z": z})),
            "quaternion": list(convert_rotation(rotation)),
            "uniformScale": combined_scale,
        },
        "arena": {
            "distanceXYCm": distance_xy,
            "withinRadius": distance_xy <= arena_radius_cm,
        },
        "prop": prop,
        "lookInfo": model_info,
        "lookInfoStatus": (
            "not-required-no-model"
            if not model
            else "resolved"
            if model_info is not None
            else "missing"
        ),
        "stateOffAction": state_off_actions.get(state_off_action_id),
        "triggerBinaryOccurrenceCount": trigger_binary_occurrence_count,
    }


def summarize(records: list[dict[str, Any]]) -> dict[str, Any]:
    definition_counts = Counter(record["propDefinitionId"] for record in records)
    classification_counts = Counter(record["classification"] for record in records)
    arena_records = [record for record in records if record["arena"]["withinRadius"]]
    arena_visual = [record for record in arena_records if record["prop"]["Model"]]
    arena_destructible = [
        record
        for record in arena_records
        if record["classification"] == "destructible-or-stateful"
    ]
    arena_extracted_static = [
        record
        for record in arena_visual
        if any(
            reference.get("extractedStaticMeshGltf")
            for reference in (record["lookInfo"] or {}).get("meshReferences", [])
        )
    ]
    arena_skeletal = [
        record
        for record in arena_visual
        if any(
            reference["kind"] == "SkeletalMesh"
            for reference in (record["lookInfo"] or {}).get("meshReferences", [])
        )
    ]
    arena_trigger_binary_occurrence = [
        record
        for record in arena_records
        if record["triggerBinaryOccurrenceCount"] is not None
        and record["triggerBinaryOccurrenceCount"] > 0
    ]
    arena_state_off_resolved = [
        record
        for record in arena_records
        if int(record["prop"]["StateOffActionId"]) != 0
        and record["stateOffAction"] is not None
    ]
    return {
        "recordCount": len(records),
        "uniqueDeployActorIdCount": len({record["deployActorId"] for record in records}),
        "uniqueRuntimePlacementIdCount": len(
            {record["runtimePlacementId"] for record in records}
        ),
        "uniquePropDefinitionCount": len(definition_counts),
        "definitionCounts": {
            str(key): definition_counts[key] for key in sorted(definition_counts)
        },
        "classificationCounts": {
            key: classification_counts[key] for key in sorted(classification_counts)
        },
        "modelResolvedCount": sum(bool(record["prop"]["Model"]) for record in records),
        "lookInfoMissingCount": sum(
            record["lookInfoStatus"] == "missing" for record in records
        ),
        "arenaRecordCount": len(arena_records),
        "arenaVisualRecordCount": len(arena_visual),
        "arenaDestructibleOrStatefulCount": len(arena_destructible),
        "arenaExtractedStaticMeshRecordCount": len(arena_extracted_static),
        "arenaSkeletalMeshRecordCount": len(arena_skeletal),
        "arenaTriggerBinaryOccurrenceRecordCount": len(
            arena_trigger_binary_occurrence
        ),
        "arenaStateOffActionResolvedRecordCount": len(arena_state_off_resolved),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--deploy-data", required=True, type=Path)
    parser.add_argument("--prop-db", required=True, type=Path)
    parser.add_argument("--game-action-db", type=Path)
    parser.add_argument("--skill-effect-db", type=Path)
    parser.add_argument("--lookinfo-dir", type=Path)
    parser.add_argument("--static-raw-root", type=Path)
    parser.add_argument("--trigger-map", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--arena-output", type=Path)
    parser.add_argument("--zone-id", type=int, default=37051)
    parser.add_argument("--arena-center-x", type=float, default=15627.9)
    parser.add_argument("--arena-center-y", type=float, default=12197.7)
    parser.add_argument("--arena-center-z", type=float, default=2324.2)
    parser.add_argument("--arena-radius-cm", type=float, default=2000.0)
    parser.add_argument("--expect-records", type=int)
    parser.add_argument("--expect-arena-records", type=int)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if (args.game_action_db is None) != (args.skill_effect_db is None):
        raise ValueError("--game-action-db and --skill-effect-db must be supplied together")
    data = args.deploy_data.read_bytes()
    offsets = marker_offsets(data)
    ends = record_ends(data, offsets)
    definition_ids = [
        struct.unpack_from("<I", data, offset + PROP_DEFINITION_ID_OFFSET)[0]
        for offset in offsets
    ]
    props = load_prop_rows(args.prop_db, definition_ids)
    state_off_actions = load_state_off_actions(
        args.game_action_db,
        args.skill_effect_db,
        (int(prop["StateOffActionId"]) for prop in props.values()),
    )
    lookinfo = parse_lookinfo(args.lookinfo_dir, args.static_raw_root)
    trigger_data = args.trigger_map.read_bytes() if args.trigger_map is not None else None
    center = (args.arena_center_x, args.arena_center_y, args.arena_center_z)
    records = [
        read_record(
            data,
            offset,
            ends[index],
            index,
            props[definition_ids[index]],
            lookinfo,
            state_off_actions,
            trigger_data,
            center,
            args.arena_radius_cm,
        )
        for index, offset in enumerate(offsets)
    ]
    summary = summarize(records)
    if args.expect_records is not None and summary["recordCount"] != args.expect_records:
        raise ValueError(f"record count mismatch: {summary['recordCount']}")
    if (
        args.expect_arena_records is not None
        and summary["arenaRecordCount"] != args.expect_arena_records
    ):
        raise ValueError(f"arena record count mismatch: {summary['arenaRecordCount']}")

    document = {
        "schemaVersion": 1,
        "zoneId": args.zone_id,
        "sourceContract": {
            "deployData": str(args.deploy_data),
            "deployDataSha256": sha256(args.deploy_data),
            "propDatabase": str(args.prop_db),
            "propDatabaseSha256": sha256(args.prop_db),
            "gameActionDatabase": (
                str(args.game_action_db) if args.game_action_db is not None else None
            ),
            "gameActionDatabaseSha256": (
                sha256(args.game_action_db) if args.game_action_db is not None else None
            ),
            "skillEffectDatabase": (
                str(args.skill_effect_db) if args.skill_effect_db is not None else None
            ),
            "skillEffectDatabaseSha256": (
                sha256(args.skill_effect_db) if args.skill_effect_db is not None else None
            ),
            "recordClass": CLASS_NAME[:-1].decode("ascii"),
            "coordinateBasis": "Client=(UE3.X, UE3.Z, -UE3.Y)*0.01",
            "rotationBasis": "B^T * UE3Rotator * B, DirectX row matrix quaternion",
            "scale": "deployScalePercent/100 * EFTable_Prop.Scale/100",
            "triggerMap": str(args.trigger_map) if args.trigger_map is not None else None,
            "triggerMapSha256": sha256(args.trigger_map) if args.trigger_map is not None else None,
            "triggerEvidence": (
                "raw little-endian deploy actor ID byte occurrence; not a parsed "
                "Trigger node/field reference"
                if args.trigger_map is not None
                else None
            ),
        },
        "triggerNodeClassCounts": (
            {
                key: value
                for key, value in sorted(
                    Counter(
                        match.group().decode("ascii")
                        for match in TRIGGER_NODE_CLASS.finditer(trigger_data)
                    ).items()
                )
            }
            if trigger_data is not None
            else {}
        ),
        "arenaQuery": {
            "centerCm": {"x": center[0], "y": center[1], "z": center[2]},
            "radiusCm": args.arena_radius_cm,
        },
        "summary": summary,
        "records": records,
    }
    atomic_write_json(args.output, document)
    if args.arena_output is not None:
        arena_document = dict(document)
        arena_records = [
            record for record in records if record["arena"]["withinRadius"]
        ]
        arena_document["globalSummary"] = summary
        arena_document["summary"] = summarize(arena_records)
        arena_document["records"] = arena_records
        atomic_write_json(args.arena_output, arena_document)
    print(json.dumps(summary, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
