#!/usr/bin/env python3
"""Validate the live Product successor to the immutable Carrier V1 receipt.

The Carrier V1 materialization receipt is a one-shot historical seal.  This
module projects explicitly reviewed live owner changes back to that seal and
validates the separate element-lineage receipt for Product-authored successor
documents.  It never writes either receipt or an authored Effect document.
"""

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
from typing import Any


class SuccessorLineageError(RuntimeError):
    pass


ALLOWED_DISPOSITIONS = frozenset(
    {"RETAIN", "REPLACED_BY", "INTENTIONALLY_REMOVED"}
)


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    )
    payload = (payload + "\n").encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise SuccessorLineageError(
            f"cannot read successor JSON: {path}: {error}"
        ) from error
    if not isinstance(value, dict):
        raise SuccessorLineageError(
            f"successor JSON root is not an object: {path}"
        )
    return value


def _strict_equal(left: Any, right: Any) -> bool:
    if type(left) is not type(right):
        return False
    if isinstance(left, dict):
        return left.keys() == right.keys() and all(
            _strict_equal(left[key], right[key]) for key in left
        )
    if isinstance(left, list):
        return len(left) == len(right) and all(
            _strict_equal(left_value, right_value)
            for left_value, right_value in zip(left, right, strict=True)
        )
    return left == right


def _object_rows(value: Any, label: str) -> list[dict[str, Any]]:
    if not isinstance(value, list) or any(
        not isinstance(row, dict) for row in value
    ):
        raise SuccessorLineageError(f"{label} rows are invalid")
    return value


def _unique_index(
    rows: list[dict[str, Any]], key: str, label: str
) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        identity = str(row.get(key) or "")
        if not identity or identity in result:
            raise SuccessorLineageError(
                f"{label} has duplicate/empty {key}: {identity}"
            )
        result[identity] = row
    return result


def _projection(receipt: dict[str, Any], name: str) -> dict[str, Any]:
    projections = receipt.get("historicalInputProjection")
    value = (projections or {}).get(name)
    if not isinstance(value, dict):
        raise SuccessorLineageError(
            f"successor receipt has no {name} historical projection"
        )
    return value


def _output_projection(receipt: dict[str, Any], name: str) -> dict[str, Any]:
    projections = receipt.get("historicalOutputProjection")
    value = (projections or {}).get(name)
    if not isinstance(value, dict):
        raise SuccessorLineageError(
            f"successor receipt has no {name} historical output projection"
        )
    return value


def _repository_path(root: Path, value: Any, label: str) -> Path:
    relative = str(value or "")
    if (
        not relative
        or "\\" in relative
        or ":" in relative
        or Path(relative).is_absolute()
    ):
        raise SuccessorLineageError(f"{label} path is invalid")
    root_resolved = root.resolve()
    path = (root_resolved / relative).resolve()
    if root_resolved not in path.parents:
        raise SuccessorLineageError(f"{label} path escapes repository")
    return path


