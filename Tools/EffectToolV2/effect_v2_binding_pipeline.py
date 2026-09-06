#!/usr/bin/env python3
"""Stable BOSS_VALTAN Effect V2 binding contract and migration helpers.

The persisted binding owner contains stable occurrence identities only.  Effect
and group body hashes belong to an ephemeral read-set snapshot which a writer
must compare again while holding the canonical Composition writer lock.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import sys
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence


BINDING_SCHEMA = "lostark.effect-v2-bindings"
BINDING_FORMAT_VERSION = 2
READ_SET_SCHEMA = "lostark.effect-v2-binding-read-set"
READ_SET_FORMAT_VERSION = 1
GROUP_SCHEMA = "lostark.effect-v2-group"
GROUP_FORMAT_VERSION = 2
VALTAN_ARCHETYPE_ID = "BOSS_VALTAN"
MAX_BINDINGS = 4096
MAX_MS = 600000
MAX_GROUP_DEPTH = 32
MAX_JSON_BYTES = 16 * 1024 * 1024
STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,160}$")
EFFECT_ID = re.compile(r"^[A-Za-z0-9_.-]{1,80}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
RESOURCE_KINDS = {"LEAF", "GROUP"}
CLOCK_BASES = {"STAGE", "CLIP_OCCURRENCE"}
REPEAT_POLICIES = {"ONCE", "EACH_LOOP"}
FOLLOW_POLICIES = {"FOLLOW_SLOT", "SNAPSHOT_AT_START"}
ROTATION_BASES = {"SLOT", "TARGET_YAW", "WORLD"}
STOP_POLICIES = {
    "NATURAL",
    "STAGE_END",
    "CLIP_OCCURRENCE_END",
    "EXPLICIT",
}
EFFECT_TYPES = {"Mesh", "Texture", "Particle", "Decal", "Trail", "ScreenPost"}
GROUP_CHILD_STOPS = {"Kill", "Deactivate"}
SLOT_FIELDS = ("mesh", "base", "noise", "mask", "emissive", "dissolve")
BLEND_VALUES = {"Alpha", "Additive", "Opaque", "Multiply"}
CLIP_CHANNEL_VALUES = {"RGB", "Alpha"}
PARTICLE_SPAWN_SHAPES = {"Point", "Sphere", "Ring", "Box"}
PARTICLE_VELOCITY_MODES = {"Fixed", "Outward", "Cone"}
PARTICLE_ALIGNMENTS = {"Camera", "Velocity", "Horizontal"}
TRAIL_EDGE_MODES = {"CenterlineCamera", "CenterlineUp", "LocalOffset"}
SCREEN_POST_PROFILES = {"ZoomBlur", "RgbNoise", "FilmNoise", "ChromaticAberration"}
ROOT_FIELDS = ("schema", "formatVersion", "archetypeId", "bindings")
BINDING_FIELDS = (
    "bindingId",
    "resource",
    "scope",
    "clock",
    "anchor",
    "stopPolicy",
)
RESOURCE_FIELDS = ("kind", "id")
SCOPE_FIELDS = ("patternId", "stageId", "actionId")
CLOCK_FIELDS = (
    "basis",
    "clipOccurrenceId",
    "startMs",
    "repeatPolicy",
)
ANCHOR_FIELDS = (
    "slotId",
    "followPolicy",
    "rotationBasis",
    "localTransform",
)
TRANSFORM_FIELDS = ("translation", "rotation", "scale")
READ_SET_FIELDS = (
    "schema",
    "formatVersion",
    "archetypeId",
    "resources",
    "readSetHash",
)
READ_SET_ROW_FIELDS = ("kind", "id", "path", "sha256")
LEAF_ROOT_FIELDS = (
    "schema",
    "formatVersion",
    "effectId",
    "effectType",
    "slots",
    "params",
    "parts",
)
GROUP_ROOT_FIELDS = ("schema", "formatVersion", "groupId", "durationMs", "children")
GROUP_CHILD_FIELDS = (
    "childId",
    "resource",
    "startMs",
    "durationMs",
    "stop",
    "localTransform",
)


class BindingContractError(ValueError):
    pass


class BindingMigrationAmbiguityError(BindingContractError):
    def __init__(self, message: str, report: Mapping[str, Any]) -> None:
        super().__init__(message)
        self.report = dict(report)


class BindingReadSetStaleError(BindingContractError):
    pass


def _reject_non_finite(value: str) -> None:
    raise BindingContractError(f"non-finite JSON number is forbidden: {value}")


def _reject_duplicate_object_keys(
    pairs: Sequence[tuple[str, Any]],
) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise BindingContractError(f"duplicate JSON object key is forbidden: {key}")
        result[key] = value
    return result


def read_json(path: Path) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise BindingContractError(f"invalid JSON: {path}: {exc}") from exc
    return read_json_bytes(payload, path.as_posix())


def read_json_bytes(payload: bytes, owner: str) -> dict[str, Any]:
    if len(payload) > MAX_JSON_BYTES:
        raise BindingContractError(
            f"invalid JSON: {owner}: document exceeds {MAX_JSON_BYTES} bytes"
        )
    try:
        value = json.loads(
            payload.decode("utf-8"),
            parse_constant=_reject_non_finite,
            object_pairs_hook=_reject_duplicate_object_keys,
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise BindingContractError(f"invalid JSON: {owner}: {exc}") from exc
    if not isinstance(value, dict):
        raise BindingContractError(f"JSON root must be an object: {owner}")
    return value


def json_text(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2) + "\n"


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def _sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _exact(value: Any, fields: Sequence[str], owner: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != set(fields):
        raise BindingContractError(
            f"{owner} fields must be exactly {', '.join(fields)}"
        )
    return value


def _stable(value: Any, owner: str) -> str:
    if not isinstance(value, str) or STABLE_ID.fullmatch(value) is None:
        raise BindingContractError(f"{owner} must be a stable ID")
    return value


def _effect_id(value: Any, owner: str) -> str:
    if not isinstance(value, str) or EFFECT_ID.fullmatch(value) is None:
        raise BindingContractError(
            f"{owner} must use 1..80 ASCII letters, digits, '.', '_' or '-'"
        )
    return value


def _is_format_version(value: Any, expected: int) -> bool:
    return (
        not isinstance(value, bool)
        and isinstance(value, (int, float))
        and math.isfinite(float(value))
        and float(value) == float(expected)
    )


def _ms(value: Any, owner: str) -> int:
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or value < 0
        or value > MAX_MS
    ):
        raise BindingContractError(f"{owner} must be an integer in [0, {MAX_MS}]")
    return value


def _vector3(value: Any, owner: str, *, scale: bool = False) -> tuple[float, ...]:
    if not isinstance(value, list) or len(value) != 3:
        raise BindingContractError(f"{owner} must be exactly three finite numbers")
    result: list[float] = []
    for component in value:
        if (
            isinstance(component, bool)
            or not isinstance(component, (int, float))
            or not math.isfinite(float(component))
        ):
            raise BindingContractError(f"{owner} must be exactly three finite numbers")
        number = float(component)
        if scale and number == 0.0:
            raise BindingContractError(f"{owner} components must be non-zero")
        result.append(number)
    return tuple(result)


def _finite_number(value: Any, owner: str, *, minimum: float | None = None) -> float:
    if (
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(float(value))
    ):
        raise BindingContractError(f"{owner} must be a finite number")
    result = float(value)
    if minimum is not None and result < minimum:
        raise BindingContractError(f"{owner} must be >= {minimum}")
    return result


def _validate_json_numbers(value: Any, owner: str) -> None:
    if value is None or isinstance(value, (bool, str)):
        return
    if isinstance(value, (int, float)):
        _finite_number(value, owner)
        return
    if isinstance(value, list):
        for ordinal, child in enumerate(value):
            _validate_json_numbers(child, f"{owner}[{ordinal}]")
        return
    if isinstance(value, dict):
        for key, child in value.items():
            _validate_json_numbers(child, f"{owner}.{key}")
        return
    raise BindingContractError(f"{owner} contains an unsupported JSON value")


def _asset_id(
    value: Any,
    owner: str,
    *,
    allow_empty: bool = True,
    slot: str | None = None,
    resource_root: Path | None = None,
) -> str:
    if not isinstance(value, str) or (not allow_empty and not value):
        raise BindingContractError(f"{owner} must be a Resources-relative asset ID")
    if not value:
        return value
    path = PurePosixPath(value)
    if (
        "\\" in value
        or path.is_absolute()
        or ".." in path.parts
        or not path.parts
        or ":" in value
    ):
        raise BindingContractError(f"{owner} must be a Resources-relative asset ID")
    if slot is not None:
        expected_suffix = ".wmodel" if slot == "mesh" else ".dds"
        if path.suffix.lower() != expected_suffix:
            raise BindingContractError(
                f"{owner} must reference a {expected_suffix} resource"
            )
    if resource_root is not None:
        try:
            canonical_root = resource_root.resolve()
            physical = canonical_root.joinpath(*path.parts).resolve()
            physical.relative_to(canonical_root)
        except (OSError, ValueError) as exc:
            raise BindingContractError(
                f"{owner} escapes Resources after canonicalization"
            ) from exc
        if not physical.is_file():
            raise BindingContractError(f"{owner} resource is missing: {value}")
    return value


def _leaf_resource_root(path: Path) -> Path:
    for environment_name in ("LOSTARK_RESOURCE_ROOT", "LOSTARK_SHARED_ASSET_ROOT"):
        configured = os.environ.get(environment_name, "").strip()
        if configured:
            return Path(configured)
    for ancestor in path.resolve().parents:
        if ancestor.name == "Data":
            return ancestor.parent / "Client/Bin/Resources"
    raise BindingContractError(
        f"Effect V2 leaf is not under a repository Data folder: {path}"
    )


def _optional_number(
    value: Mapping[str, Any], key: str, owner: str, default: float
) -> float:
    return (
        default
        if key not in value
        else _finite_number(value[key], f"{owner}.{key}")
    )


def _optional_uint(
    value: Mapping[str, Any], key: str, owner: str, default: int
) -> int:
    if key not in value:
        return default
    result = value[key]
    if (
        isinstance(result, bool)
        or not isinstance(result, int)
        or result < 0
        or result > 4294967295
    ):
        raise BindingContractError(f"{owner}.{key} must be a non-negative integer")
    return result


def _optional_bool(
    value: Mapping[str, Any], key: str, owner: str, default: bool
) -> bool:
    if key not in value:
        return default
    result = value[key]
    if not isinstance(result, bool):
        raise BindingContractError(f"{owner}.{key} must be a boolean")
    return result


def _optional_vector(
    value: Mapping[str, Any],
    key: str,
    owner: str,
    count: int,
    default: Sequence[float],
) -> tuple[float, ...]:
    if key not in value:
        return tuple(default)
    raw = value[key]
    if not isinstance(raw, list) or len(raw) != count:
        raise BindingContractError(f"{owner}.{key} must be {count} finite numbers")
    return tuple(
        _finite_number(component, f"{owner}.{key}[{ordinal}]")
        for ordinal, component in enumerate(raw)
    )


def _optional_enum(
    value: Mapping[str, Any],
    key: str,
    owner: str,
    allowed: set[str],
    default: str,
) -> str:
    if key not in value:
        return default
    result = value[key]
    if result not in allowed:
        raise BindingContractError(f"{owner}.{key} has an unknown value")
    return result


def _validate_lerp_track(value: Mapping[str, Any], key: str, owner: str) -> None:
    if key not in value:
        return
    track = value[key]
    if not isinstance(track, dict):
        raise BindingContractError(f"{owner}.{key} must be an object")
    _optional_vector(track, "start", f"{owner}.{key}", 3, (0.0, 0.0, 0.0))
    _optional_vector(track, "end", f"{owner}.{key}", 3, (0.0, 0.0, 0.0))
    _optional_bool(track, "lerp", f"{owner}.{key}", False)


def _validate_leaf_params(
    effect_id: str, effect_type: str, params: Mapping[str, Any]
) -> tuple[float, float, bool, float]:
    owner = f"Effect V2 leaf {effect_id}.params"
    for key in ("position", "rotation", "scale", "velocity"):
        _validate_lerp_track(params, key, owner)
    for key, count, default in (
        ("colorOffset", 4, (0.0, 0.0, 0.0, 0.0)),
        ("colorOffsetEnd", 4, (0.0, 0.0, 0.0, 0.0)),
        ("colorMul", 4, (1.0, 1.0, 1.0, 1.0)),
        ("colorMulEnd", 4, (1.0, 1.0, 1.0, 0.0)),
        ("rimColor", 4, (1.0, 1.0, 1.0, 1.0)),
        ("outlineColor", 4, (1.0, 1.0, 1.0, 1.0)),
        ("uvStart", 2, (0.0, 0.0)),
        ("uvSpeed", 2, (0.0, 0.0)),
        ("uvTileCount", 2, (1.0, 1.0)),
        ("noisePan", 2, (0.0, 0.0)),
    ):
        _optional_vector(params, key, owner, count, default)
    for key, default in (
        ("colorOffsetLerp", False),
        ("colorMulLerp", False),
        ("billboard", True),
        ("depthTest", True),
        ("loop", True),
        ("animationLoop", True),
        ("colorTexturesSRGB", True),
    ):
        _optional_bool(params, key, owner, default)
    _optional_enum(params, "colorClipChannel", owner, CLIP_CHANNEL_VALUES, "Alpha")
    _optional_enum(params, "blend", owner, BLEND_VALUES, "Additive")
    for key, default in (
        ("colorClip", 0.0),
        ("rimPower", 3.0),
        ("rimIntensity", 0.0),
        ("ghostAlpha", 0.0),
        ("outlineWidth", 0.0),
        ("bloomIntensity", 1.0),
        ("distortionIntensity", 0.0),
        ("noiseStrength", 0.0),
        ("noiseScale", 1.0),
        ("dissolveStart", 0.0),
        ("dissolveInEnd", 0.0),
        ("dissolveSoftness", 0.1),
    ):
        _optional_number(params, key, owner, default)
    soft_fade = _optional_number(params, "softFadeDistance", owner, 0.0)
    lifetime = _optional_number(params, "lifetime", owner, 0.0)
    play_rate = _optional_number(params, "playRate", owner, 1.0)
    mesh_pre_scale = _optional_number(params, "meshPreScale", owner, 0.01)
    loop = _optional_bool(params, "loop", owner, True)
    if soft_fade < 0.0 or lifetime < 0.0 or play_rate < 0.0 or mesh_pre_scale <= 0.0:
        raise BindingContractError(
            f"{owner} softFade/lifetime/playRate/meshPreScale is out of range"
        )
    if "animationClip" in params and not isinstance(params["animationClip"], str):
        raise BindingContractError(f"{owner}.animationClip must be a string")

    # These are the C++ runtime defaults when the optional type payload is
    # absent.  Keeping them here prevents a NATURAL group span from being
    # shorter than the body that the current parser/runtime will create.
    particle_tail = 1.0 if effect_type == "Particle" else 0.0
    if "particle" in params:
        particle = params["particle"]
        if not isinstance(particle, dict):
            raise BindingContractError(f"{owner}.particle must be an object")
        max_particles = _optional_uint(particle, "maxParticles", f"{owner}.particle", 256)
        spawn_rate = _optional_number(particle, "spawnRate", f"{owner}.particle", 20.0)
        _optional_uint(particle, "burstCount", f"{owner}.particle", 0)
        particle_lifetime = _optional_vector(
            particle, "lifetime", f"{owner}.particle", 2, (0.5, 1.0)
        )
        _optional_enum(
            particle, "spawnShape", f"{owner}.particle", PARTICLE_SPAWN_SHAPES, "Point"
        )
        _optional_enum(
            particle,
            "velocityMode",
            f"{owner}.particle",
            PARTICLE_VELOCITY_MODES,
            "Cone",
        )
        _optional_enum(
            particle,
            "alignment",
            f"{owner}.particle",
            PARTICLE_ALIGNMENTS,
            "Camera",
        )
        for key, count, default in (
            ("spawnExtents", 3, (0.5, 0.5, 0.5)),
            ("velocityMin", 3, (-0.5, 1.0, -0.5)),
            ("velocityMax", 3, (0.5, 2.0, 0.5)),
            ("speedRange", 2, (1.0, 2.0)),
            ("acceleration", 3, (0.0, -1.0, 0.0)),
            ("sizeStart", 2, (0.2, 0.2)),
            ("sizeEnd", 2, (0.0, 0.0)),
            ("rotationRange", 2, (0.0, 0.0)),
            ("spinRange", 2, (0.0, 0.0)),
            ("hueShiftRange", 2, (0.0, 0.0)),
            ("colorStart", 4, (1.0, 1.0, 1.0, 1.0)),
            ("colorEnd", 4, (1.0, 1.0, 1.0, 0.0)),
            ("meshRotationMin", 3, (0.0, 0.0, 0.0)),
            ("meshRotationMax", 3, (0.0, 0.0, 0.0)),
            ("meshSpinMin", 3, (0.0, 0.0, 0.0)),
            ("meshSpinMax", 3, (0.0, 0.0, 0.0)),
        ):
            _optional_vector(particle, key, f"{owner}.particle", count, default)
        for key, default in (
            ("spawnRadius", 0.5),
            ("spawnInnerRadius", 0.0),
            ("spawnArcDegrees", 360.0),
            ("coneAngleDegrees", 30.0),
            ("drag", 0.0),
        ):
            _optional_number(particle, key, f"{owner}.particle", default)
        for key, default in (("localSpace", True), ("subUVOverLife", True)):
            _optional_bool(particle, key, f"{owner}.particle", default)
        tile_columns = _optional_uint(particle, "tileColumns", f"{owner}.particle", 1)
        tile_rows = _optional_uint(particle, "tileRows", f"{owner}.particle", 1)
        _optional_uint(particle, "randomSeed", f"{owner}.particle", 1)
        if (
            max_particles == 0
            or max_particles > 2048
            or spawn_rate < 0.0
            or particle_lifetime[0] <= 0.0
            or particle_lifetime[1] < particle_lifetime[0]
            or tile_columns == 0
            or tile_rows == 0
        ):
            raise BindingContractError(f"{owner}.particle values are out of range")
        particle_tail = particle_lifetime[1]

    if "decal" in params:
        decal = params["decal"]
        if not isinstance(decal, dict):
            raise BindingContractError(f"{owner}.decal must be an object")
        size = _optional_vector(decal, "size", f"{owner}.decal", 2, (1.0, 1.0))
        depth = _optional_number(decal, "depth", f"{owner}.decal", 0.5)
        _optional_number(decal, "edgeFade", f"{owner}.decal", 0.0)
        _optional_number(decal, "normalCutoff", f"{owner}.decal", 0.5)
        if size[0] <= 0.0 or size[1] <= 0.0 or depth <= 0.0:
            raise BindingContractError(f"{owner}.decal size/depth must be positive")

    trail_tail = 0.35 if effect_type == "Trail" else 0.0
    if "trail" in params:
        trail = params["trail"]
        if not isinstance(trail, dict):
            raise BindingContractError(f"{owner}.trail must be an object")
        max_points = _optional_uint(trail, "maxPoints", f"{owner}.trail", 64)
        point_lifetime = _optional_number(
            trail, "pointLifetime", f"{owner}.trail", 0.35
        )
        sample_interval = _optional_number(
            trail, "sampleInterval", f"{owner}.trail", 1.0 / 60.0
        )
        min_distance = _optional_number(trail, "minDistance", f"{owner}.trail", 0.01)
        for key, default in (
            ("startWidth", 0.2),
            ("endWidth", 0.0),
            ("tilingDistance", 0.0),
        ):
            _optional_number(trail, key, f"{owner}.trail", default)
        _optional_enum(
            trail, "edgeMode", f"{owner}.trail", TRAIL_EDGE_MODES, "CenterlineCamera"
        )
        _optional_vector(
            trail, "edgeOffset", f"{owner}.trail", 3, (0.0, 1.0, 0.0)
        )
        _optional_bool(trail, "fadeWithAge", f"{owner}.trail", True)
        if (
            max_points < 2
            or max_points > 4096
            or point_lifetime <= 0.0
            or sample_interval < 0.0
            or min_distance < 0.0
        ):
            raise BindingContractError(f"{owner}.trail values are out of range")
        trail_tail = point_lifetime

    if "screenPost" in params:
        screen = params["screenPost"]
        if not isinstance(screen, dict):
            raise BindingContractError(f"{owner}.screenPost must be an object")
        _optional_enum(
            screen,
            "profile",
            f"{owner}.screenPost",
            SCREEN_POST_PROFILES,
            "ZoomBlur",
        )
        values = [
            _optional_number(screen, key, f"{owner}.screenPost", default)
            for key, default in (
                ("intensityStart", 2.0),
                ("intensityEnd", 0.0),
                ("secondaryIntensity", 0.0),
                ("frequency", 1.0),
            )
        ]
        _optional_bool(screen, "intensityLerp", f"{owner}.screenPost", True)
        _optional_vector(
            screen, "tint", f"{owner}.screenPost", 4, (1.0, 1.0, 1.0, 1.0)
        )
        random_seed = _optional_uint(
            screen, "randomSeed", f"{owner}.screenPost", 1
        )
        if any(number < 0.0 for number in values) or random_seed == 0:
            raise BindingContractError(f"{owner}.screenPost values are out of range")

    tail = particle_tail if effect_type == "Particle" else trail_tail if effect_type == "Trail" else 0.0
    return lifetime, play_rate, loop, tail


def _validate_leaf_resource(
    effect_id: str,
    path: Path,
    document: Mapping[str, Any],
    *,
    resource_root: Path | None = None,
) -> int | None:
    """Return the conservative natural wall lifetime in ms, or None if unbounded.

    Runtime advances the authored lifetime, particle ages, and trail ages by
    wallDelta * playRate.  Particle and trail bodies therefore retain their
    longest authored tail after emission stops.  A loop, zero lifetime, or
    zero playRate has no finite NATURAL boundary.
    """

    effect_id = _effect_id(effect_id, "Effect V2 leaf effectId")
    root = _exact(document, LEAF_ROOT_FIELDS, f"Effect V2 leaf {effect_id}")
    if (
        root["schema"] != "lostark.effect-v2"
        or not _is_format_version(root["formatVersion"], 1)
        or root["effectId"] != effect_id
        or path.name != f"{effect_id}.effectv2.json"
        or root["effectType"] not in EFFECT_TYPES
        or not isinstance(root["parts"], list)
    ):
        raise BindingContractError(f"Effect V2 leaf header/body is invalid: {effect_id}")
    resource_root = resource_root or _leaf_resource_root(path)
    slots = _exact(root["slots"], SLOT_FIELDS, f"Effect V2 leaf {effect_id}.slots")
    for slot in SLOT_FIELDS:
        _asset_id(
            slots[slot],
            f"Effect V2 leaf {effect_id}.slots.{slot}",
            slot=slot,
            resource_root=resource_root,
        )
    if root["effectType"] == "Mesh" and not slots["mesh"]:
        raise BindingContractError(f"Mesh Effect V2 leaf requires slots.mesh: {effect_id}")
    params = root["params"]
    if not isinstance(params, dict):
        raise BindingContractError(f"Effect V2 leaf params are invalid: {effect_id}")
    _validate_json_numbers(root, f"Effect V2 leaf {effect_id}")
    lifetime, play_rate, loop, tail = _validate_leaf_params(
        effect_id, root["effectType"], params
    )
    part_indexes: set[int] = set()
    for ordinal, raw in enumerate(root["parts"]):
        owner = f"Effect V2 leaf {effect_id}.parts[{ordinal}]"
        part = _exact(raw, ("index", "visible", "base"), owner)
        index = part["index"]
        if (
            isinstance(index, bool)
            or not isinstance(index, int)
            or index < 0
            or index > 255
            or index in part_indexes
        ):
            raise BindingContractError(f"{owner}.index must be a unique integer in [0, 255]")
        part_indexes.add(index)
        if not isinstance(part["visible"], bool):
            raise BindingContractError(f"{owner}.visible must be a boolean")
        _asset_id(
            part["base"],
            f"{owner}.base",
            slot="base",
            resource_root=resource_root,
        )
    if loop or lifetime <= 0.0 or play_rate <= 0.0:
        return None
    wall_ms = math.ceil((lifetime + tail) * 1000.0 / play_rate)
    if wall_ms <= 0 or wall_ms > MAX_MS:
        raise BindingContractError(
            f"Effect V2 leaf {effect_id} natural lifetime exceeds {MAX_MS}ms"
        )
    return wall_ms


def _gameplay_action_index(
    gameplay: Mapping[str, Any],
) -> dict[str, list[dict[str, Any]]]:
    patterns = gameplay.get("patterns")
    if not isinstance(patterns, list):
        raise BindingContractError("Valtan gameplay patterns must be an array")
    index: dict[str, list[dict[str, Any]]] = {}
    for pattern_ordinal, pattern in enumerate(patterns):
        if not isinstance(pattern, dict):
            raise BindingContractError(
                f"Valtan gameplay patterns[{pattern_ordinal}] must be an object"
            )
        pattern_id = _stable(
            pattern.get("patternId"),
            f"Valtan gameplay patterns[{pattern_ordinal}].patternId",
        )
        stages = pattern.get("stages")
        if not isinstance(stages, list):
            raise BindingContractError(f"{pattern_id}.stages must be an array")
        for stage_ordinal, stage in enumerate(stages):
            if not isinstance(stage, dict):
                raise BindingContractError(
                    f"{pattern_id}.stages[{stage_ordinal}] must be an object"
                )
            stage_id = _stable(
                stage.get("stageId"), f"{pattern_id}.stages[{stage_ordinal}].stageId"
            )
            action_id = _stable(
                stage.get("actionId"),
                f"{pattern_id}.stages[{stage_ordinal}].actionId",
            )
            duration_ms = _ms(
                stage.get("durationMs"),
                f"{pattern_id}/{stage_id}.durationMs",
            )
            index.setdefault(action_id, []).append(
                {
                    "patternId": pattern_id,
                    "stageId": stage_id,
                    "actionId": action_id,
                    "durationMs": duration_ms,
                    "sourceOwner": "Data/Valtan/Valtan.gameplay.json",
                }
            )
    return index


def _legacy_compatibility_action_index(
    legacy_compatibility: Mapping[str, Any] | None,
) -> dict[str, list[dict[str, Any]]]:
    if legacy_compatibility is None:
        return {}
    entries = legacy_compatibility.get("patternEntries")
    if not isinstance(entries, list):
        raise BindingContractError(
            "Valtan legacy compatibility patternEntries must be an array"
        )
    index: dict[str, list[dict[str, Any]]] = {}
    for entry_ordinal, entry in enumerate(entries):
        if not isinstance(entry, dict) or not isinstance(entry.get("runtimePattern"), dict):
            raise BindingContractError(
                f"Valtan legacy compatibility patternEntries[{entry_ordinal}] is invalid"
            )
        pattern = entry["runtimePattern"]
        pattern_id = _stable(
            pattern.get("patternId"),
            f"legacy compatibility patternEntries[{entry_ordinal}].patternId",
        )
        if entry.get("patternId") != pattern_id:
            raise BindingContractError(
                f"legacy compatibility Pattern identity mismatch: {pattern_id}"
            )
        stages = pattern.get("stages")
        if not isinstance(stages, list):
            raise BindingContractError(
                f"legacy compatibility {pattern_id}.stages must be an array"
            )
        for stage_ordinal, stage in enumerate(stages):
            if not isinstance(stage, dict):
                raise BindingContractError(
                    f"legacy compatibility {pattern_id}.stages[{stage_ordinal}] is invalid"
                )
            stage_id = _stable(
                stage.get("stageId"),
                f"legacy compatibility {pattern_id}.stages[{stage_ordinal}].stageId",
            )
            action_id = _stable(
                stage.get("actionId"),
                f"legacy compatibility {pattern_id}.stages[{stage_ordinal}].actionId",
            )
            duration_ms = _ms(
                stage.get("durationMs"),
                f"legacy compatibility {pattern_id}/{stage_id}.durationMs",
            )
            index.setdefault(action_id, []).append(
                {
                    "patternId": pattern_id,
                    "stageId": stage_id,
                    "actionId": action_id,
                    "durationMs": duration_ms,
                    "sourceOwner": "Data/Valtan/Valtan.legacy-compatibility.json",
                }
            )
    return index


def _canonical_action_index(
    gameplay: Mapping[str, Any],
    legacy_compatibility: Mapping[str, Any] | None,
) -> dict[str, list[dict[str, Any]]]:
    result = _gameplay_action_index(gameplay)
    for action_id, rows in _legacy_compatibility_action_index(
        legacy_compatibility
    ).items():
        result.setdefault(action_id, []).extend(rows)
    return result


def _animation_occurrence_indexes(
    animation: Mapping[str, Any],
) -> tuple[dict[str, list[dict[str, Any]]], dict[str, list[dict[str, Any]]]]:
    bindings = animation.get("bindings")
    if not isinstance(bindings, list):
        raise BindingContractError("Valtan Animation Product bindings must be an array")
    by_clip: dict[str, list[dict[str, Any]]] = {}
    by_id: dict[str, list[dict[str, Any]]] = {}
    for binding_ordinal, binding in enumerate(bindings):
        if not isinstance(binding, dict):
            raise BindingContractError(
                f"Valtan Animation Product bindings[{binding_ordinal}] must be an object"
            )
        action_id = _stable(
            binding.get("actionId"),
            f"Valtan Animation Product bindings[{binding_ordinal}].actionId",
        )
        clips = binding.get("clips")
        if not isinstance(clips, list):
            raise BindingContractError(f"Animation binding {action_id}.clips is invalid")
        for clip_ordinal, clip in enumerate(clips):
            if not isinstance(clip, dict):
                raise BindingContractError(
                    f"Animation binding {action_id}.clips[{clip_ordinal}] is invalid"
                )
            occurrence_id = _stable(
                clip.get("clipOccurrenceId"),
                f"Animation binding {action_id}.clips[{clip_ordinal}].clipOccurrenceId",
            )
            clip_name = clip.get("clip")
            if not isinstance(clip_name, str) or not clip_name:
                raise BindingContractError(
                    f"Animation binding {action_id}.clips[{clip_ordinal}].clip is invalid"
                )
            play_ms = _ms(
                clip.get("playMs"),
                f"Animation binding {action_id}/{occurrence_id}.playMs",
            )
            source_start_ms = _ms(
                clip.get("sourceStartMs"),
                f"Animation binding {action_id}/{occurrence_id}.sourceStartMs",
            )
            if not isinstance(clip.get("loop"), bool):
                raise BindingContractError(
                    f"Animation binding {action_id}/{occurrence_id}.loop is invalid"
                )
            row = {
                "actionId": action_id,
                "clipOccurrenceId": occurrence_id,
                "clip": clip_name,
                "sourceStartMs": source_start_ms,
                "playMs": play_ms,
                "loop": clip["loop"],
            }
            by_clip.setdefault(clip_name, []).append(row)
            by_id.setdefault(occurrence_id, []).append(row)
    return by_clip, by_id


def _resource_paths(repository_root: Path) -> tuple[Path, Path]:
    v2_root = repository_root / "Data/Effects/V2"
    return v2_root / "Authored", v2_root / "Groups"


def _load_resource_documents(
    repository_root: Path,
) -> tuple[dict[str, Path], dict[str, tuple[Path, dict[str, Any]]]]:
    authored_root, group_root = _resource_paths(repository_root)
    authored: dict[str, Path] = {}
    for path in sorted(authored_root.glob("*.effectv2.json")):
        document = read_json(path)
        effect_id = _effect_id(document.get("effectId"), f"{path.name}.effectId")
        if path.name != f"{effect_id}.effectv2.json" or effect_id in authored:
            raise BindingContractError(f"Effect V2 authored filename/ID is ambiguous: {path}")
        authored[effect_id] = path
    groups: dict[str, tuple[Path, dict[str, Any]]] = {}
    for path in sorted(group_root.glob("*.effectv2group.json")):
        document = read_json(path)
        group_id = _stable(document.get("groupId"), f"{path.name}.groupId")
        if (
            path.name != f"{group_id}.effectv2group.json"
            or group_id in groups
            or group_id in authored
        ):
            raise BindingContractError(f"Effect V2 group filename/ID is ambiguous: {path}")
        groups[group_id] = (path, document)
    return authored, groups


def _group_children(
    group_id: str,
    authored: Mapping[str, Path],
    groups: Mapping[str, tuple[Path, dict[str, Any]]],
    *,
    require_v2: bool,
) -> tuple[int, list[dict[str, Any]]]:
    entry = groups.get(group_id)
    if entry is None:
        raise BindingContractError(f"Effect V2 binding has no group document: {group_id}")
    path, document = entry
    version = document.get("formatVersion")
    if document.get("schema") != GROUP_SCHEMA or document.get("groupId") != group_id:
        raise BindingContractError(f"Effect V2 group header is invalid: {group_id}")
    if path.name != f"{group_id}.effectv2group.json":
        raise BindingContractError(f"Effect V2 group filename/ID mismatch: {group_id}")
    if require_v2 and not _is_format_version(version, GROUP_FORMAT_VERSION):
        raise BindingContractError(
            f"Effect V2 group {group_id} requires explicit formatVersion 2 migration"
        )
    if not (
        _is_format_version(version, 1)
        or _is_format_version(version, GROUP_FORMAT_VERSION)
    ):
        raise BindingContractError(f"unsupported Effect V2 group version: {group_id}")
    is_v2 = _is_format_version(version, GROUP_FORMAT_VERSION)
    if is_v2:
        _exact(document, GROUP_ROOT_FIELDS, f"Effect V2 group {group_id}")
    duration_ms = _ms(document.get("durationMs"), f"Effect V2 group {group_id}.durationMs")
    children = document.get("children")
    if not isinstance(children, list) or not children or len(children) > 4096:
        raise BindingContractError(
            f"Effect V2 group children must be an array in [1, 4096]: {group_id}"
        )
    normalized: list[dict[str, Any]] = []
    child_ids: set[str] = set()
    semantic_children: set[bytes] = set()
    for ordinal, raw in enumerate(children):
        owner = f"Effect V2 group {group_id}.children[{ordinal}]"
        if not isinstance(raw, dict):
            raise BindingContractError(f"{owner} is invalid")
        if is_v2:
            child = _exact(raw, GROUP_CHILD_FIELDS, owner)
            child_id = _stable(child["childId"], f"{owner}.childId")
            if child_id in child_ids:
                raise BindingContractError(
                    f"duplicate Effect V2 group childId: {group_id}/{child_id}"
                )
            child_ids.add(child_id)
            resource = _exact(child["resource"], RESOURCE_FIELDS, f"{owner}.resource")
            kind = resource["kind"]
            if kind not in RESOURCE_KINDS:
                raise BindingContractError(f"{owner}.resource.kind is invalid")
            resource_id = (
                _effect_id(resource["id"], f"{owner}.resource.id")
                if kind == "LEAF"
                else _stable(resource["id"], f"{owner}.resource.id")
            )
            transform = _exact(
                child["localTransform"], TRANSFORM_FIELDS, f"{owner}.localTransform"
            )
            _vector3(transform["translation"], f"{owner}.translation")
            _vector3(transform["rotation"], f"{owner}.rotation")
            _vector3(transform["scale"], f"{owner}.scale", scale=True)
            if child["stop"] not in GROUP_CHILD_STOPS:
                raise BindingContractError(f"{owner}.stop is invalid")
            start_ms = _ms(child["startMs"], f"{owner}.startMs")
            child_duration_ms = _ms(child["durationMs"], f"{owner}.durationMs")
            semantic_child = copy.deepcopy(dict(child))
            del semantic_child["childId"]
            semantic_identity = _canonical_bytes(semantic_child)
            if semantic_identity in semantic_children:
                raise BindingContractError(
                    f"duplicate Effect V2 group semantic child: {group_id}/{child_id}"
                )
            semantic_children.add(semantic_identity)
        else:
            # Explicit read-only compatibility for current physical group files.
            # Canonical v2 binding admission sets require_v2 and cannot take this path.
            effect_id = raw.get("effectId")
            if not isinstance(effect_id, str) or not effect_id:
                raise BindingContractError(
                    f"legacy Effect V2 group child must reference one leaf: {owner}"
                )
            kind = "LEAF"
            resource_id = _effect_id(effect_id, f"{owner}.effectId")
            start_ms = _ms(raw.get("startMs", 0), f"{owner}.startMs")
            child_duration_ms = _ms(raw.get("durationMs", 0), f"{owner}.durationMs")
            if raw.get("stop", "Deactivate") not in GROUP_CHILD_STOPS:
                raise BindingContractError(f"{owner}.stop is invalid")
            _vector3(raw.get("offset", [0, 0, 0]), f"{owner}.offset")
            _finite_number(raw.get("pitchDegrees", 0), f"{owner}.pitchDegrees")
            _finite_number(raw.get("yawDegrees", 0), f"{owner}.yawDegrees")
            _finite_number(raw.get("rollDegrees", 0), f"{owner}.rollDegrees")
            _vector3(raw.get("scale", [1, 1, 1]), f"{owner}.scale", scale=True)
        if kind == "LEAF" and resource_id not in authored:
            raise BindingContractError(
                f"Effect V2 group child has no authored leaf: {group_id}/{resource_id}"
            )
        if kind == "GROUP" and resource_id not in groups:
            raise BindingContractError(
                f"Effect V2 group child has no group document: {group_id}/{resource_id}"
            )
        normalized.append(
            {
                "childId": child_id if is_v2 else None,
                "kind": kind,
                "id": resource_id,
                "startMs": start_ms,
                "durationMs": child_duration_ms,
                "localTransform": (
                    copy.deepcopy(child["localTransform"])
                    if is_v2
                    else {
                        "translation": list(raw.get("offset", [0, 0, 0])),
                        "rotation": [
                            raw.get("pitchDegrees", 0),
                            raw.get("yawDegrees", 0),
                            raw.get("rollDegrees", 0),
                        ],
                        "scale": list(raw.get("scale", [1, 1, 1])),
                    }
                ),
            }
        )
    _validate_json_numbers(document, f"Effect V2 group {group_id}")
    return duration_ms, normalized


def _resolve_group(
    group_id: str,
    authored: Mapping[str, Path],
    groups: Mapping[str, tuple[Path, dict[str, Any]]],
    *,
    require_v2: bool,
    resource_root: Path | None = None,
    stack: tuple[str, ...] = (),
) -> tuple[list[tuple[str, int, tuple[str, ...], tuple[bytes, ...]]], int]:
    if group_id in stack:
        raise BindingContractError(
            "Effect V2 group cycle is forbidden: " + " -> ".join((*stack, group_id))
        )
    if len(stack) >= MAX_GROUP_DEPTH:
        raise BindingContractError(
            f"Effect V2 group nesting exceeds {MAX_GROUP_DEPTH}: {group_id}"
        )
    duration_ms, children = _group_children(
        group_id, authored, groups, require_v2=require_v2
    )
    leaves: list[tuple[str, int, tuple[str, ...], tuple[bytes, ...]]] = []
    maximum_end: int | None = 0
    for ordinal, child in enumerate(children):
        child_start = child["startMs"]
        if child["kind"] == "LEAF":
            effect_id = child["id"]
            natural_span = _validate_leaf_resource(
                effect_id,
                authored[effect_id],
                read_json(authored[effect_id]),
                resource_root=resource_root,
            )
            child_leaves = [
                (
                    effect_id,
                    0,
                    (child["childId"],),
                    (_canonical_bytes(child["localTransform"]),),
                )
            ]
        else:
            child_leaves, natural_span = _resolve_group(
                child["id"],
                authored,
                groups,
                require_v2=require_v2,
                resource_root=resource_root,
                stack=(*stack, group_id),
            )
            child_leaves = [
                (
                    effect_id,
                    nested_start,
                    (child["childId"], *child_id_path),
                    (
                        _canonical_bytes(child["localTransform"]),
                        *transform_path,
                    ),
                )
                for effect_id, nested_start, child_id_path, transform_path in child_leaves
            ]
        for effect_id, nested_start, child_id_path, transform_path in child_leaves:
            leaves.append(
                (
                    effect_id,
                    child_start + nested_start,
                    child_id_path,
                    transform_path,
                )
            )
        child_span: int | None = child["durationMs"] or natural_span
        if child_span is None:
            if duration_ms == 0:
                raise BindingContractError(
                    f"Effect V2 group {group_id} has unbounded natural child: "
                    f"{child['id']}"
                )
            child_end: int | None = None
        else:
            child_end = child_start + child_span
            if child_end > MAX_MS:
                raise BindingContractError(
                    f"Effect V2 group {group_id} child span exceeds {MAX_MS}ms"
                )
        if duration_ms and child_start >= duration_ms:
            raise BindingContractError(
                f"Effect V2 group {group_id} child starts at/after its duration: "
                f"{child['id']}"
            )
        if maximum_end is not None:
            maximum_end = None if child_end is None else max(maximum_end, child_end)
    if duration_ms:
        resolved_span = duration_ms if maximum_end is None else min(duration_ms, maximum_end)
    else:
        if maximum_end is None or maximum_end <= 0:
            raise BindingContractError(
                f"Effect V2 group {group_id} has no finite natural duration"
            )
        resolved_span = maximum_end
    return leaves, resolved_span


def _group_leaf_clocks(
    group_id: str,
    authored: Mapping[str, Path],
    groups: Mapping[str, tuple[Path, dict[str, Any]]],
) -> list[tuple[str, int]]:
    leaves, _span_ms = _resolve_group(
        group_id, authored, groups, require_v2=True
    )
    return [(effect_id, start_ms) for effect_id, start_ms, _ids, _trs in leaves]


def validate_binding_document(
    repository_root: Path,
    document: Mapping[str, Any],
    gameplay: Mapping[str, Any],
    animation: Mapping[str, Any],
    legacy_compatibility: Mapping[str, Any] | None = None,
    resource_root: Path | None = None,
) -> dict[str, Any]:
    """Validate and return a deep-copied formatVersion 2 BOSS_VALTAN owner."""

    document = _exact(document, ROOT_FIELDS, "BOSS_VALTAN Effect V2 bindings")
    if (
        document["schema"] != BINDING_SCHEMA
        or not _is_format_version(
            document["formatVersion"], BINDING_FORMAT_VERSION
        )
        or document["archetypeId"] != VALTAN_ARCHETYPE_ID
    ):
        raise BindingContractError(
            "BOSS_VALTAN Effect V2 bindings require formatVersion 2; "
            "run the explicit v1-to-v2 migration"
        )
    rows = document["bindings"]
    if not isinstance(rows, list) or len(rows) > MAX_BINDINGS:
        raise BindingContractError(
            f"BOSS_VALTAN Effect V2 bindings must be an array <= {MAX_BINDINGS}"
        )
    action_index = _canonical_action_index(gameplay, legacy_compatibility)
    _by_clip, occurrence_index = _animation_occurrence_indexes(animation)
    authored, groups = _load_resource_documents(repository_root)
    binding_ids: set[str] = set()
    semantic_identities: set[bytes] = set()
    validated: list[dict[str, Any]] = []
    for ordinal, raw in enumerate(rows):
        context = f"BOSS_VALTAN Effect V2 bindings[{ordinal}]"
        row = _exact(raw, BINDING_FIELDS, context)
        binding_id = _stable(row["bindingId"], f"{context}.bindingId")
        if binding_id in binding_ids:
            raise BindingContractError(f"duplicate Effect V2 bindingId: {binding_id}")
        binding_ids.add(binding_id)

        resource = _exact(row["resource"], RESOURCE_FIELDS, f"{binding_id}.resource")
        kind = resource["kind"]
        if kind not in RESOURCE_KINDS:
            raise BindingContractError(f"{binding_id}.resource.kind is invalid")
        resource_id = (
            _effect_id(resource["id"], f"{binding_id}.resource.id")
            if kind == "LEAF"
            else _stable(resource["id"], f"{binding_id}.resource.id")
        )
        if kind == "LEAF" and resource_id not in authored:
            raise BindingContractError(f"Effect V2 binding has no authored leaf: {resource_id}")
        if kind == "GROUP" and resource_id not in groups:
            raise BindingContractError(f"Effect V2 binding has no group document: {resource_id}")
        if kind == "LEAF":
            natural_span = _validate_leaf_resource(
                resource_id,
                authored[resource_id],
                read_json(authored[resource_id]),
                resource_root=resource_root,
            )
        else:
            _leaf_clocks, natural_span = _resolve_group(
                resource_id,
                authored,
                groups,
                require_v2=True,
                resource_root=resource_root,
            )

        scope = _exact(row["scope"], SCOPE_FIELDS, f"{binding_id}.scope")
        pattern_id = _stable(scope["patternId"], f"{binding_id}.scope.patternId")
        stage_id = _stable(scope["stageId"], f"{binding_id}.scope.stageId")
        action_id = _stable(scope["actionId"], f"{binding_id}.scope.actionId")
        action_matches = action_index.get(action_id, [])
        exact_matches = [
            candidate
            for candidate in action_matches
            if candidate["patternId"] == pattern_id and candidate["stageId"] == stage_id
        ]
        if len(action_matches) != 1 or len(exact_matches) != 1:
            raise BindingContractError(
                f"{binding_id}.scope does not resolve one exact Pattern/Stage/action"
            )
        stage_duration = exact_matches[0]["durationMs"]

        clock = _exact(row["clock"], CLOCK_FIELDS, f"{binding_id}.clock")
        basis = clock["basis"]
        if basis not in CLOCK_BASES:
            raise BindingContractError(f"{binding_id}.clock.basis is invalid")
        start_ms = _ms(clock["startMs"], f"{binding_id}.clock.startMs")
        repeat_policy = clock["repeatPolicy"]
        if repeat_policy not in REPEAT_POLICIES:
            raise BindingContractError(f"{binding_id}.clock.repeatPolicy is invalid")
        occurrence_id = clock["clipOccurrenceId"]
        if basis == "STAGE":
            if occurrence_id is not None or repeat_policy != "ONCE":
                raise BindingContractError(
                    f"{binding_id} STAGE clock requires null clipOccurrenceId and ONCE"
                )
        else:
            occurrence_id = _stable(
                occurrence_id, f"{binding_id}.clock.clipOccurrenceId"
            )
            occurrence_matches = occurrence_index.get(occurrence_id, [])
            if len(occurrence_matches) != 1:
                raise BindingContractError(
                    f"{binding_id}.clock.clipOccurrenceId is missing or ambiguous"
                )
            occurrence = occurrence_matches[0]
            if occurrence["actionId"] != action_id:
                raise BindingContractError(
                    f"{binding_id}.clock occurrence is not owned by scope.actionId"
                )
            if repeat_policy == "EACH_LOOP" and occurrence["loop"] is not True:
                raise BindingContractError(
                    f"{binding_id} EACH_LOOP targets a non-loop occurrence"
                )
            play_ms = occurrence["playMs"]
            if play_ms and start_ms > play_ms:
                raise BindingContractError(
                    f"{binding_id}.clock.startMs exceeds the clip occurrence"
                )
        if stage_duration and start_ms > stage_duration:
            raise BindingContractError(
                f"{binding_id}.clock.startMs exceeds the scoped Stage"
            )

        anchor = _exact(row["anchor"], ANCHOR_FIELDS, f"{binding_id}.anchor")
        _stable(anchor["slotId"], f"{binding_id}.anchor.slotId")
        if anchor["followPolicy"] not in FOLLOW_POLICIES:
            raise BindingContractError(f"{binding_id}.anchor.followPolicy is invalid")
        if anchor["rotationBasis"] not in ROTATION_BASES:
            raise BindingContractError(f"{binding_id}.anchor.rotationBasis is invalid")
        transform = _exact(
            anchor["localTransform"],
            TRANSFORM_FIELDS,
            f"{binding_id}.anchor.localTransform",
        )
        _vector3(transform["translation"], f"{binding_id}.translation")
        _vector3(transform["rotation"], f"{binding_id}.rotation")
        _vector3(transform["scale"], f"{binding_id}.scale", scale=True)
        if row["stopPolicy"] not in STOP_POLICIES:
            raise BindingContractError(f"{binding_id}.stopPolicy is invalid")
        if row["stopPolicy"] == "NATURAL" and natural_span is None:
            raise BindingContractError(
                f"{binding_id} NATURAL resource lifetime is unbounded"
            )
        if (
            row["stopPolicy"] == "CLIP_OCCURRENCE_END"
            and basis != "CLIP_OCCURRENCE"
        ):
            raise BindingContractError(
                f"{binding_id} CLIP_OCCURRENCE_END requires a clip occurrence clock"
            )

        semantic = copy.deepcopy(row)
        del semantic["bindingId"]
        semantic_identity = _canonical_bytes(semantic)
        if semantic_identity in semantic_identities:
            raise BindingContractError(
                f"duplicate Effect V2 semantic binding occurrence: {binding_id}"
            )
        semantic_identities.add(semantic_identity)
        validated.append(copy.deepcopy(row))

    if [row["bindingId"] for row in validated] != sorted(binding_ids):
        raise BindingContractError("BOSS_VALTAN Effect V2 bindings must be sorted by bindingId")

    # A repeated leaf at the same clock is valid when its full local transform
    # differs. Direct binding duplicates were rejected above, and every v2 group
    # rejects only a child whose complete semantic payload (excluding childId)
    # duplicates a sibling. Resource ID + time alone is therefore not identity.
    return copy.deepcopy(dict(document))


def _resource_read_set_rows(
    repository_root: Path, document: Mapping[str, Any]
) -> list[dict[str, str]]:
    authored, groups = _load_resource_documents(repository_root)
    requested: set[tuple[str, str]] = set()

    def add_group(group_id: str, stack: tuple[str, ...] = ()) -> None:
        if group_id in stack:
            raise BindingContractError(
                "Effect V2 group cycle is forbidden: " + " -> ".join((*stack, group_id))
            )
        if len(stack) >= MAX_GROUP_DEPTH:
            raise BindingContractError(
                f"Effect V2 group nesting exceeds {MAX_GROUP_DEPTH}: {group_id}"
            )
        entry = groups.get(group_id)
        if entry is None:
            raise BindingContractError(f"Effect V2 binding has no group document: {group_id}")
        requested.add(("GROUP", group_id))
        _duration_ms, children = _group_children(
            group_id, authored, groups, require_v2=True
        )
        for child in children:
            if child["kind"] == "LEAF":
                effect_id = child["id"]
                _validate_leaf_resource(
                    effect_id, authored[effect_id], read_json(authored[effect_id])
                )
                requested.add(("LEAF", effect_id))
            else:
                add_group(child["id"], (*stack, group_id))

    bindings = document.get("bindings")
    if not isinstance(bindings, list):
        raise BindingContractError("Effect V2 binding read-set requires a bindings array")
    for ordinal, row in enumerate(bindings):
        if not isinstance(row, dict) or not isinstance(row.get("resource"), dict):
            raise BindingContractError(
                f"Effect V2 binding read-set row is invalid: {ordinal}"
            )
        kind = row["resource"].get("kind")
        raw_resource_id = row["resource"].get("id")
        resource_id = (
            _effect_id(raw_resource_id, f"bindings[{ordinal}].resource.id")
            if kind == "LEAF"
            else _stable(raw_resource_id, f"bindings[{ordinal}].resource.id")
        )
        if kind == "LEAF":
            if resource_id not in authored:
                raise BindingContractError(
                    f"Effect V2 binding has no authored leaf: {resource_id}"
                )
            _validate_leaf_resource(
                resource_id, authored[resource_id], read_json(authored[resource_id])
            )
            requested.add((kind, resource_id))
        elif kind == "GROUP":
            _resolve_group(resource_id, authored, groups, require_v2=True)
            add_group(resource_id)
        else:
            raise BindingContractError(f"bindings[{ordinal}].resource.kind is invalid")

    rows: list[dict[str, str]] = []
    for kind, resource_id in sorted(requested):
        path = authored[resource_id] if kind == "LEAF" else groups[resource_id][0]
        try:
            relative = path.resolve().relative_to(repository_root.resolve()).as_posix()
            payload = path.read_bytes()
        except (OSError, ValueError) as exc:
            raise BindingContractError(
                f"Effect V2 resource read-set path is unavailable: {resource_id}: {exc}"
            ) from exc
        rows.append(
            {
                "kind": kind,
                "id": resource_id,
                "path": relative,
                "sha256": _sha256(payload),
            }
        )
    return rows


def build_resource_read_set(
    repository_root: Path, document: Mapping[str, Any]
) -> dict[str, Any]:
    """Capture current raw bodies at catalog-Reload time.

    A UI must retain this returned value with its immutable catalog snapshot.
    Re-running this helper when Save is pressed would bless intervening drift and
    is not a valid Composition read-set acquisition strategy.
    """
    if document.get("archetypeId") != VALTAN_ARCHETYPE_ID:
        raise BindingContractError("Effect V2 read-set only admits BOSS_VALTAN")
    rows = _resource_read_set_rows(repository_root, document)
    if len(rows) > 8192:
        raise BindingContractError("Effect V2 resource read-set exceeds 8192 resources")
    identity = {"archetypeId": VALTAN_ARCHETYPE_ID, "resources": rows}
    return {
        "schema": READ_SET_SCHEMA,
        "formatVersion": READ_SET_FORMAT_VERSION,
        "archetypeId": VALTAN_ARCHETYPE_ID,
        "resources": rows,
        "readSetHash": _sha256(_canonical_bytes(identity)),
    }


def validate_resource_read_set(snapshot: Mapping[str, Any]) -> dict[str, Any]:
    snapshot = _exact(snapshot, READ_SET_FIELDS, "Effect V2 resource read-set")
    if (
        snapshot["schema"] != READ_SET_SCHEMA
        or not _is_format_version(
            snapshot["formatVersion"], READ_SET_FORMAT_VERSION
        )
        or snapshot["archetypeId"] != VALTAN_ARCHETYPE_ID
    ):
        raise BindingContractError("Effect V2 resource read-set header is invalid")
    rows = snapshot["resources"]
    if not isinstance(rows, list) or len(rows) > 8192:
        raise BindingContractError(
            "Effect V2 resource read-set resources must be an array <= 8192"
        )
    normalized: list[dict[str, str]] = []
    seen: set[tuple[str, str]] = set()
    for ordinal, raw in enumerate(rows):
        row = _exact(raw, READ_SET_ROW_FIELDS, f"readSet.resources[{ordinal}]")
        kind = row["kind"]
        if kind not in RESOURCE_KINDS:
            raise BindingContractError(f"readSet.resources[{ordinal}].kind is invalid")
        resource_id = (
            _effect_id(row["id"], f"readSet.resources[{ordinal}].id")
            if kind == "LEAF"
            else _stable(row["id"], f"readSet.resources[{ordinal}].id")
        )
        path = row["path"]
        expected_prefix = "Data/Effects/V2/Authored/" if kind == "LEAF" else "Data/Effects/V2/Groups/"
        expected_suffix = ".effectv2.json" if kind == "LEAF" else ".effectv2group.json"
        if (
            not isinstance(path, str)
            or "\\" in path
            or not path.startswith(expected_prefix)
            or path != f"{expected_prefix}{resource_id}{expected_suffix}"
        ):
            raise BindingContractError(f"readSet.resources[{ordinal}].path is not canonical")
        digest = row["sha256"]
        if not isinstance(digest, str) or SHA256.fullmatch(digest) is None:
            raise BindingContractError(f"readSet.resources[{ordinal}].sha256 is invalid")
        identity = (kind, resource_id)
        if identity in seen:
            raise BindingContractError(f"duplicate Effect V2 read-set resource: {identity}")
        seen.add(identity)
        normalized.append(dict(row))
    if normalized != sorted(normalized, key=lambda row: (row["kind"], row["id"])):
        raise BindingContractError("Effect V2 resource read-set is not canonically sorted")
    expected_hash = _sha256(
        _canonical_bytes(
            {"archetypeId": VALTAN_ARCHETYPE_ID, "resources": normalized}
        )
    )
    if snapshot["readSetHash"] != expected_hash:
        raise BindingContractError("Effect V2 resource read-set hash is invalid")
    return copy.deepcopy(dict(snapshot))


def assert_resource_read_set_current(
    repository_root: Path,
    candidate_document: Mapping[str, Any],
    expected_snapshot: Mapping[str, Any],
) -> dict[str, Any]:
    expected = validate_resource_read_set(expected_snapshot)
    current = build_resource_read_set(repository_root, candidate_document)
    if expected != current:
        raise BindingReadSetStaleError(
            "Effect V2 referenced resource body changed; Reload Composition resources "
            "before Save"
        )
    return current


def _legacy_resource(row: Mapping[str, Any], owner: str) -> dict[str, str]:
    effect_id = row.get("effectId")
    group_id = row.get("group")
    has_effect = isinstance(effect_id, str) and bool(effect_id)
    has_group = isinstance(group_id, str) and bool(group_id)
    if has_effect == has_group:
        raise BindingContractError(f"{owner} needs exactly one effectId/group")
    return {
        "kind": "LEAF" if has_effect else "GROUP",
        "id": (
            _effect_id(effect_id, f"{owner}.resource")
            if has_effect
            else _stable(group_id, f"{owner}.resource")
        ),
    }


def _legacy_anchor(row: Mapping[str, Any], owner: str) -> dict[str, Any]:
    bone = row.get("bone")
    if not isinstance(bone, str):
        raise BindingContractError(f"{owner}.bone is invalid")
    follow = row.get("followBone")
    stop = row.get("stopWithClip")
    if not isinstance(follow, bool) or not isinstance(stop, bool):
        raise BindingContractError(f"{owner} boolean fields are invalid")
    rotation = row.get("rotation")
    rotation_basis = {"Bone": "SLOT", "TargetYaw": "TARGET_YAW", "World": "WORLD"}.get(rotation)
    if rotation_basis is None:
        raise BindingContractError(f"{owner}.rotation is invalid")
    translation = list(_vector3(row.get("offset", [0, 0, 0]), f"{owner}.offset"))
    yaw = row.get("yawDegrees", 0)
    if isinstance(yaw, bool) or not isinstance(yaw, (int, float)) or not math.isfinite(float(yaw)):
        raise BindingContractError(f"{owner}.yawDegrees is invalid")
    return {
        "slotId": bone if bone else "OWNER_ROOT",
        "followPolicy": "FOLLOW_SLOT" if follow else "SNAPSHOT_AT_START",
        "rotationBasis": rotation_basis,
        "localTransform": {
            "translation": translation,
            "rotation": [0.0, float(yaw), 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }


def migrate_v1_group_document(
    legacy: Mapping[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Create a deterministic v2 group candidate without writing canonical data."""

    if (
        not isinstance(legacy, Mapping)
        or legacy.get("schema") != GROUP_SCHEMA
        or not _is_format_version(legacy.get("formatVersion"), 1)
    ):
        raise BindingContractError("legacy Effect V2 group header/version is invalid")
    group_id = _stable(legacy.get("groupId"), "legacy Effect V2 group.groupId")
    duration_ms = _ms(
        legacy.get("durationMs", 0), f"legacy Effect V2 group {group_id}.durationMs"
    )
    rows = legacy.get("children")
    if not isinstance(rows, list) or not rows:
        raise BindingContractError(f"legacy Effect V2 group children are invalid: {group_id}")
    migrated: list[dict[str, Any]] = []
    report_rows: list[dict[str, Any]] = []
    for ordinal, raw in enumerate(rows):
        owner = f"legacy Effect V2 group {group_id}.children[{ordinal}]"
        if not isinstance(raw, Mapping):
            raise BindingContractError(f"{owner} is invalid")
        effect_id = _effect_id(raw.get("effectId"), f"{owner}.effectId")
        start_ms = _ms(raw.get("startMs", 0), f"{owner}.startMs")
        child_duration_ms = _ms(raw.get("durationMs", 0), f"{owner}.durationMs")
        stop = raw.get("stop", "Deactivate")
        if stop not in GROUP_CHILD_STOPS:
            raise BindingContractError(f"{owner}.stop is invalid")
        translation = list(_vector3(raw.get("offset", [0, 0, 0]), f"{owner}.offset"))
        rotation = [
            _finite_number(raw.get("pitchDegrees", 0), f"{owner}.pitchDegrees"),
            _finite_number(raw.get("yawDegrees", 0), f"{owner}.yawDegrees"),
            _finite_number(raw.get("rollDegrees", 0), f"{owner}.rollDegrees"),
        ]
        scale = list(
            _vector3(raw.get("scale", [1, 1, 1]), f"{owner}.scale", scale=True)
        )
        identity = {
            "legacyOrdinal": ordinal,
            "resource": {"kind": "LEAF", "id": effect_id},
            "startMs": start_ms,
            "durationMs": child_duration_ms,
            "stop": stop,
            "localTransform": {
                "translation": translation,
                "rotation": rotation,
                "scale": scale,
            },
        }
        child_id = (
            f"child.migrated.{ordinal:03d}.{_sha256(_canonical_bytes(identity))[:16]}"
        )
        migrated.append(
            {
                "childId": child_id,
                "resource": {"kind": "LEAF", "id": effect_id},
                "startMs": start_ms,
                "durationMs": child_duration_ms,
                "stop": stop,
                "localTransform": identity["localTransform"],
            }
        )
        report_rows.append(
            {
                "legacyChildIndex": ordinal,
                "childId": child_id,
                "resource": {"kind": "LEAF", "id": effect_id},
            }
        )
    return (
        {
            "schema": GROUP_SCHEMA,
            "formatVersion": GROUP_FORMAT_VERSION,
            "groupId": group_id,
            "durationMs": duration_ms,
            "children": migrated,
        },
        {
            "schema": "lostark.effect-v2-group-migration-report",
            "formatVersion": 1,
            "groupId": group_id,
            "legacyChildCount": len(rows),
            "migratedChildCount": len(migrated),
            "rows": report_rows,
        },
    )


