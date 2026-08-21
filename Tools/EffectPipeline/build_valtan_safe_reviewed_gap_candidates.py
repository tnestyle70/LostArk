#!/usr/bin/env python3
"""Build the four proof-gated, reviewed Valtan gap candidates.

This slice is deliberately separate from the broad reviewed-source generator.
It admits only two missing ordered-clip cue families and the seven trail rows
whose typed adapter packets are already renderer-ready.  Canonical authoring
documents, cues, and the Effect catalog are never written by this builder.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys
from collections import Counter
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "Tools/EffectPipeline"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_reviewed_source_family_candidates as reviewed
import build_valtan_source_occurrence_inventory as source_inventory
import build_valtan_trail_adapter_packets as trail_packets


OUTPUT_ROOT = (
    ROOT / "Data/Effects/Imported/Valtan/SafeReviewedGaps"
)
MANIFEST_PATH = OUTPUT_ROOT / "Valtan.safe-reviewed-gap-candidates.v1.json"
CUES_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
BINDINGS_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
CATALOG_PATH = ROOT / "Data/Effects/EffectCatalog.json"
TRAIL_PACKETS_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.trail-adapter-packets.v1.json"
)
WHIRLWIND_DOCUMENT_PATH = (
    ROOT / "Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json"
)

SCHEMA = "lostark.valtan-safe-reviewed-gap-candidates"
FORMAT_VERSION = 1
ADMITTED = "ADMITTED_RENDERER_READY"


class SafeGapError(RuntimeError):
    pass


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pretty_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    ).encode("utf-8")


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise SafeGapError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise SafeGapError(f"JSON root is not an object: {path}")
    return value


def seal(row: dict[str, Any], field: str) -> None:
    row.pop(field, None)
    row[field] = canonical_sha256(row)


def verify_seal(row: dict[str, Any], field: str, label: str) -> None:
    expected = row.get(field)
    clone = copy.deepcopy(row)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise SafeGapError(f"{label} {field} is stale")


def relative(path: Path) -> str:
    return path.resolve().relative_to(ROOT.resolve()).as_posix()


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    os.replace(temporary, path)


def proposed_cue(
    *,
    binding_id: str,
    occurrence_id: str,
    pattern_id: str,
    stage_id: str,
    action_id: str,
    clip_occurrence_id: str,
    effect_asset_id: str,
) -> dict[str, Any]:
    return {
        "bindingId": binding_id,
        "occurrenceId": occurrence_id,
        "patternId": pattern_id,
        "stageId": stage_id,
        "actionId": action_id,
        "clipOccurrenceId": clip_occurrence_id,
        "effectAssetId": effect_asset_id,
        "anchorSlotId": "root",
        "followPolicy": "follow",
        "stopPolicy": "natural",
        "repeatPolicy": "once",
        "sourceStartMs": 0,
        "sourceEndMs": None,
        "localTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }


SLICE_SPECS: tuple[dict[str, Any], ...] = (
    {
        "sliceId": "swing.active.clip-02",
        "patternId": "VALTAN_SWING",
        "stageId": "SWEEP",
        "actionId": "valtan.attack.swing.active",
        "clipOccurrenceId": "valtan.attack.swing.active.clip.02",
        "clip": "mesh_att_battle_1_02",
        "effectAssetId": "effect.valtan.swing.active.clip-02",
        "displayName": "VALTAN_SWING / SWEEP / CLIP 02",
        "coreCount": 141,
        "trailCount": 0,
    },
    {
        "sliceId": "four-slash.active.clip-02",
        "patternId": "VALTAN_FOUR_SLASH",
        "stageId": "SLASHES",
        "actionId": "valtan.attack.four-slash.active",
        "clipOccurrenceId": "valtan.attack.four-slash.active.clip.02",
        "clip": "mesh_att_battle_10_02",
        "effectAssetId": "effect.valtan.four-slash.active.clip-02",
        "displayName": "VALTAN_FOUR_SLASH / SLASHES / CLIP 02",
        "coreCount": 19,
        "trailCount": 1,
    },
    {
        "sliceId": "backstep.windup.trails",
        "patternId": "VALTAN_BACKSTEP_ATTACK",
        "stageId": "WINDUP",
        "actionId": "valtan.attack.backstep.windup",
        "clipOccurrenceId": "valtan.attack.backstep.windup.clip.01",
        "clip": "mesh_att_battle_20_03",
        "effectAssetId": "effect.valtan.backstep.windup.trails",
        "displayName": "VALTAN_BACKSTEP_ATTACK / WINDUP / SOURCE TRAILS",
        "coreCount": 0,
        "trailCount": 3,
    },
    {
        "sliceId": "jump-spin.spin.trails",
        "patternId": "VALTAN_JUMP_SPIN",
        "stageId": "SPIN",
        "actionId": "valtan.attack.jump-spin.spin",
        "clipOccurrenceId": "valtan.attack.jump-spin.spin.clip.01",
        "clip": "mesh_att_battle_20_03",
        "effectAssetId": "effect.valtan.jump-spin.spin.trails",
        "displayName": "VALTAN_JUMP_SPIN / SPIN / SOURCE TRAILS",
        "coreCount": 0,
        "trailCount": 3,
    },
)


def _cue_for_spec(spec: dict[str, Any]) -> dict[str, Any]:
    suffix = str(spec["sliceId"])
    return proposed_cue(
        binding_id=f"cue.valtan.{suffix}",
        occurrence_id=f"cue.valtan.{suffix}.occurrence.01",
        pattern_id=str(spec["patternId"]),
        stage_id=str(spec["stageId"]),
        action_id=str(spec["actionId"]),
        clip_occurrence_id=str(spec["clipOccurrenceId"]),
        effect_asset_id=str(spec["effectAssetId"]),
    )


def _catalog_row(spec: dict[str, Any]) -> dict[str, Any]:
    name = f"{spec['effectAssetId']}.effect.json"
    return {
        "effectAssetId": str(spec["effectAssetId"]),
        "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
        "authoringPath": f"Effects/Authored/{name}",
    }


def _binding_clip_index(bindings: dict[str, Any]) -> dict[str, tuple[str, str]]:
    if bindings.get("formatVersion") != 2:
        raise SafeGapError("Valtan pattern bindings are not formatVersion 2")
    result: dict[str, tuple[str, str]] = {}
    for binding in bindings.get("bindings", []):
        action_id = str(binding.get("actionId") or "")
        for clip in binding.get("clips", []):
            clip_id = str(clip.get("clipOccurrenceId") or "")
            if not clip_id or clip_id in result:
                raise SafeGapError("pattern binding clip identity is invalid")
            result[clip_id] = (action_id, str(clip.get("clip") or ""))
    return result


def _core_elements(
    inventory: dict[str, Any],
    spec: dict[str, Any],
    cue: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    if int(spec["coreCount"]) == 0:
        return [], []
    systems = {
        str(row.get("sourceSystemId") or ""): row
        for row in inventory.get("sourceSystems", [])
        if isinstance(row, dict)
    }
    elements: list[dict[str, Any]] = []
    projections: list[dict[str, Any]] = []
    for occurrence in inventory.get("occurrences", []):
        if (
            not isinstance(occurrence, dict)
            or occurrence.get("clipOccurrenceId") != spec["clipOccurrenceId"]
            or occurrence.get("reachabilityDisposition") != "REACHABLE_REVIEWED"
            or occurrence.get("disposition") != "EXECUTABLE_CORE"
        ):
            continue
        system = systems.get(str(occurrence.get("sourceSystemId") or ""))
        if system is None:
            raise SafeGapError("core occurrence lost its source system")
        for carrier in system.get("carriers", []):
            if not isinstance(carrier, dict) or carrier.get("disposition") != "EXECUTABLE_CORE":
                continue
            element, projection = reviewed.build_projection_seed(
                occurrence, carrier, cue
            )
            if element is None or projection.get("disposition") != "ADMITTED":
                raise SafeGapError(
                    f"safe core projection was not admitted: {spec['sliceId']}"
                )
            elements.append(element)
            projections.append(projection)
    elements.sort(key=lambda row: str(row.get("sourceNode") or ""))
    projections.sort(key=lambda row: (str(row["sourceNode"]), str(row["id"])))
    if len(elements) != int(spec["coreCount"]):
        raise SafeGapError(
            f"{spec['sliceId']} core denominator changed: {len(elements)}"
        )
    return elements, projections


def _adapter_rows(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    trail_packets.validate_document(document, ROOT)
    result: dict[str, dict[str, Any]] = {}
    for row in document.get("adapters", []):
        adapter_id = str(row.get("adapterTargetId") or "")
        if not adapter_id or adapter_id in result:
            raise SafeGapError("trail adapter identity is empty or duplicated")
        result[adapter_id] = row
    return result


def _selected_adapters(
    all_rows: dict[str, dict[str, Any]], spec: dict[str, Any]
) -> list[dict[str, Any]]:
    family = (
        trail_packets.FAMILY_CASCADE_RIBBON
        if spec["sliceId"] == "four-slash.active.clip-02"
        else trail_packets.FAMILY_ANIMATION_TRAIL
    )
    rows = [
        row
        for row in all_rows.values()
        if row.get("disposition") == ADMITTED
        and row.get("family") == family
        and (row.get("sourceIdentity") or {}).get("patternId") == spec["patternId"]
        and (row.get("sourceIdentity") or {}).get("clipOccurrenceId")
        == spec["clipOccurrenceId"]
    ]
    rows.sort(key=lambda row: str(row["adapterTargetId"]))
    if len(rows) != int(spec["trailCount"]):
        raise SafeGapError(
            f"{spec['sliceId']} trail denominator changed: {len(rows)}"
        )
    return rows


def _resource_pairs_from_packet(row: dict[str, Any]) -> list[dict[str, str]]:
    result = [
        {"slotId": str(item["slotId"]), "assetId": str(item["assetId"])}
        for item in row.get("resourcePacket", [])
    ]
    return sorted(result, key=lambda item: (item["slotId"], item["assetId"]))


def _animation_trail_element(
    row: dict[str, Any],
    spec: dict[str, Any],
    whirlwind_targets: dict[str, dict[str, Any]],
) -> tuple[dict[str, Any], dict[str, Any]]:
    packet = row.get("packet") or {}
    target_contract = row.get("target") or {}
    source_target_id = str(packet.get("targetElementId") or "")
    source_target = whirlwind_targets.get(source_target_id)
    if source_target is None or source_target.get("kind") != "trail":
        raise SafeGapError("AnimationTrail packet lost its exact source target")
    source_resources = sorted(
        copy.deepcopy(source_target.get("resources") or []),
        key=lambda item: (str(item.get("slotId") or ""), str(item.get("assetId") or "")),
    )
    if _resource_pairs_from_packet(row) != source_resources:
        raise SafeGapError("AnimationTrail source target resources changed")
    detail = source_target.get("detail") or {}
    source_contract = {
        "kind": source_target.get("kind"),
        "targetTiming": copy.deepcopy(detail.get("timing") or {}),
        "attachment": copy.deepcopy(source_target.get("actionCueAttachment") or {}),
        "trail": copy.deepcopy(detail.get("trail") or {}),
    }
    normalized = copy.deepcopy(source_contract)
    normalized["targetTiming"] = copy.deepcopy(target_contract.get("targetTiming") or {})
    normalized["trail"] = copy.deepcopy(target_contract.get("trail") or {})
    if (
        target_contract.get("kind") != "trail"
        or normalized != {
            "kind": target_contract.get("kind"),
            "targetTiming": target_contract.get("targetTiming"),
            "attachment": target_contract.get("attachment"),
            "trail": target_contract.get("trail"),
        }
    ):
        raise SafeGapError("AnimationTrail packet/target contract diverged")
    identity = row["sourceIdentity"]
    target_id = "safe-gap.trail." + canonical_sha256(
        {
            "effectAssetId": spec["effectAssetId"],
            "adapterTargetId": row["adapterTargetId"],
        }
    )[:24]
    element = copy.deepcopy(source_target)
    element["id"] = target_id
    element["displayName"] = (
        f"{spec['sliceId']} | {row['adapterTargetId'][-12:]} | AnimationTrail"
    )[:64]
    element["groupId"] = f"valtan.safe-gap.{spec['sliceId']}"
    element["sourceNode"] = "valtan.safe-gap.adapter." + canonical_sha256(
        {
            "sourceIdentity": row["sourceIdentity"],
            "adapterTargetId": row["adapterTargetId"],
        }
    )
    element["visible"] = True
    element["actionCueAttachment"] = copy.deepcopy(target_contract["attachment"])
    element["transformInheritance"] = {"enabled": False, "masterElementId": ""}
    element["detail"]["timing"] = copy.deepcopy(target_contract["targetTiming"])
    element["detail"]["trail"] = copy.deepcopy(target_contract["trail"])
    presentation = element.setdefault("sourcePresentation", {})
    presentation.update(
        {
            "enabled": True,
            "schema": "lostark.effect-source-presentation",
            "version": 1,
            "profileId": "valtan.safe-reviewed-gap.animation-trail.v1",
            "status": "reconstructed",
            "sourceActionCueId": spec["actionId"],
            "sourceEventId": identity["notifyId"],
            "sourceTimeSeconds": identity["sourceTimeSeconds"],
        }
    )
    projection = {
        "projectionId": "safe-gap.adapter-projection." + canonical_sha256(
            {"adapterTargetId": row["adapterTargetId"], "targetElementId": target_id}
        ),
        "family": row["family"],
        "sourceAdapterTargetId": row["adapterTargetId"],
        "sourceAdapterRowSha256": row["rowSha256"],
        "sourcePacketSha256": packet["packetSha256"],
        "sourceIdentitySha256": row["sourceIdentitySha256"],
        "sourceTargetElementId": source_target_id,
        "effectAssetId": spec["effectAssetId"],
        "targetElementId": target_id,
        "targetContractSha256": canonical_sha256(
            {
                "resources": element["resources"],
                "material": element["material"],
                "actionCueAttachment": element["actionCueAttachment"],
                "timing": element["detail"]["timing"],
                "trail": element["detail"]["trail"],
                "sourceRecipe": element["sourceRecipe"],
            }
        ),
        "clip": spec["clip"],
        "stageId": spec["stageId"],
        "actionId": spec["actionId"],
    }
    seal(projection, "projectionSha256")
    return element, projection


def _ribbon_element(
    row: dict[str, Any],
    spec: dict[str, Any],
    template: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    packet = row.get("packet") or {}
    target = row.get("target") or {}
    if (
        packet.get("packetLayout") != "CASCADE_RIBBON_TYPED_PACKET_V1"
        or target.get("kind") != "trail"
        or packet.get("sourceCarrierKey")
        != (row.get("sourceIdentity") or {}).get("carrierKey")
    ):
        raise SafeGapError("FourSlash ribbon packet/target identity changed")
    target_id = "safe-gap.trail." + canonical_sha256(
        {
            "effectAssetId": spec["effectAssetId"],
            "adapterTargetId": row["adapterTargetId"],
        }
    )[:24]
    element = copy.deepcopy(template)
    element["id"] = target_id
    element["displayName"] = (
        f"{spec['sliceId']} | {row['adapterTargetId'][-12:]} | ribbon"
    )[:64]
    element["groupId"] = f"valtan.safe-gap.{spec['sliceId']}"
    element["sourceNode"] = "valtan.safe-gap.adapter." + canonical_sha256(
        {
            "sourceIdentity": row["sourceIdentity"],
            "adapterTargetId": row["adapterTargetId"],
        }
    )
    element["visible"] = True
    element["kind"] = "trail"
    element["resources"] = copy.deepcopy(target["resources"])
    element["material"] = copy.deepcopy(target["material"])
    element["actionCueAttachment"] = copy.deepcopy(target["attachment"])
    element["transformInheritance"] = {"enabled": False, "masterElementId": ""}
    element["detail"]["timing"] = copy.deepcopy(target["targetTiming"])
    element["detail"]["trail"] = copy.deepcopy(target["trail"])
    element["sourceRecipe"] = copy.deepcopy(target["sourceRecipe"])
    identity = row["sourceIdentity"]
    element["sourcePresentation"] = {
        "enabled": True,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": "valtan.safe-reviewed-gap.cascade-ribbon.v1",
        "status": "reconstructed",
        "sourceObjectPath": identity["assetObjectPath"],
        "sourceActionCueId": spec["actionId"],
        "sourceEventId": identity["notifyId"],
        "sourceOccurrenceIndex": 0,
        "sourceTimeSeconds": identity["sourceTimeSeconds"],
        "parameters": [],
    }
    projection = {
        "projectionId": "safe-gap.adapter-projection." + canonical_sha256(
            {"adapterTargetId": row["adapterTargetId"], "targetElementId": target_id}
        ),
        "family": row["family"],
        "sourceAdapterTargetId": row["adapterTargetId"],
        "sourceAdapterRowSha256": row["rowSha256"],
        "sourcePacketSha256": packet["packetSha256"],
        "sourceIdentitySha256": row["sourceIdentitySha256"],
        "sourceTargetElementId": target["id"],
        "effectAssetId": spec["effectAssetId"],
        "targetElementId": target_id,
        "targetContractSha256": canonical_sha256(
            {
                "resources": element["resources"],
                "material": element["material"],
                "actionCueAttachment": element["actionCueAttachment"],
                "timing": element["detail"]["timing"],
                "trail": element["detail"]["trail"],
                "sourceRecipe": element["sourceRecipe"],
            }
        ),
        "clip": spec["clip"],
        "stageId": spec["stageId"],
        "actionId": spec["actionId"],
    }
    seal(projection, "projectionSha256")
    return element, projection


def _document(spec: dict[str, Any], elements: list[dict[str, Any]]) -> dict[str, Any]:
    result = {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": spec["effectAssetId"],
        "displayName": spec["displayName"],
        "particleSystem": {
            "uniformScaleMultiplier": 1,
            "yawOffsetDegrees": 0,
            "directionYawDegrees": 0,
            "initialSpeedMultiplier": 1,
        },
        "modelCues": [],
        "elements": sorted(
            elements, key=lambda row: (str(row.get("sourceNode") or ""), str(row.get("id") or ""))
        ),
    }
    ids = [str(row.get("id") or "") for row in result["elements"]]
    sources = [str(row.get("sourceNode") or "") for row in result["elements"]]
    if (
        not ids
        or len(ids) != len(set(ids))
        or len(sources) != len(set(sources))
        or any(not value for value in ids + sources)
    ):
        raise SafeGapError(f"candidate element identity invalid: {spec['sliceId']}")
    return result


def validate_manifest(document: dict[str, Any], output_root: Path = OUTPUT_ROOT) -> None:
    if (
        document.get("schema") != SCHEMA
        or document.get("formatVersion") != FORMAT_VERSION
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise SafeGapError("safe-gap manifest header is invalid")
    verify_seal(document, "artifactSha256", "safe-gap manifest")
    candidates = document.get("candidateDocuments")
    if not isinstance(candidates, list) or len(candidates) != 4:
        raise SafeGapError("safe-gap candidate denominator changed")
    total = 0
    ids: set[str] = set()
    for row in candidates:
        if not isinstance(row, dict):
            raise SafeGapError("safe-gap candidate row is not an object")
        effect_id = str(row.get("effectAssetId") or "")
        if not effect_id or effect_id in ids:
            raise SafeGapError("safe-gap effect identity duplicated")
        ids.add(effect_id)
        path = ROOT / str(row.get("candidatePath") or "")
        if path.parent.resolve() != output_root.resolve():
            raise SafeGapError("safe-gap candidate escaped output root")
        candidate = read_json(path)
        if (
            candidate.get("effectAssetId") != effect_id
            or raw_sha256(path) != row.get("rawSha256")
            or canonical_sha256(candidate) != row.get("canonicalSha256")
            or len(candidate.get("elements", [])) != row.get("elementCount")
        ):
            raise SafeGapError(f"safe-gap candidate identity stale: {effect_id}")
        total += int(row["elementCount"])
    if total != 167 or (document.get("summary") or {}).get("elementCount") != 167:
        raise SafeGapError("safe-gap 167-element denominator changed")
    projections = document.get("adapterProjections")
    if not isinstance(projections, list) or len(projections) != 7:
        raise SafeGapError("safe-gap adapter projection denominator changed")
    for row in projections:
        verify_seal(row, "projectionSha256", "adapter projection")
    unresolved = document.get("preservedUnresolvedTrailRows")
    if not isinstance(unresolved, list) or len(unresolved) != 6:
        raise SafeGapError("safe-gap unresolved trail denominator changed")


def build(output_root: Path = OUTPUT_ROOT) -> tuple[dict[str, Any], dict[Path, bytes]]:
    output_root = output_root.resolve()
    if output_root != OUTPUT_ROOT.resolve():
        if ROOT.resolve() not in output_root.parents:
            raise SafeGapError("custom output root escaped repository")
    cues = read_json(CUES_PATH)
    bindings = read_json(BINDINGS_PATH)
    catalog = read_json(CATALOG_PATH)
    adapter_document = read_json(TRAIL_PACKETS_PATH)
    inventory = reviewed.load_inventory(reviewed.SELECTION_PATH)
    canary = trail_packets.load_whirlwind_canary(ROOT)
    binding_clips = _binding_clip_index(bindings)
    current_cues = reviewed.cue_index(cues)
    current_catalog = reviewed.catalog_index(catalog)
    all_adapters = _adapter_rows(adapter_document)
    whirlwind = read_json(WHIRLWIND_DOCUMENT_PATH)
    whirlwind_targets = {
        str(row.get("id") or ""): row
        for row in whirlwind.get("elements", [])
        if isinstance(row, dict)
    }
    trail_template = whirlwind_targets.get(trail_packets.WHIRLWIND_TARGET_IDS[0])
    if trail_template is None:
        raise SafeGapError("Whirlwind trail template changed")

    payloads: dict[Path, bytes] = {}
    candidate_rows: list[dict[str, Any]] = []
    all_core_projections: list[dict[str, Any]] = []
    all_adapter_projections: list[dict[str, Any]] = []
    proposed_cues: list[dict[str, Any]] = []
    proposed_catalog_rows: list[dict[str, Any]] = []
    admitted_adapter_ids: set[str] = set()
    for spec in SLICE_SPECS:
        clip = binding_clips.get(str(spec["clipOccurrenceId"]))
        if clip != (spec["actionId"], spec["clip"]):
            raise SafeGapError(f"binding identity changed: {spec['sliceId']}")
        if str(spec["clipOccurrenceId"]) in current_cues and any(
            row.get("effectAssetId") == spec["effectAssetId"]
            for row in current_cues[str(spec["clipOccurrenceId"])]
        ):
            raise SafeGapError(f"safe-gap cue already exists: {spec['sliceId']}")
        if spec["effectAssetId"] in current_catalog:
            raise SafeGapError(f"safe-gap catalog ID already exists: {spec['effectAssetId']}")
        cue = _cue_for_spec(spec)
        catalog_row = _catalog_row(spec)
        core_elements, core_projections = _core_elements(inventory, spec, cue)
        elements = list(core_elements)
        adapter_projections: list[dict[str, Any]] = []
        for adapter in _selected_adapters(all_adapters, spec):
            admitted_adapter_ids.add(str(adapter["adapterTargetId"]))
            if adapter["family"] == trail_packets.FAMILY_ANIMATION_TRAIL:
                element, projection = _animation_trail_element(
                    adapter, spec, whirlwind_targets
                )
            else:
                element, projection = _ribbon_element(
                    adapter, spec, trail_template
                )
            elements.append(element)
            adapter_projections.append(projection)
        expected = int(spec["coreCount"]) + int(spec["trailCount"])
        if len(elements) != expected:
            raise SafeGapError(f"slice denominator changed: {spec['sliceId']}")
        candidate = _document(spec, elements)
        candidate_path = output_root / f"{spec['effectAssetId']}.safe-gap-candidate.effect.json"
        payload = pretty_bytes(candidate)
        payloads[candidate_path] = payload
        canonical_path = ROOT / "Data" / catalog_row["authoringPath"]
        candidate_rows.append(
            {
                "sliceId": spec["sliceId"],
                "patternId": spec["patternId"],
                "stageId": spec["stageId"],
                "actionId": spec["actionId"],
                "clipOccurrenceId": spec["clipOccurrenceId"],
                "clip": spec["clip"],
                "effectAssetId": spec["effectAssetId"],
                "candidatePath": relative(candidate_path),
                "canonicalPath": relative(canonical_path),
                "rawSha256": hashlib.sha256(payload).hexdigest(),
                "canonicalSha256": canonical_sha256(candidate),
                "elementCount": len(elements),
                "coreProjectionCount": len(core_elements),
                "trailProjectionCount": len(adapter_projections),
                "elementIds": [str(row["id"]) for row in candidate["elements"]],
                "cue": cue,
                "catalogRow": catalog_row,
            }
        )
        proposed_cues.append(cue)
        proposed_catalog_rows.append(catalog_row)
        all_core_projections.extend(core_projections)
        all_adapter_projections.extend(adapter_projections)

    unresolved = []
    for row in all_adapters.values():
        identity = row.get("sourceIdentity") or {}
        if identity.get("patternId") not in {
            "VALTAN_SWING",
            "VALTAN_FOUR_SLASH",
            "VALTAN_BACKSTEP_ATTACK",
            "VALTAN_JUMP_SPIN",
        }:
            continue
        if str(row.get("adapterTargetId") or "") in admitted_adapter_ids:
            continue
        if row.get("disposition") == ADMITTED:
            continue
        unresolved.append(
            {
                "adapterTargetId": row["adapterTargetId"],
                "family": row["family"],
                "patternId": identity.get("patternId"),
                "clipOccurrenceId": identity.get("clipOccurrenceId"),
                "disposition": row["disposition"],
                "admissionBlockers": copy.deepcopy(row.get("admissionBlockers") or []),
                "rowSha256": row["rowSha256"],
                "applicationDisposition": "PRESERVE_UNRESOLVED_NO_MUTATION",
            }
        )
    unresolved.sort(key=lambda row: str(row["adapterTargetId"]))

    candidate_rows.sort(key=lambda row: str(row["effectAssetId"]))
    all_adapter_projections.sort(key=lambda row: str(row["projectionId"]))
    manifest = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "policy": (
            "FOUR_EXACT_REVIEWED_SLICES_ONLY; MISSING_CLIP_CUES_ARE_DISTINCT; "
            "TRAIL_PACKET_TARGET_JOIN_REQUIRED; NO_UNRESOLVED_ADMISSION"
        ),
        "inputIdentity": {
            "cuePath": relative(CUES_PATH),
            "cueRawSha256": raw_sha256(CUES_PATH),
            "cueCanonicalSha256": canonical_sha256(cues),
            "cueCount": len(cues.get("cues", [])),
            "bindingPath": relative(BINDINGS_PATH),
            "bindingRawSha256": raw_sha256(BINDINGS_PATH),
            "bindingCanonicalSha256": canonical_sha256(bindings),
            "catalogPath": relative(CATALOG_PATH),
            "catalogRawSha256": raw_sha256(CATALOG_PATH),
            "catalogCanonicalSha256": canonical_sha256(catalog),
            "catalogCount": len(catalog.get("effects", [])),
            "inventoryCanonicalSha256": canonical_sha256(inventory),
            "trailAdapterPath": relative(TRAIL_PACKETS_PATH),
            "trailAdapterRawSha256": raw_sha256(TRAIL_PACKETS_PATH),
            "trailAdapterCanonicalSha256": canonical_sha256(adapter_document),
            "trailAdapterArtifactSha256": adapter_document.get("artifactSha256"),
            "whirlwindCanary": canary,
        },
        "candidateDocuments": candidate_rows,
        "coreProjections": all_core_projections,
        "adapterProjections": all_adapter_projections,
        "preservedUnresolvedTrailRows": unresolved,
        "proposedCueRows": sorted(proposed_cues, key=lambda row: row["bindingId"]),
        "proposedCatalogRows": sorted(
            proposed_catalog_rows, key=lambda row: row["effectAssetId"]
        ),
        "summary": {
            "candidateDocumentCount": 4,
            "elementCount": sum(row["elementCount"] for row in candidate_rows),
            "coreProjectionCount": len(all_core_projections),
            "adapterProjectionCount": len(all_adapter_projections),
            "familyCounts": dict(
                sorted(
                    Counter(
                        row["family"] for row in all_adapter_projections
                    ).items()
                )
            ),
            "preservedUnresolvedTrailRowCount": len(unresolved),
            "proposedCueCount": len(proposed_cues),
            "proposedCatalogCount": len(proposed_catalog_rows),
        },
    }
    seal(manifest, "artifactSha256")
    payloads[output_root / MANIFEST_PATH.name] = pretty_bytes(manifest)
    return manifest, payloads


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--dry-run", action="store_true")
    parser.add_argument("--output-root", type=Path, default=OUTPUT_ROOT)
    args = parser.parse_args()

    if args.check:
        manifest_path = args.output_root.resolve() / MANIFEST_PATH.name
        if not manifest_path.is_file():
            raise SafeGapError(f"safe-gap manifest is missing: {manifest_path}")
        manifest = read_json(manifest_path)
        validate_manifest(manifest, args.output_root)
        print(
            "Valtan safe reviewed gap candidates checked: "
            + json.dumps(manifest["summary"], sort_keys=True)
        )
        return 0

    manifest, payloads = build(args.output_root)
    if args.write:
        for path, payload in sorted(payloads.items(), key=lambda item: str(item[0])):
            write_atomic(path, payload)
        validate_manifest(manifest, args.output_root)
        label = "written"
    else:
        label = "dry-run"
    print(
        "Valtan safe reviewed gap candidates "
        + label
        + ": "
        + json.dumps(manifest["summary"], sort_keys=True)
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (SafeGapError, reviewed.CandidateError, trail_packets.AdapterError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
