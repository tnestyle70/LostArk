#!/usr/bin/env python3
"""Recover UE3 light placements from a Lost Ark level package.

`extract_ue3_placements` already resolves, decrypts and parses these packages;
this tool reuses that reader and walks the light side of the level instead of
the static meshes.  It joins each light actor (`PointLightMovable`,
`SpotLightMovable`, ...) to its light component, and picks up the lights a
`StaticLightCollectionActor` holds, whose transform lives on the component's
own cached parent-to-world matrix rather than on an actor.

Output is a `lostark.map-light-presentation` formatVersion 2 authoring document
for `Data/Maps/Authoring/<AreaId>/`.  The map publisher validates it through
`Tools/RenderingPipeline/light_resources_pipeline.py`; nothing here writes to a
runtime folder.

Usage:
  python extract_ue3_map_lights.py --package <level.upk> --area-id <AreaId>
                                   --output <AreaId.maplights.json>
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import struct
import sys
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))
import extract_ue3_placements as ue3  # noqa: E402

# UE3 centimetres and Z-up become engine metres and Y-up, the same conversion
# build_maptool_scene applies to static mesh placements.
UNIT_SCALE = 0.01
POINT_CLASSES = ("pointlightcomponent",)
SPOT_CLASSES = ("spotlightcomponent",)
DIRECTIONAL_CLASSES = ("directionallightcomponent",)
# UE3 serialises no falloff for a light using its class default.
DEFAULT_FALLOFF_EXPONENT = 2.0
BRIGHTNESS_LIMIT = 64.0
RANGE_LIMIT_METRES = 1000.0


class LightExtractionError(RuntimeError):
    pass


def scalar(properties: dict[str, Any], name: str, default: float | None) -> float | None:
    entry = properties.get(name)
    if not isinstance(entry, dict):
        return default
    value = entry.get("value")
    if not isinstance(value, (int, float)) or not math.isfinite(float(value)):
        return default
    return float(value)


def boolean(properties: dict[str, Any], name: str, default: bool) -> bool:
    entry = properties.get(name)
    if not isinstance(entry, dict) or not isinstance(entry.get("value"), bool):
        return default
    return bool(entry["value"])


def colour(properties: dict[str, Any]) -> list[float]:
    entry = properties.get("lightcolor")
    if not isinstance(entry, dict) or not isinstance(entry.get("value"), dict):
        return [1.0, 1.0, 1.0, 1.0]
    value = entry["value"]
    channels = [float(value.get(key, 255)) / 255.0 for key in ("r", "g", "b")]
    # The source alpha marks an editor swatch, not an intensity, so the runtime
    # colour always carries a solid alpha and the brightness stays separate.
    return [min(max(channel, 0.0), 1.0) for channel in channels] + [1.0]


def location(properties: dict[str, Any]) -> tuple[float, float, float] | None:
    entry = properties.get("location")
    if not isinstance(entry, dict) or not isinstance(entry.get("value"), dict):
        return None
    value = entry["value"]
    try:
        x, y, z = (float(value[key]) for key in ("x", "y", "z"))
    except (KeyError, TypeError, ValueError):
        return None
    if not all(math.isfinite(component) for component in (x, y, z)):
        return None
    return (x * UNIT_SCALE, z * UNIT_SCALE, -y * UNIT_SCALE)


def property_stream_start(serial: bytes, names: list[str], version: int) -> int | None:
    """The offset the shared reader settles on: a Lost Ark export can carry an
    UnrealScript stack frame before its property tags, so the preamble is not a
    fixed size and has to be probed the same way."""
    minimum = 4 if version >= 322 else 0
    end = min(len(serial) - 20, 256)
    for offset in range(minimum, max(minimum, end) + 1):
        try:
            ue3.parse_tagged_properties_at(serial, names, offset)
        except (ue3.ExtractionError, struct.error, IndexError):
            continue
        return offset
    return None


def raw_struct_payload(serial: bytes, names: list[str], version: int, wanted: str) -> bytes | None:
    """Reads one struct property's bytes straight out of the tagged stream.

    The shared placement reader keeps only the first 32 bytes of a struct it has
    no decoder for, which is enough to identify one but drops the translation row
    of a 64-byte matrix.  Walking the stream again for this single property keeps
    that reader's behaviour, and its signature hashes, untouched.
    """
    start = property_stream_start(serial, names, version)
    if start is None:
        return None
    reader = ue3.Reader(serial, start)
    while reader.offset < len(serial):
        property_name, _ = ue3.parse_fname(reader, names)
        if property_name.casefold() == "none":
            return None
        property_type, _ = ue3.parse_fname(reader, names)
        data_size = reader.i32()
        reader.i32()  # array index
        type_key = property_type.casefold()
        if type_key == "structproperty":
            ue3.parse_fname(reader, names)
        elif type_key == "boolproperty":
            reader.read(1)
        elif type_key == "byteproperty":
            ue3.parse_fname(reader, names)
        size = data_size + 8 if type_key == "intproperty" else data_size
        payload = reader.read(size)
        if property_name.casefold() == wanted:
            return payload
    return None


def cached_translation(
    serial: bytes, names: list[str], version: int
) -> tuple[float, float, float] | None:
    """A light inside a StaticLightCollectionActor has no actor of its own; its
    world transform sits in the component's cached parent-to-world matrix."""
    blob = raw_struct_payload(serial, names, version, "cachedparenttoworld")
    if blob is None or len(blob) < 64:
        return None
    # The payload carries four bytes ahead of the 4x4, so the translation row
    # lands at 52.  Confirmed against a light whose actor also stores Location:
    # both give (-76724.1641, -17444.8184, -13771.8057) for pointlightmovable_0.
    x, y, z = struct.unpack_from("<3f", blob, 52)
    if not all(math.isfinite(component) for component in (x, y, z)):
        return None
    return (x * UNIT_SCALE, z * UNIT_SCALE, -y * UNIT_SCALE)


