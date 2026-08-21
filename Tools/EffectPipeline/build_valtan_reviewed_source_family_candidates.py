#!/usr/bin/env python3
"""Build immutable v13 candidates for reviewed Valtan source families.

This tool is intentionally a report-only bridge.  It reads the exact reviewed
source occurrence inventory and canonical v2 cue graph, then writes candidates
only below ``Data/Effects/Imported/Valtan/ReviewedSourceFamilies``.  It never
changes Authored Effect documents, EffectCatalog, or pattern Effect cues.

Only ``REACHABLE_REVIEWED`` occurrence x ``EXECUTABLE_CORE`` carrier rows may
be projected.  A source notify is clip local, while an Effect element is cue
local, so the only valid element delay is::

    occurrence.sourceTimeSeconds - cue.sourceStartMs / 1000

Negative delays and starts at/after an explicit cue end remain visible in the
receipt and are not admitted.  Missing or multiple exact clip-occurrence cues
are likewise report-only blockers.  The frozen Whirlwind active Effect is
always excluded from both candidate creation and any reconcile proposal.
"""

from __future__ import annotations

import argparse
import copy
import json
import math
import re
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable

import build_valtan_source_occurrence_inventory as source_inventory


ROOT = Path(__file__).resolve().parents[2]
SELECTION_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.priority-source-sequence-selections.v1.json"
)
OUTPUT_ROOT = (
    ROOT / "Data/Effects/Imported/Valtan/ReviewedSourceFamilies"
)
RECEIPT_NAME = "Valtan.reviewed-source-family-candidates.v1.json"
CATALOG_PATH = ROOT / "Data/Effects/EffectCatalog.json"
CUE_PATH = source_inventory.CUE_PATH
AUTHORED_ROOT = source_inventory.AUTHORED_ROOT
IMPORTED_VALTAN_ROOT = ROOT / "Data/Effects/Imported/Valtan"

PROTECTED_EFFECT_ASSET_IDS = {
    "effect.valtan.pattern.420633.active",
}
SHA256_RE = re.compile(r"[0-9a-f]{64}")

# These are source/cue projection denominators, not visual-fidelity claims.
# A change means the reviewed sequence manifest, exact clip mapping, cue
# windows, or source carrier closure moved and must be reviewed deliberately.
EXPECTED_COUNTS = {
    "reviewedSelectedBranchCount": 24,
    "reviewedSelectedPatternCount": 24,
    "reachableSourceOccurrenceCount": 444,
    "reachableCoreProjectionCount": 630,
    "candidateDocumentCount": 37,
    "candidateClipOccurrenceCount": 37,
    "candidatePatternCount": 21,
    "admittedCoreProjectionCount": 380,
    "candidateElementCount": 380,
    "protectedCanaryProjectionCount": 3,
    "missingCueProjectionCount": 161,
    "multipleCueProjectionCount": 0,
    "negativeTimingProjectionCount": 86,
    "outsideCueWindowProjectionCount": 0,
    "effectAssetReuseDivergenceProjectionCount": 0,
}


class CandidateError(RuntimeError):
    """Raised when immutable candidate generation cannot remain exact."""


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def candidate_filename(effect_asset_id: str) -> str:
    return f"{effect_asset_id}.reviewed-source-candidate.effect.json"


def _path_below(path: Path, parent: Path) -> bool:
    resolved = path.resolve()
    root = parent.resolve()
    return resolved != root and root in resolved.parents


def validate_output_root(output_root: Path) -> None:
    if not _path_below(output_root, IMPORTED_VALTAN_ROOT):
        raise CandidateError(
            "reviewed source candidates must stay below Imported/Valtan"
        )
    authored = AUTHORED_ROOT.resolve()
    target = output_root.resolve()
    if target == authored or authored in target.parents:
        raise CandidateError("reviewed source candidates cannot target Authored")


