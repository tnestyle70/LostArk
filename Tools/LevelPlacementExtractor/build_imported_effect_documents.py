#!/usr/bin/env python3
"""Convert one normalized UE3 Cascade graph into a loadable Effect document.

The conversion is intentionally conservative.  It emits values that the current
v7 runtime can execute and records every approximation or unsupported source
module in a separate receipt.  The receipt, not a filename guess, is the audit
trail from an original Cascade emitter to an authored Element.
"""

from __future__ import annotations

import argparse
import copy
import json
import math
import re
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable


SOURCE_UNITS_TO_RUNTIME = 0.01
PROCEDURAL_MATERIAL_BASE_FALLBACK = (
    "fx_tex_00.fx_a_blankwhite_01",
    "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_blankwhite_01.dds",
)
MAX_ELEMENTS = 2048
MAX_DOCUMENT_PARTICLES = 8192
MAX_PARTICLES_PER_IMPORTED_ELEMENT = 64
SCREEN_POST_RUNTIME_PROFILES = {
    "fx_post.fx_par.par_j_rgbnoise_01": (
        "screen.rgb-noise.reconstructed.v1", "RGB_NOISE"
    ),
    "fx_post.fx_par.par_j_zoomblur_01": (
        "screen.zoom-blur.reconstructed.v1", "ZOOM_BLUR_J"
    ),
    "fx_post.fx_par.par_c_zoomblur_02": (
        "screen.zoom-blur.reconstructed.v1", "ZOOM_BLUR_C"
    ),
    "fx_post.fx_par.par_m_filmnoise_zaxis_01": (
        "screen.film-noise.reconstructed.v1", "FILM_NOISE"
    ),
}


def ue3_centimeters_to_client(value: Iterable[float]) -> list[float]:
    """Convert one UE3 spatial vector into the Client meter basis exactly once."""
    x, y, z = (float(component) for component in value)
    return [
        x * SOURCE_UNITS_TO_RUNTIME,
        z * SOURCE_UNITS_TO_RUNTIME,
        -y * SOURCE_UNITS_TO_RUNTIME,
    ]


def ue3_centimeter_range_to_client(
    minimum: Iterable[float], maximum: Iterable[float]
) -> tuple[list[float], list[float]]:
    """Convert an axis-aligned UE3 range without inverting the Client Z bounds."""
    min_x, min_y, min_z = (float(component) for component in minimum)
    max_x, max_y, max_z = (float(component) for component in maximum)
    return (
        [
            min_x * SOURCE_UNITS_TO_RUNTIME,
            min_z * SOURCE_UNITS_TO_RUNTIME,
            -max_y * SOURCE_UNITS_TO_RUNTIME,
        ],
        [
            max_x * SOURCE_UNITS_TO_RUNTIME,
            max_z * SOURCE_UNITS_TO_RUNTIME,
            -min_y * SOURCE_UNITS_TO_RUNTIME,
        ],
    )
REPRESENTED_DETAIL_MODULE_CLASSES = {
    "particlemodulerequired",
    "particlemodulespawn",
    "particlemodulelifetime",
    "particlemodulevelocity",
    "particlemoduleacceleration",
    "particlemodulesize",
    "particlemodulesize_seeded",
    "particlemodulesizemultiplylife",
    "particlemodulecolor",
    "particlemodulecolor_seeded",
    "particlemodulecoloroverlife",
    "particlemodulecolorscaleoverlife",
    "particlemodulelocation",
    "particlemodulelocation_seeded",
    "particlemodulesubuv",
    "particlemoduletypedatamesh",
    "efparticlemoduletypedatadecal",
}


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def folded(value: Any) -> str:
    return str(value or "").casefold()


def base_property_name(value: str) -> str:
    return value.split("[", 1)[0].casefold()


def unwrap(value: Any, default: Any = None) -> Any:
    if isinstance(value, dict) and "type" in value and "value" in value:
        return value["value"]
    return default if value is None else value


def prop(properties: dict[str, Any], name: str, default: Any = None) -> Any:
    return unwrap(properties.get(name.casefold()), default)


def finite_number(value: Any) -> float | None:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return None
    result = float(value)
    return result if math.isfinite(result) else None


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def safe_slug(value: str, maximum: int = 96) -> str:
    slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_.-").casefold()
    return (slug or "element")[:maximum]


def unique_id(base: str, used: set[str]) -> str:
    candidate = safe_slug(base, 120)
    suffix = 2
    while candidate in used:
        tail = f"_{suffix}"
        candidate = safe_slug(base, 120 - len(tail)) + tail
        suffix += 1
    used.add(candidate)
    return candidate


def vector_value(value: Any) -> list[float] | None:
    value = unwrap(value)
    if not isinstance(value, dict):
        return None
    result = [finite_number(value.get(axis)) for axis in ("x", "y", "z")]
    if any(component is None for component in result):
        return None
    return [float(component) for component in result if component is not None]


@dataclass
class SourceObject:
    key: str
    source_id: str
    class_name: str
    object_path: str
    properties: dict[str, Any]
    references: list[tuple[str, str]] = field(default_factory=list)
    reference_paths: list[tuple[str, str]] = field(default_factory=list)


class SourceIndex:
    def __init__(self, graph: dict[str, Any], closure: dict[str, Any]) -> None:
        self.objects: dict[str, SourceObject] = {}
        self.by_source_id: dict[str, SourceObject] = {}
        self.outgoing: dict[str, list[dict[str, Any]]] = defaultdict(list)
        for edge in graph.get("edges", []):
            self.outgoing[str(edge["sourceNodeId"])].append(edge)
        for rows in self.outgoing.values():
            rows.sort(key=lambda row: int(row.get("referenceIndex", 0)))

        for node in graph.get("nodes", []):
            package = str(node.get("package") or "")
            object_path = str(node.get("objectPath") or "")
            key = f"{package}.{object_path}".casefold()
            record = SourceObject(
                key=key,
                source_id=str(node["nodeId"]),
                class_name=str(node.get("className") or ""),
                object_path=f"{package}.{object_path}",
                properties=dict(node.get("properties") or {}),
            )
            self.objects[key] = record
            self.by_source_id[record.source_id] = record

        for edge in graph.get("edges", []):
            source = self.by_source_id.get(str(edge["sourceNodeId"]))
            target = self.by_source_id.get(str(edge.get("targetNodeId") or ""))
            if source is None:
                continue
            property_name = str(edge.get("property") or "")
            object_path = str(edge.get("objectPath") or "")
            if object_path:
                source.reference_paths.append((property_name, object_path))
            if target is not None:
                source.references.append((property_name, target.key))

        for system in graph.get("sourceSystems", []):
            for reference in system.get("unresolvedExternalReferences", []):
                source = self.by_source_id.get(str(reference.get("sourceNodeId") or ""))
                object_path = str(reference.get("objectPath") or "")
                if source is not None and object_path:
                    source.reference_paths.append(
                        (str(reference.get("property") or ""), object_path)
                    )

        for package in closure.get("packages", []):
            logical_package = str(package.get("logicalPackage") or "")
            for row in package.get("objects", []):
                object_path = str(row.get("objectPath") or "")
                key = f"{logical_package}.{object_path}".casefold()
                record = SourceObject(
                    key=key,
                    source_id=str(row.get("objectId") or key),
                    class_name=str(row.get("className") or ""),
                    object_path=f"{logical_package}.{object_path}",
                    properties=dict(row.get("properties") or {}),
                )
                self.objects[key] = record
                self.by_source_id[record.source_id] = record

        for package in closure.get("packages", []):
            logical_package = str(package.get("logicalPackage") or "")
            for row in package.get("objects", []):
                key = f"{logical_package}.{row.get('objectPath', '')}".casefold()
                source = self.objects.get(key)
                if source is None:
                    continue
                for reference in row.get("references", []):
                    object_path = str(reference.get("objectPath") or "")
                    property_name = str(reference.get("property") or "")
                    if object_path:
                        source.reference_paths.append((property_name, object_path))
                    target_key = object_path.casefold()
                    if not target_key.startswith(logical_package.casefold() + "."):
                        target_key = f"{logical_package}.{object_path}".casefold()
                    if target_key in self.objects:
                        source.references.append(
                            (property_name, target_key)
                        )

    def get_id(self, source_id: str | None) -> SourceObject | None:
        return self.by_source_id.get(str(source_id or ""))

    def get_path(self, object_path: str | None) -> SourceObject | None:
        return self.objects.get(folded(object_path))

    def referenced(self, source: SourceObject, property_name: str) -> SourceObject | None:
        wanted = property_name.casefold()
        for reference_property, target_key in source.references:
            if reference_property.casefold() == wanted:
                return self.objects.get(target_key)
        return None


def distribution_properties(value: Any) -> dict[str, Any] | None:
    value = unwrap(value)
    if not isinstance(value, dict):
        return None
    properties = value.get("properties")
    return properties if isinstance(properties, dict) else None


def distribution_target(
    index: SourceIndex,
    source: SourceObject,
    property_name: str,
) -> SourceObject | None:
    return index.referenced(source, f"{property_name}.distribution")


def distribution_float(
    index: SourceIndex,
    source: SourceObject | None,
    property_name: str,
) -> tuple[float, float, float, float, str] | None:
    if source is None:
        return None
    raw = distribution_properties(source.properties.get(property_name.casefold()))
    if raw is None:
        value = finite_number(prop(source.properties, property_name))
        return None if value is None else (value, value, value, value, "EXACT")
    target = distribution_target(index, source, property_name)
    target_class = folded(target.class_name) if target else ""
    operation = int(finite_number(prop(raw, "op")) or 0)
    if operation == 0:
        operation = 2 if "uniform" in target_class else 1
    table = prop(raw, "lookuptable", [])
    values = [number for item in table if (number := finite_number(item)) is not None]
    if len(values) >= 2:
        payload = values[2:]
        chunk_size = int(finite_number(prop(raw, "lookuptablechunksize")) or 0)
        if chunk_size <= 0:
            chunk_size = 2 if operation >= 2 else 1
        if payload and len(payload) % chunk_size == 0:
            samples = [
                payload[index : index + chunk_size]
                for index in range(0, len(payload), chunk_size)
            ]
            flattened = [value for sample in samples for value in sample]
            return (
                min(flattened), max(flattened),
                samples[0][0], samples[-1][-1], "APPROXIMATION",
            )

    if target is None:
        return None
    class_name = target_class
    if "constantcurve" in class_name:
        curve = distribution_properties(target.properties.get("constantcurve"))
        points = prop(curve or {}, "points", [])
        samples = []
        for point in points if isinstance(points, list) else []:
            value = finite_number(prop(point, "outval")) if isinstance(point, dict) else None
            if value is not None:
                samples.append(value)
        if samples:
            return min(samples), max(samples), samples[0], samples[-1], "APPROXIMATION"
    if "uniform" in class_name:
        values = [
            finite_number(prop(target.properties, name))
            for name in ("min", "max", "minlow", "minhigh", "maxlow", "maxhigh")
        ]
        samples = [value for value in values if value is not None]
        if samples:
            return min(samples), max(samples), samples[0], samples[-1], "EXACT_RANGE"
    constant = finite_number(prop(target.properties, "constant"))
    if constant is not None:
        status = "SOURCE_PARAMETER_DEFAULT" if "parameter" in class_name else "EXACT"
        return constant, constant, constant, constant, status
    return None


