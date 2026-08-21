#!/usr/bin/env python3
"""Transactionally project drawable-proven reviewed Valtan source families.

This command is deliberately fail-closed.  The immutable reviewed-source
candidate receipt is not an admission proof: an independent machine drawable
proof must cover every candidate element before ``--apply`` can write.  The
projection only appends missing stable ``(id, sourceNode)`` pairs to the 36
existing authored documents.  Existing objects, including project and hand
tuning, remain byte-for-byte equivalent as JSON values; no catalog, cue, or
legacy generic row is rewritten or deleted.
"""

from __future__ import annotations

import argparse
from copy import deepcopy
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
from typing import Any, Mapping


CANDIDATE_SCHEMA = "lostark.valtan-reviewed-source-family-candidates"
PROOF_SCHEMA = "lostark.valtan-reviewed-source-family-drawable-proof"
RECEIPT_SCHEMA = "lostark.valtan-reviewed-source-family-application-receipt"
FORMAT_VERSION = 1
BOSS_ARCHETYPE_ID = "BOSS_VALTAN"
EXPECTED_CANDIDATE_DOCUMENT_COUNT = 37
EXPECTED_CANDIDATE_ELEMENT_COUNT = 380
EXPECTED_APPLY_DOCUMENT_COUNT = 36
EXPECTED_APPLY_ELEMENT_COUNT = 280
EXCLUDED_MULTI_CUE_EFFECT_ID = "effect.valtan.front-back-front.active"
EXPECTED_EXCLUDED_ELEMENT_COUNT = 100
EXPECTED_EXCLUDED_VISUAL_TIMING_GROUP_COUNT = 4
PROTECTED_WHIRLWIND_EFFECT_ID = "effect.valtan.pattern.420633.active"

DEFAULT_CANDIDATE_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/ReviewedSourceFamilies/"
    "Valtan.reviewed-source-family-candidates.v1.json"
)
DEFAULT_APPLICATION_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/ReviewedSourceFamilies/"
    "Valtan.reviewed-source-family-application-receipt.v1.json"
)


class ProjectionError(RuntimeError):
    """The requested projection is invalid or cannot be proved safe."""


class SourceRebaseRequired(ProjectionError):
    """Canonical/source identity drift requires rebuilding the candidates."""


@dataclass(frozen=True)
class Projection:
    repository_root: Path
    outputs: Mapping[PurePosixPath, bytes]
    canonical_outputs: Mapping[PurePosixPath, bytes]
    guards: Mapping[PurePosixPath, bytes | None]
    receipt: dict[str, Any]
    changed_paths: tuple[PurePosixPath, ...]
    new_paths: tuple[PurePosixPath, ...]


def _json_bytes(value: Any, newline: str = "\n") -> bytes:
    text = json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    if newline == "\r\n":
        text = text.replace("\n", "\r\n")
    return text.encode("utf-8")


def _json_bytes_like(source: bytes, value: Any) -> bytes:
    return _json_bytes(value, "\r\n" if b"\r\n" in source else "\n")


def _sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _is_sha256(value: Any) -> bool:
    return (
        isinstance(value, str)
        and len(value) == 64
        and all(character in "0123456789abcdef" for character in value)
    )


