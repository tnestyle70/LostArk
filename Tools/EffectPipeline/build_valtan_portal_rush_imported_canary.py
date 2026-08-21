#!/usr/bin/env python3
"""Materialize the reviewed Portal Rush source family as immutable candidates.

The output is deliberately outside ``Data/Effects/Authored``.  Each admitted
``clipOccurrenceId`` owns one ordinary v13 Effect document containing every
reviewed source occurrence's drawable first-LOD carrier.  Existing authored
documents, catalog rows, and v2 cues are only inspected; the reconcile receipt
is missing-only and never retires or overwrites an authored element.

Portal Rush FINISH remains excluded while its selected source stage is
``SOURCE_TIMING_REVIEW_REQUIRED``.  This canary therefore proves PORTAL,
RUSHES, and RECOVERY without silently treating a source timing upper bound as
completion.
"""

from __future__ import annotations

import argparse
import copy
import json
from collections import defaultdict
from pathlib import Path
from typing import Any

import build_valtan_source_occurrence_inventory as source_inventory


ROOT = Path(__file__).resolve().parents[2]
PATTERN_ID = "VALTAN_PORTAL_RUSH"
SELECTION_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.priority-source-sequence-selections.v1.json"
)
OUTPUT_ROOT = ROOT / "Data/Effects/Imported/Valtan/PortalRush"
RECEIPT_NAME = "Valtan.portal-rush-imported-canary.v1.json"
CATALOG_PATH = ROOT / "Data/Effects/EffectCatalog.json"
CUE_PATH = source_inventory.CUE_PATH
AUTHORED_ROOT = source_inventory.AUTHORED_ROOT

EXPECTED_CLIPS = {
    "valtan.attack.portal-rush.portal.clip.01": {
        "sourceStageIndex": 0,
        "gameplayActionId": "valtan.attack.portal-rush.portal",
        "effectAssetId": "effect.valtan.portal-rush.portal",
    },
    "valtan.attack.portal-rush.rushes.clip.01": {
        "sourceStageIndex": 1,
        "gameplayActionId": "valtan.attack.portal-rush.rushes",
        "effectAssetId": "effect.valtan.portal-rush.rushes",
    },
    "valtan.attack.portal-rush.recovery.clip.01": {
        "sourceStageIndex": 6,
        "gameplayActionId": "valtan.attack.portal-rush.recovery",
        "effectAssetId": "effect.valtan.portal-rush.recovery",
    },
}
EXCLUDED_FINISH_CLIP = "valtan.attack.portal-rush.finish.clip.01"


class CanaryError(RuntimeError):
    """Raised when the reviewed Portal Rush canary contract drifts."""


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def candidate_filename(effect_asset_id: str) -> str:
    return f"{effect_asset_id}.imported-candidate.effect.json"


def catalog_index() -> dict[str, dict[str, Any]]:
    document = read_json(CATALOG_PATH)
    result: dict[str, dict[str, Any]] = {}
    for row in document.get("effects", []):
        effect_id = str(row.get("effectAssetId") or "")
        if not effect_id:
            continue
        if effect_id in result:
            raise CanaryError(f"duplicate Effect catalog ID: {effect_id}")
        result[effect_id] = copy.deepcopy(row)
    return result


def cue_index() -> dict[str, dict[str, Any]]:
    document = read_json(CUE_PATH)
    if document.get("formatVersion") != 2:
        raise CanaryError("Portal Rush canary requires canonical v2 cues")
    result: dict[str, dict[str, Any]] = {}
    for row in document.get("cues", []):
        clip_occurrence_id = str(row.get("clipOccurrenceId") or "")
        if not clip_occurrence_id:
            continue
        if clip_occurrence_id in result:
            raise CanaryError(
                f"duplicate cue clipOccurrenceId: {clip_occurrence_id}"
            )
        result[clip_occurrence_id] = copy.deepcopy(row)
    return result


