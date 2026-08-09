#!/usr/bin/env python3
"""Build current four-class combat Effect source intake artifacts.

This lane is intentionally upstream of Authored Effects.  It joins current
gameplay skills to Animation Tool bindings by exact clip identity, preserves
the action/combo stage shape, and either points at an exact existing Imported
artifact or materializes a new generated Source/Imported artifact set.  It
never borrows another skill's Effect and never invents a generic fallback.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


SCRIPT_ROOT = Path(__file__).resolve().parent
EXTRACTOR_ROOT = SCRIPT_ROOT.parent / "LevelPlacementExtractor"
if str(EXTRACTOR_ROOT) not in sys.path:
    sys.path.insert(0, str(EXTRACTOR_ROOT))

from build_imported_effect_documents import build_document  # noqa: E402
from build_skill_effect_source_receipt import (  # noqa: E402
    BLOCK_RE,
    collect_system_graph,
    compact_unresolved,
    find_particle_system,
    load_graphs,
    parse_animnotify,
    resolve_material_parameters,
    resolve_runtime_resource_bindings,
    source_asset_parts,
    source_package_receipts,
    required_material_sources,
)
from extract_ue3_particle_graph import extract_package  # noqa: E402
from extract_ue3_particle_module_closure import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    build_closure,
    obfuscate_package_name,
)


EXPECTED_SKILL_COUNTS = {
    "DimensionMaster": 12,
    "LanceMaster": 17,
    "Artist": 9,
    "Warlord": 13,
}

EXPECTED_STAGE_COUNTS = {
    "DimensionMaster": 15,
    "LanceMaster": 27,
    "Artist": 15,
    "Warlord": 17,
}

COMBAT_SLOTS = {
    "DIMENSIONMASTER": frozenset(
        {"LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V", "ALT_V"}
    ),
    "LANCE_MASTER": frozenset(
        {"LMB", "Q", "W", "E", "R", "A", "S", "T", "V", "ALT_V"}
    ),
    "ARTIST": frozenset(
        {"LMB", "Q", "W", "E", "R", "A", "S", "V", "ALT_V"}
    ),
    "WARLORD": frozenset(
        {"LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "X", "V", "ALT_V"}
    ),
}

ARTIST_31210_REVIEWED_RIBBON_ELEMENTS = {
    "fx_pc_sdm_01.par_t_sdm_skykongkong_weapon_01.particlespriteemitter_3.event_source-event-045": "e609656c7168de7f227e3e0ec6039d35d9587607d9078a7e368052af22edb6c0",
    "fx_pc_sdm_01.par_t_sdm_skykongkong_weapon_01.particlespriteemitter_3.event_source-event-050": "fc29e53d6049aa814115abc6cda72a89664cb0f654352a5454dcf94389e2f540",
    "fx_pc_sdm_01.par_t_sdm_skykongkong_weapon_04.particlespriteemitter_3.event_source-event-046": "1de04acc92097a334dff1d9ac151b08d17c0ae81ce794e9962f6202d623a2bc4",
    "fx_pc_sdm_01.par_t_sdm_skykongkong_weapon_04.particlespriteemitter_3.event_source-event-048": "f8fc9f4ef708d7f59c33f757f0d949d62673fe885071f86a0f97f8ec87be5933",
    "fx_pc_sdm_01.par_t_sdm_skykongkong_weapon_06.particlespriteemitter_2.event_source-event-047": "d7fe676e46016c86e0b85d5c555135c3f1bbbdad7eb2e315e5402525e427fa49",
    "fx_pc_sdm_01.par_t_sdm_skykongkong_weapon_06.particlespriteemitter_2.event_source-event-049": "d55d0350a28ff809e5d75a7452bf86586ee26cf35bf3482b20de2ca35c1067ec",
}

ARTIST_31210_REVIEWED_REASON_CODES = (
    "burst-cardinality-particle-dependent",
    "source-distribution-particle-dependent",
    "source-module-class-unsupported",
    "source-module-velocity-particle-dependent",
)

ARTIST_31210_REVIEWED_DOCUMENT_SHA256 = (
    "3f22d25f6550776d48d795ff78c77691a54a4a52c5154bb147b331a90d39cac9"
)


@dataclass(frozen=True)
class ClassConfig:
    asset_id: str
    character_class: str
    extraction_folder: str
    graph_folder: str
    runtime_effect_root: str
    resource_manifest_name: str | None = None
    runtime_cook_name: str | None = None
    resolved_action_catalog_name: str | None = None


CLASS_CONFIGS = (
    ClassConfig(
        "DimensionMaster",
        "DIMENSIONMASTER",
        "DIMENSIONMASTER/all_bound_skills",
        "particle_graphs",
        "Effect/DimensionMaster",
        "DimensionMaster.resource-source-manifest.json",
        "runtime-cook-receipt.json",
    ),
    ClassConfig(
        "LanceMaster",
        "LANCE_MASTER",
        "LANCEMASTER/all_bound_skills",
        "particle_graphs",
        "Effect/LanceMaster",
        "LanceMaster.resource-source-manifest.json",
        "runtime-cook-receipt.json",
    ),
    ClassConfig(
        "Artist",
        "ARTIST",
        "ARTIST/all_bound_skills",
        "particle_graphs",
        "Effect/Artist",
        "Artist.resource-source-manifest.json",
        "runtime-cook-receipt.json",
    ),
    ClassConfig(
        "Warlord",
        "WARLORD",
        "WARLORD/all_core_packages",
        "particle_graphs",
        "Effect/Warlord",
        None,
        "action-runtime-cook-receipt.json",
        "Warlord.action-particle-resource-catalog.json",
    ),
)


def reject_non_finite_json_constant(value: str) -> None:
    raise ValueError(f"non-finite JSON numeric constant is forbidden: {value}")


def normalize_json_for_serialization(value: Any) -> Any:
    if isinstance(value, float) and not math.isfinite(value):
        if math.isnan(value):
            return "NaN"
        return "Infinity" if value > 0.0 else "-Infinity"
    if isinstance(value, dict):
        return {
            key: normalize_json_for_serialization(child)
            for key, child in value.items()
        }
    if isinstance(value, (list, tuple)):
        return [normalize_json_for_serialization(child) for child in value]
    return value


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(
        path.read_text(encoding="utf-8-sig"),
        parse_constant=reject_non_finite_json_constant,
    )


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_json_sha256(value: Any) -> str:
    serialized = json.dumps(
        normalize_json_for_serialization(value),
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(serialized).hexdigest()


def artifact(path: Path, effect_asset_id: str | None = None) -> dict[str, Any]:
    row: dict[str, Any] = {
        "path": path.as_posix(),
        "sha256": sha256_file(path),
    }
    if effect_asset_id is not None:
        row["effectAssetId"] = effect_asset_id
    return row


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(
            normalize_json_for_serialization(value),
            ensure_ascii=False,
            indent=2,
            allow_nan=False,
        )
        + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def clip_name(value: Any, skill_id: int) -> str:
    if isinstance(value, str):
        result = value
    elif isinstance(value, dict):
        result = str(value.get("clip") or "")
    else:
        result = ""
    if not result:
        raise ValueError(f"skill {skill_id} has an invalid clip entry")
    return result


def flatten_stage(value: Any, skill_id: int) -> list[str]:
    if not isinstance(value, list) or not value:
        raise ValueError(f"skill {skill_id} has an invalid stage")
    result: list[str] = []
    for item in value:
        if isinstance(item, list):
            result.extend(flatten_stage(item, skill_id))
        else:
            result.append(clip_name(item, skill_id))
    return result


def binding_stages(values: Any, skill_id: int) -> list[list[str]]:
    """Preserve product action grouping without treating flat clips as stages."""
    if not isinstance(values, list) or not values:
        raise ValueError(f"skill {skill_id} has an invalid clips array")
    nested = [isinstance(value, list) for value in values]
    if any(nested) and not all(nested):
        raise ValueError(f"skill {skill_id} mixes flat clips and combo stages")
    if all(nested):
        return [flatten_stage(value, skill_id) for value in values]
    return [[clip_name(value, skill_id) for value in values]]


def parse_exact_clip_catalog(path: Path) -> dict[str, dict[str, Any]]:
    """Parse animnotify and reject duplicate exact clip identities."""
    seen: dict[str, int] = {}
    for line_number, line in enumerate(
        path.read_text(encoding="utf-8-sig").splitlines(), start=1
    ):
        match = BLOCK_RE.match(line)
        if match is None:
            continue
        exact = match.group("clip")
        if exact in seen:
            raise ValueError(
                f"duplicate animnotify clip identity {exact}: "
                f"lines {seen[exact]} and {line_number}"
            )
        seen[exact] = line_number
    catalog = parse_animnotify(path)
    if set(catalog) != set(seen):
        raise ValueError("animnotify clip parser identity mismatch")
    return catalog


def admitted_skills(
    player_skills: dict[str, Any], character_class: str
) -> list[dict[str, Any]]:
    slots = COMBAT_SLOTS[character_class]
    result = []
    identities: set[int] = set()
    for row in player_skills.get("skills", []):
        if str(row.get("characterClass") or "") != character_class:
            continue
        if str(row.get("inputSlot") or "") not in slots:
            continue
        if str(row.get("setsStance") or "") != "NONE":
            continue
        if str(row.get("skillKind") or "") not in {
            "ACTIVE",
            "COMBO",
            "COUNTER",
            "HOLD",
        }:
            continue
        skill_id = int(row.get("skillId", -1))
        if skill_id < 0 or skill_id in identities:
            raise ValueError(f"invalid or duplicate product skill {skill_id}")
        identities.add(skill_id)
        result.append(row)
    return result


def build_class_stage_contract(
    config: ClassConfig,
    player_skills: dict[str, Any],
    bindings_path: Path,
    animnotify_path: Path,
    player_skills_path: Path | None = None,
    enforce_expected_counts: bool = True,
) -> dict[str, Any]:
    bindings = read_json(bindings_path)
    if str(bindings.get("animationAssetId") or "") != config.asset_id:
        raise ValueError(f"{config.asset_id} animation asset mismatch")
    if str(bindings.get("characterClass") or "") != config.character_class:
        raise ValueError(f"{config.asset_id} characterClass mismatch")

    product_rows = admitted_skills(player_skills, config.character_class)
    product_by_id = {int(row["skillId"]): row for row in product_rows}
    binding_rows: dict[int, dict[str, Any]] = {}
    for row in bindings.get("bindings", []):
        skill_id = int(row.get("skillId", -1))
        if skill_id in binding_rows:
            raise ValueError(f"duplicate skillbinding {config.asset_id}/{skill_id}")
        binding_rows[skill_id] = row

    catalog = parse_exact_clip_catalog(animnotify_path)
    clip_owners: dict[str, int] = {}
    skills: list[dict[str, Any]] = []
    stage_count = 0
    clip_occurrence_count = 0
    numeric_alias_count = 0

    for product in product_rows:
        product_skill_id = int(product["skillId"])
        binding = binding_rows.get(product_skill_id)
        if binding is None:
            raise ValueError(
                f"product skill has no binding: {config.asset_id}/{product_skill_id}"
            )
        grouped = binding_stages(binding.get("clips"), product_skill_id)
        stages = []
        timeline_offset = 0.0
        sequence_index = 0
        skill_source_ids: set[int] = set()
        for stage_index, names in enumerate(grouped):
            stage_clips = []
            stage_source_ids: set[int] = set()
            stage_start = timeline_offset
            for stage_clip_index, exact_clip in enumerate(names):
                source = catalog.get(exact_clip)
                if source is None:
                    raise ValueError(
                        f"bound clip is absent from animnotify: "
                        f"{config.asset_id}/{product_skill_id}/{exact_clip}"
                    )
                previous_owner = clip_owners.setdefault(exact_clip, product_skill_id)
                if previous_owner != product_skill_id:
                    raise ValueError(
                        f"clip {exact_clip} has duplicate product owners "
                        f"{previous_owner} and {product_skill_id}"
                    )
                source_skill_id = int(source["skillId"])
                stage_source_ids.add(source_skill_id)
                skill_source_ids.add(source_skill_id)
                if source_skill_id != product_skill_id:
                    numeric_alias_count += 1
                length = float(source["lengthSeconds"])
                stage_clips.append(
                    {
                        "clip": exact_clip,
                        "stageClipIndex": stage_clip_index,
                        "sequenceIndex": sequence_index,
                        "sourceSkillId": source_skill_id,
                        "sourceLine": int(source["sourceLine"]),
                        "timelineOffsetSeconds": round(timeline_offset, 7),
                        "lengthSeconds": length,
                    }
                )
                timeline_offset += length
                sequence_index += 1
                clip_occurrence_count += 1
            if len(stage_source_ids) != 1:
                raise ValueError(
                    f"stage has contradictory source ownership: "
                    f"{config.asset_id}/{product_skill_id}/{stage_index} -> "
                    f"{sorted(stage_source_ids)}"
                )
            stages.append(
                {
                    "stageId": f"skill.{product_skill_id}.stage.{stage_index}",
                    "stageIndex": stage_index,
                    "sourceSkillIds": sorted(stage_source_ids),
                    "timelineOffsetSeconds": round(stage_start, 7),
                    "durationSeconds": round(timeline_offset - stage_start, 7),
                    "clips": stage_clips,
                    "sourceEventIds": [],
                    "sourceArtifacts": [],
                    "status": "SOURCE_ARTIFACT_PENDING",
                    "blockers": [],
                }
            )
            stage_count += 1

        skills.append(
            {
                "productSkillId": product_skill_id,
                "inputSlot": str(product.get("inputSlot") or ""),
                "skillKind": str(product.get("skillKind") or ""),
                "sourceSkillIds": sorted(skill_source_ids),
                "stages": stages,
                "status": "SOURCE_ARTIFACT_PENDING",
                "blockers": [],
            }
        )

    missing_bindings = sorted(set(product_by_id) - set(binding_rows))
    if missing_bindings:
        raise ValueError(f"missing product bindings: {missing_bindings}")
    expected_skills = EXPECTED_SKILL_COUNTS.get(config.asset_id, len(skills))
    expected_stages = EXPECTED_STAGE_COUNTS.get(config.asset_id, stage_count)
    if enforce_expected_counts and (
        len(skills) != expected_skills or stage_count != expected_stages
    ):
        raise ValueError(
            f"{config.asset_id} coverage changed: "
            f"skills={len(skills)}/{expected_skills}, "
            f"stages={stage_count}/{expected_stages}"
        )

    return {
        "schema": "lostark.combat-effect-source-stage-manifest",
        "version": 1,
        "animationAssetId": config.asset_id,
        "characterClass": config.character_class,
        "consumerContract": {
            "joinKey": "exact bound clip identity",
            "numericSkillAliasIsJoinKey": False,
            "outerCueRoot": "Player",
            "outerCueFollow": True,
            "innerSnapshotFollow": False,
        },
        "source": {
            "playerSkills": (player_skills_path or Path("Data/Balance/PlayerSkills.json")).as_posix(),
            "playerSkillsSha256": sha256_file(
                player_skills_path or Path("Data/Balance/PlayerSkills.json")
            ),
            "skillBindings": bindings_path.as_posix(),
            "skillBindingsSha256": sha256_file(bindings_path),
            "animationNotify": animnotify_path.as_posix(),
            "animationNotifySha256": sha256_file(animnotify_path),
        },
        "skills": skills,
        "summary": {
            "skillCount": len(skills),
            "stageCount": stage_count,
            "clipOccurrenceCount": clip_occurrence_count,
            "numericAliasClipOccurrenceCount": numeric_alias_count,
            "readySkillCount": 0,
            "readyStageCount": 0,
            "blockedSkillCount": 0,
            "blockedStageCount": 0,
        },
    }


def exact_timeline(
    skill: dict[str, Any], catalog: dict[str, dict[str, Any]]
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], float]:
    clips: list[dict[str, Any]] = []
    events: list[dict[str, Any]] = []
    event_index = 0
    offset = 0.0
    for stage in skill["stages"]:
        for clip_row in stage["clips"]:
            exact = str(clip_row["clip"])
            source = catalog[exact]
            source_skill_id = int(source["skillId"])
            sequence_index = int(clip_row["sequenceIndex"])
            clips.append(
                {
                    "sequenceIndex": sequence_index,
                    "clip": exact,
                    "sourceSkillId": source_skill_id,
                    "offsetSeconds": round(offset, 7),
                    "lengthSeconds": float(source["lengthSeconds"]),
                    "sourceLine": int(source["sourceLine"]),
                }
            )
            for notify in source["notifies"]:
                if notify["kind"] not in {"EFFECT", "SHAKE"}:
                    continue
                event_index += 1
                events.append(
                    {
                        "eventId": f"source-event-{event_index:03d}",
                        "clip": exact,
                        "clipSequenceIndex": sequence_index,
                        "sourceSkillId": source_skill_id,
                        "localTimeSeconds": notify["localTimeSeconds"],
                        "globalTimeSeconds": round(
                            offset + float(notify["localTimeSeconds"]), 7
                        ),
                        "durationSeconds": notify["durationSeconds"],
                        "kind": notify["kind"],
                        "sourceType": notify["sourceType"],
                        "sourceAsset": notify["sourceAsset"],
                        "sourceLine": notify["sourceLine"],
                    }
                )
            offset += float(source["lengthSeconds"])
    return clips, events, round(offset, 7)


def graph_registry(graph_root: Path, supplemental_root: Path) -> dict[str, Path]:
    result: dict[str, Path] = {}
    for root in (graph_root, supplemental_root):
        if not root.is_dir():
            continue
        for path in root.glob("*.particle-graph.json"):
            logical = path.name.removesuffix(".particle-graph.json").casefold()
            if logical in result and result[logical] != path:
                raise ValueError(f"duplicate particle graph package {logical}")
            result[logical] = path
    return result


def extract_missing_graphs(
    logical_packages: Iterable[str],
    graph_paths: dict[str, Path],
    supplemental_root: Path,
    package_root: Path,
) -> list[dict[str, Any]]:
    """Extract only exact missing logical packages into the external source pack."""
    receipts = []
    for logical in sorted(set(logical_packages), key=str.casefold):
        key = logical.casefold()
        if key in graph_paths:
            continue
        physical = package_root / f"{obfuscate_package_name(logical)}.upk"
        if not physical.is_file():
            receipts.append(
                {
                    "logicalPackage": logical,
                    "status": "SOURCE_PACKAGE_MISSING",
                    "expectedPhysicalPackage": physical.name,
                }
            )
            continue
        result = extract_package(physical, logical.upper(), LOSTARK_KR_AES_KEY)
        output = supplemental_root / f"{logical.upper()}.particle-graph.json"
        write_json_atomic(output, result)
        graph_paths[key] = output
        summary = result.get("summary", {})
        receipts.append(
            {
                "logicalPackage": logical,
                "status": (
                    "EXTRACTED"
                    if not summary.get("propertyErrorCount")
                    and not summary.get("missingParticleEmitterTargetCount")
                    and not summary.get("invalidPointLightComponentTargetCount")
                    else "EXTRACTED_WITH_GRAPH_ERRORS"
                ),
                "physicalPackage": physical.name,
                "graph": artifact(output),
                "summary": summary,
            }
        )
    return receipts


def supplemental_graph_provenance(
    logical_packages: Iterable[str],
    graph_paths: dict[str, Path],
    supplemental_root: Path,
    extraction_receipts: Iterable[dict[str, Any]],
) -> list[dict[str, Any]]:
    """Keep exact supplemental graph provenance on every deterministic run."""
    rows = {
        str(row.get("logicalPackage") or "").casefold(): row
        for row in extraction_receipts
        if row.get("logicalPackage")
    }
    supplemental_resolved = supplemental_root.resolve()
    for logical in sorted(set(logical_packages), key=str.casefold):
        key = logical.casefold()
        if key in rows:
            continue
        path = graph_paths.get(key)
        if path is None or path.parent.resolve() != supplemental_resolved:
            continue
        graph = read_json(path)
        rows[key] = {
            "logicalPackage": logical,
            "status": "EXISTING_SUPPLEMENTAL_GRAPH",
            "physicalPackage": str(graph.get("physicalPackage") or ""),
            "graph": artifact(path),
            "summary": graph.get("summary", {}),
        }
    return [rows[key] for key in sorted(rows)]


def source_package_map(resource_manifest_path: Path | None) -> dict[str, str]:
    if resource_manifest_path is None or not resource_manifest_path.is_file():
        return {}
    document = read_json(resource_manifest_path)
    candidates: dict[str, set[str]] = {}
    for row in document.get("assets", []):
        logical = str(row.get("logicalPackage") or "")
        physical = str(row.get("physicalPackage") or "")
        if logical and physical:
            candidates.setdefault(logical.casefold(), set()).add(physical)
    return {
        logical: next(iter(values))
        for logical, values in candidates.items()
        if len(values) == 1
    }


def select_action_catalog_material_bindings(
    systems: list[dict[str, Any]],
    catalog: dict[str, Any],
    character_class: str,
    product_skill_id: int,
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    """Select exact action/system-owned rows without upgrading their fidelity."""
    if catalog.get("schema") != "lostark.unbound-class-particle-resource-catalog":
        raise ValueError("resolved action catalog schema is invalid")
    if int(catalog.get("formatVersion", -1)) != 1:
        raise ValueError("resolved action catalog version is invalid")
    if str(catalog.get("characterClass") or "") != character_class:
        raise ValueError("resolved action catalog class mismatch")
    if catalog.get("bindingStatus") != "ACTION_NOTIFY_BOUND":
        raise ValueError("resolved action catalog is not action-notify bound")

    material_index = {
        str(row.get("sourceMaterialPath") or "").casefold(): row
        for row in catalog.get("materialParameterBindings", [])
        if row.get("sourceMaterialPath")
    }
    asset_index = {
        str(row.get("sourceAssetPath") or "").casefold(): row
        for row in catalog.get("assets", [])
        if row.get("sourceAssetPath")
    }
    selected = []
    status_counts: Counter[str] = Counter()
    for key, requirement in sorted(required_material_sources(systems).items()):
        source_path = str(requirement["sourceMaterialPath"])
        material = material_index.get(key)
        asset = asset_index.get(key)
        if material is None or asset is None:
            selected.append(
                {
                    "sourceMaterialPath": source_path,
                    "sourceLogicalPackage": source_path.split(".", 1)[0],
                    "resolutionStatus": "MISSING_ACTION_CATALOG_MATERIAL",
                    "candidateCount": 0,
                }
            )
            status_counts["MISSING_ACTION_CATALOG_MATERIAL"] += 1
            continue
        action_ids = asset.get("actionIds", [])
        if product_skill_id not in action_ids:
            raise ValueError(
                f"action catalog material is not owned by product skill "
                f"{product_skill_id}: {source_path}"
            )
        catalog_systems = {
            str(value).casefold() for value in asset.get("sourceSystems", [])
        }
        if not requirement["sourceSystemIds"].issubset(catalog_systems):
            raise ValueError(
                f"action catalog material lacks selected source system ownership: "
                f"{source_path}"
            )
        selected.append(material)
        status_counts[str(material.get("resolutionStatus") or "MISSING_STATUS")] += 1
    provenance = {
        "schema": catalog["schema"],
        "formatVersion": catalog["formatVersion"],
        "characterClass": catalog["characterClass"],
        "bindingStatus": catalog["bindingStatus"],
        "productSkillId": product_skill_id,
        "selectionPolicy": "EXACT_ACTION_AND_SOURCE_SYSTEM_OWNERSHIP",
        "resolutionStatusesPreserved": True,
        "statusCounts": dict(sorted(status_counts.items())),
    }
    return selected, provenance


def materialize_source_graph(
    config: ClassConfig,
    skill: dict[str, Any],
    binding_clips: Any,
    catalog: dict[str, dict[str, Any]],
    graph_paths: dict[str, Path],
    graph_output: Path,
    receipt_output: Path,
    material_catalog: dict[str, list[dict[str, Any]]] | None,
    package_mapping: dict[str, str] | None,
    resolved_action_catalog: dict[str, Any] | None,
    runtime_resources_root: Path,
    runtime_cook_receipt: Path | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    product_skill_id = int(skill["productSkillId"])
    clips, events, duration = exact_timeline(skill, catalog)
    required_packages = sorted(
        {
            str(event["sourceAsset"]).split(".", 1)[0].casefold()
            for event in events
            if event["sourceType"] == "PlayParticleEffect"
            and event["sourceAsset"]
        }
    )
    available_specs = [
        (package, graph_paths[package])
        for package in required_packages
        if package in graph_paths
    ]
    graph_index = load_graphs(available_specs) if available_specs else {
        "packages": {},
        "nodes": {},
    }

    systems_by_asset: dict[str, dict[str, Any]] = {}
    for event in events:
        if event["kind"] == "SHAKE" or event["sourceType"] == "Effect":
            event["resolutionStatus"] = "OUT_OF_EFFECT_DOCUMENT"
            continue
        if event["sourceType"] != "PlayParticleEffect" or not event["sourceAsset"]:
            event["resolutionStatus"] = "UNSUPPORTED_SOURCE_NOTIFY"
            continue
        parts = source_asset_parts(str(event["sourceAsset"]))
        if parts is None:
            event["resolutionStatus"] = "UNRESOLVED_SOURCE_SYSTEM"
            continue
        package, object_path, object_name = parts
        root = find_particle_system(graph_index, package, object_path, object_name)
        if root is None:
            event["resolutionStatus"] = "UNRESOLVED_SOURCE_SYSTEM"
            continue
        event["resolutionStatus"] = "RESOLVED_PARTICLE_GRAPH"
        event["sourceSystemId"] = str(event["sourceAsset"]).casefold()
        if event["sourceSystemId"] not in systems_by_asset:
            package_key, root_row = root
            systems_by_asset[event["sourceSystemId"]] = {
                "sourceSystemId": event["sourceSystemId"],
                "sourceAsset": event["sourceAsset"],
                "logicalPackage": graph_index["packages"][package_key]["package"],
                "objectName": root_row["objectName"],
                "objectPath": root_row.get("objectPath"),
                "graph": collect_system_graph(graph_index, package_key, root_row),
            }

    systems = sorted(
        systems_by_asset.values(), key=lambda row: str(row["sourceAsset"]).casefold()
    )
    detailed_nodes: dict[str, dict[str, Any]] = {}
    detailed_edges: list[dict[str, Any]] = []
    detailed_systems = []
    for system in systems:
        graph = system["graph"]
        detailed_nodes.update(graph["nodes"])
        detailed_edges.extend(
            {**edge, "sourceSystemId": system["sourceSystemId"]}
            for edge in graph["edges"]
        )
        detailed_systems.append(
            {
                **{key: value for key, value in system.items() if key != "graph"},
                "rootNodeId": graph["rootNodeId"],
                "nodeIds": graph["nodeIds"],
                "resourceBindings": graph["resourceBindings"],
                "unresolvedExternalReferences": graph[
                    "unresolvedExternalReferences"
                ],
                "summary": graph["summary"],
            }
        )

    if resolved_action_catalog is not None:
        material_bindings, material_catalog_evidence = select_action_catalog_material_bindings(
            systems,
            resolved_action_catalog,
            config.character_class,
            product_skill_id,
        )
    else:
        material_paths = sorted(
            {
                str(binding["objectPath"])
                for system in systems
                for binding in system["graph"]["resourceBindings"]
                if binding.get("role") == "material"
                and binding.get("objectPath")
            },
            key=str.casefold,
        )
        material_bindings = [
            resolve_material_parameters(
                path,
                material_catalog or {},
                package_mapping or {},
            )
            for path in material_paths
        ]
        material_catalog_evidence = None
    runtime_resources = resolve_runtime_resource_bindings(
        systems,
        material_bindings,
        runtime_resources_root,
        config.runtime_effect_root,
        runtime_cook_receipt,
    )
    resolved_material_count = sum(
        str(row.get("resolutionStatus") or "").startswith("RESOLVED_")
        for row in material_bindings
    )
    resolved_runtime_count = sum(
        row.get("resolutionStatus") == "RESOLVED_RUNTIME_ASSET"
        for row in runtime_resources
    )
    graph_artifact = {
        "schema": "lostark.normalized-effect-source-graph",
        "schemaVersion": 1,
        "characterClass": config.character_class,
        "skillId": product_skill_id,
        "productSkillId": product_skill_id,
        "sourceSkillIds": skill["sourceSkillIds"],
        "clipJoinPolicy": "EXACT_BOUND_CLIP_IDENTITY",
        "sourceSystems": detailed_systems,
        "nodes": [detailed_nodes[key] for key in sorted(detailed_nodes)],
        "edges": detailed_edges,
        "materialParameterBindings": material_bindings,
        "runtimeResourceBindings": runtime_resources,
        "summary": {
            "sourceSystemCount": len(systems),
            "uniqueNodeCount": len(detailed_nodes),
            "edgeOccurrenceCount": len(detailed_edges),
            "resourceBindingOccurrenceCount": sum(
                len(system["resourceBindings"]) for system in detailed_systems
            ),
            "unresolvedExternalReferenceOccurrenceCount": sum(
                len(system["unresolvedExternalReferences"])
                for system in detailed_systems
            ),
            "uniqueMaterialBindingCount": len(material_bindings),
            "resolvedMaterialBindingCount": resolved_material_count,
            "runtimeResourceBindingCount": len(runtime_resources),
            "resolvedRuntimeResourceBindingCount": resolved_runtime_count,
            "missingParticleGraphPackageCount": len(
                set(required_packages) - set(graph_paths)
            ),
        },
    }
    if material_catalog_evidence is not None:
        graph_artifact["materialBindingCatalogEvidence"] = material_catalog_evidence

    compact_systems = [
        {
            **{key: value for key, value in system.items() if key != "graph"},
            "rootNodeId": system["graph"]["rootNodeId"],
            "summary": system["graph"]["summary"],
            "resourceBindings": system["graph"]["resourceBindings"],
        }
        for system in systems
    ]
    unsupported = compact_unresolved(systems, events)
    receipt = {
        "schema": "lostark.effect-source-receipt",
        "schemaVersion": 2,
        "characterClass": config.character_class,
        "inputSlot": skill["inputSlot"],
        "skillId": product_skill_id,
        "productSkillId": product_skill_id,
        "sourceSkillIds": skill["sourceSkillIds"],
        "animationAssetId": config.asset_id,
        "clipOwnershipProvenance": {
            "joinKey": "exact bound clip identity",
            "numericSkillAliasIsJoinKey": False,
            "clips": [
                {
                    "clip": row["clip"],
                    "sequenceIndex": row["sequenceIndex"],
                    "sourceSkillId": row["sourceSkillId"],
                    "sourceLine": row["sourceLine"],
                }
                for row in clips
            ],
        },
        "animationVisualContract": {
            "combinedMeshAnimationStatus": "ORIGINAL_MATCH_CONFIRMED",
            "combinedMeshAnimationOwner": "Animation/CModel",
            "reconstructCombinedMeshAsEffectElement": False,
            "effectExtractionScope": [
                "particle",
                "texture_or_sprite",
                "trail",
                "distortion",
                "post_effect",
                "particle_owned_mesh_emitter",
            ],
        },
        "timeline": {
            "durationSeconds": duration,
            "bindingClips": binding_clips,
            "clips": clips,
            "events": events,
        },
        "sourcePackages": source_package_receipts(graph_index),
        "sourceSystems": compact_systems,
        "materialParameterBindings": material_bindings,
        "runtimeResourceBindings": runtime_resources,
        "unsupportedUnresolved": unsupported,
        "conversionState": {
            "normalizedSourceGraph": "EXTRACTED",
            "materialInstanceParameterBindings": (
                "EXTRACTED"
                if material_bindings
                and resolved_material_count == len(material_bindings)
                else "PARTIAL"
                if material_bindings
                else "PENDING"
            ),
            "runtimeElementMapping": "PENDING",
            "importedEffectDocument": "PENDING",
        },
        "summary": {
            "sourceSystemCount": len(systems),
            "effectEventCount": sum(event["kind"] == "EFFECT" for event in events),
            "resolvedParticleEventCount": sum(
                event.get("resolutionStatus") == "RESOLVED_PARTICLE_GRAPH"
                for event in events
            ),
            "unsupportedNotifyCount": sum(
                event.get("resolutionStatus") == "UNSUPPORTED_SOURCE_NOTIFY"
                for event in events
            ),
            "unresolvedSourceSystemCount": sum(
                event.get("resolutionStatus") == "UNRESOLVED_SOURCE_SYSTEM"
                for event in events
            ),
            "relatedViewShakeCount": sum(event["kind"] == "SHAKE" for event in events),
            "unsupportedUnresolvedUniqueCount": len(unsupported),
            "uniqueMaterialBindingCount": len(material_bindings),
            "resolvedMaterialBindingCount": resolved_material_count,
            "runtimeResourceBindingCount": len(runtime_resources),
            "resolvedRuntimeResourceBindingCount": resolved_runtime_count,
            "missingParticleGraphPackageCount": len(
                set(required_packages) - set(graph_paths)
            ),
        },
    }
    if material_catalog_evidence is not None:
        receipt["materialBindingCatalogEvidence"] = material_catalog_evidence
    write_json_atomic(graph_output, graph_artifact)
    write_json_atomic(receipt_output, receipt)
    return receipt, graph_artifact


def receipt_clip_names(receipt: dict[str, Any]) -> list[str]:
    return [
        str(row.get("clip") or "")
        for row in receipt.get("timeline", {}).get("clips", [])
    ]


def flattened_skill_clips(skill: dict[str, Any]) -> list[str]:
    return [
        str(clip["clip"])
        for stage in skill["stages"]
        for clip in stage["clips"]
    ]


def existing_artifact_set(
    config: ClassConfig,
    skill: dict[str, Any],
    imported_root: Path,
    resource_manifest: dict[str, Any] | None,
) -> dict[str, Any] | None:
    skill_id = int(skill["productSkillId"])
    class_root = imported_root / config.asset_id
    current_root = class_root / "CurrentCombat"
    candidates = [
        {
            "receipt": class_root / f"skill.{skill_id}.source-receipt.json",
            "graph": class_root / "Modules" / f"skill.{skill_id}.normalized-effect-graph.json",
            "closure": class_root / "Modules" / f"skill.{skill_id}.external-module-closure.json",
            "document": class_root / "Converted" / f"effect.{config.asset_id.casefold()}.skill.{skill_id}.imported.effect.json",
            "conversion": class_root / "Converted" / f"skill.{skill_id}.element-conversion-receipt.json",
            "origin": "EXISTING_GENERATED_SOURCE_TRUTH",
        },
        {
            "receipt": current_root / f"skill.{skill_id}.source-receipt.json",
            "graph": current_root / "Graphs" / f"skill.{skill_id}.normalized-effect-graph.json",
            "closure": current_root / "Modules" / f"skill.{skill_id}.external-module-closure.json",
            "document": current_root / "Converted" / f"effect.{config.asset_id.casefold()}.skill.{skill_id}.imported.effect.json",
            "conversion": current_root / "Converted" / f"skill.{skill_id}.element-conversion-receipt.json",
            "origin": "CURRENT_COMBAT_GENERATED_SOURCE_TRUTH",
        },
    ]
    if resource_manifest is not None:
        matches = [
            Path(value)
            for value in resource_manifest.get("sourceGraphs", [])
            if Path(value).parent.name == f"skill_{skill_id}"
        ]
        if len(matches) == 1:
            candidates[0]["graph"] = matches[0]

    for candidate in candidates:
        paths = tuple(
            candidate[key]
            for key in ("receipt", "graph", "closure", "document", "conversion")
        )
        if not all(path.is_file() for path in paths):
            continue
        receipt = read_json(candidate["receipt"])
        if receipt_clip_names(receipt) != flattened_skill_clips(skill):
            continue
        closure = read_json(candidate["closure"])
        if int(closure.get("summary", {}).get("unresolvedRequestCount", -1)) != 0:
            continue
        if int(closure.get("summary", {}).get("propertyErrorCount", -1)) != 0:
            continue
        document = read_json(candidate["document"])
        return {
            "sourceReceipt": artifact(candidate["receipt"]),
            "normalizedGraph": artifact(candidate["graph"]),
            "externalModuleClosure": artifact(candidate["closure"]),
            "importedDocument": artifact(
                candidate["document"], str(document.get("effectAssetId") or "")
            ),
            "conversionReceipt": artifact(candidate["conversion"]),
            "artifactOrigin": candidate["origin"],
        }
    return None


def collect_artifact_blockers(source_artifacts: dict[str, Any]) -> list[dict[str, Any]]:
    receipt = read_json(Path(source_artifacts["sourceReceipt"]["path"]))
    graph = read_json(Path(source_artifacts["normalizedGraph"]["path"]))
    closure = read_json(Path(source_artifacts["externalModuleClosure"]["path"]))
    conversion = read_json(Path(source_artifacts["conversionReceipt"]["path"]))
    blockers: list[dict[str, Any]] = []
    receipt_summary = receipt.get("summary", {})
    for field, code in (
        ("unresolvedSourceSystemCount", "UNRESOLVED_SOURCE_SYSTEM"),
        ("missingParticleGraphPackageCount", "MISSING_PARTICLE_GRAPH_PACKAGE"),
    ):
        count = int(receipt_summary.get(field, 0))
        if count:
            blockers.append({"code": code, "count": count})

    material_rows = graph.get("materialParameterBindings", [])
    unresolved_materials = [
        row
        for row in material_rows
        if not str(row.get("resolutionStatus") or "").startswith("RESOLVED_")
    ]
    if unresolved_materials:
        blockers.append(
            {
                "code": "UNRESOLVED_SOURCE_MATERIAL",
                "count": len(unresolved_materials),
                "sourceMaterialPaths": sorted(
                    {str(row.get("sourceMaterialPath") or "") for row in unresolved_materials}
                ),
            }
        )
    non_exact_materials = [
        row
        for row in material_rows
        if str(row.get("resolutionStatus") or "").startswith("RESOLVED_")
        and str(row.get("resolutionStatus")) != "RESOLVED_EXACT_SOURCE_PACKAGE"
    ]
    if non_exact_materials:
        blockers.append(
            {
                "code": "SOURCE_MATERIAL_NON_EXACT_PACKAGE",
                "count": len(non_exact_materials),
                "statusCounts": dict(
                    sorted(
                        Counter(
                            str(row.get("resolutionStatus"))
                            for row in non_exact_materials
                        ).items()
                    )
                ),
            }
        )
    unresolved_runtime = [
        row
        for row in graph.get("runtimeResourceBindings", [])
        if row.get("resolutionStatus") != "RESOLVED_RUNTIME_ASSET"
    ]
    if unresolved_runtime:
        blockers.append(
            {
                "code": "UNRESOLVED_RUNTIME_RESOURCE",
                "count": len(unresolved_runtime),
                "sourceObjectPaths": sorted(
                    {str(row.get("sourceObjectPath") or "") for row in unresolved_runtime}
                ),
            }
        )

    closure_summary = closure.get("summary", {})
    for field, code in (
        ("unresolvedRequestCount", "UNRESOLVED_EXTERNAL_MODULE_CLOSURE"),
        ("propertyErrorCount", "EXTERNAL_MODULE_PROPERTY_ERROR"),
    ):
        count = int(closure_summary.get(field, 0))
        if count:
            blockers.append({"code": code, "count": count})
    blockers.extend(collect_conversion_blockers(conversion))
    return blockers


def collect_conversion_blockers(
    conversion: dict[str, Any],
) -> list[dict[str, Any]]:
    blockers: list[dict[str, Any]] = []
    conversion_summary = conversion.get("summary", {})
    for field, code in (
        ("unsupportedEmitterCount", "UNSUPPORTED_IMPORTED_EMITTER"),
        ("missingResourceEmitterCount", "IMPORTED_EMITTER_MISSING_RESOURCE"),
        (
            "sourceMaterialPendingEmitterCount",
            "IMPORTED_SOURCE_MATERIAL_RUNTIME_PENDING",
        ),
    ):
        count = int(conversion_summary.get(field, 0))
        if count:
            blockers.append({"code": code, "count": count})

    status_counts = Counter(
        str(row.get("status") or "MISSING_STATUS")
        for row in conversion.get("elementConversions", [])
    )
    source_recipe_pending = status_counts.get("SOURCE_RECIPE_RUNTIME_PENDING", 0)
    if source_recipe_pending:
        blockers.append(
            {
                "code": "IMPORTED_SOURCE_RECIPE_RUNTIME_PENDING",
                "count": source_recipe_pending,
            }
        )
    known_statuses = {
        "CONVERTED",
        "MISSING_RESOURCE",
        "SOURCE_MATERIAL_RUNTIME_PENDING",
        "SOURCE_RECIPE_RUNTIME_PENDING",
        "UNSUPPORTED",
    }
    unknown_statuses = {
        status: count
        for status, count in status_counts.items()
        if status not in known_statuses
    }
    if unknown_statuses:
        blockers.append(
            {
                "code": "IMPORTED_UNKNOWN_CONVERSION_STATUS",
                "count": sum(unknown_statuses.values()),
                "statusCounts": dict(sorted(unknown_statuses.items())),
            }
        )
    return blockers


def generated_artifact_set(
    config: ClassConfig,
    skill: dict[str, Any],
    binding_clips: Any,
    catalog: dict[str, dict[str, Any]],
    graph_paths: dict[str, Path],
    imported_root: Path,
    material_catalog: dict[str, list[dict[str, Any]]] | None,
    package_mapping: dict[str, str] | None,
    resolved_action_catalog: dict[str, Any] | None,
    runtime_resources_root: Path,
    runtime_cook_receipt: Path | None,
    package_root: Path,
) -> tuple[dict[str, Any] | None, list[dict[str, Any]]]:
    skill_id = int(skill["productSkillId"])
    current_root = imported_root / config.asset_id / "CurrentCombat"
    receipt_path = current_root / f"skill.{skill_id}.source-receipt.json"
    graph_path = current_root / "Graphs" / f"skill.{skill_id}.normalized-effect-graph.json"
    closure_path = current_root / "Modules" / f"skill.{skill_id}.external-module-closure.json"
    document_path = (
        current_root
        / "Converted"
        / f"effect.{config.asset_id.casefold()}.skill.{skill_id}.imported.effect.json"
    )
    conversion_path = current_root / "Converted" / f"skill.{skill_id}.element-conversion-receipt.json"
    blockers: list[dict[str, Any]] = []
    try:
        receipt, graph = materialize_source_graph(
            config,
            skill,
            binding_clips,
            catalog,
            graph_paths,
            graph_path,
            receipt_path,
            material_catalog,
            package_mapping,
            resolved_action_catalog,
            runtime_resources_root,
            runtime_cook_receipt,
        )
    except Exception as error:  # Preserve the exact source failure per skill.
        return None, [
            {
                "code": "SOURCE_GRAPH_OR_RECEIPT_GENERATION_FAILED",
                "detail": f"{type(error).__name__}: {error}",
            }
        ]

    summary = receipt["summary"]
    if summary["unresolvedSourceSystemCount"]:
        blockers.append(
            {
                "code": "UNRESOLVED_SOURCE_SYSTEM",
                "count": summary["unresolvedSourceSystemCount"],
            }
        )
    if summary["missingParticleGraphPackageCount"]:
        blockers.append(
            {
                "code": "MISSING_PARTICLE_GRAPH_PACKAGE",
                "count": summary["missingParticleGraphPackageCount"],
            }
        )
    unresolved_materials = [
        row
        for row in graph.get("materialParameterBindings", [])
        if not str(row.get("resolutionStatus") or "").startswith("RESOLVED_")
    ]
    if unresolved_materials:
        blockers.append(
            {
                "code": "UNRESOLVED_SOURCE_MATERIAL",
                "count": len(unresolved_materials),
                "sourceMaterialPaths": sorted(
                    {str(row.get("sourceMaterialPath") or "") for row in unresolved_materials}
                ),
            }
        )
    unresolved_runtime = [
        row
        for row in graph.get("runtimeResourceBindings", [])
        if row.get("resolutionStatus") != "RESOLVED_RUNTIME_ASSET"
    ]
    if unresolved_runtime:
        blockers.append(
            {
                "code": "UNRESOLVED_RUNTIME_RESOURCE",
                "count": len(unresolved_runtime),
                "sourceObjectPaths": sorted(
                    {str(row.get("sourceObjectPath") or "") for row in unresolved_runtime}
                ),
            }
        )

    try:
        closure = build_closure(graph, package_root)
        write_json_atomic(closure_path, closure)
    except Exception as error:
        return None, blockers + [
            {
                "code": "EXTERNAL_MODULE_CLOSURE_GENERATION_FAILED",
                "detail": f"{type(error).__name__}: {error}",
            }
        ]
    closure_summary = closure["summary"]
    if closure_summary["unresolvedRequestCount"]:
        blockers.append(
            {
                "code": "UNRESOLVED_EXTERNAL_MODULE_CLOSURE",
                "count": closure_summary["unresolvedRequestCount"],
            }
        )
    if closure_summary["propertyErrorCount"]:
        blockers.append(
            {
                "code": "EXTERNAL_MODULE_PROPERTY_ERROR",
                "count": closure_summary["propertyErrorCount"],
            }
        )

    try:
        document, conversion = build_document(receipt, graph, closure)
        write_json_atomic(document_path, document)
        write_json_atomic(conversion_path, conversion)
    except Exception as error:
        return None, blockers + [
            {
                "code": "IMPORTED_CONVERSION_FAILED",
                "detail": f"{type(error).__name__}: {error}",
            }
        ]

    blockers.extend(collect_conversion_blockers(conversion))

    return (
        {
            "sourceReceipt": artifact(receipt_path),
            "normalizedGraph": artifact(graph_path),
            "externalModuleClosure": artifact(closure_path),
            "importedDocument": artifact(
                document_path, str(document.get("effectAssetId") or "")
            ),
            "conversionReceipt": artifact(conversion_path),
            "artifactOrigin": "CURRENT_COMBAT_GENERATED_SOURCE_TRUTH",
        },
        blockers,
    )


def attach_skill_artifacts(
    skill: dict[str, Any],
    source_artifacts: dict[str, Any] | None,
    blockers: list[dict[str, Any]],
    receipt: dict[str, Any] | None,
) -> None:
    events = receipt.get("timeline", {}).get("events", []) if receipt else []
    ready_stage_count = 0
    for stage in skill["stages"]:
        sequence_indices = {
            int(clip["sequenceIndex"]) for clip in stage.get("clips", [])
        }
        stage["sourceEventIds"] = [
            str(event["eventId"])
            for event in events
            if int(event["clipSequenceIndex"]) in sequence_indices
        ]
        if source_artifacts is not None:
            stage["sourceArtifacts"] = [
                {
                    **source_artifacts,
                    "sourceSkillId": int(stage["sourceSkillIds"][0]),
                }
            ]
        stage["blockers"] = blockers
        stage["status"] = (
            "READY"
            if source_artifacts is not None and not blockers
            else "AVAILABLE_WITH_BLOCKERS"
            if source_artifacts is not None
            else "BLOCKED"
        )
        ready_stage_count += stage["status"] == "READY"
    skill["blockers"] = blockers
    skill["status"] = (
        "READY"
        if source_artifacts is not None
        and not blockers
        and ready_stage_count == len(skill["stages"])
        else "AVAILABLE_WITH_BLOCKERS"
        if source_artifacts is not None
        else "BLOCKED"
    )


def attach_reviewed_stage_decisions(
    config: ClassConfig,
    skill: dict[str, Any],
    catalog: dict[str, dict[str, Any]],
) -> None:
    """Attach only the two user-reviewed Artist 31210 source decisions."""
    if config.asset_id != "Artist" or int(skill["productSkillId"]) != 31210:
        return
    stages = skill["stages"]
    if len(stages) != 4:
        raise ValueError("Artist 31210 reviewed stage shape drifted")

    silent_stage = stages[1]
    silent_clips = silent_stage.get("clips", [])
    if (
        len(silent_clips) != 1
        or silent_clips[0].get("clip") != "sdm_sk_skykongkong_03"
        or silent_stage.get("sourceEventIds")
    ):
        raise ValueError("Artist 31210 intentional-silence evidence drifted")
    notifies = catalog["sdm_sk_skykongkong_03"]["notifies"]
    effect_count = sum(row.get("kind") == "EFFECT" for row in notifies)
    shake_count = sum(row.get("kind") == "SHAKE" for row in notifies)
    if effect_count or shake_count:
        raise ValueError("Artist 31210 silent stage gained presentation notifies")
    observed_kinds = sorted({str(row.get("kind") or "") for row in notifies})
    silent_stage["completionDecision"] = {
        "decision": "sourceIntentionallySilent",
        "evidence": {
            "clip": "sdm_sk_skykongkong_03",
            "effectNotifyCount": effect_count,
            "shakeNotifyCount": shake_count,
            "observedNotifyKinds": observed_kinds,
        },
        "rationale": (
            "The exact bound source clip contains control-only notifies and no "
            "Effect presentation to materialize."
        ),
    }

    reviewed_stage = stages[3]
    artifacts = reviewed_stage.get("sourceArtifacts", [])
    if len(artifacts) != 1:
        raise ValueError("Artist 31210 reviewed stage source artifact is absent")
    source = artifacts[0]
    document_descriptor = source["importedDocument"]
    if document_descriptor["sha256"] != ARTIST_31210_REVIEWED_DOCUMENT_SHA256:
        raise ValueError("Artist 31210 reviewed Imported document hash drifted")
    document = read_json(Path(document_descriptor["path"]))
    conversion = read_json(Path(source["conversionReceipt"]["path"]))
    elements = {str(row.get("id") or ""): row for row in document["elements"]}
    conversion_rows = conversion.get("elementConversions", [])
    stage_event_ids = set(reviewed_stage.get("sourceEventIds", []))
    for element_id, expected_hash in ARTIST_31210_REVIEWED_RIBBON_ELEMENTS.items():
        element = elements.get(element_id)
        if element is None or canonical_json_sha256(element) != expected_hash:
            raise ValueError(f"Artist 31210 reviewed source element drifted: {element_id}")
        event_id = element_id.rsplit("event_", 1)[-1]
        if event_id not in stage_event_ids:
            raise ValueError(f"Artist 31210 reviewed event left its stage: {event_id}")
        if (
            element.get("kind") != "particle"
            or element.get("sourceRecipe", {}).get("rendererShape") != "sprite"
            or not element.get("resources")
        ):
            raise ValueError(f"Artist 31210 reviewed renderer evidence drifted: {element_id}")
        owners = [
            row
            for row in conversion_rows
            if element_id in row.get("elementIds", [])
        ]
        if len(owners) != 1:
            raise ValueError(f"Artist 31210 reviewed conversion owner drifted: {element_id}")
        owner = owners[0]
        module_classes = {
            str(row.get("className") or "")
            for row in owner.get("unrepresentedModules", [])
        }
        if (
            owner.get("status") != "SOURCE_RECIPE_RUNTIME_PENDING"
            or owner.get("rendererShape") != "sprite"
            or "particlemoduletypedataribbon" not in module_classes
        ):
            raise ValueError(f"Artist 31210 reviewed ribbon evidence drifted: {element_id}")

    reviewed_stage["rendererApproximationApproval"] = {
        "decision": "reviewedRendererApproximation",
        "sourceElementIds": list(ARTIST_31210_REVIEWED_RIBBON_ELEMENTS),
        "sourceElementSha256": dict(ARTIST_31210_REVIEWED_RIBBON_ELEMENTS),
        "sourceDocumentSha256": ARTIST_31210_REVIEWED_DOCUMENT_SHA256,
        "sourceKind": "spriteParticle",
        "targetKind": "sprite",
        "requiredReasonCodes": list(ARTIST_31210_REVIEWED_REASON_CODES),
        "rationale": (
            "User-reviewed narrow approximation of the six exact final-stage "
            "ribbon occurrences as Sprites; no generic fallback or cross-skill "
            "borrowing is authorized."
        ),
    }


def product_source_receipt(
    config: ClassConfig,
    skill: dict[str, Any],
    generated_artifacts: dict[str, Any],
    generated_receipt: dict[str, Any],
    output: Path,
) -> dict[str, Any]:
    clip_rows = [
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
    wrapper = {
        "schema": "lostark.combat-effect-product-source-receipt",
        "version": 1,
        "animationAssetId": config.asset_id,
        "characterClass": config.character_class,
        "productSkillId": int(skill["productSkillId"]),
        "sourceSkillIds": skill["sourceSkillIds"],
        "clipOwnershipProvenance": {
            "joinKey": "exact bound clip identity",
            "numericSkillAliasIsJoinKey": False,
            "clips": clip_rows,
        },
        "generatedSourceReceipt": generated_artifacts["sourceReceipt"],
        "normalizedGraph": generated_artifacts["normalizedGraph"],
        "timeline": generated_receipt.get("timeline", {}),
        "summary": generated_receipt.get("summary", {}),
    }
    write_json_atomic(output, wrapper)
    return artifact(output)


def source_graph_packages_for_skill(
    skill: dict[str, Any], catalog: dict[str, dict[str, Any]]
) -> set[str]:
    result = set()
    for stage in skill["stages"]:
        for clip in stage["clips"]:
            for notify in catalog[str(clip["clip"])]["notifies"]:
                if notify["sourceType"] != "PlayParticleEffect":
                    continue
                source_asset = str(notify.get("sourceAsset") or "")
                if "." in source_asset:
                    result.add(source_asset.split(".", 1)[0].casefold())
    return result


def build_all(
    data_root: Path,
    resource_root: Path,
    package_root: Path,
    runtime_resources_root: Path,
    materialize: bool,
    extract_missing_particle_graphs: bool = False,
) -> dict[str, Any]:
    imported_root = data_root / "Effects" / "Imported"
    player_skills_path = data_root / "Balance" / "PlayerSkills.json"
    player_skills = read_json(player_skills_path)
    external_root = resource_root / "05_Reports" / "EffectExtraction"
    material_map_path = (
        external_root
        / "DIMENSIONMASTER"
        / "materials"
        / "material_texture_map.runtime.json"
    )
    shared_material_catalog: dict[str, list[dict[str, Any]]] | None = None
    if materialize:
        raw_materials = read_json(material_map_path).get("materials", {})
        shared_material_catalog = {
            str(key).casefold(): list(value) for key, value in raw_materials.items()
        }

    manifests = []
    blocker_rows = []
    total_artifacts = Counter()
    for config in CLASS_CONFIGS:
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
        manifest = build_class_stage_contract(
            config,
            player_skills,
            bindings_path,
            animnotify_path,
            player_skills_path,
        )
        catalog = parse_exact_clip_catalog(animnotify_path)
        binding_rows = {
            int(row["skillId"]): row
            for row in read_json(bindings_path).get("bindings", [])
        }
        class_imported_root = imported_root / config.asset_id
        resource_manifest_path = (
            class_imported_root / config.resource_manifest_name
            if config.resource_manifest_name
            else None
        )
        resource_manifest = (
            read_json(resource_manifest_path)
            if resource_manifest_path is not None
            and resource_manifest_path.is_file()
            else None
        )
        extraction_root = external_root.joinpath(*config.extraction_folder.split("/"))
        graph_root = extraction_root / config.graph_folder
        supplemental_root = extraction_root / "current_combat_particle_graphs"
        graph_paths = graph_registry(graph_root, supplemental_root)
        graph_extraction_receipts = []
        existing_by_skill = {
            int(skill["productSkillId"]): existing_artifact_set(
                config, skill, imported_root, resource_manifest
            )
            for skill in manifest["skills"]
        }
        all_required_graph_packages = {
            package
            for skill in manifest["skills"]
            if existing_by_skill[int(skill["productSkillId"])] is None
            for package in source_graph_packages_for_skill(skill, catalog)
        }
        all_bound_graph_packages = {
            package
            for skill in manifest["skills"]
            for package in source_graph_packages_for_skill(skill, catalog)
        }
        if materialize and extract_missing_particle_graphs:
            graph_extraction_receipts = extract_missing_graphs(
                all_required_graph_packages,
                graph_paths,
                supplemental_root,
                package_root,
            )
        manifest["sourceGraphExtraction"] = supplemental_graph_provenance(
            all_bound_graph_packages,
            graph_paths,
            supplemental_root,
            graph_extraction_receipts,
        )
        runtime_cook_receipt = (
            extraction_root / config.runtime_cook_name
            if config.runtime_cook_name
            else None
        )
        resolved_action_catalog_path = (
            class_imported_root / config.resolved_action_catalog_name
            if config.resolved_action_catalog_name
            else None
        )
        resolved_action_catalog = (
            read_json(resolved_action_catalog_path)
            if resolved_action_catalog_path is not None
            and resolved_action_catalog_path.is_file()
            else None
        )
        package_mapping = source_package_map(resource_manifest_path)
        package_manifest_path = (
            class_imported_root
            / "CurrentCombat"
            / f"{config.asset_id}.source-package-resolution.json"
        )
        package_manifest = {
            "schema": "lostark.effect-source-package-resolution",
            "version": 1,
            "characterClass": config.character_class,
            "sourceResourceManifest": (
                artifact(resource_manifest_path)
                if resource_manifest_path is not None
                and resource_manifest_path.is_file()
                else None
            ),
            "packages": [
                {"logicalPackage": key, "physicalPackage": value}
                for key, value in sorted(package_mapping.items())
            ],
            "summary": {"resolvedPackageCount": len(package_mapping)},
        }
        if materialize and config.asset_id != "Warlord":
            write_json_atomic(package_manifest_path, package_manifest)

        for skill in manifest["skills"]:
            skill_id = int(skill["productSkillId"])
            existing = existing_by_skill[skill_id]
            blockers: list[dict[str, Any]] = []
            source_artifacts = existing
            if source_artifacts is None and materialize:
                # Missing package graphs are exact source blockers.  They are
                # extracted only under the explicit source-pack CLI flag.
                missing_graph_packages = sorted(
                    source_graph_packages_for_skill(skill, catalog) - set(graph_paths)
                )
                if missing_graph_packages:
                    blockers.append(
                        {
                            "code": "MISSING_PARTICLE_GRAPH_PACKAGE",
                            "logicalPackages": missing_graph_packages,
                            "physicalPackageCandidates": [
                                {
                                    "logicalPackage": logical,
                                    "expectedPhysicalPackage": (
                                        obfuscate_package_name(logical) + ".upk"
                                    ),
                                    "packagePresent": (
                                        package_root
                                        / (obfuscate_package_name(logical) + ".upk")
                                    ).is_file(),
                                }
                                for logical in missing_graph_packages
                            ],
                        }
                    )
                else:
                    source_artifacts, generated_blockers = generated_artifact_set(
                        config,
                        skill,
                        binding_rows[skill_id]["clips"],
                        catalog,
                        graph_paths,
                        imported_root,
                        (
                            None
                            if config.asset_id == "Warlord"
                            else shared_material_catalog
                        ),
                        (
                            None
                            if config.asset_id == "Warlord"
                            else package_mapping
                        ),
                        resolved_action_catalog,
                        runtime_resources_root,
                        runtime_cook_receipt,
                        package_root,
                    )
                    blockers.extend(generated_blockers)
            elif source_artifacts is None:
                blockers.append({"code": "SOURCE_ARTIFACT_NOT_MATERIALIZED"})

            receipt = None
            if source_artifacts is not None:
                blockers = collect_artifact_blockers(source_artifacts)
                receipt = read_json(Path(source_artifacts["sourceReceipt"]["path"]))
                wrapper_path = (
                    class_imported_root
                    / "CurrentCombat"
                    / "ProductReceipts"
                    / f"skill.{skill_id}.product-source-receipt.json"
                )
                wrapper_artifact = product_source_receipt(
                    config,
                    skill,
                    source_artifacts,
                    receipt,
                    wrapper_path,
                )
                source_artifacts = {
                    **source_artifacts,
                    "generatedSourceReceipt": source_artifacts["sourceReceipt"],
                    "sourceReceipt": wrapper_artifact,
                }
                total_artifacts["sourceReceiptCount"] += 1
                total_artifacts["closureCount"] += 1
                total_artifacts["importedDocumentCount"] += 1
            attach_skill_artifacts(skill, source_artifacts, blockers, receipt)
            attach_reviewed_stage_decisions(config, skill, catalog)
            blocker_rows.append(
                {
                    "animationAssetId": config.asset_id,
                    "characterClass": config.character_class,
                    "productSkillId": skill_id,
                    "sourceSkillIds": skill["sourceSkillIds"],
                    "status": skill["status"],
                    "blockers": blockers,
                }
            )

        summary = manifest["summary"]
        summary["readySkillCount"] = sum(
            row["status"] == "READY" for row in manifest["skills"]
        )
        summary["blockedSkillCount"] = len(manifest["skills"]) - summary["readySkillCount"]
        summary["availableWithBlockersSkillCount"] = sum(
            row["status"] == "AVAILABLE_WITH_BLOCKERS"
            for row in manifest["skills"]
        )
        summary["blockedSkillCount"] = sum(
            row["status"] == "BLOCKED" for row in manifest["skills"]
        )
        summary["readyStageCount"] = sum(
            stage["status"] == "READY"
            for row in manifest["skills"]
            for stage in row["stages"]
        )
        summary["blockedStageCount"] = summary["stageCount"] - summary["readyStageCount"]
        summary["availableWithBlockersStageCount"] = sum(
            stage["status"] == "AVAILABLE_WITH_BLOCKERS"
            for row in manifest["skills"]
            for stage in row["stages"]
        )
        summary["blockedStageCount"] = sum(
            stage["status"] == "BLOCKED"
            for row in manifest["skills"]
            for stage in row["stages"]
        )
        manifest_path = (
            class_imported_root
            / f"{config.asset_id}.combat-source-stage-manifest.json"
        )
        write_json_atomic(manifest_path, manifest)
        manifests.append(artifact(manifest_path))

    blocker_matrix = {
        "schema": "lostark.four-class-combat-effect-source-blocker-matrix",
        "version": 1,
        "policy": {
            "genericFallback": "FORBIDDEN",
            "crossSkillEffectBorrowing": "FORBIDDEN",
            "numericSkillAliasJoin": "FORBIDDEN",
            "exactBoundClipIdentityJoin": "REQUIRED",
        },
        "classStageManifests": manifests,
        "skills": blocker_rows,
        "summary": {
            "skillCount": len(blocker_rows),
            "stageCount": sum(EXPECTED_STAGE_COUNTS.values()),
            "readySkillCount": sum(row["status"] == "READY" for row in blocker_rows),
            "availableWithBlockersSkillCount": sum(
                row["status"] == "AVAILABLE_WITH_BLOCKERS" for row in blocker_rows
            ),
            "blockedSkillCount": sum(row["status"] == "BLOCKED" for row in blocker_rows),
            **dict(total_artifacts),
            "blockerCodeCounts": dict(
                sorted(
                    Counter(
                        blocker["code"]
                        for row in blocker_rows
                        for blocker in row["blockers"]
                    ).items()
                )
            ),
        },
    }
    blocker_path = imported_root / "FourClassCombat.source-blocker-matrix.json"
    write_json_atomic(blocker_path, blocker_matrix)
    return blocker_matrix


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", type=Path, default=Path("Data"))
    parser.add_argument(
        "--resource-root",
        type=Path,
        default=Path("C:/Users/user/Desktop/Resource_LostArk"),
    )
    parser.add_argument(
        "--package-root",
        type=Path,
        default=Path(
            "C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages"
        ),
    )
    parser.add_argument(
        "--runtime-resources-root",
        type=Path,
        default=Path("Client/Bin/Resources"),
    )
    parser.add_argument("--materialize", action="store_true")
    parser.add_argument("--extract-missing-particle-graphs", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    result = build_all(
        args.data_root,
        args.resource_root,
        args.package_root,
        args.runtime_resources_root,
        args.materialize,
        args.extract_missing_particle_graphs,
    )
    print(json.dumps(result["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
