#!/usr/bin/env python3
"""Build the family-first character Effect restoration inventory.

The inventory is an executable contract, not a hand-maintained target list.  It
re-joins the current playable skill, animation cue, authored document, imported
source occurrence, Effect catalog, and published direct-runtime row.  Every
occurrence is admitted by a content-addressed
Carrier x Material(nullable) x Render x Composition tuple.

Only this inventory is written.  Source, authored, runtime, and animation data
are read-only inputs.  A build is parse -> validate -> stage -> atomic replace;
check mode performs the same build and validation without writing.
"""

from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import json
import math
import os
import re
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
OUTPUT_RELATIVE_PATH = (
    "Data/Effects/Contracts/character-effect-restoration-targets.v1.json"
)
SCHEMA = "lostark.character-effect-restoration-targets"
FORMAT_VERSION = 1

ARTIST_F_GOLDEN_DOCUMENT_SHA256 = (
    "b1cc0de1e22731c16740a1152364481a0aecf731fbaf4f704cf3afdd755c3925"
)
ARTIST_F_EXPECTED_KIND_COUNTS = {"decal": 2, "particle": 14, "trail": 1}
ARTIST_F_EXPECTED_BACKEND_COUNTS = {
    "artistVisualV4": 7,
    "localDecal": 2,
    "runtimeMaterialV2": 8,
}
ARTIST_F_EXPECTED_SOURCE_RECIPE_COUNT = 14
ARTIST_F_EXPECTED_SOURCE_MODULE_COUNT = 165

NULL_MATERIAL_CARRIER_KINDS = {
    "CAMERA_IMPULSE",
    "POINT_LIGHT",
    "SCREEN_POST",
}
MATERIAL_REQUIRED_CARRIER_KINDS = {
    "ANIMATED_MODEL_CUE",
    "ANIMATION_TRAIL",
    "AUTHORED_HISTORY_TRAIL",
    "CASCADE_RIBBON",
    "LOCAL_DECAL",
    "MESH_PARTICLE",
    "SCREEN_OVERLAY",
    "SOURCE_DECAL_PARTICLE",
    "SPLINE_SEGMENTED_RIBBON",
    "SPRITE_PARTICLE",
    "STANDALONE_MESH",
    "STANDALONE_SPRITE",
}
PROVENANCE_VALUES = {"DONOR_TRANSPLANT", "PROJECT_TUNED", "SOURCE_EXACT"}
EVIDENCE_VALUES = {"MISSING", "PARTIAL", "SEALED"}
EXECUTOR_VALUES = {
    "NATIVE_DXBC",
    "NONE",
    "TYPED_HLSL_SEMANTIC_REPLAY",
    "TYPED_PRESENTATION",
    "TYPED_SOURCE_RECONSTRUCTION",
}
ADMISSION_VALUES = {"ADMITTED", "AUTHORING_ONLY", "FAIL_CLOSED"}
USER_REVIEW_VALUES = {"APPROVED", "PENDING", "REJECTED"}
CHANNEL_VALUES = {"A", "B", "G", "R", "RG", "RGB", "RGBA"}
COLOR_SPACE_VALUES = {"linear", "srgb"}


class InventoryError(ValueError):
    """The source graph or generated inventory violates the G00 contract."""


@dataclass(frozen=True)
class TargetSpec:
    target_id: str
    character_class: str
    input_slot: str
    skill_id: int
    clip_name: str
    effect_asset_id: str
    authored_path: str
    animevents_path: str
    skillbindings_path: str
    selected_element_ids: tuple[str, ...] | None
    imported_path: str | None
    provenance: str
    evidence: str
    runtime_executor: str
    runtime_admission: str
    user_review: str
    restoration_state: str
    blockers: tuple[str, ...]
    product_occurrence_effect_asset_ids: tuple[str, ...] | None = None
    product_occurrence_authored_paths: tuple[str, ...] | None = None


TARGET_SPECS = (
    TargetSpec(
        target_id="target.artist.f.31470.golden",
        character_class="ARTIST",
        input_slot="F",
        skill_id=31470,
        clip_name="sdm_sk_onestroke",
        effect_asset_id="effect.artist.skill.31470.unified",
        authored_path=(
            "Data/Effects/Authored/"
            "effect.artist.skill.31470.unified.effect.json"
        ),
        animevents_path="Data/Animation/Authored/Artist/Artist.animevents",
        skillbindings_path=(
            "Data/Animation/Authored/Artist/Artist.skillbindings.json"
        ),
        selected_element_ids=None,
        imported_path=None,
        provenance="PROJECT_TUNED",
        evidence="SEALED",
        runtime_executor="TYPED_HLSL_SEMANTIC_REPLAY",
        runtime_admission="ADMITTED",
        user_review="APPROVED",
        restoration_state="GOLDEN_CONTROL",
        blockers=(),
    ),
    TargetSpec(
        target_id="target.dimensionmaster.a.2050210.makeflow",
        character_class="DIMENSIONMASTER",
        input_slot="A",
        skill_id=2050210,
        clip_name="pc_sp_m_00_sk_sk_willowrend",
        effect_asset_id="effect.dimensionmaster.skill.2050210.unified",
        authored_path=(
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050210.unified.effect.json"
        ),
        animevents_path=(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.animevents"
        ),
        skillbindings_path=(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.skillbindings.json"
        ),
        selected_element_ids=(
            "authored.source-particle.a58f7a015a0bab4c53a664fd",
            "authored.source-particle.53c11a9082f088279597515c",
            "authored.source-particle.83f005256fb59d87b99de92f",
            "authored.source-particle.ca6dc295e0267400d6968003",
        ),
        imported_path=(
            "Data/Effects/Imported/DimensionMaster/Converted/"
            "effect.dimensionmaster.skill.2050210.imported.effect.json"
        ),
        provenance="PROJECT_TUNED",
        evidence="PARTIAL",
        runtime_executor="TYPED_SOURCE_RECONSTRUCTION",
        runtime_admission="ADMITTED",
        user_review="PENDING",
        restoration_state="SOURCE_CARRIER_CANARY",
        blockers=(
            "SOURCE_EVENT_030_TIMING_SUBSTITUTED_0_60_TO_0_25",
            "SOURCE_TEXTURE_REGISTER_SAMPLER_ABI_UNRESOLVED",
            "SOURCE_PARENT_EQUATION_NOT_PIXEL_EXACT",
        ),
        product_occurrence_effect_asset_ids=tuple(
            f"effect.dimensionmaster.skill.2050210.a{index}.unified"
            for index in range(1, 5)
        ),
        product_occurrence_authored_paths=tuple(
            "Data/Effects/Authored/"
            f"effect.dimensionmaster.skill.2050210.a{index}.unified.effect.json"
            for index in range(1, 5)
        ),
    ),
    TargetSpec(
        target_id="target.warlord.w.17060.hemisphere",
        character_class="WARLORD",
        input_slot="W",
        skill_id=17060,
        clip_name="wgl_sk_firebullet",
        effect_asset_id="effect.warlord.skill.17060.unified",
        authored_path=(
            "Data/Effects/Authored/"
            "effect.warlord.skill.17060.unified.effect.json"
        ),
        animevents_path="Data/Animation/Authored/Warlord/Warlord.animevents",
        skillbindings_path=(
            "Data/Animation/Authored/Warlord/Warlord.skillbindings.json"
        ),
        selected_element_ids=(
            "authored.source-particle.635e153de978318aa446452e",
        ),
        imported_path=(
            "Data/Effects/Imported/Warlord/CurrentCombat/Converted/"
            "effect.warlord.skill.17060.imported.effect.json"
        ),
        provenance="SOURCE_EXACT",
        evidence="PARTIAL",
        runtime_executor="TYPED_SOURCE_RECONSTRUCTION",
        runtime_admission="ADMITTED",
        user_review="PENDING",
        restoration_state="SOURCE_CARRIER_CANARY",
        blockers=(
            "REQUIRED_SOURCE_TEXTURE_ASSET_IDS_UNRESOLVED",
            "SOURCE_TEXTURE_REGISTER_SAMPLER_ABI_UNRESOLVED",
            "SOURCE_PARENT_EQUATION_NOT_PIXEL_EXACT",
        ),
    ),
)


def _reject_non_finite(value: str) -> None:
    raise InventoryError(f"non-finite JSON number is forbidden: {value}")


def _object_without_duplicate_keys(
    pairs: list[tuple[str, Any]],
) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise InventoryError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8"),
            parse_constant=_reject_non_finite,
            object_pairs_hook=_object_without_duplicate_keys,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise InventoryError(f"cannot parse JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise InventoryError(f"JSON root must be an object: {path}")
    return value


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            allow_nan=False,
            indent=2,
        )
        + "\n"
    ).encode("utf-8")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise InventoryError(message)


def _require_string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise InventoryError(f"{label} must be a non-empty string")
    return value


def _require_sha256(value: Any, label: str) -> str:
    text = _require_string(value, label)
    if re.fullmatch(r"[0-9a-f]{64}", text) is None:
        raise InventoryError(f"{label} must be a lowercase SHA-256")
    return text


def _safe_relative_path(value: Any, label: str) -> str:
    text = _require_string(value, label).replace("\\", "/")
    pure = PurePosixPath(text)
    if pure.is_absolute() or ".." in pure.parts or ":" in text:
        raise InventoryError(f"{label} escapes its declared root: {text}")
    if str(pure) != text or text.startswith("/"):
        raise InventoryError(f"{label} is not a canonical relative path: {text}")
    return text


def _safe_asset_id(value: Any, label: str) -> str:
    text = _safe_relative_path(value, label)
    if not text.startswith("Effect/"):
        raise InventoryError(f"{label} must be Resources-relative Effect/: {text}")
    return text


def _find_unique(
    rows: Iterable[Any], predicate: Any, label: str
) -> dict[str, Any]:
    matches = [row for row in rows if isinstance(row, dict) and predicate(row)]
    if len(matches) != 1:
        raise InventoryError(f"{label} expected exactly one row, found {len(matches)}")
    return matches[0]


