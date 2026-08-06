#!/usr/bin/env python3
"""Convert one normalized UE3 Cascade graph into a loadable Effect document.

The conversion is intentionally conservative.  It emits values that the current
v6 runtime can execute and records every approximation or unsupported source
module in a separate receipt.  The receipt, not a filename guess, is the audit
trail from an original Cascade emitter to an authored Element.
"""

from __future__ import annotations

import argparse
import copy
import json
import math
import re
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable


SOURCE_UNITS_TO_RUNTIME = 0.01
PROCEDURAL_MATERIAL_BASE_FALLBACK = (
    "fx_tex_00.fx_a_blankwhite_01",
    "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_blankwhite_01.dds",
)
MAX_ELEMENTS = 256
MAX_DOCUMENT_PARTICLES = 8192
MAX_PARTICLES_PER_IMPORTED_ELEMENT = 64
SUPPORTED_DETAIL_MODULE_TOKENS = (
    "required",
    "spawn",
    "lifetime",
    "velocity",
    "acceleration",
    "size",
    "color",
    "subuv",
    "location",
    "rotation",
    "orbit",
    "orientation",
    "cameraoffset",
    "parameterdynamic",
)


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
            if source is not None and target is not None:
                source.references.append((str(edge.get("property") or ""), target.key))

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
                    target_key = object_path.casefold()
                    if not target_key.startswith(logical_package.casefold() + "."):
                        target_key = f"{logical_package}.{object_path}".casefold()
                    if target_key in self.objects:
                        source.references.append(
                            (str(reference.get("property") or ""), target_key)
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
    table = prop(raw, "lookuptable", [])
    values = [number for item in table if (number := finite_number(item)) is not None]
    if values:
        return min(values), max(values), values[0], values[-1], "APPROXIMATION"

    target = distribution_target(index, source, property_name)
    if target is None:
        return None
    class_name = folded(target.class_name)
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


def table_vector_samples(raw: dict[str, Any]) -> list[list[float]]:
    table = prop(raw, "lookuptable", [])
    values = [number for item in table if (number := finite_number(item)) is not None]
    if len(values) < 3:
        return []
    chunk_size = int(finite_number(prop(raw, "lookuptablechunksize")) or 0)
    if chunk_size >= 3 and len(values) >= chunk_size:
        payload = values[-chunk_size:]
        if chunk_size % 3 == 0:
            return [payload[index : index + 3] for index in range(0, chunk_size, 3)]
    if len(values) % 4 == 0:
        return [values[index : index + 3] for index in range(0, len(values), 4)]
    usable = len(values) - len(values) % 3
    return [values[index : index + 3] for index in range(0, usable, 3)]


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
    samples = table_vector_samples(raw)
    if samples:
        minimum = [min(sample[axis] for sample in samples) for axis in range(3)]
        maximum = [max(sample[axis] for sample in samples) for axis in range(3)]
        return minimum, maximum, samples[0], samples[-1], "APPROXIMATION"

    target = distribution_target(index, source, property_name)
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

    local_space = bool(prop(required.properties, "buselocalspace", True)) if required else True
    detail["particle"]["localSpace"] = local_space
    mapping(mappings, "particle.localSpace", "EXACT", "Required.bUseLocalSpace", local_space)

    duration = finite_number(prop(required.properties, "emitterduration")) if required else None
    delay_distribution = distribution_float(index, required, "emitterdelay")
    emitter_delay = max(0.0, delay_distribution[2]) if delay_distribution else 0.0
    detail["timing"]["startDelaySeconds"] = max(0.0, event_time + emitter_delay)
    mapping(
        mappings,
        "timing.startDelaySeconds",
        "EXACT" if not delay_distribution else delay_distribution[4],
        "animation event + Required.EmitterDelay",
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

    timing_life = max(duration or 0.0, event_duration, life_max, 0.1)
    detail["timing"]["lifeTimeSeconds"] = timing_life
    mapping(mappings, "timing.lifeTimeSeconds", "APPROXIMATION", "max(EmitterDuration, notify duration, particle lifetime)", timing_life)

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
        minimum = [value * SOURCE_UNITS_TO_RUNTIME for value in velocity[0]]
        maximum = [value * SOURCE_UNITS_TO_RUNTIME for value in velocity[1]]
        detail["particle"]["initialVelocityMin"] = minimum
        detail["particle"]["initialVelocityMax"] = maximum
        mapping(mappings, "particle.initialVelocityMin/Max", "APPROXIMATION", f"{velocity_module.object_path}.StartVelocity", [minimum, maximum], "UE source units scaled by 0.01")

    acceleration_module = first_module(modules, "particlemoduleacceleration")
    acceleration = distribution_vector(index, acceleration_module, "acceleration")
    if acceleration:
        value = [component * SOURCE_UNITS_TO_RUNTIME for component in acceleration[2]]
        detail["particle"]["acceleration"] = value
        mapping(mappings, "particle.acceleration", "APPROXIMATION", f"{acceleration_module.object_path}.Acceleration", value, "random/curve range collapsed to one vector")

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
        detail["color"]["multiply"] = rgba
        mapping(mappings, "color.multiply", "APPROXIMATION", f"{color_module.object_path}.StartColor/StartAlpha", rgba)

    over_life = first_module(modules, "particlemodulecoloroverlife") or first_module(modules, "particlemodulecolorscaleoverlife")
    if over_life:
        color_property = "coloroverlife" if "coloroverlife" in folded(over_life.class_name) else "colorscaleoverlife"
        alpha_property = "alphaoverlife" if "coloroverlife" in folded(over_life.class_name) else "alphascaleoverlife"
        end_color = distribution_vector(index, over_life, color_property)
        end_alpha = distribution_float(index, over_life, alpha_property)
        if end_color or end_alpha:
            current = detail["color"]["multiply"]
            rgba = [max(0.0, value) for value in (end_color[3] if end_color else current[:3])] + [max(0.0, end_alpha[3] if end_alpha else current[3])]
            detail["linearLerp"]["colorMultiply"] = True
            detail["linearLerp"]["endColorMultiply"] = rgba
            mapping(mappings, "linearLerp.endColorMultiply", "APPROXIMATION", over_life.object_path, rgba, "full Cascade curve collapsed to start/end linear lerp")

    location_module = first_module(modules, "particlemodulelocation")
    location = distribution_vector(index, location_module, "startlocation")
    if location:
        minimum = [value * SOURCE_UNITS_TO_RUNTIME for value in location[0]]
        maximum = [value * SOURCE_UNITS_TO_RUNTIME for value in location[1]]
        detail["particle"]["initialPositionMin"] = minimum
        detail["particle"]["initialPositionMax"] = maximum
        mapping(mappings, "particle.initialPositionMin/Max", "EXACT_RANGE", f"{location_module.object_path}.StartLocation", [minimum, maximum], "UE source units scaled by 0.01")

    subuv = first_module(modules, "particlemodulesubuv")
    if subuv:
        horizontal = int(finite_number(prop(required.properties, "subimages_horizontal")) or 1) if required else 1
        vertical = int(finite_number(prop(required.properties, "subimages_vertical")) or 1) if required else 1
        if horizontal > 1 or vertical > 1:
            detail["uv"]["sequence"] = True
            detail["uv"]["tileColumns"] = max(1, horizontal)
            detail["uv"]["tileRows"] = max(1, vertical)
            detail["uv"]["sequenceTerm"] = max(0.001, life_max / (horizontal * vertical))
            mapping(mappings, "uv.sequence/tileColumns/tileRows", "APPROXIMATION", f"{required.object_path}.SubImages", [horizontal, vertical])

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


def texture_slot(parameter_name: str) -> str:
    value = folded(parameter_name)
    if "dissolve" in value:
        return "dissolve"
    if any(token in value for token in ("mask", "opacity", "alpha")):
        return "mask"
    if any(token in value for token in ("noise", "distort")):
        return "noise"
    if any(token in value for token in ("emiss", "glow", "bloom")):
        return "emissive"
    return "base"


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
                texture_candidates.append((str(texture.get("name") or ""), texture_path, texture_asset))

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
            0 if texture_slot(row[0]) == "base" else 1,
            0 if any(token in folded(row[0]) for token in ("main", "base", "diffuse", "texture")) else 1,
            folded(row[0]),
            folded(row[2]),
        ),
    )
    for parameter, source_path, asset_id in ranked:
        if asset_id in used_assets:
            continue
        slot = "base" if "base" not in used_slots else texture_slot(parameter)
        if slot in used_slots:
            for fallback in ("base", "noise", "mask", "emissive", "dissolve"):
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
    for row in scalar_rows:
        name = folded(row.get("name"))
        value = finite_number(row.get("value"))
        if value is None:
            continue
        if any(token in name for token in ("emissive", "bloom", "glowpower", "intensity")):
            detail["color"]["emissiveIntensity"] = max(0.0, value)
            mapping(mappings, "color.emissiveIntensity", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), max(0.0, value))
        elif "distort" in name and any(token in name for token in ("power", "intensity", "strength")):
            detail["color"]["distortionIntensity"] = max(0.0, value)
            detail["color"]["distortionOnBaseMaterial"] = True
            mapping(mappings, "color.distortionIntensity", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), max(0.0, value))
        elif any(token in name for token in ("panning_x", "pan_x", "uspeed", "speed_u")):
            detail["uv"]["speed"][0] = value
            mapping(mappings, "uv.speed.x", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), value)
        elif any(token in name for token in ("panning_y", "pan_y", "vspeed", "speed_v")):
            detail["uv"]["speed"][1] = value
            mapping(mappings, "uv.speed.y", "PARAMETER_NAME_HEURISTIC", str(row.get("name")), value)
    return profile