def table_vector_samples(
    raw: dict[str, Any], operation: int = 1
) -> list[list[float]]:
    table = prop(raw, "lookuptable", [])
    values = [number for item in table if (number := finite_number(item)) is not None]
    if len(values) < 5:
        return []
    values = values[2:]
    chunk_size = int(finite_number(prop(raw, "lookuptablechunksize")) or 0)
    if chunk_size <= 0:
        chunk_size = 3 * (2 if operation >= 2 else 1)
    if chunk_size < 3 or len(values) % chunk_size != 0:
        return []
    samples: list[list[float]] = []
    for entry in range(0, len(values), chunk_size):
        samples.append(values[entry : entry + 3])
        if operation >= 2:
            samples.append(values[entry + 3 : entry + 6])
    return samples


def distribution_vector(
    index: SourceIndex,
    source: SourceObject | None,
    property_name: str,
) -> tuple[list[float], list[float], list[float], list[float], str] | None:
    if source is None:
        return None
    raw = distribution_properties(source.properties.get(property_name.casefold()))
    if raw is None:
        value = vector_value(prop(source.properties, property_name))
        return None if value is None else (value, value, value, value, "EXACT")
    target = distribution_target(index, source, property_name)
    target_class = folded(target.class_name) if target else ""
    operation = int(finite_number(prop(raw, "op")) or 0)
    if operation == 0:
        operation = 2 if "uniform" in target_class else 1
    samples = table_vector_samples(raw, operation)
    if samples:
        minimum = [min(sample[axis] for sample in samples) for axis in range(3)]
        maximum = [max(sample[axis] for sample in samples) for axis in range(3)]
        return minimum, maximum, samples[0], samples[-1], "APPROXIMATION"

    if target is None:
        return None
    class_name = folded(target.class_name)
    samples = []
    for name in ("min", "max", "minlow", "minhigh", "maxlow", "maxhigh", "constant"):
        value = vector_value(prop(target.properties, name))
        if value is not None:
            samples.append(value)
    if samples:
        minimum = [min(sample[axis] for sample in samples) for axis in range(3)]
        maximum = [max(sample[axis] for sample in samples) for axis in range(3)]
        status = "SOURCE_PARAMETER_DEFAULT" if "parameter" in class_name else "EXACT_RANGE"
        return minimum, maximum, samples[0], samples[-1], status
    return None


def modules_of_class(modules: Iterable[SourceObject], token: str) -> list[SourceObject]:
    token = token.casefold()
    return [module for module in modules if token in folded(module.class_name)]


def first_module(modules: Iterable[SourceObject], token: str) -> SourceObject | None:
    return next(iter(modules_of_class(modules, token)), None)


def default_detail() -> dict[str, Any]:
    return {
        "transform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
            "velocityPerSecond": [0.0, 0.0, 0.0],
        },
        "color": {
            "offset": [0.0, 0.0, 0.0, 0.0],
            "multiply": [1.0, 1.0, 1.0, 1.0],
            "clip": 0.0,
            "emissiveIntensity": 1.0,
            "distortionIntensity": 0.0,
            "distortionOnBaseMaterial": False,
            "radialTime": 0.0,
            "radialIntensity": 0.0,
        },
        "uv": {
            "start": [0.0, 0.0],
            "speed": [0.0, 0.0],
            "wave": False,
            "waveAmplitude": [0.0, 0.0],
            "waveFrequency": 1.0,
            "sequence": False,
            "loop": True,
            "sequenceTerm": 0.1,
            "tileColumns": 1,
            "tileRows": 1,
            "tileIndex": 0,
        },
        "timing": {
            "startDelaySeconds": 0.0,
            "lifeTimeSeconds": 1.0,
            "afterImageSeconds": 0.0,
            "dissolveStartNormalized": 1.0,
        },
        "mesh": {"useModelMaterial": True},
        "sprite": {"billboard": True},
        "decal": {"size": [1.0, 1.0], "depth": 0.25},
        "linearLerp": {
            "position": False,
            "endPosition": [0.0, 0.0, 0.0],
            "rotation": False,
            "endRotationDegrees": [0.0, 0.0, 0.0],
            "revolution": False,
            "endRevolutionDegreesPerSecond": [0.0, 0.0, 0.0],
            "scale": False,
            "endScale": [1.0, 1.0, 1.0],
            "velocity": False,
            "endVelocityPerSecond": [0.0, 0.0, 0.0],
            "colorOffset": False,
            "endColorOffset": [0.0, 0.0, 0.0, 0.0],
            "colorMultiply": False,
            "endColorMultiply": [1.0, 1.0, 1.0, 1.0],
            "emissiveIntensity": False,
            "endEmissiveIntensity": 1.0,
        },
        "particle": {
            "maxParticles": 64,
            "spawnRatePerSecond": 0.0,
            "burstCount": 0,
            "randomSeed": 1,
            "lifeTimeSeconds": [0.5, 1.0],
            "initialPositionMin": [0.0, 0.0, 0.0],
            "initialPositionMax": [0.0, 0.0, 0.0],
            "initialVelocityMin": [0.0, 0.0, 0.0],
            "initialVelocityMax": [0.0, 0.0, 0.0],
            "acceleration": [0.0, 0.0, 0.0],
            "startSize": [0.2, 0.2],
            "endSize": [0.0, 0.0],
            "localSpace": True,
            "billboard": True,
        },
        "trail": {
            "maxPoints": 64,
            "pointLifeTimeSeconds": 0.35,
            "sampleIntervalSeconds": 0.0166667,
            "minimumDistance": 0.01,
            "startWidth": 0.2,
            "endWidth": 0.0,
            "faceCamera": True,
        },
        "afterImage": {
            "sampleIntervalSeconds": 0.05,
            "maxCopies": 16,
            "alphaExponent": 1.0,
        },
        "light": {
            "enabled": False,
            "profileId": "",
            "status": "reconstructed_profile",
            "range": 0.0,
            "intensity": 0.0,
            "color": [0.0, 0.0, 0.0, 0.0],
            "ambient": [0.0, 0.0, 0.0, 0.0],
            "falloffExponent": 0.0,
        },
        "screenPost": {
            "enabled": False,
            "profileId": "",
            "status": "reconstructed_profile",
            "intensity": 0.0,
            "secondaryIntensity": 0.0,
            "frequency": 0.0,
            "tint": [1.0, 1.0, 1.0, 1.0],
            "randomSeed": 1,
        },
    }


def source_parameter(
    name: str,
    parameter_type: str,
    status: str,
    source_property_path: str,
    value: Any = None,
) -> dict[str, Any]:
    row = {
        "name": name,
        "type": parameter_type,
        "status": status,
        "sourcePropertyPath": source_property_path,
        "numberValue": 0.0,
        "boolValue": False,
        "vectorValue": [0.0, 0.0, 0.0, 0.0],
        "stringValue": "",
    }
    if parameter_type == "number" and value is not None:
        number = finite_number(value)
        if number is None:
            raise ValueError(f"presentation parameter is non-finite: {name}")
        row["numberValue"] = number
    elif parameter_type == "boolean" and value is not None:
        if not isinstance(value, bool):
            raise ValueError(f"presentation parameter is not boolean: {name}")
        row["boolValue"] = value
    elif parameter_type == "vector" and value is not None:
        if not isinstance(value, list) or len(value) > 4:
            raise ValueError(f"presentation parameter vector is invalid: {name}")
        numbers = [finite_number(component) for component in value]
        if any(component is None for component in numbers):
            raise ValueError(f"presentation parameter vector is non-finite: {name}")
        row["vectorValue"] = [
            *[float(component) for component in numbers if component is not None],
            *([0.0] * (4 - len(numbers))),
        ]
    elif parameter_type == "string" and value is not None:
        row["stringValue"] = str(value)
    elif parameter_type not in {"number", "boolean", "vector", "string"}:
        raise ValueError(f"presentation parameter type is invalid: {parameter_type}")
    return row


def default_source_presentation() -> dict[str, Any]:
    return {
        "enabled": False,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": "",
        "status": "unresolved",
        "sourceObjectPath": "",
        "sourceActionCueId": "",
        "sourceEventId": "",
        "sourceOccurrenceIndex": 0,
        "sourceTimeSeconds": 0.0,
        "parameters": [],
    }


def mapping(
    rows: list[dict[str, Any]],
    target: str,
    status: str,
    source: str,
    value: Any,
    note: str = "",
) -> None:
    row = {"target": target, "status": status, "source": source, "value": value}
    if note:
        row["note"] = note
    rows.append(row)


