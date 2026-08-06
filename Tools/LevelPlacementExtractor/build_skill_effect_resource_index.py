#!/usr/bin/env python3
"""Build deterministic per-slot Effect resource manifests without copying assets."""

from __future__ import annotations

import argparse
import json
import os
from collections import Counter
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


DEFAULT_SLOTS = ("Q", "W", "E", "R", "A", "S", "D", "F", "T", "V", "ALT_V")


def load_json(path: Path) -> dict[str, Any]:
    document = json.loads(path.read_text(encoding="utf-8-sig"))
    if not isinstance(document, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return document


def repository_path(path: Path, repository_root: Path) -> str:
    try:
        return path.resolve().relative_to(repository_root.resolve()).as_posix()
    except ValueError as error:
        raise ValueError(f"path is outside repository root: {path}") from error


def validate_asset_id(value: Any) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError("effect resource assetId must be a non-empty string")
    asset_id = value.replace("\\", "/")
    pure = PurePosixPath(asset_id)
    if pure.is_absolute() or ".." in pure.parts or ":" in asset_id:
        raise ValueError(f"effect resource assetId must be Resources-relative: {value}")
    return asset_id


def classify_asset(asset_id: str, slot_id: str, *, model_cue: bool = False) -> str:
    suffix = PurePosixPath(asset_id).suffix.casefold()
    folded_slot = slot_id.casefold()
    if model_cue:
        return "model"
    if suffix == ".wmodel" or "mesh" in folded_slot or "model" in folded_slot:
        return "mesh"
    if suffix in {".dds", ".png", ".tga", ".bmp"}:
        return "texture"
    if suffix in {".wav", ".wem", ".ogg", ".mp3"}:
        return "sound"
    return "resource"


def physical_status(resource_root: Path | None, asset_id: str) -> str:
    if resource_root is None:
        return "NOT_CHECKED"
    return "PRESENT" if (resource_root / Path(asset_id)).is_file() else "MISSING"


def collect_effect_contract(
    document: dict[str, Any], resource_root: Path | None
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    assets: dict[str, dict[str, Any]] = {}
    materials: dict[tuple[str, ...], dict[str, Any]] = {}

    def add_asset(
        asset_id_value: Any,
        slot_id: str,
        *,
        element_id: str | None = None,
        model_cue_id: str | None = None,
    ) -> None:
        asset_id = validate_asset_id(asset_id_value)
        key = asset_id.casefold()
        row = assets.setdefault(
            key,
            {
                "assetId": asset_id,
                "kinds": set(),
                "slotIds": set(),
                "elementIds": set(),
                "modelCueIds": set(),
                "occurrenceCount": 0,
            },
        )
        if row["assetId"] != asset_id:
            raise ValueError(f"case-colliding asset IDs: {row['assetId']} / {asset_id}")
        row["kinds"].add(
            classify_asset(asset_id, slot_id, model_cue=model_cue_id is not None)
        )
        row["slotIds"].add(slot_id)
        if element_id:
            row["elementIds"].add(element_id)
        if model_cue_id:
            row["modelCueIds"].add(model_cue_id)
        row["occurrenceCount"] += 1

    for cue in document.get("modelCues", []):
        if not isinstance(cue, dict):
            raise ValueError("modelCues must contain objects")
        model_asset_id = cue.get("modelAssetId")
        if model_asset_id:
            add_asset(
                model_asset_id,
                "modelCue",
                model_cue_id=str(cue.get("cueId", "")),
            )

    elements = document.get("elements", [])
    if not isinstance(elements, list):
        raise ValueError("effect elements must be an array")
    for element in elements:
        if not isinstance(element, dict):
            raise ValueError("effect elements must contain objects")
        element_id = str(element.get("id", ""))
        if not element_id:
            raise ValueError("effect element is missing stable id")
        resources = element.get("resources", [])
        if not isinstance(resources, list):
            raise ValueError(f"effect element resources must be an array: {element_id}")
        for resource in resources:
            if not isinstance(resource, dict):
                raise ValueError(f"effect resource must be an object: {element_id}")
            add_asset(
                resource.get("assetId"),
                str(resource.get("slotId") or resource.get("slot") or "resource"),
                element_id=element_id,
            )

        material = element.get("material")
        if not isinstance(material, dict):
            continue
        source_profile = material.get("sourceProfile")
        if not isinstance(source_profile, dict):
            source_profile = {}
        identity = (
            str(material.get("templateId", "")),
            str(material.get("sourceMaterialPath", "")),
            str(source_profile.get("profileId", "")),
            str(source_profile.get("parentMaterialPath", "")),
            str(source_profile.get("runtimeShaderProfileId", "")),
            str(source_profile.get("semanticStatus", "")),
        )
        if not any(identity):
            continue
        row = materials.setdefault(
            identity,
            {
                "templateId": identity[0] or None,
                "sourceMaterialPath": identity[1] or None,
                "profileId": identity[2] or None,
                "parentMaterialPath": identity[3] or None,
                "runtimeShaderProfileId": identity[4] or None,
                "semanticStatus": identity[5] or None,
                "elementIds": set(),
                "occurrenceCount": 0,
            },
        )
        row["elementIds"].add(element_id)
        row["occurrenceCount"] += 1

    serialized_assets: list[dict[str, Any]] = []
    for row in sorted(assets.values(), key=lambda item: item["assetId"].casefold()):
        serialized_assets.append(
            {
                "assetId": row["assetId"],
                "kinds": sorted(row["kinds"]),
                "slotIds": sorted(row["slotIds"], key=str.casefold),
                "elementIds": sorted(row["elementIds"], key=str.casefold),
                "modelCueIds": sorted(row["modelCueIds"], key=str.casefold),
                "occurrenceCount": row["occurrenceCount"],
                "physicalStatus": physical_status(resource_root, row["assetId"]),
            }
        )

    serialized_materials: list[dict[str, Any]] = []
    for row in sorted(materials.values(), key=lambda item: tuple(identity or "" for identity in (
        item["sourceMaterialPath"], item["profileId"], item["templateId"]
    ))):
        serialized_materials.append(
            {
                **{key: value for key, value in row.items() if key not in {"elementIds", "occurrenceCount"}},
                "elementIds": sorted(row["elementIds"], key=str.casefold),
                "occurrenceCount": row["occurrenceCount"],
            }
        )
    return serialized_assets, serialized_materials


def index_by_unique(
    rows: Iterable[dict[str, Any]], field: str, label: str
) -> dict[Any, dict[str, Any]]:
    result: dict[Any, dict[str, Any]] = {}
    for row in rows:
        key = row.get(field)
        if key in result:
            raise ValueError(f"duplicate {label}: {key}")
        result[key] = row
    return result


def build_resource_index(
    player_skills_path: Path,
    skill_bindings_path: Path,
    authored_root: Path,
    repository_root: Path,
    character_class: str,
    slots: Iterable[str] = DEFAULT_SLOTS,
    resource_root: Path | None = None,
) -> tuple[dict[str, Any], dict[str, dict[str, Any]]]:
    skills_document = load_json(player_skills_path)
    bindings_document = load_json(skill_bindings_path)
    if bindings_document.get("characterClass") != character_class:
        raise ValueError("skill binding characterClass does not match requested class")

    class_skills = [
        row for row in skills_document.get("skills", [])
        if isinstance(row, dict) and row.get("characterClass") == character_class
    ]
    skills_by_slot = index_by_unique(class_skills, "inputSlot", "inputSlot")
    binding_rows = bindings_document.get("bindings", [])
    if not isinstance(binding_rows, list):
        raise ValueError("animation skill bindings must be an array")
    bindings_by_skill = index_by_unique(binding_rows, "skillId", "animation skillId")

    requested_slots = tuple(str(slot).upper() for slot in slots)
    if len(requested_slots) != len(set(requested_slots)):
        raise ValueError("requested slots contain duplicates")

    source_paths = {
        "playerSkills": repository_path(player_skills_path, repository_root),
        "skillBindings": repository_path(skill_bindings_path, repository_root),
    }
    manifests: dict[str, dict[str, Any]] = {}
    index_rows: list[dict[str, Any]] = []
    status_counts: Counter[str] = Counter()
    all_assets: set[str] = set()
    all_materials: set[tuple[str, str, str]] = set()

    for slot in requested_slots:
        skill = skills_by_slot.get(slot)
        skill_id = int(skill["skillId"]) if skill is not None else None
        binding = bindings_by_skill.get(skill_id) if skill_id is not None else None
        effect_id = str(skill.get("effectId", "")).strip() if skill is not None else ""
        effect_path = authored_root / f"{effect_id}.effect.json" if effect_id else None
        effect_document: dict[str, Any] | None = None
        assets: list[dict[str, Any]] = []
        materials: list[dict[str, Any]] = []

        if skill is None:
            status = "MISSING_CANONICAL_SKILL"
        elif binding is None:
            status = "MISSING_ANIMATION_BINDING"
        elif not effect_id:
            status = "MISSING_EFFECT_BINDING"
        elif effect_path is None or not effect_path.is_file():
            status = "MISSING_EFFECT_DOCUMENT"
        else:
            effect_document = load_json(effect_path)
            document_effect_id = effect_document.get("effectAssetId") or effect_document.get(
                "effectId"
            )
            if document_effect_id != effect_id:
                raise ValueError(
                    f"effect document identity mismatch: {effect_path} / {effect_id}"
                )
            assets, materials = collect_effect_contract(effect_document, resource_root)
            status = (
                "INDEX_READY"
                if all(row["physicalStatus"] != "MISSING" for row in assets)
                else "MISSING_RUNTIME_RESOURCE"
            )

        status_counts[status] += 1
        for asset in assets:
            all_assets.add(asset["assetId"].casefold())
        for material in materials:
            all_materials.add(
                (
                    str(material.get("sourceMaterialPath") or "").casefold(),
                    str(material.get("profileId") or "").casefold(),
                    str(material.get("templateId") or "").casefold(),
                )
            )

        manifest_path = (
            f"Data/Effects/ResourceIndex/DimensionMaster/{slot}/"
            f"DimensionMaster.{slot}.effect-resources.json"
        )
        kind_counts = Counter(
            kind for asset in assets for kind in asset.get("kinds", [])
        )
        physical_counts = Counter(asset["physicalStatus"] for asset in assets)
        manifest = {
            "schema": "lostark.skill-effect-resource-index",
            "formatVersion": 1,
            "characterClass": character_class,
            "animationAssetId": bindings_document.get("animationAssetId"),
            "inputSlot": slot,
            "skillId": skill_id,
            "displayName": skill.get("displayName") if skill is not None else None,
            "actionId": skill.get("actionId") if skill is not None else None,
            "effectId": effect_id or None,
            "bindingStatus": status,
            "animationClips": list(binding.get("clips", [])) if binding is not None else [],
            "source": {
                **source_paths,
                "authoredEffect": (
                    repository_path(effect_path, repository_root)
                    if effect_path is not None and effect_path.is_file()
                    else None
                ),
            },
            "assets": assets,
            "materials": materials,
            "summary": {
                "assetCount": len(assets),
                "assetKindCounts": dict(sorted(kind_counts.items())),
                "materialIdentityCount": len(materials),
                "physicalStatusCounts": dict(sorted(physical_counts.items())),
                "effectElementCount": (
                    len(effect_document.get("elements", []))
                    if effect_document is not None
                    else 0
                ),
                "modelCueCount": (
                    len(effect_document.get("modelCues", []))
                    if effect_document is not None
                    else 0
                ),
            },
        }
        manifests[slot] = manifest
        index_rows.append(
            {
                "inputSlot": slot,
                "skillId": skill_id,
                "effectId": effect_id or None,
                "bindingStatus": status,
                "manifest": manifest_path,
            }
        )

    class_index = {
        "schema": "lostark.class-skill-effect-resource-index",
        "formatVersion": 1,
        "characterClass": character_class,
        "animationAssetId": bindings_document.get("animationAssetId"),
        "requestedSlots": list(requested_slots),
        "source": source_paths,
        "slots": index_rows,
        "summary": {
            "slotCount": len(index_rows),
            "statusCounts": dict(sorted(status_counts.items())),
            "uniqueAssetCount": len(all_assets),
            "uniqueMaterialIdentityCount": len(all_materials),
        },
    }
    return class_index, manifests


def serialize(document: dict[str, Any]) -> str:
    return json.dumps(document, ensure_ascii=False, indent=2) + "\n"


def write_if_changed(path: Path, contents: str) -> bool:
    if path.is_file() and path.read_text(encoding="utf-8-sig") == contents:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f"{path.name}.tmp")
    temporary.write_text(contents, encoding="utf-8")
    os.replace(temporary, path)
    return True


def write_resource_index(
    output_root: Path,
    class_index: dict[str, Any],
    manifests: dict[str, dict[str, Any]],
) -> int:
    changed = 0
    for slot, manifest in manifests.items():
        changed += write_if_changed(
            output_root / slot / f"DimensionMaster.{slot}.effect-resources.json",
            serialize(manifest),
        )
    changed += write_if_changed(
        output_root / "DimensionMaster.skill-effect-resource-index.json",
        serialize(class_index),
    )
    return changed


def parse_slots(value: str) -> tuple[str, ...]:
    return tuple(item.strip().upper() for item in value.split(",") if item.strip())


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--player-skills", required=True, type=Path)
    parser.add_argument("--skill-bindings", required=True, type=Path)
    parser.add_argument("--authored-root", required=True, type=Path)
    parser.add_argument("--repository-root", type=Path, default=Path.cwd())
    parser.add_argument("--resource-root", type=Path)
    parser.add_argument("--character-class", default="DIMENSIONMASTER")
    parser.add_argument("--slots", default=",".join(DEFAULT_SLOTS))
    parser.add_argument("--output-root", required=True, type=Path)
    parser.add_argument("--require-complete", action="store_true")
    args = parser.parse_args()

    class_index, manifests = build_resource_index(
        args.player_skills,
        args.skill_bindings,
        args.authored_root,
        args.repository_root,
        args.character_class,
        parse_slots(args.slots),
        args.resource_root,
    )
    changed = write_resource_index(args.output_root, class_index, manifests)
    report = {**class_index["summary"], "changedFileCount": changed}
    print(json.dumps(report, ensure_ascii=False, sort_keys=True))
    incomplete = sum(
        count
        for status, count in class_index["summary"]["statusCounts"].items()
        if status != "INDEX_READY"
    )
    return 1 if args.require_complete and incomplete else 0


if __name__ == "__main__":
    raise SystemExit(main())
