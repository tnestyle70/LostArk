#!/usr/bin/env python3
"""Extract UE3 Action stages and presentation/gameplay-reference notifies.

Lost Ark keeps animation poses in AnimSet packages, while the clip-local
particle, trail, decal, material, sound, camera, and hit-reference timing is
serialized in ``XmlData/Action/*.loa``.  This reader preserves that source
boundary.  It does not promote ParticleHit or a visual decal to Server damage
authority; those rows remain reference evidence for a later combat timeline.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import struct
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


ACTION_MARKER = b"CEFActionObject\0"
STAGE_MARKER = b"CEFActionStage\0"
NOTIFY_RE = re.compile(rb"CEFActionNotify_[A-Za-z0-9_]+\0")
OBJECT_REFERENCE_RE = re.compile(rb"([A-Za-z][A-Za-z0-9_]*)'([^'\r\n]+)'")


PRESENTATION_CATEGORIES = {
    "Anim": "animation",
    "Stance_Anim": "animation",
    "AnimBlendDirectional": "animation",
    "PlayParticleEffect": "particle",
    "PlayCameraParticleEffect": "particle",
    "DefaultParticle": "particle",
    "PlayDecalEffect": "decal",
    "Trails": "trail",
    "TrailGhostEffect": "trail",
    "PlayStaticMesh": "mesh",
    "PlaySkeletalMesh": "mesh",
    "PawnMaterialParam": "material",
    "PawnMaterialChange": "material",
    "PostProcessChain": "post_process",
    "ViewShake": "camera",
    "AKEvent": "sound",
    "Effect": "unresolved_effect",
}

ANIMATION_NOTIFY_TYPES = {"Anim", "Stance_Anim", "AnimBlendDirectional"}

GAMEPLAY_REFERENCE_TYPES = {
    "ParticleHit",
    "CounterAttack",
    "SuperArmor",
    "SuperArmorPVP",
    "InputTiming",
    "Down",
    "AvoidPc",
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def find_all(data: bytes, marker: bytes, start: int = 0, end: int | None = None) -> list[int]:
    limit = len(data) if end is None else end
    rows: list[int] = []
    position = start
    while True:
        position = data.find(marker, position, limit)
        if position < 0:
            return rows
        rows.append(position)
        position += len(marker)


def read_length_prefixed_string(data: bytes, position: int) -> tuple[str, int] | None:
    if position < 0 or position + 4 > len(data):
        return None
    length = struct.unpack_from("<i", data, position)[0]
    if 0 < length <= 4096:
        end = position + 4 + length
        if end > len(data) or data[end - 1] != 0:
            return None
        payload = data[position + 4 : end - 1]
        for encoding in ("utf-8", "cp949"):
            try:
                return payload.decode(encoding), end
            except UnicodeDecodeError:
                continue
        return None
    if -4096 <= length < 0:
        byte_length = -length * 2
        end = position + 4 + byte_length
        if end > len(data) or data[end - 2 : end] != b"\0\0":
            return None
        try:
            return data[position + 4 : end - 2].decode("utf-16le"), end
        except UnicodeDecodeError:
            return None
    return None


def scan_length_prefixed_strings(data: bytes, start: int, end: int) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    position = start
    while position + 4 <= end:
        value = read_length_prefixed_string(data, position)
        if value is None or value[1] > end:
            position += 1
            continue
        text, next_position = value
        if all(character in "\t" or ord(character) >= 32 for character in text):
            rows.append({"sourceOffset": position, "value": text})
            position = next_position
        else:
            position += 1
    return rows


def read_action_identity(data: bytes, marker_position: int) -> tuple[int, str]:
    payload = marker_position + len(ACTION_MARKER)
    if payload + 12 > len(data):
        raise ValueError(f"truncated CEFActionObject at {marker_position}")
    action_id = struct.unpack_from("<i", data, payload + 4)[0]
    display_name = read_length_prefixed_string(data, payload + 8)
    if display_name is None:
        raise ValueError(f"missing action display name at {marker_position}")
    return action_id, display_name[0]


def read_stage_name(data: bytes, marker_position: int, stage_end: int) -> str:
    fixed = read_length_prefixed_string(data, marker_position + 63)
    if fixed is not None:
        return fixed[0]
    first_notify = min(
        (match.start() for match in NOTIFY_RE.finditer(data, marker_position, stage_end)),
        default=stage_end,
    )
    candidates = scan_length_prefixed_strings(
        data, marker_position + len(STAGE_MARKER), first_notify
    )
    candidates = [
        row["value"]
        for row in candidates
        if row["value"] and not row["value"].startswith("CEFAction")
    ]
    return candidates[0] if candidates else ""


def read_notify_timing(data: bytes, marker_position: int, marker: bytes) -> tuple[float, float, float]:
    payload = marker_position + len(marker)
    if payload + 28 > len(data):
        raise ValueError(f"truncated notify at {marker_position}")
    start, end, duration = struct.unpack_from("<fff", data, payload + 16)
    if not all(math.isfinite(value) for value in (start, end, duration)):
        raise ValueError(f"non-finite notify timing at {marker_position}")
    return start, end, duration


def read_animation_clip(data: bytes, marker_position: int) -> str | None:
    # The Action serializer places the property label at +65 and the clip
    # identifier at +78 from the start of the marker string.  Valtan has a
    # handful of non-"Anim" labels but keeps the clip at the same offset.
    value = read_length_prefixed_string(data, marker_position + 78)
    return value[0] if value is not None and value[0] else None


def object_references(block: bytes) -> list[dict[str, str]]:
    rows = []
    seen: set[tuple[str, str]] = set()
    for match in OBJECT_REFERENCE_RE.finditer(block):
        reference = (
            match.group(1).decode("ascii", "replace"),
            match.group(2).decode("ascii", "replace"),
        )
        if reference in seen:
            continue
        seen.add(reference)
        rows.append({"className": reference[0], "objectPath": reference[1]})
    return rows


def notify_contract(source_type: str, references: list[dict[str, str]]) -> tuple[str, str, str]:
    if source_type in GAMEPLAY_REFERENCE_TYPES:
        return "gameplay_reference", "REFERENCE_ONLY", "TIMING_ONLY"
    category = PRESENTATION_CATEGORIES.get(source_type, "other")
    if category == "animation":
        return category, "PRESENTATION", "SOURCE_CLIP_EXPLICIT"
    if references:
        return category, "PRESENTATION", "SOURCE_ASSET_EXPLICIT"
    if source_type in {
        "ViewShake",
        "Trails",
        "TrailGhostEffect",
        "PawnMaterialParam",
        "PawnMaterialChange",
        "PostProcessChain",
        "AKEvent",
    }:
        return category, "PRESENTATION", "SOURCE_PARAMETERS_SERIALIZED"
    if category in {"unresolved_effect", "decal", "mesh", "particle"}:
        return category, "PRESENTATION", "UNRESOLVED_SOURCE_PAYLOAD"
    return category, "REFERENCE_ONLY", "TIMING_OR_STATE_ONLY"


def parse_stage(
    data: bytes,
    action_id: int,
    action_name: str,
    stage_index: int,
    stage_start: int,
    stage_end: int,
) -> dict[str, Any]:
    matches = list(NOTIFY_RE.finditer(data, stage_start, stage_end))
    notifies: list[dict[str, Any]] = []
    animation_clips: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []
    for notify_index, match in enumerate(matches):
        marker = match.group()
        source_type = marker[:-1].decode("ascii").removeprefix("CEFActionNotify_")
        block_end = matches[notify_index + 1].start() if notify_index + 1 < len(matches) else stage_end
        start, end, duration = read_notify_timing(data, match.start(), marker)
        references = object_references(data[match.start() : block_end])
        category, authority, resolution = notify_contract(source_type, references)
        row: dict[str, Any] = {
            "notifyId": f"action-{action_id}/stage-{stage_index:03d}/notify-{notify_index:03d}",
            "sourceType": source_type,
            "category": category,
            "authority": authority,
            "resolutionStatus": resolution,
            "localTimeSeconds": float(start),
            "sourceEndSeconds": float(end),
            "durationSeconds": float(duration),
            "sourceOffset": match.start(),
            "assetReferences": references,
        }
        strings = [
            item["value"]
            for item in scan_length_prefixed_strings(
                data, match.start() - 4, block_end
            )
            if item["value"]
            and not item["value"].startswith("CEFAction")
            and item["value"] not in {"Notify", "notify", "None", "Anim"}
        ]
        if strings:
            row["serializedLabels"] = strings
        if source_type in ANIMATION_NOTIFY_TYPES:
            if source_type == "Anim":
                clip_candidates = [read_animation_clip(data, match.start())]
            else:
                clip_candidates = [
                    value for value in strings
                    if value.casefold().startswith(
                        ("sk_", "att_", "idle_", "run_", "act_", "mode_")
                    )
                ]
            clips = []
            for clip in clip_candidates:
                if clip and clip not in clips:
                    clips.append(clip)
            row["clipNames"] = clips
            if source_type == "Anim":
                row["clipName"] = clips[0] if clips else None
            for clip in clips:
                animation_clips.append(
                    {
                        "clipName": clip,
                        "lengthSeconds": float(duration),
                        "notifyId": row["notifyId"],
                    }
                )
            if not clips:
                unresolved.append(
                    {"notifyId": row["notifyId"], "reason": "ANIMATION_CLIP_NAME_MISSING"}
                )
        elif resolution == "UNRESOLVED_SOURCE_PAYLOAD":
            unresolved.append(
                {
                    "notifyId": row["notifyId"],
                    "sourceType": source_type,
                    "reason": "NOTIFY_HAS_NO_EXPLICIT_OBJECT_REFERENCE",
                }
            )
        notifies.append(row)

    return {
        "stageIndex": stage_index,
        "stageName": read_stage_name(data, stage_start, stage_end),
        "sourceOffset": stage_start,
        "animationClips": animation_clips,
        "notifies": notifies,
        "unsupportedUnresolved": unresolved,
        "summary": {
            "animationClipCount": len(animation_clips),
            "notifyCount": len(notifies),
            "explicitAssetReferenceCount": sum(
                len(row["assetReferences"]) for row in notifies
            ),
            "unresolvedNotifyCount": len(unresolved),
        },
    }


def extract_action_document(
    source_path: Path,
    profile_id: str,
    action_min: int | None = None,
    action_max: int | None = None,
) -> dict[str, Any]:
    data = source_path.read_bytes()
    action_positions = find_all(data, ACTION_MARKER)
    actions: list[dict[str, Any]] = []
    source_systems: dict[str, dict[str, Any]] = {}
    direct_assets: dict[tuple[str, str], dict[str, Any]] = {}
    notify_counts: Counter[str] = Counter()
    category_counts: Counter[str] = Counter()
    unresolved_rows: list[dict[str, Any]] = []

    for source_action_index, action_start in enumerate(action_positions):
        action_end = (
            action_positions[source_action_index + 1]
            if source_action_index + 1 < len(action_positions)
            else len(data)
        )
        action_id, display_name = read_action_identity(data, action_start)
        if action_min is not None and action_id < action_min:
            continue
        if action_max is not None and action_id > action_max:
            continue
        stage_positions = find_all(data, STAGE_MARKER, action_start, action_end)
        stages = []
        for stage_index, stage_start in enumerate(stage_positions):
            stage_end = (
                stage_positions[stage_index + 1]
                if stage_index + 1 < len(stage_positions)
                else action_end
            )
            stage = parse_stage(
                data,
                action_id,
                display_name,
                stage_index,
                stage_start,
                stage_end,
            )
            stages.append(stage)
            unresolved_rows.extend(
                {
                    **row,
                    "actionId": action_id,
                    "actionName": display_name,
                    "stageIndex": stage_index,
                    "stageName": stage["stageName"],
                }
                for row in stage["unsupportedUnresolved"]
            )
            clip_names = [row["clipName"] for row in stage["animationClips"]]
            for notify in stage["notifies"]:
                notify_counts[notify["sourceType"]] += 1
                category_counts[notify["category"]] += 1
                for reference in notify["assetReferences"]:
                    class_name = reference["className"]
                    class_key = class_name.casefold()
                    object_path = reference["objectPath"]
                    if class_key == "particlesystem":
                        target = source_systems
                        key = object_path.casefold()
                        item = target.setdefault(
                            key,
                            {
                                "sourceAsset": object_path,
                                "actionIds": set(),
                                "actionNames": set(),
                                "stageNames": set(),
                                "clipNames": set(),
                                "notifyTypes": set(),
                                "occurrenceCount": 0,
                            },
                        )
                    else:
                        role = {
                            "staticmesh": "mesh",
                            "skeletalmesh": "mesh",
                            "material": "material",
                            "materialinstanceconstant": "material",
                            "decalmaterial": "material",
                            "texture": "texture",
                            "texture2d": "texture",
                            "texturecube": "texture",
                        }.get(class_key)
                        if role is None:
                            continue
                        key = (role, object_path.casefold())
                        item = direct_assets.setdefault(
                            key,
                            {
                                "sourceAsset": object_path,
                                "role": role,
                                "classNames": set(),
                                "actionIds": set(),
                                "actionNames": set(),
                                "stageNames": set(),
                                "clipNames": set(),
                                "notifyTypes": set(),
                                "occurrenceCount": 0,
                            },
                        )
                        item["classNames"].add(class_name)
                    item["actionIds"].add(action_id)
                    item["actionNames"].add(display_name)
                    item["stageNames"].add(stage["stageName"])
                    item["clipNames"].update(clip_names)
                    item["notifyTypes"].add(notify["sourceType"])
                    item["occurrenceCount"] += 1

        actions.append(
            {
                "sourceActionIndex": source_action_index,
                "actionId": action_id,
                "displayName": display_name,
                "sourceOffset": action_start,
                "stages": stages,
                "summary": {
                    "stageCount": len(stages),
                    "animationClipCount": sum(
                        row["summary"]["animationClipCount"] for row in stages
                    ),
                    "notifyCount": sum(row["summary"]["notifyCount"] for row in stages),
                    "unresolvedNotifyCount": sum(
                        row["summary"]["unresolvedNotifyCount"] for row in stages
                    ),
                },
            }
        )

    serialized_systems = []
    for item in sorted(source_systems.values(), key=lambda row: row["sourceAsset"].casefold()):
        serialized_systems.append(
            {
                **item,
                "actionIds": sorted(item["actionIds"]),
                "actionNames": sorted(item["actionNames"], key=str.casefold),
                "stageNames": sorted(item["stageNames"], key=str.casefold),
                "clipNames": sorted(item["clipNames"], key=str.casefold),
                "notifyTypes": sorted(item["notifyTypes"], key=str.casefold),
            }
        )

    serialized_direct_assets = []
    for item in sorted(
        direct_assets.values(),
        key=lambda row: (row["role"], row["sourceAsset"].casefold()),
    ):
        serialized_direct_assets.append(
            {
                **item,
                "classNames": sorted(item["classNames"], key=str.casefold),
                "actionIds": sorted(item["actionIds"]),
                "actionNames": sorted(item["actionNames"], key=str.casefold),
                "stageNames": sorted(item["stageNames"], key=str.casefold),
                "clipNames": sorted(item["clipNames"], key=str.casefold),
                "notifyTypes": sorted(item["notifyTypes"], key=str.casefold),
            }
        )

    selected_stage_count = sum(len(row["stages"]) for row in actions)
    selected_anim_count = sum(
        stage["summary"]["animationClipCount"]
        for action in actions
        for stage in action["stages"]
    )
    selected_notify_count = sum(notify_counts.values())
    return {
        "schema": "lostark.ue3-action-effect-source",
        "formatVersion": 1,
        "profileId": profile_id,
        "source": {
            "path": source_path.as_posix(),
            "byteSize": source_path.stat().st_size,
            "sha256": sha256_file(source_path),
            "evidenceGrade": "OFFICIAL_EXTRACTED",
        },
        "ownership": {
            "animationPose": "AnimSet/CModel",
            "presentationNotifies": "Action LOA -> Effect/Animation presentation",
            "damageAndCollision": "Server combat timeline; Action hit rows are reference only",
        },
        "actionFilter": {"minimum": action_min, "maximum": action_max},
        "actions": actions,
        "particleSystems": serialized_systems,
        "meshes": [
            row for row in serialized_direct_assets if row["role"] == "mesh"
        ],
        "materials": [
            row for row in serialized_direct_assets if row["role"] == "material"
        ],
        "textures": [
            row for row in serialized_direct_assets if row["role"] == "texture"
        ],
        "unsupportedUnresolved": unresolved_rows,
        "summary": {
            "sourceActionObjectCount": len(action_positions),
            "selectedActionObjectCount": len(actions),
            "selectedStageCount": selected_stage_count,
            "selectedAnimationClipOccurrenceCount": selected_anim_count,
            "selectedNotifyCount": selected_notify_count,
            "uniqueParticleSystemCount": len(serialized_systems),
            "uniqueDirectMeshCount": sum(
                row["role"] == "mesh" for row in serialized_direct_assets
            ),
            "uniqueDirectMaterialCount": sum(
                row["role"] == "material" for row in serialized_direct_assets
            ),
            "uniqueDirectTextureCount": sum(
                row["role"] == "texture" for row in serialized_direct_assets
            ),
            "unsupportedUnresolvedCount": len(unresolved_rows),
            "notifyTypeCounts": dict(sorted(notify_counts.items())),
            "categoryCounts": dict(sorted(category_counts.items())),
        },
    }


def parse_source(value: str) -> tuple[str, Path]:
    profile, separator, raw_path = value.partition("=")
    if not separator or not profile.strip() or not raw_path.strip():
        raise argparse.ArgumentTypeError("source must be PROFILE=PATH")
    return profile.strip(), Path(raw_path.strip())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", action="append", required=True, type=parse_source)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--action-min", type=int)
    parser.add_argument("--action-max", type=int)
    args = parser.parse_args()

    args.output.mkdir(parents=True, exist_ok=True)
    manifest_rows = []
    for profile_id, source_path in args.source:
        document = extract_action_document(
            source_path, profile_id, args.action_min, args.action_max
        )
        output_path = args.output / f"{profile_id}.action-effects.json"
        output_path.write_text(
            json.dumps(document, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        manifest_rows.append(
            {
                "profileId": profile_id,
                "output": output_path.name,
                "source": document["source"],
                "summary": document["summary"],
            }
        )
        print(json.dumps(manifest_rows[-1], ensure_ascii=False))

    manifest = {
        "schema": "lostark.ue3-action-effect-source-manifest",
        "formatVersion": 1,
        "profiles": manifest_rows,
    }
    manifest_path = args.output / "action-effect-source-manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps({"manifest": str(manifest_path), "profileCount": len(manifest_rows)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
