#!/usr/bin/env python3
"""Repair only DimensionMaster Mesh material ownership from TypeDataMesh.

The source boolean is authoritative: UE3 bOverrideMaterial=true means the
emitter Material must replace the WModel material.  This tool stages complete
documents, proves that no other semantic field changed, and optionally promotes
them only while the source hashes still match.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import dimensionmaster_admitted_skills
from build_imported_effect_documents import read_json, write_json_atomic
from materialize_dimensionmaster_base_effects import (
    restore_mesh_material_override_contract,
)


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def selected_documents(
    player_skills_path: Path,
    skill_bindings_path: Path,
    requested_skill_ids: set[int],
) -> list[tuple[int, str]]:
    admitted = [
        row for row in dimensionmaster_admitted_skills(
            player_skills_path, skill_bindings_path
        )
        if str(row.get("inputSlot") or "").upper() != "ALT_V"
    ]
    admitted_ids = {int(row["skillId"]) for row in admitted}
    if requested_skill_ids:
        unknown = requested_skill_ids - admitted_ids
        if unknown:
            raise ValueError(f"requested skills are not BASE11: {sorted(unknown)}")
        admitted = [
            row for row in admitted
            if int(row["skillId"]) in requested_skill_ids
        ]
    result: list[tuple[int, str]] = []
    for row in admitted:
        skill_id = int(row["skillId"])
        effect_id = str(row["effectAssetId"])
        result.append((skill_id, effect_id))
        if str(row.get("skillKind") or "").upper() == "COMBO":
            for index in range(len(row.get("comboStages", []))):
                result.append((skill_id, f"{effect_id}.ba{index + 1}"))
    return result


def mesh_material_values(document: dict[str, Any]) -> dict[str, bool]:
    result = {}
    for element in document.get("elements", []):
        if str(element.get("kind") or "").casefold() != "particle":
            continue
        if str(
            (element.get("sourceRecipe") or {}).get("rendererShape") or ""
        ).casefold() != "mesh":
            continue
        element_id = str(element.get("id") or "")
        if not element_id or element_id.casefold() in {
            key.casefold() for key in result
        }:
            raise ValueError("mesh Element ID is missing or duplicated")
        mesh = ((element.get("detail") or {}).get("mesh") or {})
        if "useModelMaterial" not in mesh:
            raise ValueError(f"mesh Element has no material choice: {element_id}")
        result[element_id] = bool(mesh["useModelMaterial"])
    return result


def prove_only_mesh_material_values_changed(
    before: dict[str, Any], after: dict[str, Any], changed_ids: list[str],
) -> None:
    restored = copy.deepcopy(after)
    before_values = mesh_material_values(before)
    by_id = {
        str(element.get("id") or ""): element
        for element in restored.get("elements", [])
    }
    for element_id in changed_ids:
        by_id[element_id]["detail"]["mesh"]["useModelMaterial"] = (
            before_values[element_id]
        )
    if restored != before:
        raise ValueError("Mesh material repair changed an unrelated field")


def stage_repairs(
    documents: list[tuple[int, str]],
    source_root: Path,
    staged_root: Path,
    receipt_path: Path,
) -> dict[str, Any]:
    if source_root.resolve() == staged_root.resolve():
        raise ValueError("staged root must differ from source root")
    rows = []
    total = 0
    for skill_id, effect_id in documents:
        source_path = source_root / f"{effect_id}.effect.json"
        before = read_json(source_path)
        if str(before.get("effectAssetId") or "") != effect_id:
            raise ValueError(f"Effect identity mismatch: {source_path}")
        after = copy.deepcopy(before)
        correction_count = restore_mesh_material_override_contract(after)
        before_values = mesh_material_values(before)
        after_values = mesh_material_values(after)
        changed_ids = sorted(
            element_id for element_id, value in before_values.items()
            if after_values[element_id] != value
        )
        if len(changed_ids) != correction_count:
            raise ValueError(f"correction count mismatch: {effect_id}")
        prove_only_mesh_material_values_changed(before, after, changed_ids)
        staged_path = staged_root / f"{effect_id}.effect.json"
        write_json_atomic(staged_path, after)
        reparsed = read_json(staged_path)
        prove_only_mesh_material_values_changed(before, reparsed, changed_ids)
        rows.append({
            "skillId": skill_id,
            "effectAssetId": effect_id,
            "sourcePath": source_path.as_posix(),
            "sourceSha256": sha256_file(source_path),
            "stagedPath": staged_path.as_posix(),
            "stagedSha256": sha256_file(staged_path),
            "correctionCount": correction_count,
            "changedElementIds": changed_ids,
        })
        total += correction_count
    receipt = {
        "schema": "lostark.dimensionmaster-mesh-material-contract-repair",
        "formatVersion": 1,
        "sourceContract": "USE_MODEL_MATERIAL_EQUALS_NOT_BOVERRIDEMATERIAL",
        "documentCount": len(rows),
        "correctionCount": total,
        "documents": rows,
    }
    write_json_atomic(receipt_path, receipt)
    return receipt


def promote_staged(receipt: dict[str, Any]) -> None:
    pending = []
    for row in receipt.get("documents", []):
        source = Path(str(row["sourcePath"]))
        staged = Path(str(row["stagedPath"]))
        if sha256_file(source) != str(row["sourceSha256"]):
            raise ValueError(f"stale source document: {source}")
        if sha256_file(staged) != str(row["stagedSha256"]):
            raise ValueError(f"staged document hash mismatch: {staged}")
        pending.append((source, staged, source.read_bytes()))

    replaced: list[tuple[Path, bytes]] = []
    try:
        for source, staged, backup in pending:
            if not int(next(
                row["correctionCount"] for row in receipt["documents"]
                if Path(str(row["sourcePath"])) == source
            )):
                continue
            if sha256_file(source) != sha256_bytes(backup):
                raise ValueError(f"source changed during promotion: {source}")
            temporary = source.with_suffix(source.suffix + ".mesh-contract.tmp")
            if temporary.exists():
                raise ValueError(f"promotion temporary already exists: {temporary}")
            temporary.write_bytes(staged.read_bytes())
            read_json(temporary)
            temporary.replace(source)
            replaced.append((source, backup))
    except Exception:
        for source, backup in reversed(replaced):
            rollback = source.with_suffix(source.suffix + ".mesh-contract.rollback")
            rollback.write_bytes(backup)
            rollback.replace(source)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--source-root", type=Path,
        default=Path("Data/Effects/Authored"),
    )
    parser.add_argument("--staged-root", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    parser.add_argument("--skill-id", type=int, action="append", default=[])
    parser.add_argument("--promote", action="store_true")
    parser.add_argument(
        "--player-skills", type=Path,
        default=Path("Data/Balance/PlayerSkills.json"),
    )
    parser.add_argument(
        "--skill-bindings", type=Path,
        default=Path(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.skillbindings.json"
        ),
    )
    args = parser.parse_args()
    receipt = stage_repairs(
        selected_documents(
            args.player_skills, args.skill_bindings, set(args.skill_id)
        ),
        args.source_root,
        args.staged_root,
        args.receipt,
    )
    if args.promote:
        promote_staged(receipt)
    print(json.dumps({
        "documentCount": receipt["documentCount"],
        "correctionCount": receipt["correctionCount"],
        "promoted": args.promote,
    }, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
