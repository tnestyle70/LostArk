#!/usr/bin/env python3
"""Project drawable-proven Valtan priority overlays into canonical authoring.

The command is intentionally narrower than the normal Effect publisher.  It
consumes the immutable nine-target patch plan plus an independently recorded
per-element drawable proof, stages every canonical output in memory, validates
the complete catalog/cue/document closure, and only writes with explicit
``--apply``.  Existing element objects are never rewritten: a missing stable
``(id, sourceNode)`` pair is appended, while a matching row is preserved as-is.
"""

from __future__ import annotations

import argparse
from copy import deepcopy
from dataclasses import dataclass
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
from typing import Any, Iterable, Mapping


PATCH_SCHEMA = "lostark.valtan-project-authored-priority-patch-plan"
PROOF_SCHEMA = "lostark.valtan-project-authored-priority-drawable-proof"
RECEIPT_SCHEMA = "lostark.valtan-project-authored-priority-projection-receipt"
FORMAT_VERSION = 1
OWNER_ARCHETYPE_ID = "BOSS_VALTAN"
EXPECTED_TARGET_COUNT = 9

DEFAULT_PATCH_PLAN = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/"
    "Valtan.project-authored-priority.patch-plan.v1.json"
)
DEFAULT_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/"
    "Valtan.project-authored-priority.projection-receipt.v1.json"
)
CATALOG_PATH = PurePosixPath("Data/Effects/EffectCatalog.json")
CUE_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
PATTERN_BINDINGS_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
WHIRLWIND_EFFECT_ID = "effect.valtan.pattern.420633.active"
WHIRLWIND_PATTERN_ID = "VALTAN_WHIRLWIND"
WHIRLWIND_REQUIRED_FILES = (
    PurePosixPath(
        "Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json"
    ),
    PurePosixPath(
        "Data/Effects/Imported/Valtan/"
        "Valtan.420633.whirlwind-baked-edge-history.v1.json"
    ),
)
WHIRLWIND_OPTIONAL_FILES = (
    PurePosixPath("Data/Animation/Authored/Valtan/Valtan.patterneffects.json"),
    PurePosixPath(
        "Data/Effects/Imported/Valtan/Valtan.trail-adapter-packets.v1.json"
    ),
    PurePosixPath(
        "Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json"
    ),
    PurePosixPath(
        "Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
    ),
)