def project_historical_pattern_bindings(
    document: dict[str, Any], receipt: dict[str, Any]
) -> dict[str, Any]:
    projection = _projection(receipt, "patternBindings")
    rows = copy.deepcopy(
        _object_rows(document.get("bindings"), "live pattern binding")
    )
    if document.get("formatVersion") != 2:
        raise SuccessorLineageError(
            "live pattern binding formatVersion drifted"
        )
    _unique_index(rows, "actionId", "live pattern binding")

    consumed_live_ids: set[str] = set()
    for transfer in _object_rows(
        projection.get("ownerTransfers"), "pattern binding owner transfer"
    ):
        transfer_id = str(transfer.get("transferId") or "")
        live_rows = _object_rows(
            transfer.get("liveRows"), f"{transfer_id} live binding"
        )
        historical_rows = _object_rows(
            transfer.get("historicalRows"),
            f"{transfer_id} historical binding",
        )
        live_ids = [str(row.get("actionId") or "") for row in live_rows]
        if (
            not transfer_id
            or not live_ids
            or any(not value for value in live_ids)
            or consumed_live_ids.intersection(live_ids)
        ):
            raise SuccessorLineageError(
                f"pattern binding owner transfer identity drifted: {transfer_id}"
            )
        locations = [
            index
            for index, row in enumerate(rows)
            if str(row.get("actionId") or "") in set(live_ids)
        ]
        actual_rows = [rows[index] for index in locations]
        if len(locations) != len(live_rows) or not _strict_equal(
            actual_rows, live_rows
        ):
            raise SuccessorLineageError(
                f"live pattern binding successor drifted: {transfer_id}"
            )
        insert_at = min(locations)
        rows = [
            row
            for row in rows
            if str(row.get("actionId") or "") not in set(live_ids)
        ]
        for offset, row in enumerate(historical_rows):
            rows.insert(insert_at + offset, copy.deepcopy(row))
        consumed_live_ids.update(live_ids)

    projected = copy.deepcopy(document)
    projected["bindings"] = rows
    _unique_index(rows, "actionId", "historical pattern binding projection")
    clip_count = sum(
        len(_object_rows(row.get("clips"), "projected pattern clip"))
        for row in rows
    )
    if (
        projection.get("bindingCount") != len(rows)
        or projection.get("clipOccurrenceCount") != clip_count
        or projection.get("canonicalSha256")
        != canonical_sha256(projected)
    ):
        raise SuccessorLineageError(
            "historical pattern binding projection seal drifted"
        )
    return projected


def _selection_identity(row: dict[str, Any], label: str) -> tuple[str, int]:
    pattern_id = str(row.get("patternId") or "")
    source_action_id = row.get("sourceActionId")
    if not pattern_id or type(source_action_id) is not int:
        raise SuccessorLineageError(f"{label} identity is invalid")
    return pattern_id, source_action_id


