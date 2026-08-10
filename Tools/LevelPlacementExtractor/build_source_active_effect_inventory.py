#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


RENDERER_ORDER = (
    "MeshParticle",
    "SpriteParticle",
    "DecalParticle",
    "CascadeRibbon",
    "LightParticle",
    "ScreenPost",
    "StandaloneMesh",
)
TIME_TOLERANCE_SECONDS = 0.0001


class InventoryError(ValueError):
    pass


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise InventoryError(message)


def _normal(value: object) -> str:
    return str(value or "").strip().casefold()


def _canonical_sha256(value: object) -> str:
    encoded = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()


def _load_json(path: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    raw = path.read_bytes()
    try:
        document = json.loads(raw.decode("utf-8-sig"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise InventoryError(f"invalid JSON: {path}: {error}") from error
    _require(isinstance(document, dict), f"JSON root must be an object: {path}")
    return document, {
        "artifactName": path.name,
        "bytes": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
    }


def _particle_system_path(cue: dict[str, Any]) -> str:
    references = [
        item
        for item in cue.get("assetReferences", [])
        if _normal(item.get("className")) == "particlesystem"
    ]
    _require(
        len(references) == 1,
        f"cue must have exactly one ParticleSystem reference: {cue.get('cueId')}",
    )
    object_path = str(references[0].get("objectPath") or "").strip()
    _require(object_path != "", f"empty ParticleSystem path: {cue.get('cueId')}")
    return object_path


def _renderer_type(conversion: dict[str, Any]) -> str:
    target_kind = _normal(conversion.get("targetKind"))
    renderer_shape = _normal(conversion.get("rendererShape"))
    typed_data = {
        _normal(item.get("className"))
        for item in conversion.get("moduleEvidence", [])
        if "typedata" in _normal(item.get("className"))
    }

    if "particlemoduletypedataribbon" in typed_data:
        _require(
            target_kind == "particle" and renderer_shape == "sprite",
            f"Ribbon TypeData conflicts with conversion kind: {conversion.get('sourceEmitter')}",
        )
        return "CascadeRibbon"

    table = {
        ("particle", "mesh"): "MeshParticle",
        ("particle", "sprite"): "SpriteParticle",
        ("decal", "decal"): "DecalParticle",
        ("light", "light"): "LightParticle",
        ("screenpost", "screenpost"): "ScreenPost",
        ("mesh", "mesh"): "StandaloneMesh",
    }
    renderer = table.get((target_kind, renderer_shape))
    _require(
        renderer is not None,
        f"unsupported renderer classification {target_kind}/{renderer_shape}: "
        f"{conversion.get('sourceEmitter')}",
    )

    required_type_data = {
        "MeshParticle": "particlemoduletypedatamesh",
        "DecalParticle": "efparticlemoduletypedatadecal",
        "LightParticle": "efparticlemoduletypedatalight",
    }
    expected = required_type_data.get(renderer)
    if expected:
        _require(
            expected in typed_data,
            f"{renderer} is missing {expected}: {conversion.get('sourceEmitter')}",
        )
    if renderer == "SpriteParticle":
        _require(
            not typed_data,
            f"SpriteParticle has unclassified TypeData {sorted(typed_data)}: "
            f"{conversion.get('sourceEmitter')}",
        )
    return renderer


def _suffix_matches(target: str, candidate: str) -> bool:
    target_key = _normal(target)
    candidate_key = _normal(candidate)
    return (
        target_key == candidate_key
        or target_key.endswith("." + candidate_key)
        or candidate_key.endswith("." + target_key)
    )


def _resolve_one_record(
    target: str,
    candidates: Iterable[tuple[dict[str, Any], dict[str, Any]]],
    *,
    context: str,
) -> tuple[dict[str, Any], dict[str, Any]]:
    matches = [
        (record, evidence)
        for record, evidence in candidates
        if _suffix_matches(target, str(record.get("objectPath") or ""))
    ]
    _require(
        len(matches) == 1,
        f"{context} must resolve exactly once: {target}; matches={len(matches)}",
    )
    return matches[0]


def _node_evidence(node: dict[str, Any]) -> dict[str, Any]:
    return {
        "nodeId": node.get("nodeId"),
        "className": node.get("className"),
        "objectPath": node.get("objectPath"),
        "recordSha256": _canonical_sha256(node),
    }


def _build_module_resolver(
    graph: dict[str, Any], module_closure: dict[str, Any]
):
    candidates: list[tuple[dict[str, Any], dict[str, Any]]] = []
    for node in graph.get("nodes", []):
        candidates.append(
            (
                node,
                {
                    "sourceDocument": "normalizedGraph",
                    "nodeId": node.get("nodeId"),
                },
            )
        )
    for package in module_closure.get("packages", []):
        for record in package.get("objects", []):
            candidates.append(
                (
                    record,
                    {
                        "sourceDocument": "externalModuleClosure",
                        "objectId": record.get("objectId"),
                        "logicalPackage": package.get("logicalPackage"),
                        "physicalPackage": package.get("physicalPackage"),
                    },
                )
            )

    def resolve(module: dict[str, Any]) -> dict[str, Any]:
        source_path = str(module.get("objectPath") or "")
        record, origin = _resolve_one_record(
            source_path,
            candidates,
            context="module evidence",
        )
        expected_class = _normal(module.get("className"))
        actual_class = _normal(record.get("className"))
        _require(
            expected_class == actual_class,
            f"module class mismatch for {source_path}: {expected_class} != {actual_class}",
        )
        result = {
            "className": module.get("className"),
            "objectPath": source_path,
            "resolvedObjectPath": record.get("objectPath"),
            "recordSha256": _canonical_sha256(record),
        }
        result.update(origin)
        return result

    return resolve


def _shader_graph_evidence(material: dict[str, Any]) -> dict[str, Any] | None:
    source_kind = "materialGraph" if material.get("materialGraph") else "parentGraph"
    wrapper = material.get(source_kind)
    if not isinstance(wrapper, dict):
        return None
    graph = wrapper.get("graph") if isinstance(wrapper.get("graph"), dict) else wrapper
    summary = graph.get("summary") if isinstance(graph.get("summary"), dict) else {}
    return {
        "source": source_kind,
        "logicalPackage": wrapper.get("logicalPackage"),
        "physicalPackage": wrapper.get("physicalPackage"),
        "physicalPackageSha256": wrapper.get("physicalPackageSha256"),
        "materialPath": graph.get("materialPath"),
        "graphRecordSha256": _canonical_sha256(graph),
        "expressionEntryCount": summary.get("expressionEntryCount"),
        "nonNullExpressionCount": summary.get("nonNullExpressionCount"),
        "nullExpressionCount": summary.get("nullExpressionCount"),
        "unresolvedInputEdgeCount": summary.get("unresolvedInputEdgeCount"),
        "topologyStatus": summary.get("topologyStatus"),
        "runtimeExactEligible": summary.get("runtimeExactEligible"),
    }


def _compact_material(material: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {
        "sourceMaterialPath": material.get("sourceMaterialPath"),
        "sourceLogicalPackage": material.get("sourceLogicalPackage"),
        "rendererShapes": material.get("rendererShapes", []),
        "status": material.get("status"),
        "runtimeExactEligible": material.get("runtimeExactEligible", False),
        "sourcePhysicalPackage": material.get("sourcePhysicalPackage"),
        "sourcePhysicalPackageSha256": material.get("sourcePhysicalPackageSha256"),
        "materialClosureRecordSha256": _canonical_sha256(material),
    }
    source_material = material.get("material")
    if isinstance(source_material, dict):
        result["sourceMaterial"] = {
            key: source_material.get(key)
            for key in (
                "objectPath",
                "className",
                "exportIndex",
                "serialSize",
                "parent",
                "scalarParameters",
                "textureParameters",
                "vectorParameters",
                "overrideTwoSided",
                "hasStaticPermutationResource",
            )
            if key in source_material
        }
    shader = _shader_graph_evidence(material)
    if shader is not None:
        result["shaderGraph"] = shader
    return result


def _safe_resource_file(resource_root: Path, asset_id: str) -> Path:
    normalized = asset_id.replace("\\", "/")
    relative = PurePosixPath(normalized)
    _require(
        normalized != ""
        and not relative.is_absolute()
        and ".." not in relative.parts
        and ":" not in relative.parts[0],
        f"unsafe runtime asset id: {asset_id}",
    )
    root = resource_root.resolve()
    candidate = root.joinpath(*relative.parts).resolve()
    _require(
        candidate == root or root in candidate.parents,
        f"runtime asset escapes resource root: {asset_id}",
    )
    return candidate


def _resource_identity(resource_root: Path, asset_id: str) -> dict[str, Any]:
    path = _safe_resource_file(resource_root, asset_id)
    _require(path.is_file(), f"runtime resource is missing: {asset_id}")
    raw = path.read_bytes()
    suffix = path.suffix.casefold()
    resource_kind = {
        ".wmodel": "WModel",
        ".dds": "TextureDDS",
    }.get(suffix, "Other")
    return {
        "assetId": asset_id.replace("\\", "/"),
        "resourceKind": resource_kind,
        "bytes": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
    }


def _cue_evidence(cue: dict[str, Any], source_system_id: str) -> dict[str, Any]:
    payload = cue.get("typedPayload")
    _require(isinstance(payload, dict), f"cue typedPayload missing: {cue.get('cueId')}")
    attachment = payload.get("attachment")
    _require(isinstance(attachment, dict), f"cue attachment missing: {cue.get('cueId')}")
    mode = str(attachment.get("mode") or "")
    _require(
        mode in {"FOLLOW_NAMED_ANCHORS", "SNAPSHOT_ROOT"},
        f"unsupported attachment mode {mode}: {cue.get('cueId')}",
    )
    if mode == "FOLLOW_NAMED_ANCHORS":
        _require(
            attachment.get("runtimeResolutionStatus") == "EXACT_SOURCE_SOCKET"
            and attachment.get("runtimeAnchorSlotId")
            and attachment.get("runtimeBoneName"),
            f"follow cue lacks exact runtime socket: {cue.get('cueId')}",
        )
    else:
        _require(
            attachment.get("runtimeResolutionStatus") == "EXACT_ROOT_SNAPSHOT",
            f"snapshot cue lacks exact root resolution: {cue.get('cueId')}",
        )
    return {
        "cueId": cue.get("cueId"),
        "sourceSystemId": source_system_id,
        "globalTimeSeconds": cue.get("globalTimeSeconds"),
        "durationSeconds": cue.get("durationSeconds"),
        "executionEnabled": bool(cue.get("executionEnabled")),
        "sourceReceiptEventIndex": cue.get("sourceReceiptEventIndex"),
        "sourceExecutionStatus": cue.get("sourceExecutionStatus"),
        "sourceEnabledByteOffset": payload.get("sourceByteOffset"),
        "sourceEnabledByteValue": payload.get("sourceByteValue"),
        "attachment": attachment,
        "localTransform": payload.get("localTransform"),
    }


def _parse_expected_counts(values: Iterable[str]) -> dict[str, int]:
    result: dict[str, int] = {}
    for value in values:
        renderer, separator, count_text = value.partition("=")
        _require(separator == "=", f"expected count must use Renderer=count: {value}")
        renderer = renderer.strip()
        _require(renderer in RENDERER_ORDER, f"unknown renderer in expected count: {renderer}")
        _require(renderer not in result, f"duplicate expected renderer count: {renderer}")
        try:
            count = int(count_text)
        except ValueError as error:
            raise InventoryError(f"invalid expected renderer count: {value}") from error
        _require(count >= 0, f"negative expected renderer count: {value}")
        result[renderer] = count
    return result


def build_source_active_effect_inventory(
    *,
    action_cue_recipe_path: Path,
    conversion_receipt_path: Path,
    normalized_graph_path: Path,
    module_closure_path: Path,
    material_closure_path: Path,
    resource_root: Path,
    expected_renderer_counts: dict[str, int] | None = None,
    expected_active_element_count: int | None = None,
    expected_excluded_legacy_element_count: int | None = None,
    allow_legacy_conversion_input_slot_mismatch: bool = False,
) -> dict[str, Any]:
    action, action_identity = _load_json(action_cue_recipe_path)
    conversion, conversion_identity = _load_json(conversion_receipt_path)
    graph, graph_identity = _load_json(normalized_graph_path)
    module_closure, module_identity = _load_json(module_closure_path)
    material_closure, material_identity = _load_json(material_closure_path)

    _require(
        action.get("schema") == "lostark.effect-action-cue-recipe",
        "unexpected action cue recipe schema",
    )
    _require(
        conversion.get("schema") == "lostark.imported-effect-element-conversion-receipt",
        "unexpected conversion receipt schema",
    )
    _require(
        graph.get("schema") == "lostark.normalized-effect-source-graph",
        "unexpected normalized graph schema",
    )
    _require(
        module_closure.get("schema") == "lostark.ue3-particle-external-module-closure",
        "unexpected module closure schema",
    )
    _require(
        material_closure.get("schema") == "lostark.ue3-effect-material-closure",
        "unexpected material closure schema",
    )

    character_class = str(action.get("characterClass") or "")
    skill_id = int(action.get("skillId"))
    input_slot = str(action.get("inputSlot") or "")
    for label, document in (
        ("conversion", conversion),
        ("normalized graph", graph),
        ("module closure", module_closure),
        ("material closure", material_closure),
    ):
        _require(
            _normal(document.get("characterClass")) == _normal(character_class)
            and int(document.get("skillId")) == skill_id,
            f"{label} characterClass/skillId does not match action recipe",
        )
    _require(
        _normal(material_closure.get("inputSlot")) == _normal(input_slot),
        "material closure inputSlot does not match action recipe",
    )
    conversion_input_slot = str(conversion.get("inputSlot") or "")
    conversion_slot_matches = _normal(conversion_input_slot) == _normal(input_slot)
    _require(
        conversion_slot_matches or allow_legacy_conversion_input_slot_mismatch,
        "conversion receipt inputSlot does not match action recipe; use the explicit legacy "
        "mismatch flag only for a provenance-preserved stale receipt",
    )

    action_summary = action.get("summary", {})
    _require(action.get("sourceExtractionComplete") is True, "action source extraction incomplete")
    _require(action_summary.get("referenceJoinComplete") is True, "action/reference join incomplete")
    _require(
        action_summary.get("runtimeAnchorResolutionComplete") is True,
        "action runtime anchor resolution incomplete",
    )
    closure_summary = module_closure.get("summary", {})
    _require(
        int(closure_summary.get("unresolvedRequestCount", -1)) == 0
        and int(closure_summary.get("propertyErrorCount", -1)) == 0,
        "external module closure is not complete",
    )

    particle_cues: list[tuple[int, dict[str, Any], str]] = []
    cue_groups: dict[str, list[tuple[int, dict[str, Any], str]]] = defaultdict(list)
    for cue_index, cue in enumerate(action.get("cues", [])):
        if cue.get("sourceType") != "PlayParticleEffect":
            continue
        cue_id = str(cue.get("cueId") or "")
        _require(cue_id != "", "PlayParticleEffect cue is missing cueId")
        source_system_id = _particle_system_path(cue)
        item = (cue_index, cue, source_system_id)
        particle_cues.append(item)
        cue_groups[_normal(source_system_id)].append(item)
    _require(particle_cues, "action recipe contains no PlayParticleEffect cues")
    particle_cue_ids = [str(cue.get("cueId")) for _, cue, _ in particle_cues]
    _require(
        len(set(particle_cue_ids)) == len(particle_cue_ids),
        "PlayParticleEffect cueId values must be unique",
    )
    enabled_cue_count = sum(
        1 for _, cue, _ in particle_cues if cue.get("executionEnabled")
    )
    disabled_cue_count = len(particle_cues) - enabled_cue_count
    _require(
        int(action_summary.get("enabledPlayParticleEffectCount", enabled_cue_count))
        == enabled_cue_count
        and int(action_summary.get("disabledPlayParticleEffectCount", disabled_cue_count))
        == disabled_cue_count,
        "action summary execution counts do not match the cue execution bits",
    )

    conversion_rows = conversion.get("elementConversions", [])
    _require(isinstance(conversion_rows, list) and conversion_rows, "conversion receipt is empty")
    conversion_systems = {_normal(row.get("sourceSystemId")) for row in conversion_rows}
    _require(
        conversion_systems == set(cue_groups),
        "conversion source systems do not exactly match action particle cue systems",
    )

    graph_systems = {
        _normal(item.get("sourceSystemId")): item for item in graph.get("sourceSystems", [])
    }
    _require(
        len(graph_systems) == len(graph.get("sourceSystems", [])),
        "normalized graph contains duplicate sourceSystemId values",
    )
    _require(
        conversion_systems == set(graph_systems),
        "normalized graph source systems do not exactly match conversion systems",
    )
    graph_nodes_by_id = {item.get("nodeId"): item for item in graph.get("nodes", [])}
    _require(None not in graph_nodes_by_id, "normalized graph node is missing nodeId")

    enabled_systems = {
        source_key
        for source_key, cues in cue_groups.items()
        if any(bool(cue.get("executionEnabled")) for _, cue, _ in cues)
    }
    material_active_systems = {
        _normal(value) for value in material_closure.get("activeSourceSystemIds", [])
    }
    _require(
        enabled_systems == material_active_systems,
        "material closure active source systems do not match enabled action systems",
    )
    material_index = {
        _normal(item.get("sourceMaterialPath")): item
        for item in material_closure.get("materials", [])
    }
    _require(
        len(material_index) == len(material_closure.get("materials", [])),
        "material closure contains duplicate material paths",
    )

    resolve_module = _build_module_resolver(graph, module_closure)
    active_elements: list[dict[str, Any]] = []
    excluded_elements: list[dict[str, Any]] = []
    active_cue_index: dict[str, dict[str, Any]] = {}
    disabled_cue_index: dict[str, dict[str, Any]] = {}
    referenced_material_paths: set[str] = set()
    referenced_resource_ids: set[str] = set()

    for row_index, conversion_row in enumerate(conversion_rows):
        source_system_id = str(conversion_row.get("sourceSystemId") or "")
        source_key = _normal(source_system_id)
        cues = cue_groups[source_key]
        occurrences = conversion_row.get("eventOccurrences", [])
        legacy_ids = conversion_row.get("elementIds", [])
        _require(
            len(cues) == len(occurrences) == len(legacy_ids),
            f"cue/occurrence/legacy element cardinality mismatch: "
            f"{conversion_row.get('sourceEmitter')}",
        )
        renderer = _renderer_type(conversion_row)
        row_has_enabled_occurrence = any(
            bool(cue.get("executionEnabled")) for _, cue, _ in cues
        )

        graph_system = graph_systems[source_key]
        emitter_node: dict[str, Any] | None = None
        lod_node: dict[str, Any] | None = None
        resolved_modules: list[dict[str, Any]] = []
        unresolved_runtime_modules: list[dict[str, Any]] = []
        material_paths: list[str] = []
        resource_mappings: list[dict[str, Any]] = []
        if row_has_enabled_occurrence:
            system_nodes = [
                (graph_nodes_by_id[node_id], {})
                for node_id in graph_system.get("nodeIds", [])
                if node_id in graph_nodes_by_id
            ]
            emitter_node, _ = _resolve_one_record(
                str(conversion_row.get("sourceEmitter") or ""),
                system_nodes,
                context="source emitter node",
            )
            lod_node, _ = _resolve_one_record(
                str(conversion_row.get("sourceLod") or ""),
                system_nodes,
                context="source LOD node",
            )
            resolved_modules = [
                resolve_module(module)
                for module in conversion_row.get("moduleEvidence", [])
            ]
            module_by_path = {
                _normal(module.get("objectPath")): module
                for module in resolved_modules
            }
            for unsupported in conversion_row.get("unrepresentedModules", []):
                evidence = module_by_path.get(_normal(unsupported.get("objectPath")))
                if evidence is None:
                    evidence = resolve_module(unsupported)
                unresolved_runtime_modules.append(
                    {
                        "className": unsupported.get("className"),
                        "objectPath": unsupported.get("objectPath"),
                        "reason": unsupported.get("reason"),
                        "sourceRecord": evidence,
                    }
                )

            for material in conversion_row.get("materialParameterEvidence", []):
                material_path = str(material.get("sourceMaterialPath") or "")
                material_key = _normal(material_path)
                _require(
                    material_key in material_index,
                    f"active conversion material is absent from material closure: {material_path}",
                )
                material_paths.append(material_path)
                referenced_material_paths.add(material_key)

            resource_mappings = conversion_row.get("resourceMappings", [])
            for mapping in resource_mappings:
                asset_id = str(mapping.get("assetId") or "")
                _require(asset_id != "", "resource mapping has an empty assetId")
                referenced_resource_ids.add(asset_id.replace("\\", "/"))

        for occurrence_index, ((cue_index, cue, cue_source_path), occurrence, legacy_id) in enumerate(
            zip(cues, occurrences, legacy_ids)
        ):
            _require(
                abs(float(cue.get("globalTimeSeconds", 0.0)) - float(occurrence.get("globalTimeSeconds", 0.0)))
                <= TIME_TOLERANCE_SECONDS,
                f"cue/occurrence time mismatch: {cue.get('cueId')} / {legacy_id}",
            )
            _require(
                abs(float(cue.get("durationSeconds", 0.0)) - float(occurrence.get("durationSeconds", 0.0)))
                <= TIME_TOLERANCE_SECONDS,
                f"cue/occurrence duration mismatch: {cue.get('cueId')} / {legacy_id}",
            )
            cue_record = _cue_evidence(cue, cue_source_path)
            cue_id = str(cue.get("cueId") or "")
            if cue.get("executionEnabled"):
                _require(
                    emitter_node is not None and lod_node is not None,
                    f"active conversion lacks graph node evidence: {legacy_id}",
                )
                active_cue_index[cue_id] = cue_record
                active_elements.append(
                    {
                        "_actionCueIndex": cue_index,
                        "cueId": cue_id,
                        "rendererType": renderer,
                        "sourceSystemId": source_system_id,
                        "sourceSystemGraphRecordSha256": _canonical_sha256(graph_system),
                        "sourceEmitter": conversion_row.get("sourceEmitter"),
                        "sourceEmitterNode": _node_evidence(emitter_node),
                        "sourceLod": conversion_row.get("sourceLod"),
                        "sourceLodNode": _node_evidence(lod_node),
                        "selectedLegacyElementId": legacy_id,
                        "legacyConversionRowIndex": row_index,
                        "legacyOccurrenceIndex": occurrence_index,
                        "conversionStatus": conversion_row.get("status"),
                        "sourceMaterialRuntimePending": conversion_row.get(
                            "sourceMaterialRuntimePending"
                        ),
                        "presentationSourceStatus": conversion_row.get(
                            "presentationSourceStatus"
                        ),
                        "presentationProfileId": conversion_row.get("presentationProfileId"),
                        "missingResources": conversion_row.get("missingResources", []),
                        "sourceMaterials": material_paths,
                        "materialParameterEvidence": conversion_row.get(
                            "materialParameterEvidence", []
                        ),
                        "resourceMappings": resource_mappings,
                        "detailMappings": conversion_row.get("detailMappings", []),
                        "burstSource": conversion_row.get("burstSource", []),
                        "moduleEvidence": resolved_modules,
                        "runtimeUnsupportedModules": unresolved_runtime_modules,
                    }
                )
            else:
                disabled_cue_index[cue_id] = cue_record
                excluded_elements.append(
                    {
                        "_actionCueIndex": cue_index,
                        "_legacyConversionRowIndex": row_index,
                        "legacyElementId": legacy_id,
                        "cueId": cue_id,
                        "rendererType": renderer,
                        "sourceSystemId": source_system_id,
                        "sourceEmitter": conversion_row.get("sourceEmitter"),
                        "sourceLod": conversion_row.get("sourceLod"),
                        "reason": "ACTION_EXECUTION_DISABLED",
                    }
                )

    active_elements.sort(
        key=lambda item: (
            item["_actionCueIndex"],
            item["legacyConversionRowIndex"],
            item["legacyOccurrenceIndex"],
            _normal(item["sourceEmitter"]),
        )
    )
    for active_index, item in enumerate(active_elements):
        item["activeElementId"] = f"source-active-{active_index:03d}"
        del item["_actionCueIndex"]
    excluded_elements.sort(
        key=lambda item: (
            item["_actionCueIndex"],
            item["_legacyConversionRowIndex"],
            _normal(item["sourceEmitter"]),
        )
    )
    for item in excluded_elements:
        del item["_actionCueIndex"]
        del item["_legacyConversionRowIndex"]

    active_renderer_counts = Counter(
        item["rendererType"] for item in active_elements
    )
    source_partition_counts = Counter(_renderer_type(row) for row in conversion_rows)
    legacy_flattened_counts = Counter()
    for row in conversion_rows:
        legacy_flattened_counts[_renderer_type(row)] += len(row.get("elementIds", []))

    expected_renderer_counts = expected_renderer_counts or {}
    for renderer, count in expected_renderer_counts.items():
        actual = active_renderer_counts.get(renderer, 0)
        _require(
            actual == count,
            f"active {renderer} count mismatch: expected={count}, actual={actual}",
        )
    if expected_active_element_count is not None:
        _require(
            len(active_elements) == expected_active_element_count,
            f"active element count mismatch: expected={expected_active_element_count}, "
            f"actual={len(active_elements)}",
        )
    if expected_excluded_legacy_element_count is not None:
        _require(
            len(excluded_elements) == expected_excluded_legacy_element_count,
            "excluded legacy element count mismatch: "
            f"expected={expected_excluded_legacy_element_count}, actual={len(excluded_elements)}",
        )

    material_evidence = [
        _compact_material(material_index[key]) for key in sorted(referenced_material_paths)
    ]
    _require(
        referenced_material_paths == set(material_index),
        "material closure contains material records not referenced by the active inventory",
    )
    runtime_resources = [
        _resource_identity(resource_root, asset_id)
        for asset_id in sorted(referenced_resource_ids, key=str.casefold)
    ]
    material_topology_counts = Counter(
        item.get("shaderGraph", {}).get("topologyStatus")
        or (
            "NON_RENDER_BUILTIN"
            if item.get("status") == "SOURCE_BUILTIN_NON_RENDER_MATERIAL"
            else "NO_SHADER_GRAPH_EVIDENCE"
        )
        for item in material_evidence
    )
    resource_kind_counts = Counter(
        item["resourceKind"] for item in runtime_resources
    )
    unsupported_module_class_counts = Counter(
        str(module.get("className") or "")
        for item in active_elements
        for module in item["runtimeUnsupportedModules"]
    )

    active_source_systems = []
    for source_key in sorted(enabled_systems):
        system = graph_systems[source_key]
        active_source_systems.append(
            {
                "sourceSystemId": system.get("sourceSystemId"),
                "sourceAsset": system.get("sourceAsset"),
                "logicalPackage": system.get("logicalPackage"),
                "objectName": system.get("objectName"),
                "objectPath": system.get("objectPath"),
                "rootNodeId": system.get("rootNodeId"),
                "nodeCount": len(system.get("nodeIds", [])),
                "recordSha256": _canonical_sha256(system),
            }
        )

    return {
        "schema": "lostark.source-active-effect-inventory-receipt",
        "formatVersion": 1,
        "characterClass": character_class,
        "skillId": skill_id,
        "inputSlot": input_slot,
        "status": "SOURCE_ACTIVE_INVENTORY_ONLY",
        "productEmissionAllowed": False,
        "visualApprovalStatus": "MANUAL_VISUAL_PENDING",
        "selectionPolicy": {
            "activeCuePredicate": "sourceType == PlayParticleEffect && executionEnabled == true",
            "occurrenceJoin": "action cue order and time/duration must match each conversion eventOccurrence",
            "elementPolicy": "one active item per enabled action occurrence and source emitter/LOD partition",
            "rendererPolicy": "TypeData overrides the legacy rendererShape; Ribbon is never flattened to Sprite",
            "legacyElementIdsAreDiagnosticOnly": True,
            "automaticVisualApproval": False,
        },
        "source": {
            "actionCueRecipe": action_identity,
            "elementConversionReceipt": conversion_identity,
            "normalizedGraph": graph_identity,
            "externalModuleClosure": module_identity,
            "activeMaterialClosure": material_identity,
            "conversionInputSlot": conversion_input_slot,
            "conversionInputSlotStatus": (
                "MATCHES_ACTION_RECIPE"
                if conversion_slot_matches
                else "LEGACY_STALE_VALUE_PRESERVED"
            ),
        },
        "activeSourceSystems": active_source_systems,
        "activeCues": [
            active_cue_index[str(cue.get("cueId") or "")]
            for _, cue, _ in particle_cues
            if cue.get("executionEnabled")
        ],
        "activeElements": active_elements,
        "materialEvidence": material_evidence,
        "runtimeResourceIdentities": runtime_resources,
        "excludedExecutionDisabled": {
            "disabledCues": [
                disabled_cue_index[str(cue.get("cueId") or "")]
                for _, cue, _ in particle_cues
                if not cue.get("executionEnabled")
            ],
            "legacyElements": excluded_elements,
        },
        "summary": {
            "playParticleCueCount": len(particle_cues),
            "enabledPlayParticleCueCount": len(active_cue_index),
            "disabledPlayParticleCueCount": len(disabled_cue_index),
            "sourceSystemCount": len(conversion_systems),
            "activeSourceSystemCount": len(enabled_systems),
            "sourcePartitionCount": len(conversion_rows),
            "sourcePartitionRendererCounts": {
                key: source_partition_counts.get(key, 0)
                for key in RENDERER_ORDER
                if source_partition_counts.get(key, 0)
            },
            "legacyFlattenedElementCount": sum(
                len(row.get("elementIds", [])) for row in conversion_rows
            ),
            "legacyFlattenedRendererCounts": {
                key: legacy_flattened_counts.get(key, 0)
                for key in RENDERER_ORDER
                if legacy_flattened_counts.get(key, 0)
            },
            "activeElementCount": len(active_elements),
            "activeRendererCounts": {
                key: active_renderer_counts.get(key, 0)
                for key in RENDERER_ORDER
                if active_renderer_counts.get(key, 0)
            },
            "excludedExecutionDisabledLegacyElementCount": len(excluded_elements),
            "materialEvidenceCount": len(material_evidence),
            "materialShaderTopologyStatusCounts": dict(
                sorted(material_topology_counts.items())
            ),
            "runtimeExactEligibleMaterialCount": sum(
                1
                for item in material_evidence
                if item.get("runtimeExactEligible") is True
                or item.get("shaderGraph", {}).get("runtimeExactEligible") is True
            ),
            "runtimeResourceIdentityCount": len(runtime_resources),
            "runtimeResourceKindCounts": dict(sorted(resource_kind_counts.items())),
            "runtimeUnsupportedModuleOccurrenceCount": sum(
                len(item["runtimeUnsupportedModules"]) for item in active_elements
            ),
            "runtimeUnsupportedModuleClassCounts": dict(
                sorted(unsupported_module_class_counts.items())
            ),
            "sourceExtractionComplete": True,
            "runtimeExecutionComplete": False,
            "visualApprovalComplete": False,
        },
    }


def _serialized_json(document: dict[str, Any]) -> bytes:
    text = json.dumps(document, ensure_ascii=False, indent=2) + "\n"
    return text.replace("\n", "\r\n").encode("utf-8")


def check_or_write_json(
    path: Path,
    document: dict[str, Any],
    *,
    check: bool = False,
) -> None:
    expected = _serialized_json(document)
    if check:
        _require(path.is_file(), f"generated receipt is missing: {path}")
        _require(
            path.read_bytes() == expected,
            f"generated receipt is stale: {path}",
        )
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(expected)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build an execution-bit-filtered source effect inventory receipt."
    )
    parser.add_argument("--action-cue-recipe", required=True, type=Path)
    parser.add_argument("--conversion-receipt", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--module-closure", required=True, type=Path)
    parser.add_argument("--material-closure", required=True, type=Path)
    parser.add_argument("--resource-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--expected-renderer-count", action="append", default=[])
    parser.add_argument("--expected-active-element-count", type=int)
    parser.add_argument("--expected-excluded-legacy-element-count", type=int)
    parser.add_argument(
        "--allow-legacy-conversion-input-slot-mismatch",
        action="store_true",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Verify the tracked output byte-for-byte without rewriting it.",
    )
    args = parser.parse_args()

    document = build_source_active_effect_inventory(
        action_cue_recipe_path=args.action_cue_recipe,
        conversion_receipt_path=args.conversion_receipt,
        normalized_graph_path=args.normalized_graph,
        module_closure_path=args.module_closure,
        material_closure_path=args.material_closure,
        resource_root=args.resource_root,
        expected_renderer_counts=_parse_expected_counts(args.expected_renderer_count),
        expected_active_element_count=args.expected_active_element_count,
        expected_excluded_legacy_element_count=args.expected_excluded_legacy_element_count,
        allow_legacy_conversion_input_slot_mismatch=(
            args.allow_legacy_conversion_input_slot_mismatch
        ),
    )
    check_or_write_json(args.output, document, check=args.check)
    output_prefix = (
        "source-active inventory check: "
        if args.check
        else "source-active inventory: "
    )
    print(
        output_prefix + f"active={document['summary']['activeElementCount']} "
        f"excluded={document['summary']['excludedExecutionDisabledLegacyElementCount']} "
        f"renderers={document['summary']['activeRendererCounts']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
