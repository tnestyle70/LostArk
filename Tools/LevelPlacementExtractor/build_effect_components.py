#!/usr/bin/env python3
"""Split authored Effects into WFX components and compile them losslessly."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
from collections import Counter
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import dimensionmaster_admitted_skills
from build_imported_effect_documents import read_json, write_json_atomic


STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]+$")
MAX_SOURCE_ACTION_CUE_SECONDS = 60.0


def canonical_json(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def sha256_json(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def component_kind(elements: list[dict[str, Any]]) -> str:
    kinds = {str(row["kind"]) for row in elements}
    if kinds == {"light"}:
        return "light"
    if kinds == {"screenPost"}:
        return "screenPost"
    if kinds == {"trail"}:
        return "trail"
    if kinds == {"decal"}:
        return "decal"
    if "particle" in kinds:
        return "particleSystem"
    if kinds <= {"mesh", "sprite"}:
        return "visual"
    return "mixed"


def renderer_kind(element: dict[str, Any]) -> str:
    if element["kind"] != "particle":
        return str(element["kind"])
    resources = element.get("resources", [])
    mesh_binding_count = sum(
        1 for row in resources if row.get("slotId") == "meshModel"
    )
    source_recipe = element.get("sourceRecipe", {})
    if not source_recipe.get("enabled"):
        return "mesh" if mesh_binding_count == 1 else "sprite"
    renderer_shape = str(source_recipe.get("rendererShape") or "")
    if renderer_shape == "mesh" and mesh_binding_count == 1:
        return "mesh"
    if renderer_shape == "sprite" and mesh_binding_count == 0:
        return "sprite"
    raise ValueError(
        "Cascade renderer shape/resource contradiction for "
        f"{element.get('id')}: rendererShape={renderer_shape!r}, "
        f"meshModelBindings={mesh_binding_count}"
    )


def effect_identity(effect_id: str, character_class: str) -> str:
    prefix = f"effect.{character_class.casefold()}."
    if not effect_id.startswith(prefix):
        raise ValueError(
            f"Effect is outside {character_class} identity: {effect_id}"
        )
    identity = effect_id[len(prefix):]
    if not identity or not STABLE_ID.fullmatch(identity):
        raise ValueError(f"invalid Effect identity: {effect_id}")
    return identity


def component_directory(effect_id: str, character_class: str) -> str:
    return effect_identity(effect_id, character_class)


def split_document(
    document: dict[str, Any],
    character_class: str,
    input_slot: str | None = None,
) -> tuple[dict[str, Any], list[tuple[str, dict[str, Any]]]]:
    if document.get("schema") != "lostark.effect-authoring":
        raise ValueError("not an Effect authoring document")
    effect_id = str(document.get("effectAssetId") or "")
    if not STABLE_ID.fullmatch(effect_id):
        raise ValueError("invalid Effect asset ID")
    groups: dict[str, list[tuple[int, dict[str, Any]]]] = {}
    for source_index, element in enumerate(document.get("elements", [])):
        group_id = str(element.get("groupId") or element.get("id") or "")
        if not STABLE_ID.fullmatch(group_id):
            raise ValueError(f"invalid component group ID: {group_id}")
        groups.setdefault(group_id, []).append((source_index, element))
    if not groups:
        raise ValueError("Effect has no elements")

    ordered = sorted(
        groups.items(),
        key=lambda item: (
            min(float(row["detail"]["timing"]["startDelaySeconds"]) for _, row in item[1]),
            item[0],
        ),
    )
    identity = effect_identity(effect_id, character_class)
    prefix = f"effect.component.{character_class.casefold()}.{identity}"
    file_identity = identity.replace(".", "_")
    cue_rows = []
    outputs: list[tuple[str, dict[str, Any]]] = []
    for index, (group_id, indexed_elements) in enumerate(ordered):
        source_elements = [row for _, row in indexed_elements]
        start = min(
            float(row["detail"]["timing"]["startDelaySeconds"])
            for row in source_elements
        )
        component_id = f"{prefix}.{index:02d}"
        localized = []
        emitters = []
        for source_index, source in indexed_elements:
            element = copy.deepcopy(source)
            element["detail"]["timing"]["startDelaySeconds"] = max(
                0.0,
                float(element["detail"]["timing"]["startDelaySeconds"]) - start,
            )
            localized.append(element)
            emitters.append({
                "emitterId": str(element["id"]),
                "elementId": str(element["id"]),
                "sourceElementIndex": source_index,
                "renderer": renderer_kind(element),
                "visible": bool(element.get("visible", True)),
                "resourceBindingCount": len(element.get("resources", [])),
                "moduleCount": len(
                    element.get("sourceRecipe", {}).get("modules", [])
                ),
            })
        kind = component_kind(localized)
        file_name = (
            f"{character_class}_{file_identity}_{index:02d}."
            f"{kind.casefold()}.wfx.json"
        )
        source_metadata = {
            "effectAssetId": effect_id,
            "groupId": group_id,
            "sourceNodes": sorted({
                str(row.get("sourceNode") or "") for row in source_elements
            }),
            "sourceElementSha256": sha256_json(source_elements),
        }
        if input_slot:
            source_metadata["inputSlot"] = input_slot
        component = {
            "schema": "lostark.effect-component",
            "version": 1,
            "componentAssetId": component_id,
            "displayName": file_name.removesuffix(".json"),
            "componentType": kind,
            "source": source_metadata,
            "emitters": emitters,
            "document": {
                "schema": "lostark.effect-authoring",
                "version": int(document.get("version", 0)),
                "effectAssetId": component_id,
                "displayName": file_name.removesuffix(".json"),
                "particleSystem": {
                    "uniformScaleMultiplier": 1.0,
                    "yawOffsetDegrees": 0.0,
                    "directionYawDegrees": 0.0,
                    "initialSpeedMultiplier": 1.0,
                },
                "modelCues": [],
                "elements": localized,
            },
        }
        outputs.append((file_name, component))
        cue_rows.append({
            "cueId": f"component-{index:02d}",
            "componentAssetId": component_id,
            "startDelaySeconds": start,
            "visible": True,
            "anchor": "root",
            "localTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        })

    assembly = {
        "schema": "lostark.effect-assembly",
        "version": 1,
        "effectAssetId": effect_id,
        "displayName": document.get("displayName", effect_id),
        "sourceAuthoringVersion": int(document.get("version", 0)),
        "particleSystem": copy.deepcopy(document.get("particleSystem", {})),
        "modelCues": copy.deepcopy(document.get("modelCues", [])),
        "componentCues": cue_rows,
        "sourceDocumentSha256": sha256_json(document),
    }
    if input_slot:
        assembly["inputSlot"] = input_slot
    return assembly, outputs


def compile_assembly(
    assembly: dict[str, Any],
    components: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    if assembly.get("schema") != "lostark.effect-assembly" or assembly.get("version") != 1:
        raise ValueError("unsupported Effect assembly")
    staged_elements: list[tuple[int, dict[str, Any]]] = []
    ids: set[str] = set()
    for cue in assembly.get("componentCues", []):
        component_id = str(cue.get("componentAssetId") or "")
        component = components.get(component_id)
        if component is None:
            raise ValueError(f"missing Effect component: {component_id}")
        if component.get("schema") != "lostark.effect-component" or component.get("version") != 1:
            raise ValueError(f"unsupported Effect component: {component_id}")
        offset = float(cue["startDelaySeconds"])
        component_document = component.get("document", {})
        if component_document.get("effectAssetId") != component_id:
            raise ValueError(f"component Document identity mismatch: {component_id}")
        elements_by_id = {
            str(element["id"]): element
            for element in component_document.get("elements", [])
        }
        if len(elements_by_id) != len(component_document.get("elements", [])):
            raise ValueError(f"duplicate component Element ID: {component_id}")
        for emitter in component.get("emitters", []):
            element_id = str(emitter.get("elementId") or "")
            source_element = elements_by_id.get(element_id)
            if source_element is None:
                raise ValueError(
                    f"component Emitter has no Element: {component_id}/{element_id}"
                )
            element = copy.deepcopy(source_element)
            if element["id"] in ids:
                raise ValueError(f"duplicate compiled element ID: {element['id']}")
            ids.add(element["id"])
            element["detail"]["timing"]["startDelaySeconds"] = (
                float(element["detail"]["timing"]["startDelaySeconds"]) + offset
            )
            staged_elements.append((int(emitter["sourceElementIndex"]), element))
        if len(component.get("emitters", [])) != len(elements_by_id):
            raise ValueError(f"component Emitter/Element count mismatch: {component_id}")
    staged_elements.sort(key=lambda row: row[0])
    return {
        "schema": "lostark.effect-authoring",
        "version": int(assembly["sourceAuthoringVersion"]),
        "effectAssetId": assembly["effectAssetId"],
        "displayName": assembly["displayName"],
        "particleSystem": copy.deepcopy(assembly.get("particleSystem", {})),
        "modelCues": copy.deepcopy(assembly.get("modelCues", [])),
        "elements": [row for _, row in staged_elements],
    }


def action_cues_for_effect(
    recipe: dict[str, Any], effect_id: str
) -> list[dict[str, Any]]:
    cues = copy.deepcopy(recipe.get("cues", []))
    marker = ".ba"
    if marker not in effect_id:
        return fail_close_invalid_source_action_cue_times(cues)
    stage_number = int(effect_id.rsplit(marker, 1)[1])
    sequence_index = stage_number - 1
    stage = next(
        (
            row for row in recipe.get("selectedStages", [])
            if int(row.get("sequenceIndex", -1)) == sequence_index
        ),
        None,
    )
    if stage is None:
        raise ValueError(
            f"BA stage {stage_number} has no Action cue stage contract"
        )
    offset = float(stage.get("clipOffsetSeconds", 0.0))
    selected = []
    for cue in cues:
        if int(cue.get("clipSequenceIndex", -1)) != sequence_index:
            continue
        cue["globalTimeSeconds"] = max(
            0.0, float(cue.get("globalTimeSeconds", 0.0)) - offset
        )
        selected.append(cue)
    return fail_close_invalid_source_action_cue_times(selected)


def fail_close_invalid_source_action_cue_times(
    cues: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    """Disable decoded cues whose timing cannot be a gameplay presentation.

    The byte-lossless payload and decoded numbers stay in the Assembly for a
    future decoder correction.  Only the semantic execution bit is disabled,
    preventing malformed float offsets from reaching typed presentation
    channels.  The publisher independently enforces the same bound.
    """
    for cue in cues:
        if not bool(cue.get("executionEnabled")):
            continue
        values = [
            cue.get("localTimeSeconds"),
            cue.get("globalTimeSeconds"),
            cue.get("durationSeconds"),
        ]
        valid = all(
            isinstance(value, (int, float))
            and math.isfinite(float(value))
            and 0.0 <= float(value) <= MAX_SOURCE_ACTION_CUE_SECONDS
            for value in values
        )
        if valid:
            continue
        cue["executionEnabled"] = False
        cue["sourceExecutionStatus"] = "INVALID_SOURCE_TIME_FAIL_CLOSED"
        cue["executionDisabledReason"] = (
            "SOURCE_ACTION_CUE_TIME_OUTSIDE_FINITE_0_TO_60_SECONDS"
        )
    return cues


def remove_stale_generated_components(
    target_root: Path,
    expected_file_names: set[str],
    source_effect_id: str,
) -> list[str]:
    """Remove only obsolete generated WFX files owned by one source Effect."""
    if not target_root.exists():
        return []
    resolved_root = target_root.resolve()
    removed: list[str] = []
    for path in target_root.glob("*.wfx.json"):
        if path.name in expected_file_names:
            continue
        if path.resolve().parent != resolved_root:
            raise ValueError(f"component path escaped slot root: {path}")
        try:
            component = read_json(path)
        except (OSError, ValueError):
            continue
        if (
            component.get("schema") != "lostark.effect-component"
            or component.get("version") != 1
            or component.get("source", {}).get("effectAssetId")
            != source_effect_id
        ):
            continue
        path.unlink()
        removed.append(path.name)
    return sorted(removed)


def remove_relocated_generated_components(
    component_root: Path,
    expected_root: Path,
    source_effect_id: str,
) -> list[str]:
    """Remove source-owned components left in an obsolete slot directory."""
    if not component_root.exists():
        return []
    resolved_root = component_root.resolve()
    resolved_expected = expected_root.resolve()
    removed = []
    for path in component_root.rglob("*.wfx.json"):
        resolved = path.resolve()
        if resolved_expected in resolved.parents:
            continue
        if resolved_root not in resolved.parents:
            raise ValueError(f"component path escaped component root: {path}")
        try:
            component = read_json(path)
        except (OSError, ValueError):
            continue
        if (
            component.get("schema") != "lostark.effect-component"
            or component.get("version") != 1
            or component.get("source", {}).get("effectAssetId")
            != source_effect_id
        ):
            continue
        path.unlink()
        removed.append(str(path))
    return sorted(removed)


def remove_unadmitted_generated_artifacts(
    component_root: Path,
    assembly_root: Path,
    expected_effect_ids: set[str],
) -> tuple[list[str], list[str]]:
    """Remove only generated DimensionMaster artifacts absent from the catalog."""
    prefix = "effect.dimensionmaster.skill."
    removed_components = []
    if component_root.exists():
        for path in component_root.rglob("*.wfx.json"):
            try:
                component = read_json(path)
            except (OSError, ValueError):
                continue
            source_effect_id = str(
                component.get("source", {}).get("effectAssetId") or ""
            )
            if (
                component.get("schema") == "lostark.effect-component"
                and component.get("version") == 1
                and source_effect_id.startswith(prefix)
                and source_effect_id not in expected_effect_ids
            ):
                path.unlink()
                removed_components.append(str(path))

    removed_assemblies = []
    if assembly_root.exists():
        for path in assembly_root.glob("*.assembly.json"):
            try:
                assembly = read_json(path)
            except (OSError, ValueError):
                continue
            effect_id = str(assembly.get("effectAssetId") or "")
            if (
                assembly.get("schema") == "lostark.effect-assembly"
                and assembly.get("version") == 1
                and effect_id.startswith(prefix)
                and effect_id not in expected_effect_ids
            ):
                path.unlink()
                removed_assemblies.append(str(path))
    return sorted(removed_components), sorted(removed_assemblies)


def build_all(
    catalog_path: Path,
    data_root: Path,
    component_root: Path,
    assembly_root: Path,
    skill_bindings_path: Path | None = None,
) -> dict[str, Any]:
    catalog = read_json(catalog_path)
    if skill_bindings_path is None:
        skill_bindings_path = (
            data_root / "Animation" / "Authored" / "DimensionMaster" /
            "DimensionMaster.skillbindings.json"
        )
    admitted_skills = dimensionmaster_admitted_skills(
        data_root / "Balance" / "PlayerSkills.json", skill_bindings_path
    )
    skill_by_effect = {
        str(row["effectAssetId"]): row for row in admitted_skills
    }
    receipts = []
    stale_component_files: list[str] = []
    ignored_candidate_effects: list[str] = []
    action_recipe_by_skill: dict[int, dict[str, Any]] = {}
    expected_effect_ids = {
        str(entry["effectAssetId"])
        for entry in catalog.get("effects", [])
        if str(entry.get("effectAssetId") or "").startswith(
            "effect.dimensionmaster.skill."
        )
    }
    for entry in catalog.get("effects", []):
        effect_id = str(entry["effectAssetId"])
        if not effect_id.startswith("effect.dimensionmaster.skill."):
            continue
        base_id = effect_id.split(".ba", 1)[0]
        skill = skill_by_effect.get(base_id)
        if skill is None:
            ignored_candidate_effects.append(effect_id)
            continue
        slot = str(skill.get("inputSlot") or "") or None
        combo_stage = None
        if effect_id != base_id:
            match = re.fullmatch(
                re.escape(base_id) + r"\.ba([1-9][0-9]*)", effect_id
            )
            if match is None:
                raise ValueError(f"invalid combo stage Effect identity: {effect_id}")
            combo_stage = int(match.group(1))
            if str(skill.get("skillKind") or "").upper() != "COMBO":
                raise ValueError(f"non-combo Effect has BA stage: {effect_id}")
            if combo_stage > len(skill.get("comboStages", [])):
                raise ValueError(f"combo stage is outside gameplay contract: {effect_id}")
        authoring_path = data_root / str(entry["authoringPath"])
        document = read_json(authoring_path)
        assembly, component_files = split_document(
            document, "DimensionMaster", slot
        )
        assembly["sourceDocumentFileSha256"] = sha256_file(authoring_path)
        skill_id = int(skill["skillId"])
        if skill_id not in action_recipe_by_skill:
            action_path = (
                data_root / "Effects" / "Imported" / "DimensionMaster" /
                "Converted" / f"skill.{skill_id}.action-cue-recipe.json"
            )
            action_recipe_by_skill[skill_id] = (
                read_json(action_path) if action_path.is_file() else {"cues": []}
            )
        source_action_cues = action_cues_for_effect(
            action_recipe_by_skill[skill_id], effect_id
        )
        assembly["sourceActionCues"] = source_action_cues
        assembly["sourceActionCueSummary"] = {
            "cueCount": len(source_action_cues),
            "channels": dict(sorted(Counter(
                str(row.get("runtimeChannel") or "PRESENTATION_OTHER")
                for row in source_action_cues
            ).items())),
            "byteLosslessPayloadComplete": all(
                str(row.get("serializedPayload", {}).get("encoding")) ==
                "base64"
                for row in source_action_cues
            ),
        }
        directory = component_directory(effect_id, "DimensionMaster")
        target_components = component_root / directory
        components_by_id = {}
        for file_name, component in component_files:
            write_json_atomic(target_components / file_name, component)
            components_by_id[component["componentAssetId"]] = component
        removed = remove_stale_generated_components(
            target_components,
            {file_name for file_name, _ in component_files},
            effect_id,
        )
        stale_component_files.extend(
            str(target_components / file_name) for file_name in removed
        )
        stale_component_files.extend(
            remove_relocated_generated_components(
                component_root, target_components, effect_id
            )
        )
        assembly_path = assembly_root / f"{effect_id}.assembly.json"
        write_json_atomic(assembly_path, assembly)
        compiled = compile_assembly(assembly, components_by_id)
        if canonical_json(compiled) != canonical_json(document):
            raise ValueError(f"WFX compile identity mismatch: {effect_id}")
        receipt_row = {
            "effectAssetId": effect_id,
            "componentDirectory": directory,
            "assembly": str(assembly_path),
            "componentCount": len(component_files),
            "emitterCount": sum(
                len(value["emitters"]) for value in components_by_id.values()
            ),
            "sourceActionCueCount": len(source_action_cues),
            "sourceDocumentSha256": assembly["sourceDocumentSha256"],
            "compiledDocumentSha256": sha256_json(compiled),
            "compileIdentity": True,
        }
        if slot:
            receipt_row["inputSlot"] = slot
        if combo_stage is not None:
            receipt_row["comboStage"] = combo_stage
        receipts.append(receipt_row)
    removed_unadmitted_components, removed_unadmitted_assemblies = (
        remove_unadmitted_generated_artifacts(
            component_root, assembly_root, expected_effect_ids
        )
    )
    return {
        "schema": "lostark.effect-component-build-receipt",
        "version": 1,
        "characterClass": "DIMENSIONMASTER",
        "effectCount": len(receipts),
        "componentCount": sum(row["componentCount"] for row in receipts),
        "emitterCount": sum(row["emitterCount"] for row in receipts),
        "sourceActionCueCount": sum(
            row["sourceActionCueCount"] for row in receipts
        ),
        "removedStaleComponentFileCount": len(stale_component_files),
        "removedStaleComponentFiles": stale_component_files,
        "ignoredCandidateEffectCount": len(ignored_candidate_effects),
        "ignoredCandidateEffects": sorted(ignored_candidate_effects),
        "removedUnadmittedComponentFileCount": len(
            removed_unadmitted_components
        ),
        "removedUnadmittedComponentFiles": removed_unadmitted_components,
        "removedUnadmittedAssemblyFileCount": len(
            removed_unadmitted_assemblies
        ),
        "removedUnadmittedAssemblyFiles": removed_unadmitted_assemblies,
        "compileIdentityComplete": all(row["compileIdentity"] for row in receipts),
        "effects": receipts,
    }


def verify_existing(
    catalog_path: Path,
    data_root: Path,
    component_root: Path,
    assembly_root: Path,
    receipt_path: Path,
) -> dict[str, Any]:
    receipt = read_json(receipt_path)
    if (
        receipt.get("schema") != "lostark.effect-component-build-receipt"
        or receipt.get("version") != 1
    ):
        raise ValueError("unsupported Effect component build receipt")
    catalog = read_json(catalog_path)
    authored_by_id = {
        str(row["effectAssetId"]): data_root / str(row["authoringPath"])
        for row in catalog.get("effects", [])
    }
    component_count = 0
    emitter_count = 0
    action_cue_count = 0
    for row in receipt.get("effects", []):
        effect_id = str(row["effectAssetId"])
        assembly_path = assembly_root / f"{effect_id}.assembly.json"
        assembly = read_json(assembly_path)
        if assembly.get("effectAssetId") != effect_id:
            raise ValueError(f"Effect assembly identity mismatch: {effect_id}")
        directory = str(
            row.get("componentDirectory")
            or row.get("inputSlot")
            or component_directory(effect_id, "DimensionMaster")
        )
        slot_root = component_root / directory
        components: dict[str, dict[str, Any]] = {}
        for path in slot_root.glob("*.wfx.json"):
            component = read_json(path)
            component_id = str(component.get("componentAssetId") or "")
            if component_id in components:
                raise ValueError(f"duplicate component asset ID: {component_id}")
            components[component_id] = component
        expected_ids = {
            str(cue["componentAssetId"])
            for cue in assembly.get("componentCues", [])
        }
        selected = {
            component_id: components[component_id]
            for component_id in expected_ids
            if component_id in components
        }
        if selected.keys() != expected_ids:
            missing = sorted(expected_ids - selected.keys())
            raise ValueError(
                f"missing components for {effect_id}: {missing}"
            )
        compiled = compile_assembly(assembly, selected)
        source_path = authored_by_id.get(effect_id)
        if source_path is None:
            raise ValueError(f"Effect catalog has no authored path: {effect_id}")
        source = read_json(source_path)
        if canonical_json(compiled) != canonical_json(source):
            raise ValueError(f"WFX compile identity mismatch: {effect_id}")
        if sha256_json(source) != assembly.get("sourceDocumentSha256"):
            raise ValueError(f"WFX source hash mismatch: {effect_id}")
        if sha256_file(source_path) != assembly.get("sourceDocumentFileSha256"):
            raise ValueError(f"WFX source file hash mismatch: {effect_id}")
        cues = assembly.get("sourceActionCues", [])
        if not all(
            str(cue.get("serializedPayload", {}).get("encoding")) == "base64"
            and len(str(cue.get("serializedPayload", {}).get("sha256") or ""))
            == 64
            for cue in cues
        ):
            raise ValueError(f"Action cue payload is incomplete: {effect_id}")
        component_count += len(selected)
        emitter_count += sum(
            len(component.get("emitters", []))
            for component in selected.values()
        )
        action_cue_count += len(cues)
    actual = {
        "effectCount": len(receipt.get("effects", [])),
        "componentCount": component_count,
        "emitterCount": emitter_count,
        "sourceActionCueCount": action_cue_count,
        "compileIdentityComplete": True,
    }
    for name, value in actual.items():
        if name == "compileIdentityComplete":
            continue
        if int(receipt.get(name, -1)) != int(value):
            raise ValueError(
                f"Effect component receipt {name} mismatch: "
                f"{receipt.get(name)} != {value}"
            )
    return actual


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--catalog", type=Path,
        default=Path("Data/Effects/EffectCatalog.json")
    )
    parser.add_argument("--data-root", type=Path, default=Path("Data"))
    parser.add_argument(
        "--skill-bindings", type=Path,
        default=Path(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.skillbindings.json"
        )
    )
    parser.add_argument(
        "--component-root", type=Path,
        default=Path("Data/Effects/Components/DimensionMaster")
    )
    parser.add_argument(
        "--assembly-root", type=Path,
        default=Path("Data/Effects/Assemblies/DimensionMaster")
    )
    parser.add_argument(
        "--receipt", type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster/DimensionMaster.component-build.receipt.json")
    )
    parser.add_argument(
        "--verify-existing", action="store_true",
        help="Validate existing assemblies/components without writing files."
    )
    args = parser.parse_args()
    if args.verify_existing:
        result = verify_existing(
            args.catalog, args.data_root, args.component_root,
            args.assembly_root, args.receipt
        )
        print(json.dumps(result, sort_keys=True))
        return 0
    receipt = build_all(
        args.catalog, args.data_root, args.component_root,
        args.assembly_root, args.skill_bindings
    )
    write_json_atomic(args.receipt, receipt)
    print(json.dumps({
        key: receipt[key] for key in (
            "effectCount", "componentCount", "emitterCount",
            "compileIdentityComplete"
        )
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