def catalog_index(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if document.get("formatVersion") != 1:
        raise CandidateError("EffectCatalog formatVersion is not 1")
    result: dict[str, dict[str, Any]] = {}
    for row in document.get("effects", []):
        effect_id = str(row.get("effectAssetId") or "")
        if not effect_id:
            raise CandidateError("EffectCatalog contains an empty effectAssetId")
        if effect_id in result:
            raise CandidateError(f"duplicate Effect catalog ID: {effect_id}")
        result[effect_id] = copy.deepcopy(row)
    return result


def cue_index(
    document: dict[str, Any],
) -> dict[str, list[dict[str, Any]]]:
    if document.get("formatVersion") != 2:
        raise CandidateError("reviewed source candidates require canonical v2 cues")
    result: dict[str, list[dict[str, Any]]] = defaultdict(list)
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    for row in document.get("cues", []):
        binding_id = str(row.get("bindingId") or "")
        occurrence_id = str(row.get("occurrenceId") or "")
        clip_occurrence_id = str(row.get("clipOccurrenceId") or "")
        effect_asset_id = str(row.get("effectAssetId") or "")
        if not all(
            (binding_id, occurrence_id, clip_occurrence_id, effect_asset_id)
        ):
            raise CandidateError("v2 cue identity is incomplete")
        if binding_id in binding_ids:
            raise CandidateError(f"duplicate cue bindingId: {binding_id}")
        if occurrence_id in occurrence_ids:
            raise CandidateError(f"duplicate cue occurrenceId: {occurrence_id}")
        binding_ids.add(binding_id)
        occurrence_ids.add(occurrence_id)
        source_start_ms = row.get("sourceStartMs")
        source_end_ms = row.get("sourceEndMs")
        if (
            isinstance(source_start_ms, bool)
            or not isinstance(source_start_ms, int)
            or source_start_ms < 0
        ):
            raise CandidateError(f"cue sourceStartMs is invalid: {occurrence_id}")
        if source_end_ms is not None and (
            isinstance(source_end_ms, bool)
            or not isinstance(source_end_ms, int)
            or source_end_ms <= source_start_ms
        ):
            raise CandidateError(f"cue sourceEndMs is invalid: {occurrence_id}")
        result[clip_occurrence_id].append(copy.deepcopy(row))
    for rows in result.values():
        rows.sort(key=lambda row: str(row["occurrenceId"]))
    return dict(result)


def authored_document_path(catalog_row: dict[str, Any]) -> Path:
    relative = str(catalog_row.get("authoringPath") or "")
    if not relative:
        raise CandidateError("Effect catalog row has no authoringPath")
    path = (ROOT / "Data" / relative).resolve()
    if not path.is_file() or not _path_below(path, AUTHORED_ROOT):
        raise CandidateError(f"authored Effect document is missing: {relative}")
    return path


def add_v13_transform_ownership_defaults(
    element: dict[str, Any],
) -> dict[str, Any]:
    """Attach explicit no-inheritance ownership required by ordinary v13."""
    result = copy.deepcopy(element)
    result["actionCueAttachment"] = {
        "enabled": False,
        "follow": False,
        "sourceAnchorSlotId": "",
        "runtimeAnchorSlotId": "",
        "runtimeBoneName": "",
        "snapshotRootSourceBasisYawDegrees": 0,
        "socketLocalTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }
    result["transformInheritance"] = {
        "enabled": False,
        "masterElementId": "",
    }
    return result


def compress_v13_source_node(
    element: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, str]]:
    """Keep the full source key in the receipt and its hash in v13's field."""
    result = copy.deepcopy(element)
    full_source_key = str(result.get("sourceNode") or "")
    if not full_source_key:
        raise CandidateError("candidate source element lost its full source key")
    compact_key = "valtan.source." + source_inventory.sha256_bytes(
        full_source_key.encode("utf-8")
    )
    result["sourceNode"] = compact_key
    return result, {
        "id": str(result.get("id") or ""),
        "sourceNode": compact_key,
        "fullSourceKey": full_source_key,
    }


def cue_local_start_delay_seconds(
    occurrence: dict[str, Any], cue: dict[str, Any]
) -> tuple[str, float]:
    source_time = source_inventory.finite_number(
        occurrence.get("sourceTimeSeconds"), "source occurrence time"
    )
    cue_start = int(cue["sourceStartMs"]) / 1000.0
    cue_end_value = cue.get("sourceEndMs")
    delay = source_time - cue_start
    if delay < 0.0:
        return "NEGATIVE_BEFORE_CUE_START", delay
    if cue_end_value is not None and source_time >= int(cue_end_value) / 1000.0:
        return "OUTSIDE_EXPLICIT_CUE_WINDOW", delay
    if not math.isfinite(delay):
        raise CandidateError("cue-local element delay is not finite")
    return "ADMITTED", delay


def build_projection_seed(
    occurrence: dict[str, Any],
    carrier: dict[str, Any],
    cue: dict[str, Any],
) -> tuple[dict[str, Any] | None, dict[str, Any]]:
    timing_disposition, delay = cue_local_start_delay_seconds(occurrence, cue)
    visual_timing_key = (
        f"pattern={occurrence['patternId']}"
        f"|clipOccurrence={occurrence['clipOccurrenceId']}"
        f"|sourceStage={occurrence['sourceStageIndex']}"
        f"|sourceTime={float(occurrence['sourceTimeSeconds']):.9f}"
    )
    notify_system_timing_key = (
        visual_timing_key
        + f"|notify={occurrence['notifyId']}"
        + f"|sourceSystem={occurrence.get('sourceSystemId') or ''}"
    )
    projection = {
        "patternId": str(occurrence["patternId"]),
        "semanticStageId": str(occurrence.get("semanticStageId") or ""),
        "gameplayActionId": str(occurrence.get("gameplayActionId") or ""),
        "clipOccurrenceId": str(occurrence["clipOccurrenceId"]),
        "cueBindingId": str(cue["bindingId"]),
        "cueOccurrenceId": str(cue["occurrenceId"]),
        "effectAssetId": str(cue["effectAssetId"]),
        "occurrenceFullKey": str(occurrence["fullKey"]),
        "carrierKey": str(carrier["carrierKey"]),
        "sourceActionId": int(occurrence["sourceActionId"]),
        "branchId": str(occurrence["branchId"]),
        "sourceStageIndex": int(occurrence["sourceStageIndex"]),
        "sourceStagePath": str(occurrence["sourceStagePath"]),
        "notifyOrdinal": int(occurrence["notifyOrdinal"]),
        "notifyId": str(occurrence["notifyId"]),
        "sourceSystemId": str(occurrence.get("sourceSystemId") or ""),
        "sourceEmitterNodeId": str(carrier["sourceEmitterNodeId"]),
        "sourceEmitterPath": str(carrier["sourceEmitterPath"]),
        "sourceTimeSeconds": float(occurrence["sourceTimeSeconds"]),
        "sourceDurationSeconds": float(occurrence["sourceDurationSeconds"]),
        "cueSourceStartMs": int(cue["sourceStartMs"]),
        "cueSourceEndMs": cue.get("sourceEndMs"),
        "elementStartDelaySeconds": delay,
        "visualTimingGroupId": (
            "valtan.visual-timing."
            + source_inventory.sha256_bytes(
                visual_timing_key.encode("utf-8")
            )[:20]
        ),
        "visualTimingGroupKey": visual_timing_key,
        "notifySystemTimingGroupId": (
            "valtan.notify-system-timing."
            + source_inventory.sha256_bytes(
                notify_system_timing_key.encode("utf-8")
            )[:20]
        ),
        "notifySystemTimingGroupKey": notify_system_timing_key,
        "disposition": timing_disposition,
    }
    if timing_disposition != "ADMITTED":
        return None, projection
    seed = source_inventory.occurrence_element_seed(occurrence, carrier)
    timing = seed.setdefault("detail", {}).setdefault("timing", {})
    timing["startDelaySeconds"] = delay
    projection["sourceFamilyGroupId"] = str(seed.get("groupId") or "")
    seed = add_v13_transform_ownership_defaults(seed)
    compact, source_key = compress_v13_source_node(seed)
    projection.update(source_key)
    return compact, projection


def _candidate_visual_signature(elements: Iterable[dict[str, Any]]) -> str:
    """Compare shared-asset candidates without occurrence storage identity."""
    normalized = []
    for element in elements:
        row = copy.deepcopy(element)
        row.pop("id", None)
        row.pop("sourceNode", None)
        normalized.append(row)
    normalized.sort(key=source_inventory.canonical_sha256)
    return source_inventory.canonical_sha256(normalized)


def build_timing_groups(
    source_element_keys: list[dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    """Preserve visual-wave and notify/system timing identities explicitly."""
    visual_groups: dict[str, dict[str, Any]] = {}
    notify_system_groups: dict[str, dict[str, Any]] = {}
    for row in source_element_keys:
        visual_id = str(row["visualTimingGroupId"])
        visual = visual_groups.setdefault(
            visual_id,
            {
                "visualTimingGroupId": visual_id,
                "visualTimingGroupKey": str(row["visualTimingGroupKey"]),
                "patternId": str(row["patternId"]),
                "clipOccurrenceId": str(row["clipOccurrenceId"]),
                "sourceStageIndex": int(row["sourceStageIndex"]),
                "sourceTimeSeconds": float(row["sourceTimeSeconds"]),
                "elementStartDelaySeconds": float(
                    row["elementStartDelaySeconds"]
                ),
                "notifySystemTimingGroupIds": set(),
                "notifyIds": set(),
                "sourceSystemIds": set(),
                "elementIds": [],
            },
        )
        if (
            visual["visualTimingGroupKey"] != row["visualTimingGroupKey"]
            or visual["sourceTimeSeconds"] != row["sourceTimeSeconds"]
            or visual["elementStartDelaySeconds"]
            != row["elementStartDelaySeconds"]
        ):
            raise CandidateError("visual timing group identity collided")
        visual["notifySystemTimingGroupIds"].add(
            str(row["notifySystemTimingGroupId"])
        )
        visual["notifyIds"].add(str(row["notifyId"]))
        visual["sourceSystemIds"].add(str(row["sourceSystemId"]))
        visual["elementIds"].append(str(row["id"]))

        notify_system_id = str(row["notifySystemTimingGroupId"])
        notify_system = notify_system_groups.setdefault(
            notify_system_id,
            {
                "notifySystemTimingGroupId": notify_system_id,
                "notifySystemTimingGroupKey": str(
                    row["notifySystemTimingGroupKey"]
                ),
                "visualTimingGroupId": visual_id,
                "patternId": str(row["patternId"]),
                "clipOccurrenceId": str(row["clipOccurrenceId"]),
                "sourceStageIndex": int(row["sourceStageIndex"]),
                "notifyOrdinal": int(row["notifyOrdinal"]),
                "notifyId": str(row["notifyId"]),
                "sourceSystemId": str(row["sourceSystemId"]),
                "sourceTimeSeconds": float(row["sourceTimeSeconds"]),
                "elementStartDelaySeconds": float(
                    row["elementStartDelaySeconds"]
                ),
                "sourceFamilyGroupIds": set(),
                "elementIds": [],
            },
        )
        if (
            notify_system["notifySystemTimingGroupKey"]
            != row["notifySystemTimingGroupKey"]
            or notify_system["visualTimingGroupId"] != visual_id
        ):
            raise CandidateError("notify/system timing group identity collided")
        notify_system["sourceFamilyGroupIds"].add(
            str(row["sourceFamilyGroupId"])
        )
        notify_system["elementIds"].append(str(row["id"]))

    visual_rows = []
    for key in sorted(visual_groups):
        row = visual_groups[key]
        for field in (
            "notifySystemTimingGroupIds",
            "notifyIds",
            "sourceSystemIds",
        ):
            row[field] = sorted(row[field])
        row["elementIds"] = sorted(row["elementIds"])
        row["elementCount"] = len(row["elementIds"])
        visual_rows.append(row)
    notify_system_rows = []
    for key in sorted(notify_system_groups):
        row = notify_system_groups[key]
        row["sourceFamilyGroupIds"] = sorted(row["sourceFamilyGroupIds"])
        row["elementIds"] = sorted(row["elementIds"])
        row["elementCount"] = len(row["elementIds"])
        notify_system_rows.append(row)
    return visual_rows, notify_system_rows


def validate_candidate_document(
    document: dict[str, Any], effect_asset_id: str
) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_asset_id
    ):
        raise CandidateError(f"candidate v13 identity is invalid: {effect_asset_id}")
    elements = document.get("elements")
    if not isinstance(elements, list) or not elements:
        raise CandidateError(f"candidate has no elements: {effect_asset_id}")
    ids = [str(row.get("id") or "") for row in elements]
    source_nodes = [str(row.get("sourceNode") or "") for row in elements]
    if (
        any(not value for value in ids + source_nodes)
        or len(ids) != len(set(ids))
        or len(source_nodes) != len(set(source_nodes))
    ):
        raise CandidateError(f"candidate element identity is invalid: {effect_asset_id}")
    if source_nodes != sorted(source_nodes):
        raise CandidateError(f"candidate elements are not deterministic: {effect_asset_id}")
    for element in elements:
        attachment = element.get("actionCueAttachment") or {}
        inheritance = element.get("transformInheritance") or {}
        if (
            attachment.get("enabled") is not False
            or attachment.get("follow") is not False
            or inheritance != {"enabled": False, "masterElementId": ""}
        ):
            raise CandidateError("candidate v13 transform ownership is invalid")
        recipe = element.get("sourceRecipe") or {}
        resources = element.get("resources") or []
        material = element.get("material") or {}
        delay = (
            (element.get("detail") or {})
            .get("timing", {})
            .get("startDelaySeconds")
        )
        if (
            recipe.get("enabled") is not True
            or not resources
            or not str(material.get("templateId") or "")
            or isinstance(delay, bool)
            or not isinstance(delay, (int, float))
            or not math.isfinite(float(delay))
            or float(delay) < 0.0
        ):
            raise CandidateError("candidate contains a non-executable projection")
        if not any(
            str(module.get("className") or "").casefold()
            == "particlemodulerequired"
            for module in recipe.get("modules", [])
        ):
            raise CandidateError("candidate sourceRecipe lost its Required module")


def _preservation_rows(existing: dict[str, Any]) -> list[dict[str, str]]:
    rows = []
    for element in existing.get("elements", []):
        element_id = str(element.get("id") or "")
        if source_inventory.is_legacy_generic_default(element):
            disposition = "REPORT_ONLY_GENERIC_DEFAULT_RETIRE_CANDIDATE_NO_DELETE"
        elif str(element.get("sourceNode") or ""):
            disposition = "PRESERVE_EXISTING_SOURCE_OR_IMPORTED_ROW"
        else:
            disposition = "PRESERVE_PROJECT_AUTHORED_OR_TUNED_ROW"
        rows.append({"elementId": element_id, "disposition": disposition})
    return rows


def compact_reconcile_plan(
    existing: dict[str, Any],
    candidate: dict[str, Any],
    candidate_path: Path,
    source_element_keys: list[dict[str, Any]],
) -> dict[str, Any]:
    plan = source_inventory.reconcile_effect_document(
        existing, candidate["elements"]
    )
    additions = plan.pop("addElements")
    if plan.get("deleteElements"):
        raise CandidateError("report-only reconcile unexpectedly deletes elements")
    full_by_compact = {
        str(row["sourceNode"]): str(row["fullSourceKey"])
        for row in source_element_keys
    }
    rebase_rows = [
        {
            "sourceNode": compact,
            "fullSourceKey": full_by_compact.get(compact, ""),
        }
        for compact in plan.pop("sourceRebaseRequired")
    ]
    preservation = _preservation_rows(existing)
    preservation_counts = Counter(row["disposition"] for row in preservation)
    plan.update(
        {
            "mode": "MISSING_ONLY_PRESERVE_EXISTING_REPORT_ONLY",
            "candidateDocumentPath": candidate_path.relative_to(ROOT).as_posix(),
            "addElementRefs": [
                {
                    "id": str(row["id"]),
                    "sourceNode": str(row["sourceNode"]),
                }
                for row in additions
            ],
            "sourceRebaseRequiredRows": rebase_rows,
            "preservedExistingRows": preservation,
            "preservationCounts": dict(sorted(preservation_counts.items())),
            "legacyRetirementDisposition": (
                "REPORT_ONLY_UNVERIFIED_DEFAULT_SIGNATURE_NO_DELETE"
            ),
            "applyDisposition": (
                "BLOCKED_SOURCE_REBASE_REQUIRED_NO_MUTATION"
                if rebase_rows
                else "REPORT_ONLY_MISSING_ONLY_READY_NO_MUTATION"
            ),
        }
    )
    return plan


def _aggregate_blocker(
    rows: dict[tuple[str, str, str], dict[str, Any]],
    occurrence: dict[str, Any],
    projection_count: int,
    reason: str,
    **extra: Any,
) -> None:
    key = (
        reason,
        str(occurrence.get("patternId") or ""),
        str(occurrence.get("clipOccurrenceId") or ""),
    )
    row = rows.get(key)
    if row is None:
        row = {
            "reason": reason,
            "patternId": key[1],
            "semanticStageId": str(occurrence.get("semanticStageId") or ""),
            "gameplayActionId": str(occurrence.get("gameplayActionId") or ""),
            "clipOccurrenceId": key[2],
            "sourceOccurrenceFullKeys": [],
            "executableCoreProjectionCount": 0,
            **copy.deepcopy(extra),
        }
        rows[key] = row
    row["sourceOccurrenceFullKeys"].append(str(occurrence["fullKey"]))
    row["executableCoreProjectionCount"] += projection_count


def _validate_expected_counts(summary: dict[str, Any]) -> None:
    changed = []
    for field, expected in EXPECTED_COUNTS.items():
        actual = summary.get(field)
        if actual != expected:
            changed.append(f"{field}={actual!r} (expected {expected!r})")
    if changed:
        raise CandidateError(
            "reviewed source candidate denominator drifted; review the exact "
            "sequence/cue/source projection before updating EXPECTED_COUNTS: "
            + "; ".join(changed)
        )


def _validate_source_denominator(inventory: dict[str, Any]) -> None:
    if (
        inventory.get("schema")
        != "lostark.valtan-source-occurrence-inventory"
        or inventory.get("formatVersion") != 1
    ):
        raise CandidateError("source occurrence inventory header is invalid")
    selections = inventory.get("reviewedBranchSelections", [])
    if any(row.get("status") != "REVIEWED_SELECTED" for row in selections):
        raise CandidateError("candidate input contains a non-selected review row")
    for system in inventory.get("sourceSystems", []):
        for carrier in system.get("carriers", []):
            if (
                carrier.get("disposition") == "EXECUTABLE_CORE"
                and carrier.get("elementSeed") is None
            ):
                raise CandidateError(
                    "EXECUTABLE_CORE carrier payload is absent; rebuild with "
                    "include_payloads=True"
                )


def load_inventory(selection_path: Path) -> dict[str, Any]:
    selection = source_inventory.load_selection_manifest(selection_path)
    return source_inventory.build_inventory(
        {"reviewedBranchSelections": selection["selections"]},
        include_payloads=True,
        additional_repository_sources=[selection_path],
    )


def build_candidates(
    selection_path: Path = SELECTION_PATH,
    output_root: Path = OUTPUT_ROOT,
    *,
    inventory_document: dict[str, Any] | None = None,
    cue_document: dict[str, Any] | None = None,
    catalog_document: dict[str, Any] | None = None,
    enforce_expected_counts: bool = True,
) -> tuple[dict[str, bytes], dict[str, Any]]:
    selection_path = selection_path.resolve()
    output_root = output_root.resolve()
    validate_output_root(output_root)
    inventory = inventory_document or load_inventory(selection_path)
    _validate_source_denominator(inventory)
    injected_cue_document = cue_document is not None
    injected_catalog_document = catalog_document is not None
    cue_document = cue_document or read_json(CUE_PATH)
    catalog_document = catalog_document or read_json(CATALOG_PATH)
    cues_by_clip = cue_index(cue_document)
    catalogs = catalog_index(catalog_document)
    systems = {
        str(row["sourceSystemId"]): {
            str(carrier["carrierKey"]): carrier
            for carrier in row.get("carriers", [])
        }
        for row in inventory.get("sourceSystems", [])
    }

    selected_branches = [
        row
        for row in inventory.get("branches", [])
        if row.get("selectionStatus") == "REVIEWED_SELECTED"
    ]
    reviewed_patterns = sorted({str(row["patternId"]) for row in selected_branches})
    reachable_occurrences = [
        row
        for row in inventory.get("occurrences", [])
        if row.get("reachabilityDisposition") == "REACHABLE_REVIEWED"
    ]

    effect_clip_groups: dict[
        str, dict[str, dict[str, Any]]
    ] = defaultdict(dict)
    unresolved_joins: dict[tuple[str, str, str], dict[str, Any]] = {}
    rejected_timing_rows: list[dict[str, Any]] = []
    protected_projection_rows: list[dict[str, Any]] = []
    projection_receipt_rows: list[dict[str, Any]] = []
    pattern_counts: dict[str, Counter[str]] = {
        pattern_id: Counter() for pattern_id in reviewed_patterns
    }
    reachable_core_projection_count = 0

    for occurrence in sorted(reachable_occurrences, key=lambda row: row["fullKey"]):
        pattern_id = str(occurrence["patternId"])
        pattern_counts[pattern_id]["reachableSourceOccurrenceCount"] += 1
        carriers = [
            carrier
            for carrier in systems.get(str(occurrence.get("sourceSystemId") or ""), {}).values()
            if carrier.get("disposition") == "EXECUTABLE_CORE"
        ]
        carriers.sort(key=lambda row: str(row["carrierKey"]))
        if not carriers:
            continue
        carrier_count = len(carriers)
        reachable_core_projection_count += carrier_count
        pattern_counts[pattern_id]["reachableCoreProjectionCount"] += carrier_count
        clip_occurrence_id = str(occurrence.get("clipOccurrenceId") or "")
        cue_rows = cues_by_clip.get(clip_occurrence_id, [])
        if not cue_rows:
            _aggregate_blocker(
                unresolved_joins,
                occurrence,
                carrier_count,
                "NO_V2_CUE_FOR_EXACT_CLIP_OCCURRENCE",
                cueRows=[],
            )
            pattern_counts[pattern_id]["missingCueProjectionCount"] += carrier_count
            continue
        if len(cue_rows) != 1:
            _aggregate_blocker(
                unresolved_joins,
                occurrence,
                carrier_count,
                "MULTIPLE_V2_CUES_FOR_EXACT_CLIP_OCCURRENCE",
                cueRows=copy.deepcopy(cue_rows),
            )
            pattern_counts[pattern_id]["multipleCueProjectionCount"] += carrier_count
            continue
        cue = cue_rows[0]
        if (
            str(cue.get("patternId") or "") != pattern_id
            or str(cue.get("actionId") or "")
            != str(occurrence.get("gameplayActionId") or "")
            or str(cue.get("stageId") or "")
            != str(occurrence.get("semanticStageId") or "")
        ):
            _aggregate_blocker(
                unresolved_joins,
                occurrence,
                carrier_count,
                "V2_CUE_EXACT_CLIP_IDENTITY_MISMATCH",
                cueRows=[copy.deepcopy(cue)],
            )
            pattern_counts[pattern_id]["cueIdentityMismatchProjectionCount"] += carrier_count
            continue
        effect_asset_id = str(cue["effectAssetId"])
        if effect_asset_id in PROTECTED_EFFECT_ASSET_IDS:
            protected_projection_rows.extend(
                {
                    "patternId": pattern_id,
                    "clipOccurrenceId": clip_occurrence_id,
                    "cueOccurrenceId": str(cue["occurrenceId"]),
                    "effectAssetId": effect_asset_id,
                    "occurrenceFullKey": str(occurrence["fullKey"]),
                    "carrierKey": str(carrier["carrierKey"]),
                    "disposition": "PROTECTED_BYTE_CANARY_NO_CANDIDATE_NO_APPLY",
                }
                for carrier in carriers
            )
            pattern_counts[pattern_id]["protectedCanaryProjectionCount"] += carrier_count
            continue

        clip_group = effect_clip_groups[effect_asset_id].setdefault(
            clip_occurrence_id,
            {
                "patternId": pattern_id,
                "semanticStageId": str(occurrence.get("semanticStageId") or ""),
                "gameplayActionId": str(occurrence.get("gameplayActionId") or ""),
                "clipOccurrenceId": clip_occurrence_id,
                "cueRow": copy.deepcopy(cue),
                "elements": [],
                "sourceElementKeys": [],
                "sourceOccurrenceFullKeys": set(),
                "projectionRows": [],
            },
        )
        for carrier in carriers:
            seed, projection = build_projection_seed(occurrence, carrier, cue)
            projection_receipt_rows.append(copy.deepcopy(projection))
            if seed is None:
                rejected_timing_rows.append(copy.deepcopy(projection))
                field = (
                    "negativeTimingProjectionCount"
                    if projection["disposition"] == "NEGATIVE_BEFORE_CUE_START"
                    else "outsideCueWindowProjectionCount"
                )
                pattern_counts[pattern_id][field] += 1
                continue
            clip_group["elements"].append(seed)
            clip_group["sourceElementKeys"].append(copy.deepcopy(projection))
            clip_group["sourceOccurrenceFullKeys"].add(
                str(occurrence["fullKey"])
            )
            clip_group["projectionRows"].append(copy.deepcopy(projection))
            pattern_counts[pattern_id]["admittedCoreProjectionCount"] += 1

    for row in unresolved_joins.values():
        row["sourceOccurrenceFullKeys"] = sorted(
            set(row["sourceOccurrenceFullKeys"])
        )
    unresolved_join_rows = sorted(
        unresolved_joins.values(),
        key=lambda row: (
            row["reason"], row["patternId"], row["clipOccurrenceId"]
        ),
    )
    rejected_timing_rows.sort(
        key=lambda row: (row["occurrenceFullKey"], row["carrierKey"])
    )
    protected_projection_rows.sort(
        key=lambda row: (row["occurrenceFullKey"], row["carrierKey"])
    )

    files: dict[str, bytes] = {}
    receipt_documents: list[dict[str, Any]] = []
    reuse_reviews: list[dict[str, Any]] = []
    reuse_divergence_projection_count = 0
    candidate_clip_ids: set[str] = set()
    candidate_patterns: set[str] = set()
    admitted_projection_count = 0
    reuse_collapsed_projection_count = 0

    for effect_asset_id in sorted(effect_clip_groups):
        clip_groups = [
            effect_clip_groups[effect_asset_id][key]
            for key in sorted(effect_clip_groups[effect_asset_id])
            if effect_clip_groups[effect_asset_id][key]["elements"]
        ]
        if not clip_groups:
            continue
        signatures = {
            group["clipOccurrenceId"]: _candidate_visual_signature(
                group["elements"]
            )
            for group in clip_groups
        }
        if len(clip_groups) > 1 and len(set(signatures.values())) != 1:
            projection_count = sum(len(row["elements"]) for row in clip_groups)
            reuse_divergence_projection_count += projection_count
            reuse_reviews.append(
                {
                    "effectAssetId": effect_asset_id,
                    "status": "DIVERGENT_SOURCE_FAMILIES_NO_CANDIDATE_NO_APPLY",
                    "clipOccurrenceIds": sorted(signatures),
                    "clipVisualSignatureSha256": dict(sorted(signatures.items())),
                    "executableCoreProjectionCount": projection_count,
                }
            )
            for group in clip_groups:
                pattern_counts[group["patternId"]][
                    "effectAssetReuseDivergenceProjectionCount"
                ] += len(group["elements"])
                pattern_counts[group["patternId"]][
                    "admittedCoreProjectionCount"
                ] -= len(group["elements"])
            continue

        canonical_group = clip_groups[0]
        if len(clip_groups) > 1:
            collapsed = sum(len(row["elements"]) for row in clip_groups[1:])
            reuse_collapsed_projection_count += collapsed
            reuse_reviews.append(
                {
                    "effectAssetId": effect_asset_id,
                    "status": "EQUIVALENT_SOURCE_FAMILY_REUSE_COLLAPSED",
                    "canonicalClipOccurrenceId": canonical_group[
                        "clipOccurrenceId"
                    ],
                    "clipOccurrenceIds": [
                        row["clipOccurrenceId"] for row in clip_groups
                    ],
                    "visualSignatureSha256": signatures[
                        canonical_group["clipOccurrenceId"]
                    ],
                    "collapsedProjectionCount": collapsed,
                }
            )
        catalog = catalogs.get(effect_asset_id)
        if catalog is None:
            for group in clip_groups:
                pattern_counts[group["patternId"]][
                    "catalogOrAuthoredBlockerProjectionCount"
                ] += len(group["elements"])
            unresolved_join_rows.append(
                {
                    "reason": "EFFECT_CATALOG_ROW_MISSING",
                    "patternId": canonical_group["patternId"],
                    "semanticStageId": canonical_group["semanticStageId"],
                    "gameplayActionId": canonical_group["gameplayActionId"],
                    "clipOccurrenceId": canonical_group["clipOccurrenceId"],
                    "sourceOccurrenceFullKeys": sorted(
                        set().union(
                            *(row["sourceOccurrenceFullKeys"] for row in clip_groups)
                        )
                    ),
                    "executableCoreProjectionCount": sum(
                        len(row["elements"]) for row in clip_groups
                    ),
                    "effectAssetId": effect_asset_id,
                    "cueRows": [copy.deepcopy(row["cueRow"]) for row in clip_groups],
                }
            )
            continue
        authored_path = authored_document_path(catalog)
        existing = read_json(authored_path)
        if (
            existing.get("schema") != "lostark.effect-authoring"
            or existing.get("version") != 13
            or existing.get("effectAssetId") != effect_asset_id
        ):
            raise CandidateError(
                f"existing authored v13 header drifted: {effect_asset_id}"
            )

        elements = sorted(
            copy.deepcopy(canonical_group["elements"]),
            key=lambda row: str(row["sourceNode"]),
        )
        candidate = {
            key: copy.deepcopy(value)
            for key, value in existing.items()
            if key != "elements"
        }
        candidate["elements"] = elements
        validate_candidate_document(candidate, effect_asset_id)
        candidate_path = output_root / candidate_filename(effect_asset_id)
        relative_path = candidate_path.relative_to(ROOT).as_posix()
        candidate_payload = source_inventory.pretty_json_bytes(candidate)
        files[relative_path] = candidate_payload

        source_element_keys = sorted(
            copy.deepcopy(canonical_group["sourceElementKeys"]),
            key=lambda row: str(row["sourceNode"]),
        )
        visual_timing_groups, notify_system_timing_groups = build_timing_groups(
            source_element_keys
        )
        reconcile = compact_reconcile_plan(
            existing, candidate, candidate_path, source_element_keys
        )
        clip_rows = []
        for ordinal, group in enumerate(clip_groups):
            disposition = (
                "CANONICAL_CANDIDATE_SOURCE"
                if ordinal == 0
                else "EQUIVALENT_EFFECT_ASSET_REUSE_NO_DUPLICATE_ELEMENTS"
            )
            clip_rows.append(
                {
                    "patternId": group["patternId"],
                    "semanticStageId": group["semanticStageId"],
                    "gameplayActionId": group["gameplayActionId"],
                    "clipOccurrenceId": group["clipOccurrenceId"],
                    "cueDisposition": "REUSE_EXISTING_V2_NO_MUTATION",
                    "cueRow": copy.deepcopy(group["cueRow"]),
                    "sourceOccurrenceCount": len(
                        group["sourceOccurrenceFullKeys"]
                    ),
                    "sourceOccurrenceFullKeys": sorted(
                        group["sourceOccurrenceFullKeys"]
                    ),
                    "sourceProjectionCount": len(group["elements"]),
                    "candidateProjectionDisposition": disposition,
                }
            )
            candidate_clip_ids.add(group["clipOccurrenceId"])
            candidate_patterns.add(group["patternId"])
        admitted_projection_count += sum(
            len(group["elements"]) for group in clip_groups
        )
        receipt_documents.append(
            {
                "effectAssetId": effect_asset_id,
                "candidateDocumentPath": relative_path,
                "candidateDocumentSha256": source_inventory.sha256_bytes(
                    candidate_payload
                ),
                "candidateElementCount": len(elements),
                "sourceElementKeys": source_element_keys,
                "visualTimingGroups": visual_timing_groups,
                "notifySystemTimingGroups": notify_system_timing_groups,
                "clipOccurrences": clip_rows,
                "catalogDisposition": "REUSE_EXISTING_NO_MUTATION",
                "catalogRow": copy.deepcopy(catalog),
                "authoredDocumentPath": authored_path.relative_to(ROOT).as_posix(),
                "authoredDocumentSha256": source_inventory.sha256_file(
                    authored_path
                ),
                "reconcile": reconcile,
            }
        )

    receipt_documents.sort(key=lambda row: row["effectAssetId"])
    unresolved_join_rows.sort(
        key=lambda row: (
            row["reason"],
            row.get("patternId", ""),
            row.get("clipOccurrenceId", ""),
        )
    )

    protected_canaries = []
    for effect_asset_id in sorted(PROTECTED_EFFECT_ASSET_IDS):
        catalog = catalogs.get(effect_asset_id)
        if catalog is None:
            raise CandidateError(f"protected Effect catalog row is missing: {effect_asset_id}")
        path = authored_document_path(catalog)
        protected_canaries.append(
            {
                "effectAssetId": effect_asset_id,
                "authoredDocumentPath": path.relative_to(ROOT).as_posix(),
                "authoredDocumentSha256": source_inventory.sha256_file(path),
                "sourceProjectionCount": sum(
                    row["effectAssetId"] == effect_asset_id
                    for row in protected_projection_rows
                ),
                "disposition": "PROTECTED_BYTE_CANARY_NO_CANDIDATE_NO_APPLY",
            }
        )

    pattern_coverage = []
    for pattern_id in reviewed_patterns:
        counts = pattern_counts[pattern_id]
        candidate_assets = {
            row["effectAssetId"]
            for row in receipt_documents
            if any(
                clip["patternId"] == pattern_id
                for clip in row["clipOccurrences"]
            )
        }
        counts["candidateEffectAssetCount"] = len(candidate_assets)
        if candidate_assets:
            status = "IMMUTABLE_CANDIDATES_EMITTED"
        elif counts["reachableCoreProjectionCount"] == 0:
            status = "NO_REACHABLE_EXECUTABLE_CORE_PROJECTION"
        else:
            status = "ALL_CORE_PROJECTIONS_EXPLICITLY_BLOCKED_OR_EXCLUDED"
        pattern_coverage.append(
            {
                "patternId": pattern_id,
                "status": status,
                **dict(sorted(counts.items())),
            }
        )

    summary = {
        "reviewedSelectedBranchCount": len(selected_branches),
        "reviewedSelectedPatternCount": len(reviewed_patterns),
        "reachableSourceOccurrenceCount": len(reachable_occurrences),
        "reachableCoreProjectionCount": reachable_core_projection_count,
        "candidateDocumentCount": len(receipt_documents),
        "candidateClipOccurrenceCount": len(candidate_clip_ids),
        "candidatePatternCount": len(candidate_patterns),
        "admittedCoreProjectionCount": admitted_projection_count,
        "candidateElementCount": sum(
            row["candidateElementCount"] for row in receipt_documents
        ),
        "effectAssetEquivalentReuseCount": sum(
            row["status"] == "EQUIVALENT_SOURCE_FAMILY_REUSE_COLLAPSED"
            for row in reuse_reviews
        ),
        "effectAssetReuseCollapsedProjectionCount": reuse_collapsed_projection_count,
        "effectAssetReuseDivergenceCount": sum(
            row["status"] == "DIVERGENT_SOURCE_FAMILIES_NO_CANDIDATE_NO_APPLY"
            for row in reuse_reviews
        ),
        "effectAssetReuseDivergenceProjectionCount": reuse_divergence_projection_count,
        "protectedCanaryProjectionCount": len(protected_projection_rows),
        "missingCueProjectionCount": sum(
            row["executableCoreProjectionCount"]
            for row in unresolved_join_rows
            if row["reason"] == "NO_V2_CUE_FOR_EXACT_CLIP_OCCURRENCE"
        ),
        "multipleCueProjectionCount": sum(
            row["executableCoreProjectionCount"]
            for row in unresolved_join_rows
            if row["reason"] == "MULTIPLE_V2_CUES_FOR_EXACT_CLIP_OCCURRENCE"
        ),
        "cueIdentityMismatchProjectionCount": sum(
            row["executableCoreProjectionCount"]
            for row in unresolved_join_rows
            if row["reason"] == "V2_CUE_EXACT_CLIP_IDENTITY_MISMATCH"
        ),
        "negativeTimingProjectionCount": sum(
            row["disposition"] == "NEGATIVE_BEFORE_CUE_START"
            for row in rejected_timing_rows
        ),
        "outsideCueWindowProjectionCount": sum(
            row["disposition"] == "OUTSIDE_EXPLICIT_CUE_WINDOW"
            for row in rejected_timing_rows
        ),
        "preservedAuthoredElementCount": sum(
            row["reconcile"]["preservedExistingElementCount"]
            for row in receipt_documents
        ),
        "preservedExistingSourceOrImportedRowCount": sum(
            row["reconcile"]["preservationCounts"].get(
                "PRESERVE_EXISTING_SOURCE_OR_IMPORTED_ROW", 0
            )
            for row in receipt_documents
        ),
        "preservedProjectAuthoredOrTunedRowCount": sum(
            row["reconcile"]["preservationCounts"].get(
                "PRESERVE_PROJECT_AUTHORED_OR_TUNED_ROW", 0
            )
            for row in receipt_documents
        ),
        "legacyGenericRetireCandidateCount": sum(
            len(row["reconcile"]["legacyGenericRetireCandidates"])
            for row in receipt_documents
        ),
        "missingOnlyAddElementCount": sum(
            len(row["reconcile"]["addElementRefs"])
            for row in receipt_documents
        ),
        "sourceRebaseRequiredCount": sum(
            len(row["reconcile"]["sourceRebaseRequiredRows"])
            for row in receipt_documents
        ),
        "deletedElementCount": 0,
    }
    if enforce_expected_counts:
        _validate_expected_counts(summary)

    source_projection_hash_rows = sorted(
        projection_receipt_rows
        + protected_projection_rows
        + [
            {
                "reason": row["reason"],
                "patternId": row["patternId"],
                "clipOccurrenceId": row["clipOccurrenceId"],
                "sourceOccurrenceFullKeys": row["sourceOccurrenceFullKeys"],
                "executableCoreProjectionCount": row[
                    "executableCoreProjectionCount"
                ],
            }
            for row in unresolved_join_rows
        ],
        key=source_inventory.canonical_sha256,
    )
    receipt = {
        "schema": "lostark.valtan-reviewed-source-family-candidates",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "mode": "IMMUTABLE_IMPORTED_CANDIDATES_AND_REPORT_ONLY_RECONCILE",
        "scope": {
            "selectedBranchStatus": "REVIEWED_SELECTED",
            "reachabilityDisposition": "REACHABLE_REVIEWED",
            "carrierDisposition": "EXECUTABLE_CORE",
            "candidateEffectDocumentVersion": 13,
            "canonicalMutationDisposition": "NONE",
            "protectedEffectAssetIds": sorted(PROTECTED_EFFECT_ASSET_IDS),
        },
        "sources": {
            "selectionManifest": {
                "path": selection_path.relative_to(ROOT).as_posix(),
                "sha256": source_inventory.sha256_file(selection_path),
            },
            "cueDocument": {
                "path": CUE_PATH.relative_to(ROOT).as_posix(),
                "sha256": (
                    source_inventory.sha256_bytes(
                        source_inventory.pretty_json_bytes(cue_document)
                    )
                    if injected_cue_document
                    else source_inventory.sha256_file(CUE_PATH)
                ),
                "formatVersion": 2,
            },
            "effectCatalog": {
                "path": CATALOG_PATH.relative_to(ROOT).as_posix(),
                "sha256": (
                    source_inventory.sha256_bytes(
                        source_inventory.pretty_json_bytes(catalog_document)
                    )
                    if injected_catalog_document
                    else source_inventory.sha256_file(CATALOG_PATH)
                ),
                "formatVersion": 1,
            },
            "sourceInventoryRepositorySources": copy.deepcopy(
                (inventory.get("sources") or {}).get("repository", [])
            ),
            "sourceProjectionSha256": source_inventory.canonical_sha256(
                source_projection_hash_rows
            ),
        },
        "protectedCanaries": protected_canaries,
        "patternCoverage": pattern_coverage,
        "unresolvedCueJoins": unresolved_join_rows,
        "rejectedTimingProjections": rejected_timing_rows,
        "protectedCanaryProjections": protected_projection_rows,
        "effectAssetReuseReviews": reuse_reviews,
        "documents": receipt_documents,
        "summary": summary,
    }
    validate_receipt(receipt)
    receipt_path = output_root / RECEIPT_NAME
    files[receipt_path.relative_to(ROOT).as_posix()] = (
        source_inventory.pretty_json_bytes(receipt)
    )
    return files, receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    if (
        receipt.get("schema")
        != "lostark.valtan-reviewed-source-family-candidates"
        or receipt.get("formatVersion") != 1
        or receipt.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise CandidateError("reviewed source candidate receipt header is invalid")
    documents = receipt.get("documents")
    if not isinstance(documents, list):
        raise CandidateError("reviewed source candidate documents are invalid")
    effect_ids = [str(row.get("effectAssetId") or "") for row in documents]
    paths = [str(row.get("candidateDocumentPath") or "") for row in documents]
    if (
        any(not value for value in effect_ids + paths)
        or effect_ids != sorted(effect_ids)
        or len(effect_ids) != len(set(effect_ids))
        or len(paths) != len(set(paths))
        or set(effect_ids) & PROTECTED_EFFECT_ASSET_IDS
    ):
        raise CandidateError("candidate document identity or canary exclusion failed")
    for row in documents:
        if row.get("catalogDisposition") != "REUSE_EXISTING_NO_MUTATION":
            raise CandidateError("candidate catalog disposition is mutable")
        if not SHA256_RE.fullmatch(str(row.get("candidateDocumentSha256") or "")):
            raise CandidateError("candidate SHA-256 is invalid")
        reconcile = row.get("reconcile") or {}
        if reconcile.get("deleteElements") != []:
            raise CandidateError("candidate reconcile contains a deletion")
        source_element_ids = {
            str(element.get("id") or "")
            for element in row.get("sourceElementKeys", [])
        }
        visual_element_ids = {
            str(element_id)
            for group in row.get("visualTimingGroups", [])
            for element_id in group.get("elementIds", [])
        }
        notify_system_element_ids = {
            str(element_id)
            for group in row.get("notifySystemTimingGroups", [])
            for element_id in group.get("elementIds", [])
        }
        if (
            not source_element_ids
            or source_element_ids != visual_element_ids
            or source_element_ids != notify_system_element_ids
            or len(source_element_ids) != row.get("candidateElementCount")
        ):
            raise CandidateError(
                "candidate notify/system timing group closure is incomplete"
            )
        if any(
            clip.get("cueDisposition") != "REUSE_EXISTING_V2_NO_MUTATION"
            for clip in row.get("clipOccurrences", [])
        ):
            raise CandidateError("candidate cue disposition is mutable")
    summary = receipt.get("summary") or {}
    if summary.get("deletedElementCount") != 0:
        raise CandidateError("receipt reports an authored deletion")
    accounted = (
        int(summary.get("admittedCoreProjectionCount", 0))
        + int(summary.get("protectedCanaryProjectionCount", 0))
        + int(summary.get("missingCueProjectionCount", 0))
        + int(summary.get("multipleCueProjectionCount", 0))
        + int(summary.get("cueIdentityMismatchProjectionCount", 0))
        + int(summary.get("negativeTimingProjectionCount", 0))
        + int(summary.get("outsideCueWindowProjectionCount", 0))
        + int(summary.get("effectAssetReuseDivergenceProjectionCount", 0))
    )
    if accounted != summary.get("reachableCoreProjectionCount"):
        raise CandidateError(
            "reachable core projection denominator is not fully accounted"
        )


def check_exact(path: Path, payload: bytes) -> None:
    if not path.is_file():
        raise CandidateError(f"reviewed source candidate is missing: {path}")
    actual = path.read_bytes()
    if actual != payload:
        raise CandidateError(
            f"reviewed source candidate drifted: {path}\n"
            f"  expected {source_inventory.sha256_bytes(payload)}\n"
            f"  actual   {source_inventory.sha256_bytes(actual)}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--selection-manifest", type=Path, default=SELECTION_PATH)
    parser.add_argument("--output-root", type=Path, default=OUTPUT_ROOT)
    args = parser.parse_args()

    selection_path = args.selection_manifest.resolve()
    output_root = args.output_root.resolve()
    files, receipt = build_candidates(selection_path, output_root)
    if args.write:
        for relative_path, payload in sorted(files.items()):
            source_inventory.write_atomic(ROOT / relative_path, payload)
        label = "written"
    elif args.check:
        for relative_path, payload in sorted(files.items()):
            check_exact(ROOT / relative_path, payload)
        label = "checked"
    else:
        label = "dry-run"
    print(
        "Valtan reviewed source family candidates "
        f"{label}: {json.dumps(receipt['summary'], sort_keys=True)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (
        CandidateError,
        source_inventory.InventoryError,
        OSError,
        ValueError,
    ) as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