def emitter_detail(
    index: SourceIndex,
    lod: SourceObject,
    modules: list[SourceObject],
    event_time: float,
    event_duration: float,
    seed: int,
) -> tuple[dict[str, Any], list[dict[str, Any]], list[dict[str, Any]]]:
    detail = default_detail()
    mappings: list[dict[str, Any]] = []
    required = first_module(modules, "particlemodulerequired")
    spawn = first_module(modules, "particlemodulespawn")

    explicit_local_space = (
        required is not None and "buselocalspace" in required.properties
    )
    local_space = bool(prop(required.properties, "buselocalspace", True)) if required else True
    detail["particle"]["localSpace"] = local_space
    mapping(
        mappings,
        "particle.localSpace",
        "EXACT" if explicit_local_space else "SOURCE_CLASS_DEFAULT",
        "Required.bUseLocalSpace",
        local_space,
        (
            "literal source property"
            if explicit_local_space
            else "source literal absent; UE3 class default remains unverified"
        ),
    )

    duration = finite_number(prop(required.properties, "emitterduration")) if required else None
    delay_distribution = distribution_float(index, required, "emitterdelay")
    emitter_delay = max(0.0, delay_distribution[2]) if delay_distribution else 0.0
    # The skill cue and the emitter-local delay are separate clocks. Keep the
    # cue time on the Element and execute Required.EmitterDelay inside the
    # source recipe so loops and component audition preserve UE3 semantics.
    detail["timing"]["startDelaySeconds"] = max(0.0, event_time)
    mapping(
        mappings,
        "timing.startDelaySeconds",
        "EXACT" if not delay_distribution else delay_distribution[4],
        "animation event only; Required.EmitterDelay is sourceRecipe-local",
        detail["timing"]["startDelaySeconds"],
    )

    lifetime_module = first_module(modules, "particlemodulelifetime")
    lifetime = distribution_float(index, lifetime_module, "lifetime")
    if lifetime:
        life_min = clamp(min(lifetime[0], lifetime[1]), 0.001, 30.0)
        life_max = clamp(max(lifetime[0], lifetime[1]), life_min, 30.0)
        detail["particle"]["lifeTimeSeconds"] = [life_min, life_max]
        mapping(mappings, "particle.lifeTimeSeconds", lifetime[4], f"{lifetime_module.object_path}.Lifetime", [life_min, life_max])
    else:
        life_min, life_max = 0.5, 1.0
        mapping(mappings, "particle.lifeTimeSeconds", "DEFAULT_NEEDS_TUNING", "missing/parameter-only Lifetime", [life_min, life_max])

    # The Effect runtime treats timing.lifeTimeSeconds as the emitter's active
    # window, then adds particle.lifeTimeSeconds.max as the surviving-particle
    # tail.  Including particle lifetime here would therefore emit for an
    # extra lifetime and count the same tail twice (for example 20 s became
    # roughly 40 s for Instance_BGCrack_01).
    timing_life = max(duration or 0.0, event_duration, 0.1)
    detail["timing"]["lifeTimeSeconds"] = timing_life
    mapping(mappings, "timing.lifeTimeSeconds", "APPROXIMATION", "max(EmitterDuration, notify duration, 0.1); particle lifetime is a separate runtime tail", timing_life)

    rate = distribution_float(index, spawn, "rate") or distribution_float(index, required, "spawnrate")
    spawn_rate = clamp(rate[3] if rate else 0.0, 0.0, 2048.0)
    detail["particle"]["spawnRatePerSecond"] = spawn_rate
    mapping(
        mappings,
        "particle.spawnRatePerSecond",
        rate[4] if rate else "DEFAULT_NEEDS_TUNING",
        f"{spawn.object_path}.Rate" if spawn else "Required.SpawnRate",
        spawn_rate,
    )

    peak = finite_number(prop(lod.properties, "peakactiveparticles")) or 64.0
    max_particles = max(1, min(MAX_PARTICLES_PER_IMPORTED_ELEMENT, int(math.ceil(peak))))
    detail["particle"]["maxParticles"] = max_particles
    detail["particle"]["randomSeed"] = max(1, seed)
    mapping(mappings, "particle.maxParticles", "BUDGET_CLAMP", "LOD.PeakActiveParticles", max_particles, f"source={int(peak)}")

    velocity_module = first_module(modules, "particlemodulevelocity")
    velocity = distribution_vector(index, velocity_module, "startvelocity")
    if velocity:
        minimum, maximum = ue3_centimeter_range_to_client(
            velocity[0], velocity[1]
        )
        detail["particle"]["initialVelocityMin"] = minimum
        detail["particle"]["initialVelocityMax"] = maximum
        mapping(mappings, "particle.initialVelocityMin/Max", "APPROXIMATION", f"{velocity_module.object_path}.StartVelocity", [minimum, maximum], "UE3 (X,Y,Z) converted once to Client (X,Z,-Y), then source units scaled by 0.01")

    acceleration_module = first_module(modules, "particlemoduleacceleration")
    acceleration = distribution_vector(index, acceleration_module, "acceleration")
    if acceleration:
        value = ue3_centimeters_to_client(acceleration[2])
        detail["particle"]["acceleration"] = value
        mapping(mappings, "particle.acceleration", "APPROXIMATION", f"{acceleration_module.object_path}.Acceleration", value, "random/curve range collapsed to one vector after UE3 (X,Y,Z) -> Client (X,Z,-Y) conversion")

    size_module = first_module(modules, "particlemodulesize")
    size = distribution_vector(index, size_module, "startsize")
    if size:
        start_size = [max(0.01, abs(size[2][0]) * SOURCE_UNITS_TO_RUNTIME), max(0.01, abs(size[2][1]) * SOURCE_UNITS_TO_RUNTIME)]
        detail["particle"]["startSize"] = start_size
        detail["decal"]["size"] = start_size
        mapping(mappings, "particle.startSize/decal.size", "APPROXIMATION", f"{size_module.object_path}.StartSize", start_size, "XY only; UE source units scaled by 0.01")

    size_life_module = first_module(modules, "particlemodulesizemultiplylife")
    size_life = distribution_vector(index, size_life_module, "lifemultiplier")
    if size_life:
        end_size = [
            max(0.0, detail["particle"]["startSize"][axis] * max(0.0, size_life[3][axis]))
            for axis in range(2)
        ]
        detail["particle"]["endSize"] = end_size
        mapping(mappings, "particle.endSize", "APPROXIMATION", f"{size_life_module.object_path}.LifeMultiplier", end_size, "full curve collapsed to final sample")

    color_module = first_module(modules, "particlemodulecolor")
    start_color = distribution_vector(index, color_module, "startcolor")
    start_alpha = distribution_float(index, color_module, "startalpha")
    if start_color:
        rgba = [max(0.0, value) for value in start_color[2]] + [max(0.0, start_alpha[2] if start_alpha else 1.0)]
        mapping(
            mappings, "sourceRecipe.color", "SOURCE_RECIPE_OWNS_BASELINE",
            f"{color_module.object_path}.StartColor/StartAlpha", rgba,
            "Detail color multiply remains identity for authored override",
        )

    over_life = first_module(modules, "particlemodulecoloroverlife") or first_module(modules, "particlemodulecolorscaleoverlife")
    if over_life:
        color_property = "coloroverlife" if "coloroverlife" in folded(over_life.class_name) else "colorscaleoverlife"
        alpha_property = "alphaoverlife" if "coloroverlife" in folded(over_life.class_name) else "alphascaleoverlife"
        end_color = distribution_vector(index, over_life, color_property)
        end_alpha = distribution_float(index, over_life, alpha_property)
        if end_color or end_alpha:
            rgba = [max(0.0, value) for value in (end_color[3] if end_color else [1.0, 1.0, 1.0])] + [max(0.0, end_alpha[3] if end_alpha else 1.0)]
            mapping(
                mappings, "sourceRecipe.colorOverLife",
                "SOURCE_RECIPE_OWNS_BASELINE", over_life.object_path, rgba,
                "Detail color lerp remains disabled for authored override",
            )

    location_module = first_module(modules, "particlemodulelocation")
    location = distribution_vector(index, location_module, "startlocation")
    if location:
        minimum, maximum = ue3_centimeter_range_to_client(
            location[0], location[1]
        )
        detail["particle"]["initialPositionMin"] = minimum
        detail["particle"]["initialPositionMax"] = maximum
        mapping(mappings, "particle.initialPositionMin/Max", "EXACT_RANGE", f"{location_module.object_path}.StartLocation", [minimum, maximum], "UE3 (X,Y,Z) converted once to Client (X,Z,-Y), then source units scaled by 0.01")

    subuv = first_module(modules, "particlemodulesubuv")
    if subuv:
        horizontal = int(finite_number(prop(required.properties, "subimages_horizontal")) or 1) if required else 1
        vertical = int(finite_number(prop(required.properties, "subimages_vertical")) or 1) if required else 1
        if horizontal > 1 or vertical > 1:
            mapping(
                mappings, "sourceRecipe.subUV",
                "SOURCE_RECIPE_OWNS_BASELINE",
                f"{required.object_path}.SubImages", [horizontal, vertical],
                "Detail UV sequence remains identity for authored override",
            )

    bursts: list[dict[str, Any]] = []
    burst_rows = prop(spawn.properties, "burstlist", []) if spawn else []
    for burst_index, row in enumerate(burst_rows if isinstance(burst_rows, list) else []):
        if not isinstance(row, dict):
            continue
        count = int(finite_number(prop(row, "count")) or 0)
        count_low = int(finite_number(prop(row, "countlow")) or -1)
        time = max(0.0, finite_number(prop(row, "time")) or 0.0)
        if count <= 0:
            continue
        bursts.append({"index": burst_index, "timeSeconds": time, "count": count, "countLow": count_low})
    return detail, mappings, bursts


