"""Build the fail-closed Effect Program/Layout/Adapter cohort inventory.

This tool inventories authored occurrences.  It does not publish Effect data,
compile shaders, prove a renderer draw, or admit a Product result.  In
particular, a family representative DXBC is never promoted to an occurrence
Program and named native ABI evidence is never promoted to a typed packet.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

import validate_direct_authored_effect_runtime as direct_authored_runtime_contract


SCHEMA = "lostark.effect-tuple-cohort-inventory"
FORMAT_VERSION = 1
REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
OUTPUT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json"
)

AUTHORED_DIRECTORY = Path("Data/Effects/Authored")
CONTRACT_DIRECTORY = Path("Data/Effects/Contracts")
COOKED_SHADER_DIRECTORY = Path("Data/Effects/CookedShaders")
TRANSLATED_SHADER_DIRECTORY = Path("Data/Effects/TranslatedShaders")

SHADER_MAP_PATH = CONTRACT_DIRECTORY / "effect-family-shader-map-index.v1.json"
COOKED_PATH = CONTRACT_DIRECTORY / "effect-family-cooked-pixel-shaders.v1.json"
TRANSLATIONS_PATH = CONTRACT_DIRECTORY / "effect-family-hlsl-translations.v1.json"
NAMED_ABI_PATH = CONTRACT_DIRECTORY / "effect-family-named-abi.v1.json"
CHILD_PARENT_PATH = CONTRACT_DIRECTORY / "effect-child-parent-resolution.v1.json"
G00_PATH = CONTRACT_DIRECTORY / "effect-family-shader-inventory.v1.json"
RESTORATION_PATH = CONTRACT_DIRECTORY / "character-effect-restoration-targets.v1.json"
EXACT_VARIANTS_PATH = CONTRACT_DIRECTORY / "ue3-exact-cooked-shader-variants.v1.json"
AUTHORING_CATALOG_PATH = Path("Data/Effects/EffectCatalog.json")
RUNTIME_CATALOG_PATH = Path("Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json")
PLAYER_SKILLS_PATH = Path("Data/Balance/PlayerSkills.json")
VALTAN_PATTERN_BINDINGS_PATH = Path(
    "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
VALTAN_CUES_PATH = Path(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
BOSS_CATALOG_PATH = Path("Data/Actors/BossCatalog.json")
VALTAN_ENCOUNTER_PATH = Path("Data/Encounters/Valtan/ValtanEncounter.json")
VALTAN_ANIMNOTIFY_PATH = Path("Data/Animation/Reference/Valtan/Valtan.animnotify")
VALTAN_COMBAT_OBJECTS_PATH = Path("Data/Encounters/Valtan/ValtanCombatObjects.json")

TARGET_DOMAINS = {
    "effect.artist.": ("Artist", "ARTIST"),
    "effect.dimensionmaster.": ("DimensionMaster", "DIMENSIONMASTER"),
    "effect.lancemaster.": ("LanceMaster", "LANCE_MASTER"),
    "effect.warlord.": ("Warlord", "WARLORD"),
    "effect.valtan.": ("Valtan", "BOSS_VALTAN"),
}

EXPECTED_DOCUMENT_COUNTS = {
    "Artist": 35,
    "DimensionMaster": 60,
    "LanceMaster": 102,
    "Valtan": 158,
    "Warlord": 61,
}
EXPECTED_OCCURRENCE_COUNTS = {
    "Artist": 615,
    "DimensionMaster": 2169,
    "LanceMaster": 2588,
    "Valtan": 780,
    "Warlord": 1414,
}
EXPECTED_CARRIER_COUNTS = {
    "DECAL": 83,
    "MESH": 2539,
    "PRESENTATION": 109,
    "RIBBON": 19,
    "SPRITE": 4816,
}
EXPECTED_EXECUTION_COUNT = 50
EXPECTED_SOURCE_PROFILE_COUNT = 2732
EXPECTED_NO_MATERIAL_EVIDENCE_COUNT = 4784
EXPECTED_AUTHORING_CATALOG_ASSET_COUNT = 256
EXPECTED_RUNTIME_PRODUCT_ASSET_COUNT = 145
EXPECTED_RUNTIME_PRODUCT_OCCURRENCE_COUNT = 2554
EXPECTED_CHARACTER_PRODUCT_ASSET_COUNT = 99
EXPECTED_VALTAN_PATTERN_ASSET_COUNT = 44
EXPECTED_VALTAN_BOSS_VISUAL_ASSET_COUNT = 2
EXPECTED_NAMED_ABI_FITS = 69
EXPECTED_NAMED_ABI_REQUIRES_EXTENSION = 93
EXPECTED_LEGACY_REVIEW_PROJECTION_SHA256 = (
    "dfe511ca5aa4781fc87999f249f9b0f470fdf24ff92011edd8dcb09259a92906"
)
EXPECTED_COMPOSITION_PROJECTION_SHA256 = (
    "9729794c95502fb81c853d404e4fb4e7d946fecad310ddd82bc461e5b53dcfd1"
)

MAX_TEXTURE_LANES = 6
MAX_PACKED_SCALARS = 52
MAX_PACKED_VECTORS = 3

PROGRAM_STATUSES = (
    "TYPED_RUNTIME_PROGRAM_DECLARED",
    "DXBC_OCCURRENCE_EXACT",
    "DXBC_OCCURRENCE_EXACT_UNTRANSLATED",
    "DXBC_FAMILY_REPRESENTATIVE_ONLY",
    "BOUNDED_SOURCE_PROFILE_ONLY",
    "NO_PROGRAM_EVIDENCE",
    "NOT_APPLICABLE_PRESENTATION",
)
LAYOUT_STATUSES = (
    "TYPED_PACKET_CLOSED",
    "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION",
    "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
    "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION",
    "SOURCE_NAMES_ONLY",
    "UNRESOLVED",
    "NOT_APPLICABLE_PRESENTATION",
)
ADAPTER_STATUSES = (
    "TYPED_STATIC_DISPATCH_CANDIDATE",
    "RENDER_PROFILE_STATIC_CANDIDATE",
    "UNRESOLVED",
    "PRESENTATION_SEPARATE",
)
DESCRIPTOR_STATUSES = (
    "TYPED_VALUES_CLOSED",
    "SOURCE_VALUES_PRESENT_UNPACKED",
    "RESOURCE_ONLY_NO_MATERIAL_VALUES",
    "MISSING",
    "NOT_APPLICABLE_PRESENTATION",
)
COMPOSITION_STATUSES = (
    "PRODUCT_BOUND_CUE",
    "RUNTIME_PUBLISHED_WITHOUT_CONSUMER",
    "CATALOG_DECLARED_ONLY",
    "AUTHORED_ONLY",
)
PRODUCT_STATUSES = (
    "PRODUCT_JOIN_CLOSED",
    "PUBLISHED_ELEMENT_STALE",
    "RUNTIME_PUBLISHED_UNCONSUMED",
    "CATALOG_NOT_PUBLISHED",
    "AUTHORED_NOT_CATALOGED",
)
USER_REVIEW_STATUSES = (
    "APPROVED",
    "PENDING",
    "NOT_RECORDED",
    "STALE_REVIEW_RECEIPT",
)

SHA256_RE = re.compile(r"[0-9a-f]{64}")
STABLE_ID_RE = re.compile(r"[A-Za-z0-9_.-]{1,128}")
ANIMEVENT_RE = re.compile(r'^"(?P<clip>[^"]+)"\s+EFFECT\s+(?P<fields>.*)$')
ANIMEVENT_FIELD_RE = re.compile(r'(\w+)=("[^"]*"|\S+)')


class InventoryError(ValueError):
    """A contract violation detected before output replacement."""


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise InventoryError(message)


def _reject_non_finite(token: str) -> None:
    raise InventoryError("JSON contains a non-finite number: " + token)


def _object_without_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise InventoryError("JSON contains a duplicate key: " + key)
        result[key] = value
    return result


def _decode_json(payload: bytes, label: str) -> Any:
    try:
        text = payload.decode("utf-8")
    except UnicodeError as error:
        raise InventoryError(label + " is not UTF-8") from error
    try:
        return json.loads(
            text,
            object_pairs_hook=_object_without_duplicate_keys,
            parse_constant=_reject_non_finite,
        )
    except InventoryError:
        raise
    except (TypeError, ValueError) as error:
        raise InventoryError(label + " is not valid JSON") from error


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=1, allow_nan=False) + "\n").encode(
        "utf-8"
    )


def _sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _require_string(value: Any, label: str) -> str:
    _require(isinstance(value, str) and bool(value), label + " must be a non-empty string")
    return value


def _require_int(value: Any, label: str, minimum: int = 0) -> int:
    _require(
        isinstance(value, int) and not isinstance(value, bool) and value >= minimum,
        label + " must be an integer >= " + str(minimum),
    )
    return value


def _require_sha256(value: Any, label: str) -> str:
    _require(isinstance(value, str) and SHA256_RE.fullmatch(value) is not None, label + " is not SHA-256")
    return value


def _require_stable_id(value: Any, label: str) -> str:
    stable_id = _require_string(value, label)
    _require(STABLE_ID_RE.fullmatch(stable_id) is not None, label + " is not a stable ID")
    return stable_id


def _safe_relative_path(value: Any, label: str) -> str:
    path = _require_string(value, label).replace("\\", "/")
    pure = PurePosixPath(path)
    _require(not pure.is_absolute(), label + " must be repository-relative")
    _require(".." not in pure.parts, label + " must not escape the repository")
    _require(not re.match(r"^[A-Za-z]:", path), label + " must not be drive-qualified")
    return pure.as_posix()


class InputTracker:
    def __init__(self, root: Path) -> None:
        self.root = root
        self._rows: dict[str, dict[str, Any]] = {}

    def read_bytes(self, relative_path: Path | str, role: str, require_lf: bool = False) -> bytes:
        relative = Path(relative_path).as_posix()
        absolute = self.root / Path(relative)
        _require(absolute.is_file(), "required input is absent: " + relative)
        payload = absolute.read_bytes()
        if require_lf:
            _require(b"\r" not in payload, "input must be LF-only: " + relative)
        self._record(relative, role, payload, None)
        return payload

    def read_json(self, relative_path: Path | str, role: str, require_lf: bool = False) -> Any:
        relative = Path(relative_path).as_posix()
        absolute = self.root / Path(relative)
        _require(absolute.is_file(), "required input is absent: " + relative)
        payload = absolute.read_bytes()
        if require_lf:
            _require(b"\r" not in payload, "input must be LF-only: " + relative)
        document = _decode_json(payload, relative)
        self._record(relative, role, payload, canonical_sha256(document))
        return document

    def add_parsed(self, relative_path: Path | str, role: str, payload: bytes, document: Any) -> None:
        self._record(Path(relative_path).as_posix(), role, payload, canonical_sha256(document))

    def _record(self, relative: str, role: str, payload: bytes, canonical: str | None) -> None:
        identity = {
            "path": relative,
            "roles": [role],
            "rawSha256": _sha256_bytes(payload),
            "byteSize": len(payload),
            "canonicalJsonSha256": canonical,
        }
        existing = self._rows.get(relative)
        if existing is None:
            self._rows[relative] = identity
            return
        _require(
            existing["rawSha256"] == identity["rawSha256"]
            and existing["byteSize"] == identity["byteSize"]
            and existing["canonicalJsonSha256"] == canonical,
            "input identity changed during build: " + relative,
        )
        if role not in existing["roles"]:
            existing["roles"].append(role)
            existing["roles"].sort()

    def identity(self, relative_path: Path | str) -> dict[str, Any]:
        relative = Path(relative_path).as_posix()
        _require(relative in self._rows, "input was not tracked: " + relative)
        return self._rows[relative]

    def rows(self) -> list[dict[str, Any]]:
        return [self._rows[key] for key in sorted(self._rows)]


class VariantRegistry:
    def __init__(self, prefix: str) -> None:
        self.prefix = prefix
        self._payloads: dict[str, dict[str, Any]] = {}

    def add(self, payload: dict[str, Any]) -> str:
        identifier = self.prefix + canonical_sha256(payload)
        previous = self._payloads.setdefault(identifier, payload)
        _require(previous == payload, "content-address collision: " + identifier)
        return identifier

    def rows(self, id_field: str) -> list[dict[str, Any]]:
        return [
            {id_field: identifier, **self._payloads[identifier]}
            for identifier in sorted(self._payloads)
        ]


class CompositionRegistry:
    """Content-address semantic cues while retaining non-identity evidence locations."""

    def __init__(self) -> None:
        self._rows: dict[str, dict[str, Any]] = {}

    def add(self, semantic: dict[str, Any], evidence_location: dict[str, Any]) -> str:
        identifier = "composition." + canonical_sha256(semantic)
        row = self._rows.get(identifier)
        if row is None:
            self._rows[identifier] = {
                "compositionVariantId": identifier,
                "semantic": semantic,
                "evidenceLocations": [evidence_location],
            }
        else:
            _require(row["semantic"] == semantic, "composition hash collision")
            if evidence_location not in row["evidenceLocations"]:
                row["evidenceLocations"].append(evidence_location)
                row["evidenceLocations"].sort(key=canonical_json_bytes)
        return identifier

    def rows(self) -> list[dict[str, Any]]:
        return [self._rows[key] for key in sorted(self._rows)]


def _read_artifact(
    tracker: InputTracker,
    relative_path: Path,
    schema: str,
    role: str,
) -> dict[str, Any]:
    document = tracker.read_json(relative_path, role)
    _require(isinstance(document, dict), relative_path.as_posix() + " root must be an object")
    _require(document.get("schema") == schema, relative_path.as_posix() + " schema is unsupported")
    _require(document.get("formatVersion") == 1, relative_path.as_posix() + " formatVersion is unsupported")
    digest = _require_sha256(document.get("artifactSha256"), relative_path.as_posix() + ".artifactSha256")
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    _require(canonical_sha256(unsigned) == digest, relative_path.as_posix() + " artifactSha256 drifted")
    return document


def _index_unique(rows: Any, key: str, label: str) -> dict[str, dict[str, Any]]:
    _require(isinstance(rows, list), label + " must be an array")
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        _require(isinstance(row, dict), label + " row must be an object")
        value = _require_string(row.get(key), label + "." + key)
        _require(value not in result, "duplicate " + label + "." + key + ": " + value)
        row_sha = row.get("rowSha256")
        if row_sha is not None:
            unsigned = dict(row)
            unsigned.pop("rowSha256", None)
            _require(row_sha == canonical_sha256(unsigned), label + " rowSha256 drifted: " + value)
        result[value] = row
    return result


def _require_pin(
    owner: dict[str, Any],
    artifact_field: str,
    raw_field: str,
    dependency: dict[str, Any],
    dependency_identity: dict[str, Any],
    label: str,
) -> None:
    inputs = owner.get("inputs")
    _require(isinstance(inputs, dict), label + " has no inputs object")
    _require(inputs.get(artifact_field) == dependency.get("artifactSha256"), label + " artifact pin drifted")
    _require(inputs.get(raw_field) == dependency_identity["rawSha256"], label + " raw pin drifted")


def _domain_for_asset(effect_asset_id: str) -> tuple[str, str] | None:
    for prefix, domain in TARGET_DOMAINS.items():
        if effect_asset_id.startswith(prefix):
            return domain
    return None


def _occurrence_id(effect_asset_id: str, stable_id: str) -> str:
    return "occurrence." + canonical_sha256(
        {"effectAssetId": effect_asset_id, "stableId": stable_id}
    )


def _flatten_strings(value: Any, label: str) -> list[str]:
    result: list[str] = []
    if isinstance(value, str):
        _require(bool(value), label + " contains an empty string")
        result.append(value)
    elif isinstance(value, list):
        for index, child in enumerate(value):
            result.extend(_flatten_strings(child, f"{label}[{index}]"))
    else:
        raise InventoryError(label + " must contain only strings or arrays")
    return result


def _renderer_shape(element: dict[str, Any]) -> str:
    recipe = element.get("sourceRecipe")
    if not isinstance(recipe, dict):
        return ""
    shape = recipe.get("rendererShape")
    return shape.casefold() if isinstance(shape, str) else ""


def _mesh_model_assets(element: dict[str, Any]) -> list[str]:
    resources = element.get("resources")
    _require(isinstance(resources, list), "element.resources must be an array")
    result: list[str] = []
    seen_slots: set[str] = set()
    for resource in resources:
        _require(isinstance(resource, dict), "element resource must be an object")
        slot = _require_string(resource.get("slotId"), "resource.slotId")
        asset = _require_string(resource.get("assetId"), "resource.assetId")
        _require(slot not in seen_slots, "duplicate resource.slotId: " + slot)
        seen_slots.add(slot)
        if slot == "meshModel":
            result.append(asset)
    return result


def _coarse_carrier(element: dict[str, Any]) -> tuple[str, str, list[str]]:
    kind = _require_string(element.get("kind"), "element.kind")
    shape = _renderer_shape(element)
    mesh_models = _mesh_model_assets(element)
    if kind in ("light", "screenPost"):
        carrier = "PRESENTATION"
    elif kind == "decal":
        carrier = "DECAL"
    elif kind == "trail":
        carrier = "RIBBON"
    elif kind == "mesh":
        carrier = "MESH"
    elif kind == "sprite":
        carrier = "SPRITE"
    elif kind == "particle":
        carrier = "MESH" if bool(mesh_models) else "SPRITE"
    else:
        raise InventoryError("element has unknown kind: " + kind)
    return carrier, shape, mesh_models


def _source_type_data_classes(element: dict[str, Any]) -> list[str]:
    recipe = element.get("sourceRecipe")
    if not isinstance(recipe, dict):
        return []
    return sorted(
        {
            row.get("className").casefold()
            for row in recipe.get("modules", [])
            if isinstance(row, dict)
            and isinstance(row.get("className"), str)
            and "typedata" in row["className"].casefold()
        }
    )


def _dxbc_source_carrier(element: dict[str, Any], shape: str, type_data: list[str]) -> str:
    kind = element["kind"]
    if kind in ("mesh", "sprite", "decal"):
        return kind
    if kind == "particle":
        if "particlemoduletypedatamesh" in type_data:
            return "mesh"
        if "particlemoduletypedataribbon" in type_data:
            return "ribbon"
        if "particlemoduletypedataanimtrail" in type_data:
            return "animtrail"
        if shape in ("mesh", "sprite", "decal", "ribbon"):
            return shape
        return ""
    if kind == "trail":
        if "particlemoduletypedataribbon" in type_data:
            return "ribbon"
        if "particlemoduletypedataanimtrail" in type_data:
            return "animtrail"
        if shape:
            return shape
    return ""


def _fine_renderer_kind(kind: str, source_shape: str, mesh_models: list[str], type_data: list[str]) -> tuple[str, list[str]]:
    if kind == "mesh":
        return "STANDALONE_MESH", []
    if kind == "sprite":
        return "LEGACY_STANDALONE_SPRITE", []
    if kind == "decal":
        return "DECAL_PARTICLE", []
    if kind == "trail":
        if "particlemoduletypedataanimtrail" in type_data:
            return "ANIM_TRAIL", []
        if "particlemoduletypedataribbon" in type_data:
            return "CASCADE_RIBBON", []
        return "AUTHORED_LEGACY_TRAIL", []
    if kind == "particle" and "particlemoduletypedataribbon" in type_data:
        return "PARTICLE_TYPEDATA_RIBBON_REQUIRES_CARRIER_REPLACEMENT", []
    if kind == "particle" and "efparticlemoduletypedatadecal" in type_data:
        return "DECAL_PARTICLE", []
    if kind == "particle" and "particlemoduletypedataanimtrail" in type_data:
        return "ANIM_TRAIL", []
    if kind == "particle" and (mesh_models or "particlemoduletypedatamesh" in type_data):
        return "MESH_PARTICLE", []
    if kind == "particle":
        return "SPRITE_PARTICLE", []
    if kind == "light":
        return "LIGHT_PRESENTATION", []
    if kind == "screenPost":
        return "SCREEN_POST_PRESENTATION", []
    raise InventoryError("cannot derive fine renderer kind: " + kind)


def _source_profile(element: dict[str, Any]) -> dict[str, Any] | None:
    material = element.get("material")
    if not isinstance(material, dict):
        return None
    profile = material.get("sourceProfile")
    if isinstance(profile, dict) and profile.get("enabled") is True:
        return profile
    return None


def _raw_source_profile(element: dict[str, Any]) -> dict[str, Any]:
    """Return source metadata even when value replay is explicitly disabled.

    Program and Layout evidence are independent from Descriptor materialization.
    A disabled sourceProfile therefore still participates in parent/ShaderMap/
    DXBC/named-wire joins, while only an enabled sourceProfile may supply values.
    """
    material = element.get("material")
    if not isinstance(material, dict):
        return {}
    profile = material.get("sourceProfile")
    return profile if isinstance(profile, dict) else {}


def _execution(element: dict[str, Any]) -> dict[str, Any] | None:
    material = element.get("material")
    if not isinstance(material, dict):
        return None
    execution = material.get("execution")
    if isinstance(execution, dict) and execution.get("enabled") is True:
        return execution
    return None


def _validate_indexed_values(rows: Any, count: int, label: str, vector: bool) -> list[dict[str, Any]]:
    _require(isinstance(rows, list) and len(rows) == count, label + " count differs from its array")
    seen: set[int] = set()
    seen_names: set[str] = set()
    result: list[dict[str, Any]] = []
    for row in rows:
        _require(isinstance(row, dict), label + " row must be an object")
        _require(set(row) == {"name", "packedIndex", "value"}, label + " row has missing or hidden fields")
        index = _require_int(row.get("packedIndex"), label + ".packedIndex")
        _require(index < count, label + ".packedIndex is out of range")
        _require(index not in seen, "duplicate " + label + ".packedIndex")
        seen.add(index)
        name = _require_stable_id(row.get("name"), label + ".name")
        _require(name not in seen_names, "duplicate " + label + ".name: " + name)
        seen_names.add(name)
        value = row.get("value")
        if vector:
            _require(
                isinstance(value, list)
                and len(value) == 4
                and all(
                    isinstance(item, (int, float))
                    and not isinstance(item, bool)
                    and math.isfinite(float(item))
                    for item in value
                ),
                label + ".value must be float4",
            )
        else:
            _require(
                isinstance(value, (int, float))
                and not isinstance(value, bool)
                and math.isfinite(float(value)),
                label + ".value must be finite numeric",
            )
        result.append({"name": name, "packedIndex": index, "value": value})
    _require(seen == set(range(count)), label + " packed indices are not contiguous")
    return sorted(result, key=lambda item: item["packedIndex"])


def _validate_bounded_vector_values(rows: Any, capacity: int, label: str) -> list[dict[str, Any]]:
    _require(isinstance(rows, list) and len(rows) <= capacity, label + " exceeds capacity")
    seen_indices: set[int] = set()
    seen_names: set[str] = set()
    result: list[dict[str, Any]] = []
    for row in rows:
        _require(isinstance(row, dict) and set(row) == {"name", "packedIndex", "value"}, label + " row has missing or hidden fields")
        name = _require_stable_id(row.get("name"), label + ".name")
        index = _require_int(row.get("packedIndex"), label + ".packedIndex")
        value = row.get("value")
        _require(index < capacity and index not in seen_indices, label + " packed index is invalid or duplicate")
        _require(name not in seen_names, label + " name is duplicate")
        _require(
            isinstance(value, list)
            and len(value) == 4
            and all(isinstance(item, (int, float)) and not isinstance(item, bool) and math.isfinite(float(item)) for item in value),
            label + ".value must be finite float4",
        )
        seen_indices.add(index)
        seen_names.add(name)
        result.append({"name": name, "packedIndex": index, "value": value})
    return sorted(result, key=lambda item: item["packedIndex"])


def _typed_material_axes(execution: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any], dict[str, Any]]:
    allowed_fields = {
        "enabled", "version", "backend", "opcode", "passIndex", "renderState",
        "textureLaneCount", "textureMask", "textureLanes", "dynamicConsumedMask",
        "dynamicSuppressedMask", "particleColorPolicy", "particleColorConsumedMask",
        "particleColorSuppressedMask", "scalarCount", "vectorCount", "inputCount",
        "inputConsumedMask", "inputSuppressedMask", "vectorComponentConsumedMask",
        "vectorComponentSuppressedMask", "staticInputCount", "staticSelectedMask",
        "staticConsumedMask", "staticSuppressedMask", "renderInputCount",
        "renderConsumedMask", "renderSuppressedMask", "scalars", "vectors",
        "artistParameters", "colors",
    }
    if "fidelity" in execution:
        allowed_fields.add("fidelity")
    _require(set(execution) == allowed_fields, "enabled material.execution has missing or hidden fields")
    _require(execution.get("enabled") is True, "material.execution must be enabled")
    _require(execution.get("version") == 1, "material.execution.version is unsupported")
    backend = _require_string(execution.get("backend"), "material.execution.backend")
    _require(
        backend in ("runtimeMaterialV2", "artistVisualV4", "localDecal"),
        "material.execution.backend is unknown or generic: " + backend,
    )
    _require(execution.get("failClosed", False) is False, "enabled execution cannot be fail-closed")
    _require(
        execution.get("authoringApproximate", False) is False,
        "enabled execution cannot be authoring-approximate",
    )
    opcode = _require_int(execution.get("opcode"), "material.execution.opcode")
    _require(opcode <= 65535, "material.execution.opcode exceeds uint16")
    fidelity = execution.get("fidelity", "SOURCE_EXACT")
    _require(
        fidelity in ("SOURCE_EXACT", "PROJECT_TUNED_APPROX"),
        "material.execution.fidelity is unsupported",
    )
    project_tuned_opcode = backend == "runtimeMaterialV2" and opcode in (1001, 1002)
    _require(
        project_tuned_opcode == (fidelity == "PROJECT_TUNED_APPROX"),
        "material.execution fidelity/opcode contract changed",
    )
    pass_index = _require_int(execution.get("passIndex"), "material.execution.passIndex")
    _require(pass_index <= 63, "material.execution.passIndex exceeds bounded pass range")

    lane_count = _require_int(execution.get("textureLaneCount"), "textureLaneCount")
    _require(lane_count <= MAX_TEXTURE_LANES, "typed texture lane capacity exceeded")
    lanes = execution.get("textureLanes")
    _require(isinstance(lanes, list) and len(lanes) == lane_count, "textureLaneCount differs from textureLanes")
    lane_ids: set[str] = set()
    lane_roles: set[str] = set()
    texture_registers: set[int] = set()
    sampler_registers: set[int] = set()
    layout_lanes: list[dict[str, Any]] = []
    descriptor_lanes: list[dict[str, Any]] = []
    expected_mask = 0
    for lane in lanes:
        _require(isinstance(lane, dict), "texture lane must be an object")
        _require(set(lane) == {"laneId", "role", "assetId", "textureRegister", "samplerRegister", "sourceChannel", "colorSpace", "sampler"}, "texture lane has missing or hidden fields")
        lane_id = _require_stable_id(lane.get("laneId"), "texture lane laneId")
        _require(lane_id not in lane_ids, "duplicate texture laneId: " + lane_id)
        lane_ids.add(lane_id)
        texture_register = _require_int(lane.get("textureRegister"), "textureRegister")
        sampler_register = _require_int(lane.get("samplerRegister"), "samplerRegister")
        _require(texture_register == len(layout_lanes), "textureRegister topology must be contiguous from t0")
        _require(sampler_register == 5 + len(layout_lanes), "samplerRegister topology must be contiguous from s5")
        _require(texture_register not in texture_registers, "duplicate textureRegister")
        _require(sampler_register not in sampler_registers, "duplicate samplerRegister")
        texture_registers.add(texture_register)
        sampler_registers.add(sampler_register)
        expected_mask |= 1 << texture_register
        sampler = lane.get("sampler")
        _require(
            isinstance(sampler, dict)
            and set(sampler) == {"filter", "addressU", "addressV", "addressW", "mipLodBias", "maxAnisotropy", "comparison", "borderColor", "minLod", "maxLod"},
            "typed texture lane sampler has missing or hidden fields",
        )
        asset_id = _require_string(lane.get("assetId"), "typed texture lane assetId")
        safe_asset_id = _safe_relative_path(asset_id, "typed texture lane assetId")
        _require(
            safe_asset_id == asset_id and safe_asset_id.startswith("Effect/") and safe_asset_id.lower().endswith(".dds"),
            "typed texture lane assetId is not a safe Effect DDS path",
        )
        role = _require_stable_id(lane.get("role"), "typed texture lane role")
        _require(role not in lane_roles, "duplicate texture lane role: " + role)
        lane_roles.add(role)
        _require(sampler.get("filter") in ("point", "linear", "anisotropic"), "typed sampler filter is unsupported")
        for address_axis in ("addressU", "addressV", "addressW"):
            _require(sampler.get(address_axis) in ("wrap", "clamp", "mirror", "border"), "typed sampler address mode is unsupported")
        _require(sampler.get("comparison") in ("never", "less", "equal", "lessEqual", "greater", "notEqual", "greaterEqual", "always"), "typed sampler comparison is unsupported")
        for numeric_key in ("mipLodBias", "maxAnisotropy", "minLod", "maxLod"):
            numeric_value = sampler.get(numeric_key)
            _require(
                isinstance(numeric_value, (int, float))
                and not isinstance(numeric_value, bool)
                and math.isfinite(float(numeric_value)),
                "typed sampler " + numeric_key + " must be finite numeric",
            )
        _require(abs(float(sampler["mipLodBias"])) <= 16.0, "typed sampler mipLodBias exceeds codec bound")
        _require(
            isinstance(sampler["maxAnisotropy"], int)
            and not isinstance(sampler["maxAnisotropy"], bool)
            and 1 <= sampler["maxAnisotropy"] <= 16,
            "typed sampler maxAnisotropy is outside 1..16",
        )
        _require(float(sampler["minLod"]) <= float(sampler["maxLod"]), "typed sampler minLod exceeds maxLod")
        source_channel = lane.get("sourceChannel")
        _require(
            isinstance(source_channel, str)
            and len(source_channel) <= 4
            and all(character in "RGBA" for character in source_channel),
            "typed sourceChannel is invalid",
        )
        _require(lane.get("colorSpace") in ("linear", "srgb"), "typed colorSpace is unsupported")
        if backend == "localDecal":
            _require(bool(source_channel), "localDecal texture lane requires sourceChannel")
        border = sampler.get("borderColor")
        _require(
            isinstance(border, list)
            and len(border) == 4
            and all(isinstance(item, (int, float)) and not isinstance(item, bool) and math.isfinite(float(item)) for item in border),
            "typed sampler borderColor must be finite float4",
        )
        layout_lanes.append(
            {
                "laneId": lane_id,
                "role": role,
                "textureRegister": texture_register,
                "samplerRegister": sampler_register,
            }
        )
        descriptor_lanes.append(
            {
                "laneId": lane_id,
                "assetId": asset_id,
                "sourceChannel": source_channel,
                "colorSpace": lane["colorSpace"],
                "sampler": sampler,
            }
        )
    _require(execution.get("textureMask") == expected_mask, "material.execution.textureMask is inconsistent")
    if backend == "localDecal":
        _require(lane_count == 6, "localDecal requires six texture lanes")

    scalar_count = _require_int(execution.get("scalarCount"), "scalarCount")
    vector_count = _require_int(execution.get("vectorCount"), "vectorCount")
    _require(scalar_count <= MAX_PACKED_SCALARS, "typed scalar capacity exceeded")
    _require(vector_count <= MAX_PACKED_VECTORS, "typed vector capacity exceeded")
    scalars = _validate_indexed_values(execution.get("scalars"), scalar_count, "scalars", False)
    vectors = _validate_indexed_values(execution.get("vectors"), vector_count, "vectors", True)

    def validate_mask_pair(consumed_key: str, suppressed_key: str, count: int) -> tuple[int, int]:
        consumed = _require_int(execution.get(consumed_key), consumed_key)
        suppressed = _require_int(execution.get(suppressed_key), suppressed_key)
        limit = (1 << count) - 1 if count else 0
        _require((consumed & suppressed) == 0, consumed_key + " overlaps " + suppressed_key)
        _require(((consumed | suppressed) & ~limit) == 0, consumed_key + "/" + suppressed_key + " exceed declared count")
        return consumed, suppressed

    dynamic_consumed, dynamic_suppressed = validate_mask_pair("dynamicConsumedMask", "dynamicSuppressedMask", 4)
    particle_consumed, particle_suppressed = validate_mask_pair("particleColorConsumedMask", "particleColorSuppressedMask", 4)
    input_count = _require_int(execution.get("inputCount"), "inputCount")
    _require(input_count <= 64, "inputCount exceeds uint32[2] capacity")
    input_consumed = execution.get("inputConsumedMask")
    input_suppressed = execution.get("inputSuppressedMask")
    _require(isinstance(input_consumed, list) and len(input_consumed) == 2, "inputConsumedMask must be uint32[2]")
    _require(isinstance(input_suppressed, list) and len(input_suppressed) == 2, "inputSuppressedMask must be uint32[2]")
    for word_index in range(2):
        consumed_word = _require_int(input_consumed[word_index], "inputConsumedMask word")
        suppressed_word = _require_int(input_suppressed[word_index], "inputSuppressedMask word")
        _require(consumed_word <= 0xFFFFFFFF and suppressed_word <= 0xFFFFFFFF, "input masks exceed uint32")
        _require((consumed_word & suppressed_word) == 0, "input consumed/suppressed masks overlap")
        valid_bits = max(0, min(32, input_count - word_index * 32))
        limit = (1 << valid_bits) - 1 if valid_bits else 0
        _require(((consumed_word | suppressed_word) & ~limit) == 0, "input masks exceed inputCount")
    vector_consumed = execution.get("vectorComponentConsumedMask")
    vector_suppressed = execution.get("vectorComponentSuppressedMask")
    _require(isinstance(vector_consumed, list) and len(vector_consumed) == MAX_PACKED_VECTORS, "vectorComponentConsumedMask must have three rows")
    _require(isinstance(vector_suppressed, list) and len(vector_suppressed) == MAX_PACKED_VECTORS, "vectorComponentSuppressedMask must have three rows")
    for vector_index in range(MAX_PACKED_VECTORS):
        consumed = _require_int(vector_consumed[vector_index], "vector component consumed mask")
        suppressed = _require_int(vector_suppressed[vector_index], "vector component suppressed mask")
        _require(consumed <= 0xF and suppressed <= 0xF and (consumed & suppressed) == 0, "vector component masks are invalid")
        _require(vector_index < vector_count or (consumed | suppressed) == 0, "vector component mask exceeds vectorCount")
    static_count = _require_int(execution.get("staticInputCount"), "staticInputCount")
    _require(static_count <= 32, "staticInputCount exceeds uint32 capacity")
    static_consumed, static_suppressed = validate_mask_pair("staticConsumedMask", "staticSuppressedMask", static_count)
    static_selected = _require_int(execution.get("staticSelectedMask"), "staticSelectedMask")
    static_limit = (1 << static_count) - 1 if static_count else 0
    _require((static_selected & ~static_limit) == 0, "staticSelectedMask exceeds staticInputCount")
    _require((static_selected & ~(static_consumed | static_suppressed)) == 0, "staticSelectedMask is not a classified static input subset")
    render_count = _require_int(execution.get("renderInputCount"), "renderInputCount")
    _require(render_count <= 32, "renderInputCount exceeds uint32 capacity")
    validate_mask_pair("renderConsumedMask", "renderSuppressedMask", render_count)

    artist_parameters = execution.get("artistParameters")
    colors = execution.get("colors")
    _require(isinstance(artist_parameters, list) and isinstance(colors, list), "typed artist/color arrays are malformed")
    if backend == "artistVisualV4":
        _require(len(artist_parameters) <= 8 and len(colors) <= 2, "artistVisualV4 artist/color capacity exceeded")
    else:
        _require(not artist_parameters and not colors, "artist/color values are only valid for artistVisualV4")
    artist_parameters = _validate_bounded_vector_values(artist_parameters, 8, "artistParameters")
    colors = _validate_bounded_vector_values(colors, 2, "colors")

    particle_color_policy = _require_int(execution.get("particleColorPolicy"), "particleColorPolicy")
    _require(particle_color_policy <= 3, "particleColorPolicy exceeds codec bound")
    mask_fields: dict[str, Any] = {
        "dynamicConsumedMask": dynamic_consumed,
        "dynamicSuppressedMask": dynamic_suppressed,
        "particleColorPolicy": particle_color_policy,
        "particleColorConsumedMask": particle_consumed,
        "particleColorSuppressedMask": particle_suppressed,
        "inputCount": input_count,
        "inputConsumedMask": input_consumed,
        "inputSuppressedMask": input_suppressed,
        "vectorComponentConsumedMask": vector_consumed,
        "vectorComponentSuppressedMask": vector_suppressed,
        "staticInputCount": static_count,
        "staticSelectedMask": static_selected,
        "staticConsumedMask": static_consumed,
        "staticSuppressedMask": static_suppressed,
        "renderInputCount": render_count,
        "renderConsumedMask": execution["renderConsumedMask"],
        "renderSuppressedMask": execution["renderSuppressedMask"],
    }

    program = {
        "kind": "TYPED_RUNTIME_PROGRAM",
        "version": 1,
        "backend": backend,
        "opcode": opcode,
    }
    layout = {
        "kind": "TYPED_RUNTIME_PACKET",
        "version": 1,
        "backend": backend,
        "textureLaneCount": lane_count,
        "textureMask": expected_mask,
        "textureLaneTopology": sorted(layout_lanes, key=lambda item: item["textureRegister"]),
        "scalarCount": scalar_count,
        "scalarPackedIndices": [item["packedIndex"] for item in scalars],
        "vectorCount": vector_count,
        "vectorPackedIndices": [item["packedIndex"] for item in vectors],
        "maskTopology": dict(sorted(mask_fields.items())),
    }
    descriptor = {
        "kind": "TYPED_RUNTIME_VALUES",
        "textureLanes": sorted(descriptor_lanes, key=lambda item: item["laneId"]),
        "scalars": scalars,
        "vectors": vectors,
        "artistParameters": artist_parameters,
        "colors": colors,
    }
    adapter = {
        "backend": backend,
        "opcode": opcode,
        "passIndex": pass_index,
        "renderState": execution.get("renderState"),
    }
    _require(isinstance(adapter["renderState"], dict), "typed execution has no renderState")
    _require(
        set(adapter["renderState"]) == {"rasterizer", "depthStencil", "blend", "stencilReference"}
        and all(STABLE_ID_RE.fullmatch(adapter["renderState"].get(key, "")) is not None for key in ("rasterizer", "depthStencil", "blend"))
        and isinstance(adapter["renderState"].get("stencilReference"), int)
        and not isinstance(adapter["renderState"].get("stencilReference"), bool)
        and 0 <= adapter["renderState"]["stencilReference"] <= 255,
        "typed renderState is malformed",
    )
    return program, layout, descriptor, adapter


def _named_abi_capacity(row: dict[str, Any]) -> tuple[bool, dict[str, int]]:
    texture_count = len(row.get("textureSlots", [])) if isinstance(row.get("textureSlots"), list) else -1
    scalar_count = len(row.get("scalarLanes", [])) if isinstance(row.get("scalarLanes"), list) else -1
    vector_count = len(row.get("vectorLanes", [])) if isinstance(row.get("vectorLanes"), list) else -1
    _require(min(texture_count, scalar_count, vector_count) >= 0, "resolved named ABI arrays are malformed")
    counts = {
        "textureSlotCount": texture_count,
        "scalarLaneCount": scalar_count,
        "vectorLaneCount": vector_count,
    }
    return (
        texture_count <= MAX_TEXTURE_LANES
        and scalar_count <= MAX_PACKED_SCALARS
        and vector_count <= MAX_PACKED_VECTORS,
        counts,
    )


def _load_shader_evidence(tracker: InputTracker) -> dict[str, Any]:
    shader_map_document = _read_artifact(
        tracker, SHADER_MAP_PATH, "lostark.effect-family-shader-map-index", "SHADER_MAP_INDEX"
    )
    cooked_document = _read_artifact(
        tracker, COOKED_PATH, "lostark.effect-family-cooked-pixel-shaders", "COOKED_PIXEL_SHADERS"
    )
    named_document = _read_artifact(
        tracker, NAMED_ABI_PATH, "lostark.effect-family-named-abi", "NAMED_ABI"
    )
    child_document = _read_artifact(
        tracker, CHILD_PARENT_PATH, "lostark.effect-child-parent-resolution", "CHILD_PARENT_RESOLUTION"
    )
    g00_document = _read_artifact(
        tracker, G00_PATH, "lostark.effect-family-shader-inventory", "G00_CANARY"
    )
    restoration_document = _read_artifact(
        tracker, RESTORATION_PATH, "lostark.character-effect-restoration-targets", "USER_REVIEW_RECEIPT"
    )
    exact_variants_document = tracker.read_json(
        EXACT_VARIANTS_PATH, "EXACT_COOKED_VARIANT_EVIDENCE", require_lf=True
    )
    _require(
        isinstance(exact_variants_document, dict)
        and exact_variants_document.get("schema") == "lostark.effect-ue3-exact-cooked-shader-variants"
        and exact_variants_document.get("formatVersion") == 1,
        "exact cooked variant contract identity is unsupported",
    )
    contract_sha = _require_sha256(
        exact_variants_document.get("contractSha256"), "exact variant contractSha256"
    )
    exact_unsigned = dict(exact_variants_document)
    exact_unsigned.pop("contractSha256", None)
    _require(canonical_sha256(exact_unsigned) == contract_sha, "exact variant contractSha256 drifted")
    translations_document = tracker.read_json(
        TRANSLATIONS_PATH, "HLSL_TRANSLATION_INDEX"
    )

    exact_generator = exact_variants_document.get("generator")
    _require(isinstance(exact_generator, dict), "exact variant generator identity is absent")
    exact_generator_path = Path(
        _safe_relative_path(exact_generator.get("repositoryRelativePath"), "exact variant generator path")
    )
    exact_generator_payload = tracker.read_bytes(exact_generator_path, "EXACT_VARIANT_GENERATOR")
    _require(
        _sha256_bytes(exact_generator_payload) == _require_sha256(exact_generator.get("sha256"), "exact variant generator sha256"),
        "exact variant generator hash drifted",
    )
    exact_inputs = exact_variants_document.get("inputs")
    _require(isinstance(exact_inputs, dict), "exact variant input pins are absent")
    for input_name, input_pin in exact_inputs.items():
        if not isinstance(input_pin, dict) or "repositoryRelativePath" not in input_pin:
            continue
        input_path = Path(
            _safe_relative_path(input_pin.get("repositoryRelativePath"), "exact variant input path")
        )
        input_document = tracker.read_json(input_path, "EXACT_VARIANT_PINNED_INPUT")
        input_payload = (tracker.root / input_path).read_bytes()
        _require(
            _sha256_bytes(input_payload) == _require_sha256(input_pin.get("fileRawSha256"), "exact variant input raw sha256"),
            "exact variant input raw hash drifted: " + input_name,
        )
        _require(
            isinstance(input_document, dict)
            and input_document.get("schema") == input_pin.get("schema")
            and input_document.get("formatVersion") == input_pin.get("formatVersion"),
            "exact variant input identity drifted: " + input_name,
        )
        pinned_receipt = _require_sha256(input_pin.get("receiptSha256"), "exact variant input receiptSha256")
        receipt_unsigned = dict(input_document)
        receipt_unsigned.pop("receiptSha256", None)
        _require(
            input_document.get("receiptSha256") == pinned_receipt
            and canonical_sha256(receipt_unsigned) == pinned_receipt,
            "exact variant input receipt hash drifted: " + input_name,
        )

    shader_map = _index_unique(shader_map_document.get("families"), "parentMaterialPath", "shaderMap.families")
    cooked = _index_unique(cooked_document.get("families"), "parentMaterialPath", "cooked.families")
    named = _index_unique(named_document.get("families"), "parentMaterialPath", "namedAbi.families")
    child = _index_unique(child_document.get("children"), "childMaterialPath", "childParent.children")
    child_families = _index_unique(
        child_document.get("families"), "canonicalParentMaterialPath", "childParent.families"
    )
    g00 = _index_unique(g00_document.get("families"), "parentMaterialPath", "g00.families")

    _require(
        shader_map_document.get("summary", {}).get("parentMaterialCount") == len(shader_map),
        "shader-map summary denominator drifted",
    )
    cooked_expected = {
        parent
        for parent, row in shader_map.items()
        if row.get("cookedEvidence") == "COOKED_MATERIAL_MAPS_PRESENT"
    }
    _require(set(cooked) == cooked_expected, "cooked family denominator differs from shader-map evidence")

    _require_pin(
        cooked_document,
        "shaderMapArtifactSha256",
        "shaderMapRawSha256",
        shader_map_document,
        tracker.identity(SHADER_MAP_PATH),
        "cooked pixel shaders",
    )
    _require_pin(
        named_document,
        "shaderMapArtifactSha256",
        "shaderMapRawSha256",
        shader_map_document,
        tracker.identity(SHADER_MAP_PATH),
        "named ABI shader-map",
    )
    _require_pin(
        named_document,
        "cookedPixelShadersArtifactSha256",
        "cookedPixelShadersRawSha256",
        cooked_document,
        tracker.identity(COOKED_PATH),
        "named ABI cooked",
    )
    _require_pin(
        child_document,
        "cookedPixelShadersArtifactSha256",
        "cookedPixelShadersRawSha256",
        cooked_document,
        tracker.identity(COOKED_PATH),
        "child-parent cooked",
    )
    _require_pin(
        child_document,
        "namedAbiArtifactSha256",
        "namedAbiRawSha256",
        named_document,
        tracker.identity(NAMED_ABI_PATH),
        "child-parent named ABI",
    )
    g00_inputs = g00_document.get("inputs")
    _require(isinstance(g00_inputs, dict), "G00 has no inputs object")
    for artifact, identity, artifact_field, raw_field, label in (
        (shader_map_document, tracker.identity(SHADER_MAP_PATH), "shaderMapIndexArtifactSha256", "shaderMapIndexRawSha256", "shader-map"),
        (cooked_document, tracker.identity(COOKED_PATH), "cookedPixelShadersArtifactSha256", "cookedPixelShadersRawSha256", "cooked"),
        (named_document, tracker.identity(NAMED_ABI_PATH), "namedAbiArtifactSha256", "namedAbiRawSha256", "named ABI"),
        (child_document, tracker.identity(CHILD_PARENT_PATH), "childParentResolutionArtifactSha256", "childParentResolutionRawSha256", "child-parent"),
    ):
        _require(g00_inputs.get(artifact_field) == artifact.get("artifactSha256"), "G00 " + label + " artifact pin drifted")
        _require(g00_inputs.get(raw_field) == identity["rawSha256"], "G00 " + label + " raw pin drifted")
    translation_identity = tracker.identity(TRANSLATIONS_PATH)
    _require(
        g00_inputs.get("hlslTranslations") == TRANSLATIONS_PATH.as_posix()
        and g00_inputs.get("hlslTranslationsRawSha256") == translation_identity["rawSha256"]
        and g00_inputs.get("hlslTranslationsByteSize") == translation_identity["byteSize"]
        and g00_inputs.get("hlslTranslationProgramCount") == len(translations_document),
        "G00 HLSL translation input pin drifted",
    )

    _require(isinstance(translations_document, list), "HLSL translation index must be an array")
    translations: dict[str, dict[str, Any]] = {}
    function_names: set[str] = set()
    for row in translations_document:
        _require(isinstance(row, dict) and row.get("status") == "TRANSLATED", "translation row is not TRANSLATED")
        digest = _require_sha256(row.get("dxbcSha256"), "translation.dxbcSha256")
        _require(row.get("dxbc") == digest + ".dxbc", "translation DXBC filename drifted")
        function_name = _require_string(row.get("functionName"), "translation.functionName")
        _require(function_name.casefold() not in function_names, "duplicate translation functionName")
        function_names.add(function_name.casefold())
        _require(digest not in translations, "duplicate translated DXBC digest")
        translations[digest] = row

    extracted_digests: dict[str, int] = {}
    for parent, row in cooked.items():
        status = row.get("status")
        _require(status in ("EXTRACTED", "BLOCKED"), "unknown cooked status: " + parent)
        if status == "BLOCKED":
            _require(isinstance(row.get("blocker"), str) and bool(row["blocker"]), "blocked cooked row has no blocker")
            continue
        digest = _require_sha256(row.get("dxbcSha256"), "cooked.dxbcSha256")
        byte_size = _require_int(row.get("dxbcByteSize"), "cooked.dxbcByteSize", 1)
        previous_size = extracted_digests.setdefault(digest, byte_size)
        _require(previous_size == byte_size, "shared DXBC has conflicting byte sizes")
        selection = row.get("permutationSelection")
        _require(selection in ("SINGLE_PERMUTATION_FAMILY", "CHILD_MIC_ENGINE_EQUALITY"), "unknown cooked permutationSelection")
        if selection == "SINGLE_PERMUTATION_FAMILY":
            _require(row.get("permutationCount") == 1, "single-permutation family has count != 1")
        else:
            _require_string(row.get("childMaterialPath"), "cooked.childMaterialPath")
    _require(set(translations) == set(extracted_digests), "translation denominator differs from extracted DXBC set")

    expected_hlsli: set[str] = set()
    for digest, translation in translations.items():
        dxbc_path = COOKED_SHADER_DIRECTORY / (digest + ".dxbc")
        dxbc_payload = tracker.read_bytes(dxbc_path, "COOKED_DXBC")
        _require(dxbc_payload.startswith(b"DXBC"), "cooked shader has no DXBC signature: " + dxbc_path.as_posix())
        _require(len(dxbc_payload) == extracted_digests[digest], "cooked DXBC byte size drifted")
        _require(_sha256_bytes(dxbc_payload) == digest, "cooked DXBC digest drifted")
        function_name = translation["functionName"]
        hlsli_name = function_name + ".hlsli"
        expected_hlsli.add(hlsli_name)
        hlsli_path = TRANSLATED_SHADER_DIRECTORY / hlsli_name
        hlsli_payload = tracker.read_bytes(hlsli_path, "LITERAL_HLSL_TRANSLATION", require_lf=True)
        _require(
            _sha256_bytes(hlsli_payload) == _require_sha256(translation.get("hlslSha256"), "translation.hlslSha256"),
            "translated HLSLI digest drifted: " + hlsli_name,
        )
        try:
            hlsli_source = hlsli_payload.decode("utf-8")
        except UnicodeError as error:
            raise InventoryError("translated HLSLI is not UTF-8: " + hlsli_name) from error
        _require(
            re.search(r"\b" + re.escape(function_name) + r"\s*\(", hlsli_source) is not None,
            "translated HLSLI does not declare its function: " + hlsli_name,
        )
    actual_hlsli = {
        path.name
        for path in (tracker.root / TRANSLATED_SHADER_DIRECTORY).glob("*.hlsli")
        if path.is_file()
    }
    _require(actual_hlsli == expected_hlsli, "translated HLSLI file set differs from translation index")

    exact_variants = exact_variants_document.get("variants")
    _require(isinstance(exact_variants, list) and len(exact_variants) == 5, "exact variant denominator must contain five rows")
    exact_variant_blob_paths: set[str] = set()
    exact_variant_blob_sizes: dict[str, int] = {}
    exact_variants_by_source: dict[str, dict[str, Any]] = {}
    exact_variants_by_id: dict[str, dict[str, Any]] = {}

    def collect_dxbc_paths(value: Any) -> None:
        if isinstance(value, dict):
            repository_path = value.get("repositoryRelativePath")
            if isinstance(repository_path, str) and repository_path.endswith(".dxbc"):
                safe_path = _safe_relative_path(repository_path, "exact variant DXBC path")
                exact_variant_blob_paths.add(safe_path)
                _require(value.get("sha256") == Path(safe_path).stem, "exact variant DXBC path/hash identity differs")
                byte_count = _require_int(value.get("byteCount"), "exact variant DXBC byteCount", 1)
                prior_size = exact_variant_blob_sizes.setdefault(safe_path, byte_count)
                _require(prior_size == byte_count, "exact variant DXBC has conflicting byte counts")
            for child_value in value.values():
                collect_dxbc_paths(child_value)
        elif isinstance(value, list):
            for child_value in value:
                collect_dxbc_paths(child_value)

    collect_dxbc_paths(exact_variants)
    for variant in exact_variants:
        _require(isinstance(variant, dict), "exact shader variant must be an object")
        variant_key_sha = _require_sha256(variant.get("variantKeySha256"), "exact variantKeySha256")
        variant_id = _require_string(variant.get("variantId"), "exact variantId")
        _require(
            canonical_sha256(variant.get("variantKey")) == variant_key_sha
            and variant_id == "ue3.exact-cooked-ps." + variant_key_sha[:24],
            "exact shader variant key identity drifted",
        )
        variant_key = variant.get("variantKey")
        _require(isinstance(variant_key, dict), "exact shader variantKey must be an object")
        renderer_type = variant_key.get("rendererType")
        _require(renderer_type in ("SpriteParticle", "MeshParticle"), "exact variant renderer type is unsupported")
        structural_pass = variant.get("structuralVertexFactoryPass")
        _require(
            variant.get("familyId") == variant_key.get("familyId")
            and isinstance(structural_pass, dict)
            and structural_pass.get("rendererType") == renderer_type,
            "exact variant family or structural renderer identity drifted",
        )
        source_material_path = _require_string(
            variant.get("sourceMaterialPath"), "exact variant sourceMaterialPath"
        )
        _require(source_material_path not in exact_variants_by_source, "duplicate exact variant sourceMaterialPath")
        _require(variant_id not in exact_variants_by_id, "duplicate exact variantId")
        pixel_shader = variant.get("pixelShader")
        _require(isinstance(pixel_shader, dict), "exact variant has no pixelShader")
        _require(
            pixel_shader.get("exactDxbcContainer") is True
            and pixel_shader.get("shaderType") == variant_key.get("passShaderType")
            and pixel_shader.get("shaderIdHex") == variant_key.get("pixelShaderIdHex"),
            "exact variant pixel shader identity differs from its variant key",
        )
        _require_sha256(pixel_shader.get("sha256"), "exact variant pixelShader.sha256")
        _require_string(pixel_shader.get("profile"), "exact variant pixelShader.profile")
        native_binding = variant.get("nativeBinding")
        _require(isinstance(native_binding, dict), "exact shader variant has no nativeBinding")
        shader_object = native_binding.get("shaderObject")
        _require(
            native_binding.get("status") == "EXACT_NATIVE_SHADER_OBJECT_BINDING"
            and isinstance(shader_object, dict)
            and shader_object.get("shaderType") == pixel_shader.get("shaderType")
            and shader_object.get("shaderIdHex") == pixel_shader.get("shaderIdHex")
            and native_binding.get("runtimeAdmission") is False
            and native_binding.get("actualVfPassAdmission") is False
            and isinstance(native_binding.get("scalarGroups"), list)
            and isinstance(native_binding.get("vectors"), list)
            and isinstance(native_binding.get("textures"), list)
            and isinstance(native_binding.get("constantBufferClosure"), dict)
            and isinstance(native_binding.get("textureSampleClosure"), dict)
            and isinstance(native_binding.get("dxbcDeclarationClosure"), dict)
            and native_binding.get("wireEntryFormat")
            == "<u32 expressionIndexOrPackedScalarGroup,u16 baseByteOrResourceIndex,u16 numBytesOrResources,u16 bufferOrSamplerIndex>",
            "exact variant native wire is malformed or admitted",
        )
        _require_sha256(native_binding.get("bindingSemanticSha256"), "exact variant bindingSemanticSha256")
        admission = variant.get("admission")
        _require(isinstance(admission, dict), "exact shader variant has no admission object")
        expected_admission = {
            "exactPixelShaderBlob": True,
            "exactNativeBindingWire": True,
            "structuralFixedInputReplay": True,
            "sourceValueUniformCb0Closure": True,
            "sourceExactNativeScalarGroupPacking": False,
            "sourceExactTextureBindings": True,
            "sourceExactColorSpace": True,
            "sourceExactFilterSelector": True,
            "sourceExactSampler": False,
            "sourceValueReplay": False,
            "actualVfPass": False,
            "authoringPreviewCandidate": False,
            "authoringPreviewAdmission": False,
            "productRuntime": False,
            "visual": False,
        }
        _require(admission == expected_admission, "exact shader variant admission boundary drifted")
        exact_variants_by_source[source_material_path] = variant
        exact_variants_by_id[variant_id] = variant
    summary = exact_variants_document.get("summary")
    _require(
        isinstance(summary, dict)
        and summary.get("variantCount") == 5
        and summary.get("actualVfPassAdmissionCount") == 0
        and summary.get("productRuntimeAdmissionCount") == 0
        and summary.get("visualAdmissionCount") == 0,
        "exact shader variant summary overstates admission",
    )
    for relative in sorted(exact_variant_blob_paths):
        payload = tracker.read_bytes(Path(relative), "EXACT_VARIANT_DXBC")
        basename_digest = Path(relative).stem
        _require(SHA256_RE.fullmatch(basename_digest) is not None, "exact variant DXBC filename is not content-addressed")
        _require(
            payload.startswith(b"DXBC")
            and len(payload) == exact_variant_blob_sizes[relative]
            and _sha256_bytes(payload) == basename_digest,
            "exact variant DXBC identity drifted",
        )
    actual_dxbc = {
        path.relative_to(tracker.root).as_posix()
        for path in (tracker.root / COOKED_SHADER_DIRECTORY).glob("*.dxbc")
        if path.is_file()
    }
    expected_dxbc = {
        (COOKED_SHADER_DIRECTORY / (digest + ".dxbc")).as_posix()
        for digest in translations
    } | exact_variant_blob_paths
    _require(actual_dxbc == expected_dxbc, "CookedShaders DXBC set is not owned by family or exact-variant evidence")

    for parent, row in named.items():
        status = row.get("status")
        _require(status in ("RESOLVED_NAMED_MAPPING", "BLOCKED"), "unknown named ABI status: " + parent)
        if status == "RESOLVED_NAMED_MAPPING":
            _require(row.get("admits") == "NAMED_LANE_IDENTITY_ONLY", "named ABI overstates admission")
            digest = _require_sha256(row.get("dxbcSha256"), "named ABI dxbcSha256")
            _require(digest in translations, "named ABI references unknown DXBC")
            _require(
                parent in cooked
                and cooked[parent].get("status") == "EXTRACTED"
                and cooked[parent].get("dxbcSha256") == digest,
                "named ABI is not bound to the cooked family DXBC: " + parent,
            )
            _named_abi_capacity(row)
        else:
            blocker = row.get("blocker")
            _require(
                isinstance(blocker, dict)
                and set(blocker) == {"reasonCode", "candidateCount"}
                and isinstance(blocker.get("reasonCode"), str)
                and bool(blocker["reasonCode"])
                and isinstance(blocker.get("candidateCount"), int)
                and not isinstance(blocker["candidateCount"], bool)
                and blocker["candidateCount"] > 1,
                "blocked named ABI does not preserve its structured ambiguity blocker",
            )

    resolved_parent_counts: Counter[str] = Counter()
    for source_child, row in child.items():
        _require(row.get("status") in ("RESOLVED", "BLOCKED"), "unknown child-parent status")
        if row.get("status") == "RESOLVED":
            parent = _require_string(row.get("canonicalParentMaterialPath"), "child canonical parent")
            resolved_parent_counts[parent] += _require_int(row.get("elementCount"), "child elementCount", 1)
            if row.get("familyAlreadyInDenominator") is True:
                known = _require_string(row.get("knownFamilyPath"), "child knownFamilyPath")
                _require(known in shader_map, "child known family is absent from shader-map")
            else:
                _require(row.get("knownFamilyPath") is None, "new child family unexpectedly has knownFamilyPath")
        else:
            _require(bool(row.get("blocker")), "blocked child row has no blocker: " + source_child)
    _require(set(child_families) == set(resolved_parent_counts), "child-parent family projection drifted")
    for parent, count in resolved_parent_counts.items():
        _require(child_families[parent].get("recoveredElementCount") == count, "child-parent recovered count drifted")

    review_receipts: dict[tuple[str, str], dict[str, Any]] = {}
    restoration_occurrences = restoration_document.get("occurrences")
    _require(isinstance(restoration_occurrences, list), "restoration receipt has no occurrences")
    for row in restoration_occurrences:
        _require(isinstance(row, dict), "restoration occurrence must be an object")
        node = row.get("authoredNode")
        if not isinstance(node, dict) or node.get("kind") != "ELEMENT":
            continue
        asset = node.get("effectAssetId")
        stable_id = node.get("stableId")
        if not isinstance(asset, str) or not isinstance(stable_id, str):
            continue
        key = (asset, stable_id)
        _require(key not in review_receipts, "duplicate restoration review receipt")
        review_receipts[key] = row

    return {
        "shaderMapDocument": shader_map_document,
        "cookedDocument": cooked_document,
        "namedDocument": named_document,
        "childDocument": child_document,
        "g00Document": g00_document,
        "restorationDocument": restoration_document,
        "exactVariantsDocument": exact_variants_document,
        "shaderMap": shader_map,
        "cooked": cooked,
        "named": named,
        "child": child,
        "g00": g00,
        "translations": translations,
        "exactVariantsBySource": exact_variants_by_source,
        "exactVariantsById": exact_variants_by_id,
        "reviewReceipts": review_receipts,
    }


def _resolve_parent(
    authored_parent: str,
    source_material_path: str,
    child_rows: dict[str, dict[str, Any]],
) -> tuple[str, str, str | None, Any]:
    child = child_rows.get(source_material_path)
    if child is None:
        return (
            (authored_parent, "AUTHORED_PARENT_EXACT", None, None)
            if authored_parent
            else ("", "NO_PARENT_EVIDENCE", None, None)
        )
    if child.get("status") == "BLOCKED":
        blocker = child.get("blocker")
        _require(isinstance(blocker, (str, dict)) and bool(blocker), "blocked child-parent row has no blocker")
        return authored_parent, "CHILD_PARENT_BLOCKED", child.get("rowSha256"), blocker
    _require(child.get("status") == "RESOLVED", "child-parent row has an unknown status")
    if child.get("familyAlreadyInDenominator") is True:
        return child["knownFamilyPath"], "CHILD_PARENT_KNOWN_FAMILY_EXACT", child.get("rowSha256"), None
    return child["canonicalParentMaterialPath"], "CHILD_PARENT_NEW_FAMILY_EXACT", child.get("rowSha256"), None


def _source_program_axis(
    profile: dict[str, Any],
    material: dict[str, Any],
    source_carrier: str,
    fine_renderer_kind: str,
    evidence: dict[str, Any],
) -> tuple[str, dict[str, Any] | None, dict[str, Any], list[str], dict[str, Any] | None]:
    profile_enabled = profile.get("enabled") is True
    authored_parent = profile.get("parentMaterialPath") if isinstance(profile.get("parentMaterialPath"), str) else ""
    source_material = material.get("sourceMaterialPath") if isinstance(material.get("sourceMaterialPath"), str) else ""
    parent, resolution, child_receipt, child_blocker = _resolve_parent(
        authored_parent, source_material, evidence["child"]
    )
    blockers: list[str] = []
    if child_blocker is not None:
        blockers.append("CHILD_PARENT_RESOLUTION_BLOCKED")
    shader_map_row = evidence["shaderMap"].get(parent)
    cooked_row = evidence["cooked"].get(parent)
    program_candidate: dict[str, Any] | None = None
    translation: dict[str, Any] | None = None
    status = "BOUNDED_SOURCE_PROFILE_ONLY" if profile_enabled else "NO_PROGRAM_EVIDENCE"

    if not parent:
        blockers.append("PARENT_UNRESOLVED")
        if not profile_enabled:
            blockers.append("PROGRAM_EQUATION_EVIDENCE_ABSENT")
    elif shader_map_row is None:
        blockers.append("SHADERMAP_ABSENT")
        if not profile_enabled:
            blockers.append("PROGRAM_EQUATION_EVIDENCE_ABSENT")
    elif cooked_row is None or cooked_row.get("status") != "EXTRACTED":
        blockers.append("DXBC_EXTRACTION_BLOCKED" if cooked_row else "PROGRAM_EVIDENCE_ABSENT")
        if not profile_enabled:
            blockers.append("PROGRAM_EQUATION_EVIDENCE_ABSENT")
    else:
        digest = cooked_row.get("dxbcSha256")
        translation = evidence["translations"].get(digest)
        if translation is None:
            blockers.append("LITERAL_TRANSLATION_MISSING")
        selection = cooked_row.get("permutationSelection")
        permutation_match = selection == "SINGLE_PERMUTATION_FAMILY" or (
            selection == "CHILD_MIC_ENGINE_EQUALITY"
            and bool(source_material)
            and source_material == cooked_row.get("childMaterialPath")
        )
        carrier_match = bool(source_carrier) and source_carrier == cooked_row.get("carrier")
        if permutation_match and carrier_match and translation is not None:
            status = "DXBC_OCCURRENCE_EXACT"
            program_candidate = {
                "kind": "DXBC_LITERAL_TRANSLATION",
                "dxbcSha256": digest,
                "hlslSha256": translation["hlslSha256"],
                "functionName": translation["functionName"],
            }
        else:
            status = "DXBC_FAMILY_REPRESENTATIVE_ONLY"
            if not permutation_match:
                blockers.append("OCCURRENCE_STATIC_PERMUTATION_NOT_EXTRACTED")
            if not carrier_match:
                blockers.append("COOKED_CARRIER_MISMATCH")

    if child_blocker is not None and status == "DXBC_OCCURRENCE_EXACT":
        status = "DXBC_FAMILY_REPRESENTATIVE_ONLY"
        program_candidate = None
        blockers.append("CHILD_PARENT_BLOCKED_EXACT_PROMOTION_FORBIDDEN")

    program_evidence: dict[str, Any] = {
        "kind": "SOURCE_FAMILY_PROGRAM_EVIDENCE",
        "authoredParentMaterialPath": authored_parent or None,
        "sourceMaterialPath": source_material or None,
        "effectiveParentMaterialPath": parent or None,
        "parentResolution": resolution,
        "childParentRowSha256": child_receipt,
        "childParentBlocker": child_blocker,
        "shaderMapFamilyId": shader_map_row.get("familyId") if shader_map_row else None,
        "cookedStatus": cooked_row.get("status") if cooked_row else "ABSENT",
        "cookedDxbcSha256": cooked_row.get("dxbcSha256") if cooked_row and cooked_row.get("status") == "EXTRACTED" else None,
        "permutationSelection": cooked_row.get("permutationSelection") if cooked_row else None,
        "selectedChildMaterialPath": cooked_row.get("childMaterialPath") if cooked_row else None,
        "sourceCarrier": source_carrier or None,
        "cookedCarrier": cooked_row.get("carrier") if cooked_row else None,
        "occurrenceExact": status == "DXBC_OCCURRENCE_EXACT",
    }

    exact_variant = evidence["exactVariantsBySource"].get(source_material)
    if isinstance(exact_variant, dict):
        expected_fine_renderer_kind = {
            "SpriteParticle": "SPRITE_PARTICLE",
            "MeshParticle": "MESH_PARTICLE",
        }[exact_variant["variantKey"]["rendererType"]]
        renderer_match = fine_renderer_kind == expected_fine_renderer_kind
        if renderer_match and child_blocker is None:
            pixel_shader = exact_variant["pixelShader"]
            digest = pixel_shader["sha256"]
            exact_translation = evidence["translations"].get(digest)
            translation = exact_translation
            if exact_translation is None:
                status = "DXBC_OCCURRENCE_EXACT_UNTRANSLATED"
                program_candidate = None
                blockers.append("LITERAL_TRANSLATION_MISSING")
            else:
                status = "DXBC_OCCURRENCE_EXACT"
                program_candidate = {
                    "kind": "DXBC_LITERAL_TRANSLATION",
                    "dxbcSha256": digest,
                    "hlslSha256": exact_translation["hlslSha256"],
                    "functionName": exact_translation["functionName"],
                }
            program_evidence = {
                "kind": "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE",
                "variantId": exact_variant["variantId"],
                "variantKeySha256": exact_variant["variantKeySha256"],
                "sourceMaterialPath": source_material,
                "effectiveParentMaterialPath": parent or None,
                "parentResolution": resolution,
                "childParentRowSha256": child_receipt,
                "childParentBlocker": child_blocker,
                "expectedFineRendererKind": expected_fine_renderer_kind,
                "occurrenceFineRendererKind": fine_renderer_kind,
                "rendererMatch": True,
                "dxbcSha256": digest,
                "shaderProfile": pixel_shader["profile"],
                "literalTranslationAvailable": exact_translation is not None,
                "exactPixelShaderBlob": True,
                "exactNativeBindingWire": True,
                "actualVfPass": False,
                "runtimeAdmission": False,
                "visualAdmission": False,
                "occurrenceExact": True,
            }
            blockers.extend(
                (
                    "EXACT_VARIANT_ACTUAL_VF_PASS_UNPROVEN",
                    "EXACT_VARIANT_RUNTIME_ADMISSION_UNPROVEN",
                    "EXACT_VARIANT_VISUAL_ADMISSION_UNPROVEN",
                )
            )
        else:
            blockers.append(
                "EXACT_VARIANT_RENDERER_KIND_MISMATCH"
                if not renderer_match
                else "CHILD_PARENT_BLOCKED_EXACT_PROMOTION_FORBIDDEN"
            )
            # A renderer-mismatched variant can strengthen missing/bounded family
            # evidence to representative evidence, but it must never downgrade a
            # separately proven family-exact Program.
            if status != "DXBC_OCCURRENCE_EXACT":
                status = "DXBC_FAMILY_REPRESENTATIVE_ONLY"
                program_candidate = None
                program_evidence = {
                    "kind": "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE",
                    "variantId": exact_variant["variantId"],
                    "variantKeySha256": exact_variant["variantKeySha256"],
                    "sourceMaterialPath": source_material,
                    "effectiveParentMaterialPath": parent or None,
                    "parentResolution": resolution,
                    "childParentRowSha256": child_receipt,
                    "childParentBlocker": child_blocker,
                    "expectedFineRendererKind": expected_fine_renderer_kind,
                    "occurrenceFineRendererKind": fine_renderer_kind,
                    "rendererMatch": renderer_match,
                    "dxbcSha256": exact_variant["pixelShader"]["sha256"],
                    "shaderProfile": exact_variant["pixelShader"]["profile"],
                    "literalTranslationAvailable": exact_variant["pixelShader"]["sha256"] in evidence["translations"],
                    "exactPixelShaderBlob": True,
                    "exactNativeBindingWire": True,
                    "actualVfPass": False,
                    "runtimeAdmission": False,
                    "visualAdmission": False,
                    "occurrenceExact": False,
                }
    return status, program_candidate, program_evidence, blockers, translation


def _source_layout_axis(
    profile: dict[str, Any],
    program_evidence: dict[str, Any],
    evidence: dict[str, Any],
) -> tuple[str, dict[str, Any] | None, list[str]]:
    if (
        program_evidence.get("kind") == "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE"
        and program_evidence.get("occurrenceExact") is True
    ):
        variant = evidence["exactVariantsById"].get(program_evidence.get("variantId"))
        _require(isinstance(variant, dict), "exact Program evidence references an unknown variant")
        native_binding = variant["nativeBinding"]
        admission = variant["admission"]
        native_wire_payload = {
            key: native_binding[key]
            for key in (
                "status",
                "shaderObject",
                "bindingSemanticSha256",
                "scalarGroups",
                "vectors",
                "textures",
                "constantBufferClosure",
                "textureSampleClosure",
                "dxbcDeclarationClosure",
                "wireEntryFormat",
            )
        }
        payload = {
            "kind": "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE",
            "variantId": variant["variantId"],
            "variantKeySha256": variant["variantKeySha256"],
            "dxbcSha256": variant["pixelShader"]["sha256"],
            "bindingSemanticSha256": native_binding["bindingSemanticSha256"],
            "nativeBindingWireSha256": canonical_sha256(native_wire_payload),
            "nativeBindingWire": native_wire_payload,
            "nativeWireCounts": {
                "scalarGroupCount": len(native_binding["scalarGroups"]),
                "vectorCount": len(native_binding["vectors"]),
                "textureCount": len(native_binding["textures"]),
            },
            "runtimePacketTopologyMaterialized": False,
            "admits": "EXACT_NATIVE_WIRE_IDENTITY_ONLY",
            "sourceExactNativeScalarGroupPacking": admission["sourceExactNativeScalarGroupPacking"],
            "sourceExactSampler": admission["sourceExactSampler"],
            "sourceValueReplay": admission["sourceValueReplay"],
            "actualVfPass": admission["actualVfPass"],
            "runtimeAdmission": admission["productRuntime"],
            "visualAdmission": admission["visual"],
        }
        return (
            "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION",
            payload,
            [
                "EXACT_VARIANT_PACKET_TRANSLATION_REQUIRED",
                "RUNTIME_PACKET_NOT_MATERIALIZED",
                "SCALAR_VECTOR_PACKING_UNRESOLVED",
                "SAMPLER_STATE_UNPROVEN",
                "SOURCE_VALUE_REPLAY_UNPROVEN",
            ],
        )

    parent = program_evidence.get("effectiveParentMaterialPath")
    abi = evidence["named"].get(parent) if isinstance(parent, str) else None
    blockers: list[str] = []
    if isinstance(abi, dict) and abi.get("status") == "RESOLVED_NAMED_MAPPING":
        fits, counts = _named_abi_capacity(abi)
        wire = abi.get("nativeBindingWire")
        _require(isinstance(wire, dict), "resolved named ABI has no nativeBindingWire")
        payload = {
            "kind": "NAMED_NATIVE_WIRE_EVIDENCE",
            "parentMaterialPath": parent,
            "dxbcSha256": abi.get("dxbcSha256"),
            "bindingSemanticSha256": wire.get("bindingSemanticSha256"),
            "counts": counts,
            "textureSlots": abi.get("textureSlots"),
            "scalarLanes": abi.get("scalarLanes"),
            "vectorLanes": abi.get("vectorLanes"),
            "withinCountCaps": fits,
            "runtimePacketTopologyMaterialized": False,
            "admits": "NAMED_LANE_IDENTITY_ONLY",
        }
        blockers.extend(("RUNTIME_PACKET_NOT_MATERIALIZED", "SCALAR_VECTOR_PACKING_UNRESOLVED", "SAMPLER_STATE_UNPROVEN"))
        if counts["textureSlotCount"]:
            blockers.append("TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED")
        if fits:
            return "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS", payload, blockers
        blockers.append("CURRENT_PACKET_CAPACITY_EXCEEDED")
        return "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION", payload, blockers

    names_payload = {
        "kind": "SOURCE_PARAMETER_NAMES_ONLY",
        "parentMaterialPath": parent,
        "textureNames": [row.get("name") for row in profile.get("textures", []) if isinstance(row, dict)],
        "scalarNames": [row.get("name") for row in profile.get("scalars", []) if isinstance(row, dict)],
        "vectorNames": [row.get("name") for row in profile.get("vectors", []) if isinstance(row, dict)],
        "dynamicParameterSemantics": profile.get("dynamicParameterSemantics", []),
    }
    has_names = any(names_payload[key] for key in ("textureNames", "scalarNames", "vectorNames", "dynamicParameterSemantics"))
    blockers.extend(("NAMED_ABI_BLOCKED", "SOURCE_REGISTER_BINDING_UNRESOLVED", "TEXTURE_PARAMETER_TO_SLOT_UNRESOLVED"))
    if has_names:
        return "SOURCE_NAMES_ONLY", names_payload, blockers
    return "UNRESOLVED", None, blockers


def _source_descriptor(profile: dict[str, Any], resources: list[dict[str, Any]]) -> dict[str, Any]:
    return {
        "kind": "SOURCE_VALUES_UNPACKED",
        "textures": profile.get("textures", []),
        "scalars": profile.get("scalars", []),
        "vectors": profile.get("vectors", []),
        "staticSwitches": profile.get("staticSwitches", []),
        "dynamicParameterSemantics": profile.get("dynamicParameterSemantics", []),
        "resourceBindings": resources,
    }


def _binding_clips(value: Any, label: str) -> list[str]:
    if isinstance(value, str):
        return [_require_string(value, label)]
    if isinstance(value, dict):
        return [_require_string(value.get("clip"), label + ".clip")]
    if isinstance(value, list):
        result: list[str] = []
        for index, child in enumerate(value):
            result.extend(_binding_clips(child, f"{label}[{index}]"))
        return result
    raise InventoryError(label + " contains an unsupported clip entry")


def _parse_animevent_fields(text: str, label: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    cursor = 0
    for match in ANIMEVENT_FIELD_RE.finditer(text):
        _require(not text[cursor:match.start()].strip(), label + " contains malformed field text")
        key = match.group(1)
        value = match.group(2)
        _require(key not in fields, label + " contains duplicate field " + key)
        if value.startswith('"'):
            _require(value.endswith('"'), label + " contains malformed quoted field")
            value = value[1:-1]
        fields[key] = value
        cursor = match.end()
    _require(not text[cursor:].strip(), label + " contains malformed trailing field text")
    return fields


def _number_field(fields: dict[str, str], key: str, label: str, default: float | None = None) -> float:
    if key not in fields:
        _require(default is not None, label + " is missing " + key)
        return float(default)
    try:
        value = float(fields[key])
    except ValueError as error:
        raise InventoryError(label + " has non-numeric " + key) from error
    _require(math.isfinite(value), label + " has non-finite " + key)
    return value


def _build_composition(
    tracker: InputTracker,
    authored_assets: set[str],
) -> tuple[CompositionRegistry, dict[str, list[str]], dict[str, Any]]:
    registry = CompositionRegistry()
    by_asset: dict[str, list[str]] = defaultdict(list)

    player_skills = tracker.read_json(PLAYER_SKILLS_PATH, "PLAYER_SKILL_IDENTITY")
    _require(isinstance(player_skills, dict) and player_skills.get("schema") == "lostark.player-skills", "PlayerSkills schema is unsupported")
    skill_classes: dict[int, str] = {}
    for row in player_skills.get("skills", []):
        _require(isinstance(row, dict), "PlayerSkills row must be an object")
        skill_id = _require_int(row.get("skillId"), "PlayerSkills.skillId", 1)
        character_class = _require_string(row.get("characterClass"), "PlayerSkills.characterClass")
        _require(skill_id not in skill_classes, "duplicate PlayerSkills.skillId")
        skill_classes[skill_id] = character_class

    character_asset_sets: dict[str, set[str]] = {}
    character_cue_row_count = 0
    for domain_name, character_class in (
        ("Artist", "ARTIST"),
        ("DimensionMaster", "DIMENSIONMASTER"),
        ("LanceMaster", "LANCE_MASTER"),
        ("Warlord", "WARLORD"),
    ):
        binding_path = Path(f"Data/Animation/Authored/{domain_name}/{domain_name}.skillbindings.json")
        binding_document = tracker.read_json(binding_path, "CHARACTER_SKILL_BINDINGS")
        _require(
            isinstance(binding_document, dict)
            and binding_document.get("schema") == "lostark.animation-skill-bindings"
            and binding_document.get("characterClass") == character_class,
            binding_path.as_posix() + " identity is unsupported",
        )
        clip_to_skill: dict[str, set[int]] = defaultdict(set)
        seen_skill_ids: set[int] = set()
        bindings = binding_document.get("bindings")
        _require(isinstance(bindings, list), binding_path.as_posix() + " has no bindings")
        for binding in bindings:
            _require(isinstance(binding, dict), "skill binding row must be an object")
            skill_id = _require_int(binding.get("skillId"), "skill binding skillId", 1)
            _require(skill_id not in seen_skill_ids, "duplicate skill binding skillId")
            seen_skill_ids.add(skill_id)
            _require(skill_classes.get(skill_id) == character_class, "skill binding class differs from PlayerSkills")
            clips = _binding_clips(binding.get("clips"), "skill binding clips")
            _require(bool(clips), "skill binding has no clips")
            for clip in clips:
                clip_to_skill[clip].add(skill_id)

        animevent_path = Path(f"Data/Animation/Authored/{domain_name}/{domain_name}.animevents")
        payload = tracker.read_bytes(animevent_path, "CHARACTER_EFFECT_CUES")
        try:
            lines = payload.decode("utf-8").splitlines()
        except UnicodeError as error:
            raise InventoryError(animevent_path.as_posix() + " is not UTF-8") from error
        asset_set: set[str] = set()
        for line_number, line in enumerate(lines, 1):
            match = ANIMEVENT_RE.match(line)
            if match is None:
                continue
            fields = _parse_animevent_fields(match.group("fields"), f"{animevent_path}:{line_number}")
            if fields.get("effectref") != "asset":
                continue
            character_cue_row_count += 1
            effect_asset_id = _require_string(fields.get("payload"), "animevent asset payload")
            _require(effect_asset_id in authored_assets, "animevent references unknown target Effect: " + effect_asset_id)
            clip = match.group("clip")
            skill_ids = sorted(clip_to_skill.get(clip, set()))
            _require(len(skill_ids) == 1, "effectref clip does not resolve to exactly one skillId: " + character_class + "/" + clip)
            start_ms_value = _number_field(fields, "startms", "animevent cue")
            _require(start_ms_value.is_integer() and start_ms_value >= 0, "animevent startms must be a non-negative integer")
            transform = {
                "position": [_number_field(fields, key, "animevent cue", 0.0) for key in ("px", "py", "pz")],
                "rotationDegrees": [_number_field(fields, key, "animevent cue", 0.0) for key in ("rx", "ry", "rz")],
                "scale": [_number_field(fields, key, "animevent cue", 1.0) for key in ("sx", "sy", "sz")],
            }
            anchor = _require_string(fields.get("anchor"), "animevent anchor")
            follow = _require_string(fields.get("follow"), "animevent follow policy")
            stop = _require_string(fields.get("stop"), "animevent stop policy")
            semantic = {
                "kind": "CHARACTER_ANIMATION_CUE",
                "characterClass": character_class,
                "skillId": skill_ids[0],
                "clip": clip,
                "startMs": int(start_ms_value),
                "effectAssetId": effect_asset_id,
                "anchor": anchor,
                "follow": follow,
                "stop": stop,
                "localTransform": transform,
            }
            if "orientation" in fields:
                semantic["orientation"] = _require_string(fields["orientation"], "animevent orientation")
            variant_id = registry.add(
                semantic,
                {"path": animevent_path.as_posix(), "lineNumber": line_number},
            )
            by_asset[effect_asset_id].append(variant_id)
            asset_set.add(effect_asset_id)
        character_asset_sets[domain_name] = asset_set

    encounter = tracker.read_json(VALTAN_ENCOUNTER_PATH, "VALTAN_STAGE_TIMING")
    _require(
        isinstance(encounter, dict)
        and encounter.get("schema") == "lostark.encounter-profile"
        and encounter.get("encounterId") == "ENCOUNTER_VALTAN"
        and encounter.get("bossArchetypeId") == "BOSS_VALTAN",
        "Valtan encounter identity is unsupported",
    )
    encounter_stages: dict[str, dict[str, Any]] = {}
    for pattern in encounter.get("patterns", []):
        _require(isinstance(pattern, dict), "Valtan encounter pattern must be an object")
        pattern_id = _require_string(pattern.get("patternId"), "Valtan patternId")
        for stage in pattern.get("stages", []):
            _require(isinstance(stage, dict), "Valtan encounter stage must be an object")
            action_id = _require_string(stage.get("actionId"), "Valtan stage actionId")
            _require(action_id not in encounter_stages, "duplicate Valtan stage actionId")
            encounter_stages[action_id] = {
                "patternId": pattern_id,
                "stageId": _require_string(stage.get("stageId"), "Valtan stageId"),
                "durationMs": _require_int(stage.get("durationMs"), "Valtan stage durationMs"),
            }

    animnotify_payload = tracker.read_bytes(VALTAN_ANIMNOTIFY_PATH, "VALTAN_CLIP_DURATION")
    try:
        animnotify_lines = animnotify_payload.decode("utf-8").splitlines()
    except UnicodeError as error:
        raise InventoryError("Valtan.animnotify is not UTF-8") from error
    clip_durations_ms: dict[str, int] = {}
    clip_header_re = re.compile(r'^"(?P<clip>[^"]+)"\s+skill=\d+\s+len=(?P<seconds>\d+(?:\.\d+)?)')
    for line in animnotify_lines:
        match = clip_header_re.match(line)
        if match is None:
            continue
        clip = match.group("clip")
        _require(clip not in clip_durations_ms, "duplicate Valtan animnotify clip")
        clip_durations_ms[clip] = round(float(match.group("seconds")) * 1000.0)

    pattern_bindings = tracker.read_json(VALTAN_PATTERN_BINDINGS_PATH, "VALTAN_PATTERN_BINDINGS")
    _require(
        isinstance(pattern_bindings, dict)
        and pattern_bindings.get("schema") == "lostark.valtan-pattern-bindings"
        and pattern_bindings.get("bossArchetypeId") == "BOSS_VALTAN",
        "Valtan pattern bindings identity is unsupported",
    )
    clip_occurrences: dict[str, dict[str, Any]] = {}
    for binding in pattern_bindings.get("bindings", []):
        _require(isinstance(binding, dict), "Valtan pattern binding must be an object")
        action_id = _require_string(binding.get("actionId"), "Valtan binding actionId")
        _require(action_id in encounter_stages, "Valtan binding action is absent from encounter: " + action_id)
        for clip_row in binding.get("clips", []):
            _require(isinstance(clip_row, dict), "Valtan binding clip must be an object")
            occurrence_id = _require_string(clip_row.get("clipOccurrenceId"), "Valtan clipOccurrenceId")
            _require(occurrence_id not in clip_occurrences, "duplicate Valtan clipOccurrenceId")
            clip = _require_string(clip_row.get("clip"), "Valtan binding clip")
            _require(clip in clip_durations_ms, "Valtan binding clip is absent from animnotify: " + clip)
            source_start = _require_int(clip_row.get("sourceStartMs"), "Valtan sourceStartMs")
            _require(source_start <= clip_durations_ms[clip], "Valtan sourceStartMs exceeds clip duration")
            clip_occurrences[occurrence_id] = {
                "actionId": action_id,
                "clip": clip,
                "sourceStartMs": source_start,
                "clipDurationMs": clip_durations_ms[clip],
            }

    valtan_cues = tracker.read_json(VALTAN_CUES_PATH, "VALTAN_PATTERN_EFFECT_CUES")
    _require(
        isinstance(valtan_cues, dict)
        and valtan_cues.get("schema") == "lostark.valtan-pattern-effect-cues"
        and valtan_cues.get("ownerArchetypeId") == "BOSS_VALTAN",
        "Valtan cue identity is unsupported",
    )
    valtan_assets: set[str] = set()
    seen_binding_ids: set[str] = set()
    seen_occurrence_ids: set[str] = set()
    for index, cue in enumerate(valtan_cues.get("cues", [])):
        _require(isinstance(cue, dict), "Valtan cue must be an object")
        binding_id = _require_string(cue.get("bindingId"), "Valtan cue bindingId")
        cue_occurrence_id = _require_string(cue.get("occurrenceId"), "Valtan cue occurrenceId")
        _require(binding_id not in seen_binding_ids, "duplicate Valtan cue bindingId")
        _require(cue_occurrence_id not in seen_occurrence_ids, "duplicate Valtan cue occurrenceId")
        seen_binding_ids.add(binding_id)
        seen_occurrence_ids.add(cue_occurrence_id)
        clip_occurrence_id = _require_string(cue.get("clipOccurrenceId"), "Valtan cue clipOccurrenceId")
        binding = clip_occurrences.get(clip_occurrence_id)
        _require(binding is not None, "Valtan cue references unknown clipOccurrenceId")
        action_id = _require_string(cue.get("actionId"), "Valtan cue actionId")
        _require(binding["actionId"] == action_id, "Valtan cue action differs from pattern binding")
        stage = encounter_stages[action_id]
        _require(cue.get("patternId") == stage["patternId"], "Valtan cue pattern differs from encounter")
        _require(cue.get("stageId") == stage["stageId"], "Valtan cue stage differs from encounter")
        source_start = _require_int(cue.get("sourceStartMs"), "Valtan cue sourceStartMs")
        _require(source_start == binding["sourceStartMs"], "Valtan cue sourceStartMs differs from pattern binding")
        _require(source_start <= binding["clipDurationMs"], "Valtan cue sourceStartMs exceeds clip duration")
        source_end = cue.get("sourceEndMs")
        if source_end is not None:
            source_end = _require_int(source_end, "Valtan cue sourceEndMs")
            _require(source_end >= source_start, "Valtan cue sourceEndMs precedes sourceStartMs")
            _require(source_end <= binding["clipDurationMs"], "Valtan cue sourceEndMs exceeds clip duration")
        effect_asset_id = _require_string(cue.get("effectAssetId"), "Valtan cue effectAssetId")
        _require(effect_asset_id in authored_assets, "Valtan cue references unknown Effect: " + effect_asset_id)
        anchor_slot_id = _require_string(cue.get("anchorSlotId"), "Valtan cue anchorSlotId")
        follow_policy = _require_string(cue.get("followPolicy"), "Valtan cue followPolicy")
        stop_policy = _require_string(cue.get("stopPolicy"), "Valtan cue stopPolicy")
        repeat_policy = _require_string(cue.get("repeatPolicy"), "Valtan cue repeatPolicy")
        local_transform = cue.get("localTransform")
        _require(isinstance(local_transform, dict), "Valtan cue localTransform must be an object")
        for transform_key in ("position", "rotationDegrees", "scale"):
            transform_value = local_transform.get(transform_key)
            _require(
                isinstance(transform_value, list)
                and len(transform_value) == 3
                and all(
                    isinstance(item, (int, float))
                    and not isinstance(item, bool)
                    and math.isfinite(float(item))
                    for item in transform_value
                ),
                "Valtan cue localTransform." + transform_key + " must be finite float3",
            )
        semantic = {
            "kind": "VALTAN_PATTERN_CUE",
            "bindingId": binding_id,
            "occurrenceId": cue_occurrence_id,
            "patternId": stage["patternId"],
            "stageId": stage["stageId"],
            "stageDurationMs": stage["durationMs"],
            "actionId": action_id,
            "clipOccurrenceId": clip_occurrence_id,
            "clip": binding["clip"],
            "clipDurationMs": binding["clipDurationMs"],
            "effectAssetId": effect_asset_id,
            "anchorSlotId": anchor_slot_id,
            "followPolicy": follow_policy,
            "stopPolicy": stop_policy,
            "repeatPolicy": repeat_policy,
            "sourceStartMs": source_start,
            "sourceEndMs": source_end,
            "localTransform": local_transform,
        }
        variant_id = registry.add(semantic, {"path": VALTAN_CUES_PATH.as_posix(), "row": binding_id})
        by_asset[effect_asset_id].append(variant_id)
        valtan_assets.add(effect_asset_id)

    combat_objects = tracker.read_json(VALTAN_COMBAT_OBJECTS_PATH, "VALTAN_COMBAT_OBJECT_OWNERS")
    _require(
        isinstance(combat_objects, dict)
        and combat_objects.get("schema") == "lostark.valtan-combat-objects"
        and combat_objects.get("encounterId") == "ENCOUNTER_VALTAN",
        "Valtan combat-object identity is unsupported",
    )
    combat_by_archetype: dict[str, dict[str, Any]] = {}
    for row in combat_objects.get("objects", []):
        _require(isinstance(row, dict), "Valtan combat object must be an object")
        archetype = _require_string(row.get("combatObjectArchetypeId"), "combat object archetype")
        _require(archetype not in combat_by_archetype, "duplicate combat object archetype")
        owner_action = _require_string(row.get("ownerStageActionId"), "combat object owner action")
        _require(owner_action in encounter_stages, "combat object owner action is absent from encounter")
        _require(row.get("ownerPatternId") == encounter_stages[owner_action]["patternId"], "combat object owner pattern differs from encounter")
        combat_by_archetype[archetype] = row

    boss_catalog = tracker.read_json(BOSS_CATALOG_PATH, "VALTAN_BOSS_VISUAL_BINDINGS")
    _require(isinstance(boss_catalog, dict) and boss_catalog.get("schema") == "lostark.boss-catalog", "BossCatalog schema is unsupported")
    valtan_bosses = [row for row in boss_catalog.get("bosses", []) if isinstance(row, dict) and row.get("archetypeId") == "BOSS_VALTAN"]
    _require(len(valtan_bosses) == 1, "BossCatalog must contain exactly one BOSS_VALTAN")
    boss_assets: set[str] = set()
    seen_visual_ids: set[str] = set()
    for row in valtan_bosses[0].get("combatObjectVisuals", []):
        _require(isinstance(row, dict), "combatObjectVisual row must be an object")
        visual_id = _require_string(row.get("clientVisualId"), "combatObjectVisual clientVisualId")
        _require(visual_id not in seen_visual_ids, "duplicate combatObjectVisual clientVisualId")
        seen_visual_ids.add(visual_id)
        archetype = _require_string(row.get("combatObjectArchetypeId"), "combatObjectVisual archetype")
        owner = combat_by_archetype.get(archetype)
        _require(owner is not None and owner.get("clientVisualId") == visual_id, "BossCatalog combat-object owner join failed")
        effect_asset_id = _require_string(row.get("effectAssetId"), "combatObjectVisual effectAssetId")
        _require(effect_asset_id in authored_assets, "BossCatalog visual references unknown Effect")
        semantic = {
            "kind": "VALTAN_COMBAT_OBJECT_VISUAL",
            "bossArchetypeId": "BOSS_VALTAN",
            "combatObjectArchetypeId": archetype,
            "clientVisualId": visual_id,
            "ownerPatternId": owner.get("ownerPatternId"),
            "ownerStageActionId": owner.get("ownerStageActionId"),
            "effectAssetId": effect_asset_id,
        }
        variant_id = registry.add(semantic, {"path": BOSS_CATALOG_PATH.as_posix(), "row": visual_id})
        by_asset[effect_asset_id].append(variant_id)
        boss_assets.add(effect_asset_id)

    for asset in by_asset:
        by_asset[asset] = sorted(set(by_asset[asset]))
    return registry, by_asset, {
        "characterCueRowCount": character_cue_row_count,
        "characterAssetCount": len(set().union(*character_asset_sets.values())),
        "characterAssetCountsByDomain": {
            key: len(value) for key, value in sorted(character_asset_sets.items())
        },
        "valtanPatternAssetCount": len(valtan_assets),
        "valtanBossVisualAssetCount": len(boss_assets),
        "compositionVariantCount": len(registry.rows()),
        "compositionProjectionSha256": canonical_sha256(registry.rows()),
    }


def _build_authored_occurrences(
    tracker: InputTracker,
    evidence: dict[str, Any],
    registries: dict[str, VariantRegistry],
) -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]], dict[str, Any]]:
    authored_root = tracker.root / AUTHORED_DIRECTORY
    _require(authored_root.is_dir(), "authored Effect directory is absent")
    occurrences: list[dict[str, Any]] = []
    documents: dict[str, dict[str, Any]] = {}
    domain_document_counts: Counter[str] = Counter()
    domain_occurrence_counts: Counter[str] = Counter()
    carrier_counts: Counter[str] = Counter()
    fine_renderer_counts: Counter[str] = Counter()
    program_counts: Counter[str] = Counter()
    layout_counts: Counter[str] = Counter()
    adapter_counts: Counter[str] = Counter()
    descriptor_counts: Counter[str] = Counter()
    material_evidence_counts: Counter[str] = Counter()
    named_capacity_family_counts: Counter[str] = Counter()
    seen_effect_assets: set[str] = set()

    for absolute_path in sorted(authored_root.glob("*.effect.json"), key=lambda path: path.name.casefold()):
        payload = absolute_path.read_bytes()
        document = _decode_json(payload, absolute_path.name)
        _require(isinstance(document, dict), absolute_path.name + " root must be an object")
        effect_asset_id = document.get("effectAssetId")
        _require(isinstance(effect_asset_id, str), absolute_path.name + " has no effectAssetId")
        filename_target = _domain_for_asset(absolute_path.name[:-len(".effect.json")]) is not None
        id_domain = _domain_for_asset(effect_asset_id)
        _require(
            filename_target == (id_domain is not None),
            "target-prefix filename/parsed-ID scope mismatch: " + absolute_path.name,
        )
        if id_domain is None:
            continue
        domain_name, _ = id_domain
        expected_name = effect_asset_id + ".effect.json"
        _require(absolute_path.name == expected_name, "authored filename differs from effectAssetId: " + absolute_path.name)
        _require(effect_asset_id not in seen_effect_assets, "duplicate authored effectAssetId: " + effect_asset_id)
        seen_effect_assets.add(effect_asset_id)
        _require(document.get("schema") == "lostark.effect-authoring", "authored schema is unsupported: " + effect_asset_id)
        version = _require_int(document.get("version"), "authored version")
        _require(10 <= version <= 13, "authored version is outside supported 10..13: " + effect_asset_id)
        relative_path = absolute_path.relative_to(tracker.root)
        tracker.add_parsed(relative_path, "TARGET_AUTHORED_DOCUMENT", payload, document)
        elements = document.get("elements")
        _require(isinstance(elements, list), "authored document has no elements: " + effect_asset_id)
        element_hashes: dict[str, str] = {}
        element_order: list[str] = []
        domain_document_counts[domain_name] += 1

        for order, element in enumerate(elements):
            _require(isinstance(element, dict), "authored element must be an object: " + effect_asset_id)
            stable_id = _require_string(element.get("id"), "authored element.id")
            _require(stable_id not in element_hashes, "duplicate element.id: " + effect_asset_id + "/" + stable_id)
            element_sha = canonical_sha256(element)
            element_hashes[stable_id] = element_sha
            element_order.append(stable_id)
            carrier, source_shape, mesh_models = _coarse_carrier(element)
            type_data_classes = _source_type_data_classes(element)
            dxbc_source_carrier = _dxbc_source_carrier(element, source_shape, type_data_classes)
            fine_renderer_kind, vertex_factory_candidates = _fine_renderer_kind(
                element["kind"], source_shape, mesh_models, type_data_classes
            )
            mesh_body_missing = (
                "particlemoduletypedatamesh" in type_data_classes and not mesh_models
            )
            ribbon_body_misclassified = (
                "particlemoduletypedataribbon" in type_data_classes and carrier == "SPRITE"
            )
            carrier_replacement_blocked = mesh_body_missing or ribbon_body_misclassified
            carrier_counts[carrier] += 1
            fine_renderer_counts[fine_renderer_kind] += 1
            domain_occurrence_counts[domain_name] += 1
            kind = element["kind"]
            resources = [
                {"slotId": row["slotId"], "assetId": row["assetId"]}
                for row in element.get("resources", [])
            ]
            carrier_variant_id = registries["carrier"].add(
                {
                    "coarseCarrier": carrier,
                    "authoredKind": kind,
                    "fineRendererKind": fine_renderer_kind,
                    "vertexFactoryCandidates": vertex_factory_candidates,
                    "sourceRendererShape": source_shape or None,
                    "meshModelAssets": mesh_models,
                }
            )
            material = element.get("material")
            _require(isinstance(material, dict), "authored element has no material object")
            execution = _execution(element)
            profile = _source_profile(element)
            source_profile = _raw_source_profile(element)
            source_material_path = (
                material.get("sourceMaterialPath")
                if isinstance(material.get("sourceMaterialPath"), str)
                else ""
            )
            authored_parent_path = (
                source_profile.get("parentMaterialPath")
                if isinstance(source_profile.get("parentMaterialPath"), str)
                else ""
            )
            (
                source_parent_path,
                source_parent_resolution,
                source_parent_row_sha,
                source_parent_blocker,
            ) = _resolve_parent(authored_parent_path, source_material_path, evidence["child"])
            _require(not (execution is not None and profile is not None), "material enables execution and sourceProfile together")

            blockers: set[str] = set()
            program_status: str
            program_candidate_id: str | None = None
            program_evidence_id: str | None = None
            layout_status: str
            layout_candidate_id: str | None = None
            layout_evidence_id: str | None = None
            descriptor_status: str
            descriptor_variant_id: str | None = None
            adapter_status: str
            adapter_candidate_id: str | None = None
            translation: dict[str, Any] | None = None
            source_program_evidence: dict[str, Any] | None = None
            adapter_vertex_factory_candidates = list(vertex_factory_candidates)

            if carrier == "PRESENTATION":
                material_evidence_counts["none"] += 1
                program_status = "NOT_APPLICABLE_PRESENTATION"
                layout_status = "NOT_APPLICABLE_PRESENTATION"
                descriptor_status = "NOT_APPLICABLE_PRESENTATION"
                adapter_status = "PRESENTATION_SEPARATE"
            elif execution is not None:
                material_evidence_counts["typedExecution"] += 1
                program_payload, layout_payload, descriptor_payload, typed_adapter = _typed_material_axes(execution)
                program_status = "TYPED_RUNTIME_PROGRAM_DECLARED"
                program_candidate_id = registries["programCandidate"].add(program_payload)
                program_evidence_id = registries["programEvidence"].add(
                    {"kind": "TYPED_EXECUTION_RECEIPT", "programCandidateId": program_candidate_id}
                )
                layout_status = "TYPED_PACKET_CLOSED"
                layout_candidate_id = registries["layoutCandidate"].add(layout_payload)
                descriptor_status = "TYPED_VALUES_CLOSED"
                descriptor_payload["layoutCandidateId"] = layout_candidate_id
                descriptor_variant_id = registries["descriptor"].add(descriptor_payload)
                adapter_status = "TYPED_STATIC_DISPATCH_CANDIDATE"
                if not carrier_replacement_blocked:
                    adapter_candidate_id = registries["adapter"].add(
                    {
                        "kind": "TYPED_STATIC_DISPATCH",
                        "coarseCarrier": carrier,
                        "authoredKind": kind,
                        "fineRendererKind": fine_renderer_kind,
                        "vertexFactoryCandidates": adapter_vertex_factory_candidates,
                        "sourceRendererShape": source_shape or None,
                        "templateId": material.get("templateId"),
                        "renderProfile": material.get("renderProfile"),
                        **typed_adapter,
                        "runtimeVerified": False,
                    }
                    )
            else:
                material_evidence_counts["sourceProfile" if profile is not None else "none"] += 1
                program_status, program_payload, source_program_evidence, program_blockers, translation = _source_program_axis(
                    source_profile, material, dxbc_source_carrier, fine_renderer_kind, evidence
                )
                blockers.update(program_blockers)
                if program_payload is not None:
                    program_candidate_id = registries["programCandidate"].add(program_payload)
                program_evidence_id = registries["programEvidence"].add(source_program_evidence)
                cooked_parent = source_program_evidence.get("effectiveParentMaterialPath")
                cooked_adapter_row = evidence["cooked"].get(cooked_parent) if isinstance(cooked_parent, str) else None
                cooked_vfs = cooked_adapter_row.get("vertexFactoryTypes") if isinstance(cooked_adapter_row, dict) else None
                if isinstance(cooked_vfs, list):
                    _require(all(isinstance(value, str) and value for value in cooked_vfs), "cooked vertexFactoryTypes are malformed")
                    adapter_vertex_factory_candidates = sorted(set(cooked_vfs))
                layout_status, layout_payload, layout_blockers = _source_layout_axis(source_profile, source_program_evidence, evidence)
                blockers.update(layout_blockers)
                if layout_payload is not None:
                    layout_evidence_id = registries["layoutEvidence"].add(layout_payload)
                if profile is not None:
                    descriptor_status = "SOURCE_VALUES_PRESENT_UNPACKED"
                    descriptor_variant_id = registries["descriptor"].add(_source_descriptor(profile, resources))
                elif resources:
                    descriptor_status = "RESOURCE_ONLY_NO_MATERIAL_VALUES"
                    descriptor_variant_id = registries["descriptor"].add(
                        {"kind": "RESOURCE_BINDINGS_ONLY", "resourceBindings": resources}
                    )
                else:
                    descriptor_status = "MISSING"
                adapter_status = "RENDER_PROFILE_STATIC_CANDIDATE"
                if not carrier_replacement_blocked:
                    adapter_candidate_id = registries["adapter"].add(
                    {
                        "kind": "RENDER_PROFILE_STATIC_DISPATCH",
                        "coarseCarrier": carrier,
                        "authoredKind": kind,
                        "fineRendererKind": fine_renderer_kind,
                        "vertexFactoryCandidates": adapter_vertex_factory_candidates,
                        "sourceRendererShape": source_shape or None,
                        "templateId": material.get("templateId"),
                        "renderProfile": material.get("renderProfile"),
                        "runtimeVerified": False,
                    }
                    )

            if mesh_body_missing:
                blockers.add("MESH_TYPEDATA_MESH_MODEL_MISSING")
                blockers.add("SOURCE_CARRIER_REPLACEMENT_REQUIRED")
            if ribbon_body_misclassified:
                blockers.add("RIBBON_TYPEDATA_COARSE_BUCKET_SPRITE")
                blockers.add("SOURCE_CARRIER_REPLACEMENT_REQUIRED")
            if carrier_replacement_blocked and carrier != "PRESENTATION":
                adapter_status = "UNRESOLVED"
                adapter_candidate_id = None
            if source_parent_blocker is not None:
                blockers.add("CHILD_PARENT_RESOLUTION_BLOCKED")

            if carrier != "PRESENTATION":
                blockers.add("COMPILED_DRAW_DISPATCH_UNPROVEN")
                blockers.add("VERTEX_FACTORY_UNPROVEN")
                if translation is not None:
                    declarations = translation.get("declarations")
                    if isinstance(declarations, dict) and declarations.get("inputs"):
                        blockers.add("STAGE_INPUT_SEMANTICS_UNPROVEN")
                    outputs = declarations.get("outputs") if isinstance(declarations, dict) else None
                    if isinstance(outputs, list) and outputs != ["o0"]:
                        blockers.add("OUTPUT_TOPOLOGY_MRT_UNPROVEN")
                    constant_buffers = declarations.get("constantBuffers") if isinstance(declarations, dict) else None
                    if isinstance(constant_buffers, dict) and any(key != "cb0" for key in constant_buffers):
                        blockers.add("SCENE_INPUTS_UNPROVEN")
                if carrier == "MESH":
                    blockers.add("WPO_VERTEX_PROGRAM_UNPROVEN")

            program_counts[program_status] += 1
            layout_counts[layout_status] += 1
            adapter_counts[adapter_status] += 1
            descriptor_counts[descriptor_status] += 1
            if layout_status == "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS":
                named_capacity_family_counts["fitOccurrences"] += 1
            elif layout_status == "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION":
                named_capacity_family_counts["extensionOccurrences"] += 1

            occurrence = {
                "occurrenceId": _occurrence_id(effect_asset_id, stable_id),
                "effectAssetId": effect_asset_id,
                "elementId": stable_id,
                "elementOrder": order,
                "authoredPath": relative_path.as_posix(),
                "authoredVersion": version,
                "authoredElementSha256": element_sha,
                "domain": domain_name,
                "visible": element.get("visible") is True,
                "kind": kind,
                "sourceTypeDataClasses": type_data_classes,
                "sourceProfileEnabled": profile is not None,
                "sourceMaterialPath": source_material_path or None,
                "sourceParentMaterialPath": source_parent_path or None,
                "sourceParentResolution": source_parent_resolution,
                "sourceParentRowSha256": source_parent_row_sha,
                "sourceParentBlocker": source_parent_blocker,
                "sourceRendererShape": source_shape or None,
                "dxbcSourceCarrier": dxbc_source_carrier or None,
                "fineRendererKind": fine_renderer_kind,
                "vertexFactoryCandidates": adapter_vertex_factory_candidates,
                "carrier": carrier,
                "carrierVariantId": carrier_variant_id,
                "program": {
                    "status": program_status,
                    "programCandidateId": program_candidate_id,
                    "programEvidenceId": program_evidence_id,
                },
                "layout": {
                    "status": layout_status,
                    "layoutCandidateId": layout_candidate_id,
                    "layoutEvidenceId": layout_evidence_id,
                },
                "adapter": {
                    "status": adapter_status,
                    "adapterCandidateId": adapter_candidate_id,
                    "runtimeVerified": False,
                },
                "descriptor": {
                    "status": descriptor_status,
                    "descriptorVariantId": descriptor_variant_id,
                },
                "blockers": sorted(blockers),
                "_documentName": absolute_path.name,
                "_sourceProgramEvidence": source_program_evidence,
            }
            occurrences.append(occurrence)

        documents[effect_asset_id] = {
            "effectAssetId": effect_asset_id,
            "domain": domain_name,
            "authoredPath": relative_path.as_posix(),
            "authoredVersion": version,
            "canonicalSha256": canonical_sha256(document),
            "elementCount": len(elements),
            "elementOrder": element_order,
            "elementHashes": element_hashes,
        }

    named_resolved = [row for row in evidence["named"].values() if row.get("status") == "RESOLVED_NAMED_MAPPING"]
    named_fit_family_count = sum(_named_abi_capacity(row)[0] for row in named_resolved)
    named_extension_family_count = len(named_resolved) - named_fit_family_count
    named_fit_zero_texture_count = sum(
        _named_abi_capacity(row)[0] and len(row.get("textureSlots", [])) == 0
        for row in named_resolved
    )
    return occurrences, documents, {
        "domainDocumentCounts": dict(sorted(domain_document_counts.items())),
        "domainOccurrenceCounts": dict(sorted(domain_occurrence_counts.items())),
        "carrierCounts": dict(sorted(carrier_counts.items())),
        "fineRendererKindCounts": dict(sorted(fine_renderer_counts.items())),
        "programStatusCounts": {status: program_counts[status] for status in PROGRAM_STATUSES},
        "layoutStatusCounts": {status: layout_counts[status] for status in LAYOUT_STATUSES},
        "adapterStatusCounts": {status: adapter_counts[status] for status in ADAPTER_STATUSES},
        "descriptorStatusCounts": {status: descriptor_counts[status] for status in DESCRIPTOR_STATUSES},
        "materialEvidenceCounts": dict(sorted(material_evidence_counts.items())),
        "namedAbiResolvedFamilyCount": len(named_resolved),
        "namedAbiWithinCountCapsFamilyCount": named_fit_family_count,
        "namedAbiRequiresCountExtensionFamilyCount": named_extension_family_count,
        "namedAbiWithinCountCapsZeroTextureFamilyCount": named_fit_zero_texture_count,
        "namedAbiWithinCountCapsNonemptyPacketTopologyMaterializedFamilyCount": 0,
    }


def _published_element_exact_projection(
    current_order: list[str],
    current_hashes: dict[str, str],
    published_order: list[str],
    published_hashes: dict[str, str],
) -> dict[str, bool]:
    published_indices = {stable_id: index for index, stable_id in enumerate(published_order)}
    return {
        stable_id: (
            published_hashes.get(stable_id) == current_hashes[stable_id]
            and published_indices.get(stable_id) == current_index
        )
        for current_index, stable_id in enumerate(current_order)
    }


def _validate_runtime_catalog_contract(value: Any, catalog_path: Path) -> None:
    _require(
        isinstance(value, dict)
        and tuple(value) == (
            "schema",
            "formatVersion",
            "materialPrograms",
            "components",
            "effects",
        )
        and value.get("schema") == "lostark.effect-runtime-catalog"
        and type(value.get("formatVersion")) is int
        and value["formatVersion"] == 4,
        "runtime Effect catalog identity is unsupported",
    )
    try:
        direct_authored_runtime_contract.validate_runtime_catalog(value, catalog_path)
    except direct_authored_runtime_contract.ContractError as error:
        raise InventoryError(
            "runtime Effect catalog violates the publisher contract: " + str(error)
        ) from error


def _build_catalog_joins(
    tracker: InputTracker,
    documents: dict[str, dict[str, Any]],
    product_consumers: dict[str, list[str]],
) -> tuple[dict[str, Any], dict[str, Any]]:
    authoring_catalog = tracker.read_json(AUTHORING_CATALOG_PATH, "AUTHORING_EFFECT_CATALOG")
    _require(isinstance(authoring_catalog, dict) and authoring_catalog.get("formatVersion") == 1, "EffectCatalog formatVersion is unsupported")
    catalog_assets: dict[str, dict[str, Any]] = {}
    scoped_catalog_assets: dict[str, dict[str, Any]] = {}
    all_catalog_ids: set[str] = set()
    rows = authoring_catalog.get("effects")
    _require(isinstance(rows, list), "EffectCatalog.effects must be an array")
    for row in rows:
        _require(isinstance(row, dict), "EffectCatalog row must be an object")
        asset = _require_string(row.get("effectAssetId"), "EffectCatalog.effectAssetId")
        _require(asset not in all_catalog_ids, "duplicate EffectCatalog.effectAssetId")
        all_catalog_ids.add(asset)
        if _domain_for_asset(asset) is None:
            continue
        scoped_catalog_assets[asset] = row
        if asset not in documents:
            _require(
                asset == "effect.artist.skill.31470"
                and row.get("authoringPath") is None,
                "target catalog row has no authored occurrence identity: " + asset,
            )
            continue
        path = _safe_relative_path(row.get("authoringPath"), "EffectCatalog.authoringPath")
        expected = "Effects/Authored/" + asset + ".effect.json"
        _require(path == expected, "EffectCatalog authored path differs from stable asset identity")
        catalog_assets[asset] = row

    runtime_catalog = tracker.read_json(RUNTIME_CATALOG_PATH, "RUNTIME_EFFECT_CATALOG")
    _validate_runtime_catalog_contract(
        runtime_catalog,
        tracker.root / RUNTIME_CATALOG_PATH,
    )
    runtime_assets: dict[str, dict[str, Any]] = {}
    scoped_runtime_ids: set[str] = set()
    scoped_runtime_missing_authored_ids: set[str] = set()
    all_runtime_ids: set[str] = set()
    runtime_rows = runtime_catalog.get("effects")
    _require(isinstance(runtime_rows, list), "runtime Effect catalog effects must be an array")
    for row in runtime_rows:
        _require(isinstance(row, dict), "runtime Effect row must be an object")
        asset = _require_string(row.get("effectAssetId"), "runtime effectAssetId")
        _require(asset not in all_runtime_ids, "duplicate runtime effectAssetId")
        all_runtime_ids.add(asset)
        if _domain_for_asset(asset) is not None:
            scoped_runtime_ids.add(asset)
        if asset not in documents:
            if _domain_for_asset(asset) is not None:
                scoped_runtime_missing_authored_ids.add(asset)
            continue
        _require(row.get("payloadKind") == "DIRECT_AUTHORED_DOCUMENT_V13", "target runtime Effect is not direct-authored v13")
        published_relative = _safe_relative_path(row.get("authoredDocumentPath"), "runtime authoredDocumentPath")
        published_path = Path("Client/Bin/DataFiles/Effect") / Path(published_relative)
        published = tracker.read_json(published_path, "PUBLISHED_EFFECT_DOCUMENT")
        _require(isinstance(published, dict), "published Effect root must be an object")
        _require(
            published.get("schema") == "lostark.effect-authoring"
            and isinstance(published.get("version"), int)
            and not isinstance(published.get("version"), bool)
            and 10 <= published["version"] <= 13,
            "published Effect identity is unsupported",
        )
        _require(published.get("effectAssetId") == asset, "published Effect asset identity drifted")
        _require(published.get("version") == row.get("authoringFormatVersion"), "runtime catalog version differs from published document")
        published_elements = published.get("elements")
        _require(isinstance(published_elements, list), "published Effect has no elements")
        published_hashes: dict[str, str] = {}
        published_order: list[str] = []
        for element in published_elements:
            _require(isinstance(element, dict), "published element must be an object")
            stable_id = _require_string(element.get("id"), "published element.id")
            _require(stable_id not in published_hashes, "duplicate published element.id")
            published_order.append(stable_id)
            published_hashes[stable_id] = canonical_sha256(element)
        current = documents[asset]
        element_exact = _published_element_exact_projection(
            current["elementOrder"],
            current["elementHashes"],
            published_order,
            published_hashes,
        )
        runtime_assets[asset] = {
            "catalogRow": row,
            "publishedPath": published_path.as_posix(),
            "publishedCanonicalSha256": canonical_sha256(published),
            "publishedElementOrder": published_order,
            "elementExact": element_exact,
            "documentExact": published_order == current["elementOrder"] and all(element_exact.values()),
        }

    product_assets = set(product_consumers)
    joins: dict[str, Any] = {}
    for asset, document in documents.items():
        catalog_declared = asset in catalog_assets
        runtime_published = asset in runtime_assets
        product_consumed = asset in product_assets
        published = runtime_assets.get(asset)
        if product_consumed:
            composition_status = "PRODUCT_BOUND_CUE"
        elif runtime_published:
            composition_status = "RUNTIME_PUBLISHED_WITHOUT_CONSUMER"
        elif catalog_declared:
            composition_status = "CATALOG_DECLARED_ONLY"
        else:
            composition_status = "AUTHORED_ONLY"
        if product_consumed and runtime_published and catalog_declared:
            product_status = "PRODUCT_JOIN_CLOSED"
        elif not catalog_declared:
            product_status = "AUTHORED_NOT_CATALOGED"
        elif not runtime_published:
            product_status = "CATALOG_NOT_PUBLISHED"
        else:
            product_status = "RUNTIME_PUBLISHED_UNCONSUMED"
        joins[asset] = {
            "compositionStatus": composition_status,
            "productStatus": product_status,
            "scopeBits": {
                "authored": True,
                "catalogDeclared": catalog_declared,
                "runtimePublished": runtime_published,
                "productConsumed": product_consumed,
            },
            "compositionVariantIds": product_consumers.get(asset, []),
            "publishedPath": published["publishedPath"] if published else None,
            "publishedElementExact": published["elementExact"] if published else {},
        }
    runtime_published_occurrence_count = sum(
        len(runtime_assets[asset]["publishedElementOrder"]) for asset in runtime_assets
    )
    runtime_published_authored_occurrence_count = sum(
        len(documents[asset]["elementOrder"]) for asset in runtime_assets
    )
    return joins, {
        "scopedCatalogDeclaredAssetCount": len(scoped_catalog_assets),
        "authoredCatalogJoinedAssetCount": len(catalog_assets),
        "scopedCatalogWithoutOccurrenceIds": sorted(set(scoped_catalog_assets) - set(documents)),
        "runtimePublishedAssetCount": len(runtime_assets),
        "scopedRuntimeCatalogAssetCount": len(scoped_runtime_ids),
        "scopedRuntimeMissingAuthoredAssetIds": sorted(scoped_runtime_missing_authored_ids),
        "runtimePublishedOccurrenceCount": runtime_published_occurrence_count,
        "runtimePublishedAuthoredOccurrenceCount": runtime_published_authored_occurrence_count,
        "productConsumedAssetCount": len(product_assets),
        "runtimeAssetSetEqualsProductConsumerSet": scoped_runtime_ids == product_assets,
    }


def _legacy_golden_review(
    occurrence: dict[str, Any],
    review_receipts: dict[tuple[str, str], dict[str, Any]],
) -> dict[str, Any]:
    receipt = review_receipts.get((occurrence["effectAssetId"], occurrence["elementId"]))
    if receipt is None:
        return {"status": "NOT_RECORDED", "receiptOccurrenceId": None}
    node = receipt.get("authoredNode")
    _require(isinstance(node, dict), "restoration receipt has no authoredNode")
    receipt_occurrence_id = _require_string(receipt.get("occurrenceId"), "restoration occurrenceId")
    exact = (
        receipt_occurrence_id == occurrence["occurrenceId"]
        and node.get("authoredElementSha256") == occurrence["authoredElementSha256"]
    )
    review = receipt.get("userReview")
    _require(review in ("APPROVED", "PENDING"), "restoration userReview is unsupported")
    return {
        "status": review if exact else "STALE_REVIEW_RECEIPT",
        "receiptOccurrenceId": receipt_occurrence_id,
    }


def _build_cohorts(occurrences: list[dict[str, Any]]) -> list[dict[str, Any]]:
    groups: dict[str, dict[str, Any]] = {}
    for occurrence in occurrences:
        program = occurrence["program"]
        layout = occurrence["layout"]
        adapter = occurrence["adapter"]
        cohort_kind: str | None = None
        layout_identity_id: str | None = None
        if (
            program["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED"
            and layout["status"] == "TYPED_PACKET_CLOSED"
            and adapter["status"] == "TYPED_STATIC_DISPATCH_CANDIDATE"
        ):
            cohort_kind = "TYPED_EXECUTION_COHORT"
            layout_identity_id = layout["layoutCandidateId"]
        elif (
            program["status"] == "DXBC_OCCURRENCE_EXACT"
            and layout["status"] in (
                "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION",
                "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
                "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION",
            )
            and adapter["status"] == "RENDER_PROFILE_STATIC_CANDIDATE"
        ):
            cohort_kind = "NATIVE_EVIDENCE_COHORT"
            layout_identity_id = layout["layoutEvidenceId"]
        if cohort_kind is None:
            occurrence["tupleCohortId"] = None
            continue
        identity_payload = {
            "programCandidateId": program["programCandidateId"],
            "layoutIdentityId": layout_identity_id,
            "adapterCandidateId": adapter["adapterCandidateId"],
        }
        _require(all(identity_payload.values()), "cohort identity is incomplete")
        cohort_id = "cohort." + canonical_sha256(identity_payload)
        occurrence["tupleCohortId"] = cohort_id
        group = groups.setdefault(
            cohort_id,
            {
                "tupleCohortId": cohort_id,
                "kind": cohort_kind,
                **identity_payload,
                "occurrenceIds": [],
                "domains": set(),
                "productConsumedOccurrenceCount": 0,
                "runtimeVerified": False,
                "runtimeDescriptorExpansionEligible": False,
                "structuralDescriptorReuseCandidate": cohort_kind == "TYPED_EXECUTION_COHORT",
            },
        )
        _require(
            group["kind"] == cohort_kind
            and all(group[key] == identity_payload[key] for key in identity_payload),
            "tuple cohort hash collision or kind disagreement",
        )
        group["occurrenceIds"].append(occurrence["occurrenceId"])
        group["domains"].add(occurrence["domain"])
        if occurrence["scopeBits"]["productConsumed"]:
            group["productConsumedOccurrenceCount"] += 1

    rows: list[dict[str, Any]] = []
    for cohort_id in sorted(groups):
        group = groups[cohort_id]
        group["occurrenceIds"].sort()
        group["occurrenceCount"] = len(group["occurrenceIds"])
        group["domains"] = sorted(group["domains"])
        rows.append(group)
    return rows


def _build_blocker_buckets(occurrences: list[dict[str, Any]]) -> list[dict[str, Any]]:
    buckets: dict[str, list[str]] = defaultdict(list)
    for occurrence in occurrences:
        for blocker in occurrence["blockers"]:
            buckets[blocker].append(occurrence["occurrenceId"])
    return [
        {
            "blockerCode": blocker,
            "occurrenceCount": len(buckets[blocker]),
            "sampleOccurrenceIds": sorted(buckets[blocker])[:20],
        }
        for blocker in sorted(buckets)
    ]


def _build_canaries(
    occurrences: list[dict[str, Any]],
    evidence: dict[str, Any],
    composition_summary: dict[str, Any],
    catalog_summary: dict[str, Any],
) -> dict[str, Any]:
    by_document: dict[str, list[dict[str, Any]]] = defaultdict(list)
    by_key: dict[tuple[str, str], dict[str, Any]] = {}
    for occurrence in occurrences:
        by_document[occurrence["_documentName"]].append(occurrence)
        key = (occurrence["effectAssetId"], occurrence["elementId"])
        _require(key not in by_key, "duplicate occurrence identity during canary build")
        by_key[key] = occurrence

    expected_documents = {
        "effect.lancemaster.skill.34110.unified.effect.json": (83, 0),
        "effect.lancemaster.skill.34150.unified.effect.json": (186, 0),
        "effect.artist.skill.31470.unified.effect.json": (0, 17),
    }
    document_rows: list[dict[str, Any]] = []
    g00_projection: Counter[str] = Counter()
    source_denominator = 0
    for document_name, (expected_source, expected_execution) in expected_documents.items():
        rows = by_document.get(document_name, [])
        source_rows = [row for row in rows if row["sourceProfileEnabled"] is True]
        execution_rows = [row for row in rows if row["program"]["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED"]
        _require(len(source_rows) == expected_source, "G00 source row denominator drifted: " + document_name)
        _require(len(execution_rows) == expected_execution, "G00 execution control denominator drifted: " + document_name)
        source_denominator += len(source_rows)
        for row in source_rows:
            program_evidence = row["_sourceProgramEvidence"]
            parent = program_evidence.get("effectiveParentMaterialPath")
            _require(parent in evidence["g00"], "G00 secondary projection has no family: " + str(parent))
            named = row["layout"]["status"].startswith("NAMED_NATIVE_WIRE_ONLY")
            if program_evidence.get("cookedStatus") == "EXTRACTED":
                permutation_match = program_evidence.get("permutationSelection") == "SINGLE_PERMUTATION_FAMILY" or (
                    program_evidence.get("permutationSelection") == "CHILD_MIC_ENGINE_EQUALITY"
                    and bool(program_evidence.get("sourceMaterialPath"))
                    and program_evidence.get("sourceMaterialPath") == program_evidence.get("selectedChildMaterialPath")
                )
                # G00 is deliberately cross-checked with its sealed #166
                # sourceRecipe.rendererShape carrier rule, not this B3
                # inventory's stronger TypeData/standalone rule.
                carrier_match = bool(row.get("sourceRendererShape")) and row.get("sourceRendererShape") == program_evidence.get("cookedCarrier")
                exact = permutation_match and carrier_match
                status = (
                    ("PROGRAM_EXACT_" if exact else "PROGRAM_PERMUTATION_PENDING_")
                    + ("NAMED_MAPPING_ONLY" if named else "NAMED_MAPPING_MISSING")
                )
            elif program_evidence.get("cookedStatus") == "BLOCKED" and program_evidence.get("shaderMapFamilyId"):
                status = "SHADERMAP_FOUND_DXBC_MISSING"
            elif parent:
                status = "PARENT_ONLY"
            else:
                status = "UNKNOWN"
            g00_projection[status] += 1
        document_rows.append(
            {
                "document": document_name,
                "sourceRowCount": len(source_rows),
                "executionRowCount": len(execution_rows),
            }
        )
    g00_expected = evidence["g00Document"].get("summary", {}).get("statusOccurrenceCounts")
    _require(isinstance(g00_expected, dict), "G00 status occurrence summary is absent")
    _require(source_denominator == 269, "G00 secondary source denominator must be 269")
    normalized_g00_projection = {key: g00_projection[key] for key in g00_expected}
    _require(set(g00_projection).issubset(g00_expected) and normalized_g00_projection == g00_expected, "G00 secondary status projection drifted")

    artist_asset = "effect.artist.skill.31470.unified"
    artist_stable_ids = ("sprite.2b3dc6842507e910", "sprite.c65181324417a1a8")
    artist_rows = [by_key.get((artist_asset, stable_id)) for stable_id in artist_stable_ids]
    _require(all(row is not None for row in artist_rows), "Artist F horizontal golden stable IDs are absent")
    artist_tuple = {
        (
            row["program"]["programCandidateId"],
            row["layout"]["layoutCandidateId"],
            row["adapter"]["adapterCandidateId"],
        )
        for row in artist_rows
        if row is not None
    }
    _require(len(artist_tuple) == 1 and None not in next(iter(artist_tuple)), "Artist F horizontal golden does not dual-resolve one tuple")

    product_snapshot = {
        "characterAssetCount": composition_summary["characterAssetCount"],
        "valtanPatternAssetCount": composition_summary["valtanPatternAssetCount"],
        "valtanBossVisualAssetCount": composition_summary["valtanBossVisualAssetCount"],
        "productConsumedAssetCount": catalog_summary["productConsumedAssetCount"],
        "runtimePublishedAssetCount": catalog_summary["runtimePublishedAssetCount"],
        "runtimePublishedOccurrenceCount": catalog_summary["runtimePublishedOccurrenceCount"],
        "runtimePublishedAuthoredOccurrenceCount": catalog_summary[
            "runtimePublishedAuthoredOccurrenceCount"
        ],
        "runtimeAssetSetEqualsProductConsumerSet": catalog_summary["runtimeAssetSetEqualsProductConsumerSet"],
        "compositionProjectionSha256": composition_summary["compositionProjectionSha256"],
    }
    snapshot_expected = (
        product_snapshot["characterAssetCount"] == EXPECTED_CHARACTER_PRODUCT_ASSET_COUNT
        and product_snapshot["valtanPatternAssetCount"] == EXPECTED_VALTAN_PATTERN_ASSET_COUNT
        and product_snapshot["valtanBossVisualAssetCount"] == EXPECTED_VALTAN_BOSS_VISUAL_ASSET_COUNT
        and product_snapshot["productConsumedAssetCount"] == EXPECTED_RUNTIME_PRODUCT_ASSET_COUNT
        and product_snapshot["runtimePublishedAssetCount"] == EXPECTED_RUNTIME_PRODUCT_ASSET_COUNT
        and product_snapshot["runtimePublishedOccurrenceCount"] == EXPECTED_RUNTIME_PRODUCT_OCCURRENCE_COUNT
        and product_snapshot["runtimePublishedAuthoredOccurrenceCount"]
        == EXPECTED_RUNTIME_PRODUCT_OCCURRENCE_COUNT
        and product_snapshot["runtimeAssetSetEqualsProductConsumerSet"] is True
        and product_snapshot["compositionProjectionSha256"]
        == EXPECTED_COMPOSITION_PROJECTION_SHA256
    )
    return {
        "g00OccurrenceSafety": {
            "status": "MATCHES_SEALED_SECONDARY_PROJECTION",
            "sourceOccurrenceCount": source_denominator,
            "statusOccurrenceCounts": dict(sorted(normalized_g00_projection.items())),
            "documents": document_rows,
        },
        "artistFHorizontalGolden": {
            "status": "STRUCTURAL_DUAL_RESOLVE_PENDING_USER_A_B",
            "effectAssetId": artist_asset,
            "stableIds": list(artist_stable_ids),
            "tupleCohortId": artist_rows[0]["tupleCohortId"],
            "runtimeVerified": False,
            "horizontalV1Review": "PENDING_USER_A_B",
            "legacyGoldenReviews": [row["legacyGoldenReview"] for row in artist_rows],
        },
        "productConsumerRuntimeSnapshot": {
            "status": "MATCHES_CURRENT_REPOSITORY_SNAPSHOT" if snapshot_expected else "DRIFTED_FROM_CURRENT_REGRESSION_SNAPSHOT",
            **product_snapshot,
        },
    }


def _validate_registry(rows: Any, id_field: str, prefix: str) -> set[str]:
    _require(isinstance(rows, list), id_field + " registry must be an array")
    identifiers: set[str] = set()
    for row in rows:
        _require(isinstance(row, dict), id_field + " registry row must be an object")
        identifier = _require_string(row.get(id_field), id_field)
        _require(identifier not in identifiers, "duplicate " + id_field)
        identifiers.add(identifier)
        payload = dict(row)
        payload.pop(id_field)
        _require(identifier == prefix + canonical_sha256(payload), id_field + " content identity drifted")
    return identifiers


def _validate_typed_registry_projection(
    program: dict[str, Any],
    layout: dict[str, Any],
    descriptor: dict[str, Any],
    adapter: dict[str, Any],
) -> None:
    """Round-trip a split typed tuple through the authoritative codec mirror."""
    layout_lanes = layout.get("textureLaneTopology")
    descriptor_lanes = descriptor.get("textureLanes")
    _require(isinstance(layout_lanes, list) and isinstance(descriptor_lanes, list), "typed tuple texture lanes are malformed")
    descriptor_by_lane: dict[str, dict[str, Any]] = {}
    for lane in descriptor_lanes:
        _require(isinstance(lane, dict), "typed Descriptor lane must be an object")
        lane_id = lane.get("laneId")
        _require(isinstance(lane_id, str) and lane_id not in descriptor_by_lane, "typed Descriptor lane identity is duplicate")
        descriptor_by_lane[lane_id] = lane
    combined_lanes: list[dict[str, Any]] = []
    for layout_lane in sorted(layout_lanes, key=lambda value: value.get("textureRegister", -1)):
        _require(isinstance(layout_lane, dict), "typed Layout lane must be an object")
        lane_id = layout_lane.get("laneId")
        _require(lane_id in descriptor_by_lane, "typed Layout lane has no Descriptor value")
        combined = dict(layout_lane)
        combined.update(descriptor_by_lane[lane_id])
        combined_lanes.append(combined)
    mask_topology = layout.get("maskTopology")
    _require(isinstance(mask_topology, dict), "typed Layout mask topology is malformed")
    reconstructed = {
        "enabled": True,
        "version": program.get("version"),
        "backend": program.get("backend"),
        "opcode": program.get("opcode"),
        "passIndex": adapter.get("passIndex"),
        "renderState": adapter.get("renderState"),
        "textureLaneCount": layout.get("textureLaneCount"),
        "textureMask": layout.get("textureMask"),
        "textureLanes": combined_lanes,
        "scalarCount": layout.get("scalarCount"),
        "vectorCount": layout.get("vectorCount"),
        "scalars": descriptor.get("scalars"),
        "vectors": descriptor.get("vectors"),
        "artistParameters": descriptor.get("artistParameters"),
        "colors": descriptor.get("colors"),
        **mask_topology,
    }
    rebuilt_program, rebuilt_layout, rebuilt_descriptor, rebuilt_adapter = _typed_material_axes(
        reconstructed
    )
    program_payload = dict(program)
    program_payload.pop("programCandidateId", None)
    layout_payload = dict(layout)
    layout_payload.pop("layoutCandidateId", None)
    descriptor_payload = dict(descriptor)
    descriptor_payload.pop("descriptorVariantId", None)
    descriptor_payload.pop("layoutCandidateId", None)
    adapter_payload = {
        key: adapter.get(key)
        for key in ("backend", "opcode", "passIndex", "renderState")
    }
    _require(rebuilt_program == program_payload, "typed Program registry row does not round-trip through the codec")
    _require(rebuilt_layout == layout_payload, "typed Layout registry row does not round-trip through the codec")
    _require(rebuilt_descriptor == descriptor_payload, "typed Descriptor registry row does not round-trip through the codec")
    _require(rebuilt_adapter == adapter_payload, "typed Adapter registry row does not round-trip through the codec")


def _validate_exact_native_binding_wire(wire: Any) -> None:
    _require(
        isinstance(wire, dict)
        and set(wire) == {
            "status", "shaderObject", "bindingSemanticSha256", "scalarGroups", "vectors",
            "textures", "constantBufferClosure", "textureSampleClosure",
            "dxbcDeclarationClosure", "wireEntryFormat",
        }
        and wire.get("status") == "EXACT_NATIVE_SHADER_OBJECT_BINDING"
        and wire.get("wireEntryFormat")
        == "<u32 expressionIndexOrPackedScalarGroup,u16 baseByteOrResourceIndex,u16 numBytesOrResources,u16 bufferOrSamplerIndex>",
        "exact native binding wire identity is malformed or admitted",
    )
    _require_sha256(wire.get("bindingSemanticSha256"), "exact native wire bindingSemanticSha256")
    shader_object = wire.get("shaderObject")
    _require(
        isinstance(shader_object, dict)
        and set(shader_object) == {
            "shaderObjectIndex", "shaderType", "shaderTypeNameNumber", "shaderIdHex",
            "logicalOffset", "logicalEndOffset", "byteSize", "rawSha256",
            "serializedShaderCodeSha1Hex", "serializedShaderCodeSha1Sha256",
        }
        and all(
            isinstance(shader_object.get(key), int)
            and not isinstance(shader_object[key], bool)
            and shader_object[key] >= 0
            for key in (
                "shaderObjectIndex", "shaderTypeNameNumber", "logicalOffset",
                "logicalEndOffset", "byteSize",
            )
        )
        and isinstance(shader_object.get("shaderType"), str)
        and bool(shader_object["shaderType"])
        and isinstance(shader_object.get("shaderIdHex"), str)
        and re.fullmatch(r"[0-9a-f]{32}", shader_object["shaderIdHex"]) is not None
        and isinstance(shader_object.get("serializedShaderCodeSha1Hex"), str)
        and re.fullmatch(r"[0-9a-f]{40}", shader_object["serializedShaderCodeSha1Hex"])
        is not None,
        "exact native wire shader object is malformed or has hidden fields",
    )
    _require_sha256(shader_object.get("rawSha256"), "exact native shader object rawSha256")
    _require_sha256(
        shader_object.get("serializedShaderCodeSha1Sha256"),
        "exact native shader object serialized SHA1 receipt",
    )
    _require(
        shader_object["logicalEndOffset"] >= shader_object["logicalOffset"]
        and shader_object["byteSize"] == shader_object["logicalEndOffset"] - shader_object["logicalOffset"],
        "exact native shader object byte range is inconsistent",
    )

    wire_row_fields = {
        "expressionIndexOrGroup", "baseIndex", "numBytesOrResources",
        "bufferIndexOrSamplerIndex",
    }
    for lane_kind in ("scalarGroups", "vectors", "textures"):
        lanes = wire.get(lane_kind)
        _require(isinstance(lanes, list), "exact native wire " + lane_kind + " must be an array")
        for lane in lanes:
            _require(
                isinstance(lane, dict)
                and set(lane) == wire_row_fields
                and all(
                    isinstance(lane.get(key), int)
                    and not isinstance(lane[key], bool)
                    and lane[key] >= 0
                    for key in wire_row_fields
                )
                and lane["numBytesOrResources"] > 0,
                "exact native wire " + lane_kind + " row is malformed or has hidden fields",
            )

    constant_closure = wire.get("constantBufferClosure")
    _require(
        isinstance(constant_closure, dict)
        and set(constant_closure) == {
            "declaredConstantBuffer0Float4Count", "maximumNativeBoundConstantBuffer0Slot",
            "boundConstantBuffer0Slots",
        }
        and isinstance(constant_closure.get("declaredConstantBuffer0Float4Count"), int)
        and not isinstance(constant_closure["declaredConstantBuffer0Float4Count"], bool)
        and constant_closure["declaredConstantBuffer0Float4Count"] > 0
        and isinstance(constant_closure.get("maximumNativeBoundConstantBuffer0Slot"), int)
        and not isinstance(constant_closure["maximumNativeBoundConstantBuffer0Slot"], bool)
        and isinstance(constant_closure.get("boundConstantBuffer0Slots"), list)
        and all(
            isinstance(value, int) and not isinstance(value, bool) and value >= 0
            for value in constant_closure["boundConstantBuffer0Slots"]
        )
        and constant_closure["boundConstantBuffer0Slots"]
        == sorted(set(constant_closure["boundConstantBuffer0Slots"])),
        "exact native constant-buffer closure is malformed or has hidden fields",
    )

    def validate_sample_counts(value: Any, label: str) -> None:
        _require(isinstance(value, dict), label + " must be an object")
        for pair, count in value.items():
            _require(
                isinstance(pair, str)
                and re.fullmatch(r"t\d+/s\d+", pair) is not None
                and isinstance(count, int)
                and not isinstance(count, bool)
                and count > 0,
                label + " contains a malformed sample pair",
            )

    sample_closure = wire.get("textureSampleClosure")
    _require(
        isinstance(sample_closure, dict)
        and set(sample_closure) == {
            "materialSamplePairs", "unownedEngineSamplePairs", "allObservedSamplePairCounts"
        }
        and all(
            isinstance(sample_closure.get(key), list)
            and sample_closure[key] == sorted(set(sample_closure[key]))
            and all(isinstance(value, str) and re.fullmatch(r"t\d+/s\d+", value) for value in sample_closure[key])
            for key in ("materialSamplePairs", "unownedEngineSamplePairs")
        ),
        "exact native texture-sample closure is malformed or has hidden fields",
    )
    validate_sample_counts(sample_closure["allObservedSamplePairCounts"], "exact native sample counts")
    _require(
        set(sample_closure["materialSamplePairs"]).isdisjoint(sample_closure["unownedEngineSamplePairs"])
        and set(sample_closure["materialSamplePairs"] + sample_closure["unownedEngineSamplePairs"])
        == set(sample_closure["allObservedSamplePairCounts"]),
        "exact native texture-sample ownership is incomplete",
    )

    declaration = wire.get("dxbcDeclarationClosure")
    _require(
        isinstance(declaration, dict)
        and set(declaration) == {
            "profile", "normalizedDisassemblySha256", "declarationSha256",
            "instructionSha256", "instructionCount", "declaredConstantBuffer0Float4Count",
            "declaredTextureRegisters", "declaredSamplerRegisters", "observedSamplePairCounts",
        }
        and isinstance(declaration.get("profile"), str)
        and bool(declaration["profile"])
        and all(
            isinstance(declaration.get(key), int)
            and not isinstance(declaration[key], bool)
            and declaration[key] >= 0
            for key in ("instructionCount", "declaredConstantBuffer0Float4Count")
        )
        and all(
            isinstance(declaration.get(key), list)
            and declaration[key] == sorted(set(declaration[key]))
            and all(isinstance(value, int) and not isinstance(value, bool) and value >= 0 for value in declaration[key])
            for key in ("declaredTextureRegisters", "declaredSamplerRegisters")
        ),
        "exact native DXBC declaration closure is malformed or has hidden fields",
    )
    for key in ("normalizedDisassemblySha256", "declarationSha256", "instructionSha256"):
        _require_sha256(declaration.get(key), "exact native declaration " + key)
    validate_sample_counts(declaration["observedSamplePairCounts"], "exact declaration sample counts")
    _require(
        declaration["observedSamplePairCounts"] == sample_closure["allObservedSamplePairCounts"]
        and declaration["declaredConstantBuffer0Float4Count"]
        == constant_closure["declaredConstantBuffer0Float4Count"],
        "exact native declaration and binding closures disagree",
    )


def validate_inventory(document: dict[str, Any]) -> None:
    expected_top_level_fields = {
        "schema", "formatVersion", "identity", "inputs", "policies", "summary",
        "carrierVariants", "programCandidates", "programEvidence", "layoutCandidates",
        "layoutEvidence", "adapterCandidates", "descriptorVariants", "compositionVariants",
        "occurrences", "cohorts", "blockerBuckets", "canaries", "transaction",
        "artifactSha256",
    }
    _require(set(document) == expected_top_level_fields, "inventory root has missing, hidden, or admission fields")
    _require(document.get("schema") == SCHEMA and document.get("formatVersion") == FORMAT_VERSION, "inventory identity is unsupported")
    digest = _require_sha256(document.get("artifactSha256"), "inventory artifactSha256")
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    _require(canonical_sha256(unsigned) == digest, "inventory artifactSha256 drifted")
    _require(document.get("identity") == "STATIC_EVIDENCE_CANDIDATES_ONLY_NO_RUNTIME_OR_PRODUCT_ADMISSION", "inventory overstates its identity")
    input_rows = document.get("inputs")
    _require(isinstance(input_rows, list), "inventory inputs must be an array")
    known_input_roles = {
        "TARGET_AUTHORED_DOCUMENT", "COOKED_DXBC", "LITERAL_HLSL_TRANSLATION",
        "PUBLISHED_EFFECT_DOCUMENT", "EXACT_VARIANT_DXBC", "CHARACTER_EFFECT_CUES",
        "CHARACTER_SKILL_BINDINGS", "EXACT_VARIANT_PINNED_INPUT", "RUNTIME_EFFECT_CATALOG",
        "VALTAN_BOSS_VISUAL_BINDINGS", "VALTAN_PATTERN_BINDINGS",
        "VALTAN_PATTERN_EFFECT_CUES", "VALTAN_CLIP_DURATION", "PLAYER_SKILL_IDENTITY",
        "USER_REVIEW_RECEIPT", "CHILD_PARENT_RESOLUTION", "COOKED_PIXEL_SHADERS",
        "HLSL_TRANSLATION_INDEX", "NAMED_ABI", "G00_CANARY", "SHADER_MAP_INDEX",
        "EXACT_COOKED_VARIANT_EVIDENCE", "AUTHORING_EFFECT_CATALOG",
        "VALTAN_COMBAT_OBJECT_OWNERS", "VALTAN_STAGE_TIMING", "EXACT_VARIANT_GENERATOR",
    }
    for input_row in input_rows:
        _require(
            isinstance(input_row, dict)
            and set(input_row) == {"path", "roles", "rawSha256", "byteSize", "canonicalJsonSha256"}
            and isinstance(input_row.get("roles"), list)
            and input_row["roles"] == sorted(set(input_row["roles"])),
            "inventory input identity has missing or hidden fields",
        )
        input_path = _safe_relative_path(input_row.get("path"), "inventory input path")
        role_set = set(input_row["roles"])
        _require(
            role_set.issubset(known_input_roles)
            and (
                len(role_set) == 1
                or role_set == {"COOKED_DXBC", "EXACT_VARIANT_DXBC"}
            ),
            "inventory input has an unknown or impossible role combination",
        )
        if "TARGET_AUTHORED_DOCUMENT" in role_set:
            _require(
                input_path.startswith(AUTHORED_DIRECTORY.as_posix() + "/")
                and input_path.endswith(".effect.json"),
                "target authored input role is attached to the wrong path",
            )
        if role_set.intersection({"COOKED_DXBC", "EXACT_VARIANT_DXBC"}):
            _require(
                input_path.startswith(COOKED_SHADER_DIRECTORY.as_posix() + "/")
                and input_path.endswith(".dxbc"),
                "DXBC input role is attached to the wrong path",
            )
        if "LITERAL_HLSL_TRANSLATION" in role_set:
            _require(
                input_path.startswith(TRANSLATED_SHADER_DIRECTORY.as_posix() + "/")
                and input_path.endswith(".hlsli"),
                "HLSLI input role is attached to the wrong path",
            )
        if "PUBLISHED_EFFECT_DOCUMENT" in role_set:
            _require(
                input_path.startswith("Client/Bin/DataFiles/Effect/Authored/")
                and input_path.endswith(".effect.json"),
                "published Effect input role is attached to the wrong path",
            )
    published_input_paths = {
        row["path"]
        for row in input_rows
        if "PUBLISHED_EFFECT_DOCUMENT" in row["roles"]
    }
    input_rows_by_path = {row["path"]: row for row in input_rows}

    registry_specs = (
        ("carrierVariants", "carrierVariantId", "carrier."),
        ("programCandidates", "programCandidateId", "program."),
        ("programEvidence", "programEvidenceId", "program-evidence."),
        ("layoutCandidates", "layoutCandidateId", "layout."),
        ("layoutEvidence", "layoutEvidenceId", "layout-evidence."),
        ("adapterCandidates", "adapterCandidateId", "adapter."),
        ("descriptorVariants", "descriptorVariantId", "descriptor."),
    )
    registry_ids = {
        name: _validate_registry(document.get(name), id_field, prefix)
        for name, id_field, prefix in registry_specs
    }
    registry_id_fields = {name: id_field for name, id_field, _ in registry_specs}
    registry_rows = {
        name: {row[registry_id_fields[name]]: row for row in document[name]}
        for name in registry_ids
    }
    registry_kind_contracts = {
        "programCandidates": {"TYPED_RUNTIME_PROGRAM", "DXBC_LITERAL_TRANSLATION"},
        "programEvidence": {
            "TYPED_EXECUTION_RECEIPT",
            "SOURCE_FAMILY_PROGRAM_EVIDENCE",
            "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE",
        },
        "layoutCandidates": {"TYPED_RUNTIME_PACKET"},
        "layoutEvidence": {
            "NAMED_NATIVE_WIRE_EVIDENCE",
            "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE",
            "SOURCE_PARAMETER_NAMES_ONLY",
        },
        "adapterCandidates": {"TYPED_STATIC_DISPATCH", "RENDER_PROFILE_STATIC_DISPATCH"},
        "descriptorVariants": {"TYPED_RUNTIME_VALUES", "SOURCE_VALUES_UNPACKED", "RESOURCE_BINDINGS_ONLY"},
    }
    for registry_name, kinds in registry_kind_contracts.items():
        for registry_row in document[registry_name]:
            _require(registry_row.get("kind") in kinds, registry_name + " contains an unknown kind")
    exact_registry_fields = {
        "carrierVariants": {
            None: {"carrierVariantId", "coarseCarrier", "authoredKind", "fineRendererKind", "vertexFactoryCandidates", "sourceRendererShape", "meshModelAssets"},
        },
        "programCandidates": {
            "TYPED_RUNTIME_PROGRAM": {"programCandidateId", "kind", "version", "backend", "opcode"},
            "DXBC_LITERAL_TRANSLATION": {"programCandidateId", "kind", "dxbcSha256", "hlslSha256", "functionName"},
        },
        "programEvidence": {
            "TYPED_EXECUTION_RECEIPT": {"programEvidenceId", "kind", "programCandidateId"},
            "SOURCE_FAMILY_PROGRAM_EVIDENCE": {"programEvidenceId", "kind", "authoredParentMaterialPath", "sourceMaterialPath", "effectiveParentMaterialPath", "parentResolution", "childParentRowSha256", "childParentBlocker", "shaderMapFamilyId", "cookedStatus", "cookedDxbcSha256", "permutationSelection", "selectedChildMaterialPath", "sourceCarrier", "cookedCarrier", "occurrenceExact"},
            "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE": {"programEvidenceId", "kind", "variantId", "variantKeySha256", "sourceMaterialPath", "effectiveParentMaterialPath", "parentResolution", "childParentRowSha256", "childParentBlocker", "expectedFineRendererKind", "occurrenceFineRendererKind", "rendererMatch", "dxbcSha256", "shaderProfile", "literalTranslationAvailable", "exactPixelShaderBlob", "exactNativeBindingWire", "actualVfPass", "runtimeAdmission", "visualAdmission", "occurrenceExact"},
        },
        "layoutCandidates": {
            "TYPED_RUNTIME_PACKET": {"layoutCandidateId", "kind", "version", "backend", "textureLaneCount", "textureMask", "textureLaneTopology", "scalarCount", "scalarPackedIndices", "vectorCount", "vectorPackedIndices", "maskTopology"},
        },
        "layoutEvidence": {
            "NAMED_NATIVE_WIRE_EVIDENCE": {"layoutEvidenceId", "kind", "parentMaterialPath", "dxbcSha256", "bindingSemanticSha256", "counts", "textureSlots", "scalarLanes", "vectorLanes", "withinCountCaps", "runtimePacketTopologyMaterialized", "admits"},
            "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE": {"layoutEvidenceId", "kind", "variantId", "variantKeySha256", "dxbcSha256", "bindingSemanticSha256", "nativeBindingWireSha256", "nativeBindingWire", "nativeWireCounts", "runtimePacketTopologyMaterialized", "admits", "sourceExactNativeScalarGroupPacking", "sourceExactSampler", "sourceValueReplay", "actualVfPass", "runtimeAdmission", "visualAdmission"},
            "SOURCE_PARAMETER_NAMES_ONLY": {"layoutEvidenceId", "kind", "parentMaterialPath", "textureNames", "scalarNames", "vectorNames", "dynamicParameterSemantics"},
        },
        "adapterCandidates": {
            "TYPED_STATIC_DISPATCH": {"adapterCandidateId", "kind", "coarseCarrier", "authoredKind", "fineRendererKind", "vertexFactoryCandidates", "sourceRendererShape", "templateId", "renderProfile", "backend", "opcode", "passIndex", "renderState", "runtimeVerified"},
            "RENDER_PROFILE_STATIC_DISPATCH": {"adapterCandidateId", "kind", "coarseCarrier", "authoredKind", "fineRendererKind", "vertexFactoryCandidates", "sourceRendererShape", "templateId", "renderProfile", "runtimeVerified"},
        },
        "descriptorVariants": {
            "TYPED_RUNTIME_VALUES": {"descriptorVariantId", "kind", "layoutCandidateId", "textureLanes", "scalars", "vectors", "artistParameters", "colors"},
            "SOURCE_VALUES_UNPACKED": {"descriptorVariantId", "kind", "textures", "scalars", "vectors", "staticSwitches", "dynamicParameterSemantics", "resourceBindings"},
            "RESOURCE_BINDINGS_ONLY": {"descriptorVariantId", "kind", "resourceBindings"},
        },
    }
    for registry_name, rows_by_id in registry_rows.items():
        schemas = exact_registry_fields[registry_name]
        for registry_row in rows_by_id.values():
            schema_key = registry_row.get("kind") if None not in schemas else None
            _require(set(registry_row) == schemas[schema_key], registry_name + " row has missing, hidden, or admission fields")

    for row in document["programCandidates"]:
        if row["kind"] == "TYPED_RUNTIME_PROGRAM":
            _require(row.get("version") == 1 and row.get("backend") in ("runtimeMaterialV2", "artistVisualV4", "localDecal"), "typed Program candidate identity is invalid")
        else:
            dxbc_sha = _require_sha256(row.get("dxbcSha256"), "DXBC Program candidate digest")
            hlsl_sha = _require_sha256(row.get("hlslSha256"), "HLSL Program candidate digest")
            function_name = _require_string(row.get("functionName"), "HLSL Program candidate functionName")
            hlsli_path = (TRANSLATED_SHADER_DIRECTORY / (function_name + ".hlsli")).as_posix()
            dxbc_path = (COOKED_SHADER_DIRECTORY / (dxbc_sha + ".dxbc")).as_posix()
            hlsli_input = input_rows_by_path.get(hlsli_path)
            dxbc_input = input_rows_by_path.get(dxbc_path)
            _require(
                isinstance(hlsli_input, dict)
                and "LITERAL_HLSL_TRANSLATION" in hlsli_input["roles"]
                and hlsli_input["rawSha256"] == hlsl_sha,
                "literal Program candidate has no tracked translated HLSLI",
            )
            _require(
                isinstance(dxbc_input, dict)
                and set(dxbc_input["roles"]).intersection({"COOKED_DXBC", "EXACT_VARIANT_DXBC"})
                and dxbc_input["rawSha256"] == dxbc_sha,
                "literal Program candidate has no tracked exact DXBC blob",
            )
    for row in document["programEvidence"]:
        if row["kind"] == "TYPED_EXECUTION_RECEIPT":
            candidate = registry_rows["programCandidates"].get(row.get("programCandidateId"))
            _require(isinstance(candidate, dict) and candidate.get("kind") == "TYPED_RUNTIME_PROGRAM", "typed Program receipt references a non-typed candidate")
        elif row["kind"] == "SOURCE_FAMILY_PROGRAM_EVIDENCE":
            _require(isinstance(row.get("occurrenceExact"), bool), "source Program evidence occurrenceExact is malformed")
            cooked_status = row.get("cookedStatus")
            _require(
                cooked_status in ("ABSENT", "BLOCKED", "EXTRACTED"),
                "source Program evidence cookedStatus is invalid",
            )
            if cooked_status == "EXTRACTED":
                _require_sha256(
                    row.get("cookedDxbcSha256"),
                    "extracted source Program evidence cooked DXBC digest",
                )
            else:
                _require(
                    row.get("cookedDxbcSha256") is None,
                    "non-extracted source Program evidence retains a cooked DXBC digest",
                )
        else:
            _require(
                isinstance(row.get("rendererMatch"), bool)
                and row.get("occurrenceExact")
                is (
                    row.get("rendererMatch") is True
                    and row.get("parentResolution") != "CHILD_PARENT_BLOCKED"
                )
                and row.get("exactPixelShaderBlob") is True
                and row.get("exactNativeBindingWire") is True
                and row.get("actualVfPass") is False
                and row.get("runtimeAdmission") is False
                and row.get("visualAdmission") is False
                and isinstance(row.get("literalTranslationAvailable"), bool)
                and row.get("expectedFineRendererKind") in ("SPRITE_PARTICLE", "MESH_PARTICLE")
                and isinstance(row.get("occurrenceFineRendererKind"), str)
                and (row["expectedFineRendererKind"] == row["occurrenceFineRendererKind"])
                is row["rendererMatch"],
                "exact cooked variant Program evidence overstates fidelity or admission",
            )
            _require_sha256(row.get("variantKeySha256"), "exact Program variantKeySha256")
            _require_sha256(row.get("dxbcSha256"), "exact Program dxbcSha256")
        if row["kind"] != "TYPED_EXECUTION_RECEIPT":
            child_blocked = row.get("parentResolution") == "CHILD_PARENT_BLOCKED"
            _require(
                child_blocked
                == (
                    row.get("childParentBlocker") is not None
                    and isinstance(row.get("childParentRowSha256"), str)
                ),
                "Program evidence child-parent blocker receipt is incomplete",
            )
            if child_blocked:
                _require_sha256(row.get("childParentRowSha256"), "blocked child-parent rowSha256")
                blocker = row.get("childParentBlocker")
                _require(
                    isinstance(blocker, (str, dict)) and bool(blocker),
                    "Program evidence lost its sealed child-parent blocker",
                )
    for row in document["layoutEvidence"]:
        if row["kind"] == "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE":
            wire = row.get("nativeBindingWire")
            counts = row.get("nativeWireCounts")
            _require_sha256(row.get("variantKeySha256"), "exact Layout variantKeySha256")
            _require_sha256(row.get("dxbcSha256"), "exact Layout dxbcSha256")
            _require_sha256(row.get("bindingSemanticSha256"), "exact Layout bindingSemanticSha256")
            _require_sha256(row.get("nativeBindingWireSha256"), "exact Layout nativeBindingWireSha256")
            _validate_exact_native_binding_wire(wire)
            _require(
                isinstance(wire, dict)
                and set(wire) == {
                    "status", "shaderObject", "bindingSemanticSha256", "scalarGroups",
                    "vectors", "textures", "constantBufferClosure", "textureSampleClosure",
                    "dxbcDeclarationClosure", "wireEntryFormat",
                }
                and isinstance(counts, dict)
                and set(counts) == {"scalarGroupCount", "vectorCount", "textureCount"}
                and counts["scalarGroupCount"] == len(wire.get("scalarGroups", []))
                and counts["vectorCount"] == len(wire.get("vectors", []))
                and counts["textureCount"] == len(wire.get("textures", []))
                and wire.get("bindingSemanticSha256") == row.get("bindingSemanticSha256")
                and canonical_sha256(wire) == row.get("nativeBindingWireSha256")
                and row.get("runtimePacketTopologyMaterialized") is False
                and row.get("admits") == "EXACT_NATIVE_WIRE_IDENTITY_ONLY"
                and row.get("sourceExactNativeScalarGroupPacking") is False
                and row.get("sourceExactSampler") is False
                and row.get("sourceValueReplay") is False
                and row.get("actualVfPass") is False
                and row.get("runtimeAdmission") is False
                and row.get("visualAdmission") is False,
                "exact variant native wire evidence was materialized or admitted",
            )
            continue
        if row["kind"] != "NAMED_NATIVE_WIRE_EVIDENCE":
            continue
        counts = row.get("counts")
        _require(isinstance(counts, dict) and set(counts) == {"textureSlotCount", "scalarLaneCount", "vectorLaneCount"}, "named Layout counts are malformed")
        _require(
            isinstance(row.get("textureSlots"), list)
            and isinstance(row.get("scalarLanes"), list)
            and isinstance(row.get("vectorLanes"), list)
            and counts["textureSlotCount"] == len(row["textureSlots"])
            and counts["scalarLaneCount"] == len(row["scalarLanes"])
            and counts["vectorLaneCount"] == len(row["vectorLanes"]),
            "named Layout count projection drifted",
        )
        within_caps = (
            counts["textureSlotCount"] <= MAX_TEXTURE_LANES
            and counts["scalarLaneCount"] <= MAX_PACKED_SCALARS
            and counts["vectorLaneCount"] <= MAX_PACKED_VECTORS
        )
        _require(
            row.get("withinCountCaps") is within_caps
            and row.get("runtimePacketTopologyMaterialized") is False
            and row.get("admits") == "NAMED_LANE_IDENTITY_ONLY",
            "named Layout evidence was promoted to a typed runtime packet",
        )
    for row in document["descriptorVariants"]:
        if row["kind"] == "TYPED_RUNTIME_VALUES":
            _require(row.get("layoutCandidateId") in registry_ids["layoutCandidates"], "typed Descriptor references an unknown Layout")
    for row in document.get("adapterCandidates", []):
        _require(row.get("runtimeVerified") is False, "adapter candidate became runtime verified")

    composition_ids: set[str] = set()
    composition_rows_by_id: dict[str, dict[str, Any]] = {}
    for row in document.get("compositionVariants", []):
        _require(
            isinstance(row, dict)
            and set(row) == {"compositionVariantId", "semantic", "evidenceLocations"},
            "composition variant has missing, hidden, or admission fields",
        )
        identifier = _require_string(row.get("compositionVariantId"), "compositionVariantId")
        _require(identifier not in composition_ids, "duplicate compositionVariantId")
        composition_ids.add(identifier)
        composition_rows_by_id[identifier] = row
        _require(identifier == "composition." + canonical_sha256(row.get("semantic")), "composition semantic identity drifted")
        semantic = row.get("semantic")
        _require(isinstance(semantic, dict), "composition semantic must be an object")
        semantic_kind = semantic.get("kind")
        character_fields = {
            "kind", "characterClass", "skillId", "clip", "startMs", "effectAssetId",
            "anchor", "follow", "stop", "localTransform",
        }
        semantic_fields = {
            "CHARACTER_ANIMATION_CUE": (character_fields, character_fields | {"orientation"}),
            "VALTAN_PATTERN_CUE": ({
                "kind", "bindingId", "occurrenceId", "patternId", "stageId",
                "stageDurationMs", "actionId", "clipOccurrenceId", "clip", "clipDurationMs",
                "effectAssetId", "anchorSlotId", "followPolicy", "stopPolicy", "repeatPolicy",
                "sourceStartMs", "sourceEndMs", "localTransform",
            },),
            "VALTAN_COMBAT_OBJECT_VISUAL": ({
                "kind", "bossArchetypeId", "combatObjectArchetypeId", "clientVisualId",
                "ownerPatternId", "ownerStageActionId", "effectAssetId",
            },),
        }
        _require(
            semantic_kind in semantic_fields
            and any(set(semantic) == allowed for allowed in semantic_fields[semantic_kind]),
            "composition semantic has missing, hidden, or admission fields",
        )
        if semantic_kind == "CHARACTER_ANIMATION_CUE":
            _require(
                isinstance(semantic.get("skillId"), int)
                and not isinstance(semantic["skillId"], bool)
                and semantic["skillId"] > 0
                and isinstance(semantic.get("startMs"), int)
                and not isinstance(semantic["startMs"], bool)
                and semantic["startMs"] >= 0
                and all(
                    isinstance(semantic.get(key), str) and bool(semantic[key])
                    for key in (
                        "characterClass", "clip", "effectAssetId", "anchor", "follow", "stop"
                    )
                )
                and (
                    "orientation" not in semantic
                    or (isinstance(semantic["orientation"], str) and bool(semantic["orientation"]))
                ),
                "character Composition semantic has malformed identity or timing",
            )
        elif semantic_kind == "VALTAN_PATTERN_CUE":
            string_fields = (
                "bindingId", "occurrenceId", "patternId", "stageId", "actionId",
                "clipOccurrenceId", "clip", "effectAssetId", "anchorSlotId", "followPolicy",
                "stopPolicy", "repeatPolicy",
            )
            source_end_ms = semantic.get("sourceEndMs")
            _require(
                all(isinstance(semantic.get(key), str) and bool(semantic[key]) for key in string_fields)
                and all(
                    isinstance(semantic.get(key), int)
                    and not isinstance(semantic[key], bool)
                    and semantic[key] >= 0
                    for key in ("stageDurationMs", "clipDurationMs", "sourceStartMs")
                )
                and semantic["stageDurationMs"] > 0
                and semantic["clipDurationMs"] > 0
                and semantic["sourceStartMs"] <= semantic["clipDurationMs"]
                and (
                    source_end_ms is None
                    or (
                        isinstance(source_end_ms, int)
                        and not isinstance(source_end_ms, bool)
                        and semantic["sourceStartMs"] <= source_end_ms <= semantic["clipDurationMs"]
                    )
                ),
                "Valtan pattern Composition semantic has malformed identity or timing",
            )
        else:
            _require(
                all(
                    isinstance(value, str) and bool(value)
                    for key, value in semantic.items()
                    if key != "kind"
                ),
                "Valtan combat-object Composition semantic has malformed identity",
            )
        if "localTransform" in semantic:
            transform = semantic["localTransform"]
            _require(
                isinstance(transform, dict)
                and set(transform) == {"position", "rotationDegrees", "scale"}
                and all(
                    isinstance(transform.get(key), list)
                    and len(transform[key]) == 3
                    and all(
                        isinstance(value, (int, float))
                        and not isinstance(value, bool)
                        and math.isfinite(float(value))
                        for value in transform[key]
                    )
                    for key in ("position", "rotationDegrees", "scale")
                ),
                "composition localTransform is malformed",
            )
        evidence_locations = row.get("evidenceLocations")
        _require(
            isinstance(evidence_locations, list)
            and bool(evidence_locations)
            and evidence_locations == sorted(evidence_locations, key=canonical_json_bytes)
            and len({canonical_json_bytes(value) for value in evidence_locations}) == len(evidence_locations),
            "composition evidence locations are missing, duplicate, or unordered",
        )
        for location in evidence_locations:
            _require(isinstance(location, dict), "composition evidence location must be an object")
            if semantic_kind == "CHARACTER_ANIMATION_CUE":
                _require(
                    set(location) == {"path", "lineNumber"}
                    and isinstance(location.get("path"), str)
                    and isinstance(location.get("lineNumber"), int)
                    and not isinstance(location["lineNumber"], bool)
                    and location["lineNumber"] > 0,
                    "character Composition evidence location is malformed",
                )
            else:
                _require(
                    set(location) == {"path", "row"}
                    and isinstance(location.get("path"), str)
                    and isinstance(location.get("row"), str)
                    and bool(location["row"]),
                    "Valtan Composition evidence location is malformed",
                )

    composition_projection_sha = canonical_sha256(document["compositionVariants"])

    occurrences = document.get("occurrences")
    _require(isinstance(occurrences, list), "occurrences must be an array")
    occurrence_ids: set[str] = set()
    occurrence_by_id: dict[str, dict[str, Any]] = {}
    tuple_occurrence_ids: dict[str, list[str]] = defaultdict(list)
    blocker_occurrence_ids: dict[str, list[str]] = defaultdict(list)
    registry_references: dict[str, set[str]] = {name: set() for name in registry_ids}
    composition_references: set[str] = set()
    asset_scope_projection: dict[str, tuple[dict[str, bool], tuple[str, ...], str]] = {}
    catalog_declared_assets: set[str] = set()
    runtime_published_assets: set[str] = set()
    product_consumed_assets: set[str] = set()
    runtime_published_occurrence_count = 0
    published_paths_by_asset: dict[str, str] = {}
    published_assets_by_path: dict[str, str] = {}
    axis_counts = {
        "programStatusCounts": Counter(),
        "layoutStatusCounts": Counter(),
        "adapterStatusCounts": Counter(),
        "descriptorStatusCounts": Counter(),
        "compositionStatusCounts": Counter(),
        "productStatusCounts": Counter(),
        "legacyGoldenReviewStatusCounts": Counter(),
        "horizontalV1ReviewStatusCounts": Counter(),
    }
    for row in occurrences:
        _require(
            set(row) == {
                "occurrenceId", "effectAssetId", "elementId", "elementOrder", "authoredPath",
                "authoredVersion", "authoredElementSha256", "domain", "visible", "kind",
                "sourceTypeDataClasses", "sourceProfileEnabled", "sourceMaterialPath",
                "sourceParentMaterialPath", "sourceParentResolution", "sourceParentRowSha256",
                "sourceParentBlocker", "sourceRendererShape",
                "dxbcSourceCarrier", "fineRendererKind", "vertexFactoryCandidates", "carrier",
                "carrierVariantId", "program", "layout", "adapter", "descriptor", "blockers",
                "compositionStatus", "productStatus", "scopeBits", "compositionVariantIds",
                "publishedDocumentPath", "publishedElementExact", "legacyGoldenReview",
                "horizontalV1Review", "tupleCohortId",
            },
            "occurrence has missing, hidden, or admission fields",
        )
        _require(set(row["program"]) == {"status", "programCandidateId", "programEvidenceId"}, "occurrence Program axis has hidden fields")
        _require(set(row["layout"]) == {"status", "layoutCandidateId", "layoutEvidenceId"}, "occurrence Layout axis has hidden fields")
        _require(set(row["adapter"]) == {"status", "adapterCandidateId", "runtimeVerified"}, "occurrence Adapter axis has hidden fields")
        _require(set(row["descriptor"]) == {"status", "descriptorVariantId"}, "occurrence Descriptor axis has hidden fields")
        _require(set(row["legacyGoldenReview"]) == {"status", "receiptOccurrenceId"}, "occurrence legacy review has hidden fields")
        _require(isinstance(row.get("sourceProfileEnabled"), bool), "occurrence sourceProfileEnabled is malformed")
        _require(
            row["sourceProfileEnabled"]
            == (row["descriptor"]["status"] == "SOURCE_VALUES_PRESENT_UNPACKED"),
            "occurrence sourceProfileEnabled disagrees with Descriptor value evidence",
        )
        source_parent_blocked = row.get("sourceParentResolution") == "CHILD_PARENT_BLOCKED"
        _require(
            row.get("sourceParentResolution") in {
                "NO_PARENT_EVIDENCE", "AUTHORED_PARENT_EXACT", "CHILD_PARENT_BLOCKED",
                "CHILD_PARENT_KNOWN_FAMILY_EXACT", "CHILD_PARENT_NEW_FAMILY_EXACT",
            },
            "occurrence source-parent resolution is unknown",
        )
        _require(
            source_parent_blocked
            == (
                row.get("sourceParentBlocker") is not None
                and isinstance(row.get("sourceParentRowSha256"), str)
            ),
            "occurrence source-parent blocker receipt is incomplete",
        )
        if source_parent_blocked:
            _require_sha256(row.get("sourceParentRowSha256"), "occurrence blocked source-parent rowSha256")
            _require(
                isinstance(row.get("sourceParentBlocker"), (str, dict))
                and bool(row["sourceParentBlocker"])
                and "CHILD_PARENT_RESOLUTION_BLOCKED" in row.get("blockers", []),
                "occurrence source-parent blocker is not fail-closed",
            )
        elif row["sourceParentResolution"] == "NO_PARENT_EVIDENCE":
            _require(
                row.get("sourceParentMaterialPath") is None
                and row.get("sourceParentRowSha256") is None
                and row.get("sourceParentBlocker") is None,
                "no-parent occurrence retains parent evidence",
            )
        elif row["sourceParentResolution"] == "AUTHORED_PARENT_EXACT":
            _require(
                isinstance(row.get("sourceParentMaterialPath"), str)
                and bool(row["sourceParentMaterialPath"])
                and row.get("sourceParentRowSha256") is None
                and row.get("sourceParentBlocker") is None,
                "authored-parent occurrence lacks exact authored evidence",
            )
        else:
            _require(
                isinstance(row.get("sourceParentMaterialPath"), str)
                and bool(row["sourceParentMaterialPath"])
                and isinstance(row.get("sourceParentRowSha256"), str)
                and row.get("sourceParentBlocker") is None,
                "resolved child-parent occurrence has incomplete receipt",
            )
            _require_sha256(row["sourceParentRowSha256"], "resolved child-parent rowSha256")
        occurrence_id = _require_string(row.get("occurrenceId"), "occurrenceId")
        _require(occurrence_id not in occurrence_ids, "duplicate occurrenceId")
        occurrence_ids.add(occurrence_id)
        occurrence_by_id[occurrence_id] = row
        _require(occurrence_id == _occurrence_id(row["effectAssetId"], row["elementId"]), "occurrence identity drifted")
        program = row["program"]
        layout = row["layout"]
        adapter = row["adapter"]
        descriptor = row["descriptor"]
        _require(program["status"] in PROGRAM_STATUSES, "unknown Program status")
        _require(layout["status"] in LAYOUT_STATUSES, "unknown Layout status")
        _require(adapter["status"] in ADAPTER_STATUSES, "unknown Adapter status")
        _require(descriptor["status"] in DESCRIPTOR_STATUSES, "unknown Descriptor status")
        _require(row["compositionStatus"] in COMPOSITION_STATUSES, "unknown Composition status")
        _require(row["productStatus"] in PRODUCT_STATUSES, "unknown Product status")
        typed_axis_flags = (
            program["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED",
            layout["status"] == "TYPED_PACKET_CLOSED",
            descriptor["status"] == "TYPED_VALUES_CLOSED",
            adapter["status"] == "TYPED_STATIC_DISPATCH_CANDIDATE",
        )
        _require(
            all(typed_axis_flags) or not any(typed_axis_flags),
            "typed Program/Layout/Descriptor/Adapter axes are not one coherent tuple",
        )
        _require(adapter.get("runtimeVerified") is False, "occurrence Adapter became runtime verified")
        _require(row.get("carrierVariantId") in registry_ids["carrierVariants"], "occurrence references unknown carrier variant")
        registry_references["carrierVariants"].add(row["carrierVariantId"])
        carrier_variant = registry_rows["carrierVariants"][row["carrierVariantId"]]
        _require(
            carrier_variant["coarseCarrier"] == row.get("carrier")
            and carrier_variant["authoredKind"] == row.get("kind")
            and carrier_variant["fineRendererKind"] == row.get("fineRendererKind"),
            "occurrence carrier discriminator differs from its variant",
        )

        program_candidate = program.get("programCandidateId")
        program_evidence = program.get("programEvidenceId")
        if program_evidence in registry_rows["programEvidence"]:
            source_program_row = registry_rows["programEvidence"][program_evidence]
            if source_program_row["kind"] != "TYPED_EXECUTION_RECEIPT":
                _require(
                    source_program_row.get("sourceMaterialPath") == row.get("sourceMaterialPath")
                    and source_program_row.get("effectiveParentMaterialPath") == row.get("sourceParentMaterialPath")
                    and source_program_row.get("parentResolution") == row.get("sourceParentResolution")
                    and source_program_row.get("childParentRowSha256") == row.get("sourceParentRowSha256")
                    and source_program_row.get("childParentBlocker") == row.get("sourceParentBlocker"),
                    "occurrence source-parent projection differs from Program evidence",
                )
        if program["status"] in ("TYPED_RUNTIME_PROGRAM_DECLARED", "DXBC_OCCURRENCE_EXACT"):
            _require(program_candidate in registry_ids["programCandidates"], "closed Program has no valid candidate")
            _require(program_evidence in registry_ids["programEvidence"], "closed Program has no valid evidence")
        elif program["status"] in (
            "DXBC_OCCURRENCE_EXACT_UNTRANSLATED",
            "DXBC_FAMILY_REPRESENTATIVE_ONLY",
            "BOUNDED_SOURCE_PROFILE_ONLY",
            "NO_PROGRAM_EVIDENCE",
        ):
            _require(program_candidate is None and program_evidence in registry_ids["programEvidence"], "evidence-only Program identity is invalid")
        else:
            _require(program_candidate is None and program_evidence is None, "no-evidence/presentation Program retains an identity")
        if program["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED":
            candidate_row = registry_rows["programCandidates"][program_candidate]
            evidence_row = registry_rows["programEvidence"][program_evidence]
            _require(
                candidate_row["kind"] == "TYPED_RUNTIME_PROGRAM"
                and evidence_row["kind"] == "TYPED_EXECUTION_RECEIPT"
                and evidence_row["programCandidateId"] == program_candidate,
                "typed Program status disagrees with its discriminator rows",
            )
        elif program["status"] == "DXBC_OCCURRENCE_EXACT":
            candidate_row = registry_rows["programCandidates"][program_candidate]
            evidence_row = registry_rows["programEvidence"][program_evidence]
            family_permutation_exact = (
                evidence_row.get("permutationSelection") == "SINGLE_PERMUTATION_FAMILY"
                or (
                    evidence_row.get("permutationSelection") == "CHILD_MIC_ENGINE_EQUALITY"
                    and bool(evidence_row.get("sourceMaterialPath"))
                    and evidence_row.get("sourceMaterialPath")
                    == evidence_row.get("selectedChildMaterialPath")
                )
            )
            source_family_exact = (
                evidence_row["kind"] == "SOURCE_FAMILY_PROGRAM_EVIDENCE"
                and evidence_row["cookedStatus"] == "EXTRACTED"
                and evidence_row["cookedDxbcSha256"] == candidate_row["dxbcSha256"]
                and family_permutation_exact
                and row.get("sourceParentResolution") != "CHILD_PARENT_BLOCKED"
                and bool(evidence_row.get("sourceCarrier"))
                and evidence_row.get("sourceCarrier") == row.get("dxbcSourceCarrier")
                and evidence_row.get("sourceCarrier") == evidence_row.get("cookedCarrier")
            )
            exact_variant_translated = (
                evidence_row["kind"] == "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE"
                and evidence_row["dxbcSha256"] == candidate_row["dxbcSha256"]
                and evidence_row["literalTranslationAvailable"] is True
                and evidence_row["rendererMatch"] is True
                and evidence_row.get("occurrenceFineRendererKind") == row.get("fineRendererKind")
                and evidence_row.get("expectedFineRendererKind") == row.get("fineRendererKind")
            )
            _require(
                candidate_row["kind"] == "DXBC_LITERAL_TRANSLATION"
                and evidence_row["occurrenceExact"] is True
                and (source_family_exact or exact_variant_translated),
                "occurrence-exact Program status disagrees with its evidence",
            )
        elif program["status"] == "DXBC_OCCURRENCE_EXACT_UNTRANSLATED":
            evidence_row = registry_rows["programEvidence"][program_evidence]
            _require(
                evidence_row["kind"] == "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE"
                and evidence_row["rendererMatch"] is True
                and evidence_row["literalTranslationAvailable"] is False
                and evidence_row["occurrenceExact"] is True
                and evidence_row.get("occurrenceFineRendererKind") == row.get("fineRendererKind")
                and evidence_row.get("expectedFineRendererKind") == row.get("fineRendererKind"),
                "untranslated exact Program status disagrees with its evidence",
            )
            _require("LITERAL_TRANSLATION_MISSING" in row.get("blockers", []), "untranslated exact Program lacks its translation blocker")
        elif program["status"] in (
            "DXBC_FAMILY_REPRESENTATIVE_ONLY", "BOUNDED_SOURCE_PROFILE_ONLY", "NO_PROGRAM_EVIDENCE"
        ):
            evidence_row = registry_rows["programEvidence"][program_evidence]
            representative_kind_ok = (
                evidence_row["kind"] == "SOURCE_FAMILY_PROGRAM_EVIDENCE"
                or (
                    program["status"] == "DXBC_FAMILY_REPRESENTATIVE_ONLY"
                    and evidence_row["kind"] == "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE"
                    and evidence_row["occurrenceExact"] is False
                )
            )
            _require(
                representative_kind_ok
                and evidence_row["occurrenceExact"] is False,
                "representative/bounded/no-equation Program status disagrees with its evidence",
            )
            if program["status"] == "NO_PROGRAM_EVIDENCE":
                _require("PROGRAM_EQUATION_EVIDENCE_ABSENT" in row.get("blockers", []), "no-Program row lacks its equation-evidence blocker")
        if source_parent_blocked and program["status"] not in (
            "TYPED_RUNTIME_PROGRAM_DECLARED", "NOT_APPLICABLE_PRESENTATION"
        ):
            _require(
                program["status"] not in (
                    "DXBC_OCCURRENCE_EXACT", "DXBC_OCCURRENCE_EXACT_UNTRANSLATED"
                )
                and program_candidate is None,
                "blocked child-parent evidence was promoted to an occurrence-exact Program",
            )

        layout_candidate = layout.get("layoutCandidateId")
        layout_evidence = layout.get("layoutEvidenceId")
        if layout["status"] == "TYPED_PACKET_CLOSED":
            _require(layout_candidate in registry_ids["layoutCandidates"] and layout_evidence is None, "typed Layout identity is invalid")
        elif layout["status"] in (
            "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION",
            "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
            "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION",
            "SOURCE_NAMES_ONLY",
        ):
            _require(layout_candidate is None and layout_evidence in registry_ids["layoutEvidence"], "native/source Layout evidence identity is invalid")
        else:
            _require(layout_candidate is None and layout_evidence is None, "unresolved/presentation Layout retains an identity")
        if layout["status"] == "TYPED_PACKET_CLOSED":
            _require(registry_rows["layoutCandidates"][layout_candidate]["kind"] == "TYPED_RUNTIME_PACKET", "typed Layout status references a non-typed row")
        elif layout["status"] == "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION":
            layout_evidence_row = registry_rows["layoutEvidence"][layout_evidence]
            _require(
                layout_evidence_row["kind"] == "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE"
                and layout_evidence_row["runtimePacketTopologyMaterialized"] is False
                and layout_evidence_row["admits"] == "EXACT_NATIVE_WIRE_IDENTITY_ONLY",
                "exact variant Layout status disagrees with its evidence discriminator",
            )
            _require(
                program_evidence is not None
                and registry_rows["programEvidence"][program_evidence].get("kind")
                == "EXACT_COOKED_VARIANT_PROGRAM_EVIDENCE"
                and registry_rows["programEvidence"][program_evidence].get("variantId")
                == layout_evidence_row.get("variantId")
                and registry_rows["programEvidence"][program_evidence].get("variantKeySha256")
                == layout_evidence_row.get("variantKeySha256")
                and registry_rows["programEvidence"][program_evidence].get("dxbcSha256")
                == layout_evidence_row.get("dxbcSha256"),
                "exact variant Program/Layout evidence identity differs",
            )
        elif layout["status"] in (
            "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
            "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION",
        ):
            layout_evidence_row = registry_rows["layoutEvidence"][layout_evidence]
            source_program_evidence_row = registry_rows["programEvidence"][program_evidence]
            expected_within_caps = layout["status"] == "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS"
            _require(
                layout_evidence_row["kind"] == "NAMED_NATIVE_WIRE_EVIDENCE"
                and layout_evidence_row["withinCountCaps"] is expected_within_caps
                and layout_evidence_row["runtimePacketTopologyMaterialized"] is False
                and layout_evidence_row["admits"] == "NAMED_LANE_IDENTITY_ONLY",
                "named Layout status disagrees with its evidence discriminator",
            )
            _require(
                layout_evidence_row.get("parentMaterialPath")
                == source_program_evidence_row.get("effectiveParentMaterialPath")
                and (
                    source_program_evidence_row.get("kind")
                    != "SOURCE_FAMILY_PROGRAM_EVIDENCE"
                    or layout_evidence_row.get("dxbcSha256")
                    == source_program_evidence_row.get("cookedDxbcSha256")
                ),
                "named Layout evidence belongs to another Program family",
            )
        elif layout["status"] == "SOURCE_NAMES_ONLY":
            layout_evidence_row = registry_rows["layoutEvidence"][layout_evidence]
            source_program_evidence_row = registry_rows["programEvidence"][program_evidence]
            _require(
                layout_evidence_row["kind"] == "SOURCE_PARAMETER_NAMES_ONLY"
                and layout_evidence_row.get("parentMaterialPath")
                == source_program_evidence_row.get("effectiveParentMaterialPath"),
                "source-names Layout evidence belongs to another Program family",
            )

        adapter_candidate = adapter.get("adapterCandidateId")
        if adapter["status"] in ("TYPED_STATIC_DISPATCH_CANDIDATE", "RENDER_PROFILE_STATIC_CANDIDATE"):
            _require(adapter_candidate in registry_ids["adapterCandidates"], "static Adapter has no valid candidate")
        else:
            _require(adapter_candidate is None, "unresolved/presentation Adapter retains a candidate")
        if adapter_candidate is not None:
            adapter_row = registry_rows["adapterCandidates"][adapter_candidate]
            expected_adapter_kind = (
                "TYPED_STATIC_DISPATCH" if adapter["status"] == "TYPED_STATIC_DISPATCH_CANDIDATE"
                else "RENDER_PROFILE_STATIC_DISPATCH"
            )
            _require(
                adapter_row["kind"] == expected_adapter_kind
                and adapter_row["runtimeVerified"] is False
                and adapter_row["coarseCarrier"] == row.get("carrier")
                and adapter_row["authoredKind"] == row.get("kind")
                and adapter_row["fineRendererKind"] == row.get("fineRendererKind")
                and adapter_row["vertexFactoryCandidates"] == row.get("vertexFactoryCandidates"),
                "Adapter status disagrees with its candidate discriminator",
            )

        descriptor_variant = descriptor.get("descriptorVariantId")
        if descriptor["status"] in ("TYPED_VALUES_CLOSED", "SOURCE_VALUES_PRESENT_UNPACKED", "RESOURCE_ONLY_NO_MATERIAL_VALUES"):
            _require(descriptor_variant in registry_ids["descriptorVariants"], "Descriptor has no valid variant")
        else:
            _require(descriptor_variant is None, "missing/presentation Descriptor retains a variant")
        if descriptor_variant is not None:
            descriptor_kind = registry_rows["descriptorVariants"][descriptor_variant]["kind"]
            expected_descriptor_kind = {
                "TYPED_VALUES_CLOSED": "TYPED_RUNTIME_VALUES",
                "SOURCE_VALUES_PRESENT_UNPACKED": "SOURCE_VALUES_UNPACKED",
                "RESOURCE_ONLY_NO_MATERIAL_VALUES": "RESOURCE_BINDINGS_ONLY",
            }[descriptor["status"]]
            _require(descriptor_kind == expected_descriptor_kind, "Descriptor status disagrees with its variant discriminator")
            if descriptor["status"] == "TYPED_VALUES_CLOSED":
                descriptor_row = registry_rows["descriptorVariants"][descriptor_variant]
                layout_row = registry_rows["layoutCandidates"][layout_candidate]
                program_row = registry_rows["programCandidates"][program_candidate]
                adapter_row = registry_rows["adapterCandidates"][adapter_candidate]
                _require(descriptor_row["layoutCandidateId"] == layout_candidate, "typed Descriptor/Layout identity differs")
                _require(
                    program_row["backend"] == layout_row["backend"] == adapter_row["backend"]
                    and program_row["opcode"] == adapter_row["opcode"],
                    "typed Program/Layout/Adapter backend or opcode differs",
                )
                texture_topology = layout_row.get("textureLaneTopology")
                descriptor_lanes = descriptor_row.get("textureLanes")
                _require(
                    isinstance(texture_topology, list)
                    and isinstance(descriptor_lanes, list)
                    and layout_row.get("textureLaneCount") == len(texture_topology) == len(descriptor_lanes)
                    and {lane.get("laneId") for lane in texture_topology}
                    == {lane.get("laneId") for lane in descriptor_lanes},
                    "typed Descriptor texture lanes differ from Layout topology",
                )
                scalar_rows = descriptor_row.get("scalars")
                vector_rows = descriptor_row.get("vectors")
                _require(
                    isinstance(scalar_rows, list)
                    and isinstance(vector_rows, list)
                    and layout_row.get("scalarCount") == len(scalar_rows)
                    and layout_row.get("vectorCount") == len(vector_rows)
                    and sorted(item.get("packedIndex") for item in scalar_rows)
                    == layout_row.get("scalarPackedIndices")
                    and sorted(item.get("packedIndex") for item in vector_rows)
                    == layout_row.get("vectorPackedIndices"),
                    "typed Descriptor scalar/vector packing differs from Layout topology",
                )
                artist_parameters = descriptor_row.get("artistParameters")
                colors = descriptor_row.get("colors")
                _require(isinstance(artist_parameters, list) and isinstance(colors, list), "typed Descriptor artist/color arrays are malformed")
                if program_row["backend"] == "artistVisualV4":
                    _require(len(artist_parameters) <= 8 and len(colors) <= 2, "typed Descriptor artist/color capacity exceeded")
                else:
                    _require(not artist_parameters and not colors, "non-Artist typed Descriptor retains artist/color values")
                _validate_typed_registry_projection(
                    program_row, layout_row, descriptor_row, adapter_row
                )

        composition_variant_ids = row.get("compositionVariantIds")
        _require(
            isinstance(composition_variant_ids, list)
            and composition_variant_ids == sorted(set(composition_variant_ids))
            and all(identifier in composition_ids for identifier in composition_variant_ids),
            "occurrence composition references are invalid",
        )
        composition_references.update(composition_variant_ids)
        for composition_variant_id in composition_variant_ids:
            composition_row = composition_rows_by_id[composition_variant_id]
            _require(
                composition_row["semantic"].get("effectAssetId") == row.get("effectAssetId"),
                "Composition semantic is referenced by a different Effect asset",
            )
        for registry_name, identifier in (
            ("programCandidates", program_candidate),
            ("programEvidence", program_evidence),
            ("layoutCandidates", layout_candidate),
            ("layoutEvidence", layout_evidence),
            ("adapterCandidates", adapter_candidate),
            ("descriptorVariants", descriptor_variant),
        ):
            if identifier is not None:
                registry_references[registry_name].add(identifier)
        scope_bits = row.get("scopeBits")
        _require(
            isinstance(scope_bits, dict)
            and set(scope_bits) == {"authored", "catalogDeclared", "runtimePublished", "productConsumed"}
            and all(isinstance(value, bool) for value in scope_bits.values())
            and scope_bits["authored"] is True,
            "occurrence scope bits are invalid",
        )
        if scope_bits["catalogDeclared"]:
            catalog_declared_assets.add(row["effectAssetId"])
        if scope_bits["productConsumed"]:
            product_consumed_assets.add(row["effectAssetId"])
        published_document_path = row.get("publishedDocumentPath")
        if scope_bits["runtimePublished"]:
            _require(
                isinstance(published_document_path, str)
                and published_document_path in published_input_paths,
                "runtime-published occurrence has no tracked published document",
            )
            _require(
                PurePosixPath(published_document_path).name.startswith(
                    row["effectAssetId"] + "."
                ),
                "published document filename does not belong to its Effect asset",
            )
            runtime_published_assets.add(row["effectAssetId"])
            runtime_published_occurrence_count += 1
            prior_published_path = published_paths_by_asset.setdefault(
                row["effectAssetId"], published_document_path
            )
            _require(
                prior_published_path == published_document_path,
                "one Effect asset resolves to multiple published documents",
            )
            prior_published_asset = published_assets_by_path.setdefault(
                published_document_path, row["effectAssetId"]
            )
            _require(
                prior_published_asset == row["effectAssetId"],
                "one published document is claimed by multiple Effect assets",
            )
        else:
            _require(
                published_document_path is None,
                "non-published occurrence retains a published document receipt",
            )
        expected_composition_status = (
            "PRODUCT_BOUND_CUE" if scope_bits["productConsumed"] else
            "RUNTIME_PUBLISHED_WITHOUT_CONSUMER" if scope_bits["runtimePublished"] else
            "CATALOG_DECLARED_ONLY" if scope_bits["catalogDeclared"] else
            "AUTHORED_ONLY"
        )
        _require(row["compositionStatus"] == expected_composition_status, "Composition status disagrees with scope bits")
        _require(
            scope_bits["productConsumed"] == bool(composition_variant_ids),
            "Product-consumed scope and Composition references differ",
        )
        asset_projection = (
            scope_bits,
            tuple(composition_variant_ids),
            row["compositionStatus"],
        )
        prior_asset_projection = asset_scope_projection.setdefault(row["effectAssetId"], asset_projection)
        _require(prior_asset_projection == asset_projection, "one Effect asset has inconsistent Composition/scope projection")
        published_exact = row.get("publishedElementExact")
        _require(
            (scope_bits["runtimePublished"] and isinstance(published_exact, bool))
            or (not scope_bits["runtimePublished"] and published_exact is None),
            "published element identity disagrees with runtime scope",
        )
        expected_product_status = (
            "PUBLISHED_ELEMENT_STALE" if published_exact is False else
            "PRODUCT_JOIN_CLOSED" if scope_bits["productConsumed"] and scope_bits["runtimePublished"] and scope_bits["catalogDeclared"] else
            "AUTHORED_NOT_CATALOGED" if not scope_bits["catalogDeclared"] else
            "CATALOG_NOT_PUBLISHED" if not scope_bits["runtimePublished"] else
            "RUNTIME_PUBLISHED_UNCONSUMED"
        )
        _require(row["productStatus"] == expected_product_status, "Product status disagrees with occurrence-level publish/scope evidence")
        if published_exact is False:
            _require("PUBLISHED_ELEMENT_STALE" in row.get("blockers", []), "stale published element is not fail-closed")

        legacy_review = row.get("legacyGoldenReview")
        _require(
            isinstance(legacy_review, dict)
            and legacy_review.get("status") in USER_REVIEW_STATUSES,
            "legacy golden review status is unsupported",
        )
        receipt_occurrence_id = legacy_review.get("receiptOccurrenceId")
        if legacy_review["status"] == "NOT_RECORDED":
            _require(receipt_occurrence_id is None, "unrecorded legacy review retains a receipt")
        else:
            _require(receipt_occurrence_id == occurrence_id, "legacy review receipt does not match its occurrence")
        _require(row.get("horizontalV1Review") == "NOT_RECORDED", "occurrence horizontal V1 review was synthesized")
        blockers = row.get("blockers")
        _require(isinstance(blockers, list) and blockers == sorted(set(blockers)) and all(isinstance(value, str) and value for value in blockers), "occurrence blockers are invalid")
        for blocker in blockers:
            blocker_occurrence_ids[blocker].append(occurrence_id)

        typed_tuple = (
            program["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED"
            and layout["status"] == "TYPED_PACKET_CLOSED"
            and adapter["status"] == "TYPED_STATIC_DISPATCH_CANDIDATE"
        )
        native_tuple = (
            program["status"] == "DXBC_OCCURRENCE_EXACT"
            and layout["status"] in (
                "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION",
                "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
                "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION",
            )
            and adapter["status"] == "RENDER_PROFILE_STATIC_CANDIDATE"
        )
        tuple_cohort_id = row.get("tupleCohortId")
        if typed_tuple or native_tuple:
            _require(isinstance(tuple_cohort_id, str), "cohort-eligible occurrence has no tuple cohort")
            tuple_occurrence_ids[tuple_cohort_id].append(occurrence_id)
        else:
            _require(tuple_cohort_id is None, "ineligible occurrence entered a tuple cohort")
        if adapter["status"] == "PRESENTATION_SEPARATE":
            _require(row["carrier"] == "PRESENTATION", "presentation Adapter is not a presentation carrier")
        axis_counts["programStatusCounts"][program["status"]] += 1
        axis_counts["layoutStatusCounts"][layout["status"]] += 1
        axis_counts["adapterStatusCounts"][adapter["status"]] += 1
        axis_counts["descriptorStatusCounts"][descriptor["status"]] += 1
        axis_counts["compositionStatusCounts"][row["compositionStatus"]] += 1
        axis_counts["productStatusCounts"][row["productStatus"]] += 1
        axis_counts["legacyGoldenReviewStatusCounts"][legacy_review["status"]] += 1
        axis_counts["horizontalV1ReviewStatusCounts"][row["horizontalV1Review"]] += 1

    _require(
        set(published_paths_by_asset.values()) == published_input_paths,
        "published Effect input ownership and occurrence scope differ",
    )
    for registry_name, identifiers in registry_ids.items():
        _require(registry_references[registry_name] == identifiers, registry_name + " contains an orphan or unreferenced row")
    _require(composition_references == composition_ids, "composition registry contains an orphan or unreferenced row")
    legacy_review_projection = sorted(
        (
            {"occurrenceId": row["occurrenceId"], **row["legacyGoldenReview"]}
            for row in occurrences
            if row["legacyGoldenReview"]["status"] != "NOT_RECORDED"
        ),
        key=lambda value: value["occurrenceId"],
    )
    legacy_review_projection_sha = canonical_sha256(legacy_review_projection)
    _require(
        legacy_review_projection_sha == EXPECTED_LEGACY_REVIEW_PROJECTION_SHA256,
        "legacy golden review projection is not the pinned exact receipt set",
    )

    summary = document.get("summary")
    _require(isinstance(summary, dict) and summary.get("occurrenceCount") == len(occurrences), "inventory summary occurrence count drifted")
    _require(
        set(summary) == {
            "documentCount", "occurrenceCount", "domainDocumentCounts", "domainOccurrenceCounts",
            "carrierCounts", "fineRendererKindCounts", "programStatusCounts", "layoutStatusCounts",
            "adapterStatusCounts", "descriptorStatusCounts", "materialEvidenceCounts",
            "namedAbiResolvedFamilyCount", "namedAbiWithinCountCapsFamilyCount",
            "namedAbiRequiresCountExtensionFamilyCount", "namedAbiWithinCountCapsZeroTextureFamilyCount",
            "namedAbiWithinCountCapsNonemptyPacketTopologyMaterializedFamilyCount",
            "characterCueRowCount", "characterAssetCount", "characterAssetCountsByDomain",
            "valtanPatternAssetCount", "valtanBossVisualAssetCount", "compositionVariantCount",
            "compositionProjectionSha256",
            "scopedCatalogDeclaredAssetCount", "authoredCatalogJoinedAssetCount",
            "scopedCatalogWithoutOccurrenceIds", "runtimePublishedAssetCount",
            "scopedRuntimeCatalogAssetCount", "scopedRuntimeMissingAuthoredAssetIds",
            "runtimePublishedOccurrenceCount", "runtimePublishedAuthoredOccurrenceCount",
            "productConsumedAssetCount",
            "runtimeAssetSetEqualsProductConsumerSet", "compositionStatusCounts",
            "productStatusCounts", "legacyGoldenReviewStatusCounts",
            "legacyGoldenReviewProjectionSha256", "horizontalV1ReviewStatusCounts",
            "occurrenceExactLayoutStatusCounts", "cohortCount", "cohortKindCounts",
            "cohortOccurrenceCount", "runtimeVerifiedCohortCount",
            "runtimeDescriptorExpansionEligibleCohortCount",
        },
        "inventory summary has missing, hidden, or admission fields",
    )
    for key, counts in axis_counts.items():
        _require(summary.get(key) == dict(sorted(counts.items())), "inventory summary drifted: " + key)
        _require(sum(counts.values()) == len(occurrences), "axis summary denominator drifted: " + key)
    _require(
        summary.get("legacyGoldenReviewProjectionSha256") == legacy_review_projection_sha,
        "legacy golden review projection summary drifted",
    )
    _require(
        summary.get("compositionProjectionSha256") == composition_projection_sha,
        "Composition projection summary drifted",
    )
    _require(
        summary.get("authoredCatalogJoinedAssetCount") == len(catalog_declared_assets)
        and summary.get("runtimePublishedAssetCount") == len(runtime_published_assets)
        and summary.get("runtimePublishedAuthoredOccurrenceCount")
        == runtime_published_occurrence_count
        and isinstance(summary.get("runtimePublishedOccurrenceCount"), int)
        and not isinstance(summary.get("runtimePublishedOccurrenceCount"), bool)
        and summary.get("runtimePublishedOccurrenceCount") >= 0
        and summary.get("productConsumedAssetCount") == len(product_consumed_assets)
        and summary.get("scopedCatalogDeclaredAssetCount")
        == len(catalog_declared_assets) + len(summary.get("scopedCatalogWithoutOccurrenceIds", [])),
        "catalog/runtime/Product scope summary is not derived from occurrence evidence",
    )

    cohorts = document.get("cohorts")
    _require(isinstance(cohorts, list), "cohorts must be an array")
    cohort_ids: set[str] = set()
    cohort_occurrences: set[str] = set()
    cohort_kind_counts: Counter[str] = Counter()
    for cohort in cohorts:
        _require(
            isinstance(cohort, dict)
            and set(cohort) == {
                "tupleCohortId", "kind", "programCandidateId", "layoutIdentityId",
                "adapterCandidateId", "occurrenceIds", "domains",
                "productConsumedOccurrenceCount", "runtimeVerified",
                "runtimeDescriptorExpansionEligible", "structuralDescriptorReuseCandidate",
                "occurrenceCount",
            },
            "tuple cohort has missing, hidden, or admission fields",
        )
        cohort_id = _require_string(cohort.get("tupleCohortId"), "tupleCohortId")
        payload = {
            "programCandidateId": cohort.get("programCandidateId"),
            "layoutIdentityId": cohort.get("layoutIdentityId"),
            "adapterCandidateId": cohort.get("adapterCandidateId"),
        }
        _require(cohort_id == "cohort." + canonical_sha256(payload), "tuple cohort identity drifted")
        _require(cohort_id not in cohort_ids, "duplicate tuple cohort")
        cohort_ids.add(cohort_id)
        cohort_kind = cohort.get("kind")
        _require(cohort_kind in ("TYPED_EXECUTION_COHORT", "NATIVE_EVIDENCE_COHORT"), "unknown tuple cohort kind")
        _require(cohort.get("programCandidateId") in registry_ids["programCandidates"], "cohort Program candidate is unknown")
        _require(cohort.get("adapterCandidateId") in registry_ids["adapterCandidates"], "cohort Adapter candidate is unknown")
        if cohort_kind == "TYPED_EXECUTION_COHORT":
            _require(cohort.get("layoutIdentityId") in registry_ids["layoutCandidates"], "typed cohort Layout candidate is unknown")
            _require(cohort.get("structuralDescriptorReuseCandidate") is True, "typed cohort lost structural reuse candidate status")
        else:
            _require(cohort.get("layoutIdentityId") in registry_ids["layoutEvidence"], "native cohort Layout evidence is unknown")
            _require(cohort.get("structuralDescriptorReuseCandidate") is False, "native evidence cohort became structural descriptor closure")
        _require(cohort.get("runtimeVerified") is False, "cohort became runtime verified")
        _require(cohort.get("runtimeDescriptorExpansionEligible") is False, "cohort became runtime expansion eligible")
        cohort_occurrence_ids = cohort.get("occurrenceIds")
        _require(
            isinstance(cohort_occurrence_ids, list)
            and cohort_occurrence_ids == sorted(set(cohort_occurrence_ids))
            and cohort.get("occurrenceCount") == len(cohort_occurrence_ids),
            "cohort occurrence list/count is invalid",
        )
        _require(cohort_occurrence_ids == sorted(tuple_occurrence_ids.get(cohort_id, [])), "cohort occurrence back-reference drifted")
        expected_domains = sorted({occurrence_by_id[value]["domain"] for value in cohort_occurrence_ids})
        expected_product_count = sum(occurrence_by_id[value]["scopeBits"]["productConsumed"] for value in cohort_occurrence_ids)
        _require(cohort.get("domains") == expected_domains, "cohort domain projection drifted")
        _require(cohort.get("productConsumedOccurrenceCount") == expected_product_count, "cohort Product occurrence count drifted")
        for occurrence_id in cohort_occurrence_ids:
            _require(occurrence_id in occurrence_ids and occurrence_id not in cohort_occurrences, "cohort occurrence identity is invalid or duplicated")
            occurrence = occurrence_by_id[occurrence_id]
            _require(occurrence["tupleCohortId"] == cohort_id, "occurrence tuple cohort back-reference drifted")
            _require(occurrence["program"]["programCandidateId"] == cohort["programCandidateId"], "cohort/occurrence Program identity differs")
            _require(occurrence["adapter"]["adapterCandidateId"] == cohort["adapterCandidateId"], "cohort/occurrence Adapter identity differs")
            occurrence_layout_identity = occurrence["layout"]["layoutCandidateId"] if cohort_kind == "TYPED_EXECUTION_COHORT" else occurrence["layout"]["layoutEvidenceId"]
            _require(occurrence_layout_identity == cohort["layoutIdentityId"], "cohort/occurrence Layout identity differs")
            cohort_occurrences.add(occurrence_id)
        cohort_kind_counts[cohort_kind] += 1
    _require(set(tuple_occurrence_ids) == cohort_ids, "occurrence tuple cohorts and cohort registry differ")
    _require(summary.get("cohortCount") == len(cohorts), "summary cohort count drifted")
    _require(summary.get("cohortKindCounts") == dict(sorted(cohort_kind_counts.items())), "summary cohort kind counts drifted")
    _require(summary.get("cohortOccurrenceCount") == len(cohort_occurrences), "summary cohort occurrence count drifted")
    _require(summary.get("runtimeVerifiedCohortCount") == 0, "summary runtime verified cohort count is nonzero")
    _require(summary.get("runtimeDescriptorExpansionEligibleCohortCount") == 0, "summary runtime expansion eligible cohort count is nonzero")

    blocker_buckets = document.get("blockerBuckets")
    _require(isinstance(blocker_buckets, list), "blockerBuckets must be an array")
    seen_blockers: set[str] = set()
    for bucket in blocker_buckets:
        _require(
            isinstance(bucket, dict)
            and set(bucket) == {"blockerCode", "occurrenceCount", "sampleOccurrenceIds"},
            "blocker bucket has missing or hidden fields",
        )
        blocker = _require_string(bucket.get("blockerCode"), "blockerCode")
        _require(blocker not in seen_blockers and blocker in blocker_occurrence_ids, "blocker bucket is duplicate or unknown")
        seen_blockers.add(blocker)
        expected_ids = sorted(blocker_occurrence_ids[blocker])
        _require(bucket.get("occurrenceCount") == len(expected_ids), "blocker bucket count drifted")
        _require(bucket.get("sampleOccurrenceIds") == expected_ids[:20], "blocker bucket sample drifted")
    _require(seen_blockers == set(blocker_occurrence_ids), "blocker bucket coverage drifted")

    expected_policies = {
        "candidateIdentity": "CANONICAL_CONTENT_SHA256",
        "occurrenceIdentity": "EFFECT_ASSET_ID_AND_STABLE_ELEMENT_ID_SHA256",
        "representativeProgramPromotion": "FORBIDDEN",
        "namedAbiTypedPacketPromotion": "FORBIDDEN",
        "runtimeVerification": "NOT_PROVIDED_BY_THIS_ARTIFACT",
        "productAdmission": "NOT_PROVIDED_BY_THIS_ARTIFACT",
        "userVisualAdmission": "USER_ONLY",
    }
    _require(document.get("policies") == expected_policies, "inventory policy boundary drifted")
    _require(document.get("transaction") == {"model": "PARSE_VALIDATE_STAGE_ATOMIC_REPLACE", "partialCommitAllowed": False}, "inventory transaction boundary drifted")
    canaries = document.get("canaries")
    _require(
        isinstance(canaries, dict)
        and set(canaries) == {
            "g00OccurrenceSafety", "artistFHorizontalGolden", "productConsumerRuntimeSnapshot"
        },
        "inventory canaries are absent or contain hidden fields",
    )
    g00_canary = canaries.get("g00OccurrenceSafety")
    _require(
        isinstance(g00_canary, dict)
        and set(g00_canary) == {"status", "sourceOccurrenceCount", "statusOccurrenceCounts", "documents"}
        and g00_canary.get("status") == "MATCHES_SEALED_SECONDARY_PROJECTION"
        and g00_canary.get("sourceOccurrenceCount") == 269,
        "G00 occurrence safety canary drifted",
    )
    expected_g00_status_counts = {
        "PARENT_ONLY": 18,
        "PARENT_RESOLVED_PROGRAM_MISSING": 0,
        "PROGRAM_EXACT_NAMED_MAPPING_MISSING": 17,
        "PROGRAM_EXACT_NAMED_MAPPING_ONLY": 114,
        "PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_MISSING": 0,
        "PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_ONLY": 86,
        "SHADERMAP_FOUND_DXBC_MISSING": 34,
        "UNKNOWN": 0,
    }
    expected_g00_documents = [
        {"document": "effect.lancemaster.skill.34110.unified.effect.json", "sourceRowCount": 83, "executionRowCount": 0},
        {"document": "effect.lancemaster.skill.34150.unified.effect.json", "sourceRowCount": 186, "executionRowCount": 0},
        {"document": "effect.artist.skill.31470.unified.effect.json", "sourceRowCount": 0, "executionRowCount": 17},
    ]
    _require(
        g00_canary.get("statusOccurrenceCounts") == expected_g00_status_counts
        and g00_canary.get("documents") == expected_g00_documents,
        "G00 occurrence safety detail drifted",
    )
    artist_canary = canaries.get("artistFHorizontalGolden")
    artist_asset = "effect.artist.skill.31470.unified"
    artist_stable_ids = ["sprite.2b3dc6842507e910", "sprite.c65181324417a1a8"]
    artist_rows = [
        row
        for stable_id in artist_stable_ids
        for row in occurrences
        if row["effectAssetId"] == artist_asset and row["elementId"] == stable_id
    ]
    _require(
        isinstance(artist_canary, dict)
        and set(artist_canary) == {"status", "effectAssetId", "stableIds", "tupleCohortId", "runtimeVerified", "horizontalV1Review", "legacyGoldenReviews"}
        and artist_canary.get("status") == "STRUCTURAL_DUAL_RESOLVE_PENDING_USER_A_B"
        and artist_canary.get("effectAssetId") == artist_asset
        and artist_canary.get("stableIds") == artist_stable_ids
        and artist_canary.get("runtimeVerified") is False
        and artist_canary.get("horizontalV1Review") == "PENDING_USER_A_B"
        and artist_canary.get("tupleCohortId") in cohort_ids,
        "Artist F horizontal canary overstates or loses its boundary",
    )
    _require(
        len(artist_rows) == 2
        and len({row["tupleCohortId"] for row in artist_rows}) == 1
        and artist_canary["tupleCohortId"] == artist_rows[0]["tupleCohortId"]
        and artist_canary.get("legacyGoldenReviews") == [row["legacyGoldenReview"] for row in artist_rows],
        "Artist F horizontal canary projection drifted",
    )
    product_canary = canaries.get("productConsumerRuntimeSnapshot")
    _require(
        isinstance(product_canary, dict)
        and set(product_canary) == {
            "status", "characterAssetCount", "valtanPatternAssetCount",
            "valtanBossVisualAssetCount", "productConsumedAssetCount",
            "runtimePublishedAssetCount", "runtimePublishedOccurrenceCount",
            "runtimePublishedAuthoredOccurrenceCount",
            "runtimeAssetSetEqualsProductConsumerSet", "compositionProjectionSha256",
        },
        "Product consumer/runtime canary is absent or contains hidden fields",
    )
    product_projection = {
        "characterAssetCount": summary.get("characterAssetCount"),
        "valtanPatternAssetCount": summary.get("valtanPatternAssetCount"),
        "valtanBossVisualAssetCount": summary.get("valtanBossVisualAssetCount"),
        "productConsumedAssetCount": summary.get("productConsumedAssetCount"),
        "runtimePublishedAssetCount": summary.get("runtimePublishedAssetCount"),
        "runtimePublishedOccurrenceCount": summary.get("runtimePublishedOccurrenceCount"),
        "runtimePublishedAuthoredOccurrenceCount": summary.get(
            "runtimePublishedAuthoredOccurrenceCount"
        ),
        "runtimeAssetSetEqualsProductConsumerSet": summary.get("runtimeAssetSetEqualsProductConsumerSet"),
        "compositionProjectionSha256": summary.get("compositionProjectionSha256"),
    }
    _require(
        all(product_canary.get(key) == value for key, value in product_projection.items()),
        "Product consumer/runtime canary projection drifted",
    )
    product_snapshot_expected = (
        product_projection["characterAssetCount"] == EXPECTED_CHARACTER_PRODUCT_ASSET_COUNT
        and product_projection["valtanPatternAssetCount"] == EXPECTED_VALTAN_PATTERN_ASSET_COUNT
        and product_projection["valtanBossVisualAssetCount"] == EXPECTED_VALTAN_BOSS_VISUAL_ASSET_COUNT
        and product_projection["productConsumedAssetCount"] == EXPECTED_RUNTIME_PRODUCT_ASSET_COUNT
        and product_projection["runtimePublishedAssetCount"] == EXPECTED_RUNTIME_PRODUCT_ASSET_COUNT
        and product_projection["runtimePublishedOccurrenceCount"] == EXPECTED_RUNTIME_PRODUCT_OCCURRENCE_COUNT
        and product_projection["runtimePublishedAuthoredOccurrenceCount"]
        == EXPECTED_RUNTIME_PRODUCT_OCCURRENCE_COUNT
        and product_projection["runtimeAssetSetEqualsProductConsumerSet"] is True
        and product_projection["compositionProjectionSha256"]
        == EXPECTED_COMPOSITION_PROJECTION_SHA256
    )
    _require(
        product_canary.get("status") == (
            "MATCHES_CURRENT_REPOSITORY_SNAPSHOT" if product_snapshot_expected
            else "DRIFTED_FROM_CURRENT_REGRESSION_SNAPSHOT"
        ),
        "Product consumer/runtime canary status drifted",
    )


def validate_input_snapshot(document: dict[str, Any], repository_root: Path) -> None:
    inputs = document.get("inputs")
    _require(isinstance(inputs, list), "inventory inputs must be an array")
    seen_paths: set[str] = set()
    for row in inputs:
        _require(isinstance(row, dict), "inventory input row must be an object")
        relative = _safe_relative_path(row.get("path"), "inventory input path")
        _require(relative not in seen_paths, "duplicate inventory input path")
        seen_paths.add(relative)
        absolute = repository_root / Path(relative)
        _require(absolute.is_file(), "inventory input disappeared before commit: " + relative)
        payload = absolute.read_bytes()
        _require(len(payload) == row.get("byteSize"), "inventory input byte size changed before commit: " + relative)
        _require(_sha256_bytes(payload) == row.get("rawSha256"), "inventory input raw hash changed before commit: " + relative)
        canonical = row.get("canonicalJsonSha256")
        parsed: Any = None
        if canonical is not None:
            parsed = _decode_json(payload, relative)
            _require(canonical_sha256(parsed) == canonical, "inventory input canonical JSON changed before commit: " + relative)
        if "PUBLISHED_EFFECT_DOCUMENT" in row.get("roles", []):
            _require(
                relative.startswith("Client/Bin/DataFiles/Effect/Authored/")
                and isinstance(parsed, dict)
                and parsed.get("schema") == "lostark.effect-authoring"
                and isinstance(parsed.get("effectAssetId"), str)
                and PurePosixPath(relative).name.startswith(parsed["effectAssetId"] + "."),
                "published Effect input identity/path is malformed before commit",
            )

    roles_by_path = {
        row["path"]: set(row["roles"])
        for row in inputs
    }
    recorded_authored = {
        path for path, roles in roles_by_path.items() if "TARGET_AUTHORED_DOCUMENT" in roles
    }
    discovered_authored: set[str] = set()
    authored_root = repository_root / AUTHORED_DIRECTORY
    for absolute in authored_root.glob("*.effect.json"):
        payload = absolute.read_bytes()
        parsed = _decode_json(payload, absolute.as_posix())
        _require(isinstance(parsed, dict), "authored snapshot root must be an object")
        effect_asset_id = parsed.get("effectAssetId")
        filename_asset_id = absolute.name[:-len(".effect.json")]
        filename_target = _domain_for_asset(filename_asset_id) is not None
        parsed_target = isinstance(effect_asset_id, str) and _domain_for_asset(effect_asset_id) is not None
        _require(filename_target == parsed_target, "target-prefix filename/parsed-ID scope changed before commit")
        if parsed_target:
            discovered_authored.add(absolute.relative_to(repository_root).as_posix())
    _require(discovered_authored == recorded_authored, "target authored file membership changed before commit")

    recorded_hlsli = {
        path for path, roles in roles_by_path.items() if "LITERAL_HLSL_TRANSLATION" in roles
    }
    discovered_hlsli = {
        path.relative_to(repository_root).as_posix()
        for path in (repository_root / TRANSLATED_SHADER_DIRECTORY).glob("*.hlsli")
        if path.is_file()
    }
    _require(discovered_hlsli == recorded_hlsli, "translated HLSLI file membership changed before commit")

    recorded_dxbc = {
        path
        for path, roles in roles_by_path.items()
        if roles.intersection({"COOKED_DXBC", "EXACT_VARIANT_DXBC"})
    }
    discovered_dxbc = {
        path.relative_to(repository_root).as_posix()
        for path in (repository_root / COOKED_SHADER_DIRECTORY).glob("*.dxbc")
        if path.is_file()
    }
    _require(discovered_dxbc == recorded_dxbc, "cooked DXBC file membership changed before commit")


def build_inventory(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    tracker = InputTracker(repository_root.resolve())
    evidence = _load_shader_evidence(tracker)
    registries = {
        "carrier": VariantRegistry("carrier."),
        "programCandidate": VariantRegistry("program."),
        "programEvidence": VariantRegistry("program-evidence."),
        "layoutCandidate": VariantRegistry("layout."),
        "layoutEvidence": VariantRegistry("layout-evidence."),
        "adapter": VariantRegistry("adapter."),
        "descriptor": VariantRegistry("descriptor."),
    }
    occurrences, documents, authored_summary = _build_authored_occurrences(tracker, evidence, registries)
    _require(authored_summary["domainDocumentCounts"] == EXPECTED_DOCUMENT_COUNTS, "target authored document denominator drifted")
    _require(authored_summary["domainOccurrenceCounts"] == EXPECTED_OCCURRENCE_COUNTS, "target occurrence denominator drifted")
    _require(authored_summary["carrierCounts"] == EXPECTED_CARRIER_COUNTS, "coarse carrier denominator drifted")
    _require(authored_summary["materialEvidenceCounts"] == {
        "none": EXPECTED_NO_MATERIAL_EVIDENCE_COUNT,
        "sourceProfile": EXPECTED_SOURCE_PROFILE_COUNT,
        "typedExecution": EXPECTED_EXECUTION_COUNT,
    }, "material evidence denominator drifted")
    _require(authored_summary["namedAbiWithinCountCapsFamilyCount"] == EXPECTED_NAMED_ABI_FITS, "named ABI within-count family denominator drifted")
    _require(authored_summary["namedAbiRequiresCountExtensionFamilyCount"] == EXPECTED_NAMED_ABI_REQUIRES_EXTENSION, "named ABI extension family denominator drifted")
    _require(authored_summary["namedAbiWithinCountCapsZeroTextureFamilyCount"] == 2, "named ABI vacuous zero-texture denominator drifted")
    _require(authored_summary["programStatusCounts"] == {
        "TYPED_RUNTIME_PROGRAM_DECLARED": 50,
        "DXBC_OCCURRENCE_EXACT": 2148,
        "DXBC_OCCURRENCE_EXACT_UNTRANSLATED": 8,
        "DXBC_FAMILY_REPRESENTATIVE_ONLY": 3671,
        "BOUNDED_SOURCE_PROFILE_ONLY": 525,
        "NO_PROGRAM_EVIDENCE": 1055,
        "NOT_APPLICABLE_PRESENTATION": 109,
    }, "Program evidence projection drifted")
    _require(authored_summary["layoutStatusCounts"] == {
        "TYPED_PACKET_CLOSED": 50,
        "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION": 20,
        "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS": 2430,
        "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION": 3153,
        "SOURCE_NAMES_ONLY": 673,
        "UNRESOLVED": 1131,
        "NOT_APPLICABLE_PRESENTATION": 109,
    }, "Layout evidence projection drifted")
    _require(authored_summary["adapterStatusCounts"] == {
        "TYPED_STATIC_DISPATCH_CANDIDATE": 50,
        "RENDER_PROFILE_STATIC_CANDIDATE": 7385,
        "UNRESOLVED": 22,
        "PRESENTATION_SEPARATE": 109,
    }, "Adapter evidence projection drifted")
    _require(authored_summary["descriptorStatusCounts"] == {
        "TYPED_VALUES_CLOSED": 50,
        "SOURCE_VALUES_PRESENT_UNPACKED": 2732,
        "RESOURCE_ONLY_NO_MATERIAL_VALUES": 4575,
        "MISSING": 100,
        "NOT_APPLICABLE_PRESENTATION": 109,
    }, "Descriptor evidence projection drifted")
    _require(authored_summary["fineRendererKindCounts"] == {
        "ANIM_TRAIL": 3,
        "AUTHORED_LEGACY_TRAIL": 12,
        "CASCADE_RIBBON": 4,
        "DECAL_PARTICLE": 83,
        "LEGACY_STANDALONE_SPRITE": 1902,
        "LIGHT_PRESENTATION": 50,
        "MESH_PARTICLE": 1197,
        "PARTICLE_TYPEDATA_RIBBON_REQUIRES_CARRIER_REPLACEMENT": 10,
        "SCREEN_POST_PRESENTATION": 59,
        "SPRITE_PARTICLE": 2892,
        "STANDALONE_MESH": 1354,
    }, "fine renderer kind projection drifted")

    composition_registry, product_consumers, composition_summary = _build_composition(tracker, set(documents))
    catalog_joins, catalog_summary = _build_catalog_joins(tracker, documents, product_consumers)
    _require(catalog_summary["scopedCatalogDeclaredAssetCount"] == 257, "scoped target catalog denominator drifted")
    _require(catalog_summary["authoredCatalogJoinedAssetCount"] == EXPECTED_AUTHORING_CATALOG_ASSET_COUNT, "authored catalog join denominator drifted")

    for occurrence in occurrences:
        join = catalog_joins[occurrence["effectAssetId"]]
        occurrence["compositionStatus"] = join["compositionStatus"]
        occurrence["scopeBits"] = join["scopeBits"]
        occurrence["compositionVariantIds"] = join["compositionVariantIds"]
        occurrence["publishedDocumentPath"] = join["publishedPath"]
        published_exact = join["publishedElementExact"].get(occurrence["elementId"])
        occurrence["publishedElementExact"] = published_exact
        occurrence["productStatus"] = (
            "PUBLISHED_ELEMENT_STALE" if published_exact is False else join["productStatus"]
        )
        if join["scopeBits"]["productConsumed"] is False:
            occurrence["blockers"].append("PRODUCT_CONSUMER_ABSENT")
        if join["scopeBits"]["productConsumed"] and not (
            join["scopeBits"]["catalogDeclared"] and join["scopeBits"]["runtimePublished"]
        ):
            occurrence["blockers"].append("PRODUCT_RUNTIME_JOIN_OPEN")
        if published_exact is False:
            occurrence["blockers"].append("PUBLISHED_ELEMENT_STALE")
        occurrence["legacyGoldenReview"] = _legacy_golden_review(occurrence, evidence["reviewReceipts"])
        occurrence["horizontalV1Review"] = "NOT_RECORDED"
        occurrence["blockers"] = sorted(set(occurrence["blockers"]))

    cohorts = _build_cohorts(occurrences)
    canaries = _build_canaries(occurrences, evidence, composition_summary, catalog_summary)
    blocker_buckets = _build_blocker_buckets(occurrences)

    program_counts = Counter(row["program"]["status"] for row in occurrences)
    layout_counts = Counter(row["layout"]["status"] for row in occurrences)
    adapter_counts = Counter(row["adapter"]["status"] for row in occurrences)
    descriptor_counts = Counter(row["descriptor"]["status"] for row in occurrences)
    composition_counts = Counter(row["compositionStatus"] for row in occurrences)
    product_counts = Counter(row["productStatus"] for row in occurrences)
    legacy_counts = Counter(row["legacyGoldenReview"]["status"] for row in occurrences)
    horizontal_counts = Counter(row["horizontalV1Review"] for row in occurrences)
    exact_layout_counts = Counter(
        row["layout"]["status"]
        for row in occurrences
        if row["program"]["status"] in (
            "DXBC_OCCURRENCE_EXACT", "DXBC_OCCURRENCE_EXACT_UNTRANSLATED"
        )
    )
    _require(exact_layout_counts == {
        "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION": 20,
        "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS": 960,
        "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION": 1002,
        "SOURCE_NAMES_ONLY": 106,
        "UNRESOLVED": 68,
    }, "occurrence-exact Layout projection drifted")
    legacy_review_projection = sorted(
        (
            {"occurrenceId": row["occurrenceId"], **row["legacyGoldenReview"]}
            for row in occurrences
            if row["legacyGoldenReview"]["status"] != "NOT_RECORDED"
        ),
        key=lambda value: value["occurrenceId"],
    )
    legacy_review_projection_sha = canonical_sha256(legacy_review_projection)
    _require(
        legacy_review_projection_sha == EXPECTED_LEGACY_REVIEW_PROJECTION_SHA256,
        "legacy golden review receipt projection drifted",
    )

    for occurrence in occurrences:
        occurrence.pop("_documentName", None)
        occurrence.pop("_sourceProgramEvidence", None)
    occurrences.sort(key=lambda row: row["occurrenceId"])

    cohort_kind_counts = Counter(row["kind"] for row in cohorts)
    artifact: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": "STATIC_EVIDENCE_CANDIDATES_ONLY_NO_RUNTIME_OR_PRODUCT_ADMISSION",
        "inputs": tracker.rows(),
        "policies": {
            "candidateIdentity": "CANONICAL_CONTENT_SHA256",
            "occurrenceIdentity": "EFFECT_ASSET_ID_AND_STABLE_ELEMENT_ID_SHA256",
            "representativeProgramPromotion": "FORBIDDEN",
            "namedAbiTypedPacketPromotion": "FORBIDDEN",
            "runtimeVerification": "NOT_PROVIDED_BY_THIS_ARTIFACT",
            "productAdmission": "NOT_PROVIDED_BY_THIS_ARTIFACT",
            "userVisualAdmission": "USER_ONLY",
        },
        "summary": {
            "documentCount": len(documents),
            "occurrenceCount": len(occurrences),
            **authored_summary,
            **composition_summary,
            **catalog_summary,
            "programStatusCounts": dict(sorted(program_counts.items())),
            "layoutStatusCounts": dict(sorted(layout_counts.items())),
            "adapterStatusCounts": dict(sorted(adapter_counts.items())),
            "descriptorStatusCounts": dict(sorted(descriptor_counts.items())),
            "compositionStatusCounts": dict(sorted(composition_counts.items())),
            "productStatusCounts": dict(sorted(product_counts.items())),
            "legacyGoldenReviewStatusCounts": dict(sorted(legacy_counts.items())),
            "legacyGoldenReviewProjectionSha256": legacy_review_projection_sha,
            "horizontalV1ReviewStatusCounts": dict(sorted(horizontal_counts.items())),
            "occurrenceExactLayoutStatusCounts": dict(sorted(exact_layout_counts.items())),
            "cohortCount": len(cohorts),
            "cohortKindCounts": dict(sorted(cohort_kind_counts.items())),
            "cohortOccurrenceCount": sum(row["occurrenceCount"] for row in cohorts),
            "runtimeVerifiedCohortCount": 0,
            "runtimeDescriptorExpansionEligibleCohortCount": 0,
        },
        "carrierVariants": registries["carrier"].rows("carrierVariantId"),
        "programCandidates": registries["programCandidate"].rows("programCandidateId"),
        "programEvidence": registries["programEvidence"].rows("programEvidenceId"),
        "layoutCandidates": registries["layoutCandidate"].rows("layoutCandidateId"),
        "layoutEvidence": registries["layoutEvidence"].rows("layoutEvidenceId"),
        "adapterCandidates": registries["adapter"].rows("adapterCandidateId"),
        "descriptorVariants": registries["descriptor"].rows("descriptorVariantId"),
        "compositionVariants": composition_registry.rows(),
        "occurrences": occurrences,
        "cohorts": cohorts,
        "blockerBuckets": blocker_buckets,
        "canaries": canaries,
        "transaction": {
            "model": "PARSE_VALIDATE_STAGE_ATOMIC_REPLACE",
            "partialCommitAllowed": False,
        },
    }
    artifact["artifactSha256"] = canonical_sha256(artifact)
    validate_inventory(artifact)
    return artifact


def write_inventory(document: dict[str, Any], output_path: Path, repository_root: Path = REPOSITORY_ROOT) -> None:
    validate_inventory(document)
    payload = pretty_json_bytes(document)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        file_descriptor, temporary_name = tempfile.mkstemp(
            prefix=output_path.name + ".",
            suffix=".tmp",
            dir=output_path.parent,
        )
        temporary_path = Path(temporary_name)
        with os.fdopen(file_descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        staged = _decode_json(temporary_path.read_bytes(), temporary_path.name)
        _require(isinstance(staged, dict), "staged inventory root must be an object")
        validate_inventory(staged)
        validate_input_snapshot(staged, repository_root.resolve())
        os.replace(temporary_path, output_path)
        temporary_path = None
    finally:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()


def run(repository_root: Path, check: bool) -> int:
    document = build_inventory(repository_root)
    output_path = repository_root / OUTPUT_RELATIVE_PATH
    expected = pretty_json_bytes(document)
    if check:
        validate_input_snapshot(document, repository_root)
        if not output_path.is_file() or output_path.read_bytes() != expected:
            print("STALE: " + OUTPUT_RELATIVE_PATH.as_posix(), file=sys.stderr)
            return 1
        print("PASS: " + OUTPUT_RELATIVE_PATH.as_posix() + " is current")
        return 0
    write_inventory(document, output_path, repository_root)
    print(
        "WROTE: "
        + OUTPUT_RELATIVE_PATH.as_posix()
        + " ("
        + str(document["summary"]["occurrenceCount"])
        + " occurrences, "
        + str(document["summary"]["cohortCount"])
        + " static cohorts)"
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail unless the generated artifact is byte-current")
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    arguments = parser.parse_args(argv)
    try:
        return run(arguments.repository_root.resolve(), arguments.check)
    except (InventoryError, OSError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
