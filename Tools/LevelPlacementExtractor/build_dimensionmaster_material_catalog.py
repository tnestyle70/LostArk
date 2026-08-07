#!/usr/bin/env python3
"""Build the canonical DimensionMaster UModel Material extraction catalog.

The resource manifest only carries unresolved packages.  This catalog instead
enumerates every non-builtin source Material/MI used by currently admitted
DimensionMaster Effects, so parent Material evidence can be captured before
materialization.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import dimensionmaster_admitted_skills


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def is_engine_builtin(source_material_path: str) -> bool:
    return source_material_path.casefold().startswith("enginematerials.")


def selected_admitted_skills(
    player_skills_path: Path,
    skill_bindings_path: Path,
    include_alt_v: bool,
) -> list[dict[str, Any]]:
    admitted = dimensionmaster_admitted_skills(
        player_skills_path, skill_bindings_path
    )
    if include_alt_v:
        return admitted
    return [
        row for row in admitted
        if str(row.get("inputSlot") or "").upper() != "ALT_V"
    ]


def build_catalog(
    imported_root: Path,
    player_skills_path: Path,
    skill_bindings_path: Path,
    include_alt_v: bool = False,
) -> dict[str, Any]:
    """Enumerate non-builtin source material identities for admitted skills."""
    admitted = selected_admitted_skills(
        player_skills_path, skill_bindings_path, include_alt_v
    )
    candidates: dict[tuple[str, str], dict[str, Any]] = {}
    excluded_builtins: list[dict[str, Any]] = []
    source_receipts = []
    for skill in admitted:
        skill_id = int(skill["skillId"])
        receipt_path = imported_root / f"skill.{skill_id}.source-receipt.json"
        receipt = read_json(receipt_path)
        if (
            receipt.get("schema") != "lostark.effect-source-receipt"
            or int(receipt.get("skillId", -1)) != skill_id
            or str(receipt.get("characterClass") or "").upper()
            != "DIMENSIONMASTER"
        ):
            raise ValueError(
                f"DimensionMaster source receipt identity is invalid: {receipt_path}"
            )
        source_receipts.append({
            "skillId": skill_id,
            "path": receipt_path.as_posix(),
            "sha256": sha256_file(receipt_path),
        })
        for binding in receipt.get("materialParameterBindings", []):
            material_path = str(binding.get("sourceMaterialPath") or "")
            if not material_path:
                raise ValueError(
                    f"skill {skill_id} has a Material binding without sourceMaterialPath"
                )
            record = {
                "skillId": skill_id,
                "inputSlot": str(skill.get("inputSlot") or ""),
                "sourceMaterialPath": material_path,
                "sourcePhysicalPackage": str(
                    binding.get("sourcePhysicalPackage") or ""
                ) or None,
                "parentMaterialPath": str(binding.get("parent") or "") or None,
            }
            if is_engine_builtin(material_path):
                excluded_builtins.append(record)
                continue
            key = (
                material_path.casefold(),
                str(binding.get("sourcePhysicalPackage") or "").casefold(),
            )
            existing = candidates.get(key)
            if existing is None:
                candidates[key] = {
                    "sourceMaterialPath": material_path,
                    "sourcePhysicalPackage": record["sourcePhysicalPackage"],
                    "parentMaterialPath": record["parentMaterialPath"],
                    "skillIds": [skill_id],
                    "inputSlots": [record["inputSlot"]],
                }
                continue
            if skill_id not in existing["skillIds"]:
                existing["skillIds"].append(skill_id)
            if record["inputSlot"] not in existing["inputSlots"]:
                existing["inputSlots"].append(record["inputSlot"])
            parent = record["parentMaterialPath"]
            if parent and existing.get("parentMaterialPath") and (
                str(existing["parentMaterialPath"]).casefold()
                != parent.casefold()
            ):
                raise ValueError(
                    "conflicting parent Material identity for source Material: "
                    f"{material_path}"
                )
            if parent:
                existing["parentMaterialPath"] = parent

    ordered = sorted(
        candidates.values(),
        key=lambda row: (
            str(row["sourceMaterialPath"]).casefold(),
            str(row.get("sourcePhysicalPackage") or "").casefold(),
        ),
    )
    for row in ordered:
        row["skillIds"].sort()
        row["inputSlots"].sort(key=str.casefold)
    return {
        "schema": "lostark.dimensionmaster-material-candidate-catalog",
        "formatVersion": 1,
        "characterClass": "DIMENSIONMASTER",
        "includeAltV": include_alt_v,
        "skills": [
            {
                "skillId": int(row["skillId"]),
                "inputSlot": str(row.get("inputSlot") or ""),
                "effectAssetId": str(row["effectAssetId"]),
            }
            for row in admitted
        ],
        # `extract_umodel_material_dependencies.py` intentionally consumes
        # this field.  These rows are candidates, not unresolved runtime data.
        "unresolvedMaterialBindings": ordered,
        "excludedEngineBuiltinMaterialBindings": sorted(
            excluded_builtins,
            key=lambda row: (
                int(row["skillId"]),
                str(row["sourceMaterialPath"]).casefold(),
            ),
        ),
        "summary": {
            "admittedSkillCount": len(admitted),
            "materialCandidateCount": len(ordered),
            "excludedEngineBuiltinMaterialBindingCount": len(excluded_builtins),
        },
        "sourceReceipts": source_receipts,
    }


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--imported-root", type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster"),
    )
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
    parser.add_argument(
        "--output", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/ActionSource/"
            "DimensionMaster.material-candidate-catalog.json"
        ),
    )
    parser.add_argument("--include-alt-v", action="store_true")
    args = parser.parse_args()
    catalog = build_catalog(
        args.imported_root, args.player_skills, args.skill_bindings,
        args.include_alt_v,
    )
    write_json_atomic(args.output, catalog)
    print(json.dumps(catalog["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