def project_historical_selection_manifest(
    document: dict[str, Any], receipt: dict[str, Any]
) -> dict[str, Any]:
    projection = _projection(receipt, "selectionManifest")
    if (
        document.get("schema")
        != "lostark.valtan-source-branch-selections"
        or document.get("formatVersion") != 1
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise SuccessorLineageError("live selection manifest header drifted")
    rows = copy.deepcopy(
        _object_rows(document.get("selections"), "live source selection")
    )
    identities = [
        _selection_identity(row, "live source selection") for row in rows
    ]
    if len(identities) != len(set(identities)):
        raise SuccessorLineageError(
            "live source selection identity is duplicated"
        )

    consumed: set[tuple[str, int]] = set()
    for transfer in _object_rows(
        projection.get("ownerTransfers"), "selection owner transfer"
    ):
        transfer_id = str(transfer.get("transferId") or "")
        live_row = transfer.get("liveRow")
        historical_row = transfer.get("historicalRow")
        if not isinstance(live_row, dict) or not isinstance(
            historical_row, dict
        ):
            raise SuccessorLineageError(
                f"selection owner transfer row is invalid: {transfer_id}"
            )
        identity = _selection_identity(live_row, transfer_id)
        if (
            not transfer_id
            or identity in consumed
            or _selection_identity(historical_row, transfer_id) != identity
        ):
            raise SuccessorLineageError(
                f"selection owner transfer identity drifted: {transfer_id}"
            )
        locations = [
            index
            for index, row in enumerate(rows)
            if _selection_identity(row, "live source selection") == identity
        ]
        if len(locations) != 1 or not _strict_equal(
            rows[locations[0]], live_row
        ):
            raise SuccessorLineageError(
                f"live source selection successor drifted: {transfer_id}"
            )
        rows[locations[0]] = copy.deepcopy(historical_row)
        consumed.add(identity)

    projected = copy.deepcopy(document)
    projected["selections"] = rows
    if (
        projection.get("selectionCount") != len(rows)
        or projection.get("canonicalSha256")
        != canonical_sha256(projected)
    ):
        raise SuccessorLineageError(
            "historical source selection projection seal drifted"
        )
    return projected


_OWNER_FIELDS = ("patternId", "semanticStageId", "gameplayActionId")


def _owner_tuple(owner: Any, label: str) -> tuple[str, str, str]:
    if not isinstance(owner, dict) or set(owner) != set(_OWNER_FIELDS):
        raise SuccessorLineageError(f"{label} owner is invalid")
    values = tuple(str(owner.get(field) or "") for field in _OWNER_FIELDS)
    if any(not value for value in values):
        raise SuccessorLineageError(f"{label} owner is empty")
    return values


def _project_historical_owner_rows(
    rows_value: Any,
    receipt: dict[str, Any],
    projection_name: str,
) -> list[dict[str, Any]]:
    projection = _output_projection(receipt, projection_name)
    rows = copy.deepcopy(_object_rows(rows_value, projection_name))
    original_owners = [
        tuple(str(row.get(field) or "") for field in _OWNER_FIELDS)
        for row in rows
    ]
    consumed_live_owners: set[tuple[str, str, str]] = set()
    consumed_indices: set[int] = set()
    for transfer in _object_rows(
        projection.get("ownerTransfers"),
        f"{projection_name} owner transfer",
    ):
        transfer_id = str(transfer.get("transferId") or "")
        live_owner = _owner_tuple(
            transfer.get("liveOwner"), f"{transfer_id} live"
        )
        historical_owner = _owner_tuple(
            transfer.get("historicalOwner"), f"{transfer_id} historical"
        )
        expected_count = transfer.get("rowCount")
        matches = [
            index
            for index, owner in enumerate(original_owners)
            if owner == live_owner
        ]
        if (
            not transfer_id
            or live_owner in consumed_live_owners
            or type(expected_count) is not int
            or expected_count <= 0
            or len(matches) != expected_count
            or consumed_indices.intersection(matches)
        ):
            raise SuccessorLineageError(
                f"{projection_name} owner transfer drifted: {transfer_id}"
            )
        for index in matches:
            for field, value in zip(
                _OWNER_FIELDS, historical_owner, strict=True
            ):
                rows[index][field] = value
        consumed_live_owners.add(live_owner)
        consumed_indices.update(matches)

    if (
        projection.get("rowCount") != len(rows)
        or projection.get("canonicalSha256") != canonical_sha256(rows)
    ):
        raise SuccessorLineageError(
            f"historical {projection_name} projection seal drifted"
        )
    return rows


def project_historical_reviewed_projection_ledger(
    rows: Any, receipt: dict[str, Any]
) -> list[dict[str, Any]]:
    return _project_historical_owner_rows(
        rows, receipt, "reviewedProjectionLedger"
    )


def project_historical_reviewed_source_only_occurrences(
    rows: Any, receipt: dict[str, Any]
) -> list[dict[str, Any]]:
    return _project_historical_owner_rows(
        rows, receipt, "reviewedSourceOnlyOccurrences"
    )


def project_historical_encounter(
    document: dict[str, Any], receipt: dict[str, Any]
) -> dict[str, Any]:
    projection = _projection(receipt, "encounter")
    rows = copy.deepcopy(
        _object_rows(document.get("patterns"), "live encounter pattern")
    )
    _unique_index(rows, "patternId", "live encounter pattern")
    consumed_live_ids: set[str] = set()
    for transfer in _object_rows(
        projection.get("ownerTransfers"), "encounter owner transfer"
    ):
        transfer_id = str(transfer.get("transferId") or "")
        live_rows = _object_rows(
            transfer.get("liveRows"), f"{transfer_id} live encounter"
        )
        historical_rows = _object_rows(
            transfer.get("historicalRows"),
            f"{transfer_id} historical encounter",
        )
        live_ids = [str(row.get("patternId") or "") for row in live_rows]
        live_id_set = set(live_ids)
        if (
            not transfer_id
            or not live_ids
            or len(live_ids) != len(live_id_set)
            or any(not value for value in live_ids)
            or consumed_live_ids.intersection(live_id_set)
        ):
            raise SuccessorLineageError(
                f"encounter owner transfer identity drifted: {transfer_id}"
            )
        locations = [
            index
            for index, row in enumerate(rows)
            if str(row.get("patternId") or "") in live_id_set
        ]
        actual_rows = [rows[index] for index in locations]
        if len(locations) != len(live_rows) or not _strict_equal(
            actual_rows, live_rows
        ):
            raise SuccessorLineageError(
                f"live encounter successor drifted: {transfer_id}"
            )
        insert_at = min(locations)
        rows = [
            row
            for row in rows
            if str(row.get("patternId") or "") not in live_id_set
        ]
        for offset, row in enumerate(historical_rows):
            rows.insert(insert_at + offset, copy.deepcopy(row))
        consumed_live_ids.update(live_id_set)

    for insertion in _object_rows(
        projection.get("historicalInsertions"),
        "encounter historical insertion",
    ):
        ordinal = insertion.get("ordinal")
        row = insertion.get("historicalRow")
        if (
            type(ordinal) is not int
            or ordinal < 0
            or ordinal > len(rows)
            or not isinstance(row, dict)
            or not str(row.get("patternId") or "")
            or any(
                existing.get("patternId") == row.get("patternId")
                for existing in rows
            )
        ):
            raise SuccessorLineageError(
                "encounter historical insertion drifted"
            )
        rows.insert(ordinal, copy.deepcopy(row))

    projected = copy.deepcopy(document)
    projected["patterns"] = rows
    _unique_index(rows, "patternId", "historical encounter projection")
    if (
        projection.get("patternCount") != len(rows)
        or projection.get("canonicalSha256")
        != canonical_sha256(projected)
    ):
        raise SuccessorLineageError(
            "historical encounter projection seal drifted"
        )
    return projected


def project_historical_catalog(
    catalog: dict[str, Any], receipt: dict[str, Any]
) -> dict[str, Any]:
    projection = _projection(receipt, "catalog")
    rows = _object_rows(catalog.get("effects"), "live Effect catalog")
    _unique_index(rows, "effectAssetId", "live Effect catalog")
    excluded_rows = _object_rows(
        projection.get("excludedLiveRows"), "live-only catalog"
    )
    excluded = _unique_index(
        excluded_rows, "effectAssetId", "live-only catalog"
    )
    actual = {
        str(row.get("effectAssetId") or ""): row
        for row in rows
        if str(row.get("effectAssetId") or "") in excluded
    }
    if not _strict_equal(actual, excluded):
        raise SuccessorLineageError("live-only Valtan catalog rows drifted")
    projected_rows = [
        copy.deepcopy(row)
        for row in rows
        if str(row.get("effectAssetId") or "").startswith("effect.valtan.")
        and str(row.get("effectAssetId") or "") not in excluded
    ]
    projected_rows.sort(key=lambda row: str(row["effectAssetId"]))
    projected = {
        "formatVersion": catalog.get("formatVersion"),
        "effectAssetIdPrefix": "effect.valtan.",
        "effects": projected_rows,
    }
    if (
        projection.get("effectCount") != len(projected_rows)
        or projection.get("canonicalSha256")
        != canonical_sha256(projected)
    ):
        raise SuccessorLineageError(
            "historical Valtan catalog projection seal drifted"
        )
    return projected


def live_only_catalog_rows(
    receipt: dict[str, Any],
) -> list[dict[str, Any]]:
    return copy.deepcopy(
        _object_rows(
            _projection(receipt, "catalog").get("excludedLiveRows"),
            "live-only catalog",
        )
    )


def project_historical_cues(
    document: dict[str, Any], receipt: dict[str, Any]
) -> dict[str, Any]:
    projection = _projection(receipt, "cues")
    rows = copy.deepcopy(_object_rows(document.get("cues"), "live cue"))
    _unique_index(rows, "bindingId", "live cue")
    excluded_rows = _object_rows(
        projection.get("excludedLiveRows"), "live-only cue"
    )
    excluded = _unique_index(excluded_rows, "bindingId", "live-only cue")
    actual_excluded = {
        str(row.get("bindingId") or ""): row
        for row in rows
        if str(row.get("bindingId") or "") in excluded
    }
    if not _strict_equal(actual_excluded, excluded):
        raise SuccessorLineageError("live-only Valtan cue rows drifted")
    rows = [
        row
        for row in rows
        if str(row.get("bindingId") or "") not in excluded
    ]
    by_id = _unique_index(rows, "bindingId", "projected cue")
    for rebound in _object_rows(
        projection.get("rowRebounds"), "cue rebound"
    ):
        live_row = rebound.get("liveRow")
        historical_row = rebound.get("historicalRow")
        if not isinstance(live_row, dict) or not isinstance(
            historical_row, dict
        ):
            raise SuccessorLineageError("cue rebound row is invalid")
        binding_id = str(live_row.get("bindingId") or "")
        if (
            not binding_id
            or str(historical_row.get("bindingId") or "") != binding_id
            or binding_id not in by_id
            or not _strict_equal(by_id[binding_id], live_row)
        ):
            raise SuccessorLineageError(
                f"live cue rebound drifted: {binding_id}"
            )
        by_id[binding_id] = copy.deepcopy(historical_row)
    projected = copy.deepcopy(document)
    projected["cues"] = sorted(
        by_id.values(), key=lambda row: str(row["bindingId"])
    )
    if (
        projection.get("cueCount") != len(projected["cues"])
        or projection.get("canonicalSha256")
        != canonical_sha256(projected)
    ):
        raise SuccessorLineageError(
            "historical Valtan cue projection seal drifted"
        )
    return projected


def successor_documents(
    receipt: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    rows = _object_rows(receipt.get("successorDocuments"), "successor document")
    return _unique_index(rows, "effectAssetId", "successor document")


def _historical_target_index(
    historical_receipt: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    outputs = historical_receipt.get("outputs") or {}
    rows = _object_rows(outputs.get("targetDocuments"), "historical target")
    return _unique_index(rows, "effectAssetId", "historical target")


def _historical_source_nodes(
    historical_receipt: dict[str, Any], effect_id: str
) -> set[str]:
    rows = _object_rows(
        historical_receipt.get("sourceElements"), "historical source element"
    )
    result = {
        str(row.get("sourceNode") or "")
        for row in rows
        if row.get("effectAssetId") == effect_id
    }
    if "" in result:
        raise SuccessorLineageError(
            f"historical source identity is empty: {effect_id}"
        )
    return result


def _validate_document_lineage(
    *,
    entry: dict[str, Any],
    document: dict[str, Any],
    historical_receipt: dict[str, Any],
) -> None:
    effect_id = str(entry.get("effectAssetId") or "")
    if document.get("effectAssetId") != effect_id:
        raise SuccessorLineageError(
            f"successor document identity drifted: {effect_id}"
        )
    elements = _object_rows(document.get("elements"), f"{effect_id} element")
    elements_by_id = _unique_index(elements, "id", f"{effect_id} element")
    final = entry.get("finalDocument") or {}
    if (
        final.get("elementCount") != len(elements)
        or final.get("canonicalSha256") != canonical_sha256(document)
    ):
        raise SuccessorLineageError(
            f"successor final document seal drifted: {effect_id}"
        )

    historical = entry.get("historicalBaseline") or {}
    historical_targets = _historical_target_index(historical_receipt)
    historical_target = historical_targets.get(effect_id)
    is_target = historical_target is not None
    if is_target:
        if (
            historical.get("elementCount")
            != historical_target.get("elementCount")
            or historical.get("canonicalSha256")
            != historical_target.get("canonicalSha256")
        ):
            raise SuccessorLineageError(
                f"historical target seal rebound: {effect_id}"
            )
        baseline_identities = _historical_source_nodes(
            historical_receipt, effect_id
        )
    else:
        exceptions = {
            str(row.get("effectAssetId") or ""): row
            for row in _object_rows(
                historical_receipt.get("ownershipExceptions"),
                "historical ownership exception",
            )
        }
        old = exceptions.get(effect_id)
        baseline_identities = set(historical.get("baselineElementIds") or [])
        if (
            old is None
            or historical.get("elementCount") != old.get("elementCount")
            or historical.get("canonicalSha256")
            != old.get("documentCanonicalSha256")
            or len(baseline_identities) != historical.get("elementCount")
            or any(not value for value in baseline_identities)
        ):
            raise SuccessorLineageError(
                f"historical ownership exception rebound: {effect_id}"
            )

    lineage_rows = _object_rows(entry.get("lineage"), f"{effect_id} lineage")
    lineage_by_baseline = _unique_index(
        lineage_rows, "baselineIdentity", f"{effect_id} lineage"
    )
    if set(lineage_by_baseline) != baseline_identities:
        raise SuccessorLineageError(
            f"historical baseline denominator is not closed: {effect_id}"
        )

    mapped_successors: set[str] = set()
    for baseline, row in lineage_by_baseline.items():
        disposition = str(row.get("disposition") or "")
        successors = row.get("successorElementIds")
        reason = str(row.get("reason") or "")
        if (
            disposition not in ALLOWED_DISPOSITIONS
            or not isinstance(successors, list)
            or any(not isinstance(value, str) or not value for value in successors)
            or len(successors) != len(set(successors))
            or any(value not in elements_by_id for value in successors)
        ):
            raise SuccessorLineageError(
                f"successor mapping is invalid: {effect_id}/{baseline}"
            )
        if disposition == "RETAIN":
            if len(successors) != 1:
                raise SuccessorLineageError(
                    f"retained baseline must have one successor: {effect_id}/{baseline}"
                )
            element = elements_by_id[successors[0]]
            identity = (
                str(element.get("sourceNode") or "")
                if is_target
                else str(element.get("id") or "")
            )
            if identity != baseline:
                raise SuccessorLineageError(
                    f"retained baseline identity drifted: {effect_id}/{baseline}"
                )
        elif disposition == "REPLACED_BY":
            if not successors or not reason:
                raise SuccessorLineageError(
                    f"replacement mapping is incomplete: {effect_id}/{baseline}"
                )
        elif successors or not reason:
            raise SuccessorLineageError(
                f"intentional removal is incomplete: {effect_id}/{baseline}"
            )
        mapped_successors.update(successors)

    introduced_rows = _object_rows(
        entry.get("introducedElements"), f"{effect_id} introduced element"
    )
    introduced_by_id = _unique_index(
        introduced_rows,
        "successorElementId",
        f"{effect_id} introduced element",
    )
    if any(
        value not in elements_by_id
        or not str(row.get("reason") or "")
        for value, row in introduced_by_id.items()
    ):
        raise SuccessorLineageError(
            f"introduced successor identity drifted: {effect_id}"
        )
    if mapped_successors.intersection(introduced_by_id):
        raise SuccessorLineageError(
            f"successor is both mapped and introduced: {effect_id}"
        )
    if mapped_successors | set(introduced_by_id) != set(elements_by_id):
        raise SuccessorLineageError(
            f"live successor denominator is not closed: {effect_id}"
        )


def validate_receipt(
    *,
    root: Path,
    receipt: dict[str, Any],
    historical_receipt: dict[str, Any],
    catalog: dict[str, Any],
    cues: dict[str, Any],
) -> None:
    if (
        receipt.get("schema")
        != "lostark.valtan-carrier-v1-successor-lineage-receipt"
        or receipt.get("formatVersion") != 1
    ):
        raise SuccessorLineageError("successor lineage receipt header is invalid")
    historical = receipt.get("historicalCarrierReceipt") or {}
    historical_path = _repository_path(
        root, historical.get("path"), "historical Carrier V1 receipt"
    )
    if (
        historical_path.name
        != "Valtan.carrier-v1-materialization-receipt.v1.json"
        or historical.get("canonicalSha256")
        != canonical_sha256(historical_receipt)
    ):
        raise SuccessorLineageError(
            "immutable historical Carrier V1 receipt drifted"
        )

    documents = successor_documents(receipt)
    historical_targets = _historical_target_index(historical_receipt)
    divergent_targets: set[str] = set()
    for effect_id, row in historical_targets.items():
        path = _repository_path(
            root, row.get("path"), f"historical target {effect_id}"
        )
        if not path.is_file() or canonical_sha256(read_json(path)) != row.get(
            "canonicalSha256"
        ):
            divergent_targets.add(effect_id)
    registered_targets = set(documents).intersection(historical_targets)
    if divergent_targets != registered_targets:
        raise SuccessorLineageError(
            "divergent Carrier V1 target document set is not exactly registered"
        )

    for effect_id, entry in documents.items():
        path = _repository_path(
            root, entry.get("path"), f"successor document {effect_id}"
        )
        if not path.is_file():
            raise SuccessorLineageError(
                f"successor document is missing: {effect_id}"
            )
        _validate_document_lineage(
            entry=entry,
            document=read_json(path),
            historical_receipt=historical_receipt,
        )

    target_successors = set(documents).intersection(historical_targets)
    historical_exceptions = {
        str(row.get("effectAssetId") or ""): row
        for row in _object_rows(
            historical_receipt.get("ownershipExceptions"),
            "historical ownership exception",
        )
    }
    external_owner_count = 0
    for effect_id, row in historical_exceptions.items():
        if effect_id in historical_targets:
            continue
        if effect_id in documents:
            external_owner_count += int(
                (documents[effect_id].get("finalDocument") or {}).get(
                    "elementCount", -1
                )
            )
        else:
            external_owner_count += int(row.get("elementCount", -1))
    expected_summary = {
        "successorDocumentCount": len(documents),
        "historicalTargetSuccessorCount": len(target_successors),
        "historicalBaselineElementCount": sum(
            int((entry.get("historicalBaseline") or {}).get("elementCount", -1))
            for entry in documents.values()
        ),
        "finalSuccessorElementCount": sum(
            int((entry.get("finalDocument") or {}).get("elementCount", -1))
            for entry in documents.values()
        ),
        "historicalOwnerFinalElementCount": (
            int(
                (historical_receipt.get("summary") or {}).get(
                    "materializedProjectionCount", -1
                )
            )
            - sum(
                int(
                    (documents[effect_id].get("historicalBaseline") or {}).get(
                        "elementCount", -1
                    )
                )
                for effect_id in target_successors
            )
            + sum(
                int((documents[effect_id].get("finalDocument") or {}).get(
                    "elementCount", -1
                ))
                for effect_id in target_successors
            )
            + external_owner_count
        ),
    }
    if receipt.get("summary") != expected_summary:
        raise SuccessorLineageError("successor lineage summary drifted")

    live_projection = receipt.get("liveProductProjection") or {}
    valtan_catalog = {
        "formatVersion": catalog.get("formatVersion"),
        "effectAssetIdPrefix": "effect.valtan.",
        "effects": sorted(
            [
                copy.deepcopy(row)
                for row in _object_rows(catalog.get("effects"), "live catalog")
                if str(row.get("effectAssetId") or "").startswith(
                    "effect.valtan."
                )
            ],
            key=lambda row: str(row["effectAssetId"]),
        ),
    }
    catalog_seal = live_projection.get("catalog") or {}
    cue_seal = live_projection.get("cues") or {}
    cue_rows = _object_rows(cues.get("cues"), "live cue")
    if (
        catalog_seal.get("effectCount")
        != len(valtan_catalog["effects"])
        or catalog_seal.get("canonicalSha256")
        != canonical_sha256(valtan_catalog)
        or cue_seal.get("cueCount") != len(cue_rows)
        or cue_seal.get("canonicalSha256") != canonical_sha256(cues)
    ):
        raise SuccessorLineageError(
            "live Valtan catalog/cue projection seal drifted"
        )
