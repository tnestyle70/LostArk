#!/usr/bin/env python3
"""Build a loss-aware UE3 source-material contract for one Effect document."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import copy
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def folded(value: Any) -> str:
    return str(value or "").strip().casefold()


def stable_profile_id(parent_path: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", ".", folded(parent_path)).strip(".")
    digest = hashlib.sha256(folded(parent_path).encode("utf-8")).hexdigest()[:12]
    return f"ue3.material.{slug[:72]}.{digest}"


def runtime_shader_profile_id(parent_path: str, source_path: str) -> str:
    exact_profiles = {
        "bfx_m.bfx_d_pa_circ_01_ad": "effect.ue3.circle.v1",
        "fx_m.fx_j_pa_dot_ad_01": "effect.ue3.dot.v1",
        "fx_m.fx_d_pa_ring_11_tr": "effect.ue3.ring.v1",
        "fx_m.fx_c_pa_aura_02_tr": "effect.ue3.aura.v1",
        "fx_mm.fx_mm_onelayerdistortion_02_ad": (
            "effect.ue3.one-layer-distortion.v1"
        ),
        "fx_m.fx_f_pa_shine_01_0_tr": "effect.ue3.shine.v1",
        "fx_m.fx_j_pa_blacklineaura_01_tr": (
            "effect.ue3.blackline-aura.v1"
        ),
        "fx_m.fx_j_pa_linearflow_02_tr": (
            "effect.ue3.linearflow-02.v1"
        ),
        "fx_m.fx_j_pa_slice_01_tr": "effect.ue3.slice.v1",
        "fx_m.fx_m_pa_missiletrail_01_tr": (
            "effect.ue3.missiletrail-01.v1"
        ),
        "fx_m.fx_j_me_localcrack_01_tr": "effect.ue3.local-crack.v1",
        "bfx_m.bfx_i_pa_glow_01_ad": (
            "effect.ue3.procedural-center-glow.v1"
        ),
    }
    # UModel can spell a parent either as ``package.group.object`` or as the
    # shorter ``group.object`` used by the original finite-profile table.  The
    # package prefix is provenance, not a different Material.  Match only a
    # full identity or a dot-boundary suffix so similarly named objects do not
    # get promoted accidentally.
    identities = [folded(parent_path), folded(source_path)]
    for identity in identities:
        if not identity:
            continue
        for material_path, profile_id in exact_profiles.items():
            if identity == material_path or identity.endswith(
                "." + material_path
            ):
                return profile_id
    return ""


def merged_source_parameters(
    selected: dict[str, Any], graph_row: dict[str, Any]
) -> dict[str, list[dict[str, Any]]]:
    """Preserve parent defaults/groups while applying child MI overrides.

    Parent Material3 ``Collected*Parameters`` are the only source of a
    parameter's declared group and default.  A MaterialInstance override is
    matched by parameter name case-insensitively and changes only its runtime
    value.  An override without a declared parent parameter remains visible as
    such; it is not silently promoted to an inferred parent declaration.
    """
    evidence = selected.get("materialEvidence") or {}
    parameter_kinds = (
        ("textures", "collectedTextureParameters", "textures", "texture"),
        ("scalars", "collectedScalarParameters", "scalars", "value"),
        ("vectors", "collectedVectorParameters", "vectors", "value"),
        (
            "staticSwitches",
            "collectedStaticSwitchParameters",
            "static_switches",
            "value",
        ),
    )
    result: dict[str, list[dict[str, Any]]] = {}
    for output_key, parent_key, child_key, value_key in parameter_kinds:
        rows: list[dict[str, Any]] = []
        by_name: dict[str, dict[str, Any]] = {}
        for parent_row in evidence.get(parent_key, []) or []:
            name = str(parent_row.get("name") or "")
            key = folded(name)
            if not key or key in by_name:
                continue
            default_value = copy.deepcopy(parent_row.get(value_key))
            row = {
                "name": name,
                "group": str(parent_row.get("group") or ""),
                "defaultValue": default_value,
                value_key: copy.deepcopy(default_value),
                "valueSource": "PARENT_DEFAULT",
            }
            rows.append(row)
            by_name[key] = row
        child_rows = selected.get(child_key)
        if not child_rows:
            child_rows = graph_row.get(child_key) or []
        for child_row in child_rows:
            name = str(child_row.get("name") or "")
            key = folded(name)
            if not key:
                continue
            value = copy.deepcopy(child_row.get(value_key))
            if key in by_name:
                by_name[key][value_key] = value
                by_name[key]["valueSource"] = "INSTANCE_OVERRIDE"
                continue
            row = {
                "name": name,
                "group": "",
                "defaultValue": None,
                value_key: value,
                "valueSource": "INSTANCE_OVERRIDE_WITHOUT_PARENT_DECLARATION",
            }
            rows.append(row)
            by_name[key] = row
        result[output_key] = rows
    return result


def merge_parent_graph_texture_defaults(
    selected: dict[str, Any],
    parent_path: str,
    material_graph_evidence: dict[str, Any] | None,
) -> dict[str, Any]:
    """Merge only source-declared texture defaults from direct UPK evidence.

    Cooked Material3 props can omit TextureObjectParameter declarations that
    survive as expression exports.  This preserves their exact names, groups,
    and texture objects without inventing missing graph edges or claiming
    runtime-exact topology.
    """
    if not material_graph_evidence:
        return selected
    rows = [
        row for row in material_graph_evidence.get("materials", [])
        if folded(row.get("materialPath")) == folded(parent_path)
        or folded(parent_path).endswith(
            "." + folded(row.get("materialPath"))
        )
    ]
    if len(rows) != 1:
        return selected
    result = copy.deepcopy(selected)
    evidence = result.setdefault("materialEvidence", {})
    collected = evidence.setdefault("collectedTextureParameters", [])
    by_name = {
        folded(row.get("name")): row
        for row in collected
        if row.get("name")
    }
    for graph_texture in rows[0].get("namedTextures", []):
        name = str(graph_texture.get("name") or "")
        source = str(graph_texture.get("sourceObjectPath") or "")
        key = folded(name)
        if not key or not source:
            continue
        existing = by_name.get(key)
        if existing is not None:
            existing_source = str(existing.get("texture") or "")
            existing_object = folded(existing_source).rsplit(".", 1)[-1]
            graph_object = folded(source).rsplit(".", 1)[-1]
            if (
                existing_source
                and folded(existing_source) != folded(source)
                and existing_object != graph_object
            ):
                raise ValueError(
                    "parent Material texture evidence conflicts for "
                    f"{parent_path}:{name}"
                )
            continue
        declared = {
            "name": name,
            "group": str(graph_texture.get("group") or ""),
            "texture": source,
        }
        collected.append(declared)
        by_name[key] = declared
    evidence["cookedGraphTopologyStatus"] = str(
        (rows[0].get("summary") or {}).get("topologyStatus") or ""
    )
    evidence["cookedGraphRuntimeExactEligible"] = bool(
        (rows[0].get("summary") or {}).get("runtimeExactEligible")
    )
    return result


def grouped_translucent_selection(
    exact_shader_profile_id: str,
    fallback_blocked_reason: str,
    role_bindings: list[dict[str, str]],
    role_diagnostics: list[dict[str, Any]],
    exact_required_bindings_available: bool = True,
    current_safe_bindings: list[dict[str, str]] | None = None,
) -> tuple[str, str | None]:
    """Choose an exact profile or a bounded reconstructed carrier profile.

    A reconstructed material can use the grouped translucent executor only
    when an alpha/emission/base parent group has an actual resolved runtime
    asset.  Cooked parents sometimes omit that group while the imported
    emitter still has a concrete, validated Base/Mask/Emissive binding.  That
    carrier is also admitted as a visibly approximate reconstruction; the
    source profile remains ``RECONSTRUCTED_PROFILE`` and is never reported as
    graph-exact.  Noise-only and name-only mappings remain insufficient.
    """
    if exact_shader_profile_id:
        if exact_required_bindings_available:
            return exact_shader_profile_id, None
        return (
            "effect.ue3.fallback-blocked.v1",
            "MISSING_EXISTING_FINITE_PROFILE_REQUIRED_RUNTIME_RESOURCE",
        )
    if fallback_blocked_reason:
        return "effect.ue3.fallback-blocked.v1", fallback_blocked_reason

    primary_names = {
        folded(row.get("parameterName"))
        for row in role_diagnostics
        if source_group_slot(str(row.get("parentGroup") or ""))
        in {"mask", "emission", "base"}
    }
    resolved_primary_names = {
        folded(row.get("parameterName"))
        for row in role_bindings
        if folded(row.get("parameterName")) in primary_names
        and is_safe_grouped_carrier_binding(row)
    }
    if resolved_primary_names:
        return "effect.ue3.grouped-translucent.v1", None
    if primary_names:
        return (
            "effect.ue3.fallback-blocked.v1",
            "MISSING_GROUPED_TRANSPARENT_RUNTIME_RESOURCE",
        )
    safe_carrier_slots = {
        folded(row.get("slotId"))
        for row in (current_safe_bindings or [])
        if is_safe_grouped_carrier_binding(row)
    }
    if safe_carrier_slots & {"base", "mask", "emissive"}:
        return "effect.ue3.grouped-translucent.v1", None
    return "effect.ue3.fallback-blocked.v1", "UNKNOWN_GROUPED_TRANSPARENT_INPUT"


def runtime_binding_files_available(
    bindings: list[dict[str, str]], runtime_resource_root: Path | None
) -> bool:
    """Verify required finite-profile assets when a resource root is known."""
    if runtime_resource_root is None:
        return True
    for binding in bindings:
        asset_id = str(binding.get("assetId") or "")
        relative = Path(asset_id)
        if not asset_id or relative.is_absolute() or ".." in relative.parts:
            return False
        if not (runtime_resource_root / relative).is_file():
            return False
    return True


def required_runtime_bindings(shader_profile_id: str) -> list[dict[str, str]]:
    bindings = {
        "effect.ue3.ring.v1": [
            {
                "slotId": "base",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_bg_waterspray_01.dds"
                ),
            },
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_02/"
                    "fx_d_noise_009.dds"
                ),
            },
        ],
        "effect.ue3.aura.v1": [
            {
                "slotId": "base",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_a_glow_009.dds"
                ),
            },
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_HIGH_00/"
                    "fx_a_cloud_026.dds"
                ),
            },
        ],
        "effect.ue3.one-layer-distortion.v1": [
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_a_noise_002.dds"
                ),
            },
        ],
        "effect.ue3.slice.v1": [
            {
                "slotId": "base",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_06/"
                    "fx_j_voronoi_tile_01.dds"
                ),
            },
        ],
        "effect.ue3.missiletrail-01.v1": [
            {
                "slotId": "base",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_05/"
                    "fx_m_atypical_007.dds"
                ),
            },
            {
                "slotId": "mask",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_05/"
                    "fx_m_atypical_007.dds"
                ),
            },
            {
                "slotId": "noise",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_00/"
                    "fx_a_cloud_022.dds"
                ),
            },
            {
                "slotId": "dissolve",
                "assetId": (
                    "Effect/DimensionMaster/Textures/FX_TEX_05/"
                    "fx_m_noise_001.dds"
                ),
            },
        ],
    }
    return copy.deepcopy(bindings.get(shader_profile_id, []))


def material_candidates(
    material_map: dict[str, Any], source_path: str
) -> list[dict[str, Any]]:
    materials = material_map.get("materials", {})
    keys = [folded(source_path)]
    parts = source_path.split(".")
    if len(parts) > 1:
        keys.append(folded(".".join(parts[1:])))
    keys.append(folded(parts[-1]))
    matches: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()
    for key in keys:
        for row in materials.get(key, []):
            identity = (
                folded(row.get("source_file")),
                folded(row.get("material_path")),
            )
            if identity in seen:
                continue
            seen.add(identity)
            matches.append(row)
    return matches


def normalized_graph_material_candidates(
    source_graph: dict[str, Any], source_path: str
) -> list[dict[str, Any]]:
    """Return exact normalized-graph rows without suffix/name guessing.

    A skill source receipt already preserves the fully qualified material
    object path used by each emitter.  That row is sufficient fallback
    evidence when a separately exported parent-props map is unavailable, but
    only when the exact path occurs once.  Duplicate or missing rows must stay
    visible to the caller so product materialization can fail closed.
    """
    key = folded(source_path)
    if not key:
        return []
    return [
        row
        for row in source_graph.get("materialParameterBindings", [])
        if folded(row.get("sourceMaterialPath")) == key
    ]


def source_asset_packages(manifest: dict[str, Any]) -> dict[str, str]:
    packages: dict[str, str] = {}
    for row in manifest.get("assets", []):
        source_path = folded(row.get("sourceAssetPath"))
        physical = str(row.get("physicalPackage") or "")
        if source_path and physical:
            packages[source_path] = physical
    return packages


def runtime_resource_bindings(graph: dict[str, Any]) -> dict[str, str]:
    return {
        folded(row.get("sourceObjectPath")): str(row.get("assetId"))
        for row in graph.get("runtimeResourceBindings", [])
        if row.get("resolutionStatus") == "RESOLVED_RUNTIME_ASSET"
        and row.get("sourceObjectPath")
        and row.get("assetId")
    }


def manifest_texture_runtime_asset(
    resource_manifest: dict[str, Any], source_texture: str
) -> tuple[str | None, str]:
    """Resolve a parent texture through the admitted source manifest only.

    Parent Material3 props sometimes name a default texture that was not an
    instance override in the skill graph.  The class source manifest is the
    canonical bridge in that case.  A suffix match is accepted only when it is
    unique among resolved texture assets; any ambiguity stays fail-closed.
    """
    texture_key = folded(source_texture)
    if not texture_key:
        return None, "MISSING_RUNTIME_ASSET"
    object_name = texture_key.rsplit(".", 1)[-1]
    candidates = [
        row for row in resource_manifest.get("assets", [])
        if "texture" in [folded(role) for role in row.get("roles", [])]
        and folded(row.get("resolutionStatus")) == "resolved_source_package"
        and (
            folded(row.get("sourceAssetPath")) == texture_key
            or folded(row.get("sourceAssetPath")).endswith("." + object_name)
        )
    ]
    # The exact source object path always wins.  A similarly named texture in
    # another package is not allowed to turn an exact parent reference into an
    # ambiguity.
    exact = [
        row for row in candidates
        if folded(row.get("sourceAssetPath")) == texture_key
    ]
    if exact:
        candidates = exact
    if len(candidates) != 1:
        return (
            None,
            "AMBIGUOUS_MANIFEST_TEXTURE" if candidates else "MISSING_RUNTIME_ASSET",
        )
    row = candidates[0]
    logical_package = str(row.get("logicalPackage") or "").upper()
    source_name = str(row.get("sourceAssetPath") or "").rsplit(".", 1)[-1]
    if not logical_package or not source_name:
        return None, "MISSING_RUNTIME_ASSET"
    return (
        f"Effect/DimensionMaster/Textures/{logical_package}/{source_name}.dds",
        "RESOURCE_MANIFEST_SUFFIX",
    )


def source_group_slot(group: str) -> str | None:
    value = folded(group)
    if any(token in value for token in ("alpha", "opacity", "mask")):
        return "mask"
    if "dissolve" in value:
        return "dissolve"
    if any(token in value for token in ("uvdistort", "uv_noise", "noise")):
        return "noise"
    if any(token in value for token in ("emission", "emissive")):
        return "emission"
    if any(token in value for token in ("diffuse", "base", "color")):
        return "base"
    return None


def is_normal_or_bump_texture(*values: Any) -> bool:
    """Identify normal/bump payloads without assigning them a color role."""
    return any(
        any(token in folded(value) for token in ("normal", "bump"))
        for value in values
    )


def is_safe_grouped_carrier_binding(binding: dict[str, Any]) -> bool:
    """Return whether a binding can carry reconstructed color or alpha.

    Blank-white is a UE graph input, not a self-describing sprite image. It
    needs a separate mask or emissive carrier and must not reactivate a full
    quad by itself. Normal/bump assets are never color/alpha carriers.
    """
    slot = folded(binding.get("slotId"))
    asset_id = str(binding.get("assetId") or "")
    if slot not in {"base", "mask", "emissive"} or not asset_id:
        return False
    if is_normal_or_bump_texture(asset_id):
        return False
    if slot == "base" and "blankwhite" in folded(asset_id):
        return False
    return True


def evidence_backed_normal_noise_assets(
    role_diagnostics: list[dict[str, Any]],
) -> set[str]:
    """Return normal/bump textures explicitly declared as noise/distortion.

    Texture names alone are not enough to admit a normal map into the generic
    effect shader.  The parent Material's collected parameter group must name
    a noise/distortion role and the runtime asset must have resolved.
    """
    return {
        str(row.get("assetId") or "")
        for row in role_diagnostics
        if row.get("status") == "PARENT_GROUP_RUNTIME_ASSET"
        and row.get("runtimeRole") == "noise"
        and is_normal_or_bump_texture(
            row.get("parameterName"),
            row.get("sourceObjectPath"),
            row.get("assetId"),
        )
        and str(row.get("assetId") or "")
    }


def role_resolved_runtime_bindings(
    selected: dict[str, Any], graph: dict[str, Any],
    resource_manifest: dict[str, Any] | None = None,
) -> tuple[list[dict[str, str]], list[dict[str, Any]]]:
    """Map only parent-declared texture groups into the generic runtime slots.

    The parent group is source evidence for a texture's role, but it is not
    evidence for the original graph formula. Multiple textures of one role are
    therefore reported and reduced deterministically instead of being called
    runtime-exact.
    """
    evidence = selected.get("materialEvidence") or {}
    groups = {
        folded(row.get("name")): str(row.get("group") or "")
        for row in evidence.get("collectedTextureParameters", [])
        if row.get("name") and row.get("group")
    }
    runtime = runtime_resource_bindings(graph)
    def resolve_runtime_asset(texture: str) -> tuple[str | None, str]:
        exact = runtime.get(folded(texture))
        if exact:
            return exact, "GRAPH_RUNTIME_ASSET"
        suffix = "." + folded(texture).rsplit(".", 1)[-1]
        matches = {
            asset_id for source_path, asset_id in runtime.items()
            if source_path.endswith(suffix)
        }
        if len(matches) == 1:
            return next(iter(matches)), "GRAPH_RUNTIME_ASSET"
        if resource_manifest is not None:
            return manifest_texture_runtime_asset(resource_manifest, texture)
        return None, "MISSING_RUNTIME_ASSET"

    texture_rows = list(selected.get("textures") or [])
    overridden_names = {
        folded(row.get("name")) for row in texture_rows if row.get("name")
    }
    texture_rows.extend(
        {
            "name": row.get("name"),
            "texture": row.get("texture"),
        }
        for row in evidence.get("collectedTextureParameters", [])
        if folded(row.get("name")) not in overridden_names
    )
    candidates = []
    diagnostics = []
    for source_order, row in enumerate(texture_rows):
        name = str(row.get("name") or "")
        texture = str(row.get("texture") or "")
        group = groups.get(folded(name))
        slot = source_group_slot(group or "")
        asset_id, resolution_method = resolve_runtime_asset(texture)
        normal_or_bump = is_normal_or_bump_texture(name, texture, asset_id)
        normal_noise_evidence = bool(
            normal_or_bump and group and slot == "noise"
        )
        unsupported_reflection = bool(
            group and any(
                token in folded(group) for token in ("reflection", "reflect")
            )
        )
        if unsupported_reflection:
            status = "UNSUPPORTED_REFLECTION_RUNTIME_RESOURCE"
        elif normal_or_bump and not normal_noise_evidence:
            status = "BLOCKED_NORMAL_BUMP_NON_NOISE_ROLE"
        else:
            status = (
                "PARENT_GROUP_RUNTIME_ASSET"
                if group and slot and asset_id
                else "UNRESOLVED_PARENT_GROUP"
                if not group or not slot
                else resolution_method
            )
        diagnostics.append({
            "parameterName": name,
            "sourceObjectPath": texture,
            "parentGroup": group,
            "runtimeRole": slot,
            "assetId": asset_id,
            "status": status,
            "resolutionMethod": resolution_method,
            "normalBumpPolicy": (
                "EVIDENCE_BACKED_NOISE_DISTORTION"
                if normal_noise_evidence
                else "BLOCKED_OUTSIDE_NOISE_DISTORTION"
                if normal_or_bump
                else "NOT_NORMAL_OR_BUMP"
            ),
        })
        if status == "PARENT_GROUP_RUNTIME_ASSET":
            candidates.append((slot, asset_id, name, texture, source_order))

    bindings = []
    used_slots = set()
    non_emission = [row for row in candidates if row[0] != "emission"]
    emissions = [row for row in candidates if row[0] == "emission"]
    for slot, asset_id, name, texture, _ in non_emission:
        if slot in used_slots:
            continue
        used_slots.add(slot)
        bindings.append({
            "slotId": slot,
            "assetId": asset_id,
            "parameterName": name,
            "sourceObjectPath": texture,
            "status": "PARENT_GROUP_RECONSTRUCTED_BINDING",
        })
    for _, asset_id, name, texture, _ in emissions:
        slot = "emissive" if "base" in used_slots else "base"
        if slot in used_slots:
            continue
        used_slots.add(slot)
        bindings.append({
            "slotId": slot,
            "assetId": asset_id,
            "parameterName": name,
            "sourceObjectPath": texture,
            "status": "PARENT_GROUP_RECONSTRUCTED_BINDING",
        })
    return bindings, diagnostics


def blocked_runtime_asset_ids(
    selected: dict[str, Any], graph: dict[str, Any],
    evidence_backed_noise_assets: set[str] | None = None,
    role_diagnostics: list[dict[str, Any]] | None = None,
) -> list[str]:
    runtime = runtime_resource_bindings(graph)
    allowed = evidence_backed_noise_assets or set()
    result = []
    for source_path in selected.get("blockedTextureObjectPaths") or []:
        source = folded(source_path)
        asset_id = runtime.get(source)
        if not asset_id:
            suffix = "." + source.rsplit(".", 1)[-1]
            matches = {
                value for key, value in runtime.items() if key.endswith(suffix)
            }
            asset_id = next(iter(matches)) if len(matches) == 1 else None
        if asset_id and asset_id not in allowed and asset_id not in result:
            result.append(asset_id)
    # A parent-declared reflection input is not a generic Base/Mask/Emissive
    # carrier. Preserve it as named source evidence for a finite profile, but
    # remove it from legacy slots. Names such as ``*_cube`` do not prove the
    # UE export class; the sampling extractor validates the actual Texture2D.
    for row in role_diagnostics or []:
        if row.get("status") != "UNSUPPORTED_REFLECTION_RUNTIME_RESOURCE":
            continue
        asset_id = str(row.get("assetId") or "")
        if asset_id and asset_id not in result:
            result.append(asset_id)
    return result


def blocked_normal_bump_runtime_bindings(
    current_bindings: list[dict[str, str]],
    graph: dict[str, Any],
    role_diagnostics: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    """Build a slot-specific safety policy for legacy resource bindings.

    Imported documents predate the parent-Material evidence contract and may
    have put a normal map into Base/Mask/Emissive/Dissolve, or into Noise based
    on its filename alone.  Preserve those source paths in diagnostics, but do
    not send them to a color/alpha slot.  Noise is the sole permitted runtime
    use and only when the parent parameter group proves noise/distortion
    semantics.
    """
    source_paths_by_asset: dict[str, list[str]] = defaultdict(list)
    for source_path, asset_id in runtime_resource_bindings(graph).items():
        source_paths_by_asset[asset_id].append(source_path)
    allowed_noise_assets = evidence_backed_normal_noise_assets(
        role_diagnostics
    )
    evidence_normal_assets = {
        str(row.get("assetId") or "")
        for row in role_diagnostics
        if str(row.get("assetId") or "")
        and is_normal_or_bump_texture(
            row.get("parameterName"),
            row.get("sourceObjectPath"),
            row.get("assetId"),
        )
    }
    blocked = []
    for binding in current_bindings:
        slot_id = str(binding.get("slotId") or "")
        asset_id = str(binding.get("assetId") or "")
        source_paths = source_paths_by_asset.get(asset_id, [])
        if (
            asset_id not in evidence_normal_assets
            and not is_normal_or_bump_texture(asset_id, *source_paths)
        ):
            continue
        if slot_id == "noise" and asset_id in allowed_noise_assets:
            continue
        if slot_id not in {
            "base", "noise", "mask", "emissive", "dissolve"
        }:
            continue
        blocked.append({
            "slotId": slot_id,
            "assetId": asset_id,
            "sourceObjectPaths": source_paths,
            "reason": (
                "NORMAL_BUMP_NOISE_WITHOUT_PARENT_NOISE_DISTORTION_EVIDENCE"
                if slot_id == "noise"
                else "NORMAL_BUMP_FORBIDDEN_COLOR_ALPHA_ROLE"
            ),
            "policy": "PARENT_MATERIAL_TEXTURE_ROLE_FAIL_CLOSED",
        })
    return blocked


def count_status(value: Any, token: str) -> int:
    if isinstance(value, dict):
        return int(folded(value.get("status")) == folded(token)) + sum(
            count_status(child, token) for child in value.values()
        )
    if isinstance(value, list):
        return sum(count_status(child, token) for child in value)
    return 0


def normalize_scalar_parameter(row: dict[str, Any]) -> dict[str, Any]:
    result = {
        "name": str(row.get("name") or ""),
        "value": float(row.get("value") or 0.0),
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def normalize_texture_parameter(row: dict[str, Any]) -> dict[str, Any]:
    result = {
        "name": str(row.get("name") or ""),
        "sourceObjectPath": str(
            row.get("sourceObjectPath") or row.get("texture") or ""
        ),
        "assetId": str(row.get("assetId") or ""),
        "addressU": str(row.get("addressU") or "wrap"),
        "addressV": str(row.get("addressV") or "wrap"),
        "colorSpace": str(row.get("colorSpace") or "linear"),
        "samplingEvidence": str(
            row.get("samplingEvidence") or "legacy_default"
        ),
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def normalize_vector_parameter(row: dict[str, Any]) -> dict[str, Any]:
    value = row.get("value")
    if isinstance(value, dict):
        value = [
            float(value.get("r", 0.0)),
            float(value.get("g", 0.0)),
            float(value.get("b", 0.0)),
            float(value.get("a", 0.0)),
        ]
    if not isinstance(value, list) or len(value) != 4:
        value = [0.0, 0.0, 0.0, 0.0]
    result = {"name": str(row.get("name") or ""), "value": value}
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def normalize_switch_parameter(row: dict[str, Any]) -> dict[str, Any]:
    result = {
        "name": str(row.get("name") or ""),
        "value": bool(row.get("value")),
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def grouped_runtime_resource_contract_satisfied(
    source_profile: dict[str, Any], resources: list[dict[str, Any]]
) -> bool:
    """Mirror the C++ grouped-profile fail-closed resource contract.

    The publisher validates the same rule independently.  Applying it while
    materializing makes an occurrence explicitly fallback-blocked instead of
    publishing a grouped profile that the renderer will silently hide.
    """
    has_alpha = False
    has_emissive_parameter = False
    for row in source_profile.get("scalars") or []:
        identity = folded(
            f"{row.get('name') or ''} {row.get('group') or ''}"
        )
        has_alpha = has_alpha or any(
            token in identity for token in ("alpha", "mask", "opacity", "density")
        )
        has_emissive_parameter = has_emissive_parameter or "emiss" in identity
    for row in source_profile.get("vectors") or []:
        identity = folded(
            f"{row.get('name') or ''} {row.get('group') or ''}"
        )
        has_emissive_parameter = has_emissive_parameter or "emiss" in identity

    bindings = {
        folded(row.get("slotId")): str(row.get("assetId") or "")
        for row in resources
        if row.get("slotId") and row.get("assetId")
    }
    base_asset = bindings.get("base", "")
    safe_base = bool(base_asset) and not is_normal_or_bump_texture(base_asset) \
        and "blankwhite" not in folded(base_asset)
    has_mask = bool(bindings.get("mask"))
    has_emissive = bool(bindings.get("emissive"))
    has_dissolve = bool(bindings.get("dissolve"))
    if has_alpha and not (safe_base or has_mask or has_dissolve):
        return False
    if has_emissive_parameter and not (safe_base or has_emissive):
        return False
    return safe_base or has_mask or has_emissive


def finite_profile_runtime_resource_contract_satisfied(
    runtime_shader_profile_id: str,
    resources: list[dict[str, Any]],
    source_profile: dict[str, Any] | None = None,
) -> bool:
    """Require the carriers actually sampled by each bounded profile."""
    bindings = {
        folded(row.get("slotId")): str(row.get("assetId") or "")
        for row in resources
        if row.get("slotId") and row.get("assetId")
    }
    if runtime_shader_profile_id == "effect.ue3.shine.v1":
        base = bindings.get("base", "")
        return (
            bool(base)
            and not is_normal_or_bump_texture(base)
            and "blankwhite" not in folded(base)
            and bool(bindings.get("mask"))
        )
    if runtime_shader_profile_id == "effect.ue3.blackline-aura.v1":
        required = {
            "diffuse_tex", "flow_tex", "mask_a_tex", "mask_b_tex",
            "dissolve_tex",
        }
        resolved = {
            folded(row.get("name"))
            for row in (source_profile or {}).get("textures", [])
            if row.get("assetId")
        }
        return required.issubset(resolved)
    if runtime_shader_profile_id == "effect.ue3.linearflow-02.v1":
        required = {
            "diff_tex", "diff_noise_tex", "a_mask_tex", "a_noise_01_tex",
            "b_mask_tex", "b_noise_01_tex", "dissolve_tex",
        }
        resolved = {
            folded(row.get("name"))
            for row in (source_profile or {}).get("textures", [])
            if row.get("assetId")
        }
        return required.issubset(resolved)
    if runtime_shader_profile_id == "effect.ue3.slice.v1":
        base = bindings.get("base", "")
        return bool(base) and not is_normal_or_bump_texture(base)
    if runtime_shader_profile_id == "effect.ue3.missiletrail-01.v1":
        base = bindings.get("base", "")
        return (
            bool(base)
            and not is_normal_or_bump_texture(base)
            and bool(bindings.get("mask"))
            and bool(bindings.get("noise"))
            and bool(bindings.get("dissolve"))
            and bool(bindings.get("meshmodel"))
        )
    if runtime_shader_profile_id == "effect.ue3.local-crack.v1":
        required = {"normal_tex", "refle_tex", "dissolve_tex"}
        textures = {
            folded(row.get("name")): row
            for row in (source_profile or {}).get("textures", [])
            if row.get("name")
        }
        if not bool(bindings.get("meshmodel")) or not required.issubset(
            textures
        ):
            return False
        for name in required:
            row = textures[name]
            if not row.get("assetId"):
                return False
            if folded(row.get("addressU")) not in {"wrap", "clamp"}:
                return False
            if folded(row.get("addressV")) not in {"wrap", "clamp"}:
                return False
            if folded(row.get("colorSpace")) not in {"linear", "srgb"}:
                return False
            if folded(row.get("samplingEvidence")) in {
                "", "legacy_default", "missing_sampling_evidence",
            }:
                return False
        return True
    if runtime_shader_profile_id == "effect.ue3.procedural-center-glow.v1":
        return True
    return True


def element_subuv_mode(element: dict[str, Any]) -> str:
    modules = (element.get("sourceRecipe") or {}).get("modules", [])
    if not any(folded(row.get("className")) == "particlemodulesubuv"
               for row in modules):
        return "none"
    literals = {
        folded(literal.get("propertyPath")): literal.get("value")
        for module in modules
        if folded(module.get("className")) == "particlemodulerequired"
        for literal in module.get("literals", [])
    }
    allow_flip = bool(literals.get("ballowimageflipping"))
    square_flip = bool(literals.get("bsquareimageflipping"))
    random_time = float(literals.get("randomimagetime") or 0.0)
    return (
        "psuvim_linear_blend_random_flip_square"
        if allow_flip and square_flip and random_time > 0.0
        else "psuvim_linear_blend"
    )


def runtime_profile_subuv_mode(
    runtime_shader_profile_id: str, element: dict[str, Any]
) -> str:
    # Aura owns two UV domains at runtime: its fixed glow uses local 0..1 UV,
    # while the source cloud texture consumes the authored 8x4 SubUV frame.
    return element_subuv_mode(element)


def classify_dynamic_parameter(name: str) -> str:
    value = folded(name)
    exact = {
        "mask_a_offset": "mask_a_offset",
        "mask_b_offset": "mask_b_offset",
        "mask_a_distort": "mask_a_distort",
        "mask_b_distort": "mask_b_distort",
        "maksa_pan": "mask_a_pan",
        "maska_pan": "mask_a_pan",
        "flow_str": "flow_strength",
        "maksb_pan": "mask_b_pan",
        "maskb_pan": "mask_b_pan",
        "diff_pan": "diffuse_pan",
    }
    if value in exact:
        return exact[value]
    if "dissolve" in value:
        return "dissolve"
    if "pan" in value or "uv" in value:
        return "uv_pan"
    if "rot" in value:
        # The bounded runtime has no UV-rotation semantic.  Treating a
        # negative rotation/time curve as opacity makes a valid emitter fully
        # invisible, so unsupported rotation remains explicitly unbound.
        return "unbound"
    if any(token in value for token in ("opacity", "fade")) or value in {
        "alpha", "alpha_str", "alpha_strength"
    }:
        return "opacity"
    if any(token in value for token in ("emissive", "bright", "power")):
        return "emissive"
    if "noise" in value or "distort" in value:
        return "distortion"
    if "size" in value or "radius" in value:
        return "radial_size"
    return "unbound"


def dynamic_parameter_semantics(element: dict[str, Any]) -> list[str]:
    semantics = ["unbound", "unbound", "unbound", "unbound"]
    modules = (element.get("sourceRecipe") or {}).get("modules", [])
    for module in modules:
        if folded(module.get("className")) != "particlemoduleparameterdynamic":
            continue
        for literal in module.get("literals", []):
            match = re.fullmatch(
                r"dynamicparams\[(\d+)\]\.paramname",
                folded(literal.get("propertyPath")),
            )
            if not match:
                continue
            index = int(match.group(1))
            if 0 <= index < len(semantics):
                semantics[index] = classify_dynamic_parameter(
                    str(literal.get("value") or "")
                )
    return semantics


def runtime_profile_dynamic_parameter_semantics(
    runtime_shader_profile_id: str, element: dict[str, Any]
) -> list[str]:
    if runtime_shader_profile_id != "effect.ue3.missiletrail-01.v1":
        return dynamic_parameter_semantics(element)
    exact = {
        "alpha_pan": "missile_alpha_pan",
        "uv_noise_velue": "missile_noise_strength",
        "uv_noise_value": "missile_noise_strength",
        "uv_noise_pan": "missile_noise_pan",
        "alpha_dissolve": "missile_dissolve",
    }
    semantics = ["unbound", "unbound", "unbound", "unbound"]
    modules = (element.get("sourceRecipe") or {}).get("modules", [])
    for module in modules:
        if folded(module.get("className")) != "particlemoduleparameterdynamic":
            continue
        for literal in module.get("literals", []):
            match = re.fullmatch(
                r"dynamicparams\[(\d+)\]\.paramname",
                folded(literal.get("propertyPath")),
            )
            if not match:
                continue
            index = int(match.group(1))
            if 0 <= index < len(semantics):
                semantics[index] = exact.get(
                    folded(literal.get("value")), "unbound"
                )
    return semantics


def upgrade_effect_document(
    effect_document: dict[str, Any], contract: dict[str, Any]
) -> dict[str, Any]:
    identities = {
        folded(row.get("sourceMaterialPath")): row
        for row in contract.get("materialIdentities", [])
    }
    upgraded = copy.deepcopy(effect_document)
    upgraded["version"] = 12
    for element in upgraded.get("elements", []):
        material = element.setdefault("material", {})
        if folded(element.get("kind")) != "particle":
            material["sourceProfile"] = {"enabled": False}
            continue
        source_path = folded(material.get("sourceMaterialPath"))
        identity = identities.get(source_path)
        if identity is None:
            raise ValueError(
                f"Particle {element.get('id')} has no source material contract."
            )
        parameters = identity.get("sourceParameters") or {}
        material["sourceProfile"] = {
            "enabled": True,
            "profileId": identity["profileId"],
            "runtimeShaderProfileId": identity["runtimeShaderProfileId"],
            "parentMaterialPath": identity["parentMaterialPath"],
            "semanticStatus": "reconstructed_profile",
            "materialEvidenceSource": identity.get(
                "materialEvidenceSource", "MATERIAL_MAP_EXACT"
            ),
            "productAdmissionStatus": identity.get(
                "productAdmissionStatus", "BLOCKED_SOURCE_EVIDENCE"
            ),
            "sourceResourceBindings": copy.deepcopy(
                identity.get("currentResourceBindings") or []
            ),
            "textures": [
                normalize_texture_parameter(row)
                for row in parameters.get("textures") or []
            ],
            "scalars": [
                normalize_scalar_parameter(row)
                for row in parameters.get("scalars") or []
            ],
            "vectors": [
                normalize_vector_parameter(row)
                for row in parameters.get("vectors") or []
            ],
            "staticSwitches": [
                normalize_switch_parameter(row)
                for row in parameters.get("staticSwitches") or []
            ],
            "dynamicParameterSemantics": (
                runtime_profile_dynamic_parameter_semantics(
                    identity["runtimeShaderProfileId"], element
                )
            ),
            "subUVMode": runtime_profile_subuv_mode(
                identity["runtimeShaderProfileId"], element
            ),
        }
        render_state = identity.get("renderState") or {}
        if render_state.get("twoSided") is False:
            blend_mode = folded(render_state.get("blendMode"))
            if blend_mode == "blend_translucent":
                material["renderProfile"] = "alpha_one_sided_depth_read"
            elif blend_mode == "blend_additive":
                material["renderProfile"] = "additive_one_sided_depth_read"
        required = identity.get("requiredRuntimeBindings") or []
        role_resolved = identity.get("roleResolvedRuntimeBindings") or []
        blocked_assets = set(identity.get("blockedRuntimeAssetIds") or [])
        blocked_bindings = {
            (str(row.get("slotId") or ""), str(row.get("assetId") or ""))
            for row in identity.get("blockedRuntimeBindings") or []
        }
        replacements = required or role_resolved
        required_slots = {
            str(row.get("slotId")) for row in replacements
        }
        # Parent evidence replaces only the roles it actually identifies.
        # Earlier conversion already carries concrete runtime assets for the
        # remaining roles; dropping all five slots whenever one parent group
        # resolved (for example, Noise only) erased valid Base/Mask carriers
        # and made the grouped executor fail closed. Unsafe normal/bump assets
        # are still removed by the explicit blocked binding policy below.
        element["resources"] = [
            row for row in element.get("resources", [])
            if str(row.get("slotId")) not in required_slots
            and str(row.get("assetId")) not in blocked_assets
            and (
                str(row.get("slotId") or ""),
                str(row.get("assetId") or ""),
            ) not in blocked_bindings
        ] + [
            {
                "slotId": str(row.get("slotId")),
                "assetId": str(row.get("assetId")),
            }
            for row in copy.deepcopy(replacements)
        ]
        source_profile = material["sourceProfile"]
        if (
            source_profile["runtimeShaderProfileId"]
            == "effect.ue3.grouped-translucent.v1"
            and not grouped_runtime_resource_contract_satisfied(
                source_profile, element["resources"]
            )
        ):
            source_profile["runtimeShaderProfileId"] = (
                "effect.ue3.fallback-blocked.v1"
            )
        elif not finite_profile_runtime_resource_contract_satisfied(
            source_profile["runtimeShaderProfileId"], element["resources"],
            source_profile
        ):
            source_profile["runtimeShaderProfileId"] = (
                "effect.ue3.fallback-blocked.v1"
            )
    return upgraded


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staged = path.with_suffix(path.suffix + ".tmp")
    staged.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    staged.replace(path)


def build_contract(
    effect_document: dict[str, Any],
    source_graph: dict[str, Any],
    conversion_receipt: dict[str, Any],
    resource_manifest: dict[str, Any],
    material_map: dict[str, Any] | None = None,
    runtime_resource_root: Path | None = None,
    material_graph_evidence: dict[str, Any] | None = None,
    texture_sampling_evidence: dict[str, Any] | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    particle_elements = [
        row for row in effect_document.get("elements", [])
        if folded(row.get("kind")) == "particle"
    ]
    if not particle_elements:
        raise ValueError("Effect document has no Particle elements.")

    occurrence_ids_by_material: dict[str, list[str]] = defaultdict(list)
    template_counts_by_material: dict[str, Counter[str]] = defaultdict(Counter)
    resources_by_material: dict[str, dict[str, str]] = defaultdict(dict)
    source_path_spelling: dict[str, str] = {}
    for element in particle_elements:
        material = element.get("material") or {}
        source_path = str(material.get("sourceMaterialPath") or "")
        key = folded(source_path)
        if not key:
            raise ValueError(
                f"Particle {element.get('id')} has no sourceMaterialPath."
            )
        source_path_spelling.setdefault(key, source_path)
        occurrence_ids_by_material[key].append(str(element.get("id") or ""))
        template_counts_by_material[key][
            str(material.get("templateId") or "")
        ] += 1
        # Hydration removes unsafe bindings from the live runtime resource
        # list. Preserve the source-side list in sourceProfile so rerunning
        # the diagnostic builder does not forget what it previously blocked.
        source_profile = material.get("sourceProfile") or {}
        for binding in source_profile.get("sourceResourceBindings", []) or []:
            slot = str(binding.get("slotId") or "")
            asset = str(binding.get("assetId") or "")
            if slot and asset:
                resources_by_material[key].setdefault(slot, asset)
        for binding in element.get("resources", []):
            slot = str(binding.get("slotId") or "")
            asset = str(binding.get("assetId") or "")
            if slot and asset:
                resources_by_material[key].setdefault(slot, asset)

    material_map = material_map or {"materials": {}}
    manifest_packages = source_asset_packages(resource_manifest)
    sampling_by_path = {
        folded(row.get("sourceObjectPath")): row
        for row in (texture_sampling_evidence or {}).get("textures", [])
        if row.get("sourceObjectPath")
    }
    sampling_by_object: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in sampling_by_path.values():
        sampling_by_object[
            folded(row.get("sourceObjectPath")).rsplit(".", 1)[-1]
        ].append(row)

    identities: list[dict[str, Any]] = []
    profile_members: dict[str, list[str]] = defaultdict(list)
    failures: list[dict[str, Any]] = []
    for key in sorted(occurrence_ids_by_material):
        source_path = source_path_spelling[key]
        graph_candidates = normalized_graph_material_candidates(
            source_graph, source_path
        )
        graph_row = graph_candidates[0] if len(graph_candidates) == 1 else {}
        candidates = material_candidates(material_map, source_path)
        expected_package = folded(graph_row.get("sourcePhysicalPackage"))
        exact_package_candidates = [
            row for row in candidates
            if expected_package
            and folded(row.get("source_file")) == expected_package
        ]
        if len(candidates) == 1:
            selected = candidates[0]
            material_evidence_source = "MATERIAL_MAP_EXACT"
        elif len(exact_package_candidates) == 1:
            # Material object paths are not globally unique across cooked UE3
            # packages.  The source receipt already records the package that
            # owned this occurrence, so use that evidence to disambiguate the
            # material map instead of selecting by path order.
            selected = exact_package_candidates[0]
            material_evidence_source = "MATERIAL_MAP_EXACT_SOURCE_PACKAGE"
        elif not candidates and len(graph_candidates) == 1:
            selected = graph_row
            material_evidence_source = "NORMALIZED_GRAPH_EXACT"
        else:
            selected = {}
            material_evidence_source = "UNRESOLVED"
        identity_failure_start = len(failures)
        if not candidates and len(graph_candidates) == 0:
            failures.append({
                "sourceMaterialPath": source_path,
                "status": "MISSING_NORMALIZED_GRAPH_MATERIAL_BINDING",
            })
        elif not candidates and len(graph_candidates) > 1:
            failures.append({
                "sourceMaterialPath": source_path,
                "status": "AMBIGUOUS_NORMALIZED_GRAPH_MATERIAL_BINDING",
                "candidateCount": len(graph_candidates),
            })
        evidence_ref = str(selected.get("materialEvidenceRef") or "")
        if evidence_ref:
            parent_evidence = (
                material_map.get("parentMaterialEvidence") or {}
            ).get(evidence_ref)
            if not isinstance(parent_evidence, dict):
                raise ValueError(
                    f"missing parent material evidence: {evidence_ref}"
                )
            selected = copy.deepcopy(selected)
            selected["materialEvidence"] = copy.deepcopy(
                parent_evidence.get("materialEvidence", parent_evidence)
            )
        parent = str(selected.get("parent") or graph_row.get("parent") or source_path)
        selected = merge_parent_graph_texture_defaults(
            selected, parent, material_graph_evidence
        )
        parent_package = manifest_packages.get(folded(parent))
        source_package = str(
            selected.get("source_file")
            or graph_row.get("sourcePhysicalPackage")
            or manifest_packages.get(key)
            or ""
        )
        if len(candidates) > 1 and len(exact_package_candidates) != 1:
            failures.append({
                "sourceMaterialPath": source_path,
                "status": "AMBIGUOUS_MATERIAL_CANDIDATE",
                "candidateCount": len(candidates),
            })
        if not source_package and key.startswith("enginematerials."):
            source_package = "ENGINE_BUILTIN"
            parent_package = parent_package or "ENGINE_BUILTIN"
        if not source_package:
            failures.append({
                "sourceMaterialPath": source_path,
                "status": "MISSING_SOURCE_PACKAGE",
            })
        profile_id = stable_profile_id(parent)
        fallback_blocked_reason = str(
            selected.get("fallbackBlockedReason") or ""
        )
        template_counts = template_counts_by_material[key]
        pending_count = template_counts.get("effect.source_material", 0)
        role_bindings, role_diagnostics = role_resolved_runtime_bindings(
            selected, source_graph, resource_manifest
        )
        allowed_normal_noise_assets = evidence_backed_normal_noise_assets(
            role_diagnostics
        )
        blocked_assets = blocked_runtime_asset_ids(
            selected, source_graph, allowed_normal_noise_assets,
            role_diagnostics,
        )
        current_bindings = [
            {"slotId": slot, "assetId": asset}
            for slot, asset in sorted(resources_by_material[key].items())
        ]
        blocked_bindings = blocked_normal_bump_runtime_bindings(
            current_bindings, source_graph, role_diagnostics
        )
        blocked_binding_keys = {
            (row["slotId"], row["assetId"])
            for row in blocked_bindings
        }
        current_safe_bindings = [
            row for row in current_bindings
            if (row["slotId"], row["assetId"]) not in blocked_binding_keys
            and row["assetId"] not in blocked_assets
        ]
        role_safe_bindings = [
            row for row in role_bindings
            if row["assetId"] not in blocked_assets
            and (row["slotId"], row["assetId"]) not in blocked_binding_keys
        ]
        exact_shader_profile_id = runtime_shader_profile_id(parent, source_path)
        exact_required = required_runtime_bindings(exact_shader_profile_id)
        shader_profile_id, profile_blocked_reason = grouped_translucent_selection(
            exact_shader_profile_id,
            fallback_blocked_reason,
            role_safe_bindings,
            role_diagnostics,
            runtime_binding_files_available(
                exact_required, runtime_resource_root
            ),
            current_safe_bindings,
        )
        fallback_blocked_reason = profile_blocked_reason or ""
        source_parameters = merged_source_parameters(selected, graph_row)
        for texture_parameter in source_parameters.get("textures", []):
            source_object_path = str(
                texture_parameter.get("texture") or ""
            )
            asset_id, resolution_status = manifest_texture_runtime_asset(
                resource_manifest, source_object_path
            )
            texture_parameter["sourceObjectPath"] = source_object_path
            texture_parameter["assetId"] = asset_id or ""
            texture_parameter["resolutionStatus"] = resolution_status
            sampling = sampling_by_path.get(folded(source_object_path))
            if sampling is None:
                suffix_rows = sampling_by_object.get(
                    folded(source_object_path).rsplit(".", 1)[-1], []
                )
                sampling = suffix_rows[0] if len(suffix_rows) == 1 else None
            if sampling is not None:
                texture_parameter["addressU"] = str(
                    sampling.get("addressU") or "wrap"
                )
                texture_parameter["addressV"] = str(
                    sampling.get("addressV") or "wrap"
                )
                texture_parameter["colorSpace"] = str(
                    sampling.get("colorSpace") or "linear"
                )
                texture_parameter["samplingEvidence"] = str(
                    sampling.get("samplingEvidence")
                    or "ue3_property_or_class_default.v1"
                )
            else:
                texture_parameter["addressU"] = "wrap"
                texture_parameter["addressV"] = "wrap"
                texture_parameter["colorSpace"] = "linear"
                texture_parameter["samplingEvidence"] = "legacy_default"
        if (
            shader_profile_id == "effect.ue3.local-crack.v1"
            and not finite_profile_runtime_resource_contract_satisfied(
                shader_profile_id, current_bindings, source_parameters
            )
        ):
            shader_profile_id = "effect.ue3.fallback-blocked.v1"
            fallback_blocked_reason = (
                "LOCAL_CRACK_NAMED_TEXTURE_OR_SAMPLING_CONTRACT_INCOMPLETE"
            )
        profile_members[profile_id].append(source_path)
        source_evidence_resolved = len(failures) == identity_failure_start
        if not source_evidence_resolved:
            product_admission_status = "BLOCKED_SOURCE_EVIDENCE"
        elif shader_profile_id == "effect.ue3.fallback-blocked.v1":
            product_admission_status = "BLOCKED_FALLBACK_PROFILE"
        else:
            product_admission_status = "ADMITTED_RECONSTRUCTED_PROFILE"
        identities.append({
            "sourceMaterialPath": source_path,
            "sourcePhysicalPackage": source_package or None,
            "parentMaterialPath": parent,
            "parentSourcePhysicalPackage": parent_package,
            "profileId": profile_id,
            "runtimeShaderProfileId": shader_profile_id,
            "fallbackBlockedReason": fallback_blocked_reason or None,
            "materialEvidenceSource": material_evidence_source,
            "sourceEvidenceResolved": source_evidence_resolved,
            "productAdmissionStatus": product_admission_status,
            "requiredRuntimeBindings": required_runtime_bindings(shader_profile_id),
            "roleResolvedRuntimeBindings": role_bindings,
            "sourceTextureRoleDiagnostics": role_diagnostics,
            "blockedRuntimeAssetIds": blocked_assets,
            "blockedRuntimeBindings": blocked_bindings,
            "semanticStatus": "RECONSTRUCTED_PROFILE",
            "runtimeOccurrenceCount": len(occurrence_ids_by_material[key]),
            "runtimeOccurrenceElementIds": occurrence_ids_by_material[key],
            "pendingRuntimeOccurrenceCount": pending_count,
            "templateCounts": dict(sorted(template_counts.items())),
            "currentResourceBindings": current_bindings,
            "sourceParameters": source_parameters,
            "renderState": (selected.get("materialEvidence") or {}).get(
                "renderState"
            ),
            "referencedTextures": (selected.get("materialEvidence") or {}).get(
                "referencedTextures", []
            ),
            "collectedTextureParameters": (
                selected.get("materialEvidence") or {}
            ).get("collectedTextureParameters", []),
            "expressionCoverage": (selected.get("materialEvidence") or {}).get(
                "expressionCoverage",
                {
                    "entryCount": None,
                    "nonNullCount": None,
                    "nullCount": None,
                    "topologyStatus": "NOT_CAPTURED",
                },
            ),
            "materialResourceDecodeStatus": "NOT_CAPTURED",
        })

    profiles = [
        {
            "profileId": profile_id,
            "parentMaterialPath": next(
                row["parentMaterialPath"]
                for row in identities if row["profileId"] == profile_id
            ),
            "semanticStatus": "RECONSTRUCTED_PROFILE",
            "runtimeShaderProfileId": next(
                row["runtimeShaderProfileId"]
                for row in identities if row["profileId"] == profile_id
            ),
            "materialIdentityCount": len(members),
            "materialIdentities": sorted(members, key=str.casefold),
            "runtimeOccurrenceCount": sum(
                row["runtimeOccurrenceCount"]
                for row in identities if row["profileId"] == profile_id
            ),
        }
        for profile_id, members in sorted(profile_members.items())
    ]

    particle_conversions = [
        row for row in conversion_receipt.get("elementConversions", [])
        if folded(row.get("targetKind")) == "particle"
    ]
    heuristic_count = sum(
        count_status(row.get("resourceMappings", []), "PARAMETER_NAME_HEURISTIC")
        for row in particle_conversions
    )
    pending_occurrence_count = sum(
        row["pendingRuntimeOccurrenceCount"] for row in identities
    )
    summary = {
        "particleElementCount": len(particle_elements),
        "materialIdentityCount": len(identities),
        "parentProfileGroupCount": len(profiles),
        "pendingRuntimeOccurrenceCount": pending_occurrence_count,
        "standardRuntimeOccurrenceCount": (
            len(particle_elements) - pending_occurrence_count
        ),
        "parameterNameHeuristicCount": heuristic_count,
        "failureCount": len(failures),
        "sourceExactIdentityCount": len(identities),
        "runtimeExactProfileCount": 0,
        "reconstructedProfileCount": len(profiles),
        "roleResolvedIdentityCount": sum(
            bool(row["roleResolvedRuntimeBindings"]) for row in identities
        ),
        "roleResolvedRuntimeBindingCount": sum(
            len(row["roleResolvedRuntimeBindings"]) for row in identities
        ),
        "blockedNormalBumpRuntimeBindingCount": sum(
            len(row["blockedRuntimeBindings"]) for row in identities
        ),
        "fallbackBlockedMaterialIdentityCount": sum(
            row["runtimeShaderProfileId"]
            == "effect.ue3.fallback-blocked.v1"
            for row in identities
        ),
        "productAdmissibleMaterialIdentityCount": sum(
            row["productAdmissionStatus"]
            == "ADMITTED_RECONSTRUCTED_PROFILE"
            for row in identities
        ),
        "productMaterialAdmissionComplete": bool(identities) and all(
            row["productAdmissionStatus"]
            == "ADMITTED_RECONSTRUCTED_PROFILE"
            for row in identities
        ) and not failures,
    }
    contract = {
        "schema": "lostark.effect-source-material-contract",
        "formatVersion": 1,
        "effectAssetId": effect_document.get("effectAssetId"),
        "summary": summary,
        "materialIdentities": identities,
        "profiles": profiles,
        "failures": failures,
    }
    receipt = {
        "schema": "lostark.effect-source-material-contract-receipt",
        "formatVersion": 1,
        "effectAssetId": effect_document.get("effectAssetId"),
        "summary": summary,
        "failures": failures,
    }
    return contract, receipt


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--effect-document", required=True, type=Path)
    parser.add_argument("--source-graph", required=True, type=Path)
    parser.add_argument("--conversion-receipt", required=True, type=Path)
    parser.add_argument("--resource-manifest", required=True, type=Path)
    parser.add_argument(
        "--material-map",
        type=Path,
        help=(
            "Optional exported parent-props map. When omitted, an exact "
            "unique materialParameterBindings row from the normalized source "
            "graph is used; missing or duplicate rows fail closed."
        ),
    )
    parser.add_argument("--material-graph-evidence", type=Path)
    parser.add_argument("--texture-sampling-evidence", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--output-effect-document", type=Path)
    args = parser.parse_args()

    contract, receipt = build_contract(
        load_json(args.effect_document),
        load_json(args.source_graph),
        load_json(args.conversion_receipt),
        load_json(args.resource_manifest),
        load_json(args.material_map) if args.material_map is not None else {},
        material_graph_evidence=(
            load_json(args.material_graph_evidence)
            if args.material_graph_evidence is not None else None
        ),
        texture_sampling_evidence=(
            load_json(args.texture_sampling_evidence)
            if args.texture_sampling_evidence is not None else None
        ),
    )
    required_sources = [
        args.effect_document,
        args.source_graph,
        args.conversion_receipt,
        args.resource_manifest,
    ]
    if args.material_map is not None:
        required_sources.append(args.material_map)
    receipt["sources"] = [
        {"path": path.as_posix(), "sha256": sha256_file(path)}
        for path in (
            *required_sources,
            *(
                (args.material_graph_evidence,)
                if args.material_graph_evidence is not None else ()
            ),
            *(
                (args.texture_sampling_evidence,)
                if args.texture_sampling_evidence is not None else ()
            ),
        )
    ]
    write_json_atomic(args.output, contract)
    write_json_atomic(args.receipt, receipt)
    if args.output_effect_document is not None:
        write_json_atomic(
            args.output_effect_document,
            upgrade_effect_document(
                load_json(args.effect_document), contract
            ),
        )
    print(json.dumps(receipt["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not receipt["failures"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
