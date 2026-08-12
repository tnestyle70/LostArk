#!/usr/bin/env python3
"""Decode the bounded UE3 FMaterialResource tail for Artist 31470 F.

The decoded legacy texture lookups are source-exact cooked metadata.  They are
not promoted to shader UV uniforms, arithmetic graph nodes, or Product proof.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path
from typing import Any

from extract_artist_31470_shader_cache_oracle import (
    DEFAULT_GLOBAL_MATERIAL_PACKAGE,
    DEFAULT_MATERIAL_CONTRACT,
    DEFAULT_OUTPUT as DEFAULT_SHADER_CACHE_RECEIPT,
    DEFAULT_SOURCE_PACKAGE_ROOT,
    canonical_json_sha256,
    material_family_projection,
    raw_file_sha256,
    read_json,
)
from extract_ue3_effect_material_closure import load_package
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


SCHEMA = "lostark.artist-31470-material-native-resource-receipt"
FORMAT_VERSION = 1
EXPECTED_FAMILY_COUNT = 23
EXPECTED_REFERENCED_TEXTURE_COUNT = 64
EXPECTED_LOOKUP_FAMILY_COUNT = 4
EXPECTED_LOOKUP_COUNT = 7
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-native-resource.receipt.json"
)
DEFAULT_TEXTURE_BINDING_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-texture-runtime-binding.receipt.json"
)
DEFAULT_ACTIVE_MATERIAL_CLOSURE = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.active-material-closure.json"
)
DEFAULT_RUNTIME_TEXTURE_ROOT = REPO_ROOT / "Client/Bin/Resources/Effect"

MAIN_DISSOLVE_PRIORITIES = (
    {
        "label": "#9/#10_WATERTRAIL",
        "occurrenceIds": ["source-active-009", "source-active-010"],
        "familyId": "material-family-89af5c77d8e35f99",
        "baseMaterialObjectPath": "fx_m.fx_m_me_watertrail_01_tr",
        "expectedNativeReferencedTextureCount": 4,
        "expectedEffectiveBindingCount": 2,
        "expectedEffectiveUniqueTextureCount": 2,
        "expectedNativeEffectiveOverlapCount": 0,
    },
    {
        "label": "#11_SPRITEWAVE",
        "occurrenceIds": ["source-active-011"],
        "familyId": "material-family-097bd8d9597721b5",
        "baseMaterialObjectPath": "fx_m.fx_m_pa_spritewave_01_tr",
        "expectedNativeReferencedTextureCount": 2,
        "expectedEffectiveBindingCount": 7,
        "expectedEffectiveUniqueTextureCount": 6,
        "expectedNativeEffectiveOverlapCount": 1,
    },
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def canonical_text_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8").replace("\r\n", "\n").replace("\r", "\n")
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


class Cursor:
    def __init__(self, payload: bytes) -> None:
        self.payload = payload
        self.offset = 0

    def u32(self, label: str) -> int:
        require(self.offset + 4 <= len(self.payload), f"truncated {label}")
        value = struct.unpack_from("<I", self.payload, self.offset)[0]
        self.offset += 4
        return value

    def i32(self, label: str) -> int:
        require(self.offset + 4 <= len(self.payload), f"truncated {label}")
        value = struct.unpack_from("<i", self.payload, self.offset)[0]
        self.offset += 4
        return value

    def f32(self, label: str) -> float:
        require(self.offset + 4 <= len(self.payload), f"truncated {label}")
        value = struct.unpack_from("<f", self.payload, self.offset)[0]
        self.offset += 4
        require(math.isfinite(value), f"non-finite {label}")
        return value

    def bytes(self, count: int, label: str) -> bytes:
        require(count >= 0 and self.offset + count <= len(self.payload), f"truncated {label}")
        value = self.payload[self.offset : self.offset + count]
        self.offset += count
        return value


def resolve_texture_reference(package: Any, reference: int) -> dict[str, Any]:
    require(reference != 0, "ReferencedTextures contains null")
    object_path = package_ref_path(reference, package.imports, package.exports)
    if reference < 0:
        entry = package.imports[-reference - 1]
        class_name = entry.class_name
    else:
        require(reference <= len(package.exports), "ReferencedTextures export reference is invalid")
        entry = package.exports[reference - 1]
        class_name = package_ref_name(entry.class_index, package.imports, package.exports)
    require(class_name.casefold() == "texture2d", "ReferencedTextures entry is not Texture2D")
    return {
        "packageReference": reference,
        "objectPath": object_path,
        "className": class_name.casefold(),
    }


def parse_material_resource_tail(tail: bytes, package: Any) -> dict[str, Any]:
    require(len(tail) >= 84 and len(tail) % 4 == 0, "Material native tail shape is invalid")
    cursor = Cursor(tail)
    mask = cursor.u32("material resource mask")
    legacy_string_count = cursor.u32("legacy string count")
    legacy_expression_map_count = cursor.u32("legacy expression map count")
    legacy_integer = cursor.u32("legacy integer")
    require(mask == 1, "Material resource mask changed")
    require(legacy_string_count == 0, "legacy string payload is unsupported")
    require(legacy_expression_map_count == 0, "legacy expression map payload is unsupported")
    state_guid = cursor.bytes(16, "material state GUID")
    require(state_guid != b"\x00" * 16, "material state GUID is zero")
    resource_field = cursor.u32("resource field")

    texture_count_offset = cursor.offset
    texture_count = cursor.u32("ReferencedTextures count")
    require(texture_count <= 256, "ReferencedTextures count is unbounded")
    textures = []
    for index in range(texture_count):
        reference_offset = cursor.offset
        reference = cursor.i32(f"ReferencedTextures[{index}]")
        textures.append(
            {
                "index": index,
                "offsetInNativeTail": reference_offset,
                **resolve_texture_reference(package, reference),
            }
        )

    opaque_prefix_offset = cursor.offset
    opaque_prefix = [cursor.u32(f"opaquePrefixDwords[{index}]") for index in range(6)]
    lookup_count_offset = cursor.offset
    lookup_count = cursor.u32("legacy texture lookup count")
    require(lookup_count <= texture_count, "legacy texture lookup count exceeds texture count")
    lookups = []
    for index in range(lookup_count):
        entry_offset = cursor.offset
        tex_coord_index = cursor.i32(f"legacyTextureLookups[{index}].texCoordIndex")
        texture_index = cursor.i32(f"legacyTextureLookups[{index}].textureIndex")
        u_scale = cursor.f32(f"legacyTextureLookups[{index}].uScale")
        v_scale = cursor.f32(f"legacyTextureLookups[{index}].vScale")
        require(tex_coord_index >= 0, "legacy texture lookup coordinate index is negative")
        require(0 <= texture_index < texture_count, "legacy texture lookup texture index is invalid")
        lookups.append(
            {
                "index": index,
                "offsetInNativeTail": entry_offset,
                "texCoordIndex": tex_coord_index,
                "textureIndex": texture_index,
                "textureObjectPath": textures[texture_index]["objectPath"],
                "uScale": u_scale,
                "vScale": v_scale,
                "fidelity": "SOURCE_EXACT_LEGACY_TEXTURE_LOOKUP_METADATA_NOT_SHADER_UV_UNIFORM",
            }
        )

    trailer_offset = cursor.offset
    trailer = [cursor.u32(f"opaqueTrailerDwords[{index}]") for index in range(4)]
    require(cursor.offset == len(tail), "Material native tail has unparsed bytes")
    return {
        "nativeTailByteCount": len(tail),
        "nativeTailSha256": hashlib.sha256(tail).hexdigest(),
        "materialResourceMask": mask,
        "legacyStringCount": legacy_string_count,
        "legacyExpressionMapCount": legacy_expression_map_count,
        "legacyInteger": legacy_integer,
        "materialStateGuidHex": state_guid.hex(),
        "resourceField": resource_field,
        "referencedTextureCountOffsetInNativeTail": texture_count_offset,
        "referencedTextures": textures,
        "opaquePrefixOffsetInNativeTail": opaque_prefix_offset,
        "opaquePrefixDwords": opaque_prefix,
        "legacyTextureLookupCountOffsetInNativeTail": lookup_count_offset,
        "legacyTextureLookups": lookups,
        "opaqueTrailerOffsetInNativeTail": trailer_offset,
        "opaqueTrailerDwords": trailer,
        "parsedByteCount": cursor.offset,
        "unparsedByteCount": 0,
        "inlineShaderPayloadStatus": "ABSENT_TAIL_FULLY_CONSUMED_NO_DXBC_OR_SHADER_MAP_REMAINDER",
    }


def decode_export(package: Any, export_index: int, expected_path: str) -> dict[str, Any]:
    require(0 <= export_index < len(package.exports), "Material export index is invalid")
    entry = package.exports[export_index]
    object_path = package_ref_path(entry.index + 1, package.imports, package.exports)
    class_name = package_ref_name(entry.class_index, package.imports, package.exports).casefold()
    require(object_path.casefold() == expected_path.casefold(), "Material export path changed")
    require(class_name in ("material", "decalmaterial"), "Material export class changed")
    serial = package.logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
    _properties, property_end = parse_tagged_properties(serial, package.names, package.summary.version)
    tail = serial[property_end:]
    return {
        "objectPath": object_path,
        "className": class_name,
        "exportIndex": entry.index,
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": hashlib.sha256(serial).hexdigest(),
        "propertyStreamEnd": property_end,
        **parse_material_resource_tail(tail, package),
    }


def semantic_projection(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "className": row["className"],
        "materialResourceMask": row["materialResourceMask"],
        "legacyStringCount": row["legacyStringCount"],
        "legacyExpressionMapCount": row["legacyExpressionMapCount"],
        "legacyInteger": row["legacyInteger"],
        "materialStateGuidHex": row["materialStateGuidHex"],
        "resourceField": row["resourceField"],
        "referencedTextureLeaves": [
            item["objectPath"].rsplit(".", 1)[-1].casefold()
            for item in row["referencedTextures"]
        ],
        "opaquePrefixDwords": row["opaquePrefixDwords"],
        "legacyTextureLookups": [
            {
                "texCoordIndex": item["texCoordIndex"],
                "textureIndex": item["textureIndex"],
                "textureObjectLeaf": item["textureObjectPath"].rsplit(".", 1)[-1].casefold(),
                "uScale": item["uScale"],
                "vScale": item["vScale"],
            }
            for item in row["legacyTextureLookups"]
        ],
        "opaqueTrailerDwords": row["opaqueTrailerDwords"],
        "unparsedByteCount": row["unparsedByteCount"],
        "inlineShaderPayloadStatus": row["inlineShaderPayloadStatus"],
    }


def normalize_logical_texture(path: str) -> str:
    return path.strip().casefold()


def runtime_texture_inventory(runtime_texture_root: Path) -> dict[str, list[dict[str, Any]]]:
    inventory: dict[str, list[dict[str, Any]]] = {}
    if not runtime_texture_root.is_dir():
        return inventory
    for path in sorted(runtime_texture_root.rglob("*.dds"), key=lambda item: item.as_posix().casefold()):
        leaf = path.stem.casefold()
        inventory.setdefault(leaf, []).append(
            {
                "runtimeAssetId": path.relative_to(REPO_ROOT / "Client/Bin/Resources").as_posix(),
                "byteCount": path.stat().st_size,
                "rawSha256": raw_file_sha256(path),
            }
        )
    return inventory


def family_named_texture_parameters(
    families: list[dict[str, Any]],
    active_material_closure: dict[str, Any],
) -> dict[str, dict[str, set[str]]]:
    """Join base-Material texture defaults to their parameter names.

    FLegacyTextureLookup only stores a local ReferencedTextures index.  It does
    not encode the parameter name needed to decide whether an active MIC has
    replaced that base default.  The already-pinned cooked graph closure owns
    that name, so both sources must agree before override precedence is used.
    """

    by_identity: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for row in active_material_closure.get("materials", []):
        for holder_name in ("materialGraph", "parentGraph"):
            holder = row.get(holder_name)
            graph = holder.get("graph") if isinstance(holder, dict) else None
            if not isinstance(graph, dict):
                continue
            physical = str(holder.get("physicalPackage") or "").casefold()
            object_path = str(graph.get("materialPath") or "").casefold()
            by_identity.setdefault((physical, object_path), []).append(graph)

    result: dict[str, dict[str, set[str]]] = {}
    for family in families:
        identity = (
            str(family["physicalPackage"]).casefold(),
            str(family["materialObjectPath"]).casefold(),
        )
        graphs = by_identity.get(identity, [])
        require(graphs, f"active graph closure missing for {family['familyId']}")
        named: dict[str, set[str]] = {}
        for graph in graphs:
            for row in graph.get("namedTextures", []):
                logical = normalize_logical_texture(str(row.get("sourceObjectPath") or ""))
                parameter = str(row.get("name") or "").strip().casefold()
                if logical and parameter:
                    named.setdefault(logical, set()).add(parameter)
        result[family["familyId"]] = named
    return result


def active_recipe_texture_overrides(
    material_contract: dict[str, Any],
) -> dict[str, list[dict[str, Any]]]:
    active_recipe_ids = {
        str(row.get("materialRecipeId") or "")
        for row in material_contract.get("occurrences", [])
    }
    by_family: dict[str, list[dict[str, Any]]] = {}
    for recipe in material_contract.get("materialRecipes", []):
        recipe_id = str(recipe.get("recipeId") or "")
        family_id = str(recipe.get("arithmeticFamilyId") or "")
        if not family_id or recipe_id not in active_recipe_ids:
            continue
        overrides = {
            str(row.get("normalizedParameterName") or row.get("parameterName") or "")
            .strip()
            .casefold(): normalize_logical_texture(str(row.get("value") or ""))
            for row in (recipe.get("inputs") or {}).get("textureOverrides", [])
        }
        by_family.setdefault(family_id, []).append(
            {
                "recipeId": recipe_id,
                "sourceMaterialPath": recipe.get("sourceMaterialPath"),
                "textureOverrides": overrides,
            }
        )
    return by_family


def resolve_lookup_precedence(
    logical_texture: str,
    family_ids: list[str],
    named_parameters: dict[str, dict[str, set[str]]],
    recipes_by_family: dict[str, list[dict[str, Any]]],
    runtime_bound_paths: set[str],
) -> dict[str, Any]:
    resolutions = []
    for family_id in family_ids:
        parameters = sorted(named_parameters.get(family_id, {}).get(logical_texture, set()))
        require(parameters, f"lookup parameter name missing for {family_id}: {logical_texture}")
        recipes = recipes_by_family.get(family_id, [])
        require(recipes, f"active recipe missing for lookup family {family_id}")
        recipe_resolutions = []
        for recipe in recipes:
            values = sorted(
                {
                    recipe["textureOverrides"][parameter]
                    for parameter in parameters
                    if parameter in recipe["textureOverrides"]
                }
            )
            recipe_resolutions.append(
                {
                    "recipeId": recipe["recipeId"],
                    "sourceMaterialPath": recipe["sourceMaterialPath"],
                    "parameterNames": parameters,
                    "overrideValues": values,
                    "baseDefaultEffective": not bool(values),
                }
            )
        all_overridden = all(not row["baseDefaultEffective"] for row in recipe_resolutions)
        effective_override_values = sorted(
            {value for row in recipe_resolutions for value in row["overrideValues"]}
        )
        override_preserves_logical_texture = logical_texture in effective_override_values
        resolutions.append(
            {
                "familyId": family_id,
                "parameterNames": parameters,
                "activeRecipes": recipe_resolutions,
                "baseDefaultOverriddenInAllActiveRecipes": all_overridden,
                "effectiveOverrideValues": effective_override_values,
                "overridePreservesBaseLogicalTexture": override_preserves_logical_texture,
                "effectiveOverrideValuesRuntimeBound": all(
                    value in runtime_bound_paths for value in effective_override_values
                ),
            }
        )
    base_default_effective = bool(resolutions) and any(
        not row["baseDefaultOverriddenInAllActiveRecipes"] for row in resolutions
    )
    all_active_recipes_override = bool(resolutions) and all(
        row["baseDefaultOverriddenInAllActiveRecipes"] for row in resolutions
    )
    logical_texture_effective = bool(resolutions) and any(
        not row["baseDefaultOverriddenInAllActiveRecipes"]
        or row["overridePreservesBaseLogicalTexture"]
        for row in resolutions
    )
    return {
        "legacyTextureLookupResolutions": resolutions,
        "baseDefaultEffectiveForActiveArtist31470": base_default_effective,
        "baseDefaultOverriddenInAllActiveRecipes": all_active_recipes_override,
        "logicalTextureEffectiveAfterMicPrecedence": logical_texture_effective,
        "replacedWithDifferentTextureInAllActiveRecipes": (
            all_active_recipes_override and not logical_texture_effective
        ),
        "identityOverridePreservesLogicalTexture": (
            all_active_recipes_override and logical_texture_effective
        ),
    }


def build_texture_closure_diagnostics(
    rows: list[dict[str, Any]],
    material_contract: dict[str, Any],
    active_material_closure: dict[str, Any],
    texture_binding_receipt: dict[str, Any],
    runtime_texture_root: Path,
) -> dict[str, Any]:
    bound_paths = {
        normalize_logical_texture(str(row.get("logicalTexturePath") or ""))
        for row in texture_binding_receipt.get("textureResources", [])
    }
    all_effect_inventory = runtime_texture_inventory(runtime_texture_root)
    artist_inventory = runtime_texture_inventory(runtime_texture_root / "Artist")
    families = [
        {
            "familyId": row["familyId"],
            "physicalPackage": row["sourcePhysicalPackage"],
            "materialObjectPath": row["source"]["objectPath"],
        }
        for row in rows
    ]
    named_parameters = family_named_texture_parameters(families, active_material_closure)
    recipes_by_family = active_recipe_texture_overrides(material_contract)
    referenced: dict[str, dict[str, Any]] = {}
    lookup_paths: set[str] = set()
    for row in rows:
        family_id = row["familyId"]
        for texture in row["source"]["referencedTextures"]:
            logical = normalize_logical_texture(texture["objectPath"])
            target = referenced.setdefault(
                logical,
                {
                    "logicalTexturePath": texture["objectPath"],
                    "leaf": texture["objectPath"].rsplit(".", 1)[-1].casefold(),
                    "familyIds": [],
                    "legacyTextureLookupFamilyIds": [],
                },
            )
            if family_id not in target["familyIds"]:
                target["familyIds"].append(family_id)
        for lookup in row["source"]["legacyTextureLookups"]:
            logical = normalize_logical_texture(lookup["textureObjectPath"])
            lookup_paths.add(logical)
            target = referenced[logical]
            if family_id not in target["legacyTextureLookupFamilyIds"]:
                target["legacyTextureLookupFamilyIds"].append(family_id)

    diagnostics = []
    for logical, target in sorted(referenced.items()):
        all_copies = all_effect_inventory.get(target["leaf"], [])
        artist_copies = artist_inventory.get(target["leaf"], [])
        other_copies = [
            row for row in all_copies
            if not str(row["runtimeAssetId"]).casefold().startswith("effect/artist/")
        ]
        hashes = {copy["rawSha256"] for copy in all_copies}
        exact_binding = logical in bound_paths
        lookup_backed = logical in lookup_paths
        precedence = (
            resolve_lookup_precedence(
                logical,
                target["legacyTextureLookupFamilyIds"],
                named_parameters,
                recipes_by_family,
                bound_paths,
            )
            if lookup_backed
            else {
                "legacyTextureLookupResolutions": [],
                "baseDefaultEffectiveForActiveArtist31470": False,
                "baseDefaultOverriddenInAllActiveRecipes": False,
                "logicalTextureEffectiveAfterMicPrecedence": False,
                "replacedWithDifferentTextureInAllActiveRecipes": False,
                "identityOverridePreservesLogicalTexture": False,
            }
        )
        diagnostics.append(
            {
                **target,
                "runtimeBindingPresent": exact_binding,
                "legacyTextureLookupBacked": lookup_backed,
                **precedence,
                "artistDomainSameLeafRuntimeCopyCount": len(artist_copies),
                "artistDomainSameLeafRuntimeCopies": artist_copies,
                "otherEffectDomainSameLeafRuntimeCopyCount": len(other_copies),
                "otherEffectDomainSameLeafRuntimeCopies": other_copies,
                "allEffectDomainSameLeafRuntimeCopyCount": len(all_copies),
                "allEffectDomainSameLeafRuntimeCopies": all_copies,
                "sameLeafCopiesByteIdentical": len(hashes) <= 1,
                "status": (
                    "SOURCE_EXACT_BASE_DEFAULT_REPLACED_BY_ACTIVE_MIC"
                    if precedence["replacedWithDifferentTextureInAllActiveRecipes"]
                    else "EFFECTIVE_LOOKUP_TEXTURE_BOUND"
                    if precedence["logicalTextureEffectiveAfterMicPrecedence"] and exact_binding
                    else "EFFECTIVE_LOOKUP_TEXTURE_BINDING_MISSING"
                    if precedence["logicalTextureEffectiveAfterMicPrecedence"]
                    else "BOUND_BY_ARTIST_31470_RUNTIME"
                    if exact_binding
                    else "UNBOUND_REFERENCED_TEXTURE_STATIC_SUPERSET_CANDIDATE"
                ),
                "automaticBindingAdmission": False,
            }
        )
    return {
        "diagnostics": diagnostics,
        "uniqueReferencedTextureCount": len(diagnostics),
        "artistRuntimeBoundReferencedTextureCount": sum(row["runtimeBindingPresent"] for row in diagnostics),
        "artistRuntimeUnboundReferencedTextureCount": sum(not row["runtimeBindingPresent"] for row in diagnostics),
        "artistDomainAvailableReferencedTextureCount": sum(
            row["artistDomainSameLeafRuntimeCopyCount"] > 0 for row in diagnostics
        ),
        "artistDomainMissingReferencedTextureCount": sum(
            row["artistDomainSameLeafRuntimeCopyCount"] == 0 for row in diagnostics
        ),
        "artistMissingButOtherEffectDomainAvailableCount": sum(
            row["artistDomainSameLeafRuntimeCopyCount"] == 0
            and row["otherEffectDomainSameLeafRuntimeCopyCount"] > 0
            for row in diagnostics
        ),
        "allEffectDomainsMissingReferencedTextureCount": sum(
            row["allEffectDomainSameLeafRuntimeCopyCount"] == 0
            for row in diagnostics
        ),
        "lookupBackedUniqueTextureCount": len(lookup_paths),
        "unboundLookupBackedUniqueTextureCount": sum(
            row["legacyTextureLookupBacked"] and not row["runtimeBindingPresent"]
            for row in diagnostics
        ),
        "overriddenLookupBackedUniqueTextureCount": sum(
            row["baseDefaultOverriddenInAllActiveRecipes"] for row in diagnostics
        ),
        "differentReplacementLookupBackedUniqueTextureCount": sum(
            row["replacedWithDifferentTextureInAllActiveRecipes"] for row in diagnostics
        ),
        "identityOverrideLookupBackedUniqueTextureCount": sum(
            row["identityOverridePreservesLogicalTexture"] for row in diagnostics
        ),
        "effectiveLookupBackedUniqueTextureCount": sum(
            row["logicalTextureEffectiveAfterMicPrecedence"] for row in diagnostics
        ),
        "effectiveLookupMissingRuntimeBindingCount": sum(
            row["logicalTextureEffectiveAfterMicPrecedence"]
            and not row["runtimeBindingPresent"]
            for row in diagnostics
        ),
        "overriddenLookupEffectiveOverrideMissingRuntimeBindingCount": sum(
            row["baseDefaultOverriddenInAllActiveRecipes"]
            and any(
                not resolution["effectiveOverrideValuesRuntimeBound"]
                for resolution in row["legacyTextureLookupResolutions"]
            )
            for row in diagnostics
        ),
        "artistRuntimeBindingMeaning": (
            "Exact logical texture paths in the Artist 31470 runtime-binding receipt; "
            "not the same metric as same-leaf DDS availability."
        ),
        "legacyLookupMeaning": (
            "Base-Material streaming/UV dependency metadata. Active MIC override precedence "
            "must be resolved before an effective runtime texture is inferred."
        ),
    }


def build_main_dissolve_priority(
    rows: list[dict[str, Any]],
    material_contract: dict[str, Any],
) -> dict[str, Any]:
    rows_by_family = {row["familyId"]: row for row in rows}
    occurrences = {
        str(row.get("occurrenceId") or ""): row
        for row in material_contract.get("occurrences", [])
    }
    recipes = {
        str(row.get("recipeId") or ""): row
        for row in material_contract.get("materialRecipes", [])
    }
    result_rows = []
    for expected in MAIN_DISSOLVE_PRIORITIES:
        family = rows_by_family.get(expected["familyId"])
        require(family is not None, f"main dissolve family is missing: {expected['familyId']}")
        source = family["source"]
        require(
            str(source.get("objectPath") or "").casefold()
            == expected["baseMaterialObjectPath"].casefold(),
            f"main dissolve base Material changed: {expected['label']}",
        )
        native_references = [
            normalize_logical_texture(str(row["objectPath"]))
            for row in source.get("referencedTextures", [])
        ]
        require(
            len(native_references) == expected["expectedNativeReferencedTextureCount"],
            f"main dissolve native texture denominator changed: {expected['label']}",
        )
        lookups = source.get("legacyTextureLookups", [])
        require(not lookups, f"main dissolve legacy lookup unexpectedly appeared: {expected['label']}")
        recipe_ids = {
            str(occurrences[occurrence_id].get("materialRecipeId") or "")
            for occurrence_id in expected["occurrenceIds"]
        }
        require(len(recipe_ids) == 1, f"main dissolve recipe identity is ambiguous: {expected['label']}")
        recipe_id = next(iter(recipe_ids))
        recipe = recipes.get(recipe_id)
        require(recipe is not None, f"main dissolve recipe is missing: {expected['label']}")
        require(
            recipe.get("arithmeticFamilyId") == expected["familyId"],
            f"main dissolve recipe family changed: {expected['label']}",
        )
        effective_bindings = []
        for group_name in ("textureOverrides", "parentDefaults"):
            for row in (recipe.get("inputs") or {}).get(group_name, []):
                if row.get("fieldKind") != "texture":
                    continue
                effective_bindings.append(
                    {
                        "parameterName": str(
                            row.get("normalizedParameterName") or row.get("parameterName") or ""
                        ).casefold(),
                        "logicalTexturePath": normalize_logical_texture(str(row.get("value") or "")),
                        "bindingOrigin": row.get("bindingOrigin"),
                        "closerOverridePresent": row.get("closerOverridePresent"),
                    }
                )
        effective_unique = sorted({row["logicalTexturePath"] for row in effective_bindings})
        overlap = sorted(set(native_references) & set(effective_unique))
        require(
            len(effective_bindings) == expected["expectedEffectiveBindingCount"]
            and len(effective_unique) == expected["expectedEffectiveUniqueTextureCount"]
            and len(overlap) == expected["expectedNativeEffectiveOverlapCount"],
            f"main dissolve effective texture denominator changed: {expected['label']}",
        )
        result_rows.append(
            {
                "label": expected["label"],
                "occurrenceIds": expected["occurrenceIds"],
                "familyId": expected["familyId"],
                "baseMaterialObjectPath": source["objectPath"],
                "nativeReferencedTexturePaths": native_references,
                "nativeReferencedTextureCount": len(native_references),
                "legacyTextureLookupCount": 0,
                "effectiveRecipeId": recipe_id,
                "effectiveSourceMaterialPath": recipe.get("sourceMaterialPath"),
                "effectiveTextureBindings": effective_bindings,
                "effectiveBindingCount": len(effective_bindings),
                "effectiveUniqueTexturePaths": effective_unique,
                "effectiveUniqueTextureCount": len(effective_unique),
                "nativeEffectiveTextureOverlap": overlap,
                "nativeEffectiveTextureOverlapCount": len(overlap),
                "newEffectiveTextureMembershipFromNativeMetadata": [],
                "metadataEffectiveTextureMembershipDeltaCount": 0,
                "shaderUvUniformAdmission": False,
                "arithmeticGraphAdmission": False,
                "decision": (
                    "NO_LEGACY_LOOKUP_NO_NEW_EFFECTIVE_MEMBERSHIP_"
                    "EXISTING_TYPED_RECIPE_REMAINS_AUTHORITY"
                ),
            }
        )
    return {
        "rows": result_rows,
        "summary": {
            "priorityFamilyCount": 2,
            "priorityOccurrenceCount": 3,
            "legacyTextureLookupCount": 0,
            "nativeReferencedTextureCount": 6,
            "effectiveBindingCount": 9,
            "effectiveUniqueTextureCountByFamily": [2, 6],
            "nativeEffectiveTextureOverlapCount": 1,
            "metadataEffectiveTextureMembershipDeltaCount": 0,
            "shaderUvUniformAdmittedCount": 0,
            "arithmeticGraphAdmittedCount": 0,
        },
        "nextEvidenceOwner": "MATCHING_REVISION_FMATERIALSHADERMAP_OR_SHADER_MAP_ID_DERIVATION",
    }


def build_receipt(
    material_contract_path: Path,
    shader_cache_receipt_path: Path,
    source_package_root: Path,
    global_material_package_path: Path,
    texture_binding_receipt_path: Path = DEFAULT_TEXTURE_BINDING_RECEIPT,
    active_material_closure_path: Path = DEFAULT_ACTIVE_MATERIAL_CLOSURE,
    runtime_texture_root: Path = DEFAULT_RUNTIME_TEXTURE_ROOT,
) -> dict[str, Any]:
    contract = read_json(material_contract_path)
    shader_receipt = read_json(shader_cache_receipt_path)
    families = material_family_projection(contract)
    require(len(families) == EXPECTED_FAMILY_COUNT, "Material family denominator changed")
    global_rows_by_family = {
        row["familyId"]: row for row in shader_receipt.get("materialNativeKeys", [])
    }
    require(len(global_rows_by_family) == EXPECTED_FAMILY_COUNT, "global Material denominator changed")

    global_package = load_package(global_material_package_path, LOSTARK_KR_AES_KEY)
    require(global_package.summary.version == 868, "global Material package version changed")
    package_cache: dict[str, Any] = {}
    source_packages: dict[str, dict[str, Any]] = {}
    rows = []
    for family in families:
        package_name = family["physicalPackage"]
        package_path = source_package_root / package_name
        require(package_path.is_file(), f"source package is missing: {package_name}")
        require(raw_file_sha256(package_path) == family["physicalPackageSha256"], "source package SHA changed")
        if package_name not in package_cache:
            package_cache[package_name] = load_package(package_path, LOSTARK_KR_AES_KEY)
            source_package = package_cache[package_name]
            require(source_package.summary.version == 868, "source package version changed")
            source_packages[package_name] = {
                "fileName": package_name,
                "physicalByteSize": package_path.stat().st_size,
                "rawSha256": family["physicalPackageSha256"],
                "packageVersion": source_package.summary.version,
                "licenseeVersion": source_package.summary.licensee_version,
                "logicalByteSize": len(source_package.logical),
                "exportCount": len(source_package.exports),
                "hashRole": "EXTERNAL_RAW_BYTES",
            }
        source_package = package_cache[package_name]
        source = decode_export(source_package, family["materialExportIndex"], family["materialObjectPath"])

        global_identity = global_rows_by_family[family["familyId"]]
        global_copy = decode_export(
            global_package,
            global_identity["exportIndex"],
            global_identity["globalMaterialObjectPath"],
        )
        source_semantic = semantic_projection(source)
        global_semantic = semantic_projection(global_copy)
        require(source_semantic == global_semantic, "source/global Material resource semantics differ")
        rows.append(
            {
                "familyId": family["familyId"],
                "sourcePhysicalPackage": package_name,
                "sourcePhysicalPackageSha256": family["physicalPackageSha256"],
                "source": source,
                "currentGlobal": global_copy,
                "semanticProjection": source_semantic,
                "semanticSha256": canonical_json_sha256(source_semantic),
                "sourceGlobalSemanticParity": True,
                "fidelity": "SOURCE_EXACT_COOKED_MATERIAL_RESOURCE_METADATA",
                "arithmeticGraphAdmission": False,
                "shaderUvUniformAdmission": False,
                "productAdmission": False,
            }
        )

    rows.sort(key=lambda row: row["familyId"])
    texture_count = sum(len(row["source"]["referencedTextures"]) for row in rows)
    lookup_count = sum(len(row["source"]["legacyTextureLookups"]) for row in rows)
    lookup_family_count = sum(bool(row["source"]["legacyTextureLookups"]) for row in rows)
    require(texture_count == EXPECTED_REFERENCED_TEXTURE_COUNT, "ReferencedTextures denominator changed")
    require(lookup_count == EXPECTED_LOOKUP_COUNT, "legacy texture lookup denominator changed")
    require(lookup_family_count == EXPECTED_LOOKUP_FAMILY_COUNT, "lookup family denominator changed")
    require(all(row["source"]["unparsedByteCount"] == 0 for row in rows), "source tail remainder exists")
    require(all(row["currentGlobal"]["unparsedByteCount"] == 0 for row in rows), "global tail remainder exists")
    texture_closure = build_texture_closure_diagnostics(
        rows,
        contract,
        read_json(active_material_closure_path),
        read_json(texture_binding_receipt_path),
        runtime_texture_root,
    )
    require(texture_closure["uniqueReferencedTextureCount"] == 55, "unique ReferencedTextures denominator changed")
    require(texture_closure["artistRuntimeBoundReferencedTextureCount"] == 17, "runtime-bound reference denominator changed")
    require(texture_closure["artistRuntimeUnboundReferencedTextureCount"] == 38, "runtime-unbound reference denominator changed")
    require(texture_closure["artistDomainAvailableReferencedTextureCount"] == 34, "Artist DDS availability denominator changed")
    require(texture_closure["artistDomainMissingReferencedTextureCount"] == 21, "Artist DDS missing denominator changed")
    require(texture_closure["artistMissingButOtherEffectDomainAvailableCount"] == 11, "other-domain candidate denominator changed")
    require(texture_closure["allEffectDomainsMissingReferencedTextureCount"] == 10, "all-domain missing denominator changed")
    require(texture_closure["lookupBackedUniqueTextureCount"] == 7, "lookup-backed texture denominator changed")
    require(texture_closure["unboundLookupBackedUniqueTextureCount"] == 4, "unbound lookup-backed texture denominator changed")
    require(texture_closure["overriddenLookupBackedUniqueTextureCount"] == 5, "overridden lookup denominator changed")
    require(texture_closure["differentReplacementLookupBackedUniqueTextureCount"] == 4, "replacement lookup denominator changed")
    require(texture_closure["identityOverrideLookupBackedUniqueTextureCount"] == 1, "identity override denominator changed")
    require(texture_closure["effectiveLookupBackedUniqueTextureCount"] == 3, "effective lookup denominator changed")
    require(texture_closure["effectiveLookupMissingRuntimeBindingCount"] == 0, "effective lookup binding gap opened")
    require(
        texture_closure["overriddenLookupEffectiveOverrideMissingRuntimeBindingCount"] == 0,
        "effective MIC override binding gap opened",
    )
    main_dissolve_priority = build_main_dissolve_priority(rows, contract)

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "sourceFamilyProjection": {
            "familyCount": EXPECTED_FAMILY_COUNT,
            "projectionSha256": canonical_json_sha256(families),
            "rows": families,
        },
        "externalEvidence": {
            "sourcePackages": sorted(source_packages.values(), key=lambda row: row["fileName"].casefold()),
            "globalMaterialPackage": {
                "fileName": global_material_package_path.name,
                "physicalByteSize": global_material_package_path.stat().st_size,
                "rawSha256": raw_file_sha256(global_material_package_path),
                "packageVersion": global_package.summary.version,
                "licenseeVersion": global_package.summary.licensee_version,
                "logicalByteSize": len(global_package.logical),
                "exportCount": len(global_package.exports),
                "hashRole": "EXTERNAL_RAW_BYTES",
            },
        },
        "toolDependencies": [
            {
                "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
                "sha256": canonical_text_sha256(SCRIPT_PATH),
                "hashRole": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
            }
        ],
        "closureDependencies": [
            {
                "path": texture_binding_receipt_path.relative_to(REPO_ROOT).as_posix(),
                "sha256": canonical_json_sha256(read_json(texture_binding_receipt_path)),
                "hashRole": "CANONICAL_JSON",
            },
            {
                "path": active_material_closure_path.relative_to(REPO_ROOT).as_posix(),
                "sha256": canonical_json_sha256(read_json(active_material_closure_path)),
                "hashRole": "CANONICAL_JSON",
            },
            {
                "path": runtime_texture_root.relative_to(REPO_ROOT).as_posix(),
                "sha256": canonical_json_sha256(runtime_texture_inventory(runtime_texture_root)),
                "hashRole": "RUNTIME_DDS_INVENTORY_CANONICAL_JSON",
            },
        ],
        "families": rows,
        "textureClosureDiagnostics": texture_closure,
        "mainDissolvePriority": main_dissolve_priority,
        "decision": {
            "sourceExactLegacyTextureMetadata": True,
            "shaderUvUniformAdmission": False,
            "arithmeticGraphAdmission": False,
            "inlineShaderPayloadAvailable": False,
            "productAdmission": False,
            "nextEvidenceOwner": "SAME_REVISION_SHADERCACHE_OR_CONTROLLED_MATERIAL_SHADER_MAP_CAPTURE",
        },
        "summary": {
            "familyCount": len(rows),
            "sourceParsedTailCount": len(rows),
            "currentGlobalParsedTailCount": len(rows),
            "sourceGlobalSemanticParityCount": sum(row["sourceGlobalSemanticParity"] for row in rows),
            "referencedTextureCount": texture_count,
            "legacyTextureLookupFamilyCount": lookup_family_count,
            "legacyTextureLookupCount": lookup_count,
            "sourceUnparsedByteCount": sum(row["source"]["unparsedByteCount"] for row in rows),
            "currentGlobalUnparsedByteCount": sum(row["currentGlobal"]["unparsedByteCount"] for row in rows),
            "inlineShaderPayloadFamilyCount": 0,
            "uniqueReferencedTextureCount": texture_closure["uniqueReferencedTextureCount"],
            "artistRuntimeBoundReferencedTextureCount": texture_closure["artistRuntimeBoundReferencedTextureCount"],
            "artistRuntimeUnboundReferencedTextureCount": texture_closure["artistRuntimeUnboundReferencedTextureCount"],
            "artistDomainAvailableReferencedTextureCount": texture_closure["artistDomainAvailableReferencedTextureCount"],
            "artistDomainMissingReferencedTextureCount": texture_closure["artistDomainMissingReferencedTextureCount"],
            "artistMissingButOtherEffectDomainAvailableCount": texture_closure["artistMissingButOtherEffectDomainAvailableCount"],
            "allEffectDomainsMissingReferencedTextureCount": texture_closure["allEffectDomainsMissingReferencedTextureCount"],
            "lookupBackedUniqueTextureCount": texture_closure["lookupBackedUniqueTextureCount"],
            "unboundLookupBackedUniqueTextureCount": texture_closure["unboundLookupBackedUniqueTextureCount"],
            "overriddenLookupBackedUniqueTextureCount": texture_closure["overriddenLookupBackedUniqueTextureCount"],
            "differentReplacementLookupBackedUniqueTextureCount": texture_closure[
                "differentReplacementLookupBackedUniqueTextureCount"
            ],
            "identityOverrideLookupBackedUniqueTextureCount": texture_closure[
                "identityOverrideLookupBackedUniqueTextureCount"
            ],
            "effectiveLookupBackedUniqueTextureCount": texture_closure["effectiveLookupBackedUniqueTextureCount"],
            "effectiveLookupMissingRuntimeBindingCount": texture_closure["effectiveLookupMissingRuntimeBindingCount"],
            "overriddenLookupEffectiveOverrideMissingRuntimeBindingCount": texture_closure[
                "overriddenLookupEffectiveOverrideMissingRuntimeBindingCount"
            ],
            "mainDissolvePriorityFamilyCount": main_dissolve_priority["summary"]["priorityFamilyCount"],
            "mainDissolveLegacyTextureLookupCount": main_dissolve_priority["summary"]["legacyTextureLookupCount"],
            "mainDissolveMetadataEffectiveTextureMembershipDeltaCount": main_dissolve_priority["summary"][
                "metadataEffectiveTextureMembershipDeltaCount"
            ],
        },
    }
    receipt["receiptSha256"] = canonical_json_sha256(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any], material_contract_path: Path) -> None:
    require(receipt.get("schema") == SCHEMA, "native resource receipt schema mismatch")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "native resource receipt version mismatch")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(sealed), "native resource receipt digest mismatch")
    families = material_family_projection(read_json(material_contract_path))
    require(
        receipt.get("sourceFamilyProjection") == {
            "familyCount": EXPECTED_FAMILY_COUNT,
            "projectionSha256": canonical_json_sha256(families),
            "rows": families,
        },
        "native resource family projection changed",
    )
    summary = receipt.get("summary") or {}
    decision = receipt.get("decision") or {}
    require(
        summary == {
            "familyCount": 23,
            "sourceParsedTailCount": 23,
            "currentGlobalParsedTailCount": 23,
            "sourceGlobalSemanticParityCount": 23,
            "referencedTextureCount": 64,
            "legacyTextureLookupFamilyCount": 4,
            "legacyTextureLookupCount": 7,
            "sourceUnparsedByteCount": 0,
            "currentGlobalUnparsedByteCount": 0,
            "inlineShaderPayloadFamilyCount": 0,
            "uniqueReferencedTextureCount": 55,
            "artistRuntimeBoundReferencedTextureCount": 17,
            "artistRuntimeUnboundReferencedTextureCount": 38,
            "artistDomainAvailableReferencedTextureCount": 34,
            "artistDomainMissingReferencedTextureCount": 21,
            "artistMissingButOtherEffectDomainAvailableCount": 11,
            "allEffectDomainsMissingReferencedTextureCount": 10,
            "lookupBackedUniqueTextureCount": 7,
            "unboundLookupBackedUniqueTextureCount": 4,
            "overriddenLookupBackedUniqueTextureCount": 5,
            "differentReplacementLookupBackedUniqueTextureCount": 4,
            "identityOverrideLookupBackedUniqueTextureCount": 1,
            "effectiveLookupBackedUniqueTextureCount": 3,
            "effectiveLookupMissingRuntimeBindingCount": 0,
            "overriddenLookupEffectiveOverrideMissingRuntimeBindingCount": 0,
            "mainDissolvePriorityFamilyCount": 2,
            "mainDissolveLegacyTextureLookupCount": 0,
            "mainDissolveMetadataEffectiveTextureMembershipDeltaCount": 0,
        },
        "native resource denominator changed",
    )
    main_dissolve = receipt.get("mainDissolvePriority") or {}
    require(
        main_dissolve.get("summary") == {
            "priorityFamilyCount": 2,
            "priorityOccurrenceCount": 3,
            "legacyTextureLookupCount": 0,
            "nativeReferencedTextureCount": 6,
            "effectiveBindingCount": 9,
            "effectiveUniqueTextureCountByFamily": [2, 6],
            "nativeEffectiveTextureOverlapCount": 1,
            "metadataEffectiveTextureMembershipDeltaCount": 0,
            "shaderUvUniformAdmittedCount": 0,
            "arithmeticGraphAdmittedCount": 0,
        }
        and main_dissolve.get("nextEvidenceOwner")
        == "MATCHING_REVISION_FMATERIALSHADERMAP_OR_SHADER_MAP_ID_DERIVATION"
        and main_dissolve == build_main_dissolve_priority(receipt.get("families") or [], read_json(material_contract_path)),
        "main dissolve native metadata priority boundary changed",
    )
    require(
        decision.get("sourceExactLegacyTextureMetadata") is True
        and decision.get("shaderUvUniformAdmission") is False
        and decision.get("arithmeticGraphAdmission") is False
        and decision.get("inlineShaderPayloadAvailable") is False
        and decision.get("productAdmission") is False,
        "native resource fidelity boundary changed",
    )
    rows = receipt.get("families") or []
    require(len(rows) == EXPECTED_FAMILY_COUNT, "native resource family rows changed")
    for row in rows:
        require(row.get("sourceGlobalSemanticParity") is True, "source/global parity changed")
        require(row.get("semanticProjection") == semantic_projection(row["source"]), "source semantic projection changed")
        require(row.get("semanticProjection") == semantic_projection(row["currentGlobal"]), "global semantic projection changed")
        require(row.get("semanticSha256") == canonical_json_sha256(row["semanticProjection"]), "semantic digest changed")
        require(row.get("arithmeticGraphAdmission") is False, "native metadata opened arithmetic graph")
        require(row.get("shaderUvUniformAdmission") is False, "legacy lookup opened shader UV")
        require(row.get("productAdmission") is False, "native metadata opened Product")
    dependencies = receipt.get("toolDependencies") or []
    require(len(dependencies) == 1, "native resource dependency set changed")
    require(
        dependencies[0].get("path") == SCRIPT_PATH.relative_to(REPO_ROOT).as_posix()
        and dependencies[0].get("hashRole") == "TRACKED_SOURCE_EOL_CANONICAL_TEXT"
        and dependencies[0].get("sha256") == canonical_text_sha256(SCRIPT_PATH),
        "native resource tool identity changed",
    )
    closure_dependencies = receipt.get("closureDependencies") or []
    require(len(closure_dependencies) == 3, "native resource closure dependency set changed")
    expected_closure_dependencies = [
        {
            "path": DEFAULT_TEXTURE_BINDING_RECEIPT.relative_to(REPO_ROOT).as_posix(),
            "sha256": canonical_json_sha256(read_json(DEFAULT_TEXTURE_BINDING_RECEIPT)),
            "hashRole": "CANONICAL_JSON",
        },
        {
            "path": DEFAULT_ACTIVE_MATERIAL_CLOSURE.relative_to(REPO_ROOT).as_posix(),
            "sha256": canonical_json_sha256(read_json(DEFAULT_ACTIVE_MATERIAL_CLOSURE)),
            "hashRole": "CANONICAL_JSON",
        },
        {
            "path": DEFAULT_RUNTIME_TEXTURE_ROOT.relative_to(REPO_ROOT).as_posix(),
            "sha256": canonical_json_sha256(runtime_texture_inventory(DEFAULT_RUNTIME_TEXTURE_ROOT)),
            "hashRole": "RUNTIME_DDS_INVENTORY_CANONICAL_JSON",
        },
    ]
    require(closure_dependencies == expected_closure_dependencies, "native resource closure dependency changed")


def write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--shader-cache-receipt", type=Path, default=DEFAULT_SHADER_CACHE_RECEIPT)
    parser.add_argument("--source-package-root", type=Path, default=DEFAULT_SOURCE_PACKAGE_ROOT)
    parser.add_argument("--global-material-package", type=Path, default=DEFAULT_GLOBAL_MATERIAL_PACKAGE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--texture-binding-receipt", type=Path, default=DEFAULT_TEXTURE_BINDING_RECEIPT)
    parser.add_argument("--active-material-closure", type=Path, default=DEFAULT_ACTIVE_MATERIAL_CLOSURE)
    parser.add_argument("--runtime-texture-root", type=Path, default=DEFAULT_RUNTIME_TEXTURE_ROOT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        validate_receipt(read_json(args.output), args.material_contract)
    else:
        receipt = build_receipt(
            args.material_contract,
            args.shader_cache_receipt,
            args.source_package_root,
            args.global_material_package,
            args.texture_binding_receipt,
            args.active_material_closure,
            args.runtime_texture_root,
        )
        validate_receipt(receipt, args.material_contract)
        if args.check:
            require(read_json(args.output) == receipt, "native resource receipt is stale")
        else:
            write_json(args.output, receipt)
    print(
        "Artist F Material native resource check: "
        "families=23 textures=64/55 exact-bound=17 artist-dds=34/55 "
        "lookups=4/7 effective=3 replaced=4 identity-override=1 "
        "main-dissolve=2 lookup=0 delta=0 effective-missing=0 "
        "parity=23 remainder=0 inline-shader=0 product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
