#!/usr/bin/env python3
"""Select the canonical Action stage and preserve every presentation cue.

The compact ``.animnotify`` reference intentionally drops type-specific Action
payloads.  This builder joins that authored clip selection back to the original
LOA extraction and emits a byte-lossless cue recipe.  Runtime support is audited
separately; preserving a payload never implies that its semantics execute.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
import math
import re
import struct
from collections import Counter
from pathlib import Path
from typing import Any


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"expected JSON object: {path}")
    return value


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def normalize_clip(value: str) -> str:
    result = value.casefold()
    for prefix in ("pc_sp_m_00_sk_", "pc_sp_f_00_sk_"):
        if result.startswith(prefix):
            return result[len(prefix) :]
    return result


def raw_asset(notify: dict[str, Any]) -> str:
    references = notify.get("assetReferences", [])
    if not references:
        return ""
    return str(references[0].get("objectPath") or "")


def event_matches_notify(
    event: dict[str, Any], notify: dict[str, Any], tolerance: float = 0.0002
) -> bool:
    if str(event.get("sourceType") or "").casefold() != str(
        notify.get("sourceType") or ""
    ).casefold():
        return False
    if abs(
        float(event.get("localTimeSeconds", 0.0))
        - float(notify.get("localTimeSeconds", 0.0))
    ) > tolerance:
        return False
    if abs(
        float(event.get("durationSeconds", 0.0))
        - float(notify.get("durationSeconds", 0.0))
    ) > tolerance:
        return False
    if str(event.get("sourceType") or "").casefold() in {
        "playparticleeffect",
        "playcameraparticleeffect",
        "defaultparticle",
    }:
        return str(event.get("sourceAsset") or "").casefold() == raw_asset(
            notify
        ).casefold()
    return True


def match_count(
    events: list[dict[str, Any]], notifies: list[dict[str, Any]]
) -> int:
    available = list(range(len(notifies)))
    matched = 0
    for event in events:
        for index in available:
            if event_matches_notify(event, notifies[index]):
                available.remove(index)
                matched += 1
                break
    return matched


def select_stage(
    action: dict[str, Any],
    clip_name: str,
    reference_events: list[dict[str, Any]],
) -> tuple[dict[str, Any], dict[str, Any]]:
    normalized = normalize_clip(clip_name)
    candidates = [
        stage
        for stage in action.get("stages", [])
        if normalized
        in {
            normalize_clip(str(row.get("clipName") or ""))
            for row in stage.get("animationClips", [])
        }
    ]
    if not candidates:
        raise ValueError(
            f"Action {action.get('actionId')} has no stage for clip {clip_name}"
        )

    scored: list[tuple[int, int, int, dict[str, Any]]] = []
    for stage in candidates:
        notifies = list(stage.get("notifies", []))
        matched = match_count(reference_events, notifies)
        presentation_count = sum(
            row.get("authority") == "PRESENTATION"
            and row.get("category") != "animation"
            for row in notifies
        )
        scored.append(
            (
                matched,
                -abs(presentation_count - len(reference_events)),
                -int(stage.get("stageIndex", 0)),
                stage,
            )
        )
    scored.sort(key=lambda row: row[:3], reverse=True)
    selected = scored[0]
    return selected[3], {
        "candidateStageIndices": [
            int(row.get("stageIndex", 0)) for row in candidates
        ],
        "selectedStageIndex": int(selected[3].get("stageIndex", 0)),
        "referenceEventCount": len(reference_events),
        "matchedReferenceEventCount": selected[0],
        "tieBreakContract": "BASE_STAGE_LOWEST_INDEX_ON_EQUAL_SCORE",
    }


def cue_channel(source_type: str, category: str) -> str:
    explicit = {
        "DominantDirectionalLight_Control": "DIRECTIONAL_LIGHT",
        "HidePawn": "CHARACTER_VISIBILITY",
        "SceneCapture": "SCENE_CAPTURE",
        "UltimateSkillCameraControl": "CAMERA_CONTROL",
        "PlaySkeletalMeshMaterialParam": "MODEL_MATERIAL",
    }
    if source_type in explicit:
        return explicit[source_type]
    return {
        "particle": "PARTICLE_SYSTEM",
        "decal": "DECAL",
        "trail": "CHARACTER_AFTERIMAGE",
        "mesh": "MODEL_CUE",
        "material": "CHARACTER_MATERIAL",
        "post_process": "SCREEN_POST",
        "camera": "CAMERA_SHAKE",
        "sound": "SOUND",
        "light": "DIRECTIONAL_LIGHT",
        "visibility": "CHARACTER_VISIBILITY",
        "character_parts": "CHARACTER_PARTS",
    }.get(category, "PRESENTATION_OTHER")


def source_execution_status(
    runtime_channel: str, typed_payload: dict[str, Any]
) -> str:
    if runtime_channel == "MODEL_CUE":
        return (
            "RUNTIME_MODEL_BINDING_REQUIRED"
            if typed_payload.get("transformDecoded", False)
            else "UNSUPPORTED_COMPOUND_MODEL_PAYLOAD"
        )
    if runtime_channel == "MODEL_MATERIAL" and not typed_payload.get(
        "semanticDecoded", False
    ):
        return "UNSUPPORTED_MODEL_MATERIAL_SEMANTICS"
    return "SEMANTIC_EXECUTION_AUDIT_REQUIRED"


def decode_typed_payload(
    source_type: str,
    payload: dict[str, Any],
    socket_contract: dict[str, Any] | None = None,
    asset_references: list[dict[str, Any]] | None = None,
    serialized_labels: list[str] | None = None,
) -> dict[str, Any]:
    source_type_lower = source_type.casefold()
    if source_type_lower == "playskeletalmesh":
        raw = base64.b64decode(str(payload.get("data") or ""), validate=True)
        signature = b"CEFActionNotify_PlaySkeletalMesh\x00"
        if not raw.startswith(signature):
            raise ValueError("PlaySkeletalMesh payload header is invalid")
        references = asset_references or []
        labels = serialized_labels or []
        skeletal_meshes = [
            str(row.get("objectPath") or "")
            for row in references
            if str(row.get("className") or "").casefold() == "skeletalmesh"
        ]
        animation_sets = [
            str(row.get("objectPath") or "")
            for row in references
            if str(row.get("className") or "").casefold() == "animset"
        ]
        materials = [
            str(row.get("objectPath") or "")
            for row in references
            if str(row.get("className") or "").casefold()
            == "materialinstanceconstant"
        ]
        if len(skeletal_meshes) != 1 or len(animation_sets) != 1:
            raise ValueError(
                "PlaySkeletalMesh must reference one SkeletalMesh and AnimSet"
            )
        target_names = [
            str(label)
            for label in labels
            if str(label).casefold().startswith("sk_")
            and "'" not in str(label)
        ]
        result: dict[str, Any] = {
            "schema": "lostark.cef-play-skeletal-mesh",
            "version": 1,
            "enabled": True,
            "sourceSkeletalMesh": skeletal_meshes[0],
            "sourceAnimSet": animation_sets[0],
            "sourceMaterialInstances": materials,
            "sourceCueName": target_names[0] if target_names else "",
            "transformDecoded": False,
        }
        particle_data_markers = list(
            re.finditer(re.escape(b"CEFParticleData\x00"), raw)
        )
        # A standalone skeletal cue owns one CEFParticleData block.  Compound
        # cues may append particle attachments with additional blocks and stay
        # fail-closed until their nested table is decoded independently.
        if len(particle_data_markers) != 1:
            return result
        marker_end = particle_data_markers[0].end()
        transform_start = marker_end + 72
        if transform_start + 92 > len(raw):
            raise ValueError("PlaySkeletalMesh transform payload is truncated")

        def model_vector3(offset: int) -> list[float]:
            values = list(struct.unpack_from("<fff", raw, transform_start + offset))
            if not all(math.isfinite(value) for value in values):
                raise ValueError(
                    "PlaySkeletalMesh transform contains non-finite values"
                )
            return values

        position_ue = model_vector3(20)
        rotation_degrees = model_vector3(32)
        scale = model_vector3(80)
        if any(value <= 0.0 for value in scale):
            raise ValueError("PlaySkeletalMesh transform scale must be positive")
        result.update({
            "transformDecoded": True,
            "localTransform": {
                "sourcePositionUeUnits": position_ue,
                "position": [value * 0.01 for value in position_ue],
                "rotationDegrees": rotation_degrees,
                "scale": scale,
            },
            "sourceTransformByteOffset": transform_start,
        })
        return result
    if source_type_lower == "playskeletalmeshmaterialparam":
        labels = serialized_labels or []
        target_names = [
            str(label)
            for label in labels
            if str(label).casefold().startswith("sk_")
        ]
        return {
            "schema": "lostark.cef-play-skeletal-mesh-material-param",
            "version": 1,
            "enabled": True,
            "sourceCueName": target_names[0] if target_names else "",
            "semanticDecoded": False,
        }
    if source_type_lower != "playparticleeffect":
        return {"schema": "lostark.action-cue-payload", "enabled": True}
    raw = base64.b64decode(str(payload.get("data") or ""), validate=True)
    signature = b"CEFActionNotify_PlayParticleEffect\x00"
    if len(raw) <= 47 or not raw.startswith(signature):
        raise ValueError("PlayParticleEffect payload header is invalid")
    enabled_value = int(raw[47])
    if enabled_value not in (0, 1):
        raise ValueError(
            "PlayParticleEffect enabled flag is not a source boolean: "
            f"{enabled_value}"
        )
    result: dict[str, Any] = {
        "schema": "lostark.cef-play-particle-effect-header",
        "version": 2,
        "enabled": bool(enabled_value),
        "sourceByteOffset": 47,
        "sourceByteValue": enabled_value,
    }
    reference = re.search(rb"ParticleSystem'[^']+'\0", raw)
    if reference is None or reference.start() < 4:
        result["particleDataDecoded"] = False
        return result
    reference_length = struct.unpack_from("<i", raw, reference.start() - 4)[0]
    if reference_length != reference.end() - reference.start():
        raise ValueError("CEFParticleData ParticleSystem reference is malformed")

    base = reference.end()

    def read_string(position: int) -> tuple[str, int] | None:
        if position < 0 or position + 4 > len(raw):
            return None
        length = struct.unpack_from("<i", raw, position)[0]
        if length <= 1 or position + 4 + length > len(raw):
            return None
        end = position + 4 + length
        if raw[end - 1] != 0:
            return None
        try:
            return raw[position + 4 : end - 1].decode("ascii"), end
        except UnicodeDecodeError:
            return None

    source_anchors: list[str] = []
    attachment_selector_offset: int | None = None
    transform_start = base + 60
    for selector_offset, string_offset in ((52, 56), (56, 60)):
        if base + selector_offset + 4 > len(raw):
            continue
        selector = struct.unpack_from("<i", raw, base + selector_offset)[0]
        if selector <= 0 or selector > 16:
            continue
        cursor = base + string_offset
        parsed_anchors: list[str] = []
        for _ in range(selector):
            parsed = read_string(cursor)
            if parsed is None:
                parsed_anchors.clear()
                break
            anchor, cursor = parsed
            parsed_anchors.append(anchor)
        if not parsed_anchors:
            continue
        source_anchors = parsed_anchors
        transform_start = cursor
        attachment_selector_offset = selector_offset
        break
    if transform_start + 96 > len(raw):
        raise ValueError("CEFParticleData transform payload is truncated")

    def vector3(offset: int) -> list[float]:
        values = list(struct.unpack_from("<fff", raw, transform_start + offset))
        if not all(math.isfinite(value) for value in values):
            raise ValueError("CEFParticleData transform contains non-finite values")
        return values



    # The first attachment selector owns one additional serialized float in
    # CEFParticleData.  The second selector and the snapshot form share the
    # compact layout.  This is a source layout distinction, not alignment
    # padding: both forms are packed directly after the optional string.
    transform_variant_offset = (
        4 if attachment_selector_offset == 52 else 0
    )
    position_ue = vector3(16 + transform_variant_offset)
    rotation_degrees = vector3(28 + transform_variant_offset)
    scale = vector3(76 + transform_variant_offset)
    if any(value <= 0.0 for value in scale):
        raise ValueError("CEFParticleData transform scale must be positive")
    socket_index = {
        str(row.get("socketName") or "").casefold(): row
        for row in (socket_contract or {}).get("sockets", [])
        if str(row.get("socketName") or "")
    }
    bone_index = {
        str(row.get("boneName") or "").casefold(): str(row.get("boneName"))
        for row in (socket_contract or {}).get("sockets", [])
        if str(row.get("boneName") or "")
    }
    runtime_anchors = []
    for anchor in source_anchors:
        socket = socket_index.get(anchor.casefold())
        if socket is not None:
            runtime_anchors.append(
                {
                    "sourceAnchorName": anchor,
                    "runtimeAnchorSlotId": anchor,
                    "runtimeBoneName": str(socket["boneName"]),
                    "socketLocalTransform": socket["runtimeLocalTransform"],
                    "resolutionStatus": "EXACT_SOURCE_SOCKET",
                }
            )
        elif anchor.casefold().startswith("b_") or anchor.casefold() in bone_index:
            runtime_anchors.append(
                {
                    "sourceAnchorName": anchor,
                    "runtimeAnchorSlotId": anchor,
                    "runtimeBoneName": bone_index.get(
                        anchor.casefold(), anchor.casefold()
                    ),
                    "socketLocalTransform": {
                        "position": [0.0, 0.0, 0.0],
                        "rotationDegrees": [0.0, 0.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                    "resolutionStatus": "EXACT_SOURCE_BONE",
                }
            )
        else:
            runtime_anchors.append(
                {
                    "sourceAnchorName": anchor,
                    "runtimeAnchorSlotId": anchor,
                    "runtimeBoneName": "",
                    "socketLocalTransform": {
                        "position": [0.0, 0.0, 0.0],
                        "rotationDegrees": [0.0, 0.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                    "resolutionStatus": "MISSING_SOURCE_SOCKET",
                }
            )
    primary_runtime_anchor = runtime_anchors[0] if runtime_anchors else None
    result.update({
        "particleDataDecoded": True,
        "sourceParticleSystem": reference.group()[:-1].decode("ascii"),
        "attachment": {
            "mode": (
                "FOLLOW_NAMED_ANCHORS" if source_anchors
                else "SNAPSHOT_ROOT"
            ),
            "sourceAnchorNames": source_anchors,
            "runtimeAnchors": runtime_anchors,
            "runtimeAnchorSlotIds": [
                row["runtimeAnchorSlotId"] for row in runtime_anchors
            ],
            "runtimeAnchorSlotId": (
                primary_runtime_anchor["runtimeAnchorSlotId"]
                if primary_runtime_anchor is not None
                else "root"
            ),
            "runtimeBoneName": (
                primary_runtime_anchor["runtimeBoneName"]
                if primary_runtime_anchor is not None
                else ""
            ),
            "socketLocalTransform": (
                primary_runtime_anchor["socketLocalTransform"]
                if primary_runtime_anchor is not None
                else {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                }
            ),
            "runtimeResolutionStatus": (
                primary_runtime_anchor["resolutionStatus"]
                if primary_runtime_anchor is not None
                else "EXACT_ROOT_SNAPSHOT"
            ),
            "sourceSelectorByteOffset": (
                None if attachment_selector_offset is None
                else base + attachment_selector_offset
            ),
        },
        "localTransform": {
            "sourcePositionUeUnits": position_ue,
            "position": [value * 0.01 for value in position_ue],
            "rotationDegrees": rotation_degrees,
            "scale": scale,
        },
        "sourceTransformByteOffset": transform_start,
    })
    return result


def build_action_cue_recipe(
    action_source: dict[str, Any],
    source_receipt: dict[str, Any],
    socket_contract: dict[str, Any] | None = None,
) -> dict[str, Any]:
    skill_id = int(source_receipt["skillId"])
    action = next(
        (
            row
            for row in action_source.get("actions", [])
            if int(row.get("actionId", -1)) == skill_id
        ),
        None,
    )
    if action is None:
        raise ValueError(f"Action source does not contain skill {skill_id}")

    timeline = source_receipt["timeline"]
    reference_events = list(timeline.get("events", []))
    selected_stages = []
    cues = []
    matched_total = 0
    for clip in timeline.get("clips", []):
        clip_name = str(clip["clip"])
        clip_events = [
            row for row in reference_events if row.get("clip") == clip_name
        ]
        stage, evidence = select_stage(action, clip_name, clip_events)
        matched_total += int(evidence["matchedReferenceEventCount"])
        selected_stages.append(
            {
                "sequenceIndex": int(clip["sequenceIndex"]),
                "clip": clip_name,
                "clipOffsetSeconds": float(clip["offsetSeconds"]),
                "sourceStageName": str(stage.get("stageName") or ""),
                **evidence,
            }
        )
        available_reference_indices = [
            index
            for index, event in enumerate(reference_events)
            if event.get("clip") == clip_name
        ]
        for notify in stage.get("notifies", []):
            if notify.get("authority") != "PRESENTATION":
                continue
            if notify.get("category") == "animation":
                continue
            category = str(notify.get("category") or "other")
            source_type = str(notify.get("sourceType") or "")
            payload = notify.get("serializedPayload")
            if not isinstance(payload, dict) or not payload.get("data"):
                raise ValueError(
                    f"Action {skill_id} notify {notify.get('notifyId')} "
                    "does not contain its serialized payload"
                )
            reference_event_index = next(
                (
                    index
                    for index in available_reference_indices
                    if event_matches_notify(reference_events[index], notify)
                ),
                None,
            )
            if reference_event_index is not None:
                available_reference_indices.remove(reference_event_index)
            typed_payload = decode_typed_payload(
                source_type, payload, socket_contract,
                list(notify.get("assetReferences", [])),
                list(notify.get("serializedLabels", [])),
            )
            runtime_channel = cue_channel(source_type, category)
            cues.append(
                {
                    "cueId": (
                        f"skill-{skill_id}/clip-{int(clip['sequenceIndex']):03d}/"
                        f"{notify['notifyId'].rsplit('/', 1)[-1]}"
                    ),
                    "clip": clip_name,
                    "clipSequenceIndex": int(clip["sequenceIndex"]),
                    "sourceStageIndex": int(stage["stageIndex"]),
                    "localTimeSeconds": float(notify["localTimeSeconds"]),
                    "globalTimeSeconds": float(clip["offsetSeconds"])
                    + float(notify["localTimeSeconds"]),
                    "durationSeconds": float(notify["durationSeconds"]),
                    "sourceType": source_type,
                    "category": category,
                    "runtimeChannel": runtime_channel,
                    "assetReferences": notify.get("assetReferences", []),
                    "serializedLabels": notify.get("serializedLabels", []),
                    "serializedPayload": payload,
                    "typedPayload": typed_payload,
                    "executionEnabled": bool(typed_payload["enabled"]),
                    "sourceReceiptEventIndex": reference_event_index,
                    "sourceExecutionStatus": source_execution_status(
                        runtime_channel, typed_payload
                    ),
                }
            )

    type_counts = Counter(row["sourceType"] for row in cues)
    channel_counts = Counter(row["runtimeChannel"] for row in cues)
    payload_bytes = sum(
        int(row["serializedPayload"]["byteSize"]) for row in cues
    )
    reference_complete = matched_total == len(reference_events)
    return {
        "schema": "lostark.effect-action-cue-recipe",
        "formatVersion": 2,
        "characterClass": str(source_receipt["characterClass"]),
        "skillId": skill_id,
        "inputSlot": str(source_receipt["inputSlot"]),
        "variantContract": "BASE_NO_TIME_OR_SPACE_AXIS",
        "source": {
            "actionSourceSha256": hashlib.sha256(
                json.dumps(
                    action_source, ensure_ascii=False, sort_keys=True
                ).encode("utf-8")
            ).hexdigest(),
            "loaSha256": str(action_source["source"]["sha256"]),
        },
        "selectedStages": selected_stages,
        "cues": cues,
        "summary": {
            "referenceEventCount": len(reference_events),
            "matchedReferenceEventCount": matched_total,
            "presentationCueCount": len(cues),
            "serializedPayloadBytes": payload_bytes,
            "sourceTypeCounts": dict(sorted(type_counts.items())),
            "runtimeChannelCounts": dict(sorted(channel_counts.items())),
            "enabledPlayParticleEffectCount": sum(
                row["sourceType"].casefold() == "playparticleeffect"
                and row["executionEnabled"]
                for row in cues
            ),
            "disabledPlayParticleEffectCount": sum(
                row["sourceType"].casefold() == "playparticleeffect"
                and not row["executionEnabled"]
                for row in cues
            ),
            "byteLosslessPayloadComplete": all(
                row["serializedPayload"].get("encoding") == "base64"
                and int(row["serializedPayload"].get("byteSize", 0)) > 0
                and len(str(row["serializedPayload"].get("sha256") or ""))
                == 64
                for row in cues
            ),
            "referenceJoinComplete": reference_complete,
            "runtimeAnchorResolutionComplete": all(
                row["sourceType"].casefold() != "playparticleeffect"
                or not row["executionEnabled"]
                or not row["typedPayload"].get("particleDataDecoded", False)
                or not str(
                    row["typedPayload"].get("attachment", {}).get(
                        "runtimeResolutionStatus", ""
                    )
                ).startswith("MISSING_")
                for row in cues
            ),
            "runtimeModelCueTransformComplete": all(
                row["runtimeChannel"] != "MODEL_CUE"
                or not row["executionEnabled"]
                or row["typedPayload"].get("transformDecoded", False)
                for row in cues
            ),
            "runtimeModelMaterialSemanticComplete": all(
                row["runtimeChannel"] != "MODEL_MATERIAL"
                or not row["executionEnabled"]
                or row["typedPayload"].get("semanticDecoded", False)
                for row in cues
            ),
        },
        "sourceExtractionComplete": reference_complete
        and all(
            row["serializedPayload"].get("encoding") == "base64"
            for row in cues
        ),
        "runtimeExecutionComplete": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--action-source", required=True, type=Path)
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--socket-contract", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    recipe = build_action_cue_recipe(
        read_json(args.action_source),
        read_json(args.source_receipt),
        read_json(args.socket_contract) if args.socket_contract else None,
    )
    write_json_atomic(args.output, recipe)
    print(json.dumps(recipe["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if recipe["sourceExtractionComplete"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
