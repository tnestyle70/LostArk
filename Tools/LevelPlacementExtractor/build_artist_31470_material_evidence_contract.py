#!/usr/bin/env python3
"""Build the fail-closed typed Material evidence contract for Artist F 31470."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import struct
import sys
from collections import Counter
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
CONTRACT_ROOT = "ARTIST/31470/F"
EXPECTED_SOURCE_PACK_MANIFEST_BYTE_COUNT = 270014
EXPECTED_SOURCE_PACK_MANIFEST_SHA256 = (
    "8ddce11f3cdd36efc4098b127da860b3e77e0f6916263412f1089cce3967d62d"
)

EXPECTED_RECIPE_COUNT = 27
EXPECTED_OCCURRENCE_COUNT = 34
EXPECTED_SCALAR_OVERRIDE_COUNT = 342
EXPECTED_VECTOR_OVERRIDE_COUNT = 19
EXPECTED_TEXTURE_OVERRIDE_COUNT = 71
EXPECTED_GRAPH_FAMILY_COUNT = 23
EXPECTED_NULL_EXPRESSION_COUNT = 1803
EXPECTED_UNRESOLVED_EDGE_COUNT = 502
EXPECTED_GRAPH_EXPRESSION_COUNT = 925
EXPECTED_RAW_PACKAGE_COUNT = 19
EXPECTED_OCCURRENCE_MATERIAL_JOIN_SHA256 = (
    "1c56ff7bf67dc94a61129372a0e71f57a74171ee47ddf57702cd88b95606b296"
)
EXPECTED_GRAPH_FAMILY_RAW_EVIDENCE_SHA256 = (
    "4783f79575c32d58499c40b7395d97fd62615c55033e97cd08db59d39f9727d7"
)
EXPECTED_RAW_MATERIAL_BINDING_SHA256 = (
    "b4074b05ffd517520df7ade9f3e5bf643af0553e15db0cd18d9275d1b78b4138"
)
EXPECTED_RAW_RENDER_FIELD_EVIDENCE_SHA256 = (
    "33706c4a19abf5ebb1bbf7647d1fca6f6eff4bbac3357d494e1eb06425cdaeff"
)
EXPECTED_OCCURRENCE_IDENTITY_SHA256 = (
    "faf2027f930a7905e87e867cb3a89b9ca75e2647b57872045dd2369dff5e6317"
)
EXPECTED_RECIPE_IDENTITY_SHA256 = (
    "f4cb7f8d5fae3699d55eb56a1a0442c6d55a0e5f062766a23fb654d665d24e22"
)
EXPECTED_RECIPE_FAMILY_JOIN_SHA256 = (
    "9877829577c550acb7452e8ff279c7819f17151513cee53994bee116620838f2"
)
EXPECTED_CONTRACT_RENDER_FIELD_EVIDENCE_SHA256 = (
    "6d5d70af3215c36509e86340af00aafceb94182f5c93b903c4ca951283a8d5b9"
)
EXPECTED_RAW_EXACT_INPUT_LINEAGE_SHA256 = (
    "c51178bebaf92f2583cef2f7e0b80750c643e9c09c7eb4bbd1e29ddf5708e8f2"
)
EXPECTED_CONTRACT_EXACT_INPUT_LINEAGE_SHA256 = (
    "891e93591f63b03466408829b8eb961f8a363889862db1132184285f6bd420e3"
)
EXPECTED_RAW_TEXTURE_EVIDENCE_SHA256 = (
    "f1e5ac9e39cd6dceda1c7353b4032b9359f59f4993f0a50f7ea8e09bf94b0d69"
)
EXPECTED_RECIPE_COMPOSITION_SHA256 = (
    "0214ba2113f429ee5d7a31d1ca4c560780ad65b1e741e22e85e3dfc3f914bf3c"
)

BLEND_MODE_DOMAIN = (
    "blend_opaque",
    "blend_masked",
    "blend_translucent",
    "blend_additive",
    "blend_modulate",
    "blend_softmasked",
    "blend_alphacomposite",
    "blend_ditheredtranslucent",
    "blend_max",
)
LIGHTING_MODEL_DOMAIN = (
    "mlm_phong",
    "mlm_nondirectional",
    "mlm_unlit",
    "mlm_shprtdiffuse",
    "mlm_custom",
    "mlm_anisotropic",
    "mlm_max",
)
TEXTURE_ADDRESS_DOMAIN = ("ta_wrap", "ta_clamp", "ta_mirror")

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
RAW_EXACT_RENDER_FIELDS = (
    "blendmode",
    "lightingmodel",
    "twosided",
    "bdisabledepthtest",
    "opacitymaskclipvalue",
    "buseonelayerdistortion",
    "overridedtwosided",
    "bhasstaticpermutationresource",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def folded(value: Any) -> str:
    return str(value or "").casefold()


def canonical_path_targets_object(
    canonical_path: Any, object_path: Any, logical_package: Any
) -> bool:
    canonical = folded(canonical_path)
    target = folded(object_path)
    logical = folded(logical_package)
    return bool(canonical and target and logical) and canonical in {
        target,
        f"{logical}.{target}",
    }


def exact_json_int(value: Any, expected: int, label: str) -> int:
    require(type(value) is int and value == expected, f"invalid exact integer: {label}")
    return value


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


def raw_export_evidence_id(
    package_sha256: Any, object_path: Any, class_name: Any
) -> str:
    payload = (
        f"{package_sha256}::{folded(object_path)}::{folded(class_name)}"
    ).encode()
    return "render-export-" + hashlib.sha256(payload).hexdigest()[:16]


def raw_expression_evidence_id(
    package_sha256: Any, object_path: Any, source_order: Any
) -> str:
    payload = (
        f"{package_sha256}::{folded(object_path)}::{int(source_order)}"
    ).encode()
    return "graph-expression-" + hashlib.sha256(payload).hexdigest()[:16]


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
        and type(closure.get("formatVersion")) is int
        and closure.get("formatVersion") == 1
        and closure.get("characterClass") == "ARTIST"
        and type(closure.get("skillId")) is int
        and closure.get("skillId") == 31470
        and closure.get("inputSlot") == "F",
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


def validate_enum_evidence(
    field: dict[str, Any], domain: tuple[str, ...], label: str
) -> None:
    value = folded(field.get("value"))
    require(value in domain, f"enum value is outside pinned domain: {label}")
    require(
        field.get("enumDomain") == list(domain)
        and int(field.get("enumOrdinal", -1)) == domain.index(value),
        f"enum domain/ordinal evidence changed: {label}",
    )


def comparable_instance_parameters(material: dict[str, Any]) -> dict[str, Any]:
    """Return the order-preserving raw projection admitted as exact input."""
    result: dict[str, Any] = {}
    for raw_name, closure_name in (
        ("scalar", "scalarParameters"),
        ("vector", "vectorParameters"),
        ("texture", "textureParameters"),
    ):
        values = require_list(material.get(closure_name), closure_name)
        projected: list[dict[str, Any]] = []
        for index, value in enumerate(values):
            require(isinstance(value, dict), f"invalid {closure_name}[{index}]")
            name = str(value.get("name") or "")
            require(bool(name.strip()), f"blank {closure_name}[{index}] name")
            if raw_name == "scalar":
                projected.append(
                    {"name": name, "value": finite_number(value.get("value"), name)}
                )
            elif raw_name == "vector":
                vector = require_list(value.get("value"), f"{closure_name}[{index}].value")
                require(len(vector) == 4, f"invalid vector width: {name}")
                projected.append(
                    {
                        "name": name,
                        "value": [
                            finite_number(component, f"{name}[{axis}]")
                            for axis, component in enumerate(vector)
                        ],
                    }
                )
            else:
                package_index = value.get("packageIndex")
                source_path = str(value.get("sourceObjectPath") or "")
                require(
                    isinstance(package_index, int)
                    and not isinstance(package_index, bool)
                    and package_index != 0
                    and bool(source_path),
                    f"invalid texture reference: {name}",
                )
                projected.append(
                    {
                        "name": name,
                        "sourceObjectPath": source_path,
                        "packageIndex": package_index,
                    }
                )
        result[raw_name] = projected
    return result


def comparable_expression_projection(expression: dict[str, Any]) -> dict[str, Any]:
    return {
        "sourceOrder": expression.get("sourceOrder"),
        "exportIndex": expression.get("exportIndex"),
        "className": expression.get("className"),
        "objectPath": expression.get("objectPath"),
        "parameterName": expression.get("parameterName"),
        "group": expression.get("group"),
        "defaultValue": expression.get("defaultValue"),
        "textureObjectPath": expression.get("textureObjectPath"),
        "inputs": expression.get("inputs"),
        "serialSize": expression.get("serialSize"),
        "propertyStreamEnd": expression.get("propertyStreamEnd"),
    }


def comparable_raw_expression_projection(expression: dict[str, Any]) -> dict[str, Any]:
    projection = copy.deepcopy(expression.get("projection"))
    require(isinstance(projection, dict), "raw graph expression projection is missing")
    projection.pop("texturePackageIndex", None)
    projection["inputs"] = [
        {
            key: value
            for key, value in input_row.items()
            if key != "propertyRecordSha256"
        }
        for input_row in require_list(projection.get("inputs"), "raw expression inputs")
    ]
    return projection


def immutable_export_identity(export: dict[str, Any]) -> dict[str, Any]:
    return {
        name: copy.deepcopy(export.get(name))
        for name in (
            "evidenceId",
            "logicalPackage",
            "physicalPackage",
            "physicalPackageSha256",
            "exportIndex",
            "packageReference",
            "objectPath",
            "className",
            "serialOffset",
            "serialSize",
            "serialSha256",
            "propertyStreamEnd",
            "trailingByteCount",
            "trailingBytesSha256",
        )
    }


def raw_exact_input_fixture_payload(
    rows: list[dict[str, Any]],
    bindings: dict[str, dict[str, Any]],
    exports: dict[str, dict[str, Any]],
    graph_expressions: dict[tuple[str, int], dict[str, Any]],
) -> list[dict[str, Any]]:
    fixture: list[dict[str, Any]] = []
    consumed_sources: set[str] = set()
    consumed_bases: set[str] = set()
    for row in rows:
        binding = bindings[folded(row["sourceMaterialPath"])]
        source_id = str(binding["sourceExportEvidenceId"])
        source = exports[source_id]
        if source_id not in consumed_sources:
            consumed_sources.add(source_id)
            for kind, field_name in (
                ("scalar", "scalarparametervalues"),
                ("vector", "vectorparametervalues"),
                ("texture", "textureparametervalues"),
            ):
                field = source["fields"].get(field_name)
                if not isinstance(field, dict) or field.get("status") != "SERIALIZED_EXPLICIT":
                    continue
                fixture.append(
                    {
                        "kind": "INSTANCE_ARRAY",
                        "parameterKind": kind,
                        "exportIdentity": immutable_export_identity(source),
                        "property": copy.deepcopy(field),
                        "decodedProjection": copy.deepcopy(
                            source.get("instanceParameters", {}).get(kind)
                        ),
                    }
                )
        base_id = str(binding["renderStateExportEvidenceId"])
        if base_id in consumed_bases:
            continue
        consumed_bases.add(base_id)
        base = exports[base_id]
        fixture.append(
            {
                "kind": "BASE_EXPRESSIONS_ARRAY",
                "exportIdentity": immutable_export_identity(base),
                "property": copy.deepcopy(base["fields"]["expressions"]),
            }
        )
        raw_references = require_list(
            base["fields"]["expressions"].get("value"),
            f"{base_id}.rawExpressions",
        )
        for source_order, reference in enumerate(raw_references):
            if reference == 0:
                continue
            expression = graph_expressions[(base_id, source_order)]
            projection = expression.get("projection", {})
            if not projection.get("parameterName") or (
                projection.get("defaultValue") is None
                and not projection.get("textureObjectPath")
            ):
                continue
            fixture.append(
                {
                    "kind": "PARAMETER_EXPRESSION",
                    "baseExportEvidenceId": base_id,
                    "baseExpressionsRecordSha256": base["fields"]["expressions"].get(
                        "recordSha256"
                    ),
                    "sourceOrder": source_order,
                    "rawReference": reference,
                    "expressionIdentity": immutable_export_identity(expression),
                    "fields": {
                        name: copy.deepcopy(expression.get("fields", {}).get(name))
                        for name in (
                            "parametername",
                            "defaultvalue",
                            "texture",
                        )
                    },
                    "decodedProjection": {
                        name: copy.deepcopy(projection.get(name))
                        for name in (
                            "parameterName",
                            "defaultValue",
                            "texturePackageIndex",
                            "textureObjectPath",
                        )
                    },
                }
            )
    return sorted(
        fixture,
        key=lambda row: canonical_sha256(row),
    )


def raw_texture_fixture_payload(
    texture_exports: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    return [
        copy.deepcopy(texture_exports[key]) for key in sorted(texture_exports)
    ]


def validate_render_receipt(
    receipt: dict[str, Any], rows: list[dict[str, Any]]
) -> tuple[
    dict[str, dict[str, Any]],
    dict[str, dict[str, Any]],
    dict[tuple[str, int], dict[str, Any]],
    dict[str, dict[str, Any]],
]:
    require(
        receipt.get("schema")
        == "lostark.artist-31470-material-render-state-evidence-receipt"
        and type(receipt.get("formatVersion")) is int
        and receipt.get("formatVersion") == 3
        and receipt.get("root") == CONTRACT_ROOT,
        "unsupported render-state evidence receipt",
    )
    validate_self_digest(receipt, "receiptSha256", "render-state receipt")
    manifest = receipt.get("source", {}).get("sourcePackManifest")
    require(
        isinstance(manifest, dict)
        and manifest.get("hashDomain") == "RAW_ARTIFACT_BYTES"
        and type(manifest.get("byteCount")) is int
        and manifest.get("byteCount") == EXPECTED_SOURCE_PACK_MANIFEST_BYTE_COUNT
        and manifest.get("sha256") == EXPECTED_SOURCE_PACK_MANIFEST_SHA256
        and type(manifest.get("schemaVersion")) is int
        and manifest.get("schemaVersion") == 1
        and manifest.get("identityStatus")
        == "AUTHENTICATED_IMMUTABLE_FIXTURE",
        "source-pack manifest provenance is not authenticated",
    )
    export_rows = require_list(receipt.get("exports"), "render-state exports")
    exports: dict[str, dict[str, Any]] = {}
    for export in export_rows:
        require(isinstance(export, dict), "render-state export must be an object")
        evidence_id = str(export.get("evidenceId") or "")
        require(bool(evidence_id), "render-state export evidence ID is blank")
        require(evidence_id not in exports, f"duplicate render export: {evidence_id}")
        require_sha256(export.get("physicalPackageSha256"), f"{evidence_id}.package")
        require_sha256(export.get("serialSha256"), f"{evidence_id}.serial")
        require(
            bool(export.get("logicalPackage"))
            and evidence_id
            == raw_export_evidence_id(
                export.get("physicalPackageSha256"),
                export.get("objectPath"),
                export.get("className"),
            )
            and type(export.get("exportIndex")) is int
            and type(export.get("packageReference")) is int
            and export.get("packageReference") == export.get("exportIndex") + 1
            and type(export.get("serialOffset")) is int
            and type(export.get("serialSize")) is int
            and export.get("serialOffset") >= 0
            and export.get("serialSize") > 0
            and type(export.get("propertyStreamEnd")) is int
            and type(export.get("trailingByteCount")) is int
            and 0 <= export.get("propertyStreamEnd") <= export.get("serialSize")
            and export.get("trailingByteCount")
            == export.get("serialSize") - export.get("propertyStreamEnd"),
            f"render export immutable identity is invalid: {evidence_id}",
        )
        require_sha256(
            export.get("trailingBytesSha256"), f"{evidence_id}.trailingBytes"
        )
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
                encoded_sha256 = require_sha256(
                    field.get("encodedValueSha256"),
                    f"{evidence_id}.{field_name}.value",
                )
                encoded_hex = str(field.get("encodedValueHex") or "")
                try:
                    encoded_bytes = bytes.fromhex(encoded_hex)
                except ValueError as error:
                    raise ValueError(
                        f"invalid encoded tagged-property bytes: {evidence_id}.{field_name}"
                    ) from error
                require(
                    bool(encoded_bytes)
                    and hashlib.sha256(encoded_bytes).hexdigest() == encoded_sha256,
                    f"encoded tagged-property hash changed: {evidence_id}.{field_name}",
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
                    validate_enum_evidence(
                        field,
                        BLEND_MODE_DOMAIN
                        if normalized_field == "blendmode"
                        else LIGHTING_MODEL_DOMAIN,
                        f"{evidence_id}.{field_name}",
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
                    require(
                        len(encoded_bytes) == 1
                        and encoded_bytes[0] in {0, 1}
                        and value is bool(encoded_bytes[0]),
                        f"render boolean disagrees with raw tagged bytes: {evidence_id}.{field_name}",
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
        require(
            canonical_path_targets_object(
                path,
                source_export.get("objectPath"),
                source_export.get("logicalPackage"),
            ),
            f"canonical source Material path does not target raw export: {path}",
        )
        expected_source_identity = {
            "canonicalSourceMaterialPath": path,
            "logicalPackage": source_export["logicalPackage"],
            "physicalPackage": source_export["physicalPackage"],
            "physicalPackageSha256": source_export["physicalPackageSha256"],
            "exportIndex": source_export["exportIndex"],
            "objectPath": source_export["objectPath"],
            "rawExportEvidenceId": source_export["evidenceId"],
            "propertyStreamEnd": source_export["propertyStreamEnd"],
            "trailingByteCount": source_export["trailingByteCount"],
            "trailingBytesSha256": source_export["trailingBytesSha256"],
        }
        require(
            binding.get("sourceMaterialIdentity") == expected_source_identity,
            f"canonical source Material identity binding changed: {path}",
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
            require(
                source_export.get("instanceParameters")
                == comparable_instance_parameters(material),
                f"raw instance parameter projection disagrees with closure: {path}",
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
        raw_parent_reference = None
        if folded(material.get("className")) == "materialinstanceconstant":
            raw_parent_reference = source_export["fields"]["parent"].get(
                "resolvedObjectPath"
            )
            require(
                canonical_path_targets_object(
                    raw_parent_reference,
                    base_export.get("objectPath"),
                    base_export.get("logicalPackage"),
                )
                and canonical_path_targets_object(
                    material.get("parent"),
                    base_export.get("objectPath"),
                    base_export.get("logicalPackage"),
                ),
                f"raw MIC Parent does not select parentGraph export: {path}",
            )
        else:
            require(
                source_export["evidenceId"] == base_export["evidenceId"],
                f"raw Material graph does not target source export: {path}",
            )
        expected_graph_identity = {
            "logicalPackage": base_export["logicalPackage"],
            "physicalPackage": base_export["physicalPackage"],
            "physicalPackageSha256": base_export["physicalPackageSha256"],
            "exportIndex": base_export["exportIndex"],
            "objectPath": base_export["objectPath"],
            "rawExportEvidenceId": base_export["evidenceId"],
            "rawParentReferencePath": raw_parent_reference,
            "propertyStreamEnd": base_export["propertyStreamEnd"],
            "trailingByteCount": base_export["trailingByteCount"],
            "trailingBytesSha256": base_export["trailingBytesSha256"],
        }
        require(
            binding.get("selectedGraphIdentity") == expected_graph_identity,
            f"selected parentGraph identity binding changed: {path}",
        )

    graph_expressions: dict[tuple[str, int], dict[str, Any]] = {}
    graph_expressions_by_id: dict[str, dict[str, Any]] = {}
    for expression in require_list(
        receipt.get("graphExpressions"), "raw graph-expression evidence"
    ):
        require(isinstance(expression, dict), "raw graph expression must be an object")
        evidence_id = str(expression.get("evidenceId") or "")
        base_id = str(expression.get("baseMaterialEvidenceId") or "")
        source_order = int(expression.get("sourceOrder", -1))
        require(
            bool(evidence_id)
            and evidence_id not in graph_expressions_by_id
            and base_id in exports
            and source_order >= 0
            and evidence_id
            == raw_expression_evidence_id(
                expression.get("physicalPackageSha256"),
                expression.get("objectPath"),
                source_order,
            ),
            f"invalid raw graph-expression identity: {evidence_id}",
        )
        key = (base_id, source_order)
        require(key not in graph_expressions, f"duplicate raw graph expression: {key}")
        require_sha256(expression.get("physicalPackageSha256"), f"{evidence_id}.package")
        require_sha256(expression.get("serialSha256"), f"{evidence_id}.serial")
        require(
            int(expression.get("packageReference", -1))
            == int(expression.get("exportIndex", -2)) + 1
            == int(expression.get("rawReferenceFromBaseExpressions", -3)),
            f"raw graph-expression reference changed: {evidence_id}",
        )
        base = exports[base_id]
        require(
            folded(expression.get("physicalPackage"))
            == folded(base.get("physicalPackage"))
            and expression.get("physicalPackageSha256")
            == base.get("physicalPackageSha256")
            and folded(expression.get("logicalPackage"))
            == folded(base.get("logicalPackage"))
            and folded(expression.get("className")).startswith("materialexpression")
            and bool(expression.get("objectPath")),
            f"raw graph-expression package/class identity changed: {evidence_id}",
        )
        for field_name, field in expression.get("fields", {}).items():
            require(isinstance(field, dict), f"invalid expression field: {evidence_id}.{field_name}")
            status = field.get("status")
            require(
                status in {"SERIALIZED_EXPLICIT", "OMITTED_FROM_EXPORT"},
                f"invalid expression field status: {evidence_id}.{field_name}",
            )
            if status == "SERIALIZED_EXPLICIT":
                require_sha256(field.get("recordSha256"), f"{evidence_id}.{field_name}.record")
                require_sha256(field.get("encodedValueSha256"), f"{evidence_id}.{field_name}.value")
        graph_expressions[key] = expression
        graph_expressions_by_id[evidence_id] = expression

    consumed_expression_ids: set[str] = set()
    consumed_base_ids: set[str] = set()
    for row in rows:
        _, holder = graph_holder(row)
        graph = holder["graph"]
        binding = bindings[folded(row["sourceMaterialPath"])]
        base_id = str(binding["renderStateExportEvidenceId"])
        if base_id in consumed_base_ids:
            continue
        consumed_base_ids.add(base_id)
        expressions_field = exports[base_id]["fields"].get("expressions")
        require(
            isinstance(expressions_field, dict)
            and expressions_field.get("status") == "SERIALIZED_EXPLICIT",
            f"raw base Expressions array is missing: {base_id}",
        )
        raw_references = require_list(
            expressions_field.get("value"), f"{base_id}.expressions"
        )
        summary = graph.get("summary", {})
        require(
            len(raw_references) == int(summary.get("expressionEntryCount", -1))
            and sum(reference != 0 for reference in raw_references)
            == int(summary.get("nonNullExpressionCount", -1)),
            f"raw expression denominator disagrees with graph closure: {base_id}",
        )
        closure_by_order: dict[int, dict[str, Any]] = {}
        for closure_expression in require_list(
            graph.get("expressions"), f"{base_id}.closureExpressions"
        ):
            source_order = int(closure_expression.get("sourceOrder", -1))
            require(
                source_order >= 0 and source_order not in closure_by_order,
                f"duplicate closure graph expression order: {base_id}:{source_order}",
            )
            closure_by_order[source_order] = closure_expression
        require(
            len(closure_by_order) == sum(reference != 0 for reference in raw_references),
            f"closure graph expression coverage changed: {base_id}",
        )
        for source_order, raw_reference in enumerate(raw_references):
            if raw_reference == 0:
                require(
                    source_order not in closure_by_order,
                    f"closure expression occupies a raw null slot: {base_id}:{source_order}",
                )
                continue
            key = (base_id, source_order)
            require(key in graph_expressions, f"raw graph-expression evidence missing: {key}")
            expression = graph_expressions[key]
            require(
                int(expression.get("rawReferenceFromBaseExpressions", -1))
                == raw_reference
                and comparable_raw_expression_projection(expression)
                == comparable_expression_projection(closure_by_order[source_order]),
                f"raw graph-expression projection disagrees with closure: {key}",
            )
            consumed_expression_ids.add(str(expression["evidenceId"]))
    require(
        len(consumed_base_ids) == EXPECTED_GRAPH_FAMILY_COUNT
        and len(graph_expressions_by_id) == EXPECTED_GRAPH_EXPRESSION_COUNT
        and consumed_expression_ids == set(graph_expressions_by_id),
        "raw graph-expression evidence coverage changed",
    )

    texture_exports: dict[str, dict[str, Any]] = {}
    for texture_row in require_list(
        receipt.get("textureSamplerExports"), "raw texture sampler exports"
    ):
        require(isinstance(texture_row, dict), "raw texture sampler row must be an object")
        key = folded(texture_row.get("logicalObjectPath"))
        require(key in EXPECTED_DDS_BINDINGS, f"unexpected raw Texture2D evidence: {key}")
        require(key not in texture_exports, f"duplicate raw Texture2D evidence: {key}")
        export = texture_row.get("export")
        sampling = texture_row.get("sampling")
        dds = texture_row.get("dds")
        require(
            isinstance(export, dict)
            and isinstance(sampling, dict)
            and isinstance(dds, dict)
            and folded(export.get("className")) == "texture2d"
            and folded(export.get("objectPath")) == key.split(".")[-1]
            and int(export.get("packageReference", -1))
            == int(export.get("exportIndex", -2)) + 1,
            f"raw Texture2D identity is invalid: {key}",
        )
        require_sha256(export.get("physicalPackageSha256"), f"{key}.package")
        require_sha256(export.get("serialSha256"), f"{key}.serial")
        for axis, enum_field in (("addressU", "addressx"), ("addressV", "addressy")):
            field = export.get("fields", {}).get(enum_field)
            require(isinstance(field, dict), f"raw sampler field missing: {key}.{enum_field}")
            if field.get("status") == "SERIALIZED_EXPLICIT":
                validate_enum_evidence(field, TEXTURE_ADDRESS_DOMAIN, f"{key}.{enum_field}")
                require(
                    sampling.get(axis) == folded(field.get("value")).removeprefix("ta_"),
                    f"raw sampler projection changed: {key}.{enum_field}",
                )
            else:
                require(
                    field.get("status") == "OMITTED_FROM_EXPORT"
                    and sampling.get(f"{axis}Evidence") == "UE3_TEXTURE_CLASS_DEFAULT",
                    f"raw sampler default provenance changed: {key}.{enum_field}",
                )
        require(
            int(dds.get("byteCount", -1)) == EXPECTED_DDS_BINDINGS[key]["ddsByteCount"]
            and dds.get("sha256") == EXPECTED_DDS_BINDINGS[key]["ddsSha256"],
            f"raw Texture2D DDS binding changed: {key}",
        )
        texture_exports[key] = texture_row
    require(
        set(texture_exports) == set(EXPECTED_DDS_BINDINGS),
        "raw Texture2D evidence coverage changed",
    )
    exact_input_fixture_sha256 = canonical_sha256(
        raw_exact_input_fixture_payload(
            rows, bindings, exports, graph_expressions
        )
    )
    if EXPECTED_RAW_EXACT_INPUT_LINEAGE_SHA256:
        require(
            exact_input_fixture_sha256
            == EXPECTED_RAW_EXACT_INPUT_LINEAGE_SHA256,
            "raw exact-input lineage fixture changed",
        )
    raw_texture_sha256 = canonical_sha256(
        raw_texture_fixture_payload(texture_exports)
    )
    if EXPECTED_RAW_TEXTURE_EVIDENCE_SHA256:
        require(
            raw_texture_sha256 == EXPECTED_RAW_TEXTURE_EVIDENCE_SHA256,
            "raw Texture2D evidence fixture changed",
        )

    raw_binding_fixture = sorted(
        [
            {
                "sourceMaterialPath": binding["sourceMaterialPath"],
                "sourceMaterialIdentity": binding["sourceMaterialIdentity"],
                "selectedGraphIdentity": binding["selectedGraphIdentity"],
            }
            for binding in bindings.values()
        ],
        key=lambda row: folded(row["sourceMaterialPath"]),
    )
    require(
        canonical_sha256(raw_binding_fixture)
        == EXPECTED_RAW_MATERIAL_BINDING_SHA256,
        "raw Material/parentGraph binding fixture changed",
    )
    raw_render_fixture: list[dict[str, Any]] = []
    for export in sorted(exports.values(), key=lambda row: row["evidenceId"]):
        for field_name in sorted(RAW_EXACT_RENDER_FIELDS):
            field = export["fields"].get(field_name)
            if not isinstance(field, dict) or field.get("status") != "SERIALIZED_EXPLICIT":
                continue
            raw_render_fixture.append(
                {
                    "exportEvidenceId": export["evidenceId"],
                    "exportIdentity": {
                        name: export[name]
                        for name in (
                            "logicalPackage",
                            "physicalPackage",
                            "physicalPackageSha256",
                            "exportIndex",
                            "packageReference",
                            "objectPath",
                            "className",
                            "serialOffset",
                            "serialSize",
                            "serialSha256",
                        )
                    },
                    "fieldName": field_name,
                    "field": field,
                }
            )
    require(
        canonical_sha256(raw_render_fixture)
        == EXPECTED_RAW_RENDER_FIELD_EVIDENCE_SHA256,
        "raw exact render-field evidence fixture changed",
    )

    summary = receipt.get("summary")
    require(isinstance(summary, dict), "render-state receipt summary missing")
    expected_summary = {
        "materialRecipeCount": EXPECTED_RECIPE_COUNT,
        "uniqueRawExportCount": 48,
        "rawPackageCount": EXPECTED_RAW_PACKAGE_COUNT,
        "uniqueBaseMaterialGraphCount": EXPECTED_GRAPH_FAMILY_COUNT,
        "graphExpressionEvidenceCount": EXPECTED_GRAPH_EXPRESSION_COUNT,
        "textureSamplerExportCount": 4,
        "sourceMaterialInstanceCount": 25,
        "sourceRawMaterialCount": 2,
    }
    for name, expected in expected_summary.items():
        require(int(summary.get(name, -1)) == expected, f"render receipt summary drift: {name}")
    return bindings, exports, graph_expressions, texture_exports


def nested_struct_property(row: Any, name: str, label: str) -> dict[str, Any]:
    require(isinstance(row, dict), f"raw struct row is invalid: {label}")
    by_name = {folded(key): value for key, value in row.items()}
    value = by_name.get(name.casefold())
    require(isinstance(value, dict), f"raw struct property is missing: {label}.{name}")
    return value


def raw_linear_color(value: Any, label: str) -> list[float]:
    require(
        isinstance(value, dict)
        and type(value.get("size")) is int
        and value.get("size") == 16,
        f"raw linear color shape is invalid: {label}",
    )
    encoded_hex = str(value.get("hex") or "")
    try:
        encoded = bytes.fromhex(encoded_hex)
    except ValueError as error:
        raise ValueError(f"raw linear color bytes are invalid: {label}") from error
    require(len(encoded) == 16, f"raw linear color byte count changed: {label}")
    return [float(component) for component in struct.unpack("<4f", encoded)]


def immutable_property_lineage(
    export: dict[str, Any], field: dict[str, Any], label: str
) -> dict[str, Any]:
    require(
        field.get("status") == "SERIALIZED_EXPLICIT",
        f"exact property lineage is not explicit: {label}",
    )
    encoded_sha256 = require_sha256(
        field.get("encodedValueSha256"), f"{label}.encodedValue"
    )
    encoded_hex = str(field.get("encodedValueHex") or "")
    try:
        encoded_bytes = bytes.fromhex(encoded_hex)
    except ValueError as error:
        raise ValueError(f"exact property bytes are invalid: {label}") from error
    require(
        bool(encoded_bytes)
        and hashlib.sha256(encoded_bytes).hexdigest() == encoded_sha256,
        f"exact property bytes changed: {label}",
    )
    record_sha256 = require_sha256(field.get("recordSha256"), f"{label}.record")
    return {
        "export": immutable_export_identity(export),
        "property": {
            name: copy.deepcopy(field.get(name))
            for name in (
                "propertyName",
                "arrayIndex",
                "propertyType",
                "structType",
                "declaredDataSize",
                "serializedPayloadSize",
                "tagOffset",
                "valueOffset",
                "recordEndOffset",
                "absoluteLogicalTagOffset",
                "absoluteLogicalValueOffset",
                "absoluteLogicalRecordEndOffset",
            )
        },
        "encodedValueHex": encoded_hex,
        "encodedValueSha256": encoded_sha256,
        "recordSha256": record_sha256,
    }


def validate_property_lineage(lineage: Any, label: str) -> None:
    require(isinstance(lineage, dict), f"property lineage is missing: {label}")
    export = lineage.get("export")
    property_identity = lineage.get("property")
    require(
        isinstance(export, dict)
        and isinstance(property_identity, dict)
        and bool(export.get("evidenceId"))
        and type(export.get("exportIndex")) is int
        and type(export.get("packageReference")) is int
        and export.get("packageReference") == export.get("exportIndex") + 1,
        f"property lineage owner is invalid: {label}",
    )
    require_sha256(export.get("physicalPackageSha256"), f"{label}.package")
    require_sha256(export.get("serialSha256"), f"{label}.serial")
    encoded_sha256 = require_sha256(
        lineage.get("encodedValueSha256"), f"{label}.encodedValue"
    )
    encoded_hex = str(lineage.get("encodedValueHex") or "")
    try:
        encoded_bytes = bytes.fromhex(encoded_hex)
    except ValueError as error:
        raise ValueError(f"property lineage bytes are invalid: {label}") from error
    require(
        bool(encoded_bytes)
        and hashlib.sha256(encoded_bytes).hexdigest() == encoded_sha256,
        f"property lineage bytes changed: {label}",
    )
    require_sha256(lineage.get("recordSha256"), f"{label}.record")
    require(
        type(property_identity.get("arrayIndex")) is int
        and type(property_identity.get("declaredDataSize")) is int
        and type(property_identity.get("serializedPayloadSize")) is int
        and type(property_identity.get("tagOffset")) is int
        and type(property_identity.get("valueOffset")) is int
        and type(property_identity.get("recordEndOffset")) is int
        and 0 <= property_identity["tagOffset"]
        <= property_identity["valueOffset"]
        <= property_identity["recordEndOffset"]
        <= export["serialSize"],
        f"property lineage offsets are invalid: {label}",
    )


def raw_instance_parameter_projection(
    source_export: dict[str, Any],
    array_record: dict[str, Any],
    kind: str,
    label: str,
) -> list[dict[str, Any]]:
    raw_rows = require_list(array_record.get("value"), f"{label}.rawArray")
    projected_rows = require_list(
        source_export.get("instanceParameters", {}).get(kind),
        f"{label}.rawProjection",
    )
    require(
        len(raw_rows) == len(projected_rows),
        f"raw parameter array/projection count changed: {label}",
    )
    result: list[dict[str, Any]] = []
    for index, (raw_row, projected) in enumerate(zip(raw_rows, projected_rows)):
        name_field = nested_struct_property(
            raw_row, "parametername", f"{label}[{index}]"
        )
        value_field = nested_struct_property(
            raw_row, "parametervalue", f"{label}[{index}]"
        )
        guid_field = nested_struct_property(
            raw_row, "expressionguid", f"{label}[{index}]"
        )
        name = str(name_field.get("value") or "")
        require(
            folded(name_field.get("type")) == "nameproperty" and bool(name),
            f"raw parameter name is invalid: {label}[{index}]",
        )
        require(isinstance(projected, dict), f"raw parameter projection is invalid: {label}[{index}]")
        require(
            projected.get("name") == name,
            f"raw parameter name disagrees with projection: {label}[{index}]",
        )
        decoded: dict[str, Any] = {
            "parameterName": name,
            "normalizedParameterName": name.casefold(),
        }
        if kind == "scalar":
            raw_value = finite_number(
                value_field.get("value"), f"{label}[{index}].value"
            )
            require(
                folded(value_field.get("type")) == "floatproperty"
                and projected.get("value") == raw_value,
                f"raw scalar value disagrees with projection: {label}[{index}]",
            )
            decoded["value"] = raw_value
        elif kind == "vector":
            require(
                folded(value_field.get("type")) == "structproperty"
                and folded(value_field.get("structType")) == "linearcolor",
                f"raw vector type is invalid: {label}[{index}]",
            )
            raw_value = raw_linear_color(
                value_field.get("value"), f"{label}[{index}].value"
            )
            require(
                projected.get("value") == raw_value,
                f"raw vector value disagrees with projection: {label}[{index}]",
            )
            decoded["value"] = raw_value
        elif kind == "texture":
            raw_reference = value_field.get("value")
            require(
                folded(value_field.get("type")) == "objectproperty"
                and type(raw_reference) is int
                and raw_reference != 0
                and projected.get("packageIndex") == raw_reference
                and bool(projected.get("sourceObjectPath")),
                f"raw texture reference disagrees with projection: {label}[{index}]",
            )
            decoded["value"] = str(projected["sourceObjectPath"])
            decoded["packageIndex"] = raw_reference
        else:
            raise ValueError(f"unsupported raw parameter kind: {kind}")
        guid_value = guid_field.get("value")
        require(
            folded(guid_field.get("type")) == "structproperty"
            and folded(guid_field.get("structType")) == "guid"
            and isinstance(guid_value, dict)
            and type(guid_value.get("size")) is int
            and guid_value.get("size") == 16
            and len(str(guid_value.get("hex") or "")) == 32,
            f"raw expression GUID is invalid: {label}[{index}]",
        )
        decoded["expressionGuidHex"] = str(guid_value["hex"])
        result.append(decoded)
    return result


def parameter_rows(
    row: dict[str, Any],
    kind: str,
    array_name: str,
    array_record: dict[str, Any] | None,
    source_export: dict[str, Any],
) -> list[dict[str, Any]]:
    material_path = str(row["sourceMaterialPath"])
    values = require_list(row["material"].get(array_name), f"{material_path}.{array_name}")
    seen: set[str] = set()
    result: list[dict[str, Any]] = []
    if not values:
        require(
            require_list(
                source_export.get("instanceParameters", {}).get(kind, []),
                f"{material_path}.{array_name}.rawProjection",
            )
            == [],
            f"raw empty parameter projection changed: {material_path}.{array_name}",
        )
        return result
    require(
        isinstance(array_record, dict)
        and array_record.get("status") == "SERIALIZED_EXPLICIT",
        f"raw parameter array lineage missing: {material_path}.{array_name}",
    )
    raw_projection = raw_instance_parameter_projection(
        source_export,
        array_record,
        kind,
        f"{material_path}.{array_name}",
    )
    property_lineage = immutable_property_lineage(
        source_export,
        array_record,
        f"{material_path}.{array_name}",
    )
    recipe_id = stable_id("material-recipe", material_path)
    require(
        len(raw_projection) == len(values),
        f"closure/raw parameter count changed: {material_path}.{array_name}",
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
        raw_value = raw_projection[index]
        require(
            raw_value["parameterName"] == name
            and raw_value["normalizedParameterName"] == key
            and raw_value["value"] == decoded_value,
            f"closure input disagrees with raw tagged array: {material_path}.{name}",
        )
        if kind == "texture":
            require(
                raw_value["packageIndex"] == int(value["packageIndex"]),
                f"closure texture reference disagrees with raw tagged array: {material_path}.{name}",
            )
        lineage = {
            "owner": {
                "recipeId": recipe_id,
                "canonicalSourceMaterialPath": material_path,
                "rawMaterialExport": immutable_export_identity(source_export),
            },
            "fieldKind": kind,
            "bindingOrigin": "INSTANCE_OVERRIDE",
            "serializedArrayIndex": index,
            "decodedCanonicalValue": copy.deepcopy(raw_value),
            "propertyLineage": property_lineage,
        }
        lineage_sha256 = canonical_sha256(lineage)
        field_id = stable_id(
            "material-input",
            material_path,
            kind,
            index,
            name,
            "INSTANCE_OVERRIDE",
            lineage_sha256,
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
                "parameterArrayRecordSha256": array_record["recordSha256"],
                "lineage": lineage,
                "lineageSha256": lineage_sha256,
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
    graph_expressions: dict[tuple[str, int], dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    material_path = str(row["sourceMaterialPath"])
    material_class = folded(row["material"].get("className"))
    holder_name, holder = graph_holder(row)
    graph = holder["graph"]
    base_export = exports[str(binding["renderStateExportEvidenceId"])]
    source_export = exports[str(binding["sourceExportEvidenceId"])]
    binding_origin = (
        "PARENT_DEFAULT"
        if material_class == "materialinstanceconstant"
        else "SELF_DEFAULT"
    )
    if binding_origin == "PARENT_DEFAULT":
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
        raw_expression = graph_expressions.get(
            (str(base_export["evidenceId"]), source_order)
        )
        require(
            isinstance(raw_expression, dict)
            and int(raw_expression.get("exportIndex", -1)) == expression_export
            and folded(raw_expression.get("objectPath"))
            == folded(expression.get("objectPath")),
            f"raw parent-default expression join missing: {material_path}.{name}",
        )
        raw_field_name = "texture" if kind == "texture" else "defaultvalue"
        raw_default_field = raw_expression.get("fields", {}).get(raw_field_name)
        exact_value_proven = (
            isinstance(raw_default_field, dict)
            and raw_default_field.get("status") == "SERIALIZED_EXPLICIT"
            and (
                (
                    kind == "texture"
                    and folded(raw_default_field.get("resolvedObjectPath"))
                    == folded(texture_path)
                    and isinstance(raw_default_field.get("value"), int)
                    and not isinstance(raw_default_field.get("value"), bool)
                    and int(raw_default_field["value"]) != 0
                )
                or (
                    kind != "texture"
                    and raw_default_field.get("value") == default_value
                )
            )
        )
        fidelity = (
            "SOURCE_EXACT_INPUT"
            if exact_value_proven
            else "UNRESOLVED_PARENT_DEFAULT_EVIDENCE"
        )
        property_lineage = (
            immutable_property_lineage(
                raw_expression,
                raw_default_field,
                f"{material_path}.{name}.parentDefault",
            )
            if exact_value_proven
            else None
        )
        decoded_canonical_value: dict[str, Any] = {
            "parameterName": name,
            "normalizedParameterName": normalized_name,
            "value": copy.deepcopy(value),
        }
        if kind == "texture" and exact_value_proven:
            decoded_canonical_value["packageIndex"] = int(
                raw_default_field["value"]
            )
        lineage = {
            "owner": {
                "recipeId": stable_id("material-recipe", material_path),
                "canonicalSourceMaterialPath": material_path,
                "rawMaterialExport": immutable_export_identity(source_export),
            },
            "fieldKind": kind,
            "bindingOrigin": binding_origin,
            "expressionSourceOrder": source_order,
            "expressionExportIndex": expression_export,
            "parameterName": name,
            "normalizedParameterName": normalized_name,
            "rawBaseMaterialExport": immutable_export_identity(base_export),
            "rawExpressionExport": immutable_export_identity(raw_expression),
            "decodedCanonicalValue": decoded_canonical_value,
            "propertyLineage": property_lineage,
            "inheritanceEdgeEvidenceId": (
                binding["sourceExportEvidenceId"]
                if binding_origin == "PARENT_DEFAULT"
                else None
            ),
        }
        lineage_sha256 = canonical_sha256(lineage)
        evidence = {
            "fieldId": stable_id(
                "material-input",
                material_path,
                kind,
                source_order,
                name,
                binding_origin,
                lineage_sha256,
            ),
            "fieldKind": kind,
            "parameterName": name,
            "normalizedParameterName": normalized_name,
            "expressionSourceOrder": source_order,
            "expressionExportIndex": expression_export,
            "expressionObjectPath": expression.get("objectPath"),
            "bindingOrigin": binding_origin,
            "value": value,
            "fidelity": fidelity,
            "closerOverridePresent": normalized_name in override_names.get(kind, set()),
            "provenance": {
                "physicalPackage": base_export["physicalPackage"],
                "physicalPackageSha256": base_export["physicalPackageSha256"],
                "materialObjectPath": base_export["objectPath"],
                "materialExportIndex": base_export["exportIndex"],
                "graphHolder": holder_name,
                "graphExpressionEvidenceId": raw_expression["evidenceId"],
                "graphExpressionSerialSha256": raw_expression["serialSha256"],
                "valuePropertyRecordSha256": (
                    raw_default_field.get("recordSha256")
                    if isinstance(raw_default_field, dict)
                    and raw_default_field.get("status") == "SERIALIZED_EXPLICIT"
                    else None
                ),
                "inheritanceEdgeEvidenceId": (
                    binding["sourceExportEvidenceId"]
                    if binding_origin == "PARENT_DEFAULT"
                    else None
                ),
                "lineage": lineage,
                "lineageSha256": lineage_sha256,
            },
        }
        if not exact_value_proven:
            evidence["blocker"] = "PARENT_DEFAULT_VALUE_EVIDENCE_UNRESOLVED"
        if kind == "staticSwitch":
            evidence["selectionRole"] = "PARENT_DEFAULT_NOT_INSTANCE_SELECTION"
            static_defaults.append(evidence)
            continue
        if evidence["closerOverridePresent"]:
            continue
        if kind == "texture":
            evidence["texturePackageIndex"] = (
                int(raw_default_field["value"])
                if exact_value_proven
                else None
            )
            evidence["sampler"] = {
                "fidelity": "UNRESOLVED",
                "blocker": "SAMPLER_EVIDENCE_MISSING",
            }
        defaults.append(evidence)
    return defaults, static_defaults


def build_graph_families(
    rows: list[dict[str, Any]],
    bindings: dict[str, dict[str, Any]],
    exports: dict[str, dict[str, Any]],
    graph_expressions: dict[tuple[str, int], dict[str, Any]],
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
        summary = graph["summary"]
        binding = bindings[folded(row["sourceMaterialPath"])]
        base_id = str(binding["renderStateExportEvidenceId"])
        raw_references = require_list(
            exports[base_id]["fields"]["expressions"].get("value"),
            f"{base_id}.rawExpressions",
        )
        non_null = sum(reference != 0 for reference in raw_references)
        raw_expression_rows = [
            graph_expressions[(base_id, source_order)]
            for source_order, reference in enumerate(raw_references)
            if reference != 0
        ]
        unresolved_edges = sum(
            int(input_row.get("packageIndex", -1)) == 0
            for expression in raw_expression_rows
            for input_row in require_list(
                expression.get("projection", {}).get("inputs"),
                f"{expression['evidenceId']}.inputs",
            )
        )
        raw_counts = {
            "expressionEntryCount": len(raw_references),
            "nonNullExpressionCount": non_null,
            "nullExpressionCount": len(raw_references) - non_null,
            "unresolvedInputEdgeCount": unresolved_edges,
        }
        for name, value in raw_counts.items():
            require(
                int(summary.get(name, -1)) == value,
                f"graph summary disagrees with raw expression evidence: {graph_path}.{name}",
            )
        expression_evidence_digest = canonical_sha256(
            [
                {
                    "evidenceId": expression["evidenceId"],
                    "sourceOrder": expression["sourceOrder"],
                    "rawReference": expression["rawReferenceFromBaseExpressions"],
                    "inputPackageIndices": [
                        input_row.get("packageIndex")
                        for input_row in expression["projection"]["inputs"]
                    ],
                }
                for expression in raw_expression_rows
            ]
        )
        exact_identity = {
            "physicalPackage": holder["physicalPackage"],
            "physicalPackageSha256": package_sha,
            "materialObjectPath": graph_path,
            "materialExportIndex": int(graph.get("materialExportIndex", -1)),
        }
        raw_evidence = {
            **raw_counts,
            "baseMaterialEvidenceId": base_id,
            "expressionsRecordSha256": exports[base_id]["fields"][
                "expressions"
            ]["recordSha256"],
            "expressionEvidenceSha256": expression_evidence_digest,
        }
        family_id = stable_id(
            "material-family",
            canonical_sha256(exact_identity),
            canonical_sha256(raw_evidence),
        )
        family = {
            "familyId": family_id,
            "graphProvenance": "RECONSTRUCTED_GRAPH",
            "sourceExactGraph": False,
            "exactIdentity": exact_identity,
            "cookedEvidence": {
                "topologyStatus": summary["topologyStatus"],
                **raw_counts,
            },
            "rawEvidence": raw_evidence,
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
        family["identitySha256"] = canonical_sha256(
            {
                "exactIdentity": family["exactIdentity"],
                "rawEvidence": family["rawEvidence"],
            }
        )
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
                "encodedValueHex": raw["encodedValueHex"],
                "encodedValueSha256": raw["encodedValueSha256"],
                "propertyType": raw["propertyType"],
                "tagOffset": raw["tagOffset"],
                "valueOffset": raw["valueOffset"],
                "recordEndOffset": raw["recordEndOffset"],
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
            "encodedValueHex": override["encodedValueHex"],
            "encodedValueSha256": override["encodedValueSha256"],
            "propertyType": override["propertyType"],
            "tagOffset": override["tagOffset"],
            "valueOffset": override["valueOffset"],
            "recordEndOffset": override["recordEndOffset"],
            "exportEvidenceId": source_export["evidenceId"],
        }
    return {
        "fields": fields,
        "partialCullExact": fields["twosided"]["fidelity"]
        == "SOURCE_EXACT_PARTIAL_CULL",
        "fullCullModeExact": False,
        "fullRenderStateExact": False,
    }


def validate_dds_receipt(
    dds_receipt: dict[str, Any],
    texture_exports: dict[str, dict[str, Any]] | None = None,
) -> list[dict[str, Any]]:
    require(
        dds_receipt.get("schema")
        == "lostark.artist-effect-exact-dds-recovery-receipt"
        and type(dds_receipt.get("formatVersion")) is int
        and dds_receipt.get("formatVersion") == 1
        and dds_receipt.get("characterClass") == "ARTIST"
        and type(dds_receipt.get("skillId")) is int
        and dds_receipt.get("skillId") == 31470
        and dds_receipt.get("inputSlot") == "F",
        "unsupported exact DDS receipt",
    )
    manifest = dds_receipt.get("sourceEvidence", {}).get(
        "sourcePackManifest"
    )
    require(
        isinstance(manifest, dict)
        and type(manifest.get("byteCount")) is int
        and manifest.get("byteCount")
        == EXPECTED_SOURCE_PACK_MANIFEST_BYTE_COUNT
        and manifest.get("sha256")
        == EXPECTED_SOURCE_PACK_MANIFEST_SHA256,
        "exact DDS receipt source-pack manifest is not authenticated",
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
        asset_copy = copy.deepcopy(asset)
        if texture_exports is not None:
            require(key in texture_exports, f"raw Texture2D evidence missing: {key}")
            raw_texture = texture_exports[key]
            raw_export = raw_texture["export"]
            identity_fields = (
                "physicalPackage",
                "physicalPackageByteCount",
                "physicalPackageSha256",
                "className",
                "exportIndex",
                "packageReference",
                "serialOffset",
                "serialSize",
                "serialSha256",
                "propertyStreamEnd",
            )
            require(
                all(
                    folded(raw_export.get(name)) == folded(texture.get(name))
                    if name in {"physicalPackage", "className"}
                    else raw_export.get(name) == texture.get(name)
                    for name in identity_fields
                )
                and raw_texture.get("sampling") == sampling
                and raw_texture.get("dds", {}).get("relativePath")
                == asset.get("sourceExtractedDdsRelativePath")
                and raw_texture.get("dds", {}).get("byteCount") == dds.get("byteCount")
                and raw_texture.get("dds", {}).get("sha256") == dds.get("sha256"),
                f"DDS receipt disagrees with raw Texture2D export: {key}",
            )
            asset_copy["rawTextureEvidence"] = copy.deepcopy(raw_texture)
        by_path[key] = asset_copy
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
            require(
                bool(field.get("provenance", {}).get("graphExpressionEvidenceId"))
                and bool(
                    field.get("provenance", {}).get(
                        "valuePropertyRecordSha256"
                    )
                )
                and isinstance(field.get("texturePackageIndex"), int)
                and int(field["texturePackageIndex"]) != 0,
                f"parent DDS binding lacks raw expression texture reference: {logical_path}",
            )
            direct_names = {
                folded(item.get("parameterName"))
                for item in recipe["inputs"]["textureOverrides"]
            }
            require(
                expected["parameterName"].casefold() not in direct_names,
                f"parent DDS default was shadowed by an override: {logical_path}",
            )
        raw_texture = asset.get("rawTextureEvidence")
        require(isinstance(raw_texture, dict), f"raw Texture2D join missing: {logical_path}")
        raw_export = raw_texture["export"]
        sampling = raw_texture["sampling"]
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
                "rawTextureExportEvidenceId": raw_export["evidenceId"],
                "rawTextureClassName": raw_export["className"],
                "rawTextureExportIndex": raw_export["exportIndex"],
                "rawTexturePackageReference": raw_export["packageReference"],
                "rawTextureSerialOffset": raw_export["serialOffset"],
                "rawTextureSerialSize": raw_export["serialSize"],
                "rawSamplerFields": copy.deepcopy(raw_export["fields"]),
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


def occurrence_identity_payload(
    *,
    active_element_id: Any,
    cue_id: Any,
    renderer_type: Any,
    source_system_id: Any,
    source_emitter: Any,
    source_material_path: Any,
    recipe: dict[str, Any],
) -> dict[str, Any]:
    recipe_identity = recipe.get("identity")
    family_evidence = recipe.get("arithmeticFamilyEvidence")
    require(
        isinstance(recipe_identity, dict) and isinstance(family_evidence, dict),
        "recipe identity evidence is missing",
    )
    selected_graph = recipe_identity.get("selectedGraphIdentity")
    require(isinstance(selected_graph, dict), "selected graph identity is missing")
    return {
        "activeElementId": str(active_element_id or ""),
        "cueId": str(cue_id or ""),
        "rendererType": str(renderer_type or ""),
        "sourceSystemId": str(source_system_id or ""),
        "sourceEmitter": str(source_emitter or ""),
        "sourceMaterialPath": str(source_material_path or ""),
        "materialRecipeId": str(recipe.get("recipeId") or ""),
        "recipeCompositionSha256": str(
            recipe.get("compositionSha256") or ""
        ),
        "rawMaterialEvidence": {
            "rawExportEvidenceId": recipe_identity.get("rawExportEvidenceId"),
            "physicalPackageSha256": recipe_identity.get(
                "physicalPackageSha256"
            ),
            "exportIndex": recipe_identity.get("materialExportIndex"),
            "objectPath": recipe_identity.get("materialObjectPath"),
            "selectedGraphRawExportEvidenceId": selected_graph.get(
                "rawExportEvidenceId"
            ),
            "arithmeticFamilyId": family_evidence.get("familyId"),
            "familyIdentitySha256": family_evidence.get(
                "familyIdentitySha256"
            ),
        },
    }


def recipe_composition_payload(recipe: dict[str, Any]) -> dict[str, Any]:
    return {
        "recipeId": recipe.get("recipeId"),
        "sourceMaterialPath": recipe.get("sourceMaterialPath"),
        "rendererShapes": copy.deepcopy(recipe.get("rendererShapes")),
        "identity": copy.deepcopy(recipe.get("identity")),
        "inputs": copy.deepcopy(recipe.get("inputs")),
        "staticPermutation": copy.deepcopy(recipe.get("staticPermutation")),
        "renderState": copy.deepcopy(recipe.get("renderState")),
        "arithmeticFamilyId": recipe.get("arithmeticFamilyId"),
        "arithmeticFamilyEvidence": copy.deepcopy(
            recipe.get("arithmeticFamilyEvidence")
        ),
        "blockers": copy.deepcopy(recipe.get("blockers")),
        "blockerCount": recipe.get("blockerCount"),
        "admission": copy.deepcopy(recipe.get("admission")),
    }


def contract_exact_input_lineage_fixture_payload(
    recipes: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for recipe in sorted(recipes, key=lambda row: row["recipeId"]):
        for collection_name, fields in (
            ("scalarOverrides", recipe["inputs"]["scalarOverrides"]),
            ("vectorOverrides", recipe["inputs"]["vectorOverrides"]),
            ("textureOverrides", recipe["inputs"]["textureOverrides"]),
            ("parentDefaults", recipe["inputs"]["parentDefaults"]),
            (
                "staticParentDefaults",
                recipe["staticPermutation"]["parentDefaults"],
            ),
        ):
            for field in fields:
                result.append(
                    {
                        "recipeId": recipe["recipeId"],
                        "collection": collection_name,
                        "field": copy.deepcopy(field),
                    }
                )
    return result


def build_occurrence_rows(
    active_inventory: dict[str, Any], recipe_by_path: dict[str, dict[str, Any]]
) -> tuple[list[dict[str, Any]], str, str, set[str]]:
    require(
        active_inventory.get("schema")
        == "lostark.source-active-effect-inventory-receipt"
        and type(active_inventory.get("formatVersion")) is int
        and active_inventory.get("formatVersion") == 1
        and active_inventory.get("characterClass") == "ARTIST"
        and type(active_inventory.get("skillId")) is int
        and active_inventory.get("skillId") == 31470
        and active_inventory.get("inputSlot") == "F",
        "unsupported active inventory",
    )
    elements = require_list(active_inventory.get("activeElements"), "active elements")
    require(len(elements) == 35, f"active element denominator changed: {len(elements)}")
    seen: set[str] = set()
    occurrences: list[dict[str, Any]] = []
    join_rows: list[dict[str, str]] = []
    used_recipe_ids: set[str] = set()
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
        material_parameter_evidence = require_list(
            element.get("materialParameterEvidence"),
            f"{active_id}.materialParameterEvidence",
        )
        require(
            len(material_parameter_evidence) == 1
            and isinstance(material_parameter_evidence[0], dict)
            and folded(material_parameter_evidence[0].get("sourceMaterialPath"))
            == folded(material_path),
            f"occurrence material evidence join changed: {active_id}",
        )
        join_rows.append(
            {"activeElementId": active_id, "sourceMaterialPath": material_path}
        )
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
        used_recipe_ids.add(str(recipe["recipeId"]))
        identity = occurrence_identity_payload(
            active_element_id=active_id,
            cue_id=element.get("cueId"),
            renderer_type=element.get("rendererType"),
            source_system_id=element.get("sourceSystemId"),
            source_emitter=element.get("sourceEmitter"),
            source_material_path=material_path,
            recipe=recipe,
        )
        occurrences.append(
            {
                "occurrenceId": active_id,
                "cueId": element.get("cueId"),
                "rendererType": element.get("rendererType"),
                "sourceSystemId": element.get("sourceSystemId"),
                "sourceEmitter": element.get("sourceEmitter"),
                "sourceMaterialPath": material_path,
                "materialRecipeId": recipe["recipeId"],
                "identity": identity,
                "identitySha256": canonical_sha256(identity),
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
    join_sha256 = canonical_sha256(
        sorted(join_rows, key=lambda row: row["activeElementId"])
    )
    require(
        join_sha256 == EXPECTED_OCCURRENCE_MATERIAL_JOIN_SHA256,
        "active occurrence/material stable join changed",
    )
    require(
        used_recipe_ids == {str(recipe["recipeId"]) for recipe in recipe_by_path.values()},
        "not every Material recipe is used by an active occurrence",
    )
    sorted_occurrences = sorted(occurrences, key=lambda row: row["occurrenceId"])
    identity_sha256 = canonical_sha256(
        [row["identity"] for row in sorted_occurrences]
    )
    return sorted_occurrences, join_sha256, identity_sha256, used_recipe_ids


def recipe_identity_fixture_payload(
    recipes: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    return [
        {
            "recipeId": recipe["recipeId"],
            "sourceMaterialPath": recipe["sourceMaterialPath"],
            "identity": recipe["identity"],
        }
        for recipe in sorted(recipes, key=lambda row: row["recipeId"])
    ]


def recipe_family_fixture_payload(
    recipes: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    return [
        {
            "recipeId": recipe["recipeId"],
            "sourceMaterialPath": recipe["sourceMaterialPath"],
            "arithmeticFamilyId": recipe["arithmeticFamilyId"],
            "arithmeticFamilyEvidence": recipe["arithmeticFamilyEvidence"],
        }
        for recipe in sorted(recipes, key=lambda row: row["recipeId"])
    ]


def contract_render_field_fixture_payload(
    recipes: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for recipe in sorted(recipes, key=lambda row: row["recipeId"]):
        fields = recipe.get("renderState", {}).get("fields", {})
        require(isinstance(fields, dict), "recipe render fields are missing")
        for field_name in sorted(fields):
            field = fields[field_name]
            if field.get("status") != "SERIALIZED_EXPLICIT":
                continue
            result.append(
                {
                    "recipeId": recipe["recipeId"],
                    "fieldName": field_name,
                    "field": field,
                }
            )
    return result


def validate_emitted_input_field(
    recipe: dict[str, Any], collection_name: str, field: dict[str, Any]
) -> None:
    recipe_id = str(recipe.get("recipeId") or "")
    material_path = str(recipe.get("sourceMaterialPath") or "")
    identity = recipe.get("identity")
    require(isinstance(identity, dict), f"recipe identity missing: {recipe_id}")
    provenance = field.get("provenance")
    lineage = provenance.get("lineage") if isinstance(provenance, dict) else None
    require(
        isinstance(lineage, dict)
        and provenance.get("lineageSha256") == canonical_sha256(lineage),
        f"input lineage digest changed: {recipe_id}.{collection_name}",
    )
    owner = lineage.get("owner")
    require(
        isinstance(owner, dict)
        and owner.get("recipeId") == recipe_id
        and owner.get("canonicalSourceMaterialPath") == material_path
        and owner.get("rawMaterialExport") == identity.get("rawMaterialExport")
        and lineage.get("fieldKind") == field.get("fieldKind")
        and lineage.get("bindingOrigin") == field.get("bindingOrigin")
        and lineage.get("parameterName") in {None, field.get("parameterName")}
        and lineage.get("normalizedParameterName")
        in {None, field.get("normalizedParameterName")},
        f"input lineage owner changed: {recipe_id}.{collection_name}",
    )
    decoded = lineage.get("decodedCanonicalValue")
    require(
        isinstance(decoded, dict)
        and decoded.get("parameterName") == field.get("parameterName")
        and decoded.get("normalizedParameterName")
        == field.get("normalizedParameterName")
        and decoded.get("value") == field.get("value"),
        f"input decoded lineage changed: {recipe_id}.{collection_name}",
    )
    binding_origin = str(field.get("bindingOrigin") or "")
    lineage_sha256 = str(provenance["lineageSha256"])
    if binding_origin == "INSTANCE_OVERRIDE":
        index = field.get("serializedArrayIndex")
        require(
            type(index) is int
            and lineage.get("serializedArrayIndex") == index
            and collection_name
            in {"scalarOverrides", "vectorOverrides", "textureOverrides"},
            f"instance input order changed: {recipe_id}.{collection_name}",
        )
        property_lineage = lineage.get("propertyLineage")
        validate_property_lineage(
            property_lineage,
            f"{recipe_id}.{collection_name}[{index}]",
        )
        require(
            property_lineage.get("export") == identity.get("rawMaterialExport"),
            f"instance input raw owner changed: {recipe_id}.{collection_name}",
        )
        expected_id = stable_id(
            "material-input",
            material_path,
            field.get("fieldKind"),
            index,
            field.get("parameterName"),
            binding_origin,
            lineage_sha256,
        )
        if field.get("fieldKind") == "texture":
            require(
                type(field.get("packageIndex")) is int
                and decoded.get("packageIndex") == field.get("packageIndex"),
                f"texture input package reference changed: {recipe_id}.{collection_name}",
            )
    else:
        require(
            binding_origin in {"PARENT_DEFAULT", "SELF_DEFAULT"}
            and collection_name in {"parentDefaults", "staticParentDefaults"},
            f"default input origin changed: {recipe_id}.{collection_name}",
        )
        source_order = field.get("expressionSourceOrder")
        require(
            type(source_order) is int
            and lineage.get("expressionSourceOrder") == source_order
            and lineage.get("expressionExportIndex")
            == field.get("expressionExportIndex")
            and lineage.get("parameterName") == field.get("parameterName")
            and lineage.get("normalizedParameterName")
            == field.get("normalizedParameterName"),
            f"default input expression identity changed: {recipe_id}.{collection_name}",
        )
        raw_base = lineage.get("rawBaseMaterialExport")
        raw_expression = lineage.get("rawExpressionExport")
        selected_graph = identity.get("selectedGraphIdentity")
        require(
            isinstance(raw_base, dict)
            and isinstance(raw_expression, dict)
            and isinstance(selected_graph, dict)
            and raw_base.get("evidenceId")
            == selected_graph.get("rawExportEvidenceId")
            and raw_expression.get("evidenceId")
            == field.get("provenance", {}).get(
                "graphExpressionEvidenceId"
            ),
            f"default input raw expression owner changed: {recipe_id}.{collection_name}",
        )
        property_lineage = lineage.get("propertyLineage")
        if field.get("fidelity") == "SOURCE_EXACT_INPUT":
            validate_property_lineage(
                property_lineage,
                f"{recipe_id}.{collection_name}[{source_order}]",
            )
            require(
                property_lineage.get("export") == raw_expression,
                f"default input raw property owner changed: {recipe_id}.{collection_name}",
            )
        else:
            require(
                property_lineage is None,
                f"unresolved default gained property lineage: {recipe_id}.{collection_name}",
            )
        if binding_origin == "PARENT_DEFAULT":
            require(
                lineage.get("inheritanceEdgeEvidenceId")
                == identity.get("rawExportEvidenceId"),
                f"parent default inheritance edge changed: {recipe_id}.{collection_name}",
            )
        else:
            require(
                lineage.get("inheritanceEdgeEvidenceId") is None,
                f"self default gained inheritance edge: {recipe_id}.{collection_name}",
            )
        expected_id = stable_id(
            "material-input",
            material_path,
            field.get("fieldKind"),
            source_order,
            field.get("parameterName"),
            binding_origin,
            lineage_sha256,
        )
        if field.get("fieldKind") == "texture" and field.get("fidelity") == "SOURCE_EXACT_INPUT":
            require(
                type(field.get("texturePackageIndex")) is int
                and decoded.get("packageIndex")
                == field.get("texturePackageIndex"),
                f"default texture package reference changed: {recipe_id}.{collection_name}",
            )
    require(
        field.get("fieldId") == expected_id,
        f"input field ID is not lineage-derived: {recipe_id}.{collection_name}",
    )


def build_contract(
    active_inventory: dict[str, Any],
    material_closure: dict[str, Any],
    dds_receipt: dict[str, Any],
    render_receipt: dict[str, Any],
    source_evidence: dict[str, Any] | None = None,
) -> dict[str, Any]:
    rows = closure_rows(material_closure)
    validate_parent_cycles(rows)
    (
        render_bindings,
        render_exports,
        graph_expressions,
        texture_exports,
    ) = validate_render_receipt(
        render_receipt, rows
    )
    graph_families, family_by_recipe = build_graph_families(
        rows, render_bindings, render_exports, graph_expressions
    )
    graph_family_by_id = {
        str(family["familyId"]): family for family in graph_families
    }
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
            source_export,
        )
        vector_inputs = parameter_rows(
            row,
            "vector",
            "vectorParameters",
            array_records.get("vectorparametervalues"),
            source_export,
        )
        texture_inputs = parameter_rows(
            row,
            "texture",
            "textureParameters",
            array_records.get("textureparametervalues"),
            source_export,
        )
        instance_inputs = {
            "scalar": scalar_inputs,
            "vector": vector_inputs,
            "texture": texture_inputs,
        }
        parent_defaults, static_defaults = parent_default_rows(
            row, instance_inputs, binding, render_exports, graph_expressions
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
        if any(
            field.get("fidelity") != "SOURCE_EXACT_INPUT"
            for field in parent_defaults + static_defaults
        ):
            blockers.add("PARENT_DEFAULT_VALUE_EVIDENCE_UNRESOLVED")
        recipe = {
            "recipeId": stable_id("material-recipe", material_path),
            "sourceMaterialPath": material_path,
            "rendererShapes": copy.deepcopy(row.get("rendererShapes", [])),
            "identity": {
                "fidelity": "SOURCE_EXACT_INPUT",
                "canonicalSourceMaterialPath": material_path,
                "logicalPackage": source_export["logicalPackage"],
                "physicalPackage": row["sourcePhysicalPackage"],
                "physicalPackageSha256": require_sha256(
                    row.get("sourcePhysicalPackageSha256"),
                    f"{material_path}.sourcePackage",
                ),
                "materialObjectPath": material["objectPath"],
                "materialClass": material["className"],
                "materialExportIndex": source_export["exportIndex"],
                "materialPackageReference": source_export[
                    "packageReference"
                ],
                "materialSerialOffset": source_export["serialOffset"],
                "materialSerialSize": source_export["serialSize"],
                "materialSerialSha256": source_export["serialSha256"],
                "rawExportEvidenceId": source_export["evidenceId"],
                "rawMaterialExport": immutable_export_identity(
                    source_export
                ),
                "selectedGraphIdentity": copy.deepcopy(
                    binding["selectedGraphIdentity"]
                ),
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
            "arithmeticFamilyEvidence": {
                "familyId": family_by_recipe[key],
                "familyIdentitySha256": graph_family_by_id[
                    family_by_recipe[key]
                ]["identitySha256"],
                "baseMaterialEvidenceId": graph_family_by_id[
                    family_by_recipe[key]
                ]["rawEvidence"]["baseMaterialEvidenceId"],
                "exactIdentity": copy.deepcopy(
                    graph_family_by_id[family_by_recipe[key]]["exactIdentity"]
                ),
            },
            "blockers": sorted(blockers),
        }
        recipe["blockerCount"] = len(recipe["blockers"])
        recipe["admission"] = {
            "executable": recipe["blockerCount"] == 0,
            "product": recipe["blockerCount"] == 0,
        }
        recipes.append(recipe)

    recipes.sort(key=lambda row: row["recipeId"])
    dds_assets = validate_dds_receipt(dds_receipt, texture_exports)
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

    for recipe in recipes:
        recipe["compositionSha256"] = canonical_sha256(
            recipe_composition_payload(recipe)
        )

    recipe_by_path = {folded(row["sourceMaterialPath"]): row for row in recipes}
    (
        occurrences,
        occurrence_join_sha256,
        occurrence_identity_sha256,
        used_recipe_ids,
    ) = build_occurrence_rows(active_inventory, recipe_by_path)
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
    source_exact_input_recipe_count = sum(
        all(
            field.get("fidelity") == "SOURCE_EXACT_INPUT"
            for collection in (
                recipe["inputs"]["scalarOverrides"],
                recipe["inputs"]["vectorOverrides"],
                recipe["inputs"]["textureOverrides"],
                recipe["inputs"]["parentDefaults"],
                recipe["staticPermutation"]["parentDefaults"],
            )
            for field in collection
        )
        for recipe in recipes
    )
    recipe_identity_sha256 = canonical_sha256(
        recipe_identity_fixture_payload(recipes)
    )
    recipe_family_join_sha256 = canonical_sha256(
        recipe_family_fixture_payload(recipes)
    )
    render_field_evidence_sha256 = canonical_sha256(
        contract_render_field_fixture_payload(recipes)
    )
    exact_input_lineage_sha256 = canonical_sha256(
        contract_exact_input_lineage_fixture_payload(recipes)
    )
    recipe_composition_sha256 = canonical_sha256(
        [
            {
                "recipeId": recipe["recipeId"],
                "compositionSha256": recipe["compositionSha256"],
            }
            for recipe in recipes
        ]
    )

    contract: dict[str, Any] = {
        "schema": "lostark.artist-31470-typed-material-evidence-contract",
        "formatVersion": 3,
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
            "usedMaterialRecipeCount": len(used_recipe_ids),
            "unusedMaterialRecipeCount": len(recipes) - len(used_recipe_ids),
            "unexpectedOccurrenceMaterialCount": 0,
            "occurrenceMaterialJoinSha256": occurrence_join_sha256,
            "occurrenceIdentitySha256": occurrence_identity_sha256,
            "recipeIdentitySha256": recipe_identity_sha256,
            "recipeFamilyJoinSha256": recipe_family_join_sha256,
            "renderFieldEvidenceSha256": render_field_evidence_sha256,
            "exactInputLineageSha256": exact_input_lineage_sha256,
            "recipeCompositionSha256": recipe_composition_sha256,
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
            "sourceExactInputRecipeCount": source_exact_input_recipe_count,
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
        and type(contract.get("formatVersion")) is int
        and contract.get("formatVersion") == 3
        and contract.get("characterClass") == "ARTIST"
        and type(contract.get("skillId")) is int
        and contract.get("skillId") == 31470
        and contract.get("inputSlot") == "F",
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
    input_field_owners: dict[str, str] = {}
    exact_sampler_field_ids: set[str] = set()
    family_ids = {str(row.get("familyId") or "") for row in families}
    require(len(family_ids) == len(families) and "" not in family_ids, "duplicate graph family ID")
    require(
        [str(row.get("familyId") or "") for row in families]
        == sorted(family_ids),
        "graph families are not in stable identity order",
    )
    family_by_id = {str(row["familyId"]): row for row in families}
    for family in families:
        exact_identity = family.get("exactIdentity")
        raw_evidence = family.get("rawEvidence")
        require(
            isinstance(exact_identity, dict)
            and isinstance(raw_evidence, dict),
            "graph family exact/raw identity is missing",
        )
        expected_family_id = stable_id(
            "material-family",
            canonical_sha256(exact_identity),
            canonical_sha256(raw_evidence),
        )
        require(
            family.get("familyId") == expected_family_id
            and int(exact_identity.get("materialExportIndex", -1)) >= 0
            and bool(exact_identity.get("physicalPackage"))
            and bool(exact_identity.get("materialObjectPath"))
            and family.get("identitySha256")
            == canonical_sha256(
                {
                    "exactIdentity": exact_identity,
                    "rawEvidence": raw_evidence,
                }
            ),
            f"graph family ID is not exact-identity-derived: {family.get('familyId')}",
        )
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
        cooked_evidence = family.get("cookedEvidence")
        require(
            isinstance(cooked_evidence, dict)
            and isinstance(raw_evidence, dict),
            "graph family raw/cooked evidence is missing",
        )
        for name in (
            "expressionEntryCount",
            "nonNullExpressionCount",
            "nullExpressionCount",
            "unresolvedInputEdgeCount",
        ):
            require(
                int(cooked_evidence.get(name, -1))
                == int(raw_evidence.get(name, -2)),
                f"graph family count was redistributed: {family.get('familyId')}.{name}",
            )
        require(
            int(raw_evidence["expressionEntryCount"])
            == int(raw_evidence["nonNullExpressionCount"])
            + int(raw_evidence["nullExpressionCount"])
            and bool(raw_evidence.get("baseMaterialEvidenceId"))
            and bool(raw_evidence.get("expressionsRecordSha256"))
            and bool(raw_evidence.get("expressionEvidenceSha256")),
            f"graph family raw evidence is invalid: {family.get('familyId')}",
        )
        require_sha256(
            raw_evidence.get("expressionsRecordSha256"),
            f"{family.get('familyId')}.expressionsRecord",
        )
        require_sha256(
            raw_evidence.get("expressionEvidenceSha256"),
            f"{family.get('familyId')}.expressionEvidence",
        )

    require(
        sum(int(row["rawEvidence"]["nullExpressionCount"]) for row in families)
        == EXPECTED_NULL_EXPRESSION_COUNT
        and sum(
            int(row["rawEvidence"]["unresolvedInputEdgeCount"])
            for row in families
        )
        == EXPECTED_UNRESOLVED_EDGE_COUNT,
        "graph family raw aggregate changed",
    )
    require(
        canonical_sha256(
            sorted(
                [
                    {
                        "familyId": row["familyId"],
                        "rawEvidence": row["rawEvidence"],
                    }
                    for row in families
                ],
                key=lambda row: row["familyId"],
            )
        )
        == EXPECTED_GRAPH_FAMILY_RAW_EVIDENCE_SHA256,
        "graph family raw evidence fixture changed",
    )

    for recipe in recipes:
        require(isinstance(recipe, dict), "contract recipe must be an object")
        recipe_id = str(recipe.get("recipeId") or "")
        require(bool(recipe_id), "contract recipe ID is blank")
        require(recipe_id not in recipe_by_id, f"duplicate contract recipe: {recipe_id}")
        recipe_by_id[recipe_id] = recipe
        source_material_path = str(recipe.get("sourceMaterialPath") or "")
        identity = recipe.get("identity")
        require(
            recipe_id == stable_id("material-recipe", source_material_path)
            and isinstance(identity, dict)
            and identity.get("canonicalSourceMaterialPath") == source_material_path
            and canonical_path_targets_object(
                source_material_path,
                identity.get("materialObjectPath"),
                identity.get("logicalPackage"),
            )
            and bool(identity.get("rawExportEvidenceId"))
            and int(identity.get("materialExportIndex", -1)) >= 0,
            f"recipe ID is not raw Material identity-derived: {recipe_id}",
        )
        raw_material_export = identity.get("rawMaterialExport")
        require(
            isinstance(raw_material_export, dict)
            and raw_material_export.get("evidenceId")
            == identity.get("rawExportEvidenceId")
            and raw_material_export.get("logicalPackage")
            == identity.get("logicalPackage")
            and raw_material_export.get("physicalPackage")
            == identity.get("physicalPackage")
            and raw_material_export.get("physicalPackageSha256")
            == identity.get("physicalPackageSha256")
            and raw_material_export.get("exportIndex")
            == identity.get("materialExportIndex")
            and raw_material_export.get("packageReference")
            == identity.get("materialPackageReference")
            and raw_material_export.get("objectPath")
            == identity.get("materialObjectPath")
            and raw_material_export.get("className")
            == identity.get("materialClass")
            and raw_material_export.get("serialOffset")
            == identity.get("materialSerialOffset")
            and raw_material_export.get("serialSize")
            == identity.get("materialSerialSize")
            and raw_material_export.get("serialSha256")
            == identity.get("materialSerialSha256")
            and type(identity.get("materialPackageReference")) is int
            and identity.get("materialPackageReference")
            == identity.get("materialExportIndex") + 1,
            f"recipe raw Material export identity changed: {recipe_id}",
        )
        require_sha256(
            identity.get("physicalPackageSha256"),
            f"{recipe_id}.physicalPackage",
        )
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
        family_id = str(recipe.get("arithmeticFamilyId") or "")
        require(family_id in family_ids, f"recipe family missing: {recipe_id}")
        family = family_by_id[family_id]
        selected_graph = identity.get("selectedGraphIdentity")
        family_evidence = recipe.get("arithmeticFamilyEvidence")
        require(
            isinstance(selected_graph, dict)
            and isinstance(family_evidence, dict)
            and family_evidence
            == {
                "familyId": family_id,
                "familyIdentitySha256": family["identitySha256"],
                "baseMaterialEvidenceId": family["rawEvidence"][
                    "baseMaterialEvidenceId"
                ],
                "exactIdentity": family["exactIdentity"],
            }
            and selected_graph.get("rawExportEvidenceId")
            == family["rawEvidence"]["baseMaterialEvidenceId"]
            and selected_graph.get("physicalPackageSha256")
            == family["exactIdentity"]["physicalPackageSha256"]
            and selected_graph.get("exportIndex")
            == family["exactIdentity"]["materialExportIndex"]
            and folded(selected_graph.get("objectPath"))
            == folded(family["exactIdentity"]["materialObjectPath"]),
            f"recipe-to-family raw evidence join changed: {recipe_id}",
        )
        if folded(identity.get("materialClass")) == "materialinstanceconstant":
            require(
                canonical_path_targets_object(
                    selected_graph.get("rawParentReferencePath"),
                    selected_graph.get("objectPath"),
                    selected_graph.get("logicalPackage"),
                ),
                f"recipe raw MIC Parent join changed: {recipe_id}",
            )
        else:
            require(
                selected_graph.get("rawParentReferencePath") is None
                and selected_graph.get("rawExportEvidenceId")
                == identity.get("rawExportEvidenceId"),
                f"raw Material self-graph join changed: {recipe_id}",
            )
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
        for field in require_list(
            static.get("parentDefaults"), f"{recipe_id}.staticParentDefaults"
        ):
            require(isinstance(field, dict), f"invalid static parent default: {recipe_id}")
            validate_emitted_input_field(
                recipe, "staticParentDefaults", field
            )
            provenance = field.get("provenance")
            require(
                isinstance(provenance, dict)
                and bool(provenance.get("graphExpressionEvidenceId"))
                and bool(provenance.get("graphExpressionSerialSha256")),
                f"static parent default raw-expression join missing: {recipe_id}",
            )
            if field.get("fidelity") == "SOURCE_EXACT_INPUT":
                require(
                    bool(provenance.get("valuePropertyRecordSha256")),
                    f"static parent default exact value proof missing: {recipe_id}",
                )
            else:
                require(
                    field.get("fidelity")
                    == "UNRESOLVED_PARENT_DEFAULT_EVIDENCE"
                    and field.get("blocker")
                    == "PARENT_DEFAULT_VALUE_EVIDENCE_UNRESOLVED",
                    f"static parent default fidelity invalid: {recipe_id}",
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
        render_fields = render.get("fields")
        require(isinstance(render_fields, dict), f"render fields missing: {recipe_id}")
        for field_name, field in render_fields.items():
            require(isinstance(field, dict), f"invalid render field: {recipe_id}.{field_name}")
            status = field.get("status")
            require(
                status in {"SERIALIZED_EXPLICIT", "OMITTED_FROM_EXPORT"},
                f"invalid render field status: {recipe_id}.{field_name}",
            )
            if status != "SERIALIZED_EXPLICIT":
                continue
            encoded_hex = str(field.get("encodedValueHex") or "")
            try:
                encoded_bytes = bytes.fromhex(encoded_hex)
            except ValueError as error:
                raise ValueError(
                    f"invalid contract render bytes: {recipe_id}.{field_name}"
                ) from error
            require(
                bool(encoded_bytes)
                and hashlib.sha256(encoded_bytes).hexdigest()
                == field.get("encodedValueSha256")
                and bool(field.get("recordSha256"))
                and bool(field.get("exportEvidenceId"))
                and 0 <= int(field.get("tagOffset", -1))
                <= int(field.get("valueOffset", -1))
                <= int(field.get("recordEndOffset", -1)),
                f"contract render field raw evidence changed: {recipe_id}.{field_name}",
            )
            if folded(field.get("propertyType")) == "boolproperty":
                require(
                    len(encoded_bytes) == 1
                    and encoded_bytes[0] in {0, 1}
                    and field.get("value") is bool(encoded_bytes[0]),
                    f"contract render bool disagrees with raw bytes: {recipe_id}.{field_name}",
                )
            if field_name == "blendmode":
                require(
                    folded(field.get("value")) in BLEND_MODE_DOMAIN,
                    f"contract blend enum is invalid: {recipe_id}",
                )
            elif field_name == "lightingmodel":
                require(
                    folded(field.get("value")) in LIGHTING_MODEL_DOMAIN,
                    f"contract lighting enum is invalid: {recipe_id}",
                )
        unresolved_render_default = any(
            render_fields.get(field_name, {}).get("status") == "OMITTED_FROM_EXPORT"
            for field_name in REQUIRED_RENDER_FIELDS
        )
        require(
            ("RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED" in blockers)
            == unresolved_render_default,
            f"render default blocker is not field-derived: {recipe_id}",
        )
        twosided = render_fields.get("twosided")
        exact_partial_cull = (
            isinstance(twosided, dict)
            and twosided.get("status") == "SERIALIZED_EXPLICIT"
            and twosided.get("fidelity") == "SOURCE_EXACT_PARTIAL_CULL"
            and isinstance(twosided.get("value"), bool)
            and bool(twosided.get("recordSha256"))
            and bool(twosided.get("exportEvidenceId"))
        )
        require(
            bool(render.get("partialCullExact")) == exact_partial_cull,
            f"partial cull exactness is not raw-field-derived: {recipe_id}",
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
                validate_emitted_input_field(recipe, collection_name, field)
                field_id = str(field.get("fieldId") or "")
                require(bool(field_id) and field_id not in field_ids, f"duplicate input field ID: {recipe_id}")
                field_ids.add(field_id)
                require(
                    field_id not in all_input_fields,
                    f"duplicate global input field ID: {field_id}",
                )
                all_input_fields[field_id] = field
                input_field_owners[field_id] = recipe_id
                if collection_name == "parentDefaults":
                    provenance = field.get("provenance")
                    require(
                        isinstance(provenance, dict)
                        and bool(provenance.get("graphExpressionEvidenceId"))
                        and bool(provenance.get("graphExpressionSerialSha256")),
                        f"parent default raw-expression join is incomplete: {field_id}",
                    )
                    if field.get("fidelity") == "SOURCE_EXACT_INPUT":
                        require(
                            bool(provenance.get("valuePropertyRecordSha256"))
                            and field.get("blocker") is None,
                            f"parent default exact value proof is incomplete: {field_id}",
                        )
                    else:
                        require(
                            field.get("fidelity")
                            == "UNRESOLVED_PARENT_DEFAULT_EVIDENCE"
                            and field.get("blocker")
                            == "PARENT_DEFAULT_VALUE_EVIDENCE_UNRESOLVED"
                            and "PARENT_DEFAULT_VALUE_EVIDENCE_UNRESOLVED"
                            in blockers,
                            f"unproven parent default was not blocked: {field_id}",
                        )
                else:
                    require(
                        field.get("fidelity") == "SOURCE_EXACT_INPUT",
                        f"instance input lost exact-input fidelity: {field_id}",
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
        unresolved_parent_default = any(
            field.get("fidelity") != "SOURCE_EXACT_INPUT"
            for field in inputs["parentDefaults"]
        ) or any(
            field.get("fidelity") != "SOURCE_EXACT_INPUT"
            for field in require_list(
                static.get("parentDefaults"), f"{recipe_id}.staticParentDefaults"
            )
        )
        require(
            ("PARENT_DEFAULT_VALUE_EVIDENCE_UNRESOLVED" in blockers)
            == unresolved_parent_default,
            f"parent-default blocker is not evidence-derived: {recipe_id}",
        )
        texture_fields = list(inputs["textureOverrides"]) + [
            field for field in inputs["parentDefaults"]
            if field.get("fieldKind") == "texture"
        ]
        for field in texture_fields:
            sampler = field.get("sampler")
            require(isinstance(sampler, dict), f"texture sampler state missing: {field['fieldId']}")
            if sampler.get("fidelity") == "SOURCE_EXACT_SAMPLER":
                provenance = sampler.get("provenance")
                require(
                    bool(sampler.get("bindingId"))
                    and isinstance(provenance, dict)
                    and bool(provenance.get("rawTextureExportEvidenceId"))
                    and folded(provenance.get("rawTextureClassName")) == "texture2d"
                    and isinstance(provenance.get("rawTextureExportIndex"), int)
                    and int(provenance.get("rawTexturePackageReference", -1))
                    == int(provenance.get("rawTextureExportIndex", -2)) + 1
                    and isinstance(provenance.get("rawTextureSerialOffset"), int)
                    and int(provenance.get("rawTextureSerialOffset", -1)) >= 0
                    and isinstance(provenance.get("rawTextureSerialSize"), int)
                    and int(provenance.get("rawTextureSerialSize", 0)) > 0
                    and bool(provenance.get("rawSamplerFields")),
                    f"exact sampler provenance is incomplete: {field['fieldId']}",
                )
                exact_sampler_field_ids.add(field["fieldId"])
            else:
                require(
                    set(sampler) == {"fidelity", "blocker"}
                    and sampler.get("blocker") == "SAMPLER_EVIDENCE_MISSING",
                    f"unproven sampler gained values: {field['fieldId']}",
                )
        unresolved_sampler = any(
            field.get("sampler", {}).get("fidelity") != "SOURCE_EXACT_SAMPLER"
            for field in texture_fields
        )
        require(
            ("SAMPLER_BINDINGS_INCOMPLETE" in blockers) == unresolved_sampler,
            f"sampler blocker is not field-derived: {recipe_id}",
        )
        require(
            recipe.get("compositionSha256")
            == canonical_sha256(recipe_composition_payload(recipe)),
            f"recipe composition digest changed: {recipe_id}",
        )

    sampler_field_ids: set[str] = set()
    for sampler in samplers:
        field_id = str(sampler.get("inputFieldId") or "")
        require(
            field_id in all_input_fields
            and field_id not in sampler_field_ids
            and sampler.get("materialRecipeId")
            == input_field_owners[field_id]
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

    occurrence_ids: set[str] = set()
    used_recipe_ids: set[str] = set()
    require(
        [row.get("occurrenceId") for row in occurrences]
        == sorted(str(row.get("occurrenceId") or "") for row in occurrences),
        "contract occurrences are not in stable identity order",
    )
    for occurrence in occurrences:
        require(isinstance(occurrence, dict), "contract occurrence must be an object")
        occurrence_id = str(occurrence.get("occurrenceId") or "")
        require(
            bool(occurrence_id) and occurrence_id not in occurrence_ids,
            f"duplicate contract occurrence: {occurrence_id}",
        )
        occurrence_ids.add(occurrence_id)
        recipe_id = str(occurrence.get("materialRecipeId") or "")
        require(recipe_id in recipe_by_id, f"occurrence recipe is missing: {recipe_id}")
        recipe = recipe_by_id[recipe_id]
        used_recipe_ids.add(recipe_id)
        expected_identity = occurrence_identity_payload(
            active_element_id=occurrence_id,
            cue_id=occurrence.get("cueId"),
            renderer_type=occurrence.get("rendererType"),
            source_system_id=occurrence.get("sourceSystemId"),
            source_emitter=occurrence.get("sourceEmitter"),
            source_material_path=occurrence.get("sourceMaterialPath"),
            recipe=recipe,
        )
        require(
            folded(occurrence.get("sourceMaterialPath"))
            == folded(recipe.get("sourceMaterialPath"))
            and occurrence.get("identity") == expected_identity
            and occurrence.get("identitySha256")
            == canonical_sha256(expected_identity)
            and occurrence.get("blockers") == recipe["blockers"]
            and int(occurrence.get("blockerCount", -1)) == recipe["blockerCount"]
            and occurrence.get("admission") == recipe["admission"],
            f"occurrence blocker set diverged: {occurrence.get('occurrenceId')}",
        )
    require(
        used_recipe_ids == set(recipe_by_id),
        "contract occurrence set does not use every Material recipe",
    )
    occurrence_identity_sha256 = canonical_sha256(
        [row["identity"] for row in occurrences]
    )

    summary = contract.get("summary")
    require(isinstance(summary, dict), "contract summary missing")
    recipe_identity_sha256 = canonical_sha256(
        recipe_identity_fixture_payload(recipes)
    )
    recipe_family_join_sha256 = canonical_sha256(
        recipe_family_fixture_payload(recipes)
    )
    render_field_evidence_sha256 = canonical_sha256(
        contract_render_field_fixture_payload(recipes)
    )
    exact_input_lineage_sha256 = canonical_sha256(
        contract_exact_input_lineage_fixture_payload(recipes)
    )
    recipe_composition_sha256 = canonical_sha256(
        [
            {
                "recipeId": recipe["recipeId"],
                "compositionSha256": recipe["compositionSha256"],
            }
            for recipe in recipes
        ]
    )
    require(
        summary.get("occurrenceIdentitySha256") == occurrence_identity_sha256
        == EXPECTED_OCCURRENCE_IDENTITY_SHA256
        and summary.get("recipeIdentitySha256") == recipe_identity_sha256
        == EXPECTED_RECIPE_IDENTITY_SHA256
        and summary.get("recipeFamilyJoinSha256") == recipe_family_join_sha256
        == EXPECTED_RECIPE_FAMILY_JOIN_SHA256
        and summary.get("renderFieldEvidenceSha256")
        == render_field_evidence_sha256
        == EXPECTED_CONTRACT_RENDER_FIELD_EVIDENCE_SHA256
        and summary.get("exactInputLineageSha256")
        == exact_input_lineage_sha256
        == EXPECTED_CONTRACT_EXACT_INPUT_LINEAGE_SHA256
        and summary.get("recipeCompositionSha256")
        == recipe_composition_sha256
        == EXPECTED_RECIPE_COMPOSITION_SHA256,
        "contract identity summary is not emitted-field-derived",
    )
    expected_summary = {
        "materialRecipeCount": EXPECTED_RECIPE_COUNT,
        "materialOccurrenceCount": EXPECTED_OCCURRENCE_COUNT,
        "usedMaterialRecipeCount": EXPECTED_RECIPE_COUNT,
        "unusedMaterialRecipeCount": 0,
        "unexpectedOccurrenceMaterialCount": 0,
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
    require(
        summary.get("occurrenceMaterialJoinSha256")
        == EXPECTED_OCCURRENCE_MATERIAL_JOIN_SHA256,
        "contract occurrence/material join digest changed",
    )
    partial_cull_count = sum(
        bool(recipe["renderState"]["partialCullExact"])
        for recipe in recipes
    )
    require(
        int(summary.get("sourceExactPartialCullRecipeCount", -1))
        == partial_cull_count,
        "partial-cull summary is not recipe-derived",
    )
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
    dds_source = render_receipt.get("source", {}).get("exactDdsReceipt", {})
    require(
        dds_source.get("hashDomain") == "TRACKED_DERIVED_EOL_CANONICAL_TEXT"
        and dds_source.get("canonicalTextSha256")
        == tracked_json_text_sha256(dds_receipt_path),
        "render-state receipt exact-DDS hash mismatch",
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
        (
            "materialClosureParser",
            REPO_ROOT
            / "Tools/LevelPlacementExtractor/"
            "extract_ue3_effect_material_closure.py",
        ),
        (
            "materialGraphParser",
            REPO_ROOT
            / "Tools/LevelPlacementExtractor/extract_ue3_material_graph.py",
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
    render_receipt: dict[str, Any],
    source_package_root: Path,
    exact_dds_root: Path,
    source_pack_manifest: Path,
) -> None:
    packages = unique_files_by_name(source_package_root, "*.upk")
    require(
        render_receipt.get("schema")
        == "lostark.artist-31470-material-render-state-evidence-receipt"
        and render_receipt.get("formatVersion") == 3,
        "unsupported render-state evidence receipt for deep verification",
    )
    validate_self_digest(render_receipt, "receiptSha256", "render-state receipt")
    texture_exports = {
        folded(row.get("logicalObjectPath")): row
        for row in require_list(
            render_receipt.get("textureSamplerExports"),
            "raw texture sampler exports",
        )
    }
    require(
        len(texture_exports) == 4,
        "raw Texture2D evidence denominator changed",
    )
    assets = validate_dds_receipt(dds_receipt, texture_exports)
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
            load_json(args.render_state_receipt),
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
