#!/usr/bin/env python3
"""Split authored Effects into WFX components and compile them losslessly."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import shlex
from collections import Counter
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import presentation_slot
from build_imported_effect_documents import read_json, write_json_atomic


STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]+$")
MAX_SOURCE_ACTION_CUE_SECONDS = 60.0
MAX_RUNTIME_DISPLAY_NAME_BYTES = 64
PRODUCT_CLASS_CONTRACTS = {
    "DimensionMaster": "DIMENSIONMASTER",
    "LanceMaster": "LANCE_MASTER",
    "Artist": "ARTIST",
    "Warlord": "WARLORD",
}
COMBAT_INPUT_SLOTS = {
    "DimensionMaster": frozenset({
        "LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V",
        "ALT_V",
    }),
    "LanceMaster": frozenset({
        "LMB", "Q", "W", "E", "R", "A", "S", "T", "V", "ALT_V",
    }),
    "Artist": frozenset({
        "LMB", "Q", "W", "E", "R", "A", "S", "V", "ALT_V",
    }),
    "Warlord": frozenset({
        "LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "X",
        "V", "ALT_V",
    }),
}


def bounded_runtime_display_name(value: str) -> str:
    """Return a deterministic, nonblank UTF-8 name accepted by the runtime codec."""
    normalized = value.strip()
    if not normalized:
        raise ValueError("runtime display name must not be blank")
    encoded = normalized.encode("utf-8")
    if len(encoded) <= MAX_RUNTIME_DISPLAY_NAME_BYTES:
        return normalized

    suffix = "~" + hashlib.sha256(encoded).hexdigest()[:10]
    head_budget = MAX_RUNTIME_DISPLAY_NAME_BYTES - len(suffix.encode("ascii"))
    head = normalized.encode("utf-8")[:head_budget]
    while head:
        try:
            truncated = head.decode("utf-8")
            break
        except UnicodeDecodeError:
            head = head[:-1]
    else:
        raise ValueError("runtime display name cannot be bounded")
    return truncated + suffix


def _append_binding_clips(value: Any, skill_id: int, clips: list[str]) -> None:
    if isinstance(value, str):
        clip = value
    elif isinstance(value, dict):
        clip = str(value.get("clip") or "")
    elif isinstance(value, list):
        if not value:
            raise ValueError(f"invalid animation binding clip: {skill_id}")
        for nested in value:
            _append_binding_clips(nested, skill_id, clips)
        return
    else:
        clip = ""
    if not clip:
        raise ValueError(f"invalid animation binding clip: {skill_id}")
    clips.append(clip)


def read_product_effect_cues(
    animevents_path: Path,
    animation_asset_id: str,
    available_clips: set[str],
    catalog_effect_ids: set[str],
) -> list[tuple[str, str]]:
    """Read only typed product Effect references from an animevent document."""
    try:
        lines = animevents_path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise ValueError(
            f"animation Effect cue document could not be read: {error}"
        ) from error
    if not lines:
        raise ValueError("animation Effect cue document is empty")
    try:
        header = shlex.split(lines[0], posix=True)
    except ValueError as error:
        raise ValueError("animation Effect cue header is invalid") from error
    if (
        len(header) != 4
        or header[0] != "LOSTARK_ANIM_EVENTS"
        or header[2] != animation_asset_id
    ):
        raise ValueError(
            f"animation Effect cue identity mismatch: {animation_asset_id}"
        )
    event_lines = [line for line in lines[1:] if line.strip()]
    try:
        version = int(header[1])
        declared_count = int(header[3])
    except ValueError as error:
        raise ValueError("animation Effect cue version/count is invalid") from error
    if version < 3 or version > 5:
        raise ValueError("animation Effect cue version must be in [3, 5]")
    if declared_count != len(event_lines):
        raise ValueError(
            "animation Effect cue row count mismatch: "
            f"{declared_count} != {len(event_lines)}"
        )

    def parse_uint32(value: str, label: str, line_number: int) -> int:
        if not value.isdecimal():
            raise ValueError(f"{label} is invalid at line {line_number}")
        parsed = int(value)
        if parsed > 0xFFFFFFFF:
            raise ValueError(f"{label} is invalid at line {line_number}")
        return parsed

    def parse_float(value: str, label: str, line_number: int) -> float:
        try:
            parsed = float(value)
        except ValueError as error:
            raise ValueError(f"{label} is invalid at line {line_number}") from error
        if not math.isfinite(parsed):
            raise ValueError(f"{label} is invalid at line {line_number}")
        return parsed

    result: list[tuple[str, str]] = []
    admitted_keys: set[tuple[str, int, str, str]] = set()
    for line_number, line in enumerate(event_lines, start=2):
        try:
            tokens = shlex.split(line, posix=True)
        except ValueError as error:
            raise ValueError(
                f"animation Effect cue syntax is invalid at line {line_number}"
            ) from error
        if len(tokens) < 3:
            raise ValueError(
                f"animation Effect cue row is incomplete at line {line_number}"
            )
        if tokens[1] != "EFFECT":
            continue
        attributes: dict[str, str] = {}
        for token in tokens[2:]:
            if "=" not in token or token.startswith("="):
                raise ValueError(
                    "animation Effect cue has an invalid field at line "
                    f"{line_number}: {token}"
                )
            key, value = token.split("=", 1)
            if key in attributes:
                raise ValueError(
                    "animation Effect cue has duplicate attribute at line "
                    f"{line_number}: {key}"
                )
            attributes[key] = value
        if attributes.get("effectref") != "asset":
            continue
        effect_id = str(attributes.get("payload") or "")
        if not STABLE_ID.fullmatch(effect_id):
            raise ValueError(
                f"animation Product Effect ID is invalid at line {line_number}"
            )
        start_ms = parse_uint32(
            str(attributes.get("startms") or ""), "startms", line_number
        )
        end_ms = start_ms
        if "endms" in attributes:
            end_ms = parse_uint32(attributes["endms"], "endms", line_number)
        if end_ms < start_ms:
            raise ValueError(f"endms precedes startms at line {line_number}")
        anchor = attributes.get("anchor", "root")
        if not anchor:
            raise ValueError(f"anchor is empty at line {line_number}")
        follow = attributes.get("follow", "follow")
        if follow not in {"follow", "snapshot"}:
            raise ValueError(f"follow policy is invalid at line {line_number}")
        stop = attributes.get("stop", "natural")
        if stop not in {"natural", "cue_end"}:
            raise ValueError(f"stop policy is invalid at line {line_number}")
        if stop == "cue_end" and end_ms <= start_ms:
            raise ValueError(
                f"cue_end requires endms greater than startms at line {line_number}"
            )
        for field, default in (
            ("px", 0.0), ("py", 0.0), ("pz", 0.0),
            ("rx", 0.0), ("ry", 0.0), ("rz", 0.0),
            ("sx", 1.0), ("sy", 1.0), ("sz", 1.0),
        ):
            parsed = parse_float(
                attributes.get(field, str(default)), field, line_number
            )
            if field.startswith("s") and parsed <= 0.0:
                raise ValueError(f"{field} must be positive at line {line_number}")
        if tokens[0] not in available_clips:
            raise ValueError(
                "animation Product cue is not owned by a bound skill clip: "
                f"{tokens[0]}"
            )
        if effect_id not in catalog_effect_ids:
            raise ValueError(
                f"animation Product cue is missing from Effect catalog: {effect_id}"
            )
        key = (tokens[0], start_ms, effect_id, anchor)
        if key in admitted_keys:
            raise ValueError("duplicate admitted animation Product Effect cue")
        admitted_keys.add(key)
        result.append((tokens[0], effect_id))
    return result


def admitted_skills_for_class(
    player_skills_path: Path,
    skill_bindings_path: Path,
    animevents_path: Path,
    animation_asset_id: str,
    catalog_effect_ids: set[str],
) -> list[dict[str, Any]]:
    """Admit catalog-backed skills through gameplay and animation truth.

    DimensionMaster historically requires PlayerSkills.effectId for every
    canonical Effect.  The other rollout classes did not have product Effect
    IDs before Authored restoration, so their actual product identity is the
    catalog entry plus animation asset cue.  An existing non-empty gameplay
    effectId must still match the canonical stable ID exactly.
    """
    stable_class = PRODUCT_CLASS_CONTRACTS.get(animation_asset_id)
    if stable_class is None:
        raise ValueError(
            f"unsupported product Effect class: {animation_asset_id}"
        )
    bindings_root = read_json(skill_bindings_path)
    if str(bindings_root.get("animationAssetId") or "") != animation_asset_id:
        raise ValueError(
            f"animation asset binding mismatch: {animation_asset_id}"
        )
    if str(bindings_root.get("characterClass") or "") != stable_class:
        raise ValueError(
            f"animation class binding mismatch: {animation_asset_id}"
        )

    clips_by_skill: dict[int, tuple[list[str], list[Any]]] = {}
    skill_by_clip: dict[str, int] = {}
    for row in bindings_root.get("bindings", []):
        skill_id = int(row["skillId"])
        if skill_id in clips_by_skill:
            raise ValueError(f"duplicate skillbinding: {skill_id}")
        raw_clips = row.get("clips", [])
        if not isinstance(raw_clips, list):
            raise ValueError(f"invalid skillbinding: {skill_id}")
        clips: list[str] = []
        for value in raw_clips:
            _append_binding_clips(value, skill_id, clips)
        if clips:
            clips_by_skill[skill_id] = (clips, copy.deepcopy(raw_clips))

        for clip in clips:
            previous_owner = skill_by_clip.setdefault(clip, skill_id)
            if previous_owner != skill_id:
                raise ValueError(
                    f"animation clip has multiple skill owners: {clip}"
                )

    prefix = f"effect.{animation_asset_id.casefold()}.skill."
    admitted_base_ids = {
        admitted_effect_variant(effect_id)[0]
        for effect_id in catalog_effect_ids
        if effect_id.startswith(prefix)
    }
    product_effects_by_skill: dict[int, set[str]] = {}
    for clip, effect_id in read_product_effect_cues(
        animevents_path,
        animation_asset_id,
        set(skill_by_clip),
        catalog_effect_ids,
    ):
        owner_skill_id = skill_by_clip.get(clip)
        assert owner_skill_id is not None
        base_id, _variant = admitted_effect_variant(effect_id)
        try:
            effect_skill_id = int(base_id.rsplit(".", 1)[1])
        except (IndexError, ValueError) as error:
            raise ValueError(
                f"animation Product cue has invalid skill identity: {effect_id}"
            ) from error
        if effect_skill_id != owner_skill_id:
            raise ValueError(
                "animation Product cue is owned by the wrong skill clip: "
                f"{effect_id} -> {clip}"
            )
        product_effects_by_skill.setdefault(owner_skill_id, set()).add(effect_id)

    product_effect_ids = {
        effect_id
        for effect_ids in product_effects_by_skill.values()
        for effect_id in effect_ids
    }
    if animation_asset_id != "DimensionMaster":
        catalog_only = sorted(catalog_effect_ids - product_effect_ids)
        if catalog_only:
            raise ValueError(
                "non-DimensionMaster Effect catalog entries require an exact "
                f"animation Product cue: {catalog_only}"
            )
    result: list[dict[str, Any]] = []
    admitted_skills: set[int] = set()
    for row in read_json(player_skills_path).get("skills", []):
        if str(row.get("characterClass") or "") != stable_class:
            continue
        if (
            str(row.get("inputSlot") or "") not in COMBAT_INPUT_SLOTS[
                animation_asset_id
            ]
            or str(row.get("setsStance") or "NONE") != "NONE"
        ):
            continue
        skill_id = int(row["skillId"])
        effect_id = f"{prefix}{skill_id}"
        if effect_id not in admitted_base_ids:
            continue
        gameplay_effect_id = str(row.get("effectId") or "")
        if gameplay_effect_id and gameplay_effect_id != effect_id:
            raise ValueError(
                f"canonical Effect identity mismatch: "
                f"{gameplay_effect_id} != {effect_id}"
            )
        if animation_asset_id == "DimensionMaster" and not gameplay_effect_id:
            raise ValueError(
                f"DimensionMaster canonical Effect is missing: {effect_id}"
            )
        binding_clips = clips_by_skill.get(skill_id)
        if not binding_clips:
            raise ValueError(
                f"catalog Effect has no skillbinding: {effect_id}"
            )
        if skill_id in admitted_skills:
            raise ValueError(f"duplicate catalog-backed skill: {skill_id}")
        clips, raw_clips = binding_clips
        result.append({
            "skillId": skill_id,
            "effectAssetId": effect_id,
            "inputSlot": presentation_slot(row.get("inputSlot")),
            "displayName": str(row.get("displayName") or effect_id),
            "skillKind": str(row.get("skillKind") or ""),
            "comboStages": copy.deepcopy(row.get("comboStages", [])),
            "clips": clips,
            "clipBindings": raw_clips,
            "productEffectIds": sorted(
                product_effects_by_skill.get(skill_id, set())
            ),
        })
        admitted_skills.add(skill_id)
    missing = sorted(
        effect_id for effect_id in admitted_base_ids
        if int(effect_id.rsplit(".", 1)[1]) not in admitted_skills
    )
    if missing:
        raise ValueError(
            f"catalog Effects have no gameplay skill contract: {missing}"
        )
    return result


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


def admitted_effect_variant(effect_id: str) -> tuple[str, str | None]:
    baseline_match = re.fullmatch(
        r"(.+)\.authored-baseline(?:\.clip([1-9][0-9]*))?", effect_id
    )
    if baseline_match is not None:
        base_id = baseline_match.group(1)
        if not base_id:
            raise ValueError(f"invalid Authored Baseline Effect identity: {effect_id}")
        clip_number = baseline_match.group(2)
        variant = "authored-baseline"
        if clip_number is not None:
            variant += f".clip{clip_number}"
        return base_id, variant
    if ".authored-baseline" in effect_id:
        raise ValueError(f"invalid Authored Baseline Effect identity: {effect_id}")
    ba_clip_match = re.fullmatch(
        r"(.+)\.ba([1-9][0-9]*)\.clip([1-9][0-9]*)", effect_id
    )
    if ba_clip_match is not None:
        return (
            ba_clip_match.group(1),
            f"ba{ba_clip_match.group(2)}.clip{ba_clip_match.group(3)}",
        )
    return effect_id.split(".ba", 1)[0], None


def validate_ba_stage_contract(
    skill: dict[str, Any], effect_id: str, stage_number: int
) -> None:
    """Validate a Battle Action stage for COMBO/HOLD/COUNTER presentations."""
    stages = skill.get("comboStages", [])
    if not isinstance(stages, list) or not stages:
        raise ValueError(f"Effect has BA stage without action-stage contract: {effect_id}")
    if stage_number < 1 or stage_number > len(stages):
        raise ValueError(f"action stage is outside gameplay contract: {effect_id}")


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
        component_display_name = bounded_runtime_display_name(
            file_name.removesuffix(".wfx.json")
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
            "displayName": component_display_name,
            "componentType": kind,
            "source": source_metadata,
            "emitters": emitters,
            "document": {
                "schema": "lostark.effect-authoring",
                "version": int(document.get("version", 0)),
                "effectAssetId": component_id,
                "displayName": component_display_name,
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
    character_class: str = "DimensionMaster",
) -> tuple[list[str], list[str]]:
    """Remove only generated class-owned artifacts absent from the catalog."""
    if character_class not in PRODUCT_CLASS_CONTRACTS:
        raise ValueError(f"unsupported product Effect class: {character_class}")
    prefix = f"effect.{character_class.casefold()}.skill."
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
    character_class: str = "DimensionMaster",
    animevents_path: Path | None = None,
) -> dict[str, Any]:
    catalog = read_json(catalog_path)
    stable_class = PRODUCT_CLASS_CONTRACTS.get(character_class)
    if stable_class is None:
        raise ValueError(f"unsupported product Effect class: {character_class}")
    if skill_bindings_path is None:
        skill_bindings_path = (
            data_root / "Animation" / "Authored" / character_class /
            f"{character_class}.skillbindings.json"
        )
    if animevents_path is None:
        animevents_path = (
            data_root / "Animation" / "Authored" / character_class /
            f"{character_class}.animevents"
        )
    class_prefix = f"effect.{character_class.casefold()}.skill."
    class_effect_ids = {
        str(entry["effectAssetId"])
        for entry in catalog.get("effects", [])
        if str(entry.get("effectAssetId") or "").startswith(class_prefix)
    }
    admitted_skills = admitted_skills_for_class(
        data_root / "Balance" / "PlayerSkills.json",
        skill_bindings_path,
        animevents_path,
        character_class,
        class_effect_ids,
    )
    skill_by_effect = {
        str(row["effectAssetId"]): row for row in admitted_skills
    }
    receipts = []
    stale_component_files: list[str] = []
    ignored_candidate_effects: list[str] = []
    action_recipe_by_skill: dict[int, dict[str, Any] | None] = {}
    expected_effect_ids = class_effect_ids
    for entry in catalog.get("effects", []):
        effect_id = str(entry["effectAssetId"])
        if not effect_id.startswith(class_prefix):
            continue
        base_id, authored_variant = admitted_effect_variant(effect_id)
        skill = skill_by_effect.get(base_id)
        if skill is None:
            ignored_candidate_effects.append(effect_id)
            continue
        slot = str(skill.get("inputSlot") or "") or None
        combo_stage = None
        if effect_id != base_id:
            ba_match = re.fullmatch(
                re.escape(base_id)
                + r"\.ba([1-9][0-9]*)(?:\.clip([1-9][0-9]*))?",
                effect_id,
            )
            if ba_match is not None:
                combo_stage = int(ba_match.group(1))
                validate_ba_stage_contract(skill, effect_id, combo_stage)
            elif authored_variant is None:
                raise ValueError(f"invalid combo stage Effect identity: {effect_id}")
        authoring_path = data_root / str(entry["authoringPath"])
        document = read_json(authoring_path)
        assembly, component_files = split_document(
            document, character_class, slot
        )
        assembly["sourceDocumentFileSha256"] = sha256_file(authoring_path)
        skill_id = int(skill["skillId"])
        if skill_id not in action_recipe_by_skill:
            action_path = (
                data_root / "Effects" / "Imported" / character_class /
                "Converted" / f"skill.{skill_id}.action-cue-recipe.json"
            )
            action_recipe_by_skill[skill_id] = (
                read_json(action_path) if action_path.is_file() else None
            )
        action_recipe = action_recipe_by_skill[skill_id]
        source_action_cues = (
            [] if authored_variant is not None or action_recipe is None
            else action_cues_for_effect(action_recipe, effect_id)
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
        directory = component_directory(effect_id, character_class)
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
            component_root, assembly_root, expected_effect_ids,
            character_class,
        )
    )
    return {
        "schema": "lostark.effect-component-build-receipt",
        "version": 1,
        "characterClass": stable_class,
        "animationAssetId": character_class,
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
    character_class: str = "DimensionMaster",
    skill_bindings_path: Path | None = None,
    animevents_path: Path | None = None,
) -> dict[str, Any]:
    if character_class not in PRODUCT_CLASS_CONTRACTS:
        raise ValueError(f"unsupported product Effect class: {character_class}")
    catalog = read_json(catalog_path)
    if skill_bindings_path is None:
        skill_bindings_path = (
            data_root / "Animation" / "Authored" / character_class /
            f"{character_class}.skillbindings.json"
        )
    if animevents_path is None:
        animevents_path = (
            data_root / "Animation" / "Authored" / character_class /
            f"{character_class}.animevents"
        )
    class_prefix = f"effect.{character_class.casefold()}.skill."
    class_effect_ids = {
        str(entry.get("effectAssetId") or "")
        for entry in catalog.get("effects", [])
        if str(entry.get("effectAssetId") or "").startswith(class_prefix)
    }
    admitted_skills_for_class(
        data_root / "Balance" / "PlayerSkills.json",
        skill_bindings_path,
        animevents_path,
        character_class,
        class_effect_ids,
    )
    if not class_effect_ids:
        unexpected_artifacts = []
        if component_root.exists():
            for path in component_root.rglob("*.wfx.json"):
                component = read_json(path)
                source_effect_id = str(
                    component.get("source", {}).get("effectAssetId") or ""
                )
                if source_effect_id.startswith(class_prefix):
                    unexpected_artifacts.append(str(path))
        if assembly_root.exists():
            for path in assembly_root.glob("*.assembly.json"):
                assembly = read_json(path)
                effect_id = str(assembly.get("effectAssetId") or "")
                if effect_id.startswith(class_prefix):
                    unexpected_artifacts.append(str(path))
        if unexpected_artifacts:
            raise ValueError(
                "unadmitted generated Effect artifacts exist: "
                f"{sorted(unexpected_artifacts)}"
            )

        if receipt_path.is_file():
            receipt = read_json(receipt_path)
            if (
                receipt.get("schema") != "lostark.effect-component-build-receipt"
                or receipt.get("version") != 1
                or receipt.get("animationAssetId") != character_class
                or receipt.get("characterClass")
                != PRODUCT_CLASS_CONTRACTS[character_class]
                or receipt.get("effects") != []
                or any(
                    int(receipt.get(name, -1)) != 0
                    for name in (
                        "effectCount", "componentCount", "emitterCount",
                        "sourceActionCueCount",
                    )
                )
                or receipt.get("compileIdentityComplete") is not True
            ):
                raise ValueError(
                    "zero-product Effect component receipt mismatch: "
                    f"{character_class}"
                )
        return {
            "effectCount": 0,
            "componentCount": 0,
            "emitterCount": 0,
            "sourceActionCueCount": 0,
            "compileIdentityComplete": True,
        }

    receipt = read_json(receipt_path)
    if (
        receipt.get("schema") != "lostark.effect-component-build-receipt"
        or receipt.get("version") != 1
    ):
        raise ValueError("unsupported Effect component build receipt")
    receipt_effect_ids = [
        str(row.get("effectAssetId") or "")
        for row in receipt.get("effects", [])
        if isinstance(row, dict)
    ]
    if (
        len(receipt_effect_ids) != len(set(receipt_effect_ids))
        or set(receipt_effect_ids) != class_effect_ids
    ):
        raise ValueError(
            "Effect component receipt catalog coverage mismatch: "
            f"missing={sorted(class_effect_ids - set(receipt_effect_ids))}, "
            f"extra={sorted(set(receipt_effect_ids) - class_effect_ids)}"
        )
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
            or component_directory(effect_id, character_class)
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
        "--character-class",
        choices=tuple(PRODUCT_CLASS_CONTRACTS),
        default="DimensionMaster",
        help="Animation asset/class owner to compile.",
    )
    parser.add_argument(
        "--all-product-classes", action="store_true",
        help="Compile or verify every supported product Effect class.",
    )
    parser.add_argument(
        "--skill-bindings", type=Path,
        default=None,
    )
    parser.add_argument(
        "--animevents", type=Path,
        default=None,
    )
    parser.add_argument(
        "--component-root", type=Path,
        default=None,
    )
    parser.add_argument(
        "--assembly-root", type=Path,
        default=None,
    )
    parser.add_argument(
        "--receipt", type=Path,
        default=None,
    )
    parser.add_argument(
        "--receipt-root", type=Path,
        default=None,
        help=(
            "Receipt directory for --all-product-classes.  This keeps "
            "generated product receipts outside read-only Imported evidence."
        ),
    )
    parser.add_argument(
        "--verify-existing", action="store_true",
        help="Validate existing assemblies/components without writing files."
    )
    args = parser.parse_args()
    custom_paths = (
        args.skill_bindings,
        args.animevents,
        args.component_root,
        args.assembly_root,
        args.receipt,
    )
    if args.all_product_classes and any(path is not None for path in custom_paths):
        parser.error(
            "--all-product-classes does not accept class-specific paths"
        )
    if args.receipt_root is not None and not args.all_product_classes:
        parser.error("--receipt-root requires --all-product-classes")

    classes = (
        tuple(PRODUCT_CLASS_CONTRACTS)
        if args.all_product_classes
        else (args.character_class,)
    )
    results: dict[str, dict[str, Any]] = {}
    for character_class in classes:
        skill_bindings = args.skill_bindings or (
            args.data_root / "Animation" / "Authored" / character_class /
            f"{character_class}.skillbindings.json"
        )
        animevents = args.animevents or (
            args.data_root / "Animation" / "Authored" / character_class /
            f"{character_class}.animevents"
        )
        component_root = args.component_root or (
            args.data_root / "Effects" / "Components" / character_class
        )
        assembly_root = args.assembly_root or (
            args.data_root / "Effects" / "Assemblies" / character_class
        )
        receipt_path = args.receipt or (
            args.receipt_root / f"{character_class}.component-build.receipt.json"
            if args.receipt_root is not None
            else args.data_root / "Effects" / "Imported" / character_class /
            f"{character_class}.component-build.receipt.json"
        )
        if args.verify_existing:
            result = verify_existing(
                args.catalog, args.data_root, component_root,
                assembly_root, receipt_path, character_class,
                skill_bindings, animevents,
            )
        else:
            receipt = build_all(
                args.catalog, args.data_root, component_root,
                assembly_root, skill_bindings, character_class,
                animevents,
            )
            write_json_atomic(receipt_path, receipt)
            result = {
                key: receipt[key] for key in (
                    "effectCount", "componentCount", "emitterCount",
                    "sourceActionCueCount", "compileIdentityComplete",
                )
            }
        results[character_class] = result
    if len(results) == 1:
        print(json.dumps(next(iter(results.values())), sort_keys=True))
    else:
        print(json.dumps({
            "classes": results,
            "effectCount": sum(
                row["effectCount"] for row in results.values()
            ),
            "componentCount": sum(
                row["componentCount"] for row in results.values()
            ),
            "emitterCount": sum(
                row["emitterCount"] for row in results.values()
            ),
            "sourceActionCueCount": sum(
                row["sourceActionCueCount"] for row in results.values()
            ),
            "compileIdentityComplete": all(
                row["compileIdentityComplete"] for row in results.values()
            ),
        }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
