#!/usr/bin/env python3
"""Project manifest-selected effect roles by changing only element visibility.

The projector is intentionally character- and skill-agnostic.  A role-composition
manifest owns the exact authored documents, stable element IDs, source systems,
duplicate provenance, and receipts.  Projection never edits material, resource,
source-recipe, provenance, timing, transform, or catalog data.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any, Callable


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = ROOT / (
    "Data/Effects/AuthoredCorrections/LanceMaster/"
    "LanceMaster.q-w-e-r-a-s.user-role-composition.json"
)
EXPECTED_SCHEMA = "lostark.effect-role-composition"
EXPECTED_PROJECTION_SCHEMA = "lostark.effect-role-visibility-projection"
SOURCE_SYSTEM_PATTERN = re.compile(
    r"\|element:(.+?)\."
    r"(?:particlespriteemitter|particlemeshemitter|particlebeam2emitter|particletrail2emitter)"
    r"(?:_\d+)?(?:\.|$)",
    re.IGNORECASE,
)


class ProjectionError(RuntimeError):
    pass


def canonical_json_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise ProjectionError(f"cannot read JSON '{path}': {error}") from error
    if not isinstance(value, dict):
        raise ProjectionError(f"JSON root is not an object: {path}")
    return value


def safe_repo_path(root: Path, relative: str) -> Path:
    if not isinstance(relative, str) or not relative or "\\" in relative:
        raise ProjectionError(f"invalid repository-relative path: {relative!r}")
    pure = Path(relative)
    if pure.is_absolute() or ":" in relative or any(part == ".." for part in pure.parts):
        raise ProjectionError(f"unsafe repository-relative path: {relative!r}")
    resolved_root = root.resolve()
    resolved = (resolved_root / pure).resolve()
    try:
        resolved.relative_to(resolved_root)
    except ValueError as error:
        raise ProjectionError(f"path escapes repository root: {relative!r}") from error
    return resolved


def execution_class(element: dict[str, Any]) -> str:
    execution = element.get("material", {}).get("execution", {})
    if execution.get("authoringApproximate") is True:
        return "AUTHORING_APPROXIMATE"
    if execution.get("failClosed") is True:
        return "HARD_FAIL_CLOSED"
    return "FULL"


def source_system(element: dict[str, Any]) -> str:
    source_node = element.get("sourceNode")
    match = SOURCE_SYSTEM_PATTERN.search(source_node if isinstance(source_node, str) else "")
    if match is None:
        raise ProjectionError(f"cannot resolve source system: {element.get('id', '')}")
    return match.group(1).lower()


def source_event_id(element: dict[str, Any]) -> str | None:
    value = element.get("sourcePresentation", {}).get("sourceEventId")
    return value if isinstance(value, str) and value else None


def normalized_duplicate_payload(element: dict[str, Any]) -> dict[str, Any]:
    value = copy.deepcopy(element)
    for key in ("id", "displayName", "sourceNode", "visible"):
        value.pop(key, None)
    presentation = value.get("sourcePresentation", {})
    if isinstance(presentation, dict):
        for key in ("sourceEventId", "sourceOccurrenceIndex", "sourceTimeSeconds"):
            presentation.pop(key, None)
    return value


def document_without_visibility(document: dict[str, Any]) -> dict[str, Any]:
    value = copy.deepcopy(document)
    elements = value.get("elements")
    if not isinstance(elements, list):
        raise ProjectionError("effect document elements are invalid")
    for element in elements:
        if not isinstance(element, dict):
            raise ProjectionError("effect document element is invalid")
        element.pop("visible", None)
    return value


def selected_ids_sha(elements: list[dict[str, Any]]) -> str:
    return hashlib.sha256(
        "\n".join(sorted(element["id"] for element in elements)).encode("utf-8")
    ).hexdigest()


def resource_signature(elements: list[dict[str, Any]]) -> str:
    rows: list[str] = []
    for element in elements:
        for resource in element.get("resources", []):
            if isinstance(resource, dict) and resource.get("assetId"):
                rows.append(
                    f"{element['id']}|R|{resource.get('slotId', '')}|{resource['assetId']}"
                )
        profile = element.get("material", {}).get("sourceProfile", {})
        for texture in profile.get("textures", []) if isinstance(profile, dict) else []:
            if isinstance(texture, dict) and texture.get("assetId"):
                rows.append(
                    f"{element['id']}|T|{texture.get('name', '')}|{texture['assetId']}"
                )
    return hashlib.sha256("\n".join(sorted(rows)).encode("utf-8")).hexdigest()


def _unique_rows(rows: Any, key: str, context: str) -> dict[str, dict[str, Any]]:
    if not isinstance(rows, list):
        raise ProjectionError(f"{context} rows are invalid")
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        value = row.get(key) if isinstance(row, dict) else None
        if not isinstance(value, str) or not value or value in result:
            raise ProjectionError(f"{context} identity is missing or duplicated: {value!r}")
        result[value] = row
    return result


def _validate_skill(
    skill: dict[str, Any], document: dict[str, Any], path: Path
) -> tuple[set[str], dict[str, int]]:
    elements = document.get("elements")
    if not isinstance(elements, list):
        raise ProjectionError(f"document elements are invalid: {path}")
    by_id = _unique_rows(elements, "id", f"{path} element")

    invariant_hash = canonical_json_sha256(document_without_visibility(document))
    if invariant_hash != skill.get("nonVisibilityDocumentCanonicalSha256"):
        raise ProjectionError(f"non-visibility document contract drifted: {path}")

    selected: set[str] = set()
    for role in skill.get("roles", []):
        if not isinstance(role, dict):
            raise ProjectionError(f"role row is invalid: {path}")
        role_name = role.get("role")
        expected_system = role.get("sourceSystem")
        stable_ids = role.get("stableIds")
        occurrences = role.get("sourceOccurrences")
        if (
            not isinstance(role_name, str)
            or not role_name
            or not isinstance(expected_system, str)
            or not expected_system
            or not isinstance(stable_ids, list)
            or not isinstance(occurrences, list)
        ):
            raise ProjectionError(f"role contract is incomplete: {path}")
        allowed_events = {
            occurrence.get("sourceEventId")
            for occurrence in occurrences
            if isinstance(occurrence, dict)
        }
        if len(allowed_events) != len(occurrences):
            raise ProjectionError(f"role source occurrence is duplicated: {path}/{role_name}")
        for stable_id in stable_ids:
            if not isinstance(stable_id, str) or stable_id not in by_id or stable_id in selected:
                raise ProjectionError(f"selected stable ID drifted: {path}/{stable_id!r}")
            element = by_id[stable_id]
            if execution_class(element) not in ("FULL", "AUTHORING_APPROXIMATE"):
                raise ProjectionError(f"Hard element cannot be selected: {path}/{stable_id}")
            if source_system(element) != expected_system.lower():
                raise ProjectionError(f"selected source system drifted: {path}/{stable_id}")
            if source_event_id(element) not in allowed_events:
                raise ProjectionError(f"selected source occurrence drifted: {path}/{stable_id}")
            selected.add(stable_id)

    duplicate_hidden: set[str] = set()
    for group in skill.get("duplicateGroups", []):
        if not isinstance(group, dict) or group.get("typedReason") != "HIDDEN_UNPOSITIONED":
            raise ProjectionError(f"duplicate provenance is invalid: {path}")
        keep_id = group.get("keepStableId")
        hidden_ids = group.get("hiddenStableIds")
        if keep_id not in selected or not isinstance(hidden_ids, list) or not hidden_ids:
            raise ProjectionError(f"duplicate keep/hidden contract is invalid: {path}")
        keep_payload = normalized_duplicate_payload(by_id[keep_id])
        for stable_id in hidden_ids:
            if (
                not isinstance(stable_id, str)
                or stable_id not in by_id
                or stable_id in selected
                or stable_id in duplicate_hidden
            ):
                raise ProjectionError(f"duplicate hidden stable ID drifted: {path}/{stable_id!r}")
            element = by_id[stable_id]
            if execution_class(element) not in ("FULL", "AUTHORING_APPROXIMATE"):
                raise ProjectionError(f"Hard element cannot be a role duplicate: {path}/{stable_id}")
            if normalized_duplicate_payload(element) != keep_payload:
                raise ProjectionError(f"duplicate render payload drifted: {path}/{stable_id}")
            duplicate_hidden.add(stable_id)

    explicit_hidden: set[str] = set()
    for row in skill.get("explicitlyHiddenRoleRows", []):
        stable_id = row.get("stableId") if isinstance(row, dict) else None
        reason = row.get("typedReason") if isinstance(row, dict) else None
        if (
            not isinstance(stable_id, str)
            or stable_id not in by_id
            or stable_id in selected
            or stable_id in duplicate_hidden
            or stable_id in explicit_hidden
            or not isinstance(reason, str)
            or not reason
        ):
            raise ProjectionError(f"explicit hidden role row drifted: {path}/{stable_id!r}")
        if execution_class(by_id[stable_id]) not in ("FULL", "AUTHORING_APPROXIMATE"):
            raise ProjectionError(f"Hard element cannot be role-excluded: {path}/{stable_id}")
        explicit_hidden.add(stable_id)

    hard = {
        stable_id
        for stable_id, element in by_id.items()
        if execution_class(element) == "HARD_FAIL_CLOSED"
    }
    accounted = selected | duplicate_hidden | explicit_hidden | hard
    if accounted != set(by_id) or sum(
        len(value) for value in (selected, duplicate_hidden, explicit_hidden, hard)
    ) != len(accounted):
        raise ProjectionError(f"role visibility partition is incomplete or overlapping: {path}")

    selected_elements = [by_id[stable_id] for stable_id in selected]
    receipt = skill.get("selectionReceipt")
    expected_receipt = {
        "elementCount": len(elements),
        "visibleBefore": len(selected) + len(duplicate_hidden) + len(explicit_hidden),
        "visibleAfter": len(selected),
        "selectedIdsSha256": selected_ids_sha(selected_elements),
        "selectedResourceSignatureSha256": resource_signature(selected_elements),
        "selectedFullCount": sum(
            execution_class(element) == "FULL" for element in selected_elements
        ),
        "selectedApproximateCount": sum(
            execution_class(element) == "AUTHORING_APPROXIMATE"
            for element in selected_elements
        ),
        "hardLockedCount": len(hard),
        "hiddenUnpositionedCount": len(duplicate_hidden),
        "explicitRoleExcludedCount": len(explicit_hidden),
    }
    if receipt != expected_receipt:
        raise ProjectionError(f"selection receipt drifted: {path}")
    return selected, {
        "elements": len(elements),
        "selected": len(selected),
        "full": expected_receipt["selectedFullCount"],
        "approximate": expected_receipt["selectedApproximateCount"],
        "hard": len(hard),
        "duplicates": len(duplicate_hidden),
        "excluded": len(explicit_hidden),
    }


def build_projection(
    root: Path, manifest_path: Path, require_projected: bool
) -> tuple[dict[Path, bytes], dict[str, int]]:
    manifest = load_json(manifest_path)
    projection = manifest.get("visibilityProjection")
    skills = manifest.get("skills")
    if (
        manifest.get("schema") != EXPECTED_SCHEMA
        or manifest.get("formatVersion") != 1
        or not isinstance(projection, dict)
        or projection.get("schema") != EXPECTED_PROJECTION_SCHEMA
        or projection.get("formatVersion") != 1
        or not isinstance(skills, list)
    ):
        raise ProjectionError("role visibility manifest schema is missing or invalid")

    materialized = [skill for skill in skills if skill.get("effectDocument") is not None]
    paths = [skill.get("effectDocument") for skill in materialized]
    if (
        len(paths) != projection.get("targetDocumentCount")
        or len(set(paths)) != len(paths)
    ):
        raise ProjectionError("role visibility target document denominator drifted")

    payloads: dict[Path, bytes] = {}
    totals = {
        "targetDocumentCount": len(materialized),
        "targetElementCount": 0,
        "selectedVisibleCount": 0,
        "selectedFullCount": 0,
        "selectedApproximateCount": 0,
        "hardLockedCount": 0,
        "hiddenUnpositionedCount": 0,
        "explicitRoleExcludedCount": 0,
        "changedDocumentCount": 0,
        "changedVisibilityCount": 0,
    }
    for skill in materialized:
        path = safe_repo_path(root, skill["effectDocument"])
        original = path.read_bytes()
        document = load_json(path)
        selected, counts = _validate_skill(skill, document, path)
        changed = 0
        for element in document["elements"]:
            expected = element["id"] in selected
            if element.get("visible") is not expected:
                if require_projected:
                    raise ProjectionError(f"role visibility projection is stale: {path}/{element['id']}")
                element["visible"] = expected
                changed += 1
        if changed:
            payloads[path] = encode_json_like(original, document)
            totals["changedDocumentCount"] += 1
            totals["changedVisibilityCount"] += changed
        totals["targetElementCount"] += counts["elements"]
        totals["selectedVisibleCount"] += counts["selected"]
        totals["selectedFullCount"] += counts["full"]
        totals["selectedApproximateCount"] += counts["approximate"]
        totals["hardLockedCount"] += counts["hard"]
        totals["hiddenUnpositionedCount"] += counts["duplicates"]
        totals["explicitRoleExcludedCount"] += counts["excluded"]

    expected_totals = projection.get("receipt")
    receipt_totals = {
        key: totals[key]
        for key in (
            "targetElementCount",
            "selectedVisibleCount",
            "selectedFullCount",
            "selectedApproximateCount",
            "hardLockedCount",
            "hiddenUnpositionedCount",
            "explicitRoleExcludedCount",
        )
    }
    if expected_totals != receipt_totals:
        raise ProjectionError("role visibility corpus receipt drifted")
    return payloads, totals


def encode_json_like(original: bytes, value: dict[str, Any]) -> bytes:
    bom = original.startswith(b"\xef\xbb\xbf")
    newline = "\r\n" if b"\r\n" in original else "\n"
    text = json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    if newline != "\n":
        text = text.replace("\n", newline)
    return (b"\xef\xbb\xbf" if bom else b"") + text.encode("utf-8")


def _write_temp(path: Path, payload: bytes) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(
        mode="wb", delete=False, dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
    )
    try:
        handle.write(payload)
        handle.flush()
        os.fsync(handle.fileno())
        return Path(handle.name)
    finally:
        handle.close()


def write_payloads_transactionally(
    payloads: dict[Path, bytes],
    replace_func: Callable[[str | os.PathLike[str], str | os.PathLike[str]], None]
    | None = None,
) -> None:
    if not payloads:
        return
    replace = replace_func or os.replace
    originals = {path: path.read_bytes() if path.exists() else None for path in payloads}
    staged = {path: _write_temp(path, payload) for path, payload in payloads.items()}
    committed: list[Path] = []
    try:
        for path in sorted(staged, key=lambda value: value.as_posix()):
            replace(staged[path], path)
            committed.append(path)
    except Exception:
        restore_errors: list[Exception] = []
        for path in reversed(committed):
            try:
                original = originals[path]
                if original is None:
                    path.unlink(missing_ok=True)
                else:
                    restore = _write_temp(path, original)
                    try:
                        replace(restore, path)
                    finally:
                        restore.unlink(missing_ok=True)
            except Exception as error:  # pragma: no cover - catastrophic I/O path
                restore_errors.append(error)
        if restore_errors:
            raise ProjectionError(
                f"projection commit and rollback both failed: {restore_errors[0]}"
            )
        raise
    finally:
        for path in staged.values():
            path.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--root", type=Path, default=ROOT)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()

    payloads, result = build_projection(
        args.root.resolve(), args.manifest.resolve(), require_projected=args.check
    )
    if args.write:
        write_payloads_transactionally(payloads)
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
