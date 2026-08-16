#!/usr/bin/env python3
"""Atomically admit exact current-combat intake and explicit source-only targets.

This transaction is intentionally narrower than the normal four-class rebuild.
It copies only named skill artifacts, adds only those skills to one class stage
manifest, deterministically rebuilds the shared source-Material contract, and
invokes the explicit-target Track A seam.  Catalog, animevents, role visibility,
and every pre-existing authored Effect outside the named targets are immutable.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys
import tempfile
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
PIPELINE_ROOT = Path(__file__).resolve().parent
if str(PIPELINE_ROOT) not in sys.path:
    sys.path.insert(0, str(PIPELINE_ROOT))

import build_four_class_combat_source_intake as intake  # noqa: E402
import build_four_class_source_material_contract as material_contract  # noqa: E402
import materialize_four_class_track_a_candidates as materializer  # noqa: E402


TRANSACTION_RECEIPT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.targeted-current-combat-transaction.receipt.json"
)
PRE_DF_LANCE_MANIFEST_SHA256 = (
    "a22ab8babdc7bada4b699838fd7ed4c32588f020bda49c5681a0ec4ecc151b90"
)


class TargetedTransactionError(RuntimeError):
    """Raised when a targeted transaction cannot prove its exact boundary."""


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_source_artifact_text(path: Path) -> str:
    payload = path.read_bytes()
    if payload.startswith(b"\xef\xbb\xbf"):
        raise TargetedTransactionError(
            f"source artifact must be UTF-8 without BOM: {path}"
        )
    return payload.decode("utf-8")


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def serialized(value: dict[str, Any]) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"


def repository_label(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT.resolve()).as_posix()
    except ValueError as error:
        raise TargetedTransactionError(
            f"transaction output escaped the repository: {path}"
        ) from error


def descriptor(path: Path, source: Path, effect_id: str | None = None) -> dict[str, Any]:
    row: dict[str, Any] = {
        "path": repository_label(path),
        "sha256": raw_sha256(source),
    }
    if effect_id is not None:
        row["effectAssetId"] = effect_id
    return row


def parse_target(value: str) -> tuple[int, int, int, str]:
    parts = value.split(":", 3)
    if len(parts) != 4:
        raise TargetedTransactionError(
            "target must be SKILL:STAGE:STAGE_CLIP:EFFECT_ID: " + value
        )
    try:
        skill_id, stage_index, stage_clip_index = map(int, parts[:3])
    except ValueError as error:
        raise TargetedTransactionError(f"invalid target numeric identity: {value}") from error
    return skill_id, stage_index, stage_clip_index, parts[3]


def find_config(asset_id: str) -> intake.ClassConfig:
    matches = [row for row in intake.MATERIALIZED_CLASS_CONFIGS if row.asset_id == asset_id]
    if len(matches) != 1:
        raise TargetedTransactionError(
            f"materialized class config is missing/ambiguous: {asset_id}"
        )
    return matches[0]


def canonical_artifact_paths(asset_id: str, skill_id: int) -> dict[str, Path]:
    current = ROOT / "Data/Effects/Imported" / asset_id / "CurrentCombat"
    return {
        "sourceReceipt": current / f"skill.{skill_id}.source-receipt.json",
        "normalizedGraph": current / "Graphs" / f"skill.{skill_id}.normalized-effect-graph.json",
        "externalModuleClosure": current / "Modules" / f"skill.{skill_id}.external-module-closure.json",
        "importedDocument": current / "Converted" / f"effect.{asset_id.casefold()}.skill.{skill_id}.imported.effect.json",
        "conversionReceipt": current / "Converted" / f"skill.{skill_id}.element-conversion-receipt.json",
        "productSourceReceipt": current / "ProductReceipts" / f"skill.{skill_id}.product-source-receipt.json",
    }


def source_artifact_paths(source_root: Path, asset_id: str, skill_id: int) -> dict[str, Path]:
    return {
        "sourceReceipt": source_root / f"skill.{skill_id}.source-receipt.json",
        "normalizedGraph": source_root / "Graphs" / f"skill.{skill_id}.normalized-effect-graph.json",
        "externalModuleClosure": source_root / "Modules" / f"skill.{skill_id}.external-module-closure.json",
        "importedDocument": source_root / "Converted" / f"effect.{asset_id.casefold()}.skill.{skill_id}.imported.effect.json",
        "conversionReceipt": source_root / "Converted" / f"skill.{skill_id}.element-conversion-receipt.json",
    }


def product_source_receipt_value(
    config: intake.ClassConfig,
    skill: dict[str, Any],
    generated: dict[str, Any],
    receipt: dict[str, Any],
) -> dict[str, Any]:
    clips = [
        {
            "clip": clip["clip"],
            "stageIndex": stage["stageIndex"],
            "stageClipIndex": clip["stageClipIndex"],
            "sequenceIndex": clip["sequenceIndex"],
            "sourceSkillId": clip["sourceSkillId"],
            "sourceLine": clip["sourceLine"],
        }
        for stage in skill["stages"]
        for clip in stage["clips"]
    ]
    return {
        "schema": "lostark.combat-effect-product-source-receipt",
        "version": 1,
        "animationAssetId": config.asset_id,
        "characterClass": config.character_class,
        "productSkillId": int(skill["productSkillId"]),
        "sourceSkillIds": skill["sourceSkillIds"],
        "clipOwnershipProvenance": {
            "joinKey": "exact bound clip identity",
            "numericSkillAliasIsJoinKey": False,
            "clips": clips,
        },
        "generatedSourceReceipt": generated["sourceReceipt"],
        "normalizedGraph": generated["normalizedGraph"],
        "timeline": receipt.get("timeline", {}),
        "summary": receipt.get("summary", {}),
    }


def build_intake_outputs(
    *,
    config: intake.ClassConfig,
    source_root: Path,
    targets: list[tuple[int, int, int, str]],
) -> tuple[dict[Path, str], dict[str, Any], list[str]]:
    data_root = ROOT / "Data"
    bindings_path = (
        data_root
        / "Animation"
        / "Authored"
        / config.asset_id
        / f"{config.asset_id}.skillbindings.json"
    )
    animnotify_path = (
        data_root
        / "Animation"
        / "Reference"
        / config.asset_id
        / f"{config.asset_id}.animnotify"
    )
    player_skills_path = data_root / "Balance" / "PlayerSkills.json"
    base_manifest = intake.build_class_stage_contract(
        config,
        intake.read_json(player_skills_path),
        bindings_path,
        animnotify_path,
        player_skills_path,
    )
    manifest_path = (
        data_root
        / "Effects"
        / "Imported"
        / config.asset_id
        / f"{config.asset_id}.combat-source-stage-manifest.json"
    )
    current_manifest = intake.read_json(manifest_path)
    if (
        current_manifest.get("schema")
        != "lostark.combat-effect-source-stage-manifest"
        or current_manifest.get("characterClass") != config.character_class
    ):
        raise TargetedTransactionError(f"stage manifest identity changed: {manifest_path}")

    requested = {row[0]: row for row in targets}
    if len(requested) != len(targets):
        raise TargetedTransactionError("target skill IDs must be unique")
    base_by_skill = {
        int(row["productSkillId"]): copy.deepcopy(row)
        for row in base_manifest.get("skills", [])
    }
    current_by_skill = {
        int(row["productSkillId"]): copy.deepcopy(row)
        for row in current_manifest.get("skills", [])
    }
    if set(current_by_skill) - set(base_by_skill):
        raise TargetedTransactionError("current manifest has skills outside exact bindings")

    outputs: dict[Path, str] = {}
    new_skills: dict[int, dict[str, Any]] = {}
    target_specs: list[str] = []
    source_rows: list[dict[str, Any]] = []
    for skill_id, stage_index, stage_clip_index, target_effect_id in targets:
        skill = base_by_skill.get(skill_id)
        if skill is None:
            raise TargetedTransactionError(f"target skill is not bound: {skill_id}")
        stages = [
            row
            for row in skill.get("stages", [])
            if int(row.get("stageIndex", -1)) == stage_index
        ]
        if len(stages) != 1:
            raise TargetedTransactionError(f"target stage is missing: {skill_id}/{stage_index}")
        clips = [
            row
            for row in stages[0].get("clips", [])
            if int(row.get("stageClipIndex", -1)) == stage_clip_index
        ]
        if len(stages) != 1 or len(clips) != 1 or len(skill.get("stages", [])) != 1 or len(stages[0].get("clips", [])) != 1:
            raise TargetedTransactionError(
                "this source-only transaction requires one exact stage/clip per skill: "
                f"{skill_id}"
            )

        sources = source_artifact_paths(source_root, config.asset_id, skill_id)
        missing = [path for path in sources.values() if not path.is_file()]
        if missing:
            raise TargetedTransactionError(
                f"target source artifact is missing: {skill_id}/{missing}"
            )
        canonical = canonical_artifact_paths(config.asset_id, skill_id)
        receipt = intake.read_json(sources["sourceReceipt"])
        source_clips = intake.receipt_clip_names(receipt)
        bound_clips = intake.flattened_skill_clips(skill)
        if (
            int(receipt.get("productSkillId", -1)) != skill_id
            or receipt.get("characterClass") != config.character_class
            or source_clips != bound_clips
        ):
            raise TargetedTransactionError(
                f"target receipt clip/skill identity changed: {skill_id}"
            )
        closure = intake.read_json(sources["externalModuleClosure"])
        if (
            int(closure.get("summary", {}).get("unresolvedRequestCount", -1)) != 0
            or int(closure.get("summary", {}).get("propertyErrorCount", -1)) != 0
        ):
            raise TargetedTransactionError(f"target closure is incomplete: {skill_id}")
        imported = intake.read_json(sources["importedDocument"])
        effect_id = str(imported.get("effectAssetId") or "")
        if effect_id != f"effect.{config.character_class.casefold()}.skill.{skill_id}.imported":
            raise TargetedTransactionError(f"Imported Effect identity changed: {effect_id}")

        generated = {
            "sourceReceipt": descriptor(canonical["sourceReceipt"], sources["sourceReceipt"]),
            "normalizedGraph": descriptor(canonical["normalizedGraph"], sources["normalizedGraph"]),
            "externalModuleClosure": descriptor(canonical["externalModuleClosure"], sources["externalModuleClosure"]),
            "importedDocument": descriptor(canonical["importedDocument"], sources["importedDocument"], effect_id),
            "conversionReceipt": descriptor(canonical["conversionReceipt"], sources["conversionReceipt"]),
            "artifactOrigin": "CURRENT_COMBAT_GENERATED_SOURCE_TRUTH",
        }
        wrapper = product_source_receipt_value(config, skill, generated, receipt)
        wrapper_payload = serialized(wrapper)
        wrapper_sha = hashlib.sha256(wrapper_payload.encode("utf-8")).hexdigest()
        source_artifacts = {
            **generated,
            "generatedSourceReceipt": generated["sourceReceipt"],
            "sourceReceipt": {
                "path": repository_label(canonical["productSourceReceipt"]),
                "sha256": wrapper_sha,
            },
        }

        temporary_artifacts = {
            "sourceReceipt": {"path": sources["sourceReceipt"].as_posix()},
            "normalizedGraph": {"path": sources["normalizedGraph"].as_posix()},
            "externalModuleClosure": {"path": sources["externalModuleClosure"].as_posix()},
            "importedDocument": {"path": sources["importedDocument"].as_posix()},
            "conversionReceipt": {"path": sources["conversionReceipt"].as_posix()},
        }
        blockers = intake.collect_artifact_blockers(temporary_artifacts)
        intake.attach_skill_artifacts(skill, source_artifacts, blockers, receipt)
        skill["materializationLane"] = materializer.TARGETED_CURRENT_COMBAT_LANE
        skill["targetedSourceOnlyTargets"] = [
            {
                "stageIndex": stage_index,
                "stageClipIndex": stage_clip_index,
                "clip": clips[0]["clip"],
                "effectAssetId": target_effect_id,
            }
        ]
        new_skills[skill_id] = skill
        for key in (
            "sourceReceipt",
            "normalizedGraph",
            "externalModuleClosure",
            "importedDocument",
            "conversionReceipt",
        ):
            payload = load_source_artifact_text(sources[key])
            if hashlib.sha256(payload.encode("utf-8")).hexdigest() != raw_sha256(
                sources[key]
            ):
                raise TargetedTransactionError(
                    f"source artifact text hash drift: {sources[key]}"
                )
            outputs[canonical[key]] = payload
        outputs[canonical["productSourceReceipt"]] = wrapper_payload
        target_specs.append(
            f"{config.character_class}:{skill_id}:{stage_index}:"
            f"{stage_clip_index}:{target_effect_id}"
        )
        source_rows.append(
            {
                "skillId": skill_id,
                "sourceReceiptSha256": raw_sha256(sources["sourceReceipt"]),
                "normalizedGraphSha256": raw_sha256(sources["normalizedGraph"]),
                "externalModuleClosureSha256": raw_sha256(sources["externalModuleClosure"]),
                "importedDocumentSha256": raw_sha256(sources["importedDocument"]),
                "conversionReceiptSha256": raw_sha256(sources["conversionReceipt"]),
                "blockers": blockers,
            }
        )

    merged_manifest = copy.deepcopy(current_manifest)
    merged_manifest["skills"] = [
        new_skills[skill_id]
        if skill_id in new_skills
        else current_by_skill[skill_id]
        for skill_id in base_by_skill
        if skill_id in current_by_skill or skill_id in new_skills
    ]
    if len(merged_manifest["skills"]) != len(current_by_skill) + len(
        set(new_skills) - set(current_by_skill)
    ):
        raise TargetedTransactionError("manifest merge changed an unrelated skill")
    merged_manifest["summary"] = copy.deepcopy(base_manifest["summary"])
    merged_manifest["summary"].update(
        {
            "readySkillCount": sum(row["status"] == "READY" for row in merged_manifest["skills"]),
            "readyStageCount": sum(
                stage["status"] == "READY"
                for row in merged_manifest["skills"]
                for stage in row["stages"]
            ),
            "blockedSkillCount": sum(row["status"] == "BLOCKED" for row in merged_manifest["skills"]),
            "blockedStageCount": sum(
                stage["status"] == "BLOCKED"
                for row in merged_manifest["skills"]
                for stage in row["stages"]
            ),
            "availableWithBlockersSkillCount": sum(
                row["status"] == "AVAILABLE_WITH_BLOCKERS"
                for row in merged_manifest["skills"]
            ),
            "availableWithBlockersStageCount": sum(
                stage["status"] == "AVAILABLE_WITH_BLOCKERS"
                for row in merged_manifest["skills"]
                for stage in row["stages"]
            ),
        }
    )
    for skill_id, old_skill in current_by_skill.items():
        if skill_id not in new_skills:
            current_row = next(
                row
                for row in merged_manifest["skills"]
                if int(row["productSkillId"]) == skill_id
            )
            if current_row != old_skill:
                raise TargetedTransactionError(
                    f"manifest merge mutated existing skill: {skill_id}"
                )
    outputs[manifest_path] = serialized(merged_manifest)
    return outputs, {"sources": source_rows, "manifest": merged_manifest}, target_specs


def immutable_snapshot(excluded: set[Path]) -> dict[Path, str]:
    paths = set((ROOT / "Data/Effects/Authored").glob("*.effect.json"))
    paths.update((ROOT / "Data/Animation/Authored").rglob("*.animevents"))
    paths.add(ROOT / "Data/Effects/EffectCatalog.json")
    paths.update(
        path
        for path in materializer.CLASS_MANIFESTS.values()
        if path not in excluded
    )
    return {
        path: raw_sha256(path)
        for path in sorted(paths, key=lambda value: value.as_posix())
        if path.is_file() and path not in excluded
    }


def restore_snapshots(snapshots: dict[Path, bytes | None]) -> None:
    for path, payload in reversed(list(snapshots.items())):
        if payload is None:
            if path.exists():
                path.unlink()
            continue
        path.parent.mkdir(parents=True, exist_ok=True)
        descriptor_id, temporary_name = tempfile.mkstemp(
            prefix=f".{path.name}.", suffix=".rollback", dir=path.parent
        )
        with os.fdopen(descriptor_id, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_name, path)


def run_transaction(
    *,
    config: intake.ClassConfig,
    source_root: Path,
    targets: list[tuple[int, int, int, str]],
) -> dict[str, Any]:
    intake_outputs, intake_receipt, target_specs = build_intake_outputs(
        config=config,
        source_root=source_root,
        targets=targets,
    )
    target_paths = {
        ROOT / "Data/Effects/Authored" / f"{row[3]}.effect.json"
        for row in targets
    }
    manifest_path = materializer.CLASS_MANIFESTS[config.character_class]
    immutable_before = immutable_snapshot(target_paths | {manifest_path})
    snapshots: dict[Path, bytes | None] = {}

    def remember(paths: Any) -> None:
        for path in paths:
            if path not in snapshots:
                snapshots[path] = path.read_bytes() if path.is_file() else None

    try:
        remember(intake_outputs)
        materializer.commit_transaction(intake_outputs)

        contract, contract_receipt = material_contract.build_projection()
        contract_outputs = {
            material_contract.OUTPUT_PATH: material_contract.serialized(contract),
            material_contract.RECEIPT_PATH: material_contract.serialized(contract_receipt),
        }
        remember(contract_outputs)
        materializer.commit_transaction(contract_outputs)
        material_contract.check(contract_outputs)

        target_outputs, target_receipt = materializer.build_targeted_projection(
            target_specs
        )
        remember(target_outputs)
        materializer.commit_transaction(target_outputs)
        materializer.check_outputs(target_outputs)

        # A source-only target has no rollback blueprint on its first pass.  Rebuild
        # once after the authored documents exist so the receipt records the same
        # stable-reimport decision that every subsequent --check observes.  The
        # authored payloads themselves must already be a fixed point.
        stable_outputs, target_receipt = materializer.build_targeted_projection(
            target_specs
        )
        for target_path in target_paths:
            if stable_outputs.get(target_path) != target_outputs.get(target_path):
                raise TargetedTransactionError(
                    f"targeted authored document is not a stable fixed point: "
                    f"{repository_label(target_path)}"
                )
        materializer.commit_transaction(stable_outputs)
        materializer.check_outputs(stable_outputs)
        target_outputs = stable_outputs

        immutable_after = immutable_snapshot(target_paths | {manifest_path})
        if immutable_after != immutable_before:
            changed = sorted(
                repository_label(path)
                for path in set(immutable_before) | set(immutable_after)
                if immutable_before.get(path) != immutable_after.get(path)
            )
            raise TargetedTransactionError(
                "transaction changed an immutable authored/catalog/animation file: "
                + ", ".join(changed)
            )

        changed_paths = sorted(
            set(intake_outputs) | set(contract_outputs) | set(target_outputs),
            key=lambda path: path.as_posix(),
        )
        receipt = {
            "schema": "lostark.targeted-current-combat-transaction-receipt",
            "formatVersion": 1,
            "characterClass": config.character_class,
            "animationAssetId": config.asset_id,
            "policy": {
                "explicitTargetsOnly": True,
                "defaultFullGateUnchanged": True,
                "globalDenominatorBypass": False,
                "sourceOnlyTargets": True,
                "catalogMutation": False,
                "animeventMutation": False,
                "roleVisibilityMutation": False,
                "rollbackOnFailure": True,
            },
            "sourceRoot": source_root.as_posix(),
            "targetSpecs": target_specs,
            "intake": {
                "manifestCanonicalSha256": canonical_sha256(
                    intake_receipt["manifest"]
                ),
                "sources": intake_receipt["sources"],
            },
            "sourceMaterialContract": contract_receipt["counts"],
            "targetedMaterialization": target_receipt["counts"],
            "immutableFileCount": len(immutable_before),
            "changedFiles": [
                {
                    "path": repository_label(path),
                    "sha256": raw_sha256(path),
                }
                for path in changed_paths
            ],
        }
        receipt["artifactSha256"] = canonical_sha256(receipt)
        receipt_output = {TRANSACTION_RECEIPT_PATH: serialized(receipt)}
        remember(receipt_output)
        materializer.commit_transaction(receipt_output)
        return receipt
    except Exception:
        restore_snapshots(snapshots)
        raise


def check_transaction(
    *,
    config: intake.ClassConfig,
    source_root: Path,
    targets: list[tuple[int, int, int, str]],
) -> dict[str, Any]:
    intake_outputs, _, target_specs = build_intake_outputs(
        config=config,
        source_root=source_root,
        targets=targets,
    )
    materializer.check_outputs(intake_outputs)
    contract, contract_receipt = material_contract.build_projection()
    material_contract.check(
        {
            material_contract.OUTPUT_PATH: material_contract.serialized(contract),
            material_contract.RECEIPT_PATH: material_contract.serialized(
                contract_receipt
            ),
        }
    )
    outputs, target_receipt = materializer.build_targeted_projection(target_specs)
    materializer.check_outputs(outputs)
    if not TRANSACTION_RECEIPT_PATH.is_file():
        raise TargetedTransactionError("transaction receipt is missing")
    return {
        "sourceMaterialContract": contract_receipt["counts"],
        "targetedMaterialization": target_receipt["counts"],
    }


def rollback_partial_df_transaction(config: intake.ClassConfig) -> dict[str, Any]:
    receipt = intake.read_json(TRANSACTION_RECEIPT_PATH)
    if (
        receipt.get("schema")
        != "lostark.targeted-current-combat-transaction-receipt"
        or receipt.get("characterClass") != "LANCE_MASTER"
        or receipt.get("targetSpecs")
        != [
            "LANCE_MASTER:34110:0:0:effect.lancemaster.skill.34110.unified",
            "LANCE_MASTER:34150:0:0:effect.lancemaster.skill.34150.unified",
        ]
    ):
        raise TargetedTransactionError("refusing to rollback an unknown transaction")
    changed = {
        ROOT / row["path"]: row["sha256"]
        for row in receipt.get("changedFiles", [])
    }
    manifest_path = materializer.CLASS_MANIFESTS["LANCE_MASTER"]
    contract_paths = {
        material_contract.OUTPUT_PATH,
        material_contract.RECEIPT_PATH,
    }
    created_paths = set(changed) - contract_paths - {manifest_path}
    snapshots = {
        path: path.read_bytes() if path.is_file() else None
        for path in created_paths
        | contract_paths
        | {manifest_path, TRANSACTION_RECEIPT_PATH}
    }
    try:
        for path in sorted(created_paths, key=lambda row: row.as_posix()):
            if (
                not path.resolve().is_relative_to(ROOT.resolve())
                or not path.is_file()
                or raw_sha256(path) != changed[path]
            ):
                raise TargetedTransactionError(
                    f"partial output changed since transaction: {path}"
                )

        manifest = intake.read_json(manifest_path)
        target_rows = [
            row
            for row in manifest.get("skills", [])
            if int(row.get("productSkillId", -1)) in {34110, 34150}
        ]
        if (
            len(target_rows) != 2
            or any(
                row.get("materializationLane")
                != materializer.TARGETED_CURRENT_COMBAT_LANE
                for row in target_rows
            )
        ):
            raise TargetedTransactionError("Lance D/F manifest rows are not ours")
        manifest["skills"] = [
            row
            for row in manifest["skills"]
            if int(row["productSkillId"]) not in {34110, 34150}
        ]
        stages = [stage for row in manifest["skills"] for stage in row["stages"]]
        clips = [clip for stage in stages for clip in stage["clips"]]
        manifest["summary"].update(
            {
                "skillCount": len(manifest["skills"]),
                "stageCount": len(stages),
                "clipOccurrenceCount": len(clips),
                "numericAliasClipOccurrenceCount": 0,
                "readySkillCount": sum(
                    row["status"] == "READY" for row in manifest["skills"]
                ),
                "readyStageCount": sum(stage["status"] == "READY" for stage in stages),
                "blockedSkillCount": sum(
                    row["status"] == "BLOCKED" for row in manifest["skills"]
                ),
                "blockedStageCount": sum(
                    stage["status"] == "BLOCKED" for stage in stages
                ),
                "availableWithBlockersSkillCount": sum(
                    row["status"] == "AVAILABLE_WITH_BLOCKERS"
                    for row in manifest["skills"]
                ),
                "availableWithBlockersStageCount": sum(
                    stage["status"] == "AVAILABLE_WITH_BLOCKERS"
                    for stage in stages
                ),
            }
        )
        manifest_payload = serialized(manifest).replace("\n", "\r\n")
        if hashlib.sha256(manifest_payload.encode("utf-8")).hexdigest() != (
            PRE_DF_LANCE_MANIFEST_SHA256
        ):
            raise TargetedTransactionError("reconstructed pre-D/F manifest hash drifted")
        materializer.commit_transaction({manifest_path: manifest_payload})

        for path in sorted(created_paths, key=lambda row: row.as_posix()):
            path.unlink()

        contract, contract_receipt = material_contract.build_projection()
        counts = contract_receipt["counts"]
        if (
            counts.get("compiledMaterialIdentityCount") != 792
            or counts.get("totalMaterialIdentityCount") != 843
            or counts.get("sourceArtifactCount") != 54
        ):
            raise TargetedTransactionError(
                f"pre-D/F material contract denominator drifted: {counts}"
            )
        contract_outputs = {
            material_contract.OUTPUT_PATH: material_contract.serialized(contract),
            material_contract.RECEIPT_PATH: material_contract.serialized(
                contract_receipt
            ),
        }
        materializer.commit_transaction(contract_outputs)
        material_contract.check(contract_outputs)
        if TRANSACTION_RECEIPT_PATH.is_file():
            TRANSACTION_RECEIPT_PATH.unlink()
        return {
            "manifestSha256": raw_sha256(manifest_path),
            "removedPartialFileCount": len(created_paths) + 1,
            "sourceMaterialContract": counts,
        }
    except Exception:
        restore_snapshots(snapshots)
        raise


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--rollback-partial", action="store_true")
    parser.add_argument("--asset-id", required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument(
        "--target",
        action="append",
        required=True,
        metavar="SKILL:STAGE:STAGE_CLIP:EFFECT_ID",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    config = find_config(args.asset_id)
    targets = [parse_target(value) for value in args.target]
    if args.rollback_partial:
        result = rollback_partial_df_transaction(config)
    else:
        result = (
        run_transaction(
            config=config,
            source_root=args.source_root.resolve(),
            targets=targets,
        )
        if args.write
        else check_transaction(
            config=config,
            source_root=args.source_root.resolve(),
            targets=targets,
        )
        )
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (TargetedTransactionError, materializer.RestorationError, material_contract.CorpusContractError, ValueError) as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