SHA256_LENGTH = 64
DISABLED_SOURCE_RECIPE = {
    "enabled": False,
    "rendererShape": "",
    "emitterDelaySeconds": 0,
    "emitterDurationSeconds": 0,
    "emitterLoopCount": 1,
    "bursts": [],
    "modules": [],
}
HIGH_JUMP_EFFECT_ID = "effect.valtan.high-jump.airborne"
HIGH_JUMP_AXE_MODEL_ASSET_ID = "Character/Valtan/ValtanWeapon.wmodel"
HIGH_JUMP_CATALOG_ROW = {
    "effectAssetId": HIGH_JUMP_EFFECT_ID,
    "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
    "authoringPath": (
        "Effects/Authored/effect.valtan.high-jump.airborne.effect.json"
    ),
}
HIGH_JUMP_CUE_ROW = {
    "bindingId": "cue.valtan.high-jump.airborne.project-authored",
    "occurrenceId": (
        "cue.valtan.high-jump.airborne.project-authored.occurrence.01"
    ),
    "patternId": "VALTAN_HIGH_JUMP",
    "stageId": "AIRBORNE",
    "actionId": "valtan.attack.high-jump.airborne",
    "clipOccurrenceId": "valtan.attack.high-jump.airborne.clip.01",
    "effectAssetId": HIGH_JUMP_EFFECT_ID,
    "anchorSlotId": "root",
    "followPolicy": "snapshot",
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


class ProjectionError(RuntimeError):
    """Raised when the projection cannot prove an atomic safe result."""


class SourceRebaseRequired(ProjectionError):
    """Raised when an immutable projected identity changed in canonical data."""


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
        and len(value) == SHA256_LENGTH
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
        value = json.loads(
            payload.decode("utf-8"), object_pairs_hook=no_duplicates
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ProjectionError(f"invalid JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ProjectionError(f"JSON root must be an object: {path}")
    return value, payload


def _relative_path(value: str | PurePosixPath, label: str) -> PurePosixPath:
    text = value.as_posix() if isinstance(value, PurePosixPath) else value
    if not isinstance(text, str) or not text or "\\" in text:
        raise ProjectionError(f"{label} must be a repository-relative POSIX path")
    path = PurePosixPath(text)
    if path.is_absolute() or ".." in path.parts or "." in path.parts:
        raise ProjectionError(f"{label} escapes the repository: {text}")
    return path


def _repository_path(root: Path, relative: PurePosixPath) -> Path:
    candidate = root.joinpath(*relative.parts)
    try:
        candidate.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise ProjectionError(
            f"repository output escaped root: {relative.as_posix()}"
        ) from exc
    return candidate


def _path_inside_repository(root: Path, path: Path, label: str) -> PurePosixPath:
    absolute = path if path.is_absolute() else root / path
    try:
        relative = absolute.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise ProjectionError(f"{label} must be inside the repository") from exc
    return _relative_path(relative.as_posix(), label)


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise ProjectionError(f"{label} must be an array")
    return value


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProjectionError(f"{label} must be an object")
    return value


def _validate_patch_plan(plan: dict[str, Any]) -> None:
    if (
        plan.get("schema") != PATCH_SCHEMA
        or plan.get("formatVersion") != FORMAT_VERSION
        or plan.get("ownerArchetypeId") != OWNER_ARCHETYPE_ID
    ):
        raise ProjectionError("project-authored patch-plan identity is invalid")
    policy = _require_object(plan.get("policy"), "patch-plan.policy")
    if (
        policy.get("reconcileMode") != "MISSING_ONLY"
        or policy.get("canonicalMutationPerformed") is not False
        or policy.get("sourceRecipeEnabled") is not False
        or policy.get("sourcePresentationEnabled") is not False
    ):
        raise ProjectionError("project-authored patch-plan safety policy changed")

    targets = _require_list(plan.get("targets"), "patch-plan.targets")
    if len(targets) != EXPECTED_TARGET_COUNT:
        raise ProjectionError("priority target count must remain nine")
    seen_effects: set[str] = set()
    for index, raw_target in enumerate(targets):
        target = _require_object(raw_target, f"patch-plan.targets[{index}]")
        effect_id = target.get("targetEffectAssetId")
        if not isinstance(effect_id, str) or effect_id in seen_effects:
            raise ProjectionError("priority target Effect IDs must be unique")
        seen_effects.add(effect_id)
        expected_target = (
            f"Data/Effects/Authored/{effect_id}.effect.json"
        )
        if target.get("targetAuthoringPath") != expected_target:
            raise ProjectionError(
                f"target authoring path is not canonical: {effect_id}"
            )
        if target.get("canonicalState") not in (
            "EXISTING_DOCUMENT",
            "MISSING_DOCUMENT",
        ):
            raise ProjectionError(f"target canonical state is invalid: {effect_id}")
        if (
            (effect_id == HIGH_JUMP_EFFECT_ID)
            != (target.get("canonicalState") == "MISSING_DOCUMENT")
        ):
            raise ProjectionError(
                "HIGH_JUMP AIRBORNE must remain the only missing canonical document"
            )
        overlay_path = target.get("overlayDocumentPath")
        overlay_sha = target.get("overlayDocumentSha256")
        if not isinstance(overlay_path, str) or not _is_sha256(overlay_sha):
            raise ProjectionError(f"target candidate identity is invalid: {effect_id}")
        desired = _require_list(
            target.get("desiredElements"), f"{effect_id}.desiredElements"
        )
        if not desired:
            raise ProjectionError(f"target has no desired elements: {effect_id}")
        desired_pairs: set[tuple[str, str]] = set()
        for row in desired:
            row = _require_object(row, f"{effect_id}.desiredElement")
            element_id = row.get("elementId")
            source_node = row.get("sourceNode")
            action = row.get("reconcileAction")
            if (
                not isinstance(element_id, str)
                or not isinstance(source_node, str)
                or not source_node.startswith("project-authored:")
                or (element_id, source_node) in desired_pairs
                or action not in ("APPEND_MISSING", "PRESERVE_EXISTING")
            ):
                raise ProjectionError(
                    f"desired stable identity is invalid: {effect_id}"
                )
            desired_pairs.add((element_id, source_node))

    patch = _require_object(
        plan.get("highJumpAirbornePatch"), "highJumpAirbornePatch"
    )
    if patch.get("catalogRow") != HIGH_JUMP_CATALOG_ROW:
        raise ProjectionError("HIGH_JUMP AIRBORNE catalog row changed")
    if patch.get("cueRow") != HIGH_JUMP_CUE_ROW:
        raise ProjectionError("HIGH_JUMP AIRBORNE cue row changed")
    authority = _require_object(patch.get("authority"), "highJump.authority")
    if (
        authority.get("presentationOnly") is not True
        or authority.get("serverGameplayChange") is not False
        or authority.get("projectileAuthorityStatus")
        != "PRESENTATION_ONLY_OFFICIAL_ASSET_REUSE"
    ):
        raise ProjectionError("HIGH_JUMP AIRBORNE authority boundary changed")

    presentations = _require_list(
        plan.get("projectilePresentations"),
        "patch-plan.projectilePresentations",
    )
    expected_presentations = {
        f"project-authored:valtan.high-jump.airborne.beat-{beat:02d}.axe"
        for beat in (1, 2, 3)
    }
    actual_presentations = {
        row.get("presentationId")
        for row in presentations
        if isinstance(row, dict)
        and row.get("disposition")
        == "PROJECT_AUTHORED_OFFICIAL_ASSET_REUSE"
        and row.get("modelAssetId") == HIGH_JUMP_AXE_MODEL_ASSET_ID
        and row.get("geometryProvenance") == "OFFICIAL_GEOMETRY_EXACT"
        and row.get("baseTextureProvenance")
        == "OFFICIAL_MODEL_MATERIAL_BASE_TEXTURE_EXACT"
        and row.get("trajectoryTimingProvenance") == "PROJECT_AUTHORED"
        and row.get("sourceActionPayloadClaim") == "NONE"
        and row.get("presentationOnly") is True
        and row.get("serverGameplayChange") is False
    }
    if (
        len(presentations) != 3
        or actual_presentations != expected_presentations
    ):
        raise ProjectionError(
            "three presentation-only HIGH_JUMP official axe rows must remain explicit"
        )


def _validate_candidate_document(document: dict[str, Any], effect_id: str) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
    ):
        raise ProjectionError(f"candidate Effect identity is invalid: {effect_id}")
    elements = _require_list(document.get("elements"), f"{effect_id}.elements")
    if not elements:
        raise ProjectionError(f"candidate Effect has no elements: {effect_id}")
    seen_ids: set[str] = set()
    seen_nodes: set[str] = set()
    for raw_element in elements:
        element = _require_object(raw_element, f"{effect_id}.element")
        element_id = element.get("id")
        source_node = element.get("sourceNode")
        if (
            not isinstance(element_id, str)
            or not isinstance(source_node, str)
            or not source_node.startswith("project-authored:")
            or element_id in seen_ids
            or source_node in seen_nodes
        ):
            raise ProjectionError(f"candidate stable identity is invalid: {effect_id}")
        seen_ids.add(element_id)
        seen_nodes.add(source_node)
        if element.get("kind") == "light":
            raise ProjectionError("priority candidate cannot add standalone Light")
        if element.get("sourceRecipe") != DISABLED_SOURCE_RECIPE:
            raise ProjectionError(
                f"priority candidate sourceRecipe changed: {effect_id}/{element_id}"
            )
        if element.get("sourcePresentation") != {"enabled": False}:
            raise ProjectionError(
                f"priority candidate sourcePresentation changed: {effect_id}/{element_id}"
            )
        material = _require_object(
            element.get("material"), f"{effect_id}/{element_id}.material"
        )
        if material.get("renderProfile") not in {
            "alpha_two_sided_depth_read",
            "additive_two_sided_depth_read",
        }:
            raise ProjectionError(
                f"priority candidate material profile is not admitted: {effect_id}/{element_id}"
            )
        timing = _require_object(
            _require_object(element.get("detail"), "element.detail").get("timing"),
            "element.detail.timing",
        )
        start = timing.get("startDelaySeconds")
        lifetime = timing.get("lifeTimeSeconds")
        if (
            not isinstance(start, (int, float))
            or isinstance(start, bool)
            or not isinstance(lifetime, (int, float))
            or isinstance(lifetime, bool)
            or not math.isfinite(float(start))
            or not math.isfinite(float(lifetime))
            or start < 0
            or lifetime <= 0
        ):
            raise ProjectionError(
                f"priority candidate timing is invalid: {effect_id}/{element_id}"
            )

    if effect_id == HIGH_JUMP_EFFECT_ID:
        expected_by_kind = {
            "decal": {
                f"project-axe-beat-{beat:02d}-target-decal"
                for beat in (1, 2, 3)
            },
            "mesh": {
                f"project-axe-beat-{beat:02d}-falling-axe"
                for beat in (1, 2, 3)
            },
            "particle": {
                f"project-axe-beat-{beat:02d}-ground-impact"
                for beat in (1, 2, 3)
            },
        }
        actual_by_kind = {
            kind: {
                element["id"]
                for element in elements
                if element.get("kind") == kind
            }
            for kind in expected_by_kind
        }
        if (
            len(elements) != 9
            or actual_by_kind != expected_by_kind
            or any(element.get("kind") not in expected_by_kind for element in elements)
        ):
            raise ProjectionError(
                "HIGH_JUMP AIRBORNE family denominator changed"
            )
        for element in elements:
            if element.get("kind") != "mesh":
                continue
            detail = _require_object(element.get("detail"), "axe.detail")
            mesh = _require_object(detail.get("mesh"), "axe.detail.mesh")
            action_attachment = _require_object(
                element.get("actionCueAttachment"),
                "axe.actionCueAttachment",
            )
            if (
                element.get("resources")
                != [{
                    "slotId": "meshModel",
                    "assetId": HIGH_JUMP_AXE_MODEL_ASSET_ID,
                }]
                or mesh.get("useModelMaterial") is not True
                or mesh.get("modelPreScale") != 1.0
                or action_attachment.get("enabled") is not False
                or action_attachment.get("follow") is not False
            ):
                raise ProjectionError(
                    "HIGH_JUMP Mesh must reuse the official Valtan axe as an "
                    "independent snapshot-root presentation"
                )


def _validate_proof(
    proof: dict[str, Any],
    plan_sha: str,
    targets: list[dict[str, Any]],
    candidates: Mapping[str, dict[str, Any]],
    root: Path,
) -> tuple[PurePosixPath, bytes, str]:
    if tuple(proof.keys()) != (
        "schema",
        "formatVersion",
        "ownerArchetypeId",
        "patchPlanSha256",
        "drawableSweepPath",
        "drawableSweepSha256",
        "resourceRoot",
        "targets",
    ):
        raise ProjectionError("drawable proof root fields/order are invalid")
    if (
        proof.get("schema") != PROOF_SCHEMA
        or proof.get("formatVersion") != FORMAT_VERSION
        or proof.get("ownerArchetypeId") != OWNER_ARCHETYPE_ID
        or proof.get("patchPlanSha256") != plan_sha
    ):
        raise ProjectionError("drawable proof identity or patch-plan SHA is stale")

    sweep_relative = _relative_path(
        proof.get("drawableSweepPath"), "drawable proof sweep path"
    )
    sweep, sweep_payload = _load_json_bytes(
        _repository_path(root, sweep_relative)
    )
    if (
        not _is_sha256(proof.get("drawableSweepSha256"))
        or _sha256(sweep_payload) != proof["drawableSweepSha256"]
    ):
        raise ProjectionError("drawable proof sweep SHA is stale")
    if tuple(sweep.keys()) != (
        "schema",
        "formatVersion",
        "resourceRoot",
        "sampleRateHz",
        "documents",
    ) or (
        sweep.get("schema") != "lostark.effect-document-drawable-sweep"
        or sweep.get("formatVersion") != FORMAT_VERSION
        or sweep.get("sampleRateHz") != 60
        or sweep.get("resourceRoot") != proof.get("resourceRoot")
    ):
        raise ProjectionError("drawable proof sweep identity is invalid")
    resource_root = proof.get("resourceRoot")
    if not isinstance(resource_root, str) or not Path(resource_root).is_absolute():
        raise ProjectionError("drawable proof resource root is not absolute")
    try:
        resolved_resource_root = Path(resource_root).resolve(strict=True)
    except OSError as exc:
        raise ProjectionError("drawable proof resource root is stale") from exc
    if not resolved_resource_root.is_dir():
        raise ProjectionError("drawable proof resource root is not a directory")
    sweep_documents = [
        _require_object(row, "drawable-sweep.document")
        for row in _require_list(sweep.get("documents"), "drawable-sweep.documents")
    ]
    if len(sweep_documents) != EXPECTED_TARGET_COUNT:
        raise ProjectionError("drawable proof sweep must cover all nine candidates")

    rows = _require_list(proof.get("targets"), "drawable-proof.targets")
    if len(rows) != EXPECTED_TARGET_COUNT:
        raise ProjectionError("drawable proof must cover all nine candidates")
    proof_by_effect: dict[str, dict[str, Any]] = {}
    for raw_row in rows:
        row = _require_object(raw_row, "drawable-proof.target")
        if tuple(row.keys()) != (
            "effectAssetId",
            "overlayDocumentSha256",
            "disposition",
            "elements",
        ):
            raise ProjectionError("drawable proof target fields/order are invalid")
        effect_id = row.get("effectAssetId")
        if not isinstance(effect_id, str) or effect_id in proof_by_effect:
            raise ProjectionError("drawable proof target identity is duplicated")
        proof_by_effect[effect_id] = row

    for target_index, target in enumerate(targets):
        effect_id = target["targetEffectAssetId"]
        row = proof_by_effect.get(effect_id)
        if row is None:
            raise ProjectionError(f"drawable proof is missing target: {effect_id}")
        if (
            row.get("overlayDocumentSha256")
            != target["overlayDocumentSha256"]
            or row.get("disposition") != "DRAWABLE_PROOF_PASS"
        ):
            raise ProjectionError(f"drawable proof did not admit target: {effect_id}")
        element_rows = _require_list(
            row.get("elements"), f"drawable-proof.{effect_id}.elements"
        )
        sweep_document = sweep_documents[target_index]
        if tuple(sweep_document.keys()) != (
            "documentPath",
            "effectAssetId",
            "durationSeconds",
            "sampleCount",
            "visibleElementCount",
            "preparedElementCount",
            "drawnElementCount",
            "disposition",
            "elements",
        ) or sweep_document.get("effectAssetId") != effect_id:
            raise ProjectionError(
                f"drawable proof sweep target identity changed: {effect_id}"
            )
        recorded_document_path = sweep_document.get("documentPath")
        if (
            not isinstance(recorded_document_path, str)
            or not Path(recorded_document_path).is_absolute()
        ):
            raise ProjectionError(
                f"drawable proof sweep document path is invalid: {effect_id}"
            )
        overlay_relative = _relative_path(
            target["overlayDocumentPath"], f"{effect_id} overlay path"
        )
        try:
            resolved_recorded_path = Path(recorded_document_path).resolve(strict=True)
            resolved_overlay_path = _repository_path(
                root, overlay_relative
            ).resolve(strict=True)
        except OSError as exc:
            raise ProjectionError(
                f"drawable proof sweep document path is stale: {effect_id}"
            ) from exc
        if resolved_recorded_path != resolved_overlay_path:
            raise ProjectionError(
                f"drawable proof sweep document path changed: {effect_id}"
            )
        sweep_elements = _require_list(
            sweep_document.get("elements"),
            f"drawable-sweep.{effect_id}.elements",
        )
        if sweep_elements != element_rows:
            raise ProjectionError(
                f"drawable proof does not match recorded sweep: {effect_id}"
            )
        expected_ids = [
            element["id"] for element in candidates[effect_id]["elements"]
        ]
        sample_count = sweep_document.get("sampleCount")
        duration = sweep_document.get("durationSeconds")
        if (
            sweep_document.get("disposition") != "DRAWABLE_PROOF_PASS"
            or type(sample_count) is not int
            or sample_count < 2
            or not isinstance(duration, (int, float))
            or isinstance(duration, bool)
            or not math.isfinite(float(duration))
            or duration <= 0
            or duration > 60
            or any(
                type(sweep_document.get(key)) is not int
                or sweep_document[key] != len(expected_ids)
                for key in (
                    "visibleElementCount",
                    "preparedElementCount",
                    "drawnElementCount",
                )
            )
        ):
            raise ProjectionError(
                f"drawable proof sweep document closure changed: {effect_id}"
            )
        if [element.get("elementId") for element in element_rows] != expected_ids:
            raise ProjectionError(
                f"drawable proof element denominator changed: {effect_id}"
            )
        for element in element_rows:
            if tuple(element.keys()) != (
                "elementId",
                "disposition",
                "preparedSamples",
                "attemptedSamples",
                "submittedDraws",
                "suppressedDraws",
                "failedDraws",
                "committedDraws",
            ):
                raise ProjectionError(
                    f"drawable proof element fields/order are invalid: {effect_id}"
                )
            if element.get("disposition") != "DRAWABLE_PROOF_PASS":
                raise ProjectionError(
                    f"candidate element lacks drawable PASS: {effect_id}"
                )
            for key in (
                "preparedSamples",
                "attemptedSamples",
                "submittedDraws",
                "committedDraws",
            ):
                value = element.get(key)
                if type(value) is not int or value < 1:
                    raise ProjectionError(
                        f"drawable proof {key} is invalid: {effect_id}"
                    )
            if (
                element["preparedSamples"] > sample_count
                or element["attemptedSamples"] > sample_count
            ):
                raise ProjectionError(
                    f"drawable proof sample count is invalid: {effect_id}"
                )
            for key in ("suppressedDraws", "failedDraws"):
                value = element.get(key)
                if type(value) is not int or value < 0:
                    raise ProjectionError(
                        f"drawable proof {key} is invalid: {effect_id}"
                    )
            if element["failedDraws"] != 0:
                raise ProjectionError(
                    f"drawable proof recorded failed draws: {effect_id}"
                )
    if set(proof_by_effect) != {row["targetEffectAssetId"] for row in targets}:
        raise ProjectionError("drawable proof contains an unknown target")
    return sweep_relative, sweep_payload, resource_root


def _element_indexes(
    document: dict[str, Any], effect_id: str
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    by_id: dict[str, dict[str, Any]] = {}
    by_source: dict[str, dict[str, Any]] = {}
    for raw_element in _require_list(document.get("elements"), f"{effect_id}.elements"):
        element = _require_object(raw_element, f"{effect_id}.element")
        element_id = element.get("id")
        source_node = element.get("sourceNode")
        if not isinstance(element_id, str) or not isinstance(source_node, str):
            raise ProjectionError(f"canonical element identity is invalid: {effect_id}")
        if element_id in by_id:
            raise ProjectionError(f"duplicate canonical element ID: {effect_id}/{element_id}")
        by_id[element_id] = element
        if source_node:
            if source_node in by_source:
                raise ProjectionError(
                    f"duplicate canonical sourceNode: {effect_id}/{source_node}"
                )
            by_source[source_node] = element
    return by_id, by_source


def _immutable_projection_identity(element: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": element.get("id"),
        "sourceNode": element.get("sourceNode"),
        "groupId": element.get("groupId"),
        "kind": element.get("kind"),
        "resources": element.get("resources"),
        "material": element.get("material"),
        "actionCueAttachment": element.get("actionCueAttachment"),
        "transformInheritance": element.get("transformInheritance"),
        "sourceRecipe": element.get("sourceRecipe"),
        "sourcePresentation": element.get("sourcePresentation"),
    }


def _reconcile_document(
    canonical: dict[str, Any] | None,
    candidate: dict[str, Any],
    target: dict[str, Any],
) -> tuple[dict[str, Any], list[str], list[str]]:
    effect_id = target["targetEffectAssetId"]
    candidate_by_pair = {
        (element["id"], element["sourceNode"]): element
        for element in candidate["elements"]
    }
    desired = _require_list(target.get("desiredElements"), f"{effect_id}.desired")
    append_pairs = [
        (row["elementId"], row["sourceNode"])
        for row in desired
        if row["reconcileAction"] == "APPEND_MISSING"
    ]
    preserve_pairs = [
        (row["elementId"], row["sourceNode"])
        for row in desired
        if row["reconcileAction"] == "PRESERVE_EXISTING"
    ]
    if set(candidate_by_pair) != set(append_pairs):
        raise ProjectionError(
            f"candidate elements differ from APPEND_MISSING plan: {effect_id}"
        )
    if canonical is None:
        if preserve_pairs:
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED preserved document is missing: {effect_id}"
            )
        staged = deepcopy(candidate)
        appended = [element["id"] for element in staged["elements"]]
        return staged, appended, []
    if (
        canonical.get("schema") != "lostark.effect-authoring"
        or canonical.get("version") != 13
        or canonical.get("effectAssetId") != effect_id
    ):
        raise ProjectionError(f"canonical Effect identity is invalid: {effect_id}")

    staged = deepcopy(canonical)
    by_id, by_source = _element_indexes(staged, effect_id)

    preserved: list[str] = []
    for element_id, source_node in preserve_pairs:
        existing_by_id = by_id.get(element_id)
        existing_by_source = by_source.get(source_node)
        if (
            existing_by_id is None
            or existing_by_source is None
            or existing_by_id is not existing_by_source
        ):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED preserved identity changed: {effect_id}/{element_id}"
            )
        preserved.append(element_id)

    appended: list[str] = []
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
            if _immutable_projection_identity(existing_by_id) != (
                _immutable_projection_identity(candidate_element)
            ):
                raise SourceRebaseRequired(
                    f"SOURCE_REBASE_REQUIRED immutable recipe/material drift: {effect_id}/{element_id}"
                )
            preserved.append(element_id)
            continue
        copied = deepcopy(candidate_element)
        staged["elements"].append(copied)
        by_id[element_id] = copied
        by_source[source_node] = copied
        appended.append(element_id)

    _element_indexes(staged, effect_id)
    return staged, appended, preserved


def _validate_catalog(catalog: dict[str, Any]) -> None:
    if catalog.get("formatVersion") != 1:
        raise ProjectionError("EffectCatalog.json must remain formatVersion 1")
    rows = _require_list(catalog.get("effects"), "EffectCatalog.effects")
    ids: list[str] = []
    for raw_row in rows:
        row = _require_object(raw_row, "EffectCatalog row")
        effect_id = row.get("effectAssetId")
        if not isinstance(effect_id, str) or effect_id in ids:
            raise ProjectionError("EffectCatalog IDs are invalid or duplicated")
        ids.append(effect_id)
    if ids != sorted(ids):
        raise ProjectionError("EffectCatalog rows are not sorted by effectAssetId")


def _stage_catalog(catalog: dict[str, Any]) -> tuple[dict[str, Any], bool]:
    _validate_catalog(catalog)
    staged = deepcopy(catalog)
    rows = staged["effects"]
    existing = next(
        (row for row in rows if row["effectAssetId"] == HIGH_JUMP_EFFECT_ID),
        None,
    )
    appended = existing is None
    if existing is not None and existing != HIGH_JUMP_CATALOG_ROW:
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED HIGH_JUMP AIRBORNE catalog row drift"
        )
    if existing is None:
        rows.append(deepcopy(HIGH_JUMP_CATALOG_ROW))
        rows.sort(key=lambda row: row["effectAssetId"])
    _validate_catalog(staged)
    return staged, appended