def selected_lod_partitions(
    graph: dict[str, Any],
    index: SourceIndex,
    closure: dict[str, Any],
) -> list[tuple[dict[str, Any], SourceObject, SourceObject, list[SourceObject]]]:
    external_by_lod: dict[str, list[SourceObject]] = defaultdict(list)
    for package in closure.get("packages", []):
        for request in package.get("requestedReferences", []):
            record = index.get_path(request.get("objectPath"))
            if record is not None:
                external_by_lod[str(request.get("sourceNodeId"))].append(record)

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
            modules = []
            for edge in index.outgoing.get(lod.source_id, []):
                target = index.get_id(str(edge.get("targetNodeId") or ""))
                if target is not None:
                    modules.append(target)
            modules.extend(external_by_lod.get(lod.source_id, []))
            deduplicated = {module.key: module for module in modules}
            partitions.append((system, emitter, lod, list(deduplicated.values())))
    return partitions


def classify(system: dict[str, Any], modules: list[SourceObject]) -> tuple[str | None, str | None, str | None]:
    classes = [folded(module.class_name) for module in modules]
    if any("typedatalight" in name for name in classes):
        return None, "LIGHT_RENDERER_NOT_SUPPORTED", None
    if folded(system.get("sourceSystemId")).startswith("fx_post"):
        return None, "SCREEN_SPACE_POST_EFFECT_NOT_SUPPORTED", None
    if any("typedatadecal" in name for name in classes):
        return "decal", None, "decal"
    if any("typedatamesh" in name for name in classes):
        return "particle", None, "mesh"
    return "particle", None, "sprite"