def _load_json_bytes(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise ProjectionError(f"cannot read JSON {path}: {exc}") from exc
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ProjectionError(f"JSON must be UTF-8 without BOM: {path}")

    def no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            if key in result:
                raise ProjectionError(f"duplicate JSON property {key!r}: {path}")
            result[key] = value
        return result

    try:
        value = json.loads(payload.decode("utf-8"), object_pairs_hook=no_duplicates)
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ProjectionError(f"invalid JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ProjectionError(f"JSON root must be an object: {path}")
    return value, payload


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProjectionError(f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise ProjectionError(f"{label} must be an array")
    return value


def _relative_path(value: str | PurePosixPath, label: str) -> PurePosixPath:
    text = value.as_posix() if isinstance(value, PurePosixPath) else value
    if not isinstance(text, str) or not text or "\\" in text:
        raise ProjectionError(f"{label} must be a repository-relative POSIX path")
    path = PurePosixPath(text)
    if path.is_absolute() or "." in path.parts or ".." in path.parts:
        raise ProjectionError(f"{label} escapes the repository: {text}")
    return path


def _repository_path(root: Path, relative: PurePosixPath) -> Path:
    candidate = root.joinpath(*relative.parts)
    try:
        candidate.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise ProjectionError(f"repository path escaped root: {relative}") from exc
    return candidate


def _path_inside_repository(root: Path, path: Path, label: str) -> PurePosixPath:
    absolute = path if path.is_absolute() else root / path
    try:
        relative = absolute.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise ProjectionError(f"{label} must be inside the repository") from exc
    return _relative_path(relative.as_posix(), label)


def _validate_candidate_receipt(receipt: dict[str, Any]) -> list[dict[str, Any]]:
    if (
        receipt.get("schema") != CANDIDATE_SCHEMA
        or receipt.get("formatVersion") != FORMAT_VERSION
        or receipt.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
        or receipt.get("mode")
        != "IMMUTABLE_IMPORTED_CANDIDATES_AND_REPORT_ONLY_RECONCILE"
    ):
        raise ProjectionError("reviewed-source candidate receipt identity is invalid")
    scope = _require_object(receipt.get("scope"), "candidate receipt scope")
    if (
        scope.get("selectedBranchStatus") != "REVIEWED_SELECTED"
        or scope.get("reachabilityDisposition") != "REACHABLE_REVIEWED"
        or scope.get("carrierDisposition") != "EXECUTABLE_CORE"
        or scope.get("candidateEffectDocumentVersion") != 13
        or scope.get("canonicalMutationDisposition") != "NONE"
    ):
        raise ProjectionError("reviewed-source candidate scope is invalid")
    summary = _require_object(receipt.get("summary"), "candidate receipt summary")
    expected = {
        "candidateDocumentCount": EXPECTED_CANDIDATE_DOCUMENT_COUNT,
        "admittedCoreProjectionCount": EXPECTED_CANDIDATE_ELEMENT_COUNT,
        "candidateElementCount": EXPECTED_CANDIDATE_ELEMENT_COUNT,
        "missingOnlyAddElementCount": EXPECTED_CANDIDATE_ELEMENT_COUNT,
        "sourceRebaseRequiredCount": 0,
        "deletedElementCount": 0,
    }
    for key, value in expected.items():
        if summary.get(key) != value:
            raise ProjectionError(
                f"candidate receipt data drift: summary.{key} must be {value}"
            )
    documents = [
        _require_object(row, "candidate receipt document")
        for row in _require_list(receipt.get("documents"), "candidate receipt documents")
    ]
    if len(documents) != EXPECTED_CANDIDATE_DOCUMENT_COUNT:
        raise ProjectionError("candidate receipt must contain exactly 37 documents")
    effect_ids = [row.get("effectAssetId") for row in documents]
    if any(not isinstance(value, str) or not value for value in effect_ids):
        raise ProjectionError("candidate receipt effect IDs are invalid")
    if len(set(effect_ids)) != len(effect_ids) or effect_ids != sorted(effect_ids):
        raise ProjectionError("candidate receipt effect IDs must be unique and sorted")
    protected = set(scope.get("protectedEffectAssetIds", []))
    if PROTECTED_WHIRLWIND_EFFECT_ID not in protected:
        raise ProjectionError("Whirlwind active is not protected by the candidate receipt")
    if protected.intersection(effect_ids):
        raise ProjectionError("a protected canary was emitted as a candidate")
    return documents


def _validate_source_guard_rows(receipt: dict[str, Any]) -> list[dict[str, str]]:
    sources = _require_object(receipt.get("sources"), "candidate receipt sources")
    rows: list[dict[str, str]] = []
    for key in ("selectionManifest", "cueDocument", "effectCatalog"):
        row = _require_object(sources.get(key), f"candidate receipt sources.{key}")
        rows.append({"path": row.get("path"), "sha256": row.get("sha256")})
    for raw in _require_list(
        sources.get("sourceInventoryRepositorySources"),
        "candidate receipt source inventory guards",
    ):
        row = _require_object(raw, "candidate receipt source inventory guard")
        rows.append({"path": row.get("path"), "sha256": row.get("sha256")})
    normalized: dict[str, dict[str, str]] = {}
    for row in rows:
        relative = _relative_path(row["path"], "source guard path")
        sha = row["sha256"]
        if not _is_sha256(sha):
            raise ProjectionError(f"source guard SHA is invalid: {relative}")
        text = relative.as_posix()
        previous = normalized.get(text)
        if previous is not None and previous["sha256"] != sha:
            raise ProjectionError(f"source guard has divergent SHAs: {text}")
        normalized[text] = {"path": text, "sha256": sha}
    return [normalized[key] for key in sorted(normalized)]


def _validate_candidate_document(
    document: dict[str, Any], row: dict[str, Any]
) -> list[dict[str, Any]]:
    effect_id = row["effectAssetId"]
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
    ):
        raise ProjectionError(f"candidate document identity is invalid: {effect_id}")
    elements = [
        _require_object(value, f"{effect_id} candidate element")
        for value in _require_list(document.get("elements"), f"{effect_id}.elements")
    ]
    if len(elements) != row.get("candidateElementCount"):
        raise ProjectionError(f"candidate element count drift: {effect_id}")
    key_rows = [
        _require_object(value, f"{effect_id} sourceElementKey")
        for value in _require_list(row.get("sourceElementKeys"), f"{effect_id}.sourceElementKeys")
    ]
    element_pairs: list[tuple[str, str]] = []
    key_pairs: list[tuple[str, str]] = []
    for element in elements:
        element_id = element.get("id")
        source_node = element.get("sourceNode")
        if not isinstance(element_id, str) or not isinstance(source_node, str):
            raise ProjectionError(f"candidate stable identity is invalid: {effect_id}")
        source_recipe = _require_object(
            element.get("sourceRecipe"), f"{effect_id}/{element_id}.sourceRecipe"
        )
        if source_recipe.get("enabled") is not True:
            raise ProjectionError(f"candidate source recipe is disabled: {effect_id}/{element_id}")
        timing = _require_object(
            _require_object(element.get("detail"), f"{effect_id}/{element_id}.detail").get("timing"),
            f"{effect_id}/{element_id}.timing",
        )
        delay = timing.get("startDelaySeconds")
        if not isinstance(delay, (int, float)) or isinstance(delay, bool) or delay < 0:
            raise ProjectionError(f"candidate cue-local timing is invalid: {effect_id}/{element_id}")
        element_pairs.append((element_id, source_node))
    for key in key_rows:
        element_id = key.get("id")
        source_node = key.get("sourceNode")
        if not isinstance(element_id, str) or not isinstance(source_node, str):
            raise ProjectionError(f"candidate receipt source identity is invalid: {effect_id}")
        if key.get("disposition") != "ADMITTED":
            raise ProjectionError(f"candidate receipt contains a non-admitted row: {effect_id}")
        delay = key.get("elementStartDelaySeconds")
        if not isinstance(delay, (int, float)) or isinstance(delay, bool) or delay < 0:
            raise ProjectionError(f"candidate receipt timing is invalid: {effect_id}")
        key_pairs.append((element_id, source_node))
    if element_pairs != key_pairs or len(set(element_pairs)) != len(element_pairs):
        raise ProjectionError(f"candidate document/source receipt identity drift: {effect_id}")
    for element, key in zip(elements, key_rows):
        delay = element["detail"]["timing"]["startDelaySeconds"]
        if delay != key["elementStartDelaySeconds"]:
            raise ProjectionError(f"candidate cue-local timing drift: {effect_id}/{element['id']}")
    reconcile = _require_object(row.get("reconcile"), f"{effect_id}.reconcile")
    if (
        reconcile.get("mode") != "MISSING_ONLY_PRESERVE_EXISTING_REPORT_ONLY"
        or reconcile.get("deleteElements") != []
        or reconcile.get("sourceRebaseRequiredRows") != []
    ):
        raise ProjectionError(f"candidate reconcile policy is unsafe: {effect_id}")
    add_pairs = [
        (value.get("id"), value.get("sourceNode"))
        for value in _require_list(reconcile.get("addElementRefs"), f"{effect_id}.addElementRefs")
        if isinstance(value, dict)
    ]
    if add_pairs != element_pairs:
        raise ProjectionError(f"candidate missing-only references drift: {effect_id}")
    return elements


def _validate_catalog_and_cues(
    catalog: dict[str, Any], cues: dict[str, Any], documents: list[dict[str, Any]]
) -> None:
    if catalog.get("formatVersion") != 1:
        raise ProjectionError("EffectCatalog.json must remain formatVersion 1")
    catalog_rows = [
        _require_object(row, "EffectCatalog row")
        for row in _require_list(catalog.get("effects"), "EffectCatalog.effects")
    ]
    catalog_by_id: dict[str, dict[str, Any]] = {}
    for row in catalog_rows:
        effect_id = row.get("effectAssetId")
        if not isinstance(effect_id, str) or effect_id in catalog_by_id:
            raise ProjectionError("EffectCatalog IDs are invalid or duplicated")
        catalog_by_id[effect_id] = row
    if (
        cues.get("schema") != "lostark.valtan-pattern-effect-cues"
        or cues.get("formatVersion") != 2
        or cues.get("ownerArchetypeId") != BOSS_ARCHETYPE_ID
    ):
        raise ProjectionError("Valtan cue document must remain exact v2")
    cue_rows = [
        _require_object(row, "Valtan cue row")
        for row in _require_list(cues.get("cues"), "Valtan cues")
    ]
    for document in documents:
        effect_id = document["effectAssetId"]
        if catalog_by_id.get(effect_id) != document.get("catalogRow"):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED catalog row drift: {effect_id}"
            )
        if document.get("catalogDisposition") != "REUSE_EXISTING_NO_MUTATION":
            raise ProjectionError(f"candidate catalog disposition is unsafe: {effect_id}")
        for clip in _require_list(document.get("clipOccurrences"), f"{effect_id}.clipOccurrences"):
            clip_row = _require_object(clip, f"{effect_id}.clipOccurrence")
            if clip_row.get("cueDisposition") != "REUSE_EXISTING_V2_NO_MUTATION":
                raise ProjectionError(f"candidate cue disposition is unsafe: {effect_id}")
            expected = _require_object(clip_row.get("cueRow"), f"{effect_id}.cueRow")
            matches = [row for row in cue_rows if row == expected]
            if len(matches) != 1 or expected.get("effectAssetId") != effect_id:
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED exact cue v2 row drift: {effect_id}"
                )