def _validate_pattern_bindings(bindings: dict[str, Any]) -> dict[str, set[str]]:
    if bindings.get("formatVersion") != 2:
        raise ProjectionError("Valtan.patternbindings.json must remain formatVersion 2")
    rows = _require_list(bindings.get("bindings"), "patternbindings.bindings")
    action_clips: dict[str, set[str]] = {}
    all_clip_occurrences: set[str] = set()
    for raw_row in rows:
        row = _require_object(raw_row, "pattern binding")
        action_id = row.get("actionId")
        if not isinstance(action_id, str) or action_id in action_clips:
            raise ProjectionError("Valtan action binding IDs are invalid or duplicated")
        clips: set[str] = set()
        for raw_clip in _require_list(row.get("clips"), f"{action_id}.clips"):
            clip = _require_object(raw_clip, f"{action_id}.clip")
            occurrence = clip.get("clipOccurrenceId")
            if (
                not isinstance(occurrence, str)
                or occurrence in clips
                or occurrence in all_clip_occurrences
            ):
                raise ProjectionError("Valtan clip occurrence IDs are duplicated")
            clips.add(occurrence)
            all_clip_occurrences.add(occurrence)
        action_clips[action_id] = clips
    return action_clips


def _validate_cues(
    cues: dict[str, Any],
    action_clips: Mapping[str, set[str]],
    catalog_effect_ids: set[str],
) -> None:
    if (
        cues.get("schema") != "lostark.valtan-pattern-effect-cues"
        or cues.get("formatVersion") != 2
        or cues.get("ownerArchetypeId") != OWNER_ARCHETYPE_ID
    ):
        raise ProjectionError("Valtan cue v2 document identity is invalid")
    rows = _require_list(cues.get("cues"), "Valtan cues")
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    semantic_ids: set[tuple[str, str, str, str, str]] = set()
    for raw_row in rows:
        row = _require_object(raw_row, "Valtan cue row")
        binding_id = row.get("bindingId")
        occurrence_id = row.get("occurrenceId")
        action_id = row.get("actionId")
        clip_occurrence = row.get("clipOccurrenceId")
        effect_id = row.get("effectAssetId")
        if (
            not isinstance(binding_id, str)
            or binding_id in binding_ids
            or not isinstance(occurrence_id, str)
            or occurrence_id in occurrence_ids
        ):
            raise ProjectionError("Valtan cue v2 identities are invalid or duplicated")
        binding_ids.add(binding_id)
        occurrence_ids.add(occurrence_id)
        semantic = (
            row.get("patternId"),
            row.get("stageId"),
            action_id,
            clip_occurrence,
            effect_id,
        )
        if semantic in semantic_ids:
            raise ProjectionError("Valtan cue semantic occurrence is duplicated")
        semantic_ids.add(semantic)
        if action_id not in action_clips or clip_occurrence not in action_clips[action_id]:
            raise ProjectionError(
                f"Valtan cue clip occurrence does not belong to action: {binding_id}"
            )
        if effect_id not in catalog_effect_ids:
            raise ProjectionError(f"Valtan cue Effect is not catalogued: {binding_id}")
        if row.get("repeatPolicy") not in ("once", "each_loop"):
            raise ProjectionError(f"Valtan cue repeatPolicy is invalid: {binding_id}")
        if row.get("followPolicy") not in ("follow", "snapshot"):
            raise ProjectionError(f"Valtan cue followPolicy is invalid: {binding_id}")
        if row.get("stopPolicy") not in ("natural", "cue_end"):
            raise ProjectionError(f"Valtan cue stopPolicy is invalid: {binding_id}")
        start = row.get("sourceStartMs")
        end = row.get("sourceEndMs")
        if type(start) is not int or start < 0:
            raise ProjectionError(f"Valtan cue start is invalid: {binding_id}")
        if end is not None and (type(end) is not int or end <= start):
            raise ProjectionError(f"Valtan cue end is invalid: {binding_id}")


