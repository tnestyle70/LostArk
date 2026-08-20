#!/usr/bin/env python3
"""Evaluate UE3 cooked material uniform expressions into their native CB0 rows.

This is the class-neutral G03-5 bridge between the G03-3 exact material-map
receipt and a future source-value GPU replay.  It deliberately owns only the
numeric material slice:

* decode scalar/vector overrides from the pinned MIC property stream while
  preserving the raw ``FName`` number;
* merge a MaterialInstanceConstant hierarchy from parent to leaf by
  ``(base text casefold, raw number)``;
* evaluate the exact cooked uniform-expression tree with float32 semantics;
* pack the values through the exact native scalar/vector CB0 wire recovered by
  G03-3.

Textures, samplers, engine-owned CB rows, actual VF/pass selection, runtime
admission, and visual fidelity remain outside this stage.  FoldedMath ordinal
0/2 and Periodic/Sine opcode meaning are checked against a pinned local Epic
UnrealEngine source file before evaluation.  Native scalar group lane order and
padding remain an explicit runtime-admission blocker until their source ABI is
proven independently.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import struct
import sys
from pathlib import Path
from typing import Any, Iterable


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOLS = REPOSITORY_ROOT / "Tools" / "EffectPipeline"
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
for path in (EFFECT_TOOLS, LEVEL_TOOLS):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from extract_ue3_material_shader_maps import (  # noqa: E402
    DEFAULT_SOURCE_ROOT,
    canonical_json_sha256,
    digest_file,
    read_json,
    require,
    sha256_bytes,
    write_json_atomic,
)
from extract_ue3_effect_material_closure import load_package  # noqa: E402
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


MANIFEST_SCHEMA = "lostark.effect-ue3-source-value-uniform-evaluation-targets"
MANIFEST_FORMAT_VERSION = 1
RECEIPT_SCHEMA = "lostark.effect-ue3-source-value-uniform-evaluation-receipt"
RECEIPT_FORMAT_VERSION = 1
INPUT_RECEIPT_SCHEMA = "lostark.effect-ue3-material-shader-map-receipt"
INPUT_RECEIPT_FORMAT_VERSION = 3

DEFAULT_MANIFEST = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.source-value-uniform-evaluation.targets.json"
)
DEFAULT_INPUT_RECEIPT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-material-maps.receipt.json"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.source-value-uniform-evaluation.receipt.json"
)
DEFAULT_OFFICIAL_SOURCE = Path(
    "C:/Users/user/Desktop/UnrealEngine/UnrealEngine/Engine/Source/Runtime/"
    "Engine/Private/Materials/MaterialUniformExpressions.h"
)

STATUS_EXACT = "EXACT_MATERIAL_SHADER_MAP"
STATUS_CLOSED = "EXACT_SOURCE_VALUE_UNIFORM_CB0_CLOSURE"
MIC_CLASSES = {"materialinstanceconstant", "materialinstancetimevarying"}
BASE_MATERIAL_CLASSES = {"material", "material3", "decalmaterial"}


def _float32(value: float) -> float:
    result = struct.unpack("<f", struct.pack("<f", float(value)))[0]
    require(math.isfinite(result), "uniform evaluation produced a non-finite float")
    return result


def _float4(values: Iterable[float]) -> list[float]:
    result = [_float32(value) for value in values]
    require(len(result) == 4, "uniform expression did not produce float4")
    return result


def _splat(value: float) -> list[float]:
    scalar = _float32(value)
    return [scalar, scalar, scalar, scalar]


def _signed_fractional_float32(value: float) -> float:
    """Mirror FMath::Fractional: Value - TruncToFloat(Value)."""

    source = _float32(value)
    integral = math.modf(source)[1]
    return _float32(source - integral)


def _parameter_key(name: str, number: int) -> tuple[str, int]:
    require(isinstance(name, str) and name, "parameter FName text is absent")
    require(isinstance(number, int) and number >= 0, "parameter FName number is invalid")
    return name.casefold(), number


def _public_parameter_key(key: tuple[str, int]) -> dict[str, Any]:
    return {"nameCasefold": key[0], "number": key[1]}


def _seal(document: dict[str, Any]) -> None:
    document.pop("receiptSha256", None)
    document["receiptSha256"] = canonical_json_sha256(document)


def _fname_raw(
    data: bytes, offset: int, names: list[str]
) -> tuple[str, int, int]:
    require(offset + 8 <= len(data), "FName is truncated")
    name_index, number = struct.unpack_from("<ii", data, offset)
    require(0 <= name_index < len(names), "FName index is outside NameTable")
    require(number >= 0, "FName number is negative")
    return names[name_index], number, offset + 8


def _property_tags_at(
    data: bytes, names: list[str], start: int
) -> tuple[list[dict[str, Any]], int]:
    """Read one UE3 tagged-property stream while retaining raw payloads."""

    rows: list[dict[str, Any]] = []
    offset = start
    while offset < len(data):
        name, name_number, offset = _fname_raw(data, offset, names)
        if name.casefold() == "none":
            require(name_number == 0, "numbered None terminator is invalid")
            return rows, offset
        property_type, type_number, offset = _fname_raw(data, offset, names)
        require(type_number == 0, "numbered property type is unsupported")
        require(offset + 8 <= len(data), "property header is truncated")
        data_size, array_index = struct.unpack_from("<ii", data, offset)
        offset += 8
        require(data_size >= 0 and array_index >= 0, "property header is invalid")

        struct_type: str | None = None
        struct_type_number = 0
        bool_value: bool | None = None
        folded_type = property_type.casefold()
        if folded_type == "structproperty":
            struct_type, struct_type_number, offset = _fname_raw(data, offset, names)
        elif folded_type == "boolproperty":
            require(offset < len(data), "BoolProperty value is truncated")
            bool_value = bool(data[offset])
            offset += 1
        elif folded_type == "byteproperty":
            _, _, offset = _fname_raw(data, offset, names)

        serialized_size = data_size + (8 if folded_type == "intproperty" else 0)
        require(offset + serialized_size <= len(data), "property payload is truncated")
        payload = data[offset : offset + serialized_size]
        offset += serialized_size
        rows.append(
            {
                "name": name,
                "nameNumber": name_number,
                "type": property_type,
                "arrayIndex": array_index,
                "structType": struct_type,
                "structTypeNumber": struct_type_number,
                "boolValue": bool_value,
                "payload": payload,
            }
        )
    raise ValueError("tagged property stream has no None terminator")


def _locate_raw_property_stream(
    serial: bytes, names: list[str], package_version: int
) -> tuple[list[dict[str, Any]], int, int]:
    """Locate the same property stream admitted by the shared parser."""

    decoded_properties, expected_end = parse_tagged_properties(
        serial, names, package_version
    )
    expected_names = {str(name).casefold() for name in decoded_properties}
    minimum = 4 if package_version >= 322 else 0
    search_end = min(len(serial) - 20, 256)
    candidates = []
    for start in range(minimum, max(minimum, search_end) + 1):
        try:
            _, _, after_name = _fname_raw(serial, start, names)
            property_type, _, _ = _fname_raw(serial, after_name, names)
            if not property_type.casefold().endswith("property"):
                continue
            rows, end = _property_tags_at(serial, names, start)
            actual_names = {str(row["name"]).casefold() for row in rows}
            if end == expected_end and actual_names == expected_names:
                candidates.append((rows, start, end))
        except (ValueError, struct.error):
            continue
    require(
        len(candidates) == 1,
        f"raw property stream is absent or ambiguous: {len(candidates)}",
    )
    return candidates[0]


def _parameter_array_rows(
    payload: bytes, names: list[str], value_kind: str
) -> list[dict[str, Any]]:
    require(len(payload) >= 4, "parameter array payload is truncated")
    count = struct.unpack_from("<i", payload, 0)[0]
    require(0 <= count <= 4096, "parameter array count is invalid")
    offset = 4
    result = []
    seen: set[tuple[str, int]] = set()
    for _ in range(count):
        properties, offset = _property_tags_at(payload, names, offset)
        by_name = {row["name"].casefold(): row for row in properties}
        require(len(by_name) == len(properties), "parameter item property is duplicated")
        name_row = by_name.get("parametername")
        value_row = by_name.get("parametervalue")
        require(name_row is not None and value_row is not None, "parameter item is incomplete")
        require(
            name_row["type"].casefold() == "nameproperty"
            and len(name_row["payload"]) == 8,
            "parameter identity is not an FName",
        )
        name, number, name_end = _fname_raw(name_row["payload"], 0, names)
        require(name_end == 8, "parameter FName payload has trailing bytes")
        key = _parameter_key(name, number)
        require(key not in seen, "parameter key is duplicated within one MIC")
        seen.add(key)

        if value_kind == "scalar":
            require(
                value_row["type"].casefold() == "floatproperty"
                and len(value_row["payload"]) == 4,
                "scalar parameter value shape changed",
            )
            value: float | list[float] = _float32(
                struct.unpack_from("<f", value_row["payload"], 0)[0]
            )
        else:
            require(
                value_kind == "vector"
                and value_row["type"].casefold() == "structproperty"
                and str(value_row["structType"] or "").casefold() == "linearcolor"
                and value_row["structTypeNumber"] == 0
                and len(value_row["payload"]) == 16,
                "vector parameter value shape changed",
            )
            value = _float4(struct.unpack_from("<4f", value_row["payload"], 0))
        result.append(
            {
                "parameterName": name,
                "parameterNameNumber": number,
                "parameterKey": _public_parameter_key(key),
                "value": value,
            }
        )
    require(offset == len(payload), "parameter array has trailing bytes")
    return result


def decode_mic_numeric_overrides(
    serial: bytes, names: list[str], package_version: int
) -> dict[str, Any]:
    properties, start, end = _locate_raw_property_stream(
        serial, names, package_version
    )
    by_name = {row["name"].casefold(): row for row in properties}
    require(len(by_name) == len(properties), "top-level MIC property is duplicated")

    scalar_row = by_name.get("scalarparametervalues")
    vector_row = by_name.get("vectorparametervalues")
    parent_row = by_name.get("parent")
    require(parent_row is not None, "MIC Parent property is absent")
    require(
        parent_row["type"].casefold() == "objectproperty"
        and len(parent_row["payload"]) == 4,
        "MIC Parent property shape changed",
    )
    parent_reference = struct.unpack_from("<i", parent_row["payload"], 0)[0]
    require(parent_reference != 0, "MIC Parent reference is null")

    def array(row: dict[str, Any] | None, kind: str) -> list[dict[str, Any]]:
        if row is None:
            return []
        require(row["type"].casefold() == "arrayproperty", "MIC parameter array type changed")
        return _parameter_array_rows(row["payload"], names, kind)

    return {
        "propertyStreamStart": start,
        "propertyStreamEnd": end,
        "scalarOverrides": array(scalar_row, "scalar"),
        "vectorOverrides": array(vector_row, "vector"),
        "parentReference": parent_reference,
    }


def merge_effective_overrides(
    layers: list[dict[str, Any]], collection: str
) -> list[dict[str, Any]]:
    """Apply one numeric override collection in root-to-leaf order."""

    require(collection in ("scalarOverrides", "vectorOverrides"), "override collection is invalid")
    effective: dict[tuple[str, int], dict[str, Any]] = {}
    for layer_index, layer in enumerate(layers):
        local: set[tuple[str, int]] = set()
        for row in layer[collection]:
            key = _parameter_key(row["parameterName"], row["parameterNameNumber"])
            require(key not in local, "parameter key is duplicated within one hierarchy layer")
            local.add(key)
            effective[key] = {
                **row,
                "sourceHierarchyIndex": layer_index,
                "sourceObjectPath": layer["objectPath"],
            }
    return [effective[key] for key in sorted(effective)]


def _package_export_class(package: Any, export: Any) -> str:
    return str(package_ref_name(export.class_index, package.imports, package.exports) or "")


def _reference_class(package: Any, reference: int) -> str:
    if reference > 0:
        require(reference <= len(package.exports), "parent export reference is invalid")
        return _package_export_class(package, package.exports[reference - 1])
    require(-reference <= len(package.imports), "parent import reference is invalid")
    return package.imports[-reference - 1].class_name


def _export_index(packages: dict[str, Any]) -> dict[str, list[tuple[Any, Any]]]:
    result: dict[str, list[tuple[Any, Any]]] = {}
    for package in packages.values():
        for export in package.exports:
            path = package_ref_path(export.index + 1, package.imports, package.exports)
            result.setdefault(path.casefold(), []).append((package, export))
    return result


def decode_mic_hierarchy(
    package: Any,
    export: Any,
    global_exports: dict[str, list[tuple[Any, Any]]],
    *,
    visited: set[tuple[str, int]] | None = None,
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    """Return numeric MIC layers root-to-leaf and the terminal base Material."""

    if visited is None:
        visited = set()
    identity = (str(package.path).casefold(), export.index)
    require(identity not in visited, "MIC parent hierarchy contains a cycle")
    visited = set(visited)
    visited.add(identity)

    class_name = _package_export_class(package, export)
    require(class_name.casefold() in MIC_CLASSES, "hierarchy export is not a MIC")
    serial = package.logical[
        export.serial_offset : export.serial_offset + export.serial_size
    ]
    decoded = decode_mic_numeric_overrides(
        serial, package.names, package.summary.version
    )
    parent_reference = decoded["parentReference"]
    parent_path = package_ref_path(
        parent_reference, package.imports, package.exports
    )
    parent_class = _reference_class(package, parent_reference)
    layer = {
        "objectPath": package_ref_path(
            export.index + 1, package.imports, package.exports
        ),
        "className": class_name,
        "sourcePackageFileName": package.path.name,
        "exportIndexZeroBased": export.index,
        "serialSha256": sha256_bytes(serial),
        "propertyStreamStart": decoded["propertyStreamStart"],
        "propertyStreamEnd": decoded["propertyStreamEnd"],
        "scalarOverrides": decoded["scalarOverrides"],
        "vectorOverrides": decoded["vectorOverrides"],
        "parentReference": parent_reference,
        "parentObjectPath": parent_path,
        "parentClassName": parent_class,
    }

    folded_parent_class = parent_class.casefold()
    if folded_parent_class in BASE_MATERIAL_CLASSES:
        return [layer], {
            "objectPath": parent_path,
            "className": parent_class,
            "sourcePackageFileName": (
                package.path.name if parent_reference > 0 else None
            ),
            "referenceKind": "EXPORT" if parent_reference > 0 else "IMPORT",
        }
    require(folded_parent_class in MIC_CLASSES, "MIC parent is not a supported Material class")
    if parent_reference > 0:
        parent_package, parent_export = package, package.exports[parent_reference - 1]
    else:
        matches = global_exports.get(parent_path.casefold(), [])
        require(
            len(matches) == 1,
            "external MIC parent export is absent or ambiguous in pinned source packages",
        )
        parent_package, parent_export = matches[0]
    parent_layers, terminal = decode_mic_hierarchy(
        parent_package,
        parent_export,
        global_exports,
        visited=visited,
    )
    return parent_layers + [layer], terminal


def _effective_map(rows: list[dict[str, Any]]) -> dict[tuple[str, int], Any]:
    return {
        _parameter_key(row["parameterName"], row["parameterNameNumber"]): row["value"]
        for row in rows
    }


def evaluate_expression(
    expression: dict[str, Any],
    scalar_overrides: dict[tuple[str, int], Any],
    vector_overrides: dict[tuple[str, int], Any],
    *,
    game_time_seconds: float,
    real_time_seconds: float,
    stats: dict[str, Any] | None = None,
) -> list[float]:
    """Evaluate the expression shapes admitted by the W exact maps."""

    if stats is None:
        stats = {
            "nodeTypeCounts": {},
            "foldedMathOperationOrdinalCounts": {},
            "scalarOverrideHitCount": 0,
            "scalarDefaultHitCount": 0,
            "vectorOverrideHitCount": 0,
            "vectorDefaultHitCount": 0,
        }
    type_name = str(expression["typeName"]).casefold()
    stats["nodeTypeCounts"][type_name] = stats["nodeTypeCounts"].get(type_name, 0) + 1

    if type_name == "fmaterialuniformexpressionconstant":
        return _float4(expression["value"])
    if type_name == "fmaterialuniformexpressionscalarparameter":
        key = _parameter_key(
            expression["parameterName"], expression["parameterNameNumber"]
        )
        if key in scalar_overrides:
            stats["scalarOverrideHitCount"] += 1
            return _splat(scalar_overrides[key])
        stats["scalarDefaultHitCount"] += 1
        return _splat(expression["defaultValue"])
    if type_name == "fmaterialuniformexpressionvectorparameter":
        key = _parameter_key(
            expression["parameterName"], expression["parameterNameNumber"]
        )
        if key in vector_overrides:
            stats["vectorOverrideHitCount"] += 1
            return _float4(vector_overrides[key])
        stats["vectorDefaultHitCount"] += 1
        return _float4(expression["defaultValue"])
    if type_name == "fmaterialuniformexpressiontime":
        return _splat(game_time_seconds)
    if type_name == "fmaterialuniformexpressionrealtime":
        return _splat(real_time_seconds)

    def child(name: str) -> list[float]:
        return evaluate_expression(
            expression[name],
            scalar_overrides,
            vector_overrides,
            game_time_seconds=game_time_seconds,
            real_time_seconds=real_time_seconds,
            stats=stats,
        )

    if type_name == "fmaterialuniformexpressionfoldedmath":
        a, b = child("a"), child("b")
        ordinal = int(expression["operationOrdinal"])
        counts = stats["foldedMathOperationOrdinalCounts"]
        counts[str(ordinal)] = counts.get(str(ordinal), 0) + 1
        require(ordinal in (0, 2), f"FoldedMath ordinal is not source-proven for G03-5: {ordinal}")
        if ordinal == 0:
            return _float4(left + right for left, right in zip(a, b))
        return _float4(left * right for left, right in zip(a, b))
    if type_name == "fmaterialuniformexpressionappendvector":
        a, b = child("a"), child("b")
        count = int(expression["componentsFromA"])
        require(1 <= count <= 3, "AppendVector component count is invalid")
        return _float4(a[:count] + b[: 4 - count])
    if type_name == "fmaterialuniformexpressionperiodic":
        return _float4(_signed_fractional_float32(value) for value in child("input"))
    if type_name == "fmaterialuniformexpressionsine":
        operation = math.cos if bool(expression["isCosine"]) else math.sin
        return _float4(operation(value) for value in child("input"))

    raise ValueError(f"uniform expression type is not source-proven for G03-5: {type_name}")


def evaluate_uniform_set_into_cb0(
    uniform_set: dict[str, Any],
    native_binding: dict[str, Any],
    effective_scalars: list[dict[str, Any]],
    effective_vectors: list[dict[str, Any]],
    context: dict[str, Any],
) -> dict[str, Any]:
    scalar_map = _effective_map(effective_scalars)
    vector_map = _effective_map(effective_vectors)
    stats: dict[str, Any] = {
        "nodeTypeCounts": {},
        "foldedMathOperationOrdinalCounts": {},
        "scalarOverrideHitCount": 0,
        "scalarDefaultHitCount": 0,
        "vectorOverrideHitCount": 0,
        "vectorDefaultHitCount": 0,
    }

    def evaluate_rows(name: str) -> list[list[float]]:
        return [
            evaluate_expression(
                expression,
                scalar_map,
                vector_map,
                game_time_seconds=float(context["gameTimeSeconds"]),
                real_time_seconds=float(context["realTimeSeconds"]),
                stats=stats,
            )
            for expression in uniform_set[name]
        ]

    vector_values = evaluate_rows("pixelVectorExpressions")
    scalar_vectors = evaluate_rows("pixelScalarExpressions")
    scalar_values = [row[0] for row in scalar_vectors]
    declared_count = int(
        native_binding["constantBufferClosure"][
            "declaredConstantBuffer0Float4Count"
        ]
    )
    cb0: list[list[float] | None] = [None] * declared_count
    material_rows = []

    for wire in native_binding["scalarGroups"]:
        group = int(wire["expressionIndexOrGroup"])
        slot = int(wire["baseIndex"]) // 16
        require(int(wire["baseIndex"]) % 16 == 0, "scalar CB0 wire is unaligned")
        require(cb0[slot] is None, "native CB0 slot is written more than once")
        value = [0.0, 0.0, 0.0, 0.0]
        indices = []
        for lane in range(4):
            expression_index = group * 4 + lane
            if expression_index < len(scalar_values):
                value[lane] = scalar_values[expression_index]
                indices.append(expression_index)
        cb0[slot] = _float4(value)
        material_rows.append(
            {
                "slot": slot,
                "source": "PIXEL_SCALAR_EXPRESSION_GROUP",
                "expressionIndices": indices,
                "value": cb0[slot],
            }
        )

    for wire in native_binding["vectors"]:
        expression_index = int(wire["expressionIndexOrGroup"])
        slot = int(wire["baseIndex"]) // 16
        require(int(wire["baseIndex"]) % 16 == 0, "vector CB0 wire is unaligned")
        require(cb0[slot] is None, "native CB0 slot is written more than once")
        cb0[slot] = vector_values[expression_index]
        material_rows.append(
            {
                "slot": slot,
                "source": "PIXEL_VECTOR_EXPRESSION",
                "expressionIndex": expression_index,
                "value": cb0[slot],
            }
        )

    material_rows.sort(key=lambda row: row["slot"])
    expected_slots = native_binding["constantBufferClosure"]["boundConstantBuffer0Slots"]
    require(
        [row["slot"] for row in material_rows] == expected_slots,
        "source-value CB0 rows do not close over native material slots",
    )
    all_rows = [
        {
            "slot": slot,
            "ownership": (
                "MATERIAL_UNIFORM_EXPRESSION"
                if value is not None
                else "ENGINE_OR_RENDERER_INPUT_UNBOUND_AT_G03_5"
            ),
            "value": value,
        }
        for slot, value in enumerate(cb0)
    ]
    require(
        all(
            value is None or all(math.isfinite(component) for component in value)
            for value in cb0
        ),
        "source-value CB0 contains non-finite data",
    )
    return {
        "evaluationContext": context,
        "pixelVectorExpressionValues": vector_values,
        "pixelScalarExpressionValues": scalar_values,
        "pixelVectorValuesSemanticSha256": canonical_json_sha256(vector_values),
        "pixelScalarValuesSemanticSha256": canonical_json_sha256(scalar_values),
        "evaluationStats": stats,
        "nativeCb0": {
            "declaredFloat4Count": declared_count,
            "materialBoundFloat4Count": len(material_rows),
            "engineOrRendererUnboundFloat4Count": sum(value is None for value in cb0),
            "materialRows": material_rows,
            "allRows": all_rows,
            "materialRowsSemanticSha256": canonical_json_sha256(material_rows),
        },
    }


def validate_official_source(path: Path, expected: dict[str, Any]) -> dict[str, Any]:
    require(path.is_file(), f"official UnrealEngine source is missing: {path}")
    raw_sha = digest_file(path)
    require(raw_sha == expected["rawSha256"], "official engine source SHA changed")
    text = path.read_text(encoding="utf-8-sig")
    enum_match = re.search(
        r"enum\s+EFoldedMathOperation\s*\{(?P<body>.*?)\};",
        text,
        re.DOTALL,
    )
    require(enum_match is not None, "EFoldedMathOperation enum is absent")
    enum_names = re.findall(r"\bFMO_[A-Za-z]+\b", enum_match.group("body"))
    require(
        enum_names == expected["evidence"]["foldedMathOperationOrder"],
        "FoldedMath ordinal order changed",
    )
    require(
        re.search(r"case\s+FMO_Add\s*:\s*OutData\.WriteOpcode\([^;]*::Add\)", text)
        and re.search(r"case\s+FMO_Mul\s*:\s*OutData\.WriteOpcode\([^;]*::Mul\)", text),
        "FoldedMath Add/Mul opcode mapping changed",
    )
    require(
        "FMaterialUniformExpressionPeriodic" in text
        and "EPreshaderOpcode::Fractional" in text,
        "Periodic Fractional opcode evidence is absent",
    )
    require(
        re.search(
            r"bIsCosine\s*\?\s*UE::Shader::EPreshaderOpcode::Cos\s*:\s*"
            r"UE::Shader::EPreshaderOpcode::Sin",
            text,
        ),
        "Sine Sin/Cos opcode evidence is absent",
    )
    lines = text.splitlines()

    def line_of(fragment: str) -> int:
        return next(index + 1 for index, line in enumerate(lines) if fragment in line)

    return {
        "path": path.as_posix(),
        "repository": expected["repository"],
        "tag": expected["tag"],
        "commit": expected["commit"],
        "physicalByteSize": path.stat().st_size,
        "rawSha256": raw_sha,
        "verifiedSemantics": {
            "foldedMathOrdinal0": "ADD",
            "foldedMathOrdinal2": "MUL",
            "periodic": "FRACTIONAL",
            "sine": "SIN_OR_COS_BY_SERIALIZED_BOOL",
        },
        "evidenceLinesOneBased": {
            "foldedMathEnum": line_of("enum EFoldedMathOperation"),
            "foldedMathAddOpcode": line_of("case FMO_Add:"),
            "foldedMathMulOpcode": line_of("case FMO_Mul:"),
            "periodicClass": line_of("class FMaterialUniformExpressionPeriodic"),
            "sineClass": line_of("class FMaterialUniformExpressionSine"),
        },
    }


def validate_manifest(document: dict[str, Any]) -> None:
    require(document.get("schema") == MANIFEST_SCHEMA, "source-value manifest schema changed")
    require(document.get("formatVersion") == MANIFEST_FORMAT_VERSION, "source-value manifest version changed")
    targets = document.get("targets")
    require(isinstance(targets, list) and len(targets) == len(set(targets)), "source-value targets are invalid")
    summary = document.get("summary", {})
    require(
        summary.get("expectedExactInputTargetCount") == len(targets)
        and summary.get("expectedSourceValueUniformCb0ClosureCount") == len(targets)
        and summary.get("expectedBlockedTargetCount") == 0,
        "source-value target denominator changed",
    )
    require(
        summary.get("sliceExcludedAndFamilyLiteFallbackPreserved") is True,
        "Slice family-lite preservation contract changed",
    )


def validate_input_receipt(
    path: Path, expected: dict[str, Any], target_ids: list[str]
) -> dict[str, Any]:
    require(path.is_file(), f"G03-3 receipt is missing: {path}")
    require(digest_file(path) == expected["rawSha256"], "G03-3 receipt raw SHA changed")
    receipt = read_json(path)
    require(receipt.get("schema") == INPUT_RECEIPT_SCHEMA, "G03-3 receipt schema changed")
    require(receipt.get("formatVersion") == INPUT_RECEIPT_FORMAT_VERSION, "G03-3 receipt version changed")
    require(receipt.get("receiptSha256") == expected["receiptSha256"], "G03-3 receipt seal changed")
    rows = {row["targetId"]: row for row in receipt["targets"]}
    require(set(target_ids).issubset(rows), "source-value target is absent from G03-3")
    for target_id in target_ids:
        row = rows[target_id]
        require(row["status"] == STATUS_EXACT, f"source-value target is not exact: {target_id}")
        require(
            row["nativeShaderObjectBinding"]["status"]
            == "EXACT_NATIVE_SHADER_OBJECT_BINDING",
            f"source-value target lacks native CB0 wire: {target_id}",
        )
    return receipt


def _source_packages(
    source_root: Path, receipt: dict[str, Any]
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    packages: dict[str, Any] = {}
    identities = []
    for expected in receipt["inputs"]["sourcePackages"]:
        path = source_root / expected["fileName"]
        require(path.is_file(), f"pinned source package is missing: {path}")
        raw_sha = digest_file(path)
        require(raw_sha == expected["rawSha256"], f"source package SHA changed: {path.name}")
        packages[path.name.casefold()] = load_package(path, LOSTARK_KR_AES_KEY)
        identities.append(
            {
                "fileName": path.name,
                "physicalByteSize": path.stat().st_size,
                "rawSha256": raw_sha,
            }
        )
    return packages, identities


def build_receipt(
    manifest_path: Path,
    input_receipt_path: Path,
    source_root: Path,
    official_source_path: Path,
) -> dict[str, Any]:
    manifest = read_json(manifest_path)
    validate_manifest(manifest)
    target_ids = list(manifest["targets"])
    input_expected = manifest["inputs"]["exactMaterialMapReceipt"]
    input_receipt = validate_input_receipt(
        input_receipt_path, input_expected, target_ids
    )
    official_source = validate_official_source(
        official_source_path, manifest["inputs"]["officialEngineSource"]
    )
    packages, source_identities = _source_packages(source_root, input_receipt)
    global_exports = _export_index(packages)
    input_by_id = {row["targetId"]: row for row in input_receipt["targets"]}
    context = manifest["evaluationContext"]

    targets = []
    for target_id in target_ids:
        source = input_by_id[target_id]
        package_name = source["mic"]["sourcePackageFileName"]
        package = packages[package_name.casefold()]
        export_index = int(source["mic"]["exportIndexZeroBased"])
        require(export_index < len(package.exports), "MIC export index is invalid")
        export = package.exports[export_index]
        actual_path = package_ref_path(
            export.index + 1, package.imports, package.exports
        )
        require(
            actual_path.casefold() == source["mic"]["micObjectPath"].casefold(),
            f"MIC object path changed: {target_id}",
        )
        layers, terminal_parent = decode_mic_hierarchy(
            package, export, global_exports
        )
        effective_scalars = merge_effective_overrides(layers, "scalarOverrides")
        effective_vectors = merge_effective_overrides(layers, "vectorOverrides")
        evaluation = evaluate_uniform_set_into_cb0(
            source["materialMap"]["uniformExpressionSet"],
            source["nativeShaderObjectBinding"],
            effective_scalars,
            effective_vectors,
            context,
        )
        targets.append(
            {
                "targetId": target_id,
                "familyId": source["familyId"],
                "rendererType": source["rendererType"],
                "status": STATUS_CLOSED,
                "micHierarchyRootToLeaf": layers,
                "terminalParentMaterial": terminal_parent,
                "effectiveScalarOverrides": effective_scalars,
                "effectiveVectorOverrides": effective_vectors,
                "uniformExpressionCounts": source["materialMap"]["uniformExpressionCounts"],
                "sourceValueUniformEvaluation": evaluation,
                "sourceValueUniformCb0ClosureAdmission": True,
                "sourceValueReplayAdmission": False,
                "actualVfPassAdmission": False,
                "runtimeAdmission": False,
                "visualAdmission": False,
            }
        )

    expected_count = manifest["summary"]["expectedSourceValueUniformCb0ClosureCount"]
    require(len(targets) == expected_count, "source-value CB0 closure denominator changed")
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": RECEIPT_FORMAT_VERSION,
        "identity": manifest["identity"],
        "scope": {
            "stage": "G03_5_SOURCE_VALUE_UNIFORM_CB0_CLOSURE",
            "classNeutralEvaluator": True,
            "numericMaterialUniformSliceClosed": True,
            "nativeScalarGroupPackingSourceClosed": False,
            "texturesAndSamplersClosed": False,
            "engineOwnedConstantRowsClosed": False,
            "sourceValueReplayAdmission": False,
            "actualVfPassAdmission": False,
            "runtimeAdmission": False,
            "visualAdmission": False,
        },
        "inputs": {
            "manifest": {
                "repoRelativePath": manifest_path.relative_to(REPOSITORY_ROOT).as_posix(),
                "rawSha256": digest_file(manifest_path),
            },
            "g03_3ExactMaterialMapReceipt": {
                "repoRelativePath": input_receipt_path.relative_to(REPOSITORY_ROOT).as_posix(),
                "rawSha256": digest_file(input_receipt_path),
                "receiptSha256": input_receipt["receiptSha256"],
            },
            "evaluator": {
                "repoRelativePath": Path(__file__).resolve().relative_to(REPOSITORY_ROOT).as_posix(),
                "rawSha256": digest_file(Path(__file__).resolve()),
            },
            "officialEngineSource": official_source,
            "sourcePackages": source_identities,
        },
        "parameterIdentityAndPrecedence": manifest["parameterIdentity"],
        "targets": targets,
        "summary": {
            "exactInputTargetCount": len(targets),
            "sourceValueUniformCb0ClosureCount": sum(
                row["sourceValueUniformCb0ClosureAdmission"] for row in targets
            ),
            "sourceValueReplayAdmissionCount": 0,
            "actualVfPassAdmissionCount": 0,
            "runtimeAdmissionCount": 0,
            "visualAdmissionCount": 0,
            "result": "PASS_G03_5_SOURCE_VALUE_UNIFORM_CB0_CLOSED_TEXTURE_SAMPLER_REPLAY_OPEN",
        },
        "decision": {
            "foldedMathOrdinal0": "SOURCE_PROVEN_ADD",
            "foldedMathOrdinal2": "SOURCE_PROVEN_MUL_AND_ARTIST_COOKED_CORROBORATED",
            "sourceValueUniformCb0ClosureAdmission": True,
            "sourceValueReplayAdmission": False,
            "remainingBlockers": [
                "NATIVE_SCALAR_GROUP_LANE_ORDER_AND_PADDING_SOURCE_ABI_NOT_PROVEN",
                "PARENT_DEFAULT_AND_EFFECTIVE_TEXTURE_CLOSURE_INCOMPLETE",
                "SOURCE_SAMPLER_FILTER_ADDRESS_AND_COLORSPACE_EVIDENCE_INCOMPLETE",
                "ENGINE_OR_RENDERER_CB0_ROWS_AND_SCENE_DEPTH_INPUTS_NOT_SOURCE_CLOSED",
                "ACTUAL_VF_PASS_ADMISSION_OPEN"
            ],
            "sliceExcluded": True,
            "sliceSourceScaleSizePoint25Untouched": True,
            "familyLiteFallbackPreserved": True,
        },
    }
    _seal(receipt)
    return receipt


def validate_output_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == RECEIPT_SCHEMA, "output receipt schema changed")
    require(receipt.get("formatVersion") == RECEIPT_FORMAT_VERSION, "output receipt version changed")
    require(
        receipt["scope"]["nativeScalarGroupPackingSourceClosed"] is False
        and "NATIVE_SCALAR_GROUP_LANE_ORDER_AND_PADDING_SOURCE_ABI_NOT_PROVEN"
        in receipt["decision"]["remainingBlockers"],
        "native scalar packing admission boundary changed",
    )
    summary = receipt["summary"]
    require(
        summary["sourceValueUniformCb0ClosureCount"]
        == summary["exactInputTargetCount"]
        and summary["sourceValueReplayAdmissionCount"] == 0,
        "output receipt admission boundary changed",
    )
    for target in receipt["targets"]:
        require(target["status"] == STATUS_CLOSED, "target uniform closure failed")
        require(
            target["sourceValueUniformCb0ClosureAdmission"] is True
            and target["sourceValueReplayAdmission"] is False
            and target["runtimeAdmission"] is False
            and target["visualAdmission"] is False,
            "target admission boundary changed",
        )
        cb0 = target["sourceValueUniformEvaluation"]["nativeCb0"]
        require(
            cb0["materialBoundFloat4Count"]
            + cb0["engineOrRendererUnboundFloat4Count"]
            == cb0["declaredFloat4Count"],
            "target CB0 ownership denominator changed",
        )
    expected_seal = receipt["receiptSha256"]
    candidate = dict(receipt)
    candidate.pop("receiptSha256")
    require(canonical_json_sha256(candidate) == expected_seal, "output receipt seal changed")


def check_or_write(path: Path, receipt: dict[str, Any], check: bool) -> None:
    validate_output_receipt(receipt)
    if check:
        require(path.is_file(), f"source-value receipt is missing: {path}")
        existing = read_json(path)
        require(existing == receipt, "source-value receipt is stale")
        return
    write_json_atomic(path, receipt)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--input-receipt", type=Path, default=DEFAULT_INPUT_RECEIPT)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--official-source", type=Path, default=DEFAULT_OFFICIAL_SOURCE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)
    try:
        receipt = build_receipt(
            args.manifest.resolve(),
            args.input_receipt.resolve(),
            args.source_root.resolve(),
            args.official_source.resolve(),
        )
        check_or_write(args.output.resolve(), receipt, args.check)
        print(
            "UE3 source-value uniform CB0 PASS: "
            f"targets={receipt['summary']['sourceValueUniformCb0ClosureCount']} "
            "sourceReplay=0 runtime=0 visual=0"
        )
        return 0
    except (ValueError, OSError, KeyError, IndexError, struct.error) as error:
        print(f"UE3 source-value uniform CB0 FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