def validate_candidate_document(
    document: dict[str, Any],
    effect_asset_id: str,
) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_asset_id
    ):
        raise CanaryError(f"candidate v13 identity is invalid: {effect_asset_id}")
    elements = document.get("elements")
    if not isinstance(elements, list) or not elements:
        raise CanaryError(f"candidate has no drawable elements: {effect_asset_id}")
    ids = [str(row.get("id") or "") for row in elements]
    source_nodes = [str(row.get("sourceNode") or "") for row in elements]
    if (
        any(not value for value in ids + source_nodes)
        or len(ids) != len(set(ids))
        or len(source_nodes) != len(set(source_nodes))
    ):
        raise CanaryError(f"candidate element identity is not unique: {effect_asset_id}")
    if source_nodes != sorted(source_nodes):
        raise CanaryError(f"candidate elements are not deterministic: {effect_asset_id}")
    for element in elements:
        attachment = element.get("actionCueAttachment") or {}
        inheritance = element.get("transformInheritance") or {}
        if (
            attachment.get("enabled") is not False
            or attachment.get("follow") is not False
            or inheritance
            != {"enabled": False, "masterElementId": ""}
        ):
            raise CanaryError("candidate v13 transform ownership is invalid")
        recipe = element.get("sourceRecipe") or {}
        resources = element.get("resources") or []
        material = element.get("material") or {}
        if recipe.get("enabled") is not True:
            raise CanaryError("candidate contains a non-executable sourceRecipe")
        if not resources:
            raise CanaryError("candidate contains a carrier with no runtime resource")
        if not str(material.get("templateId") or ""):
            raise CanaryError("candidate contains a carrier with no material template")
        if not any(
            str(module.get("className") or "").casefold()
            == "particlemodulerequired"
            for module in recipe.get("modules", [])
        ):
            raise CanaryError("candidate sourceRecipe lost its Required module")


def add_v13_transform_ownership_defaults(
    element: dict[str, Any],
) -> dict[str, Any]:
    """Attach the explicit no-inheritance shape required by ordinary v13."""
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
    """Keep the full source key in the receipt and its hash in v13's 256 bytes."""
    result = copy.deepcopy(element)
    full_source_key = str(result.get("sourceNode") or "")
    if not full_source_key:
        raise CanaryError("candidate source element lost its full source key")
    compact_key = "valtan.source." + source_inventory.sha256_bytes(
        full_source_key.encode("utf-8")
    )
    result["sourceNode"] = compact_key
    return result, {
        "id": str(result.get("id") or ""),
        "sourceNode": compact_key,
        "fullSourceKey": full_source_key,
    }


def compact_reconcile_plan(
    existing: dict[str, Any],
    candidate: dict[str, Any],
    candidate_path: Path,
) -> dict[str, Any]:
    plan = source_inventory.reconcile_effect_document(
        existing, candidate["elements"]
    )
    additions = plan.pop("addElements")
    plan["mode"] = "MISSING_ONLY_PRESERVE_EXISTING"
    plan["candidateDocumentPath"] = candidate_path.relative_to(ROOT).as_posix()
    plan["addElementRefs"] = [
        {
            "id": str(row["id"]),
            "sourceNode": str(row["sourceNode"]),
        }
        for row in additions
    ]
    plan["legacyRetirementDisposition"] = (
        "REPORT_ONLY_UNVERIFIED_DEFAULT_SIGNATURE_NO_DELETE"
    )
    if plan.get("deleteElements"):
        raise CanaryError("missing-only reconcile unexpectedly deletes elements")
    return plan