def build_document(
    source_receipt: dict[str, Any],
    graph: dict[str, Any],
    closure: dict[str, Any],
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

    for partition_index, (system, emitter, lod, modules) in enumerate(
        selected_lod_partitions(graph, index, closure), start=1
    ):
        kind, unsupported_reason, renderer_shape = classify(system, modules)
        source_system_id = str(system["sourceSystemId"])
        source_events = events.get(folded(source_system_id), [])
        event = source_events[0] if source_events else {}
        event_time = finite_number(event.get("globalTimeSeconds")) or 0.0
        event_duration = finite_number(event.get("durationSeconds")) or 0.0
        module_ids = {lod.source_id, emitter.source_id, *(module.source_id for module in modules)}
        resources, resource_receipt, material_rows = choose_resources(system, module_ids, graph)
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

        if renderer_shape == "mesh" and not any(row["slotId"] == "meshModel" for row in resources):
            reason = "MESH_RENDERER_HAS_NO_RUNTIME_MESH_BINDING"
            unsupported.append({"sourceSystemId": source_system_id, "sourceEmitter": emitter.object_path, "reason": reason})
            conversions.append({"sourceSystemId": source_system_id, "sourceEmitter": emitter.object_path, "status": "UNSUPPORTED", "reason": reason, "elementIds": [], "moduleEvidence": module_evidence})
            continue
        if renderer_shape != "mesh" and kind in {"particle", "decal"} and not any(row["slotId"] == "base" for row in resources):
            reason = "RENDERER_HAS_NO_RUNTIME_BASE_TEXTURE_BINDING"
            unsupported.append({"sourceSystemId": source_system_id, "sourceEmitter": emitter.object_path, "reason": reason})
            conversions.append({"sourceSystemId": source_system_id, "sourceEmitter": emitter.object_path, "status": "UNSUPPORTED", "reason": reason, "elementIds": [], "moduleEvidence": module_evidence})
            continue

        detail, detail_mappings, bursts = emitter_detail(index, lod, modules, event_time, event_duration, partition_index)
        profile = material_detail(material_rows, detail, detail_mappings)
        if renderer_shape == "mesh":
            detail["mesh"]["useModelMaterial"] = not any(row["slotId"] == "base" for row in resources)
            detail["particle"]["billboard"] = False
        if kind == "decal":
            detail["particle"]["spawnRatePerSecond"] = 0.0
            detail["particle"]["burstCount"] = 0

        base_burst = sum(row["count"] for row in bursts if row["timeSeconds"] <= 0.00001)
        delayed_bursts = [row for row in bursts if row["timeSeconds"] > 0.00001]
        if kind == "particle":
            detail["particle"]["burstCount"] = min(base_burst, detail["particle"]["maxParticles"])
            total_particle_budget += detail["particle"]["maxParticles"]

        element_ids = []
        should_emit_base = kind != "particle" or detail["particle"]["spawnRatePerSecond"] > 0.0 or base_burst > 0 or not delayed_bursts
        if should_emit_base:
            element_id = unique_id(f"{safe_slug(source_system_id, 55)}.{safe_slug(emitter.object_path.split('.')[-1], 45)}", used_ids)
            elements.append(
                {
                    "id": element_id,
                    "displayName": emitter.object_path.split(".")[-1][:64],
                    "groupId": safe_slug(source_system_id, 120),
                    "sourceNode": f"{source_system_id}|{emitter.object_path}"[:256],
                    "visible": True,
                    "kind": kind,
                    "resources": resources,
                    "material": {"templateId": "effect.standard", "renderProfile": profile},
                    "detail": detail,
                }
            )
            element_ids.append(element_id)

        if kind == "particle":
            for burst in delayed_bursts:
                burst_detail = copy.deepcopy(detail)
                burst_detail["timing"]["startDelaySeconds"] += burst["timeSeconds"]
                burst_detail["timing"]["lifeTimeSeconds"] = max(burst_detail["particle"]["lifeTimeSeconds"][1], 0.1)
                burst_detail["particle"]["spawnRatePerSecond"] = 0.0
                burst_detail["particle"]["burstCount"] = min(burst["count"], burst_detail["particle"]["maxParticles"])
                element_id = unique_id(f"{safe_slug(source_system_id, 48)}.{safe_slug(emitter.object_path.split('.')[-1], 42)}.burst_{burst['index']}", used_ids)
                elements.append(
                    {
                        "id": element_id,
                        "displayName": f"{emitter.object_path.split('.')[-1]} Burst {burst['index']}"[:64],
                        "groupId": safe_slug(source_system_id, 120),
                        "sourceNode": f"{source_system_id}|{emitter.object_path}|burst:{burst['index']}"[:256],
                        "visible": True,
                        "kind": kind,
                        "resources": resources,
                        "material": {"templateId": "effect.standard", "renderProfile": profile},
                        "detail": burst_detail,
                    }
                )
                element_ids.append(element_id)
                total_particle_budget += burst_detail["particle"]["maxParticles"]

        represented_tokens = {
            token for token in SUPPORTED_DETAIL_MODULE_TOKENS
            if any(token in folded(module.class_name) for module in modules)
        }
        unrepresented = [
            {"className": module.class_name, "objectPath": module.object_path}
            for module in modules
            if not any(token in folded(module.class_name) for token in represented_tokens)
            and "typedata" not in folded(module.class_name)
        ]
        conversions.append(
            {
                "sourceSystemId": source_system_id,
                "sourceEmitter": emitter.object_path,
                "sourceLod": lod.object_path,
                "targetKind": kind,
                "rendererShape": renderer_shape,
                "status": "APPROXIMATION_REQUIRES_VISUAL_TUNING" if renderer_shape == "mesh" or kind == "decal" or any(row["status"] != "EXACT" for row in detail_mappings) else "EXACT",
                "eventTimeSeconds": event_time,
                "elementIds": element_ids,
                "resourceMappings": resource_receipt,
                "detailMappings": detail_mappings,
                "burstSource": bursts,
                "moduleEvidence": module_evidence,
                "unrepresentedModules": unrepresented,
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
    document = {
        "schema": "lostark.effect-authoring",
        "version": 6,
        "effectAssetId": f"effect.{character_class.casefold()}.skill.{skill_id}.imported",
        "displayName": f"DimensionMaster F {skill_id} Imported Portal Draft",
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
            "curvePolicy": "start/end or range approximation; raw source remains in graph/module closure",
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
            "Procedural source materials without a texture parameter use fx_a_blankwhite_01 as an explicit preview fallback; their shape cannot be recovered by texture binding alone.",
            "UE source position, velocity, acceleration and size use a 0.01 project-scale assumption and require visual scale verification.",
            "Light renderer, screen-space post process, source camera shake and assetless Effect notifies stay outside this Effect Document.",
            "Dimension Summon model animation spawn/synchronization is a presentation cue concern and is not encoded as a particle Element.",
        ],
        "summary": {
            "sourceEmitterPartitionCount": len(selected_lod_partitions(graph, index, closure)),
            "emittedElementCount": len(elements),
            "convertedEmitterCount": sum(bool(row.get("elementIds")) for row in conversions),
            "unsupportedEmitterCount": len(unsupported),
            "particleBudget": total_particle_budget,
            "externalPackageCount": closure.get("summary", {}).get("packageCount", 0),
            "externalRequestCount": closure.get("summary", {}).get("requestCount", 0),
            "externalUnresolvedRequestCount": closure.get("summary", {}).get("unresolvedRequestCount", 0),
        },
    }
    return document, receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--output-document", required=True, type=Path)
    parser.add_argument("--output-receipt", required=True, type=Path)
    args = parser.parse_args()

    document, receipt = build_document(
        read_json(args.source_receipt),
        read_json(args.normalized_graph),
        read_json(args.external_module_closure),
    )
    write_json_atomic(args.output_document, document)
    write_json_atomic(args.output_receipt, receipt)
    print(
        json.dumps(
            {
                "document": str(args.output_document),
                "receipt": str(args.output_receipt),
                **receipt["summary"],
            },
            ensure_ascii=False,
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