class InputTracker:
    def __init__(self, repository_root: Path) -> None:
        self.repository_root = repository_root
        self._rows: dict[str, dict[str, Any]] = {}

    def json(self, relative_path: str, role: str) -> dict[str, Any]:
        relative_path = _safe_relative_path(relative_path, f"input {role}")
        path = self.repository_root / Path(*PurePosixPath(relative_path).parts)
        value = read_json(path)
        self._record(
            relative_path,
            role,
            sha256_file(path),
            canonical_sha256(value),
        )
        return value

    def text(self, relative_path: str, role: str) -> str:
        relative_path = _safe_relative_path(relative_path, f"input {role}")
        path = self.repository_root / Path(*PurePosixPath(relative_path).parts)
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as error:
            raise InventoryError(f"cannot read text {path}: {error}") from error
        self._record(relative_path, role, sha256_file(path), None)
        return text

    def _record(
        self,
        relative_path: str,
        role: str,
        file_sha256: str,
        canonical_json_sha256: str | None,
    ) -> None:
        existing = self._rows.get(relative_path)
        if existing is None:
            self._rows[relative_path] = {
                "path": relative_path,
                "roles": [role],
                "fileSha256": file_sha256,
                "canonicalJsonSha256": canonical_json_sha256,
            }
            return
        if (
            existing["fileSha256"] != file_sha256
            or existing["canonicalJsonSha256"] != canonical_json_sha256
        ):
            raise InventoryError(f"input identity changed while building: {relative_path}")
        if role not in existing["roles"]:
            existing["roles"].append(role)
            existing["roles"].sort()

    def rows(self) -> list[dict[str, Any]]:
        return [copy.deepcopy(self._rows[key]) for key in sorted(self._rows)]


class VariantRegistry:
    def __init__(self, prefix: str, id_field: str) -> None:
        self.prefix = prefix
        self.id_field = id_field
        self._rows: dict[str, dict[str, Any]] = {}

    def add(self, payload: dict[str, Any]) -> str:
        variant_id = f"{self.prefix}.{canonical_sha256(payload)}"
        row = {self.id_field: variant_id, **copy.deepcopy(payload)}
        existing = self._rows.get(variant_id)
        if existing is not None and existing != row:
            raise InventoryError(f"variant hash collision: {variant_id}")
        self._rows[variant_id] = row
        return variant_id

    def rows(self) -> list[dict[str, Any]]:
        return [copy.deepcopy(self._rows[key]) for key in sorted(self._rows)]


def _renderer_type_data_classes(element: dict[str, Any]) -> list[str]:
    recipe = element.get("sourceRecipe") or {}
    classes = {
        str(module.get("className", "")).casefold()
        for module in recipe.get("modules", [])
        if isinstance(module, dict)
        and str(module.get("className", "")).casefold().startswith(
            "particlemoduletypedata"
        )
    }
    return sorted(classes)


def _carrier_kind(element: dict[str, Any]) -> str:
    kind = element.get("kind")
    resources = element.get("resources") or []
    has_mesh = any(
        isinstance(row, dict) and row.get("slotId") == "meshModel"
        for row in resources
    )
    type_data = _renderer_type_data_classes(element)
    if "particlemoduletypedataribbon" in type_data:
        return "CASCADE_RIBBON"
    if kind == "decal":
        return "LOCAL_DECAL"
    if kind == "trail":
        return "AUTHORED_HISTORY_TRAIL"
    if kind == "particle" and has_mesh:
        return "MESH_PARTICLE"
    if kind == "particle":
        return "SPRITE_PARTICLE"
    raise InventoryError(f"unsupported authored element kind: {kind!r}")


def _resource_asset(
    element: dict[str, Any], slot_id: str
) -> str | None:
    matches = [
        row.get("assetId")
        for row in element.get("resources", [])
        if isinstance(row, dict) and row.get("slotId") == slot_id
    ]
    if len(matches) > 1:
        raise InventoryError(f"duplicate {slot_id} resource on {element.get('id')}")
    return str(matches[0]) if matches else None


def _carrier_payload(element: dict[str, Any]) -> dict[str, Any]:
    recipe = element.get("sourceRecipe") or {}
    geometry_asset_id = _resource_asset(element, "meshModel")
    particle = (element.get("detail") or {}).get("particle") or {}
    return {
        "carrierKind": _carrier_kind(element),
        "authoringNodeKind": str(element.get("kind")),
        "geometryAssetId": geometry_asset_id,
        "rendererShape": str(recipe.get("rendererShape") or ""),
        "sourceTypeDataClasses": _renderer_type_data_classes(element),
        "localSpace": bool(particle.get("localSpace", False)),
        "sourceRecipeSha256": (
            canonical_sha256(recipe) if recipe.get("enabled") else None
        ),
    }


def _runtime_texture_lane(lane: dict[str, Any]) -> dict[str, Any]:
    asset_id = _safe_asset_id(lane.get("assetId"), "runtime texture lane assetId")
    channel = str(lane.get("sourceChannel") or "RGBA").upper()
    if channel not in CHANNEL_VALUES:
        raise InventoryError(f"unsupported runtime source channel: {channel}")
    sampler = copy.deepcopy(lane.get("sampler") or {})
    return {
        "laneId": _require_string(lane.get("laneId"), "texture laneId"),
        "semanticRole": _require_string(lane.get("role"), "texture semantic role"),
        "assetId": asset_id,
        "sourceObjectPath": None,
        "sourceChannel": channel,
        "channelEvidence": (
            "EXPLICIT_RUNTIME_PACKET"
            if lane.get("sourceChannel")
            else "RUNTIME_DEFAULT_FULL_VECTOR"
        ),
        "textureRegister": lane.get("textureRegister"),
        "samplerRegister": lane.get("samplerRegister"),
        "colorSpace": str(lane.get("colorSpace")),
        "sampler": {
            "filter": sampler.get("filter"),
            "addressU": sampler.get("addressU"),
            "addressV": sampler.get("addressV"),
            "addressW": sampler.get("addressW"),
            "mipLodBias": sampler.get("mipLodBias"),
            "maxAnisotropy": sampler.get("maxAnisotropy"),
            "comparison": sampler.get("comparison"),
            "borderColor": sampler.get("borderColor"),
            "minLod": sampler.get("minLod"),
            "maxLod": sampler.get("maxLod"),
            "evidence": "AUTHORED_RUNTIME_PACKET",
        },
        "storage": {
            "format": None,
            "alphaMode": "UNKNOWN_EXTERNAL_RUNTIME_INPUT",
        },
    }