def event_index(source_receipt: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    result: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for event in source_receipt.get("timeline", {}).get("events", []):
        if event.get("resolutionStatus") == "RESOLVED_PARTICLE_GRAPH" and event.get("sourceSystemId"):
            result[folded(event["sourceSystemId"])].append(event)
    return result


def apply_action_cue_transform(
    detail: dict[str, Any],
    event: dict[str, Any],
    previous_event: dict[str, Any] | None = None,
) -> None:
    def transform(row: dict[str, Any] | None) -> dict[str, Any] | None:
        if not isinstance(row, dict):
            return None
        payload = row.get("actionCuePayload")
        if not isinstance(payload, dict) or not payload.get(
            "particleDataDecoded", False
        ):
            return None
        value = payload.get("localTransform")
        return value if isinstance(value, dict) else None

    current = transform(event)
    if current is None:
        return
    previous = transform(previous_event)
    previous_position = (
        previous.get("position", [0.0, 0.0, 0.0])
        if previous is not None else [0.0, 0.0, 0.0]
    )
    previous_rotation = (
        previous.get("rotationDegrees", [0.0, 0.0, 0.0])
        if previous is not None else [0.0, 0.0, 0.0]
    )
    previous_scale = (
        previous.get("scale", [1.0, 1.0, 1.0])
        if previous is not None else [1.0, 1.0, 1.0]
    )
    target = detail["transform"]
    for index in range(3):
        target["position"][index] += (
            float(current["position"][index])
            - float(previous_position[index])
        )
        target["rotationDegrees"][index] += (
            float(current["rotationDegrees"][index])
            - float(previous_rotation[index])
        )
        denominator = float(previous_scale[index])
        if denominator <= 0.0:
            raise ValueError("previous Action cue scale must be positive")
        target["scale"][index] *= (
            float(current["scale"][index]) / denominator
        )


def action_cue_attachment(event: dict[str, Any]) -> dict[str, Any]:
    payload = event.get("actionCuePayload")
    if not isinstance(payload, dict) or not payload.get(
        "particleDataDecoded", False
    ):
        return {
            "enabled": False,
            "follow": False,
            "sourceAnchorSlotId": "",
            "runtimeAnchorSlotId": "",
            "runtimeBoneName": "",
            "socketLocalTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        }
    attachment = payload.get("attachment")
    if not isinstance(attachment, dict):
        raise ValueError("decoded Action cue has no attachment contract")
    follow = attachment.get("mode") == "FOLLOW_NAMED_ANCHORS"
    runtime_bone = str(attachment.get("runtimeBoneName") or "")
    if follow and not runtime_bone:
        raise ValueError(
            "follow Action cue has no exact runtime bone resolution: "
            f"{attachment.get('sourceAnchorNames', [])} "
            f"({event.get('sourceActionCueId', 'unknown cue')})"
        )
    source_names = attachment.get("sourceAnchorNames", [])
    return {
        "enabled": True,
        "follow": follow,
        "sourceAnchorSlotId": (
            str(source_names[0]) if source_names else "root"
        ),
        "runtimeAnchorSlotId": str(
            attachment.get("runtimeAnchorSlotId") or "root"
        ),
        "runtimeBoneName": runtime_bone,
        "socketLocalTransform": copy.deepcopy(
            attachment.get("socketLocalTransform")
            or {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            }
        ),
    }


def runtime_index(graph: dict[str, Any]) -> dict[str, str]:
    result = {}
    for row in graph.get("runtimeResourceBindings", []):
        if row.get("resolutionStatus") == "RESOLVED_RUNTIME_ASSET" and row.get("sourceObjectPath") and row.get("assetId"):
            result[folded(row["sourceObjectPath"])] = str(row["assetId"])
    return result


def material_index(graph: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {
        folded(row.get("sourceMaterialPath")): row
        for row in graph.get("materialParameterBindings", [])
        if row.get("sourceMaterialPath")
    }


def texture_slot(parameter_name: str) -> str | None:
    value = folded(parameter_name)
    if "dissolve" in value:
        return "dissolve"
    if any(token in value for token in ("normal", "bump")):
        return None
    if any(token in value for token in ("mask", "opacity", "alpha")):
        return "mask"
    if any(token in value for token in ("noise", "distort")):
        return "noise"
    if any(token in value for token in ("emiss", "glow", "bloom")):
        return "emissive"
    if any(token in value for token in (
        "reflection", "reflect", "refle", "environment",
    )):
        return "base"
    return "base"


def has_explicit_texture_semantic(parameter_name: str) -> bool:
    value = folded(parameter_name)
    return any(token in value for token in (
        "dissolve", "normal", "bump", "mask", "opacity", "alpha",
        "noise", "distort", "emiss", "glow", "bloom",
        "reflection", "reflect", "refle", "environment",
    ))


def choose_resources(
    system: dict[str, Any],
    module_ids: set[str],
    graph: dict[str, Any],
) -> tuple[list[dict[str, str]], list[dict[str, Any]], list[dict[str, Any]]]:
    runtime = runtime_index(graph)
    materials = material_index(graph)
    meshes: list[tuple[str, str]] = []
    texture_candidates: list[tuple[str, str, str]] = []
    material_rows: list[dict[str, Any]] = []
    for binding in system.get("resourceBindings", []):
        if str(binding.get("sourceNodeId")) not in module_ids:
            continue
        role = folded(binding.get("role"))
        source_path = str(binding.get("objectPath") or "")
        asset_id = runtime.get(folded(source_path))
        if role == "mesh" and asset_id:
            meshes.append((source_path, asset_id))
        if role != "material":
            continue
        material = materials.get(folded(source_path))
        if material is None:
            continue
        material_rows.append(material)
        for texture in material.get("textures", []):
            texture_path = str(texture.get("texture") or "")
            texture_asset = runtime.get(folded(texture_path))
            if texture_asset:
                texture_candidates.append((
                    str(texture.get("name") or ""),
                    texture_path,
                    texture_asset,
                ))

    resources: list[dict[str, str]] = []
    receipt: list[dict[str, Any]] = []
    if meshes:
        source_path, asset_id = meshes[0]
        resources.append({"slotId": "meshModel", "assetId": asset_id})
        receipt.append({"slotId": "meshModel", "sourceObjectPath": source_path, "assetId": asset_id, "status": "EXACT_RUNTIME_BINDING"})

    used_slots: set[str] = set()
    used_assets: set[str] = set()
    ranked = sorted(
        texture_candidates,
        key=lambda row: (
            1 if texture_slot(row[0]) is None else 0,
            0 if texture_slot(row[0]) == "base" else 1,
            0 if any(token in folded(row[0]) for token in (
                "main", "base", "diffuse", "texture",
            )) else 1,
            folded(row[0]),
            folded(row[2]),
        ),
    )
    has_base_candidate = any(
        texture_slot(row[0]) == "base" for row in ranked
    )
    primary_emissive = next((
        row for row in ranked
        if not has_base_candidate and texture_slot(row[0]) == "emissive"
    ), None)
    for parameter, source_path, asset_id in ranked:
        if asset_id in used_assets:
            continue
        resolved_slot = texture_slot(parameter)
        if resolved_slot is None:
            receipt.append({
                "slotId": None,
                "parameterName": parameter,
                "sourceObjectPath": source_path,
                "assetId": asset_id,
                "status": "FALLBACK_BLOCKED_UNRESOLVED_TEXTURE_ROLE",
            })
            continue
        slot = "base" if (parameter, source_path, asset_id) == \
            primary_emissive else resolved_slot
        if slot in used_slots:
            if has_explicit_texture_semantic(parameter):
                continue
            for fallback in (
                "base", "noise", "mask", "emissive", "dissolve",
            ):
                if fallback not in used_slots:
                    slot = fallback
                    break
            else:
                continue
        used_slots.add(slot)
        used_assets.add(asset_id)
        resources.append({"slotId": slot, "assetId": asset_id})
        receipt.append({"slotId": slot, "parameterName": parameter, "sourceObjectPath": source_path, "assetId": asset_id, "status": "PARAMETER_NAME_HEURISTIC"})
        if len(used_slots) == 5:
            break
    if material_rows and "base" not in used_slots:
        source_path, asset_id = PROCEDURAL_MATERIAL_BASE_FALLBACK
        if runtime.get(folded(source_path)) == asset_id:
            resources.append({"slotId": "base", "assetId": asset_id})
            receipt.append(
                {
                    "slotId": "base",
                    "sourceObjectPath": source_path,
                    "assetId": asset_id,
                    "status": "PROCEDURAL_MATERIAL_WHITE_FALLBACK",
                    "note": "Original material has no resolved texture parameter; replace after visual review.",
                }
            )
    return resources, receipt, material_rows


def material_detail(material_rows: list[dict[str, Any]], detail: dict[str, Any], mappings: list[dict[str, Any]]) -> str:
    names = " ".join(folded(row.get("sourceMaterialPath")) for row in material_rows)
    profile = "additive_two_sided_depth_read" if any(token in names for token in ("_ad", "add", "glow")) else "alpha_two_sided_depth_read"
    scalar_rows = [scalar for material in material_rows for scalar in material.get("scalars", [])]
    emissive_candidates: list[tuple[int, int, dict[str, Any], float]] = []
    for scalar_index, row in enumerate(scalar_rows):
        name = folded(row.get("name"))
        value = finite_number(row.get("value"))
        if value is None:
            continue
        if "distort" in name and any(token in name for token in ("power", "intensity", "strength")):
            detail["color"]["distortionIntensity"] = max(0.0, value)
            detail["color"]["distortionOnBaseMaterial"] = True
            mapping(mappings, "color.distortionIntensity", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), max(0.0, value))
        elif (
            any(token in name for token in ("emissive", "emission", "emissiion", "bloom", "glow"))
            and any(token in name for token in ("intensity", "strength", "_str", "multiply", "value", "velue"))
            and not any(token in name for token in ("flicker", "speed", "tile", "pan", "texcoord", "_min", "min_", "_max", "max_", "power"))
        ):
            priority = 0 if "intensity" in name else 1
            emissive_candidates.append((priority, scalar_index, row, value))
        elif any(token in name for token in ("panning_x", "pan_x", "uspeed", "speed_u")):
            detail["uv"]["speed"][0] = value
            mapping(mappings, "uv.speed.x", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), value)
        elif any(token in name for token in ("panning_y", "pan_y", "vspeed", "speed_v")):
            detail["uv"]["speed"][1] = value
            mapping(mappings, "uv.speed.y", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), value)
    if emissive_candidates:
        _, _, selected, source_value = min(emissive_candidates, key=lambda row: (row[0], row[1]))
        mapped_value = clamp(source_value, 0.0, 100.0)
        detail["color"]["emissiveIntensity"] = mapped_value
        mapping(
            mappings,
            "color.emissiveIntensity",
            "PARAMETER_NAME_HEURISTIC",
            str(selected.get("name")),
            mapped_value,
            "explicit emissive strength/intensity scalar; runtime safety clamp [0, 100]"
            if mapped_value != source_value
            else "explicit emissive strength/intensity scalar",
        )
    return profile


def selected_lod_partitions(
    graph: dict[str, Any],
    index: SourceIndex,
    closure: dict[str, Any],
) -> list[tuple[dict[str, Any], SourceObject, SourceObject, list[SourceObject]]]:
    # One UE3 LOD stores RequiredModule, Modules[], TypeDataModule and
    # SpawnModule in a single ordered reference stream. Internal exports and
    # external imports must be merged by referenceIndex; appending imports
    # afterwards changes Cascade execution order. The normalized graph owns
    # that occurrence order while the closure owns decoded external payloads.
    external_by_lod: dict[str, list[tuple[int, SourceObject]]] = defaultdict(list)
    for system in graph.get("sourceSystems", []):
        for request in system.get("unresolvedExternalReferences", []):
            record = index.get_path(request.get("objectPath"))
            if record is not None:
                external_by_lod[str(request.get("sourceNodeId"))].append(
                    (int(request.get("referenceIndex", 0)), record)
                )

    def occurrence(
        source: SourceObject,
        source_lod_id: str,
        reference_index: int,
    ) -> SourceObject:
        # A single module object may intentionally appear more than once in
        # Modules[]. Preserve every occurrence rather than deduplicating the
        # source object path.
        return SourceObject(
            key=(
                f"{source.key}#occurrence:{source_lod_id}:"
                f"{reference_index}"
            ),
            source_id=f"{source.source_id}@ref:{reference_index}",
            class_name=source.class_name,
            object_path=source.object_path,
            properties=source.properties,
            references=source.references,
            reference_paths=source.reference_paths,
        )

    partitions = []
    for system in graph.get("sourceSystems", []):
        root = index.get_id(str(system.get("rootNodeId")))
        if root is None:
            continue
        emitter_edges = [
            row for row in index.outgoing.get(root.source_id, [])
            if base_property_name(str(row.get("property") or "")) == "emitters" and row.get("targetNodeId")
        ]
        for emitter_edge in emitter_edges:
            emitter = index.get_id(str(emitter_edge.get("targetNodeId")))
            if emitter is None:
                continue
            lod_edge = next(
                (
                    row for row in index.outgoing.get(emitter.source_id, [])
                    if base_property_name(str(row.get("property") or "")) == "lodlevels" and row.get("targetNodeId")
                ),
                None,
            )
            lod = index.get_id(str(lod_edge.get("targetNodeId"))) if lod_edge else None
            if lod is None:
                continue
            ordered_references: list[tuple[int, SourceObject]] = []
            for edge in index.outgoing.get(lod.source_id, []):
                target = index.get_id(str(edge.get("targetNodeId") or ""))
                if target is not None:
                    ordered_references.append(
                        (int(edge.get("referenceIndex", 0)), target)
                    )
            ordered_references.extend(external_by_lod.get(lod.source_id, []))
            ordered_references.sort(key=lambda row: row[0])
            modules = [
                occurrence(target, lod.source_id, reference_index)
                for reference_index, target in ordered_references
            ]
            partitions.append((system, emitter, lod, modules))
    return partitions


