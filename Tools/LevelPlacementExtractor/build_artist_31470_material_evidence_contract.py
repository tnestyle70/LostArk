#!/usr/bin/env python3
"""Build the fail-closed typed Material evidence contract for Artist F 31470."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")

EXPECTED_RECIPE_COUNT = 27
EXPECTED_OCCURRENCE_COUNT = 34
EXPECTED_SCALAR_OVERRIDE_COUNT = 342
EXPECTED_VECTOR_OVERRIDE_COUNT = 19
EXPECTED_TEXTURE_OVERRIDE_COUNT = 71
EXPECTED_GRAPH_FAMILY_COUNT = 23
EXPECTED_NULL_EXPRESSION_COUNT = 1803
EXPECTED_UNRESOLVED_EDGE_COUNT = 502

EXPECTED_DDS_BINDINGS: dict[str, dict[str, Any]] = {
    "fx_tex_00.fx_a_noise_011": {
        "materialPath": "fx_m_mi_01.fx_mi.fx_h_me_watertrail_01_2_tr",
        "parameterName": "uv_noise_tex",
        "bindingEvidence": "MATERIAL_INSTANCE_OVERRIDE_EXACT",
        "bindingOrigin": "INSTANCE_OVERRIDE",
        "ddsByteCount": 16512,
        "ddsSha256": "c8d94d0750517d5654416e71bcd8666d79630c478c757d196e37ba056f87cc53",
        "addressU": "wrap",
        "addressV": "wrap",
        "colorSpace": "srgb",
        "addressUEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
        "addressVEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
        "colorSpaceEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
    },
    "fx_tex_03.fx_e_ring_001_cl": {
        "materialPath": "fx_m_mi_03.fx_mi.fx_m_pa_skull_02_23_tr",
        "parameterName": "mainalpha_tex",
        "bindingEvidence": "MATERIAL_INSTANCE_OVERRIDE_EXACT",
        "bindingOrigin": "INSTANCE_OVERRIDE",
        "ddsByteCount": 65664,
        "ddsSha256": "3c8987c8bc4bda1d3fd0f4840124e4fc1ba2eb3899307df8fe5852c4a760738e",
        "addressU": "clamp",
        "addressV": "clamp",
        "colorSpace": "srgb",
        "addressUEvidence": "SERIALIZED_PROPERTY_EXACT",
        "addressVEvidence": "SERIALIZED_PROPERTY_EXACT",
        "colorSpaceEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
    },
    "fx_tex_00.fx_a_decal_014": {
        "materialPath": "fx_m_mi_04.fx_mi.fx_d_de_unlit_01_01_tr",
        "parameterName": "emissive_tex",
        "bindingEvidence": "MATERIAL_INSTANCE_OVERRIDE_EXACT",
        "bindingOrigin": "INSTANCE_OVERRIDE",
        "ddsByteCount": 65664,
        "ddsSha256": "c2ead8a025cea1f70c8b03f9a488748ebd616828695199539d4e5010e5dc24ef",
        "addressU": "wrap",
        "addressV": "wrap",
        "colorSpace": "srgb",
        "addressUEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
        "addressVEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
        "colorSpaceEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
    },
    "fx_tex_01.fx_c_atypical_016": {
        "materialPath": "fx_m_mi_o_00.fx_mi.fx_o_transition_05_2_ma",
        "parameterName": "dissolve_texture",
        "bindingEvidence": "PARENT_DEFAULT_TEXTURE_EXACT",
        "bindingOrigin": "PARENT_DEFAULT",
        "ddsByteCount": 32896,
        "ddsSha256": "bbd2dc3ba79d24a5806af63d00dee302a04fb5c6e8be343275a63a9353981f43",
        "addressU": "wrap",
        "addressV": "wrap",
        "colorSpace": "srgb",
        "addressUEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
        "addressVEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
        "colorSpaceEvidence": "UE3_TEXTURE_CLASS_DEFAULT",
    },
}

REQUIRED_RENDER_FIELDS = (
    "blendmode",
    "lightingmodel",
    "twosided",
    "bdisabledepthtest",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def folded(value: Any) -> str:
    return str(value or "").casefold()


def require_sha256(value: Any, label: str) -> str:
    result = str(value or "")
    require(bool(SHA256_RE.fullmatch(result)), f"invalid SHA-256 for {label}")
    return result


def require_list(value: Any, label: str) -> list[Any]:
    require(isinstance(value, list), f"{label} must be a list")
    return value


def reject_duplicate_object_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON object key: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream, object_pairs_hook=reject_duplicate_object_keys)
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def normalize_tracked_text_bytes(content: bytes) -> bytes:
    require(not content.startswith(b"\xef\xbb\xbf"), "tracked JSON must not have a BOM")
    try:
        text = content.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("tracked JSON is not UTF-8") from error
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    try:
        json.loads(normalized, object_pairs_hook=reject_duplicate_object_keys)
    except json.JSONDecodeError as error:
        raise ValueError("tracked JSON is invalid") from error
    return normalized.encode("utf-8")


def tracked_json_text_sha256(path: Path) -> str:
    return hashlib.sha256(
        normalize_tracked_text_bytes(path.read_bytes())
    ).hexdigest()


def tracked_source_text_sha256(path: Path) -> str:
    content = path.read_bytes()
    require(not content.startswith(b"\xef\xbb\xbf"), "tracked source must not have a BOM")
    try:
        text = content.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("tracked source is not UTF-8") from error
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    return hashlib.sha256(normalized.encode("utf-8")).hexdigest()


def raw_file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical_payload(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_payload(value)).hexdigest()


def output_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            indent=2,
            sort_keys=False,
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def check_or_write_tracked_json(
    path: Path, value: dict[str, Any], check: bool
) -> None:
    expected = output_bytes(value)
    if check:
        require(path.is_file(), f"generated output is missing: {path}")
        current = normalize_tracked_text_bytes(path.read_bytes())
        require(current == expected, f"generated output is stale: {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(expected)


def repository_path(path: Path) -> str:
    return path.resolve().relative_to(REPO_ROOT).as_posix()


def stable_id(prefix: str, *parts: Any) -> str:
    payload = "::".join(folded(part) for part in parts).encode("utf-8")
    return f"{prefix}-{hashlib.sha256(payload).hexdigest()[:16]}"


def finite_number(value: Any, label: str) -> float:
    require(
        isinstance(value, (int, float))
        and not isinstance(value, bool)
        and math.isfinite(float(value)),
        f"{label} must be a finite number",
    )
    return float(value)


def validate_self_digest(document: dict[str, Any], field: str, label: str) -> None:
    expected = require_sha256(document.get(field), f"{label}.{field}")
    payload = copy.deepcopy(document)
    del payload[field]
    require(
        canonical_sha256(payload) == expected,
        f"{label} self digest mismatch",
    )


def closure_rows(closure: dict[str, Any]) -> list[dict[str, Any]]:
    require(
        closure.get("schema") == "lostark.ue3-effect-material-closure"
        and closure.get("formatVersion") == 1,
        "unsupported active material closure",
    )
    rows = require_list(closure.get("materials"), "material closure rows")
    rendered: list[dict[str, Any]] = []
    builtin_count = 0
    seen: set[str] = set()
    for row in rows:
        require(isinstance(row, dict), "material closure row must be an object")
        path = str(row.get("sourceMaterialPath") or "")
        require(bool(path), "material closure path is blank")
        key = path.casefold()
        require(key not in seen, f"duplicate material closure path: {path}")
        seen.add(key)
        material = row.get("material")
        if material is None:
            require(
                key == "enginematerials.defaultparticle"
                and row.get("status") == "SOURCE_BUILTIN_NON_RENDER_MATERIAL",
                f"unexpected material-less closure row: {path}",
            )
            builtin_count += 1
            continue
        require(isinstance(material, dict), f"material payload invalid: {path}")
        rendered.append(row)
    require(builtin_count == 1, f"engine builtin denominator changed: {builtin_count}")
    require(
        len(rendered) == EXPECTED_RECIPE_COUNT,
        f"rendered material recipe denominator changed: {len(rendered)}",
    )
    return sorted(rendered, key=lambda row: folded(row["sourceMaterialPath"]))


def validate_parent_cycles(rows: list[dict[str, Any]]) -> None:
    by_path = {folded(row["sourceMaterialPath"]): row for row in rows}
    edges: dict[str, str] = {}
    for key, row in by_path.items():
        material = row["material"]
        parent = folded(material.get("parent"))
        if parent in by_path:
            edges[key] = parent
    visiting: set[str] = set()
    visited: set[str] = set()

    def visit(node: str) -> None:
        if node in visiting:
            raise ValueError(f"material parent cycle detected at {node}")
        if node in visited:
            return
        visiting.add(node)
        if node in edges:
            visit(edges[node])
        visiting.remove(node)
        visited.add(node)

    for node in sorted(by_path):
        visit(node)


def graph_holder(row: dict[str, Any]) -> tuple[str, dict[str, Any]]:
    holder_name = "materialGraph" if row.get("materialGraph") is not None else "parentGraph"
    holder = row.get(holder_name)
    require(isinstance(holder, dict), f"{holder_name} is missing: {row['sourceMaterialPath']}")
    graph = holder.get("graph")
    require(isinstance(graph, dict), f"graph payload is missing: {row['sourceMaterialPath']}")
    return holder_name, holder


def validate_graph(graph: dict[str, Any], material_path: str) -> None:
    summary = graph.get("summary")
    require(isinstance(summary, dict), f"graph summary missing: {material_path}")
    require(
        summary.get("topologyStatus") == "COOKED_PARTIAL"
        and not bool(summary.get("runtimeExactEligible")),
        f"Artist F graph must remain COOKED_PARTIAL: {material_path}",
    )
    null_count = int(summary.get("nullExpressionCount", -1))
    edge_count = int(summary.get("unresolvedInputEdgeCount", -1))
    require(
        null_count > 0 or edge_count > 0,
        f"cooked-stripped evidence vanished: {material_path}",
    )


def validate_render_receipt(
    receipt: dict[str, Any], rows: list[dict[str, Any]]
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    require(
        receipt.get("schema")
        == "lostark.artist-31470-material-render-state-evidence-receipt"
        and receipt.get("formatVersion") == 1,
        "unsupported render-state evidence receipt",
    )
    validate_self_digest(receipt, "receiptSha256", "render-state receipt")
    export_rows = require_list(receipt.get("exports"), "render-state exports")
    exports: dict[str, dict[str, Any]] = {}
    for export in export_rows:
        require(isinstance(export, dict), "render-state export must be an object")
        evidence_id = str(export.get("evidenceId") or "")
        require(bool(evidence_id), "render-state export evidence ID is blank")
        require(evidence_id not in exports, f"duplicate render export: {evidence_id}")
        require_sha256(export.get("physicalPackageSha256"), f"{evidence_id}.package")
        require_sha256(export.get("serialSha256"), f"{evidence_id}.serial")
        fields = export.get("fields")
        require(isinstance(fields, dict), f"render-state fields missing: {evidence_id}")
        for field_name, field in fields.items():
            require(isinstance(field, dict), f"invalid render field: {field_name}")
            status = field.get("status")
            require(
                status in {"SERIALIZED_EXPLICIT", "OMITTED_FROM_EXPORT"},
                f"invalid render field status: {field_name}",
            )
            if status == "SERIALIZED_EXPLICIT":
                require_sha256(field.get("recordSha256"), f"{evidence_id}.{field_name}.record")
                require_sha256(
                    field.get("encodedValueSha256"),
                    f"{evidence_id}.{field_name}.value",
                )
                offsets = [
                    int(field.get(name, -1))
                    for name in ("tagOffset", "valueOffset", "recordEndOffset")
                ]
                require(
                    0 <= offsets[0] <= offsets[1] <= offsets[2]
                    <= int(export.get("serialSize", -1)),
                    f"invalid tagged-property offsets: {evidence_id}.{field_name}",
                )
                value = field.get("value")
                normalized_field = field_name.casefold()
                if normalized_field in {"blendmode", "lightingmodel"}:
                    require(
                        isinstance(value, str) and bool(value),
                        f"render enum field is invalid: {evidence_id}.{field_name}",
                    )
                elif normalized_field in {
                    "twosided",
                    "bdisabledepthtest",
                    "buseonelayerdistortion",
                    "overridedtwosided",
                    "bhasstaticpermutationresource",
                }:
                    require(
                        isinstance(value, bool),
                        f"render boolean field is invalid: {evidence_id}.{field_name}",
                    )
                elif normalized_field == "opacitymaskclipvalue":
                    finite_number(value, f"{evidence_id}.{field_name}")
                elif normalized_field in {
                    "scalarparametervalues",
                    "vectorparametervalues",
                    "textureparametervalues",
                }:
                    require(
                        isinstance(value, list),
                        f"parameter array field is invalid: {evidence_id}.{field_name}",
                    )
                elif normalized_field == "parent":
                    require(
                        isinstance(value, int)
                        and not isinstance(value, bool)
                        and value != 0
                        and bool(field.get("resolvedObjectPath")),
                        f"parent reference field is invalid: {evidence_id}.{field_name}",
                    )
            else:
                require(
                    field.get("fidelity") == "UNRESOLVED_DEFAULT_PROVENANCE",
                    f"omitted field gained a default: {evidence_id}.{field_name}",
                )
        exports[evidence_id] = export

    bindings: dict[str, dict[str, Any]] = {}
    for binding in require_list(receipt.get("bindings"), "render-state bindings"):
        require(isinstance(binding, dict), "render-state binding must be an object")
        key = folded(binding.get("sourceMaterialPath"))
        require(bool(key), "render-state binding path is blank")
        require(key not in bindings, f"duplicate render-state binding: {key}")
        require(
            binding.get("sourceExportEvidenceId") in exports
            and binding.get("renderStateExportEvidenceId") in exports,
            f"render-state binding references missing export: {key}",
        )
        bindings[key] = binding
    require(
        len(bindings) == EXPECTED_RECIPE_COUNT,
        f"render-state binding denominator changed: {len(bindings)}",
    )

    for row in rows:
        path = str(row["sourceMaterialPath"])
        key = path.casefold()
        require(key in bindings, f"render-state binding missing: {path}")
        binding = bindings[key]
        source_export = exports[str(binding["sourceExportEvidenceId"])]
        material = row["material"]
        require(
            folded(source_export.get("physicalPackage"))
            == folded(row.get("sourcePhysicalPackage"))
            and source_export.get("physicalPackageSha256")
            == row.get("sourcePhysicalPackageSha256")
            and folded(source_export.get("objectPath"))
            == folded(material.get("objectPath"))
            and folded(source_export.get("className"))
            == folded(material.get("className")),
            f"raw source export identity disagrees with closure: {path}",
        )
        if folded(material.get("className")) == "materialinstanceconstant":
            parent_field = source_export["fields"].get("parent")
            require(
                isinstance(parent_field, dict)
                and parent_field.get("status") == "SERIALIZED_EXPLICIT"
                and folded(parent_field.get("resolvedObjectPath"))
                == folded(material.get("parent")),
                f"exact inheritance edge missing: {path}",
            )
            for field_name, closure_name in (
                ("overridedtwosided", "overrideTwoSided"),
                ("bhasstaticpermutationresource", "hasStaticPermutationResource"),
            ):
                evidence = source_export["fields"].get(field_name)
                expected_value = material.get(closure_name)
                if expected_value is None:
                    require(
                        isinstance(evidence, dict)
                        and evidence.get("status") == "OMITTED_FROM_EXPORT",
                        f"omitted source flag became explicit: {path}.{field_name}",
                    )
                else:
                    require(
                        isinstance(evidence, dict)
                        and evidence.get("status") == "SERIALIZED_EXPLICIT"
                        and evidence.get("value") == expected_value,
                        f"source flag disagrees with closure: {path}.{field_name}",
                    )
            for field_name, closure_name in (
                ("scalarparametervalues", "scalarParameters"),
                ("vectorparametervalues", "vectorParameters"),
                ("textureparametervalues", "textureParameters"),
            ):
                if require_list(material.get(closure_name), f"{path}.{closure_name}"):
                    require(
                        source_export["fields"][field_name]["status"]
                        == "SERIALIZED_EXPLICIT",
                        f"raw parameter array evidence missing: {path}.{field_name}",
                    )

        holder_name, holder = graph_holder(row)
        graph = holder["graph"]
        base_export = exports[str(binding["renderStateExportEvidenceId"])]
        require(
            folded(base_export.get("physicalPackage"))
            == folded(holder.get("physicalPackage"))
            and base_export.get("physicalPackageSha256")
            == holder.get("physicalPackageSha256")
            and folded(base_export.get("objectPath"))
            == folded(graph.get("materialPath"))
            and int(base_export.get("exportIndex", -1))
            == int(graph.get("materialExportIndex", -2)),
            f"raw base Material identity disagrees with {holder_name}: {path}",
        )
    return bindings, exports


def parameter_rows(
    row: dict[str, Any],
    kind: str,
    array_name: str,
    array_record: dict[str, Any] | None,
) -> list[dict[str, Any]]:
    material_path = str(row["sourceMaterialPath"])
    values = require_list(row["material"].get(array_name), f"{material_path}.{array_name}")
    seen: set[str] = set()
    result: list[dict[str, Any]] = []
    record_sha = None
    if array_record and array_record.get("status") == "SERIALIZED_EXPLICIT":
        record_sha = require_sha256(
            array_record.get("recordSha256"),
            f"{material_path}.{array_name}.record",
        )
    for index, value in enumerate(values):
        require(isinstance(value, dict), f"invalid parameter row: {material_path}.{array_name}[{index}]")
        name = str(value.get("name") or "")
        require(bool(name.strip()), f"blank parameter name: {material_path}.{array_name}[{index}]")
        key = name.casefold()
        require(key not in seen, f"duplicate parameter name: {material_path}.{array_name}.{name}")
        seen.add(key)
        if kind == "scalar":
            decoded_value: Any = finite_number(value.get("value"), f"{material_path}.{name}")
        elif kind == "vector":
            raw_vector = require_list(value.get("value"), f"{material_path}.{name}")
            require(len(raw_vector) == 4, f"vector parameter is not float4: {material_path}.{name}")
            decoded_value = [
                finite_number(component, f"{material_path}.{name}[{axis}]")
                for axis, component in enumerate(raw_vector)
            ]
        elif kind == "texture":
            decoded_value = str(value.get("sourceObjectPath") or "")
            require(bool(decoded_value), f"texture override is null: {material_path}.{name}")
            require(
                isinstance(value.get("packageIndex"), int)
                and not isinstance(value.get("packageIndex"), bool)
                and int(value["packageIndex"]) != 0,
                f"texture package reference is invalid: {material_path}.{name}",
            )
        else:
            raise ValueError(f"unsupported parameter kind: {kind}")
        field_id = stable_id(
            "material-input",
            material_path,
            kind,
            index,
            name,
            "INSTANCE_OVERRIDE",
        )
        evidence: dict[str, Any] = {
            "fieldId": field_id,
            "fieldKind": kind,
            "parameterName": name,
            "normalizedParameterName": key,
            "serializedArrayIndex": index,
            "bindingOrigin": "INSTANCE_OVERRIDE",
            "value": decoded_value,
            "fidelity": "SOURCE_EXACT_INPUT",
            "provenance": {
                "physicalPackage": row["sourcePhysicalPackage"],
                "physicalPackageSha256": row["sourcePhysicalPackageSha256"],
                "materialObjectPath": row["material"]["objectPath"],
                "parameterArrayRecordSha256": record_sha,
            },
        }
        if kind == "texture":
            evidence["packageIndex"] = int(value["packageIndex"])
            evidence["sampler"] = {
                "fidelity": "UNRESOLVED",
                "blocker": "SAMPLER_EVIDENCE_MISSING",
            }
        result.append(evidence)
    return result


def expression_parameter_kind(expression: dict[str, Any]) -> str | None:
    class_name = folded(expression.get("className"))
    if "texturesampleparameter" in class_name:
        return "texture"
    if "staticswitchparameter" in class_name:
        return "staticSwitch"
    if "scalarparameter" in class_name:
        return "scalar"
    if "vectorparameter" in class_name:
        return "vector"
    return None


def parent_default_rows(
    row: dict[str, Any],
    instance_inputs: dict[str, list[dict[str, Any]]],
    binding: dict[str, Any],
    exports: dict[str, dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    material_path = str(row["sourceMaterialPath"])
    material_class = folded(row["material"].get("className"))
    holder_name, holder = graph_holder(row)
    graph = holder["graph"]
    base_export = exports[str(binding["renderStateExportEvidenceId"])]
    binding_origin = (
        "PARENT_DEFAULT"
        if material_class == "materialinstanceconstant"
        else "SELF_DEFAULT"
    )
    if binding_origin == "PARENT_DEFAULT":
        source_export = exports[str(binding["sourceExportEvidenceId"])]
        parent_field = source_export["fields"]["parent"]
        require(
            parent_field.get("status") == "SERIALIZED_EXPLICIT"
            and folded(parent_field.get("resolvedObjectPath"))
            == folded(row["material"].get("parent")),
            f"parent default lacks exact inheritance edge: {material_path}",
        )

    override_names = {
        kind: {field["normalizedParameterName"] for field in fields}
        for kind, fields in instance_inputs.items()
    }
    defaults: list[dict[str, Any]] = []
    static_defaults: list[dict[str, Any]] = []
    expressions = require_list(graph.get("expressions"), f"{material_path}.expressions")
    for expression in expressions:
        require(isinstance(expression, dict), f"invalid graph expression: {material_path}")
        name = str(expression.get("parameterName") or "")
        kind = expression_parameter_kind(expression)
        if not name or kind is None:
            continue
        normalized_name = name.casefold()
        texture_path = str(expression.get("textureObjectPath") or "")
        default_value = expression.get("defaultValue")
        if kind == "texture":
            if not texture_path:
                continue
            value: Any = texture_path
        elif default_value is not None:
            value = copy.deepcopy(default_value)
        else:
            continue
        expression_export = int(expression.get("exportIndex", -1))
        source_order = int(expression.get("sourceOrder", -1))
        require(
            expression_export >= 0 and source_order >= 0,
            f"parent default expression identity missing: {material_path}.{name}",
        )
        evidence = {
            "fieldId": stable_id(
                "material-default",
                material_path,
                kind,
                source_order,
                expression_export,
                name,
                binding_origin,
            ),
            "fieldKind": kind,
            "parameterName": name,
            "normalizedParameterName": normalized_name,
            "expressionSourceOrder": source_order,
            "expressionExportIndex": expression_export,
            "expressionObjectPath": expression.get("objectPath"),
            "bindingOrigin": binding_origin,
            "value": value,
            "fidelity": "SOURCE_EXACT_INPUT",
            "closerOverridePresent": normalized_name in override_names.get(kind, set()),
            "provenance": {
                "physicalPackage": base_export["physicalPackage"],
                "physicalPackageSha256": base_export["physicalPackageSha256"],
                "materialObjectPath": base_export["objectPath"],
                "materialExportIndex": base_export["exportIndex"],
                "graphHolder": holder_name,
                "inheritanceEdgeEvidenceId": (
                    binding["sourceExportEvidenceId"]
                    if binding_origin == "PARENT_DEFAULT"
                    else None
                ),
            },
        }
        if kind == "staticSwitch":
            evidence["selectionRole"] = "PARENT_DEFAULT_NOT_INSTANCE_SELECTION"
            static_defaults.append(evidence)
            continue
        if evidence["closerOverridePresent"]:
            continue
        if kind == "texture":
            evidence["sampler"] = {
                "fidelity": "UNRESOLVED",
                "blocker": "SAMPLER_EVIDENCE_MISSING",
            }
        defaults.append(evidence)
    return defaults, static_defaults


def build_graph_families(
    rows: list[dict[str, Any]],
) -> tuple[list[dict[str, Any]], dict[str, str]]:
    families: dict[str, dict[str, Any]] = {}
    recipe_family: dict[str, str] = {}
    for row in rows:
        _, holder = graph_holder(row)
        graph = holder["graph"]
        validate_graph(graph, str(row["sourceMaterialPath"]))
        package_sha = require_sha256(
            holder.get("physicalPackageSha256"),
            f"{row['sourceMaterialPath']}.graphPackage",
        )
        graph_path = str(graph.get("materialPath") or "")
        require(bool(graph_path), f"graph Material path is blank: {row['sourceMaterialPath']}")
        key = f"{package_sha}::{graph_path.casefold()}"
        family_id = stable_id("material-family", package_sha, graph_path)
        summary = graph["summary"]
        family = {
            "familyId": family_id,
            "graphProvenance": "RECONSTRUCTED_GRAPH",
            "sourceExactGraph": False,
            "exactIdentity": {
                "physicalPackage": holder["physicalPackage"],
                "physicalPackageSha256": package_sha,
                "materialObjectPath": graph_path,
                "materialExportIndex": int(graph.get("materialExportIndex", -1)),
            },
            "cookedEvidence": {
                "topologyStatus": summary["topologyStatus"],
                "expressionEntryCount": int(summary.get("expressionEntryCount", -1)),
                "nonNullExpressionCount": int(summary.get("nonNullExpressionCount", -1)),
                "nullExpressionCount": int(summary.get("nullExpressionCount", -1)),
                "unresolvedInputEdgeCount": int(
                    summary.get("unresolvedInputEdgeCount", -1)
                ),
            },
            "evaluator": {
                "evaluatorId": stable_id(
                    "reconstructed-evaluator", package_sha, graph_path
                ),
                "fidelity": "RECONSTRUCTED_ARITHMETIC_FAMILY",
                "sourceExact": False,
                "implemented": False,
            },
            "blockers": [
                "COOKED_STRIPPED_ARITHMETIC_GRAPH",
                "RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED",
            ],
        }
        previous = families.get(key)
        require(previous is None or previous == family, f"graph family drift: {key}")
        families[key] = family
        recipe_family[folded(row["sourceMaterialPath"])] = family_id

    result = sorted(families.values(), key=lambda row: row["familyId"])
    require(
        len(result) == EXPECTED_GRAPH_FAMILY_COUNT,
        f"graph family denominator changed: {len(result)}",
    )
    null_count = sum(row["cookedEvidence"]["nullExpressionCount"] for row in result)
    edge_count = sum(
        row["cookedEvidence"]["unresolvedInputEdgeCount"] for row in result
    )
    require(
        null_count == EXPECTED_NULL_EXPRESSION_COUNT,
        f"cooked null-expression denominator changed: {null_count}",
    )
    require(
        edge_count == EXPECTED_UNRESOLVED_EDGE_COUNT,
        f"unresolved graph-edge denominator changed: {edge_count}",
    )
    return result, recipe_family


def build_render_state(
    row: dict[str, Any],
    binding: dict[str, Any],
    exports: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    source_export = exports[str(binding["sourceExportEvidenceId"])]
    base_export = exports[str(binding["renderStateExportEvidenceId"])]
    fields: dict[str, dict[str, Any]] = {}
    for field_name in REQUIRED_RENDER_FIELDS + (
        "opacitymaskclipvalue",
        "buseonelayerdistortion",
    ):
        raw = base_export["fields"].get(field_name)
        require(isinstance(raw, dict), f"render-state field absent from receipt: {field_name}")
        if raw.get("status") == "SERIALIZED_EXPLICIT":
            fields[field_name] = {
                "status": "SERIALIZED_EXPLICIT",
                "value": copy.deepcopy(raw.get("value")),
                "bindingOrigin": binding["renderStateOrigin"],
                "fidelity": (
                    "SOURCE_EXACT_PARTIAL_CULL"
                    if field_name == "twosided"
                    else "SOURCE_EXACT_RENDER_STATE"
                ),
                "recordSha256": raw["recordSha256"],
                "exportEvidenceId": base_export["evidenceId"],
            }
        else:
            fields[field_name] = {
                "status": "OMITTED_FROM_EXPORT",
                "bindingOrigin": binding["renderStateOrigin"],
                "fidelity": "UNRESOLVED_DEFAULT_PROVENANCE",
                "blocker": "RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED",
            }

    override = source_export["fields"].get("overridedtwosided")
    if (
        isinstance(override, dict)
        and override.get("status") == "SERIALIZED_EXPLICIT"
    ):
        fields["twosided"] = {
            "status": "SERIALIZED_EXPLICIT",
            "value": override.get("value"),
            "bindingOrigin": "INSTANCE_OVERRIDE",
            "fidelity": "SOURCE_EXACT_PARTIAL_CULL",
            "recordSha256": override["recordSha256"],
            "exportEvidenceId": source_export["evidenceId"],
        }
    return {
        "fields": fields,
        "partialCullExact": fields["twosided"]["fidelity"]
        == "SOURCE_EXACT_PARTIAL_CULL",
        "fullCullModeExact": False,
        "fullRenderStateExact": False,
    }


def validate_dds_receipt(dds_receipt: dict[str, Any]) -> list[dict[str, Any]]:
    require(
        dds_receipt.get("schema")
        == "lostark.artist-effect-exact-dds-recovery-receipt"
        and dds_receipt.get("formatVersion") == 1,
        "unsupported exact DDS receipt",
    )
    assets = require_list(dds_receipt.get("assets"), "exact DDS assets")
    require(len(assets) == 4, f"exact DDS denominator changed: {len(assets)}")
    by_path: dict[str, dict[str, Any]] = {}
    for asset in assets:
        require(isinstance(asset, dict), "exact DDS asset must be an object")
        key = folded(asset.get("logicalObjectPath"))
        require(key in EXPECTED_DDS_BINDINGS, f"unexpected exact DDS asset: {key}")
        require(key not in by_path, f"duplicate exact DDS asset: {key}")
        expected = EXPECTED_DDS_BINDINGS[key]
        binding = asset.get("sourceMaterialBinding")
        texture = asset.get("sourceTexture2D")
        dds = asset.get("dds")
        require(
            isinstance(binding, dict)
            and isinstance(texture, dict)
            and isinstance(dds, dict),
            f"exact DDS evidence is incomplete: {key}",
        )
        for actual_name, expected_name in (
            (binding.get("materialObjectPath"), expected["materialPath"]),
            (binding.get("parameterName"), expected["parameterName"]),
            (binding.get("bindingEvidence"), expected["bindingEvidence"]),
        ):
            require(folded(actual_name) == folded(expected_name), f"DDS binding drift: {key}")
        require(
            int(dds.get("byteCount", -1)) == expected["ddsByteCount"]
            and dds.get("sha256") == expected["ddsSha256"],
            f"DDS raw hash drift: {key}",
        )
        require_sha256(texture.get("physicalPackageSha256"), f"{key}.texturePackage")
        require_sha256(texture.get("serialSha256"), f"{key}.textureSerial")
        sampling = texture.get("sampling")
        require(isinstance(sampling, dict), f"sampler evidence missing: {key}")
        for field_name in (
            "addressU",
            "addressV",
            "colorSpace",
            "addressUEvidence",
            "addressVEvidence",
            "colorSpaceEvidence",
        ):
            require(
                sampling.get(field_name) == expected[field_name],
                f"sampler evidence drift: {key}.{field_name}",
            )
        by_path[key] = asset
    require(
        set(by_path) == set(EXPECTED_DDS_BINDINGS),
        "exact DDS golden set changed",
    )
    return [by_path[key] for key in sorted(by_path)]


def apply_exact_sampler_bindings(
    recipes: list[dict[str, Any]], dds_assets: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    recipes_by_path = {folded(row["sourceMaterialPath"]): row for row in recipes}
    result: list[dict[str, Any]] = []
    for asset in dds_assets:
        logical_path = str(asset["logicalObjectPath"])
        expected = EXPECTED_DDS_BINDINGS[logical_path.casefold()]
        recipe_key = expected["materialPath"].casefold()
        require(recipe_key in recipes_by_path, f"DDS recipe join missing: {logical_path}")
        recipe = recipes_by_path[recipe_key]
        binding_evidence = asset["sourceMaterialBinding"]
        require(
            folded(binding_evidence.get("materialPhysicalPackage"))
            == folded(recipe["identity"]["physicalPackage"])
            and binding_evidence.get("materialPhysicalPackageSha256")
            == recipe["identity"]["physicalPackageSha256"],
            f"DDS material package identity drift: {logical_path}",
        )
        kind = "textureOverrides" if expected["bindingOrigin"] == "INSTANCE_OVERRIDE" else "parentDefaults"
        candidates = [
            field for field in recipe["inputs"][kind]
            if field.get("fieldKind") == "texture"
            and folded(field.get("parameterName"))
            == expected["parameterName"].casefold()
            and folded(field.get("value")) == logical_path.casefold()
            and field.get("bindingOrigin") == expected["bindingOrigin"]
        ]
        require(len(candidates) == 1, f"DDS input join is not unique: {logical_path}")
        field = candidates[0]
        if expected["bindingOrigin"] == "PARENT_DEFAULT":
            require(
                not field.get("closerOverridePresent"),
                f"parent DDS default has a closer override: {logical_path}",
            )
            direct_names = {
                folded(item.get("parameterName"))
                for item in recipe["inputs"]["textureOverrides"]
            }
            require(
                expected["parameterName"].casefold() not in direct_names,
                f"parent DDS default was shadowed by an override: {logical_path}",
            )
        sampling = asset["sourceTexture2D"]["sampling"]
        binding_id = stable_id(
            "exact-sampler",
            logical_path,
            expected["materialPath"],
            expected["parameterName"],
            expected["bindingOrigin"],
        )
        field["sampler"] = {
            "bindingId": binding_id,
            "fidelity": "SOURCE_EXACT_SAMPLER",
            "addressU": sampling["addressU"],
            "addressV": sampling["addressV"],
            "colorSpace": sampling["colorSpace"],
            "provenance": {
                "addressUEvidence": sampling["addressUEvidence"],
                "addressVEvidence": sampling["addressVEvidence"],
                "colorSpaceEvidence": sampling["colorSpaceEvidence"],
                "ddsSha256": asset["dds"]["sha256"],
                "texturePackageSha256": asset["sourceTexture2D"][
                    "physicalPackageSha256"
                ],
                "textureSerialSha256": asset["sourceTexture2D"]["serialSha256"],
            },
        }
        result.append(
            {
                "bindingId": binding_id,
                "materialRecipeId": recipe["recipeId"],
                "inputFieldId": field["fieldId"],
                "logicalTexturePath": logical_path,
                "parameterName": expected["parameterName"],
                "bindingOrigin": expected["bindingOrigin"],
                "fidelity": "SOURCE_EXACT_SAMPLER",
                "dds": {
                    "relativePath": asset["sourceExtractedDdsRelativePath"],
                    "byteCount": asset["dds"]["byteCount"],
                    "sha256": asset["dds"]["sha256"],
                },
                "sampling": copy.deepcopy(field["sampler"]),
            }
        )
    return sorted(result, key=lambda row: row["bindingId"])


def build_occurrence_rows(
    active_inventory: dict[str, Any], recipe_by_path: dict[str, dict[str, Any]]
) -> list[dict[str, Any]]:
    require(
        active_inventory.get("schema")
        == "lostark.source-active-effect-inventory-receipt"
        and active_inventory.get("formatVersion") == 1,
        "unsupported active inventory",
    )
    elements = require_list(active_inventory.get("activeElements"), "active elements")
    require(len(elements) == 35, f"active element denominator changed: {len(elements)}")
    seen: set[str] = set()
    occurrences: list[dict[str, Any]] = []
    builtin_count = 0
    for element in elements:
        require(isinstance(element, dict), "active element must be an object")
        active_id = str(element.get("activeElementId") or "")
        require(bool(active_id), "active element ID is blank")
        require(active_id not in seen, f"duplicate active element ID: {active_id}")
        seen.add(active_id)
        materials = require_list(
            element.get("sourceMaterials"), f"{active_id}.sourceMaterials"
        )
        require(len(materials) == 1, f"active material join is not singular: {active_id}")
        material_path = str(materials[0] or "")
        key = material_path.casefold()
        if key == "enginematerials.defaultparticle":
            require(
                element.get("rendererType") == "LightParticle",
                "engine builtin material is not the Light occurrence",
            )
            builtin_count += 1
            continue
        require(key in recipe_by_path, f"active occurrence material is unresolved: {active_id}")
        recipe = recipe_by_path[key]
        occurrences.append(
            {
                "occurrenceId": active_id,
                "cueId": element.get("cueId"),
                "rendererType": element.get("rendererType"),
                "sourceSystemId": element.get("sourceSystemId"),
                "sourceEmitter": element.get("sourceEmitter"),
                "sourceMaterialPath": material_path,
                "materialRecipeId": recipe["recipeId"],
                "blockers": copy.deepcopy(recipe["blockers"]),
                "blockerCount": len(recipe["blockers"]),
                "admission": {
                    "executable": len(recipe["blockers"]) == 0,
                    "product": len(recipe["blockers"]) == 0,
                },
            }
        )
    require(builtin_count == 1, f"Light builtin occurrence denominator changed: {builtin_count}")
    require(
        len(occurrences) == EXPECTED_OCCURRENCE_COUNT,
        f"rendered material occurrence denominator changed: {len(occurrences)}",
    )
    return sorted(occurrences, key=lambda row: row["occurrenceId"])


def build_contract(
    active_inventory: dict[str, Any],
    material_closure: dict[str, Any],
    dds_receipt: dict[str, Any],
    render_receipt: dict[str, Any],
    source_evidence: dict[str, Any] | None = None,
) -> dict[str, Any]:
    rows = closure_rows(material_closure)
    validate_parent_cycles(rows)
    render_bindings, render_exports = validate_render_receipt(
        render_receipt, rows
    )
    graph_families, family_by_recipe = build_graph_families(rows)
    recipes: list[dict[str, Any]] = []

    for row in rows:
        material_path = str(row["sourceMaterialPath"])
        key = material_path.casefold()
        material = row["material"]
        binding = render_bindings[key]
        source_export = render_exports[str(binding["sourceExportEvidenceId"])]
        array_records = source_export.get("fields", {})
        scalar_inputs = parameter_rows(
            row,
            "scalar",
            "scalarParameters",
            array_records.get("scalarparametervalues"),
        )
        vector_inputs = parameter_rows(
            row,
            "vector",
            "vectorParameters",
            array_records.get("vectorparametervalues"),
        )
        texture_inputs = parameter_rows(
            row,
            "texture",
            "textureParameters",
            array_records.get("textureparametervalues"),
        )
        instance_inputs = {
            "scalar": scalar_inputs,
            "vector": vector_inputs,
            "texture": texture_inputs,
        }
        parent_defaults, static_defaults = parent_default_rows(
            row, instance_inputs, binding, render_exports
        )
        render_state = build_render_state(row, binding, render_exports)
        static_flag = source_export["fields"].get(
            "bhasstaticpermutationresource"
        )
        blockers = {
            "COOKED_STRIPPED_ARITHMETIC_GRAPH",
            "RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED",
            "STATIC_PERMUTATION_SELECTIONS_UNRESOLVED",
            "FULL_CULL_MODE_UNRESOLVED",
            "FULL_RENDER_STATE_UNRESOLVED",
            "PRODUCT_RUNTIME_MATERIAL_COMPILER_UNIMPLEMENTED",
        }
        if any(
            render_state["fields"][field]["status"] == "OMITTED_FROM_EXPORT"
            for field in REQUIRED_RENDER_FIELDS
        ):
            blockers.add("RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED")
        recipe = {
            "recipeId": stable_id("material-recipe", material_path),
            "sourceMaterialPath": material_path,
            "rendererShapes": copy.deepcopy(row.get("rendererShapes", [])),
            "identity": {
                "fidelity": "SOURCE_EXACT_INPUT",
                "physicalPackage": row["sourcePhysicalPackage"],
                "physicalPackageSha256": require_sha256(
                    row.get("sourcePhysicalPackageSha256"),
                    f"{material_path}.sourcePackage",
                ),
                "materialObjectPath": material["objectPath"],
                "materialClass": material["className"],
                "materialExportIndex": source_export["exportIndex"],
                "rawExportEvidenceId": source_export["evidenceId"],
            },
            "inputs": {
                "scalarOverrides": scalar_inputs,
                "vectorOverrides": vector_inputs,
                "textureOverrides": texture_inputs,
                "parentDefaults": parent_defaults,
            },
            "staticPermutation": {
                "resourcePresentFlag": (
                    static_flag.get("value")
                    if isinstance(static_flag, dict)
                    and static_flag.get("status") == "SERIALIZED_EXPLICIT"
                    else None
                ),
                "resourcePresentFlagIsSelection": False,
                "selectedParameters": [],
                "parentDefaults": static_defaults,
                "fidelity": "UNRESOLVED",
                "sourceExact": False,
            },
            "renderState": render_state,
            "arithmeticFamilyId": family_by_recipe[key],
            "blockers": sorted(blockers),
        }
        recipe["blockerCount"] = len(recipe["blockers"])
        recipe["admission"] = {
            "executable": recipe["blockerCount"] == 0,
            "product": recipe["blockerCount"] == 0,
        }
        recipes.append(recipe)

    recipes.sort(key=lambda row: row["recipeId"])
    dds_assets = validate_dds_receipt(dds_receipt)
    exact_sampler_bindings = apply_exact_sampler_bindings(recipes, dds_assets)
    for recipe in recipes:
        unresolved_sampler_count = sum(
            field["sampler"]["fidelity"] != "SOURCE_EXACT_SAMPLER"
            for collection in (
                recipe["inputs"]["textureOverrides"],
                [
                    field for field in recipe["inputs"]["parentDefaults"]
                    if field["fieldKind"] == "texture"
                ],
            )
            for field in collection
        )
        if unresolved_sampler_count:
            recipe["blockers"] = sorted(
                {*recipe["blockers"], "SAMPLER_BINDINGS_INCOMPLETE"}
            )
            recipe["blockerCount"] = len(recipe["blockers"])
            recipe["admission"] = {
                "executable": recipe["blockerCount"] == 0,
                "product": recipe["blockerCount"] == 0,
            }

    recipe_by_path = {folded(row["sourceMaterialPath"]): row for row in recipes}
    occurrences = build_occurrence_rows(active_inventory, recipe_by_path)
    scalar_count = sum(len(row["inputs"]["scalarOverrides"]) for row in recipes)
    vector_count = sum(len(row["inputs"]["vectorOverrides"]) for row in recipes)
    texture_count = sum(len(row["inputs"]["textureOverrides"]) for row in recipes)
    require(
        scalar_count == EXPECTED_SCALAR_OVERRIDE_COUNT,
        f"scalar override denominator changed: {scalar_count}",
    )
    require(
        vector_count == EXPECTED_VECTOR_OVERRIDE_COUNT,
        f"vector override denominator changed: {vector_count}",
    )
    require(
        texture_count == EXPECTED_TEXTURE_OVERRIDE_COUNT,
        f"texture override denominator changed: {texture_count}",
    )
    direct_exact = sum(
        field["sampler"]["fidelity"] == "SOURCE_EXACT_SAMPLER"
        for row in recipes
        for field in row["inputs"]["textureOverrides"]
    )
    parent_exact = sum(
        field.get("fieldKind") == "texture"
        and field["sampler"]["fidelity"] == "SOURCE_EXACT_SAMPLER"
        for row in recipes
        for field in row["inputs"]["parentDefaults"]
    )
    require(direct_exact == 3, f"exact override sampler denominator changed: {direct_exact}")
    require(parent_exact == 1, f"exact parent sampler denominator changed: {parent_exact}")
    aggregate_blockers = sorted(
        {blocker for recipe in recipes for blocker in recipe["blockers"]}
    )

    contract: dict[str, Any] = {
        "schema": "lostark.artist-31470-typed-material-evidence-contract",
        "formatVersion": 1,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "fidelityVocabulary": [
            "SOURCE_EXACT_INPUT",
            "SOURCE_EXACT_SAMPLER",
            "SOURCE_EXACT_STATIC_PERMUTATION",
            "SOURCE_EXACT_RENDER_STATE",
            "SOURCE_EXACT_PARTIAL_CULL",
            "RECONSTRUCTED_ARITHMETIC_FAMILY",
        ],
        "sourceEvidence": copy.deepcopy(source_evidence or {}),
        "graphFamilies": graph_families,
        "exactSamplerBindings": exact_sampler_bindings,
        "materialRecipes": recipes,
        "occurrences": occurrences,
        "admission": {
            "executableRecipeCount": sum(
                recipe["admission"]["executable"] for recipe in recipes
            ),
            "productRecipeCount": sum(
                recipe["admission"]["product"] for recipe in recipes
            ),
            "executableOccurrenceCount": sum(
                occurrence["admission"]["executable"] for occurrence in occurrences
            ),
            "productOccurrenceCount": sum(
                occurrence["admission"]["product"] for occurrence in occurrences
            ),
            "blockers": aggregate_blockers,
        },
        "summary": {
            "renderedEffectOccurrenceCount": 35,
            "engineBuiltinMaterialOccurrenceCount": 1,
            "materialRecipeCount": len(recipes),
            "materialOccurrenceCount": len(occurrences),
            "scalarOverrideCount": scalar_count,
            "vectorOverrideCount": vector_count,
            "directTextureOverrideCount": texture_count,
            "directTextureExactSamplerCount": direct_exact,
            "directTextureUnprovenSamplerCount": texture_count - direct_exact,
            "parentDefaultExactSamplerCount": parent_exact,
            "exactSamplerBindingCount": len(exact_sampler_bindings),
            "arithmeticFamilyCount": len(graph_families),
            "cookedStrippedNullExpressionCount": sum(
                row["cookedEvidence"]["nullExpressionCount"]
                for row in graph_families
            ),
            "unresolvedGraphEdgeCount": sum(
                row["cookedEvidence"]["unresolvedInputEdgeCount"]
                for row in graph_families
            ),
            "sourceExactGraphFamilyCount": 0,
            "reconstructedGraphFamilyCount": len(graph_families),
            "sourceExactInputRecipeCount": len(recipes),
            "sourceExactStaticPermutationRecipeCount": 0,
            "sourceExactFullRenderStateRecipeCount": 0,
            "sourceExactPartialCullRecipeCount": sum(
                row["renderState"]["partialCullExact"] for row in recipes
            ),
            "implementedArithmeticEvaluatorCount": 0,
        },
    }
    contract["contractSha256"] = canonical_sha256(contract)
    validate_contract(contract)
    return contract


def validate_contract(
    contract: dict[str, Any], verify_digest: bool = True
) -> None:
    require(
        contract.get("schema")
        == "lostark.artist-31470-typed-material-evidence-contract"
        and contract.get("formatVersion") == 1,
        "unsupported typed Material contract",
    )
    if verify_digest:
        validate_self_digest(contract, "contractSha256", "typed Material contract")
    recipes = require_list(contract.get("materialRecipes"), "contract recipes")
    occurrences = require_list(contract.get("occurrences"), "contract occurrences")
    families = require_list(contract.get("graphFamilies"), "contract graph families")
    samplers = require_list(
        contract.get("exactSamplerBindings"), "contract exact samplers"
    )
    require(len(recipes) == EXPECTED_RECIPE_COUNT, "contract recipe denominator changed")
    require(
        len(occurrences) == EXPECTED_OCCURRENCE_COUNT,
        "contract occurrence denominator changed",
    )
    require(len(families) == EXPECTED_GRAPH_FAMILY_COUNT, "contract family denominator changed")
    require(len(samplers) == 4, "contract exact sampler denominator changed")
    recipe_by_id: dict[str, dict[str, Any]] = {}
    all_input_fields: dict[str, dict[str, Any]] = {}
    exact_sampler_field_ids: set[str] = set()
    family_ids = {str(row.get("familyId") or "") for row in families}
    require(len(family_ids) == len(families) and "" not in family_ids, "duplicate graph family ID")
    for family in families:
        require(
            family.get("graphProvenance") == "RECONSTRUCTED_GRAPH"
            and not bool(family.get("sourceExactGraph")),
            "cooked graph was laundered as Source exact",
        )
        evaluator = family.get("evaluator")
        require(
            isinstance(evaluator, dict)
            and evaluator.get("fidelity") == "RECONSTRUCTED_ARITHMETIC_FAMILY"
            and not bool(evaluator.get("sourceExact"))
            and not bool(evaluator.get("implemented")),
            "reconstructed evaluator fidelity was laundered",
        )
        require(
            "COOKED_STRIPPED_ARITHMETIC_GRAPH" in family.get("blockers", [])
            and "RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED"
            in family.get("blockers", []),
            "graph family blocker set was weakened",
        )

    for recipe in recipes:
        require(isinstance(recipe, dict), "contract recipe must be an object")
        recipe_id = str(recipe.get("recipeId") or "")
        require(bool(recipe_id), "contract recipe ID is blank")
        require(recipe_id not in recipe_by_id, f"duplicate contract recipe: {recipe_id}")
        recipe_by_id[recipe_id] = recipe
        blockers = require_list(recipe.get("blockers"), f"{recipe_id}.blockers")
        require(
            blockers == sorted(set(blockers)),
            f"recipe blockers are not a sorted set: {recipe_id}",
        )
        require(int(recipe.get("blockerCount", -1)) == len(blockers), f"recipe blocker count mismatch: {recipe_id}")
        admission = recipe.get("admission")
        require(isinstance(admission, dict), f"recipe admission missing: {recipe_id}")
        allowed = len(blockers) == 0
        require(
            bool(admission.get("executable")) == allowed
            and bool(admission.get("product")) == allowed,
            f"recipe admission is not blocker-derived: {recipe_id}",
        )
        require(recipe.get("arithmeticFamilyId") in family_ids, f"recipe family missing: {recipe_id}")
        static = recipe.get("staticPermutation")
        require(
            isinstance(static, dict)
            and static.get("resourcePresentFlagIsSelection") is False
            and static.get("selectedParameters") == []
            and static.get("fidelity") == "UNRESOLVED"
            and not bool(static.get("sourceExact"))
            and "STATIC_PERMUTATION_SELECTIONS_UNRESOLVED" in blockers,
            f"static permutation was laundered: {recipe_id}",
        )
        render = recipe.get("renderState")
        require(
            isinstance(render, dict)
            and render.get("fullCullModeExact") is False
            and render.get("fullRenderStateExact") is False
            and "FULL_CULL_MODE_UNRESOLVED" in blockers
            and "FULL_RENDER_STATE_UNRESOLVED" in blockers,
            f"partial render state was promoted to full exact: {recipe_id}",
        )
        require(
            "COOKED_STRIPPED_ARITHMETIC_GRAPH" in blockers
            and "RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED" in blockers
            and "PRODUCT_RUNTIME_MATERIAL_COMPILER_UNIMPLEMENTED" in blockers,
            f"required recipe blocker missing: {recipe_id}",
        )
        inputs = recipe.get("inputs")
        require(isinstance(inputs, dict), f"recipe inputs missing: {recipe_id}")
        field_ids: set[str] = set()
        for collection_name in (
            "scalarOverrides",
            "vectorOverrides",
            "textureOverrides",
            "parentDefaults",
        ):
            for field in require_list(inputs.get(collection_name), f"{recipe_id}.{collection_name}"):
                require(isinstance(field, dict), f"invalid input field: {recipe_id}")
                field_id = str(field.get("fieldId") or "")
                require(bool(field_id) and field_id not in field_ids, f"duplicate input field ID: {recipe_id}")
                field_ids.add(field_id)
                require(
                    field_id not in all_input_fields,
                    f"duplicate global input field ID: {field_id}",
                )
                all_input_fields[field_id] = field
                require(
                    field.get("fidelity") == "SOURCE_EXACT_INPUT",
                    f"input field lost exact-input fidelity: {field_id}",
                )
                if field.get("bindingOrigin") == "PARENT_DEFAULT":
                    require(
                        field.get("closerOverridePresent") is False
                        and bool(
                            field.get("provenance", {}).get(
                                "inheritanceEdgeEvidenceId"
                            )
                        ),
                        f"parent default proof is incomplete: {field_id}",
                    )
        texture_fields = list(inputs["textureOverrides"]) + [
            field for field in inputs["parentDefaults"]
            if field.get("fieldKind") == "texture"
        ]
        for field in texture_fields:
            sampler = field.get("sampler")
            require(isinstance(sampler, dict), f"texture sampler state missing: {field['fieldId']}")
            if sampler.get("fidelity") == "SOURCE_EXACT_SAMPLER":
                require(
                    bool(sampler.get("bindingId"))
                    and isinstance(sampler.get("provenance"), dict),
                    f"exact sampler provenance is incomplete: {field['fieldId']}",
                )
                exact_sampler_field_ids.add(field["fieldId"])
            else:
                require(
                    set(sampler) == {"fidelity", "blocker"}
                    and sampler.get("blocker") == "SAMPLER_EVIDENCE_MISSING",
                    f"unproven sampler gained values: {field['fieldId']}",
                )

    sampler_field_ids: set[str] = set()
    for sampler in samplers:
        field_id = str(sampler.get("inputFieldId") or "")
        require(
            field_id in all_input_fields
            and field_id not in sampler_field_ids
            and sampler.get("fidelity") == "SOURCE_EXACT_SAMPLER"
            and sampler.get("bindingOrigin")
            == all_input_fields[field_id].get("bindingOrigin")
            and sampler.get("bindingId")
            == all_input_fields[field_id].get("sampler", {}).get("bindingId"),
            f"exact sampler binding join is invalid: {field_id}",
        )
        sampler_field_ids.add(field_id)
    require(
        sampler_field_ids == exact_sampler_field_ids,
        "exact sampler field set was laundered",
    )

    for occurrence in occurrences:
        require(isinstance(occurrence, dict), "contract occurrence must be an object")
        recipe_id = str(occurrence.get("materialRecipeId") or "")
        require(recipe_id in recipe_by_id, f"occurrence recipe is missing: {recipe_id}")
        recipe = recipe_by_id[recipe_id]
        require(
            occurrence.get("blockers") == recipe["blockers"]
            and int(occurrence.get("blockerCount", -1)) == recipe["blockerCount"]
            and occurrence.get("admission") == recipe["admission"],
            f"occurrence blocker set diverged: {occurrence.get('occurrenceId')}",
        )

    summary = contract.get("summary")
    require(isinstance(summary, dict), "contract summary missing")
    expected_summary = {
        "materialRecipeCount": EXPECTED_RECIPE_COUNT,
        "materialOccurrenceCount": EXPECTED_OCCURRENCE_COUNT,
        "scalarOverrideCount": EXPECTED_SCALAR_OVERRIDE_COUNT,
        "vectorOverrideCount": EXPECTED_VECTOR_OVERRIDE_COUNT,
        "directTextureOverrideCount": EXPECTED_TEXTURE_OVERRIDE_COUNT,
        "directTextureExactSamplerCount": 3,
        "directTextureUnprovenSamplerCount": 68,
        "parentDefaultExactSamplerCount": 1,
        "exactSamplerBindingCount": 4,
        "arithmeticFamilyCount": EXPECTED_GRAPH_FAMILY_COUNT,
        "cookedStrippedNullExpressionCount": EXPECTED_NULL_EXPRESSION_COUNT,
        "unresolvedGraphEdgeCount": EXPECTED_UNRESOLVED_EDGE_COUNT,
        "sourceExactGraphFamilyCount": 0,
        "reconstructedGraphFamilyCount": EXPECTED_GRAPH_FAMILY_COUNT,
        "sourceExactInputRecipeCount": EXPECTED_RECIPE_COUNT,
        "sourceExactStaticPermutationRecipeCount": 0,
        "sourceExactFullRenderStateRecipeCount": 0,
        "sourceExactPartialCullRecipeCount": 18,
        "implementedArithmeticEvaluatorCount": 0,
    }
    for name, expected in expected_summary.items():
        require(int(summary.get(name, -1)) == expected, f"contract summary drift: {name}")
    sampler_origins = Counter(row.get("bindingOrigin") for row in samplers)
    require(
        sampler_origins == Counter({"INSTANCE_OVERRIDE": 3, "PARENT_DEFAULT": 1}),
        "exact sampler origin denominator changed",
    )
    admission = contract.get("admission")
    require(isinstance(admission, dict), "contract aggregate admission missing")
    for name in (
        "executableRecipeCount",
        "productRecipeCount",
        "executableOccurrenceCount",
        "productOccurrenceCount",
    ):
        require(int(admission.get(name, -1)) == 0, f"Material Product admission opened: {name}")
    aggregate = sorted(
        {blocker for recipe in recipes for blocker in recipe["blockers"]}
    )
    require(admission.get("blockers") == aggregate, "aggregate blocker set was weakened")


def build_from_paths(
    active_inventory_path: Path,
    material_closure_path: Path,
    dds_receipt_path: Path,
    render_receipt_path: Path,
) -> dict[str, Any]:
    active_inventory = load_json(active_inventory_path)
    material_closure = load_json(material_closure_path)
    dds_receipt = load_json(dds_receipt_path)
    render_receipt = load_json(render_receipt_path)
    render_source = render_receipt.get("source", {}).get("activeMaterialClosure", {})
    require(
        render_source.get("hashDomain") == "TRACKED_DERIVED_EOL_CANONICAL_TEXT"
        and render_source.get("canonicalTextSha256")
        == tracked_json_text_sha256(material_closure_path),
        "render-state receipt material-closure hash mismatch",
    )
    for evidence_name, expected_path in (
        (
            "generator",
            REPO_ROOT
            / "Tools/LevelPlacementExtractor/"
            "extract_artist_31470_material_render_state.py",
        ),
        (
            "rawPackageParser",
            REPO_ROOT
            / "Tools/LevelPlacementExtractor/extract_ue3_placements.py",
        ),
    ):
        tool = render_receipt.get("source", {}).get(evidence_name, {})
        require(
            isinstance(tool, dict)
            and tool.get("path") == repository_path(expected_path)
            and tool.get("hashDomain")
            == "TRACKED_SOURCE_EOL_CANONICAL_TEXT"
            and tool.get("canonicalTextSha256")
            == tracked_source_text_sha256(expected_path),
            f"render-state receipt {evidence_name} hash mismatch",
        )
    source_evidence = {
        "activeInventory": {
            "path": repository_path(active_inventory_path),
            "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
            "canonicalTextSha256": tracked_json_text_sha256(active_inventory_path),
        },
        "activeMaterialClosure": {
            "path": repository_path(material_closure_path),
            "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
            "canonicalTextSha256": tracked_json_text_sha256(material_closure_path),
        },
        "exactDdsReceipt": {
            "path": repository_path(dds_receipt_path),
            "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
            "canonicalTextSha256": tracked_json_text_sha256(dds_receipt_path),
        },
        "renderStateReceipt": {
            "path": repository_path(render_receipt_path),
            "hashDomain": "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
            "canonicalTextSha256": tracked_json_text_sha256(render_receipt_path),
            "receiptSha256": render_receipt.get("receiptSha256"),
        },
        "externalSourcePackManifest": copy.deepcopy(
            dds_receipt.get("sourceEvidence", {}).get("sourcePackManifest", {})
        ),
    }
    source_evidence["externalSourcePackManifest"]["hashDomain"] = (
        "RAW_ARTIFACT_BYTES"
    )
    return build_contract(
        active_inventory,
        material_closure,
        dds_receipt,
        render_receipt,
        source_evidence,
    )


def unique_files_by_name(root: Path, pattern: str) -> dict[str, Path]:
    require(root.is_dir(), f"raw artifact root is missing: {root}")
    result: dict[str, Path] = {}
    for path in root.rglob(pattern):
        key = path.name.casefold()
        require(key not in result, f"duplicate raw artifact filename: {path.name}")
        result[key] = path
    return result


def verify_external_artifacts(
    dds_receipt: dict[str, Any],
    source_package_root: Path,
    exact_dds_root: Path,
    source_pack_manifest: Path,
) -> None:
    packages = unique_files_by_name(source_package_root, "*.upk")
    assets = validate_dds_receipt(dds_receipt)
    for asset in assets:
        texture = asset["sourceTexture2D"]
        package_name = str(texture.get("physicalPackage") or "")
        require(package_name.casefold() in packages, f"texture UPK is missing: {package_name}")
        package_path = packages[package_name.casefold()]
        require(
            package_path.stat().st_size == int(texture.get("physicalPackageByteCount", -1))
            and raw_file_sha256(package_path) == texture.get("physicalPackageSha256"),
            f"texture UPK raw bytes changed: {package_name}",
        )
        dds_path = exact_dds_root / str(asset.get("sourceExtractedDdsRelativePath") or "")
        require(dds_path.is_file(), f"exact DDS is missing: {dds_path}")
        require(
            dds_path.stat().st_size == int(asset["dds"].get("byteCount", -1))
            and raw_file_sha256(dds_path) == asset["dds"].get("sha256"),
            f"exact DDS raw bytes changed: {dds_path}",
        )
    manifest_evidence = dds_receipt.get("sourceEvidence", {}).get(
        "sourcePackManifest", {}
    )
    require(source_pack_manifest.is_file(), "source-pack manifest is missing")
    require(
        source_pack_manifest.stat().st_size
        == int(manifest_evidence.get("byteCount", -1))
        and raw_file_sha256(source_pack_manifest)
        == manifest_evidence.get("sha256"),
        "external source-pack manifest raw bytes changed",
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--material-closure", required=True, type=Path)
    parser.add_argument("--exact-dds-receipt", required=True, type=Path)
    parser.add_argument("--render-state-receipt", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--source-package-root", type=Path)
    parser.add_argument("--exact-dds-root", type=Path)
    parser.add_argument("--source-pack-manifest", type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    deep_values = (
        args.source_package_root,
        args.exact_dds_root,
        args.source_pack_manifest,
    )
    require(
        all(value is None for value in deep_values)
        or all(value is not None for value in deep_values),
        "deep raw verification requires source package root, DDS root, and manifest",
    )
    if all(value is not None for value in deep_values):
        verify_external_artifacts(
            load_json(args.exact_dds_receipt),
            args.source_package_root,
            args.exact_dds_root,
            args.source_pack_manifest,
        )
    contract = build_from_paths(
        args.active_inventory,
        args.material_closure,
        args.exact_dds_receipt,
        args.render_state_receipt,
    )
    check_or_write_tracked_json(args.output, contract, args.check)
    print(
        "Artist F typed Material evidence "
        f"{'check' if args.check else 'write'}: "
        f"recipes={contract['summary']['materialRecipeCount']} "
        f"occurrences={contract['summary']['materialOccurrenceCount']} "
        "samplers=3+1 graphs=23 reconstructed product=0"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