def _source_texture_lanes(
    element: dict[str, Any]
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    profile = ((element.get("material") or {}).get("sourceProfile") or {})
    lanes: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []
    seen_assets: set[str] = set()
    used_lane_ids: set[str] = set()

    def unique_lane_id(seed: str) -> str:
        normalized = re.sub(r"[^a-z0-9_.-]+", "-", seed.casefold()).strip("-")
        candidate = normalized or "unnamed"
        suffix = 2
        while candidate in used_lane_ids:
            candidate = f"{normalized}.{suffix}"
            suffix += 1
        used_lane_ids.add(candidate)
        return candidate

    for texture in profile.get("textures", []):
        if not isinstance(texture, dict) or texture.get("name") == "umodel_dependency":
            continue
        raw_asset = str(texture.get("assetId") or "")
        asset_id = _safe_asset_id(raw_asset, "source texture assetId") if raw_asset else None
        if asset_id is None:
            unresolved.append(
                {
                    "semanticRole": _require_string(
                        texture.get("name"), "unresolved source texture semantic role"
                    ),
                    "sourceObjectPath": str(
                        texture.get("sourceObjectPath") or ""
                    ) or None,
                    "consumptionStatus": "NOT_BOUND_BY_CURRENT_AUTHORED_RESOURCE_PACKET",
                }
            )
            continue
        if asset_id:
            seen_assets.add(asset_id.casefold())
        lanes.append(
            {
                "laneId": unique_lane_id(f"source.{texture.get('name', '')}"),
                "semanticRole": _require_string(
                    texture.get("name"), "source texture semantic role"
                ),
                "assetId": asset_id,
                "sourceObjectPath": str(texture.get("sourceObjectPath") or "") or None,
                "sourceChannel": "RGBA",
                "channelEvidence": "SOURCE_PARAMETER_FULL_VECTOR_DEFAULT",
                "textureRegister": None,
                "samplerRegister": None,
                "colorSpace": str(texture.get("colorSpace")),
                "sampler": {
                    "filter": None,
                    "addressU": texture.get("addressU"),
                    "addressV": texture.get("addressV"),
                    "addressW": None,
                    "mipLodBias": None,
                    "maxAnisotropy": None,
                    "comparison": None,
                    "borderColor": None,
                    "minLod": None,
                    "maxLod": None,
                    "evidence": texture.get("samplingEvidence"),
                },
                "storage": {
                    "format": None,
                    "alphaMode": "UNKNOWN_EXTERNAL_RUNTIME_INPUT",
                },
            }
        )

    for resource in element.get("resources", []):
        if not isinstance(resource, dict) or resource.get("slotId") == "meshModel":
            continue
        asset_id = _safe_asset_id(
            resource.get("assetId"), "authored fallback resource assetId"
        )
        if asset_id.casefold() in seen_assets:
            continue
        seen_assets.add(asset_id.casefold())
        slot_id = _require_string(resource.get("slotId"), "resource slotId")
        lanes.append(
            {
                "laneId": unique_lane_id(f"authored-resource.{slot_id}"),
                "semanticRole": f"authored_resource.{slot_id}",
                "assetId": asset_id,
                "sourceObjectPath": None,
                "sourceChannel": "RGBA",
                "channelEvidence": "AUTHORED_RESOURCE_FULL_VECTOR_DEFAULT",
                "textureRegister": None,
                "samplerRegister": None,
                "colorSpace": "srgb",
                "sampler": {
                    "filter": None,
                    "addressU": None,
                    "addressV": None,
                    "addressW": None,
                    "mipLodBias": None,
                    "maxAnisotropy": None,
                    "comparison": None,
                    "borderColor": None,
                    "minLod": None,
                    "maxLod": None,
                    "evidence": "AUTHORED_RESOURCE_ONLY",
                },
                "storage": {
                    "format": None,
                    "alphaMode": "UNKNOWN_EXTERNAL_RUNTIME_INPUT",
                },
            }
        )
    return lanes, unresolved


def _emissive_policy(lanes: list[dict[str, Any]]) -> dict[str, Any]:
    resolved = [lane for lane in lanes if lane.get("assetId")]
    emissive = [
        lane
        for lane in resolved
        if "emissive" in str(lane.get("semanticRole", "")).casefold()
    ]
    if emissive:
        return {
            "kind": "TEXTURE_LANE",
            "laneId": emissive[0]["laneId"],
            "channel": "RGB",
            "constant": None,
        }
    preferred = [
        lane
        for lane in resolved
        if any(
            token in str(lane.get("semanticRole", "")).casefold()
            for token in ("base", "diff", "main", "color", "tex_main")
        )
    ]
    if preferred or resolved:
        lane = (preferred or resolved)[0]
        return {
            "kind": "BASE_LUMINANCE",
            "laneId": lane["laneId"],
            "channel": "RGB",
            "constant": None,
        }
    return {"kind": "NONE", "laneId": None, "channel": None, "constant": None}


def _material_payload(element: dict[str, Any]) -> dict[str, Any]:
    material = element.get("material") or {}
    execution = material.get("execution") or {}
    profile = material.get("sourceProfile") or {}
    if execution.get("enabled"):
        lanes = [_runtime_texture_lane(row) for row in execution.get("textureLanes", [])]
        unresolved_source_texture_evidence: list[dict[str, Any]] = []
        backend = _require_string(execution.get("backend"), "material execution backend")
        opcode = execution.get("opcode")
        formula = {
            "kind": "AUTHORED_RUNTIME_PACKET",
            "backend": backend,
            "opcode": opcode,
            "profileId": None,
            "recoveredEquationId": f"{backend}.opcode.{opcode}.v1",
        }
        parameter_masks = {
            "dynamicConsumedMask": execution.get("dynamicConsumedMask"),
            "dynamicSuppressedMask": execution.get("dynamicSuppressedMask"),
            "particleColorPolicy": execution.get("particleColorPolicy"),
            "particleColorConsumedMask": execution.get("particleColorConsumedMask"),
            "particleColorSuppressedMask": execution.get("particleColorSuppressedMask"),
            "scalarVectorInputConsumedMask": execution.get("inputConsumedMask"),
            "scalarVectorInputSuppressedMask": execution.get("inputSuppressedMask"),
            "vectorComponentConsumedMask": execution.get("vectorComponentConsumedMask"),
            "vectorComponentSuppressedMask": execution.get("vectorComponentSuppressedMask"),
            "staticSelectedMask": execution.get("staticSelectedMask"),
            "staticConsumedMask": execution.get("staticConsumedMask"),
            "staticSuppressedMask": execution.get("staticSuppressedMask"),
        }
        static_payload = {
            key: execution.get(key)
            for key in (
                "staticInputCount",
                "staticSelectedMask",
                "staticConsumedMask",
                "staticSuppressedMask",
            )
        }
        value_payload = {
            key: execution.get(key)
            for key in ("scalars", "vectors", "artistParameters", "colors")
        }
        coverage_equation = f"{backend}.opcode.{opcode}.coverage.v1"
        parent_material_path = None
    elif profile.get("enabled"):
        lanes, unresolved_source_texture_evidence = _source_texture_lanes(element)
        runtime_profile = _require_string(
            profile.get("runtimeShaderProfileId"), "source runtimeShaderProfileId"
        )
        formula = {
            "kind": "SOURCE_PROFILE",
            "backend": runtime_profile,
            "opcode": None,
            "profileId": _require_string(profile.get("profileId"), "source profileId"),
            "recoveredEquationId": f"{runtime_profile}.reconstructed.v1",
        }
        parameter_masks = {
            "dynamicConsumedMask": None,
            "dynamicSuppressedMask": None,
            "particleColorPolicy": None,
            "particleColorConsumedMask": None,
            "particleColorSuppressedMask": None,
            "scalarVectorInputConsumedMask": None,
            "scalarVectorInputSuppressedMask": None,
            "vectorComponentConsumedMask": None,
            "vectorComponentSuppressedMask": None,
            "staticSelectedMask": None,
            "staticConsumedMask": None,
            "staticSuppressedMask": None,
        }
        static_payload = profile.get("staticSwitches") or []
        value_payload = {
            "scalars": profile.get("scalars") or [],
            "vectors": profile.get("vectors") or [],
            "dynamicParameterSemantics": (
                profile.get("dynamicParameterSemantics") or []
            ),
        }
        coverage_equation = f"{runtime_profile}.coverage.v1"
        parent_material_path = _require_string(
            profile.get("parentMaterialPath"), "source parentMaterialPath"
        )
    else:
        raise InventoryError(f"element {element.get('id')} has no material execution")

    return {
        "templateId": _require_string(material.get("templateId"), "material templateId"),
        "childMaterialPath": _require_string(
            material.get("sourceMaterialPath"), "sourceMaterialPath"
        ),
        "parentMaterialPath": parent_material_path,
        "formulaExecutor": formula,
        "textureLanes": lanes,
        "unresolvedSourceTextureEvidence": unresolved_source_texture_evidence,
        "parameterMasks": parameter_masks,
        "staticSetSha256": canonical_sha256(static_payload),
        "parameterValuesSha256": canonical_sha256(value_payload),
        "coveragePolicy": {
            "kind": "RECOVERED_EQUATION",
            "laneId": None,
            "channel": None,
            "equationId": coverage_equation,
            "opaqueAlphaBehavior": "EQUATION_OWNS_COVERAGE",
        },
        "emissiveSourcePolicy": _emissive_policy(lanes),
        "missingLanePolicy": {
            "mode": "FAIL_CLOSED",
            "explicitConstants": [],
        },
    }


def _render_payload(
    element: dict[str, Any], carrier_kind: str
) -> dict[str, Any]:
    material = element.get("material") or {}
    execution = material.get("execution") or {}
    profile = material.get("sourceProfile") or {}
    if execution.get("enabled"):
        backend = execution.get("backend")
        opcode = execution.get("opcode")
        pass_index = execution.get("passIndex")
        render_state = copy.deepcopy(execution.get("renderState"))
        exact_state = True
    else:
        backend = profile.get("runtimeShaderProfileId")
        opcode = None
        pass_index = None
        render_state = None
        exact_state = False
    return {
        "allowedCarrierKinds": [carrier_kind],
        "renderProfile": _require_string(material.get("renderProfile"), "renderProfile"),
        "backend": backend,
        "opcode": opcode,
        "passIndex": pass_index,
        "renderState": render_state,
        "renderStateEvidenceSealed": exact_state,
        "sceneInputs": [],
        "outputRoles": ["RT0_COLOR"],
        "hdrStage": "EFFECT_SCENE_LINEAR_BEFORE_TONEMAP",
    }


def _composition_payload(
    spec: TargetSpec,
    cohort_id: str,
    event: dict[str, Any],
    element: dict[str, Any],
    element_order: int,
) -> dict[str, Any]:
    detail = element.get("detail") or {}
    return {
        "targetId": spec.target_id,
        "cohortId": cohort_id,
        "characterClass": spec.character_class,
        "inputSlot": spec.input_slot,
        "skillId": spec.skill_id,
        "clipName": spec.clip_name,
        "effectAssetId": spec.effect_asset_id,
        "animationCue": copy.deepcopy(event),
        "elementOrder": element_order,
        "visible": bool(element.get("visible", False)),
        "timingOwner": "AUTHORED_ELEMENT",
        "timing": copy.deepcopy(detail.get("timing") or {}),
        "attachment": copy.deepcopy(element.get("actionCueAttachment") or {}),
        "transformSha256": canonical_sha256((detail.get("transform") or {})),
        "stopPolicy": str(event["stop"]),
    }


ANIMEVENT_PATTERN = re.compile(
    r'^"(?P<clip>[^"]+)" EFFECT startms=(?P<startms>\d+) '
    r'payload="(?P<payload>[^"]+)" effectref=asset '
    r'(?P<tail>.*)$'
)
ANIMEVENT_FIELD_PATTERN = re.compile(r'(\w+)=("[^"]*"|\S+)')


def _animation_event(
    text: str, clip_name: str, effect_asset_id: str
) -> dict[str, Any]:
    matches: list[dict[str, Any]] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        match = ANIMEVENT_PATTERN.match(line)
        if match is None:
            continue
        values = match.groupdict()
        if values["clip"] == clip_name and values["payload"] == effect_asset_id:
            fields = {
                key: raw_value[1:-1]
                if raw_value.startswith('"') and raw_value.endswith('"')
                else raw_value
                for key, raw_value in ANIMEVENT_FIELD_PATTERN.findall(values["tail"])
            }
            for required in ("anchor", "follow", "stop"):
                _require_string(fields.get(required), f"animevent {required}")
            matches.append(
                {
                    "clipName": clip_name,
                    "effectAssetId": effect_asset_id,
                    "startMilliseconds": int(values["startms"]),
                    "anchor": fields["anchor"],
                    "follow": fields["follow"],
                    "stop": fields["stop"],
                    "typedOptions": {
                        key: fields[key]
                        for key in sorted(fields)
                        if key not in {
                            "anchor", "follow", "stop",
                            "px", "py", "pz", "rx", "ry", "rz",
                            "sx", "sy", "sz",
                        }
                    },
                    "sourceLine": line_number,
                }
            )
    if len(matches) != 1:
        raise InventoryError(
            f"animevent {clip_name}/{effect_asset_id} expected one row, found {len(matches)}"
        )
    return matches[0]


def _source_element(
    authored_element: dict[str, Any], imported: dict[str, Any] | None
) -> dict[str, Any] | None:
    if imported is None:
        return None
    source_node = _require_string(
        authored_element.get("sourceNode"), "source-backed authored sourceNode"
    )
    marker = "|element:"
    if marker not in source_node:
        raise InventoryError(f"sourceNode has no exact element identity: {source_node}")
    imported_id = source_node.split(marker, 1)[1]
    return _find_unique(
        imported.get("elements", []),
        lambda row: row.get("id") == imported_id,
        f"imported source element {imported_id}",
    )


def _occurrence_provenance_and_evidence(
    spec: TargetSpec,
    authored_element: dict[str, Any],
    source_element: dict[str, Any] | None,
) -> tuple[str, str]:
    """Resolve mixed current/source lineage without flattening a whole target.

    Dimension A deliberately keeps the visible project-retimed 0.25 occurrence
    beside three source-timed occurrences.  Its target-level defaults therefore
    cannot truthfully describe every row.
    """

    if spec.skill_id != 2050210:
        return spec.provenance, spec.evidence
    _require(source_element is not None, "Dimension A occurrence lost source lineage")
    authored_start = float(
        (((authored_element.get("detail") or {}).get("timing") or {}).get(
            "startDelaySeconds", 0
        ))
    )
    source_start = float(
        (((source_element.get("detail") or {}).get("timing") or {}).get(
            "startDelaySeconds", 0
        ))
    )
    provenance = (
        "SOURCE_EXACT"
        if math.isclose(authored_start, source_start, rel_tol=0, abs_tol=1e-7)
        else "PROJECT_TUNED"
    )
    return provenance, "PARTIAL"


def _occurrence_blockers(
    spec: TargetSpec,
    provenance: str,
    published: bool,
) -> list[str]:
    blockers = list(spec.blockers)
    timing_blocker = "SOURCE_EVENT_030_TIMING_SUBSTITUTED_0_60_TO_0_25"
    if spec.skill_id == 2050210 and provenance == "SOURCE_EXACT":
        blockers = [blocker for blocker in blockers if blocker != timing_blocker]
    if not published:
        blockers.append("PUBLISHED_DIRECT_DOCUMENT_STALE_FOR_OCCURRENCE")
    return blockers


def _validate_known_canary(
    spec: TargetSpec, document: dict[str, Any], elements: list[dict[str, Any]]
) -> None:
    if spec.skill_id == 31470:
        document_sha = canonical_sha256(document)
        _require(
            document_sha == ARTIST_F_GOLDEN_DOCUMENT_SHA256,
            "Artist F golden authored document identity changed",
        )
        kind_counts = dict(sorted(collections.Counter(
            str(row.get("kind")) for row in elements
        ).items()))
        _require(
            kind_counts == ARTIST_F_EXPECTED_KIND_COUNTS,
            f"Artist F kind denominator changed: {kind_counts}",
        )
        backend_counts = dict(sorted(collections.Counter(
            str(((row.get("material") or {}).get("execution") or {}).get("backend"))
            for row in elements
        ).items()))
        _require(
            backend_counts == ARTIST_F_EXPECTED_BACKEND_COUNTS,
            f"Artist F backend denominator changed: {backend_counts}",
        )
        recipe_count = sum(
            bool((row.get("sourceRecipe") or {}).get("enabled")) for row in elements
        )
        module_count = sum(
            len((row.get("sourceRecipe") or {}).get("modules", [])) for row in elements
        )
        _require(
            recipe_count == ARTIST_F_EXPECTED_SOURCE_RECIPE_COUNT,
            f"Artist F source recipe denominator changed: {recipe_count}",
        )
        _require(
            module_count == ARTIST_F_EXPECTED_SOURCE_MODULE_COUNT,
            f"Artist F source module denominator changed: {module_count}",
        )
        _require(
            all(
                bool((((row.get("material") or {}).get("execution") or {}).get("enabled")))
                for row in elements
            ),
            "Artist F contains an element without material execution",
        )
        return

    if spec.skill_id == 2050210:
        expected = (
            (
                "authored.source-particle.a58f7a015a0bab4c53a664fd",
                "source-event-030",
                0.25,
            ),
            (
                "authored.source-particle.53c11a9082f088279597515c",
                "source-event-030",
                0.6,
            ),
            (
                "authored.source-particle.83f005256fb59d87b99de92f",
                "source-event-045",
                0.9,
            ),
            (
                "authored.source-particle.ca6dc295e0267400d6968003",
                "source-event-060",
                1.3,
            ),
        )
        _require(len(elements) == len(expected), "Dimension A must select four MakeFlow rows")
        for element, (expected_id, source_suffix, expected_start) in zip(
            elements, expected
        ):
            _require(element.get("id") == expected_id, "Dimension A MakeFlow order changed")
            _require(
                str(element.get("sourceNode") or "").endswith(source_suffix),
                "Dimension A MakeFlow source event changed",
            )
            actual_start = float(
                (((element.get("detail") or {}).get("timing") or {}).get(
                    "startDelaySeconds", 0
                ))
            )
            _require(
                math.isclose(actual_start, expected_start, rel_tol=0, abs_tol=1e-7),
                "Dimension A MakeFlow timing changed",
            )
            _require(
                _resource_asset(element, "meshModel")
                == "Effect/DimensionMaster/Meshes/fm_h_swing_05.wmodel",
                "Dimension A MakeFlow canary mesh changed",
            )
            _require(
                (element.get("material") or {}).get("sourceMaterialPath")
                == "fx_m_mi_k_00.fx_mi.fx_k_me_makeflow_03_05_tr",
                "Dimension A MakeFlow child material changed",
            )
            _require(
                ((element.get("material") or {}).get("sourceProfile") or {}).get(
                    "parentMaterialPath"
                )
                == "fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr",
                "Dimension A MakeFlow parent material changed",
            )
        return
    element = elements[0]
    if spec.skill_id == 17060:
        _require(
            _resource_asset(element, "meshModel")
            == "Effect/Warlord/Meshes/FX_SM_00/fm_a_hemisphere_012.wmodel",
            "Warlord W hemisphere canary mesh changed",
        )
        _require(
            (element.get("material") or {}).get("sourceMaterialPath")
            == "fx_m_mi_05.fx_m.fx_a_me_panning_02_ad",
            "Warlord W hemisphere child material changed",
        )


def _runtime_catalog_row(
    runtime_catalog: dict[str, Any], effect_asset_id: str
) -> dict[str, Any]:
    row = _find_unique(
        runtime_catalog.get("effects", []),
        lambda item: item.get("effectAssetId") == effect_asset_id,
        f"runtime catalog {effect_asset_id}",
    )
    _require(
        row.get("payloadKind") == "DIRECT_AUTHORED_DOCUMENT_V13"
        and row.get("authoringFormatVersion") == 13,
        f"runtime catalog row is not direct v13: {effect_asset_id}",
    )
    return row


def build_inventory(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    tracker = InputTracker(repository_root)
    player_skills = tracker.json("Data/Balance/PlayerSkills.json", "PLAYABLE_SKILLS")
    effect_catalog = tracker.json("Data/Effects/EffectCatalog.json", "AUTHORING_CATALOG")
    runtime_catalog_path = "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
    runtime_catalog = tracker.json(runtime_catalog_path, "DIRECT_RUNTIME_CATALOG")

    carriers = VariantRegistry("carrier", "carrierVariantId")
    materials = VariantRegistry("material", "materialVariantId")
    renders = VariantRegistry("render", "renderVariantId")
    compositions = VariantRegistry("composition", "compositionVariantId")
    cohorts: list[dict[str, Any]] = []
    occurrences: list[dict[str, Any]] = []
    targets: list[dict[str, Any]] = []
    substitution_duplication_receipts: list[dict[str, Any]] = []

    for spec in TARGET_SPECS:
        skill = _find_unique(
            player_skills.get("skills", []),
            lambda row: row.get("skillId") == spec.skill_id
            and row.get("characterClass") == spec.character_class
            and row.get("inputSlot") == spec.input_slot,
            f"playable skill {spec.character_class}/{spec.input_slot}",
        )
        bindings = tracker.json(spec.skillbindings_path, f"{spec.target_id}:SKILL_BINDING")
        binding_rows = bindings.get("skills", bindings.get("bindings", []))
        binding = _find_unique(
            binding_rows,
            lambda row: row.get("skillId") == spec.skill_id,
            f"skill binding {spec.skill_id}",
        )
        _require(
            binding.get("clips") == [spec.clip_name],
            f"skill binding clips changed for {spec.skill_id}: {binding.get('clips')}",
        )
        animevents = tracker.text(spec.animevents_path, f"{spec.target_id}:ANIMATION_CUE")

        catalog_row = _find_unique(
            effect_catalog.get("effects", []),
            lambda row: row.get("effectAssetId") == spec.effect_asset_id,
            f"authoring catalog {spec.effect_asset_id}",
        )
        expected_authoring_path = spec.authored_path.removeprefix("Data/")
        _require(
            catalog_row.get("authoringPath") == expected_authoring_path,
            f"catalog authoringPath changed for {spec.effect_asset_id}",
        )
        document = tracker.json(spec.authored_path, f"{spec.target_id}:CURRENT_AUTHORED")
        _require(
            document.get("schema") == "lostark.effect-authoring"
            and document.get("version") == 13
            and document.get("effectAssetId") == spec.effect_asset_id,
            f"authored document contract changed: {spec.authored_path}",
        )
        all_elements = document.get("elements", [])
        _require(isinstance(all_elements, list), f"elements is not an array: {spec.authored_path}")
        if spec.selected_element_ids is None:
            selected = [row for row in all_elements if isinstance(row, dict)]
        else:
            selected = [
                _find_unique(
                    all_elements,
                    lambda row, element_id=element_id: row.get("id") == element_id,
                    f"selected authored element {element_id}",
                )
                for element_id in spec.selected_element_ids
            ]
        _require(selected, f"target has no selected elements: {spec.target_id}")
        _validate_known_canary(spec, document, selected)

        imported = (
            tracker.json(spec.imported_path, f"{spec.target_id}:IMMUTABLE_SOURCE")
            if spec.imported_path
            else None
        )
        product_effect_ids = (
            spec.product_occurrence_effect_asset_ids
            if spec.product_occurrence_effect_asset_ids is not None
            else tuple(spec.effect_asset_id for _ in selected)
        )
        product_authored_paths = (
            spec.product_occurrence_authored_paths
            if spec.product_occurrence_authored_paths is not None
            else tuple(spec.authored_path for _ in selected)
        )
        _require(
            len(product_effect_ids) == len(selected)
            and len(product_authored_paths) == len(selected),
            f"Product occurrence mapping cardinality changed: {spec.target_id}",
        )
        product_bindings: list[dict[str, Any]] = []
        product_cache: dict[tuple[str, str], dict[str, Any]] = {}
        for product_effect_id, product_authored_path in zip(
            product_effect_ids, product_authored_paths
        ):
            cache_key = (product_effect_id, product_authored_path)
            product_binding = product_cache.get(cache_key)
            if product_binding is None:
                event = _animation_event(
                    animevents, spec.clip_name, product_effect_id
                )
                product_catalog_row = _find_unique(
                    effect_catalog.get("effects", []),
                    lambda row, effect_id=product_effect_id: row.get(
                        "effectAssetId"
                    )
                    == effect_id,
                    f"authoring catalog {product_effect_id}",
                )
                expected_product_path = product_authored_path.removeprefix(
                    "Data/"
                )
                _require(
                    product_catalog_row.get("authoringPath")
                    == expected_product_path,
                    f"catalog authoringPath changed for {product_effect_id}",
                )
                product_document = tracker.json(
                    product_authored_path,
                    f"{spec.target_id}:PRODUCT_OCCURRENCE_AUTHORED",
                )
                _require(
                    product_document.get("schema") == "lostark.effect-authoring"
                    and product_document.get("version") == 13
                    and product_document.get("effectAssetId")
                    == product_effect_id,
                    f"Product occurrence document changed: {product_effect_id}",
                )
                runtime_row = _runtime_catalog_row(
                    runtime_catalog, product_effect_id
                )
                sealed_relative = (
                    "Client/Bin/DataFiles/Effect/" + _safe_relative_path(
                        runtime_row.get("authoredDocumentPath"),
                        f"runtime authored path {product_effect_id}",
                    )
                )
                sealed_document = tracker.json(
                    sealed_relative,
                    f"{spec.target_id}:PUBLISHED_PRODUCT_OCCURRENCE",
                )
                _require(
                    sealed_document.get("effectAssetId") == product_effect_id,
                    "published Product occurrence identity changed: "
                    f"{product_effect_id}",
                )
                product_elements = product_document.get("elements")
                sealed_elements = sealed_document.get("elements")
                _require(
                    isinstance(product_elements, list)
                    and isinstance(sealed_elements, list),
                    f"Product occurrence elements changed: {product_effect_id}",
                )
                product_binding = {
                    "effectAssetId": product_effect_id,
                    "authoredPath": product_authored_path,
                    "event": event,
                    "elements": product_elements,
                    "sealedRelative": sealed_relative,
                    "sealedDocument": sealed_document,
                    "sealedElements": sealed_elements,
                }
                product_cache[cache_key] = product_binding
            product_bindings.append(product_binding)

        cohort_id = f"cohort.{spec.target_id}"
        cohort_occurrences: list[str] = []
        target_source_rows: list[dict[str, Any]] = []
        for element, product_binding in zip(selected, product_bindings):
            element_id = _require_string(element.get("id"), "authored element id")
            authored_order = all_elements.index(element)
            event = product_binding["event"]
            product_elements = product_binding["elements"]
            product_element = _find_unique(
                product_elements,
                lambda row: row.get("id") == element_id,
                f"Product occurrence element {element_id}",
            )
            product_order = product_elements.index(product_element)
            source = _source_element(element, imported)
            occurrence_provenance, occurrence_evidence = (
                _occurrence_provenance_and_evidence(spec, element, source)
            )
            published_matches = [
                row
                for row in product_binding["sealedElements"]
                if isinstance(row, dict) and row.get("id") == element_id
            ]
            _require(
                len(published_matches) <= 1,
                f"published direct document duplicates stable ID: {element_id}",
            )
            published_element = published_matches[0] if published_matches else None
            if (
                published_element is not None
                and canonical_sha256(published_element)
                != canonical_sha256(product_element)
            ):
                published_element = None
            runtime_admission = (
                spec.runtime_admission
                if published_element is not None
                else "AUTHORING_ONLY"
            )
            product_join_status = (
                "CLOSED" if published_element is not None else "AUTHORED_NOT_PUBLISHED"
            )
            carrier_payload = _carrier_payload(element)
            carrier_id = carriers.add(carrier_payload)
            material_payload = _material_payload(element)
            material_id = materials.add(material_payload)
            render_id = renders.add(
                _render_payload(element, carrier_payload["carrierKind"])
            )
            composition_id = compositions.add(
                _composition_payload(
                    spec, cohort_id, event, product_element, product_order
                )
            )
            family_tuple = {
                "carrierVariantId": carrier_id,
                "materialVariantId": material_id,
                "renderVariantId": render_id,
                "compositionVariantId": composition_id,
            }
            occurrence_id = (
                "occurrence."
                + canonical_sha256(
                    {"effectAssetId": spec.effect_asset_id, "stableId": element_id}
                )
            )
            field_receipts = [
                {
                    "fieldGroup": "CURRENT_DIRECT_AUTHORED_CLOSURE",
                    "provenance": occurrence_provenance,
                    "evidence": occurrence_evidence,
                    "receiptPath": spec.authored_path,
                    "receiptSha256": canonical_sha256(element),
                }
            ]
            source_identity = None
            if source is not None and spec.imported_path is not None:
                source_identity = {
                    "path": spec.imported_path,
                    "stableId": source.get("id"),
                    "elementSha256": canonical_sha256(source),
                }
                field_receipts.insert(
                    0,
                    {
                        "fieldGroup": "SOURCE_CARRIER_AND_OCCURRENCE",
                        "provenance": "SOURCE_EXACT",
                        "evidence": "SEALED",
                        "receiptPath": spec.imported_path,
                        "receiptSha256": canonical_sha256(source),
                    },
                )
            execution = ((element.get("material") or {}).get("execution") or {})
            shader_kind = (
                "TYPED_RUNTIME_PACKET"
                if execution.get("enabled")
                else "SOURCE_GRAPH_WITHOUT_NATIVE_SHADER_ABI"
            )
            occurrence = {
                "occurrenceId": occurrence_id,
                "authoredNode": {
                    "kind": "ELEMENT",
                    "effectAssetId": spec.effect_asset_id,
                    "stableId": element_id,
                    "sourceNode": str(element.get("sourceNode") or "") or None,
                    "authoredPath": spec.authored_path,
                    "authoredElementSha256": canonical_sha256(element),
                    "sourceIdentity": source_identity,
                },
                "substitutionDuplicationReceiptId": None,
                "familyTuple": family_tuple,
                "executionClosureId": canonical_sha256(family_tuple),
                "fieldReceipts": field_receipts,
                "provenance": occurrence_provenance,
                "evidence": occurrence_evidence,
                "shaderEvidence": {
                    "kind": shader_kind,
                    "receiptPath": spec.authored_path,
                    "receiptSha256": canonical_sha256(
                        execution if execution.get("enabled") else (element.get("material") or {}).get("sourceProfile")
                    ),
                },
                "runtimeExecutor": spec.runtime_executor,
                "runtimeAdmission": runtime_admission,
                "productJoin": {
                    "status": product_join_status,
                    "effectAssetId": product_binding["effectAssetId"],
                    "authoringPath": product_binding["authoredPath"],
                    "authoredElementSha256": canonical_sha256(
                        product_element
                    ),
                    "authoringCatalogPath": "Data/Effects/EffectCatalog.json",
                    "animationCuePath": spec.animevents_path,
                    "runtimeCatalogPath": runtime_catalog_path,
                    "publishedDocumentPath": product_binding["sealedRelative"],
                    "publishedDocumentSha256": canonical_sha256(
                        product_binding["sealedDocument"]
                    ),
                    "publishedElementSha256": (
                        canonical_sha256(published_element)
                        if published_element is not None
                        else None
                    ),
                },
                "materialInputs": {
                    "emissiveIntensity": float(
                        (((element.get("detail") or {}).get("color") or {}).get(
                            "emissiveIntensity", 0
                        ))
                    )
                },
                "userReview": spec.user_review,
                "failureScope": "THIS_OCCURRENCE",
                "blockers": _occurrence_blockers(
                    spec,
                    occurrence_provenance,
                    published_element is not None,
                ),
            }
            occurrences.append(occurrence)
            cohort_occurrences.append(occurrence_id)
            if source is not None:
                target_source_rows.append(
                    {
                        "occurrence": occurrence,
                        "sourceNode": element.get("sourceNode"),
                        "sourceStableId": source.get("id"),
                        "sourceStartSeconds": float(
                            (((source.get("detail") or {}).get("timing") or {}).get(
                                "startDelaySeconds", 0
                            ))
                        ),
                        "authoredStartSeconds": float(
                            (((element.get("detail") or {}).get("timing") or {}).get(
                                "startDelaySeconds", 0
                            ))
                        ),
                        "productLocalStartSeconds": float(
                            (((product_element.get("detail") or {}).get(
                                "timing"
                            ) or {}).get("startDelaySeconds", 0))
                        ),
                        "cueSourceStartMilliseconds": int(
                            event["startMilliseconds"]
                        ),
                    }
                )

        source_groups: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
        for source_row in target_source_rows:
            source_groups[str(source_row["sourceNode"])].append(source_row)
        for source_node, source_rows in sorted(source_groups.items()):
            has_substitution = any(
                not math.isclose(
                    row["sourceStartSeconds"],
                    row["authoredStartSeconds"],
                    rel_tol=0,
                    abs_tol=1e-7,
                )
                for row in source_rows
            )
            if len(source_rows) == 1 and not has_substitution:
                continue
            source_stable_ids = {row["sourceStableId"] for row in source_rows}
            source_starts = {row["sourceStartSeconds"] for row in source_rows}
            _require(
                len(source_stable_ids) == 1 and len(source_starts) == 1,
                f"source lineage group is ambiguous: {source_node}",
            )
            decisions = []
            for row in source_rows:
                substituted = not math.isclose(
                    row["sourceStartSeconds"],
                    row["authoredStartSeconds"],
                    rel_tol=0,
                    abs_tol=1e-7,
                )
                decisions.append(
                    {
                        "occurrenceId": row["occurrence"]["occurrenceId"],
                        "currentStableId": row["occurrence"]["authoredNode"]["stableId"],
                        "authoredStartSeconds": row["authoredStartSeconds"],
                        "productLocalStartSeconds": row[
                            "productLocalStartSeconds"
                        ],
                        "cueSourceStartMilliseconds": row[
                            "cueSourceStartMilliseconds"
                        ],
                        "timingProvenance": (
                            "PROJECT_TUNED" if substituted else "SOURCE_EXACT"
                        ),
                        "sourceOccurrenceDisposition": (
                            "EVIDENCE_ONLY_SUBSTITUTED_NOT_ADMITTED"
                            if substituted
                            else "ADMITTED_SOURCE_OCCURRENCE"
                        ),
                    }
                )
            receipt_payload = {
                "targetId": spec.target_id,
                "currentEffectAssetId": spec.effect_asset_id,
                "sourceEffectAssetId": imported.get("effectAssetId"),
                "sourceStableId": next(iter(source_stable_ids)),
                "sourceNode": source_node,
                "sourceStartSeconds": next(iter(source_starts)),
                "occurrenceDecisions": decisions,
                "authorizationScope": "EXACT_OCCURRENCE_LIST_ONLY",
                "fieldProvenance": {
                    "carrier": "SOURCE_EXACT",
                    "materialIdentity": "SOURCE_EXACT",
                },
                "targetCardinality": (
                    4 if spec.skill_id == 2050210 else len(selected)
                ),
            }
            receipt_id = "substitution-duplication." + canonical_sha256(
                receipt_payload
            )
            substitution_duplication_receipts.append(
                {
                    "substitutionDuplicationReceiptId": receipt_id,
                    **receipt_payload,
                }
            )
            for row in source_rows:
                row["occurrence"]["substitutionDuplicationReceiptId"] = receipt_id

        cohorts.append(
            {
                "cohortId": cohort_id,
                "targetId": spec.target_id,
                "orderedOccurrenceIds": cohort_occurrences,
                "failurePolicy": "ISOLATE_FAILED_OCCURRENCE_KEEP_VALID_SIBLINGS",
            }
        )
        targets.append(
            {
                "targetId": spec.target_id,
                "characterClass": spec.character_class,
                "inputSlot": spec.input_slot,
                "skillId": spec.skill_id,
                "clipName": spec.clip_name,
                "effectAssetId": spec.effect_asset_id,
                "authoredPath": spec.authored_path,
                "cohortIds": [cohort_id],
                "selectedOccurrenceCount": len(cohort_occurrences),
                "currentAuthoredElementCount": len(all_elements),
                "restorationState": spec.restoration_state,
                "userReview": spec.user_review,
            }
        )

    summary = {
        "targetCount": len(targets),
        "cohortCount": len(cohorts),
        "occurrenceCount": len(occurrences),
        "carrierVariantCount": len(carriers.rows()),
        "materialVariantCount": len(materials.rows()),
        "renderVariantCount": len(renders.rows()),
        "compositionVariantCount": len(compositions.rows()),
        "substitutionDuplicationReceiptCount": len(
            substitution_duplication_receipts
        ),
        "runtimeAdmissionCounts": dict(sorted(collections.Counter(
            row["runtimeAdmission"] for row in occurrences
        ).items())),
        "userReviewCounts": dict(sorted(collections.Counter(
            row["userReview"] for row in occurrences
        ).items())),
    }
    inventory = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "inputs": tracker.rows(),
        "policies": {
            "executionClosure": {
                "algorithm": "SHA-256",
                "encoding": "UTF-8",
                "canonicalJson": "SORTED_KEYS_NO_WHITESPACE_UTF8",
                "tupleFields": [
                    "carrierVariantId",
                    "materialVariantId",
                    "renderVariantId",
                    "compositionVariantId",
                ],
                "variantIdsAreContentAddressed": True,
            },
            "nullMaterialCarrierKinds": sorted(NULL_MATERIAL_CARRIER_KINDS),
            "materialRequiredCarrierKinds": sorted(MATERIAL_REQUIRED_CARRIER_KINDS),
            "sourceTypeDataProjection": {
                "particlemoduletypedataribbon": ["CASCADE_RIBBON"],
                "particlemoduletypedatamesh": ["MESH_PARTICLE"],
            },
            "rgbaLaneContract": {
                "requiredFields": [
                    "laneId",
                    "semanticRole",
                    "assetId",
                    "sourceChannel",
                    "textureRegister",
                    "samplerRegister",
                    "colorSpace",
                    "sampler",
                ],
                "implicitDefaultSrvForbidden": True,
                "coverageMustNameLaneChannelOrEquation": True,
                "emissiveMustNameTextureBaseLuminanceConstantOrNone": True,
                "missingLaneModes": ["EXPLICIT_CONSTANT", "FAIL_CLOSED"],
            },
        },
        "summary": summary,
        "carrierVariants": carriers.rows(),
        "materialVariants": materials.rows(),
        "renderVariants": renders.rows(),
        "compositionVariants": compositions.rows(),
        "substitutionDuplicationReceipts": substitution_duplication_receipts,
        "cohorts": cohorts,
        "occurrences": occurrences,
        "targets": targets,
        "transaction": {
            "loadOrder": ["parse", "validate", "stage", "commit"],
            "commitMode": "ATOMIC_REPLACE_AFTER_FULL_VALIDATION",
            "failurePolicy": "PRESERVE_PREVIOUS_OUTPUT",
        },
        "artifactSha256": "",
    }
    inventory["artifactSha256"] = _artifact_sha256(inventory)
    validate_inventory(inventory)
    return inventory


def _artifact_sha256(inventory: dict[str, Any]) -> str:
    payload = copy.deepcopy(inventory)
    payload.pop("artifactSha256", None)
    return canonical_sha256(payload)


def _index_unique(
    rows: Any, id_field: str, label: str
) -> dict[str, dict[str, Any]]:
    if not isinstance(rows, list):
        raise InventoryError(f"{label} must be an array")
    index: dict[str, dict[str, Any]] = {}
    for row_number, row in enumerate(rows):
        if not isinstance(row, dict):
            raise InventoryError(f"{label}[{row_number}] must be an object")
        row_id = _require_string(row.get(id_field), f"{label}[{row_number}].{id_field}")
        if row_id in index:
            raise InventoryError(f"duplicate {id_field}: {row_id}")
        index[row_id] = row
    return index


def _validate_variant_identity(
    row: dict[str, Any], id_field: str, prefix: str
) -> None:
    variant_id = row[id_field]
    payload = copy.deepcopy(row)
    del payload[id_field]
    expected = f"{prefix}.{canonical_sha256(payload)}"
    if variant_id != expected:
        raise InventoryError(f"{id_field} content hash mismatch: {variant_id}")


def _validate_sampler(sampler: Any, label: str, admitted: bool) -> None:
    if not isinstance(sampler, dict):
        raise InventoryError(f"{label} must be an object")
    required = {
        "filter",
        "addressU",
        "addressV",
        "addressW",
        "mipLodBias",
        "maxAnisotropy",
        "comparison",
        "borderColor",
        "minLod",
        "maxLod",
        "evidence",
    }
    if set(sampler) != required:
        raise InventoryError(f"{label} fields changed: {sorted(sampler)}")
    if admitted and any(sampler[key] is None for key in required - {"evidence"}):
        raise InventoryError(f"{label} is incomplete for an admitted occurrence")


def _validate_material_variant(
    material: dict[str, Any], admitted: bool
) -> None:
    formula = material.get("formulaExecutor") or {}
    requires_sealed_runtime_packet = (
        admitted and formula.get("kind") == "AUTHORED_RUNTIME_PACKET"
    )
    unresolved_evidence = material.get("unresolvedSourceTextureEvidence")
    if not isinstance(unresolved_evidence, list):
        raise InventoryError(
            "material unresolvedSourceTextureEvidence must be an array"
        )
    for row in unresolved_evidence:
        if (
            not isinstance(row, dict)
            or set(row)
            != {"semanticRole", "sourceObjectPath", "consumptionStatus"}
            or row.get("consumptionStatus")
            != "NOT_BOUND_BY_CURRENT_AUTHORED_RESOURCE_PACKET"
        ):
            raise InventoryError("unresolved source texture evidence is invalid")
        _require_string(row.get("semanticRole"), "unresolved semanticRole")
    lanes = material.get("textureLanes")
    if not isinstance(lanes, list):
        raise InventoryError("material textureLanes must be an array")
    lane_index: dict[str, dict[str, Any]] = {}
    for lane_number, lane in enumerate(lanes):
        if not isinstance(lane, dict):
            raise InventoryError(f"textureLanes[{lane_number}] must be an object")
        required = {
            "laneId",
            "semanticRole",
            "assetId",
            "sourceObjectPath",
            "sourceChannel",
            "channelEvidence",
            "textureRegister",
            "samplerRegister",
            "colorSpace",
            "sampler",
            "storage",
        }
        if set(lane) != required:
            raise InventoryError(f"texture lane fields changed: {sorted(lane)}")
        lane_id = _require_string(lane.get("laneId"), "texture laneId")
        if lane_id in lane_index:
            raise InventoryError(f"duplicate texture laneId: {lane_id}")
        lane_index[lane_id] = lane
        _require_string(lane.get("semanticRole"), f"{lane_id}.semanticRole")
        asset_id = lane.get("assetId")
        if asset_id is not None:
            _safe_asset_id(asset_id, f"{lane_id}.assetId")
        if lane.get("sourceChannel") not in CHANNEL_VALUES:
            raise InventoryError(f"{lane_id}.sourceChannel is invalid")
        if lane.get("colorSpace") not in COLOR_SPACE_VALUES:
            raise InventoryError(f"{lane_id}.colorSpace is invalid")
        for field in ("textureRegister", "samplerRegister"):
            value = lane.get(field)
            if value is not None and (not isinstance(value, int) or value < 0):
                raise InventoryError(f"{lane_id}.{field} is invalid")
            if requires_sealed_runtime_packet and value is None:
                raise InventoryError(f"{lane_id}.{field} is unresolved for admission")
        _validate_sampler(
            lane.get("sampler"),
            f"{lane_id}.sampler",
            requires_sealed_runtime_packet,
        )
        storage = lane.get("storage")
        if not isinstance(storage, dict) or set(storage) != {"format", "alphaMode"}:
            raise InventoryError(f"{lane_id}.storage fields changed")

    coverage = material.get("coveragePolicy")
    if not isinstance(coverage, dict):
        raise InventoryError("coveragePolicy must be an object")
    if set(coverage) != {
        "kind", "laneId", "channel", "equationId", "opaqueAlphaBehavior"
    }:
        raise InventoryError("coveragePolicy fields changed")
    coverage_kind = coverage.get("kind")
    if coverage_kind == "TEXTURE_CHANNEL":
        lane_id = _require_string(coverage.get("laneId"), "coveragePolicy.laneId")
        lane = lane_index.get(lane_id)
        if lane is None:
            raise InventoryError(f"coveragePolicy references unknown lane: {lane_id}")
        channel = coverage.get("channel")
        if channel not in CHANNEL_VALUES:
            raise InventoryError("coveragePolicy channel is invalid")
        if "A" in str(channel) and (lane.get("storage") or {}).get("alphaMode") == "NONE":
            raise InventoryError(
                "coveragePolicy reads alpha from a no-alpha texture; implicit base.a "
                "would create opaque/black card coverage"
            )
        if coverage.get("equationId") is not None:
            raise InventoryError("texture coverage cannot also name an equation")
    elif coverage_kind == "RECOVERED_EQUATION":
        _require_string(coverage.get("equationId"), "coveragePolicy.equationId")
        if coverage.get("laneId") is not None or coverage.get("channel") is not None:
            raise InventoryError("equation coverage must not imply a texture lane")
    else:
        raise InventoryError(f"unsupported coveragePolicy kind: {coverage_kind}")

    emissive = material.get("emissiveSourcePolicy")
    if not isinstance(emissive, dict) or set(emissive) != {
        "kind", "laneId", "channel", "constant"
    }:
        raise InventoryError("emissiveSourcePolicy fields changed")
    emissive_kind = emissive.get("kind")
    if emissive_kind in {"TEXTURE_LANE", "BASE_LUMINANCE"}:
        lane_id = _require_string(emissive.get("laneId"), "emissiveSourcePolicy.laneId")
        lane = lane_index.get(lane_id)
        if lane is None:
            raise InventoryError(f"emissiveSourcePolicy references unknown lane: {lane_id}")
        if lane.get("assetId") is None:
            raise InventoryError("emissive source lane is null")
        if emissive.get("channel") not in CHANNEL_VALUES:
            raise InventoryError("emissive source channel is invalid")
        if emissive.get("constant") is not None:
            raise InventoryError("texture emissive source cannot also be constant")
    elif emissive_kind == "CONSTANT":
        constant = emissive.get("constant")
        if not isinstance(constant, list) or len(constant) not in {3, 4}:
            raise InventoryError("constant emissive source must be RGB or RGBA")
    elif emissive_kind == "NONE":
        if any(emissive.get(key) is not None for key in ("laneId", "channel", "constant")):
            raise InventoryError("NONE emissive source carries unexpected data")
    else:
        raise InventoryError(f"unsupported emissiveSourcePolicy kind: {emissive_kind}")

    missing = material.get("missingLanePolicy")
    if not isinstance(missing, dict) or set(missing) != {"mode", "explicitConstants"}:
        raise InventoryError("missingLanePolicy fields changed")
    mode = missing.get("mode")
    if mode not in {"EXPLICIT_CONSTANT", "FAIL_CLOSED"}:
        raise InventoryError("implicit default SRV fallback is forbidden")
    constants = missing.get("explicitConstants")
    if not isinstance(constants, list):
        raise InventoryError("missingLanePolicy.explicitConstants must be an array")
    constant_ids = {
        row.get("laneId")
        for row in constants
        if isinstance(row, dict)
        and isinstance(row.get("value"), list)
        and len(row.get("value")) == 4
    }
    missing_ids = {lane_id for lane_id, lane in lane_index.items() if lane.get("assetId") is None}
    if mode == "EXPLICIT_CONSTANT" and missing_ids != constant_ids:
        raise InventoryError("every missing lane requires one explicit RGBA constant")
    if mode == "FAIL_CLOSED" and constants:
        raise InventoryError("FAIL_CLOSED missing-lane policy cannot carry constants")
    if admitted and missing_ids:
        raise InventoryError("admitted material has unresolved texture lanes")


def validate_inventory(inventory: dict[str, Any]) -> None:
    if inventory.get("schema") != SCHEMA or inventory.get("formatVersion") != FORMAT_VERSION:
        raise InventoryError("inventory schema/version changed")
    carrier_index = _index_unique(
        inventory.get("carrierVariants"), "carrierVariantId", "carrierVariants"
    )
    material_index = _index_unique(
        inventory.get("materialVariants"), "materialVariantId", "materialVariants"
    )
    render_index = _index_unique(
        inventory.get("renderVariants"), "renderVariantId", "renderVariants"
    )
    composition_index = _index_unique(
        inventory.get("compositionVariants"),
        "compositionVariantId",
        "compositionVariants",
    )
    substitution_receipt_index = _index_unique(
        inventory.get("substitutionDuplicationReceipts"),
        "substitutionDuplicationReceiptId",
        "substitutionDuplicationReceipts",
    )
    cohort_index = _index_unique(inventory.get("cohorts"), "cohortId", "cohorts")
    occurrence_index = _index_unique(
        inventory.get("occurrences"), "occurrenceId", "occurrences"
    )
    target_index = _index_unique(inventory.get("targets"), "targetId", "targets")

    material_admitted: dict[str, bool] = collections.defaultdict(bool)
    material_fail_closed: dict[str, bool] = collections.defaultdict(bool)
    source_node_occurrences: dict[str, list[str]] = collections.defaultdict(list)
    for occurrence in occurrence_index.values():
        family_tuple = occurrence.get("familyTuple")
        if not isinstance(family_tuple, dict) or set(family_tuple) != {
            "carrierVariantId",
            "materialVariantId",
            "renderVariantId",
            "compositionVariantId",
        }:
            raise InventoryError("occurrence familyTuple fields changed")
        carrier_id = family_tuple.get("carrierVariantId")
        render_id = family_tuple.get("renderVariantId")
        composition_id = family_tuple.get("compositionVariantId")
        material_id = family_tuple.get("materialVariantId")
        if carrier_id not in carrier_index:
            raise InventoryError(f"unknown carrierVariantId: {carrier_id}")
        if render_id not in render_index:
            raise InventoryError(f"unknown renderVariantId: {render_id}")
        if composition_id not in composition_index:
            raise InventoryError(f"unknown compositionVariantId: {composition_id}")
        carrier_kind = carrier_index[carrier_id].get("carrierKind")
        if material_id is None:
            if carrier_kind not in NULL_MATERIAL_CARRIER_KINDS:
                raise InventoryError(
                    f"materialVariantId=null is illegal for {carrier_kind}"
                )
        elif material_id not in material_index:
            raise InventoryError(f"unknown materialVariantId: {material_id}")
        elif carrier_kind in NULL_MATERIAL_CARRIER_KINDS:
            raise InventoryError(
                f"presentation-only {carrier_kind} must use materialVariantId=null"
            )
        if carrier_kind in MATERIAL_REQUIRED_CARRIER_KINDS and material_id is None:
            raise InventoryError(f"{carrier_kind} requires a material variant")
        type_data = carrier_index[carrier_id].get("sourceTypeDataClasses") or []
        if "particlemoduletypedataribbon" in type_data and carrier_kind != "CASCADE_RIBBON":
            raise InventoryError(
                "TypeDataRibbon cannot be projected to sprite/mesh fallback"
            )
        if carrier_kind not in (render_index[render_id].get("allowedCarrierKinds") or []):
            raise InventoryError("family tuple carrier/render mismatch")
        expected_closure = canonical_sha256(family_tuple)
        if occurrence.get("executionClosureId") != expected_closure:
            raise InventoryError("executionClosureId tuple mismatch")
        if occurrence.get("provenance") not in PROVENANCE_VALUES:
            raise InventoryError("occurrence provenance is invalid")
        if occurrence.get("evidence") not in EVIDENCE_VALUES:
            raise InventoryError("occurrence evidence is invalid")
        if occurrence.get("runtimeExecutor") not in EXECUTOR_VALUES:
            raise InventoryError("occurrence runtimeExecutor is invalid")
        admission = occurrence.get("runtimeAdmission")
        if admission not in ADMISSION_VALUES:
            raise InventoryError("occurrence runtimeAdmission is invalid")
        if occurrence.get("userReview") not in USER_REVIEW_VALUES:
            raise InventoryError("occurrence userReview is invalid")
        if occurrence.get("failureScope") != "THIS_OCCURRENCE":
            raise InventoryError("failureScope must isolate this occurrence")
        field_receipts = occurrence.get("fieldReceipts")
        if not isinstance(field_receipts, list):
            raise InventoryError("occurrence fieldReceipts must be an array")
        current_receipts = [
            receipt
            for receipt in field_receipts
            if isinstance(receipt, dict)
            and receipt.get("fieldGroup") == "CURRENT_DIRECT_AUTHORED_CLOSURE"
        ]
        if (
            len(current_receipts) != 1
            or current_receipts[0].get("provenance")
            != occurrence.get("provenance")
            or current_receipts[0].get("evidence") != occurrence.get("evidence")
        ):
            raise InventoryError(
                "occurrence top-level provenance/evidence does not match its current field receipt"
            )
        product_join = occurrence.get("productJoin")
        if not isinstance(product_join, dict):
            raise InventoryError("occurrence productJoin must be an object")
        _require_string(
            product_join.get("effectAssetId"),
            "productJoin.effectAssetId",
        )
        _safe_relative_path(
            product_join.get("authoringPath"),
            "productJoin.authoringPath",
        )
        _require_sha256(
            product_join.get("authoredElementSha256"),
            "productJoin.authoredElementSha256",
        )
        product_status = product_join.get("status")
        published_element_sha = product_join.get("publishedElementSha256")
        if product_status == "CLOSED":
            _require_sha256(
                published_element_sha,
                "productJoin.publishedElementSha256",
            )
            if published_element_sha != product_join.get(
                "authoredElementSha256"
            ):
                raise InventoryError(
                    "closed Product join does not publish its occurrence-local "
                    "authored closure"
                )
        elif product_status == "AUTHORED_NOT_PUBLISHED":
            if (
                admission != "AUTHORING_ONLY"
                or published_element_sha is not None
                or "PUBLISHED_DIRECT_DOCUMENT_STALE_FOR_OCCURRENCE"
                not in (occurrence.get("blockers") or [])
            ):
                raise InventoryError(
                    "unpublished authored occurrence must remain AUTHORING_ONLY"
                )
        else:
            raise InventoryError(f"unsupported productJoin status: {product_status}")
        authored_node = occurrence.get("authoredNode") or {}
        source_node = authored_node.get("sourceNode")
        receipt_id = occurrence.get("substitutionDuplicationReceiptId")
        if source_node is not None:
            source_node_occurrences[str(source_node)].append(
                occurrence["occurrenceId"]
            )
        if receipt_id is not None and receipt_id not in substitution_receipt_index:
            raise InventoryError(
                f"unknown substitutionDuplicationReceiptId: {receipt_id}"
            )
        if material_id is not None:
            material_admitted[material_id] |= admission == "ADMITTED"
            material_fail_closed[material_id] |= admission == "FAIL_CLOSED"

    for receipt_id, receipt in substitution_receipt_index.items():
        payload = copy.deepcopy(receipt)
        del payload["substitutionDuplicationReceiptId"]
        expected_receipt_id = "substitution-duplication." + canonical_sha256(payload)
        if receipt_id != expected_receipt_id:
            raise InventoryError(
                "substitutionDuplicationReceiptId content hash mismatch"
            )
        decisions = receipt.get("occurrenceDecisions")
        if not isinstance(decisions, list) or not decisions:
            raise InventoryError(
                "substitution/duplication receipt decisions are missing"
            )
        authorized = [
            decision.get("occurrenceId")
            for decision in decisions
            if isinstance(decision, dict)
        ]
        if (
            len(authorized) != len(decisions)
            or len(authorized) != len(set(authorized))
            or any(occurrence_id not in occurrence_index for occurrence_id in authorized)
        ):
            raise InventoryError(
                "substitution/duplication receipt authorized occurrence list is invalid"
            )
        source_start = receipt.get("sourceStartSeconds")
        if not isinstance(source_start, (int, float)) or not math.isfinite(
            float(source_start)
        ):
            raise InventoryError(
                "substitution/duplication receipt source timing is invalid"
            )
        for decision in decisions:
            occurrence_id = decision["occurrenceId"]
            occurrence = occurrence_index[occurrence_id]
            authored_node = occurrence.get("authoredNode") or {}
            authored_start = decision.get("authoredStartSeconds")
            product_local_start = decision.get("productLocalStartSeconds")
            cue_source_start_ms = decision.get("cueSourceStartMilliseconds")
            timing_provenance = decision.get("timingProvenance")
            substituted = not math.isclose(
                float(source_start),
                float(authored_start),
                rel_tol=0,
                abs_tol=1e-7,
            ) if isinstance(authored_start, (int, float)) else True
            expected_timing_provenance = (
                "PROJECT_TUNED" if substituted else "SOURCE_EXACT"
            )
            expected_disposition = (
                "EVIDENCE_ONLY_SUBSTITUTED_NOT_ADMITTED"
                if substituted
                else "ADMITTED_SOURCE_OCCURRENCE"
            )
            composition = composition_index[
                occurrence["familyTuple"]["compositionVariantId"]
            ]
            occurrence_start = (composition.get("timing") or {}).get(
                "startDelaySeconds"
            )
            animation_cue_start_ms = (composition.get("animationCue") or {}).get(
                "startMilliseconds"
            )
            if (
                occurrence.get("substitutionDuplicationReceiptId") != receipt_id
                or authored_node.get("stableId") != decision.get("currentStableId")
                or authored_node.get("sourceNode") != receipt.get("sourceNode")
                or (authored_node.get("sourceIdentity") or {}).get("stableId")
                != receipt.get("sourceStableId")
                or timing_provenance != expected_timing_provenance
                or occurrence.get("provenance") != expected_timing_provenance
                or decision.get("sourceOccurrenceDisposition")
                != expected_disposition
                or not isinstance(occurrence_start, (int, float))
                or not isinstance(product_local_start, (int, float))
                or not math.isclose(
                    float(occurrence_start),
                    float(product_local_start),
                    rel_tol=0,
                    abs_tol=1e-7,
                )
                or not isinstance(cue_source_start_ms, int)
                or animation_cue_start_ms != cue_source_start_ms
                or not isinstance(authored_start, (int, float))
                or not math.isclose(
                    float(authored_start) * 1000.0,
                    float(cue_source_start_ms),
                    rel_tol=0,
                    abs_tol=1e-4,
                )
            ):
                raise InventoryError(
                    "substitution/duplication receipt does not exactly key current/source IDs"
                )
        provenance = receipt.get("fieldProvenance") or {}
        if (
            provenance.get("carrier") != "SOURCE_EXACT"
            or provenance.get("materialIdentity") != "SOURCE_EXACT"
        ):
            raise InventoryError(
                "substitution/duplication receipt field provenance is invalid"
            )
        if not isinstance(receipt.get("targetCardinality"), int) or receipt[
            "targetCardinality"
        ] < len(authorized):
            raise InventoryError(
                "substitution/duplication receipt target cardinality is invalid"
            )

    for source_node, occurrence_ids in source_node_occurrences.items():
        if len(occurrence_ids) < 2:
            continue
        receipt_ids = {
            occurrence_index[occurrence_id].get(
                "substitutionDuplicationReceiptId"
            )
            for occurrence_id in occurrence_ids
        }
        if len(receipt_ids) != 1 or None in receipt_ids:
            raise InventoryError(
                f"silent duplicate sourceNode is forbidden: {source_node}"
            )
        receipt = substitution_receipt_index[next(iter(receipt_ids))]
        authorized = {
            decision["occurrenceId"]
            for decision in receipt["occurrenceDecisions"]
        }
        if authorized != set(occurrence_ids):
            raise InventoryError(
                f"duplicate sourceNode exceeds exact receipt authorization: {source_node}"
            )

    for material_id, material in material_index.items():
        _validate_material_variant(material, material_admitted[material_id])
        missing = {
            lane["laneId"]
            for lane in material.get("textureLanes", [])
            if lane.get("assetId") is None
        }
        if missing and material["missingLanePolicy"]["mode"] == "FAIL_CLOSED":
            references = [
                row
                for row in occurrence_index.values()
                if row["familyTuple"]["materialVariantId"] == material_id
            ]
            if any(row["runtimeAdmission"] != "FAIL_CLOSED" for row in references):
                raise InventoryError(
                    "unresolved FAIL_CLOSED material lane was admitted or authoring-only"
                )

    for occurrence in occurrence_index.values():
        material_id = occurrence["familyTuple"]["materialVariantId"]
        if material_id is None:
            continue
        emissive_intensity = (occurrence.get("materialInputs") or {}).get(
            "emissiveIntensity"
        )
        if not isinstance(emissive_intensity, (int, float)) or not math.isfinite(
            emissive_intensity
        ):
            raise InventoryError("material emissiveIntensity is not finite")
        emissive = material_index[material_id]["emissiveSourcePolicy"]
        if emissive_intensity > 0 and emissive.get("kind") == "NONE":
            raise InventoryError(
                "emissiveIntensity > 0 has no emissive texture/base/constant source"
            )

    for carrier in carrier_index.values():
        _validate_variant_identity(carrier, "carrierVariantId", "carrier")
    for material in material_index.values():
        _validate_variant_identity(material, "materialVariantId", "material")
    for render in render_index.values():
        _validate_variant_identity(render, "renderVariantId", "render")
    for composition in composition_index.values():
        _validate_variant_identity(
            composition, "compositionVariantId", "composition"
        )

    for cohort in cohort_index.values():
        target_id = cohort.get("targetId")
        if target_id not in target_index:
            raise InventoryError(f"cohort references unknown targetId: {target_id}")
        ordered = cohort.get("orderedOccurrenceIds")
        if not isinstance(ordered, list) or len(ordered) != len(set(ordered)):
            raise InventoryError("cohort occurrence order is missing or duplicated")
        if any(occurrence_id not in occurrence_index for occurrence_id in ordered):
            raise InventoryError("cohort references unknown occurrenceId")

    for target in target_index.values():
        cohort_ids = target.get("cohortIds")
        if not isinstance(cohort_ids, list) or any(
            cohort_id not in cohort_index for cohort_id in cohort_ids
        ):
            raise InventoryError("target references unknown cohortId")
        actual = sum(
            len(cohort_index[cohort_id]["orderedOccurrenceIds"])
            for cohort_id in cohort_ids
        )
        if target.get("selectedOccurrenceCount") != actual:
            raise InventoryError("target selectedOccurrenceCount mismatch")

    expected_summary = {
        "targetCount": len(target_index),
        "cohortCount": len(cohort_index),
        "occurrenceCount": len(occurrence_index),
        "carrierVariantCount": len(carrier_index),
        "materialVariantCount": len(material_index),
        "renderVariantCount": len(render_index),
        "compositionVariantCount": len(composition_index),
        "substitutionDuplicationReceiptCount": len(
            substitution_receipt_index
        ),
        "runtimeAdmissionCounts": dict(sorted(collections.Counter(
            row["runtimeAdmission"] for row in occurrence_index.values()
        ).items())),
        "userReviewCounts": dict(sorted(collections.Counter(
            row["userReview"] for row in occurrence_index.values()
        ).items())),
    }
    if inventory.get("summary") != expected_summary:
        raise InventoryError("inventory summary is stale")
    artifact_sha = _require_sha256(inventory.get("artifactSha256"), "artifactSha256")
    if _artifact_sha256(inventory) != artifact_sha:
        raise InventoryError("inventory artifactSha256 is stale")


def write_inventory_transactionally(
    inventory: dict[str, Any], output_path: Path
) -> None:
    validate_inventory(inventory)
    payload = pretty_json_bytes(inventory)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            delete=False,
            dir=output_path.parent,
            prefix=f".{output_path.name}.",
            suffix=".staging",
        ) as temporary:
            temporary_path = Path(temporary.name)
            temporary.write(payload)
            temporary.flush()
            os.fsync(temporary.fileno())
        os.replace(temporary_path, output_path)
        temporary_path = None
    finally:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()