def _validate_proof(
    proof: dict[str, Any], receipt_sha: str,
    documents: list[dict[str, Any]], candidates: Mapping[str, list[dict[str, Any]]]
) -> None:
    if (
        proof.get("schema") != PROOF_SCHEMA
        or proof.get("formatVersion") != FORMAT_VERSION
        or proof.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
        or proof.get("candidateReceiptSha256") != receipt_sha
    ):
        raise ProjectionError("drawable proof identity/candidate receipt binding is invalid")
    targets = [
        _require_object(row, "drawable proof target")
        for row in _require_list(proof.get("targets"), "drawable proof targets")
    ]
    if len(targets) != EXPECTED_APPLY_DOCUMENT_COUNT:
        raise ProjectionError("drawable proof must cover exactly 36 applicable candidate documents")
    by_effect: dict[str, dict[str, Any]] = {}
    for target in targets:
        effect_id = target.get("effectAssetId")
        if not isinstance(effect_id, str) or effect_id in by_effect:
            raise ProjectionError("drawable proof effect IDs are invalid or duplicated")
        by_effect[effect_id] = target
    expected_effects = [row["effectAssetId"] for row in documents]
    if [row.get("effectAssetId") for row in targets] != expected_effects:
        raise ProjectionError("drawable proof targets must exactly match candidate order")
    for row in documents:
        effect_id = row["effectAssetId"]
        target = by_effect[effect_id]
        if (
            target.get("candidateDocumentSha256") != row["candidateDocumentSha256"]
            or target.get("disposition") != "DRAWABLE_PROOF_PASS"
        ):
            raise ProjectionError(f"drawable proof target did not PASS exact candidate: {effect_id}")
        proof_elements = [
            _require_object(value, f"{effect_id} drawable proof element")
            for value in _require_list(target.get("elements"), f"{effect_id}.proof.elements")
        ]
        expected_pairs = [(value["id"], value["sourceNode"]) for value in candidates[effect_id]]
        actual_pairs = [(value.get("elementId"), value.get("sourceNode")) for value in proof_elements]
        if actual_pairs != expected_pairs:
            raise ProjectionError(f"drawable proof element closure/order drift: {effect_id}")
        for element in proof_elements:
            if element.get("disposition") != "DRAWABLE_PROOF_PASS":
                raise ProjectionError(f"drawable proof element did not PASS: {effect_id}")
            for key in ("attemptedSamples", "submittedDraws", "committedDraws"):
                value = element.get(key)
                if type(value) is not int or value < 1:
                    raise ProjectionError(f"drawable proof {key} is invalid: {effect_id}")
            for key in ("suppressedDraws", "failedDraws"):
                value = element.get(key)
                if type(value) is not int or value < 0:
                    raise ProjectionError(f"drawable proof {key} is invalid: {effect_id}")
            if element["failedDraws"] != 0:
                raise ProjectionError(f"drawable proof recorded failed draws: {effect_id}")


