#!/usr/bin/env python3
"""Proof-gated atomic application of four FRONT_BACK_FRONT source waves.

The applicator adds four catalog rows, four clip-occurrence-qualified cue-v2
rows, and four direct-authored v13 documents in one transaction.  It never
changes the animation binding, aggregate cue/document/project overlay,
Whirlwind canary, or Server gameplay data.  A separate machine drawable proof
must cover all 100 source elements before any canonical write can begin.
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


CANDIDATE_SCHEMA = "lostark.valtan-front-back-front-source-wave-candidates"
PROOF_SCHEMA = "lostark.valtan-front-back-front-source-wave-drawable-proof"
RECEIPT_SCHEMA = "lostark.valtan-front-back-front-source-wave-application-receipt"
FORMAT_VERSION = 1
BOSS_ARCHETYPE_ID = "BOSS_VALTAN"
AGGREGATE_EFFECT_ID = "effect.valtan.front-back-front.active"
WHIRLWIND_EFFECT_ID = "effect.valtan.pattern.420633.active"
CLIP_OCCURRENCE_ID = "valtan.attack.front-back-front.active.clip.01"
EXPECTED_TARGET_COUNT = 4
EXPECTED_ELEMENT_COUNT = 100

DEFAULT_CANDIDATE_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/"
    "Valtan.front-back-front-source-wave-candidates.v1.json"
)
DEFAULT_APPLICATION_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/"
    "Valtan.front-back-front-source-wave-application-receipt.v1.json"
)
CATALOG_PATH = PurePosixPath("Data/Effects/EffectCatalog.json")
CUE_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)


class ProjectionError(RuntimeError):
    """The projection lacks a complete, safe, deterministic proof."""


class SourceRebaseRequired(ProjectionError):
    """A stable source/canonical identity changed after candidate creation."""


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


def _json_sha(value: Any) -> str:
    return _sha256(_json_bytes(value))


def _is_sha256(value: Any) -> bool:
    return (
        isinstance(value, str)
        and len(value) == 64
        and all(character in "0123456789abcdef" for character in value)
    )


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProjectionError(f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise ProjectionError(f"{label} must be an array")
    return value


def _relative(value: Any, label: str) -> PurePosixPath:
    text = value.as_posix() if isinstance(value, PurePosixPath) else value
    if not isinstance(text, str) or not text or "\\" in text:
        raise ProjectionError(f"{label} must be a repository-relative POSIX path")
    path = PurePosixPath(text)
    if path.is_absolute() or "." in path.parts or ".." in path.parts:
        raise ProjectionError(f"{label} escaped repository")
    return path


def _repository_path(root: Path, relative: PurePosixPath) -> Path:
    path = root.joinpath(*relative.parts)
    try:
        path.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise ProjectionError(f"path escaped repository: {relative}") from exc
    return path


def _path_inside(root: Path, path: Path, label: str) -> PurePosixPath:
    absolute = path if path.is_absolute() else root / path
    try:
        relative = absolute.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise ProjectionError(f"{label} must be inside the repository") from exc
    return _relative(relative.as_posix(), label)


def _load_json(path: Path) -> tuple[dict[str, Any], bytes]:
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
                raise ProjectionError(f"duplicate property {key!r}: {path}")
            result[key] = value
        return result

    try:
        value = json.loads(payload.decode("utf-8"), object_pairs_hook=no_duplicates)
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ProjectionError(f"invalid JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ProjectionError(f"JSON root must be an object: {path}")
    return value, payload


def _validate_candidate_receipt(receipt: dict[str, Any]) -> list[dict[str, Any]]:
    if (
        receipt.get("schema") != CANDIDATE_SCHEMA
        or receipt.get("formatVersion") != FORMAT_VERSION
        or receipt.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
        or receipt.get("mode")
        != "IMMUTABLE_FOUR_WAVE_CANDIDATES_NO_CANONICAL_MUTATION"
    ):
        raise ProjectionError("FRONT_BACK_FRONT candidate receipt identity is invalid")
    summary = _require_object(receipt.get("summary"), "candidate summary")
    expected_summary = {
        "candidateDocumentCount": 4,
        "candidateElementCount": 100,
        "elementsPerCandidateDocument": 25,
        "visualTimingGroupCount": 4,
        "notifySystemTimingGroupCount": 12,
        "catalogAppendRowCount": 4,
        "cueAppendRowCount": 4,
        "targetAuthoredDocumentCount": 4,
        "aggregateSourceElementAppendCount": 0,
        "duplicateSourceElementCount": 0,
        "auxiliarySourceWaveCount": 1,
        "canonicalMutationCount": 0,
    }
    for key, value in expected_summary.items():
        if summary.get(key) != value:
            raise ProjectionError(f"candidate summary drift: {key} must be {value}")
    clip = _require_object(receipt.get("clipIdentity"), "clip identity")
    if (
        clip.get("clipOccurrenceId") != CLIP_OCCURRENCE_ID
        or clip.get("clipName") != "mesh_att_battle_19_01"
        or clip.get("clipSourceStartMs") != 0
        or clip.get("clipPlayMs") != 0
        or clip.get("clipPlayRate") != 1.0
        or clip.get("clipLoop") is not True
        or clip.get("stageDurationMs") != 5000
        or clip.get("serverHitCount") != 3
        or clip.get("animationBindingMutationDisposition") != "FORBIDDEN"
    ):
        raise ProjectionError("FRONT_BACK_FRONT binding/clip immutable identity drift")
    aggregate = _require_object(receipt.get("aggregateCanary"), "aggregate canary")
    if (
        aggregate.get("effectAssetId") != AGGREGATE_EFFECT_ID
        or aggregate.get("displayLabel") != "Project Tuned Aggregate"
        or aggregate.get("sourceElementAppendCount") != 0
        or aggregate.get("disposition")
        != "PRESERVE_EXISTING_PROJECT_TUNED_AGGREGATE"
    ):
        raise ProjectionError("aggregate canary policy is invalid")
    whirlwind = _require_object(receipt.get("whirlwindCanary"), "Whirlwind canary")
    if whirlwind.get("effectAssetId") != WHIRLWIND_EFFECT_ID:
        raise ProjectionError("Whirlwind active canary is missing")
    display = _require_object(receipt.get("allEffectsClipDisplay"), "All Effects display")
    display_rows = _require_list(display.get("cueDisplayOrder"), "All Effects cue display order")
    if (
        display.get("expectedCueCountAfterApply") != 5
        or len(display_rows) != 5
        or [row.get("displayLabel") for row in display_rows]
        != [
            "Project Tuned Aggregate", "Source Wave 01", "Source Wave 02",
            "Source Wave 03", "Source Wave Aux",
        ]
    ):
        raise ProjectionError("All Effects five-cue display identity drift")
    candidates = [
        _require_object(row, "source-wave candidate")
        for row in _require_list(receipt.get("candidates"), "source-wave candidates")
    ]
    if len(candidates) != EXPECTED_TARGET_COUNT:
        raise ProjectionError("source-wave candidate count must be four")
    if [row.get("waveOrdinal") for row in candidates] != [1, 2, 3, 4]:
        raise ProjectionError("source-wave candidates are not in chronological ordinal order")
    if [row.get("cueSourceStartMs") for row in candidates] != [1169, 2253, 3224, 4220]:
        raise ProjectionError("source-wave safe integer-floor cue starts drifted")
    if any(row.get("candidateElementCount") != 25 for row in candidates):
        raise ProjectionError("each source-wave candidate must contain exactly 25 elements")
    auxiliary = candidates[-1]
    if (
        auxiliary.get("waveId") != "auxiliary-source-wave"
        or auxiliary.get("presentationRole") != "auxiliary-source-wave"
        or auxiliary.get("gameplayHitDisposition")
        != "FORBIDDEN_AUXILIARY_NOT_GAMEPLAY_HIT"
    ):
        raise ProjectionError("fourth source wave was mislabeled as gameplay")
    return candidates


def _validate_candidate_document(
    document: dict[str, Any], row: dict[str, Any]
) -> list[dict[str, Any]]:
    effect_id = row["effectAssetId"]
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
        or document.get("displayName") != row.get("documentDisplayName")
    ):
        raise ProjectionError(f"source-wave candidate document identity drift: {effect_id}")
    elements = [
        _require_object(value, f"{effect_id} candidate element")
        for value in _require_list(document.get("elements"), f"{effect_id}.elements")
    ]
    expected_pairs = [
        (value.get("elementId"), value.get("sourceNode"))
        for value in _require_list(row.get("candidateElements"), f"{effect_id}.candidateElements")
        if isinstance(value, dict)
    ]
    actual_pairs = [(value.get("id"), value.get("sourceNode")) for value in elements]
    if len(elements) != 25 or actual_pairs != expected_pairs or len(set(actual_pairs)) != 25:
        raise ProjectionError(f"source-wave candidate element closure drift: {effect_id}")
    expected_delay = row.get("cueLocalStartDelaySeconds")
    for element in elements:
        detail = _require_object(element.get("detail"), f"{effect_id}.detail")
        timing = _require_object(detail.get("timing"), f"{effect_id}.timing")
        delay = timing.get("startDelaySeconds")
        if (
            not isinstance(delay, (int, float))
            or isinstance(delay, bool)
            or delay != expected_delay
            or delay < 0.0
            or delay >= 0.001000001
        ):
            raise ProjectionError(f"source-wave cue-local delay drift: {effect_id}")
        if _require_object(element.get("sourceRecipe"), f"{effect_id}.sourceRecipe").get("enabled") is not True:
            raise ProjectionError(f"source-wave source recipe disabled: {effect_id}")
    cue = _require_object(row.get("cueRow"), f"{effect_id}.cueRow")
    if (
        cue.get("effectAssetId") != effect_id
        or cue.get("clipOccurrenceId") != CLIP_OCCURRENCE_ID
        or cue.get("sourceStartMs") != row.get("cueSourceStartMs")
        or cue.get("sourceEndMs") is not None
        or cue.get("stopPolicy") != "natural"
        or cue.get("repeatPolicy") != "once"
    ):
        raise ProjectionError(f"source-wave cue policy/window drift: {effect_id}")
    catalog = _require_object(row.get("catalogRow"), f"{effect_id}.catalogRow")
    if (
        catalog.get("effectAssetId") != effect_id
        or catalog.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13"
        or catalog.get("authoringPath")
        != f"Effects/Authored/{effect_id}.effect.json"
    ):
        raise ProjectionError(f"source-wave catalog row drift: {effect_id}")
    return elements


def _validate_proof(
    proof: dict[str, Any], candidate_receipt_sha: str,
    candidates: list[dict[str, Any]], documents: Mapping[str, list[dict[str, Any]]]
) -> None:
    if (
        proof.get("schema") != PROOF_SCHEMA
        or proof.get("formatVersion") != FORMAT_VERSION
        or proof.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
        or proof.get("candidateReceiptSha256") != candidate_receipt_sha
    ):
        raise ProjectionError("source-wave drawable proof binding is invalid")
    targets = [
        _require_object(row, "source-wave proof target")
        for row in _require_list(proof.get("targets"), "source-wave proof targets")
    ]
    if len(targets) != EXPECTED_TARGET_COUNT:
        raise ProjectionError("drawable proof must cover all four source-wave documents")
    if [row.get("effectAssetId") for row in targets] != [row["effectAssetId"] for row in candidates]:
        raise ProjectionError("drawable proof target order/closure drift")
    for candidate, target in zip(candidates, targets):
        effect_id = candidate["effectAssetId"]
        if (
            target.get("candidateDocumentSha256")
            != candidate.get("candidateDocumentSha256")
            or target.get("disposition") != "DRAWABLE_PROOF_PASS"
        ):
            raise ProjectionError(f"drawable proof target did not PASS: {effect_id}")
        proof_elements = [
            _require_object(row, f"{effect_id} proof element")
            for row in _require_list(target.get("elements"), f"{effect_id}.proof.elements")
        ]
        actual_pairs = [(row.get("elementId"), row.get("sourceNode")) for row in proof_elements]
        expected_pairs = [(row["id"], row["sourceNode"]) for row in documents[effect_id]]
        if actual_pairs != expected_pairs:
            raise ProjectionError(f"drawable proof element closure drift: {effect_id}")
        for element in proof_elements:
            if element.get("disposition") != "DRAWABLE_PROOF_PASS":
                raise ProjectionError(f"drawable proof element did not PASS: {effect_id}")
            for key in ("attemptedSamples", "submittedDraws", "committedDraws"):
                if type(element.get(key)) is not int or element[key] < 1:
                    raise ProjectionError(f"drawable proof {key} is invalid: {effect_id}")
            for key in ("suppressedDraws", "failedDraws"):
                if type(element.get(key)) is not int or element[key] < 0:
                    raise ProjectionError(f"drawable proof {key} is invalid: {effect_id}")
            if element["failedDraws"] != 0:
                raise ProjectionError(f"drawable proof recorded a failed draw: {effect_id}")


def _element_pairs(document: dict[str, Any], label: str) -> dict[tuple[str, str], dict[str, Any]]:
    pairs: dict[tuple[str, str], dict[str, Any]] = {}
    ids: set[str] = set()
    sources: set[str] = set()
    for raw in _require_list(document.get("elements"), f"{label}.elements"):
        row = _require_object(raw, f"{label}.element")
        element_id = row.get("id")
        source_node = row.get("sourceNode")
        if not isinstance(element_id, str) or not isinstance(source_node, str):
            raise ProjectionError(f"{label} has an invalid element identity")
        if element_id in ids or (source_node and source_node in sources):
            raise SourceRebaseRequired(f"SOURCE_REBASE_REQUIRED duplicate element identity: {label}")
        ids.add(element_id)
        if source_node:
            sources.add(source_node)
        pairs[(element_id, source_node)] = row
    return pairs


def _validate_no_aggregate_source_duplication(
    aggregate: dict[str, Any], project_overlay: dict[str, Any],
    candidate_pairs: set[tuple[str, str]],
) -> None:
    for label, document in (
        ("aggregate authored document", aggregate),
        ("aggregate project overlay", project_overlay),
    ):
        rows = _element_pairs(document, label)
        by_id = {pair[0] for pair in rows}
        by_source = {pair[1] for pair in rows if pair[1]}
        for element_id, source_node in candidate_pairs:
            if element_id in by_id or source_node in by_source:
                raise SourceRebaseRequired(
                    "SOURCE_REBASE_REQUIRED exact source element leaked into " + label
                )


def _stage_catalog(
    catalog: dict[str, Any], candidates: list[dict[str, Any]]
) -> dict[str, Any]:
    if catalog.get("formatVersion") != 1:
        raise ProjectionError("EffectCatalog must remain formatVersion 1")
    staged = deepcopy(catalog)
    rows = _require_list(staged.get("effects"), "EffectCatalog.effects")
    for row in rows:
        _require_object(row, "EffectCatalog row")
    ids = [row.get("effectAssetId") for row in rows]
    if any(not isinstance(value, str) for value in ids) or len(set(ids)) != len(ids):
        raise ProjectionError("EffectCatalog IDs are invalid or duplicated")
    if ids != sorted(ids):
        raise ProjectionError("EffectCatalog rows are not sorted")
    by_id = {row["effectAssetId"]: row for row in rows}
    for candidate in candidates:
        expected = candidate["catalogRow"]
        effect_id = candidate["effectAssetId"]
        existing = by_id.get(effect_id)
        if existing is not None and existing != expected:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED source-wave catalog row drift: {effect_id}"
            )
        if existing is None:
            copied = deepcopy(expected)
            rows.append(copied)
            by_id[effect_id] = copied
    rows.sort(key=lambda row: row["effectAssetId"])
    return staged


def _stage_cues(cues: dict[str, Any], candidates: list[dict[str, Any]]) -> dict[str, Any]:
    if (
        cues.get("schema") != "lostark.valtan-pattern-effect-cues"
        or cues.get("formatVersion") != 2
        or cues.get("ownerArchetypeId") != BOSS_ARCHETYPE_ID
    ):
        raise ProjectionError("Valtan cue document must remain exact v2")
    staged = deepcopy(cues)
    rows = _require_list(staged.get("cues"), "Valtan cues")
    for row in rows:
        _require_object(row, "Valtan cue row")
    binding_ids: dict[str, dict[str, Any]] = {}
    occurrence_ids: dict[str, dict[str, Any]] = {}
    for row in rows:
        binding_id = row.get("bindingId")
        occurrence_id = row.get("occurrenceId")
        if (
            not isinstance(binding_id, str)
            or not isinstance(occurrence_id, str)
            or binding_id in binding_ids
            or occurrence_id in occurrence_ids
        ):
            raise ProjectionError("Valtan cue IDs are invalid or duplicated")
        binding_ids[binding_id] = row
        occurrence_ids[occurrence_id] = row
    for candidate in candidates:
        expected = candidate["cueRow"]
        binding = binding_ids.get(expected["bindingId"])
        occurrence = occurrence_ids.get(expected["occurrenceId"])
        if (binding is None) != (occurrence is None):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED split source-wave cue identity: {candidate['effectAssetId']}"
            )
        if binding is not None:
            if binding is not occurrence or binding != expected:
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED source-wave cue drift: {candidate['effectAssetId']}"
                )
            continue
        copied = deepcopy(expected)
        rows.append(copied)
        binding_ids[copied["bindingId"]] = copied
        occurrence_ids[copied["occurrenceId"]] = copied
    rows.sort(key=lambda row: (row["patternId"], row["actionId"], row["bindingId"]))
    clip_rows = sorted(
        [row for row in rows if row.get("clipOccurrenceId") == CLIP_OCCURRENCE_ID],
        key=lambda row: (row["sourceStartMs"], row["occurrenceId"]),
    )
    expected_ids = [AGGREGATE_EFFECT_ID] + [row["effectAssetId"] for row in candidates]
    if [row["effectAssetId"] for row in clip_rows] != expected_ids:
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED All Effects clip does not resolve exactly five ordered cues"
        )
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
        else _path_inside(root, candidate_receipt, "candidate receipt")
    )
    receipt_relative = (
        receipt_path if isinstance(receipt_path, PurePosixPath)
        else _path_inside(root, receipt_path, "application receipt")
    )
    candidate_relative = _relative(candidate_relative, "candidate receipt")
    receipt_relative = _relative(receipt_relative, "application receipt")
    proof_relative = _path_inside(root, drawable_proof, "drawable proof")
    if receipt_relative in (candidate_relative, proof_relative):
        raise ProjectionError("application receipt collides with an immutable input")

    candidate_receipt_value, candidate_receipt_payload = _load_json(
        _repository_path(root, candidate_relative)
    )
    candidates = _validate_candidate_receipt(candidate_receipt_value)
    proof, proof_payload = _load_json(_repository_path(root, proof_relative))
    candidate_documents: dict[str, dict[str, Any]] = {}
    candidate_payloads: dict[str, bytes] = {}
    candidate_paths: dict[str, PurePosixPath] = {}
    candidate_elements: dict[str, list[dict[str, Any]]] = {}
    global_pairs: set[tuple[str, str]] = set()
    for row in candidates:
        effect_id = row["effectAssetId"]
        relative = _relative(row.get("candidateDocumentPath"), f"{effect_id} candidate path")
        document, payload = _load_json(_repository_path(root, relative))
        if (
            not _is_sha256(row.get("candidateDocumentSha256"))
            or _sha256(payload) != row["candidateDocumentSha256"]
        ):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED source-wave candidate SHA drift: {effect_id}"
            )
        elements = _validate_candidate_document(document, row)
        pairs = {(element["id"], element["sourceNode"]) for element in elements}
        if global_pairs.intersection(pairs):
            raise ProjectionError("a source element was assigned to more than one wave")
        global_pairs.update(pairs)
        candidate_documents[effect_id] = document
        candidate_payloads[effect_id] = payload
        candidate_paths[effect_id] = relative
        candidate_elements[effect_id] = elements
    if len(global_pairs) != EXPECTED_ELEMENT_COUNT:
        raise ProjectionError("source-wave element closure must be exactly 100")
    _validate_proof(
        proof, _sha256(candidate_receipt_payload), candidates, candidate_elements
    )

    prior_receipt_path = _repository_path(root, receipt_relative)
    prior_receipt: dict[str, Any] | None = None
    prior_receipt_payload: bytes | None = None
    if prior_receipt_path.is_file():
        prior_receipt, prior_receipt_payload = _load_json(prior_receipt_path)
        prior_inputs = _require_object(prior_receipt.get("inputs"), "prior receipt inputs")
        if (
            prior_receipt.get("schema") != RECEIPT_SCHEMA
            or prior_receipt.get("formatVersion") != FORMAT_VERSION
            or prior_receipt.get("transactionStatus") != "COMMITTED"
            or prior_inputs.get("candidateReceiptSha256")
            != _sha256(candidate_receipt_payload)
            or prior_inputs.get("drawableProofSha256") != _sha256(proof_payload)
        ):
            raise SourceRebaseRequired(
                "SOURCE_REBASE_REQUIRED existing source-wave application receipt is stale"
            )

    guards: dict[PurePosixPath, bytes | None] = {
        candidate_relative: candidate_receipt_payload,
        proof_relative: proof_payload,
        receipt_relative: prior_receipt_payload,
    }
    source_guard_hashes = {
        row["path"]: row["sha256"]
        for row in _require_list(candidate_receipt_value.get("sourceGuards"), "source guards")
        if isinstance(row, dict)
    }
    for path_text, expected_sha in sorted(source_guard_hashes.items()):
        relative = _relative(path_text, "source guard path")
        path = _repository_path(root, relative)
        try:
            payload = path.read_bytes()
        except OSError as exc:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED immutable source guard disappeared: {relative}"
            ) from exc
        if relative not in (CATALOG_PATH, CUE_PATH) and _sha256(payload) != expected_sha:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED immutable source guard drift: {relative}"
            )
        guards[relative] = payload

    aggregate_canary = candidate_receipt_value["aggregateCanary"]
    aggregate_relative = _relative(
        aggregate_canary["authoredDocumentPath"], "aggregate authored canary path"
    )
    aggregate_document, aggregate_payload = _load_json(
        _repository_path(root, aggregate_relative)
    )
    if _sha256(aggregate_payload) != aggregate_canary["authoredDocumentSha256"]:
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED aggregate authored byte canary drift"
        )
    guards[aggregate_relative] = aggregate_payload
    project_overlay_relative = _relative(
        aggregate_canary["projectOverlayPath"], "aggregate project overlay path"
    )
    project_overlay, project_overlay_payload = _load_json(
        _repository_path(root, project_overlay_relative)
    )
    if _sha256(project_overlay_payload) != aggregate_canary["projectOverlaySha256"]:
        raise SourceRebaseRequired("SOURCE_REBASE_REQUIRED aggregate project overlay drift")
    guards[project_overlay_relative] = project_overlay_payload
    project_plan_relative = _relative(
        aggregate_canary["projectPatchPlanPath"], "aggregate project plan path"
    )
    project_plan_payload = _repository_path(root, project_plan_relative).read_bytes()
    if _sha256(project_plan_payload) != aggregate_canary["projectPatchPlanSha256"]:
        raise SourceRebaseRequired("SOURCE_REBASE_REQUIRED aggregate project plan drift")
    guards[project_plan_relative] = project_plan_payload
    whirlwind = candidate_receipt_value["whirlwindCanary"]
    whirlwind_relative = _relative(
        whirlwind["authoredDocumentPath"], "Whirlwind canary path"
    )
    whirlwind_payload = _repository_path(root, whirlwind_relative).read_bytes()
    if _sha256(whirlwind_payload) != whirlwind["authoredDocumentSha256"]:
        raise SourceRebaseRequired("SOURCE_REBASE_REQUIRED Whirlwind active canary drift")
    guards[whirlwind_relative] = whirlwind_payload
    _validate_no_aggregate_source_duplication(
        aggregate_document, project_overlay, global_pairs
    )

    catalog, catalog_payload = _load_json(_repository_path(root, CATALOG_PATH))
    cues, cue_payload = _load_json(_repository_path(root, CUE_PATH))
    guards[CATALOG_PATH] = catalog_payload
    guards[CUE_PATH] = cue_payload
    if prior_receipt is None:
        if (
            _sha256(catalog_payload) != source_guard_hashes.get(CATALOG_PATH.as_posix())
            or _sha256(cue_payload) != source_guard_hashes.get(CUE_PATH.as_posix())
        ):
            raise SourceRebaseRequired(
                "SOURCE_REBASE_REQUIRED catalog/cue baseline drift before first transaction"
            )
    else:
        prior_catalog = _require_object(prior_receipt.get("catalogOutput"), "prior catalog output")
        prior_cue = _require_object(prior_receipt.get("cueOutput"), "prior cue output")
        if (
            _sha256(catalog_payload) != prior_catalog.get("sha256")
            or _sha256(cue_payload) != prior_cue.get("sha256")
        ):
            raise SourceRebaseRequired(
                "SOURCE_REBASE_REQUIRED catalog/cue drift after prior transaction"
            )
    catalog_rows = _require_list(catalog.get("effects"), "EffectCatalog.effects")
    aggregate_catalog = next(
        (row for row in catalog_rows if isinstance(row, dict) and row.get("effectAssetId") == AGGREGATE_EFFECT_ID),
        None,
    )
    cue_rows = _require_list(cues.get("cues"), "Valtan cues")
    aggregate_cue = next(
        (
            row for row in cue_rows
            if isinstance(row, dict)
            and row.get("effectAssetId") == AGGREGATE_EFFECT_ID
            and row.get("clipOccurrenceId") == CLIP_OCCURRENCE_ID
        ),
        None,
    )
    if (
        aggregate_catalog != aggregate_canary["catalogRow"]
        or _json_sha(aggregate_catalog) != aggregate_canary["catalogRowSha256"]
        or aggregate_cue != aggregate_canary["cueRow"]
        or _json_sha(aggregate_cue) != aggregate_canary["cueRowSha256"]
    ):
        raise SourceRebaseRequired("SOURCE_REBASE_REQUIRED aggregate catalog/cue canary drift")

    staged_catalog = _stage_catalog(catalog, candidates)
    staged_cues = _stage_cues(cues, candidates)
    staged_catalog_payload = _json_bytes_like(catalog_payload, staged_catalog)
    staged_cue_payload = _json_bytes_like(cue_payload, staged_cues)
    canonical_outputs: dict[PurePosixPath, bytes] = {
        CATALOG_PATH: staged_catalog_payload,
        CUE_PATH: staged_cue_payload,
    }
    target_receipts: list[dict[str, Any]] = []
    global_candidate_ids = {pair[0] for pair in global_pairs}
    global_candidate_sources = {pair[1] for pair in global_pairs}
    for candidate in candidates:
        effect_id = candidate["effectAssetId"]
        target_relative = _relative(
            candidate["targetAuthoredDocumentPath"], f"{effect_id} target path"
        )
        target_path = _repository_path(root, target_relative)
        if target_path.is_file():
            if prior_receipt is None:
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED unexpected source-wave authored document: {effect_id}"
                )
            canonical, canonical_payload = _load_json(target_path)
            if (
                canonical.get("schema") != "lostark.effect-authoring"
                or canonical.get("version") != 13
                or canonical.get("effectAssetId") != effect_id
            ):
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED source-wave authored identity drift: {effect_id}"
                )
            canonical_pairs = _element_pairs(canonical, effect_id)
            expected_pairs = {
                (row["id"], row["sourceNode"])
                for row in candidate_elements[effect_id]
            }
            projected_pairs = {
                pair
                for pair in canonical_pairs
                if pair[0] in global_candidate_ids
                or pair[1] in global_candidate_sources
            }
            if projected_pairs != expected_pairs:
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED source-wave element identity/partition drift: {effect_id}"
                )
            staged_document_payload = canonical_payload
        else:
            if prior_receipt is not None:
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED applied source-wave document disappeared: {effect_id}"
                )
            staged_document_payload = candidate_payloads[effect_id]
        guards[target_relative] = target_path.read_bytes() if target_path.is_file() else None
        canonical_outputs[target_relative] = staged_document_payload
        target_receipts.append(
            {
                "waveOrdinal": candidate["waveOrdinal"],
                "waveId": candidate["waveId"],
                "displayLabel": candidate["displayLabel"],
                "presentationRole": candidate["presentationRole"],
                "gameplayHitDisposition": candidate["gameplayHitDisposition"],
                "effectAssetId": effect_id,
                "candidateDocumentPath": candidate_paths[effect_id].as_posix(),
                "candidateDocumentSha256": _sha256(candidate_payloads[effect_id]),
                "targetAuthoredDocumentPath": target_relative.as_posix(),
                "candidateElementCount": 25,
                "cueSourceStartMs": candidate["cueSourceStartMs"],
                "cueLocalStartDelaySeconds": candidate["cueLocalStartDelaySeconds"],
                "catalogRow": deepcopy(candidate["catalogRow"]),
                "cueRow": deepcopy(candidate["cueRow"]),
                "finalDocumentSha256": _sha256(staged_document_payload),
            }
        )
        guards[candidate_paths[effect_id]] = candidate_payloads[effect_id]

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
            "animationBindingMutation": "FORBIDDEN",
            "aggregateDocumentMutation": "FORBIDDEN",
            "aggregateProjectOverlayMutation": "FORBIDDEN",
            "aggregateSourceElementAppend": "FORBIDDEN",
            "gameplayAuthorityMutation": "FORBIDDEN",
            "fourthWaveRole": "AUXILIARY_SOURCE_WAVE_NOT_GAMEPLAY_HIT",
            "failureAction": "ROLLBACK_ALL_OUTPUTS",
        },
        "closure": {
            "sourceWaveDocumentCount": 4,
            "sourceElementCount": 100,
            "elementsPerSourceWave": 25,
            "catalogAppendedOrPreservedRowCount": 4,
            "cueAppendedOrPreservedRowCount": 4,
            "allEffectsClipCueCount": 5,
            "aggregateSourceElementAppendCount": 0,
            "duplicateSourceElementCount": 0,
            "gameplayMutationCount": 0,
        },
        "allEffectsClipDisplay": deepcopy(candidate_receipt_value["allEffectsClipDisplay"]),
        "aggregateCanary": deepcopy(aggregate_canary),
        "whirlwindCanary": deepcopy(whirlwind),
        "targets": target_receipts,
        "catalogOutput": {
            "path": CATALOG_PATH.as_posix(),
            "sha256": _sha256(staged_catalog_payload),
            "appendedOrPreservedRows": [deepcopy(row["catalogRow"]) for row in candidates],
        },
        "cueOutput": {
            "path": CUE_PATH.as_posix(),
            "sha256": _sha256(staged_cue_payload),
            "appendedOrPreservedRows": [deepcopy(row["cueRow"]) for row in candidates],
        },
        "canonicalOutputs": [
            {
                "path": relative.as_posix(),
                "role": (
                    "EFFECT_CATALOG" if relative == CATALOG_PATH else
                    "VALTAN_CUE_DOCUMENT" if relative == CUE_PATH else
                    "AUTHORED_EFFECT_DOCUMENT"
                ),
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
        tempfile.mkdtemp(prefix=".valtan-front-back-front-waves.", dir=root)
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
            staged = staged_root.joinpath(*relative.parts)
            staged.parent.mkdir(parents=True, exist_ok=True)
            staged.write_bytes(projection.outputs[relative])
            if staged.read_bytes() != projection.outputs[relative]:
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
                "application failed and rollback was incomplete: " + "; ".join(rollback_errors)
            ) from exc
        if isinstance(exc, ProjectionError):
            raise
        raise ProjectionError(f"application failed; all outputs rolled back: {exc}") from exc
    finally:
        shutil.rmtree(transaction_root, ignore_errors=True)


def check_projection(projection: Projection) -> None:
    stale = []
    for relative, expected in projection.outputs.items():
        path = _repository_path(projection.repository_root, relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            stale.append(relative.as_posix())
    if stale:
        raise ProjectionError(
            "source-wave projection is missing or stale: " + ", ".join(sorted(stale))
        )


def _summary(projection: Projection, mode: str) -> str:
    closure = projection.receipt["closure"]
    return (
        f"[PASS] FRONT_BACK_FRONT four source waves {mode}: "
        f"documents={closure['sourceWaveDocumentCount']}, "
        f"elements={closure['sourceElementCount']}, "
        f"clipCues={closure['allEffectsClipCueCount']}, "
        f"canonicalOutputs={len(projection.canonical_outputs)}, "
        f"changed={len(projection.changed_paths)}, new={len(projection.new_paths)}, "
        "aggregateAppend=0, gameplayMutations=0"
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