def float4(value: Any, component_count: int) -> list[float]:
    if component_count == 1:
        number = finite_number(unwrap(value))
        return [float(number or 0.0), 0.0, 0.0, 0.0]
    vector = vector_value(value) or [0.0, 0.0, 0.0]
    return [*vector[:component_count], *([0.0] * (4 - component_count))]


def curve_key(point: dict[str, Any], component_count: int) -> dict[str, Any] | None:
    time = finite_number(prop(point, "inval"))
    if time is None:
        return None
    value = float4(prop(point, "outval"), component_count)
    arrive = float4(prop(point, "arrivetangent"), component_count)
    leave = float4(prop(point, "leavetangent"), component_count)
    mode = folded(prop(point, "interpmode", "cim_linear"))
    interpolation = (
        "constant" if "constant" in mode else
        "linear" if "linear" in mode else
        "cubic"
    )
    return {
        "time": time,
        "minimum": value,
        "maximum": value,
        "arriveTangentMinimum": arrive,
        "leaveTangentMinimum": leave,
        "arriveTangentMaximum": arrive,
        "leaveTangentMaximum": leave,
        "interpolation": interpolation,
    }


def referenced_distribution_defaults(
    target: SourceObject | None,
    component_count: int,
) -> tuple[list[float], list[float], list[dict[str, Any]]]:
    zero = [0.0, 0.0, 0.0, 0.0]
    if target is None:
        return zero, zero, []
    class_name = folded(target.class_name)
    if "constantcurve" in class_name:
        curve_name = "constantcurve"
        curve = distribution_properties(target.properties.get(curve_name))
        points = prop(curve or {}, "points", [])
        keys = [
            key for row in points if isinstance(row, dict)
            if (key := curve_key(row, component_count)) is not None
        ] if isinstance(points, list) else []
        if keys:
            return keys[0]["minimum"], keys[0]["maximum"], keys
    if "uniformrange" in class_name:
        candidates = [
            float4(prop(target.properties, name), component_count)
            for name in ("minlow", "minhigh", "maxlow", "maxhigh")
        ]
        minimum = [min(row[index] for row in candidates) for index in range(4)]
        maximum = [max(row[index] for row in candidates) for index in range(4)]
        return minimum, maximum, []
    if "uniform" in class_name:
        return (
            float4(prop(target.properties, "min"), component_count),
            float4(prop(target.properties, "max"), component_count),
            [],
        )
    constant = prop(target.properties, "constant")
    if constant is not None:
        value = float4(constant, component_count)
        return value, value, []
    return zero, zero, []


def build_distribution_recipe(
    index: SourceIndex,
    module: SourceObject,
    property_path: str,
    raw_wrapper: dict[str, Any],
) -> dict[str, Any]:
    component_count = 1 if folded(raw_wrapper.get("structType")) == "rawdistributionfloat" else 3
    raw = distribution_properties(raw_wrapper) or {}
    target = distribution_target(index, module, property_path)
    default_minimum, default_maximum, keys = referenced_distribution_defaults(
        target, component_count
    )
    lookup_table = [
        number for item in prop(raw, "lookuptable", [])
        if (number := finite_number(item)) is not None
    ]
    operation = int(finite_number(prop(raw, "op")) or 0)
    if operation == 0:
        target_class = folded(target.class_name) if target else ""
        operation = 2 if "uniform" in target_class else 1
    if operation not in (1, 2, 3):
        raise ValueError(
            f"unsupported UE3 raw distribution operation {operation}: "
            f"{module.object_path}.{property_path}"
        )
    chunk_size = int(finite_number(prop(raw, "lookuptablechunksize")) or 0)
    if lookup_table and chunk_size == 0:
        chunk_size = component_count * (2 if operation >= 2 else 1)
    expected_num_elements = 2 if operation >= 2 else 1
    num_elements = int(finite_number(prop(raw, "lookuptablenumelements")) or 0)
    if lookup_table and num_elements == 0:
        num_elements = expected_num_elements
    required_values = component_count * expected_num_elements
    if lookup_table and (
        len(lookup_table) < 2 + required_values
        or chunk_size != required_values
        or num_elements != expected_num_elements
        or (len(lookup_table) - 2) % chunk_size != 0
    ):
        raise ValueError(
            "malformed UE3 cooked distribution lookup payload: "
            f"{module.object_path}.{property_path}"
        )
    raw_type = finite_number(prop(raw, "type"))
    if component_count > 1 and raw_type is not None:
        # UE3 serializes FRawDistributionVector::Type as a bit field.  Bits
        # 0..2 are the runtime LockFlag consumed by GetValue3Random.  Keep
        # the already-cooked lookup payload intact and forward that flag;
        # higher bits describe source-curve metadata, not another sample.
        random_lock_axes = int(raw_type) & 0x07
    else:
        random_lock_axes = {
            "edvlf_xy": 1,
            "edvlf_xz": 2,
            "edvlf_yz": 3,
            "edvlf_xyz": 4,
        }.get(folded(prop(target.properties, "lockedaxes")) if target else "", 0)
    return {
        "propertyPath": property_path,
        "sourceClass": target.class_name if target else "",
        "sourceObjectPath": target.object_path if target else "",
        "componentCount": component_count,
        "operation": operation,
        "randomLockAxes": random_lock_axes,
        "lookupTableChunkSize": max(0, chunk_size),
        "lookupTableNumElements": max(0, num_elements),
        "lookupTableTimeScale": finite_number(prop(raw, "lookuptabletimescale")) or 0.0,
        "lookupTableStartTime": finite_number(prop(raw, "lookuptablestarttime")) or 0.0,
        "defaultMinimum": default_minimum,
        "defaultMaximum": default_maximum,
        "lookupTable": lookup_table,
        "keys": keys,
    }


