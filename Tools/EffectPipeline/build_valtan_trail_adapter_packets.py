#!/usr/bin/env python3
"""Build reviewed Valtan typed trail adapter packets without source invention.

The source-occurrence inventory is intentionally an evidence inventory.  This
builder consumes only reviewed/reachable source stage coordinates and projects
three source families onto the existing Effect renderer carriers:

* EFData AnimationTrail -> baked-edge AnimationTrail carrier,
* TrailGhost -> the same baked-edge carrier when exact edge history exists,
* Cascade TypeDataRibbon -> the existing CascadeRibbon carrier.

Missing geometry, typed literals, or runtime resources are retained as
``UNRESOLVED_RUNTIME_ADAPTER`` rows.  They are never silently dropped and they
are never replaced by a source-only clip or an invented visual.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_INVENTORY = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.source-occurrence-inventory.v1.json"
)
DEFAULT_SELECTIONS = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.priority-source-sequence-selections.v1.json"
)
DEFAULT_OUTPUT = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.trail-adapter-packets.v1.json"
)
SOURCE_CATALOG = (
    ROOT
    / "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
WHIRLWIND_HISTORY = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.420633.whirlwind-baked-edge-history.v1.json"
)
WHIRLWIND_EFFECT = (
    ROOT
    / "Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json"
)
VISUAL_PROGRAM_CORPUS = (
    ROOT
    / "Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json"
)
VISUAL_PROGRAM_RUNTIME = (
    ROOT
    / "Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
)

SCHEMA = "lostark.valtan-trail-adapter-packets"
FORMAT_VERSION = 1
WHIRLWIND_HISTORY_ID = "valtan.420633.animnotify-trails-479.baked-edges"
WHIRLWIND_HISTORY_ARTIFACT_SHA256 = (
    "46bf2e83ace7d798a2ff34489cc4eb223a716ac75159d799f6dd306707112a64"
)
WHIRLWIND_HISTORY_RAW_SHA256 = (
    "34f0d15e31afdca31887828961c8de450ebc027d4dcf54ff0c0d9312df3fd64e"
)
WHIRLWIND_EFFECT_RAW_SHA256 = (
    "e94ddfd463da840962ddf8d8a0b604af4ef833925c2ac1cfd0f8e25522a5bea7"
)
WHIRLWIND_EFFECT_CANONICAL_SHA256 = (
    "95c233b9fb9ded959a5527381f5ae0bd1a85b8c97c5a0a6b8341f92364ecd68a"
)
WHIRLWIND_EFFECT_SOURCE_RAW_SHA256 = (
    "ac0c4464fd1fc05dcb0b107e796c015b1ca4c76dcf9beb06023b5cf5d890bf94"
)
WHIRLWIND_RUNTIME_PROGRAM_SHA256 = (
    "3a3c3a42dc9423303c458fa90cdea8ba719c74853aba59d7d942f7b2b17b6fab"
)
WHIRLWIND_PROJECTED_DOCUMENT_SHA256 = (
    "23cf5425d5c7f080a14263ef2613e715b1df206892b61ffb371fc1248988b483"
)
WHIRLWIND_PROJECTED_TYPED_CODEC_SHA256 = (
    "bdb2178b309901f360a77739bb3ec7219926bcbdd8b9e4e9a5b2b6b0911c1b02"
)
WHIRLWIND_SUPPLEMENTAL_ROW_SHA256S = {
    "37b0bd1d943b630616745f7efb9004913e07a7d93529c2ab5c555eb8472c9eb6",
    "981ff0495ea692899a04a72172a02a117ad8d3c8a58596ebee03588a3c60d790",
    "ac3b368edac4ef0c26e3e019378d9379903e8d73610813c6d9bc5a66c41d129c",
    "452d705f221e498a477b168b918acfa93b8bf61e06534e7d36734be8b30288a4",
}
WHIRLWIND_OUTER_OBJECT = "mn_rpbf_00_420621_0_3_0"
WHIRLWIND_TARGET_IDS = (
    "valtan.420633.notify004.emitter5259",
    "valtan.420633.notify004.emitter5260",
    "valtan.420633.notify004.emitter5258",
)

FAMILY_ANIMATION_TRAIL = "ANIMATION_TRAIL"
FAMILY_TRAIL_GHOST = "TRAIL_GHOST"
FAMILY_CASCADE_RIBBON = "CASCADE_RIBBON"
ADMITTED = "ADMITTED_RENDERER_READY"
UNRESOLVED = "UNRESOLVED_RUNTIME_ADAPTER"


class AdapterError(RuntimeError):
    """Raised when an immutable input or generated contract is invalid."""


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


def inventory_canonical_sha256(value: Any) -> str:
    """Match the source inventory's canonical hash, including its newline."""

    return hashlib.sha256(canonical_bytes(value) + b"\n").hexdigest()


def pretty_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    ).encode("utf-8")


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise AdapterError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise AdapterError(f"JSON root is not an object: {path}")
    return value


def resolve_runtime_resources_root(repository_root: Path) -> Path:
    """Locate ignored runtime inputs for normal and linked Git worktrees."""

    direct = (repository_root / "Client/Bin/Resources").resolve()
    if direct.is_dir():
        return direct
    configured = os.environ.get("LOSTARK_RUNTIME_RESOURCES_ROOT")
    if configured:
        candidate = Path(configured).resolve()
        if candidate.is_dir():
            return candidate
    try:
        result = subprocess.run(
            [
                "git",
                "-C",
                str(repository_root),
                "rev-parse",
                "--git-common-dir",
            ],
            check=True,
            capture_output=True,
            text=True,
            timeout=5,
        )
        common = Path(result.stdout.strip())
        if not common.is_absolute():
            common = repository_root / common
        linked = (common.resolve().parent / "Client/Bin/Resources").resolve()
        if linked.is_dir():
            return linked
    except (OSError, subprocess.SubprocessError):
        pass
    return direct


def seal(row: dict[str, Any], field: str) -> None:
    row.pop(field, None)
    row[field] = canonical_sha256(row)


def verify_seal(row: dict[str, Any], field: str, label: str) -> None:
    expected = row.get(field)
    clone = copy.deepcopy(row)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise AdapterError(f"{label} {field} is stale")