def build_canary(
    selection_path: Path = SELECTION_PATH,
    output_root: Path = OUTPUT_ROOT,
) -> tuple[dict[str, bytes], dict[str, Any]]:
    selection = source_inventory.load_selection_manifest(selection_path)
    inventory = source_inventory.build_inventory(
        {"reviewedBranchSelections": selection["selections"]},
        include_payloads=True,
        additional_repository_sources=[selection_path],
    )
    systems = {
        row["sourceSystemId"]: {
            carrier["carrierKey"]: carrier for carrier in row["carriers"]
        }
        for row in inventory["sourceSystems"]
    }
    occurrences_by_clip: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for occurrence in inventory["occurrences"]:
        if occurrence["patternId"] != PATTERN_ID:
            continue
        clip_occurrence_id = str(occurrence.get("clipOccurrenceId") or "")
        if occurrence["reachabilityDisposition"] == "REACHABLE_REVIEWED":
            occurrences_by_clip[clip_occurrence_id].append(occurrence)
        if (
            clip_occurrence_id == EXCLUDED_FINISH_CLIP
            and occurrence["reachabilityDisposition"] == "REACHABLE_REVIEWED"
        ):
            raise CanaryError("Portal FINISH was admitted before timing review")
    if set(occurrences_by_clip) != set(EXPECTED_CLIPS):
        raise CanaryError(
            "Portal Rush reviewed reachable clip set drifted: "
            + repr(sorted(occurrences_by_clip))
        )

    catalogs = catalog_index()
    cues = cue_index()
    files: dict[str, bytes] = {}
    receipt_documents = []
    projection_rows = []
    for clip_occurrence_id in sorted(EXPECTED_CLIPS):
        expected = EXPECTED_CLIPS[clip_occurrence_id]
        effect_id = expected["effectAssetId"]
        cue = cues.get(clip_occurrence_id)
        catalog = catalogs.get(effect_id)
        if (
            cue is None
            or cue.get("patternId") != PATTERN_ID
            or cue.get("actionId") != expected["gameplayActionId"]
            or cue.get("effectAssetId") != effect_id
            or cue.get("repeatPolicy") != "once"
        ):
            raise CanaryError(f"canonical v2 cue drifted: {clip_occurrence_id}")
        if catalog is None:
            raise CanaryError(f"Effect catalog row is missing: {effect_id}")
        authored_path = ROOT / "Data" / str(catalog.get("authoringPath") or "")
        if not authored_path.is_file() or AUTHORED_ROOT not in authored_path.parents:
            raise CanaryError(f"authored Portal Rush document is missing: {effect_id}")
        existing = read_json(authored_path)

        seeds = []
        source_element_keys = []
        occurrence_keys = []
        for occurrence in sorted(
            occurrences_by_clip[clip_occurrence_id],
            key=lambda row: row["fullKey"],
        ):
            if occurrence["sourceStageIndex"] != expected["sourceStageIndex"]:
                raise CanaryError(
                    f"Portal Rush exact source stage drifted: {clip_occurrence_id}"
                )
            occurrence_keys.append(occurrence["fullKey"])
            for carrier in systems.get(
                str(occurrence.get("sourceSystemId") or ""), {}
            ).values():
                if carrier["disposition"] != "EXECUTABLE_CORE":
                    continue
                if carrier.get("elementSeed") is None:
                    raise CanaryError("executable carrier has no element seed")
                seeded, source_key = compress_v13_source_node(
                    add_v13_transform_ownership_defaults(
                        source_inventory.occurrence_element_seed(
                            occurrence, carrier
                        )
                    )
                )
                seeds.append(seeded)
                source_element_keys.append(source_key)
                projection_rows.append(
                    {
                        "clipOccurrenceId": clip_occurrence_id,
                        "occurrenceFullKey": occurrence["fullKey"],
                        "carrierKey": carrier["carrierKey"],
                    }
                )
        seeds.sort(key=lambda row: str(row["sourceNode"]))
        candidate = {
            key: copy.deepcopy(value)
            for key, value in existing.items()
            if key != "elements"
        }
        candidate["elements"] = seeds
        validate_candidate_document(candidate, effect_id)

        candidate_path = output_root / candidate_filename(effect_id)
        relative_path = candidate_path.relative_to(ROOT).as_posix()
        candidate_payload = source_inventory.pretty_json_bytes(candidate)
        files[relative_path] = candidate_payload
        reconcile = compact_reconcile_plan(existing, candidate, candidate_path)
        receipt_documents.append(
            {
                "clipOccurrenceId": clip_occurrence_id,
                "sourceStageIndex": expected["sourceStageIndex"],
                "gameplayActionId": expected["gameplayActionId"],
                "effectAssetId": effect_id,
                "candidateDocumentPath": relative_path,
                "candidateDocumentSha256": source_inventory.sha256_bytes(
                    candidate_payload
                ),
                "sourceOccurrenceCount": len(occurrence_keys),
                "sourceOccurrenceFullKeys": sorted(occurrence_keys),
                "sourceElementKeys": sorted(
                    source_element_keys, key=lambda row: row["sourceNode"]
                ),
                "executableElementCount": len(seeds),
                "catalogDisposition": "REUSE_EXISTING_NO_MUTATION",
                "catalogRow": catalog,
                "cueDisposition": "REUSE_EXISTING_V2_NO_MUTATION",
                "cueRow": cue,
                "authoredDocumentPath": authored_path.relative_to(ROOT).as_posix(),
                "authoredDocumentSha256": source_inventory.sha256_file(
                    authored_path
                ),
                "reconcile": reconcile,
            }
        )

    receipt = {
        "schema": "lostark.valtan-portal-rush-imported-canary",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "patternId": PATTERN_ID,
        "mode": "IMMUTABLE_IMPORTED_CANDIDATES_AND_REPORT_ONLY_RECONCILE",
        "sourceSelectionManifest": {
            "path": selection_path.relative_to(ROOT).as_posix(),
            "sha256": source_inventory.sha256_file(selection_path),
        },
        "sourceProjectionSha256": source_inventory.canonical_sha256(
            sorted(
                projection_rows,
                key=lambda row: (
                    row["clipOccurrenceId"],
                    row["occurrenceFullKey"],
                    row["carrierKey"],
                ),
            )
        ),
        "excluded": [
            {
                "clipOccurrenceId": EXCLUDED_FINISH_CLIP,
                "disposition": "SOURCE_TIMING_REVIEW_REQUIRED",
            }
        ],
        "documents": receipt_documents,
        "summary": {
            "candidateDocumentCount": len(receipt_documents),
            "clipOccurrenceCount": len(receipt_documents),
            "sourceOccurrenceCount": sum(
                row["sourceOccurrenceCount"] for row in receipt_documents
            ),
            "executableElementCount": sum(
                row["executableElementCount"] for row in receipt_documents
            ),
            "preservedAuthoredElementCount": sum(
                row["reconcile"]["preservedExistingElementCount"]
                for row in receipt_documents
            ),
            "missingOnlyAddElementCount": sum(
                len(row["reconcile"]["addElementRefs"])
                for row in receipt_documents
            ),
            "deletedElementCount": 0,
            "sourceRebaseRequiredCount": sum(
                len(row["reconcile"]["sourceRebaseRequired"])
                for row in receipt_documents
            ),
            "excludedTimingReviewClipCount": 1,
        },
    }
    if receipt["summary"]["candidateDocumentCount"] != 3:
        raise CanaryError("Portal Rush canary must emit exactly three documents")
    if receipt["summary"]["deletedElementCount"] != 0:
        raise CanaryError("Portal Rush canary may not delete authored elements")
    receipt_path = output_root / RECEIPT_NAME
    files[receipt_path.relative_to(ROOT).as_posix()] = (
        source_inventory.pretty_json_bytes(receipt)
    )
    return files, receipt


def check_exact(path: Path, payload: bytes) -> None:
    if not path.is_file() or path.read_bytes() != payload:
        raise CanaryError(f"Portal Rush imported canary drifted: {path}")


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
    authored_root = AUTHORED_ROOT.resolve()
    if output_root == authored_root or authored_root in output_root.parents:
        parser.error("Portal Rush imported canary cannot target Authored")
    files, receipt = build_canary(selection_path, output_root)
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
        "Valtan Portal Rush imported canary "
        f"{label}: {json.dumps(receipt['summary'], sort_keys=True)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (CanaryError, source_inventory.InventoryError) as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