def migrate_v1_document(
    legacy: Mapping[str, Any],
    gameplay: Mapping[str, Any],
    animation: Mapping[str, Any],
    legacy_compatibility: Mapping[str, Any] | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Expand global clip-name fan-out into every exact admitted occurrence."""

    legacy = _exact(legacy, ROOT_FIELDS, "legacy BOSS_VALTAN Effect V2 bindings")
    if (
        legacy["schema"] != BINDING_SCHEMA
        or not _is_format_version(legacy["formatVersion"], 1)
        or legacy["archetypeId"] != VALTAN_ARCHETYPE_ID
        or not isinstance(legacy["bindings"], list)
    ):
        raise BindingContractError("legacy BOSS_VALTAN binding header/version is invalid")
    action_index = _canonical_action_index(gameplay, legacy_compatibility)
    by_clip, occurrence_index = _animation_occurrence_indexes(animation)
    migrated: list[dict[str, Any]] = []
    report_rows: list[dict[str, Any]] = []
    ambiguities: list[dict[str, Any]] = []

    for ordinal, row in enumerate(legacy["bindings"]):
        owner = f"legacy bindings[{ordinal}]"
        if not isinstance(row, dict):
            raise BindingContractError(f"{owner} must be an object")
        resource = _legacy_resource(row, owner)
        anchor = _legacy_anchor(row, owner)
        start_ms = _ms(row.get("startMs"), f"{owner}.startMs")
        targets: list[tuple[dict[str, Any], dict[str, Any] | None]] = []
        has_stage = isinstance(row.get("stage"), str) and bool(row.get("stage"))
        has_clip = isinstance(row.get("clip"), str) and bool(row.get("clip"))
        if has_stage == has_clip:
            raise BindingContractError(f"{owner} needs exactly one stage/clip")
        stop_policy = (
            ("STAGE_END" if has_stage else "CLIP_OCCURRENCE_END")
            if row.get("stopWithClip")
            else "NATURAL"
        )
        if has_stage:
            action_id = _stable(row["stage"], f"{owner}.stage")
            matches = action_index.get(action_id, [])
            if len(matches) != 1:
                ambiguities.append(
                    {
                        "legacyRowIndex": ordinal,
                        "kind": "STAGE",
                        "source": action_id,
                        "reason": "action does not resolve one Pattern/Stage owner",
                        "candidateScopes": copy.deepcopy(matches),
                    }
                )
                continue
            targets.append((matches[0], None))
        else:
            clip_name = row["clip"]
            occurrences = by_clip.get(clip_name, [])
            if not occurrences:
                ambiguities.append(
                    {
                        "legacyRowIndex": ordinal,
                        "kind": "CLIP_NAME",
                        "source": clip_name,
                        "reason": "clip has no admitted occurrence",
                        "candidateScopes": [],
                    }
                )
                continue
            for occurrence in occurrences:
                occurrence_id = occurrence["clipOccurrenceId"]
                if len(occurrence_index.get(occurrence_id, [])) != 1:
                    ambiguities.append(
                        {
                            "legacyRowIndex": ordinal,
                            "kind": "CLIP_NAME",
                            "source": clip_name,
                            "reason": "clipOccurrenceId has multiple owners",
                            "clipOccurrenceId": occurrence_id,
                        }
                    )
                    continue
                scopes = action_index.get(occurrence["actionId"], [])
                if len(scopes) != 1:
                    ambiguities.append(
                        {
                            "legacyRowIndex": ordinal,
                            "kind": "CLIP_NAME",
                            "source": clip_name,
                            "reason": "occurrence action does not resolve one Pattern/Stage owner",
                            "clipOccurrenceId": occurrence_id,
                            "candidateScopes": copy.deepcopy(scopes),
                        }
                    )
                    continue
                targets.append((scopes[0], occurrence))
        report_rows.append(
            {
                "legacyRowIndex": ordinal,
                "sourceKind": "STAGE" if has_stage else "CLIP_NAME",
                "source": row["stage"] if has_stage else row["clip"],
                "expandedOccurrenceCount": len(targets),
                "expandedScopes": [
                    {
                        "patternId": scope["patternId"],
                        "stageId": scope["stageId"],
                        "actionId": scope["actionId"],
                        "clipOccurrenceId": (
                            None if occurrence is None else occurrence["clipOccurrenceId"]
                        ),
                        "sourceOwner": scope["sourceOwner"],
                    }
                    for scope, occurrence in targets
                ],
            }
        )
        for expansion_ordinal, (scope, occurrence) in enumerate(targets):
            occurrence_id = (
                None if occurrence is None else occurrence["clipOccurrenceId"]
            )
            local_start_ms = (
                start_ms
                if occurrence is None
                else max(0, start_ms - occurrence["sourceStartMs"])
            )
            seed = {
                "legacyRowIndex": ordinal,
                "expansionOrdinal": expansion_ordinal,
                "resource": resource,
                "scope": scope,
                "clipOccurrenceId": occurrence_id,
                "startMs": local_start_ms,
                "anchor": anchor,
                "stopPolicy": stop_policy,
            }
            suffix = _sha256(_canonical_bytes(seed))[:16]
            migrated.append(
                {
                    "bindingId": f"binding.valtan.migrated.{ordinal:03d}.{suffix}",
                    "resource": copy.deepcopy(resource),
                    "scope": {
                        "patternId": scope["patternId"],
                        "stageId": scope["stageId"],
                        "actionId": scope["actionId"],
                    },
                    "clock": {
                        "basis": "STAGE" if occurrence_id is None else "CLIP_OCCURRENCE",
                        "clipOccurrenceId": occurrence_id,
                        "startMs": local_start_ms,
                        "repeatPolicy": (
                            "EACH_LOOP"
                            if occurrence is not None and occurrence["loop"] is True
                            else "ONCE"
                        ),
                    },
                    "anchor": copy.deepcopy(anchor),
                    "stopPolicy": stop_policy,
                }
            )

    report = {
        "schema": "lostark.effect-v2-binding-migration-report",
        "formatVersion": 1,
        "archetypeId": VALTAN_ARCHETYPE_ID,
        "legacyBindingCount": len(legacy["bindings"]),
        "migratedBindingCount": len(migrated),
        "rows": report_rows,
        "rejectedRows": ambiguities,
    }
    if ambiguities:
        raise BindingMigrationAmbiguityError(
            "legacy Effect V2 binding migration is ambiguous; no candidate was written",
            report,
        )
    migrated.sort(key=lambda row: row["bindingId"])
    return (
        {
            "schema": BINDING_SCHEMA,
            "formatVersion": BINDING_FORMAT_VERSION,
            "archetypeId": VALTAN_ARCHETYPE_ID,
            "bindings": migrated,
        },
        report,
    )


def apply_binding_mutations(
    document: Mapping[str, Any], operations: Sequence[Mapping[str, Any]]
) -> dict[str, Any]:
    """Apply exact mutations by stable bindingId; vector ordinals are never IDs."""

    candidate = copy.deepcopy(dict(document))
    rows = candidate.get("bindings")
    if not isinstance(rows, list) or not isinstance(operations, Sequence):
        raise BindingContractError("Effect V2 binding mutation input is invalid")

    def index() -> dict[str, int]:
        result: dict[str, int] = {}
        for ordinal, row in enumerate(rows):
            if not isinstance(row, dict):
                raise BindingContractError("Effect V2 binding mutation baseline is malformed")
            binding_id = _stable(row.get("bindingId"), "binding mutation baseline bindingId")
            if binding_id in result:
                raise BindingContractError(f"duplicate Effect V2 bindingId: {binding_id}")
            result[binding_id] = ordinal
        return result

    for ordinal, raw in enumerate(operations):
        if not isinstance(raw, Mapping):
            raise BindingContractError(f"binding mutation operations[{ordinal}] is invalid")
        operation = dict(raw)
        kind = operation.get("op")
        binding_id = _stable(
            operation.get("bindingId"), f"binding mutation operations[{ordinal}].bindingId"
        )
        positions = index()
        if binding_id not in positions:
            raise BindingContractError(f"unknown Effect V2 bindingId: {binding_id}")
        position = positions[binding_id]
        if kind == "DELETE_BINDING":
            _exact(operation, ("op", "bindingId"), f"binding mutation {ordinal}")
            del rows[position]
        elif kind == "DUPLICATE_BINDING":
            _exact(
                operation,
                ("op", "bindingId", "newBindingId"),
                f"binding mutation {ordinal}",
            )
            new_id = _stable(operation["newBindingId"], f"binding mutation {ordinal}.newBindingId")
            if new_id in positions:
                raise BindingContractError(f"duplicate Effect V2 bindingId: {new_id}")
            duplicate = copy.deepcopy(rows[position])
            duplicate["bindingId"] = new_id
            rows.append(duplicate)
        elif kind == "MOVE_BINDING":
            _exact(
                operation,
                ("op", "bindingId", "scope", "clock"),
                f"binding mutation {ordinal}",
            )
            rows[position]["scope"] = copy.deepcopy(operation["scope"])
            rows[position]["clock"] = copy.deepcopy(operation["clock"])
        elif kind == "UPDATE_BINDING":
            _exact(
                operation,
                (
                    "op",
                    "bindingId",
                    "resource",
                    "scope",
                    "clock",
                    "anchor",
                    "stopPolicy",
                ),
                f"binding mutation {ordinal}",
            )
            rows[position] = {
                "bindingId": binding_id,
                "resource": copy.deepcopy(operation["resource"]),
                "scope": copy.deepcopy(operation["scope"]),
                "clock": copy.deepcopy(operation["clock"]),
                "anchor": copy.deepcopy(operation["anchor"]),
                "stopPolicy": operation["stopPolicy"],
            }
        else:
            raise BindingContractError(f"unsupported Effect V2 binding mutation: {kind}")
    rows.sort(key=lambda row: row["bindingId"])
    return candidate


def _write_atomic_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.unlink(missing_ok=True)
    payload = json_text(value).encode("utf-8")
    try:
        with temporary.open("xb") as handle:
            handle.write(payload)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _reject_canonical_migration_output(repository_root: Path, path: Path) -> None:
    try:
        relative = path.resolve().relative_to(repository_root.resolve()).as_posix()
    except ValueError:
        return
    if relative == "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json" or (
        relative.startswith("Data/Effects/V2/Groups/")
        and relative.endswith(".effectv2group.json")
    ):
        raise BindingContractError(
            "Effect V2 helper CLI cannot write a canonical owner; write a staged "
            "candidate and commit it through the Composition writer lock"
        )


def _validate_helper_output_paths(
    repository_root: Path,
    *,
    input_path: Path | None = None,
    output_path: Path,
    report_path: Path | None = None,
) -> None:
    resolved = [output_path.resolve()]
    if report_path is not None:
        resolved.append(report_path.resolve())
    for path in resolved:
        _reject_canonical_migration_output(repository_root, path)
    if len(set(resolved)) != len(resolved):
        raise BindingContractError("Effect V2 helper output/report paths must be distinct")
    if input_path is not None and input_path.resolve() in resolved:
        raise BindingContractError(
            "Effect V2 helper input and output/report paths must be distinct"
        )


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=Path(__file__).resolve().parents[2])
    subparsers = parser.add_subparsers(dest="command", required=True)
    migrate = subparsers.add_parser("migrate-v1")
    migrate.add_argument("--input", type=Path, required=True)
    migrate.add_argument("--output", type=Path, required=True)
    migrate.add_argument("--report", type=Path, required=True)
    migrate_group = subparsers.add_parser("migrate-group-v1")
    migrate_group.add_argument("--input", type=Path, required=True)
    migrate_group.add_argument("--output", type=Path, required=True)
    migrate_group.add_argument("--report", type=Path, required=True)
    snapshot = subparsers.add_parser(
        "snapshot",
        help="capture current bodies for a catalog Reload fixture; never recompute at Save",
    )
    snapshot.add_argument("--bindings", type=Path, required=True)
    snapshot.add_argument("--output", type=Path, required=True)
    validate = subparsers.add_parser("validate")
    validate.add_argument("--bindings", type=Path, required=True)
    arguments = parser.parse_args(argv)
    repository_root = arguments.repository_root.resolve()
    try:
        if arguments.command == "migrate-v1":
            _validate_helper_output_paths(
                repository_root,
                input_path=arguments.input,
                output_path=arguments.output,
                report_path=arguments.report,
            )
            migrated, report = migrate_v1_document(
                read_json(arguments.input.resolve()),
                read_json(repository_root / "Data/Valtan/Valtan.gameplay.json"),
                read_json(
                    repository_root
                    / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
                ),
                read_json(
                    repository_root
                    / "Data/Valtan/Valtan.legacy-compatibility.json"
                ),
            )
            _write_atomic_json(arguments.output.resolve(), migrated)
            _write_atomic_json(arguments.report.resolve(), report)
            result = report
        elif arguments.command == "migrate-group-v1":
            _validate_helper_output_paths(
                repository_root,
                input_path=arguments.input,
                output_path=arguments.output,
                report_path=arguments.report,
            )
            migrated, report = migrate_v1_group_document(
                read_json(arguments.input.resolve())
            )
            _write_atomic_json(arguments.output.resolve(), migrated)
            _write_atomic_json(arguments.report.resolve(), report)
            result = report
        elif arguments.command == "snapshot":
            _validate_helper_output_paths(
                repository_root,
                input_path=arguments.bindings,
                output_path=arguments.output,
            )
            document = read_json(arguments.bindings.resolve())
            result = build_resource_read_set(repository_root, document)
            _write_atomic_json(arguments.output.resolve(), result)
        else:
            document = read_json(arguments.bindings.resolve())
            validate_binding_document(
                repository_root,
                document,
                read_json(repository_root / "Data/Valtan/Valtan.gameplay.json"),
                read_json(
                    repository_root
                    / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
                ),
                read_json(
                    repository_root
                    / "Data/Valtan/Valtan.legacy-compatibility.json"
                ),
            )
            result = {"bindingCount": len(document["bindings"])}
    except BindingMigrationAmbiguityError as exc:
        _write_atomic_json(arguments.report.resolve(), exc.report)
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    except BindingContractError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