def finite(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise AdapterError(f"{label} is not numeric")
    result = float(value)
    if not math.isfinite(result):
        raise AdapterError(f"{label} is not finite")
    return result


def stable_source_identity(
    occurrence: dict[str, Any],
    clip_occurrence_id: str,
    *,
    asset_reference: dict[str, Any] | None = None,
    carrier_key: str | None = None,
) -> dict[str, Any]:
    asset = asset_reference or occurrence.get("assetReference") or {}
    return {
        "patternId": str(occurrence.get("patternId") or ""),
        "clipOccurrenceId": clip_occurrence_id,
        "sourceActionId": int(occurrence.get("sourceActionId")),
        "profileId": str(occurrence.get("profileId") or ""),
        "sourceStageIndex": int(occurrence.get("sourceStageIndex")),
        "sourceClipOrdinal": int(occurrence.get("sourceClipOrdinal")),
        "notifyId": str(occurrence.get("notifyId") or ""),
        "sourceType": str(occurrence.get("sourceType") or ""),
        "sourceTimeSeconds": finite(
            occurrence.get("sourceTimeSeconds", 0.0), "source time"
        ),
        "sourceDurationSeconds": finite(
            occurrence.get("sourceDurationSeconds", 0.0), "source duration"
        ),
        "assetClassName": str(asset.get("className") or ""),
        "assetObjectPath": str(asset.get("objectPath") or ""),
        "carrierKey": carrier_key,
    }


def _branch_index(inventory: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in inventory.get("branches", []):
        if not isinstance(row, dict):
            continue
        branch_id = str(row.get("branchId") or "")
        if branch_id:
            result[branch_id] = row
    return result


def reviewed_mapping_index(
    selections: dict[str, Any] | None,
) -> dict[tuple[str, int, str, int, int], dict[str, Any]]:
    """Index selected coordinates while accepting additive manifest fields."""

    if selections is None:
        return {}
    if (
        selections.get("schema") != "lostark.valtan-source-branch-selections"
        or selections.get("formatVersion") != 1
        or not isinstance(selections.get("selections"), list)
    ):
        raise AdapterError("reviewed selection manifest header is invalid")
    result: dict[tuple[str, int, str, int, int], dict[str, Any]] = {}
    for selection in selections["selections"]:
        if not isinstance(selection, dict):
            raise AdapterError("reviewed selection row is not an object")
        if selection.get("status") != "REVIEWED_SELECTED":
            continue
        group = (
            str(selection.get("patternId") or ""),
            int(selection.get("sourceActionId")),
            str(selection.get("profileId") or ""),
        )
        for mapping in selection.get("stageMappings", []):
            if not isinstance(mapping, dict):
                raise AdapterError("reviewed stage mapping is not an object")
            coordinate = group + (
                int(mapping.get("sourceStageIndex")),
                int(mapping.get("sourceClipOrdinal")),
            )
            evidence = {
                "selectedBranchId": str(selection.get("branchId") or ""),
                "sequenceIndex": selection.get("sequenceIndex"),
                "sourceSequencePathSha256": selection.get(
                    "sourceSequencePathSha256"
                ),
                "timingDisposition": str(
                    mapping.get("timingDisposition") or ""
                ),
                "clipOccurrenceId": mapping.get("clipOccurrenceId"),
                "reviewBasis": mapping.get("reviewBasis")
                or selection.get("reviewBasis"),
            }
            previous = result.get(coordinate)
            if previous is not None and previous != evidence:
                raise AdapterError(
                    "two reviewed selections own one source coordinate: "
                    + repr(coordinate)
                )
            result[coordinate] = evidence
    return result


def reviewed_reachable_occurrences(
    inventory: dict[str, Any],
    selections: dict[str, Any] | None,
) -> list[dict[str, Any]]:
    """Return only product-reachable occurrences; never infer from clip name."""

    if (
        inventory.get("schema")
        != "lostark.valtan-source-occurrence-inventory"
        or inventory.get("formatVersion") != 1
        or not isinstance(inventory.get("occurrences"), list)
    ):
        raise AdapterError("Valtan source occurrence inventory header is invalid")
    external = reviewed_mapping_index(selections)
    branch_by_id = _branch_index(inventory)
    rows: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()
    for occurrence in inventory["occurrences"]:
        if not isinstance(occurrence, dict):
            raise AdapterError("source occurrence is not an object")
        ordinal = occurrence.get("sourceClipOrdinal")
        if ordinal is None:
            continue
        coordinate = (
            str(occurrence.get("patternId") or ""),
            int(occurrence.get("sourceActionId")),
            str(occurrence.get("profileId") or ""),
            int(occurrence.get("sourceStageIndex")),
            int(ordinal),
        )
        mapping = external.get(coordinate)
        if mapping is not None:
            if (
                mapping["timingDisposition"] != "REACHABLE"
                or not isinstance(mapping.get("clipOccurrenceId"), str)
                or not mapping["clipOccurrenceId"]
            ):
                continue
            clip_occurrence_id = mapping["clipOccurrenceId"]
            selection_evidence = mapping
        elif (
            occurrence.get("reachabilityDisposition") == "REACHABLE_REVIEWED"
            and occurrence.get("timingDisposition") == "REACHABLE"
            and isinstance(occurrence.get("clipOccurrenceId"), str)
            and occurrence.get("clipOccurrenceId")
        ):
            clip_occurrence_id = occurrence["clipOccurrenceId"]
            selection_evidence = {
                "selectedBranchId": str(occurrence.get("branchId") or ""),
                "sequenceIndex": None,
                "sourceSequencePathSha256": None,
                "timingDisposition": "REACHABLE",
                "clipOccurrenceId": clip_occurrence_id,
                "reviewBasis": occurrence.get("mappingReviewBasis"),
            }
        else:
            continue
        key = (str(occurrence.get("fullKey") or ""), clip_occurrence_id)
        if key in seen:
            raise AdapterError("duplicate reviewed source occurrence: " + repr(key))
        seen.add(key)
        staged = copy.deepcopy(occurrence)
        staged["effectiveClipOccurrenceId"] = clip_occurrence_id
        staged["selectionEvidence"] = copy.deepcopy(selection_evidence)
        branch = branch_by_id.get(str(occurrence.get("branchId") or ""), {})
        staged["inventoryBranchEvidence"] = {
            "branchId": str(occurrence.get("branchId") or ""),
            "branchOrdinal": branch.get("branchOrdinal"),
            "sequenceIndex": branch.get("sequenceIndex"),
            "sourceSequencePathSha256": branch.get(
                "sourceSequencePathSha256"
            ),
        }
        rows.append(staged)
    rows.sort(
        key=lambda row: (
            row["effectiveClipOccurrenceId"],
            str(row.get("fullKey") or ""),
        )
    )
    return rows


def _source_asset_name(asset: dict[str, Any]) -> str:
    path = str(asset.get("objectPath") or "")
    return path.rsplit(".", 1)[-1].casefold()


def _notify_group_key(row: dict[str, Any]) -> tuple[Any, ...]:
    return (
        row["effectiveClipOccurrenceId"],
        str(row.get("patternId") or ""),
        int(row.get("sourceActionId")),
        str(row.get("profileId") or ""),
        int(row.get("sourceStageIndex")),
        int(row.get("sourceClipOrdinal")),
        str(row.get("notifyId") or ""),
        str(row.get("sourceType") or ""),
        finite(row.get("sourceTimeSeconds", 0.0), "source time"),
        finite(row.get("sourceDurationSeconds", 0.0), "source duration"),
    )


def load_whirlwind_canary(repository_root: Path) -> dict[str, Any]:
    history_path = repository_root / WHIRLWIND_HISTORY.relative_to(ROOT)
    effect_path = repository_root / WHIRLWIND_EFFECT.relative_to(ROOT)
    history = read_json(history_path)
    effect = read_json(effect_path)
    corpus = read_json(
        repository_root / VISUAL_PROGRAM_CORPUS.relative_to(ROOT)
    )
    runtime = read_json(
        repository_root / VISUAL_PROGRAM_RUNTIME.relative_to(ROOT)
    )
    history_clone = copy.deepcopy(history)
    artifact_sha = history_clone.pop("artifactSha256", None)
    source = history.get("source") or {}
    outer = source.get("outerEfData") or {}
    history_export = source.get("historyExport") or {}
    playback = history.get("playback") or {}
    elements = effect.get("elements") or []
    kinds = Counter(
        str(row.get("kind") or "") for row in elements if isinstance(row, dict)
    )
    target_ids = {
        str(row.get("id") or "") for row in elements if isinstance(row, dict)
    }
    supplemental = [
        row
        for row in corpus.get("supplementalElements", [])
        if isinstance(row, dict)
        and (row.get("selector") or {}).get("effectAssetId")
        == "effect.valtan.pattern.420633.active"
    ]
    runtime_programs = [
        row
        for row in runtime.get("programs", [])
        if isinstance(row, dict)
        and row.get("effectAssetId")
        == "effect.valtan.pattern.420633.active"
    ]
    runtime_program = runtime_programs[0] if len(runtime_programs) == 1 else {}
    base_identity = runtime_program.get("baseDocumentIdentity") or {}
    projected = runtime_program.get("projectedDocument") or {}
    valid = (
        history.get("schema") == "lostark.valtan-baked-edge-history"
        and history.get("formatVersion") == 1
        and history.get("historyId") == WHIRLWIND_HISTORY_ID
        and artifact_sha == WHIRLWIND_HISTORY_ARTIFACT_SHA256
        and canonical_sha256(history_clone) == artifact_sha
        and raw_sha256(history_path) == WHIRLWIND_HISTORY_RAW_SHA256
        and raw_sha256(effect_path) == WHIRLWIND_EFFECT_RAW_SHA256
        and canonical_sha256(effect) == WHIRLWIND_EFFECT_CANONICAL_SHA256
        and history.get("sampleCount") == len(history.get("samples") or []) == 409
        and abs(float(playback.get("clampSeconds")) - 1.2000000476837158)
        <= 1e-7
        and abs(
            float(history_export.get("sourceEndTimeSeconds"))
            - 3.200000047683716
        )
        <= 1e-7
        and str(outer.get("objectPath") or "").casefold()
        == WHIRLWIND_OUTER_OBJECT
        and len(elements) == 9
        and kinds["trail"] == 3
        and kinds["light"] == 1
        and set(WHIRLWIND_TARGET_IDS).issubset(target_ids)
        and len(supplemental) == 4
        and {
            str(row.get("rowSha256") or "") for row in supplemental
        }
        == WHIRLWIND_SUPPLEMENTAL_ROW_SHA256S
        and len(runtime_programs) == 1
        and base_identity.get("rawSha256")
        == WHIRLWIND_EFFECT_SOURCE_RAW_SHA256
        and base_identity.get("canonicalSha256")
        == WHIRLWIND_EFFECT_CANONICAL_SHA256
        and runtime_program.get("projectedDocumentCanonicalByteCount")
        == 124507
        and runtime_program.get("projectedDocumentSha256")
        == WHIRLWIND_PROJECTED_DOCUMENT_SHA256
        and runtime_program.get("projectedDocumentTypedCodecSha256")
        == WHIRLWIND_PROJECTED_TYPED_CODEC_SHA256
        and runtime_program.get("programSha256")
        == WHIRLWIND_RUNTIME_PROGRAM_SHA256
        and len(projected.get("elements") or []) == 9
    )
    if not valid:
        raise AdapterError(
            "Whirlwind 409-sample/1.2-second/9-of-9 byte canary changed"
        )
    return {
        "status": "AUTHORED_AND_RUNTIME_SLICE_BYTE_IDENTICAL",
        "historyId": WHIRLWIND_HISTORY_ID,
        "historyArtifactSha256": artifact_sha,
        "historyRawSha256": WHIRLWIND_HISTORY_RAW_SHA256,
        "effectRawSha256": WHIRLWIND_EFFECT_RAW_SHA256,
        "effectCanonicalSha256": WHIRLWIND_EFFECT_CANONICAL_SHA256,
        "runtimeBaseSourceRawSha256": WHIRLWIND_EFFECT_SOURCE_RAW_SHA256,
        "runtimeProjectedDocumentCanonicalByteCount": 124507,
        "runtimeProjectedDocumentSha256": (
            WHIRLWIND_PROJECTED_DOCUMENT_SHA256
        ),
        "runtimeProjectedDocumentTypedCodecSha256": (
            WHIRLWIND_PROJECTED_TYPED_CODEC_SHA256
        ),
        "runtimeProgramSha256": WHIRLWIND_RUNTIME_PROGRAM_SHA256,
        "sampleCount": 409,
        "sourceEndTimeSeconds": 3.200000047683716,
        "playbackClampSeconds": 1.2000000476837158,
        "effectElementCount": 9,
        "coreElementCount": 5,
        "animationTrailTargetCount": 3,
        "deferredLightCount": 1,
    }


def _identity_transform() -> dict[str, Any]:
    return {
        "position": [0.0, 0.0, 0.0],
        "rotationDegrees": [0.0, 0.0, 0.0],
        "scale": [1.0, 1.0, 1.0],
    }


def _default_attachment() -> dict[str, Any]:
    return {
        "enabled": False,
        "follow": False,
        "sourceAnchorSlotId": "",
        "runtimeAnchorSlotId": "",
        "runtimeBoneName": "",
        "snapshotRootSourceBasisYawDegrees": 0.0,
        "socketLocalTransform": _identity_transform(),
    }


def _resource_packet(
    runtime_resources_root: Path,
    resources: Iterable[dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[str]]:
    packet: list[dict[str, Any]] = []
    blockers: list[str] = []
    seen_slots: set[str] = set()
    resources_root = runtime_resources_root.resolve()
    for resource in resources:
        if not isinstance(resource, dict):
            blockers.append("MALFORMED_RUNTIME_RESOURCE_BINDING")
            continue
        slot = str(resource.get("slotId") or "")
        asset_id = str(resource.get("assetId") or "")
        pure = PurePosixPath(asset_id)
        if (
            not slot
            or not asset_id
            or pure.is_absolute()
            or ".." in pure.parts
            or slot in seen_slots
        ):
            blockers.append("INVALID_RUNTIME_RESOURCE_BINDING")
            continue
        seen_slots.add(slot)
        path = (resources_root / Path(*pure.parts)).resolve()
        try:
            path.relative_to(resources_root)
        except ValueError:
            blockers.append("RUNTIME_RESOURCE_ESCAPES_RESOURCES_ROOT")
            continue
        if not path.is_file():
            blockers.append("RUNTIME_RESOURCE_PAYLOAD_MISSING")
            continue
        packet.append(
            {
                "slotId": slot,
                "assetId": asset_id,
                "rawSha256": raw_sha256(path),
                "byteCount": path.stat().st_size,
            }
        )
    if not any(row["slotId"] == "base" for row in packet):
        blockers.append("DRAWABLE_BASE_RUNTIME_RESOURCE_MISSING")
    packet.sort(key=lambda row: (row["slotId"], row["assetId"]))
    return packet, sorted(set(blockers))


def _target_contract_from_element(element: dict[str, Any]) -> dict[str, Any]:
    detail = element.get("detail") or {}
    timing = copy.deepcopy(detail.get("timing") or {})
    trail = copy.deepcopy(detail.get("trail") or {})
    attachment = copy.deepcopy(
        element.get("actionCueAttachment") or _default_attachment()
    )
    return {
        "kind": "trail",
        "targetTiming": timing,
        "attachment": attachment,
        "trail": trail,
    }


def _row_evidence(occurrence: dict[str, Any]) -> dict[str, Any]:
    return {
        "inventoryOccurrenceId": str(occurrence.get("occurrenceId") or ""),
        "inventoryFullKey": str(occurrence.get("fullKey") or ""),
        "inventoryBranch": copy.deepcopy(
            occurrence.get("inventoryBranchEvidence") or {}
        ),
        "reviewedSelection": copy.deepcopy(
            occurrence.get("selectionEvidence") or {}
        ),
    }


def _finish_row(row: dict[str, Any]) -> dict[str, Any]:
    identity = row["sourceIdentity"]
    row["sourceIdentitySha256"] = canonical_sha256(identity)
    if not row.get("adapterTargetId"):
        suffix = canonical_sha256(
            {
                "sourceIdentity": identity,
                "targetVariant": row.get("targetVariant"),
            }
        )[:24]
        row["adapterTargetId"] = f"valtan.trail-adapter.{suffix}"
    seal(row, "rowSha256")
    return row


def _unresolved_row(
    family: str,
    occurrence: dict[str, Any],
    identity: dict[str, Any],
    blockers: Iterable[str],
    *,
    target_variant: str | None = None,
) -> dict[str, Any]:
    blocker_list = sorted({str(value) for value in blockers if str(value)})
    if not blocker_list:
        blocker_list = ["RUNTIME_ADAPTER_EVIDENCE_MISSING"]
    return _finish_row(
        {
            "adapterTargetId": "",
            "targetVariant": target_variant,
            "sourceIdentity": identity,
            "evidence": _row_evidence(occurrence),
            "family": family,
            "projectionKind": "ADAPTER_PACKET_V1",
            "disposition": UNRESOLVED,
            "target": None,
            "packet": None,
            "resourcePacket": [],
            "admissionBlockers": blocker_list,
            "preservedLimitations": [],
        }
    )


def _renderer_contract() -> dict[str, Any]:
    return {
        "sharedRendererPath": "EFFECT_TRAIL_SHARED_HEADLESS_V1",
        "minimumRuntimePointCount": 2,
        "onePointDisposition": "SUPPRESSED_INSUFFICIENT_POINTS",
        "twoPointDisposition": "PREPARED_NONZERO_DRAW",
        "newRuntimeManager": False,
        "newShader": False,
    }


def _admitted_row(
    family: str,
    occurrence: dict[str, Any],
    identity: dict[str, Any],
    target: dict[str, Any],
    packet: dict[str, Any],
    resources: list[dict[str, Any]],
    limitations: Iterable[str],
    *,
    target_variant: str | None = None,
    adapter_target_id: str = "",
) -> dict[str, Any]:
    packet["rendererAdmission"] = _renderer_contract()
    seal(packet, "packetSha256")
    return _finish_row(
        {
            "adapterTargetId": adapter_target_id,
            "targetVariant": target_variant,
            "sourceIdentity": identity,
            "evidence": _row_evidence(occurrence),
            "family": family,
            "projectionKind": "ADAPTER_PACKET_V1",
            "disposition": ADMITTED,
            "target": target,
            "packet": packet,
            "resourcePacket": resources,
            "admissionBlockers": [],
            "preservedLimitations": sorted(set(limitations)),
        }
    )


def _whirlwind_animation_rows(
    repository_root: Path,
    runtime_resources_root: Path,
    occurrence: dict[str, Any],
    identity: dict[str, Any],
) -> list[dict[str, Any]]:
    effect = read_json(
        repository_root / WHIRLWIND_EFFECT.relative_to(ROOT)
    )
    by_id = {
        str(row.get("id") or ""): row
        for row in effect.get("elements", [])
        if isinstance(row, dict)
    }
    rows: list[dict[str, Any]] = []
    for target_id in WHIRLWIND_TARGET_IDS:
        element = by_id.get(target_id)
        if element is None or element.get("kind") != "trail":
            raise AdapterError("Whirlwind typed trail target changed: " + target_id)
        resources, blockers = _resource_packet(
            runtime_resources_root, element.get("resources") or []
        )
        if blockers:
            raise AdapterError(
                "Whirlwind typed trail resource closure changed: "
                + target_id
                + ": "
                + ", ".join(blockers)
            )
        contract = _target_contract_from_element(element)
        contract["id"] = target_id
        contract["sourceDocument"] = (
            "Data/Effects/Authored/"
            "effect.valtan.pattern.420633.active.effect.json"
        )
        contract["targetTiming"]["lifeTimeSeconds"] = 1.2000000476837158
        contract["trail"]["maxPoints"] = 409
        contract["trail"]["pointLifeTimeSeconds"] = 1.2000000476837158
        contract["trail"]["minimumDistance"] = 0.0
        contract["trail"]["faceCamera"] = False
        packet = {
            "packetVersion": 2,
            "adapterId": "animation-trail-document-v12",
            "packetLayout": "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1",
            "runtimeCarrier": "EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1",
            "rendererReady": True,
            "sourceNotifyType": "Trails",
            "sourceEventId": identity["notifyId"],
            "sourceAsset": identity["assetObjectPath"],
            "localTimeSeconds": identity["sourceTimeSeconds"],
            "durationSeconds": identity["sourceDurationSeconds"],
            "targetElementId": target_id,
            "targetTiming": copy.deepcopy(contract["targetTiming"]),
            "attachment": copy.deepcopy(contract["attachment"]),
            "trail": copy.deepcopy(contract["trail"]),
            "history": {
                "historyId": WHIRLWIND_HISTORY_ID,
                "artifactSha256": WHIRLWIND_HISTORY_ARTIFACT_SHA256,
                "sampleCount": 409,
                "playbackClampSeconds": 1.2000000476837158,
                "coordinateBasis": "UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS",
            },
        }
        rows.append(
            _admitted_row(
                FAMILY_ANIMATION_TRAIL,
                occurrence,
                identity,
                contract,
                packet,
                resources,
                [
                    "SOURCE_BAKED_EDGE_GEOMETRY_EXACT_"
                    "MATERIAL_REPLAY_BOUNDED"
                ],
                target_variant=target_id,
            )
        )
    return rows


def _edge_history_blockers(history: Any) -> list[str]:
    if not isinstance(history, dict):
        return ["EXACT_EDGE_HISTORY_MISSING"]
    samples = history.get("samples")
    if not isinstance(samples, list) or len(samples) < 2:
        return ["EDGE_HISTORY_REQUIRES_AT_LEAST_TWO_SAMPLES"]
    previous = -math.inf
    for sample in samples:
        if not isinstance(sample, dict):
            return ["EDGE_HISTORY_SAMPLE_MALFORMED"]
        try:
            timestamp = finite(sample.get("relativeTimeSeconds"), "history time")
            edges = [
                sample.get("firstEdgeMeters"),
                sample.get("controlPointMeters"),
                sample.get("secondEdgeMeters"),
            ]
            if timestamp <= previous or any(
                not isinstance(edge, list)
                or len(edge) != 3
                or any(not math.isfinite(float(component)) for component in edge)
                for edge in edges
            ):
                return ["EDGE_HISTORY_SAMPLE_DOMAIN_INVALID"]
        except (AdapterError, TypeError, ValueError):
            return ["EDGE_HISTORY_SAMPLE_DOMAIN_INVALID"]
        previous = timestamp
    if not str(history.get("coordinateBasis") or ""):
        return ["EDGE_HISTORY_COORDINATE_BASIS_MISSING"]
    expected = history.get("historySha256")
    clone = copy.deepcopy(history)
    clone.pop("historySha256", None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        return ["EDGE_HISTORY_SHA256_STALE"]
    return []


def _trail_ghost_row(
    runtime_resources_root: Path,
    occurrence: dict[str, Any],
    identity: dict[str, Any],
) -> dict[str, Any]:
    evidence = occurrence.get("runtimeAdapterEvidence")
    if not isinstance(evidence, dict):
        return _unresolved_row(
            FAMILY_TRAIL_GHOST,
            occurrence,
            identity,
            [
                "TRAIL_GHOST_EXACT_EDGE_HISTORY_MISSING",
                "TRAIL_GHOST_RENDERER_TARGET_MISSING",
            ],
        )
    history = evidence.get("edgeHistory")
    target = evidence.get("target")
    blockers = _edge_history_blockers(history)
    if not isinstance(target, dict) or target.get("kind") != "trail":
        blockers.append("TRAIL_GHOST_RENDERER_TARGET_MISSING")
        resources: list[dict[str, Any]] = []
    else:
        resources, resource_blockers = _resource_packet(
            runtime_resources_root, target.get("resources") or []
        )
        blockers.extend(resource_blockers)
        trail = target.get("trail") or {}
        trail_valid = isinstance(trail, dict)
        try:
            trail_valid = (
                trail_valid
                and int(trail.get("maxPoints") or 0) >= 2
                and finite(
                    trail.get("pointLifeTimeSeconds", 0.0),
                    "TrailGhost point lifetime",
                )
                > 0.0
            )
        except (AdapterError, TypeError, ValueError):
            trail_valid = False
        if not trail_valid:
            blockers.append("TRAIL_GHOST_TARGET_TRAIL_CONTRACT_INVALID")
    if blockers:
        return _unresolved_row(
            FAMILY_TRAIL_GHOST,
            occurrence,
            identity,
            blockers,
        )
    assert isinstance(history, dict) and isinstance(target, dict)
    projected_target = copy.deepcopy(target)
    projected_target["id"] = (
        projected_target.get("id")
        or f"valtan.trail-ghost.{canonical_sha256(identity)[:20]}"
    )
    packet = {
        "packetVersion": 2,
        "adapterId": "animation-trail-document-v12",
        "packetLayout": "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1",
        "runtimeCarrier": "EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1",
        "rendererReady": True,
        "sourceNotifyType": "TrailGhostEffect",
        "sourceEventId": identity["notifyId"],
        "sourceAsset": identity["assetObjectPath"],
        "localTimeSeconds": identity["sourceTimeSeconds"],
        "durationSeconds": identity["sourceDurationSeconds"],
        "targetElementId": projected_target["id"],
        "targetTiming": copy.deepcopy(
            projected_target.get("targetTiming") or {}
        ),
        "attachment": copy.deepcopy(
            projected_target.get("attachment") or _default_attachment()
        ),
        "trail": copy.deepcopy(projected_target["trail"]),
        "history": copy.deepcopy(history),
    }
    return _admitted_row(
        FAMILY_TRAIL_GHOST,
        occurrence,
        identity,
        projected_target,
        packet,
        resources,
        ["TRAIL_GHOST_NORMALIZED_TO_SHARED_BAKED_EDGE_CARRIER_V1"],
        target_variant=str(projected_target["id"]),
    )


def _literal_number(
    module: dict[str, Any], property_path: str, default: float | None = None
) -> float | None:
    values = [
        row.get("value")
        for row in module.get("literals", [])
        if isinstance(row, dict)
        and row.get("kind") == "number"
        and row.get("propertyPath") == property_path
    ]
    if not values:
        return default
    if len(values) != 1:
        return None
    try:
        return finite(values[0], "typed ribbon literal")
    except AdapterError:
        return None


def _ribbon_row(
    runtime_resources_root: Path,
    occurrence: dict[str, Any],
    identity: dict[str, Any],
    carrier: dict[str, Any],
    evidence: dict[str, Any] | None,
) -> dict[str, Any]:
    blockers: list[str] = []
    if evidence is None:
        blockers.append("CASCADE_RIBBON_TYPED_SOURCE_RECIPE_MISSING")
        blockers.append("CASCADE_RIBBON_TARGET_SEED_MISSING")
        return _unresolved_row(
            FAMILY_CASCADE_RIBBON, occurrence, identity, blockers
        )
    recipe = evidence.get("sourceRecipe")
    detail = evidence.get("detail")
    if not isinstance(recipe, dict) or not isinstance(detail, dict):
        return _unresolved_row(
            FAMILY_CASCADE_RIBBON,
            occurrence,
            identity,
            ["CASCADE_RIBBON_TYPED_SOURCE_RECIPE_MISSING"],
        )
    if recipe.get("enabled") is not True:
        blockers.append("CASCADE_RIBBON_SOURCE_RECIPE_DISABLED")
    expected_recipe_sha = carrier.get("sourceRecipeSha256")
    if (
        isinstance(expected_recipe_sha, str)
        and expected_recipe_sha
        and inventory_canonical_sha256(recipe) != expected_recipe_sha
    ):
        blockers.append("CASCADE_RIBBON_SOURCE_RECIPE_SHA256_MISMATCH")
    expected_resources_sha = carrier.get("runtimeResourceBindingSha256")
    if (
        isinstance(expected_resources_sha, str)
        and expected_resources_sha
        and inventory_canonical_sha256(
            evidence.get("runtimeResources") or []
        )
        != expected_resources_sha
    ):
        blockers.append("CASCADE_RIBBON_RUNTIME_RESOURCE_SHA256_MISMATCH")
    typed = [
        module
        for module in recipe.get("modules", [])
        if isinstance(module, dict)
        and str(module.get("className") or "").casefold()
        == "particlemoduletypedataribbon"
    ]
    if len(typed) != 1:
        blockers.append("CASCADE_RIBBON_TYPEDATA_MODULE_NOT_UNIQUE")
        typed_module: dict[str, Any] = {}
    else:
        typed_module = typed[0]
        if (
            not str(typed_module.get("stableId") or "")
            or not str(typed_module.get("objectPath") or "")
        ):
            blockers.append("CASCADE_RIBBON_TYPEDATA_IDENTITY_MISSING")
    tiling = _literal_number(typed_module, "tilingdistance")
    step = _literal_number(typed_module, "distancetessellationstepsize")
    lod = _literal_number(typed_module, "lodvalidity")
    tangent = _literal_number(
        typed_module, "tangenttessellationscalar", 0.0
    )
    maximum = _literal_number(typed_module, "maxparticleintrailcount")
    if tiling is None:
        blockers.append("CASCADE_RIBBON_TILING_DISTANCE_MISSING")
    if step is None or step <= 0.0:
        blockers.append("CASCADE_RIBBON_TESSELLATION_STEP_MISSING")
    if lod is None:
        blockers.append("CASCADE_RIBBON_LOD_VALIDITY_MISSING")
    trail = copy.deepcopy(detail.get("trail") or {})
    try:
        operational_max = int(maximum or trail.get("maxPoints") or 0)
    except (TypeError, ValueError):
        operational_max = 0
    if operational_max < 2:
        blockers.append("CASCADE_RIBBON_OPERATIONAL_MAX_POINTS_INVALID")
    resources, resource_blockers = _resource_packet(
        runtime_resources_root, evidence.get("runtimeResources") or []
    )
    blockers.extend(resource_blockers)
    if blockers:
        return _unresolved_row(
            FAMILY_CASCADE_RIBBON, occurrence, identity, blockers
        )
    projected_recipe = copy.deepcopy(recipe)
    projected_recipe["rendererShape"] = "ribbon"
    target_id = f"valtan.cascade-ribbon.{canonical_sha256(identity)[:20]}"
    target = {
        "id": target_id,
        "kind": "trail",
        "displayName": str(evidence.get("emitterPath") or "").rsplit(
            ".", 1
        )[-1][:64],
        "resources": [
            {"slotId": row["slotId"], "assetId": row["assetId"]}
            for row in resources
        ],
        "material": copy.deepcopy(evidence.get("material") or {}),
        "targetTiming": copy.deepcopy(detail.get("timing") or {}),
        "attachment": _default_attachment(),
        "trail": trail,
        "sourceRecipe": projected_recipe,
    }
    target["trail"]["maxPoints"] = operational_max
    target["trail"]["tilingDistanceWorldUnits"] = float(tiling) * 0.01
    target["trail"]["distanceTessellationStepWorldUnits"] = (
        float(step) * 0.01
    )
    packet = {
        "packetVersion": 1,
        "adapterId": "cascade-ribbon-document-v12",
        "packetLayout": "CASCADE_RIBBON_TYPED_PACKET_V1",
        "runtimeCarrier": "EFFECT_TYPED_CASCADE_RIBBON_V1",
        "rendererReady": True,
        "typeDataStableId": str(typed_module.get("stableId") or ""),
        "typeDataClassName": "particlemoduletypedataribbon",
        "typeDataObjectPath": str(typed_module.get("objectPath") or ""),
        "typeDataModuleSha256": canonical_sha256(typed_module),
        "resolvedRendererShape": "ribbon",
        "tilingDistance": float(tiling) * 0.01,
        "distanceTessellationStepSize": float(step) * 0.01,
        "tangentTessellationScalar": float(tangent or 0.0),
        "lodValidity": float(lod),
        "operationalMaxPoints": operational_max,
        "targetTiming": copy.deepcopy(target["targetTiming"]),
        "attachment": copy.deepcopy(target["attachment"]),
        "trail": copy.deepcopy(target["trail"]),
        "sourceRecipeSha256": canonical_sha256(recipe),
        "projectedRecipeSha256": canonical_sha256(projected_recipe),
        "moduleClosureSha256": canonical_sha256(recipe.get("modules") or []),
        "moduleCount": len(recipe.get("modules") or []),
        "sourceCarrierKey": str(carrier.get("carrierKey") or ""),
    }
    return _admitted_row(
        FAMILY_CASCADE_RIBBON,
        occurrence,
        identity,
        target,
        packet,
        resources,
        ["CASCADE_RIBBON_BOUNDED_RECONSTRUCTION_NOT_NATIVE_SOURCE_EXACT"],
        target_variant=target_id,
    )


def repository_ribbon_evidence(
    repository_root: Path,
    inventory: dict[str, Any],
    wanted_carrier_keys: set[str],
) -> tuple[dict[str, dict[str, Any]], dict[str, Any]]:
    """Decode only wanted ribbon systems through the existing source pipeline."""

    if not wanted_carrier_keys:
        return {}, {"status": "NOT_REQUIRED", "sourceGraphCount": 0}
    if repository_root.resolve() != ROOT.resolve():
        return {}, {"status": "NOT_AVAILABLE_FOR_FIXTURE_ROOT", "sourceGraphCount": 0}
    tools = repository_root / "Tools/EffectPipeline"
    level_tools = repository_root / "Tools/LevelPlacementExtractor"
    for path in (tools, level_tools):
        if str(path) not in sys.path:
            sys.path.insert(0, str(path))
    try:
        import build_imported_effect_documents as imported_effects
        import build_skill_effect_source_receipt as source_receipts
        import build_valtan_source_occurrence_inventory as source_inventory

        catalog = read_json(repository_root / SOURCE_CATALOG.relative_to(ROOT))
        specs = source_inventory.graph_specs(catalog)
        runtime_bindings, cook_descriptor = source_inventory.runtime_cook_receipt(
            specs
        )
        systems = {
            str(row.get("sourceSystemId") or ""): row
            for row in inventory.get("sourceSystems", [])
            if isinstance(row, dict)
        }
        wanted_systems = {
            system_id
            for system_id, system in systems.items()
            if any(
                str(carrier.get("carrierKey") or "") in wanted_carrier_keys
                for carrier in system.get("carriers", [])
                if isinstance(carrier, dict)
            )
        }
        catalog_systems = {
            str(row.get("sourceAsset") or "").casefold(): row
            for row in catalog.get("sourceSystems", [])
            if isinstance(row, dict)
        }
        specs_by_package = {
            package.casefold(): (package, path) for package, path in specs
        }
        grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
        for system_id in wanted_systems:
            row = catalog_systems.get(system_id)
            if row is not None:
                grouped[str(row.get("logicalPackage") or "").casefold()].append(
                    row
                )
        result: dict[str, dict[str, Any]] = {}
        graph_sources: list[dict[str, Any]] = []
        for package_key in sorted(grouped):
            spec = specs_by_package.get(package_key)
            if spec is None:
                continue
            graph_index = source_receipts.load_graphs([spec])
            graph_sources.append(
                {
                    "logicalPackage": spec[0],
                    "rawSha256": raw_sha256(spec[1]),
                }
            )
            for catalog_system in grouped[package_key]:
                source_asset = str(catalog_system.get("sourceAsset") or "")
                parts = source_receipts.source_asset_parts(source_asset)
                root = (
                    source_receipts.find_particle_system(graph_index, *parts)
                    if parts is not None
                    else None
                )
                if root is None:
                    continue
                graph_package, root_row = root
                graph = source_receipts.collect_system_graph(
                    graph_index, graph_package, root_row
                )
                system = {
                    "sourceSystemId": source_asset.casefold(),
                    "sourceAsset": source_asset,
                    "logicalPackage": spec[0],
                    "objectName": str(root_row.get("objectName") or ""),
                    "objectPath": str(root_row.get("objectPath") or ""),
                    "rootNodeId": graph["rootNodeId"],
                    "nodeIds": graph["nodeIds"],
                    "resourceBindings": graph["resourceBindings"],
                    "unresolvedExternalReferences": graph[
                        "unresolvedExternalReferences"
                    ],
                    "summary": graph["summary"],
                }
                normalized = {
                    "schema": "lostark.normalized-effect-source-graph",
                    "schemaVersion": 1,
                    "characterClass": "VALTAN",
                    "skillId": 0,
                    "sourceSystems": [system],
                    "nodes": [
                        graph["nodes"][key] for key in sorted(graph["nodes"])
                    ],
                    "edges": graph["edges"],
                    "materialParameterBindings": (
                        source_inventory.material_rows_for_system(catalog, graph)
                    ),
                    "runtimeResourceBindings": runtime_bindings,
                }
                index = imported_effects.SourceIndex(
                    normalized, {"packages": []}
                )
                emitter_occurrences: Counter[str] = Counter()
                partitions = imported_effects.selected_lod_partitions(
                    normalized, index, {"packages": []}
                )
                for partition_ordinal, (
                    selected_system,
                    emitter,
                    lod,
                    modules,
                ) in enumerate(partitions):
                    occurrence_ordinal = emitter_occurrences[emitter.source_id]
                    emitter_occurrences[emitter.source_id] += 1
                    module_rows = source_inventory.module_occurrences(modules)
                    module_hash = source_inventory.canonical_sha256(module_rows)
                    carrier_key = (
                        f"system={source_asset.casefold()}"
                        f"|emitter={emitter.source_id}"
                        f"|emitterOccurrence={occurrence_ordinal}"
                        f"|lod={lod.source_id}|moduleOrder={module_hash}"
                    )
                    if carrier_key not in wanted_carrier_keys:
                        continue
                    if not any(
                        str(module.class_name).casefold()
                        == "particlemoduletypedataribbon"
                        for module in modules
                    ):
                        continue
                    detail, _mappings, bursts = imported_effects.emitter_detail(
                        index,
                        lod,
                        modules,
                        0.0,
                        0.0,
                        partition_ordinal + 1,
                    )
                    recipe = imported_effects.build_source_recipe(
                        index, modules, "sprite", bursts
                    )
                    module_ids = {
                        lod.source_id,
                        emitter.source_id,
                        *(
                            module.source_id.split("@ref:", 1)[0]
                            for module in modules
                        ),
                    }
                    runtime_resources, _receipt, materials = (
                        imported_effects.choose_resources(
                            selected_system, module_ids, normalized
                        )
                    )
                    source_material = (
                        str(materials[0].get("sourceMaterialPath") or "")
                        if materials
                        else ""
                    )
                    result[carrier_key] = {
                        "sourceRecipe": recipe,
                        "detail": detail,
                        "runtimeResources": runtime_resources,
                        "emitterPath": emitter.object_path,
                        "material": {
                            "templateId": "effect.standard",
                            "sourceMaterialPath": source_material,
                            "renderProfile": (
                                "additive_two_sided_depth_read"
                                if any(
                                    token in source_material.casefold()
                                    for token in ("_ad", "add")
                                )
                                else "alpha_two_sided_depth_read"
                            ),
                            "sourceProfile": {"enabled": False},
                        },
                    }
        return result, {
            "status": "SOURCE_GRAPH_TYPED_RECIPE_DECODED",
            "sourceGraphCount": len(graph_sources),
            "sourceGraphs": graph_sources,
            "runtimeCookReceipt": cook_descriptor,
            "decodedCarrierCount": len(result),
        }
    except Exception as error:  # fail closed into explicit unresolved rows
        return {}, {
            "status": "SOURCE_GRAPH_ENRICHMENT_UNAVAILABLE",
            "sourceGraphCount": 0,
            "reason": type(error).__name__,
        }


def _system_index(inventory: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {
        str(row.get("sourceSystemId") or ""): row
        for row in inventory.get("sourceSystems", [])
        if isinstance(row, dict)
    }


def build_document(
    repository_root: Path,
    inventory: dict[str, Any],
    selections: dict[str, Any] | None,
    *,
    ribbon_evidence: dict[str, dict[str, Any]] | None = None,
    enrich_source_graphs: bool = True,
    require_whirlwind_canary: bool = True,
    runtime_resources_root: Path | None = None,
) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    resources_root = (
        runtime_resources_root.resolve()
        if runtime_resources_root is not None
        else resolve_runtime_resources_root(repository_root)
    )
    canary = (
        load_whirlwind_canary(repository_root)
        if require_whirlwind_canary
        else {"status": "FIXTURE_BYPASS"}
    )
    reviewed = reviewed_reachable_occurrences(inventory, selections)
    systems = _system_index(inventory)
    excluded_light = 0
    excluded_dust = 0
    trail_source_rows: list[dict[str, Any]] = []
    for row in reviewed:
        system = systems.get(str(row.get("sourceSystemId") or ""))
        if str(row.get("category") or "").casefold() == "light":
            excluded_light += 1
            continue
        if system is not None and system.get("explicitGenericDust") is True:
            excluded_dust += 1
            continue
        trail_source_rows.append(row)

    wanted_carriers: set[str] = set()
    for row in trail_source_rows:
        system = systems.get(str(row.get("sourceSystemId") or ""))
        if system is None:
            continue
        for carrier in system.get("carriers", []):
            if (
                isinstance(carrier, dict)
                and carrier.get("runtimeAdapterType") == "RIBBON"
            ):
                wanted_carriers.add(str(carrier.get("carrierKey") or ""))
    evidence = dict(ribbon_evidence or {})
    enrichment = {"status": "CALLER_SUPPLIED", "sourceGraphCount": 0}
    if enrich_source_graphs:
        decoded, enrichment = repository_ribbon_evidence(
            repository_root, inventory, wanted_carriers - set(evidence)
        )
        evidence.update(decoded)

    rows: list[dict[str, Any]] = []
    grouped: dict[tuple[Any, ...], list[dict[str, Any]]] = defaultdict(list)
    for row in trail_source_rows:
        grouped[_notify_group_key(row)].append(row)
    for group_key in sorted(grouped, key=repr):
        group = grouped[group_key]
        exemplar = group[0]
        clip_occurrence_id = exemplar["effectiveClipOccurrenceId"]
        source_type = str(exemplar.get("sourceType") or "").casefold()
        assets = [
            row.get("assetReference")
            for row in group
            if isinstance(row.get("assetReference"), dict)
        ]
        efdata = next(
            (
                asset
                for asset in assets
                if str(asset.get("className") or "").casefold()
                == "efdata_animnotify_trails"
            ),
            None,
        )
        if source_type == "trails":
            primary = efdata or (assets[0] if assets else None)
            identity = stable_source_identity(
                exemplar, clip_occurrence_id, asset_reference=primary
            )
            if efdata is not None and _source_asset_name(efdata) == WHIRLWIND_OUTER_OBJECT:
                rows.extend(
                    _whirlwind_animation_rows(
                        repository_root, resources_root, exemplar, identity
                    )
                )
            else:
                rows.append(
                    _unresolved_row(
                        FAMILY_ANIMATION_TRAIL,
                        exemplar,
                        identity,
                        [
                            "ANIMATION_TRAIL_EXACT_BAKED_EDGE_HISTORY_MISSING",
                            "ANIMATION_TRAIL_RENDERER_TARGET_MISSING",
                        ],
                    )
                )
        elif source_type == "trailghosteffect":
            identity = stable_source_identity(
                exemplar, clip_occurrence_id, asset_reference=assets[0] if assets else None
            )
            rows.append(
                _trail_ghost_row(resources_root, exemplar, identity)
            )

        seen_systems: set[str] = set()
        for source_row in group:
            system_id = str(source_row.get("sourceSystemId") or "")
            if not system_id or system_id in seen_systems:
                continue
            seen_systems.add(system_id)
            system = systems.get(system_id)
            if system is None:
                continue
            for carrier in system.get("carriers", []):
                if (
                    not isinstance(carrier, dict)
                    or carrier.get("runtimeAdapterType") != "RIBBON"
                ):
                    continue
                carrier_key = str(carrier.get("carrierKey") or "")
                identity = stable_source_identity(
                    source_row,
                    clip_occurrence_id,
                    carrier_key=carrier_key,
                )
                rows.append(
                    _ribbon_row(
                        resources_root,
                        source_row,
                        identity,
                        carrier,
                        evidence.get(carrier_key),
                    )
                )

    rows.sort(key=lambda row: row["adapterTargetId"])
    family_counts = Counter(row["family"] for row in rows)
    dispositions = Counter(row["disposition"] for row in rows)
    unresolved_blockers = Counter(
        blocker
        for row in rows
        for blocker in row.get("admissionBlockers", [])
    )
    reviewed_selection_count = sum(
        isinstance(row, dict) and row.get("status") == "REVIEWED_SELECTED"
        for row in (selections or {}).get("selections", [])
    )
    source_carrier_count = sum(
        len(system.get("carriers", []))
        for system in inventory.get("sourceSystems", [])
        if isinstance(system, dict)
    )
    document = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "selectionPolicy": (
            "REVIEWED_SELECTED_REACHABLE_STAGE_CLIP_ONLY; "
            "NO_SOURCE_ONLY_CLIP_INSERTION; MISSING_ADAPTER_EVIDENCE_"
            "PRESERVED_UNRESOLVED"
        ),
        "deferredPolicy": {
            "light": "EXCLUDED_DEFERRED",
            "explicitGenericDust": "EXCLUDED_DEFERRED",
            "animationTrail": "TYPED_ADAPTER_REQUIRED",
            "trailGhost": "TYPED_ADAPTER_REQUIRED",
            "cascadeRibbon": "TYPED_ADAPTER_REQUIRED",
        },
        "inputIdentity": {
            "inventoryCanonicalSha256": canonical_sha256(inventory),
            "selectionCanonicalSha256": (
                canonical_sha256(selections) if selections is not None else None
            ),
            "sourceGraphEnrichment": enrichment,
        },
        "whirlwindCanary": canary,
        "adapters": rows,
        "summary": {
            "reviewedSelectionCount": reviewed_selection_count,
            "sourceInventoryOccurrenceCount": len(
                inventory.get("occurrences", [])
            ),
            "sourceInventoryCarrierCount": source_carrier_count,
            "reviewedReachableSourceOccurrenceCount": len(reviewed),
            "trailAdapterRowCount": len(rows),
            "familyCounts": dict(sorted(family_counts.items())),
            "dispositionCounts": dict(sorted(dispositions.items())),
            "admittedRendererReadyCount": dispositions[ADMITTED],
            "unresolvedRuntimeAdapterCount": dispositions[UNRESOLVED],
            "unresolvedBlockerCounts": dict(sorted(unresolved_blockers.items())),
            "excludedDeferredLightOccurrenceCount": excluded_light,
            "excludedExplicitGenericDustOccurrenceCount": excluded_dust,
            "sourceOnlyInsertedClipCount": 0,
            "droppedTrailAdapterRowCount": 0,
            "duplicateAdapterTargetCount": 0,
        },
    }
    seal(document, "artifactSha256")
    validate_document(
        document,
        repository_root,
        runtime_resources_root=resources_root,
    )
    return document


def renderer_probe_disposition(
    row: dict[str, Any], runtime_point_count: int
) -> str:
    if row.get("disposition") != ADMITTED:
        return "SUPPRESSED_UNADMITTED"
    packet = row.get("packet") or {}
    admission = packet.get("rendererAdmission") or {}
    minimum = int(admission.get("minimumRuntimePointCount") or 0)
    if runtime_point_count < minimum:
        return "SUPPRESSED_INSUFFICIENT_POINTS"
    if not row.get("resourcePacket"):
        return "SUPPRESSED_MISSING_RUNTIME_RESOURCE"
    return "PREPARED_NONZERO_DRAW"


def validate_document(
    document: dict[str, Any],
    repository_root: Path | None = None,
    *,
    runtime_resources_root: Path | None = None,
) -> None:
    if (
        document.get("schema") != SCHEMA
        or document.get("formatVersion") != FORMAT_VERSION
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise AdapterError("trail adapter document header is invalid")
    verify_seal(document, "artifactSha256", "trail adapter document")
    adapters = document.get("adapters")
    if not isinstance(adapters, list):
        raise AdapterError("trail adapter rows are not a list")
    ids: set[str] = set()
    for index, row in enumerate(adapters):
        if not isinstance(row, dict):
            raise AdapterError(f"adapter row {index} is not an object")
        verify_seal(row, "rowSha256", f"adapter row {index}")
        target_id = str(row.get("adapterTargetId") or "")
        if not target_id or target_id in ids:
            raise AdapterError("adapter target identity is empty or duplicated")
        ids.add(target_id)
        identity = row.get("sourceIdentity")
        if (
            not isinstance(identity, dict)
            or canonical_sha256(identity) != row.get("sourceIdentitySha256")
            or not str(identity.get("clipOccurrenceId") or "")
        ):
            raise AdapterError("adapter source identity is stale")
        family = row.get("family")
        if family not in {
            FAMILY_ANIMATION_TRAIL,
            FAMILY_TRAIL_GHOST,
            FAMILY_CASCADE_RIBBON,
        }:
            raise AdapterError("adapter family is invalid")
        disposition = row.get("disposition")
        blockers = row.get("admissionBlockers")
        if disposition == UNRESOLVED:
            if (
                row.get("target") is not None
                or row.get("packet") is not None
                or row.get("resourcePacket") != []
                or not isinstance(blockers, list)
                or not blockers
            ):
                raise AdapterError("unresolved adapter row was partially admitted")
            continue
        if disposition != ADMITTED:
            raise AdapterError("adapter disposition is invalid")
        if blockers != []:
            raise AdapterError("admitted adapter retains blockers")
        target = row.get("target")
        packet = row.get("packet")
        resources = row.get("resourcePacket")
        if (
            not isinstance(target, dict)
            or target.get("kind") != "trail"
            or not isinstance(packet, dict)
            or packet.get("rendererReady") is not True
            or not isinstance(resources, list)
            or not resources
        ):
            raise AdapterError("admitted adapter target/packet is incomplete")
        verify_seal(packet, "packetSha256", "typed adapter packet")
        admission = packet.get("rendererAdmission") or {}
        if (
            admission.get("sharedRendererPath")
            != "EFFECT_TRAIL_SHARED_HEADLESS_V1"
            or admission.get("minimumRuntimePointCount") != 2
            or admission.get("twoPointDisposition")
            != "PREPARED_NONZERO_DRAW"
            or admission.get("newRuntimeManager") is not False
            or admission.get("newShader") is not False
        ):
            raise AdapterError("renderer admission contract is invalid")
        if family in {FAMILY_ANIMATION_TRAIL, FAMILY_TRAIL_GHOST}:
            if (
                packet.get("packetLayout")
                != "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1"
                or packet.get("runtimeCarrier")
                != "EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1"
                or not isinstance(packet.get("history"), dict)
            ):
                raise AdapterError("baked-edge adapter carrier is invalid")
        else:
            if (
                packet.get("packetLayout")
                != "CASCADE_RIBBON_TYPED_PACKET_V1"
                or packet.get("runtimeCarrier")
                != "EFFECT_TYPED_CASCADE_RIBBON_V1"
                or packet.get("resolvedRendererShape") != "ribbon"
                or finite(packet.get("distanceTessellationStepSize"), "step")
                <= 0.0
                or int(packet.get("operationalMaxPoints") or 0) < 2
            ):
                raise AdapterError("CascadeRibbon adapter carrier is invalid")
        slots: set[str] = set()
        for resource in resources:
            if not isinstance(resource, dict):
                raise AdapterError("adapter resource is malformed")
            slot = str(resource.get("slotId") or "")
            if not slot or slot in slots:
                raise AdapterError("adapter resource slot is empty or duplicated")
            slots.add(slot)
            if repository_root is not None:
                asset_id = PurePosixPath(str(resource.get("assetId") or ""))
                resources_root = (
                    runtime_resources_root
                    if runtime_resources_root is not None
                    else repository_root / "Client/Bin/Resources"
                )
                path = resources_root / Path(*asset_id.parts)
                if (
                    not path.is_file()
                    or raw_sha256(path) != resource.get("rawSha256")
                    or path.stat().st_size != resource.get("byteCount")
                ):
                    raise AdapterError("adapter runtime resource identity is stale")
        if "base" not in slots:
            raise AdapterError("adapter has no drawable base resource")
        if renderer_probe_disposition(row, 1) != "SUPPRESSED_INSUFFICIENT_POINTS":
            raise AdapterError("one-point renderer suppression changed")
        if renderer_probe_disposition(row, 2) != "PREPARED_NONZERO_DRAW":
            raise AdapterError("two-point renderer preparation changed")

    summary = document.get("summary") or {}
    dispositions = Counter(row.get("disposition") for row in adapters)
    if (
        not isinstance(summary.get("reviewedSelectionCount"), int)
        or not isinstance(summary.get("sourceInventoryOccurrenceCount"), int)
        or not isinstance(summary.get("sourceInventoryCarrierCount"), int)
        or summary.get("trailAdapterRowCount") != len(adapters)
        or summary.get("admittedRendererReadyCount") != dispositions[ADMITTED]
        or summary.get("unresolvedRuntimeAdapterCount") != dispositions[UNRESOLVED]
        or summary.get("sourceOnlyInsertedClipCount") != 0
        or summary.get("droppedTrailAdapterRowCount") != 0
        or summary.get("duplicateAdapterTargetCount") != 0
    ):
        raise AdapterError("trail adapter summary is stale")


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    os.replace(temporary, path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--dry-run", action="store_true")
    parser.add_argument("--inventory", type=Path, default=DEFAULT_INVENTORY)
    parser.add_argument("--selection-manifest", type=Path, default=DEFAULT_SELECTIONS)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--runtime-resources-root",
        type=Path,
        help=(
            "physical Client/Bin/Resources root; required for an isolated "
            "worktree whose ignored runtime inputs are not hydrated"
        ),
    )
    parser.add_argument("--skip-source-graph-enrichment", action="store_true")
    args = parser.parse_args()

    inventory = read_json(args.inventory)
    selections = (
        read_json(args.selection_manifest)
        if args.selection_manifest.is_file()
        else None
    )
    document = build_document(
        ROOT,
        inventory,
        selections,
        enrich_source_graphs=not args.skip_source_graph_enrichment,
        runtime_resources_root=args.runtime_resources_root,
    )
    payload = pretty_bytes(document)
    if args.write:
        write_atomic(args.output, payload)
        label = "written"
    elif args.check:
        if not args.output.is_file() or args.output.read_bytes() != payload:
            raise AdapterError(f"checked trail adapter manifest drifted: {args.output}")
        label = "checked"
    else:
        label = "dry-run"
    print(
        "Valtan trail adapter packets "
        f"{label}: {json.dumps(document['summary'], sort_keys=True)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AdapterError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
