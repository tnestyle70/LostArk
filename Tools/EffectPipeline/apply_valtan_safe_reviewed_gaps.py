#!/usr/bin/env python3
"""Atomically apply the proven four-document Valtan safe-gap slice."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "Tools/EffectPipeline"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_safe_reviewed_gap_candidates as candidates
import build_valtan_safe_reviewed_gap_drawable_proof as drawable_proof
import build_valtan_trail_adapter_packets as trail_packets
import apply_valtan_four_slash_weapon_trail as four_slash_applicator
import build_valtan_four_slash_weapon_trail_candidate as four_slash_candidate
import build_valtan_four_slash_weapon_trail_drawable_proof as four_slash_proof


RECEIPT_PATH = (
    candidates.OUTPUT_ROOT
    / "Valtan.safe-reviewed-gap-application-receipt.v1.json"
)
BOSS_CATALOG_PATH = ROOT / "Data/Actors/BossCatalog.json"
SCHEMA = "lostark.valtan-safe-reviewed-gap-application-receipt"
FORMAT_VERSION = 1

FOUR_SLASH_EFFECT_ASSET_ID = four_slash_candidate.SOURCE_EFFECT_ASSET_ID
FOUR_SLASH_MANIFEST_RELATIVE_PATH = (
    four_slash_candidate.OUTPUT_DIRECTORY_RELATIVE_PATH
    / four_slash_candidate.MANIFEST_FILENAME
)
FOUR_SLASH_PROOF_RELATIVE_PATH = four_slash_proof.PROOF_RELATIVE_PATH
FOUR_SLASH_RECEIPT_RELATIVE_PATH = (
    four_slash_applicator.RECEIPT_RELATIVE_PATH
)

PRE_TRANSFER_APPLIED_CUE_COUNT = 108
PRE_TRANSFER_APPLIED_CUE_CANONICAL_SHA256 = (
    "bd72b332beeebff4681eb804ba8ced5223400756753b1d350b1e71ec0e2a6b1e"
)
PRE_TRANSFER_APPLIED_CATALOG_COUNT = 315
PRE_TRANSFER_APPLIED_CATALOG_CANONICAL_SHA256 = (
    "02c9ed470c454b86a572f388853fb7c0b25380087fa6f4f0b4bf7667163fd89a"
)

COMBAT_OBJECT_OWNERSHIP_TRANSFERS = (
    {
        "retiredCue": {
            "bindingId": "cue.valtan.high-jump.airborne.project-authored",
            "occurrenceId": (
                "cue.valtan.high-jump.airborne.project-authored.occurrence.01"
            ),
            "patternId": "VALTAN_HIGH_JUMP",
            "stageId": "AIRBORNE",
            "actionId": "valtan.attack.high-jump.airborne",
            "clipOccurrenceId": "valtan.attack.high-jump.airborne.clip.01",
            "effectAssetId": "effect.valtan.high-jump.airborne",
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
        },
        "combatObjectVisual": {
            "combatObjectArchetypeId": (
                "combatobject.valtan.high-jump.target-axe"
            ),
            "clientVisualId": (
                "combatobject.visual.valtan.high-jump.target-axe.v1"
            ),
            "effectAssetId": "effect.valtan.sky-axe.active",
        },
        "activeCatalogRow": {
            "effectAssetId": "effect.valtan.sky-axe.active",
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": (
                "Effects/Authored/effect.valtan.sky-axe.active.effect.json"
            ),
        },
        "retiredCatalogRow": {
            "effectAssetId": "effect.valtan.high-jump.airborne",
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": (
                "Effects/Authored/effect.valtan.high-jump.airborne.effect.json"
            ),
        },
    },
    {
        "retiredCue": {
            "bindingId": "cue.valtan.red-blade-wave.active",
            "occurrenceId": "cue.valtan.red-blade-wave.active.occurrence.01",
            "patternId": "VALTAN_RED_BLADE_WAVE",
            "stageId": "PROJECTILE",
            "actionId": "valtan.attack.red-blade-wave.active",
            "clipOccurrenceId": "valtan.attack.red-blade-wave.active.clip.01",
            "effectAssetId": "effect.valtan.red-blade-wave.active",
            "anchorSlotId": "root",
            "followPolicy": "follow",
            "stopPolicy": "cue_end",
            "repeatPolicy": "once",
            "sourceStartMs": 0,
            "sourceEndMs": 1000,
            "localTransform": {
                "position": [0, 0, 0],
                "rotationDegrees": [0, 0, 0],
                "scale": [1, 1, 1],
            },
        },
        "combatObjectVisual": {
            "combatObjectArchetypeId": (
                "combatobject.valtan.red-blade-wave.projectile"
            ),
            "clientVisualId": (
                "combatobject.visual.valtan.red-blade-wave.projectile.v1"
            ),
            "effectAssetId": "effect.valtan.red-blade-wave.active",
        },
        "activeCatalogRow": {
            "effectAssetId": "effect.valtan.red-blade-wave.active",
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": (
                "Effects/Authored/effect.valtan.red-blade-wave.active.effect.json"
            ),
        },
        "retiredCatalogRow": None,
    },
)


class ApplyError(RuntimeError):
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


def raw_sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def raw_sha256(path: Path) -> str:
    return raw_sha256_bytes(path.read_bytes())


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def read_json(path: Path) -> dict[str, Any]:
    try:
        result = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ApplyError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(result, dict):
        raise ApplyError(f"JSON root is not an object: {path}")
    return result


def seal(row: dict[str, Any], field: str) -> None:
    row.pop(field, None)
    row[field] = canonical_sha256(row)


def verify_seal(row: dict[str, Any], field: str, label: str) -> None:
    expected = row.get(field)
    clone = copy.deepcopy(row)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise ApplyError(f"{label} {field} is stale")


def relative(path: Path) -> str:
    return path.resolve().relative_to(ROOT.resolve()).as_posix()


def _load_inputs() -> tuple[dict[str, Any], dict[str, Any]]:
    manifest = read_json(candidates.MANIFEST_PATH)
    candidates.validate_manifest(manifest)
    proof = read_json(drawable_proof.PROOF_PATH)
    drawable_proof.validate_proof(proof)
    if (
        proof.get("candidateManifest", {}).get("rawSha256")
        != raw_sha256(candidates.MANIFEST_PATH)
        or proof.get("candidateManifest", {}).get("artifactSha256")
        != manifest.get("artifactSha256")
        or proof.get("drawableSweep", {}).get("rawSha256")
        != raw_sha256(drawable_proof.SWEEP_PATH)
    ):
        raise ApplyError("safe-gap proof no longer seals its candidate/sweep")
    trail_packets.load_whirlwind_canary(ROOT)
    return manifest, proof


def _index(rows: list[dict[str, Any]], field: str, label: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        key = str(row.get(field) or "")
        if not key or key in result:
            raise ApplyError(f"{label} identity is empty or duplicated")
        result[key] = row
    return result


def _ownership_ledger() -> list[dict[str, Any]]:
    return [
        {
            "bindingId": transfer["retiredCue"]["bindingId"],
            "occurrenceId": transfer["retiredCue"]["occurrenceId"],
            "patternId": transfer["retiredCue"]["patternId"],
            "stageId": transfer["retiredCue"]["stageId"],
            "actionId": transfer["retiredCue"]["actionId"],
            "clipOccurrenceId": transfer["retiredCue"]["clipOccurrenceId"],
            "retiredEffectAssetId": transfer["retiredCue"]["effectAssetId"],
            "retiredSourceStartMs": transfer["retiredCue"]["sourceStartMs"],
            "retiredSourceEndMs": transfer["retiredCue"]["sourceEndMs"],
            "replacementCombatObjectArchetypeId": transfer[
                "combatObjectVisual"
            ]["combatObjectArchetypeId"],
            "replacementClientVisualId": transfer["combatObjectVisual"][
                "clientVisualId"
            ],
            "replacementEffectAssetId": transfer["combatObjectVisual"][
                "effectAssetId"
            ],
        }
        for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
    ]


def _validate_boss_catalog_ownership(boss_catalog: dict[str, Any]) -> None:
    if (
        boss_catalog.get("schema") != "lostark.boss-catalog"
        or boss_catalog.get("formatVersion") != 3
        or not isinstance(boss_catalog.get("bosses"), list)
    ):
        raise ApplyError("BossCatalog combat-object ownership header is invalid")
    valtan = [
        row
        for row in boss_catalog["bosses"]
        if isinstance(row, dict) and row.get("archetypeId") == "BOSS_VALTAN"
    ]
    expected_visuals = [
        copy.deepcopy(transfer["combatObjectVisual"])
        for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
    ]
    if len(valtan) != 1 or valtan[0].get("combatObjectVisuals") != expected_visuals:
        raise ApplyError(
            "BossCatalog Valtan combat-object ownership is not the exact two-row contract"
        )


def _validate_applied_ownership_transfer(
    cues: dict[str, Any], catalog: dict[str, Any]
) -> None:
    cue_rows = _index(cues.get("cues", []), "bindingId", "cue")
    cue_occurrences = _index(
        cues.get("cues", []), "occurrenceId", "cue occurrence"
    )
    catalog_rows = _index(
        catalog.get("effects", []), "effectAssetId", "catalog"
    )
    retired_actions = {
        transfer["retiredCue"]["actionId"]
        for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
    }
    for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS:
        retired_cue = transfer["retiredCue"]
        if (
            retired_cue["bindingId"] in cue_rows
            or retired_cue["occurrenceId"] in cue_occurrences
        ):
            raise ApplyError("retired boss-root cue was restored or rebound")
        active_catalog_row = transfer["activeCatalogRow"]
        if catalog_rows.get(active_catalog_row["effectAssetId"]) != active_catalog_row:
            raise ApplyError("combat-object Effect catalog row drifted")
        retired_catalog_row = transfer["retiredCatalogRow"]
        if (
            retired_catalog_row is not None
            and retired_catalog_row["effectAssetId"] in catalog_rows
        ):
            raise ApplyError("retired boss-root Effect catalog row was restored")
    if any(
        row.get("actionId") in retired_actions
        for row in cues.get("cues", [])
        if isinstance(row, dict)
    ):
        raise ApplyError("combat-object-owned action regained a boss-root cue")

    reconstructed_cues = copy.deepcopy(cues)
    reconstructed_cues["cues"] = sorted(
        reconstructed_cues["cues"]
        + [
            copy.deepcopy(transfer["retiredCue"])
            for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
        ],
        key=lambda row: row["bindingId"],
    )
    if (
        len(cues.get("cues", []))
        != PRE_TRANSFER_APPLIED_CUE_COUNT
        - len(COMBAT_OBJECT_OWNERSHIP_TRANSFERS)
        or len(reconstructed_cues["cues"]) != PRE_TRANSFER_APPLIED_CUE_COUNT
        or canonical_sha256(reconstructed_cues)
        != PRE_TRANSFER_APPLIED_CUE_CANONICAL_SHA256
    ):
        raise ApplyError(
            "canonical cues are not the exact pre-transfer state minus retired cues"
        )

    reconstructed_catalog = copy.deepcopy(catalog)
    reconstructed_catalog_rows = _index(
        reconstructed_catalog["effects"], "effectAssetId", "catalog"
    )
    for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS:
        retired_catalog_row = transfer["retiredCatalogRow"]
        if retired_catalog_row is None:
            continue
        active_id = transfer["activeCatalogRow"]["effectAssetId"]
        reconstructed_catalog_rows.pop(active_id)
        reconstructed_catalog_rows[retired_catalog_row["effectAssetId"]] = (
            copy.deepcopy(retired_catalog_row)
        )
    reconstructed_catalog["effects"] = sorted(
        reconstructed_catalog_rows.values(),
        key=lambda row: row["effectAssetId"],
    )
    if (
        len(catalog.get("effects", [])) != PRE_TRANSFER_APPLIED_CATALOG_COUNT
        or len(reconstructed_catalog["effects"])
        != PRE_TRANSFER_APPLIED_CATALOG_COUNT
        or canonical_sha256(reconstructed_catalog)
        != PRE_TRANSFER_APPLIED_CATALOG_CANONICAL_SHA256
    ):
        raise ApplyError(
            "canonical catalog is not the exact combat-object ownership transfer"
        )


def _apply_ownership_transfer(
    cues: dict[str, Any], catalog: dict[str, Any]
) -> None:
    cue_rows = _index(cues.get("cues", []), "bindingId", "cue")
    catalog_rows = _index(
        catalog.get("effects", []), "effectAssetId", "catalog"
    )
    for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS:
        retired_cue = transfer["retiredCue"]
        if cue_rows.get(retired_cue["bindingId"]) != retired_cue:
            raise ApplyError("pre-transfer boss-root cue identity drifted")
        cue_rows.pop(retired_cue["bindingId"])
        retired_catalog_row = transfer["retiredCatalogRow"]
        if retired_catalog_row is not None:
            if (
                catalog_rows.get(retired_catalog_row["effectAssetId"])
                != retired_catalog_row
            ):
                raise ApplyError("pre-transfer Effect catalog identity drifted")
            catalog_rows.pop(retired_catalog_row["effectAssetId"])
        active_catalog_row = transfer["activeCatalogRow"]
        active_id = active_catalog_row["effectAssetId"]
        if active_id in catalog_rows and catalog_rows[active_id] != active_catalog_row:
            raise ApplyError("combat-object Effect catalog row is rebound")
        catalog_rows[active_id] = copy.deepcopy(active_catalog_row)
    cues["cues"] = sorted(cue_rows.values(), key=lambda row: row["bindingId"])
    catalog["effects"] = sorted(
        catalog_rows.values(), key=lambda row: row["effectAssetId"]
    )


def _repository_path(root: Path, relative_path: Any) -> Path:
    parts = tuple(relative_path.parts)
    resolved_root = root.resolve()
    result = root.joinpath(*parts).resolve()
    if result != resolved_root and resolved_root not in result.parents:
        raise ApplyError(f"repository-relative path escaped root: {relative_path}")
    return result


def _four_slash_safe_candidate(
    manifest: dict[str, Any],
) -> dict[str, Any]:
    matches = [
        row
        for row in manifest.get("candidateDocuments", [])
        if isinstance(row, dict)
        and row.get("effectAssetId") == FOUR_SLASH_EFFECT_ASSET_ID
    ]
    if len(matches) != 1:
        raise ApplyError("safe-gap FourSlash source denominator changed")
    return matches[0]


def _four_slash_artifact_identity(
    root: Path,
    relative_path: Any,
    document: dict[str, Any],
) -> dict[str, Any]:
    path = _repository_path(root, relative_path)
    artifact_sha256 = document.get("artifactSha256")
    if not isinstance(artifact_sha256, str) or len(artifact_sha256) != 64:
        raise ApplyError(f"sealed artifact identity is invalid: {relative_path}")
    return {
        "path": relative_path.as_posix(),
        "rawSha256": raw_sha256(path),
        "artifactSha256": artifact_sha256,
    }


def _validate_four_slash_composition(
    manifest: dict[str, Any], repository_root: Path = ROOT
) -> dict[str, Any] | None:
    """Validate the optional proof-gated row over our exact 20-row source.

    SafeReviewedGaps continues to own and hash the reconstructed 20-element
    source view.  ProjectTunedFourSlashWeaponTrail owns exactly one appended
    row.  The two transactions remain independently reproducible without
    allowing either applicator to rewrite the other's row.
    """

    root = repository_root.resolve()
    safe_candidate = _four_slash_safe_candidate(manifest)
    canonical_relative = four_slash_applicator.relative_from_text(
        safe_candidate.get("canonicalPath"), "safe-gap FourSlash canonicalPath"
    )
    canonical_path = _repository_path(root, canonical_relative)
    try:
        canonical, canonical_payload = four_slash_applicator.read_json_bytes(
            canonical_path
        )
    except four_slash_applicator.ApplicationError as error:
        raise ApplyError(
            f"cannot read FourSlash canonical composition: {error}"
        ) from error
    if (
        canonical.get("schema") != "lostark.effect-authoring"
        or canonical.get("version") != 13
        or canonical.get("effectAssetId") != FOUR_SLASH_EFFECT_ASSET_ID
        or not isinstance(canonical.get("elements"), list)
    ):
        raise ApplyError("safe-gap FourSlash canonical document identity changed")

    elements = canonical["elements"]
    downstream_id = four_slash_candidate.CANDIDATE_ELEMENT_ID
    downstream_node = four_slash_candidate.CANDIDATE_SOURCE_NODE
    downstream_slot = four_slash_candidate.RUNTIME_ANCHOR_SLOT_ID
    id_indices = [
        index
        for index, row in enumerate(elements)
        if isinstance(row, dict) and row.get("id") == downstream_id
    ]
    node_indices = [
        index
        for index, row in enumerate(elements)
        if isinstance(row, dict) and row.get("sourceNode") == downstream_node
    ]
    slot_indices = [
        index
        for index, row in enumerate(elements)
        if isinstance(row, dict)
        and (row.get("actionCueAttachment") or {}).get(
            "runtimeAnchorSlotId"
        )
        == downstream_slot
    ]
    receipt_path = _repository_path(root, FOUR_SLASH_RECEIPT_RELATIVE_PATH)

    if not id_indices and not node_indices and not slot_indices:
        if receipt_path.is_file():
            raise ApplyError(
                "FourSlash application receipt exists without its downstream row"
            )
        if (
            len(elements) != safe_candidate["elementCount"]
            or [row.get("id") for row in elements]
            != safe_candidate["elementIds"]
            or raw_sha256_bytes(canonical_payload)
            != safe_candidate["rawSha256"]
            or canonical_sha256(canonical)
            != safe_candidate["canonicalSha256"]
        ):
            raise ApplyError("safe-gap FourSlash source document drifted")
        return None

    if (
        id_indices != node_indices
        or id_indices != slot_indices
        or len(id_indices) != 1
    ):
        raise ApplyError("FourSlash downstream stable row identity was rebound")
    if not receipt_path.is_file():
        raise ApplyError(
            "FourSlash downstream row is missing its committed application receipt"
        )

    try:
        projection = four_slash_applicator.collect_projection(root)
    except (four_slash_applicator.ApplicationError, OSError) as error:
        raise ApplyError(f"FourSlash downstream proof closure failed: {error}") from error
    if not projection.already_applied or projection.changed_paths:
        raise ApplyError("FourSlash downstream projection is not committed exactly")

    manifest_path = _repository_path(root, FOUR_SLASH_MANIFEST_RELATIVE_PATH)
    proof_path = _repository_path(root, FOUR_SLASH_PROOF_RELATIVE_PATH)
    downstream_manifest = read_json(manifest_path)
    downstream_proof = read_json(proof_path)
    downstream_receipt = read_json(receipt_path)
    target = downstream_manifest.get("target") or {}
    candidate_relative = four_slash_applicator.relative_from_text(
        target.get("candidatePath"), "FourSlash candidatePath"
    )
    target_canonical_relative = four_slash_applicator.relative_from_text(
        target.get("canonicalPath"), "FourSlash canonicalPath"
    )
    if target_canonical_relative != canonical_relative:
        raise ApplyError("FourSlash downstream target rebound its canonical path")
    candidate_path = _repository_path(root, candidate_relative)
    try:
        candidate_document, candidate_payload = (
            four_slash_applicator.read_json_bytes(candidate_path)
        )
    except four_slash_applicator.ApplicationError as error:
        raise ApplyError(f"cannot read FourSlash candidate: {error}") from error
    candidate_elements = candidate_document.get("elements") or []
    if len(candidate_elements) != 1 or not isinstance(candidate_elements[0], dict):
        raise ApplyError("FourSlash downstream candidate denominator changed")
    candidate_element = candidate_elements[0]

    inserted_index = id_indices[0]
    projected_element = elements[inserted_index]
    if (
        inserted_index != safe_candidate["elementCount"]
        or len(elements) != safe_candidate["elementCount"] + 1
        or projected_element != candidate_element
        or canonical_sha256(projected_element)
        != canonical_sha256(candidate_element)
    ):
        raise ApplyError("FourSlash downstream row value or append position drifted")

    source_view = copy.deepcopy(canonical)
    source_view["elements"].pop(inserted_index)
    source_payload = four_slash_applicator.json_bytes_like(
        canonical_payload, source_view
    )
    if (
        len(source_view["elements"]) != safe_candidate["elementCount"]
        or [row.get("id") for row in source_view["elements"]]
        != safe_candidate["elementIds"]
        or raw_sha256_bytes(source_payload) != safe_candidate["rawSha256"]
        or canonical_sha256(source_view) != safe_candidate["canonicalSha256"]
    ):
        raise ApplyError(
            "FourSlash reconstructed safe-gap 20-element source view drifted"
        )

    downstream_input = downstream_manifest.get("inputIdentity") or {}
    source_proof = downstream_proof.get("sourcePreservation") or {}
    expected_apply = {
        "preApplyRawSha256": raw_sha256_bytes(source_payload),
        "postApplyRawSha256": raw_sha256_bytes(canonical_payload),
    }
    if (
        downstream_input.get("canonicalDocumentRawSha256")
        != safe_candidate["rawSha256"]
        or downstream_input.get("canonicalDocumentCanonicalSha256")
        != safe_candidate["canonicalSha256"]
        or source_proof.get("canonicalRawSha256")
        != safe_candidate["rawSha256"]
        or downstream_receipt.get("canonicalApply") != expected_apply
    ):
        raise ApplyError(
            "FourSlash downstream receipt is not based on the exact safe-gap source"
        )

    return {
        "status": "APPLIED_PROOF_GATED_EXACT_EXTENSION",
        "candidateManifest": _four_slash_artifact_identity(
            root, FOUR_SLASH_MANIFEST_RELATIVE_PATH, downstream_manifest
        ),
        "drawableProof": _four_slash_artifact_identity(
            root, FOUR_SLASH_PROOF_RELATIVE_PATH, downstream_proof
        ),
        "applicationReceipt": _four_slash_artifact_identity(
            root, FOUR_SLASH_RECEIPT_RELATIVE_PATH, downstream_receipt
        ),
        "candidateDocument": {
            "path": candidate_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(candidate_payload),
            "canonicalSha256": canonical_sha256(candidate_document),
            "elementId": downstream_id,
            "elementSha256": canonical_sha256(candidate_element),
        },
        "safeSourceView": {
            "rawSha256": raw_sha256_bytes(source_payload),
            "canonicalSha256": canonical_sha256(source_view),
            "elementCount": len(source_view["elements"]),
            "elementIds": [row["id"] for row in source_view["elements"]],
        },
        "canonicalOutput": {
            "path": canonical_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(canonical_payload),
            "canonicalSha256": canonical_sha256(canonical),
            "elementCount": len(elements),
            "elementIds": [row["id"] for row in elements],
        },
        "projection": {
            "insertedElementIndex": inserted_index,
            "elementId": downstream_id,
            "sourceNode": downstream_node,
            "runtimeAnchorSlotId": downstream_slot,
            "runtimeBoneName": four_slash_candidate.RUNTIME_BONE_NAME,
            "elementSha256": canonical_sha256(projected_element),
            "elementValue": copy.deepcopy(projected_element),
        },
    }


def _state_and_composition(
    manifest: dict[str, Any],
    cues: dict[str, Any],
    catalog: dict[str, Any],
    boss_catalog: dict[str, Any],
) -> tuple[str, dict[str, Any] | None]:
    _validate_boss_catalog_ownership(boss_catalog)
    input_identity = manifest["inputIdentity"]
    cue_rows = _index(cues.get("cues", []), "bindingId", "cue")
    catalog_rows = _index(catalog.get("effects", []), "effectAssetId", "catalog")
    proposed_cues = _index(manifest["proposedCueRows"], "bindingId", "proposed cue")
    proposed_catalog = _index(
        manifest["proposedCatalogRows"], "effectAssetId", "proposed catalog"
    )
    cue_matches = {
        key: cue_rows.get(key) == value for key, value in proposed_cues.items()
    }
    catalog_matches = {
        key: catalog_rows.get(key) == value
        for key, value in proposed_catalog.items()
    }
    none_present = all(key not in cue_rows for key in proposed_cues) and all(
        key not in catalog_rows for key in proposed_catalog
    )
    preimage_exact = (
        none_present
        and len(cues.get("cues", [])) == input_identity["cueCount"] == 104
        and len(catalog.get("effects", [])) == input_identity["catalogCount"]
        and canonical_sha256(cues) == input_identity["cueCanonicalSha256"]
        and canonical_sha256(catalog) == input_identity["catalogCanonicalSha256"]
    )
    all_present = all(cue_matches.values()) and all(catalog_matches.values())
    if preimage_exact:
        return "PREAPPLY_EXACT", None
    if all_present:
        _validate_applied_ownership_transfer(cues, catalog)
        downstream: dict[str, Any] | None = None
        for row in manifest["candidateDocuments"]:
            if row["effectAssetId"] == FOUR_SLASH_EFFECT_ASSET_ID:
                downstream = _validate_four_slash_composition(manifest)
                continue
            canonical_path = ROOT / row["canonicalPath"]
            if (
                not canonical_path.is_file()
                or raw_sha256(canonical_path) != row["rawSha256"]
            ):
                raise ApplyError("applied canonical Effect document drifted")
        return "APPLIED_EXACT", downstream
    raise ApplyError("canonical cue/catalog state is partial or diverged")


def _state(
    manifest: dict[str, Any],
    cues: dict[str, Any],
    catalog: dict[str, Any],
    boss_catalog: dict[str, Any],
) -> str:
    return _state_and_composition(manifest, cues, catalog, boss_catalog)[0]


def _post_documents(
    manifest: dict[str, Any], state: str
) -> tuple[dict[Path, bytes], list[dict[str, Any]]]:
    writes: dict[Path, bytes] = {}
    rows: list[dict[str, Any]] = []
    for candidate_row in manifest["candidateDocuments"]:
        candidate_path = ROOT / candidate_row["candidatePath"]
        canonical_path = ROOT / candidate_row["canonicalPath"]
        payload = candidate_path.read_bytes()
        document = read_json(candidate_path)
        if (
            raw_sha256_bytes(payload) != candidate_row["rawSha256"]
            or canonical_sha256(document) != candidate_row["canonicalSha256"]
        ):
            raise ApplyError("candidate document identity changed")
        if state == "PREAPPLY_EXACT":
            if canonical_path.exists():
                raise ApplyError(f"new canonical path already exists: {canonical_path}")
            writes[canonical_path] = payload
        rows.append(
            {
                "sliceId": candidate_row["sliceId"],
                "effectAssetId": candidate_row["effectAssetId"],
                "canonicalPath": candidate_row["canonicalPath"],
                "rawSha256": candidate_row["rawSha256"],
                "canonicalSha256": candidate_row["canonicalSha256"],
                "elementCount": candidate_row["elementCount"],
                "elementIds": copy.deepcopy(candidate_row["elementIds"]),
            }
        )
    return writes, rows


def _receipt(
    manifest: dict[str, Any],
    proof: dict[str, Any],
    cue_payload: bytes,
    catalog_payload: bytes,
    boss_catalog: dict[str, Any],
    canonical_documents: list[dict[str, Any]],
    downstream_four_slash: dict[str, Any] | None,
) -> dict[str, Any]:
    cue_document = json.loads(cue_payload.decode("utf-8"))
    catalog_document = json.loads(catalog_payload.decode("utf-8"))
    candidates_by_effect = {
        row["effectAssetId"]: row for row in manifest["candidateDocuments"]
    }
    trail_projections = []
    for projection in manifest["adapterProjections"]:
        candidate = candidates_by_effect[projection["effectAssetId"]]
        row = copy.deepcopy(projection)
        row["canonicalPath"] = candidate["canonicalPath"]
        row["cueBindingId"] = candidate["cue"]["bindingId"]
        row["cueOccurrenceId"] = candidate["cue"]["occurrenceId"]
        row["clipOccurrenceId"] = candidate["clipOccurrenceId"]
        row["applicationDisposition"] = "APPLIED_EXACT_MISSING_ONLY"
        seal(row, "applicationProjectionSha256")
        trail_projections.append(row)
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "disposition": "APPLIED_PROOF_GATED_IDEMPOTENT",
        "candidateManifest": {
            "path": relative(candidates.MANIFEST_PATH),
            "rawSha256": raw_sha256(candidates.MANIFEST_PATH),
            "artifactSha256": manifest["artifactSha256"],
        },
        "drawableProof": {
            "path": relative(drawable_proof.PROOF_PATH),
            "rawSha256": raw_sha256(drawable_proof.PROOF_PATH),
            "artifactSha256": proof["artifactSha256"],
        },
        "canonicalCueDocument": {
            "path": relative(candidates.CUES_PATH),
            "rawSha256": raw_sha256_bytes(cue_payload),
            "canonicalSha256": canonical_sha256(cue_document),
            "cueCount": len(cue_document["cues"]),
            "addedBindingIds": sorted(
                row["bindingId"] for row in manifest["proposedCueRows"]
            ),
            "retiredBindingIds": sorted(
                transfer["retiredCue"]["bindingId"]
                for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
            ),
        },
        "canonicalCatalogDocument": {
            "path": relative(candidates.CATALOG_PATH),
            "rawSha256": raw_sha256_bytes(catalog_payload),
            "canonicalSha256": canonical_sha256(catalog_document),
            "effectCount": len(catalog_document["effects"]),
            "addedEffectAssetIds": sorted(
                row["effectAssetId"] for row in manifest["proposedCatalogRows"]
            ),
            "ownershipTransferAddedEffectAssetIds": sorted(
                transfer["activeCatalogRow"]["effectAssetId"]
                for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
                if transfer["retiredCatalogRow"] is not None
            ),
            "ownershipTransferRetiredEffectAssetIds": sorted(
                transfer["retiredCatalogRow"]["effectAssetId"]
                for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
                if transfer["retiredCatalogRow"] is not None
            ),
        },
        "combatObjectOwnershipTransfer": {
            "bossCatalogPath": relative(BOSS_CATALOG_PATH),
            "bossCatalogRawSha256": raw_sha256(BOSS_CATALOG_PATH),
            "bossCatalogCanonicalSha256": canonical_sha256(boss_catalog),
            "preTransferCueCanonicalSha256": (
                PRE_TRANSFER_APPLIED_CUE_CANONICAL_SHA256
            ),
            "preTransferCatalogCanonicalSha256": (
                PRE_TRANSFER_APPLIED_CATALOG_CANONICAL_SHA256
            ),
            "retiredBossRootCues": _ownership_ledger(),
            "combatObjectVisuals": [
                copy.deepcopy(transfer["combatObjectVisual"])
                for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
            ],
        },
        "canonicalDocuments": sorted(
            canonical_documents, key=lambda row: row["effectAssetId"]
        ),
        "downstreamFourSlashWeaponTrail": copy.deepcopy(
            downstream_four_slash
        ),
        "trailProjections": sorted(
            trail_projections, key=lambda row: row["projectionId"]
        ),
        "preservedUnresolvedTrailRows": copy.deepcopy(
            manifest["preservedUnresolvedTrailRows"]
        ),
        "summary": {
            "addedCueCount": 4,
            "addedCatalogCount": 4,
            "addedDocumentCount": 4,
            "addedElementCount": 167,
            "coreProjectionCount": 160,
            "animationTrailProjectionCount": 6,
            "cascadeRibbonProjectionCount": 1,
            "preservedUnresolvedTrailRowCount": 6,
            "canonicalMutationFileCount": 6,
            "retiredBossRootCueCount": 2,
            "combatObjectVisualCount": 2,
            "ownershipTransferAddedCatalogCount": 1,
            "ownershipTransferRetiredCatalogCount": 1,
            "downstreamExtensionCount": (
                1 if downstream_four_slash is not None else 0
            ),
        },
    }
    seal(receipt, "artifactSha256")
    return receipt


def validate_receipt(
    document: dict[str, Any],
    expected_downstream_four_slash: dict[str, Any] | None = None,
    *,
    resolve_downstream: bool = True,
) -> None:
    if (
        document.get("schema") != SCHEMA
        or document.get("formatVersion") != FORMAT_VERSION
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
        or document.get("disposition") != "APPLIED_PROOF_GATED_IDEMPOTENT"
    ):
        raise ApplyError("safe-gap application receipt header is invalid")
    verify_seal(document, "artifactSha256", "safe-gap application receipt")
    summary = document.get("summary") or {}
    if (
        summary.get("addedElementCount") != 167
        or summary.get("animationTrailProjectionCount") != 6
        or summary.get("cascadeRibbonProjectionCount") != 1
        or summary.get("retiredBossRootCueCount") != 2
        or summary.get("combatObjectVisualCount") != 2
        or summary.get("ownershipTransferAddedCatalogCount") != 1
        or summary.get("ownershipTransferRetiredCatalogCount") != 1
        or summary.get("downstreamExtensionCount")
        != (1 if document.get("downstreamFourSlashWeaponTrail") is not None else 0)
        or len(document.get("trailProjections") or []) != 7
    ):
        raise ApplyError("safe-gap application receipt denominator changed")
    expected_ownership = {
        "bossCatalogPath": relative(BOSS_CATALOG_PATH),
        "bossCatalogRawSha256": raw_sha256(BOSS_CATALOG_PATH),
        "bossCatalogCanonicalSha256": canonical_sha256(
            read_json(BOSS_CATALOG_PATH)
        ),
        "preTransferCueCanonicalSha256": (
            PRE_TRANSFER_APPLIED_CUE_CANONICAL_SHA256
        ),
        "preTransferCatalogCanonicalSha256": (
            PRE_TRANSFER_APPLIED_CATALOG_CANONICAL_SHA256
        ),
        "retiredBossRootCues": _ownership_ledger(),
        "combatObjectVisuals": [
            copy.deepcopy(transfer["combatObjectVisual"])
            for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
        ],
    }
    if document.get("combatObjectOwnershipTransfer") != expected_ownership:
        raise ApplyError("safe-gap combat-object ownership receipt drifted")
    if resolve_downstream:
        manifest = read_json(candidates.MANIFEST_PATH)
        candidates.validate_manifest(manifest)
        expected_downstream_four_slash = _validate_four_slash_composition(
            manifest
        )
    if (
        document.get("downstreamFourSlashWeaponTrail")
        != expected_downstream_four_slash
    ):
        raise ApplyError("safe-gap downstream FourSlash receipt drifted")
    canonical_cues = document.get("canonicalCueDocument") or {}
    canonical_catalog = document.get("canonicalCatalogDocument") or {}
    if (
        canonical_cues.get("cueCount") != 106
        or canonical_cues.get("retiredBindingIds")
        != sorted(
            transfer["retiredCue"]["bindingId"]
            for transfer in COMBAT_OBJECT_OWNERSHIP_TRANSFERS
        )
        or canonical_catalog.get("effectCount") != 315
        or canonical_catalog.get("ownershipTransferAddedEffectAssetIds")
        != ["effect.valtan.sky-axe.active"]
        or canonical_catalog.get("ownershipTransferRetiredEffectAssetIds")
        != ["effect.valtan.high-jump.airborne"]
    ):
        raise ApplyError("safe-gap canonical ownership denominator changed")
    for row in document["trailProjections"]:
        verify_seal(row, "applicationProjectionSha256", "trail application projection")


def _atomic_replace(writes: dict[Path, bytes]) -> None:
    staged: dict[Path, Path] = {}
    originals: dict[Path, bytes | None] = {}
    replaced: list[Path] = []
    try:
        for path, payload in sorted(writes.items(), key=lambda item: str(item[0])):
            path.parent.mkdir(parents=True, exist_ok=True)
            temporary = path.with_suffix(path.suffix + f".safe-gap.{os.getpid()}.tmp")
            temporary.write_bytes(payload)
            staged[path] = temporary
            originals[path] = path.read_bytes() if path.is_file() else None
        for path in sorted(staged, key=str):
            os.replace(staged[path], path)
            replaced.append(path)
    except OSError as error:
        for path in reversed(replaced):
            original = originals[path]
            try:
                if original is None:
                    path.unlink(missing_ok=True)
                else:
                    restore = path.with_suffix(path.suffix + f".safe-gap.restore.{os.getpid()}.tmp")
                    restore.write_bytes(original)
                    os.replace(restore, path)
            except OSError:
                pass
        raise ApplyError(f"safe-gap atomic transaction failed: {error}") from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def expected_application() -> tuple[str, dict[Path, bytes], dict[str, Any]]:
    manifest, proof = _load_inputs()
    cues = read_json(candidates.CUES_PATH)
    catalog = read_json(candidates.CATALOG_PATH)
    boss_catalog = read_json(BOSS_CATALOG_PATH)
    state, downstream_four_slash = _state_and_composition(
        manifest, cues, catalog, boss_catalog
    )
    writes, canonical_documents = _post_documents(manifest, state)
    if state == "PREAPPLY_EXACT":
        cues = copy.deepcopy(cues)
        catalog = copy.deepcopy(catalog)
        cues["cues"] = sorted(
            cues["cues"] + copy.deepcopy(manifest["proposedCueRows"]),
            key=lambda row: row["bindingId"],
        )
        catalog["effects"] = sorted(
            catalog["effects"] + copy.deepcopy(manifest["proposedCatalogRows"]),
            key=lambda row: row["effectAssetId"],
        )
        _apply_ownership_transfer(cues, catalog)
        _validate_applied_ownership_transfer(cues, catalog)
        cue_payload = pretty_bytes(cues)
        catalog_payload = pretty_bytes(catalog)
        writes[candidates.CUES_PATH] = cue_payload
        writes[candidates.CATALOG_PATH] = catalog_payload
    else:
        cue_payload = candidates.CUES_PATH.read_bytes()
        catalog_payload = candidates.CATALOG_PATH.read_bytes()
    receipt = _receipt(
        manifest,
        proof,
        cue_payload,
        catalog_payload,
        boss_catalog,
        canonical_documents,
        downstream_four_slash,
    )
    validate_receipt(
        receipt,
        downstream_four_slash,
        resolve_downstream=False,
    )
    writes[RECEIPT_PATH] = pretty_bytes(receipt)
    return state, writes, receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--apply", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    state, writes, receipt = expected_application()
    if args.apply:
        changed = {
            path: payload
            for path, payload in writes.items()
            if not path.is_file() or path.read_bytes() != payload
        }
        _atomic_replace(changed)
        final_state, final_writes, final_receipt = expected_application()
        if final_state != "APPLIED_EXACT" or any(
            not path.is_file() or path.read_bytes() != payload
            for path, payload in final_writes.items()
        ):
            raise ApplyError("safe-gap post-apply verification failed")
        print(
            "Valtan safe reviewed gaps applied: changed="
            + str(len(changed))
            + " "
            + json.dumps(final_receipt["summary"], sort_keys=True)
        )
        return 0
    if state != "APPLIED_EXACT":
        raise ApplyError("safe-gap canonical state has not been applied")
    drift = [
        str(path)
        for path, payload in writes.items()
        if not path.is_file() or path.read_bytes() != payload
    ]
    if drift:
        raise ApplyError("safe-gap applied state drifted: " + ", ".join(drift))
    print(
        "Valtan safe reviewed gaps checked: changed=0 "
        + json.dumps(receipt["summary"], sort_keys=True)
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (
        ApplyError,
        candidates.SafeGapError,
        drawable_proof.ProofError,
        trail_packets.AdapterError,
    ) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