def rotation_degrees(properties: dict[str, Any]) -> list[float]:
    """UE3 aims a light down its rotator's forward axis; the engine aims a light
    down the Z axis of XMMatrixRotationRollPitchYaw(pitch, yaw, roll).  This
    converts the one into the other through the same axis swap the positions
    use, so a spot cone keeps pointing where the source pointed it."""
    entry = properties.get("rotation")
    if not isinstance(entry, dict) or not isinstance(entry.get("value"), dict):
        return [0.0, 0.0, 0.0]
    degrees = entry["value"].get("degrees")
    if not isinstance(degrees, dict):
        return [0.0, 0.0, 0.0]
    pitch = math.radians(float(degrees.get("pitch", 0.0)))
    yaw = math.radians(float(degrees.get("yaw", 0.0)))
    forward = (
        math.cos(pitch) * math.cos(yaw),
        math.cos(pitch) * math.sin(yaw),
        math.sin(pitch),
    )
    x, y, z = forward[0], forward[2], -forward[1]
    length = math.sqrt(x * x + y * y + z * z)
    if not math.isfinite(length) or length <= 1e-9:
        return [0.0, 0.0, 0.0]
    x, y, z = x / length, y / length, z / length
    out_pitch = math.degrees(math.asin(max(-1.0, min(1.0, -y))))
    out_yaw = math.degrees(math.atan2(x, z))
    return [round(out_pitch, 6), round(out_yaw, 6), 0.0]


