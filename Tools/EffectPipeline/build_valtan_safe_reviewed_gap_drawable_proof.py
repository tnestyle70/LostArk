#!/usr/bin/env python3
"""Seal the 167-element drawable and adapter-join proof for safe Valtan gaps."""

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
import build_valtan_trail_adapter_packets as trail_packets


SWEEP_PATH = (
    candidates.OUTPUT_ROOT
    / "DrawableProof/Valtan.safe-reviewed-gaps.drawable-sweep.v1.json"
)
PROOF_PATH = (
    candidates.OUTPUT_ROOT
    / "DrawableProof/Valtan.safe-reviewed-gaps.drawable-proof.v1.json"
)
SCHEMA = "lostark.valtan-safe-reviewed-gap-drawable-proof"
FORMAT_VERSION = 1


class ProofError(RuntimeError):
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


def read_json(path: Path) -> dict[str, Any]:
    try:
        result = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ProofError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(result, dict):
        raise ProofError(f"JSON root is not an object: {path}")
    return result


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def seal(row: dict[str, Any], field: str) -> None:
    row.pop(field, None)
    row[field] = canonical_sha256(row)


def verify_seal(row: dict[str, Any], field: str, label: str) -> None:
    expected = row.get(field)
    clone = copy.deepcopy(row)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise ProofError(f"{label} {field} is stale")


def relative(path: Path) -> str:
    return path.resolve().relative_to(ROOT.resolve()).as_posix()


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    os.replace(temporary, path)


def _target_contract(element: dict[str, Any]) -> dict[str, Any]:
    detail = element.get("detail") or {}
    return {
        "resources": element.get("resources"),
        "material": element.get("material"),
        "actionCueAttachment": element.get("actionCueAttachment"),
        "timing": detail.get("timing"),
        "trail": detail.get("trail"),
        "sourceRecipe": element.get("sourceRecipe"),
    }


def validate_proof(document: dict[str, Any]) -> None:
    if (
        document.get("schema") != SCHEMA
        or document.get("formatVersion") != FORMAT_VERSION
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
        or document.get("disposition") != "PROOF_PASS_CANONICAL_APPLY_ALLOWED"
    ):
        raise ProofError("safe-gap drawable proof header is invalid")
    verify_seal(document, "artifactSha256", "safe-gap drawable proof")
    summary = document.get("summary") or {}
    if (
        summary.get("documentCount") != 4
        or summary.get("visibleElementCount") != 167
        or summary.get("preparedElementCount") != 167
        or summary.get("drawnElementCount") != 167
        or summary.get("adapterProjectionCount") != 7
        or summary.get("animationTrailProjectionCount") != 6
        or summary.get("cascadeRibbonProjectionCount") != 1
        or summary.get("failedDrawCount") != 0
    ):
        raise ProofError("safe-gap proof denominator changed")


