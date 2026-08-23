"""Build the fail-closed Valtan 669 horizontal RT0 application ledger.

The ledger is an application decision over the sealed tuple inventory.  It is
not a registry publisher and it never turns source names or unpacked values
into a runtime packet.  A future Valtan registry fragment owns publication.
A row receives a binding identity only after Program, Layout, Descriptor, and
Adapter evidence are all closed.  With the current inputs that intentionally
means that evidence-blocked rows remain unbound.
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
from collections import Counter
from pathlib import Path
from typing import Any, Iterable


SCHEMA = "lostark.valtan-effect-v1-horizontal-rt0-application"
FORMAT_VERSION = 1
REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
OUTPUT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/valtan-effect-v1-horizontal-rt0-application.v1.json"
)
SCHEMA_RELATIVE_PATH = Path(
    "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-effect-v1-horizontal-rt0-application.schema.json"
)

TUPLE_INVENTORY_PATH = Path(
    "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json"
)
SOURCE_MATERIAL_EVIDENCE_PATH = Path(
    "Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json"
)
FAMILY_MANIFEST_PATH = Path(
    "Data/Effects/Contracts/effect-family-manifest.v1.json"
)
MISSING_FAMILY_INVENTORY_PATH = Path(
    "Data/Effects/Contracts/effect-missing-family-inventory.v1.json"
)

EXPECTED_ROW_COUNT = 669
EXPECTED_CARRIER_COUNTS = {
    "SPRITE": 458,
    "MESH": 174,
    "DECAL": 33,
    "RIBBON": 3,
    "PRESENTATION": 1,
}
EXPECTED_FINE_RENDERER_COUNTS = {
    "SPRITE_PARTICLE": 458,
    "MESH_PARTICLE": 173,
    "STANDALONE_MESH": 1,
    "DECAL_PARTICLE": 33,
    "ANIM_TRAIL": 3,
    "LIGHT_PRESENTATION": 1,
}
EXPECTED_PATTERN_OCCURRENCE_COUNTS = {
    "VALTAN_ARENA_BREAK_109": 5,
    "VALTAN_ARMOR_BREAK_OPENING": 10,
    "VALTAN_BACKSTEP_ATTACK": 3,
    "VALTAN_CHARGE_GRAB_ROAR": 22,
    "VALTAN_DOWN_SMASH": 7,
    "VALTAN_EARTHQUAKE_SMASH": 70,
    "VALTAN_FIST_IN_OUT": 20,
    "VALTAN_FLOOR_WIPE_130": 28,
    "VALTAN_FOUR_SLASH": 38,
    "VALTAN_FRONT_BACK_FRONT": 108,
    "VALTAN_GHOST_TRANSITION_15": 16,
    "VALTAN_GROUND_WAVE_SMASH": 9,
    "VALTAN_HIGH_JUMP": 29,
    "VALTAN_IMPRISON_ROAR": 31,
    "VALTAN_JUMP_SPIN": 2,
    "VALTAN_LEDGE_ROAR": 16,
    "VALTAN_MAGIC_CHOICE": 19,
    "VALTAN_PARRY": 16,
    "VALTAN_PORTAL_RUSH": 28,
    "VALTAN_RED_BLADE_WAVE": 13,
    "VALTAN_SUPER_SMASH": 2,
    "VALTAN_SWING": 149,
    "VALTAN_TRIPLE_COUNTER": 16,
    "VALTAN_WHIRLWIND": 12,
}

MAX_TEXTURE_LANES = 6
MAX_PACKED_SCALARS = 52
MAX_PACKED_VECTORS = 3

PROGRAM_FIDELITY_BY_STATUS = {
    "DXBC_OCCURRENCE_EXACT": "SOURCE_EXACT",
    "DXBC_OCCURRENCE_EXACT_UNTRANSLATED": "SOURCE_EXACT_UNTRANSLATED",
    "DXBC_FAMILY_REPRESENTATIVE_ONLY": "FAMILY_REPRESENTATIVE",
    "BOUNDED_SOURCE_PROFILE_ONLY": "BOUNDED_TRANSLATED",
    "NO_PROGRAM_EVIDENCE": "PROJECT_RECONSTRUCTED",
    "NOT_APPLICABLE_PRESENTATION": "NOT_APPLICABLE_PRESENTATION",
}

BASE_CAPABILITY_BY_FINE_RENDERER = {
    "SPRITE_PARTICLE": "SPRITE_PARTICLE_STANDARD_RT0",
    "MESH_PARTICLE": "MESH_PARTICLE_CMODEL_RT0",
    "STANDALONE_MESH": "STANDALONE_MESH_RT0",
    "DECAL_PARTICLE": "LOCAL_DECAL_PROJECTOR_RT0",
    "ANIM_TRAIL": "TRAIL_RIBBON_RT0",
    "LIGHT_PRESENTATION": "LIGHT_PRESENTATION",
}

APPLICATION_STATES = {
    "RT0_APPLICATION_CANDIDATE",
    "FEATURE_DEFERRED",
    "EVIDENCE_BLOCKED",
    "PRESENTATION_DEFERRED",
}

SHA256_RE = re.compile(r"[0-9a-f]{64}")
STABLE_ID_RE = re.compile(r"[A-Za-z0-9_.-]{1,160}")
THIN_MESH_NAME_RE = re.compile(
    r"(?:crackline|ring_|square|card|trail|swing|cracklight|helix)", re.IGNORECASE
)


class ApplicationError(ValueError):
    """A contract violation detected before output replacement."""


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ApplicationError(message)


def _reject_non_finite(token: str) -> None:
    raise ApplicationError("JSON contains a non-finite number: " + token)


def _object_without_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ApplicationError("JSON contains a duplicate key: " + key)
        result[key] = value
    return result


def _decode_json(payload: bytes, label: str) -> Any:
    try:
        text = payload.decode("utf-8")
    except UnicodeError as error:
        raise ApplicationError(label + " is not UTF-8") from error
    try:
        return json.loads(
            text,
            object_pairs_hook=_object_without_duplicate_keys,
            parse_constant=_reject_non_finite,
        )
    except ApplicationError:
        raise
    except (TypeError, ValueError) as error:
        raise ApplicationError(label + " is not valid JSON") from error


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
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            indent=2,
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def _require_sha256(value: Any, label: str) -> str:
    _require(isinstance(value, str) and SHA256_RE.fullmatch(value) is not None, label)
    return value


def _require_stable_id(value: Any, label: str) -> str:
    _require(
        isinstance(value, str) and STABLE_ID_RE.fullmatch(value) is not None,
        label,
    )
    return value


def _content_id(prefix: str, payload: Any) -> str:
    return prefix + canonical_sha256(payload)


def _verify_artifact_hash(document: dict[str, Any], label: str) -> str:
    digest = _require_sha256(document.get("artifactSha256"), label + ".artifactSha256")
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    _require(canonical_sha256(unsigned) == digest, label + " artifactSha256 drifted")
    return digest


def _load_input(
    repository_root: Path,
    relative_path: Path,
    roles: Iterable[str],
    *,
    artifact_schema: str | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    path = repository_root / relative_path
    _require(path.is_file(), relative_path.as_posix() + " is missing")
    payload = path.read_bytes()
    document = _decode_json(payload, relative_path.as_posix())
    _require(isinstance(document, dict), relative_path.as_posix() + " root must be object")
    artifact_sha256: str | None = None
    if artifact_schema is not None:
        _require(document.get("schema") == artifact_schema, relative_path.as_posix() + " schema drifted")
        _require(document.get("formatVersion") == 1, relative_path.as_posix() + " version drifted")
        artifact_sha256 = _verify_artifact_hash(document, relative_path.as_posix())
    identity = {
        "path": relative_path.as_posix(),
        "roles": sorted(set(roles)),
        "rawSha256": hashlib.sha256(payload).hexdigest(),
        "byteSize": len(payload),
        "artifactSha256": artifact_sha256,
    }
    return document, identity


def _unique_index(
    rows: Iterable[dict[str, Any]], key: str, label: str, *, allow_null: bool = False
) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        value = row.get(key)
        if value is None and allow_null:
            continue
        _require(isinstance(value, str) and value, label + " has invalid " + key)
        _require(value not in result, label + " has duplicate " + key + ": " + value)
        result[value] = row
    return result


def _normalize_blend_mode(value: Any) -> str | None:
    if not isinstance(value, str):
        return None
    lowered = value.casefold()
    if "translucent" in lowered:
        return "TRANSLUCENT"
    if "additive" in lowered:
        return "ADDITIVE"
    if "masked" in lowered:
        return "MASKED"
    if "modulate" in lowered:
        return "MODULATE"
    return None


def _family_render_state_index(
    family_manifest: dict[str, Any], missing_inventory: dict[str, Any]
) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}

    def add(
        path: Any,
        props: Any,
        provenance: str,
        evidence_id: Any,
    ) -> None:
        if not isinstance(path, str) or not isinstance(props, dict):
            return
        blend = _normalize_blend_mode(props.get("blendMode"))
        two_sided = props.get("twoSided")
        depth_off = props.get("disableDepthTest")
        if blend is None or not isinstance(two_sided, bool) or not isinstance(depth_off, bool):
            return
        row = {
            "blendMode": blend,
            "twoSided": two_sided,
            "disableDepthTest": depth_off,
            "usesDistortion": None,
            "provenance": provenance,
            "evidenceId": evidence_id,
        }
        key = path.casefold()
        previous = result.get(key)
        if previous is not None:
            comparable = ("blendMode", "twoSided", "disableDepthTest")
            _require(
                all(previous[field] == row[field] for field in comparable),
                "family render-state sources disagree for " + path,
            )
            return
        result[key] = row

    for family in family_manifest.get("families", []):
        add(
            family.get("parentMaterialPath"),
            family.get("evidence"),
            "PARENT_FAMILY_MANIFEST_FALLBACK",
            family.get("rowSha256"),
        )
    for family in missing_inventory.get("families", []):
        add(
            family.get("parentMaterialPath"),
            family.get("parentProps"),
            "PARENT_MISSING_FAMILY_PROPS_FALLBACK",
            family.get("rowSha256"),
        )
    return result


def _source_material_evidence_id(row: dict[str, Any]) -> str:
    return _content_id("source-material-evidence.", row)


def _resolve_source_render_state(
    occurrence: dict[str, Any],
    source_material: dict[str, Any] | None,
    family_render_states: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    source_path = occurrence.get("sourceMaterialPath")
    source_parent_blocker = occurrence.get("sourceParentBlocker")
    state: dict[str, Any] | None = None
    evidence_id: str | None = None
    provenance = "UNRESOLVED"

    if source_material is not None:
        declaration = source_material.get("parentDeclaration")
        render_state = declaration.get("renderState") if isinstance(declaration, dict) else None
        if isinstance(render_state, dict):
            blend = _normalize_blend_mode(render_state.get("blendMode"))
            two_sided = render_state.get("twoSided")
            depth_off = render_state.get("disableDepthTest")
            if blend is not None and isinstance(two_sided, bool) and isinstance(depth_off, bool):
                distortion = render_state.get("usesDistortion")
                _require(
                    distortion is None or isinstance(distortion, bool),
                    "source usesDistortion must be boolean or null",
                )
                state = {
                    "blendMode": blend,
                    "twoSided": two_sided,
                    "disableDepthTest": depth_off,
                    "usesDistortion": distortion,
                }
                provenance = "SEALED_CHILD_PARENT_DECLARATION"
                evidence_id = _source_material_evidence_id(source_material)

    if state is None and source_parent_blocker is None:
        fallback_paths = [source_path, occurrence.get("sourceParentMaterialPath")]
        for fallback_path in fallback_paths:
            if not isinstance(fallback_path, str):
                continue
            fallback = family_render_states.get(fallback_path.casefold())
            if fallback is None:
                continue
            state = {
                "blendMode": fallback["blendMode"],
                "twoSided": fallback["twoSided"],
                "disableDepthTest": fallback["disableDepthTest"],
                "usesDistortion": fallback["usesDistortion"],
            }
            provenance = fallback["provenance"]
            evidence_id = fallback["evidenceId"]
            break

    resolved = state is not None
    admission_usable = resolved and source_parent_blocker is None
    if resolved:
        distortion_suffix = (
            "distortion"
            if state["usesDistortion"] is True
            else "no_distortion"
            if state["usesDistortion"] is False
            else "distortion_unknown"
        )
        render_state_id = "_".join(
            (
                state["blendMode"].casefold(),
                "two_sided" if state["twoSided"] else "one_sided",
                "depth_off" if state["disableDepthTest"] else "depth_read",
                distortion_suffix,
            )
        )
    else:
        state = {
            "blendMode": None,
            "twoSided": None,
            "disableDepthTest": None,
            "usesDistortion": None,
        }
        render_state_id = "unknown"

    return {
        "status": (
            "RESOLVED_ADMISSION_USABLE"
            if admission_usable
            else "RESOLVED_DIAGNOSTIC_ONLY"
            if resolved
            else "UNRESOLVED"
        ),
        "provenance": provenance,
        "renderStateId": render_state_id,
        **state,
        "admissionUsable": admission_usable,
        "evidenceId": evidence_id,
    }


def _current_render_state(render_profile: Any) -> dict[str, Any] | None:
    if not isinstance(render_profile, str):
        return None
    pieces = render_profile.split("_")
    if not pieces:
        return None
    blend = {
        "alpha": "TRANSLUCENT",
        "additive": "ADDITIVE",
        "masked": "MASKED",
        "modulate": "MODULATE",
    }.get(pieces[0])
    if blend is None:
        return None
    if "two" in pieces and "sided" in pieces:
        two_sided = True
    elif "one" in pieces and "sided" in pieces:
        two_sided = False
    else:
        return None
    if "depth" not in pieces:
        return None
    disable_depth = "off" in pieces
    return {
        "blendMode": blend,
        "twoSided": two_sided,
        "disableDepthTest": disable_depth,
    }


def _source_matches_current(
    source: dict[str, Any], current_profile: Any
) -> bool | None:
    current = _current_render_state(current_profile)
    if source.get("status") not in (
        "RESOLVED_ADMISSION_USABLE",
        "RESOLVED_DIAGNOSTIC_ONLY",
    ) or current is None:
        return None
    return all(
        source[field] == current[field]
        for field in ("blendMode", "twoSided", "disableDepthTest")
    )


def _layout_projection(
    occurrence: dict[str, Any],
    layout_evidence_by_id: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    axis = occurrence["layout"]
    evidence_id = axis.get("layoutEvidenceId")
    evidence = layout_evidence_by_id.get(evidence_id)
    required_inputs: list[str] = []
    counts: dict[str, int] | None = None
    if evidence is not None and evidence.get("kind") == "NAMED_NATIVE_WIRE_EVIDENCE":
        raw_counts = evidence.get("counts")
        _require(isinstance(raw_counts, dict), "named layout evidence lacks counts")
        counts = {
            "textureLaneCount": int(raw_counts.get("textureSlotCount", 0)),
            "scalarLaneCount": int(raw_counts.get("scalarLaneCount", 0)),
            "vectorLaneCount": int(raw_counts.get("vectorLaneCount", 0)),
        }
        for lane in evidence.get("textureSlots", []):
            required_inputs.append(
                "texture:"
                + str(lane.get("textureRegister"))
                + ":"
                + str(lane.get("samplerRegister"))
                + ":"
                + str(lane.get("parameterName") or ("ref" + str(lane.get("referencedTextureIndex"))))
            )
        for lane in evidence.get("scalarLanes", []):
            names = lane.get("parameterNames") or []
            required_inputs.append(
                "scalar:"
                + str(lane.get("constantRegister"))
                + ":"
                + (
                    ",".join(str(value) for value in names)
                    if names
                    else str(lane.get("expression"))
                )
            )
        for lane in evidence.get("vectorLanes", []):
            names = lane.get("parameterNames") or []
            required_inputs.append(
                "vector:"
                + str(lane.get("constantRegister"))
                + ":"
                + ",".join(str(value) for value in names)
            )
    elif evidence is not None and evidence.get("kind") == "SOURCE_PARAMETER_NAMES_ONLY":
        for field, prefix in (
            ("textureNames", "texture-name:"),
            ("scalarNames", "scalar-name:"),
            ("vectorNames", "vector-name:"),
        ):
            required_inputs.extend(prefix + str(value) for value in evidence.get(field, []))

    required_inputs = sorted(set(required_inputs))
    within_caps: bool | None = None
    if counts is not None:
        within_caps = (
            counts["textureLaneCount"] <= MAX_TEXTURE_LANES
            and counts["scalarLaneCount"] <= MAX_PACKED_SCALARS
            and counts["vectorLaneCount"] <= MAX_PACKED_VECTORS
        )
    signature_payload = {
        "layoutStatus": axis.get("status"),
        "layoutEvidenceId": evidence_id,
        "requiredInputIds": required_inputs,
        "counts": counts,
    }
    return {
        "inventoryStatus": axis.get("status"),
        "layoutCandidateId": axis.get("layoutCandidateId"),
        "layoutEvidenceId": evidence_id,
        "layoutSignature": _content_id("layout-signature.", signature_payload),
        "requiredInputIds": required_inputs,
        "requiredInputCounts": counts,
        "withinInitialPacketCaps": within_caps,
    }


def _descriptor_projection(
    occurrence: dict[str, Any],
    descriptor_by_id: dict[str, dict[str, Any]],
    source_material: dict[str, Any] | None,
) -> dict[str, Any]:
    axis = occurrence["descriptor"]
    descriptor_id = axis.get("descriptorVariantId")
    descriptor = descriptor_by_id.get(descriptor_id)
    resources = descriptor.get("resourceBindings", []) if descriptor else []
    has_base = any(row.get("slotId") == "base" for row in resources)

    value_evidence_ids: list[str] = []
    counts = {
        "childTextureCount": 0,
        "childScalarCount": 0,
        "childVectorCount": 0,
        "parentTextureCount": 0,
        "parentScalarCount": 0,
        "parentVectorCount": 0,
    }
    evidence_status = "NO_SOURCE_MATERIAL_EVIDENCE"
    source_material_evidence_id: str | None = None
    if source_material is not None:
        source_material_evidence_id = _source_material_evidence_id(source_material)
        counts["childTextureCount"] = len(source_material.get("instanceTextures", []))
        counts["childScalarCount"] = len(source_material.get("instanceScalars", []))
        counts["childVectorCount"] = len(source_material.get("instanceVectors", []))
        declaration = source_material.get("parentDeclaration")
        if isinstance(declaration, dict):
            counts["parentTextureCount"] = len(declaration.get("collectedTextureParameters", []))
            counts["parentScalarCount"] = len(declaration.get("collectedScalarParameters", []))
            counts["parentVectorCount"] = len(declaration.get("collectedVectorParameters", []))
        child_values = any(counts[key] for key in counts if key.startswith("child"))
        parent_values = any(counts[key] for key in counts if key.startswith("parent"))
        if child_values or parent_values:
            value_evidence_ids.append(source_material_evidence_id)
        if child_values and parent_values:
            evidence_status = "SOURCE_CHILD_AND_PARENT_VALUES_AVAILABLE_UNPACKED"
        elif child_values:
            evidence_status = "SOURCE_CHILD_VALUES_AVAILABLE_PARENT_DEFAULTS_MISSING"
        elif parent_values:
            evidence_status = "SOURCE_PARENT_DEFAULTS_AVAILABLE_UNPACKED"
        else:
            evidence_status = "DIRECT_MATERIAL_NO_VALUE_PACKET"

    if occurrence.get("fineRendererKind") == "LIGHT_PRESENTATION":
        evidence_status = "NOT_APPLICABLE_PRESENTATION"
        packet_closure = "NOT_APPLICABLE_PRESENTATION"
    else:
        packet_closure = "NOT_MATERIALIZED"

    return {
        "inventoryStatus": axis.get("status"),
        "descriptorVariantId": descriptor_id,
        "hasBaseResource": has_base,
        "sourceMaterialEvidenceId": source_material_evidence_id,
        "evidenceStatus": evidence_status,
        "valueEvidenceIds": value_evidence_ids,
        "sourceParameterCounts": counts,
        "packetClosure": packet_closure,
    }


def _mesh_topology(
    occurrence: dict[str, Any], carrier_by_id: dict[str, dict[str, Any]]
) -> tuple[str, str]:
    fine = occurrence.get("fineRendererKind")
    if fine == "SPRITE_PARTICLE":
        return "BILLBOARD_QUAD", "CARRIER_KIND"
    if fine == "DECAL_PARTICLE":
        return "PROJECTED_VOLUME", "CARRIER_KIND"
    if fine == "ANIM_TRAIL":
        return "RIBBON_STRIP", "CARRIER_KIND"
    if fine == "LIGHT_PRESENTATION":
        return "PRESENTATION_ONLY", "CARRIER_KIND"
    if fine == "STANDALONE_MESH":
        return "STANDALONE_MESH_UNKNOWN", "CARRIER_KIND_ONLY"
    if fine == "MESH_PARTICLE":
        carrier = carrier_by_id.get(occurrence.get("carrierVariantId"))
        assets = carrier.get("meshModelAssets", []) if carrier else []
        if any(THIN_MESH_NAME_RE.search(str(asset)) for asset in assets):
            return "THIN_SURFACE_PROXY", "ASSET_NAME_PROXY_ONLY"
        return "VOLUME_PROXY", "ASSET_NAME_PROXY_ONLY"
    return "UNKNOWN", "UNRESOLVED"


def _application_decision(
    occurrence: dict[str, Any],
    layout: dict[str, Any],
    descriptor: dict[str, Any],
    source_render_state: dict[str, Any],
) -> dict[str, Any]:
    fine = occurrence["fineRendererKind"]
    base_capability = BASE_CAPABILITY_BY_FINE_RENDERER[fine]
    evidence_blockers: set[str] = set(occurrence.get("blockers", []))
    feature_blockers: set[str] = set()
    required_capability = base_capability

    if occurrence.get("sourceParentBlocker") is not None:
        evidence_blockers.add("SOURCE_PARENT_RESOLUTION_BLOCKED")
    if source_render_state["status"] == "UNRESOLVED":
        evidence_blockers.add("SOURCE_RENDER_STATE_UNRESOLVED")
    elif not source_render_state["admissionUsable"]:
        evidence_blockers.add("SOURCE_RENDER_STATE_DIAGNOSTIC_ONLY")

    if source_render_state["usesDistortion"] is None and fine not in (
        "ANIM_TRAIL",
        "LIGHT_PRESENTATION",
        "STANDALONE_MESH",
    ):
        evidence_blockers.add("SOURCE_DISTORTION_USAGE_UNRESOLVED")
    elif source_render_state["usesDistortion"] is True:
        feature_blockers.add("DISTORTION_OR_NONZERO_MRT_REQUIRED")
        required_capability = fine + "_DISTORTION_MRT"

    blend = source_render_state.get("blendMode")
    if blend == "MODULATE":
        feature_blockers.add("MODULATE_BLEND_COMPILED_ADAPTER_REQUIRED")
        required_capability = fine + "_MODULATE_RT0"
    if source_render_state.get("disableDepthTest") is True:
        feature_blockers.add("DEPTH_OFF_COMPILED_ADAPTER_REQUIRED")
        required_capability = fine + "_DEPTH_OFF_RT0"

    if fine == "ANIM_TRAIL":
        feature_blockers.add("TRAIL_RIBBON_ADAPTER_REQUIRED")
    elif fine == "STANDALONE_MESH":
        feature_blockers.add("STANDALONE_MESH_ADAPTER_REQUIRED")
    elif fine == "LIGHT_PRESENTATION":
        feature_blockers.add("LIGHT_PRESENTATION_LANE_REQUIRED")

    layout_status = layout["inventoryStatus"]
    if layout_status == "UNRESOLVED":
        evidence_blockers.add("LAYOUT_UNRESOLVED")
    elif layout_status == "SOURCE_NAMES_ONLY":
        evidence_blockers.add("LAYOUT_REGISTER_BINDINGS_UNRESOLVED")
    elif layout_status == "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION":
        evidence_blockers.add("LAYOUT_PACKET_CAPACITY_EXTENSION_REQUIRED")
    if layout.get("layoutCandidateId") is None and fine != "LIGHT_PRESENTATION":
        evidence_blockers.add("LAYOUT_PACKET_NOT_MATERIALIZED")

    if descriptor["packetClosure"] == "NOT_MATERIALIZED":
        evidence_blockers.add("DESCRIPTOR_PACKET_NOT_MATERIALIZED")
    if fine in ("SPRITE_PARTICLE", "MESH_PARTICLE", "DECAL_PARTICLE"):
        if not descriptor["hasBaseResource"]:
            evidence_blockers.add("DESCRIPTOR_BASE_RESOURCE_MISSING")
        if not descriptor["valueEvidenceIds"]:
            evidence_blockers.add("SOURCE_VALUE_PACKET_EVIDENCE_ABSENT")

    if "SCENE_INPUTS_UNPROVEN" in evidence_blockers:
        evidence_blockers.add("SCENE_INPUT_REQUIREMENT_UNRESOLVED")
    if "OUTPUT_TOPOLOGY_MRT_UNPROVEN" in evidence_blockers:
        evidence_blockers.add("MRT_OUTPUT_REQUIREMENT_UNRESOLVED")

    if fine == "LIGHT_PRESENTATION":
        state = "PRESENTATION_DEFERRED"
    elif feature_blockers:
        state = "FEATURE_DEFERRED"
    elif evidence_blockers:
        state = "EVIDENCE_BLOCKED"
    else:
        state = "RT0_APPLICATION_CANDIDATE"

    blockers = sorted(feature_blockers | evidence_blockers)
    binding_identity: dict[str, Any] | None = None
    if state == "RT0_APPLICATION_CANDIDATE":
        binding_payload = {
            "occurrenceId": occurrence["occurrenceId"],
            "authoredElementSha256": occurrence["authoredElementSha256"],
            "programEvidenceId": occurrence["program"].get("programEvidenceId"),
            "layoutSignature": layout["layoutSignature"],
            "descriptorVariantId": descriptor["descriptorVariantId"],
            "sourceRenderStateId": source_render_state["renderStateId"],
            "requiredCapabilityId": required_capability,
        }
        binding_identity = {
            "status": "CANDIDATE_NOT_PUBLISHED",
            "bindingId": _content_id("effect-binding.", binding_payload),
        }

    return {
        "state": state,
        "requiredCapabilityId": required_capability,
        "blockers": blockers,
        "bindingIdentity": binding_identity,
        "vertexFidelity": (
            "VERTEX_PROGRAM_UNPROVEN"
            if "WPO_VERTEX_PROGRAM_UNPROVEN" in occurrence.get("blockers", [])
            else "NO_VERTEX_PROGRAM_BLOCKER_RECORDED"
        ),
    }


def _build_row(
    occurrence: dict[str, Any],
    carrier_by_id: dict[str, dict[str, Any]],
    composition_by_id: dict[str, dict[str, Any]],
    layout_evidence_by_id: dict[str, dict[str, Any]],
    adapter_by_id: dict[str, dict[str, Any]],
    descriptor_by_id: dict[str, dict[str, Any]],
    source_material_by_path: dict[str, dict[str, Any]],
    family_render_states: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    source_path = occurrence.get("sourceMaterialPath")
    source_material = (
        source_material_by_path.get(source_path.casefold())
        if isinstance(source_path, str)
        else None
    )
    layout = _layout_projection(occurrence, layout_evidence_by_id)
    descriptor = _descriptor_projection(occurrence, descriptor_by_id, source_material)
    source_render_state = _resolve_source_render_state(
        occurrence, source_material, family_render_states
    )
    adapter_axis = occurrence["adapter"]
    adapter = adapter_by_id.get(adapter_axis.get("adapterCandidateId"))
    current_render_profile = adapter.get("renderProfile") if adapter else None
    source_matches_current = _source_matches_current(
        source_render_state, current_render_profile
    )
    topology_class, topology_evidence = _mesh_topology(occurrence, carrier_by_id)
    program_status = occurrence["program"]["status"]
    _require(program_status in PROGRAM_FIDELITY_BY_STATUS, "unknown Program status")
    application = _application_decision(
        occurrence, layout, descriptor, source_render_state
    )
    row_identity_payload = {
        "occurrenceId": occurrence["occurrenceId"],
        "authoredElementSha256": occurrence["authoredElementSha256"],
    }
    composition_ids = occurrence.get("compositionVariantIds", [])
    _require(
        isinstance(composition_ids, list) and len(composition_ids) == 1,
        occurrence["occurrenceId"] + " must resolve to exactly one Product composition",
    )
    composition_id = composition_ids[0]
    composition_variant = composition_by_id.get(composition_id)
    _require(
        composition_variant is not None,
        occurrence["occurrenceId"] + " composition identity is dangling",
    )
    composition_semantic = composition_variant.get("semantic")
    _require(isinstance(composition_semantic, dict), occurrence["occurrenceId"] + " composition semantic is missing")
    composition_kind = composition_semantic.get("kind")
    _require(
        composition_kind in ("VALTAN_PATTERN_CUE", "VALTAN_COMBAT_OBJECT_VISUAL"),
        occurrence["occurrenceId"] + " composition is not a Valtan Product visual",
    )
    _require(
        composition_semantic.get("effectAssetId") == occurrence["effectAssetId"],
        occurrence["occurrenceId"] + " composition effect target drifted",
    )
    return {
        "applicationRowId": _content_id("valtan-rt0-application.", row_identity_payload),
        "occurrence": {
            "occurrenceId": occurrence["occurrenceId"],
            "effectAssetId": occurrence["effectAssetId"],
            "elementId": occurrence["elementId"],
            "authoredPath": occurrence["authoredPath"],
            "authoredElementSha256": occurrence["authoredElementSha256"],
            "sourceMaterialPath": source_path,
            "sourceParentMaterialPath": occurrence.get("sourceParentMaterialPath"),
            "sourceParentRowSha256": occurrence.get("sourceParentRowSha256"),
            "sourceParentBlocker": occurrence.get("sourceParentBlocker"),
        },
        "compositionIds": composition_ids,
        "composition": {
            "compositionId": composition_id,
            "kind": composition_kind,
            "bindingId": composition_semantic.get("bindingId"),
            "cueOccurrenceId": composition_semantic.get("occurrenceId"),
            "patternId": composition_semantic.get("patternId")
            or composition_semantic.get("ownerPatternId"),
            "stageId": composition_semantic.get("stageId"),
            "actionId": composition_semantic.get("actionId")
            or composition_semantic.get("ownerStageActionId"),
            "clientVisualId": composition_semantic.get("clientVisualId"),
        },
        "carrier": {
            "carrierKind": occurrence["carrier"],
            "fineRendererKind": occurrence["fineRendererKind"],
            "sourceRendererShape": occurrence.get("sourceRendererShape"),
            "sourceVertexFactoryCandidates": occurrence.get("vertexFactoryCandidates", []),
            "carrierEvidenceId": occurrence.get("carrierVariantId"),
            "topologyClass": topology_class,
            "topologyEvidence": topology_evidence,
        },
        "program": {
            "inventoryStatus": program_status,
            "fidelity": PROGRAM_FIDELITY_BY_STATUS[program_status],
            "programCandidateId": occurrence["program"].get("programCandidateId"),
            "programEvidenceId": occurrence["program"].get("programEvidenceId"),
        },
        "layout": layout,
        "descriptor": descriptor,
        "adapter": {
            "inventoryStatus": adapter_axis.get("status"),
            "currentAdapterCandidateId": adapter_axis.get("adapterCandidateId"),
            "currentRenderProfile": current_render_profile,
            "sourceRenderState": source_render_state,
            "sourceStateMatchesCurrent": source_matches_current,
        },
        "application": application,
        "manualProductReview": "PENDING_USER_PRODUCT_REVIEW",
    }


def _summary(rows: list[dict[str, Any]]) -> dict[str, Any]:
    def nested_counts(value_getter) -> dict[str, dict[str, int]]:
        by_carrier: dict[str, Counter[str]] = {}
        for row in rows:
            carrier = row["carrier"]["carrierKind"]
            by_carrier.setdefault(carrier, Counter())[value_getter(row)] += 1
        return {
            carrier: dict(sorted(counts.items()))
            for carrier, counts in sorted(by_carrier.items())
        }

    carrier_counts = Counter(row["carrier"]["carrierKind"] for row in rows)
    fine_counts = Counter(row["carrier"]["fineRendererKind"] for row in rows)
    program_counts = Counter(row["program"]["fidelity"] for row in rows)
    layout_counts = Counter(row["layout"]["inventoryStatus"] for row in rows)
    descriptor_counts = Counter(row["descriptor"]["evidenceStatus"] for row in rows)
    source_state_counts = Counter(
        row["adapter"]["sourceRenderState"]["renderStateId"] for row in rows
    )
    application_counts = Counter(row["application"]["state"] for row in rows)
    pattern_counts = Counter(row["composition"]["patternId"] for row in rows)
    pattern_stage_counts = Counter(
        row["composition"]["patternId"]
        + "/"
        + (row["composition"]["stageId"] or "COMBAT_OBJECT_VISUAL")
        for row in rows
    )
    mismatch_count = sum(
        row["adapter"]["sourceStateMatchesCurrent"] is False for row in rows
    )
    known_comparison_count = sum(
        row["adapter"]["sourceStateMatchesCurrent"] is not None for row in rows
    )
    return {
        "rowCount": len(rows),
        "carrierCounts": dict(sorted(carrier_counts.items())),
        "fineRendererCounts": dict(sorted(fine_counts.items())),
        "productPatternCount": len(pattern_counts),
        "patternOccurrenceCounts": dict(sorted(pattern_counts.items())),
        "patternStageOccurrenceCounts": dict(sorted(pattern_stage_counts.items())),
        "programFidelityCounts": dict(sorted(program_counts.items())),
        "programFidelityByCarrierCounts": nested_counts(
            lambda row: row["program"]["fidelity"]
        ),
        "layoutStatusCounts": dict(sorted(layout_counts.items())),
        "descriptorEvidenceCounts": dict(sorted(descriptor_counts.items())),
        "sourceRenderStateCounts": dict(sorted(source_state_counts.items())),
        "sourceRenderStateByCarrierCounts": nested_counts(
            lambda row: row["adapter"]["sourceRenderState"]["renderStateId"]
        ),
        "sourceCurrentComparableCount": known_comparison_count,
        "sourceCurrentMismatchCount": mismatch_count,
        "applicationStateCounts": dict(sorted(application_counts.items())),
        "applicationStateByCarrierCounts": nested_counts(
            lambda row: row["application"]["state"]
        ),
        "bindingIdentityCount": sum(
            row["application"]["bindingIdentity"] is not None for row in rows
        ),
        "unresolvedWithoutDiagnosisCount": sum(
            row["application"]["state"] != "RT0_APPLICATION_CANDIDATE"
            and not row["application"]["blockers"]
            for row in rows
        ),
    }


def validate_application(document: dict[str, Any]) -> None:
    expected_root = {
        "schema",
        "formatVersion",
        "identity",
        "inputs",
        "policy",
        "summary",
        "rows",
        "transaction",
        "artifactSha256",
    }
    _require(set(document) == expected_root, "application root fields drifted")
    _require(document.get("schema") == SCHEMA, "application schema drifted")
    _require(document.get("formatVersion") == FORMAT_VERSION, "application version drifted")
    _require(
        document.get("identity")
        == "APPLICATION_DECISIONS_ONLY_NO_RUNTIME_BINDING_AUTHORITY",
        "application identity overstates runtime authority",
    )
    digest = _require_sha256(document.get("artifactSha256"), "artifactSha256")
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    _require(canonical_sha256(unsigned) == digest, "artifactSha256 drifted")

    inputs = document.get("inputs")
    _require(isinstance(inputs, list) and len(inputs) == 4, "inputs must contain four seals")
    paths: set[str] = set()
    for row in inputs:
        _require(isinstance(row, dict), "input seal must be object")
        path = row.get("path")
        _require(isinstance(path, str) and path not in paths, "input path duplicated")
        paths.add(path)
        _require_sha256(row.get("rawSha256"), "input rawSha256 invalid")
        _require(isinstance(row.get("byteSize"), int) and row["byteSize"] > 0, "input size invalid")
        artifact_digest = row.get("artifactSha256")
        _require(artifact_digest is None or SHA256_RE.fullmatch(artifact_digest) is not None, "input artifactSha256 invalid")

    policy = document.get("policy")
    _require(isinstance(policy, dict), "policy must be object")
    _require(policy.get("runtimeBindingAuthority") == "NONE", "ledger cannot grant binding authority")
    _require(
        policy.get("registryBindingPublication")
        == "SEPARATE_VALTAN_REGISTRY_FRAGMENT_REQUIRED",
        "ledger cannot publish registry bindings",
    )
    _require(policy.get("sourceRenderStatePrecedence") == [
        "SEALED_CHILD_PARENT_DECLARATION",
        "PARENT_FAMILY_PROPS_FALLBACK",
        "UNRESOLVED_FAIL_CLOSED",
    ], "source render-state precedence drifted")

    rows = document.get("rows")
    _require(isinstance(rows, list) and len(rows) == EXPECTED_ROW_COUNT, "row count drifted")
    row_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    target_keys: set[tuple[str, str]] = set()
    binding_ids: set[str] = set()
    for row in rows:
        _require(isinstance(row, dict), "application row must be object")
        row_id = _require_stable_id(row.get("applicationRowId"), "applicationRowId invalid")
        _require(row_id not in row_ids, "duplicate applicationRowId")
        row_ids.add(row_id)
        occurrence = row.get("occurrence")
        _require(isinstance(occurrence, dict), row_id + " occurrence missing")
        occurrence_id = _require_stable_id(occurrence.get("occurrenceId"), row_id + " occurrenceId invalid")
        _require(occurrence_id not in occurrence_ids, "duplicate occurrenceId")
        occurrence_ids.add(occurrence_id)
        target = (occurrence.get("effectAssetId"), occurrence.get("elementId"))
        _require(all(isinstance(value, str) and value for value in target), row_id + " target invalid")
        _require(target not in target_keys, "duplicate authored target")
        target_keys.add(target)
        expected_row_id = _content_id(
            "valtan-rt0-application.",
            {
                "occurrenceId": occurrence_id,
                "authoredElementSha256": occurrence.get("authoredElementSha256"),
            },
        )
        _require(row_id == expected_row_id, row_id + " identity drifted")

        composition_ids = row.get("compositionIds")
        composition = row.get("composition")
        _require(
            isinstance(composition_ids, list) and len(composition_ids) == 1,
            row_id + " must carry exactly one Product composition ID",
        )
        _require(isinstance(composition, dict), row_id + " composition missing")
        _require(
            composition.get("compositionId") == composition_ids[0],
            row_id + " composition identity drifted",
        )
        _require(
            composition.get("kind")
            in ("VALTAN_PATTERN_CUE", "VALTAN_COMBAT_OBJECT_VISUAL"),
            row_id + " composition kind invalid",
        )
        for field in ("patternId", "actionId"):
            _require_stable_id(composition.get(field), row_id + " composition " + field + " invalid")
        for field in ("bindingId", "cueOccurrenceId", "stageId", "clientVisualId"):
            value = composition.get(field)
            _require(
                value is None
                or (isinstance(value, str) and STABLE_ID_RE.fullmatch(value) is not None),
                row_id + " composition " + field + " invalid",
            )
        if composition["kind"] == "VALTAN_PATTERN_CUE":
            _require(
                all(composition[field] is not None for field in ("bindingId", "cueOccurrenceId", "stageId"))
                and composition["clientVisualId"] is None,
                row_id + " pattern cue composition fields drifted",
            )
        else:
            _require(
                composition["bindingId"] is None
                and composition["cueOccurrenceId"] is None
                and composition["stageId"] is None
                and composition["clientVisualId"] is not None,
                row_id + " combat-object composition fields drifted",
            )

        source = row.get("adapter", {}).get("sourceRenderState")
        _require(isinstance(source, dict), row_id + " source render state missing")
        if occurrence.get("sourceParentBlocker") is not None:
            _require(source.get("admissionUsable") is False, row_id + " blocked parent became admissible")
        if source.get("status") == "UNRESOLVED":
            _require(source.get("admissionUsable") is False, row_id + " unresolved state became admissible")

        application = row.get("application")
        _require(isinstance(application, dict), row_id + " application missing")
        state = application.get("state")
        _require(state in APPLICATION_STATES, row_id + " application state invalid")
        blockers = application.get("blockers")
        _require(isinstance(blockers, list) and blockers == sorted(set(blockers)), row_id + " blockers invalid")
        binding = application.get("bindingIdentity")
        if state == "RT0_APPLICATION_CANDIDATE":
            _require(not blockers, row_id + " candidate has blockers")
            _require(isinstance(binding, dict), row_id + " candidate lacks binding identity")
            binding_id = _require_stable_id(binding.get("bindingId"), row_id + " bindingId invalid")
            _require(binding.get("status") == "CANDIDATE_NOT_PUBLISHED", row_id + " binding status invalid")
            _require(binding_id not in binding_ids, "duplicate bindingId")
            binding_ids.add(binding_id)
        else:
            _require(blockers, row_id + " deferred/blocked row lacks diagnosis")
            _require(binding is None, row_id + " deferred/blocked row issued binding")
        if row.get("descriptor", {}).get("packetClosure") == "NOT_MATERIALIZED":
            _require(binding is None, row_id + " unmaterialized packet issued binding")
        _require(row.get("manualProductReview") == "PENDING_USER_PRODUCT_REVIEW", row_id + " review overstated")

    computed_summary = _summary(rows)
    _require(document.get("summary") == computed_summary, "summary drifted")
    _require(computed_summary["rowCount"] == EXPECTED_ROW_COUNT, "summary row count drifted")
    _require(computed_summary["carrierCounts"] == EXPECTED_CARRIER_COUNTS, "carrier counts drifted")
    _require(computed_summary["fineRendererCounts"] == EXPECTED_FINE_RENDERER_COUNTS, "fine renderer counts drifted")
    _require(computed_summary["productPatternCount"] == 24, "Product pattern count drifted")
    _require(
        computed_summary["patternOccurrenceCounts"]
        == EXPECTED_PATTERN_OCCURRENCE_COUNTS,
        "Product pattern occurrence counts drifted",
    )
    _require(sum(computed_summary["applicationStateCounts"].values()) == EXPECTED_ROW_COUNT, "application states do not close")
    _require(computed_summary["unresolvedWithoutDiagnosisCount"] == 0, "undiagnosed rows remain")
    _require(computed_summary["bindingIdentityCount"] == len(binding_ids), "binding summary drifted")

    transaction = document.get("transaction")
    _require(
        transaction
        == {
            "mode": "BUILD_VALIDATE_ATOMIC_REPLACE",
            "partialCommitAllowed": False,
            "runtimeMutation": False,
        },
        "transaction policy drifted",
    )


def build_application(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    tuple_inventory, tuple_input = _load_input(
        repository_root,
        TUPLE_INVENTORY_PATH,
        ("PRODUCT_OCCURRENCE_DENOMINATOR", "PROGRAM_LAYOUT_DESCRIPTOR_ADAPTER_EVIDENCE"),
        artifact_schema="lostark.effect-tuple-cohort-inventory",
    )
    source_materials, source_input = _load_input(
        repository_root,
        SOURCE_MATERIAL_EVIDENCE_PATH,
        ("SOURCE_CHILD_VALUES", "SOURCE_PARENT_DEFAULTS", "SOURCE_RENDER_STATE"),
    )
    _require(source_materials.get("schema") == "lostark.valtan-source-material-evidence", "source material schema drifted")
    _require(source_materials.get("formatVersion") == 1, "source material version drifted")
    family_manifest, family_input = _load_input(
        repository_root,
        FAMILY_MANIFEST_PATH,
        ("PARENT_RENDER_STATE_FALLBACK",),
        artifact_schema="lostark.effect-family-manifest",
    )
    missing_families, missing_input = _load_input(
        repository_root,
        MISSING_FAMILY_INVENTORY_PATH,
        ("PARENT_RENDER_STATE_FALLBACK",),
        artifact_schema="lostark.effect-missing-family-inventory",
    )

    product_occurrences = [
        row
        for row in tuple_inventory.get("occurrences", [])
        if row.get("domain") == "Valtan"
        and row.get("scopeBits", {}).get("productConsumed") is True
        and row.get("productStatus") == "PRODUCT_JOIN_CLOSED"
    ]
    _require(len(product_occurrences) == EXPECTED_ROW_COUNT, "Valtan Product denominator drifted")
    _unique_index(product_occurrences, "occurrenceId", "Valtan Product occurrences")

    carrier_by_id = _unique_index(tuple_inventory.get("carrierVariants", []), "carrierVariantId", "carrier variants")
    composition_by_id = _unique_index(
        tuple_inventory.get("compositionVariants", []),
        "compositionVariantId",
        "composition variants",
    )
    layout_evidence_by_id = _unique_index(tuple_inventory.get("layoutEvidence", []), "layoutEvidenceId", "layout evidence")
    adapter_by_id = _unique_index(tuple_inventory.get("adapterCandidates", []), "adapterCandidateId", "adapter candidates")
    descriptor_by_id = _unique_index(tuple_inventory.get("descriptorVariants", []), "descriptorVariantId", "descriptor variants")

    source_material_rows = source_materials.get("materials")
    _require(isinstance(source_material_rows, list), "source materials array missing")
    source_material_by_exact_path = _unique_index(source_material_rows, "sourceMaterialPath", "source materials")
    source_material_by_path = {
        key.casefold(): value for key, value in source_material_by_exact_path.items()
    }
    _require(len(source_material_by_path) == len(source_material_by_exact_path), "case-folded source material paths collide")
    family_render_states = _family_render_state_index(family_manifest, missing_families)

    rows = [
        _build_row(
            occurrence,
            carrier_by_id,
            composition_by_id,
            layout_evidence_by_id,
            adapter_by_id,
            descriptor_by_id,
            source_material_by_path,
            family_render_states,
        )
        for occurrence in sorted(product_occurrences, key=lambda row: row["occurrenceId"])
    ]
    application = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": "APPLICATION_DECISIONS_ONLY_NO_RUNTIME_BINDING_AUTHORITY",
        "inputs": sorted(
            (tuple_input, source_input, family_input, missing_input),
            key=lambda row: row["path"],
        ),
        "policy": {
            "runtimeBindingAuthority": "NONE",
            "registryBindingPublication": "SEPARATE_VALTAN_REGISTRY_FRAGMENT_REQUIRED",
            "sourceRenderStatePrecedence": [
                "SEALED_CHILD_PARENT_DECLARATION",
                "PARENT_FAMILY_PROPS_FALLBACK",
                "UNRESOLVED_FAIL_CLOSED",
            ],
            "blockedParentRenderStateUse": "DIAGNOSTIC_ONLY",
            "unknownDistortionUse": "NOT_FALSE_FAIL_CLOSED",
            "descriptorPacketPolicy": "SOURCE_VALUES_REQUIRE_TYPED_PACKET_MATERIALIZATION",
            "manualProductReview": "REQUIRED_PER_OCCURRENCE",
        },
        "summary": _summary(rows),
        "rows": rows,
        "transaction": {
            "mode": "BUILD_VALIDATE_ATOMIC_REPLACE",
            "partialCommitAllowed": False,
            "runtimeMutation": False,
        },
    }
    application["artifactSha256"] = canonical_sha256(application)
    validate_application(application)
    return application


def _write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(handle, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the canonical artifact is stale")
    parser.add_argument("--output", type=Path, default=None, help="override the output path")
    arguments = parser.parse_args(argv)
    try:
        document = build_application(REPOSITORY_ROOT)
        payload = pretty_json_bytes(document)
        output_path = arguments.output or (REPOSITORY_ROOT / OUTPUT_RELATIVE_PATH)
        if arguments.check:
            _require(output_path.is_file(), str(output_path) + " is missing")
            _require(output_path.read_bytes() == payload, str(output_path) + " is stale")
            print("Valtan horizontal RT0 application ledger is current: " + str(output_path))
            return 0
        _write_atomic(output_path, payload)
        print("Wrote " + str(output_path))
        return 0
    except (ApplicationError, OSError, ValueError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