def _stage_cues(
    cues: dict[str, Any],
    action_clips: Mapping[str, set[str]],
    catalog_effect_ids: set[str],
) -> tuple[dict[str, Any], bool]:
    _validate_cues(cues, action_clips, catalog_effect_ids)
    staged = deepcopy(cues)
    rows = staged["cues"]
    by_binding = {
        row["bindingId"]: row for row in rows
    }
    by_occurrence = {
        row["occurrenceId"]: row for row in rows
    }
    semantic_key = (
        HIGH_JUMP_CUE_ROW["patternId"],
        HIGH_JUMP_CUE_ROW["stageId"],
        HIGH_JUMP_CUE_ROW["actionId"],
        HIGH_JUMP_CUE_ROW["clipOccurrenceId"],
        HIGH_JUMP_CUE_ROW["effectAssetId"],
    )
    semantic_match = next(
        (
            row
            for row in rows
            if (
                row.get("patternId"),
                row.get("stageId"),
                row.get("actionId"),
                row.get("clipOccurrenceId"),
                row.get("effectAssetId"),
            )
            == semantic_key
        ),
        None,
    )
    identity_matches = {
        id(row)
        for row in (
            by_binding.get(HIGH_JUMP_CUE_ROW["bindingId"]),
            by_occurrence.get(HIGH_JUMP_CUE_ROW["occurrenceId"]),
            semantic_match,
        )
        if row is not None
    }
    if len(identity_matches) > 1:
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED HIGH_JUMP AIRBORNE cue identity split"
        )
    existing = (
        by_binding.get(HIGH_JUMP_CUE_ROW["bindingId"])
        or by_occurrence.get(HIGH_JUMP_CUE_ROW["occurrenceId"])
        or semantic_match
    )
    appended = existing is None
    if existing is not None and existing != HIGH_JUMP_CUE_ROW:
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED HIGH_JUMP AIRBORNE cue row drift"
        )
    if existing is None:
        rows.append(deepcopy(HIGH_JUMP_CUE_ROW))
    _validate_cues(staged, action_clips, catalog_effect_ids)
    return staged, appended