def flatten_source_properties(
    index: SourceIndex,
    module: SourceObject,
    semantic_overlay: dict[str, Any] | None = None,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    literals: list[dict[str, Any]] = []
    distributions: list[dict[str, Any]] = []

    def visit(value: Any, path: str) -> None:
        if isinstance(value, dict) and value.get("type") == "structproperty" and folded(value.get("structType")) in {"rawdistributionfloat", "rawdistributionvector"}:
            distributions.append(build_distribution_recipe(index, module, path, value))
            return
        if isinstance(value, dict) and "type" in value and "value" in value:
            visit(value.get("value"), path)
            return
        if isinstance(value, bool):
            literals.append({"propertyPath": path, "kind": "boolean", "value": value})
            return
        if isinstance(value, (int, float)) and not isinstance(value, bool):
            number = finite_number(value)
            if number is not None:
                literals.append({"propertyPath": path, "kind": "number", "value": number})
            return
        if isinstance(value, str):
            literals.append({"propertyPath": path, "kind": "string", "value": value})
            return
        if isinstance(value, list):
            for item_index, item in enumerate(value):
                visit(item, f"{path}[{item_index}]")
            return
        if isinstance(value, dict):
            for name, item in value.items():
                child = f"{path}.{folded(name)}" if path else folded(name)
                visit(item, child)

    for property_name, property_value in module.properties.items():
        visit(property_value, folded(property_name))

    unique_reference_paths: list[tuple[str, str]] = []
    seen_reference_identities: set[tuple[str, str]] = set()
    for property_name, object_path in module.reference_paths:
        key = (folded(property_name), object_path.casefold())
        if key in seen_reference_identities:
            continue
        seen_reference_identities.add(key)
        unique_reference_paths.append((property_name, object_path))
    reference_property_counts = Counter(
        folded(property_name)
        for property_name, _object_path in unique_reference_paths
    )
    reference_property_indices: Counter[str] = Counter()
    for property_name, object_path in unique_reference_paths:
        property_key = folded(property_name)
        property_index = reference_property_indices[property_key]
        reference_property_indices[property_key] += 1
        reference_path = (
            f"{property_key}[{property_index}].objectpath"
            if reference_property_counts[property_key] > 1
            else property_key + ".objectpath"
        )
        literals.append(
            {"propertyPath": reference_path, "kind": "string", "value": object_path}
        )

    semantic_root = semantic_overlay or {}
    module_overlays = semantic_root.get("modules")
    if not isinstance(module_overlays, dict):
        # Backward compatibility for callers that pass only the modules map.
        module_overlays = semantic_root
    overlay = module_overlays.get(folded(module.object_path))
    if isinstance(overlay, dict):
        random_seeds = overlay.get("randomSeeds")
        if isinstance(random_seeds, list):
            literals = [
                row for row in literals
                if not str(row.get("propertyPath", "")).startswith("randomseedinfo.")
            ]
            for seed_index, seed in enumerate(random_seeds):
                literals.append(
                    {
                        "propertyPath": f"randomseedinfo.randomseeds[{seed_index}]",
                        "kind": "number",
                        "value": int(seed),
                    }
                )
            literals.append(
                {
                    "propertyPath": "randomseedinfo.bresetseedonemitterlooping",
                    "kind": "boolean",
                    "value": bool(overlay.get("resetSeedOnEmitterLooping", False)),
                }
            )
            for property_path, overlay_name in (
                ("randomseedinfo.brandomlyselectseedarray", "randomlySelectSeedArray"),
                ("randomseedinfo.bgetseedfrominstance", "getSeedFromInstance"),
                ("randomseedinfo.binstanceseedisindex", "instanceSeedIsIndex"),
            ):
                literals.append(
                    {
                        "propertyPath": property_path,
                        "kind": "boolean",
                        "value": bool(overlay.get(overlay_name, False)),
                    }
                )
    vector_field_object_paths = sorted({
        str(object_path)
        for property_name, object_path in module.reference_paths
        if folded(property_name) == "vectorfield" and str(object_path)
    }, key=str.casefold)
    if len(vector_field_object_paths) > 1:
        raise ValueError(
            "local vector field has conflicting source references: "
            f"{module.object_path}: {vector_field_object_paths}"
        )
    source_vector_field_path = (
        vector_field_object_paths[0] if vector_field_object_paths else ""
    )
    overlay_vector_field_path = (
        str(overlay.get("vectorFieldObjectPath") or "")
        if isinstance(overlay, dict) else ""
    )
    if (
        source_vector_field_path
        and overlay_vector_field_path
        and folded(source_vector_field_path) != folded(overlay_vector_field_path)
    ):
        raise ValueError(
            "local vector field overlay/source identity mismatch: "
            f"{module.object_path}: {overlay_vector_field_path} != "
            f"{source_vector_field_path}"
        )
    vector_field_object_path = (
        overlay_vector_field_path or source_vector_field_path
    )
    vector_field_asset_id = (
        overlay.get("vectorFieldAssetId")
        if isinstance(overlay, dict) else None
    )
    if (
        folded(module.class_name) == "particlemodulelocalvectorfield"
        and vector_field_object_path
        and not vector_field_asset_id
    ):
        vector_field_rows = semantic_root.get("vectorFields", [])
        matches = [
            row for row in vector_field_rows
            if isinstance(row, dict)
            and folded(row.get("sourceObjectPath"))
            == folded(vector_field_object_path)
        ] if isinstance(vector_field_rows, list) else []
        if len(matches) > 1:
            raise ValueError(
                "local vector field has duplicate semantic assets: "
                f"{vector_field_object_path}"
            )
        if matches:
            vector_field_asset_id = matches[0].get("assetId")
    if isinstance(vector_field_asset_id, str) and vector_field_asset_id:
        literals = [
            row for row in literals
            if row.get("propertyPath") not in {
                "vectorfield.objectpath", "vectorfield.assetid"
            }
        ]
        if vector_field_object_path:
            literals.append({
                "propertyPath": "vectorfield.objectpath",
                "kind": "string",
                "value": vector_field_object_path,
            })
        literals.append({
            "propertyPath": "vectorfield.assetid",
            "kind": "string",
            "value": vector_field_asset_id,
        })
    literals.sort(key=lambda row: row["propertyPath"])
    distributions.sort(key=lambda row: row["propertyPath"])
    return literals, distributions


def build_source_recipe(
    index: SourceIndex,
    modules: list[SourceObject],
    renderer_shape: str,
    bursts: list[dict[str, Any]],
    semantic_overlay: dict[str, Any] | None = None,
) -> dict[str, Any]:
    required = first_module(modules, "particlemodulerequired")
    delay_distribution = distribution_float(index, required, "emitterdelay")
    emitter_delay = max(0.0, delay_distribution[2]) if delay_distribution else (
        max(0.0, finite_number(prop(required.properties, "emitterdelay")) or 0.0)
        if required else 0.0
    )
    emitter_duration = max(0.0, finite_number(prop(required.properties, "emitterduration")) or 0.0) if required else 0.0
    emitter_loops = max(0, int(finite_number(prop(required.properties, "emitterloops")) or 1)) if required else 1
    source_modules = []
    for module in modules:
        literals, distributions = flatten_source_properties(
            index, module, semantic_overlay
        )
        source_modules.append(
            {
                "stableId": module.source_id,
                "className": folded(module.class_name),
                "objectPath": module.object_path,
                "literals": literals,
                "distributions": distributions,
            }
        )
    return {
        "enabled": True,
        "rendererShape": renderer_shape,
        "emitterDelaySeconds": emitter_delay,
        "emitterDurationSeconds": emitter_duration,
        "emitterLoopCount": emitter_loops,
        "bursts": [
            {
                "timeSeconds": row["timeSeconds"],
                "countMinimum": row["count"] if row["countLow"] < 0 else row["countLow"],
                "countMaximum": row["count"],
            }
            for row in sorted(bursts, key=lambda item: (item["timeSeconds"], item["index"]))
        ],
        "modules": source_modules,
    }


def presentation_identity(
    event: dict[str, Any],
    source_object_path: str,
    profile_id: str,
    status: str,
    parameters: list[dict[str, Any]],
    occurrence_index: int = 0,
) -> dict[str, Any]:
    return {
        "enabled": True,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": profile_id,
        "status": status,
        "sourceObjectPath": source_object_path,
        "sourceActionCueId": str(event.get("sourceActionCueId") or ""),
        "sourceEventId": str(event.get("eventId") or ""),
        "sourceOccurrenceIndex": occurrence_index,
        "sourceTimeSeconds": finite_number(
            event.get("globalTimeSeconds")
        ) or 0.0,
        "parameters": parameters,
    }


def source_start_color(
    index: SourceIndex, modules: list[SourceObject]
) -> tuple[list[float], list[dict[str, Any]]]:
    module = first_module(modules, "particlemodulecolor")
    color = distribution_vector(index, module, "startcolor")
    alpha = distribution_float(index, module, "startalpha")
    rgba = [1.0, 1.0, 1.0, 1.0]
    parameters: list[dict[str, Any]] = []
    if color is not None:
        rgba[:3] = [float(value) for value in color[2][:3]]
        parameters.append(source_parameter(
            "particleStartColor", "vector", "source_distribution",
            f"{module.object_path}.StartColor", [*rgba[:3], 0.0],
        ))
    if alpha is not None:
        rgba[3] = float(alpha[2])
        parameters.append(source_parameter(
            "particleStartAlpha", "number", "source_distribution",
            f"{module.object_path}.StartAlpha", rgba[3],
        ))
    return rgba, parameters


def first_distribution_number(row: dict[str, Any]) -> float | None:
    keys = row.get("keys")
    if isinstance(keys, list) and keys:
        minimum = keys[0].get("minimum")
        if isinstance(minimum, list) and minimum:
            return finite_number(minimum[0])
    table = row.get("lookupTable")
    if isinstance(table, list) and len(table) >= 3:
        return finite_number(table[2])
    minimum = row.get("defaultMinimum")
    if isinstance(minimum, list) and minimum:
        return finite_number(minimum[0])
    return None


def source_dynamic_parameters(
    source_recipe: dict[str, Any]
) -> tuple[dict[str, float], list[dict[str, Any]]]:
    values: dict[str, float] = {}
    parameters: list[dict[str, Any]] = []
    name_pattern = re.compile(r"dynamicparams\[(\d+)\]\.paramname$")
    for module in source_recipe.get("modules", []):
        if folded(module.get("className")) != "particlemoduleparameterdynamic":
            continue
        names: dict[int, str] = {}
        for literal in module.get("literals", []):
            match = name_pattern.fullmatch(
                str(literal.get("propertyPath") or "")
            )
            if match and str(literal.get("value") or ""):
                names[int(match.group(1))] = str(literal["value"])
        distributions = {
            str(row.get("propertyPath") or ""): row
            for row in module.get("distributions", [])
        }
        for parameter_index, parameter_name in sorted(names.items()):
            property_path = f"dynamicparams[{parameter_index}].paramvalue"
            distribution = distributions.get(property_path)
            value = (
                first_distribution_number(distribution)
                if distribution is not None else None
            )
            if value is None:
                parameters.append(source_parameter(
                    parameter_name, "number", "unresolved_class_default",
                    f"{module.get('objectPath', '')}.{property_path}",
                ))
                continue
            values[folded(parameter_name)] = value
            parameters.append(source_parameter(
                parameter_name, "number", "source_distribution",
                f"{module.get('objectPath', '')}.{property_path}", value,
            ))
    return values, parameters


def referenced_point_light_component(
    index: SourceIndex, modules: list[SourceObject]
) -> tuple[SourceObject | None, str]:
    light_modules = [
        module for module in modules
        if "typedatalight" in folded(module.class_name)
    ]
    if len(light_modules) != 1:
        raise ValueError(
            "Cascade light emitter must own exactly one TypeDataLight module"
        )
    module = light_modules[0]
    target = index.referenced(module, "pointlightcomponent")
    paths = sorted({
        str(object_path)
        for property_name, object_path in module.reference_paths
        if folded(property_name) == "pointlightcomponent"
        and str(object_path)
    }, key=str.casefold)
    if len(paths) != 1:
        return None, ""
    if target is None:
        target = index.get_path(paths[0])
    if target is None:
        package = module.object_path.split(".", 1)[0]
        target = index.get_path(f"{package}.{paths[0]}")
    if target is None or folded(target.class_name) != "pointlightcomponent":
        return None, paths[0]
    return target, target.object_path


def point_light_presentation(
    index: SourceIndex,
    modules: list[SourceObject],
    detail: dict[str, Any],
    event: dict[str, Any],
    occurrence_index: int = 0,
) -> dict[str, Any]:
    component, source_path = referenced_point_light_component(index, modules)
    runtime = detail["light"]
    runtime["profileId"] = "light.point.reconstructed.v1"
    runtime["status"] = "reconstructed_profile"
    if component is None:
        runtime["enabled"] = False
        parameters = [source_parameter(
            "pointLightComponent", "string", "unresolved_class_default",
            "TypeDataLight.PointLightComponent", source_path,
        )]
        presentation = presentation_identity(
            event, source_path, runtime["profileId"], "unresolved",
            parameters, occurrence_index,
        )
        presentation["enabled"] = False
        return presentation

    parameters: list[dict[str, Any]] = []

    def explicit_number(property_name: str, runtime_name: str) -> float | None:
        item = component.properties.get(property_name)
        value = finite_number(unwrap(item)) if item is not None else None
        parameters.append(source_parameter(
            runtime_name,
            "number",
            "source_explicit" if value is not None
            else "unresolved_class_default",
            f"{component.object_path}.{property_name}",
            value,
        ))
        return value

    brightness = explicit_number("brightness", "brightness")
    radius_ue = explicit_number("radius", "radiusUeUnits")
    falloff = explicit_number("falloffexponent", "falloffExponent")
    light_color_item = component.properties.get("lightcolor")
    light_color = unwrap(light_color_item) if light_color_item else None
    explicit_color: list[float] | None = None
    if isinstance(light_color, dict) and all(
        finite_number(light_color.get(channel)) is not None
        for channel in ("r", "g", "b", "a")
    ):
        explicit_color = [
            float(light_color[channel]) / 255.0
            for channel in ("r", "g", "b", "a")
        ]
    parameters.append(source_parameter(
        "lightColor", "vector",
        "source_explicit" if explicit_color is not None
        else "unresolved_class_default",
        f"{component.object_path}.lightcolor", explicit_color,
    ))
    parameters.append(source_parameter(
        "ambientColor", "vector", "unresolved_class_default",
        "PointLightComponent class default", None,
    ))

    source_color, source_color_parameters = source_start_color(index, modules)
    parameters.extend(source_color_parameters)
    size = detail["particle"]["startSize"]
    particle_range = max(float(value) for value in size)
    parameters.append(source_parameter(
        "particleStartSizeRange", "number", "source_distribution",
        "ParticleModuleSize.StartSize|maxAxis|UE_TO_RUNTIME_0.01",
        particle_range,
    ))

    runtime.update({
        "enabled": brightness is not None,
        "range": (
            max(0.0, radius_ue * SOURCE_UNITS_TO_RUNTIME)
            if radius_ue is not None else max(0.0, particle_range)
        ),
        "intensity": max(0.0, brightness or 0.0),
        "color": explicit_color or source_color,
        "ambient": [0.0, 0.0, 0.0, 0.0],
        "falloffExponent": max(0.0, falloff or 0.0),
    })
    required_exact = (
        brightness is not None
        and radius_ue is not None
        and falloff is not None
        and explicit_color is not None
    )
    return presentation_identity(
        event,
        component.object_path,
        runtime["profileId"],
        "source_exact" if required_exact else "unresolved",
        parameters,
        occurrence_index,
    )


def screen_post_presentation(
    source_system_id: str,
    source_recipe: dict[str, Any],
    detail: dict[str, Any],
    event: dict[str, Any],
    source_object_path: str,
    occurrence_index: int = 0,
) -> dict[str, Any]:
    runtime = detail["screenPost"]
    contract = SCREEN_POST_RUNTIME_PROFILES.get(folded(source_system_id))
    if contract is None:
        parameters = [source_parameter(
            "sourceSystemId", "string", "unresolved_class_default",
            "ParticleSystem.objectPath", source_system_id,
        )]
        presentation = presentation_identity(
            event, source_object_path, "", "unresolved", parameters,
            occurrence_index,
        )
        presentation["enabled"] = False
        return presentation

    profile_id, subtype = contract
    dynamic_values, parameters = source_dynamic_parameters(source_recipe)
    parameters.insert(0, source_parameter(
        "sourceSubtype", "string", "source_explicit",
        "ParticleSystem.objectPath", subtype,
    ))
    tint = [1.0, 1.0, 1.0, 1.0]
    runtime_enabled = True
    presentation_status = "reconstructed"
    if subtype == "RGB_NOISE":
        intensity = max(0.0, dynamic_values.get("rgb_str", 0.0))
        # ``powerx`` remains in source provenance, but the decoded graph does
        # not prove that it is an additive full-screen noise gain.  The finite
        # runtime profile reconstructs only the RGB channel separation.
        secondary = 0.0
    elif subtype.startswith("ZOOM_BLUR"):
        intensity = max(0.0, dynamic_values.get("blurstrength", 0.0))
        secondary = 0.0
    else:
        # FilmNoise has neither a DynamicParameter nor decoded parent-graph
        # gain.  Inventing unit gain exposes full-screen static, so retain the
        # occurrence as evidence and fail closed at runtime.
        intensity = 0.0
        secondary = 0.0
        runtime_enabled = False
        presentation_status = "unresolved"
    runtime.update({
        "enabled": runtime_enabled,
        "profileId": profile_id,
        "status": "reconstructed_profile",
        "intensity": intensity,
        "secondaryIntensity": secondary,
        "frequency": 0.0,
        "tint": tint,
        "randomSeed": int(detail["particle"]["randomSeed"]),
    })
    presentation = presentation_identity(
        event, source_object_path, profile_id, presentation_status, parameters,
        occurrence_index,
    )
    presentation["enabled"] = runtime_enabled
    return presentation


def classify(system: dict[str, Any], modules: list[SourceObject]) -> tuple[str | None, str | None, str | None]:
    classes = [folded(module.class_name) for module in modules]
    if any("typedatalight" in name for name in classes):
        return "light", None, "light"
    if folded(system.get("sourceSystemId")).startswith("fx_post"):
        return "screenPost", None, "screenPost"
    if any("typedatadecal" in name for name in classes):
        return "decal", None, "decal"
    if any("typedatamesh" in name for name in classes):
        return "particle", None, "mesh"
    return "particle", None, "sprite"


def mesh_uses_model_material(
    modules: list[SourceObject], detail_mappings: list[dict[str, Any]],
) -> bool:
    mesh_modules = [
        module for module in modules
        if folded(module.class_name) == "particlemoduletypedatamesh"
    ]
    if len(mesh_modules) != 1:
        raise ValueError(
            "Cascade mesh emitter must own exactly one TypeDataMesh module"
        )
    module = mesh_modules[0]
    explicit = "boverridematerial" in module.properties
    override_value = prop(module.properties, "boverridematerial", False)
    if not isinstance(override_value, bool):
        raise ValueError("TypeDataMesh.bOverrideMaterial must be boolean")
    use_model_material = not override_value
    mapping(
        detail_mappings,
        "detail.mesh.useModelMaterial",
        "EXACT" if explicit else "SOURCE_CLASS_DEFAULT",
        f"{module.object_path}.bOverrideMaterial",
        use_model_material,
        "runtime useModelMaterial is the inverse of UE3 bOverrideMaterial",
    )
    return use_model_material


def build_document(
    source_receipt: dict[str, Any],
    graph: dict[str, Any],
    closure: dict[str, Any],
    semantic_overlay: dict[str, Any] | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    if int(source_receipt["skillId"]) != int(graph["skillId"]):
        raise ValueError("source receipt and normalized graph skillId differ")
    if int(graph["skillId"]) != int(closure["skillId"]):
        raise ValueError("normalized graph and external closure skillId differ")

    index = SourceIndex(graph, closure)
    events = event_index(source_receipt)
    used_ids: set[str] = set()
    elements: list[dict[str, Any]] = []
    conversions: list[dict[str, Any]] = []
    unsupported = []
    total_particle_budget = 0

    source_partitions = selected_lod_partitions(graph, index, closure)
    active_partitions = [
        partition
        for partition in source_partitions
        if folded(partition[0].get("sourceSystemId")) in events
    ]

    # A normalized graph can contain every variant referenced by the source
    # action package.  Only systems reached by the selected action timeline are
    # executable for this document.  Materializing an unreferenced system at
    # t=0 reactivates disabled FX variants (for example DimensionMaster A's
    # FX-07 branch) and is not a valid fallback.
    for partition_index, (system, emitter, lod, modules) in enumerate(
        active_partitions, start=1
    ):
        kind, unsupported_reason, renderer_shape = classify(system, modules)
        source_system_id = str(system["sourceSystemId"])
        source_events = events.get(folded(source_system_id), [])
        event = source_events[0] if source_events else {}
        event_time = finite_number(event.get("globalTimeSeconds")) or 0.0
        event_duration = finite_number(event.get("durationSeconds")) or 0.0
        module_ids = {
            lod.source_id,
            emitter.source_id,
            *(
                module.source_id.split("@ref:", 1)[0]
                for module in modules
            ),
        }
        resources, resource_receipt, material_rows = choose_resources(system, module_ids, graph)
        source_material_path = str(
            material_rows[0].get("sourceMaterialPath") or ""
        ) if material_rows else ""
        module_evidence = [
            {"className": module.class_name, "objectPath": module.object_path}
            for module in modules
        ]
        if unsupported_reason:
            row = {
                "sourceSystemId": source_system_id,
                "sourceEmitter": emitter.object_path,
                "sourceLod": lod.object_path,
                "reason": unsupported_reason,
                "moduleEvidence": module_evidence,
            }
            unsupported.append(row)
            conversions.append({**row, "status": "UNSUPPORTED", "elementIds": []})
            continue

        missing_resources = []
        if renderer_shape == "mesh" and not any(row["slotId"] == "meshModel" for row in resources):
            missing_resources.append("MESH_RENDERER_HAS_NO_RUNTIME_MESH_BINDING")
        source_material_pending = (
            renderer_shape not in {"mesh", "light", "screenPost"}
            and kind in {"particle", "decal"}
            and not any(row["slotId"] == "base" for row in resources)
            and bool(source_material_path)
        )
        if renderer_shape not in {"mesh", "light", "screenPost"} and kind in {"particle", "decal"} and not any(row["slotId"] == "base" for row in resources) and not source_material_pending:
            missing_resources.append("RENDERER_HAS_NO_RUNTIME_BASE_TEXTURE_BINDING")

        detail, detail_mappings, bursts = emitter_detail(index, lod, modules, event_time, event_duration, partition_index)
        apply_action_cue_transform(detail, event)
        profile = material_detail(material_rows, detail, detail_mappings)
        if renderer_shape == "mesh":
            detail["mesh"]["useModelMaterial"] = mesh_uses_model_material(
                modules, detail_mappings
            )
            detail["particle"]["billboard"] = False
        if kind == "decal":
            detail["particle"]["spawnRatePerSecond"] = 0.0
            detail["particle"]["burstCount"] = 0

        base_burst = sum(row["count"] for row in bursts if row["timeSeconds"] <= 0.00001)
        delayed_bursts = [row for row in bursts if row["timeSeconds"] > 0.00001]
        if kind == "particle":
            detail["particle"]["burstCount"] = 0

        source_recipe = build_source_recipe(
            index, modules, renderer_shape, bursts,
            semantic_overlay,
        )
        source_presentation = default_source_presentation()
        if renderer_shape == "light":
            source_presentation = point_light_presentation(
                index, modules, detail, event,
            )
        elif renderer_shape == "screenPost":
            source_presentation = screen_post_presentation(
                source_system_id,
                source_recipe,
                detail,
                event,
                emitter.object_path,
            )

        element_ids = []
        should_emit_base = True
        if should_emit_base:
            element_id = unique_id(f"{safe_slug(source_system_id, 55)}.{safe_slug(emitter.object_path.split('.')[-1], 45)}", used_ids)
            material_template = (
                "effect.source_material"
                if kind not in ("mesh", "light", "screenPost")
                and not any(row.get("slotId") in ("base", "meshModel") for row in resources)
                and source_material_path
                else "effect.standard"
            )
            elements.append(
                {
                    "id": element_id,
                    "displayName": emitter.object_path.split(".")[-1][:64],
                    "groupId": safe_slug(source_system_id, 120),
                    "sourceNode": f"{source_system_id}|{emitter.object_path}"[:256],
                    "visible": True,
                    "kind": kind,
                    "resources": resources,
                    "material": {
                        "templateId": material_template,
                        "sourceMaterialPath": source_material_path,
                        "renderProfile": profile,
                        "sourceProfile": {"enabled": False},
                    },
                    "actionCueAttachment": action_cue_attachment(event),
                    "detail": detail,
                    "sourceRecipe": source_recipe,
                    "sourcePresentation": source_presentation,
                }
            )
            element_ids.append(element_id)
            if kind == "particle":
                total_particle_budget += detail["particle"]["maxParticles"]

        # A Cascade ParticleSystem can be triggered more than once by one
        # animation timeline.  Its emitter recipe is shared, but every notify is
        # a distinct runtime occurrence.  Keeping only source_events[0] silently
        # dropped repeated impacts such as super_instance BGCrack_01.
        first_occurrence_element_ids = list(element_ids)
        for occurrence_index, occurrence in enumerate(source_events[1:], start=2):
            occurrence_time = (
                finite_number(occurrence.get("globalTimeSeconds")) or 0.0
            )
            time_offset = occurrence_time - event_time
            occurrence_id = str(
                occurrence.get("eventId") or f"occurrence-{occurrence_index}"
            )
            for source_element_id in first_occurrence_element_ids:
                source_element = next(
                    row for row in elements if row["id"] == source_element_id
                )
                duplicated = copy.deepcopy(source_element)
                duplicated["id"] = unique_id(
                    f"{source_element_id}.event_{safe_slug(occurrence_id, 32)}",
                    used_ids,
                )
                duplicated["groupId"] = safe_slug(
                    f"{source_element['groupId']}.{occurrence_id}", 120
                )
                duplicated["sourceNode"] = (
                    f"{source_element['sourceNode']}|event:{occurrence_id}"[:256]
                )
                duplicated["detail"]["timing"]["startDelaySeconds"] += time_offset
                apply_action_cue_transform(
                    duplicated["detail"], occurrence, event
                )
                duplicated["actionCueAttachment"] = action_cue_attachment(
                    occurrence
                )
                duplicated_presentation = duplicated["sourcePresentation"]
                duplicated_presentation["sourceActionCueId"] = str(
                    occurrence.get("sourceActionCueId") or ""
                )
                duplicated_presentation["sourceEventId"] = str(
                    occurrence.get("eventId") or ""
                )
                duplicated_presentation["sourceOccurrenceIndex"] = (
                    occurrence_index - 1
                )
                duplicated_presentation["sourceTimeSeconds"] = occurrence_time
                elements.append(duplicated)
                element_ids.append(duplicated["id"])
                if kind == "particle":
                    total_particle_budget += duplicated["detail"]["particle"][
                        "maxParticles"
                    ]

        unrepresented = [
            {
                "className": module.class_name,
                "objectPath": module.object_path,
                "reason": "CURRENT_EFFECT_DOCUMENT_HAS_NO_EQUIVALENT_MAPPING",
            }
            for module in modules
            if folded(module.class_name) not in REPRESENTED_DETAIL_MODULE_CLASSES
        ]
        conversions.append(
            {
                "sourceSystemId": source_system_id,
                "sourceEmitter": emitter.object_path,
                "sourceLod": lod.object_path,
                "targetKind": kind,
                "rendererShape": renderer_shape,
                "status": (
                    "MISSING_RESOURCE" if missing_resources
                    else "SOURCE_MATERIAL_RUNTIME_PENDING" if source_material_pending
                    else "SOURCE_RECIPE_RUNTIME_PENDING"
                ),
                "sourceMaterialRuntimePending": source_material_pending,
                "missingResources": missing_resources,
                "eventTimeSeconds": event_time,
                "eventOccurrences": [
                    {
                        "eventId": row.get("eventId"),
                        "globalTimeSeconds": row.get("globalTimeSeconds"),
                        "durationSeconds": row.get("durationSeconds"),
                    }
                    for row in source_events
                ],
                "elementIds": element_ids,
                "resourceMappings": resource_receipt,
                "detailMappings": detail_mappings,
                "burstSource": bursts,
                "moduleEvidence": module_evidence,
                "unrepresentedModules": unrepresented,
                "presentationSourceStatus": source_presentation["status"],
                "presentationProfileId": source_presentation["profileId"],
                "materialParameterEvidence": [
                    {
                        "sourceMaterialPath": row.get("sourceMaterialPath"),
                        "scalars": row.get("scalars", []),
                        "vectors": row.get("vectors", []),
                    }
                    for row in material_rows
                ],
            }
        )

    if len(elements) > MAX_ELEMENTS:
        raise ValueError(f"converted Effect document has {len(elements)} elements; max is {MAX_ELEMENTS}")
    if total_particle_budget > MAX_DOCUMENT_PARTICLES:
        raise ValueError(f"converted Effect document particle budget is {total_particle_budget}; max is {MAX_DOCUMENT_PARTICLES}")

    character_class = str(source_receipt["characterClass"])
    skill_id = int(source_receipt["skillId"])
    input_slot = str(source_receipt.get("inputSlot") or "Unbound")
    document = {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": f"effect.{character_class.casefold()}.skill.{skill_id}.imported",
        "displayName": f"{character_class} {input_slot} {skill_id} Imported Cascade Draft",
        "particleSystem": {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        },
        "modelCues": [],
        "elements": elements,
    }
    receipt = {
        "schema": "lostark.imported-effect-element-conversion-receipt",
        "schemaVersion": 1,
        "characterClass": character_class,
        "skillId": skill_id,
        "inputSlot": source_receipt.get("inputSlot"),
        "sourcePolicy": {
            "partition": "ParticleSystem/Emitter/FIRST_LOD",
            "coordinateScale": SOURCE_UNITS_TO_RUNTIME,
            "curvePolicy": "v9 preserves baked lookup tables and referenced curve keys; v8 Detail remains a compatibility preview",
            "materialSlotPolicy": "Material Instance parameter binding first, parameter-name heuristic second",
        },
        "documentEffectAssetId": document["effectAssetId"],
        "elementConversions": conversions,
        "unsupportedEmitters": unsupported,
        "sourceUnsupportedOrUnresolved": source_receipt.get("unsupportedUnresolved", []),
        "manualTuningBoundaries": [
            "Mesh particle emitters use the thin particle+meshModel path; per-particle mesh rotation, orbit and full 3D size curves still require tuning or later expansion.",
            "Cascade curves are collapsed to the current Detail panel's start/end or min/max fields.",
            "Simple StartLocation ranges map to particle spawn position min/max; sphere/cylinder volumes, orbit, camera offset, seeded distributions and dynamic parameters remain source evidence.",
            "Material texture slots use original parameter bindings, but Base/Noise/Mask/Emissive/Dissolve semantic assignment needs thumbnail review.",
            "Procedural source materials preserve sourceMaterialPath and use effect.source_material; expression-graph execution remains fail-closed in coverage until implemented.",
            "UE source position, velocity, acceleration and size use a 0.01 project-scale assumption and require visual scale verification.",
            "Point-light and known screen-post emitters carry finite reconstructed runtime profiles plus lossless source provenance; unknown source semantics remain unresolved and fail closed.",
            "Source camera shake and assetless Effect notifies remain separate typed presentation-channel work.",
            "Dimension Summon model animation spawn/synchronization is a presentation cue concern and is not encoded as a particle Element.",
        ],
        "summary": {
            "sourceEmitterPartitionCount": len(active_partitions),
            "graphEmitterPartitionCount": len(source_partitions),
            "inactiveSourceSystemEmitterPartitionCount": (
                len(source_partitions) - len(active_partitions)
            ),
            "emittedElementCount": len(elements),
            "convertedEmitterCount": sum(bool(row.get("elementIds")) for row in conversions),
            "unsupportedEmitterCount": len(unsupported),
            "missingResourceEmitterCount": sum(bool(row.get("missingResources")) for row in conversions),
            "sourceMaterialPendingEmitterCount": sum(bool(row.get("sourceMaterialRuntimePending")) for row in conversions),
            "presentationSourceExactEmitterCount": sum(
                row.get("presentationSourceStatus") == "source_exact"
                for row in conversions
            ),
            "presentationReconstructedEmitterCount": sum(
                row.get("presentationSourceStatus") == "reconstructed"
                for row in conversions
            ),
            "presentationUnresolvedEmitterCount": sum(
                row.get("presentationSourceStatus") == "unresolved"
                and bool(row.get("presentationProfileId"))
                for row in conversions
            ),
            "particleBudget": total_particle_budget,
            "externalPackageCount": closure.get("summary", {}).get("packageCount", 0),
            "externalRequestCount": closure.get("summary", {}).get("requestCount", 0),
            "externalUnresolvedRequestCount": closure.get("summary", {}).get("unresolvedRequestCount", 0),
        },
    }
    return document, receipt


def promote_document(
    imported_document: dict[str, Any],
    effect_asset_id: str,
    display_name: str,
    model_cues: list[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    imported_id = str(imported_document.get("effectAssetId") or "")
    if not imported_id.endswith(".imported"):
        raise ValueError("promotion source Effect ID must end with .imported")
    if imported_id.removesuffix(".imported") != effect_asset_id:
        raise ValueError("promotion target must match the Imported Effect ID")
    if not effect_asset_id or not display_name.strip():
        raise ValueError("promotion target ID and display name are required")
    promoted = copy.deepcopy(imported_document)
    promoted["version"] = 12
    promoted["effectAssetId"] = effect_asset_id
    promoted["displayName"] = display_name
    promoted["particleSystem"] = copy.deepcopy(imported_document.get(
        "particleSystem",
        {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        },
    ))
    promoted["modelCues"] = copy.deepcopy(model_cues or [])
    return promoted


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--output-document", required=True, type=Path)
    parser.add_argument("--output-receipt", required=True, type=Path)
    parser.add_argument("--output-authored-document", type=Path)
    parser.add_argument("--authored-effect-asset-id")
    parser.add_argument("--authored-display-name")
    parser.add_argument("--model-cue-id")
    parser.add_argument("--model-cue-asset-id")
    parser.add_argument("--model-cue-clip")
    parser.add_argument("--model-cue-duration-seconds", type=float)
    parser.add_argument("--model-cue-local-position", nargs=3, type=float,
                        default=(0.0, 0.0, 0.0))
    parser.add_argument("--model-cue-local-rotation", nargs=3, type=float,
                        default=(0.0, 0.0, 0.0))
    parser.add_argument("--model-cue-local-scale", nargs=3, type=float,
                        default=(1.0, 1.0, 1.0))
    parser.add_argument("--model-cue-asset-pre-scale", nargs=3, type=float,
                        default=(1.0, 1.0, 1.0))
    parser.add_argument("--model-cue-asset-pre-rotation", nargs=3, type=float,
                        default=(0.0, 0.0, 0.0))
    args = parser.parse_args()

    document, receipt = build_document(
        read_json(args.source_receipt),
        read_json(args.normalized_graph),
        read_json(args.external_module_closure),
    )
    write_json_atomic(args.output_document, document)
    write_json_atomic(args.output_receipt, receipt)
    authored_output = None
    if args.output_authored_document is not None:
        if not args.authored_effect_asset_id or not args.authored_display_name:
            parser.error("Authored output requires target Effect ID and display name")
        cue_fields = (
            args.model_cue_id,
            args.model_cue_asset_id,
            args.model_cue_clip,
            args.model_cue_duration_seconds,
        )
        if any(value is not None for value in cue_fields) and not all(
            value is not None for value in cue_fields
        ):
            parser.error("Model Cue ID, asset, clip, and duration are all required")
        model_cues = []
        if all(value is not None for value in cue_fields):
            model_cues.append(
                {
                    "cueId": args.model_cue_id,
                    "modelAssetId": args.model_cue_asset_id,
                    "clipName": args.model_cue_clip,
                    "startDelaySeconds": 0.0,
                    "durationSeconds": args.model_cue_duration_seconds,
                    "visible": True,
                    "localTransform": {
                        "position": list(args.model_cue_local_position),
                        "rotationDegrees": list(args.model_cue_local_rotation),
                        "scale": list(args.model_cue_local_scale),
                    },
                    "assetPreTransform": {
                        "scale": list(args.model_cue_asset_pre_scale),
                        "rotationDegrees": list(args.model_cue_asset_pre_rotation),
                    },
                }
            )
        authored = promote_document(
            document,
            args.authored_effect_asset_id,
            args.authored_display_name,
            model_cues,
        )
        write_json_atomic(args.output_authored_document, authored)
        authored_output = str(args.output_authored_document)
    print(
        json.dumps(
            {
                "document": str(args.output_document),
                "receipt": str(args.output_receipt),
                "authoredDocument": authored_output,
                **receipt["summary"],
            },
            ensure_ascii=False,
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