def collect(package: Path, area_id: str, aes_key: str) -> dict[str, Any]:
    physical = package.read_bytes()
    summary = ue3.parse_summary(physical)
    logical = ue3.decompress_package(physical, summary, aes_key)
    names = ue3.parse_name_table(logical, summary)
    imports = ue3.parse_import_table(logical, summary, names)
    exports = ue3.parse_export_table(logical, summary, names)

    parsed: dict[int, dict[str, Any]] = {}
    serial_of: dict[int, bytes] = {}
    class_of: dict[int, str] = {}
    for entry in exports:
        class_name = ue3.package_ref_name(entry.class_index, imports, exports).casefold()
        class_of[entry.index] = class_name
        if "light" not in class_name or "lightmap" in class_name or entry.serial_size <= 0:
            continue
        serial = logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
        try:
            properties, _ = ue3.parse_tagged_properties(serial, names, summary.version)
        except ue3.ExtractionError:
            continue
        parsed[entry.index] = properties
        serial_of[entry.index] = serial

    by_index = {entry.index: entry for entry in exports}
    # An actor points at its component; remember the reverse so a component can
    # read the transform its actor owns.
    actor_of_component: dict[int, int] = {}
    for index, properties in parsed.items():
        entry = properties.get("lightcomponent")
        if isinstance(entry, dict) and isinstance(entry.get("value"), int):
            component = entry["value"] - 1
            if component >= 0:
                actor_of_component[component] = index

    lights: list[dict[str, Any]] = []
    skipped: list[dict[str, Any]] = []
    for index in sorted(parsed):
        class_name = class_of.get(index, "")
        if class_name in POINT_CLASSES:
            kind = "POINT"
        elif class_name in SPOT_CLASSES:
            kind = "SPOT"
        elif class_name in DIRECTIONAL_CLASSES:
            kind = "DIRECTIONAL"
        else:
            continue
        component = parsed[index]
        actor_index = actor_of_component.get(index)
        actor = parsed.get(actor_index, {}) if actor_index is not None else {}
        position = location(actor)
        if position is None:
            position = cached_translation(serial_of[index], names, summary.version)
        object_name = by_index[index].object_name
        if position is None:
            skipped.append({"object": object_name, "reason": "no-transform"})
            continue

        brightness = scalar(component, "brightness", 1.0) or 0.0
        radius = (scalar(component, "radius", 0.0) or 0.0) * UNIT_SCALE
        if kind != "DIRECTIONAL" and not 0.01 <= radius <= RANGE_LIMIT_METRES:
            skipped.append({"object": object_name, "reason": f"range {radius:.4f} m out of range"})
            continue
        if not 0.0 <= brightness <= BRIGHTNESS_LIMIT:
            skipped.append({"object": object_name, "reason": f"brightness {brightness} out of range"})
            continue

        inner = outer = 0.0
        if kind == "SPOT":
            inner = min(max(scalar(component, "innerconeangle", 1.0) or 1.0, 0.01), 89.9)
            outer = min(max(scalar(component, "outerconeangle", 45.0) or 45.0, inner), 89.9)

        owner = by_index[actor_index].object_name if actor_index is not None else object_name
        lights.append(
            {
                "lightId": f"light.{area_id.lower()}.{owner.lower()}.{index}",
                "displayName": f"{owner} / {object_name}",
                "kind": kind,
                "groupId": "source",
                "enabled": boolean(component, "benabled", True),
                "position": [round(value, 6) for value in position],
                "rotationDegrees": rotation_degrees(actor),
                "rangeMeters": round(radius, 6),
                "falloffExponent": round(
                    scalar(component, "falloffexponent", DEFAULT_FALLOFF_EXPONENT)
                    or DEFAULT_FALLOFF_EXPONENT,
                    6,
                ),
                "innerConeDegrees": round(inner, 6),
                "outerConeDegrees": round(outer, 6),
                "color": [round(value, 6) for value in colour(component)],
                "brightness": round(brightness, 6),
            }
        )

    return {
        "document": {
            "schema": "lostark.map-light-presentation",
            "formatVersion": 2,
            "areaId": area_id,
            "provenance": "PROJECT_AUTHORED",
            "nextLightOrdinal": len(lights) + 1,
            "lights": lights,
        },
        "skipped": skipped,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--package", required=True, type=Path)
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=ue3.LOSTARK_KR_AES_KEY)
    args = parser.parse_args()

    result = collect(args.package, args.area_id, args.aes_key)
    document = result["document"]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(document, ensure_ascii=False, indent=1, allow_nan=False) + "\n",
        encoding="utf-8",
    )
    print(f"{args.output}: {len(document['lights'])} lights")
    for light in document["lights"]:
        print(
            "   %-52s %-11s pos=%s range=%.2fm bright=%.2f color=%s enabled=%s"
            % (
                light["lightId"],
                light["kind"],
                light["position"],
                light["rangeMeters"],
                light["brightness"],
                light["color"][:3],
                light["enabled"],
            )
        )
    for row in result["skipped"]:
        print(f"   skipped {row['object']}: {row['reason']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