def _whirlwind_canary(
    root: Path, catalog: dict[str, Any], cues: dict[str, Any]
) -> dict[str, Any]:
    files: list[dict[str, str]] = []
    for relative in WHIRLWIND_REQUIRED_FILES:
        path = _repository_path(root, relative)
        if not path.is_file():
            raise ProjectionError(
                f"required Whirlwind canary is missing: {relative.as_posix()}"
            )
        files.append({"path": relative.as_posix(), "sha256": _sha256(path.read_bytes())})
    for relative in WHIRLWIND_OPTIONAL_FILES:
        path = _repository_path(root, relative)
        if path.is_file():
            files.append(
                {"path": relative.as_posix(), "sha256": _sha256(path.read_bytes())}
            )
    catalog_rows = [
        row
        for row in catalog["effects"]
        if row.get("effectAssetId") == WHIRLWIND_EFFECT_ID
    ]
    if len(catalog_rows) != 1:
        raise ProjectionError("Whirlwind catalog canary row is missing or duplicated")
    cue_rows = [
        row for row in cues["cues"] if row.get("patternId") == WHIRLWIND_PATTERN_ID
    ]
    if not cue_rows:
        raise ProjectionError("Whirlwind cue canary rows are missing")
    return {
        "files": files,
        "catalogRowSha256": _json_sha(catalog_rows[0]),
        "cueRowsSha256": _json_sha(cue_rows),
    }