def _element_indexes(
    document: dict[str, Any], effect_id: str
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    by_id: dict[str, dict[str, Any]] = {}
    by_source: dict[str, dict[str, Any]] = {}
    for raw in _require_list(document.get("elements"), f"{effect_id}.elements"):
        element = _require_object(raw, f"{effect_id}.element")
        element_id = element.get("id")
        source_node = element.get("sourceNode")
        if not isinstance(element_id, str) or not isinstance(source_node, str):
            raise ProjectionError(f"canonical element identity is invalid: {effect_id}")
        if element_id in by_id:
            raise ProjectionError(f"duplicate canonical element ID: {effect_id}/{element_id}")
        by_id[element_id] = element
        if source_node:
            if source_node in by_source:
                raise ProjectionError(f"duplicate canonical sourceNode: {effect_id}/{source_node}")
            by_source[source_node] = element
    return by_id, by_source


def _immutable_projection_identity(element: dict[str, Any]) -> dict[str, Any]:
    return {
        key: element.get(key)
        for key in (
            "id", "sourceNode", "groupId", "kind", "resources", "material",
            "sourceRecipe", "sourcePresentation", "inventoryRendererShape",
            "actionCueAttachment", "transformInheritance",
        )
    }


def _reconcile_document(
    canonical: dict[str, Any], candidate: dict[str, Any], effect_id: str
) -> dict[str, Any]:
    if (
        canonical.get("schema") != "lostark.effect-authoring"
        or canonical.get("version") != 13
        or canonical.get("effectAssetId") != effect_id
    ):
        raise SourceRebaseRequired(
            f"SOURCE_REBASE_REQUIRED canonical document identity drift: {effect_id}"
        )
    original_elements = deepcopy(_require_list(canonical.get("elements"), f"{effect_id}.elements"))
    staged = deepcopy(canonical)
    by_id, by_source = _element_indexes(staged, effect_id)
    appended = 0
    for candidate_element in candidate["elements"]:
        element_id = candidate_element["id"]
        source_node = candidate_element["sourceNode"]
        existing_by_id = by_id.get(element_id)
        existing_by_source = by_source.get(source_node)
        if (existing_by_id is None) != (existing_by_source is None):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED stable identity collision: {effect_id}/{element_id}"
            )
        if existing_by_id is not None:
            if existing_by_id is not existing_by_source:
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED split stable identity: {effect_id}/{element_id}"
                )
            if _immutable_projection_identity(existing_by_id) != _immutable_projection_identity(candidate_element):
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED immutable source row drift: {effect_id}/{element_id}"
                )
            continue
        copied = deepcopy(candidate_element)
        staged["elements"].append(copied)
        by_id[element_id] = copied
        by_source[source_node] = copied
        appended += 1
    _element_indexes(staged, effect_id)
    if staged["elements"][: len(original_elements)] != original_elements:
        raise ProjectionError(f"existing elements were rewritten or reordered: {effect_id}")
    if len(staged["elements"]) != len(original_elements) + appended:
        raise ProjectionError(f"element deletion occurred during projection: {effect_id}")
    return staged


