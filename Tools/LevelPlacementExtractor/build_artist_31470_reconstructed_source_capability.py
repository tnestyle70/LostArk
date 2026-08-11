#!/usr/bin/env python3
"""Build the Artist F reconstructed Source capability evidence.

This artifact is intentionally not a source-exact or runtime-admission receipt.
It binds the 29 blocked Source module occurrences to seven closed policy
families and records deterministic, finite numeric samples for independent
review.  Current-revision evidence remains cross-revision evidence and every
upstream blocker remains visible.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import sys
from collections import Counter
from pathlib import Path
from typing import Any

from build_artist_31470_custom_handler_oracle import (
    FIXED_ORACLE_OCCURRENCE_SEED,
    FRANDOM_STREAM_ALGORITHM,
    random_stream_units,
    validate_receipt as validate_custom_handler_receipt,
)
from build_artist_31470_source_execution_semantics import (
    canonical_sha256,
    canonical_text_sha256,
    evaluate_descriptor,
    json_bytes,
    require,
    sha256_file,
    validate_receipt as validate_source_execution_receipt,
)
from build_artist_31470_source_oracle_acquisition import (
    validate_receipt as validate_acquisition_receipt,
)
from effect_source_contract_io import load_strict_json_object


SCHEMA = "lostark.effect-reconstructed-source-capability"
FORMAT_VERSION = 1
POLICY_NAME = "RECONSTRUCTED_APPROVED_V1"
OUTPUT_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-source-capability.receipt.json"
)
SOURCE_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)
CUSTOM_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.custom-handler-oracle.receipt.json"
)
ACQUISITION_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-oracle-acquisition.receipt.json"
)
SOURCE_RECEIPT_PATH = "Data/Effects/Imported/Artist/skill.31470.source-receipt.json"

PINNED_INPUTS = {
    SOURCE_PATH: {
        "canonicalTextSha256": "de15843dfa2f151371c1e26c472f2d42a0bfd7c7f8c8a41a3cdd2da08eaccb9a",
        "selfSha256": "7e1113dd05bcc9b51056cacc27da1805f7a6d26f65dda5b72c99d26c3141a71c",
    },
    CUSTOM_PATH: {
        "canonicalTextSha256": "c436e69e40e7ad13079940e9ed88d1522942c10a135e19c1d19b020e86b797ac",
        "selfSha256": "0da627b3ed5b100014f2a2ac1fa3591d861c6a241befee65ca856b406dedaadc",
    },
    ACQUISITION_PATH: {
        "canonicalTextSha256": "05d8c90b2000fbc6aa699f63805784b2013a700aa94ee5e85c4d4a30459cafbc",
        "selfSha256": "c49f4dfcabc09c765b2127c620c5dbc8676d18819de008eb813e583b3f07e98d",
    },
}

EXECUTION_DEPENDENCY_PATHS = (
    "Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py",
    "Tools/LevelPlacementExtractor/build_artist_31470_source_execution_semantics.py",
    "Tools/LevelPlacementExtractor/build_artist_31470_custom_handler_oracle.py",
    "Tools/LevelPlacementExtractor/build_artist_31470_source_oracle_acquisition.py",
    "Tools/LevelPlacementExtractor/effect_source_contract_io.py",
)

SAMPLE_TIMES = (0.0, 0.25, 1.0)
ABSOLUTE_TOLERANCE = 1.0e-6
RELATIVE_TOLERANCE = 1.0e-6
FIXED_CONTEXT = {
    "inputPosition": [1.0, -2.0, 0.5],
    "inputVelocity": [1.0, -2.0, 0.5],
    "baseVector": [1.0, 1.0, 1.0],
    "groundPlaneZ": 0.0,
}

EVIDENCE_BLOCKERS = (
    "CURRENT_REVISION_CROSS_REVISION_EVIDENCE",
    "SOURCE_EXACT_NOT_CLAIMED",
    "INDEPENDENT_REVIEW_PENDING",
    "RUNTIME_HANDLER_CONSUMPTION_PENDING",
    "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED",
)

EXPECTED_FAMILY_COUNTS = {
    "source.reconstructed.seeded.v1": 11,
    "source.reconstructed.cylinder-spin.v1": 5,
    "source.reconstructed.ground.v1": 2,
    "source.reconstructed.decal.v1": 3,
    "source.reconstructed.light.v1": 1,
    "source.reconstructed.velocity.v1": 4,
    "source.reconstructed.ef-vector-multiply.v1": 3,
}

IMPLEMENTATION_VERSIONS = {
    family_id: 2 if family_id == "source.reconstructed.decal.v1" else 1
    for family_id in EXPECTED_FAMILY_COUNTS
}

DECAL_SOURCE_CLASS = "efparticlemoduletypedatadecal"
EXPECTED_DECAL_NEAR_PLANE = -300.0
EXPECTED_DECAL_CDO_VALUES = {
    "defaultsize": {"x": 50.0, "y": 50.0},
    "farplane": 300.0,
    "blendrange": {"x": 100.0, "y": 100.0},
    "bonlycalcrotationyaw": True,
    "bsupported3ddrawmode": True,
}

CLASS_CONTRACTS: dict[str, dict[str, Any]] = {
    "particlemodulecolor_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_COLOR",
        "literals": ("brequiresloopingnotification", "lodvalidity", "randomseedinfo.properties.randomseeds[0]", "randomseedinfo.size"),
        "distributions": ("startalpha", "startcolor"),
    },
    "particlemodulelifetime_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_LIFETIME",
        "literals": ("lodvalidity", "randomseedinfo.properties.randomseeds[0]", "randomseedinfo.size"),
        "distributions": ("lifetime",),
    },
    "particlemodulelocation_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_LOCATION",
        "literals": ("lodvalidity", "randomseedinfo.hex", "randomseedinfo.size"),
        "distributions": ("startlocation",),
    },
    "particlemodulelocationprimitivecylinder_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_CYLINDER",
        "literals": ("b3ddrawmode", "badjustforworldspace", "heightaxis", "lodvalidity", "negative_x", "negative_z", "positive_x", "positive_z", "randomseedinfo.hex", "randomseedinfo.size", "surfaceonly", "velocity"),
        "distributions": ("startheight", "startlocation", "startradius", "velocityscale"),
    },
    "particlemodulemeshrotation_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_MESH_ROTATION",
        "literals": ("lodvalidity", "randomseedinfo.hex", "randomseedinfo.size"),
        "distributions": ("startrotation",),
    },
    "particlemodulesize_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_SIZE",
        "literals": ("lodvalidity", "randomseedinfo.hex", "randomseedinfo.size"),
        "distributions": ("startsize",),
    },
    "particlemodulevelocity_seeded": {
        "family": "source.reconstructed.seeded.v1",
        "variant": "SEEDED_VELOCITY",
        "literals": ("lodvalidity", "randomseedinfo.hex", "randomseedinfo.size"),
        "distributions": ("startvelocity", "startvelocityradial"),
    },
    "efparticlemodulelocationprimitivecylinderspin": {
        "family": "source.reconstructed.cylinder-spin.v1",
        "variant": "EF_CYLINDER_SPIN",
        "literals": ("badjustforworldspace", "benabled", "lodvalidity", "negative_x", "positive_y", "spinaxis", "surfaceonly", "velocity"),
        "distributions": ("spinangle", "startcylinderrot", "startheight", "startlocation", "startradius", "velocityscale"),
    },
    "efparticlemodulelocationprimitivecylinderspin_seeded": {
        "family": "source.reconstructed.cylinder-spin.v1",
        "variant": "EF_CYLINDER_SPIN_SEEDED",
        "literals": ("badjustforworldspace", "lodvalidity", "negative_x", "negative_y", "randomseedinfo.hex", "randomseedinfo.properties.randomseeds[0]", "randomseedinfo.size", "surfaceonly", "velocity"),
        "distributions": ("spinangle", "startcylinderrot", "startheight", "startlocation", "startradius", "velocityscale"),
    },
    "efparticlemodulelocationonground": {
        "family": "source.reconstructed.ground.v1",
        "variant": "EF_GROUND_PLANE_QUERY",
        "literals": ("lodvalidity", "skiplocation.distribution.objectpath"),
        "distributions": ("adjustlocation", "skiplocation"),
    },
    "efparticlemoduletypedatadecal": {
        "family": "source.reconstructed.decal.v1",
        "variant": "EF_DECAL_DESCRIPTOR",
        "literals": ("lodvalidity", "nearplane"),
        "distributions": (),
    },
    "efparticlemoduletypedatalight": {
        "family": "source.reconstructed.light.v1",
        "variant": "EF_POINT_LIGHT_DESCRIPTOR",
        "literals": ("b3ddrawmode", "lodvalidity", "pointlightcomponent", "pointlightcomponent.objectpath"),
        "distributions": (),
    },
    "efparticlemodulevelocityoverlifetime": {
        "family": "source.reconstructed.velocity.v1",
        "variant": "EF_VELOCITY_OVER_LIFETIME",
        "literals": ("lodvalidity",),
        "distributions": ("veloverlife",),
    },
    "particlemodulecolorscaleoverlife": {
        "family": "source.reconstructed.ef-vector-multiply.v1",
        "variant": "EF_COLOR_SCALE_PARAMETER_MULTIPLY",
        "literals": ("colorscaleoverlife.distribution.objectpath", "lodvalidity"),
        "distributions": ("alphascaleoverlife", "colorscaleoverlife"),
    },
    "particlemodulemeshrotation": {
        "family": "source.reconstructed.ef-vector-multiply.v1",
        "variant": "EF_MESH_ROTATION_PARAMETER_MULTIPLY",
        "literals": ("lodvalidity", "startrotation.distribution.objectpath"),
        "distributions": ("startrotation",),
    },
}


def tracked_json_sha256(path: Path) -> str:
    payload = path.read_bytes()
    require(not payload.startswith(b"\xef\xbb\xbf"),
            f"tracked JSON must not contain UTF-8 BOM: {path}")
    try:
        text = payload.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError(f"tracked JSON is not UTF-8: {path}") from exc
    canonical = text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    return hashlib.sha256(canonical).hexdigest()


def _execution_dependencies(root: Path) -> list[dict[str, Any]]:
    rows = []
    for relative_path in EXECUTION_DEPENDENCY_PATHS:
        path = root / relative_path
        require(path.is_file(), f"execution dependency is missing: {relative_path}")
        rows.append({
            "path": relative_path,
            "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT_NO_BOM",
            "canonicalTextSha256": canonical_text_sha256(path),
        })
    return rows


def _schema_object(fields: dict[str, Any]) -> dict[str, Any]:
    return {"type": "OBJECT", "additionalFieldsAllowed": False, "fields": fields}


def _schema_vector(length: int) -> dict[str, Any]:
    return {"type": "FLOAT64_VECTOR", "length": length}


def _schema_enum(*values: str) -> dict[str, Any]:
    return {"type": "ENUM", "values": list(values)}


def _literal_value_schema(kind: str) -> dict[str, Any]:
    mapping = {
        "number": {"type": "FLOAT64"},
        "boolean": {"type": "BOOLEAN"},
        "string": {"type": "STRING"},
    }
    require(kind in mapping, f"unknown literal kind: {kind}")
    return mapping[kind]


def _default_row(field_path: str, value: Any) -> dict[str, Any]:
    return {
        "fieldPath": field_path,
        "value": copy.deepcopy(value),
        "provenance": "RECONSTRUCTED_APPROVED_V1_POLICY_DEFAULT_NOT_SOURCE_EVIDENCE",
        "sourceExact": False,
    }


def _decal_policy_defaults(
    source: dict[str, Any], modules: list[dict[str, Any]]
) -> dict[str, Any]:
    current_defaults = source.get("currentRevisionDefaultEvidence") or {}
    require(
        current_defaults.get("status") == "CURRENT_REVISION_DEFAULTS_NOT_SOURCE_EXACT",
        "Decal current-revision default evidence status changed",
    )
    decal_evidence = current_defaults.get("decal") or {}
    require(
        decal_evidence.get("provenance") == "CURRENT_EFGAME_CDO"
        and json_bytes(decal_evidence.get("values"))
        == json_bytes(EXPECTED_DECAL_CDO_VALUES),
        "Decal current EFGAME CDO values changed",
    )

    cdo = (current_defaults.get("classDefaultObjects") or {}).get(
        "efParticleModuleTypeDataDecal"
    ) or {}
    expected_properties = {
        "defaultsize": {
            "type": "structproperty",
            "structtype": "vector2d",
            "value": EXPECTED_DECAL_CDO_VALUES["defaultsize"],
        },
        "farplane": {
            "type": "floatproperty",
            "structtype": None,
            "value": EXPECTED_DECAL_CDO_VALUES["farplane"],
        },
        "blendrange": {
            "type": "structproperty",
            "structtype": "vector2d",
            "value": EXPECTED_DECAL_CDO_VALUES["blendrange"],
        },
        "bonlycalcrotationyaw": {
            "type": "boolproperty",
            "structtype": None,
            "value": EXPECTED_DECAL_CDO_VALUES["bonlycalcrotationyaw"],
        },
        "bsupported3ddrawmode": {
            "type": "boolproperty",
            "structtype": None,
            "value": EXPECTED_DECAL_CDO_VALUES["bsupported3ddrawmode"],
        },
    }
    record_sha = cdo.get("recordSha256")
    require(
        cdo.get("objectPath") == "Default__EFParticleModuleTypeDataDecal"
        and cdo.get("className") == DECAL_SOURCE_CLASS
        and type(cdo.get("exportIndex")) is int
        and cdo["exportIndex"] > 0
        and type(record_sha) is str
        and len(record_sha) == 64
        and all(character in "0123456789abcdef" for character in record_sha)
        and json_bytes(cdo.get("properties")) == json_bytes(expected_properties),
        "Decal current EFGAME CDO record changed",
    )

    require(len(modules) == EXPECTED_FAMILY_COUNTS["source.reconstructed.decal.v1"],
            "Decal occurrence denominator changed")
    implicit_default_ids = set()
    for module in modules:
        literal_rows = [
            row for row in module["typedPayload"]["literals"]
            if row["propertyPath"] == "nearplane"
        ]
        require(
            len(literal_rows) == 1
            and literal_rows[0]["kind"] == "number"
            and type(literal_rows[0]["value"]) is float
            and literal_rows[0]["value"] == EXPECTED_DECAL_NEAR_PLANE,
            f"Decal nearPlane literal changed: {module['moduleOccurrenceId']}",
        )
        default_rows = module.get("implicitDefaults") or []
        require(
            len(default_rows) == 1,
            f"Decal implicit default set is not unique: {module['moduleOccurrenceId']}",
        )
        default_row = default_rows[0]
        default_id = default_row.get("defaultId")
        require(
            type(default_id) is str
            and default_id.startswith(module["moduleOccurrenceId"] + "::default:")
            and default_id not in implicit_default_ids
            and default_row.get("family") == "Decal"
            and default_row.get("fieldPath") == "typedata.decal.class-default-set"
            and default_row.get("decision") == "READY_FOR_HANDLER"
            and default_row.get("provenance") == "CURRENT_EFGAME_CDO"
            and json_bytes(default_row.get("values"))
            == json_bytes(EXPECTED_DECAL_CDO_VALUES),
            f"Decal implicit default set changed: {module['moduleOccurrenceId']}",
        )
        implicit_default_ids.add(default_id)

    return {
        "farPlane": EXPECTED_DECAL_CDO_VALUES["farplane"],
        "defaultSize": [
            EXPECTED_DECAL_CDO_VALUES["defaultsize"]["x"],
            EXPECTED_DECAL_CDO_VALUES["defaultsize"]["y"],
        ],
        "blendRange": [
            EXPECTED_DECAL_CDO_VALUES["blendrange"]["x"],
            EXPECTED_DECAL_CDO_VALUES["blendrange"]["y"],
        ],
        "yawOnly": EXPECTED_DECAL_CDO_VALUES["bonlycalcrotationyaw"],
        "supports3dDrawMode": EXPECTED_DECAL_CDO_VALUES["bsupported3ddrawmode"],
    }


def _variant_default_rows(
    variant: str, decal_defaults: dict[str, Any] | None = None
) -> list[dict[str, Any]]:
    values: dict[str, Any] = {
        "fixedContext.inputPosition": FIXED_CONTEXT["inputPosition"],
        "fixedContext.inputVelocity": FIXED_CONTEXT["inputVelocity"],
        "fixedContext.baseVector": FIXED_CONTEXT["baseVector"],
        "fixedContext.groundPlaneZ": FIXED_CONTEXT["groundPlaneZ"],
        "seed.fallbackOccurrenceSeed": FIXED_ORACLE_OCCURRENCE_SEED,
    }
    if variant == "SEEDED_COLOR":
        values["sourceLiterals.brequiresloopingnotification"] = False
    if variant == "SEEDED_CYLINDER":
        values.update({
            "sourceLiterals.heightaxis": "pmlpc_heightaxis_z",
            "sourceLiterals.badjustforworldspace": False,
            "sourceLiterals.surfaceonly": False,
            "sourceLiterals.velocity": False,
            "sourceLiterals.positive_x": False,
            "sourceLiterals.negative_x": False,
            "sourceLiterals.positive_z": False,
            "sourceLiterals.negative_z": False,
            "distribution.startcylinderrot": [0.0],
            "distribution.spinangle": [0.0],
        })
    if variant in ("EF_CYLINDER_SPIN", "EF_CYLINDER_SPIN_SEEDED"):
        values.update({
            "sourceLiterals.spinaxis": "pmlpcs_axis_z",
            "sourceLiterals.benabled": True,
            "sourceLiterals.badjustforworldspace": False,
            "sourceLiterals.surfaceonly": False,
            "sourceLiterals.velocity": False,
            "sourceLiterals.positive_y": False,
            "sourceLiterals.negative_x": False,
            "sourceLiterals.negative_y": False,
        })
    if variant == "EF_GROUND_PLANE_QUERY":
        values["ground.skipThreshold"] = 0.5
    if variant == "EF_DECAL_DESCRIPTOR":
        require(decal_defaults is not None, "Decal CDO defaults are missing")
        values.update({
            "decal.farPlane": copy.deepcopy(decal_defaults["farPlane"]),
            "decal.defaultSize": copy.deepcopy(decal_defaults["defaultSize"]),
            "decal.blendRange": copy.deepcopy(decal_defaults["blendRange"]),
            "decal.yawOnly": decal_defaults["yawOnly"],
            "decal.supports3dDrawMode": decal_defaults["supports3dDrawMode"],
        })
    return [_default_row(path, value) for path, value in sorted(values.items())]


def _output_schema(variant: str, contract: dict[str, Any]) -> dict[str, Any]:
    fields: dict[str, Any] = {"variant": _schema_enum(variant)}
    if variant == "SEEDED_COLOR":
        fields.update({"color": _schema_vector(3), "alpha": {"type": "FLOAT64"}, "requiresLoopingNotification": {"type": "BOOLEAN"}})
    elif variant == "SEEDED_LIFETIME":
        fields["lifetime"] = {"type": "FLOAT64"}
    elif variant == "SEEDED_LOCATION":
        fields["position"] = _schema_vector(3)
    elif variant in ("SEEDED_CYLINDER", "EF_CYLINDER_SPIN", "EF_CYLINDER_SPIN_SEEDED"):
        flag_paths = [
            path for path in contract["literals"]
            if path in {
                "badjustforworldspace", "benabled", "surfaceonly", "velocity",
                "positive_x", "positive_y", "positive_z",
                "negative_x", "negative_y", "negative_z",
            }
        ]
        fields.update({
            "position": _schema_vector(3),
            "velocity": _schema_vector(3),
            "angleRadians": {"type": "FLOAT64"},
            "axis": _schema_enum("X", "Y", "Z"),
            "flags": _schema_object({path: {"type": "BOOLEAN"} for path in sorted(flag_paths)}),
        })
    elif variant == "SEEDED_MESH_ROTATION":
        fields["rotation"] = _schema_vector(3)
    elif variant == "SEEDED_SIZE":
        fields["size"] = _schema_vector(3)
    elif variant == "SEEDED_VELOCITY":
        fields.update({"velocity": _schema_vector(3), "baseVelocity": _schema_vector(3)})
    elif variant == "EF_GROUND_PLANE_QUERY":
        fields.update({"position": _schema_vector(3), "queryApplied": {"type": "BOOLEAN"}, "skipLocation": {"type": "FLOAT64"}})
    elif variant == "EF_DECAL_DESCRIPTOR":
        fields.update({
            "frustum": _schema_vector(6),
            "yawOnly": {"type": "BOOLEAN"},
            "supports3dDrawMode": {"type": "BOOLEAN"},
        })
    elif variant == "EF_POINT_LIGHT_DESCRIPTOR":
        fields.update({
            "brightness": {"type": "FLOAT64"},
            "radius": {"type": "FLOAT64"},
            "falloffExponent": {"type": "FLOAT64"},
            "linearColor": _schema_vector(4),
            "castCompositeShadow": {"type": "BOOLEAN"},
            "affectCompositeShadowDirection": {"type": "BOOLEAN"},
        })
    elif variant == "EF_VELOCITY_OVER_LIFETIME":
        fields["velocity"] = _schema_vector(3)
    elif variant == "EF_COLOR_SCALE_PARAMETER_MULTIPLY":
        fields.update({"alphaScale": {"type": "FLOAT64"}, "mappedParameter": _schema_vector(3), "multipliedVector": _schema_vector(3)})
    elif variant == "EF_MESH_ROTATION_PARAMETER_MULTIPLY":
        fields.update({"mappedParameter": _schema_vector(3), "multipliedVector": _schema_vector(3)})
    else:
        raise ValueError(f"unknown output schema variant: {variant}")
    return _schema_object(fields)


def _extra_input_schema(variant: str) -> dict[str, Any]:
    if variant == "EF_GROUND_PLANE_QUERY":
        return {"inputPosition": _schema_vector(3), "groundPlaneZ": {"type": "FLOAT64"}}
    if variant == "EF_DECAL_DESCRIPTOR":
        return {
            "nearPlane": {"type": "FLOAT64"}, "farPlane": {"type": "FLOAT64"},
            "defaultSize": _schema_vector(2), "blendRange": _schema_vector(2),
            "yawOnly": {"type": "BOOLEAN"},
            "supports3dDrawMode": {"type": "BOOLEAN"},
        }
    if variant == "EF_POINT_LIGHT_DESCRIPTOR":
        return {"pointLightFields": _schema_object({
            "brightness": {"type": "FLOAT64"},
            "bcastcompositeshadow": {"type": "BOOLEAN"},
            "baffectcompositeshadowdirection": {"type": "BOOLEAN"},
            "radius": {"type": "FLOAT64"},
            "falloffexponent": {"type": "FLOAT64"},
            "lightcolor": _schema_object({key: {"type": "UINT8"} for key in ("r", "g", "b", "a")}),
        })}
    if variant == "EF_VELOCITY_OVER_LIFETIME":
        return {"inputVelocity": _schema_vector(3)}
    if variant in ("EF_COLOR_SCALE_PARAMETER_MULTIPLY", "EF_MESH_ROTATION_PARAMETER_MULTIPLY"):
        return {"parameterFields": _schema_object({
            "parameterName": {"type": "STRING"},
            "parameterInput": {"type": "OPTIONAL", "valueSchema": _schema_vector(3)},
            "constant": _schema_vector(3),
            "paramModes": {"type": "ENUM_VECTOR", "length": 3, "values": ["dpm_normal", "dpm_abs", "dpm_direct"]},
            "minimumInput": _schema_vector(3), "maximumInput": _schema_vector(3),
            "minimumOutput": _schema_vector(3), "maximumOutput": _schema_vector(3),
            "baseVector": _schema_vector(3),
        })}
    return {}


def _blocked_modules_by_class(source: dict[str, Any], source_class: str) -> list[dict[str, Any]]:
    return [
        module for occurrence in source["occurrences"] for module in occurrence["modules"]
        if module["decision"] == "BLOCKED" and module["exactSourceClass"] == source_class
    ]


def _variant_binding(source: dict[str, Any], source_class: str) -> dict[str, Any]:
    contract = CLASS_CONTRACTS[source_class]
    modules = _blocked_modules_by_class(source, source_class)
    require(modules, f"variant source class has no occurrence: {source_class}")
    decal_defaults = (
        _decal_policy_defaults(source, modules)
        if source_class == DECAL_SOURCE_CLASS else None
    )
    literal_fields: dict[str, Any] = {}
    for property_path in contract["literals"]:
        kinds = {
            row["kind"] for module in modules for row in module["typedPayload"]["literals"]
            if row["propertyPath"] == property_path
        }
        require(len(kinds) == 1, f"literal type is not exact: {source_class} {property_path}")
        literal_fields[property_path] = {
            "type": "PRESENCE_WRAPPED",
            "valueSchema": _literal_value_schema(next(iter(kinds))),
        }
    distribution_fields: dict[str, Any] = {}
    for property_path in contract["distributions"]:
        if contract["family"] == "source.reconstructed.ef-vector-multiply.v1" and property_path in ("colorscaleoverlife", "startrotation"):
            continue
        lengths = set()
        for module in modules:
            descriptor = _payload_descriptor(module, property_path)
            if descriptor is not None:
                lengths.add(max(1, int(descriptor["componentCount"])))
            else:
                adapter = _adapter(module, property_path)
                samples = adapter.get("numericOracleSamples") or []
                values = [row["value"] for row in samples if "value" in row]
                require(values, f"distribution schema sample missing: {source_class} {property_path}")
                lengths.update(len(value) for value in values)
        require(len(lengths) == 1, f"distribution vector length changed: {source_class} {property_path}")
        distribution_fields[property_path] = _schema_vector(next(iter(lengths)))
    input_fields = {
        "time": {"type": "FLOAT64"},
        "fixedSeed": {"type": "INT32"},
        "randomUnits": _schema_vector(4),
        "sourceLiterals": _schema_object(literal_fields),
        "evaluatedDistributions": _schema_object(distribution_fields),
        **_extra_input_schema(contract["variant"]),
    }
    return {
        "exactSourceClass": source_class,
        "variant": contract["variant"],
        "expectedOccurrenceCount": len(modules),
        "inputSchema": _schema_object(input_fields),
        "outputSchema": _output_schema(contract["variant"], contract),
        "explicitDefaults": _variant_default_rows(
            contract["variant"], decal_defaults
        ),
        "allowedLiteralPaths": list(contract["literals"]),
        "distributionPropertyPaths": list(contract["distributions"]),
    }


def build_family_policies(root: Path, source: dict[str, Any]) -> list[dict[str, Any]]:
    algorithms = {
        "source.reconstructed.seeded.v1": "UE3_LCG_FIXED_STREAM_THEN_EXACT_CLASS_VARIANT_DISTRIBUTION_PROJECTION",
        "source.reconstructed.cylinder-spin.v1": "EVALUATE_SIX_DISTRIBUTIONS_THEN_AXIS_PLANE_POLAR_POSITION_AND_TANGENT_VELOCITY",
        "source.reconstructed.ground.v1": "FIXED_GROUND_PLANE_QUERY_WITH_EXPLICIT_SKIP_PARAMETER_FALLBACK",
        "source.reconstructed.decal.v1": "EXPLICIT_DECAL_FRUSTUM_DESCRIPTOR_FROM_SOURCE_NEAR_PLANE_AND_CURRENT_EFGAME_CDO_DEFAULTS",
        "source.reconstructed.light.v1": "POINT_LIGHT_DESCRIPTOR_FROM_EXPLICIT_INSTANCE_AND_CURRENT_ARCHETYPE_CDO_FIELDS",
        "source.reconstructed.velocity.v1": "COMPONENTWISE_VELOCITY_MULTIPLY_OVER_LIFETIME",
        "source.reconstructed.ef-vector-multiply.v1": "DPM_COMPONENT_MAP_THEN_COMPONENTWISE_BASE_VECTOR_MULTIPLY",
    }
    dependencies = _execution_dependencies(root)
    dependency_sha = canonical_sha256(dependencies)
    policies = []
    for family_id in EXPECTED_FAMILY_COUNTS:
        bindings = [
            _variant_binding(source, source_class)
            for source_class, contract in CLASS_CONTRACTS.items()
            if contract["family"] == family_id
        ]
        bindings.sort(key=lambda row: row["exactSourceClass"])
        semantic_contract = {
            "algorithm": algorithms[family_id],
            "variantBindings": bindings,
            "sampleTimes": list(SAMPLE_TIMES),
            "randomStreamAlgorithm": FRANDOM_STREAM_ALGORITHM,
            "fallbackOccurrenceSeed": FIXED_ORACLE_OCCURRENCE_SEED,
            "fixedContext": FIXED_CONTEXT,
            "absoluteTolerance": ABSOLUTE_TOLERANCE,
            "relativeTolerance": RELATIVE_TOLERANCE,
            "executionDependencySetSha256": dependency_sha,
            "genericFallbackAllowed": False,
        }
        semantic_sha = canonical_sha256(semantic_contract)
        implementation = {
            "implementationId": family_id + ".implementation",
            "implementationVersion": IMPLEMENTATION_VERSIONS[family_id],
            "familySemanticImplementationSha256": semantic_sha,
            "semanticContract": semantic_contract,
        }
        policies.append({
            "policyFamilyId": family_id,
            **implementation,
            "implementationSha256": canonical_sha256(implementation),
            "executionDependencies": dependencies,
            "sampleContract": {
                "times": list(SAMPLE_TIMES),
                "randomStreamAlgorithm": FRANDOM_STREAM_ALGORITHM,
                "fallbackOccurrenceSeed": FIXED_ORACLE_OCCURRENCE_SEED,
                "absoluteTolerance": ABSOLUTE_TOLERANCE,
                "relativeTolerance": RELATIVE_TOLERANCE,
                "finiteValuesRequired": True,
            },
            "evidenceFidelity": "CURRENT_REVISION_CROSS_REVISION_EVIDENCE",
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "productAdmission": False,
        })
    require(len(policies) == 7, "policy family denominator changed")
    return policies


def _literal_map(module: dict[str, Any]) -> dict[str, Any]:
    rows = module["typedPayload"]["literals"]
    result: dict[str, Any] = {}
    for row in rows:
        path = str(row["propertyPath"])
        require(path not in result, f"duplicate literal path: {module['moduleOccurrenceId']} {path}")
        result[path] = copy.deepcopy(row["value"])
    return result


def _vector(value: Any, count: int = 3) -> list[float]:
    if isinstance(value, dict):
        order = ("x", "y", "z", "w")
        require(all(order[index] in value for index in range(count)),
                "vector object component is missing")
        result = [float(value[order[index]]) for index in range(count)]
    elif isinstance(value, list):
        require(len(value) >= count, "vector array component is missing")
        result = [float(value[index]) for index in range(count)]
    else:
        require(count == 1, "scalar cannot silently fill vector components")
        result = [float(value)]
    require(all(math.isfinite(item) for item in result), "non-finite vector input")
    return result


def _scalar(value: Any) -> float:
    result = _vector(value, 1)[0]
    require(math.isfinite(result), "non-finite scalar input")
    return result


def _variant_binding_for_policy(policy: dict[str, Any], variant: str) -> dict[str, Any]:
    matches = [
        row for row in policy["semanticContract"]["variantBindings"]
        if row["variant"] == variant
    ]
    require(len(matches) == 1, f"variant binding is not unique: {variant}")
    return matches[0]


def _default_map(binding: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for row in binding["explicitDefaults"]:
        require(row["fieldPath"] not in result,
                f"duplicate explicit default: {row['fieldPath']}")
        require(row["provenance"]
                == "RECONSTRUCTED_APPROVED_V1_POLICY_DEFAULT_NOT_SOURCE_EVIDENCE"
                and row["sourceExact"] is False,
                f"default provenance changed: {row['fieldPath']}")
        result[row["fieldPath"]] = copy.deepcopy(row["value"])
    return result


def _resolved_literal(
    literals: dict[str, Any],
    property_path: str,
    defaults: dict[str, Any],
) -> Any:
    if property_path in literals:
        return copy.deepcopy(literals[property_path])
    default_path = "sourceLiterals." + property_path
    require(default_path in defaults,
            f"missing literal has no explicit reconstructed default: {property_path}")
    return copy.deepcopy(defaults[default_path])


def _presence_wrapped_literals(
    literals: dict[str, Any],
    contract: dict[str, Any],
) -> dict[str, Any]:
    return {
        property_path: {
            "present": property_path in literals,
            "value": copy.deepcopy(literals[property_path])
            if property_path in literals else None,
        }
        for property_path in contract["literals"]
    }


def _payload_descriptor(module: dict[str, Any], property_path: str) -> dict[str, Any] | None:
    matches = [
        row["descriptor"] for row in module["typedPayload"]["distributions"]
        if row["descriptor"].get("propertyPath") == property_path
    ]
    require(len(matches) <= 1,
            f"duplicate payload distribution: {module['moduleOccurrenceId']} {property_path}")
    return matches[0] if matches else None


def _adapter(module: dict[str, Any], property_path: str) -> dict[str, Any]:
    matches = [
        row for row in module["distributionAdapters"]
        if str(row.get("distributionId", "")).endswith("::distribution:" + property_path)
    ]
    require(len(matches) == 1,
            f"distribution adapter mismatch: {module['moduleOccurrenceId']} {property_path}")
    return matches[0]


def _evaluated_distribution(
    module: dict[str, Any],
    property_path: str,
    time: float,
    random_units: list[float],
) -> list[float]:
    descriptor = _payload_descriptor(module, property_path)
    if descriptor is not None and descriptor.get("payloadStatus") == "INLINE_SOURCE_PAYLOAD":
        value = evaluate_descriptor(descriptor, time, tuple(random_units))
        return _vector(value, max(1, int(descriptor.get("componentCount", 1))))
    adapter = _adapter(module, property_path)
    samples = adapter.get("numericOracleSamples") or []
    timed = [row for row in samples if row.get("time") == time and "value" in row]
    if len(timed) == 1:
        return _vector(timed[0]["value"], max(1, len(timed[0]["value"])))
    fallback = [row for row in samples if row.get("branch") == "CONSTANT_FALLBACK"]
    if len(fallback) == 1:
        return _vector(fallback[0]["value"], max(1, len(fallback[0]["value"])))
    raise ValueError(
        f"no closed numeric distribution input: {module['moduleOccurrenceId']} {property_path}"
    )


def _seed(module: dict[str, Any], defaults: dict[str, Any]) -> tuple[int, str]:
    seed = module.get("seed")
    if isinstance(seed, dict) and seed.get("randomSeeds"):
        value = int(seed["randomSeeds"][0])
        return value, "SOURCE_DECODED_FIRST_RANDOM_SEED"
    require("seed.fallbackOccurrenceSeed" in defaults,
            "missing seed has no explicit reconstructed default")
    return int(defaults["seed.fallbackOccurrenceSeed"]), "POLICY_FIXED_OCCURRENCE_SEED"


def _axis(literals: dict[str, Any], defaults: dict[str, Any], custom: bool) -> str:
    raw = _resolved_literal(
        literals, "spinaxis" if custom else "heightaxis", defaults
    )
    mapping = {
        "pmlpcs_axis_x": "X",
        "pmlpcs_axis_y": "Y",
        "pmlpcs_axis_z": "Z",
        "pmlpc_heightaxis_x": "X",
        "pmlpc_heightaxis_y": "Y",
        "pmlpc_heightaxis_z": "Z",
    }
    require(raw in mapping, f"unknown cylinder axis: {raw}")
    return mapping[raw]


def _axis_project(axis: str, radial_x: float, radial_y: float, height: float) -> list[float]:
    if axis == "X":
        return [height, radial_x, radial_y]
    if axis == "Y":
        return [radial_x, height, radial_y]
    require(axis == "Z", f"unknown projected axis: {axis}")
    return [radial_x, radial_y, height]


def _cylinder_state(
    distributions: dict[str, list[float]],
    literals: dict[str, Any],
    contract: dict[str, Any],
    defaults: dict[str, Any],
    time: float,
    *,
    spin: bool,
) -> dict[str, Any]:
    radius = _scalar(distributions["startradius"])
    height = _scalar(distributions["startheight"])
    start = _vector(distributions["startlocation"], 3)
    if spin:
        base_angle = _scalar(distributions["startcylinderrot"])
        spin_angle = _scalar(distributions["spinangle"])
    else:
        require("distribution.startcylinderrot" in defaults
                and "distribution.spinangle" in defaults,
                "standard cylinder angle defaults are missing")
        base_angle = _scalar(defaults["distribution.startcylinderrot"])
        spin_angle = _scalar(defaults["distribution.spinangle"])
    angle = math.radians(base_angle + spin_angle * time)
    axis = _axis(literals, defaults, spin)
    execution_flag_paths = [
        path for path in contract["literals"]
        if path in {
            "badjustforworldspace", "benabled", "surfaceonly", "velocity",
            "positive_x", "positive_y", "positive_z",
            "negative_x", "negative_y", "negative_z",
        }
    ]
    flags = {
        path: bool(_resolved_literal(literals, path, defaults))
        for path in sorted(execution_flag_paths)
    }
    x_sign = -1.0 if flags.get("negative_x", False) else 1.0
    y_sign = -1.0 if flags.get("negative_y", False) else 1.0
    radial_x = radius * math.cos(angle) * x_sign
    radial_y = radius * math.sin(angle) * y_sign
    projected = _axis_project(axis, radial_x, radial_y, height)
    position = [start[index] + projected[index] for index in range(3)]
    velocity_scale = _scalar(distributions["velocityscale"])
    tangent = _axis_project(
        axis,
        -math.sin(angle) * velocity_scale * x_sign,
        math.cos(angle) * velocity_scale * y_sign,
        0.0,
    )
    velocity_enabled = flags.get("velocity", False)
    velocity = tangent if velocity_enabled else [0.0, 0.0, 0.0]
    return {
        "position": position,
        "velocity": velocity,
        "angleRadians": angle,
        "axis": axis,
        "flags": flags,
    }


def _current_field_map(adapter: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for row in adapter.get("currentRevisionFields", []):
        path = str(row["fieldPath"])
        require(path not in result, f"duplicate current field: {path}")
        result[path] = copy.deepcopy(row["value"])
    return result


def _dpm_component(value: float | None, constant: float, mode: str,
                   minimum_input: float, maximum_input: float,
                   minimum_output: float, maximum_output: float) -> float:
    if value is None:
        return constant
    if mode == "dpm_direct":
        return value
    if mode == "dpm_abs":
        value = abs(value)
    require(mode in ("dpm_normal", "dpm_abs"), f"unknown DPM mode: {mode}")
    if maximum_input == minimum_input:
        return minimum_output
    alpha = (value - minimum_input) / (maximum_input - minimum_input)
    alpha = max(0.0, min(1.0, alpha))
    return minimum_output + alpha * (maximum_output - minimum_output)


def evaluate_policy_sample(
    module: dict[str, Any],
    occurrence: dict[str, Any],
    policy: dict[str, Any],
    time: float,
    random_units: list[float],
    fixed_seed: int,
    point_light: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    contract = CLASS_CONTRACTS[module["exactSourceClass"]]
    variant = contract["variant"]
    binding = _variant_binding_for_policy(policy, variant)
    defaults = _default_map(binding)
    literals = _literal_map(module)
    distributions = {
        path: _evaluated_distribution(module, path, time, random_units)
        for path in contract["distributions"]
        if not (
            contract["family"] == "source.reconstructed.ef-vector-multiply.v1"
            and path in ("colorscaleoverlife", "startrotation")
        )
    }
    typed_inputs: dict[str, Any] = {
        "time": time,
        "fixedSeed": fixed_seed,
        "randomUnits": random_units,
        "sourceLiterals": _presence_wrapped_literals(literals, contract),
        "evaluatedDistributions": {
            path: distributions[path] for path in sorted(distributions)
        },
    }

    if contract["family"] == "source.reconstructed.seeded.v1":
        if variant == "SEEDED_COLOR":
            output = {
                "variant": variant,
                "color": _vector(distributions["startcolor"], 3),
                "alpha": _scalar(distributions["startalpha"]),
                "requiresLoopingNotification": bool(_resolved_literal(
                    literals, "brequiresloopingnotification", defaults
                )),
            }
        elif variant == "SEEDED_LIFETIME":
            output = {"variant": variant, "lifetime": _scalar(distributions["lifetime"])}
        elif variant == "SEEDED_LOCATION":
            output = {"variant": variant, "position": _vector(distributions["startlocation"], 3)}
        elif variant == "SEEDED_CYLINDER":
            state = _cylinder_state(
                distributions, literals, contract, defaults, time, spin=False
            )
            output = {"variant": variant, **state}
        elif variant == "SEEDED_MESH_ROTATION":
            output = {"variant": variant, "rotation": _vector(distributions["startrotation"], 3)}
        elif variant == "SEEDED_SIZE":
            output = {"variant": variant, "size": _vector(distributions["startsize"], 3)}
        elif variant == "SEEDED_VELOCITY":
            velocity = _vector(distributions["startvelocity"], 3)
            radial = _scalar(distributions["startvelocityradial"])
            position = _vector(defaults["fixedContext.inputPosition"], 3)
            length = math.sqrt(sum(item * item for item in position))
            radial_direction = [item / length for item in position]
            output = {
                "variant": variant,
                "velocity": [velocity[index] + radial * radial_direction[index] for index in range(3)],
                "baseVelocity": velocity,
            }
        else:
            raise ValueError(f"unknown seeded variant: {variant}")
    elif contract["family"] == "source.reconstructed.cylinder-spin.v1":
        output = {
            "variant": variant,
            **_cylinder_state(
                distributions, literals, contract, defaults, time, spin=True
            ),
        }
    elif contract["family"] == "source.reconstructed.ground.v1":
        adjust = _vector(distributions["adjustlocation"], 3)
        skip = _scalar(distributions["skiplocation"])
        position = _vector(defaults["fixedContext.inputPosition"], 3)
        ground_plane_z = float(defaults["fixedContext.groundPlaneZ"])
        skip_threshold = float(defaults["ground.skipThreshold"])
        query_applied = skip < skip_threshold
        if query_applied:
            position = [position[0] + adjust[0], position[1] + adjust[1], ground_plane_z + adjust[2]]
        typed_inputs.update({"inputPosition": _vector(defaults["fixedContext.inputPosition"], 3), "groundPlaneZ": ground_plane_z})
        output = {"variant": variant, "position": position, "queryApplied": query_applied, "skipLocation": skip}
    elif contract["family"] == "source.reconstructed.decal.v1":
        near_plane_value = _resolved_literal(literals, "nearplane", defaults)
        require(
            type(near_plane_value) is float
            and near_plane_value == EXPECTED_DECAL_NEAR_PLANE,
            f"Decal nearPlane literal changed: {module['moduleOccurrenceId']}",
        )
        near_plane = near_plane_value
        far_plane = float(defaults["decal.farPlane"])
        default_size = _vector(defaults["decal.defaultSize"], 2)
        blend_range = _vector(defaults["decal.blendRange"], 2)
        yaw_only = bool(defaults["decal.yawOnly"])
        supports_3d_draw_mode = bool(defaults["decal.supports3dDrawMode"])
        typed_inputs.update({
            "nearPlane": near_plane,
            "farPlane": far_plane,
            "defaultSize": default_size,
            "blendRange": blend_range,
            "yawOnly": yaw_only,
            "supports3dDrawMode": supports_3d_draw_mode,
        })
        output = {
            "variant": variant,
            "frustum": [near_plane, far_plane, default_size[0], default_size[1], blend_range[0], blend_range[1]],
            "yawOnly": yaw_only,
            "supports3dDrawMode": supports_3d_draw_mode,
        }
    elif contract["family"] == "source.reconstructed.light.v1":
        fields = {row["fieldPath"]: copy.deepcopy(row["value"]) for row in point_light["fields"]}
        require(set(fields) == {"brightness", "bcastcompositeshadow", "baffectcompositeshadowdirection", "radius", "falloffexponent", "lightcolor"}, "point-light field coverage changed")
        typed_inputs["pointLightFields"] = copy.deepcopy(fields)
        color = fields["lightcolor"]
        output = {
            "variant": variant,
            "brightness": float(fields["brightness"]),
            "radius": float(fields["radius"]),
            "falloffExponent": float(fields["falloffexponent"]),
            "linearColor": [float(color[key]) / 255.0 for key in ("r", "g", "b", "a")],
            "castCompositeShadow": bool(fields["bcastcompositeshadow"]),
            "affectCompositeShadowDirection": bool(fields["baffectcompositeshadowdirection"]),
        }
    elif contract["family"] == "source.reconstructed.velocity.v1":
        scale = _vector(distributions["veloverlife"], 3)
        velocity = _vector(defaults["fixedContext.inputVelocity"], 3)
        typed_inputs["inputVelocity"] = velocity
        output = {"variant": variant, "velocity": [velocity[index] * scale[index] for index in range(3)]}
    elif contract["family"] == "source.reconstructed.ef-vector-multiply.v1":
        custom_path = "colorscaleoverlife" if variant == "EF_COLOR_SCALE_PARAMETER_MULTIPLY" else "startrotation"
        adapter = _adapter(module, custom_path)
        fields = _current_field_map(adapter)
        constant = _vector(fields["constant"], 3)
        minimum_input = _vector(fields["mininput"], 3)
        maximum_input = _vector(fields["maxinput"], 3)
        minimum_output = _vector(fields["minoutput"], 3)
        maximum_output = _vector(fields["maxoutput"], 3)
        modes = [str(fields["parammodes"]), str(fields["parammodes[1]"]), str(fields["parammodes[2]"])]
        parameter_name = str(fields["parametername"])
        matches = [row for row in occurrence["actionCueParameterInputs"] if str(row["name"]).lower() == parameter_name.lower()]
        require(len(matches) <= 1, f"duplicate action parameter: {parameter_name}")
        parameter = _vector(matches[0]["value"], 3) if matches else None
        mapped = [
            _dpm_component(
                None if parameter is None else parameter[index], constant[index], modes[index],
                minimum_input[index], maximum_input[index], minimum_output[index], maximum_output[index],
            )
            for index in range(3)
        ]
        base = _vector(defaults["fixedContext.baseVector"], 3)
        typed_inputs["parameterFields"] = {
            "parameterName": parameter_name,
            "parameterInput": parameter,
            "constant": constant,
            "paramModes": modes,
            "minimumInput": minimum_input,
            "maximumInput": maximum_input,
            "minimumOutput": minimum_output,
            "maximumOutput": maximum_output,
            "baseVector": base,
        }
        output: dict[str, Any] = {
            "variant": variant,
            "mappedParameter": mapped,
            "multipliedVector": [base[index] * mapped[index] for index in range(3)],
        }
        if variant == "EF_COLOR_SCALE_PARAMETER_MULTIPLY":
            output["alphaScale"] = _scalar(distributions["alphascaleoverlife"])
    else:
        raise ValueError(f"unknown policy family: {contract['family']}")

    require(_finite_numeric_tree(typed_inputs), "non-finite typed policy input")
    require(_finite_numeric_tree(output), "non-finite reconstructed output")
    validate_typed_value(typed_inputs, binding["inputSchema"], variant + ".input")
    validate_typed_value(output, binding["outputSchema"], variant + ".output")
    return typed_inputs, output


def _finite_numeric_tree(value: Any) -> bool:
    if type(value) is float:
        return math.isfinite(value)
    if type(value) is int:
        return True
    if isinstance(value, list):
        return all(_finite_numeric_tree(item) for item in value)
    if isinstance(value, dict):
        return all(_finite_numeric_tree(item) for item in value.values())
    return value is None or isinstance(value, (str, bool))


def validate_typed_value(value: Any, schema: dict[str, Any], path: str) -> None:
    schema_type = schema.get("type")
    if schema_type == "OBJECT":
        require(isinstance(value, dict), f"typed object required: {path}")
        fields = schema.get("fields")
        require(isinstance(fields, dict)
                and schema.get("additionalFieldsAllowed") is False,
                f"object schema is not closed: {path}")
        require(set(value) == set(fields), f"typed object keys changed: {path}")
        for key, field_schema in fields.items():
            validate_typed_value(value[key], field_schema, path + "." + key)
        return
    if schema_type == "FLOAT64":
        require(type(value) is float and math.isfinite(value),
                f"finite FLOAT64 required: {path}")
        return
    if schema_type == "INT32":
        require(type(value) is int and -(1 << 31) <= value < (1 << 31),
                f"INT32 required: {path}")
        return
    if schema_type == "UINT8":
        require(type(value) is int and 0 <= value <= 255,
                f"UINT8 required: {path}")
        return
    if schema_type == "BOOLEAN":
        require(type(value) is bool, f"BOOLEAN required: {path}")
        return
    if schema_type == "STRING":
        require(type(value) is str, f"STRING required: {path}")
        return
    if schema_type == "ENUM":
        require(type(value) is str and value in schema.get("values", []),
                f"closed ENUM required: {path}")
        return
    if schema_type == "FLOAT64_VECTOR":
        require(isinstance(value, list) and len(value) == schema.get("length")
                and all(type(item) is float and math.isfinite(item) for item in value),
                f"exact finite FLOAT64 vector required: {path}")
        return
    if schema_type == "ENUM_VECTOR":
        require(isinstance(value, list) and len(value) == schema.get("length")
                and all(type(item) is str and item in schema.get("values", []) for item in value),
                f"exact closed enum vector required: {path}")
        return
    if schema_type == "OPTIONAL":
        if value is not None:
            validate_typed_value(value, schema["valueSchema"], path)
        return
    if schema_type == "PRESENCE_WRAPPED":
        require(isinstance(value, dict) and set(value) == {"present", "value"}
                and type(value["present"]) is bool,
                f"presence wrapper changed: {path}")
        if value["present"]:
            require(value["value"] is not None,
                    f"present literal has null value: {path}")
            validate_typed_value(value["value"], schema["valueSchema"], path + ".value")
        else:
            require(value["value"] is None,
                    f"absent literal carries executable value: {path}")
        return
    raise ValueError(f"unknown recursive schema type: {path} {schema_type}")


def _distribution_binding(module: dict[str, Any], property_path: str) -> dict[str, Any]:
    adapter = _adapter(module, property_path)
    descriptor = _payload_descriptor(module, property_path)
    result = {
        "propertyPath": property_path,
        "distributionId": adapter["distributionId"],
        "payloadDistributionId": adapter.get("payloadDistributionId", ""),
        "decision": adapter["decision"],
        "sourceFidelity": adapter.get("sourceFidelity", adapter.get("fieldProvenance", {}).get("rawFieldSourceFidelity", "MODULE_SOURCE_EVIDENCE")),
        "sourceEraIdentityPinned": adapter.get("sourceEraIdentityPinned") is True,
        "sourceEraIdentityFieldPresent": "sourceEraIdentityPinned" in adapter,
        "blockers": sorted(set(adapter.get("blockers", []))),
        "descriptorSha256": canonical_sha256(descriptor) if descriptor is not None else None,
        "currentRevisionFields": copy.deepcopy(adapter.get("currentRevisionFields", [])),
    }
    return result


def _property_consumption(module: dict[str, Any]) -> list[dict[str, Any]]:
    module_id = module["moduleOccurrenceId"]
    literal_ids = {row["literalId"] for row in module["typedPayload"]["literals"]}
    payload_distribution_ids = {
        row["payloadDistributionId"] for row in module["typedPayload"]["distributions"]
    }
    semantic_distribution_ids = {
        row["distributionId"] for row in module["distributionAdapters"]
    }
    result = []
    for source_property in module["properties"]:
        property_id = source_property["propertyId"]
        require(property_id.startswith(module_id + "::property:"),
                f"property owner changed: {property_id}")
        bound_literal_ids = list(source_property["payloadLiteralIds"])
        bound_payload_distribution_ids = list(source_property["payloadDistributionIds"])
        bound_semantic_distribution_ids = list(source_property["semanticDistributionIds"])
        require(set(bound_literal_ids).issubset(literal_ids)
                and set(bound_payload_distribution_ids).issubset(payload_distribution_ids)
                and set(bound_semantic_distribution_ids).issubset(semantic_distribution_ids),
                f"property binding escaped module ownership: {property_id}")
        if source_property["decision"] == "VERIFIED_IRRELEVANT":
            require(source_property.get("irrelevanceOracleId"),
                    f"irrelevant property lacks oracle: {property_id}")
            decision = "PRESERVED_VERIFIED_IRRELEVANT"
            semantic_role = "UPSTREAM_IRRELEVANCE_ORACLE"
            output_dependency = False
        else:
            require(
                bound_literal_ids
                or bound_payload_distribution_ids
                or bound_semantic_distribution_ids,
                f"execution property has no typed binding: {property_id}",
            )
            decision = "RECONSTRUCTED_POLICY_INPUT_CONSUMED"
            property_path = source_property["propertyPath"]
            if property_path == "randomseedinfo":
                semantic_role = "FIXED_SEED_AND_RANDOM_STREAM_INPUT"
            elif bound_semantic_distribution_ids:
                semantic_role = "EVALUATED_DISTRIBUTION_AND_VARIANT_OUTPUT"
            elif property_path.endswith("objectpath") or property_path == "pointlightcomponent":
                semantic_role = "EXACT_TYPED_IDENTITY_BINDING_SELECTS_VARIANT_INPUT"
            else:
                semantic_role = "TYPED_LITERAL_AND_VARIANT_OUTPUT_OR_CONTEXT"
            output_dependency = True
        result.append({
            "propertyId": property_id,
            "propertyPath": source_property["propertyPath"],
            "sourceDecision": source_property["decision"],
            "sourceFidelity": source_property["sourceFidelity"],
            "capabilityConsumptionDecision": decision,
            "semanticRole": semantic_role,
            "outputDependencyRequired": output_dependency,
            "irrelevanceOracleId": source_property.get("irrelevanceOracleId", ""),
            "payloadLiteralIds": bound_literal_ids,
            "payloadDistributionIds": bound_payload_distribution_ids,
            "semanticDistributionIds": bound_semantic_distribution_ids,
            "preservedBlockers": sorted(set(source_property.get("blockers", []))),
        })
    require(len(result) == len(module["properties"])
            and len({row["propertyId"] for row in result}) == len(result),
            f"property consumption coverage changed: {module_id}")
    consumed_literal_ids = [
        identifier for row in result for identifier in row["payloadLiteralIds"]
    ]
    consumed_payload_distribution_ids = [
        identifier for row in result for identifier in row["payloadDistributionIds"]
    ]
    consumed_semantic_distribution_ids = [
        identifier for row in result for identifier in row["semanticDistributionIds"]
    ]
    require(len(consumed_literal_ids) == len(set(consumed_literal_ids))
            and set(consumed_literal_ids) == literal_ids,
            f"literal property ownership is not exact: {module_id}")
    require(len(consumed_payload_distribution_ids)
            == len(set(consumed_payload_distribution_ids))
            and set(consumed_payload_distribution_ids) == payload_distribution_ids,
            f"payload distribution property ownership is not exact: {module_id}")
    require(len(consumed_semantic_distribution_ids)
            == len(set(consumed_semantic_distribution_ids))
            and set(consumed_semantic_distribution_ids) == semantic_distribution_ids,
            f"semantic distribution property ownership is not exact: {module_id}")
    return result


def _source_rows(source: dict[str, Any]) -> list[tuple[dict[str, Any], dict[str, Any]]]:
    rows: list[tuple[dict[str, Any], dict[str, Any]]] = []
    for occurrence in source["occurrences"]:
        for module in occurrence["modules"]:
            if module["decision"] == "BLOCKED":
                rows.append((occurrence, module))
    rows.sort(key=lambda item: item[1]["moduleOccurrenceId"])
    return rows


def build_occurrence_rows(
    source: dict[str, Any],
    policies: list[dict[str, Any]],
    custom: dict[str, Any] | None = None,
    acquisition: dict[str, Any] | None = None,
) -> list[dict[str, Any]]:
    policy_by_id = {row["policyFamilyId"]: row for row in policies}
    result: list[dict[str, Any]] = []
    observed_literals: dict[str, set[str]] = {
        source_class: set() for source_class in CLASS_CONTRACTS
    }
    observed_distributions: dict[str, set[str]] = {
        source_class: set() for source_class in CLASS_CONTRACTS
    }
    custom_blockers: dict[str, set[str]] = {}
    if custom is not None:
        for owner in custom["moduleBlockerOwnership"]:
            custom_blockers.setdefault(owner["moduleOccurrenceId"], set()).update(
                owner["remainingBlockers"]
            )
        for owner in custom["distributionBlockerOwnership"]:
            custom_blockers.setdefault(owner["moduleOccurrenceId"], set()).update(
                owner["remainingBlockers"]
            )
    acquisition_blockers = set()
    if acquisition is not None:
        acquisition_blockers.update(acquisition["productAdmission"]["blockers"])
    for occurrence, module in _source_rows(source):
        source_class = module["exactSourceClass"]
        require(source_class in CLASS_CONTRACTS,
                f"unknown reconstructed source class: {source_class}")
        contract = CLASS_CONTRACTS[source_class]
        literal_paths = tuple(sorted(row["propertyPath"] for row in module["typedPayload"]["literals"]))
        distribution_paths = tuple(sorted(row["descriptor"]["propertyPath"] for row in module["typedPayload"]["distributions"]))
        require(set(literal_paths).issubset(contract["literals"]),
                f"literal coverage changed: {module['moduleOccurrenceId']}")
        require(set(distribution_paths).issubset(contract["distributions"]),
                f"distribution coverage changed: {module['moduleOccurrenceId']}")
        observed_literals[source_class].update(literal_paths)
        observed_distributions[source_class].update(distribution_paths)
        policy = policy_by_id[contract["family"]]
        binding = _variant_binding_for_policy(policy, contract["variant"])
        require(binding["exactSourceClass"] == source_class,
                f"variant class owner changed: {contract['variant']}")
        defaults = _default_map(binding)
        seed, seed_source = _seed(module, defaults)
        samples = []
        for sample_index, time in enumerate(SAMPLE_TIMES):
            units = list(random_stream_units(seed, sample_index * 4, 4))
            typed_inputs, output = evaluate_policy_sample(
                module, occurrence, policy, time, units, seed,
                source["pointLightAdapter"]
            )
            samples.append({
                "sampleId": f"{module['moduleOccurrenceId']}::reconstructed:{sample_index:03d}",
                "time": time,
                "fixedSeed": seed,
                "fixedSeedSource": seed_source,
                "randomUnits": units,
                "typedInputs": typed_inputs,
                "typedInputSha256": canonical_sha256(typed_inputs),
                "output": output,
                "outputSha256": canonical_sha256(output),
                "absoluteTolerance": ABSOLUTE_TOLERANCE,
                "relativeTolerance": RELATIVE_TOLERANCE,
            })
        upstream_blockers = set(module.get("blockers", []))
        for adapter in module["distributionAdapters"]:
            upstream_blockers.update(adapter.get("blockers", []))
        upstream_blockers.update(custom_blockers.get(module["moduleOccurrenceId"], set()))
        upstream_blockers.update(acquisition_blockers)
        blockers = sorted(upstream_blockers | set(EVIDENCE_BLOCKERS))
        result.append({
            "moduleOccurrenceId": module["moduleOccurrenceId"],
            "occurrenceCompositeId": occurrence["occurrenceCompositeId"],
            "sourceOccurrenceId": occurrence["sourceOccurrenceId"],
            "exactSourceClass": source_class,
            "sourceObjectId": module["sourceObjectId"],
            "sourceRecordSha256": module["sourceRecordSha256"],
            "typedPayloadSha256": module["typedPayload"]["payloadSha256"],
            "policyFamilyId": contract["family"],
            "variant": contract["variant"],
            "implementationId": policy["implementationId"],
            "implementationVersion": policy["implementationVersion"],
            "implementationSha256": policy["implementationSha256"],
            "familySemanticImplementationSha256": policy["familySemanticImplementationSha256"],
            "variantInputSchemaSha256": canonical_sha256(binding["inputSchema"]),
            "variantOutputSchemaSha256": canonical_sha256(binding["outputSchema"]),
            "explicitDefaultsSha256": canonical_sha256(binding["explicitDefaults"]),
            "sourceLiteralBindings": copy.deepcopy(module["typedPayload"]["literals"]),
            "distributionBindings": [
                _distribution_binding(module, path)
                for path in contract["distributions"]
            ],
            "propertyConsumption": _property_consumption(module),
            "seedBinding": copy.deepcopy(module.get("seed")),
            "actionCueParameterInputs": copy.deepcopy(occurrence["actionCueParameterInputs"]),
            "numericSamples": samples,
            "capabilityDecision": "READY_FOR_RECONSTRUCTED_REVIEW",
            "sourceEvidenceFidelity": "CURRENT_REVISION_CROSS_REVISION_EVIDENCE",
            "sourceExact": False,
            "currentEvidencePromotedToSourceExact": False,
            "preservedEvidenceBlockers": blockers,
            "runtimeExecutionAdmission": False,
            "productAdmission": False,
        })
    for source_class, contract in CLASS_CONTRACTS.items():
        require(observed_literals[source_class] == set(contract["literals"]),
                f"class literal union changed: {source_class}")
        require(observed_distributions[source_class] == set(contract["distributions"]),
                f"class distribution union changed: {source_class}")
    if custom is not None:
        require(set(custom_blockers) == {row["moduleOccurrenceId"] for row in result},
                "custom-handler blocker ownership coverage changed")
    global_ids: dict[str, list[str]] = {
        "moduleOccurrenceId": [row["moduleOccurrenceId"] for row in result],
        "propertyId": [item["propertyId"] for row in result for item in row["propertyConsumption"]],
        "literalId": [item["literalId"] for row in result for item in row["sourceLiteralBindings"]],
        "distributionId": [item["distributionId"] for row in result for item in row["distributionBindings"]],
        "payloadDistributionId": [item["payloadDistributionId"] for row in result for item in row["distributionBindings"]],
        "sampleId": [item["sampleId"] for row in result for item in row["numericSamples"]],
    }
    for id_kind, identifiers in global_ids.items():
        require(all(identifier for identifier in identifiers)
                and len(identifiers) == len(set(identifiers)),
                f"global {id_kind} ownership is not unique")
    for row in result:
        module_id = row["moduleOccurrenceId"]
        require(all(item["distributionId"].startswith(module_id + "::distribution:")
                    and item["payloadDistributionId"].startswith(module_id + "::payload-distribution:")
                    for item in row["distributionBindings"]),
                f"distribution ownership escaped occurrence: {module_id}")
    return result


def _artifact_identity(root: Path, relative_path: str, value: dict[str, Any]) -> dict[str, Any]:
    pin = PINNED_INPUTS[relative_path]
    path = root / relative_path
    require(tracked_json_sha256(path) == pin["canonicalTextSha256"],
            f"pinned canonical tracked JSON changed: {relative_path}")
    require(value.get("receiptSha256") == pin["selfSha256"],
            f"pinned self identity changed: {relative_path}")
    return {
        "path": relative_path,
        "hashDomain": "TRACKED_JSON_EOL_CANONICAL_LF_NO_BOM",
        "canonicalTextSha256": pin["canonicalTextSha256"],
        "selfSha256": pin["selfSha256"],
    }


def build_receipt(
    root: Path,
    source: dict[str, Any],
    custom: dict[str, Any],
    acquisition: dict[str, Any],
    source_receipt: dict[str, Any],
) -> dict[str, Any]:
    validate_source_execution_receipt(source)
    validate_custom_handler_receipt(custom, source)
    validate_acquisition_receipt(
        acquisition, source, custom, source_receipt, root, compare_rebuilt=True
    )
    policies = build_family_policies(root, source)
    occurrences = build_occurrence_rows(source, policies, custom, acquisition)
    family_counts = Counter(row["policyFamilyId"] for row in occurrences)
    require(dict(sorted(family_counts.items())) == EXPECTED_FAMILY_COUNTS,
            "reconstructed family denominator changed")
    require(len(occurrences) == 29
            and len({row["moduleOccurrenceId"] for row in occurrences}) == 29,
            "reconstructed occurrence denominator changed")
    property_rows = [item for row in occurrences for item in row["propertyConsumption"]]
    distribution_rows = [item for row in occurrences for item in row["distributionBindings"]]
    consumed_property_count = sum(
        row["capabilityConsumptionDecision"] == "RECONSTRUCTED_POLICY_INPUT_CONSUMED"
        for row in property_rows
    )
    irrelevant_property_count = sum(
        row["capabilityConsumptionDecision"] == "PRESERVED_VERIFIED_IRRELEVANT"
        for row in property_rows
    )
    source_era_pinned_count = sum(
        row["sourceEraIdentityPinned"] is True for row in distribution_rows
    )
    source_era_field_missing_count = sum(
        row["sourceEraIdentityFieldPresent"] is False for row in distribution_rows
    )
    dependencies = _execution_dependencies(root)

    result = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "SOURCE_RECONSTRUCTED_CAPABILITY_EVIDENCE_ONLY",
        "policy": {
            "name": POLICY_NAME,
            "approvalScope": "CAPABILITY_AND_NUMERIC_ORACLE_REVIEW_ONLY",
            "sourceExactClaimAllowed": False,
            "currentEvidencePromotionAllowed": False,
            "runtimeExecutionAdmissionAllowed": False,
            "productAdmissionAllowed": False,
        },
        "frozenSourceInputs": [
            _artifact_identity(root, SOURCE_PATH, source),
            _artifact_identity(root, CUSTOM_PATH, custom),
            _artifact_identity(root, ACQUISITION_PATH, acquisition),
        ],
        "toolIdentity": {
            "path": "Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py",
            "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT_NO_BOM",
            "canonicalTextSha256": canonical_text_sha256(Path(__file__).resolve()),
        },
        "executionDependencies": dependencies,
        "executionDependencySetSha256": canonical_sha256(dependencies),
        "familyPolicies": policies,
        "occurrences": occurrences,
        "summary": {
            "policyFamilyCount": 7,
            "moduleOccurrenceCount": 29,
            "familyOccurrenceCounts": dict(sorted(family_counts.items())),
            "readyForReconstructedReviewCount": 29,
            "numericSampleCount": 87,
            "numericSampleMissingReadyCount": 0,
            "sourcePropertyRowCount": len(property_rows),
            "policyInputConsumedPropertyCount": consumed_property_count,
            "preservedIrrelevantPropertyCount": irrelevant_property_count,
            "unconsumedPropertyCount": 0,
            "distributionBindingCount": len(distribution_rows),
            "sourceEraIdentityPinnedDistributionCount": source_era_pinned_count,
            "sourceEraIdentityUnpinnedDistributionCount": len(distribution_rows) - source_era_pinned_count,
            "sourceEraIdentityFieldMissingDistributionCount": source_era_field_missing_count,
            "globalDuplicateIdCount": 0,
            "unknownRowCount": 0,
            "ownerlessRowCount": 0,
            "genericFallbackCount": 0,
            "sourceExactCount": 0,
            "currentEvidencePromotedCount": 0,
            "runtimeExecutionAdmissionCount": 0,
            "productAdmissionCount": 0,
        },
        "blockerUnion": sorted({blocker for row in occurrences for blocker in row["preservedEvidenceBlockers"]}),
        "runtimeExecutionAdmission": {
            "allowed": False,
            "decision": "BLOCKED_CAPABILITY_EVIDENCE_AWAITS_INDEPENDENT_REVIEW_AND_RUNTIME_CONSUMPTION",
        },
        "productAdmission": {
            "allowed": False,
            "decision": "BLOCKED_FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED",
        },
    }
    result["receiptSha256"] = canonical_sha256(result)
    _validate_receipt(
        result, root, source, custom, acquisition, source_receipt,
        compare_rebuilt=False,
    )
    return result


def _validate_receipt(
    receipt: dict[str, Any],
    root: Path,
    source: dict[str, Any],
    custom: dict[str, Any],
    acquisition: dict[str, Any],
    source_receipt: dict[str, Any],
    *,
    compare_rebuilt: bool = True,
) -> None:
    require(receipt.get("schema") == SCHEMA, "capability schema changed")
    require(type(receipt.get("formatVersion")) is int
            and receipt["formatVersion"] == FORMAT_VERSION,
            "capability version changed")
    require(receipt.get("characterClass") == "ARTIST"
            and type(receipt.get("skillId")) is int
            and receipt["skillId"] == 31470
            and receipt.get("inputSlot") == "F",
            "capability root identity changed")
    unsigned = copy.deepcopy(receipt)
    self_hash = unsigned.pop("receiptSha256", "")
    require(len(self_hash) == 64 and canonical_sha256(unsigned) == self_hash,
            "capability self hash changed")
    policy_gate = receipt.get("policy") or {}
    require(policy_gate.get("name") == POLICY_NAME
            and policy_gate.get("sourceExactClaimAllowed") is False
            and policy_gate.get("currentEvidencePromotionAllowed") is False
            and policy_gate.get("runtimeExecutionAdmissionAllowed") is False
            and policy_gate.get("productAdmissionAllowed") is False,
            "capability policy gate changed")

    expected_inputs = [
        _artifact_identity(root, SOURCE_PATH, source),
        _artifact_identity(root, CUSTOM_PATH, custom),
        _artifact_identity(root, ACQUISITION_PATH, acquisition),
    ]
    require(receipt.get("frozenSourceInputs") == expected_inputs,
            "capability frozen Source input identity changed")
    expected_tool = {
        "path": "Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py",
        "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT_NO_BOM",
        "canonicalTextSha256": canonical_text_sha256(Path(__file__).resolve()),
    }
    require(receipt.get("toolIdentity") == expected_tool,
            "capability tool identity changed")

    expected_dependencies = _execution_dependencies(root)
    require(receipt.get("executionDependencies") == expected_dependencies
            and receipt.get("executionDependencySetSha256")
            == canonical_sha256(expected_dependencies),
            "execution dependency identity changed")
    expected_policies = build_family_policies(root, source)
    require(json_bytes(receipt.get("familyPolicies")) == json_bytes(expected_policies),
            "capability family policy changed")
    for policy in receipt["familyPolicies"]:
        implementation = {
            "implementationId": policy["implementationId"],
            "implementationVersion": policy["implementationVersion"],
            "familySemanticImplementationSha256": policy["familySemanticImplementationSha256"],
            "semanticContract": copy.deepcopy(policy["semanticContract"]),
        }
        require(canonical_sha256(implementation) == policy["implementationSha256"],
                f"implementation hash changed: {policy['policyFamilyId']}")
        require(canonical_sha256(policy["semanticContract"])
                == policy["familySemanticImplementationSha256"],
                f"family semantic implementation hash changed: {policy['policyFamilyId']}")
        require(policy["executionDependencies"] == expected_dependencies
                and policy["semanticContract"]["executionDependencySetSha256"]
                == canonical_sha256(expected_dependencies)
                and policy["semanticContract"]["genericFallbackAllowed"] is False,
                "family execution dependency or fallback gate changed")
        sample_contract = policy["sampleContract"]
        require(sample_contract["times"] == list(SAMPLE_TIMES)
                and type(sample_contract["absoluteTolerance"]) is float
                and math.isfinite(sample_contract["absoluteTolerance"])
                and sample_contract["absoluteTolerance"] > 0.0
                and type(sample_contract["relativeTolerance"]) is float
                and math.isfinite(sample_contract["relativeTolerance"])
                and sample_contract["relativeTolerance"] > 0.0,
                "numeric tolerance contract changed")
        require(policy["sourceExact"] is False
                and policy["runtimeExecutionAdmission"] is False
                and policy["productAdmission"] is False,
                "family fidelity/admission changed")

    expected_rows = build_occurrence_rows(source, expected_policies, custom, acquisition)
    occurrences = receipt["occurrences"]
    require(len(occurrences) == 29
            and len({row["moduleOccurrenceId"] for row in occurrences}) == 29,
            "capability occurrence identity changed")
    counts = Counter(row["policyFamilyId"] for row in occurrences)
    require(dict(sorted(counts.items())) == EXPECTED_FAMILY_COUNTS,
            "capability family counts changed")
    policy_by_id = {row["policyFamilyId"]: row for row in receipt["familyPolicies"]}
    source_by_id = {
        module["moduleOccurrenceId"]: (occurrence, module)
        for occurrence, module in _source_rows(source)
    }
    global_ids: dict[str, list[str]] = {
        "moduleOccurrenceId": [row["moduleOccurrenceId"] for row in occurrences],
        "propertyId": [item["propertyId"] for row in occurrences for item in row["propertyConsumption"]],
        "literalId": [item["literalId"] for row in occurrences for item in row["sourceLiteralBindings"]],
        "distributionId": [item["distributionId"] for row in occurrences for item in row["distributionBindings"]],
        "payloadDistributionId": [item["payloadDistributionId"] for row in occurrences for item in row["distributionBindings"]],
        "sampleId": [item["sampleId"] for row in occurrences for item in row["numericSamples"]],
    }
    for id_kind, identifiers in global_ids.items():
        require(all(type(identifier) is str and identifier for identifier in identifiers)
                and len(identifiers) == len(set(identifiers)),
                f"public validator rejected duplicate/foreign {id_kind}")
    for row in occurrences:
        module_id = row["moduleOccurrenceId"]
        require(module_id in source_by_id, f"foreign module occurrence id: {module_id}")
        source_occurrence, source_module = source_by_id[module_id]
        require(row["occurrenceCompositeId"] == source_occurrence["occurrenceCompositeId"]
                and row["sourceOccurrenceId"] == source_occurrence["sourceOccurrenceId"]
                and row["exactSourceClass"] == source_module["exactSourceClass"]
                and row["sourceObjectId"] == source_module["sourceObjectId"]
                and row["sourceRecordSha256"] == source_module["sourceRecordSha256"]
                and row["typedPayloadSha256"] == source_module["typedPayload"]["payloadSha256"],
                f"occurrence/source ownership changed: {module_id}")
        require(row["capabilityDecision"] == "READY_FOR_RECONSTRUCTED_REVIEW"
                and row["sourceEvidenceFidelity"] == "CURRENT_REVISION_CROSS_REVISION_EVIDENCE"
                and row["sourceExact"] is False
                and row["currentEvidencePromotedToSourceExact"] is False
                and row["runtimeExecutionAdmission"] is False
                and row["productAdmission"] is False,
                f"occurrence fidelity/admission changed: {row['moduleOccurrenceId']}")
        require(set(EVIDENCE_BLOCKERS).issubset(row["preservedEvidenceBlockers"]),
                f"evidence blocker lost: {row['moduleOccurrenceId']}")
        require(len(row["numericSamples"]) == 3
                and [sample["time"] for sample in row["numericSamples"]] == list(SAMPLE_TIMES),
                f"numeric sample coverage changed: {row['moduleOccurrenceId']}")
        policy = policy_by_id[row["policyFamilyId"]]
        binding = _variant_binding_for_policy(policy, row["variant"])
        require(binding["exactSourceClass"] == row["exactSourceClass"]
                and row["implementationId"] == policy["implementationId"]
                and row["implementationVersion"] == policy["implementationVersion"]
                and row["implementationSha256"] == policy["implementationSha256"]
                and row["familySemanticImplementationSha256"]
                == policy["familySemanticImplementationSha256"]
                and row["variantInputSchemaSha256"] == canonical_sha256(binding["inputSchema"])
                and row["variantOutputSchemaSha256"] == canonical_sha256(binding["outputSchema"])
                and row["explicitDefaultsSha256"] == canonical_sha256(binding["explicitDefaults"]),
                f"variant implementation ownership changed: {module_id}")
        source_property_ids = {item["propertyId"] for item in source_module["properties"]}
        property_rows = row["propertyConsumption"]
        require({item["propertyId"] for item in property_rows} == source_property_ids
                and all(item["propertyId"].startswith(module_id + "::property:")
                        and item["capabilityConsumptionDecision"] in (
                            "RECONSTRUCTED_POLICY_INPUT_CONSUMED",
                            "PRESERVED_VERIFIED_IRRELEVANT",
                        )
                        and (
                            item["capabilityConsumptionDecision"]
                            != "PRESERVED_VERIFIED_IRRELEVANT"
                            or bool(item["irrelevanceOracleId"])
                        )
                        for item in property_rows),
                f"property consumption ownership changed: {module_id}")
        source_distribution_ids = {
            item["distributionId"] for item in source_module["distributionAdapters"]
        }
        source_payload_distribution_ids = {
            item["payloadDistributionId"]
            for item in source_module["typedPayload"]["distributions"]
        }
        require({item["distributionId"] for item in row["distributionBindings"]}
                == source_distribution_ids
                and {item["payloadDistributionId"] for item in row["distributionBindings"]}
                == source_payload_distribution_ids
                and all(item["distributionId"] in source_distribution_ids
                    and item["payloadDistributionId"] in source_payload_distribution_ids
                    and item["distributionId"].startswith(module_id + "::distribution:")
                    and item["payloadDistributionId"].startswith(module_id + "::payload-distribution:")
                    and type(item["sourceEraIdentityPinned"]) is bool
                    and item["sourceEraIdentityPinned"] is False
                    and type(item["sourceEraIdentityFieldPresent"]) is bool
                    for item in row["distributionBindings"]),
                f"distribution ownership/fidelity changed: {module_id}")
        for sample in row["numericSamples"]:
            require(_finite_numeric_tree(sample["typedInputs"])
                    and _finite_numeric_tree(sample["output"])
                    and canonical_sha256(sample["typedInputs"]) == sample["typedInputSha256"]
                    and canonical_sha256(sample["output"]) == sample["outputSha256"],
                    f"numeric sample integrity changed: {sample['sampleId']}")
            validate_typed_value(
                sample["typedInputs"], binding["inputSchema"],
                sample["sampleId"] + ".input",
            )
            validate_typed_value(
                sample["output"], binding["outputSchema"],
                sample["sampleId"] + ".output",
            )

    require(json_bytes(receipt.get("occurrences")) == json_bytes(expected_rows),
            "capability occurrence rows differ from deterministic source projection")

    property_rows = [item for row in occurrences for item in row["propertyConsumption"]]
    distribution_rows = [item for row in occurrences for item in row["distributionBindings"]]
    expected_summary = {
        "policyFamilyCount": 7,
        "moduleOccurrenceCount": 29,
        "familyOccurrenceCounts": EXPECTED_FAMILY_COUNTS,
        "readyForReconstructedReviewCount": 29,
        "numericSampleCount": 87,
        "numericSampleMissingReadyCount": 0,
        "sourcePropertyRowCount": len(property_rows),
        "policyInputConsumedPropertyCount": sum(
            item["capabilityConsumptionDecision"] == "RECONSTRUCTED_POLICY_INPUT_CONSUMED"
            for item in property_rows
        ),
        "preservedIrrelevantPropertyCount": sum(
            item["capabilityConsumptionDecision"] == "PRESERVED_VERIFIED_IRRELEVANT"
            for item in property_rows
        ),
        "unconsumedPropertyCount": 0,
        "distributionBindingCount": len(distribution_rows),
        "sourceEraIdentityPinnedDistributionCount": 0,
        "sourceEraIdentityUnpinnedDistributionCount": len(distribution_rows),
        "sourceEraIdentityFieldMissingDistributionCount": sum(
            item["sourceEraIdentityFieldPresent"] is False
            for item in distribution_rows
        ),
        "globalDuplicateIdCount": 0,
        "unknownRowCount": 0,
        "ownerlessRowCount": 0,
        "genericFallbackCount": 0,
        "sourceExactCount": 0,
        "currentEvidencePromotedCount": 0,
        "runtimeExecutionAdmissionCount": 0,
        "productAdmissionCount": 0,
    }
    require(receipt.get("summary") == expected_summary, "capability summary changed")
    require(receipt.get("blockerUnion")
            == sorted({blocker for row in occurrences for blocker in row["preservedEvidenceBlockers"]}),
            "capability blocker union changed")
    require(receipt.get("runtimeExecutionAdmission", {}).get("allowed") is False
            and receipt.get("productAdmission", {}).get("allowed") is False,
            "capability granted execution or Product admission")

    if compare_rebuilt:
        expected = build_receipt(root, source, custom, acquisition, source_receipt)
        require(json_bytes(receipt) == json_bytes(expected),
                "capability receipt differs from deterministic reconstruction")


def validate_receipt(
    receipt: dict[str, Any],
    root: Path,
    source: dict[str, Any],
    custom: dict[str, Any],
    acquisition: dict[str, Any],
    source_receipt: dict[str, Any],
) -> None:
    _validate_receipt(
        receipt, root, source, custom, acquisition, source_receipt,
        compare_rebuilt=True,
    )


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=root / SOURCE_PATH)
    parser.add_argument("--custom", type=Path, default=root / CUSTOM_PATH)
    parser.add_argument("--acquisition", type=Path, default=root / ACQUISITION_PATH)
    parser.add_argument("--source-receipt", type=Path, default=root / SOURCE_RECEIPT_PATH)
    parser.add_argument("--output", type=Path, default=root / OUTPUT_PATH)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    source = load_strict_json_object(args.source)
    custom = load_strict_json_object(args.custom)
    acquisition = load_strict_json_object(args.acquisition)
    source_receipt = load_strict_json_object(args.source_receipt)
    result = build_receipt(root, source, custom, acquisition, source_receipt)
    if args.check:
        current = load_strict_json_object(args.output)
        validate_receipt(current, root, source, custom, acquisition, source_receipt)
        require(json_bytes(current) == json_bytes(result), "capability receipt is stale")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_suffix(args.output.suffix + ".tmp")
        temporary.write_bytes(json_bytes(result))
        temporary.replace(args.output)
    print(
        "Artist F reconstructed Source capability: "
        f"families={result['summary']['policyFamilyCount']} "
        f"occurrences={result['summary']['moduleOccurrenceCount']} "
        f"samples={result['summary']['numericSampleCount']} "
        "unknown=0 ownerless=0 genericFallback=0 execution=false product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