def _verify_whirlwind_unchanged(
    before: dict[str, Any],
    root: Path,
    staged_catalog: dict[str, Any],
    staged_cues: dict[str, Any],
) -> None:
    after = _whirlwind_canary(root, staged_catalog, staged_cues)
    if after != before:
        raise ProjectionError("Whirlwind files/catalog/cues canary changed")


def _output_role(relative: PurePosixPath) -> str:
    if relative == CATALOG_PATH:
        return "EFFECT_CATALOG"
    if relative == CUE_PATH:
        return "VALTAN_CUE_DOCUMENT"
    return "AUTHORED_EFFECT_DOCUMENT"


def collect_projection(
    repository_root: Path,
    *,
    patch_plan: Path | PurePosixPath = DEFAULT_PATCH_PLAN,
    drawable_proof: Path,
    receipt_path: Path | PurePosixPath = DEFAULT_RECEIPT,
) -> Projection:
    root = repository_root.resolve()
    patch_relative = (
        patch_plan
        if isinstance(patch_plan, PurePosixPath)
        else _path_inside_repository(root, patch_plan, "patch plan")
    )
    receipt_relative = (
        receipt_path
        if isinstance(receipt_path, PurePosixPath)
        else _path_inside_repository(root, receipt_path, "projection receipt")
    )
    patch_relative = _relative_path(patch_relative, "patch plan")
    receipt_relative = _relative_path(receipt_relative, "projection receipt")
    proof_relative = _path_inside_repository(root, drawable_proof, "drawable proof")

    plan, plan_payload = _load_json_bytes(_repository_path(root, patch_relative))
    proof, proof_payload = _load_json_bytes(_repository_path(root, proof_relative))
    _validate_patch_plan(plan)
    prior_receipt_path = _repository_path(root, receipt_relative)
    prior_receipt_exists = prior_receipt_path.is_file()
    prior_inputs: dict[str, Any] | None = None
    if prior_receipt_exists:
        prior_receipt, _ = _load_json_bytes(prior_receipt_path)
        prior_inputs = _require_object(
            prior_receipt.get("inputs"), "existing projection receipt inputs"
        )
        if (
            prior_receipt.get("schema") != RECEIPT_SCHEMA
            or prior_receipt.get("formatVersion") != FORMAT_VERSION
            or prior_receipt.get("transactionStatus") != "COMMITTED"
            or prior_inputs.get("patchPlanSha256") != _sha256(plan_payload)
        ):
            raise SourceRebaseRequired(
                "SOURCE_REBASE_REQUIRED existing projection receipt is stale"
            )
    targets = [
        _require_object(row, "patch-plan target") for row in plan["targets"]
    ]

    candidates: dict[str, dict[str, Any]] = {}
    candidate_payloads: dict[str, bytes] = {}
    candidate_relatives: dict[str, PurePosixPath] = {}
    seen_candidate_paths: set[PurePosixPath] = set()
    for target in targets:
        effect_id = target["targetEffectAssetId"]
        relative = _relative_path(
            target["overlayDocumentPath"], f"{effect_id} overlay path"
        )
        if relative in seen_candidate_paths:
            raise ProjectionError("candidate overlay path is duplicated")
        seen_candidate_paths.add(relative)
        document, payload = _load_json_bytes(_repository_path(root, relative))
        if _sha256(payload) != target["overlayDocumentSha256"]:
            raise ProjectionError(f"candidate SHA is stale: {effect_id}")
        _validate_candidate_document(document, effect_id)
        candidates[effect_id] = document
        candidate_payloads[effect_id] = payload
        candidate_relatives[effect_id] = relative
    sweep_relative, sweep_payload, drawable_resource_root = _validate_proof(
        proof, _sha256(plan_payload), targets, candidates, root
    )
    if prior_inputs is not None and (
        prior_inputs.get("drawableSweepPath") != sweep_relative.as_posix()
        or prior_inputs.get("drawableSweepSha256") != _sha256(sweep_payload)
        or prior_inputs.get("drawableResourceRoot") != drawable_resource_root
        or prior_inputs.get("drawableProofPath") != proof_relative.as_posix()
        or prior_inputs.get("drawableProofSha256") != _sha256(proof_payload)
    ):
        raise SourceRebaseRequired(
            "SOURCE_REBASE_REQUIRED existing projection receipt drawable proof is stale"
        )

    catalog, catalog_payload = _load_json_bytes(_repository_path(root, CATALOG_PATH))
    cues, cue_payload = _load_json_bytes(_repository_path(root, CUE_PATH))
    bindings, binding_payload = _load_json_bytes(
        _repository_path(root, PATTERN_BINDINGS_PATH)
    )
    action_clips = _validate_pattern_bindings(bindings)
    _validate_catalog(catalog)
    _validate_cues(
        cues,
        action_clips,
        {row["effectAssetId"] for row in catalog["effects"]},
    )
    whirlwind = _whirlwind_canary(root, catalog, cues)

    canonical_outputs: dict[PurePosixPath, bytes] = {}
    target_receipts: list[dict[str, Any]] = []
    appended_project_elements = 0
    preserved_project_elements = 0
    authored_newline = "\r\n"
    for target in targets:
        effect_id = target["targetEffectAssetId"]
        target_relative = _relative_path(
            target["targetAuthoringPath"], f"{effect_id} target path"
        )
        target_path = _repository_path(root, target_relative)
        canonical: dict[str, Any] | None
        canonical_payload: bytes | None
        if target_path.is_file():
            canonical, canonical_payload = _load_json_bytes(target_path)
            authored_newline = "\r\n" if b"\r\n" in canonical_payload else authored_newline
        else:
            canonical = None
            canonical_payload = None
        if (
            target["canonicalState"] == "EXISTING_DOCUMENT"
            and canonical is None
        ):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED existing canonical document disappeared: {effect_id}"
            )
        if (
            target["canonicalState"] == "MISSING_DOCUMENT"
            and canonical is not None
            and not prior_receipt_exists
        ):
            raise SourceRebaseRequired(
                f"SOURCE_REBASE_REQUIRED unexpected canonical document appeared: {effect_id}"
            )
        staged, appended, preserved = _reconcile_document(
            canonical, candidates[effect_id], target
        )
        _validate_candidate_document(
            {
                **staged,
                "elements": [
                    element
                    for element in staged["elements"]
                    if element.get("sourceNode", "").startswith("project-authored:")
                    and element.get("id")
                    in {
                        row["elementId"]
                        for row in target["desiredElements"]
                        if row["reconcileAction"] == "APPEND_MISSING"
                    }
                ],
            },
            effect_id,
        )
        staged_payload = (
            _json_bytes_like(canonical_payload, staged)
            if canonical_payload is not None
            else _json_bytes(staged, authored_newline)
        )
        canonical_outputs[target_relative] = staged_payload
        appended_project_elements += len(appended)
        preserved_project_elements += len(preserved)
        target_receipts.append(
            {
                "effectAssetId": effect_id,
                "overlayDocumentPath": candidate_relatives[effect_id].as_posix(),
                "overlayDocumentSha256": _sha256(candidate_payloads[effect_id]),
                "targetAuthoringPath": target_relative.as_posix(),
                "projectedElementIds": [
                    element["id"] for element in candidates[effect_id]["elements"]
                ],
                "finalDocumentSha256": _sha256(staged_payload),
            }
        )

    staged_catalog, catalog_appended = _stage_catalog(catalog)
    staged_catalog_payload = _json_bytes_like(catalog_payload, staged_catalog)
    canonical_outputs[CATALOG_PATH] = staged_catalog_payload
    catalog_effect_ids = {
        row["effectAssetId"] for row in staged_catalog["effects"]
    }
    staged_cues, cue_appended = _stage_cues(
        cues, action_clips, catalog_effect_ids
    )
    staged_cue_payload = _json_bytes_like(cue_payload, staged_cues)
    canonical_outputs[CUE_PATH] = staged_cue_payload
    _verify_whirlwind_unchanged(whirlwind, root, staged_catalog, staged_cues)

    high_jump_target = next(
        row for row in target_receipts if row["effectAssetId"] == HIGH_JUMP_EFFECT_ID
    )
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "ownerArchetypeId": OWNER_ARCHETYPE_ID,
        "transactionStatus": "COMMITTED",
        "inputs": {
            "patchPlanPath": patch_relative.as_posix(),
            "patchPlanSha256": _sha256(plan_payload),
            "drawableSweepPath": sweep_relative.as_posix(),
            "drawableSweepSha256": _sha256(sweep_payload),
            "drawableResourceRoot": drawable_resource_root,
            "drawableProofPath": proof_relative.as_posix(),
            "drawableProofSha256": _sha256(proof_payload),
        },
        "policy": {
            "reconcileMode": "APPEND_MISSING_PRESERVE_EXISTING",
            "existingElementMutation": "FORBIDDEN",
            "identityDrift": "SOURCE_REBASE_REQUIRED",
            "legacyGenericElementDeletion": "FORBIDDEN",
            "failureAction": "ROLLBACK_ALL_OUTPUTS",
        },
        "closure": {
            "candidateDocumentCount": len(target_receipts),
            "projectedElementCount": sum(
                len(row["projectedElementIds"]) for row in target_receipts
            ),
            "highJumpCatalogRowCount": 1,
            "highJumpCueRowCount": 1,
            "officialAxePresentationCount": len(
                plan["projectilePresentations"]
            ),
        },
        "targets": target_receipts,
        "canonicalOutputs": [
            {
                "path": relative.as_posix(),
                "role": _output_role(relative),
                "sha256": _sha256(payload),
            }
            for relative, payload in sorted(
                canonical_outputs.items(), key=lambda item: item[0].as_posix()
            )
        ],
        "highJumpAirbornePatch": {
            "documentSha256": high_jump_target["finalDocumentSha256"],
            "catalogRowSha256": _json_sha(HIGH_JUMP_CATALOG_ROW),
            "cueRowSha256": _json_sha(HIGH_JUMP_CUE_ROW),
            "officialAxePresentationIds": sorted(
                row["presentationId"]
                for row in plan["projectilePresentations"]
            ),
        },
        "whirlwindCanary": whirlwind,
    }
    receipt_payload = _json_bytes(receipt)
    outputs = dict(canonical_outputs)
    outputs[receipt_relative] = receipt_payload

    protected_inputs = {
        patch_relative,
        sweep_relative,
        proof_relative,
        PATTERN_BINDINGS_PATH,
        *candidate_relatives.values(),
        *WHIRLWIND_REQUIRED_FILES,
        *(
            relative
            for relative in WHIRLWIND_OPTIONAL_FILES
            if _repository_path(root, relative).is_file()
        ),
    }
    if receipt_relative in protected_inputs:
        raise ProjectionError("projection receipt path collides with an immutable input")
    if len(outputs) != len(canonical_outputs) + 1:
        raise ProjectionError("projection output paths are not unique")

    guards: dict[PurePosixPath, bytes | None] = {
        patch_relative: plan_payload,
        sweep_relative: sweep_payload,
        proof_relative: proof_payload,
        CATALOG_PATH: catalog_payload,
        CUE_PATH: cue_payload,
        PATTERN_BINDINGS_PATH: binding_payload,
    }
    for effect_id, relative in candidate_relatives.items():
        guards[relative] = candidate_payloads[effect_id]
    for relative in canonical_outputs:
        if relative in guards:
            continue
        path = _repository_path(root, relative)
        guards[relative] = path.read_bytes() if path.is_file() else None
    receipt_disk = _repository_path(root, receipt_relative)
    guards[receipt_relative] = (
        receipt_disk.read_bytes() if receipt_disk.is_file() else None
    )
    for row in whirlwind["files"]:
        relative = _relative_path(row["path"], "Whirlwind canary path")
        guards[relative] = _repository_path(root, relative).read_bytes()

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
                f"input changed after staging; transaction not started: {relative.as_posix()}"
            )