def collect_projection(
    repository_root: Path,
    *,
    candidate_receipt: Path | PurePosixPath = DEFAULT_CANDIDATE_RECEIPT,
    drawable_proof: Path,
    receipt_path: Path | PurePosixPath = DEFAULT_APPLICATION_RECEIPT,
) -> Projection:
    root = repository_root.resolve()
    candidate_relative = (
        candidate_receipt if isinstance(candidate_receipt, PurePosixPath)
        else _path_inside_repository(root, candidate_receipt, "candidate receipt")
    )
    receipt_relative = (
        receipt_path if isinstance(receipt_path, PurePosixPath)
        else _path_inside_repository(root, receipt_path, "application receipt")
    )
    candidate_relative = _relative_path(candidate_relative, "candidate receipt")
    receipt_relative = _relative_path(receipt_relative, "application receipt")
    proof_relative = _path_inside_repository(root, drawable_proof, "drawable proof")
    if receipt_relative in (candidate_relative, proof_relative):
        raise ProjectionError("application receipt collides with an immutable input")

    candidate_receipt_value, candidate_receipt_payload = _load_json_bytes(
        _repository_path(root, candidate_relative)
    )
    documents = _validate_candidate_receipt(candidate_receipt_value)
    proof, proof_payload = _load_json_bytes(_repository_path(root, proof_relative))

    candidate_documents: dict[str, dict[str, Any]] = {}
    candidate_elements: dict[str, list[dict[str, Any]]] = {}
    candidate_payloads: dict[str, bytes] = {}
    candidate_paths: dict[str, PurePosixPath] = {}
    global_ids: set[str] = set()
    global_sources: set[str] = set()
    for row in documents:
        effect_id = row["effectAssetId"]
        relative = _relative_path(row.get("candidateDocumentPath"), f"{effect_id} candidate path")
        if relative in candidate_paths.values():
            raise ProjectionError("candidate document path is duplicated")
        document, payload = _load_json_bytes(_repository_path(root, relative))
        if not _is_sha256(row.get("candidateDocumentSha256")) or _sha256(payload) != row["candidateDocumentSha256"]:
            raise SourceRebaseRequired(f"SOURCE_REBASE_REQUIRED candidate SHA drift: {effect_id}")
        elements = _validate_candidate_document(document, row)
        for element in elements:
            if element["id"] in global_ids or element["sourceNode"] in global_sources:
                raise ProjectionError("candidate stable identities must be globally unique")
            global_ids.add(element["id"])
            global_sources.add(element["sourceNode"])
        candidate_documents[effect_id] = document
        candidate_elements[effect_id] = elements
        candidate_payloads[effect_id] = payload
        candidate_paths[effect_id] = relative
    if len(global_ids) != EXPECTED_CANDIDATE_ELEMENT_COUNT:
        raise ProjectionError("candidate element closure is not exactly 380")
    excluded_rows = [
        row for row in documents if row["effectAssetId"] == EXCLUDED_MULTI_CUE_EFFECT_ID
    ]
    if len(excluded_rows) != 1:
        raise ProjectionError("FRONT_BACK_FRONT aggregate exclusion target is missing")
    excluded_row = excluded_rows[0]
    if (
        excluded_row.get("candidateElementCount") != EXPECTED_EXCLUDED_ELEMENT_COUNT
        or len(_require_list(excluded_row.get("visualTimingGroups"), "FRONT_BACK_FRONT visual groups"))
        != EXPECTED_EXCLUDED_VISUAL_TIMING_GROUP_COUNT
    ):
        raise ProjectionError(
            "FRONT_BACK_FRONT aggregate exclusion must remain 100 elements / 4 visual groups"
        )
    applicable_documents = [
        row for row in documents if row["effectAssetId"] != EXCLUDED_MULTI_CUE_EFFECT_ID
    ]
    if (
        len(applicable_documents) != EXPECTED_APPLY_DOCUMENT_COUNT
        or sum(row["candidateElementCount"] for row in applicable_documents)
        != EXPECTED_APPLY_ELEMENT_COUNT
    ):
        raise ProjectionError("applicable source projection closure must remain 36 docs / 280 elements")
    _validate_proof(
        proof, _sha256(candidate_receipt_payload), applicable_documents, candidate_elements
    )

    guards: dict[PurePosixPath, bytes | None] = {
        candidate_relative: candidate_receipt_payload,
        proof_relative: proof_payload,
    }
    source_guard_rows = _validate_source_guard_rows(candidate_receipt_value)
    source_values: dict[str, dict[str, Any]] = {}
    for row in source_guard_rows:
        relative = _relative_path(row["path"], "source guard path")
        path = _repository_path(root, relative)
        try:
            payload = path.read_bytes()
        except OSError as exc:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED source guard disappeared: {relative}"
            ) from exc
        if _sha256(payload) != row["sha256"]:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED source guard SHA drift: {relative}"
            )
        guards[relative] = payload
        if relative.as_posix().endswith("EffectCatalog.json"):
            source_values["catalog"], _ = _load_json_bytes(path)
        if relative.as_posix().endswith("Valtan.patterneffectcues.json"):
            source_values["cues"], _ = _load_json_bytes(path)
    if set(source_values) != {"catalog", "cues"}:
        raise ProjectionError("candidate source guards do not identify catalog and cue")
    _validate_catalog_and_cues(
        source_values["catalog"], source_values["cues"], documents
    )

    protected_receipts: list[dict[str, Any]] = []
    protected_ids: set[str] = set()
    for raw in _require_list(
        candidate_receipt_value.get("protectedCanaries"), "protected canaries"
    ):
        row = _require_object(raw, "protected canary")
        effect_id = row.get("effectAssetId")
        relative = _relative_path(row.get("authoredDocumentPath"), "protected canary path")
        expected_sha = row.get("authoredDocumentSha256")
        if not isinstance(effect_id, str) or effect_id in protected_ids or not _is_sha256(expected_sha):
            raise ProjectionError("protected canary identity/SHA is invalid")
        protected_ids.add(effect_id)
        path = _repository_path(root, relative)
        try:
            payload = path.read_bytes()
        except OSError as exc:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED protected canary disappeared: {effect_id}"
            ) from exc
        if _sha256(payload) != expected_sha:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED protected canary SHA drift: {effect_id}"
            )
        guards[relative] = payload
        protected_receipts.append(
            {"effectAssetId": effect_id, "path": relative.as_posix(), "sha256": expected_sha}
        )
    if PROTECTED_WHIRLWIND_EFFECT_ID not in protected_ids:
        raise ProjectionError("Whirlwind active protected byte canary is missing")

    canonical_outputs: dict[PurePosixPath, bytes] = {}
    target_receipts: list[dict[str, Any]] = []
    for row in applicable_documents:
        effect_id = row["effectAssetId"]
        target_relative = _relative_path(row.get("authoredDocumentPath"), f"{effect_id} authored path")
        if target_relative in canonical_outputs or target_relative in candidate_paths.values():
            raise ProjectionError("authored output path is duplicated or collides with a candidate")
        target_path = _repository_path(root, target_relative)
        if not target_path.is_file():
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED existing authored document disappeared: {effect_id}"
            )
        canonical, canonical_payload = _load_json_bytes(target_path)
        staged = _reconcile_document(canonical, candidate_documents[effect_id], effect_id)
        staged_payload = _json_bytes_like(canonical_payload, staged)
        canonical_outputs[target_relative] = staged_payload
        guards[target_relative] = canonical_payload
        target_receipts.append(
            {
                "effectAssetId": effect_id,
                "candidateDocumentPath": candidate_paths[effect_id].as_posix(),
                "candidateDocumentSha256": _sha256(candidate_payloads[effect_id]),
                "targetAuthoredDocumentPath": target_relative.as_posix(),
                "candidateElementCount": len(candidate_elements[effect_id]),
                "candidateElements": [
                    {"elementId": element["id"], "sourceNode": element["sourceNode"]}
                    for element in candidate_elements[effect_id]
                ],
                "reconcileDisposition": "MISSING_ONLY_APPEND_OR_DEEP_PRESERVE_EXISTING",
                "finalDocumentSha256": _sha256(staged_payload),
            }
        )

    if len(canonical_outputs) != EXPECTED_APPLY_DOCUMENT_COUNT:
        raise ProjectionError("canonical output closure is not exactly 36 documents")

    excluded_target_relative = _relative_path(
        excluded_row.get("authoredDocumentPath"),
        "FRONT_BACK_FRONT excluded authored path",
    )
    excluded_target_path = _repository_path(root, excluded_target_relative)
    if not excluded_target_path.is_file():
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED FRONT_BACK_FRONT excluded aggregate document disappeared"
        )
    excluded_target_payload = excluded_target_path.read_bytes()
    guards[excluded_target_relative] = excluded_target_payload
    excluded_receipt = {
        "effectAssetId": EXCLUDED_MULTI_CUE_EFFECT_ID,
        "candidateDocumentPath": candidate_paths[EXCLUDED_MULTI_CUE_EFFECT_ID].as_posix(),
        "candidateDocumentSha256": _sha256(candidate_payloads[EXCLUDED_MULTI_CUE_EFFECT_ID]),
        "targetAuthoredDocumentPath": excluded_target_relative.as_posix(),
        "candidateElementCount": EXPECTED_EXCLUDED_ELEMENT_COUNT,
        "visualTimingGroups": [
            {
                "visualTimingGroupId": group["visualTimingGroupId"],
                "visualTimingGroupKey": group["visualTimingGroupKey"],
                "sourceTimeSeconds": group["sourceTimeSeconds"],
                "elementIds": group["elementIds"],
                "elementCount": group["elementCount"],
            }
            for group in excluded_row["visualTimingGroups"]
        ],
        "notifySystemTimingGroups": [
            {
                "notifySystemTimingGroupId": group["notifySystemTimingGroupId"],
                "notifySystemTimingGroupKey": group["notifySystemTimingGroupKey"],
                "visualTimingGroupId": group["visualTimingGroupId"],
                "notifyId": group["notifyId"],
                "sourceSystemId": group["sourceSystemId"],
                "elementIds": group["elementIds"],
                "elementCount": group["elementCount"],
            }
            for group in excluded_row["notifySystemTimingGroups"]
        ],
        "disposition": "EXCLUDED_PENDING_MULTI_CUE_SPLIT",
        "reason": "FOUR_VISUAL_WAVES_MUST_NOT_FLATTEN_INTO_ONE_AGGREGATE_CUE_DOCUMENT",
        "noMutationSha256": _sha256(excluded_target_payload),
    }
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": BOSS_ARCHETYPE_ID,
        "transactionStatus": "COMMITTED",
        "inputs": {
            "candidateReceiptPath": candidate_relative.as_posix(),
            "candidateReceiptSha256": _sha256(candidate_receipt_payload),
            "drawableProofPath": proof_relative.as_posix(),
            "drawableProofSha256": _sha256(proof_payload),
        },
        "policy": {
            "reconcileMode": "MISSING_ONLY_APPEND_OR_DEEP_PRESERVE_EXISTING",
            "existingElementMutation": "FORBIDDEN",
            "legacyGenericElementDeletion": "FORBIDDEN",
            "catalogMutation": "FORBIDDEN",
            "cueMutation": "FORBIDDEN",
            "identityDrift": "SOURCE_REBASE_REQUIRED",
            "failureAction": "ROLLBACK_ALL_OUTPUTS",
        },
        "closure": {
            "inputCandidateDocumentCount": EXPECTED_CANDIDATE_DOCUMENT_COUNT,
            "inputCandidateElementCount": EXPECTED_CANDIDATE_ELEMENT_COUNT,
            "applicableCandidateDocumentCount": EXPECTED_APPLY_DOCUMENT_COUNT,
            "projectedSourceElementCount": EXPECTED_APPLY_ELEMENT_COUNT,
            "canonicalAuthoredDocumentCount": EXPECTED_APPLY_DOCUMENT_COUNT,
            "excludedPendingMultiCueSplitDocumentCount": 1,
            "excludedPendingMultiCueSplitElementCount": EXPECTED_EXCLUDED_ELEMENT_COUNT,
            "legacyGenericDeletedElementCount": 0,
            "catalogMutationCount": 0,
            "cueMutationCount": 0,
            "protectedCanaryCount": len(protected_receipts),
        },
        "sourceGuards": source_guard_rows,
        "protectedCanaries": protected_receipts,
        "excludedTargets": [excluded_receipt],
        "targets": target_receipts,
        "canonicalOutputs": [
            {
                "path": relative.as_posix(),
                "role": "AUTHORED_EFFECT_DOCUMENT",
                "sha256": _sha256(payload),
            }
            for relative, payload in sorted(
                canonical_outputs.items(), key=lambda item: item[0].as_posix()
            )
        ],
    }
    receipt_payload = _json_bytes(receipt)
    outputs = dict(canonical_outputs)
    outputs[receipt_relative] = receipt_payload
    if receipt_relative in guards or receipt_relative in canonical_outputs:
        raise ProjectionError("application receipt path collides with a guarded/output path")
    receipt_disk = _repository_path(root, receipt_relative)
    guards[receipt_relative] = receipt_disk.read_bytes() if receipt_disk.is_file() else None
    for effect_id, relative in candidate_paths.items():
        guards[relative] = candidate_payloads[effect_id]

    changed: list[PurePosixPath] = []
    new: list[PurePosixPath] = []
    for relative, payload in outputs.items():
        current = guards.get(relative)
        if current != payload:
            changed.append(relative)
            if current is None:
                new.append(relative)
    return Projection(
        repository_root=root,
        outputs=outputs,
        canonical_outputs=canonical_outputs,
        guards=guards,
        receipt=receipt,
        changed_paths=tuple(sorted(changed, key=PurePosixPath.as_posix)),
        new_paths=tuple(sorted(new, key=PurePosixPath.as_posix)),
    )