def build(
    manifest_path: Path = candidates.MANIFEST_PATH,
    sweep_path: Path = SWEEP_PATH,
) -> dict[str, Any]:
    manifest = read_json(manifest_path)
    candidates.validate_manifest(manifest)
    sweep = read_json(sweep_path)
    if (
        sweep.get("schema") != "lostark.effect-document-drawable-sweep"
        or sweep.get("formatVersion") != 1
        or sweep.get("sampleRateHz") != 60
    ):
        raise ProofError("drawable sweep header is invalid")
    candidate_by_effect = {
        str(row["effectAssetId"]): row for row in manifest["candidateDocuments"]
    }
    sweep_by_effect = {
        str(row.get("effectAssetId") or ""): row
        for row in sweep.get("documents", [])
        if isinstance(row, dict)
    }
    if set(candidate_by_effect) != set(sweep_by_effect):
        raise ProofError("drawable sweep candidate set is not exact")

    document_proofs: list[dict[str, Any]] = []
    total_visible = total_prepared = total_drawn = total_failed = 0
    documents: dict[str, dict[str, Any]] = {}
    for effect_id in sorted(candidate_by_effect):
        candidate_row = candidate_by_effect[effect_id]
        path = ROOT / candidate_row["candidatePath"]
        candidate = read_json(path)
        documents[effect_id] = candidate
        sweep_row = sweep_by_effect[effect_id]
        expected_ids = set(candidate_row["elementIds"])
        element_rows = sweep_row.get("elements") or []
        actual_ids = {
            str(row.get("elementId") or "")
            for row in element_rows
            if isinstance(row, dict)
        }
        if (
            sweep_row.get("disposition") != "DRAWABLE_PROOF_PASS"
            or actual_ids != expected_ids
            or sweep_row.get("visibleElementCount") != len(expected_ids)
            or sweep_row.get("preparedElementCount") != len(expected_ids)
            or sweep_row.get("drawnElementCount") != len(expected_ids)
        ):
            raise ProofError(f"drawable sweep denominator changed: {effect_id}")
        for row in element_rows:
            if (
                row.get("disposition") != "DRAWABLE_PROOF_PASS"
                or int(row.get("preparedSamples") or 0) <= 0
                or int(row.get("attemptedSamples") or 0) <= 0
                or int(row.get("submittedDraws") or 0) <= 0
                or int(row.get("committedDraws") or 0) <= 0
                or int(row.get("failedDraws") or 0) != 0
            ):
                raise ProofError(
                    f"element did not draw transactionally: {effect_id}/"
                    f"{row.get('elementId')}"
                )
            total_failed += int(row.get("failedDraws") or 0)
        total_visible += int(sweep_row["visibleElementCount"])
        total_prepared += int(sweep_row["preparedElementCount"])
        total_drawn += int(sweep_row["drawnElementCount"])
        document_proofs.append(
            {
                "effectAssetId": effect_id,
                "candidatePath": candidate_row["candidatePath"],
                "candidateRawSha256": raw_sha256(path),
                "candidateCanonicalSha256": canonical_sha256(candidate),
                "elementCount": len(expected_ids),
                "sweepElementRowsSha256": canonical_sha256(element_rows),
                "disposition": "DRAWABLE_PROOF_PASS",
            }
        )

    adapter_document = read_json(candidates.TRAIL_PACKETS_PATH)
    trail_packets.validate_document(adapter_document, ROOT)
    adapters = {
        str(row["adapterTargetId"]): row for row in adapter_document["adapters"]
    }
    adapter_proofs: list[dict[str, Any]] = []
    animation_count = ribbon_count = 0
    for projection in manifest["adapterProjections"]:
        verify_seal(projection, "projectionSha256", "adapter projection")
        source = adapters.get(str(projection["sourceAdapterTargetId"]))
        if source is None or source.get("disposition") != "ADMITTED_RENDERER_READY":
            raise ProofError("adapter projection lost its admitted source row")
        packet = source.get("packet") or {}
        if (
            source.get("rowSha256") != projection["sourceAdapterRowSha256"]
            or packet.get("packetSha256") != projection["sourcePacketSha256"]
            or source.get("sourceIdentitySha256")
            != projection["sourceIdentitySha256"]
            or (packet.get("rendererAdmission") or {}).get("sharedRendererPath")
            != "EFFECT_TRAIL_SHARED_HEADLESS_V1"
        ):
            raise ProofError("adapter packet identity changed")
        target_doc = documents.get(str(projection["effectAssetId"]))
        targets = [
            row
            for row in (target_doc or {}).get("elements", [])
            if row.get("id") == projection["targetElementId"]
        ]
        if (
            len(targets) != 1
            or canonical_sha256(_target_contract(targets[0]))
            != projection["targetContractSha256"]
        ):
            raise ProofError("adapter packet/target document join changed")
        family = str(source["family"])
        if family == trail_packets.FAMILY_ANIMATION_TRAIL:
            history = packet.get("history") or {}
            if (
                packet.get("packetLayout")
                != "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1"
                or history.get("historyId") != trail_packets.WHIRLWIND_HISTORY_ID
                or history.get("sampleCount") != 409
                or abs(float(history.get("playbackClampSeconds")) - 1.2000000476837158)
                > 1e-7
            ):
                raise ProofError("AnimationTrail exact history join changed")
            animation_count += 1
        elif family == trail_packets.FAMILY_CASCADE_RIBBON:
            if (
                packet.get("packetLayout") != "CASCADE_RIBBON_TYPED_PACKET_V1"
                or packet.get("resolvedRendererShape") != "ribbon"
                or packet.get("sourceCarrierKey")
                != (source.get("sourceIdentity") or {}).get("carrierKey")
            ):
                raise ProofError("CascadeRibbon typed carrier join changed")
            ribbon_count += 1
        else:
            raise ProofError("safe-gap adapter family changed")
        adapter_proofs.append(
            {
                "projectionId": projection["projectionId"],
                "sourceAdapterTargetId": source["adapterTargetId"],
                "sourceAdapterRowSha256": source["rowSha256"],
                "sourcePacketSha256": packet["packetSha256"],
                "family": family,
                "effectAssetId": projection["effectAssetId"],
                "targetElementId": projection["targetElementId"],
                "targetContractSha256": projection["targetContractSha256"],
                "disposition": "PACKET_TARGET_DRAW_JOIN_PASS",
            }
        )

    canary = trail_packets.load_whirlwind_canary(ROOT)
    if (
        total_visible != total_prepared or total_visible != total_drawn
        or total_visible != 167
        or total_failed != 0
        or len(adapter_proofs) != 7
        or animation_count != 6
        or ribbon_count != 1
    ):
        raise ProofError("safe-gap proof aggregate changed")
    proof = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "disposition": "PROOF_PASS_CANONICAL_APPLY_ALLOWED",
        "candidateManifest": {
            "path": relative(manifest_path),
            "rawSha256": raw_sha256(manifest_path),
            "canonicalSha256": canonical_sha256(manifest),
            "artifactSha256": manifest["artifactSha256"],
        },
        "drawableSweep": {
            "path": relative(sweep_path),
            "rawSha256": raw_sha256(sweep_path),
            "canonicalSha256": canonical_sha256(sweep),
            "sampleRateHz": 60,
        },
        "trailAdapterPackets": {
            "path": relative(candidates.TRAIL_PACKETS_PATH),
            "rawSha256": raw_sha256(candidates.TRAIL_PACKETS_PATH),
            "canonicalSha256": canonical_sha256(adapter_document),
            "artifactSha256": adapter_document["artifactSha256"],
        },
        "whirlwindCanary": canary,
        "documents": document_proofs,
        "adapterJoins": sorted(
            adapter_proofs, key=lambda row: str(row["projectionId"])
        ),
        "preservedUnresolvedTrailRows": copy.deepcopy(
            manifest["preservedUnresolvedTrailRows"]
        ),
        "summary": {
            "documentCount": len(document_proofs),
            "visibleElementCount": total_visible,
            "preparedElementCount": total_prepared,
            "drawnElementCount": total_drawn,
            "failedDrawCount": total_failed,
            "adapterProjectionCount": len(adapter_proofs),
            "animationTrailProjectionCount": animation_count,
            "cascadeRibbonProjectionCount": ribbon_count,
            "preservedUnresolvedTrailRowCount": len(
                manifest["preservedUnresolvedTrailRows"]
            ),
        },
    }
    seal(proof, "artifactSha256")
    validate_proof(proof)
    return proof


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--dry-run", action="store_true")
    parser.add_argument("--manifest", type=Path, default=candidates.MANIFEST_PATH)
    parser.add_argument("--drawable-sweep", type=Path, default=SWEEP_PATH)
    parser.add_argument("--output", type=Path, default=PROOF_PATH)
    args = parser.parse_args()
    proof = build(args.manifest, args.drawable_sweep)
    payload = pretty_bytes(proof)
    if args.write:
        write_atomic(args.output, payload)
        label = "written"
    elif args.check:
        if not args.output.is_file() or args.output.read_bytes() != payload:
            raise ProofError(f"safe-gap drawable proof drifted: {args.output}")
        label = "checked"
    else:
        label = "dry-run"
    print(
        "Valtan safe reviewed gap drawable proof "
        + label
        + ": "
        + json.dumps(proof["summary"], sort_keys=True)
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ProofError, candidates.SafeGapError, trail_packets.AdapterError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