def commit_projection(
    projection: Projection, *, failure_after_promote: int | None = None
) -> None:
    _verify_guards(projection)
    if failure_after_promote is not None and failure_after_promote < 1:
        raise ProjectionError("failure_after_promote must be positive")
    root = projection.repository_root
    transaction_root = Path(tempfile.mkdtemp(prefix=".valtan-priority-projection.", dir=root))
    staged_root = transaction_root / "staged"
    backup_root = transaction_root / "backup"
    promoted: list[PurePosixPath] = []
    try:
        write_order = [
            relative
            for relative in sorted(
                projection.canonical_outputs, key=PurePosixPath.as_posix
            )
            if projection.guards.get(relative)
            != projection.canonical_outputs[relative]
        ]
        receipt_relative = next(
            relative
            for relative in projection.outputs
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
            stage_path = staged_root.joinpath(*relative.parts)
            os.replace(stage_path, target)
            promoted.append(relative)
            if (
                failure_after_promote is not None
                and len(promoted) == failure_after_promote
            ):
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
                "projection failed and rollback was incomplete: "
                + "; ".join(rollback_errors)
            ) from exc
        if isinstance(exc, ProjectionError):
            raise
        raise ProjectionError(f"projection failed; all outputs rolled back: {exc}") from exc
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
            "projection is not applied or is stale: " + ", ".join(sorted(stale))
        )


def _summary(projection: Projection, mode: str) -> str:
    new = set(projection.new_paths)
    changed_existing = sum(path not in new for path in projection.changed_paths)
    closure = projection.receipt["closure"]
    return (
        f"[PASS] Valtan priority overlay projection {mode}: "
        f"targets={closure['candidateDocumentCount']}, "
        f"projectElements={closure['projectedElementCount']}, "
        f"canonicalOutputs={len(projection.canonical_outputs)}, "
        f"changedExisting={changed_existing}, newFiles={len(new)}, "
        f"officialAxePresentations={closure['officialAxePresentationCount']}"
    )


def _make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--apply", action="store_true")
    parser.add_argument(
        "--repo-root", type=Path, default=Path(__file__).resolve().parents[2]
    )
    parser.add_argument(
        "--patch-plan", type=Path, default=Path(DEFAULT_PATCH_PLAN.as_posix())
    )
    parser.add_argument("--drawable-proof", type=Path, required=True)
    parser.add_argument(
        "--receipt", type=Path, default=Path(DEFAULT_RECEIPT.as_posix())
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _make_parser().parse_args(argv)
    try:
        projection = collect_projection(
            args.repo_root,
            patch_plan=args.patch_plan,
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