def _verify_guards(projection: Projection) -> None:
    for relative, expected in projection.guards.items():
        path = _repository_path(projection.repository_root, relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            raise ProjectionError(
                f"input changed after staging; transaction not started: {relative}"
            )


def commit_projection(
    projection: Projection, *, failure_after_promote: int | None = None
) -> None:
    _verify_guards(projection)
    if failure_after_promote is not None and failure_after_promote < 1:
        raise ProjectionError("failure_after_promote must be positive")
    root = projection.repository_root
    transaction_root = Path(
        tempfile.mkdtemp(prefix=".valtan-reviewed-source-apply.", dir=root)
    )
    staged_root = transaction_root / "staged"
    backup_root = transaction_root / "backup"
    promoted: list[PurePosixPath] = []
    try:
        write_order = [
            relative
            for relative in sorted(projection.canonical_outputs, key=PurePosixPath.as_posix)
            if projection.guards.get(relative) != projection.canonical_outputs[relative]
        ]
        receipt_relative = next(
            relative for relative in projection.outputs
            if relative not in projection.canonical_outputs
        )
        if projection.guards.get(receipt_relative) != projection.outputs[receipt_relative]:
            write_order.append(receipt_relative)
        for relative in write_order:
            stage_path = staged_root.joinpath(*relative.parts)
            stage_path.parent.mkdir(parents=True, exist_ok=True)
            stage_path.write_bytes(projection.outputs[relative])
            if stage_path.read_bytes() != projection.outputs[relative]:
                raise ProjectionError(f"staged output verification failed: {relative}")
        _verify_guards(projection)
        for relative in write_order:
            target = _repository_path(root, relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.is_file():
                backup = backup_root.joinpath(*relative.parts)
                backup.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(target, backup)
            os.replace(staged_root.joinpath(*relative.parts), target)
            promoted.append(relative)
            if failure_after_promote is not None and len(promoted) == failure_after_promote:
                raise OSError("injected transaction failure")
    except Exception as exc:
        rollback_errors: list[str] = []
        for relative in reversed(promoted):
            target = _repository_path(root, relative)
            backup = backup_root.joinpath(*relative.parts)
            try:
                if backup.is_file():
                    os.replace(backup, target)
                elif target.exists():
                    target.unlink()
            except OSError as rollback_exc:
                rollback_errors.append(f"{relative}: {rollback_exc}")
        if rollback_errors:
            raise ProjectionError(
                "projection failed and rollback was incomplete: " + "; ".join(rollback_errors)
            ) from exc
        if isinstance(exc, ProjectionError):
            raise
        raise ProjectionError(f"projection failed; all outputs rolled back: {exc}") from exc
    finally:
        shutil.rmtree(transaction_root, ignore_errors=True)


def check_projection(projection: Projection) -> None:
    stale: list[str] = []
    for relative, expected in projection.outputs.items():
        path = _repository_path(projection.repository_root, relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            stale.append(relative.as_posix())
    if stale:
        raise ProjectionError(
            "projection is not applied or is stale: " + ", ".join(sorted(stale))
        )


def _summary(projection: Projection, mode: str) -> str:
    closure = projection.receipt["closure"]
    return (
        f"[PASS] Valtan reviewed-source family application {mode}: "
        f"documents={closure['applicableCandidateDocumentCount']}, "
        f"sourceElements={closure['projectedSourceElementCount']}, "
        f"canonicalOutputs={len(projection.canonical_outputs)}, "
        f"changed={len(projection.changed_paths)}, new={len(projection.new_paths)}, "
        "excludedMultiCue=1/100, catalogMutations=0, cueMutations=0, legacyDeletes=0"
    )


def _make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--apply", action="store_true")
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--candidate-receipt", type=Path, default=Path(DEFAULT_CANDIDATE_RECEIPT.as_posix()))
    parser.add_argument("--drawable-proof", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, default=Path(DEFAULT_APPLICATION_RECEIPT.as_posix()))
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _make_parser().parse_args(argv)
    try:
        projection = collect_projection(
            args.repo_root,
            candidate_receipt=args.candidate_receipt,
            drawable_proof=args.drawable_proof,
            receipt_path=args.receipt,
        )
        if args.check:
            check_projection(projection)
            mode = "CHECK"
        elif args.apply:
            commit_projection(projection)
            mode = "APPLY"
        else:
            mode = "DRY_RUN"
    except ProjectionError as exc:
        print(f"[FAILURE] {exc}", file=sys.stderr)
        return 1
    print(_summary(projection, mode))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