def check_inventory(inventory: dict[str, Any], output_path: Path) -> None:
    validate_inventory(inventory)
    if not output_path.is_file():
        raise InventoryError(f"inventory output is missing: {output_path}")
    existing = read_json(output_path)
    validate_inventory(existing)
    if existing != inventory:
        raise InventoryError(
            "inventory output is stale: "
            f"disk={existing.get('artifactSha256')} "
            f"rebuilt={inventory.get('artifactSha256')}"
        )


def _parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mode",
        choices=("build", "check"),
        default="build",
        help="build atomically replaces the inventory; check is read-only",
    )
    parser.add_argument("--root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument("--output", type=Path, default=None)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    arguments = _parse_args(argv)
    root = arguments.root.resolve()
    output = arguments.output or (
        root / Path(*PurePosixPath(OUTPUT_RELATIVE_PATH).parts)
    )
    inventory = build_inventory(root)
    if arguments.mode == "check":
        check_inventory(inventory, output)
        print(
            "PASS: character Effect restoration inventory matches "
            f"{inventory['summary']['occurrenceCount']} occurrences "
            f"(sha256={inventory['artifactSha256'][:16]})."
        )
    else:
        write_inventory_transactionally(inventory, output)
        try:
            display = output.resolve().relative_to(root)
        except ValueError:
            display = output.resolve()
        print(
            f"WROTE {display} "
            f"({inventory['summary']['occurrenceCount']} occurrences, "
            f"sha256={inventory['artifactSha256'][:16]})."
        )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except InventoryError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
